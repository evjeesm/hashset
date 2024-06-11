#include "../src/hashset.h"
#include <check.h>
#include <stdio.h>
#include <stdlib.h>

static hashset_t *set;
static hashset_t *other;

static hash_t hash_int(const void *ptr, size_t size)
{
    (void)size;
    return hash(*(int*)ptr);
}

static void setup_empty(void)
{
    set = hs_create(.value_size = sizeof(int),
        .hashfunc = hash_int
    );
}

static void setup_full(void)
{
    const size_t cap = 100;

    set = hs_create(.value_size = sizeof(int),
        .hashfunc = hash_int,
        .initial_cap = cap
    );

    // full capacity
    for (int i = 0; i < (int)cap; ++i)
    {
        ck_assert_uint_eq(HS_SUCCESS, hs_insert(&set, &i));
    }
}

static void setup_two_sets(void)
{
    const size_t cap = 10;
    set = hs_create(
        .value_size = sizeof(int),
        .hashfunc = hash_int,
        .initial_cap = cap
    );

    /* 1 - 10 */
    for (int i = 1; i <= 10; ++i)
    {
        ck_assert_uint_eq(HS_SUCCESS, hs_insert(&set, &i));
    }

    other = hs_create(
        .value_size = sizeof(int),
        .hashfunc = hash_int,
        .initial_cap = cap
    );

    /* 5 - 15 */
    for (int i = 5; i <= 15; ++i)
    {
        ck_assert_uint_eq(HS_SUCCESS, hs_insert(&other, &i));
    }
}

static void teardown(void)
{
    hs_destroy(set);
}

static void teardown_two_sets(void)
{
    hs_destroy(set);
    hs_destroy(other);
}

START_TEST (test_hs_create)
{
    ck_assert_ptr_nonnull(set);
    ck_assert_uint_eq(hs_count(set), 0);
}
END_TEST


START_TEST (test_hs_insert)
{
    const int value = 100;
    hs_status_t status = hs_insert(&set, &value);
    ck_assert_uint_eq(HS_SUCCESS, status);

    status = hs_insert(&set, &value);
    ck_assert_uint_eq(HS_ALREADY_EXISTS, status);
}
END_TEST


START_TEST (test_hs_insert_full)
{
    int cap = hs_capacity(set);

    // full capacity
    for (int i = 0; i < cap; ++i)
    {
        ck_assert_uint_eq(HS_SUCCESS, hs_insert(&set, &i));
    }

    cap = hs_capacity(set);
    size_t count = hs_count(set);
    ck_assert_uint_eq(count, cap);
}
END_TEST


START_TEST (test_hs_insert_unique)
{
    int element = 10;

    hs_insert(&set, &element);
    hs_insert(&set, &element);
    hs_insert(&set, &element);
    ck_assert_uint_eq(hs_count(set), 1);

}
END_TEST


START_TEST (test_hs_insert_rehash)
{
    int old_cap = hs_capacity(set);

    // full capacity
    for (int i = 0; i < old_cap; ++i)
    {
        ck_assert_uint_eq(HS_SUCCESS, hs_insert(&set, &i));
    }

    int value = 999;
    hs_insert(&set, &value);

    int new_cap = hs_capacity(set);
    size_t count = hs_count(set);

    ck_assert_uint_eq(new_cap, 512);
    ck_assert_uint_eq(count, old_cap + 1);

    // check that all elements rehashed properly
    for (int i = 0; i < old_cap; ++i)
    {
        ck_assert(hs_contains(set, &i));
    }
    ck_assert(hs_contains(set, &value));
}
END_TEST


static bool exact(const void *const element, void *const param)
{
    return *(int*) element == *(int*) param;
}

START_TEST (test_hs_values)
{
    const int amount = 3;
    for (int i = 0; i < amount; ++i)
    {
        hs_insert(&set, &i);
    }

    vector_t *values = hs_values(set);

    ck_assert_ptr_nonnull(values);
    ck_assert_uint_eq(vector_capacity(values), amount);

    for (int i = 0; i < amount; ++i)
    {
        vector_linear_find(values, amount, exact, &i);
    }

    vector_destroy(values);
}
END_TEST


/****************************************************
*  Test Case: Remove
*   (use with `setup_full` fixture)
****************************************************/

START_TEST (test_hs_remove)
{
    const int cap = (int)hs_capacity(set);

    // delete all
    for (int i = 0; i < cap; ++i)
    {
        hs_remove(set, &i);
        ck_assert(!hs_contains(set, &i));
    }

    ck_assert_uint_eq(hs_count(set), 0);
}
END_TEST


static bool even(const void *const element, void *param)
{
    (void) param;
    return *((int*)element) % 2 == 0;
}

START_TEST (test_hs_remove_many)
{
    const size_t cap = hs_capacity(set);
    size_t remove_count = hs_remove_many(set, even, NULL); /* removes all even numbers */

    ck_assert_uint_eq(remove_count, cap / 2);
    ck_assert_uint_eq(hs_count(set), cap / 2);

    ck_assert(hs_contains(set, TMP_REF(int, 1)));
    ck_assert(!hs_contains(set, TMP_REF(int, 0)));
}
END_TEST


/****************************************************
*  Test Case: Operations
*   (use with `setup_two_sets` and `teardown_two_sets` fixture)
****************************************************/

START_TEST (test_hs_add)
{
    ck_assert_uint_eq(HS_SUCCESS, hs_add(&set, other));

    /* includes both ranges */
    for (int i = 1; i <= 15; ++i)
    {
        ck_assert(hs_contains(set, &i));
    }
    ck_assert_uint_eq(hs_count(set), 15);
}
END_TEST


START_TEST (test_hs_intersect)
{
    hs_intersect(&set, other);

    /* shared range [5, 10] */
    for (int i = 5; i <= 10; ++i)
    {
        ck_assert(hs_contains(set, &i));
    }
    ck_assert_uint_eq(hs_count(set), 6);
}
END_TEST


START_TEST (test_hs_subtract)
{
    hs_subtract(&set, other);

    /* [1 - 10] - [5 - 15] => [1 - 4] */
    for (int i = 1; i <= 4; ++i)
    {
        ck_assert(hs_contains(set, &i));
    }
    ck_assert_uint_eq(hs_count(set), 4);
}
END_TEST


START_TEST (test_hs_make_symdiff)
{
    hashset_t *symdiff = hs_make_symdiff(set, other);
    ck_assert_ptr_nonnull(symdiff);

    for (int i = 1; i <= 4; ++i)
    {
        ck_assert(hs_contains(symdiff, &i));
    }

    for (int i = 11; i <= 15; ++i)
    {
        ck_assert(hs_contains(symdiff, &i));
    }

    ck_assert_uint_eq(hs_count(symdiff), 9);
    hs_destroy(symdiff);
}
END_TEST



Suite *hash_set_suite(void)
{
    Suite *s;
    TCase *tc_core, *tc_remove, *tc_operations;

    s = suite_create("Hash Map");
    
    /* Core test case */
    tc_core = tcase_create("Core");
    tcase_add_checked_fixture(tc_core, setup_empty, teardown);
    tcase_add_test(tc_core, test_hs_create);
    tcase_add_test(tc_core, test_hs_insert);
    tcase_add_test(tc_core, test_hs_insert_unique);
    tcase_add_test(tc_core, test_hs_insert_full);
    tcase_add_test(tc_core, test_hs_insert_rehash);
    tcase_add_test(tc_core, test_hs_values);
    suite_add_tcase(s, tc_core);

    tc_remove = tcase_create("Remove");
    tcase_add_checked_fixture(tc_remove, setup_full, teardown);
    tcase_add_test(tc_remove, test_hs_remove);
    tcase_add_test(tc_remove, test_hs_remove_many);

    suite_add_tcase(s, tc_remove);

    tc_operations = tcase_create("Operations");
    tcase_add_checked_fixture(tc_operations, setup_two_sets, teardown_two_sets);
    tcase_add_test(tc_operations, test_hs_add);
    tcase_add_test(tc_operations, test_hs_intersect);
    tcase_add_test(tc_operations, test_hs_subtract);
    tcase_add_test(tc_operations, test_hs_make_symdiff);

    suite_add_tcase(s, tc_operations);

    return s;
}


int main(void)
{
    int number_failed;
    Suite *s;
    SRunner *sr;

    s = hash_set_suite();
    sr = srunner_create(s);

    /* srunner_set_fork_status(sr, CK_NOFORK); */
    srunner_run_all(sr, CK_NORMAL);
    number_failed = srunner_ntests_failed(sr);
    srunner_free(sr);

    return (number_failed == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}


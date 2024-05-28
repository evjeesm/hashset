#include "../src/hashset.h"
#include <check.h>
#include <stdio.h>
#include <stdlib.h>

static hashset_t *set;

static hash_t hash_int(const void *ptr, size_t size)
{
    (void)size;
    return hash(*(int*)ptr);
}

static void setup_empty(void)
{
    hs_create(set,
        .value_size = sizeof(int),
        .hashfunc = hash_int
    );
}

static void setup_full(void)
{
    const size_t cap = 100;

    hs_create(set,
        .value_size = sizeof(int),
        .hashfunc = hash_int,
        .initial_cap = cap
    );
    
    // full capacity
    for (int i = 0; i < (int)cap; ++i)
    {
        ck_assert(hs_insert(&set, &i));
    }
}

static void teardown(void)
{
    hs_destroy(set);
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
    bool retval = hs_insert(&set, &value);
    ck_assert(retval);

    retval = hs_insert(&set, &value);
    ck_assert(!retval);
}
END_TEST


START_TEST (test_hs_insert_full)
{
    int cap = hs_capacity(set);

    // full capacity
    for (int i = 0; i < cap; ++i)
    {
        ck_assert(hs_insert(&set, &i));
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
        ck_assert(hs_insert(&set, &i));
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


// START_TEST (test_hs_values)
// {
//     const int expected_cap = 10;
//     for (int key = 0; key < expected_cap; ++key)
//     {
//         int val = key + 10;
//         ck_assert(hs_insert(&set, &key, &val));
//     }
//
//     vector_t *keys = hs_keys(set);
//     vector_t *values = hs_values(set);
//
//     ck_assert_uint_eq(vector_capacity(keys), expected_cap);
//     ck_assert_uint_eq(vector_capacity(values), expected_cap);
//
//     for (size_t i = 0; i < vector_capacity(keys); ++i)
//     {
//         int key = *(int*)vector_get(keys, i);
//         int value = *(int*)vector_get(values, i);
//
//         ck_assert_int_eq(value, key + 10);
//     }
//
//     vector_destroy(keys);
//     vector_destroy(values);
// }
// END_TEST

Suite *hash_set_suite(void)
{
    Suite *s;
    TCase *tc_core, *tc_remove;

    s = suite_create("Hash Map");
    
    /* Core test case */
    tc_core = tcase_create("Core");

    tcase_add_checked_fixture(tc_core, setup_empty, teardown);
    tcase_add_test(tc_core, test_hs_create);
    tcase_add_test(tc_core, test_hs_insert);
    tcase_add_test(tc_core, test_hs_insert_unique);
    tcase_add_test(tc_core, test_hs_insert_full);
    tcase_add_test(tc_core, test_hs_insert_rehash);
    suite_add_tcase(s, tc_core);

    tc_remove = tcase_create("Remove");

    tcase_add_checked_fixture(tc_remove, setup_full, teardown);
    tcase_add_test(tc_remove, test_hs_remove);
    tcase_add_test(tc_remove, test_hs_remove_many);
    // tcase_add_test(tc_remove, test_hs_values);
    
    suite_add_tcase(s, tc_remove);

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


#include "../src/hashset.h"
#include <check.h>
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
//
//
// START_TEST (test_hs_insert_full)
// {
//     int cap = hs_capacity(set);
//
//     // full capacity
//     for (int i = 0; i < cap; ++i)
//     {
//         ck_assert(hs_insert(&set, &i));
//     }
//
//     cap = hs_capacity(set);
//     size_t count = hs_count(set);
//     ck_assert_uint_eq(count, cap);
// }
// END_TEST
//
//
// START_TEST (test_hs_insert_rehash)
// {
//     int old_cap = hs_capacity(set);
//
//     // full capacity
//     for (int i = 0; i < old_cap; ++i)
//     {
//         ck_assert(hs_insert(&set, &i, &i));
//     }
//
//     int value = 999;
//     hs_insert(&set, &value);
//
//     int new_cap = hs_capacity(set);
//     size_t count = hs_count(set);
//
//     ck_assert_uint_eq(new_cap, 512);
//     ck_assert_uint_eq(count, old_cap + 1);
//
//     // check that all elements rehashed properly
//     for (int i = 0; i < old_cap; ++i)
//     {
//         ck_assert_mem_eq(hs_get(set, &i), &i, sizeof(int));
//     }
//     ck_assert_mem_eq(hs_get(set, &value), &value, sizeof(int));
// }
// END_TEST
//
//
// START_TEST (test_hs_remove)
// {
//     const int key = 534;
//     const int value = 12;
//     ck_assert(hs_insert(&set, &key, &value));
//     hs_remove(set, &key);
//     ck_assert_ptr_null(hs_get(set, &key));
//     ck_assert_uint_eq(hs_count(set), 0);
//
//     const int cap = (int)hs_capacity(set);
//     // full capacity
//     for (int i = 0; i < cap; ++i)
//     {
//         ck_assert(hs_insert(&set, &i, &i));
//     }
//
//     // delete all
//     for (int i = 0; i < cap; ++i)
//     {
//         hs_remove(set, &i);
//         ck_assert_ptr_null(hs_get(set, &i));
//     }
//
//     ck_assert_uint_eq(hs_count(set), 0);
// }
// END_TEST
//
//
// START_TEST (test_hs_keys_values)
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
    TCase *tc_core;

    s = suite_create("Hash Map");
    
    /* Core test case */
    tc_core = tcase_create("Core");

    tcase_add_checked_fixture(tc_core, setup_empty, teardown);
    tcase_add_test(tc_core, test_hs_create);
    tcase_add_test(tc_core, test_hs_insert);
    // tcase_add_test(tc_core, test_hs_insert_full);
    // tcase_add_test(tc_core, test_hs_insert_rehash);
    // tcase_add_test(tc_core, test_hs_remove);
    // tcase_add_test(tc_core, test_hs_keys_values);
    //
    suite_add_tcase(s, tc_core);

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


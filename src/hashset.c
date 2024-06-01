#include "hashset.h"
#include "bitset.h"
#include <assert.h>
#include <stdlib.h>
#include <string.h>

#define ALIGNMENT sizeof(size_t)
#define BIT_FIELD_LEN 2
#define LARGE_PRIME 0x7fffffffu

typedef struct hs_header
{
    size_t value_size;
    hashfunc_t hashfunc;

    unsigned int a; /* random factors for multiplicative hashing */
    unsigned int b;

    char usage_tbl[];
}
hs_header_t;

typedef enum hs_slot_status
{
    HS_SLOT_UNUSED = 0,
    HS_SLOT_USED,
    HS_SLOT_DELETED
}
hs_slot_status_t;


/***                          ***
* === forward declarations  === *
***                          ***/

static size_t calc_aligned_size(const size_t size, const size_t alignment);
static size_t calc_usage_tbl_size(const size_t capacity);

static hs_header_t *get_hs_header(const hashset_t *const set);

static size_t hash_to_index(const hs_header_t *header, const hash_t hash, const size_t capacity);
static void set_value(hashset_t *const set, const size_t index, const void *const value);
static char *get_value(const hashset_t *const set, const size_t index);

static void randomize_factors(hs_header_t *const header);
static void rehash(hashset_t **const set);
static bool contained_in(const void *const element, void *const param);
static bool not_contained_in(const void *const element, void *const param);
static bool contained_in_both(const void *const element, void *const param);


/***                       ***
* === API implementation === *
***                       ***/

void hs_create_(hashset_t **const set, const hs_opts_t *const opts)
{
    assert(opts->value_size && "value_size wasn't provided");
    assert(opts->hashfunc && "hashfunc wasn't provided");

    const size_t aligned_value_size = calc_aligned_size(opts->value_size, ALIGNMENT);
    const size_t usage_tbl_size = calc_usage_tbl_size(opts->initial_cap);

    /* allocate storage for hashset */
    vector_create(*set,
        .data_offset = sizeof(hs_header_t) + usage_tbl_size,
        .initial_cap = opts->initial_cap,
        .element_size = aligned_value_size
    );

    /* initializing hashset related data */
    hs_header_t *header = get_hs_header(*set);

    *header = (hs_header_t){
       .value_size = opts->value_size,
       .hashfunc = opts->hashfunc
    };

    bitset_init(header->usage_tbl, usage_tbl_size);
    randomize_factors(header);
}


hashset_t *hs_clone(const hashset_t *const set)
{
    assert(set);
    return vector_clone(set);
}


void hs_destroy(hashset_t *const set)
{
    assert(set);
    vector_destroy(set);
}


bool hs_contains(const hashset_t *const set, const void *const value)
{
    assert(set);
    assert(value);

    const hs_header_t* header = get_hs_header(set);
    const size_t capacity = hs_capacity(set);
    const size_t start_index = hash_to_index(header,
        header->hashfunc(value, header->value_size),
        capacity);

    for (size_t i = 0; i < capacity; ++i)
    {
        const size_t index = (i + start_index) % capacity;
        const hs_slot_status_t slot_stat = bitset_test(header->usage_tbl, BIT_FIELD_LEN, index);
        switch (slot_stat)
        {
            case HS_SLOT_UNUSED: return false;

            case HS_SLOT_USED:
                if (0 == memcmp(get_value(set, index), value, header->value_size))
                {
                    return true;
                }
                break;

            case HS_SLOT_DELETED: continue;
        }
    }

    return false;
}


bool hs_insert(hashset_t **set, const void *const value)
{
    assert(set && *set);
    assert(value);

    hs_header_t* header = get_hs_header(*set);
    const size_t capacity = hs_capacity(*set);
    const size_t start_index = hash_to_index(header,
        header->hashfunc(value, header->value_size),
        capacity);

    for (size_t i = 0; i < capacity; ++i)
    {
        const size_t index = (i + start_index) % capacity;
        const hs_slot_status_t slot_stat = bitset_test(header->usage_tbl, BIT_FIELD_LEN, index);
        if (HS_SLOT_USED != slot_stat)
        {
            bitset_set(header->usage_tbl, BIT_FIELD_LEN, index, HS_SLOT_USED);
            set_value(*set, index, value);
            return true;
        }
        else if (0 == memcmp(value, get_value(*set, index), header->value_size))
        {
            return false;
        }
    }

    rehash(set); /* rehash never fails unless allocation error
                    occires in which case vector_error_handler will exit from the application. */
    (void)hs_insert(set, value);
    return true;
}


void hs_remove(hashset_t *const set, const void *const value)
{
    assert(set);
    assert(value);

    hs_header_t* header = get_hs_header(set);
    const size_t capacity = vector_capacity(set);
    const size_t start_index = hash_to_index(header,
        header->hashfunc(value, header->value_size),
        capacity);

    for (size_t i = 0; i < capacity; ++i)
    {
        const size_t index = (i + start_index) % capacity;
        const hs_slot_status_t slot_stat = bitset_test(header->usage_tbl, BIT_FIELD_LEN, index);
        switch (slot_stat)
        {
            case HS_SLOT_UNUSED:
                return;

            case HS_SLOT_USED:
                if (0 == memcmp(value, get_value(set, index), header->value_size))
                {
                    bitset_set(header->usage_tbl, BIT_FIELD_LEN, index, HS_SLOT_DELETED);
                    return;
                }
                break;

            case HS_SLOT_DELETED:
                continue;
        }
    }
}


size_t hs_remove_many(hashset_t *const set, const predicate_t predicate, void *const param)
{
    assert(set);
    assert(predicate);

    hs_header_t *header = get_hs_header(set);
    const size_t capacity = hs_capacity(set);
    size_t removes = 0;

    for (size_t i = 0; i < capacity; ++i)
    {
        if (HS_SLOT_USED == bitset_test(header->usage_tbl, BIT_FIELD_LEN, i)
            && predicate(get_value(set, i), param))
        {
            bitset_set(header->usage_tbl, BIT_FIELD_LEN, i, HS_SLOT_DELETED);
            ++removes;
        }
    }

    return removes;
}


void hs_add(hashset_t **const set, const hashset_t *const other)
{
    assert(set && *set);
    assert(other);

    hs_header_t *other_header = get_hs_header(other);
    const size_t other_cap = hs_capacity(other);

    for (size_t i = 0; i < other_cap; ++i)
    {
        if (HS_SLOT_USED == bitset_test(other_header->usage_tbl, BIT_FIELD_LEN, i))
        {
            (void) hs_insert(set, get_value(other, i));
        }
    }
}


void hs_intersect(hashset_t **const set, const hashset_t *const other)
{
    assert(set && *set);
    assert(other);

    (void) hs_remove_many(*set, not_contained_in, (void*)other);
}


void hs_subtract(hashset_t **const set, const hashset_t *const other)
{
    assert(set && *set);
    assert(other);

    (void) hs_remove_many(*set, contained_in, (void*)other);
}


hashset_t *hs_make_union(const hashset_t *const first, const hashset_t *const second)
{
    assert(first);
    assert(second);

    hashset_t *result = hs_clone(first);
    hs_add(&result, second);
    return result;
}


hashset_t *hs_make_intersection(const hashset_t *const first, const hashset_t *const second)
{
    assert(first);
    assert(second);

    hashset_t *result = hs_clone(first);
    hs_intersect(&result, second);
    return result;
}


hashset_t *hs_make_diff(const hashset_t *const first, const hashset_t *const second)
{
    assert(first);
    assert(second);

    hashset_t *result = hs_clone(first);
    hs_subtract(&result, second);
    return result;
}


struct two_sets
{
    const hashset_t *const a, *const b;
};

hashset_t *hs_make_symdiff(const hashset_t *const first, const hashset_t *const second)
{
    assert(first);
    assert(second);

    hashset_t *result = hs_make_union(first, second);
    struct two_sets sets = {first, second};
    hs_remove_many(result, contained_in_both, &sets);
    return result;
}


size_t hs_capacity(const hashset_t *const set)
{
    assert(set);

    return vector_initial_capacity(set);
}


size_t hs_count(const hashset_t *const set)
{
    assert(set);

    const size_t capacity = hs_capacity(set);
    const hs_header_t *header = get_hs_header(set);
    size_t count = 0;

    for (size_t i = 0; i < capacity; ++i)
    {
        if (HS_SLOT_USED == bitset_test(header->usage_tbl, BIT_FIELD_LEN, i))
        {
            ++count;
        }
    }
    return count;
}


vector_t *hs_values(const hashset_t *const set)
{
    assert(set);

    const hs_header_t *header = get_hs_header(set);
    const size_t capacity = hs_capacity(set);

    vector_t *values;
    vector_create(values,
        .element_size = calc_aligned_size(header->value_size, ALIGNMENT),
        .initial_cap = hs_count(set));

    for (size_t slot = 0, value = 0; slot < capacity; ++slot)
    {
        if (HS_SLOT_USED == bitset_test(header->usage_tbl, BIT_FIELD_LEN, slot))
        {
            vector_set(values, value++, get_value(set, slot));
        }
    }

    return values;
}


/***                     ***
* === static functions === *
***                     ***/

/*
* Function calculates size of the element while respecting requirement for alignment.
*/
static size_t calc_aligned_size(const size_t size, const size_t alignment)
{
    return (size + alignment - 1) / alignment * alignment;
}


static size_t calc_usage_tbl_size(const size_t capacity)
{
    return calc_aligned_size(capacity * BIT_FIELD_LEN / BYTE, ALIGNMENT);
}


/*
* Function gives an access to the hash set header that is allocated 
* after vector's control struct.
*/
static hs_header_t *get_hs_header(const hashset_t *const set)
{
    return (hs_header_t*)vector_get_ext_header(set);
}


static char *get_value(const hashset_t *const set, const size_t index)
{
    return (char*)vector_get(set, index);
}


static void set_value(hashset_t *const set, const size_t index, const void *const value)
{
    const hs_header_t *header = get_hs_header(set);
    char *entry = (char*) vector_get(set, index);
    memcpy(entry, value, header->value_size);
}


/*
* `a` and `b` factors used in conversion of the hash code into index.
* randomization makes hash function less pridictable.
*/
static void randomize_factors(hs_header_t *const header)
{
    header->a = (rand() % (LARGE_PRIME-1)) + 1;  /* [1, p-1] */
    header->b = (rand() % (LARGE_PRIME));        /* [0, p-1] */
}


/*
* Calculates index utilizing multiplicative hashing. (a*h + b) mod p mod c
*/
static size_t hash_to_index(const hs_header_t *header, const hash_t hash, const size_t capacity)
{
    return ((header->a * hash + header->b) % LARGE_PRIME) % capacity;
}


static void rehash(hashset_t **const set)
{
    const hs_header_t *old_header = get_hs_header(*set);
    const size_t prev_capacity = vector_initial_capacity(*set);

    hashset_t *new;
    const size_t new_capacity = 2 * vector_initial_capacity(*set);

    hs_create(new,
        .initial_cap = new_capacity,
        .value_size = old_header->value_size,
        .hashfunc = old_header->hashfunc,
    );

    for (size_t i = 0; i < prev_capacity; ++i)
    {
        if (HS_SLOT_USED == bitset_test(old_header->usage_tbl, BIT_FIELD_LEN, i))
        {
            (void) hs_insert(&new, get_value(*set, i)); /* always succeedes */
        }
    }

    hs_destroy(*set);
    *set = new;
}


static bool contained_in(const void *const element, void *const param)
{
    const hashset_t *const other = param;
    return hs_contains(other, element);
}


static bool not_contained_in(const void *const element, void *const param)
{
    return !contained_in(element, param);
}


static bool contained_in_both(const void *const element, void *const param)
{
    struct two_sets *sets = param;
    return hs_contains(sets->a, element) && hs_contains(sets->b, element);
}


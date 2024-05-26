#ifndef _HASHSET_H_
#define _HASHSET_H_

#include "hash.h"
#include "vector.h"

typedef vector_t hashset_t;

typedef struct hs_opts
{
    size_t value_size;
    size_t initial_cap;
    hashfunc_t hashfunc;
}
hs_opts_t;

/*
* The wrapper for `hs_create_` function that provides default values.
*/
#define hs_create(hs_ptr, ...) {\
    _Pragma("GCC diagnostic push") \
    _Pragma("GCC diagnostic ignored \"-Woverride-init\"") \
    hs_create_(&hs_ptr, &(hs_opts_t){ \
        .initial_cap = 256, \
        __VA_ARGS__ \
    }); \
    _Pragma("GCC diagnostic pop") \
}


/*
* Creates hashset
*/
void hs_create_(hashset_t **const set, const hs_opts_t *opts);


/*
* Release hashset resources.
*/
void hs_destroy(hashset_t *const set);


/*
* Checks if value is a part of the set.
*/
bool hs_contains(hashset_t *const set, const void *const value);


/*
* Inserts new value into the set.
* Returns true when value added and false if it already contained.
*/
bool hs_insert(hashset_t **const set, const void *const value);


/*
* Remove value from hashset. If there is no such value,
* then an operation considered successfull.
*/
void hs_remove(hashset_t *const set, const void *const key);


/*
* Returns current set capacity.
*/
size_t hs_capacity(const hashset_t *const set);


/*
* Returns amount of elements in the set.
*/
size_t hs_count(const hashset_t *const set);


#endif/*_HASHSET_H_*/

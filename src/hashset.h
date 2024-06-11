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

typedef enum hs_status_t
{
    HS_SUCCESS = VECTOR_SUCCESS,
    HS_ALREADY_EXISTS = VECTOR_STATUS_LAST
}
hs_status_t;

/*
* The wrapper for `hs_create_` function that provides default values.
*/
#define hs_create(...) \
    hs_create_(&(hs_opts_t){ \
        .initial_cap = 256, \
        __VA_ARGS__ \
    })

/*
* Creates hashset
*/
hashset_t *hs_create_(const hs_opts_t *const opts);


/*
* Makes exact copy of the original set.
*/ 
hashset_t *hs_clone(const hashset_t *const set);


/*
* Release hashset resources.
*/
void hs_destroy(hashset_t *const set);


/*
* Checks if value is a part of the set.
*/
bool hs_contains(const hashset_t *const set, const void *const value);


/*
* Inserts new value into the set.
* Returns true when value added and false if it already contained.
*/
hs_status_t hs_insert(hashset_t **const set, const void *const value);


/*
* Shrink hashmap and perform rehash,
* reserving free space portion of currently stored elements
* (`reserve` param is a positive real number that denotes 
*  how much free space to reserve relative to a current amount of elements being stored:
*    = 0.0f -> do not reserve any space
*    = 1.0f -> reserve as match free space as elements currently stored, etc...
*  ).
*/
hs_status_t hs_shrink_reserve(hashset_t **const set, const float reserve);


/*
* Remove value from hashset. If there is no such value,
* then an operation considered successfull.
*/
void hs_remove(hashset_t *const set, const void *const key);


/*
* Removes many values that match predicate condition.
*/ 
size_t hs_remove_many(hashset_t *const set, const predicate_t predicate, void *const param);


/*
* Modifies `set` in a way that it will contain union of itself with `other` set. (OR)
*/
hs_status_t hs_add(hashset_t **const set, const hashset_t *const other);


/*
* Modifies `set` in a way that it will contain intersection of itself with `other` set. (AND)
*/
void hs_intersect(hashset_t **const set, const hashset_t *const other);


/*
* Modifies `set` by subtracting `other`s set elements. (set - other)
*/
void hs_subtract(hashset_t **const set, const hashset_t *const other);


/*
* Makes new set that has elements of both sets. (OR)
*/
hashset_t *hs_make_union(hashset_t *const first, const hashset_t *const second);


/*
* Makes new set of elements that exist in both sets at once. (AND)
*/
hashset_t *hs_make_intersection(hashset_t *const first, const hashset_t *const second);


/*
* Makes new set that contains elements presented in `first` set,
* but not presented in `second`. (first - second)
*/
hashset_t *hs_make_diff(hashset_t *const first, const hashset_t *const second);


/*
* Makes new set that contains elements that not presented in both sets. (NAND)
*/
hashset_t *hs_make_symdiff(hashset_t *const first, const hashset_t *const second);


/*
* Returns current set capacity.
*/
size_t hs_capacity(const hashset_t *const set);


/*
* Returns amount of elements in the set.
*/
size_t hs_count(const hashset_t *const set);


/*
* Returns contiguous array of set values.
*/
vector_t *hs_values(const hashset_t *const set);


#endif/*_HASHSET_H_*/

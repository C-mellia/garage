#include <stdlib.h>
#include <string.h>
#include <regex.h>

#include <garage/log.h>
#include <garage/array.h>
#include <garage/vec.h>
#include <garage/deque.h>
// #include <garage/container.h>

#include "./.array.c"
#include "./.deque.c"
#include "./.search.c"

Array arr_from_vec(Vec vec) {
    if (!vec) return 0;
    Array arr = arr_new(vec->align);
    if (arr_check_cap(arr, vec->cap)) {
        memcpy(arr->mem, vec->mem, vec->cap * vec->align);
        arr->len = vec->cap;
    }
    return arr;
}

Vec vec_from_arr(Array arr) {
    if (!arr) return 0;
    Vec vec = vec_new(arr->align, arr->len);
    memcpy(vec->mem, arr->mem, arr->len * arr->align);
    return vec;
}

void *arr_search_item(Array arr, const void *data) {
    assert(arr, "Array is not initialized\n");
    return data? mem_search_item(arr->mem, arr->mem + arr->len * arr->align, data, arr->align): 0;
}

void *arr_search_mem(Array arr, const void *data, size_t len) {
    assert(arr, "Array is not initialized\n");
    return data? mem_search_mem(arr->mem, arr->mem + arr->len * arr->align, data, len, arr->align): 0;
}

void *vec_search_item(Vec vec, const void *data) {
    assert(vec, "Vec is not initialized at this point\n");
    return data? mem_search_item(vec->mem, vec->mem + vec->cap * vec->align, data, vec->align): 0;
}

void *vec_search_mem(Vec vec, const void *data, size_t len) {
    assert(vec, "Vec is not initialized at this point\n");
    return data? mem_search_mem(vec->mem, vec->mem + vec->cap * vec->align, data, len, vec->align): 0;
}

Deque deq_from_arr(Array arr) {
    if (!arr) return 0;
    Deque deq = deq_new(arr->align);
    if (deq_check_cap(deq, arr->len) == 0) {
        deq_wrap_memcpy_from(deq, arr->mem, 0, arr->len * arr->align);
        deq->len = arr->len;
    }
    return deq;
}

Deque deq_from_vec(Vec vec) {
    if (!vec) return 0;
    Deque deq = deq_new(vec->align);
    if (deq_check_cap(deq, vec->cap) == 0) {
        deq_wrap_memcpy_from(deq, vec->mem, 0, vec->cap * vec->align);
        deq->len = vec->cap;
    }
    return deq;
}

void *arr_search_item_func(Array arr, int (*cmp)(const void *item)) {
    assert(arr, "Array is not initialized\n");
    return cmp? mem_search_item_func(arr->mem, arr->mem + arr->len * arr->align, cmp, arr->align): 0;
}

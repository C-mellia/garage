#include <stdlib.h>
#include <string.h>
#include <regex.h>

#include <garage/log.h>
#include <garage/array.h>
#include <garage/vec.h>
#include <garage/deque.h>
#include <garage/slice.h>
// #include <garage/container.h>

#include "./.array.c"
#include "./.deque.c"
#include "./.search.c"

Array arr_from_slice(Slice slice) {
    if (!slice) return 0;
    Array arr = arr_new(slice->align);
    arr_check_cap(arr, slice->len);
    memcpy(arr->_mem, slice->mem, slice->len * slice->align), arr->_len = slice->len;
    return arr;
}

Array arr_from_vec(Vec vec) {
    if (!vec) return 0;
    Array arr = arr_new(vec->align);
    if (arr_check_cap(arr, vec->cap)) {
        memcpy(arr->_mem, vec->mem, vec->cap * vec->align);
        arr->_len = vec->cap;
    }
    return arr;
}

Vec vec_from_arr(Array arr) {
    if (!arr) return 0;
    Vec vec = vec_new(arr->_align, arr->_len);
    if (arr->_mem) memcpy(vec->mem, arr->_mem, arr->_len * arr->_align);
    return vec;
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
    Deque deq = deq_new(arr->_align);
    if (deq_check_cap(deq, arr->_len) == 0) {
        deq_wrap_memcpy_from(deq, arr->_mem, 0, arr->_len * arr->_align);
        deq->len = arr->_len;
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

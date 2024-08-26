#include <stdlib.h>
#include <string.h>
#include <regex.h>

#include <garage/log.h>
#include <garage/container.h>

#include "./.array.c"
#include "./.deque.c"
#include "./.search.c"

Array arr_from_star(StatArr star) {
    if (!star) return 0;
    Array arr = arr_new(star->align);
    if (arr_check_cap(arr, star->len)) {
        memcpy(arr->mem, star->mem, star->len * star->align);
        arr->len = star->len;
    }
    return arr;
}

StatArr star_from_arr(Array arr) {
    if (!arr) return 0;
    StatArr star = star_new(arr->align, arr->len);
    memcpy(star->mem, arr->mem, arr->len * arr->align);
    return star;
}

void *arr_search_item(Array arr, const void *data) {
    assert(arr, "Static Array is not initialized\n");
    return data? mem_search_item(arr->mem, arr->mem + arr->len * arr->align, data, arr->align): 0;
}

void *arr_search_mem(Array arr, const void *data, size_t len) {
    assert(arr, "Static Array is not initialized\n");
    return data? mem_search_mem(arr->mem, arr->mem + arr->len * arr->align, data, len, arr->align): 0;
}

void *star_search_item(StatArr star, const void *data) {
    assert(star, "Static Array is not initialized\n");
    return data? mem_search_item(star->mem, star->mem + star->len * star->align, data, star->align): 0;
}

void *star_search_mem(StatArr star, const void *data, size_t len) {
    assert(star, "Static Array is not initialized\n");
    return data? mem_search_mem(star->mem, star->mem + star->len * star->align, data, len, star->align): 0;
}

Deque deque_from_arr(Array arr) {
    if (!arr) return 0;
    Deque dq = deque_new(arr->align);
    if (deque_check_cap(dq, arr->len) == 0) {
        deque_wrap_memcpy_from(dq, arr->mem, 0, arr->len * arr->align);
        dq->len = arr->len;
    }
    return dq;
}

Deque deque_from_star(StatArr star) {
    if (!star) return 0;
    Deque dq = deque_new(star->align);
    if (deque_check_cap(dq, star->len) == 0) {
        deque_wrap_memcpy_from(dq, star->mem, 0, star->len * star->align);
        dq->len = star->len;
    }
    return dq;
}

void *arr_search_item_func(Array arr, Cmp cmp) {
    assert(arr, "Array is not initialized\n");
    return cmp? mem_search_item_func(arr->mem, arr->mem + arr->len * arr->align, cmp, arr->align): 0;
}
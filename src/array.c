#include <string.h>
#include <stdlib.h>

#include <garage/array.h>
#include <garage/garage.h>

static inline void arr_realloc(Array arr, size_t cap) {
    void *new_mem = malloc(cap * arr->layout);
    code_trap(new_mem, "arr_realloc: null\n");
    memcpy(new_mem, arr->mem, arr->len * arr->layout);
    free(arr->mem), arr->mem = new_mem, arr->cap = cap;
}

static inline void arr_check_cap(Array arr, size_t len) {
    size_t cap = arr->cap;
    while(cap < len) cap = cap? cap * 2: 10;
    if (cap != arr->cap) arr_realloc(arr, cap);
}

Array arr_new(size_t layout) {
    Array arr = malloc(sizeof *arr);
    code_trap(arr, "arr_new: null\n");
    memset(arr, 0, sizeof *arr);
    arr->layout = layout;
    return arr;
}

void arr_cleanup(Array arr) {
    if (arr) {
        if (arr->mem) free(arr->mem);
        free(arr);
    }
}

void *arr_get(Array arr, size_t idx) {
    code_trap(arr, "arr_get: null\n");
    return idx < arr->len? arr->mem + idx * arr->layout: 0;
}

void *arr_push_back(Array arr, void *data) {
    code_trap(arr && data, "arr_push_back: null\n");
    arr_check_cap(arr, arr->len + 1);
    return memcpy(arr->mem + arr->len++ * arr->layout, data, arr->layout);
}

void *arr_push_front(Array arr, void *data) {
    code_trap(arr && data, "arr_push_front: null\n");
    arr_check_cap(arr, arr->len + 1);
    memmove(arr->mem + arr->layout, arr->mem, arr->len++ * arr->layout);
    return memcpy(arr->mem, data, arr->layout);
}

void *arr_pop_back(Array arr) {
    code_trap(arr, "arr_pop_back: null\n");
    if (!arr->len) return 0;
    return arr->mem + --arr->len * arr->layout;
}

void *arr_pop_front(Array arr) {
    code_trap(arr, "arr_pop_front: null\n");
    if (!arr->len) return 0;
    uint8_t buf[arr->layout];
    memcpy(buf, arr->mem, sizeof buf);
    memmove(arr->mem, arr->mem + arr->layout, --arr->len * arr->layout);
    return memcpy(arr->mem + arr->len * arr->layout, buf, sizeof buf);
}

void *arr_drop(Array arr, size_t idx) {
    code_trap(arr, "arr_drop: null\n");
    if (idx >= arr->len) return 0;
    uint8_t buf[arr->layout];
    memcpy(buf, arr->mem + idx * arr->layout, sizeof buf);
    memmove(arr->mem + idx * arr->layout,
            arr->mem + (idx + 1) * arr->layout,
            idx + 1 == arr->len? 0: (arr->len - idx - 2) * arr->layout);
    return memcpy(arr->mem + --arr->len * arr->layout, buf, sizeof buf);
}

void *arr_insert(Array arr, size_t idx, void *data) {
    code_trap(arr && data, "arr_insert: null\n");
    if (idx > arr->len) return (void *) 0;
    arr_check_cap(arr, arr->len + 1);
    memmove(arr->mem + (idx + 1) * arr->layout,
            arr->mem + idx * arr->layout,
            (arr->len++ - idx) * arr->layout);
    return memcpy(arr->mem + idx * arr->layout, data, arr->layout);
}

void *arr_front(Array arr) {
    code_trap(arr, "arr_front: null\n");
    return arr->len? arr->mem: 0;
}

void *arr_back(Array arr) {
    code_trap(arr, "arr_back: null\n");
    return arr->len? arr->mem + arr->layout * (arr->len - 1): 0;
}

void arr_deb_print(Array arr) {
    if (!arr) {
        printf("(nil)\n");
    } else {
        printf("{mem: %p, len: %lu, cap: %lu, layout: %lu}\n",
               arr->mem, arr->len, arr->cap, arr->layout);
    }
}

void arr_reserve(Array arr, size_t cap) {
    code_trap(arr, "arr_reserve: null\n");
    arr_check_cap(arr, arr->cap + cap);
}

void arr_resize(Array arr, size_t len, void *data) {
    code_trap(arr, "arr_resize: null\n");
    arr_check_cap(arr, len);
    if (len > arr->len) {
        if (data) {
            for (size_t i = 0 ; i < len - arr->len; ++i) {
                memcpy(arr->mem + (arr->len + i) * arr->layout, data, arr->layout);
            }
        } else {
            memset(arr->mem + arr->len * arr->layout, 0, (len - arr->len) * arr->layout);
        }
    }
    arr->len = len;
}

size_t arr_len(Array arr) {
    code_trap(arr, "arr_len: null\n");
    return arr->len;
}

size_t arr_layout(Array arr) {
    code_trap(arr, "arr_layout: null\n");
    return arr->layout;
}

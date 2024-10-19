#ifndef _GARAGE_ARRAY_H
#define _GARAGE_ARRAY_H 1

#include "./.slice.c"

static inline __attribute__((unused))
size_t cap_inc(size_t cap);
static inline __attribute__((unused))
void __arr_init(Array arr, size_t align);
static inline __attribute__((unused))
void *__arr_get(Array arr, size_t idx);
static inline __attribute__((unused))
void arr_realloc(Array arr, size_t cap);
static inline __attribute__((unused))
int arr_check_cap(Array arr, size_t len);
static inline __attribute__((unused))
void *mem_dup(const void *mem, size_t align, size_t len);
static inline __attribute__((unused))
void *mem_dup_zero_end(const void *mem, size_t align, size_t len);

static size_t cap_inc(size_t cap) {
    return cap? cap * 2: 10;
}

static void __arr_init(Array arr, size_t align) {
    memset(arr, 0, sizeof *arr), arr->slice_align = align;
}

static void *__arr_get(Array arr, size_t idx) {
    Slice slice = (void *)arr->slice;
    return __slice_get(slice, idx);
}

static void arr_realloc(Array arr, size_t cap) {
    void *new_mem = malloc(cap * arr->slice_align);
    alloc_check(malloc, new_mem, cap * arr->slice_align);
    if (arr->slice_mem) memcpy(new_mem, arr->slice_mem, arr->len * arr->slice_align), free(arr->slice_mem);
    arr->slice_mem = new_mem, arr->slice_len = cap;
}

static int arr_check_cap(Array arr, size_t len) {
    size_t cap = arr->slice_len;
    while(cap < len) cap = cap_inc(cap);
    if (cap != arr->slice_len) return arr_realloc(arr, cap), 0;
    return -1;
}

static void *mem_dup(const void *mem, size_t align, size_t len) {
    void *new_mem = malloc(len * align);
    alloc_check(malloc, new_mem, len * align);
    return memcpy(new_mem, mem, len * align);
}

static void *mem_dup_zero_end(const void *mem, size_t align, size_t len) {
    void *new_mem = malloc((len + 1) * align);
    alloc_check(malloc, new_mem, (len + 1) * align);
    return memset(new_mem + len * align, 0, align), memcpy(new_mem, mem, len * align);
}

#endif // _GARAGE_ARRAY_H

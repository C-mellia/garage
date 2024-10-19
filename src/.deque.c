#ifndef _GARAGE_DEQUE_H
#define _GARAGE_DEQUE_H 1

#include <garage/slice.h>
#include <garage/array.h>
#include "./.array.c"
#include "./.slice.c"

static __attribute__((always_inline, unused))
inline void *top_clamp(void *end, void *ptr, size_t len);
static __attribute__((always_inline, unused))
inline void *bottom_clamp(void *begin, void *ptr, size_t len);
static __attribute__((always_inline, unused))
inline void *deq_wrap_memcpy_to(void *new_mem, Deque deq, size_t idx, size_t len);
static __attribute__((always_inline, unused))
inline void *deq_wrap_memcpy_from(Deque deq, void *mem, size_t idx, size_t len);
static __attribute__((always_inline, unused))
inline void *deq_wrap_memmove(Deque deq, size_t dst, size_t idx, size_t len);
static __attribute__((unused))
void deq_realloc(Deque deq, size_t cap);
static __attribute__((unused))
int deq_check_cap(Deque deq, size_t len);
static __attribute__((unused))
void *__deq_get(Deque deq, size_t idx);
static __attribute__((unused))
void __deq_init(Deque deq, size_t align);

static inline void *top_clamp(void *end, void *ptr, size_t len) {
    while (ptr >= end) ptr -= len;
    return ptr;
}

static inline void *bottom_clamp(void *begin, void *ptr, size_t len) {
    while (ptr < begin) ptr += len;
    return ptr;
}

static inline void *deq_wrap_memcpy_to(void *new_mem, Deque deq, size_t idx, size_t len) {
    Slice slice = (void *)deq->slice;
    void *begin = __deq_get(deq, idx);
    len *= slice->align;
    intptr_t wrapped = (begin + len) - __slice_get(slice, slice->len);
    if (wrapped > 0) {
        memcpy(new_mem, begin, len - wrapped);
        memcpy(new_mem + len - wrapped, slice->mem, wrapped);
    } else {
        memcpy(new_mem, begin, len);
    }
    return new_mem;
}

static inline void *deq_wrap_memcpy_from(Deque deq, void *mem, size_t idx, size_t len) {
    Slice slice = (void *)deq->slice;
    void *begin = __deq_get(deq, idx);
    len *= slice->align;
    intptr_t wrapped = (begin + len) - __slice_get(slice, slice->len);
    if (wrapped > 0) {
        memcpy(begin, mem, len - wrapped);
        memcpy(slice->mem, mem + len - wrapped, wrapped);
    } else {
        memcpy(begin, mem, len);
    }
    return begin;
}

static void *deq_wrap_memmove(Deque deq, size_t dst, size_t idx, size_t len) {
    uint8_t buf[len];
    deq_wrap_memcpy_to(buf, deq, idx, len);
    return deq_wrap_memcpy_from(deq, buf, dst, len);
}

static void deq_realloc(Deque deq, size_t cap) {
    Slice slice = (void *)deq->slice;
    void *new_mem = malloc(slice->align * cap);
    alloc_check(malloc, new_mem, slice->align * cap);
    if (slice->mem) {
        deq_wrap_memcpy_to(new_mem, deq, 0, deq->len);
        free(slice->mem);
    }
    slice->len = cap, slice->mem = new_mem, deq->begin = 0;
}

// slightly different than array's check cap
static int deq_check_cap(Deque deq, size_t len) {
    Slice slice = (void *)deq->slice;
    size_t cap = slice->len;
    while (cap < len) cap = cap? cap * 2: 10;
    if (cap != slice->len) return deq_realloc(deq, cap), 0;
    return -1;
}

static void *__deq_get(Deque deq, size_t idx) {
    Slice slice = (void *)deq->slice;
    return top_clamp(__slice_get(slice, slice->len), __slice_get(slice, deq->begin + idx), slice->len * slice->align);
}

static void __deq_init(Deque deq, size_t align) {
    Slice slice = (void *)deq->slice;
    memset(deq, 0, sizeof *deq), slice->align = align;
}

#endif // _GARAGE_DEQUE_H

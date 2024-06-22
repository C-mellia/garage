#include <string.h>
#include <stdlib.h>
#include <stdint.h>

#include <garage/garage.h>
#include <garage/deque.h>

static inline void *top_clamp(void *end, void *ptr, size_t len) {
    while (ptr >= end) ptr -= len;
    return ptr;
}

static inline void *bottom_clamp(void *begin, void *ptr, size_t len) {
    while (ptr < begin) ptr += len;
    return ptr;
}

static inline void *deque_wrap_memcpy_to(void *new_data, Deque dq, size_t idx, size_t len) {
    void *begin = top_clamp(dq->data + dq->cap * dq->layout, dq->begin + idx * dq->layout, dq->cap * dq->layout);
    intptr_t wrapped = begin + len - (dq->data + dq->cap * dq->layout);
    if (wrapped > 0) {
        memcpy(new_data, begin, len - wrapped);
        memcpy(new_data + len - wrapped, dq->data, wrapped);
    } else {
        memcpy(new_data, begin, len);
    }
    return new_data;
}

static inline void *deque_wrap_memcpy_from(Deque dq, void *data, size_t idx, size_t len) {
    void *begin = top_clamp(dq->data + dq->cap * dq->layout, dq->begin + idx * dq->layout, dq->cap * dq->layout);
    intptr_t wrapped = begin + len - (dq->data + dq->cap * dq->layout);
    if (wrapped > 0) {
        memcpy(begin, data, len - wrapped);
        memcpy(dq->data, data + len - wrapped, wrapped);
    } else {
        memcpy(begin, data, len);
    }
    return begin;
}

static inline void deque_realloc(Deque dq, size_t cap) {
    void *new_data = malloc(dq->layout * cap);
    dq->cap = cap;
    if (dq->data) {
        deque_wrap_memcpy_to(new_data, dq, 0, dq->len * dq->layout);
        free(dq->data);
    }
    dq->data = new_data, dq->begin = dq->data;
}

static inline void deque_check_cap(Deque dq, size_t len) {
    size_t cap = dq->cap;
    while (cap < len) cap = cap? cap * 2: 10;
    if (cap != dq->cap) deque_realloc(dq, cap);
}

Deque deque_new(size_t layout) {
    Deque dq = malloc(sizeof *dq);
    code_trap(dq, "deque_new: null\n");
    return memset(dq, 0, sizeof *dq), dq->layout = layout, dq;
}

void *deque_push_back(Deque dq, void *data) {
    code_trap(dq, "deque_push_back: null\n");
    if (!data) return 0;
    deque_check_cap(dq, dq->len + 1);
    // dq->len is not used in the function call
    return deque_wrap_memcpy_from(dq, data, dq->len++, dq->layout);
}

void deque_cleanup(Deque dq) {
    if (!dq) return;
    if (dq->data) free(dq->data);
    free(dq);
}

void *deque_push_front(Deque dq, void *data) {
    code_trap(dq, "deque_push_front: null\n");
    if (!data) return 0;
    deque_check_cap(dq, ++dq->len);
    dq->begin = bottom_clamp(dq->data, dq->begin - dq->layout, dq->cap * dq->layout);
    return deque_wrap_memcpy_from(dq, data, 0, dq->layout);
}

void *deque_pop_back(Deque dq) {
    code_trap(dq, "deque_pop_back: null\n");
    if (!dq->len) return 0;
    return top_clamp(dq->data + dq->cap * dq->layout, dq->begin + --dq->len * dq->layout, dq->cap * dq->layout);
}

void *deque_pop_front(Deque dq) {
    void *begin;
    code_trap(dq, "deque_pop_front: null\n");
    if (!dq->len) return 0;
    begin = dq->begin;
    dq->begin = top_clamp(dq->data + dq->cap * dq->layout, dq->begin + dq->layout, dq->cap * dq->layout);
    return --dq->len, begin;
}

size_t deque_len(Deque dq) {
    code_trap(dq, "deque_len: null\n");
    return dq->len;
}

void deque_deb_print(Deque dq) {
    if (!dq) {
        printf("(nil)\n");
    } else {
        printf("{data: %p, begin: %p, len: %lu, cap: %lu, layout: %lu}\n",
               dq->data, dq->begin, dq->len, dq->cap, dq->layout);
    }
}

void *deque_get(Deque dq, size_t idx) {
    code_trap(dq, "deque_get: null\n");
    return idx < dq->len?
    top_clamp(dq->data + dq->cap * dq->layout, dq->begin + idx * dq->layout, dq->len * dq->layout): 0;
}

void *deque_front(Deque dq) {
    code_trap(dq, "deque_front: null\n");
    return dq->len? dq->begin: 0;
}

void *deque_back(Deque dq) {
    code_trap(dq, "deque_back: null\n");
    return dq->len? top_clamp(dq->data, dq->begin + (dq->len - 1) * dq->layout, dq->cap * dq->layout): 0;
}

void *deque_insert(Deque dq, size_t idx, void *data) {
    void *buf;
    size_t len;
    code_trap(dq, "deque_insert: null\n");
    if (idx > dq->len || !data) return 0;
    deque_check_cap(dq, dq->len + 1);
    len = (dq->len++ - idx) * dq->layout, buf = alloca(len);
    deque_wrap_memcpy_to(buf, dq, idx, len);
    deque_wrap_memcpy_from(dq, buf, idx + 1, len);
    return deque_wrap_memcpy_from(dq, data, idx, dq->layout);
}

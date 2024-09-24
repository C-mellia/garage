#ifndef _GARAGE_DEQUE_H
#define _GARAGE_DEQUE_H 1

static __attribute__((always_inline, unused))
inline void *top_clamp(void *end, void *ptr, size_t len);
static __attribute__((always_inline, unused))
inline void *bottom_clamp(void *begin, void *ptr, size_t len);
static __attribute__((always_inline, unused))
inline void *deq_wrap_memcpy_to(void *new_mem, Deque deq, size_t idx, size_t len);
static __attribute__((always_inline, unused))
inline void *deq_wrap_memcpy_from(Deque deq, void *mem, size_t idx, size_t len);
static __attribute__((unused))
void deq_realloc(Deque deq, size_t cap);
static __attribute__((unused))
int deq_check_cap(Deque deq, size_t len);

static inline void *top_clamp(void *end, void *ptr, size_t len) {
    while (ptr >= end) ptr -= len;
    return ptr;
}

static inline void *bottom_clamp(void *begin, void *ptr, size_t len) {
    while (ptr < begin) ptr += len;
    return ptr;
}

static inline void *deq_wrap_memcpy_to(void *new_mem, Deque deq, size_t idx, size_t len) {
    void *begin = top_clamp(deq->mem + deq->cap * deq->align, deq->begin + idx * deq->align, deq->cap * deq->align);
    intptr_t wrapped = begin + len - (deq->mem + deq->cap * deq->align);
    if (wrapped > 0) {
        memcpy(new_mem, begin, len - wrapped);
        memcpy(new_mem + len - wrapped, deq->mem, wrapped);
    } else {
        memcpy(new_mem, begin, len);
    }
    return new_mem;
}

static inline void *deq_wrap_memcpy_from(Deque deq, void *mem, size_t idx, size_t len) {
    void *begin = top_clamp(deq->mem + deq->cap * deq->align, deq->begin + idx * deq->align, deq->cap * deq->align);
    intptr_t wrapped = begin + len - (deq->mem + deq->cap * deq->align);
    if (wrapped > 0) {
        memcpy(begin, mem, len - wrapped);
        memcpy(deq->mem, mem + len - wrapped, wrapped);
    } else {
        memcpy(begin, mem, len);
    }
    return begin;
}

static void deq_realloc(Deque deq, size_t cap) {
    void *new_mem = malloc(deq->align * cap);
    deq->cap = cap;
    if (deq->mem) {
        deq_wrap_memcpy_to(new_mem, deq, 0, deq->len * deq->align);
        free(deq->mem);
    }
    deq->mem = new_mem, deq->begin = deq->mem;
}

static int deq_check_cap(Deque deq, size_t len) {
    size_t cap = deq->cap;
    while (cap < len) cap = cap? cap * 2: 10;
    if (cap != deq->cap) return deq_realloc(deq, cap), 0;
    return -1;
}

#endif // _GARAGE_DEQUE_H

static inline void *top_clamp(void *end, void *ptr, size_t len) {
    while (ptr >= end) ptr -= len;
    return ptr;
}

static inline void *bottom_clamp(void *begin, void *ptr, size_t len) {
    while (ptr < begin) ptr += len;
    return ptr;
}

static inline void *deque_wrap_memcpy_to(void *new_mem, Deque dq, size_t idx, size_t len) {
    void *begin = top_clamp(dq->mem + dq->cap * dq->align, dq->begin + idx * dq->align, dq->cap * dq->align);
    intptr_t wrapped = begin + len - (dq->mem + dq->cap * dq->align);
    if (wrapped > 0) {
        memcpy(new_mem, begin, len - wrapped);
        memcpy(new_mem + len - wrapped, dq->mem, wrapped);
    } else {
        memcpy(new_mem, begin, len);
    }
    return new_mem;
}

static inline void *deque_wrap_memcpy_from(Deque dq, void *mem, size_t idx, size_t len) {
    void *begin = top_clamp(dq->mem + dq->cap * dq->align, dq->begin + idx * dq->align, dq->cap * dq->align);
    intptr_t wrapped = begin + len - (dq->mem + dq->cap * dq->align);
    if (wrapped > 0) {
        memcpy(begin, mem, len - wrapped);
        memcpy(dq->mem, mem + len - wrapped, wrapped);
    } else {
        memcpy(begin, mem, len);
    }
    return begin;
}

static void deque_realloc(Deque dq, size_t cap) {
    void *new_mem = malloc(dq->align * cap);
    dq->cap = cap;
    if (dq->mem) {
        deque_wrap_memcpy_to(new_mem, dq, 0, dq->len * dq->align);
        free(dq->mem);
    }
    dq->mem = new_mem, dq->begin = dq->mem;
}

static int deque_check_cap(Deque dq, size_t len) {
    size_t cap = dq->cap;
    while (cap < len) cap = cap? cap * 2: 10;
    if (cap != dq->cap) return deque_realloc(dq, cap), 0;
    return -1;
}

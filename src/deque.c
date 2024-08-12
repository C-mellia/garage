#include <string.h>
#include <stdlib.h>
#include <stdint.h>

#include <garage/garage.h>
#include <garage/deque.h>
#include <garage/log.h>

#include "./.deque.c"

Deque deque_new(size_t align) {
    Deque dq = malloc(sizeof *dq);
    assert(dq, "Deque is not initialized\n");
    return memset(dq, 0, sizeof *dq), dq->align = align, dq;
}

void *deque_push_back(Deque dq, void *mem) {
    assert(dq, "Deque is not initialized\n");
    if (!mem) return 0;
    deque_check_cap(dq, dq->len + 1);
    // dq->len is not used in the function call
    return deque_wrap_memcpy_from(dq, mem, dq->len++, dq->align);
}

void deque_cleanup(Deque dq) {
    if (!dq) return;
    if (dq->mem) free(dq->mem);
    free(dq);
}

void *deque_push_front(Deque dq, void *mem) {
    assert(dq, "Deque is not initialized\n");
    if (!mem) return 0;
    deque_check_cap(dq, ++dq->len);
    dq->begin = bottom_clamp(dq->mem, dq->begin - dq->align, dq->cap * dq->align);
    return deque_wrap_memcpy_from(dq, mem, 0, dq->align);
}

void *deque_pop_back(Deque dq) {
    assert(dq, "Deque is not initialized\n");
    if (!dq->len) return 0;
    return top_clamp(dq->mem + dq->cap * dq->align, dq->begin + --dq->len * dq->align, dq->cap * dq->align);
}

void *deque_pop_front(Deque dq) {
    void *begin;
    assert(dq, "Deque is not initialized\n");
    if (!dq->len) return 0;
    begin = dq->begin;
    dq->begin = top_clamp(dq->mem + dq->cap * dq->align, dq->begin + dq->align, dq->cap * dq->align);
    return --dq->len, begin;
}

void deque_deb_print(Deque dq) {
    if (!dq) {
        printf("(nil)");
    } else {
        printf("{mem: %p, begin: %p, len: %lu, cap: %lu, align: %lu}",
               dq->mem, dq->begin, dq->len, dq->cap, dq->align);
    }
}

void *deque_get(Deque dq, size_t idx) {
    assert(dq, "Deque is not initialized\n");
    return idx < dq->len? top_clamp(dq->mem + dq->cap * dq->align, dq->begin + idx * dq->align, dq->len * dq->align): 0;
}

void *deque_front(Deque dq) {
    assert(dq, "Deque is not initialized\n");
    return deque_get(dq, 0);
}

void *deque_back(Deque dq) {
    assert(dq, "Deque is not initialized\n");
    return dq->len? deque_get(dq, dq->len - 1): 0;
}

void *deque_insert(Deque dq, size_t idx, void *mem) {
    assert(dq, "Deque is not initialized\n");
    if (idx > dq->len || !mem) return 0;
    deque_check_cap(dq, dq->len + 1);
    size_t len = (dq->len++ - idx) * dq->align;
    void *buf = alloca(len);
    deque_wrap_memcpy_to(buf, dq, idx, len), deque_wrap_memcpy_from(dq, buf, idx + 1, len);
    return deque_wrap_memcpy_from(dq, mem, idx, dq->align);
}

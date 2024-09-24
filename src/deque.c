#include <string.h>
#include <stdlib.h>
#include <stdint.h>

#include <garage/garage.h>
#include <garage/deque.h>
#include <garage/log.h>

#include "./.deque.c"

static __attribute__((unused))
void __deq_init(Deque deq, size_t align);

void deq_init(Deque deq, size_t align) {
    if (!deq) return;
    __deq_init(deq, align);
}

Deque deq_new(size_t align) {
    Deque deq = malloc(sizeof *deq);
    alloc_check(malloc, deq, sizeof *deq);
    return __deq_init(deq, align), deq;
}

void *deq_push_back(Deque deq, void *mem) {
    nul_check(Deque, deq);
    if (!mem) return 0;
    deq_check_cap(deq, deq->len + 1);
    // deq->len is not used in the function call
    return deq_wrap_memcpy_from(deq, mem, deq->len++, deq->align);
}

void deq_cleanup(Deque deq) {
    if (!deq) return;
    if (deq->mem) free(deq->mem);
    deq_init(deq, deq->align);
}

void deq_drop(Deque *deq) {
    if (deq && *deq) deq_cleanup(*deq), free(*deq), *deq = 0;
}

void *deq_push_front(Deque deq, void *mem) {
    nul_check(Deque, deq);
    if (!mem) return 0;
    deq_check_cap(deq, ++deq->len);
    deq->begin = bottom_clamp(deq->mem, deq->begin - deq->align, deq->cap * deq->align);
    return deq_wrap_memcpy_from(deq, mem, 0, deq->align);
}

void *deq_pop_back(Deque deq) {
    nul_check(Deque, deq);
    if (!deq->len) return 0;
    return top_clamp(deq->mem + deq->cap * deq->align, deq->begin + --deq->len * deq->align, deq->cap * deq->align);
}

void *deq_pop_front(Deque deq) {
    void *begin;
    nul_check(Deque, deq);
    if (!deq->len) return 0;
    begin = deq->begin;
    deq->begin = top_clamp(deq->mem + deq->cap * deq->align, deq->begin + deq->align, deq->cap * deq->align);
    return --deq->len, begin;
}

void deq_deb_print(Deque deq) {
    if (!deq) {
        printf("(nil)");
    } else {
        printf("{mem: %p, begin: %p, len: %lu, cap: %lu, align: %lu}",
               deq->mem, deq->begin, deq->len, deq->cap, deq->align);
    }
}

void *deq_get(Deque deq, size_t idx) {
    nul_check(Deque, deq);
    return idx < deq->len? top_clamp(deq->mem + deq->cap * deq->align, deq->begin + idx * deq->align, deq->len * deq->align): 0;
}

void *deq_front(Deque deq) {
    nul_check(Deque, deq);
    return deq_get(deq, 0);
}

void *deq_back(Deque deq) {
    nul_check(Deque, deq);
    return deq->len? deq_get(deq, deq->len - 1): 0;
}

void *deq_insert(Deque deq, size_t idx, void *mem) {
    nul_check(Deque, deq);
    if (idx > deq->len || !mem) return 0;
    deq_check_cap(deq, deq->len + 1);
    size_t len = (deq->len++ - idx) * deq->align;
    void *buf = alloca(len);
    deq_wrap_memcpy_to(buf, deq, idx, len), deq_wrap_memcpy_from(deq, buf, idx + 1, len);
    return deq_wrap_memcpy_from(deq, mem, idx, deq->align);
}

static void __deq_init(Deque deq, size_t align) {
    memset(deq, 0, sizeof *deq), deq->align = align;
}

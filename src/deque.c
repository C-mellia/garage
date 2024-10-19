#include <string.h>
#include <stdlib.h>
#include <stdint.h>

#include <garage/garage.h>
#include <garage/deque.h>
#include <garage/log.h>
#include <garage/string.h>
#include <garage/slice.h>

#include "./.deque.c"

void deq_init(Deque deq, size_t align) {
    if (!deq) return;
    __deq_init(deq, align);
}

Deque deq_new(size_t align) {
    Deque deq = malloc(sizeof *deq);
    alloc_check(malloc, deq, sizeof *deq);
    return __deq_init(deq, align), deq;
}

void deq_cleanup(Deque deq) {
    if (!deq) return;
    Slice slice = (void *)deq->slice;
    if (slice->mem) free(slice->mem);
    slice->mem = 0, deq->begin = deq->len = slice->len = 0;
}

void deq_drop(Deque *deq) {
    if (deq && *deq) deq_cleanup(*deq), free(*deq), *deq = 0;
}

void *deq_push_back(Deque deq, void *mem) {
    nul_check(Deque, deq);
    Slice slice = (void *)deq->slice;
    deq_check_cap(deq, deq->len + 1);
    if (mem) {
        return memcpy(__deq_get(deq, deq->len++), mem, slice->align);
    } else {
        return memset(__deq_get(deq, deq->len++), 0, slice->align);
    }
}

void *deq_push_front(Deque deq, void *mem) {
    nul_check(Deque, deq);
    Slice slice = (void *)deq->slice;
    deq_check_cap(deq, ++deq->len);
    deq->begin = deq->begin? deq->begin - 1: slice->len - 1;
    if (mem) {
        return memcpy(__slice_get(slice, deq->begin), mem, slice->align);
    } else {
        return memset(__slice_get(slice, deq->begin), 0, slice->align);
    }
}

void *deq_pop_back(Deque deq) {
    nul_check(Deque, deq);
    if (!deq->len) return 0;
    return __deq_get(deq, --deq->len);
}

void *deq_pop_front(Deque deq) {
    nul_check(Deque, deq);
    Slice slice = (void *)deq->slice;
    if (!deq->len) return 0;
    void *begin = __slice_get(slice, deq->begin);
    deq->begin = deq->begin + 1 < slice->len? deq->begin + 1: 0;
    return --deq->len, begin;
}

int deq_deb_dprint(int fd, Deque deq) {
    if (!deq) dprintf(fd, "(nil)");
    String Cleanup(string_drop) string = string_new();
    string_fmt(string, "{begin: 0x%lx, len: %lu, slice: ", deq->begin, deq->len);
    string_fmt_func(string, (void *)slice_deb_dprint, (void *)deq->slice);
    string_fmt(string, "}");
    return string_dprint(fd, string);
}

int deq_deb_print(Deque deq) {
    return fflush(stdout), deq_deb_dprint(1, deq);
}

int deq_hex_dprint(int fd, Deque deq) {
    if (!deq) return dprintf(fd, "(nil)");
    String Cleanup(string_drop) string = string_new();
    Slice slice = (void *)deq->slice;
    string_fmt(string, "[");
    for (size_t i = 0; i < deq->len; ++i) {
        string_fmt(string, "0x");
        string_from_anyint_hex(string, __deq_get(deq, i), slice->align);
        if (i + 1 < deq->len) string_fmt(string, ", ");
    }
    string_fmt(string, "]");
    return string_dprint(fd, string);
}

int deq_hex_print(Deque deq) {
    return fflush(stdout), deq_hex_dprint(1, deq);
}

int deq_idx_dprint(int fd, Deque deq) {
    if (!deq) return dprintf(fd, "(nil)");
    String Cleanup(string_drop) string = string_new();
    Slice slice = (void *)deq->slice;
    string_fmt(string, "[");
    for (size_t i = 0; i < deq->len; ++i) {
        size_t off = (__deq_get(deq, i) - slice->mem) / slice->align;
        string_fmt(string, "+0x%lx", off);
        if (i + 1 < deq->len) string_fmt(string, ", ");
    }
    string_fmt(string, "]");
    return string_dprint(fd, string);
}

int deq_idx_print(Deque deq) {
    return fflush(stdout), deq_idx_dprint(1, deq);
}

void *deq_remove(Deque deq, size_t idx) {
    nul_check(Deque, deq);
    Slice slice = (void *)deq->slice;
    if (idx >= deq->len) return 0;
    uint8_t buf[slice->align];
    memcpy(buf, __deq_get(deq, idx), sizeof buf);
    deq_wrap_memmove(deq, idx, idx + 1, deq->len - idx - 1);
    return memcpy(__deq_get(deq, deq->len--), buf, sizeof buf); // end of queue
}

void *deq_insert(Deque deq, size_t idx, void *mem) {
    nul_check(Deque, deq);
    if (idx > deq->len) return 0;
    Slice slice = (void *)deq->slice;
    deq_check_cap(deq, deq->len + 1);
    deq_wrap_memmove(deq, idx + 1, idx, deq->len++ - idx);
    if (mem) {
        return memcpy(__deq_get(deq, idx), mem, slice->align);
    } else {
        return memset(__deq_get(deq, idx), 0, slice->align);
    }
}

void *deq_get(Deque deq, size_t idx) {
    nul_check(Deque, deq);
    return idx < deq->len? __deq_get(deq, idx): 0;
}

void *deq_front(Deque deq) {
    nul_check(Deque, deq);
    return deq->len? __deq_get(deq, 0): 0;
}

void *deq_back(Deque deq) {
    nul_check(Deque, deq);
    return deq->len? __deq_get(deq, deq->len - 1): 0;
}

size_t deq_len(Deque deq) {
    nul_check(Deque, deq);
    return deq->len;
}

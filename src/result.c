#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

#include <garage/garage.h>
#include <garage/array.h>
#include <garage/string.h>
#include <garage/result.h>
#include <garage/log.h>
#include <garage/vec.h>

static inline void __res_init(Result res, int tag, void *src, size_t cap);

void res_init(Result res, int code, void *src, size_t len) {
    if (!res) return;
    __res_init(res, code, src, len);
}

Result res_new(int code, void *src, size_t cap) {
    Result res = malloc(sizeof *res + cap);
    alloc_check(malloc, res, sizeof *res + cap);
    return __res_init(res, code, src, cap), res;
}

void res_cleanup(Result res) {
    (void) res;
}

void res_drop(Result *res) {
    if (res && *res) res_cleanup(*res), free(*res), *res = 0;
}

void *res_consume(Result res) {
    nul_check(Result, res);
    void *data = malloc(res->cap);
    alloc_check(malloc, data, res->cap);
    memcpy(data, res->data, res->cap);
    return data;
}

int res_deb_dprint(int fd, Result res) {
    if (!res) return dprintf(fd, "(nil)");
    return dprintf(fd, "{tag: %d, cap: %zu, data: %p}", res->tag, res->cap, res->data);
}

int res_deb_print(Result res) {
    return res_deb_dprint(1, res);
}

static inline void __res_init(Result res, int tag, void *src, size_t cap) {
    res->tag = tag, memcpy(res->data, src, cap), res->cap = cap;
}

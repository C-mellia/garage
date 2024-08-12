#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

#include <garage/garage.h>
#include <garage/array.h>
#include <garage/string.h>
#include <garage/error.h>
#include <garage/log.h>

Result ok_new(void *src, size_t len) {
    Result res = malloc(sizeof *res + len);
    assert(res, "ok_new: malloc failed\n");
    res->code = 0, res->len = len, src? memcpy(res->data, src, len): memset(res->data, 0, len);
    return res;
}

Result err_new(const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    Array arr = arr_new(1);
    string_vfmt(arr, fmt, args);
    va_end(args);
    Result res = malloc(sizeof *res + arr->len + 1);
    assert(res, "err_new: malloc failed\n");
    res->code = -1, res->len = arr->len, strncpy((void *)res->data, arr->mem, arr->len);
    arr_cleanup(arr);
    return res;
}

void res_cleanup(Result res) {
    if (res) free(res);
}

void *res_consume(Result res, int fd) {
    if (!res) return res_cleanup(res), NULL;
    if (res->code == -1) {
        dprintf(fd, "%.*s\n", (int)res->len, res->data);
        res_cleanup(res);
        panic("attempting to consume an error result\n");
        __builtin_unreachable();
    } else if (res->len == 0) {
        return res_cleanup(res), NULL;
    } else {
        void *data = malloc(res->len);
        assert(data, "res_consume: malloc failed\n");
        memcpy(data, res->data, res->len);
        return res_cleanup(res), data;
    }
}

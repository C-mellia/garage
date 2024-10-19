#include <stdlib.h>
#include <string.h>

#include <garage/array.h>
#include <garage/slice.h>
#include <garage/log.h>
#include <garage/string.h>
#include <garage/garage.h>
#include <garage/scanner.h>

static inline void __scnr_init(Scanner scnr, void *mem, size_t align, size_t len);

void scnr_init(Scanner scnr, void *mem, size_t align, size_t len) {
    nul_check(Scanner, scnr);
    __scnr_init(scnr, mem, align, len);
}

Scanner scnr_new(void *mem, size_t align, size_t len) {
    Scanner scnr = malloc(sizeof *scnr);
    alloc_check(malloc, scnr, sizeof *scnr);
    return __scnr_init(scnr, mem, align, len), scnr;
}

void scnr_cleanup(Scanner scnr) {
    Array errs = (void *)scnr->errs;
    Slice slice = (void *)scnr->src;
    for (size_t i = 0; i < arr_len(errs); ++i) {
        string_drop(arr_get(errs, i));
    }
    arr_cleanup(errs), slice_cleanup(slice);
}

void scnr_drop(Scanner *scnr) {
    if (scnr && *scnr) scnr_cleanup(*scnr), free(*scnr), *scnr = 0;
}

void *scnr_peek(Scanner scnr, size_t off) {
    nul_check(Scanner, scnr);
    Slice slice = (void *)scnr->src;
    return slice_get(slice, off + scnr->pos);
}

int scnr_deb_dprint(int fd, Scanner scnr) {
    if (!scnr) return dprintf(fd, "(nil)");
    Slice slice = (void *)scnr->src;
    String Cleanup(string_drop) string = string_new();
    string_fmt(string, "{pos: %zu, slice: ", scnr->pos);
    string_fmt_func(string, (void *)slice_deb_dprint, slice);
    string_fmt(string, "}");
    return string_dprint(fd, string);
}

int scnr_deb_print(Scanner scnr) {
    return scnr_deb_dprint(1, scnr);
}

size_t scnr_consume(Scanner scnr, size_t count) {
    nul_check(Scanner, scnr);
    Slice slice = (void *)scnr->src;
    size_t consume = scnr->pos + count < slice->len? slice->len - scnr->pos: 0;
    return scnr->pos += consume, count - consume;
}

static void __scnr_init(Scanner scnr, void *mem, size_t align, size_t len) {
    Array errs = (void *)scnr->errs;
    Slice slice = (void *)scnr->src;
    scnr->pos = 0, arr_init(errs, sizeof (String)), slice_init(slice, mem, align, len);
}

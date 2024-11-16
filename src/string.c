#include <stdlib.h>
#include <string.h>

#include <unistd.h>
#include <fcntl.h>

#include <garage/garage.h>
#include <garage/log.h>
#include <garage/string.h>
#include <garage/ascii.h>
#include <garage/slice.h>

#include "./.array.c"
#include "./.string.c"
// #include "./.search.c"

#define READ_BUF_LEN 8

static inline void __string_init(String string);

void string_init(String string) {
    nul_check(String, string);
    __string_init(string);
}

String string_new() {
    String string = malloc(sizeof *string);
    alloc_check(malloc, string, sizeof *string);
    return __string_init(string), string;
}

// little endian, so the lsb comes first
void string_fmt_anyint_hex(String string, const void *data, size_t align) {
    nul_check(String, string);
    Array arr = (void *)string->arr;
    for (size_t i = 0; i < align; ++i) {
        uint16_t hex = byte_in_hex(deref(uint8_t, data + i));
        void *hex_p = &hex;
        arr_push_back(arr, hex_p);
        arr_push_back(arr, hex_p + 1);
    }
}

void string_cleanup(String string) {
    Array arr = (void *)string->arr;
    arr_cleanup(arr);
}

void *string_drop(String *string) {
    if (string) string_cleanup(*string), free(*string), *string = 0;
    return string;
}

int string_deb_dprint(int fd, String string) {
    if (!string) return dprintf(fd, "(nil)");
    return arr_deb_dprint(fd, (void *)string->arr);
}

int string_deb_print(String string) {
    return fflush(stdout), string_deb_dprint(1, string);
}

int string_dprint(int fd, String string) {
    nul_check(String, string);
    Array arr = (void *)string->arr;
    return write(fd, arr->slice_mem, arr->len * arr->slice_align);
}

int string_print(String string) {
    return fflush(stdout), string_dprint(1, string);
}

size_t string_cap(String string) {
    nul_check(String, string);
    return string->arr_slice_len;
}

size_t string_len(String string) {
    nul_check(String, string);
    return string->arr_len;
}

size_t string_align(String string) {
    nul_check(String, string);
    return string->arr_slice_align;
}

void *string_get(String string, size_t idx) {
    nul_check(String, string);
    return idx < string->arr_len? __arr_get((void *)string->arr, idx): 0;
}

void string_push_back(String string, void *data) {
    nul_check(String, string);
    arr_push_back((void *)string->arr, data);
}

void string_push_front(String string, void *data) {
    nul_check(String, string);
    arr_push_front((void *)string->arr, data);
}

void *string_pop_back(String string) {
    nul_check(String, string);
    return arr_pop_back((void *)string->arr);
}

void *string_pop_front(String string) {
    nul_check(String, string);
    return arr_pop_front((void *)string->arr);
}

void string_vfmt(String string, const char *fmt, va_list args) {
    nul_check(String, string);
    int Cleanup(fd_drop) fd = buffered_vprintf(fmt, args);
    string_from_file(fd, string);
}

void string_fmt(String string, const char *fmt, ...) {
    va_list args;
    nul_check(String, string);
    va_start(args, fmt);
    string_vfmt(string, fmt, args);
    va_end(args);
}

void string_fmt_func(String string, Dprint dprint, void *obj) {
    nul_check(String, string);
    int Cleanup(fd_drop) fd = object_dprint_redirect(obj, dprint);
    assert(fd != -1, "Failed to redirect dprint\n");
    string_from_file(fd, string);
}

void string_from_file(int fd, String string) {
    __string_from_file(fd, string, READ_BUF_LEN);
}

static inline void __string_init(String string) {
    Array arr = (void *)string->arr;
    arr_init(arr, sizeof(uint8_t));
}

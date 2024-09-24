#include <stdlib.h>
#include <string.h>

#include <unistd.h>
#include <fcntl.h>

#include <garage/garage.h>
#include <garage/log.h>
#include <garage/string.h>
#include <garage/ascii.h>

#include "./.string.c"
// #include "./.search.c"

#define READ_BUF_LEN 8

String string_new() {
    return arr_new(1);
}

void string_cleanup(String string) {
    arr_cleanup(string);
}

void string_drop(String *string) {
    if (string && *string) string_cleanup(*string), free(*string), *string = 0;
}

// little endian, so the lsb comes first
void string_from_anyint_hex(String string, const void *data, size_t align) {
    assert(string, "String is not initialized\n");
    for (size_t i = 0; i < align; ++i) {
        uint16_t hex = byte_in_hex(deref(uint8_t, data + i));
        void *hex_p = &hex;
        arr_push_back(string, hex_p);
        arr_push_back(string, hex_p + 1);
    }
}

void string_vfmt(String string, const char *fmt, va_list args) {
    assert(string, "String is not initailized\n");
    int fd = buffered_vprintf(fmt, args);
    string_from_file(fd, string);
    close(fd);
}

void string_fmt(String string, const char *fmt, ...) {
    va_list args;
    assert(string, "String is not initialized\n");
    va_start(args, fmt);
    string_vfmt(string, fmt, args);
    va_end(args);
}

int string_dprint(int fd, String string) {
    assert(string, "String not initialized\n");
    return write(fd, string->mem, string->len * string->align);
}

int string_print(String string) {
    return string_dprint(1, string);
}

void string_from_file(int fd, String string) {
    __string_from_file(fd, string, READ_BUF_LEN);
}

void string_fmt_func(String string, Dprint dprint, void *obj) {
    assert(string, "String is not initialized\n");
    int fd = object_dprint_redirect(obj, dprint);
    assert(fd != -1, "Failed to redirect dprint\n");
    string_from_file(fd, string);
    close(fd);
}

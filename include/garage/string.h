#ifndef GARAGE_STRING_H
#   define GARAGE_STRING_H 1

#include <stdarg.h>

#include <garage/garage.h>
#include <garage/array.h>

typedef struct string {
    Phantom arr;

    void *_mem;
    size_t _len, _cap, _align;
} *String;

// most of the function push additional item to the back of array

void string_init(String string);
String string_new();
void string_from_anyint_hex(String string, const void *data, size_t align);
void string_from_file(int fd, String string);
void string_cleanup(String string);
void string_drop(String *string);

int string_dprint(int fd, String string);
int string_print(String string);

void string_vfmt(String string, const char *fmt, va_list args);
void string_fmt(String string, const char *fmt, ...) __attribute__((format(printf, 2, 3)));
void string_fmt_func(String string, Dprint dprint, void *obj);

#endif // GARAGE_STRING_H

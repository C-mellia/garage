#ifndef GARAGE_STRING_H
#define GARAGE_STRING_H 1

#include <garage/array.h>

// most of the function push additional item to the back of array



void string_from_anyint_hex(Array arr, const void *data, size_t align);
void string_from_file(int fd, Array arr);
void string_vfmt(Array arr, const char *fmt, va_list args);
void string_fmt(Array arr, const char *fmt, ...) __attribute__((format(printf, 2, 3)));
int string_dprint(int fd, Array arr);
int string_print(Array arr);
void string_fmt_func(Array arr, Dprint dprint, void *obj);

#endif // GARAGE_STRING_H

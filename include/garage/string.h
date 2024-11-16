#ifndef GARAGE_STRING_H
#   define GARAGE_STRING_H 1

#include <stdarg.h>

#include <garage/garage.h>
#include <garage/array.h>

typedef struct string {
    Phantom arr;
    struct {
        size_t arr_len;

        Phantom arr_slice;
        struct {
            void *arr_slice_mem;
            size_t arr_slice_align, arr_slice_len;
        };
    };
} *String;

// most of the function push additional item to the back of array

void string_init(String string);
String string_new();
void string_fmt_anyint_hex(String string, const void *data, size_t align);
void string_from_file(int fd, String string);
void string_cleanup(String string);
void *string_drop(String *string);

int string_deb_dprint(int fd, String string);
int string_deb_print(String string);

int string_dprint(int fd, String string);
int string_print(String string);

size_t string_cap(String string);
size_t string_len(String string);
size_t string_align(String string);
void *string_get(String string, size_t idx);

void string_push_back(String string, void *data);
void string_push_front(String string, void *data);
void *string_pop_back(String string);
void *string_pop_front(String string);

void string_vfmt(String string, const char *fmt, va_list args);
void string_fmt(String string, const char *fmt, ...) __attribute__((format(printf, 2, 3)));
void string_fmt_func(String string, Dprint dprint, void *obj);

#endif // GARAGE_STRING_H

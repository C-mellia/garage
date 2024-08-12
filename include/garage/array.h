#ifndef ARRAY_H
#define ARRAY_H 1

#include <stdint.h>
#include <stddef.h>
#include <stdarg.h>
#include <garage/garage.h>

typedef struct Array {
    size_t len, cap, align;
    void *mem;
} *Array;

typedef struct StatArr {
    size_t len, align;
    Phantom mem;
} *StatArr;

Array arr_new(size_t align);
void arr_cleanup(Array arr);
void *arr_get(Array arr, size_t idx);
int arr_reserve(Array arr, size_t cap);
int arr_resize(Array arr, size_t len, const void *data);
Array arr_clone(Array arr);
int arr_reinterp(Array arr, size_t align);

void *arr_push_back(Array arr, const void *data);
void *arr_push_front(Array arr, const void *data);
void *arr_pop_back(Array arr);
void *arr_pop_front(Array arr);

void *arr_front(Array arr);
void *arr_back(Array arr);

int arr_deb_dprint(int fd, Array arr);
int arr_hex_dprint(int fd, Array arr);

int arr_deb_print(Array arr);
int arr_hex_print(Array arr);

void *arr_drop(Array arr, size_t idx);
void *arr_insert(Array arr, size_t idx, const void *data);


StatArr star_new(size_t align, size_t cap);
void star_cleanup(StatArr star);
void *star_get(StatArr star, size_t idx);
StatArr star_clone(StatArr star);
void star_reinterp(StatArr star, size_t align);

int star_deb_dprint(int fd, StatArr star);
int star_hex_dprint(int fd, StatArr star);

int star_deb_print(StatArr star);
int star_hex_print(StatArr star);

#endif // ARRAY_H

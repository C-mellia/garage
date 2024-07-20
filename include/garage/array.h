#ifndef ARRAY_H
#define ARRAY_H 1

#include <stdint.h>
#include <stddef.h>

typedef struct Array {
    void *mem;
    size_t len, cap, layout;
} *Array;

Array arr_new(size_t layout);
void arr_cleanup(Array arr);
void *arr_get(Array arr, size_t idx);
void arr_reserve(Array arr, size_t cap);
int arr_resize(Array arr, size_t len, void *data);
size_t arr_len(Array arr);
size_t arr_layout(Array arr);
Array arr_clone(Array arr);
int arr_interp(Array arr, size_t layout);

void *arr_push_back(Array arr, void *data);
void *arr_push_front(Array arr, void *data);
void *arr_pop_back(Array arr);
void *arr_pop_front(Array arr);

void *arr_front(Array arr);
void *arr_back(Array arr);

int arr_deb_print(Array arr);

void *arr_drop(Array arr, size_t idx);
void *arr_insert(Array arr, size_t idx, void *data);

#endif // ARRAY_H

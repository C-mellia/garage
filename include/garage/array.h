#ifndef ARRAY_H
#   define ARRAY_H 1

#include <stddef.h>

struct Vec;
struct Deque;
struct Slice;
struct RandomEngine;

typedef struct Array {
    size_t len, cap, align;
    void *mem;
} *Array;

void arr_init(Array arr, size_t align);
Array arr_new(size_t align);
Array arr_clone(Array arr);
Array arr_from_slice(struct Slice *slice);
void arr_cleanup(Array arr);
void arr_drop(Array *arr);
void *arr_get(Array arr, size_t idx);
int arr_reserve(Array arr, size_t cap);
int arr_resize(Array arr, size_t len, const void *data);
int arr_reinterp(Array arr, size_t align);
void *arr_dup_mem(Array arr);
void *arr_dup_mem_zero_end(Array arr);

void *arr_push_back(Array arr, const void *data);
void *arr_push_front(Array arr, const void *data);
void *arr_pop_back(Array arr);
void *arr_pop_front(Array arr);

void *arr_front(Array arr);
void *arr_back(Array arr);
void *arr_begin(Array arr);
void *arr_end(Array arr);

int arr_deb_print(Array arr);
int arr_deb_dprint(int fd, Array arr);

int arr_hex_print(Array arr);
int arr_hex_dprint(int fd, Array arr);

__attribute__((nonnull(3)))
int arr_parse(Array arr, struct Slice *slice, int (*parse)(struct Slice *elem, void *data));

void *arr_remove(Array arr, size_t idx);
void *arr_insert(Array arr, size_t idx, const void *data);

void *arr_search_item(Array arr, const void *data);
void *arr_search_mem(Array arr, const void *data, size_t len);
void *arr_search_item_func(Array arr, int (*cmp)(const void *item));

Array arr_from_vec(struct Vec *vec);
Array arr_from_deque(struct Deque *dq);
void arr_random(struct RandomEngine *re, Array/* Array */ arr, size_t align, size_t items);

#endif // ARRAY_H

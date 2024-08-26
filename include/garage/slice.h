#ifndef GARAGE_SCAN_H
#define GARAGE_SCAN_H 1

#include <stddef.h>

struct Array;

typedef struct Slice {
    void *mem;
    size_t align, len;
} *Slice;

// slice.c
Slice slice_new(void *mem, size_t align, size_t len);
Slice slice_from_arr(struct Array *arr);
Slice arr_range(struct Array *arr, size_t begin, size_t end);
Slice arr_range_inc(struct Array *arr, size_t front, size_t back);
Slice slice_clone(Slice slice);
void slice_cleanup(Slice slice);
void slice_drop(Slice *slice);
void *slice_get(Slice slice, size_t idx);

int slice_deb_dprint(int fd, Slice slice);
int slice_deb_print(Slice slice);

int slice_dprint(int fd, Slice slice);
int slice_print(Slice slice);

int slice_hex_dprint(int fd, Slice slice);
int slice_hex_print(Slice slice);

void *slice_front(Slice slice);
void *slice_back(Slice slice);
void *slice_begin(Slice slice);
void *slice_end(Slice slice);

void slice_trim(Slice slice, void *mem, size_t len);
void slice_trim_left(Slice slice, void *mem, size_t len);
void slice_trim_right(Slice slice, void *mem, size_t len);

Slice slice_split_once(Slice slice, void *item);
Slice slice_split_once_mem(Slice slice, void *mem, size_t len);
Slice slice_split_at(Slice slice, void *pos);
Slice slice_split_at_idx(Slice slice, size_t idx);

void *slice_search_item(Slice slice, const void *data);
void *slice_search_mem(Slice slice, const void *data, size_t len);

// algo.c
// cmp: < 0; == 0; > 0
void *slice_lomuto_partition(Slice slice, int (*cmp) (const void *lhs, const void *rhs));
void slice_insertion_sort(Slice slice, int (*cmp)(const void *lhd, const void *rhs));
void *slice_kth_minimum(Slice __slice, size_t k, int (*cmp)(const void *lhs, const void *rhs));
void slice_quick_sort(Slice slice, int (*cmp)(const void *lhs, const void *rhs));

#endif // GARAGE_SCAN_H

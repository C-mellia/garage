#include <alloca.h>
#include <string.h>

#include <garage/slice.h>
#include <garage/log.h>
#include <garage/garage.h>

#include "./.algo.c"

void *slice_lomuto_partition(Slice slice, int (*cmp) (const void *lhs, const void *rhs)) {
    assert(slice, "Slice is not initialized at this point\n");
    return mem_lomuto_partition(slice->mem, slice->len, cmp, slice->align);
}

void slice_insertion_sort(Slice slice, int (*cmp)(const void *lhs, const void *rhs)) {
    assert(slice, "Slice is not initialized at this point\n");
    return mem_insertion_sort(slice->mem, slice->len, cmp, slice->align);
}

void *slice_kth_minimum(Slice slice, size_t k, int (*cmp)(const void *lhs, const void *rhs)) {
    assert(slice, "Slice is not initialized at this point\n");
    return mem_kth_minimum(slice->mem, slice->len, k, cmp, slice->align);
}

void slice_quick_sort(Slice slice, int (*cmp)(const void *lhs, const void *rhs)) {
    assert(slice, "Slice is not initialized at this point\n");
    return mem_quick_sort(slice->mem, slice->len, cmp, slice->align);
}

#include <alloca.h>
#include <string.h>

#include <garage/garage.h>
#include <garage/slice.h>
#include <garage/log.h>
#include <garage/garage.h>

#include "./.algo.c"

void *slice_lomuto_partition(Slice slice, int (*cmp) (const void *lhs, const void *rhs)) {
    nul_check(Slice, slice);
    return mem_lomuto_partition(slice->mem, slice->len, cmp, slice->align);
}

void slice_insertion_sort(Slice slice, int (*cmp)(const void *lhs, const void *rhs)) {
    nul_check(Slice, slice);
    return mem_insertion_sort(slice->mem, slice->len, cmp, slice->align);
}

void *slice_kth_minimum(Slice slice, size_t k, int (*cmp)(const void *lhs, const void *rhs)) {
    nul_check(Slice, slice);
    return mem_kth_minimum(slice->mem, slice->len, k, cmp, slice->align);
}

void slice_quick_sort(Slice slice, int (*cmp)(const void *lhs, const void *rhs)) {
    nul_check(Slice, slice);
    return mem_quick_sort(slice->mem, slice->len, cmp, slice->align);
}

int slice_cmp(Slice lhs, Slice rhs, int (*cmp)(const void *lhs, const void *rhs)) {
    if (!lhs || !rhs) return lhs - rhs;
    assert(lhs->align == rhs->align, "Expected a equivalent alignment\n");
    return mem_cmp(lhs->mem, rhs->mem, lhs->len < rhs->len? lhs->len: rhs->len, cmp, lhs->align);
}

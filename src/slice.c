#include <stdlib.h>
#include <string.h>
#include <regex.h>

#include <garage/garage.h>
#include <garage/slice.h>
#include <garage/log.h>
#include <garage/string.h>
#include <garage/array.h>

#include "./.array.c"
#include "./.slice.c"
#include "./.search.c"

void slice_init(Slice slice, void *mem, size_t align, size_t len) {
    if (!slice) return;
    __slice_init(slice, mem, align, len);
}

Slice slice_new(void *mem, size_t align, size_t len) {
    Slice slice = malloc(sizeof *slice);
    alloc_check(malloc, slice, sizeof *slice);
    return __slice_init(slice, mem, align, len), slice;
}

Slice slice_copy(Slice slice, Slice oth) {
    nul_check(Slice, slice), nul_check(Slice, oth);
    return __slice_init(slice, oth->mem, oth->align, oth->len), slice;
}

Slice slice_move(Slice slice, Slice oth) {
    nul_check(Slice, slice), nul_check(Slice, oth);
    return __slice_init(slice, oth->mem, oth->align, oth->len), memset(oth, 0, sizeof *oth), slice;
}

Slice slice_clone(Slice slice) {
    if (!slice) return 0;
    return slice_new(slice->mem, slice->align, slice->len);
}

void slice_cleanup(Slice slice) {
    (void) slice;
    // if (!slice) return;
    // slice->mem = 0, slice->len = 0;
}

void slice_drop(Slice *slice) {
    if (slice && *slice) slice_cleanup(*slice), free(*slice), *slice = 0;
}

void *slice_get(Slice slice, size_t idx) {
    nul_check(Slice, slice);
    return idx < slice->len? slice->mem + idx * slice->align: 0;
}

void *slice_dup_mem(Slice slice) {
    nul_check(Slice, slice);
    return mem_dup(slice->mem, slice->align, slice->len);
}

void *slice_dup_mem_zero_end(Slice slice) {
    nul_check(Slice, slice);
    return mem_dup_zero_end(slice->mem, slice->align, slice->len);
}

void *slice_front(Slice slice) {
    nul_check(Slice, slice);
    return slice_get(slice, 0);
}

void *slice_back(Slice slice) {
    nul_check(Slice, slice);
    return slice_get(slice, slice->len - 1);
}

void *slice_begin(Slice slice) {
    nul_check(Slice, slice);
    return __slice_begin(slice);
}

void *slice_end(Slice slice) {
    nul_check(Slice, slice);
    return __slice_end(slice);
}

void slice_trim(Slice slice, void *mem, size_t len) {
    nul_check(Slice, slice);
    slice_trim_left(slice, mem, len), slice_trim_right(slice, mem, len);
}

// e.g.
// slice_trim_left(slice/* align=1 */, " \t\n", 3);
// const int data[] = { 1, 2, 3 };
// slice_trim_left(slice/* align=4 */, data, sizeof data / sizeof *data);
void slice_trim_left(Slice slice, void *mem, size_t len) {
    nul_check(Slice, slice);
    if (!mem || !len) return;
    while (slice->len && mem_search_item(mem, mem + slice->align * len, __slice_front(slice), slice->align)) slice->mem += slice->align, --slice->len;
}

void slice_trim_right(Slice slice, void *mem, size_t len) {
    nul_check(Slice, slice);
    if (!mem || !len) return;
    while (slice->len && mem_search_item(mem, mem + slice->align * len, __slice_back(slice), slice->align)) --slice->len;
}

void slice_split(Array/* Slice */ arr, Slice slice, void *mem, size_t len) {
    nul_check(Slice, slice), nul_check(Array, arr);
    if (!mem) return;
    while (slice->len) {
        void *res = mem_search_mem(__slice_begin(slice), __slice_end(slice), mem, len, slice->align);
        size_t left_len = res? (res - __slice_begin(slice) + slice->align - 1) / slice->align: slice->len;
        Slice left = slice_new(__slice_begin(slice), slice->align, left_len);
        slice->mem = max_clamp(res + left->align * len, __slice_end(slice));
        slice->len = left->len + len < slice->len? slice->len - left->len - len: 0;
        arr_push_back(arr, &left);
    }
}

Slice slice_split_once(Slice slice, void *item, size_t len) {
    nul_check(Slice, slice);
    if (!item) return 0;
    void *mem_pos = mem_search_mem(__slice_begin(slice), __slice_end(slice), item, len, slice->align)? : __slice_end(slice);
    Slice left = slice_new(__slice_begin(slice), slice->align, (mem_pos - __slice_begin(slice)) / slice->align);
    slice->mem = max_clamp(mem_pos + slice->align, __slice_end(slice));
    slice->len = slice->len > left->len + 1? slice->len - left->len - 1: 0;
    return left;
}

Slice slice_split_once_mem(Slice slice, void *mem, size_t len) {
    nul_check(Slice, slice);
    if (!mem || !len) return 0;
    void *mem_pos = mem_search_mem(__slice_begin(slice), __slice_end(slice), mem, len, slice->align)? : __slice_end(slice);
    Slice left = slice_new(__slice_begin(slice), slice->align, (mem_pos - slice->mem) / slice->align);
    slice->mem = max_clamp(mem_pos + slice->align * len, __slice_end(slice));
    slice->len = slice->len > left->len + len? slice->len - left->len - len: 0;
    return left;
}

int slice_deb_dprint(int fd, Slice slice) {
    if (!slice || !slice->mem) return dprintf(fd, "(nil)");
    return dprintf(fd, "{mem: %p, align: %zu, len: %zu}",
                   slice->mem, slice->align, slice->len);
}

int slice_deb_print(Slice slice) {
    return fflush(stdout), slice_deb_dprint(1, slice);
}

void *slice_search_item(Slice slice, const void *data) {
    nul_check(Slice, slice);
    return mem_search_item(__slice_begin(slice), __slice_end(slice), data, slice->align);
}

void *slice_search_mem(Slice slice, const void *data, size_t len) {
    nul_check(Slice, slice);
    return mem_search_mem(__slice_begin(slice), __slice_end(slice), data, len, slice->align);
}

void *slice_search_regex(Slice slice, regex_t *regex) {
    nul_check(Slice, slice);
    assert(slice->align == 1, "Expected an alignment of 1, got %zu\n", slice->align);
    return mem_search_regex(__slice_begin(slice), __slice_end(slice), regex, slice->align);
}

Slice slice_split_at(Slice slice, void *pos) {
    nul_check(Slice, slice);
    if (pos < __slice_begin(slice) || pos > __slice_end(slice)) return 0;
    return __slice_split_at(slice, pos);
}

Slice slice_split_at_idx(Slice slice, size_t idx) {
    nul_check(Slice, slice);
    if (idx >= slice->len) return 0;
    return __slice_split_at(slice, __slice_get(slice, idx));
}

int slice_dprint(int fd, Slice slice) {
    if (!slice || !slice->mem) return dprintf(fd, "(nil)");
    return dprintf(fd, "%.*s", (int) slice->len, (char *)slice->mem);
}

int slice_print(Slice slice) {
    return slice_dprint(1, slice);
}

int slice_hex_dprint(int fd, Slice slice) {
    if (!slice) return dprintf(fd, "(nil)");
    String Cleanup(string_drop) string = string_new();
    string_fmt(string, "[");
    for (size_t i = 0; i < slice->len; ++i) {
        string_fmt(string, "0x");
        string_from_anyint_hex(string, __slice_get(slice, i), slice->align);
        if (i + 1 < slice->len) string_fmt(string, ", ");
    }
    string_fmt(string, "]");
    return string_dprint(fd, string);
}

int slice_hex_print(Slice slice) {
    return slice_hex_dprint(1, slice);
}

Slice arr_range(Array arr, size_t begin, size_t end) {
    if (!arr) return 0;
    Slice slice = (void *)arr->slice;
    return begin < end
    ? slice_new(__slice_get(slice, begin), arr->slice_align, end - begin)
    : slice_new(0, arr->slice_align, 0);
}

Slice arr_range_inc(Array arr, size_t front, size_t back) {
    if (!arr) return 0;
    return front < back + 1
    ? slice_new(__arr_get(arr, front), arr->slice_align, back - front + 1)
    : slice_new(0, arr->slice_align, 0);
}

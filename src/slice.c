#include <stdlib.h>
#include <string.h>
#include <regex.h>

#include <garage/slice.h>
#include <garage/log.h>
#include <garage/container.h>
#include <garage/string.h>
#include <garage/array.h>

#include "./.search.c"

static void __unused_symbols(void) __attribute__((unused));

static inline void *__slice_get(Slice slice, size_t idx) __attribute__((always_inline, pure));
static inline void *__slice_begin(Slice slice) __attribute__((always_inline, pure));
static inline void *__slice_end(Slice slice) __attribute__((always_inline, pure));
static inline void *__slice_front(Slice slice) __attribute__((always_inline, pure));
static inline void *__slice_back(Slice slice) __attribute__((always_inline, pure));
static inline void *max_clamp(void *val, void *max) __attribute__((always_inline, pure));

static void __unused_symbols(void) {
    (void) mem_search_item_func;
}

static inline void *__slice_get(Slice slice, size_t idx) {
    return slice->mem + idx * slice->align;
}

static inline void *__slice_begin(Slice slice) {
    return slice->mem;
}

static inline void *__slice_end(Slice slice) {
    return slice->mem + slice->len * slice->align;
}

static inline void *__slice_front(Slice slice) {
    return slice->mem;
}

static inline void *__slice_back(Slice slice) {
    return slice->mem + (slice->len - 1) * slice->align;
}

static inline Slice __slice_split_at(Slice slice, void *pos) {
    Slice left = slice_new(__slice_begin(slice), slice->align, (pos - slice->mem) / slice->align);
    slice->mem = max_clamp(pos, __slice_end(slice));
    return left;
}

static inline void *max_clamp(void *val, void *max) {
    return val > max? max: val;
}

Slice slice_new(void *mem, size_t align, size_t len) {
    Slice slice = malloc(sizeof *slice);
    assert(slice, "Failed to allocate memory of size: %zu\n", sizeof *slice);
    slice->mem = mem, slice->len = len, slice->align = align;
    return slice;
}

Slice slice_from_arr(Array arr) {
    assert(arr, "Array is not initialized at this point\n");
    return slice_new(arr->mem, arr->align, arr->len);
}

void slice_cleanup(Slice slice) {
    if (slice) free(slice);
}

void slice_drop(Slice *slice) {
    if (slice && *slice) slice_cleanup(*slice), *slice = 0;
}

void *slice_get(Slice slice, size_t idx) {
    assert(slice, "Slice is not initialized at this point\n");
    return idx < slice->len? slice->mem + idx * slice->align: 0;
}

void *slice_front(Slice slice) {
    assert(slice, "Slice is not initialized at this point\n");
    return slice_get(slice, 0);
}

void *slice_back(Slice slice) {
    assert(slice, "Slice is not initialized at this point\n");
    return slice_get(slice, slice->len - 1);
}

void *slice_begin(Slice slice) {
    assert(slice, "Slice is not initialized at this point\n");
    return __slice_begin(slice);
}

void *slice_end(Slice slice) {
    assert(slice, "Slice is not initialized at this point\n");
    return __slice_end(slice);
}

void slice_trim(Slice slice, void *mem, size_t len) {
    assert(slice, "Slice is not initialized at this point\n");
    slice_trim_left(slice, mem, len), slice_trim_right(slice, mem, len);
}

// e.g.
// slice_trim_left(slice/* align=1 */, " \t\n", 3);
// const int data[] = { 1, 2, 3 };
// slice_trim_left(slice/* align=4 */, data, sizeof data / sizeof *data);
void slice_trim_left(Slice slice, void *mem, size_t len) {
    assert(slice, "Slice is not initialized at this point\n");
    if (!mem || !len) return;
    while (slice->len && mem_search_item(mem, mem + slice->align * len, __slice_front(slice), slice->align)) slice->mem += slice->align, --slice->len;
}

void slice_trim_right(Slice slice, void *mem, size_t len) {
    assert(slice, "Slice is not initialized at this point\n");
    if (!mem || !len) return;
    while (slice->len && mem_search_item(mem, mem + slice->align * len, __slice_back(slice), slice->align)) --slice->len;
}

Slice slice_split_once(Slice slice, void *item) {
    assert(slice, "Slice is not initialized at this point\n");
    if (!item) return 0;
    void *item_pos = mem_search_item(__slice_begin(slice), __slice_end(slice), item, slice->align)? : __slice_end(slice);
    Slice left = slice_new(__slice_begin(slice), slice->align, (item_pos - __slice_begin(slice)) / slice->align);
    slice->mem = max_clamp(item_pos + slice->align, __slice_end(slice));
    slice->len = slice->len > left->len + 1? slice->len - left->len - 1: 0;
    return left;
}

Slice slice_split_once_mem(Slice slice, void *mem, size_t len) {
    assert(slice, "Slice is not initialized at this point\n");
    if (!mem || !len) return 0;
    void *mem_pos = mem_search_mem(__slice_begin(slice), __slice_end(slice), mem, len, slice->align)? : __slice_end(slice);
    Slice left = slice_new(__slice_begin(slice), slice->align, (mem_pos - slice->mem) / slice->align);
    slice->mem = max_clamp(mem_pos + slice->align * len, __slice_end(slice));
    slice->len = slice->len > left->len + len? slice->len - left->len - len: 0;
    return left;
}

int slice_deb_dprint(int fd, Slice slice) {
    if (!slice) {
        return dprintf(fd, "(nil)");
    } else {
        return dprintf(fd, "{mem: %p, align: %zu, len: %zu}",
                       slice->mem, slice->align, slice->len);
    }
}

int slice_deb_print(Slice slice) {
    return slice_deb_dprint(1, slice);
}

void *slice_search_item(Slice slice, const void *data) {
    assert(slice, "Slice is not initialized at this point\n");
    return mem_search_item(__slice_begin(slice), __slice_end(slice), data, slice->align);
}

void *slice_search_mem(Slice slice, const void *data, size_t len) {
    assert(slice, "Slice is not initialized at this point\n");
    return mem_search_mem(__slice_begin(slice), __slice_end(slice), data, len, slice->align);
}

void *slice_search_regex(Slice slice, regex_t *regex) {
    assert(slice, "Slice is not initialized at this point\n");
    assert(slice->align == 1, "Expected an alignment of 1, got %zu\n", slice->align);
    return mem_search_regex(__slice_begin(slice), __slice_end(slice), regex, slice->align);
}

Slice slice_split_at(Slice slice, void *pos) {
    assert(slice, "Slice is not initialized at this point\n");
    if (pos < __slice_begin(slice) || pos > __slice_end(slice)) return 0;
    return __slice_split_at(slice, pos);
}

Slice slice_split_at_idx(Slice slice, size_t idx) {
    assert(slice, "Slice is not initialized at this point\n");
    if (idx >= slice->len) return 0;
    return __slice_split_at(slice, __slice_get(slice, idx));
}

int slice_dprint(int fd, Slice slice) {
    if (!slice || !slice->mem) {
        return dprintf(fd, "(nil)");
    } else {
        return dprintf(fd, "%.*s", (int) slice->len, (char *)slice->mem);
    }
}

int slice_print(Slice slice) {
    return slice_dprint(1, slice);
}

int slice_hex_dprint(int fd, Slice slice) {
    if (!slice) {
        return dprintf(fd, "(nil)");
    } else {
        String string = string_new();
        string_fmt(string, "[");
        for (size_t i = 0; i < slice->len; ++i) {
            string_fmt(string, "0x");
            string_from_anyint_hex(string, __slice_get(slice, i), slice->align);
            if (i + 1 < slice->len) string_fmt(string, ", ");
        }
        string_fmt(string, "]");
        return ({ int len = string_dprint(fd, string); string_cleanup(string), len; });
    }
}

int slice_hex_print(Slice slice) {
    return slice_hex_dprint(1, slice);
}

Slice arr_range(Array arr, size_t begin, size_t end) {
    if (!arr) return 0;
    return slice_new(arr->mem + begin * arr->align, arr->align, end - begin);
}

Slice arr_range_inc(Array arr, size_t front, size_t back) {
    if (!arr) return 0;
    return slice_new(arr->mem + front * arr->align, arr->align, back - front + 1);
}

Slice slice_clone(Slice slice) {
    if (!slice) return 0;
    return slice_new(slice->mem, slice->align, slice->len);
}

#include <string.h>
#include <stdlib.h>
#include <stdint.h>

#include <unistd.h>

#include <garage/array.h>
#include <garage/string.h>
#include <garage/garage.h>
#include <garage/log.h>
#include <garage/slice.h>
#include <garage/random.h>

#include "./.slice.c"
#include "./.array.c"
#include "./.search.c"

static __attribute__((unused))
void __arr_init(Array arr, size_t align);

void arr_init(Array arr, size_t align) {
    if (!arr) return;
    __arr_init(arr, align);
}

Array arr_new(size_t align) {
    if (!align) return 0;
    Array arr = malloc(sizeof *arr);
    alloc_check(malloc, arr, sizeof *arr);
    return __arr_init(arr, align), arr;
}

Array arr_move(Array arr, Array oth) {
    nul_check(Array, arr), nul_check(Array, oth);
    arr_cleanup(arr);
    void *slice = arr->slice;
    __slice_init(slice, oth->_mem, oth->_align, oth->_len), arr->cap = oth->cap;
    memset(oth, 0, sizeof *oth);
    return arr;
}

Array arr_clone(Array arr) {
    if (!arr) return 0;
    Array new_arr = arr_new(arr->_align);
    if (!arr_check_cap(new_arr, arr->_len)) memcpy(new_arr->_mem, arr->_mem, arr->_len * arr->_align);
    new_arr->_len = arr->_len;
    return new_arr;
}

void arr_cleanup(Array arr) {
    if (!arr) return;
    if (arr->_mem) free(arr->_mem);
    arr->_mem = 0, arr->_len = arr->cap = 0;
}

void arr_drop(Array *arr) {
    if (arr && *arr) arr_cleanup(*arr), free(*arr), *arr = 0;
}

int arr_deb_dprint(int fd, Array arr) {
    if (!arr) return dprintf(fd, "(nil)");
    String Cleanup(string_drop) string = string_new();
    string_fmt(string, "{cap: %lu, ", arr->cap);
    string_fmt_func(string, (void *)slice_deb_dprint, (void *)arr->slice);
    string_fmt(string, "}");
    return string_dprint(fd, string);
}

int arr_deb_print(Array arr) {
    return fflush(stdout), arr_deb_dprint(1, arr);
}

int arr_hex_dprint(int fd, Array arr) {
    if (!arr) return dprintf(fd, "(nil)");
    String Cleanup(string_drop) string = string_new();
    string_fmt(string, "[");
    for (size_t i = 0; i < arr->_len; ++i) {
        string_fmt(string, "0x");
        string_from_anyint_hex(string, __arr_get(arr, i), arr->_align);
        if (i + 1 < arr->_len) string_fmt(string, ", ");
    }
    string_fmt(string, "]");
    return string_dprint(fd, string);
}

int arr_hex_print(Array arr) {
    return arr_hex_dprint(1, arr);
}

void *arr_get(Array arr, size_t idx) {
    nul_check(Array, arr);
    return idx < arr->_len? __slice_get((void *)arr->slice, idx): 0;
}

int arr_reserve(Array arr, size_t cap) {
    nul_check(Array, arr);
    return arr_check_cap(arr, cap);
}

// return 0 if recapped otherwise -1
// set increased memory with provided data offsetted by arr's align, if data is not null
int arr_resize(Array arr, size_t len, const void *data) {
    nul_check(Array, arr);
    int res = arr_check_cap(arr, len);
    void *slice = arr->slice;
    if (data && len > arr->_len) {
        for (size_t i = arr->_len; i < len; ++i) {
            memcpy(__slice_get(slice, i), data, arr->_align);
        }
    }
    arr->_len = len;
    return res;
}

int arr_reinterp(Array arr, size_t align) {
    nul_check(Array, arr);
    if (!align || arr->_align == align) return -1;
    arr->_len = (arr->_len * arr->_align + align - 1) / align, arr->_align = align;
    return 0;
}

void *arr_dup_mem(Array arr) {
    nul_check(Array, arr);
    return mem_dup(arr->_mem, arr->_align, arr->_len);
}

void *arr_dup_mem_zero_end(Array arr) {
    nul_check(Array, arr);
    return mem_dup_zero_end(arr->_mem, arr->_align, arr->_len);
}

void *arr_push_back(Array arr, const void *data) {
    nul_check(Array, arr);
    arr_check_cap(arr, arr->_len + 1);
    if (data) {
        return memcpy(__arr_get(arr, arr->_len++), data, arr->_align);
    } else {
        return memset(__arr_get(arr, arr->_len++), 0, arr->_align);
    }
}

void *arr_push_front(Array arr, const void *data) {
    nul_check(Array, arr);
    arr_check_cap(arr, arr->_len + 1);
    memmove(arr->_mem + arr->_align, arr->_mem, arr->_len++ * arr->_align);
    if (data) {
        return memcpy(arr->_mem, data, arr->_align);
    } else {
        return memset(arr->_mem, 0, arr->_align);
    }
}

void *arr_pop_back(Array arr) {
    nul_check(Array, arr);
    if (!arr->_len) return 0;
    return __slice_get((void *)arr->slice, --arr->_len);
}

void *arr_pop_front(Array arr) {
    nul_check(Array, arr);
    if (!arr->_len) return 0;
    uint8_t buf[arr->_align];
    void *slice = arr->slice;
    memcpy(buf, arr->_mem, sizeof buf);
    memmove(arr->_mem, arr->_mem + arr->_align, --arr->_len * arr->_align);
    return memcpy(__slice_get(slice, arr->_len), buf, sizeof buf);
}

void *arr_front(Array arr) {
    nul_check(Array, arr);
    return arr->_len? __arr_get(arr, 0): 0;
}

void *arr_back(Array arr) {
    nul_check(Array, arr);
    return arr->_len? __arr_get(arr, arr->_len - 1): 0;
}

void *arr_begin(Array arr) {
    nul_check(Array, arr);
    return arr->_mem;
}

void *arr_end(Array arr) {
    nul_check(Array, arr);
    return __slice_get((void *)arr->slice, arr->_len);
}

size_t arr_len(Array arr) {
    nul_check(Array, arr);
    return arr->_len;
}

size_t arr_cap(Array arr) {
    nul_check(Array, arr);
    return arr->cap;
}

int arr_parse(Array arr, Slice __slice, int (*parse)(Slice elem, void *data)) {
    nul_check(Array, arr);
    if (!__slice || !__slice->len) return -1;
    assert(__slice->align == 1, "Expected an alignment of 1, got %zu\n", __slice->align);
    Slice Cleanup(slice_drop) slice = slice_clone(__slice);
    char front = deref(char, slice_front(slice)), back = deref(char, slice_back(slice));
    if (front != '[' || back != ']') return -1;
    slice->len -= 2, ++slice->mem;
    void *data = alloca(arr->_align);
    while (slice->len) {
        Slice Cleanup(slice_drop) elem = slice_split_once(slice, ",", 1);
        slice_trim(elem, " ", 1);
        if (parse(elem, data)) return -1;
        arr_push_back(arr, data);
    }
    return 0;
}

void *arr_remove(Array arr, size_t idx) {
    nul_check(Array, arr);
    if (idx >= arr->_len) return 0;
    uint8_t buf[arr->_align];
    void *slice = arr->slice;
    memcpy(buf, __slice_get(slice, idx), sizeof buf);
    memmove(__slice_get(slice, idx),
            __slice_get(slice, idx + 1),
            idx + 1 == arr->_len? 0: (arr->_len - idx - 2) * arr->_align);
    return memcpy(__slice_get(slice, --arr->_len), buf, sizeof buf);
}

void *arr_insert(Array arr, size_t idx, const void *data) {
    nul_check(Array, arr);
    if (idx > arr->_len) return 0;
    Slice slice = (void *)arr->slice;
    arr_check_cap(arr, arr->_len + 1);
    memmove(__slice_get(slice, idx + 1),
            __slice_get(slice, idx),
            (arr->_len++ - idx) * arr->_align);
    return memcpy(__slice_get(slice, idx), data, arr->_align);
}

void *arr_search_item(Array arr, const void *data) {
    nul_check(Array, arr);
    return data? mem_search_item(arr->_mem, arr->_mem + arr->_len * arr->_align, data, arr->_align): 0;
}

void *arr_search_mem(Array arr, const void *data, size_t len) {
    nul_check(Array, arr);
    return data? mem_search_mem(arr->_mem, arr->_mem + arr->_len * arr->_align, data, len, arr->_align): 0;
}

void *arr_search_item_func(Array arr, int (*cmp)(const void *item)) {
    nul_check(Array, arr);
    return cmp? mem_search_item_func(arr->_mem, arr->_mem + arr->_len * arr->_align, cmp, arr->_align): 0;
}

void arr_random(RandomEngine re, Array/* Array */ arr, size_t align, size_t items) {
    nul_check(RandomEngine, re), nul_check(Array, arr);
    Array subarr = arr_new(align);
    uint8_t item[align * items];
    if (read(re->fd, item, sizeof *item) < (ssize_t)items) return arr_drop(&subarr);
    for (size_t i = 0; i < items; ++i) {
        arr_push_back(subarr, item + align * i);
    }
    arr_push_back(arr, &subarr);
}

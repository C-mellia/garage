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
    Slice slice = (void *)arr->slice;
    __slice_init(slice, oth->slice_mem, oth->slice_align, oth->slice_len), arr->len = oth->len;
    memset(oth, 0, sizeof *oth);
    return arr;
}

Array arr_copy(Array arr, Array oth) {
    nul_check(Array, arr), nul_check(Array, oth);
    arr_check_cap(arr, oth->len);
    memcpy(arr->slice_mem, oth->slice_mem, oth->len * oth->slice_align), arr->len = oth->len;
    return arr;
}

Array arr_clone(Array arr) {
    if (!arr) return 0;
    Array new_arr = arr_new(arr->slice_align);
    if (!arr_check_cap(new_arr, arr->len)) memcpy(new_arr->slice_mem, arr->slice_mem, arr->len * arr->slice_align);
    new_arr->len = arr->len;
    return new_arr;
}

void arr_cleanup(Array arr) {
    if (!arr) return;
    if (arr->slice_mem) free(arr->slice_mem);
    arr->slice_mem = 0, arr->len = arr->slice_len = 0;
}

void *arr_drop(Array *arr) {
    if (arr) arr_cleanup(*arr), free(*arr), *arr = 0;
    return arr;
}

int arr_deb_dprint(int fd, Array arr) {
    if (!arr) return dprintf(fd, "(nil)");
    String Cleanup(string_drop) string = string_new();
    string_fmt(string, "{len: %lu, ", arr->len);
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
    for (size_t i = 0; i < arr->len; ++i) {
        string_fmt(string, "0x");
        string_from_anyint_hex(string, __arr_get(arr, i), arr->slice_align);
        if (i + 1 < arr->len) string_fmt(string, ", ");
    }
    string_fmt(string, "]");
    return string_dprint(fd, string);
}

int arr_hex_print(Array arr) {
    return arr_hex_dprint(1, arr);
}

void *arr_get(Array arr, size_t idx) {
    nul_check(Array, arr);
    return idx < arr->len? __arr_get(arr, idx): 0;
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
    if (data && len > arr->len) {
        for (size_t i = arr->len; i < len; ++i) {
            memcpy(__arr_get(arr, i), data, arr->slice_align);
        }
    }
    arr->len = len;
    return res;
}

int arr_reinterp(Array arr, size_t align) {
    nul_check(Array, arr);
    Slice slice = (void *)arr->slice;
    if (!align || slice->align == align) return -1;
    slice->len = (slice->len * slice->align + align - 1) / align, slice->align = align;
    return 0;
}

void *arr_dup_mem(Array arr) {
    nul_check(Array, arr);
    Slice slice = (void *)arr->slice;
    return mem_dup(slice->mem, slice->align, arr->len);
}

void *arr_dup_mem_zero_end(Array arr) {
    nul_check(Array, arr);
    Slice slice = (void *)arr->slice;
    return mem_dup_zero_end(slice->mem, slice->align, arr->len);
}

void *arr_push_back(Array arr, const void *data) {
    nul_check(Array, arr);
    arr_check_cap(arr, arr->len + 1);
    if (data) {
        return memcpy(__arr_get(arr, arr->len++), data, arr->slice_align);
    } else {
        return memset(__arr_get(arr, arr->len++), 0, arr->slice_align);
    }
}

void *arr_push_front(Array arr, const void *data) {
    nul_check(Array, arr);
    Slice slice = (void *)arr->slice;
    arr_check_cap(arr, arr->len + 1);
    memmove(slice->mem + slice->align, slice->mem, arr->len++ * slice->align);
    if (data) {
        return memcpy(slice->mem, data, slice->align);
    } else {
        return memset(slice->mem, 0, slice->align);
    }
}

void *arr_pop_back(Array arr) {
    nul_check(Array, arr);
    if (!arr->len) return 0;
    return __arr_get(arr, --arr->len);
}

void *arr_pop_front(Array arr) {
    nul_check(Array, arr);
    Slice slice = (void *)arr->slice;
    if (!arr->len) return 0;
    uint8_t buf[slice->align];
    memcpy(buf, slice->mem, sizeof buf);
    memmove(slice->mem, slice->mem + slice->align, --arr->len * slice->align);
    return memcpy(__slice_get(slice, arr->len), buf, sizeof buf);
}

void *arr_front(Array arr) {
    nul_check(Array, arr);
    return arr->len? __arr_get(arr, 0): 0;
}

void *arr_back(Array arr) {
    nul_check(Array, arr);
    return arr->len? __arr_get(arr, arr->len - 1): 0;
}

void *arr_begin(Array arr) {
    nul_check(Array, arr);
    return arr->slice_mem;
}

void *arr_end(Array arr) {
    nul_check(Array, arr);
    return __arr_get(arr, arr->len);
}

size_t arr_len(Array arr) {
    nul_check(Array, arr);
    return arr->len;
}

size_t arr_cap(Array arr) {
    nul_check(Array, arr);
    return arr->slice_len;
}

int arr_parse(Array arr, Slice __in, int (*parse)(Slice elem, void *data)) {
    nul_check(Array, arr);
    if (!__in || !__in->len) return -1;
    assert(__in->align == 1, "Expected an alignment of 1, got %zu\n", __in->align);
    Slice slice = (void *)arr->slice;
    Slice Cleanup(slice_drop) in = slice_clone(__in);
    char front = deref(char, slice_front(in)), back = deref(char, slice_back(in));
    if (front != '[' || back != ']') return -1;
    in->len -= 2, ++in->mem;
    void *data = alloca(slice->align);
    while (in->len) {
        Slice Cleanup(slice_drop) elem = slice_split_once(in, ",", 1);
        slice_trim(elem, " ", 1);
        if (parse(elem, data)) return -1;
        arr_push_back(arr, data);
    }
    return 0;
}

void *arr_remove(Array arr, size_t idx) {
    nul_check(Array, arr);
    Slice slice = (void *)arr->slice;
    if (idx >= arr->len) return 0;
    uint8_t buf[slice->align];
    memcpy(buf, __slice_get(slice, idx), sizeof buf);
    memmove(__slice_get(slice, idx),
            __slice_get(slice, idx + 1),
            idx + 1 == arr->len? 0: (arr->len - idx - 2) * slice->align);
    return memcpy(__slice_get(slice, --arr->len), buf, sizeof buf);
}

void *arr_insert(Array arr, size_t idx, const void *data) {
    nul_check(Array, arr);
    Slice slice = (void *)arr->slice;
    if (idx > arr->len) return 0;
    arr_check_cap(arr, arr->len + 1);
    memmove(__slice_get(slice, idx + 1),
            __slice_get(slice, idx),
            (arr->len++ - idx) * slice->align);
    return memcpy(__slice_get(slice, idx), data, slice->align);
}

void *arr_search_item(Array arr, const void *data) {
    nul_check(Array, arr);
    return data? mem_search_item(arr->slice_mem, __arr_get(arr, arr->len), data, arr->slice_align): 0;
}

void *arr_search_mem(Array arr, const void *data, size_t len) {
    nul_check(Array, arr);
    return data? mem_search_mem(arr->slice_mem, __arr_get(arr, arr->len), data, len, arr->slice_align): 0;
}

void *arr_search_item_func(Array arr, int (*cmp)(const void *item)) {
    nul_check(Array, arr);
    return cmp? mem_search_item_func(arr->slice_mem, __arr_get(arr, arr->len), cmp, arr->slice_align): 0;
}

void arr_random(RandomEngine re, Array/* Array */ arr, size_t align, size_t items) {
    nul_check(RandomEngine, re), nul_check(Array, arr);
    Array subarr = arr_new(align);
    uint8_t item[align * items];
    if (read(re->fd, item, sizeof *item) < (ssize_t)items) {
        arr_drop(&subarr);
    } else {
        for (size_t i = 0; i < items; ++i) {
            arr_push_back(subarr, item + align * i);
        }
        arr_push_back(arr, &subarr);
    }
}

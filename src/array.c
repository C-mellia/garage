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

#include "./.array.c"

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

Array arr_copy(Array arr, Array oth) {
    nul_check(Array, arr), nul_check(Array, oth);
    arr_cleanup(arr), arr->mem = oth->mem, arr->len = oth->len, arr->cap = oth->cap, arr->align = oth->align;
    return arr;
}

Array arr_move(Array arr, Array oth) {
    nul_check(Array, arr), nul_check(Array, oth);
    arr_cleanup(arr), arr->mem = oth->mem, arr->len = oth->len, arr->cap = oth->cap, arr->align = oth->align;
    memset(oth, 0, sizeof *oth);
    return arr;
}

Array arr_clone(Array arr) {
    if (!arr) return 0;
    Array new_arr = arr_new(arr->align);
    if (!arr_check_cap(new_arr, arr->len)) memcpy(new_arr->mem, arr->mem, arr->len * arr->align);
    new_arr->len = arr->len;
    return new_arr;
}

void arr_cleanup(Array arr) {
    if (!arr) return;
    if (arr->mem) free(arr->mem);
    arr->mem = 0, arr->len = arr->cap = 0;
}

void arr_drop(Array *arr) {
    if (arr && *arr) arr_cleanup(*arr), free(*arr), *arr = 0;
}

void *arr_get(Array arr, size_t idx) {
    nul_check(Array, arr);
    return idx < arr->len? arr->mem + idx * arr->align: 0;
}

void *arr_push_back(Array arr, const void *data) {
    nul_check(Array, arr);
    if (!data) return 0;
    arr_check_cap(arr, arr->len + 1);
    return memcpy(arr->mem + arr->len++ * arr->align, data, arr->align);
}

void *arr_push_front(Array arr, const void *data) {
    nul_check(Array, arr);
    if (!data) return 0;
    arr_check_cap(arr, arr->len + 1);
    memmove(arr->mem + arr->align, arr->mem, arr->len++ * arr->align);
    return memcpy(arr->mem, data, arr->align);
}

void *arr_pop_back(Array arr) {
    nul_check(Array, arr);
    if (!arr->len) return 0;
    return arr->mem + --arr->len * arr->align;
}

void *arr_pop_front(Array arr) {
    nul_check(Array, arr);
    if (!arr->len) return 0;
    uint8_t buf[arr->align];
    memcpy(buf, arr->mem, sizeof buf);
    memmove(arr->mem, arr->mem + arr->align, --arr->len * arr->align);
    return memcpy(arr->mem + arr->len * arr->align, buf, sizeof buf);
}

void *arr_remove(Array arr, size_t idx) {
    nul_check(Array, arr);
    if (idx >= arr->len) return 0;
    uint8_t buf[arr->align];
    memcpy(buf, arr->mem + idx * arr->align, sizeof buf);
    memmove(arr->mem + idx * arr->align,
            arr->mem + (idx + 1) * arr->align,
            idx + 1 == arr->len? 0: (arr->len - idx - 2) * arr->align);
    return memcpy(arr->mem + --arr->len * arr->align, buf, sizeof buf);
}

void *arr_insert(Array arr, size_t idx, const void *data) {
    nul_check(Array, arr);
    if (idx > arr->len) return (void *) 0;
    arr_check_cap(arr, arr->len + 1);
    memmove(arr->mem + (idx + 1) * arr->align,
            arr->mem + idx * arr->align,
            (arr->len++ - idx) * arr->align);
    return memcpy(arr->mem + idx * arr->align, data, arr->align);
}

int arr_deb_dprint(int fd, Array arr) {
    if (!arr) {
        return dprintf(fd, "(nil)");
    } else {
        return dprintf(fd, "{mem: %p, len: %lu, cap: %lu, align: %lu}",
               arr->mem, arr->len, arr->cap, arr->align);
    }
}

int arr_deb_print(Array arr) {
    return arr_deb_dprint(1, arr);
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
            memcpy(arr->mem + i * arr->align, data, arr->align);
        }
    }
    arr->len = len;
    return res;
}

int arr_reinterp(Array arr, size_t align) {
    nul_check(Array, arr);
    if (!align || arr->align == align) return -1;
    arr->len = (arr->len * arr->align + align - 1) / align, arr->align = align;
    return 0;
}

void *arr_dup_mem(Array arr) {
    nul_check(Array, arr);
    return mem_dup(arr->mem, arr->align, arr->len);
}

void *arr_dup_mem_zero_end(Array arr) {
    nul_check(Array, arr);
    return mem_dup_zero_end(arr->mem, arr->align, arr->len);
}

int arr_hex_dprint(int fd, Array arr) {
    if (!arr) return dprintf(fd, "(nil)");
    String Cleanup(string_drop) string = string_new();
    string_fmt(string, "[");
    for (size_t i = 0; i < arr->len; ++i) {
        string_fmt(string, "0x");
        string_from_anyint_hex(string, arr_get(arr, i), arr->align);
        if (i + 1 < arr->len) string_fmt(string, ", ");
    }
    string_fmt(string, "]");
    return string_dprint(fd, string);
}

int arr_hex_print(Array arr) {
    return arr_hex_dprint(1, arr);
}

void *arr_front(Array arr) {
    return arr_get(arr, 0);
}

void *arr_back(Array arr) {
    nul_check(Array, arr);
    return arr->len? arr->mem + (arr->len - 1) * arr->align: 0;
}

void *arr_begin(Array arr) {
    nul_check(Array, arr);
    return arr->mem;
}

void *arr_end(Array arr) {
    nul_check(Array, arr);
    return arr->mem + arr->len * arr->align;
}

int arr_parse(Array arr, Slice __slice, int (*parse)(Slice elem, void *data)) {
    nul_check(Array, arr);
    if (!__slice || !__slice->len) return -1;
    assert(__slice->align == 1, "Expected an alignment of 1, got %zu\n", __slice->align);
    Slice Cleanup(slice_drop) slice = slice_clone(__slice);
    char front = deref(char, slice_front(slice)), back = deref(char, slice_back(slice));
    if (front != '[' || back != ']') return -1;
    slice->len -= 2, ++slice->mem;
    void *data = alloca(arr->align);
    while (slice->len) {
        Slice Cleanup(slice_drop) elem = slice_split_once(slice, ",", 1);
        slice_trim(elem, " ", 1);
        if (parse(elem, data)) return -1;
        arr_push_back(arr, data);
    }
    return 0;
}

Array arr_from_slice(Slice slice) {
    if (!slice) return 0;
    Array arr = arr_new(slice->align);
    arr_check_cap(arr, slice->len);
    memcpy(arr->mem, slice->mem, slice->len * slice->align), arr->len = slice->len;
    return arr;
}

void arr_random(RandomEngine re, Array/* Array */ arr, size_t align, size_t items) {
    nul_check(RandomEngine, re), nul_check(Array, arr);
    Array subarr = arr_new(align);
    void *item = alloca(align);
    arr_push_back(arr, &subarr);
    for (size_t i = 0; i < items; ++i) {
        if (read(re->fd, item, align) == -1) return arr_cleanup(arr_pop_back(arr));
        arr_push_back(subarr, item);
    }
}

static void __arr_init(Array arr, size_t align) {
    memset(arr, 0, sizeof *arr), arr->align = align;
}

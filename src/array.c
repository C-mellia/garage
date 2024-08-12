#include <string.h>
#include <stdlib.h>
#include <stdint.h>

#include <unistd.h>

#include <garage/array.h>
#include <garage/string.h>
#include <garage/garage.h>
#include <garage/log.h>

#include "./.array.c"

Array arr_new(size_t align) {
    if (!align) return 0;
    Array arr = malloc(sizeof *arr);
    assert(arr, "malloc failed for sizeof 0x%lx\n", sizeof *arr);
    memset(arr, 0, sizeof *arr), arr->align = align;
    return arr;
}

void arr_cleanup(Array arr) {
    if (arr) {
        if (arr->mem) free(arr->mem);
        free(arr);
    }
}

void *arr_get(Array arr, size_t idx) {
    assert(arr, "Array is not initialized\n");
    return idx < arr->len? arr->mem + idx * arr->align: 0;
}

void *arr_push_back(Array arr, const void *data) {
    assert(arr, "Array is not initialized\n");
    if (!data) return 0;
    arr_check_cap(arr, arr->len + 1);
    return memcpy(arr->mem + arr->len++ * arr->align, data, arr->align);
}

void *arr_push_front(Array arr, const void *data) {
    assert(arr, "Array is not initialized\n");
    if (!data) return 0;
    arr_check_cap(arr, arr->len + 1);
    memmove(arr->mem + arr->align, arr->mem, arr->len++ * arr->align);
    return memcpy(arr->mem, data, arr->align);
}

void *arr_pop_back(Array arr) {
    assert(arr, "Array is not initialized\n");
    if (!arr->len) return 0;
    return arr->mem + --arr->len * arr->align;
}

void *arr_pop_front(Array arr) {
    assert(arr, "Array is not initialized\n");
    if (!arr->len) return 0;
    uint8_t buf[arr->align];
    memcpy(buf, arr->mem, sizeof buf);
    memmove(arr->mem, arr->mem + arr->align, --arr->len * arr->align);
    return memcpy(arr->mem + arr->len * arr->align, buf, sizeof buf);
}

void *arr_drop(Array arr, size_t idx) {
    assert(arr, "Array is not initialized\n");
    if (idx >= arr->len) return 0;
    uint8_t buf[arr->align];
    memcpy(buf, arr->mem + idx * arr->align, sizeof buf);
    memmove(arr->mem + idx * arr->align,
            arr->mem + (idx + 1) * arr->align,
            idx + 1 == arr->len? 0: (arr->len - idx - 2) * arr->align);
    return memcpy(arr->mem + --arr->len * arr->align, buf, sizeof buf);
}

void *arr_insert(Array arr, size_t idx, const void *data) {
    assert(arr && data, "Array is not initialized\n");
    if (idx > arr->len) return (void *) 0;
    arr_check_cap(arr, arr->len + 1);
    memmove(arr->mem + (idx + 1) * arr->align,
            arr->mem + idx * arr->align,
            (arr->len++ - idx) * arr->align);
    return memcpy(arr->mem + idx * arr->align, data, arr->align);
}

void *arr_front(Array arr) {
    assert(arr, "Array is not initialized\n");
    return arr->len? arr->mem: 0;
}

void *arr_back(Array arr) {
    assert(arr, "Array is not initialized\n");
    return arr->len? arr->mem + arr->align * (arr->len - 1): 0;
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
    assert(arr, "Array is not initialized\n");
    return arr_check_cap(arr, cap);
}

// return 0 if recapped otherwise -1
// set increased memory with provided data offsetted by arr's align, if data is not null
int arr_resize(Array arr, size_t len, const void *data) {
    assert(arr, "Array is not initialized\n");
    int res = arr_check_cap(arr, len);
    if (data && len > arr->len) {
        for (size_t i = arr->len; i < len; ++i) {
            memcpy(arr->mem + i * arr->align, data, arr->align);
        }
    }
    arr->len = len;
    return res;
}

Array arr_clone(Array arr) {
    if (!arr) return 0;
    Array new_arr = arr_new(arr->align);
    if (arr_check_cap(new_arr, arr->len) == 0) memcpy(new_arr->mem, arr->mem, arr->len * arr->align);
    new_arr->len = arr->len;
    return new_arr;
}

int arr_reinterp(Array arr, size_t align) {
    assert(arr, "Array is not initialized\n");
    if (!align || arr->align == align) return -1;
    arr->len = (arr->len * arr->align + align - 1) / align, arr->align = align;
    return 0;
}

StatArr star_new(size_t align, size_t len) {
    if (!align) return 0;
    StatArr star = malloc(sizeof *star + align * len);
    assert(star, "Static Array is not initialized\n");
    memset(star, 0, sizeof *star + align * len);
    star->align = align, star->len = len;
    return star;
}

void star_cleanup(StatArr star) {
    if (star) free(star);
}

void *star_get(StatArr star, size_t idx) {
    assert(star, "Static Array is not initialized\n");
    return idx < star->len? star->mem + idx * star->align: 0;
}

StatArr star_clone(StatArr star) {
    StatArr new_star;
    if (!star) return 0;
    new_star = star_new(star->align, star->len);
    memcpy(new_star->mem, star->mem, star->len * star->align);
    return new_star;
}

int star_deb_dprint(int fd, StatArr star) {
    if (!star) {
        return dprintf(fd, "(nil)");
    } else {
        return dprintf(fd, "{len: %lu, align: %lu, mem: %p}",
               star->len, star->align, star->mem);
    }
}

int star_deb_print(StatArr star) {
    return star_deb_dprint(1, star);
}

int arr_hex_dprint(int fd, Array arr) {
    Array str = arr_new(1);
    string_fmt(str, "[");
    for (size_t i = 0; i < arr->len; ++i) {
        string_fmt(str, "0x");
        string_from_anyint_hex(str, arr_get(arr, i), arr->align);
        if (i + 1 < arr->len) string_fmt(str, ", ");
    }
    string_fmt(str, "]");
    return ({ int len = string_dprint(fd, str); arr_cleanup(str), len; });
}

int arr_hex_print(Array arr) {
    return arr_hex_dprint(1, arr);
}

int star_hex_print(StatArr star) {
    return star_hex_dprint(1, star);
}

int star_hex_dprint(int fd, StatArr star) {
    if (!star) return dprintf(fd, "(nil)");
    Array str = arr_new(1);
    string_fmt(str, "[");
    for (size_t i = 0; i < star->len; ++i) {
        string_fmt(str, "0x"), string_from_anyint_hex(str, star_get(star, i), star->align);
        if (i + 1 < star->len) string_fmt(str, ", ");
    }
    string_fmt(str, "]");
    return ({ int len = string_dprint(fd, str); arr_cleanup(str); len; });
}

void star_reinterp(StatArr star, size_t align) {
    assert(star, "Static Array is not initialized\n");
    if (!align || star->align == align) return;
    star->len = (star->len * star->align + align - 1) / align, star->align = align;
}

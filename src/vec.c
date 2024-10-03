#include <stdlib.h>
#include <string.h>

#include <garage/vec.h>
#include <garage/log.h>
#include <garage/string.h>

static inline void __vec_init(Vec vec, size_t align, size_t cap);

void vec_init(Vec vec, size_t align, size_t cap) {
    if (vec) __vec_init(vec, align, cap);
}

Vec vec_new(size_t align, size_t cap) {
    #define SIZE sizeof *vec + align * cap
    if (!align) return 0;
    Vec vec = malloc(SIZE);
    alloc_check(malloc, vec, SIZE);
    return __vec_init(vec, align, cap), vec;
    #undef SIZE
}

Vec vec_zero(size_t align, size_t cap) {
    Vec vec = vec_new(align, cap);
    memset(vec->mem, 0, vec->cap * vec->align);
    return vec;
}

Vec vec_clone(Vec vec) {
    if (!vec) return 0;
    Vec new_vec = vec_new(vec->align, vec->cap);
    memcpy(new_vec->mem, vec->mem, vec->cap * vec->align);
    return new_vec;
}

void vec_cleanup(Vec vec) {
    (void) vec;
}

void vec_drop(Vec *vec) {
    if (vec && *vec) vec_cleanup(*vec), free(*vec), *vec = 0;
}

void *vec_get(Vec vec, size_t idx) {
    nul_check(Vec, vec);
    return idx < vec->cap? vec->mem + idx * vec->align: 0;
}

int vec_deb_dprint(int fd, Vec vec) {
    if (!vec) return dprintf(fd, "(nil)");
    return dprintf(fd, "{cap: %lu, align: %lu, mem: %p}",
                   vec->cap, vec->align, vec->mem);
}

int vec_deb_print(Vec vec) {
    return vec_deb_dprint(1, vec);
}

int vec_hex_dprint(int fd, Vec vec) {
    if (!vec) return dprintf(fd, "(nil)");
    String Cleanup(string_drop) str = string_new();
    string_fmt(str, "[");
    for (size_t i = 0; i < vec->cap; ++i) {
        string_fmt(str, "0x"), string_from_anyint_hex(str, vec_get(vec, i), vec->align);
        if (i + 1 < vec->cap) string_fmt(str, ", ");
    }
    string_fmt(str, "]");
    return string_dprint(fd, str);
}

int vec_hex_print(Vec vec) {
    return vec_hex_dprint(1, vec);
}

void vec_reinterp(Vec vec, size_t align) {
    nul_check(Vec, vec);
    if (!align || vec->align == align) return;
    vec->cap = (vec->cap * vec->align + align - 1) / align, vec->align = align;
}

static inline void __vec_init(Vec vec, size_t align, size_t cap) {
    vec->align = align, vec->cap = cap, memset(vec->mem, 0, cap * align);
}

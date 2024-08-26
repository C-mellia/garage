#include <stdlib.h>
#include <string.h>

#include <garage/statarr.h>
#include <garage/log.h>
#include <garage/string.h>

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

void star_drop(StatArr *star) {
    if (star && *star) star_cleanup(*star), *star = 0;
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

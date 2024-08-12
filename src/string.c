#include <stdlib.h>
#include <string.h>

#include <unistd.h>
#include <fcntl.h>

#include <garage/garage.h>
#include <garage/log.h>
#include <garage/string.h>

#define READ_BUF_LEN 8

static inline uint16_t byte_in_hex(uint8_t byte) {
    char ch[2] = {0};
    ch[0] = byte / 16 + (byte / 16 < 10? '0': 'a' - 10);
    ch[1] = byte % 16 + (byte % 16 < 10? '0': 'a' - 10);
    return *(uint16_t *) ch;
}

static void __string_from_file(int fd, Array arr, size_t buf_len) {
    __label__ L0;
    char buf[buf_len] = {};
L0:
    size_t prev_len = arr->len;
    int cnt = read(fd, buf, buf_len);
    assert(cnt != -1, "%s:%d:%s: failed to read file", __FILE__, __LINE__, __func__);
    arr_resize(arr, arr->len + cnt, 0), memcpy(arr_get(arr, prev_len), buf, cnt);
    if (cnt == (int)buf_len) goto L0;
}

// little endian, so the lsb comes first
void string_from_anyint_hex(Array arr, const void *data, size_t align) {
    assert(arr, "Array is not initialized\n");
    for (size_t i = 0; i < align; ++i) {
        uint16_t hex = byte_in_hex(deref(uint8_t, data + i));
        void *hex_p = &hex;
        arr_push_back(arr, hex_p);
        arr_push_back(arr, hex_p + 1);
    }
}

void string_vfmt(Array arr, const char *fmt, va_list args) {
    assert(arr, "Array is not initailized\n");
    int fd = buffered_vprintf(fmt, args);
    string_from_file(fd, arr);
    close(fd);
}

void string_fmt(Array arr, const char *fmt, ...) {
    va_list args;
    assert(arr, "%s:%d:%s: Array is not initialized\n",
              __FILE__, __LINE__, __func__);
    va_start(args, fmt);
    string_vfmt(arr, fmt, args);
    va_end(args);
}

int string_dprint(int fd, Array arr) {
    assert(arr, "Array not initialized\n");
    assert(arr->align == 1, "Expect an align of 1, got %zu\n", arr->align);
    int res = write(fd, arr->mem, arr->len);
    return res;
}

int string_print(Array arr) {
    return string_dprint(1, arr);
}

void string_from_file(int fd, Array arr) {
    __string_from_file(fd, arr, READ_BUF_LEN);
}

void string_fmt_func(Array arr, Dprint dprint, void *obj) {
    assert(arr, "Array is not initialized\n");
    int fd = object_dprint_redirect(obj, dprint);
    assert(fd, "Failed to redirect dprint\n");
    string_from_file(fd, arr);
    close(fd);
}

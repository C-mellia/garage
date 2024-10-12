#ifndef _GARAGE_STRING_H
#define _GARAGE_STRING_H 1

static inline __attribute__((unused))
uint16_t byte_in_hex(uint8_t byte);
static __attribute__((unused))
void __string_from_file(int fd, String string, size_t buf_len);

static uint16_t byte_in_hex(uint8_t byte) {
    char ch[2] = {0};
    ch[0] = byte / 16 + (byte / 16 < 10? '0': 'a' - 10);
    ch[1] = byte % 16 + (byte % 16 < 10? '0': 'a' - 10);
    return *(uint16_t *) ch;
}

static void __string_from_file(int fd, String string, size_t buf_len) {
    __label__ L0;
    char buf[buf_len] = {};
    Array arr = (void *)string->arr;
L0:
    size_t prev_len = arr->_len;
    int cnt = read(fd, buf, buf_len);
    assert(cnt != -1, "%s:%d:%s: failed to read file", __FILE__, __LINE__, __func__);
    arr_resize(arr, arr->_len + cnt, 0), memcpy(arr_get(arr, prev_len), buf, cnt);
    if (cnt == (int)buf_len) goto L0;
}

#endif // _GARAGE_STRING_H

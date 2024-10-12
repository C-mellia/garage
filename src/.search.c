#ifndef _GARAGE_SEARCH_H
#define _GARAGE_SEARCH_H 1

#include <regex.h>

static __attribute__((unused))
void *mem_search_item(void *begin, void *end, const void *data, size_t align);
static __attribute__((unused))
void *mem_search_item_func(void *begin, void *end, int (*cmp)(const void *item), size_t align);
static __attribute__((unused))
void *mem_search_mem(void *begin, void *end, const void *data, size_t len, size_t align);
static __attribute__((unused))
void *mem_search_regex(void *begin, void *end, const regex_t *regex, size_t align);

static void *mem_search_item(void *begin, void *end, const void *data, size_t align) {
    while (begin < end) {
        if (!memcmp(begin, data, align)) return begin;
        begin += align;
    }
    return 0;
}

static void *mem_search_item_func(void *begin, void *end, int (*cmp)(const void *item), size_t align) {
    while (begin < end) {
        if (cmp(begin)) return begin;
        begin += align;
    }
    return 0;
}

static void *mem_search_mem(void *begin, void *end, const void *data, size_t len, size_t align) {
    size_t range_off;
    void *range_end = begin;
    // printf("%.*s\n", (int)len, (const char *)data);
    while (range_off = range_end - begin, begin < end) {
        if (begin < range_end && memcmp(begin, data, range_off)) {
            begin += align;
        } else if ((size_t) range_off == len * align) {
            return begin;
        } else {
            range_end += align;
        }
    }
    return 0;
}

static void *mem_search_regex(void *begin, void *end, const regex_t *regex, size_t align) {
    regmatch_t match;
    while (begin < end) {
        if (!regexec(regex, begin, 1, &match, 0)) return begin;
        begin += align;
    }
    return 0;
}

#endif // _GARAGE_SEARCH_H

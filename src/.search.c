#ifndef _GARAGE_SEARCH_H
#define _GARAGE_SEARCH_H 1

static __attribute__((unused))
void *mem_search_item(void *begin, void *end, const void *data, size_t align);
static __attribute__((unused))
void *mem_search_item_func(void *begin, void *end, Cmp cmp, size_t align);
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

static void *mem_search_item_func(void *begin, void *end, Cmp cmp, size_t align) {
    while (begin < end) {
        if (cmp(begin)) return begin;
        begin += align;
    }
    return 0;
}

static void *mem_search_mem(void *begin, void *end, const void *data, size_t len, size_t align) {
    size_t range_len;
    void *range_end = begin;
    while (range_len = range_end - begin, begin < end) {
        if (begin < range_end && memcmp(begin, data, range_len)) {
            begin = range_end;
        } else if ((size_t) range_len == len * align) {
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

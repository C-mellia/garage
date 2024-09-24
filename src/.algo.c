#ifndef _GARAGE_ALGO_C
#define _GARAGE_ALGO_C 1

static __attribute__((unused))
void *mem_lomuto_partition(void *mem, size_t len, int (*cmp)(const void *lhs, const void *rhs), size_t align);
static __attribute__((unused))
void mem_insertion_sort(void *mem, size_t len, int (*cmp)(const void *lhs, const void *rhs), size_t align);
static __attribute__((unused))
void *mem_kth_minimum(void *mem, size_t len, size_t k, int (*cmp)(const void *lhs, const void *rhs), size_t align);
static __attribute__((unused))
void mem_quick_sort(void *mem, size_t len, int (*cmp)(const void *lhs, const void *rhs), size_t align);
static __attribute__((unused))
int mem_cmp(void *lhs, void *rhs, size_t len, int (*cmp)(const void *lhs, const void *rhs), size_t align);

static inline void *mem_lomuto_partition(void *mem, size_t len, int (*cmp)(const void *lhs, const void *rhs), size_t align) {
    if (!len) return mem;
    void *front = mem, *back = mem + (len - 1) * align, *pivot = back, *part = front;
    for(void *item = front; item < back; item += align) {
        if (cmp(item, pivot) < 0) {
            if (item != part) memswap(item, part, align);
            part += align;
        }
    }
    if(part != pivot) memswap(part, pivot, align);
    return part;
}

static inline void mem_insertion_sort(void *mem, size_t len, int (*cmp)(const void *lhs, const void *rhs), size_t align) {
    void *begin = mem, *end = mem + len * align, *val = alloca(align), *prev;
    for (void *hole = begin; hole < end; hole += align) {
        memcpy(val, hole, align), prev = hole;
        for (; prev > begin && cmp(val, prev - align) < 0; prev -= align) memcpy(prev, prev - align, align);
        if (prev < hole) memcpy(prev, val, align);
    }
}

static inline void *mem_kth_minimum(void *mem, size_t len, size_t k, int (*cmp)(const void *lhs, const void *rhs), size_t align) {
    if (k >= len) return 0;
    for (void *pivot; ; ) {
        if (pivot = mem_lomuto_partition(mem, len, cmp, align), !pivot) return 0;
        size_t off = (pivot - mem) / align;
        if (off == k) return pivot;
        else if (off < k) mem = pivot + align, len -= off + 1, k -= off + 1;
        else len = off;
    }
}

static inline void mem_quick_sort(void *mem, size_t len, int (*cmp)(const void *lhs, const void *rhs), size_t align) {
    if (!len) return;
    void *pivot = mem_lomuto_partition(mem, len, cmp, align);
    if (!pivot) return;
    size_t off = (pivot - mem) / align;
    mem_quick_sort(mem, off, cmp, align);
    mem_quick_sort(pivot + align, len - off - 1, cmp, align);
}

static inline int mem_cmp(void *lhs, void *rhs, size_t len, int (*cmp)(const void *lhs, const void *rhs), size_t align) {
    for (void *end = lhs + len * align; lhs < end; lhs += align, rhs += align) {
        int res = cmp(lhs, rhs);
        if (res) return res;
    }
    return 0;
}

#endif // _GARAGE_ALGO_C

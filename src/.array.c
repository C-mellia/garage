static inline size_t cap_inc(size_t cap) {
    return cap? cap * 2: 10;
}

static inline void arr_realloc(Array arr, size_t cap) {
    void *new_mem = malloc(cap * arr->align);
    assert(new_mem, "malloc failed for size of 0x%lx\n", cap * arr->align);
    memcpy(new_mem, arr->mem, arr->len * arr->align);
    free(arr->mem), arr->mem = new_mem, arr->cap = cap;
}

static inline int arr_check_cap(Array arr, size_t len) {
    size_t cap = arr->cap;
    while(cap < len) cap = cap_inc(cap);
    if (cap != arr->cap) return arr_realloc(arr, cap), 0;
    return -1;
}

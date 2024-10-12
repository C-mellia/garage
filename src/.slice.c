#ifndef _GARAGE_SLICE_H
#define _GARAGE_SLICE_H 1

static inline __attribute__((always_inline, pure, unused))
void *__slice_get(Slice slice, size_t idx);
static inline __attribute__((always_inline, pure, unused))
void *__slice_begin(Slice slice);
static inline __attribute__((always_inline, pure, unused))
void *__slice_end(Slice slice);
static inline __attribute__((always_inline, pure, unused))
void *__slice_front(Slice slice);
static inline __attribute__((always_inline, pure, unused))
void *__slice_back(Slice slice);
static inline __attribute__((always_inline, unused))
Slice __slice_split_at(Slice slice, void *pos);
static inline __attribute__((always_inline, pure, unused))
void *max_clamp(void *val, void *max);
static __attribute__((unused))
void __slice_init(Slice slice, void *mem, size_t align, size_t len);

static void *__slice_get(Slice slice, size_t idx) {
    return slice->mem + idx * slice->align;
}

static void *__slice_begin(Slice slice) {
    return slice->mem;
}

static void *__slice_end(Slice slice) {
    return slice->mem + slice->len * slice->align;
}

static void *__slice_front(Slice slice) {
    return slice->mem;
}

static void *__slice_back(Slice slice) {
    return slice->mem + (slice->len - 1) * slice->align;
}

static Slice __slice_split_at(Slice slice, void *pos) {
    Slice left = slice_new(__slice_begin(slice), slice->align, (pos - slice->mem) / slice->align);
    slice->mem = max_clamp(pos, __slice_end(slice));
    slice->len -= left->len;
    return left;
}

static void *max_clamp(void *val, void *max) {
    return val > max? max: val;
}

static void __slice_init(Slice slice, void *mem, size_t align, size_t len) {
    slice->mem = mem, slice->align = align, slice->len = len;
}

#endif // _GARAGE_SLICE_H

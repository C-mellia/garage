#ifndef _GARAGE_TENSOR_C
#define _GARAGE_TENSOR_C 1

static inline __attribute__((unused))
void __tensor_init(Tensor tensor, size_t align, size_t *shape, size_t dim, size_t cap);
static inline __attribute__((unused))
void *__tensor_get(Tensor tensor, size_t idx);
static inline __attribute__((unused))
size_t __tensor_get_idx(Tensor tensor, size_t *indices, size_t dim);
static inline __attribute__((unused))
size_t c_array_mult(size_t *arr, size_t len);
static inline __attribute__((unused))
void string_fmt_multidim_mem_hex(String string, void *mem, size_t *shape, size_t dim, size_t align);
static inline __attribute__((unused))
void dummy_add(void *a, void *b);

static void __tensor_init(Tensor tensor, size_t align, size_t *shape, size_t dim, size_t cap) {
    tensor->align = align, tensor->cap = cap, tensor->dim = dim;
    memcpy(tensor->meta, shape, sizeof(size_t) * dim);
}

static void *__tensor_get(Tensor tensor, size_t idx) {
    return (void *)tensor->meta + tensor->dim * sizeof(size_t) + tensor->align * idx;
}

/**
 * Suppose tensor with shape of (2, 3, 4) and indices of (1, 2, 3)
 * the coordinate mapped to the 1D array is 1 * 3 * 4 + 2 * 4 + 3 = 19
 * provided indices with dimension less than the tensor's dimension will be
 * padded with 0 at the end
 */
static size_t __tensor_get_idx(Tensor tensor, size_t *indices, size_t dim) {
    size_t idx = 0;
    for (size_t i = 0; i < tensor->dim; ++i) {
        const size_t dim_idx = i < dim? indices[i]: 0, dim_len = ((size_t *)tensor->meta)[i];
        if (dim_idx >= dim_len) return tensor->cap;
        idx = idx * dim_len + dim_idx;
    }
    return idx;
}

static size_t c_array_mult(size_t *arr, size_t len) {
    size_t res = 1;
    for (size_t i = 0; i < len; ++i) res *= arr[i];
    return res;
}

static void string_fmt_multidim_mem_hex(String string, void *mem, size_t *shape, size_t dim, size_t align) {
    if (!mem || !dim) {
        string_fmt(string, "(nil)");
    } else if (dim == 1) {
        string_fmt(string, "[");
        for (size_t i = 0; i < shape[0]; ++i) {
            string_fmt(string, "0x");
            string_fmt_anyint_hex(string, mem + i * align, align);
            if (i + 1 < shape[0]) string_fmt(string, ", ");
        }
        string_fmt(string, "]");
    } else {
        string_fmt(string, "[");
        for (size_t i = 0; i < shape[0]; ++i) {
            string_fmt_multidim_mem_hex(string, mem + i * c_array_mult(shape + 1, dim - 1) * align, shape + 1, dim - 1, align);
            if (i + 1 < shape[0]) string_fmt(string, ", ");
        }
        string_fmt(string, "]");
    }
}

static void dummy_add(void *a, void *b) {
    (void) a, (void) b;
}

#endif // _GARAGE_TENSOR_C

#include <stdlib.h>
#include <string.h>
#include <garage/prelude.h>
#include <garage/tensor.h>
#include "./.tensor.c"

void tensor_init(size_t dim; Tensor tensor, size_t align, size_t shape[dim], size_t dim) {
    nul_check(Tensor, tensor), __tensor_init(tensor, align, shape, dim, c_array_mult(shape, dim));
}

Tensor tensor_new(size_t dim; size_t align, size_t shape[dim], size_t dim) {
#define alloc_size (sizeof *tensor + sizeof(size_t) * dim + align * cap)
    const size_t cap = c_array_mult(shape, dim);
    Tensor tensor = malloc(alloc_size);
    alloc_check(malloc, tensor, alloc_size);
    return __tensor_init(tensor, align, shape, dim, cap), tensor;
}

#undef alloc_size

void tensor_cleanup(Tensor tensor) {
    (void)tensor;
}

void *tensor_drop(Tensor *tensor) {
    if (tensor) tensor_cleanup(*tensor), free(*tensor), *tensor = 0;
    return tensor;
}

int tensor_deb_dprint(int fd, Tensor tensor) {
    if (!tensor) return dprintf(fd, "(nil)");
    String Cleanup(string_drop) string = string_new();
    string_fmt(string, "{align: %zu, cap: %zu, ",
               tensor->align, tensor->cap);
    if (!tensor->dim) {
        string_fmt(string, "(nil)}");
    } else {
        string_fmt(string, "shape: (");
        for (size_t i = 0; i < tensor_dim(tensor); ++i) {
            string_fmt(string, "%zu", tensor_idx_len(tensor, i));
            if (i + 1 < tensor_dim(tensor)) string_fmt(string, ", ");
        }
        string_fmt(string, "), mem: %p}", tensor_begin(tensor));
    }
    return string_dprint(fd, string);
}

int tensor_deb_print(Tensor tensor) {
    return fflush(stdout), tensor_deb_dprint(1, tensor);
}

/**
 * [[0x00, 0x01], [0x02, 0x03], [0x04, 0x05]] (3, 2)
 * [[[0x00, 0x01], [0x02, 0x03]], [[0x04, 0x05], [0x06, 0x07]]] (2, 2, 2)
 */
int tensor_hex_dprint(int fd, Tensor tensor) {
    if (!tensor) return dprintf(fd, "(nil)");
    String Cleanup(string_drop) string = string_new();
    string_fmt_multidim_mem_hex(string, __tensor_get(tensor, 0), (void *)tensor->meta, tensor->dim, tensor->align);
    return string_dprint(fd, string);
}

int tensor_hex_print(Tensor tensor) {
    return fflush(stdout), tensor_hex_dprint(1, tensor);
}

size_t *tensor_shape(Tensor tensor) {
    nul_check(Tensor, tensor);
    return (void *)(tensor->meta);
}

void *tensor_begin(Tensor tensor) {
    nul_check(Tensor, tensor);
    return tensor->cap? __tensor_get(tensor, 0): 0;
}

void *tensor_end(Tensor tensor) {
    nul_check(Tensor, tensor);
    return tensor->cap? __tensor_get(tensor, tensor->cap): 0;
}

size_t tensor_dim(Tensor tensor) {
    nul_check(Tensor, tensor);
    return tensor->dim;
}

size_t tensor_cap(Tensor tensor) {
    nul_check(Tensor, tensor);
    return tensor->cap;
}

/**
 * get the length of the idx-th dimension
 * e.g. for shape of (2, 3, 4), idx_len(0) = 2, idx_len(1) = 3, idx_len(2) = 4
 */
size_t tensor_idx_len(Tensor tensor, size_t idx) {
    nul_check(Tensor, tensor);
    return idx < tensor->dim? ((size_t *)tensor->meta)[idx]: 0;
}

void *tensor_get(Tensor tensor, size_t idx) {
    nul_check(Tensor, tensor);
    return idx < tensor->cap? __tensor_get(tensor, idx): 0;
}

void *tensor_idx(Tensor tensor, size_t *indices, size_t dim) {
    nul_check(Tensor, tensor);
    size_t off = indices? __tensor_get_idx(tensor, indices, dim): 0;
    return off < tensor->cap? __tensor_get(tensor, off): 0;
}

void tensor_add(Tensor tensor, void *data, void (*add)(void *, void *)) {
    nul_check(Tensor, tensor);
    if (!data) return;
    add = add? : dummy_add;
    for (size_t i = 0; i < tensor->cap; ++i) {
        add(__tensor_get(tensor, i), data);
    }
}

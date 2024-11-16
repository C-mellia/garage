#ifndef GARAGE_TENSOR_H
#define GARAGE_TENSOR_H 1

#include <garage/types.h>

typedef struct tensor {
    size_t align, cap, dim;
    /**
     * shape: [size_t; dim]
     * mem: [<T>; cap]
     */
    Phantom meta;
} *Tensor;

void tensor_init(size_t dim; Tensor tensor, size_t align, size_t shape[dim], size_t dim);
Tensor tensor_new(size_t dim; size_t align, size_t shape[dim], size_t dim);
void tensor_cleanup(Tensor tensor);
void *tensor_drop(Tensor *tensor);

int tensor_deb_dprint(int fd, Tensor tensor);
int tensor_deb_print(Tensor tensor);

int tensor_hex_dprint(int fd, Tensor tensor);
int tensor_hex_print(Tensor tensor);

size_t *tensor_shape(Tensor tensor);
void *tensor_begin(Tensor tensor);
void *tensor_end(Tensor tensor);
size_t tensor_dim(Tensor tensor);
size_t tensor_cap(Tensor tensor);

size_t tensor_idx_len(Tensor tensor, size_t idx);
void *tensor_get(Tensor tensor, size_t idx);
void *tensor_idx(Tensor tensor, size_t *indices, size_t dim);

void tensor_add(Tensor tensor, void *data, void (*add)(void *, void *));
void tensor_scale(Tensor tensor, void *data, void (*scale)(void *, void *));

#endif // GARAGE_TENSOR_H

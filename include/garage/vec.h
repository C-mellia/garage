#ifndef GARAGE_VEC_H
#   define GARAGE_VEC_H 1

#include <garage/garage.h>

struct array;
struct deque;

typedef struct vec {
    size_t align, cap;
    Phantom mem;
} *Vec;

void vec_init(Vec vec, size_t align, size_t cap);
Vec vec_new(size_t align, size_t cap);
Vec vec_zero(size_t align, size_t cap);
Vec vec_clone(Vec vec);
void vec_cleanup(Vec vec);
void *vec_get(Vec vec, size_t idx);
void vec_reinterp(Vec vec, size_t align);

int vec_deb_dprint(int fd, Vec vec);
int vec_deb_print(Vec vec);

int vec_hex_dprint(int fd, Vec vec);
int vec_hex_print(Vec vec);

void *vec_search_item(Vec vec, const void *data);
void *vec_search_mem(Vec vec, const void *data, size_t len);

Vec vec_from_arr(struct array *arr);
Vec vec_from_deque(struct deque *dq);

#endif // GARAGE_VEC_H

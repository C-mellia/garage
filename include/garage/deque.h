#ifndef GARAGE_QUEUE_H
#   define GARAGE_QUEUE_H 1

#include <garage/garage.h>
#include <garage/array.h>

struct vec;
struct array;

typedef struct deque {
    size_t begin, len;

    Phantom slice;
    struct {
        void *slice_mem;
        size_t slice_align, slice_len;
    };
} *Deque;

void deq_init(Deque deq, size_t align);
Deque deq_new(size_t align);
void deq_cleanup(Deque deq);
void *deq_drop(Deque *deq);

int deq_deb_dprint(int fd, Deque deq);
int deq_deb_print(Deque deq);

int deq_hex_dprint(int fd, Deque deq);
int deq_hex_print(Deque deq);

int deq_idx_dprint(int fd, Deque deq);
int deq_idx_print(Deque deq);

void deq_reserve(Deque deq, size_t len);
void deq_resize(Deque deq, size_t len, void *data);

void *deq_push_back(Deque deq, void *data);
void *deq_push_front(Deque deq, void *data);
void *deq_pop_back(Deque deq);
void *deq_pop_front(Deque deq);

void *deq_remove(Deque deq, size_t idx);
void *deq_insert(Deque deq, size_t idx, void *data);

void *deq_get(Deque deq, size_t idx);
void *deq_front(Deque deq);
void *deq_back(Deque deq);
size_t deq_len(Deque deq);

void *deq_search_item(Deque deq, const void *data);
void *deq_search_mem(Deque deq, const void *data, size_t len);

Deque deq_from_arr(struct array *arr);
Deque deq_from_vec(struct vec *vec);

#endif // GARAGE_QUEUE_H

#ifndef GARAGE_QUEUE_H
#define GARAGE_QUEUE_H 1

#include <garage/array.h>

typedef struct Deque {
    void *data, *begin;
    size_t len, cap, layout;
} *Deque;

Deque deque_new(size_t layout);
void deque_cleanup(Deque dq);
void *deque_push_back(Deque dq, void *data);
void *deque_push_front(Deque dq, void *data);
void *deque_pop_back(Deque dq);
void *deque_pop_front(Deque dq);
size_t deque_len(Deque dq);
void *deque_get(Deque dq, size_t idx);
void *deque_insert(Deque dq, size_t idx, void *data);

void *deque_front(Deque dq);
void *deque_back(Deque dq);

void deque_deb_print(Deque dq);

#endif // GARAGE_QUEUE_H

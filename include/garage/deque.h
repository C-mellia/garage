#ifndef GARAGE_QUEUE_H
#define GARAGE_QUEUE_H 1

#include <garage/array.h>

typedef struct Stack {
    void *mem, *top;
    size_t len, cap, align;
} *Stack;

typedef struct Deque {
    void *mem, *begin;
    size_t len, cap, align;
} *Deque;

Deque deque_new(size_t align);
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

Stack stack_new(size_t align);
void stack_cleanup(Stack st);
void *stack_push(Stack st, void *data);
void *stack_pop(Stack st);
void *stack_get(Stack st, size_t idx);
void *stack_insert(Stack st, size_t idx, void *data);

void *stack_top(Stack st);
void *stack_bottom(Stack st);

#endif // GARAGE_QUEUE_H

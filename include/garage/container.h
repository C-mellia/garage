#ifndef GARAGE_CONTAINER_H
#define GARAGE_CONTAINER_H 1

#include <garage/array.h>
#include <garage/deque.h>

void *arr_search_item(Array arr, const void *data);
void *arr_search_mem(Array arr, const void *data, size_t len);

void *star_search_item(StatArr star, const void *data);
void *star_search_mem(StatArr star, const void *data, size_t len);

void *deque_search_item(Deque dq, const void *data);
void *deque_search_mem(Deque dq, const void *data, size_t len);

Array arr_from_star(StatArr arr);
StatArr star_from_arr(Array arr);
Deque deque_from_arr(Array arr);
Deque deque_from_star(StatArr star);
Array arr_from_deque(Deque dq);
StatArr star_from_deque(Deque dq);

#endif // GARAGE_CONTAINER_H

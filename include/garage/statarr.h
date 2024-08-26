#ifndef GARAGE_STATARR_H
#   define GARAGE_STATARR_H 1

#include <garage/garage.h>

struct Array;
struct Deque;

typedef struct StatArr {
    size_t len, align, cap;
    Phantom mem;
} *StatArr;

StatArr star_new(size_t align, size_t cap);
void star_cleanup(StatArr star);
void *star_get(StatArr star, size_t idx);
StatArr star_clone(StatArr star);
void star_reinterp(StatArr star, size_t align);

int star_deb_dprint(int fd, StatArr star);
int star_hex_dprint(int fd, StatArr star);

int star_deb_print(StatArr star);
int star_hex_print(StatArr star);

void *star_search_item(StatArr star, const void *data);
void *star_search_mem(StatArr star, const void *data, size_t len);

StatArr star_from_arr(struct Array *arr);
StatArr star_from_deque(struct Deque *dq);

#endif // GARAGE_STATARR_H

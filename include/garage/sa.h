#ifndef GARAGE_SA_H
#   define GARAGE_SA_H 1

#include <garage/garage.h>

#define MAX_OFFS 0x10L

typedef struct sa {
    void *top;
    size_t cap, offs[MAX_OFFS], *off;
    Phantom mem;
} *Sa;

void sa_init(Sa sa, size_t cap);
Sa sa_new(size_t cap);
void sa_cleanup(Sa sa);
void sa_drop(Sa *sa);

int sa_deb_dprint(int fd, Sa sa);
int sa_deb_print(Sa sa);

void *sa_alloc(Sa sa, size_t bytes);
void sa_push(Sa sa);
void sa_pop(Sa sa);

int sa_stack_empty(Sa sa);
size_t sa_stack_size(Sa sa);

void sa_diag(Sa sa);

#endif // GARAGE_SA_H

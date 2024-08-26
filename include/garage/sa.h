#ifndef GARAGE_SA_H
#   define GARAGE_SA_H 1

#define MAX_OFFS 0x10L

typedef struct Sa {
    void *mem, *top;
    size_t cap;
    size_t offs[MAX_OFFS];
    size_t *off;
    pthread_mutex_t m;
} *Sa;

Sa sa_new(size_t cap);
void sa_cleanup(Sa sa);
void sa_drop(Sa *sa);

void *sa_alloc(Sa sa, size_t bytes);
void sa_push(Sa sa);
void sa_pop(Sa sa);

int sa_stack_empty(Sa sa);
size_t sa_stack_size(Sa sa);

void sa_diag(Sa sa);

#endif // GARAGE_SA_H

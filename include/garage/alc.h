#ifndef GARAGE_ALC_H
#   define GARAGE_ALC_H 1

#include <garage/garage.h>

typedef struct alc_node {
    struct alc_node *next, *prev;
    size_t len;
} *AlcNode;

typedef struct alc {
    size_t cap;
    Phantom mem;
} *Alc;

void alc_init(Alc alc, size_t cap);
Alc alc_new(size_t cap);
void alc_cleanup(Alc alc);
void alc_drop(Alc *alc);

void *alc_alloc(Alc alc, size_t len);
int alc_free(Alc alc, void *ptr);
int alc_node_count(Alc alc);

int alc_deb_dprint(int fd, Alc alc);
int alc_deb_print(Alc alc);

#endif // GARAGE_ALC_H

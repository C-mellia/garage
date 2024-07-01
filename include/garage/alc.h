#ifndef GARAGE_ALC_H
#define GARAGE_ALC_H 1

typedef struct AlcNode {
    struct AlcNode *next, *prev;
    size_t len;
} *AlcNode;

typedef struct Alc {
    AlcNode begin;
    size_t cap;
} *Alc;

Alc alc_new(size_t cap);
int alc_cleanup(Alc alc);
void *alc_alloc(Alc alc, size_t len);
int alc_free(Alc alc, void *ptr);

#endif // GARAGE_ALC_H

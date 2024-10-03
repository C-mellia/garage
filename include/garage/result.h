#ifndef GARAGE_RESULT_H
#define GARAGE_RESULT_H 1

typedef struct result {
    int tag;
    size_t cap;
    Phantom data;
} *Result;

void res_init(Result res, int tag, void *src, size_t cap);
Result res_new(int tag, void *src, size_t cap);
void res_cleanup(Result res);
void res_drop(Result *res);
void *res_consume(Result res);

int res_deb_dprint(int fd, Result res);
int res_deb_print(Result res);

#endif // GARAGE_RESULT_H

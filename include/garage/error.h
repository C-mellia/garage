#ifndef GARAGE_ERROR_H
#define GARAGE_ERROR_H 1

typedef struct Result {
    int code;
    size_t len;
    Phantom data;
} *Result;

Result ok_new(void *src, size_t len);
Result err_new(const char *fmt, ...);
void res_cleanup(Result res);
void *res_consume(Result res, int fd);

#endif // GARAGE_ERROR_H

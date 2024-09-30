#ifndef GARAGE_SCANNER_H
#define GARAGE_SCANNER_H 1

#include <stddef.h>

#include <garage/garage.h>

typedef struct Scanner {
    size_t pos;

    Phantom /* Array<String> */errs;
    void *_errs_mem;
    size_t _errs_len, _errs_cap, _errs_align;

    Phantom /* Slice<char> */src;
    void *_src_mem;
    size_t _src_align, _src_len;
} *Scanner;

void scnr_init(Scanner scnr, void *mem, size_t align, size_t len);
Scanner scnr_new(void *mem, size_t align, size_t len);
void scnr_cleanup(Scanner scnr);
void scnr_drop(Scanner *scnr);

int scnr_deb_dprint(int fd, Scanner scnr);
int scnr_deb_print(Scanner scnr);

void *scnr_peek(Scanner scnr, size_t off);
size_t scnr_consume(Scanner scnr, size_t count);

#endif // GARAGE_SCANNER_H

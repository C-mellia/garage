#ifndef GARAGE_H
#define GARAGE_H 1

#include <stdio.h>
#include <stdint.h>
#include <pthread.h>

// ## macros ##

#define deref(Type, ptr) *(Type *)(ptr)
// defines a item in a enum string array to help stringify enums
// for example:
/// typedef enum Enum {
///     FOO,
///     BAR,
///     __ENUM_COUNT,
/// } Enum;
///
/// static struct { const char *str, size_t len; } const enum_str[] = {
///     ENUM_STR_ITEM(FOO),
///     ENUM_STR_ITEM(BAR),
/// };
#define ENUM_STR_ITEM(tag) [tag] = { .str = #tag, .len = sizeof(#tag) - 1 }

#define MAX_OFFS 0x10


// ## interface ##

typedef uint8_t Phantom[0];
typedef int (*Dprint)(int fd, void *obj);
typedef struct StackAllocator {
    void *mem, *top;
    size_t cap;
    size_t offs[MAX_OFFS];
    size_t *off;
    pthread_mutex_t m;
} *StackAllocator;

void setup_env(char *logfname, int auto_report, int fallback_to_stderr, void (*exec_startup)(void), void (*exec_cleanup)(void));
void cleanup(void);
void _abort(void);
void gracefully_exit(void);
int buffered_printf(const char *fmt, ...) __attribute__((format(printf, 1, 2)));
int buffered_vprintf(const char *fmt, va_list args);
int object_dprint_redirect(void *obj, Dprint dprint);

StackAllocator sa_new(size_t cap);
void *sa_alloc(StackAllocator sa, size_t bytes);
void sa_push(StackAllocator sa);
void sa_pop(StackAllocator sa);
int sa_stack_empty(StackAllocator sa);
size_t sa_stack_size(StackAllocator sa);
void sa_cleanup(StackAllocator sa);
void sa_diag(StackAllocator sa);

#endif // GARAGE_H

#ifndef GARAGE_H
#   define GARAGE_H 1

#include <stdio.h>
#include <stdint.h>
#include <pthread.h>

// ## macros ##

#define deref(Type, ptr) *(Type *)(ptr)
/// defines a item in a enum string array to help stringify enums
/// for example:
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

/// assign a cleanup function to a variable that will be called when the
/// variable goes out of scope
/// .e.g:
/// void cleanup(void *ptr) { free(ptr); }
/// void *ptr Cleanup(cleanup) = malloc(10);
#define Cleanup(func) __attribute__((cleanup(func)))

// ## interface ##

typedef uint8_t Phantom[0];
typedef int (*Dprint)(int fd, void *obj);

__attribute__((nonnull(1, 2)))
void memswap(void *lhs, void *rhs, size_t align);
void setup_env(char *logfname, int auto_report, int fallback_to_stderr, void (*exec_startup)(void), void (*exec_cleanup)(void));
void cleanup(void);
void _abort(void);
void gracefully_exit(void);
int buffered_printf(const char *fmt, ...) __attribute__((format(printf, 1, 2)));
int buffered_vprintf(const char *fmt, va_list args);
int object_dprint_redirect(void *obj, Dprint dprint);
void fd_drop(int *fd);

#endif // GARAGE_H

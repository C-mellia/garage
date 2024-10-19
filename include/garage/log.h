#ifndef GARAGE_LOG_H
#   define GARAGE_LOG_H 1

#include <garage/garage.h>

#ifndef GARAGE_RELEASE
#define probe() \
    log_fmt("%s:%d:%s: ", __FILE__, __LINE__, __func__)
#else // GARAGE_RELEASE
#define probe() (void)0
#endif // GARAGE_RELEASE

#define assert(cond, msg, ...) \
    (void)(!!(cond) || log_fmt("%s:%d:%s: " msg, __FILE__, __LINE__, __func__, ##__VA_ARGS__))
#define panic(msg, ...) \
    (void) (log_fmt("%s:%d:%s: " msg, __FILE__, __LINE__, __func__, ##__VA_ARGS__), _abort())
#define nul_check(Type, obj) assert((obj), #Type "(" #obj ") is NULL at this point\n")
#define alloc_check(allocate, ptr, size) assert((ptr), #allocate " failed for size of 0x%lx\n", size);


extern int logfd;

int log_fmt(const char *msg, ...) __attribute__((format(printf, 1, 2)));

#endif // GARAGE_LOG_H

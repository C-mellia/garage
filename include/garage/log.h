#ifndef GARAGE_LOG_H
#   define GARAGE_LOG_H 1

#include <garage/garage.h>

#ifndef GARAGE_RELEASE
#define probe() \
    log_fmt("%s:%d:%s: ", __FILE__, __LINE__, __func__)
#else // GARAGE_RELEASE
#define probe()
#endif // GARAGE_RELEASE

#define assert(cond, msg, ...) \
    (void) ((cond) || (panic(msg, ##__VA_ARGS__), 0))
#define panic(msg, ...) \
    (probe(), log_fmt(msg, ##__VA_ARGS__), _abort())
#define nul_check(Type, obj) assert((obj), #Type " is NULL at this point\n")
#define alloc_check(allocate, ptr, size) assert((ptr), #allocate " failed for size of 0x%lx\n", size);


extern int logfd;

void log_fmt(const char *msg, ...) __attribute__((format(printf, 1, 2)));

#endif // GARAGE_LOG_H

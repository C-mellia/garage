#ifndef GARAGE_LOG_H
#   define GARAGE_LOG_H 1

#include <garage/garage.h>

#ifndef GARAGE_RELEASE
#define probe() \
    report("%s:%d:%s: ", __FILE__, __LINE__, __func__)
#else // GARAGE_RELEASE
#define probe()
#endif // GARAGE_RELEASE

#define assert(cond, msg, ...) \
    (void) ((cond) || (panic(msg, ##__VA_ARGS__), 0))
#define panic(msg, ...) \
    probe(), report(msg, ##__VA_ARGS__), _abort()

extern int logfd;

void report(const char *msg, ...) __attribute__((format(printf, 1, 2)));

#endif // GARAGE_LOG_H

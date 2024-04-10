#ifndef GARAGE_H
#define GARAGE_H 0

#include <stdio.h>
#include <stdint.h>

// ## macros ##


#define code_probe() \
    report("INFO: probing into code at file: %s(%d):%s\n", __FILE__, __LINE__, __FUNCTION__)\

#define code_trap(cond, msg, ...) \
	if (!(cond)) panic(msg, ##__VA_ARGS__)

#define panic(msg, ...) do {\
	code_probe();\
	report(msg, ##__VA_ARGS__);\
    _abort();\
} while(0)

#define MAX_OFFS 0x10


// ## interface ##


typedef struct StackAllocator {
    void *mem, *top;
    size_t cap;
	size_t offs[MAX_OFFS];
	size_t *off;
} *StackAllocator;

typedef struct App {
	char *logfname;
	int auto_report;
	int fallback_to_stderr;
    void (*exec_startup)(void);
    void (*exec_cleanup)(void);
} App;

extern int logfd;
extern App app;

void setup_env(void);
void handle_signal(int sig);
void cleanup(void);
void report(const char *msg, ...);
void _abort(void);

StackAllocator sa_new(size_t cap);
void *sa_alloc(StackAllocator sa, size_t bytes);
void sa_push(StackAllocator sa);
void sa_pop(StackAllocator sa);
int sa_stack_empty(StackAllocator sa);
void sa_cleanup(StackAllocator sa);
void sa_diag(StackAllocator sa);

#endif // GARAGE_H

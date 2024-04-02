#ifndef GARAGE_H
#define GARAGE_H 0

/*
#include <stdlib.h>
#include <signal.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
 */


// ## macros ##


#define code_probe() \
    dprintf(logfd, "INFO: probing into code at file: %s(%d):%s\n", __FILE__, __LINE__, __FUNCTION__)\

#define code_trap(cond, msg, ...) \
	if (!(cond)) panic(msg, ##__VA_ARGS__)

#define panic(msg, ...) do {\
	code_probe();\
	dprintf(logfd, msg, ##__VA_ARGS__);\
	raise(SIGABRT);\
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
} App;

StackAllocator ga = 0;
int logfd = -1;
App app = {0};

void setup_env(void);
void handle_signal(int sig);
void cleanup(void);

GlobAlloc sa_new(size_t cap);
void *sa_alloc(GlobAlloc g, size_t bytes);
void sa_push(GlobAlloc g);
void sa_pop(GlobAlloc g);
int sa_stack_empty(GlobAlloc g);
void sa_cleanup(GlobAlloc g);

// ## implementation ##


StackAllocator sa_new(size_t cap) {
    StackAllocator g = malloc(sizeof *g);
    code_trap(g, "Failed to initialize saal allocator at\n");
    g->mem = malloc(cap);
    code_trap(g->mem, "Failed to initialize saal allocator due to big size memory allocation not supported\n");
	g->off = g->offs + MAX_OFFS;
    g->top = g->mem + cap;
    g->cap = cap;
    return g;
}

void *sa_alloc(StackAllocator g, size_t bytes) {
	if (sa_stack_empty(g) && logfd > 0) {
		dprintf(logfd, "WARNING: saal allocated memory will not be released by glob_pop\n");
	}
    return g->top < g->mem + bytes? 0: ({g->top -= bytes;});
}

void sa_pop(StackAllocator g) {
	code_trap(g && g->mem, "ERROR: ga not initialized\n");
	if (g && g->mem) {
		g->top = g->mem + *g->off;
		++g->off;
	}
}

void sa_push(StackAllocator g) {
	if (g && g->mem) {
		code_trap(g->off > g->offs, "Global allocator stack overflow\n");
		*(--g->off) = g->top - g->mem;
	}
}

int sa_stack_empty(StackAllocator g) {
	return g->off == g->offs + MAX_OFFS;
}

void sa_cleanup(StackAllocator g) {
    if (g) {
		if (!sa_stack_empty(g) && logfd > 0) {
			dprintf(logfd, "WARNING: undefined behavior due to unbalanced sa_push and glob_pop\n");
		}
        if (g->mem) free(g->mem);
        free(g);
    }
}

void setup_env(void) {
	signal(SIGABRT, handle_signal);
	signal(SIGSEGV, handle_signal);
	ga = sa_new(0x1000);
	if (app.logfname) {
		logfd = open(app.logfname, 
				O_WRONLY | O_CREAT | O_TRUNC, 
				S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
	} else if (app.fallback_to_stderr) {
		logfd = 2;
	}
}

void handle_signal(int sig) {
	switch(sig) {
		case SIGABRT:
			cleanup();
			break;
		case SIGSEGV:
			cleanup();
			break;
	}
	dprintf(logfd, "ERROR: signal %d received\n", sig);
	exit(1);
}

void cleanup(void) {
	sa_cleanup(ga);
	if (logfd != -1) {
		close(logfd);
		if (app.auto_report) {
			execlp("less", "less", app.logfname, (char *)0);
		}
	}
}

#endif // GARAGE_H

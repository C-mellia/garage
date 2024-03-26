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


typedef struct GlobAlloc {
    void *mem, *top;
    size_t cap;
	size_t offs[MAX_OFFS];
	size_t *off;
} *GlobAlloc;

typedef struct App {
	char *logfname;
	int auto_report;
} App;

GlobAlloc ga = 0;
int logfd = -1;
App app = {
	.logfname = "log",
	.auto_report = 0,
};

void setup_env(void);
void handle_signal(int sig);
void cleanup(void);

GlobAlloc glob_new(size_t cap);
void *glob_alloc(GlobAlloc g, size_t bytes);
void glob_rewind(GlobAlloc g);
void glob_set(GlobAlloc g);
void glob_cleanup(GlobAlloc g);

// ## implementation ##


GlobAlloc glob_new(size_t cap) {
    GlobAlloc g = malloc(sizeof *g);
    code_trap(g, "Failed to initialize global allocator at\n");
    g->mem = malloc(cap);
    code_trap(g->mem, "Failed to initialize global allocator due to big size memory allocation not supported\n");
	g->off = g->offs + MAX_OFFS;
    g->top = g->mem + cap;
    g->cap = cap;
    return g;
}

void *glob_alloc(GlobAlloc g, size_t bytes) {
    return g->top < g->mem + bytes? 0: ({g->top -= bytes;});
}

void glob_rewind(GlobAlloc g) {
	if (g && g->mem) {
		g->top = g->mem + *g->off;
		++g->off;
	}
}

void glob_set(GlobAlloc g) {
	if (g && g->mem) {
		code_trap(g->off > g->offs, "Global allocator stack overflow\n");
		*(--g->off) = g->top - g->mem;
	}
}

void glob_cleanup(GlobAlloc g) {
    if (g) {
        if (g->mem) free(g->mem);
        free(g);
    }
}

void setup_env(void) {
	signal(SIGABRT, handle_signal);
	signal(SIGSEGV, handle_signal);
	ga = glob_new(0x1000);
	logfd = open(app.logfname, O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
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
	glob_cleanup(ga);
	if (logfd != -1) { 
		close(logfd);
		if (app.auto_report) {
			execlp("less", "less", app.logfname, (char *)0);
		}
	}
}

#endif // GARAGE_H


#include <stdlib.h>
#include <signal.h>
#include <stdarg.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include "garage.h"

int logfd = -1;
App app = {0};

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
		report("WARNING: saal allocated memory will not be released by glob_pop\n");
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

void sa_push(StackAllocator sa) {
	if (sa && sa->mem) {
		code_trap(sa->off > sa->offs, "Global allocator stack overflow\n");
		*(--sa->off) = sa->top - sa->mem;
	}
}

int sa_stack_empty(StackAllocator sa) {
	return sa->off == sa->offs + MAX_OFFS;
}

void sa_cleanup(StackAllocator sa) {
    if (sa) {
		if (!sa_stack_empty(sa) && logfd > 0) {
			report("WARNING: undefined behavior due to unbalanced sa_push and glob_pop\n");
		}
        if (sa->mem) free(sa->mem);
        free(sa);
    }
}

void sa_diag(StackAllocator sa) {
    report("Usage: %lu bytes\n", sa->mem + sa->cap - sa->top);
}

void setup_env(void) {
	signal(SIGABRT, handle_signal);
	signal(SIGSEGV, handle_signal);
    signal(SIGINT, handle_signal);

	if (app.logfname) {
		logfd = open(app.logfname,
				O_WRONLY | O_CREAT | O_TRUNC,
				S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
	} else if (app.fallback_to_stderr) {
		logfd = 2;
	}

    if (app.exec_startup) {
        app.exec_startup();
    }
}

void handle_signal(int sig) {
	switch(sig) {
		case SIGABRT:
			cleanup();
            report("Aborted\n");
            exit(SIGABRT);
		case SIGSEGV:
			cleanup();
            report("Segmentation fault\n");
            exit(SIGSEGV);
        case SIGINT:
            cleanup();
            report("Interrupted\n");
            exit(SIGINT);
	}
}

static inline size_t file_size(const char *fname) {
    struct stat st;
    if (stat(fname, &st) == 0) {
        return st.st_size;
    }
    return 0;
}

void cleanup(void) {
    if (app.exec_cleanup) {
        app.exec_cleanup();
    }

	if (logfd != -1) {
		close(logfd);
		if (app.auto_report && file_size(app.logfname)) {
            char buf[0x100];
            sprintf(buf, "less %s", app.logfname);
            system(buf);
		}
	}
}

__attribute__((nonnull(1), format(printf, 1, 2))) void report(const char *msg, ...) {
    va_list args;
    va_start(args, msg);
    vdprintf(logfd, msg, args);
    va_end(args);
}

void _abort(void) {
    raise(SIGABRT);
}

void set_app(
		char *logfname,
		int auto_report,
		int fallback_to_stderr,
		void (*exec_startup)(void),
		void (*exec_cleanup)(void)) {
	app.logfname = logfname;
	app.auto_report = auto_report;
	app.fallback_to_stderr = fallback_to_stderr;
	app.exec_startup = exec_startup;
	app.exec_cleanup = exec_cleanup;
}

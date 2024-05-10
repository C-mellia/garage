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
    StackAllocator sa = malloc(sizeof *sa);
    code_trap(sa, "sa_new: null\n");
    sa->mem = malloc(cap);
    code_trap(sa->mem, "sa_new: null\n");
    sa->off = sa->offs + MAX_OFFS;
    sa->top = sa->mem + cap;
    sa->cap = cap;
    return sa;
}

void *sa_alloc(StackAllocator sa, size_t bytes) {
    if (sa_stack_empty(sa) && logfd > 0) {
        report("sa_alloc: empty stack warning\n");
    }
    return sa->top < sa->mem + bytes? 0: ({sa->top -= bytes;});
}

void sa_pop(StackAllocator sa) {
    code_trap(sa && sa->mem, "sa_pop: null\n");
    if (sa && sa->mem) {
        code_trap(sa->off != sa->offs + MAX_OFFS, "sa_pop: stack underflow\n");
        sa->top = sa->mem + *sa->off++;
    }
}

void sa_push(StackAllocator sa) {
    if (sa && sa->mem) {
        code_trap(sa->off > sa->offs, "sa_push: stack overflow\n");
        *(--sa->off) = sa->top - sa->mem;
    }
}

int sa_stack_empty(StackAllocator sa) {
    code_trap(sa, "sa_stack_empty: null\n");
    return sa->off == sa->offs + MAX_OFFS;
}

void sa_cleanup(StackAllocator sa) {
    if (sa) {
        if (!sa_stack_empty(sa) && logfd > 0) {
            report("sa_cleanup: unmaching push and pop\n");
        }
        if (sa->mem) free(sa->mem);
        free(sa);
    }
}

static inline double clamp(double val, double bot, double up) {
    return val > up? up: val < bot? bot: val;
}

void sa_diag(StackAllocator sa) {
    if (sa->off == sa->offs + MAX_OFFS) {
        report("sa_diag: empty stack\n");
        size_t usage = sa->mem + sa->cap - sa->top;
        double perc = clamp((double)usage / sa->cap * 100, 0.f, 100.f);
        if (usage <= 0x400) {
            report("Usage: 0x%lx bytes, %f%%\n", usage, perc);
        } else if (usage <= 0x100000) {
            double mega_bytes = (double)usage / 0x400;
            report("Usage: %f mega bytes, %f%%\n", mega_bytes, perc);
        } else {
            double giga_bytes = (double)usage / 0x100000;
            report("Usage: %f giga bytes\n, %f%%", giga_bytes, perc);
        }
        return;
    }
    for (size_t idx = 0; idx < sa_stack_size(sa); ++idx) {
        size_t usage;
        if (idx == sa_stack_size(sa) - 1) {
            usage = sa->mem + sa->off[idx] - sa->top;
        } else {
            usage = sa->off[idx + 1] - sa->off[idx];
        }
        double perc = clamp((double)usage / sa->cap * 100, 0.f, 100.f);
        if (usage <= 0x400) {
            report("stack[%lu] Usage: 0x%lx bytes, %f%%\n", idx, usage, perc);
        } else if (usage <= 0x100000) {
            double mega_bytes = (double)usage / 0x400;
            report("stack[%lu] Usage: %f mega bytes, %f%%\n", idx, mega_bytes, perc);
        } else {
            double giga_bytes = (double)usage / 0x100000;
            report("stack[%lu] Usage: %f giga bytes\n, %f%%", idx, giga_bytes, perc);
        }
    }
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
    if (logfd >= 0) vdprintf(logfd, msg, args);
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

size_t sa_stack_size(StackAllocator sa) {
    code_trap(sa, "sa_stack_size: null\n");
    return sa->offs + MAX_OFFS -  sa->off;
}

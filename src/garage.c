#include <stdlib.h>
#include <signal.h>
#include <stdarg.h>
#include <stdio.h>

#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <pthread.h>

#include <garage/garage.h>

static int logfd = -1;
static App app = 0;

typedef void (*handle_func)(int);

static handle_func handle_abrt = 0, handle_segv = 0, handle_int = 0;

static inline void handle_signal(int sig) {
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
        case SIGUSR1:
            cleanup();
            exit(0);
        default:;
    }
}

StackAllocator sa_new(size_t cap) {
    StackAllocator sa = mmap(0, sizeof *sa + cap, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    code_trap(sa, "sa_new: null\n");
    sa->mem = sa + 1;
    sa->off = sa->offs + MAX_OFFS;
    sa->top = sa->mem + cap;
    sa->cap = cap;
    code_trap(pthread_mutex_init(&sa->m, 0) == 0, "sa_new: mutex init\n");
    return sa;
}

void *sa_alloc(StackAllocator sa, size_t bytes) {
    if (sa_stack_empty(sa) && logfd > 0) {
        report("sa_alloc: empty stack warning\n");
    }
    code_trap(pthread_mutex_lock(&sa->m) == 0, "sa_alloc: mutex lock\n");
    void *res = sa->top < sa->mem + bytes? 0: (sa->top -= bytes);
    code_trap(pthread_mutex_unlock(&sa->m) == 0, "sa_alloc: mutex unlock\n");
    return res;
}

void sa_pop(StackAllocator sa) {
    code_trap(sa && sa->mem, "sa_pop: null\n");
    if (sa && sa->mem) {
        code_trap(sa->off != sa->offs + MAX_OFFS, "sa_pop: stack underflow\n");
        code_trap(pthread_mutex_lock(&sa->m) == 0, "sa_pop: mutex lock\n");
        sa->top = sa->mem + *sa->off++;
        code_trap(pthread_mutex_unlock(&sa->m) == 0, "sa_pop: mutex unlock\n");
    }
}

void sa_push(StackAllocator sa) {
    if (sa && sa->mem) {
        code_trap(sa->off > sa->offs, "sa_push: stack overflow\n");
        code_trap(pthread_mutex_lock(&sa->m) == 0, "sa_push: mutex lock\n");
        *(--sa->off) = sa->top - sa->mem;
        code_trap(pthread_mutex_unlock(&sa->m) == 0, "sa_push: mutex unlock\n");
    }
}

int sa_stack_empty(StackAllocator sa) {
    code_trap(sa, "sa_stack_empty: null\n");
    return sa->off == sa->offs + MAX_OFFS;
}

void sa_cleanup(StackAllocator sa) {
    if (sa) {
        code_trap(pthread_mutex_destroy(&sa->m) == 0, "sa_cleanup: mutex destroy\n");
        if (!sa_stack_empty(sa) && logfd > 0) {
            report("sa_cleanup: unmaching push and pop\n");
        }
        munmap(sa, sizeof *sa + sa->cap);
    }
}

static inline double clamp(double val, double bot, double up) {
    return val > up? up: val < bot? bot: val;
}

void sa_diag(StackAllocator sa) {
    size_t stack_size = sa_stack_size(sa);
    if (!stack_size) {
        report("sa_diag: empty stack\n");
        return;
    } else if (stack_size == 1) {
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
    } else {
        for (size_t idx = 0; idx < stack_size; ++idx) {
            size_t usage, stack_idx = stack_size - idx - 1;
            if (idx == stack_size - 1) {
                usage = sa->off[stack_idx] + sa->mem - sa->top;
            } else {
                usage = sa->off[stack_idx] - sa->off[stack_idx - 1];
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
}

void setup_env(
    char *logfname,
    int auto_report,
    int fallback_to_stderr,
    void (*exec_startup)(void),
    void (*exec_cleanup)(void)
) {
    if (app = malloc(sizeof *app), !app) exit(127);
    app->logfname = logfname;
    app->auto_report = auto_report;
    app->fallback_to_stderr = fallback_to_stderr;
    app->exec_startup = exec_startup;
    app->exec_cleanup = exec_cleanup;

    handle_abrt = signal(SIGABRT, handle_signal);
    handle_segv = signal(SIGSEGV, handle_signal);
    handle_int = signal(SIGINT, handle_signal);
    signal(SIGUSR1, handle_signal);

    if (app->logfname) {
        logfd = open(app->logfname,
                O_WRONLY | O_CREAT | O_TRUNC,
                S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
    } else if (app->fallback_to_stderr) {
        logfd = 2;
    }

    if (app->exec_startup) app->exec_startup();
}

static inline size_t file_size(const char *fname) {
    struct stat st;
    if (stat(fname, &st) == 0) {
        return st.st_size;
    }
    return 0;
}

void cleanup(void) {
    // Note: restore original handle functions in case handling signals multiple times
    signal(SIGABRT, handle_abrt);
    signal(SIGSEGV, handle_segv);
    signal(SIGINT, handle_int);
    if (!app) return;
    if (app->exec_cleanup) app->exec_cleanup();

    if (logfd != -1) {
        close(logfd);
        if (app->auto_report && file_size(app->logfname)) {
            char buf[0x100];
            sprintf(buf, "less %s", app->logfname);
            system(buf);
        }
    }
    free(app);
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

size_t sa_stack_size(StackAllocator sa) {
    code_trap(sa, "sa_stack_size: null\n");
    return sa->offs + MAX_OFFS -  sa->off;
}

void gracefully_exit(void) {
    raise(SIGUSR1);
}

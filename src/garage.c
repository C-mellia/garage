#include <stdlib.h>
#include <signal.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>

#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <pthread.h>

#include <garage/garage.h>
#include <garage/log.h>

typedef struct Garage {
    char *logfname;
    int auto_report;
    int fallback_to_stderr;
    void (*exec_startup)(void);
    void (*exec_cleanup)(void);
} *Garage;

typedef void (*handle_func)(int);

int logfd = -1;

static Garage garage = 0;

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

void setup_env(
    char *logfname,
    int auto_report,
    int fallback_to_stderr,
    void (*exec_startup)(void),
    void (*exec_cleanup)(void)
) {
    if (garage = malloc(sizeof *garage), !garage) exit(69);
    garage->logfname = logfname;
    garage->auto_report = auto_report;
    garage->fallback_to_stderr = fallback_to_stderr;
    garage->exec_startup = exec_startup;
    garage->exec_cleanup = exec_cleanup;

    handle_abrt = signal(SIGABRT, handle_signal);
    handle_segv = signal(SIGSEGV, handle_signal);
    handle_int = signal(SIGINT, handle_signal);
    signal(SIGUSR1, handle_signal);

    if (garage->logfname) {
        logfd = open(garage->logfname,
                O_WRONLY | O_CREAT | O_TRUNC,
                S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
    } else if (garage->fallback_to_stderr) {
        logfd = 2;
    }

    if (garage->exec_startup) garage->exec_startup();
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
    if (!garage) return;
    if (garage->exec_cleanup) garage->exec_cleanup();

    if (logfd != -1) {
        close(logfd);
        if (garage->auto_report && file_size(garage->logfname)) {
            char buf[0x100];
            sprintf(buf, "less %s", garage->logfname);
            system(buf);
        }
    }
    free(garage);
}

void report(const char *msg, ...) {
    va_list args;
    va_start(args, msg);
    if (logfd >= 0) vdprintf(logfd, msg, args);
    va_end(args);
}

void _abort(void) {
    raise(SIGABRT);
}

void gracefully_exit(void) {
    raise(SIGUSR1);
}

int buffered_printf(const char *fmt, ...) {
    int fd;
    va_list args;

    va_start(args, fmt);
    fd = buffered_vprintf(fmt, args);
    va_end(args);

    return fd;
}

int buffered_vprintf(const char *fmt, va_list args) {
    int pfd[2] = {0};

    if (pipe(pfd) == -1) return -1;

    int flags = fcntl(pfd[1], F_GETFL, 0);
    fcntl(pfd[1], F_SETFL, flags | O_NONBLOCK);

    vdprintf(pfd[1], fmt, args);

    close(pfd[1]);
    return pfd[0];
}

int object_dprint_redirect(void *obj, Dprint dprint) {
    int pfd[2] = {0};
    if (pipe(pfd) == -1) return -1;
    dprint(pfd[1], obj);
    close(pfd[1]);
    return pfd[0];
}

void memswap(void *lhs, void *rhs, size_t len) {
    void *buf = alloca(len);
    memcpy(buf, lhs, len);
    memcpy(lhs, rhs, len);
    memcpy(rhs, buf, len);
}

void fd_drop(int *fd) {
    if (fd && *fd != -1) close(*fd), *fd = -1;
}

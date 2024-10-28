#include <stdlib.h>
#include <string.h>

#include <sys/mman.h>
#include <unistd.h>
#include <fcntl.h>

#include <garage/sa.h>
#include <garage/log.h>

static inline double clamp(double val, double bot, double up);
static void __sa_init(Sa sa, size_t cap);

void sa_init(Sa sa, size_t cap) {
    if (!sa) return;
    __sa_init(sa, cap);
}

Sa sa_new(size_t cap) {
    Sa sa = mmap(0, sizeof *sa + cap, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    alloc_check(mmap, sa, sizeof *sa + cap);
    return __sa_init(sa, cap), sa;
}

void sa_cleanup(Sa sa) {
    if (!sa) return;
    if (!sa_stack_empty(sa) && __logfd > 0) {
        log_fmt("Warning: The amount of times, in which push operations and pop operations are called respectively does not match together\n");
    }
}

void sa_drop(Sa *sa) {
    if (sa && *sa) sa_cleanup(*sa), munmap(*sa, sizeof **sa + (*sa)->cap), *sa = 0;
}

int sa_deb_dprint(int fd, Sa sa) {
    if (!sa) return dprintf(fd, "(nil)");
    void *bottom = sa->mem + sa->cap;
    size_t alloc_size = bottom - sa->top;
    return dprintf(fd, "{bottom: %p, alloc_size: %lu, stack_size: %zu, cap: %lu}\n",
                   bottom, alloc_size, sa_stack_size(sa), sa->cap);
}

int sa_deb_print(Sa sa) {
    return fflush(stdout), sa_deb_dprint(1, sa);
}

void *sa_alloc(Sa sa, size_t bytes) {
    nul_check(Sa, sa);
    if (sa_stack_empty(sa) && __logfd > 0) {
        log_fmt("Warning: Attempting to allocate memory on Stack Allocator, while pointer stack is empty\n");
    }
    void *res = sa->top < (void *)sa->mem + bytes? 0: (sa->top -= bytes);
    return res;
}

void sa_pop(Sa sa) {
    nul_check(Sa, sa);
    assert(sa->off != sa->offs + MAX_OFFS, "Pointer stack underflow.\n");
    sa->top = sa->mem + *sa->off++;
}

void sa_push(Sa sa) {
    nul_check(Sa, sa);
    assert(sa->off > sa->offs, "Pointer stack overflow. Push operations at most: %zu\n", MAX_OFFS);
    *(--sa->off) = sa->top - (void *)sa->mem;
}

int sa_stack_empty(Sa sa) {
    nul_check(Sa, sa);
    return sa->off == sa->offs + MAX_OFFS;
}

void sa_diag(Sa sa) {
    nul_check(Sa, sa);
    size_t stack_size = sa_stack_size(sa);
    if (!stack_size) {
        log_fmt("Warning: No push operation called on the Stack Allocator, or "
               "perhaps there are equivalent amount of call of pop operations on "
               "the Stack Allocator at this point\n");
        return;
    } else if (stack_size == 1) {
        size_t usage = (void *)sa->mem + sa->cap - sa->top;
        double perc = clamp((double)usage / sa->cap * 100, 0.f, 100.f);
        if (usage <= 0x400) {
            log_fmt("Usage: 0x%lx bytes, %f%%\n", usage, perc);
        } else if (usage <= 0x100000) {
            double mega_bytes = (double)usage / 0x400;
            log_fmt("Usage: %f mega bytes, %f%%\n", mega_bytes, perc);
        } else {
            double giga_bytes = (double)usage / 0x100000;
            log_fmt("Usage: %f giga bytes\n, %f%%", giga_bytes, perc);
        }
    } else {
        for (size_t idx = 0; idx < stack_size; ++idx) {
            size_t usage, stack_idx = stack_size - idx - 1;
            if (idx == stack_size - 1) {
                usage = sa->off[stack_idx] + (void *)sa->mem - sa->top;
            } else {
                usage = sa->off[stack_idx] - sa->off[stack_idx - 1];
            }
            double perc = clamp((double)usage / sa->cap * 100, 0.f, 100.f);
            if (usage <= 0x400) {
                log_fmt("stack[%lu] Usage: 0x%lx bytes, %f%%\n", idx, usage, perc);
            } else if (usage <= 0x100000) {
                double mega_bytes = (double)usage / 0x400;
                log_fmt("stack[%lu] Usage: %f mega bytes, %f%%\n", idx, mega_bytes, perc);
            } else {
                double giga_bytes = (double)usage / 0x100000;
                log_fmt("stack[%lu] Usage: %f giga bytes\n, %f%%", idx, giga_bytes, perc);
            }
        }
    }
}

size_t sa_stack_size(Sa sa) {
    nul_check(Sa, sa);
    return sa->offs + MAX_OFFS -  sa->off;
}

static inline double clamp(double val, double bot, double up) {
    return val > up? up: val < bot? bot: val;
}

static void __sa_init(Sa sa, size_t cap) {
    sa->top = sa->mem + cap, sa->cap = cap, sa->off = sa->offs + MAX_OFFS;
}

#include <stdlib.h>
#include <string.h>

#include <sys/mman.h>
#include <unistd.h>
#include <fcntl.h>

#include <garage/sa.h>
#include <garage/log.h>

static inline double clamp(double val, double bot, double up) {
    return val > up? up: val < bot? bot: val;
}

Sa sa_new(size_t cap) {
    Sa sa = mmap(0, sizeof *sa + cap, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    assert(sa, "Failed to allocate memory of size: %zu\n", sizeof *sa + cap);
    sa->mem = sa + 1;
    sa->off = sa->offs + MAX_OFFS;
    sa->top = sa->mem + cap;
    sa->cap = cap;
    assert(pthread_mutex_init(&sa->m, 0) == 0, "sa_new: mutex init\n");
    return sa;
}

void *sa_alloc(Sa sa, size_t bytes) {
    if (sa_stack_empty(sa) && logfd > 0) {
        report("Warning: Attempting to allocate memory on Stack Allocator, while pointer stack is empty\n");
    }
    assert(pthread_mutex_lock(&sa->m) == 0, "Mutex lock operation failed\n");
    void *res = sa->top < sa->mem + bytes? 0: (sa->top -= bytes);
    assert(pthread_mutex_unlock(&sa->m) == 0, "Mutex unlock operation failed\n");
    return res;
}

void sa_pop(Sa sa) {
    assert(sa, "Stack Allocator is not initialized\n");
    if (sa && sa->mem) {
        assert(sa->off != sa->offs + MAX_OFFS,
               "Pointer stack underflow.\n");
        assert(pthread_mutex_lock(&sa->m) == 0, "Mutex lock operation failed\n");
        sa->top = sa->mem + *sa->off++;
        assert(pthread_mutex_unlock(&sa->m) == 0, "Mutex unlock operation failed\n");
    }
}

void sa_push(Sa sa) {
    if (sa && sa->mem) {
        assert(sa->off > sa->offs, "Pointer stack overflow. Push operations at most: %zu\n", MAX_OFFS);
        assert(pthread_mutex_lock(&sa->m) == 0, "Mutex lock operation failed\n");
        *(--sa->off) = sa->top - sa->mem;
        assert(pthread_mutex_unlock(&sa->m) == 0, "Mutex unlock operation failed\n");
    }
}

int sa_stack_empty(Sa sa) {
    assert(sa, "Stack Allocator is not initialized at this point\n");
    return sa->off == sa->offs + MAX_OFFS;
}

void sa_cleanup(Sa sa) {
    if (sa) {
        assert(pthread_mutex_destroy(&sa->m) == 0, "Mutex destroy operation failed\n");
        if (!sa_stack_empty(sa) && logfd > 0) {
            report("Warning: The amount of times, in which push operations and pop operations are called respectively does not match together\n");
        }
        munmap(sa, sizeof *sa + sa->cap);
    }
}

void sa_diag(Sa sa) {
    size_t stack_size = sa_stack_size(sa);
    if (!stack_size) {
        report("Warning: No push operation called on the Stack Allocator, or "
               "perhaps there are equivalent amount of call of pop operations on "
               "the Stack Allocator at this point\n");
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

size_t sa_stack_size(Sa sa) {
    assert(sa, "Stack Allocator is not initialized at this point\n");
    return sa->offs + MAX_OFFS -  sa->off;
}

void sa_drop(Sa *sa) {
    if (sa && *sa) sa_cleanup(*sa), *sa = 0;
}

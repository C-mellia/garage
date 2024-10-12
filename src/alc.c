#include <string.h>

#include <sys/mman.h>

#include <garage/garage.h>
#include <garage/alc.h>
#include <garage/log.h>

#include "./.alc.c"

static inline void __alc_init(Alc alc, size_t cap);

void alc_init(Alc alc, size_t cap) {
    nul_check(Alc, alc);
    __alc_init(alc, cap);
}

Alc alc_new(size_t cap) {
#define alloc_size (cap + sizeof *alc + sizeof (struct alc_node))
    Alc alc = mmap(0, alloc_size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    alloc_check(mmap, alc, alloc_size);
    assert(alc != MAP_FAILED, "alc_new: null\n");
    return __alc_init(alc, cap), alc;
}
#undef alloc_size

void alc_cleanup(Alc alc) {
    AlcNode begin = (void *)alc->mem;
    memset(begin, 0, sizeof *begin);
}

void alc_drop(Alc *alc) {
    if (alc && *alc) alc_cleanup(*alc), munmap(*alc, sizeof **alc + sizeof (struct alc_node) + (*alc)->cap), *alc = 0;
}

void *alc_alloc(Alc alc, size_t len) {
    nul_check(Alc, alc);
    AlcNode begin = (void *)alc->mem, node;
    if (!len || (node = node_next_fit(begin, len), !node)) return 0;
    if (!node->len && node_gap(node) >= 0) {
        node->len = len;
    } else {
        node = node->next = node->next->prev = node_set((void *)node_mem(node) + node->len, node->next, node, len);
    }
    return node_mem(node);
}

int alc_free(Alc alc, void *ptr) {
    nul_check(Alc, alc);
    AlcNode begin = (void *)alc->mem, node;
    if (!ptr || (node = node_locate_ptr(begin, ptr), !node)) return -1;
    if (!node->prev) {
        if (!node->next->len) node->next = 0;
        node->len = 0;
    } else {
        node->next->prev = node->prev, node->prev->next = !node->prev->len && !node->next->len? node->next->next: node->next;
    }
    return 0;
}

int alc_deb_dprint(int fd, Alc alc) {
    if (!alc) return dprintf(fd, "(nil)");
    size_t alloc_size = 0;
    size_t node_count = 0;
    for (AlcNode node = (void *)alc->mem; node; node = node->next) {
        alloc_size += node->len, ++node_count;
    }
    return dprintf(fd, "{alloc_size: %lu, node_count: %zu, cap: %lu}", alloc_size, node_count, alc->cap);
}

int alc_deb_print(Alc alc) {
    return fflush(stdout), alc_deb_dprint(1, alc);
}

static inline void __alc_init(Alc alc, size_t cap) {
    AlcNode begin = (void *)alc->mem;
    memset(begin, 0, sizeof *begin), alc->cap = cap;
}

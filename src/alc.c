#include <string.h>

#include <sys/mman.h>

#include <garage/garage.h>
#include <garage/alc.h>

static inline AlcNode node_set(AlcNode node, AlcNode next, AlcNode prev, size_t len) {
    return node->next = next, node->prev = prev, node->len = len, node;
}

static inline void *node_mem(AlcNode node) {
    return node + 1;
}

static inline intptr_t node_gap(AlcNode node) {
    return node->next? (intptr_t) ((void *)node->next - node_mem(node) - node->len): -1;
}

static inline int node_fit_len(AlcNode node, size_t len) {
    return (!node->len && len < (size_t) node_gap(node))
    || (node->len && sizeof *node + len < (size_t) node_gap(node));
}

static inline AlcNode node_next_fit(AlcNode node, size_t len) {
    while (node->next && !node_fit_len(node, len)) node = node->next;
    return node;
}

static inline AlcNode node_locate_ptr(AlcNode node, void *ptr) {
    while (node->next && node_mem(node) != ptr) node = node->next;
    return node;
}

Alc alc_new(size_t cap) {
    Alc alc = mmap(0, cap + sizeof *alc + sizeof *alc->begin, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    code_trap(alc != MAP_FAILED, "alc_new: null\n");
    alc->cap = cap, alc->begin = (AlcNode) (alc + 1);
    return memset(alc->begin, 0, sizeof *alc->begin), alc;
}

int alc_cleanup(Alc alc) {
    return munmap(alc, sizeof *alc + sizeof *alc->begin + alc->cap);
}

void *alc_alloc(Alc alc, size_t len) {
    AlcNode node;
    code_trap(alc, "alc_alloc: null\n");
    if (!len || (node = node_next_fit(alc->begin, len), !node)) return 0;
    if (!node->len && node_gap(node) >= 0) {
        node->len = len;
    } else {
        node = node->next = node->next->prev = node_set((void *)node_mem(node) + node->len, node->next, node, len);
    }
    return node_mem(node);
}

int alc_free(Alc alc, void *ptr) {
    AlcNode node;
    code_trap(alc, "alc_free: null\n");
    if (!ptr || (node = node_locate_ptr(alc->begin, ptr), !node)) return -1;
    if (!node->prev) {
        if (!node->next->len) node->next = 0;
        node->len = 0;
    } else {
        node->next->prev = node->prev, node->prev->next = !node->prev->len && !node->next->len? node->next->next: node->next;
    }
    return 0;
}

#ifndef _GARAGE_ALC_C
#define _GARAGE_ALC_C 1

static inline AlcNode node_set(AlcNode node, AlcNode next, AlcNode prev, size_t len);
static inline void *node_mem(AlcNode node);
static inline intptr_t node_gap(AlcNode node);
static inline int node_fit_len(AlcNode node, size_t len);
static inline AlcNode node_next_fit(AlcNode node, size_t len);
static inline AlcNode node_locate_ptr(AlcNode node, void *ptr);

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

#endif // _GARAGE_ALC_C

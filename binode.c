#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <fcntl.h>
#include "garage.h"
#include "binode.h"

#define BINODE_INSERT(node, n, first, second) do {\
	if ((node)->first) {\
		(node)->first->pr = n;\
		(n)->second = (node)->first;\
	}\
	(node)->first = n;\
	(n)->pr = node;\
} while(0)

#define BINODE_BACKSERT(node, n, child) do {\
	Binode parent = node->pr;\
	if (parent) {\
		parent->child = n;\
		(n)->pr = parent;\
		if (parent->lc == node) {\
			(n)->lc = node;\
		} else if (parent->rc == node) {\
			(n)->rc = node;\
		} else {\
			panic("invalid node");\
		}\
		(node)->pr = n;\
	}\
} while(0)

#define NODE_SIBLING(node, child) \
	((node)->child == (node)->lc? (node)->rc: (node)->lc)

#define NODE_SET_SIBLING(node, child, n) do {\
	if ((node)->child == (node)->lc) {\
		(node)->rc = n;\
	} else {\
		(node)->lc = n;\
	}\
} while(0)

#define NODE_BALANCE(node, child) do {\
	Binode n = (node)->child;\
	if (!n) break;\
	if((n->lc == n->child && n->h[1] > n->h[0]) || (n->rc == n->child && n->h[1] < n->h[0])) {\
		Binode m = NODE_SIBLING(n, child);\
		NODE_SET_SIBLING(n, child, m->child);\
		if (m->child) {\
			m->child->pr = n;\
		}\
		(node)->child = m;\
		m->pr = node;\
		m->child = n;\
		n->pr = m;\
		binode_height(n);\
		binode_height(m);\
		binode_height(node);\
		n = m;\
	}\
	n->pr = (node)->pr;\
	if ((node)->pr) {\
		if ((node)->pr->lc == node) {\
			(node)->pr->lc = n;\
		} else {\
			(node)->pr->rc = n;\
		}\
	}\
	(node)->child = NODE_SIBLING(n, child);\
	if (NODE_SIBLING(n, child)) {\
		NODE_SIBLING(n, child)->pr = node;\
	}\
	NODE_SET_SIBLING(n, child, node);\
	node->pr = n;\
	binode_height(node);\
	binode_height(n);\
} while(0)

#define BINODE_RESET_PR(node, n) do {\
	if (node == (node)->pr->lc) {\
		(node)->pr->lc = n;\
	} else {\
		(node)->pr->rc = n;\
	}\
} while(0)

static inline int8_t max8(int8_t a, int8_t b) {
	return a > b? a: b;
}

static inline void binode_height(Binode node) {
	node->h[0] = node->lc? max8(node->lc->h[0], node->lc->h[1]) + 1: 0;
	node->h[1] = node->rc? max8(node->rc->h[0], node->rc->h[1]) + 1: 0;
}

static inline Binode binode_balance(Binode node) {
	int8_t b = node->h[1] - node->h[0];
	if (b > 1) NODE_BALANCE(node, rc);
	else if (b < -1) NODE_BALANCE(node, lc);
	else return 0;
	return node->pr;
}

static inline Binode btree_locate(Binode node, int key) {
	while(node && node->key != key) node = key < node->key? node->lc: node->rc;
	return node;
}

Binode binode_new(StackAllocator sa, int key) {
	Binode n = sa_alloc(sa, sizeof(*n));
	code_trap(n, "memory allocation failed");
	if (n) {
		memset(n, 0, sizeof *n);
		n->key = key;
	}
	return n;
}

Binode preorder_begin(Binode root) {
	return root;
}

Binode preorder_end(Binode root) {
	Binode parent;
	while (root) {
		parent = root->pr;
		if (parent && parent->lc == root && parent->rc) {
			root = parent->rc;
			break;
		} else {
			root = parent;
		}
	}
	return root;
}

Binode preorder_next(Binode node) {
	if (node && node->lc) return node->lc;
	if (node && node->rc) return node->rc;
	return preorder_end(node);
}

Binode inorder_begin(Binode root) {
	while(root && root->lc) root = root->lc;
	return root;
}

Binode inorder_end(Binode root) {
	Binode parent;
	while(root) {
		parent = root->pr;
		if (parent && parent->rc == root) {
			root = inorder_begin(parent->rc);
			break;
		}
		root = parent;
	}
	return root;
}

Binode inorder_next(Binode node) {
	if (node && node->rc) return inorder_begin(node->rc);
	return inorder_end(node);
}

Binode postorder_begin(Binode root) {
	while(root) {
		if (root->lc) {
			root = root->lc;
		} else if (root->rc) {
			root = root->rc;
		} else {
			break;
		}
	}
	return root;
}

Binode postorder_end(Binode root) {
	return root;
}

Binode postorder_next(Binode node) {
	Binode parent;
	if (node->pr) {
		parent = node->pr;
		if (parent->lc == node && parent->rc) {
			return postorder_begin(parent->rc);
		}
		return parent;
	}
	return 0;
}

void binode_ll_insert(Binode node, Binode n) {
	if (!node) return;
	BINODE_INSERT(node, n, lc, lc);
}

void binode_lr_insert(Binode node, Binode n) {
	if (!node) return;
	BINODE_INSERT(node, n, lc, rc);
}

void binode_rl_insert(Binode node, Binode n) {
	if (!node) return;
	BINODE_INSERT(node, n, rc, lc);
}

void binode_rr_insert(Binode node, Binode n) {
	if (!node) return;
	BINODE_INSERT(node, n, rc, rc);
}

void bionde_lbacksert(Binode node, Binode n) {
	if (n) BINODE_BACKSERT(node, n, lc);
}

void binode_rbacksert(Binode node, Binode n) {
	if (n) BINODE_BACKSERT(node, n, rc);
}

Binode binode_rebalance(Binode node) {
	while(node) {
		binode_height(node);
		Binode r = binode_balance(node);
		node = r? : node;
		if (!node->pr) break;
		node = node->pr;
	}
	return node;
}

Binode _btree_insert(StackAllocator sa, Binode node, int key) {
	Binode n = node;
	while(n) {
		if ((key < n->key && !n->lc) || (key > n->key && !n->rc)) break;
		if (key == n->key) return node;
		n = key < n->key? n->lc: n->rc;
	}
	if (!n) return 0;
	n = binode_new(sa, key);
	if (key < node->key) binode_ll_insert(node, n);
	else if (key > node->key) binode_rl_insert(node, n);
	return binode_rebalance(node);
}

void binode_print(Binode node) {
	char buf[0x100];
	int offs[3];
	if (node->lc) {
		offs[0] = sprintf(buf, "%d", node->lc->key);
	} else {
		offs[0] = sprintf(buf, "null");
	}
	if (node->rc) {
		offs[1] = sprintf(buf + offs[0], "%d", node->rc->key);
	} else {
		offs[1] = sprintf(buf + offs[0], "null");
	}
	if (node->pr) {
		offs[2] = sprintf(buf + offs[0] + offs[1], "%d", node->pr->key);
	} else {
		offs[2] = sprintf(buf + offs[0] + offs[1], "null");
	}
	printf("key: %d, balance: %d, lc: %.*s rc: %.*s pr: %.*s\n",
			node->key, (int)(node->h[1] - node->h[0]), offs[0], buf, offs[1], buf + offs[0], offs[2], buf + offs[0] + offs[1]);
}

Binode _btree_delete(Binode node, int key) {
	Binode temp = btree_locate(node, key);
	if (!temp) return node;
	node = temp;
	if (node->lc) {
		Binode n = node->lc;
		while(n->rc) n = n->rc;
		node->key = n->key;
		BINODE_RESET_PR(n, n->lc);
		if (n->lc) n->lc->pr = n->pr;
		node = n->pr;
	} else if (node->rc) {
		if (node->pr) BINODE_RESET_PR(node, node->rc);
		node->rc->pr = node->pr;
		node = node->rc;
	} else if (node->pr) {
		BINODE_RESET_PR(node, 0);
		node = node->pr;
	} else {
		return 0;
	}
	return node? binode_rebalance(node): 0;
}

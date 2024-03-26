#ifndef NODE_H
#define NODE_H 1

/*
#include <string.h>
#include <stdint.h>
 */

typedef struct Binode {
	struct Binode *lc, *rc, *pr;
	int key;
	int8_t h[2];
} *Binode;

Binode binode_new(int key);

void binode_ll_insert(Binode node, Binode n);
void binode_lr_insert(Binode node, Binode n);
void binode_rl_insert(Binode node, Binode n);
void binode_rr_insert(Binode node, Binode n);

void binode_lbacksert(Binode node, Binode n);
void binode_rbacksert(Binode node, Binode n);

Binode preorder_begin(Binode root);
Binode preorder_end(Binode root);
Binode preorder_next(Binode node);

Binode inorder_begin(Binode root);
Binode inorder_end(Binode root);
Binode inorder_next(Binode node);

Binode postorder_begin(Binode root);
Binode postorder_end(Binode root);
Binode postorder_next(Binode node);

#endif // NODE_H

Binode binode_new(int key) {
	Binode n = glob_alloc(ga, sizeof(*n));
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

#define BINODE_INSERT(node, n, first, second) do {\
	if ((node)->first) {\
		(node)->first->pr = n;\
		(n)->second = (node)->first;\
	}\
	(node)->first = n;\
	(n)->pr = node;\
} while(0)

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

void bionde_lbacksert(Binode node, Binode n) {
	if (n) BINODE_BACKSERT(node, n, lc);
}

void binode_rbacksert(Binode node, Binode n) {
	if (n) BINODE_BACKSERT(node, n, rc);
}

int8_t max8(int8_t a, int8_t b) {
	return a > b? a: b;
}

void binode_height(Binode node) {
	node->h[0] = node->lc? max8(node->lc->h[0], node->lc->h[1]) + 1: 0;
	node->h[1] = node->rc? max8(node->rc->h[0], node->rc->h[1]) + 1: 0;
}

#undef BINODE_BACKSERT
#undef BINODE_INSERT

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
	if(NODE_SIBLING(n, child)) {\
		Binode m = NODE_SIBLING(n, child);\
		if (m->child) {\
			NODE_SET_SIBLING(n, child, m->child);\
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
} while(0)

static inline Binode binode_balance(Binode node) {
	int8_t b = node->h[1] - node->h[0];
	if (b > 1) NODE_BALANCE(node, rc);
	else if (b < -1) NODE_BALANCE(node, lc);
	else return 0;
	return node->pr;
}

int binode_insert(Binode node, int key) {
	while(node && node->key != key) {
		if ((key < node->key && !node->lc) || (key > node->key && !node->rc)) {
			break;
		}
		node = node->key < key? node->rc: node->lc;
	}
	if (!node || node->key == key) return -1;
	Binode n = binode_new(key);
	if (!n) return -1;
	if (key < node->key) {
		binode_ll_insert(node, n);
	} else if (key > node->key) {
		binode_rl_insert(node, n);
	}
	while(node) {
		binode_height(node);
		Binode r = binode_balance(node);
		node = r? : node;
		node = node->pr;
	}
	return 0;
}

int binode_get(Binode node) {
	code_trap(node, "invalid node");
	return node->key;
}

Binode binode_root(Binode node) {
	while(node && node->pr) node = node->pr;
	return node;
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

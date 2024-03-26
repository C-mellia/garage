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

Binode binode_ll_insert(Binode node, int key);
Binode binode_lr_insert(Binode node, int key);
Binode binode_rl_insert(Binode node, int key);
Binode binode_rr_insert(Binode node, int key);

Binode bionde_ll_backsert(Binode node, int key);
Binode bionde_lr_backsert(Binode node, int key);
Binode bionde_rl_backsert(Binode node, int key);
Binode bionde_rr_backsert(Binode node, int key);

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
		if (parent && parent->lc == root) {
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

Binode binode_ll_insert(Binode node, Binode n) {
	if (n) BINODE_INSERT(node, n, lc, lc);
}

Binode binode_lr_insert(Binode node, Binode n) {
	if (n) BINODE_INSERT(node, n, lc, rc);
}

Binode binode_rl_insert(Binode node, Binode n) {
	if (n) BINODE_INSERT(node, n, rc, lc);
}

Binode binode_rr_insert(Binode node, Binode n) {
	if (n) BINODE_INSERT(node, n, rc, rc);
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

Binode bionde_lbacksert(Binode node, Binode n) {
	if (n) BINODE_BACKSERT(node, n, lc);
}

Binode binode_rbacksert(Binode node, Binode n) {
	if (n) BINODE_BACKSERT(node, n, rc);
}

#undef BINODE_BACKSERT
#undef BINODE_INSERT

int8_t max8(int8_t a, int8_t b) {
	return a > b? a: b;
}

#define NODE_SIBLING(node, child) \
	((node)->child == (node)->lc? (node)->rc: (node)->lc)

#define NODE_SET_SIBLING(node, child, n) \
	((node)->child == (node)->lc? (node)->rc = n: (node)->lc = n)

#define NODE_BALANCE(node, child) do {\
	Binode n = (node)->child;\
	if(NODE_SIBLING(n, child)) {\
		Binode m = NODE_SIBLING(n, child);\
		if (m->child) {\
			NODE_SET_SIBLING(n, child, m->child);\
			NODE_SIBLING(m, child)->pr = n;\
		}\
		(node)->child = m;\
		m->pr = node;\
		m->child = n;\
		(n)->pr = m;\
	}\
	(n)->pr = (node)->pr;\
	if ((node)->pr) {\
		if ((node)->pr->lc == node) {\
			(node)->pr->lc = n;\
		} else {\
			(node)->pr->rc = n;\
		}\
	}\
	(n)->lc = node;\
} while(0)

void binode_balance(Binode node) {
	int8_t b = node->h[1] - node->h[0];
	if (b > 1) { 
		Binode n = node->rc;
		NODE_BALANCE(node, n, rc);
	} else if (b < -1) {
		Binode n = node->lc;
		NODE_BALANCE(node, n, lc);
	}
}

int binode_insert(Binode node, int key) {
	while(node && node->key != key) {
		if ((key < node->key && node->lc) || (key > node->key && node->rc)) {
			break;
		}
	}
	if (key < node->key && !node->lc) {
		Binode n = binode_ll_insert(node, key);
		if (!node->rc) {
			while(node) {
			}
		}
	} else if (key > node->key && node->rc) {
	} else {
		return -1;
	}
	return 0;
	while(node) {
		if (key < node->key) {
			if (node->lc) {
				node = node->lc;
			} else {
				if (node->h[1] > 0) {
					Binode n = binode_ll_insert(node, key), n1;
					int child;
					while (n) {
						node->h[0] = max8(n->h[0], n->h[1]) + 1;
						if (node->h[1] - node->h[0] > 1) {
						} else if (node->h[1] - node->h[0] < -1) {
						}
						node = n;
						n = n->pr;
					}
				}
				break;
			}
		}
	}
}

int binode_get(Binode node) {
	code_trap(node, "invalid node");
	return node->key;
}

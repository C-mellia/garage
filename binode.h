#ifndef NODE_H
#define NODE_H 1

/*
#include <string.h>
#include <stdint.h>
 */

typedef struct Binode {
	struct Binode *lc, *rc, *pr;
	int key;
	uint8_t h[2];
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

Binode binode_ll_insert(Binode node, int key) {
	Binode n = binode_new(key);
	if (n) BINODE_INSERT(node, n, lc, lc);
	return n;
}

Binode binode_lr_insert(Binode node, int key) {
	Binode n = binode_new(key);
	if (n) BINODE_INSERT(node, n, lc, rc);
	return n;
}

Binode binode_rl_insert(Binode node, int key) {
	Binode n = binode_new(key);
	if (n) BINODE_INSERT(node, n, rc, lc);
	return n;
}

Binode binode_rr_insert(Binode node, int key) {
	Binode n = binode_new(key);
	if (n) BINODE_INSERT(node, n, rc, rc);
	return n;
}

#undef BINODE_INSERT

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

Binode bionde_lbacksert(Binode node, int key) {
	Binode n = binode_new(key);
	if (n) BINODE_BACKSERT(node, n, lc);
	return n;
}

Binode binode_rbacksert(Binode node, int key) {
	Binode n = binode_new(key);
	if (n) BINODE_BACKSERT(node, n, rc);
	return n;
}

#undef BINODE_BACKSERT

void binode_insert(Binode node, int key) {
	while(node) {
		if (key < node->key) {
			if (node->lc) {
				node = node->lc;
			} else {
				Binode n = binode_rbacksert(node, key);
				while (n) {
					n->h[1] = max(node->h[0], node->h[1]);
					node = n;
					n = n->pr;
				}
				break;
			}
		} else {
			if (node->rc) {
				node = node->rc;
			} else {
				node = binode_lbacksert(node, key);
				if (node) {
					node->h[0] = max(node->lc->h[0], node->lc->h[1]) + 1;
					node->h[1] = max(node->rc->h[0], node->rc->h[1]) + 1;
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

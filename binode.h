#ifndef NODE_H
#define NODE_H 1

// #include <string.h>
// #include <stdint.h>

#define btree_delete(node, key) do {\
	node = _btree_delete(node, key);\
} while(0)

#define btree_insert(node, key) do {\
	node = _btree_insert(node, key);\
	if (!node) node = binode_new(key);\
} while(0)

typedef struct Binode {
	struct Binode *lc, *rc, *pr;
	int key;
	int8_t h[2];
} *Binode;

Binode binode_new(StackAllocator sa, int key);
void binode_print(Binode node);

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


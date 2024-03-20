#ifndef NODE_H
#define NODE_H 1

typedef struct Binode {
	struct Binode *lc, *rc, *pr;
	int key;
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
	Binode n = global_alloc(global_allocator, sizeof(*n));
	if (n) {
		n->lc = n->rc = n->pr = 0;
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

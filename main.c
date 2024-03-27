#include <stdio.h>
#include <stdint.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include "garage.h"
#include "binode.h"

int main(void) {
	setup_env();
	Binode root = binode_new(0);
	btree_insert(root, 1);
	btree_insert(root, 2);
	btree_insert(root, 3);
	btree_insert(root, 4);
	// btree_insert(root, 5);
	// btree_delete(root, 5);
	// binode_ll_insert(root, binode_new(1));
	// binode_ll_insert(root, binode_new(2));
	// binode_ll_insert(root, binode_new(3));
	// binode_rl_insert(root, binode_new(4));
	// binode_rl_insert(root, binode_new(5));

	Binode begin = preorder_begin(root), end = preorder_end(root);
	printf("%p %p\n", begin, end);
	while(begin != end) {
		binode_print(begin);
		begin = preorder_next(begin);
	}
	cleanup();
	return 0;
}

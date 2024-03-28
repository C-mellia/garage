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
	for (int i = 0; i < 10; ++i) {
		btree_insert(root, i);
	}
	// btree_delete(root, 5);
	// binode_ll_insert(root, binode_new(1));
	// binode_ll_insert(root, binode_new(2));
	// binode_ll_insert(root, binode_new(3));
	// binode_rl_insert(root, binode_new(4));
	// binode_rl_insert(root, binode_new(5));

	Binode begin = preorder_begin(root), end = preorder_end(root);
	while(begin != end) {
		binode_print(begin);
		begin = preorder_next(begin);
	}
	cleanup();
	return 0;
}

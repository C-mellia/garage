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
	Binode root = 0;
	for (int i = 0; i < 10; ++i) {
		btree_insert(root, i);
	}

	Binode begin = preorder_begin(root), end = preorder_end(root);
	while(begin != end) {
		binode_print(begin);
		begin = preorder_next(begin);
	}
	cleanup();
	return 0;
}

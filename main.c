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
	app = (App) { .logfname = "log", .auto_report = 1, .fallback_to_stderr = 0 };
	setup_env();
	glob_push(ga);
	Binode root = 0;
	for (int i = 0; i < 10; ++i) {
		btree_insert(root, i);
	}

	Binode begin = preorder_begin(root), end = preorder_end(root);
	while(begin != end) {
		binode_print(begin);
		begin = preorder_next(begin);
	}
	glob_pop(ga);
	cleanup();
	return 0;
}

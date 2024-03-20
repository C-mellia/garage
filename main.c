#include <stdio.h>
#include <stdint.h>
#include <signal.h>
#include <stdlib.h>
#include "global_allocator.h"
#include "binode.h"

void handle_signal(int sig);
void clean_up(void);

int main(void) {
	global_allocator = global_allocator_init(0x1000);
	Binode *root = binode_new(global_allocator, 0);
	signal(SIGSEGV, handle_signal);
	clean_up();
	return 0;
}

void clean_up(void) {
	global_allocator_deinit(global_allocator);
}

void handle_signal(int sig) {
	switch(sig) {
		case SIGSEGV:
			clean_up();
			exit(1);
			break;
		default:
			break;
	}
}

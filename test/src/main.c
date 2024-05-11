#include <stdio.h>
#include <garage.h>

void nest_startup(void);
void nest_cleanup(void);
void body(void);

StackAllocator sa = 0;

int main(void) {
    set_app("log", 1, 1, nest_startup, nest_cleanup);
    setup_env();
    body();
    cleanup();
    return 0;
}

void nest_startup(void) {
    sa = sa_new(0x100);
    sa_push(sa);

    sa_push(sa);
    sa_alloc(sa, 1);
    sa_alloc(sa, 1);
    sa_alloc(sa, 1);
    sa_diag(sa);
}

void nest_cleanup(void) {
    sa_pop(sa);
    sa_pop(sa);
    sa_cleanup(sa);
}

void body(void) {
}

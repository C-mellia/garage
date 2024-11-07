#include <stdio.h>

#include <garage/prelude.h>

static void nest_startup(void);
static void nest_cleanup(void);
static void body(void);

int main(void) {
    setup_env("log", 1, 1, nest_startup, nest_cleanup);
    body();
    cleanup();
    return 0;
}

static void nest_startup(void) {
}

static void nest_cleanup(void) {
}

static void body(void) {
    printf("Hello, world\n");
}

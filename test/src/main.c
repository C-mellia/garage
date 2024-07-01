#include <stdio.h>
#include <string.h>

#include <pthread.h>

#include <garage/garage.h>
#include <garage/array.h>
#include <garage/ascii.h>

void nest_startup(void);
void nest_cleanup(void);
void body(void);

StackAllocator sa = 0;

int main(void) {
    setup_env("log", 1, 1, nest_startup, nest_cleanup);
    body();
    cleanup();
    return 0;
}

void nest_startup(void) {
    sa = sa_new(0x100);
    sa_push(sa);
}

void nest_cleanup(void) {
    sa_pop(sa);
    sa_cleanup(sa);
}

void body(void) {
    printf("%d\n", !0);
    // for (uint8_t ch = 0; ch < 0x80; ++ch) {
    //     printf("\\%o -> %s\n", ch, ascii_cat_format(ch_cat(ch)));
    // }
}

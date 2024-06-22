#include <stdio.h>
#include <string.h>

#include <pthread.h>

#include <garage/deque.h>
#include <garage/array.h>
#include <garage/garage.h>

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
}

void nest_cleanup(void) {
    sa_pop(sa);
    sa_cleanup(sa);
}

static void deque_int_print(Deque dq) {
    for (size_t i = 0; i < dq->len; ++i) {
        int *p = deque_get(dq, i);
        printf("%d\n", *p);
    }
}

void body(void) {
    Deque dq = deque_new(sizeof (int));
    int val;
    for (size_t i = 0; i < 100; ++i) {
        val = i, deque_push_back(dq, &val);
        deque_deb_print(dq);
        printf("%p\n", deque_pop_front(dq));
    }
    deque_cleanup(dq);
}

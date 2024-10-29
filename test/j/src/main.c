#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>

#include <garage/prelude.h>
#include <garage/ascii.h>
#include <garage/stream.h>

static void nest_startup(void);
static void nest_cleanup(void);
static void body(void);

const char *const input_fname = "input";

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
    int Cleanup(fd_drop) fd = open(input_fname, O_RDONLY);
    assert(fd != -1, "Failed to open file '%s'\n", input_fname);
    Stream Cleanup(stream_drop) stream = stream_new(ENGINE_FILE_DESCRIPTOR, fd);
    for (;;) {
        void *peek = stream_peek(stream, 0);
        if (!peek) break;
        ch_deb_print(peek), printf("\n");
        stream_consume(stream, 1);
    }
}

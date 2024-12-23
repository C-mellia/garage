#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>

#include <garage/prelude.h>
#include <garage/ascii.h>
#include <garage/stream.h>

#include <input_pipe.h>
#include <input.h>

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

#if 0
    Stream Cleanup(stream_drop) stream = stream_new(ENGINE_FILE_DESCRIPTOR, fd);
    for (;;) {
        void *peek = stream_peek(stream, 0);
        ch_deb_print(peek), printf("\n");
        if (!peek) break;
        stream_consume(stream, 1);
    }
    stream_deb_print(stream), printf("\n");
#elif 0

    InputTokPipe Cleanup(input_tok_pipe_drop) input_tok_pipe = input_tok_pipe_new(fd);
    input_tok_pipe_deb_print(input_tok_pipe), printf("\n");
    Stream tok_stream = (void *)input_tok_pipe->tok_stream;
    for (;;) {
        InputTok *peek = stream_peek(tok_stream, 0);
        if (!peek) break;
        input_tok_deb_print(*peek), printf("\n");
        if ((*peek)->type == INPUT_TOK_EOF) break;
        // do {
        //     Slice Cleanup(slice_drop) slice = slice_from_arr((void *)(*peek)->arr);
        //     slice_print(slice);
        // } while(0);
        stream_consume(tok_stream, 1);
    }

#else

    InputPipe Cleanup(input_pipe_drop) input_pipe = input_pipe_new(fd);
    // input_pipe_deb_print(input_pipe), printf("\n");
    for (;;) {
        Input *peek = input_pipe_peek(input_pipe, 0);
        if (!peek) break;
        input_deb_print(*peek), printf("\n");
        if ((*peek)->status == INPUT_EMPTY) break;
        input_pipe_consume(input_pipe, 1);
    }
#endif
}

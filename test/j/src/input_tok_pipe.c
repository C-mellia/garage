#include <stdlib.h>
#include <string.h>

#include <garage/prelude.h>
#include <garage/deque.h>
#include <garage/ascii.h>

#include <input_tok.h>
#include <input_tok_pipe.h>
#include "./.input_tok_pipe.c"

void input_tok_pipe_init(InputTokPipe input_tok_pipe, int fd) {
    nul_check(InputTokPipe, input_tok_pipe), __input_tok_pipe_init(input_tok_pipe, fd);
}

InputTokPipe input_tok_pipe_new(int fd) {
    InputTokPipe input_tok_pipe = malloc(sizeof *input_tok_pipe);
    alloc_check(malloc, input_tok_pipe, sizeof *input_tok_pipe);
    return __input_tok_pipe_init(input_tok_pipe, fd), input_tok_pipe;
}

void input_tok_pipe_cleanup(InputTokPipe input_tok_pipe) {
    InputTokPipeData data = (void *)input_tok_pipe->data;
    Deque stack = (void *)data->stack;
    deq_cleanup(stack);

    stream_cleanup((void *)input_tok_pipe->fd_stream);
    stream_cleanup((void *)input_tok_pipe->tok_stream);
}

void *input_tok_pipe_drop(InputTokPipe *input_tok_pipe) {
    if (input_tok_pipe) input_tok_pipe_cleanup(*input_tok_pipe), free(*input_tok_pipe), *input_tok_pipe = 0;
    return input_tok_pipe;
}

int input_tok_pipe_deb_dprint(int fd, InputTokPipe input_tok_pipe) {
    if (!input_tok_pipe) return dprintf(fd, "(nil)");
    InputTokPipeData data = (void *)input_tok_pipe->data;
    Coord coord = (void *)data->coord;
    Deque stack = (void *)data->stack;

    String Cleanup(string_drop) string = string_new();
    string_fmt(string, "{coord: {ln: %zu, col: %zu}, stack: ", coord->ln, coord->col);
    string_fmt_func(string, (void *)deq_deb_dprint, stack);
    string_fmt(string, ", tok_stream: ");
    string_fmt_func(string, (void *)stream_deb_dprint, (void *)input_tok_pipe->tok_stream);
    string_fmt(string, "}");
    return string_dprint(fd, string);
}

int input_tok_pipe_deb_print(InputTokPipe input_tok_pipe) {
    return fflush(stdout), input_tok_pipe_deb_dprint(1, input_tok_pipe);
}

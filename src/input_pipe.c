#include <stdlib.h>
#include <string.h>
#include <garage/prelude.h>
#include <garage/input_pipe.h>
#include <garage/input.h>

typedef struct tok_stream_status3 {
    int should_stop, should_consume, should_stash;
} *TokStreamStatus3;

typedef struct input_stream_status2 {
    int should_stop, should_consume;
} *InputStreamStatus2;

#include "./.input_pipe.c"

void input_pipe_init(InputPipe input_pipe, int fd) {
    nul_check(InputPipe, input_pipe), __input_pipe_init(input_pipe, fd);
}

InputPipe input_pipe_new(int fd) {
    InputPipe input_pipe = malloc(sizeof *input_pipe);
    alloc_check(malloc, input_pipe, sizeof *input_pipe);
    return __input_pipe_init(input_pipe, fd), input_pipe;
}

void input_pipe_cleanup(InputPipe input_pipe) {
    if (!input_pipe) return;
    deq_cleanup((void *)input_pipe->stack);
    stream_cleanup((void *)input_pipe->input_stream);
    stream_cleanup((void *)input_pipe->tok_stream);
    stream_cleanup((void *)input_pipe->fd_stream);
}

void *input_pipe_drop(InputPipe *input_pipe) {
    if (input_pipe) input_pipe_cleanup(*input_pipe), free(*input_pipe), *input_pipe = 0;
    return input_pipe;
}

int input_pipe_deb_dprint(int fd, InputPipe input_pipe) {
    if (!input_pipe) return dprintf(fd, "(nil)");
    Deque stack = (void *)input_pipe->stack;
    Stream stream = (void *)input_pipe->input_stream;
    String Cleanup(string_drop) string = string_new();

    string_fmt(string, "{stack: ");
    string_fmt_func(string, (void *)deq_deb_dprint, stack);
    string_fmt(string, ", input_stream: ");
    string_fmt_func(string, (void *)stream_deb_dprint, stream);
    string_fmt(string, "}");
    return string_dprint(fd, string);
}

int input_pipe_deb_print(InputPipe input_pipe) {
    return fflush(stdout), input_pipe_deb_dprint(1, input_pipe);
}

void *input_pipe_peek(InputPipe input_pipe, size_t idx) {
    nul_check(InputPipe, input_pipe);
    Stream input_stream = (void *)input_pipe->input_stream;
    return stream_peek(input_stream, idx);
}

void input_pipe_consume(InputPipe input_pipe, size_t count) {
    nul_check(InputPipe, input_pipe);
    Stream input_stream = (void *)input_pipe->input_stream;
    return(void) stream_consume(input_stream, count);
}

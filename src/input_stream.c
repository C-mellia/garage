#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>

#include <garage/prelude.h>
#include <garage/stream.h>
#include <garage/ascii.h>
#include <garage/input_tok.h>
#include <garage/input.h>
#include <garage/input_stream.h>

#include "./.input_tok.c"
#include "./.input.c"

static inline __attribute__((unused))
void __input_stream_init(InputStream input_stream, int fd);

void input_stream_init(InputStream input_stream, int fd) {
    nul_check(InputStream, input_stream), __input_stream_init(input_stream, fd);
}

InputStream input_stream_new(int fd) {
    InputStream input_stream = malloc(sizeof *input_stream);
    alloc_check(malloc, input_stream, sizeof *input_stream);
    return __input_stream_init(input_stream, fd), input_stream;
}

void input_stream_cleanup(InputStream input_stream) {
    if (!input_stream) return;
    stream_cleanup((void *)input_stream->stream);
}

void *input_stream_drop(InputStream *input_stream) {
    if (input_stream) input_stream_cleanup(*input_stream), free(*input_stream), *input_stream = 0;
    return input_stream;
}

int input_stream_deb_dprint(int fd, InputStream input_stream) {
    if (!input_stream) return dprintf(fd, "(nil)");
    String Cleanup(string_drop) string = string_new();
    Coord coord = (void *)input_stream->coord;
    Stream stream = (void *)input_stream->stream;
    string_fmt(string, "{coord: {ln: %zu, col: %zu}, stream: ", coord->ln, coord->col);
    string_fmt_func(string, (void *)stream_deb_dprint, stream);
    string_fmt(string, "}");
    return string_dprint(fd, string);
}

int input_stream_deb_print(InputStream input_stream) {
    return fflush(stdout), input_stream_deb_dprint(1, input_stream);
}

static void __input_stream_init(InputStream input_stream, int fd) {
    memset((void *)input_stream->coord, 0, sizeof (struct coord));
    Stream stream = (void *)input_stream->stream;
    Coord coord = (void *)input_stream->coord;
    Stream ch_stream = stream_new(ENGINE_FILE_DESCRIPTOR, fd);
    Stream tok_stream = stream_new(ENGINE_NESTED_STREAM, ch_stream, (void *)input_tok_stream_next, (void *)input_tok_should_end, coord, (void *)input_tok_drop, sizeof(struct input_tok));
    stream_init(stream, ENGINE_NESTED_STREAM, tok_stream, (void *)input_stream_next, (void *)input_should_end, 0, (void *)input_drop, sizeof(struct input));
}

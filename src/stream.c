#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>

#include <garage/log.h>
#include <garage/string.h>
#include <garage/slice.h>
#include <garage/deque.h>
#include <garage/engine.h>
#include <garage/stream.h>

#include "./.stream.c"

void stream_init(Stream stream, EngineType engine_type, ...) {
    nul_check(Stream, stream);
    va_list Cleanup(va_list_drop) args = {0};
    va_start(args, engine_type), __stream_vinit(stream, engine_type, args);
}

Stream stream_new(EngineType engine_type, ...) {
    va_list Cleanup(va_list_drop) args = {0};
    Stream stream = malloc(sizeof *stream);
    alloc_check(malloc, stream, sizeof *stream);
    va_start(args, engine_type);
    return __stream_vinit(stream, engine_type, args), stream;
}

void stream_cleanup(Stream stream) {
    if (!stream) return;
    void *(*drop)(void **item) = stream->engine_drop? : (void *)dummy_drop;
    engine_cleanup((void *)stream->engine);
    while (deq_len((void *)stream->deq)) {
        drop(deq_pop_front((void *)stream->deq));
    }
    deq_cleanup((void *)stream->deq);
}

void *stream_drop(Stream *stream) {
    if (stream) stream_cleanup(*stream), free(*stream), *stream = 0;
    return stream;
}

int stream_deb_dprint(int fd, Stream stream) {
    if (!stream) return dprintf(fd, "(nil)");
    String Cleanup(string_drop) string = string_new();
    // printf("%d, %p, %zu, %p\n", stream->engine_type, stream->engine_produce, stream->engine_align, stream->engine_data);
    string_fmt(string, "{engine: ");
    string_fmt_func(string, (void *)engine_deb_dprint, (void *)stream->engine);
    string_fmt(string, ", deq: ");
    string_fmt_func(string, (void *)deq_deb_dprint, (void *)stream->deq);
    string_fmt(string, "}");
    return string_dprint(fd, string);
}

int stream_deb_print(Stream stream) {
    return fflush(stdout), stream_deb_dprint(1, stream);
}

void *stream_peek(Stream stream, size_t off) {
    nul_check(Stream, stream);
    deq_buffer_check_len((void *)stream->deq, (void *)stream->engine, off + 1);
    return deq_get((void *)stream->deq, off);
}

size_t stream_consume(Stream stream, size_t count) {
    nul_check(Stream, stream);
    void *(*drop)(void **item) = stream->engine_drop? : (void *)dummy_drop;
    deq_buffer_check_len((void *)stream->deq, (void *)stream->engine, count);
    // deq_hex_print((void *)stream->deq), printf("\n");
    while (count && drop(deq_pop_front((void *)stream->deq))) --count;
    return count;
}

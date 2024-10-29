#ifndef GARAGE_STREAM_H
#define GARAGE_STREAM_H 1

#include <garage/types.h>
#include <garage/engine.h>

typedef struct stream_status3 {
    int should_stop, should_peek, should_consume;
} *StreamStatus3;

/**
* A different design than `scanner` that datas are not buffered in a dynamic
* array instead a deque is used for optimized pop_front, and to allow only read
* data when needed.
*/
typedef struct stream {
    Phantom deq;
    struct {
        size_t deq_begin, deq_len;

        Phantom deq_slice;
        struct {
            void *deq_slice_mem;
            size_t deq_slice_align, deq_slice_len;
        };
    };

    Phantom engine;
    struct {
        EngineType engine_type;
        int *(*engine_produce) (void *engine_data, void *buf, size_t len, size_t align);
        void *(*engine_drop)(void **item);
        size_t engine_align;
        void *engine_data;
    };
} *Stream;

void stream_init(Stream stream, EngineType engine_type, ...);
Stream stream_new(EngineType engine_type, ...);
void stream_cleanup(Stream stream);
void *stream_drop(Stream *stream);

int stream_deb_dprint(int fd, Stream stream);
int stream_deb_print(Stream stream);

void *stream_peek(Stream stream, size_t off);
size_t stream_consume(Stream stream, size_t count);

#endif // GARAGE_STREAM_H

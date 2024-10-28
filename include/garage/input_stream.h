#ifndef INPUT_STREAM_H
#define INPUT_STREAM_H 1

#include <garage/engine.h>
#include <garage/types.h>
#include <stddef.h>

/**
* (file) -> ch_stream -> tok_stream -> input_stream -> (user space)
*/
typedef struct input_stream {
    Phantom coord;
    struct {
        size_t ln, col;
    };

    Phantom stream;
    struct {
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
    };
} *InputStream;

void input_stream_init(InputStream input_stream, int fd);
InputStream input_stream_new(int fd);
void input_stream_cleanup(InputStream input_stream);
void *input_stream_drop(InputStream *input_stream);

int input_stream_deb_dprint(int fd, InputStream input_stream);
int input_stream_deb_print(InputStream input_stream);

#endif // INPUT_STREAM_H

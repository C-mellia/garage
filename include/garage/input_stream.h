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

    Phantom stack;
    struct {
        size_t stack_len;

        Phantom stack_slice;
        struct {
            void *stack_slice_mem;
            size_t stack_slice_align, stack_slice_len;
        };
    };

    Phantom input_stream;
    struct {
        Phantom input_stream_deq;
        struct {
            size_t input_stream_deb_begin, input_stream_deb_len;

            Phantom input_stream_deb_slice;
            struct {
                void *input_stream_deb_slice_mem;
                size_t input_stream_deb_slice_align, input_stream_deb_slice_len;
            };
        };

        Phantom input_stream_engine;
        struct {
            EngineType input_stream_engine_type;
            int *(*input_stream_engine_produce) (void *input_stream_engine_data, void *buf, size_t len, size_t align);
            void *(*input_stream_engine_drop)(void **item);
            size_t input_stream_engine_align;
            void *input_stream_engine_data;
        };
    };

    Phantom input_tok_stream;
    struct {
        Phantom input_tok_stream_deq;
        struct {
            size_t input_tok_stream_deq_begin, input_tok_stream_deq_len;

            Phantom input_tok_stream_deq_slice;
            struct {
                void *input_tok_stream_deq_slice_mem;
                size_t input_tok_stream_deq_slice_align, input_tok_stream_deq_slice_len;
            };
        };

        Phantom input_tok_stream_engine;
        struct {
            EngineType input_tok_stream_engine_type;
            int *(*input_tok_stream_engine_produce) (void *input_tok_stream_engine_data, void *buf, size_t len, size_t align);
            void *(*input_tok_stream_engine_drop)(void **item);
            size_t input_tok_stream_engine_align;
            void *input_tok_stream_engine_data;
        };
    };

    Phantom fd_stream;
    struct {
        Phantom fd_stream_deq;
        struct {
            size_t fd_stream_deq_begin, fd_stream_deq_len;

            Phantom fd_stream_deq_slice;
            struct {
                void *fd_stream_deq_slice_mem;
                size_t fd_stream_deq_slice_align, fd_stream_deq_slice_len;
            };
        };

        Phantom fd_stream_engine;
        struct {
            EngineType fd_stream_engine_type;
            int *(*fd_stream_engine_produce) (void *fd_stream_engine_data, void *buf, size_t len, size_t align);
            void *(*fd_stream_engine_drop)(void **item);
            size_t fd_stream_engine_align;
            void *fd_stream_engine_data;
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

#ifndef INPUT_TOK_PIPE_H
#define INPUT_TOK_PIPE_H 1

#include <garage/stream.h>

typedef struct input_pipe_data {
    /**
     * An extra stack to store the tokens that are processed but not yet
     * ready to be piped to the streams. And therefore requires a separate
     * stack to be able to push tokens at the back of the stack, and also
     * pop tokens from the back of the stack. If the tokens at the front of
     * the stack is available, signified by another integer signal, it is
     * also possible to pop tokens from the front of the stack. It is
     * supposedly a deque, but from the perspective of the purpose of the
     * stack, it will still be considered a stack.
     *
     * Currently only the tokenization process requires an extra stack in the
     * design
     */
    Phantom/* InputTok */ stack;
    struct {
        size_t deq_begin, deq_len;

        Phantom deq_slice;
        struct {
            void *deq_slice_mem;
            size_t deq_slice_align, deq_slice_len;
        };
    };
} *InputPipeData;

typedef struct input_pipe {
    Phantom data;
    struct {
        Phantom stack;
        struct {
            size_t deq_begin, deq_len;

            Phantom deq_slice;
            struct {
                void *deq_slice_mem;
                size_t deq_slice_align, deq_slice_len;
            };
        };
    };

    Phantom input_stream;
    struct {
        Phantom input_stream_deq;
        struct {
            size_t input_stream_deq_begin, input_stream_deq_len;

            Phantom input_stream_deq_slice;
            struct {
                void *input_stream_deq_slice_mem;
                size_t input_stream_deq_slice_align, input_stream_deq_slice_len;
            };
        };

        Phantom input_stream_engine;
        struct {
            EngineType input_stream_engine_type;
            void *(*input_stream_engine_drop)(void **item);
            size_t input_stream_engine_align;
            void *input_stream_engine_data;
        };
    };

    Phantom tok_stream;
    struct {
        Phantom tok_stream_deq;
        struct {
            size_t tok_stream_deq_begin, tok_stream_deq_len;

            Phantom tok_stream_deq_slice;
            struct {
                void *tok_stream_deq_slice_mem;
                size_t tok_stream_deq_slice_align, tok_stream_deq_slice_len;
            };
        };

        Phantom tok_stream_engine;
        struct {
            EngineType tok_stream_engine_type;
            void *(*tok_stream_engine_drop)(void **item);
            size_t tok_stream_engine_align;
            void *tok_stream_engine_data;
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
            void *(*fd_stream_engine_drop)(void **item);
            size_t fd_stream_engine_align;
            void *fd_stream_engine_data;
        };
    };
} *InputPipe;

void input_pipe_init(InputPipe input_pipe, int fd);
InputPipe input_pipe_new(int fd);
void input_pipe_cleanup(InputPipe input_pipe);
void *input_pipe_drop(InputPipe *input_pipe);

int input_pipe_deb_dprint(int fd, InputPipe input_pipe);
int input_pipe_deb_print(InputPipe input_pipe);

void *input_pipe_peek(InputPipe input_pipe, size_t idx);
void input_pipe_consume(InputPipe input_pipe, size_t count);

#endif // INPUT_TOK_PIPE_H

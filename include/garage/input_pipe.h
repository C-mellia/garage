#ifndef INPUT_TOK_PIPE_H
#define INPUT_TOK_PIPE_H 1

#include <garage/stream.h>

typedef struct pipe_status3 {
    int should_stop, should_consume, should_stash;
} *PipeStatus3;

typedef struct coord {
    size_t ln, col;
} *Coord;

typedef struct input_tok_pipe_data {
    Phantom coord;
    struct {
        size_t ln, col;
    };

    /**
     * An extra stack to store the tokens that are processed but not yet
     * ready to be piped to the streams. And therefore requires a separate
     * stack to be able to push tokens at the back of the stack, and also
     * pop tokens from the back of the stack. If the tokens at the front of
     * the stack is available, signified by another integer signal, it is
     * also possible to pop tokens from the front of the stack. It is
     * supposedly a deque, but from the perspective of the purpose of the
     * stack, it will still be considered a stack.
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
} *InputTokPipeData;

typedef struct input_tok_pipe {
    Phantom data;
    struct {
        Phantom coord;
        struct {
            size_t ln, col;
        };

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
} *InputTokPipe;

void input_tok_pipe_init(InputTokPipe input_tok_pipe, int fd);
InputTokPipe input_tok_pipe_new(int fd);
void input_tok_pipe_cleanup(InputTokPipe input_tok_pipe);
void *input_tok_pipe_drop(InputTokPipe *input_tok_pipe);

int input_tok_pipe_deb_dprint(int fd, InputTokPipe input_tok_pipe);
int input_tok_pipe_deb_print(InputTokPipe input_tok_pipe);

#endif // INPUT_TOK_PIPE_H

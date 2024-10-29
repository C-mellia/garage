#ifndef _INPUT_STREAM_C
#define _INPUT_STREAM_C 1

static inline __attribute__((unused))
void __input_stream_init(InputTokPipe input_tok_pipe, int fd);
static inline __attribute__((unused))
InputTok input_tok_stream_next(Stream input_tok_stream, InputTokPipeData data);
/**
* Possibly multiple tokens will be popped from the stack, but eventually only
* one token will be returned.
*/
static inline __attribute__((unused))
InputTok input_tok_stack_pop(Deque stack, size_t ln, size_t col, size_t avail_toks);
static inline __attribute__((unused))
void input_tok_add_ch(InputTok input_tok, Stream fd_stream, StreamStatus3 status, void *peek);

static void __input_tok_pipe_init(InputTokPipe input_tok_pipe, int fd) {
    InputTokPipeData data = (void *)input_tok_pipe->data;
    memset(data, 0, sizeof *data);
    Coord coord = (void *)data->coord;
    Deque stack = (void *)data->stack;

    Stream tok_stream = (void *)input_tok_pipe->tok_stream, fd_stream = (void *)input_tok_type->fd_stream;
    memset(coord, 0, sizeof *coord), deque_init(stack, sizeof(InputTok));
    stream_init(fd_stream, ENGINE_FILE_DESCRIPTOR, fd);
    stream_init(tok_stream, ENGINE_NESTED_STREAM,
                (void *)input_tok_stream_next, (void *)input_tok_should_end,
                data, (void *)input_tok_drop,
                sizeof(InputTok));
}

static InputTok input_tok_stream_next(Stream fd_stream, InputTokPipeData data) {
    Coord coord = (void *)data->coord;
    Deque stack = (void *)data->stack;
    InputTok tok = input_tok_stack_pop(stack, data->avail_toks);
    struct stream_status3 status = {
        .should_stop = 0, .should_peek = 0, .should_consume = 0,
    };
    for (; !status->should_stop;) {
        void *peek = stream_peek(stream, 0);
        if (!peek) break;
        input_tok_add_ch(tok, fd_stream, status, peek);
        stream_consume(stream, 1);
    }
}

static InputTok input_tok_stack_pop(Deque stack, size_t ln, size_t col, size_t avail_toks) {
    size_t unavail = avail_toks < deq_len(stack)? deq_len(stack) - avail_toks: 0;
    if (!unavail) return input_tok_new(INPUT_TOK_NONE, ln, col);
    InputTok buf[unavail];

    // InputTok *ptr = deq_pop_back_mem(stack, buf, unavail);
}

#endif // _INPUT_STREAM_C

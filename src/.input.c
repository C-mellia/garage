#ifndef _INPUT_C
#define _INPUT_C 1

static inline __attribute__((unused))
void __input_init(Input input);
static inline __attribute__((unused))
Input input_stream_next(Stream tok_stream);
static inline __attribute__((unused))
int input_should_end(Input input);
static inline __attribute__((unused))
void input_stream_add_tok(Input input, Stream tok_stream, StreamStatus3 status, Array errs, InputTok peek);

static void __input_init(Input input) {
    memset(input, 0, sizeof *input);
}

static Input input_stream_next(Stream tok_stream) {
    nul_check(Stream, tok_stream);
    Array Cleanup(arr_drop) errs = arr_new(sizeof (struct string));
    Input input = input_new();
    struct stream_status3 __status = {
        .should_stop = 0, .should_peek = 0, .should_consume = 0,
    }, *status = &__status;
    for (; !status->should_stop;) {
        void *peek = stream_peek(tok_stream, status->should_peek);
        InputTok tok = deref(InputTok, peek);
        input_stream_add_tok(input, tok_stream, status, errs, tok);
        stream_consume(tok_stream, status->should_consume);
    }
    if (arr_len(errs)) {
        for (size_t i = 0; i < arr_len(errs); ++i) {
            String Cleanup(string_drop) string = deref(String, arr_get(errs, i));
            string_dprint(__logfd, string), dprintf(__logfd, "\n");
        }
        input_drop(&input), panic("%zu Error(s) occured during the process of creating `input` objects", arr_len(errs));
    } else {
        return input;
    }
}

static int input_should_end(Input input) {
    return !input || (!input->in && !input->res);
}

static void input_stream_add_tok(Input input, Stream tok_stream, StreamStatus3 status, Array errs, InputTok peek) {
    (void)tok_stream;
#define invalid_tok(FMT, ...) ({\
    String msg_string = string_new();\
    string_fmt(msg_string, "%s:%d:%s: Invalid InputToken: " FMT ": with tok: ",\
               __FILE__, __LINE__, __func__, ##__VA_ARGS__);\
    string_fmt_func(msg_string, (void *)input_tok_deb_dprint, peek);\
    arr_push_back(errs, &msg_string);\
})

#define null_token(FMT, ...) ({\
    String msg_string = string_new();\
    string_fmt(msg_string, "%s:%d:%s: Unexpected null token: " FMT ": with tok: ",\
               __FILE__, __LINE__, __func__, ##__VA_ARGS__);\
    string_fmt_func(msg_string, (void *)input_tok_deb_dprint, peek);\
    arr_push_back(errs, &msg_string);\
})

#define inval_input_struct(FMT, ...) ({\
    String msg_string = string_new();\
    string_fmt(msg_string, "%s:%d:%s: Invalid Input structure: " FMT ": with tok: ",\
               __FILE__, __LINE__, __func__, ##__VA_ARGS__);\
    string_fmt_func(msg_string, (void *)input_tok_deb_dprint, peek);\
    arr_push_back(errs, &msg_string);\
})

#define set_status(SHOULD_STOP, SHOULD_PEEK, SHOULD_CONSUME) ({\
    status->should_stop = (SHOULD_STOP), status->should_peek = (SHOULD_PEEK), status->should_consume = (SHOULD_CONSUME);\
})

    if (!peek) {
        null_token("input token `peek` is null at this point"), set_status(1, 0, 0);
        return;
    }

    switch(input->status) {
        case INPUT_EMPTY: {
            if (peek->type == INPUT_TOK_EOF) {
                set_status(1, 0, 1);
            } else if (peek->type == INPUT_TOK_TEXT) {
                input->in = input_tok_clone(peek), set_status(0, 0, 1);
            } else if (peek->type == INPUT_TOK_EQ_SPLIT) {
                input->eq_splitter = input_tok_clone(peek), set_status(1, 0, 1);
            } else {
                invalid_tok("unexpected token occured, expected `text` or `eof` token"), set_status(1, 0, 0);
            }
        } break;

        case INPUT_IN: {
            if (peek->type == INPUT_TOK_EOF) {
                set_status(1, 0, 1);
            } else if (peek->type == INPUT_TOK_MINUS_SPLIT) {
                input->minus_splitter = input_tok_clone(peek), status->should_consume = 1;
            } else if (peek->type == INPUT_TOK_EQ_SPLIT) {
                input->eq_splitter = input_tok_clone(peek), status->should_consume = 1, status->should_stop = 1;
            } else {
                invalid_tok("unexpected token occured, expected `eq_splitter` or `minus_splitter` or `eof` token"), set_status(1, 0, 0);
            }
        } break;

        case INPUT_IN_RES: {
            if (peek->type == INPUT_TOK_EOF) {
                set_status(1, 0, 1);
            } else if (peek->type == INPUT_TOK_EQ_SPLIT) {
                input->eq_splitter = input_tok_clone(peek), set_status(1, 0, 1);
            } else {
                invalid_tok("unexpected token occured, expected `eq_splitter` or `eof` token"), set_status(1, 0, 0);
            }
        } break;

        case INPUT_FULL: {
            set_status(1, 0, 1);
        } break;

        default: inval_input_struct("invalid structure of `input`, {in: %p, minus_splitter: %p, res: %p, eq_splitter: %p}",
                                    input->in, input->minus_splitter, input->res, input->eq_splitter), set_status(1, 0, 0);
    }
}

#undef inval_input_struct
#undef null_token
#undef set_status
#undef invalid_tok

#endif // _INPUT_C

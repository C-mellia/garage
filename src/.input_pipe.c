#ifndef _GARAGE_INPUT_PIPE_C
#define _GARAGE_INPUT_PIPE_C 1

#include <garage/deque.h>
#include <garage/ascii.h>
#include <garage/input_tok.h>

static inline __attribute__((unused))
void __input_pipe_init(InputPipe input_pipe, int fd);
static inline __attribute__((unused))
ssize_t input_tok_stream_next(Deque deq, Stream tok_stream, InputPipeData data);
static inline __attribute__((unused))
void input_tok_stack_concat_text(Deque stack);
static inline __attribute__((unused))
void input_tok_add_ch(InputTok tok, TokStreamStatus3 status, Array errs, void *peek);
static inline __attribute__((unused))
ssize_t input_stream_next(Deque deq, Stream input_stream);
static inline __attribute__((unused))
void input_add_tok(Input input, InputStreamStatus2 status, Array errs, InputTok *peek);

extern const char *__input_tok_type_str[__INPUT_TOK_COUNT];

static void __input_pipe_init(InputPipe input_pipe, int fd) {
    InputPipeData data = (void *)input_pipe->data;
    Deque stack = (void *)input_pipe->stack;

    Stream input_stream = (void *)input_pipe->input_stream;
    Stream tok_stream = (void *)input_pipe->tok_stream;
    Stream fd_stream = (void *)input_pipe->fd_stream;

    deq_init(stack, sizeof (InputTok));
    stream_init(fd_stream, ENGINE_FILE_DESCRIPTOR, fd);
    stream_init(
        tok_stream, ENGINE_NESTED_STREAM,
        fd_stream, (void *)input_tok_stream_next,
        data, (void *)input_tok_drop,
        sizeof(InputTok)
    );
    stream_init(
        input_stream, ENGINE_NESTED_STREAM,
        tok_stream, (void *)input_stream_next,
        0, (void *)input_drop,
        sizeof(Input)
    );
}

static ssize_t input_tok_stream_next(Deque deq, Stream tok_stream, InputPipeData data) {
    Deque stack = (void *)data->stack;
    struct tok_stream_status3 __status = {
        .should_stop = 0, .should_consume = 0, .should_stash = 0,
    }, *status = &__status;

    ssize_t prev_len = deq_len(deq);

    Array Cleanup(arr_drop) errs = arr_new(sizeof(String));
    InputTok tok = input_tok_new(INPUT_TOK_NONE);

    for (; !status->should_stop;) {
        void *peek = stream_peek(tok_stream, 0);
        input_tok_add_ch(tok, status, errs, peek);

        if (status->should_consume) {
            if (arr_push_back((void *)tok->arr, peek), stream_consume(tok_stream, 1)) break;
        }

        if (status->should_stash) {
            deq_push_back(stack, &tok);
            tok = input_tok_new(INPUT_TOK_NONE);
        }
    }

    if (arr_len(errs)) {
        for (size_t i = 0; i < arr_len(errs); ++i) {
            String Cleanup(string_drop) err = deref(String, arr_get(errs, i));
            string_dprint(__logfd, err), dprintf(__logfd, "\n");
        }
        dprintf(__logfd, "%zu errors occurred during tokenization\n", arr_len(errs));
        _abort(), __builtin_unreachable();
    } else {
        if (deq_len(stack)) input_tok_stack_concat_text(stack);
        while (deq_len(stack)) deq_push_back(deq, deq_pop_front(stack));
        if (tok->type > INPUT_TOK_NONE) deq_push_back(deq, &tok);
        else input_tok_drop(&tok);
    }

    return (ssize_t)deq_len(deq) - prev_len;
}

static void input_tok_stack_concat_text(Deque stack) {
    InputTok tok = deref(InputTok, deq_pop_back(stack));
    while (deq_len(stack)) {
        InputTok prev = deref(InputTok, deq_pop_back(stack));
        arr_cat((void *)prev->arr, (void *)tok->arr);
        input_tok_drop(&tok), tok = prev;
    }
    deq_push_back(stack, &tok);
}

static void input_tok_add_ch(InputTok tok, TokStreamStatus3 status, Array errs, void *peek) {
#define invalid_char(FMT, ...) ({\
    String __err = string_new();\
    string_fmt(__err, "%s:%d:%s: Invalid Character: " FMT ": with ch: ",\
               __FILE__, __LINE__, __func__, ##__VA_ARGS__);\
    string_fmt_func(__err, (void *)ch_deb_dprint, peek);\
    arr_push_back(errs, &__err);\
})

#define not_implemented(FMT, ...) ({\
    String __err = string_new();\
    string_fmt(__err, "%s:%d:%s: Not implemented: " FMT ": with ch: ",\
               __FILE__, __LINE__, __func__, ##__VA_ARGS__);\
    string_fmt_func(__err, (void *)ch_deb_dprint, peek);\
    arr_push_back(errs, &__err);\
})

#define set_status(SHOULD_STOP, SHOULD_CONSUME, SHOULD_STASH) ({\
    int __should_stop = (SHOULD_STOP), __should_consume = (SHOULD_CONSUME), __should_stash = (SHOULD_STASH);\
    status->should_stop = __should_stop, status->should_consume = __should_consume, status->should_stash = __should_stash;\
})

#define ch_is_text(peek) (ch_is_alpha(peek) || ch_is_num(peek) || ch_is_pun(peek) || ch_is_ws(peek))

#define get_type_str (tok->type < __INPUT_TOK_COUNT? __input_tok_type_str[tok->type]: "Invalid Input Token Type")

    switch(tok->type) {
        case INPUT_TOK_NONE: {
            if (!peek) {
                tok->type = INPUT_TOK_EOF, set_status(0, 0, 0);
            } else if (ch_is(peek, "\n", 1)) {
                tok->type = INPUT_TOK_TEXT, set_status(0, 1, 1);
            } else if (ch_is(peek, "=", 1)) {
                tok->type = __INPUT_TOK_EQ_SPLITTER0, set_status(0, 0, 0);
            } else if (ch_is(peek, "-", 1)) {
                tok->type = __INPUT_TOK_MINUS_SPLITTER0, set_status(0, 0, 0);
            } else if (ch_is_text(peek)) {
                tok->type = INPUT_TOK_TEXT, set_status(0, 1, 0);
            } else {
                invalid_char("In `INPUT_TOK_NONE` case"), set_status(1, 0, 0);
            }
        } break;

        case INPUT_TOK_EOF: {
            set_status(1, 0, 0);
        } break;

        case INPUT_TOK_TEXT: {
            if (!peek) {
                set_status(1, 0, 1);
            } else if (ch_is(peek, "\n", 1)) {
                set_status(0, 1, 1);
            } else if (ch_is_text(peek)) {
                set_status(0, 1, 0);
            } else {
                invalid_char("In `INPUT_TOK_TEXT` case"), set_status(1, 0, 0);
            }
        } break;

        case __INPUT_TOK_EQ_SPLITTER0: {
            if (!peek) {
                tok->type = INPUT_TOK_TEXT, set_status(1, 0, 0);
            } else if (ch_is(peek, "=", 1)) {
                tok->type = __INPUT_TOK_EQ_SPLITTER1, set_status(0, 1, 0);
            } else if (ch_is_text(peek)) {
                tok->type = INPUT_TOK_TEXT, set_status(0, 1, 0);
            } else {
                invalid_char("In `__INPUT_TOK_EQ_SPLITTER0` case"), set_status(1, 0, 0);
            }
        } break;

        case __INPUT_TOK_EQ_SPLITTER1: {
            if (!peek) {
                tok->type = INPUT_TOK_TEXT, set_status(1, 0, 0);
            } else if (ch_is(peek, "=", 1)) {
                tok->type = __INPUT_TOK_EQ_SPLITTER2, set_status(0, 1, 0);
            } else if (ch_is_text(peek)) {
                tok->type = INPUT_TOK_TEXT, set_status(0, 1, 0);
            } else {
                invalid_char("In `__INPUT_TOK_EQ_SPLITTER1` case"), set_status(1, 0, 0);
            }
        } break;

        case __INPUT_TOK_EQ_SPLITTER2: {
            if (!peek) {
                tok->type = INPUT_TOK_TEXT, set_status(1, 0, 0);
            } else if (ch_is(peek, "=", 1)) {
                tok->type = __INPUT_TOK_EQ_SPLITTER3, set_status(0, 1, 0);
            } else if (ch_is_text(peek)) {
                tok->type = INPUT_TOK_TEXT, set_status(0, 1, 0);
            } else {
                invalid_char("In `__INPUT_TOK_EQ_SPLITTER2` case"), set_status(1, 0, 0);
            }
        } break;

        case __INPUT_TOK_EQ_SPLITTER3: {
            if (!peek) {
                tok->type = INPUT_TOK_TEXT, set_status(1, 0, 0);
            } else if (ch_is(peek, "=", 1)) {
                tok->type = __INPUT_TOK_EQ_SPLITTER4, set_status(0, 1, 0);
            } else if (ch_is_text(peek)) {
                tok->type = INPUT_TOK_TEXT, set_status(0, 1, 0);
            } else {
                invalid_char("In `__INPUT_TOK_EQ_SPLITTER3` case"), set_status(1, 0, 0);
            }
        } break;

        case __INPUT_TOK_EQ_SPLITTER4: {
            if (!peek) {
                tok->type = INPUT_TOK_TEXT, set_status(1, 0, 0);
            } else if (ch_is(peek, "=", 1)) {
                tok->type = __INPUT_TOK_EQ_SPLITTER5, set_status(0, 1, 0);
            } else if (ch_is_text(peek)) {
                tok->type = INPUT_TOK_TEXT, set_status(0, 1, 0);
            } else {
                invalid_char("In `__INPUT_TOK_EQ_SPLITTER4` case"), set_status(1, 0, 0);
            }
        } break;

        case __INPUT_TOK_EQ_SPLITTER5: {
            if (!peek) {
                tok->type = INPUT_TOK_TEXT, set_status(1, 0, 0);
            } else if (ch_is(peek, "=", 1)) {
                tok->type = INPUT_TOK_EQ_SPLITTER, set_status(0, 1, 0);
            } else if (ch_is_text(peek)) {
                tok->type = INPUT_TOK_TEXT, set_status(0, 1, 0);
            } else {
                invalid_char("In `__INPUT_TOK_EQ_SPLITTER5` case"), set_status(1, 0, 0);
            }
        } break;

        case INPUT_TOK_EQ_SPLITTER: {
            if (!peek) {
                set_status(1, 1, 0);
            } else if (ch_is(peek, "\n", 1)) {
                set_status(1, 1, 0);
            } else if (ch_is_text(peek)) {
                set_status(0, 1, 0);
            } else {
                invalid_char("In `INPUT_TOK_EQ_SPLITTER` case"), set_status(1, 0, 0);
            }
        } break;

        case __INPUT_TOK_MINUS_SPLITTER0: {
            if (!peek) {
                tok->type = INPUT_TOK_TEXT, set_status(1, 0, 0);
            } else if (ch_is(peek, "-", 1)) {
                tok->type = __INPUT_TOK_MINUS_SPLITTER1, set_status(0, 1, 0);
            } else if (ch_is_text(peek)) {
                tok->type = INPUT_TOK_TEXT, set_status(0, 1, 0);
            } else {
                invalid_char("In `__INPUT_TOK_MINUS_SPLITTER0` case"), set_status(1, 0, 0);
            }
        } break;

        case __INPUT_TOK_MINUS_SPLITTER1: {
            if (!peek) {
                tok->type = INPUT_TOK_TEXT, set_status(1, 0, 0);
            } else if (ch_is(peek, "-", 1)) {
                tok->type = __INPUT_TOK_MINUS_SPLITTER2, set_status(0, 1, 0);
            } else if (ch_is_text(peek)) {
                tok->type = INPUT_TOK_TEXT, set_status(0, 1, 0);
            } else {
                invalid_char("In `__INPUT_TOK_MINUS_SPLITTER1` case"), set_status(1, 0, 0);
            }
        } break;

        case __INPUT_TOK_MINUS_SPLITTER2: {
            if (!peek) {
                tok->type = INPUT_TOK_TEXT, set_status(1, 0, 0);
            } else if (ch_is(peek, "-", 1)) {
                tok->type = __INPUT_TOK_MINUS_SPLITTER3, set_status(0, 1, 0);
            } else if (ch_is_text(peek)) {
                tok->type = INPUT_TOK_TEXT, set_status(0, 1, 0);
            } else {
                invalid_char("In `__INPUT_TOK_MINUS_SPLITTER2` case"), set_status(1, 0, 0);
            }
        } break;

        case __INPUT_TOK_MINUS_SPLITTER3: {
            if (!peek) {
                tok->type = INPUT_TOK_TEXT, set_status(1, 0, 0);
            } else if (ch_is(peek, "-", 1)) {
                tok->type = __INPUT_TOK_MINUS_SPLITTER4, set_status(0, 1, 0);
            } else if (ch_is_text(peek)) {
                tok->type = INPUT_TOK_TEXT, set_status(0, 1, 0);
            } else {
                invalid_char("In `__INPUT_TOK_MINUS_SPLITTER3` case"), set_status(1, 0, 0);
            }
        } break;

        case __INPUT_TOK_MINUS_SPLITTER4: {
            if (!peek) {
                tok->type = INPUT_TOK_TEXT, set_status(1, 0, 0);
            } else if (ch_is(peek, "-", 1)) {
                tok->type = __INPUT_TOK_MINUS_SPLITTER5, set_status(0, 1, 0);
            } else if (ch_is_text(peek)) {
                tok->type = INPUT_TOK_TEXT, set_status(0, 1, 0);
            } else {
                invalid_char("In `__INPUT_TOK_MINUS_SPLITTER4` case"), set_status(1, 0, 0);
            }
        } break;

        case __INPUT_TOK_MINUS_SPLITTER5: {
            if (!peek) {
                tok->type = INPUT_TOK_TEXT, set_status(1, 0, 0);
            } else if (ch_is(peek, "-", 1)) {
                tok->type = INPUT_TOK_MINUS_SPLITTER, set_status(0, 1, 0);
            } else if (ch_is_text(peek)) {
                tok->type = INPUT_TOK_TEXT, set_status(0, 1, 0);
            } else {
                invalid_char("In `__INPUT_TOK_MINUS_SPLITTER5` case"), set_status(1, 0, 0);
            }
        } break;

        case INPUT_TOK_MINUS_SPLITTER: {
            if (!peek) {
                set_status(1, 1, 0);
            } else if (ch_is(peek, "\n", 1)) {
                set_status(1, 1, 0);
            } else if (ch_is_text(peek)) {
                set_status(0, 1, 0);
            } else {
                invalid_char("In `INPUT_TOK_MINUS_SPLITTER` case"), set_status(1, 0, 0);
            }
        } break;

        default: not_implemented("Unknown token type: %s(%d)", get_type_str, tok->type), set_status(1, 0, 0);
    }
}

#undef ch_is_text
#undef get_type_str
#undef set_status
#undef not_implemented
#undef invalid_char

static ssize_t input_stream_next(Deque deq, Stream input_stream) {
    Input input = input_new(INPUT_EMPTY);
    struct input_stream_status2 __status = {
        .should_stop = 0, .should_consume = 0,
    }, *status = &__status;
    Array Cleanup(arr_drop) errs = arr_new(sizeof(String));

    ssize_t prev_len = deq_len(deq);

    for (; !status->should_stop;) {
        void *peek = stream_peek(input_stream, 0);
        input_add_tok(input, status, errs, peek);
        if (status->should_consume && stream_consume(input_stream, 1)) break;
    }

    if (arr_len(errs)) {
        for (size_t i = 0; i < arr_len(errs); ++i) {
            String Cleanup(string_drop) err = deref(String, arr_get(errs, i));
            string_dprint(__logfd, err), dprintf(__logfd, "\n");
        }
        dprintf(__logfd, "%zu errors occurred during tokenization\n", arr_len(errs));
        _abort(), __builtin_unreachable();
    } else {
        deq_push_back(deq, &input);
    }

    return (ssize_t)deq_len(deq) - prev_len;
}

static void input_add_tok(Input input, InputStreamStatus2 status, Array errs, InputTok *peek) {
#define invalid_tok(FMT, ...) ({\
    String __err = string_new();\
    string_fmt(__err, "%s:%d:%s: Invalid Token: " FMT ": with tok: ",\
               __FILE__, __LINE__, __func__, ##__VA_ARGS__);\
    string_fmt_func(__err, (void *)input_tok_deb_dprint, tok);\
    arr_push_back(errs, &__err);\
})

#define null_token(FMT, ...) ({\
    String msg_string = string_new();\
    string_fmt(msg_string, "%s:%d:%s: Unexpected null token: " FMT,\
               __FILE__, __LINE__, __func__, ##__VA_ARGS__);\
    arr_push_back(errs, &msg_string);\
})

#define inval_input_struct(FMT, ...) ({\
    String msg_string = string_new();\
    string_fmt(msg_string, "%s:%d:%s: Invalid Input structure: " FMT ": with tok: ",\
               __FILE__, __LINE__, __func__, ##__VA_ARGS__);\
    string_fmt_func(msg_string, (void *)input_tok_deb_dprint, tok);\
    arr_push_back(errs, &msg_string);\
})

#define set_status(SHOULD_STOP, SHOULD_CONSUME) ({\
    int __should_stop = (SHOULD_STOP), __should_consume = (SHOULD_CONSUME);\
    status->should_stop = __should_stop, status->should_consume = __should_consume;\
})

    if (!peek) {
        null_token("input token `peek` is null at this point"), set_status(1, 0);
        return;
    }

    InputTok tok = *peek;
    // input_tok_deb_dprint(__logfd, tok), dprintf(__logfd, "\n");

    switch(input->status) {
        case INPUT_EMPTY: {
            if (tok->type == INPUT_TOK_EOF) {
                set_status(1, 1);
            } else if (tok->type == INPUT_TOK_TEXT) {
                input->in = input_tok_clone(tok), set_status(0, 1);
                input->status = INPUT_IN;
            } else if (tok->type == INPUT_TOK_EQ_SPLITTER) {
                input->eq_splitter = input_tok_clone(tok), set_status(1, 1);
                input->status = INPUT_FULL;
            } else {
                invalid_tok("unexpected token occured, expected `text` or `eof` token"), set_status(1, 0);
            }
        } break;

        case INPUT_IN: {
            if (tok->type == INPUT_TOK_EOF) {
                set_status(1, 1);
            } else if (tok->type == INPUT_TOK_MINUS_SPLITTER) {
                input->minus_splitter = input_tok_clone(tok), set_status(0, 1);
                input->status = INPUT_IN_RES;
            } else if (tok->type == INPUT_TOK_EQ_SPLITTER) {
                input->eq_splitter = input_tok_clone(tok), set_status(0, 1);
                input->status = INPUT_FULL;
            } else {
                invalid_tok("unexpected token occured, expected `eq_splitter` or `minus_splitter` or `eof` token"), set_status(1, 0);
            }
        } break;

        case __INPUT_IN_RES: {
            if (tok->type == INPUT_TOK_EOF) {
                inval_input_struct("unexpected eof token occured, expected `text` or `eq_splitter` token"), set_status(1, 1);
            } else if (tok->type == INPUT_TOK_TEXT) {
                input->res = input_tok_clone(tok), set_status(0, 1);
                input->status = INPUT_IN_RES;
            } else if (tok->type == INPUT_TOK_EQ_SPLITTER) {
                input->eq_splitter = input_tok_clone(tok), set_status(0, 1);
                input->status = INPUT_FULL;
            } else {
                invalid_tok("unexpected token occured, expected `text` or `eq_splitter` or `eof` token"), set_status(1, 0);
            }
        } break;

        case INPUT_IN_RES: {
            if (tok->type == INPUT_TOK_EOF) {
                set_status(1, 1);
            } else if (tok->type == INPUT_TOK_TEXT) {
                input->res = input_tok_clone(tok), set_status(0, 1);
            } else if (tok->type == INPUT_TOK_EQ_SPLITTER) {
                input->eq_splitter = input_tok_clone(tok), set_status(0, 1);
                input->status = INPUT_FULL;
            } else {
                invalid_tok("unexpected token occured, expected `eq_splitter` or `eof` token"), set_status(1, 0);
            }
        } break;

        case INPUT_FULL: {
            set_status(1, 1);
        } break;

        default: inval_input_struct("invalid structure of `input`, {in: %p, minus_splitter: %p, res: %p, eq_splitter: %p}",
                                    input->in, input->minus_splitter, input->res, input->eq_splitter), set_status(1, 0);
    }
}

#undef set_status
#undef inval_input_struct
#undef null_token
#undef invalid_tok

#endif // _GARAGE_INPUT_PIPE_C

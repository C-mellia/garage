#ifndef _INPUT_TOK_PIPE_C
#define _INPUT_TOK_PIPE_C 1

static inline __attribute__((unused))
void __input_tok_pipe_init(InputTokPipe input_tok_pipe, int fd);
static inline __attribute__((unused))
ssize_t input_tok_stream_next(Deque deq, Stream input_tok_stream, InputTokPipeData data);
static inline __attribute__((unused))
void input_tok_stack_concat_text(Deque stack);
static inline __attribute__((unused))
void input_tok_add_ch(InputTok input_tok, PipeStatus3 status, Array errs, void *peek);

extern const char *const __input_tok_type_str[__INPUT_TOK_COUNT];

static void __input_tok_pipe_init(InputTokPipe input_tok_pipe, int fd) {
    InputTokPipeData data = (void *)input_tok_pipe->data;
    memset(data, 0, sizeof *data);
    Coord coord = (void *)data->coord;
    Deque stack = (void *)data->stack;

    Stream tok_stream = (void *)input_tok_pipe->tok_stream, fd_stream = (void *)input_tok_pipe->fd_stream;
    memset(coord, 0, sizeof *coord), deq_init(stack, sizeof(InputTok));
    stream_init(fd_stream, ENGINE_FILE_DESCRIPTOR, fd);
    stream_init(tok_stream, ENGINE_NESTED_STREAM,
                fd_stream, (void *)input_tok_stream_next,
                data, (void *)input_tok_drop,
                sizeof(InputTok));
}

static ssize_t input_tok_stream_next(Deque deq, Stream fd_stream, InputTokPipeData data) {
    Coord coord = (void *)data->coord;
    Deque stack = (void *)data->stack;
    struct pipe_status3 __status = {
        .should_stop = 0, .should_consume = 0, .should_stash = 0,
    }, *status = &__status;

    ssize_t prev_len = deq_len(deq);

    Array Cleanup(arr_drop) errs = arr_new(sizeof(String));
    InputTok tok = input_tok_new(INPUT_TOK_NONE, coord->ln, coord->col);

    for (; !status->should_stop;) {
        void *peek = stream_peek(fd_stream, 0);
        input_tok_add_ch(tok, status, errs, peek);

        if (status->should_consume) {
            if (ch_is(peek, "\n", 1)) {
                ++coord->ln, coord->col = 1;
            } else {
                ++coord->col;
            }
            if (arr_push_back((void *)tok->arr, peek), stream_consume(fd_stream, 1)) break;;
        }

        if (status->should_stash) {
            deq_push_back(stack, &tok);
            tok = input_tok_new(INPUT_TOK_NONE, coord->ln, coord->col);
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

static void input_tok_add_ch(InputTok tok, PipeStatus3 status, Array errs, void *peek) {
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

    // do {
    //     String Cleanup(string_drop) string = string_new();
    //     string_fmt(string, "{type: '%s'(%d), ln: %zu, col: %zu, arr: ",
    //                get_type_str, tok->type, tok->ln, tok->col);
    //     string_fmt_func(string, (void *)arr_deb_dprint, (void *)tok->arr);
    //     string_fmt(string, ", peek: "), string_fmt_func(string, (void *)ch_deb_dprint, peek);
    //     string_fmt(string, "}");
    //     string_dprint(__logfd, string), dprintf(__logfd, "\n");
    // } while(0);

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

#undef get_type_str
#undef ch_is_text
#undef set_status
#undef not_implemented

#endif // _INPUT_TOK_PIPE_C

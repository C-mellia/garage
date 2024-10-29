#ifndef _INPUT_TOK_C
#define _INPUT_TOK_C 1

typedef struct input_tok_stream_data {
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
} *InputTokStreamData;

static inline __attribute__((unused))
void __input_tok_init(InputTok input_tok, InputTokType type, size_t ln, size_t col);
static inline __attribute__((unused))
InputTok input_tok_stream_next(struct stream *ch_stream, InputTokStreamData data);
static inline __attribute__((unused))
int input_tok_should_end(InputTok input_tok);
static inline __attribute__((unused))
void input_tok_stream_add_ch(InputTok input_tok, Stream ch_stream, Array stack, StreamStatus3 status, Array errs, char *peek);
static inline __attribute__((unused))
int ch_stream_cmp(Stream stream, const char *splitter, int len);

static const char eq_splitter[sizeof EQ_SPLITTER] = EQ_SPLITTER;
static const char minus_splitter[sizeof MINUS_SPLITTER] = MINUS_SPLITTER;
extern const char *const __input_tok_type_str[__INPUT_TOK_COUNT];

static void __input_tok_init(InputTok input_tok, InputTokType type, size_t ln, size_t col) {
    input_tok->type = type, input_tok->ln = ln, input_tok->col = col;
    arr_init((void *)input_tok->arr, 1);
}

static InputTok input_tok_stream_next(Stream ch_stream, InputTokStreamData data) {
    nul_check(Stream, ch_stream);
    Coord coord = (void *)data->coord;
    Array stack = (void *)data->stack;

    struct stream_status3 __status = {
        .should_stop = 0, .should_peek = 0, .should_consume = 0,
    }, *status = &__status;

    Array Cleanup(arr_drop) errs = arr_new(sizeof (struct string));
    InputTok input_tok = input_tok_new(INPUT_TOK_NONE, coord->ln, coord->col);
    Array arr = (void *)input_tok->arr;

    for (; !status->should_stop;) {
        void *peek = stream_peek(ch_stream, status->should_peek);
        input_tok_stream_add_ch(input_tok, ch_stream, stack, status, errs, peek);

        for (int i = 0; i < status->should_consume; ++i) {
            peek = stream_peek(ch_stream, i);
            if (ch_is(peek, "\n", 1)) {
                ++coord->ln, coord->col = 1;
            } else {
                ++coord->col;
            }
            arr_push_back(arr, peek);
        }
        stream_consume(ch_stream, status->should_consume);
    }

    if (arr_len(errs)) {
        for (size_t i = 0; i < arr_len(errs); ++i) {
            String Cleanup(string_drop) string = deref(String, arr_get(errs, i));
            string_print(string), printf("\n");
        }
        input_tok_drop(&input_tok), panic("input_tokenization failed with %lu errors\n", arr_len(errs));
    } else {
        return input_tok;
    }
}

static int input_tok_should_end(InputTok input_tok) {
    return input_tok->type == INPUT_TOK_EOF;
}

/**
* + `Array stack`: use an additional stack to solve the ambiguity in the syntax
*
* TODO: integrate the stack features with the current tokenization subroutine
*/
static void input_tok_stream_add_ch(InputTok input_tok, Stream ch_stream, Array stack, StreamStatus3 status, Array errs, char *peek) {
#define not_implemented(FMT, ...) ({\
    String msg_string = string_new();\
    string_fmt(msg_string, "%s:%d:%s: Not implemented: " FMT ": with ch: ", \
               __FILE__, __LINE__, __func__, ##__VA_ARGS__);\
    string_fmt_func(msg_string, (void *)ch_deb_dprint, peek);\
    arr_push_back(errs, &msg_string);\
})

#define invalid_char(FMT, ...) ({\
    String msg_string = string_new();\
    string_fmt(msg_string, "%s:%d:%s: Invalid Character: " FMT ": with ch: ",\
               __FILE__, __LINE__, __func__, ##__VA_ARGS__);\
    string_fmt_func(msg_string, (void *)ch_deb_dprint, peek);\
    arr_push_back(errs, &msg_string);\
})

    // alphabetics, digits, punctuators, whitespaces
#define ch_is_text(peek) (ch_is_alpha(peek) || ch_is_num(peek) || ch_is_pun(peek) || ch_is_ws(peek))

#define get_type_str (input_tok->type >= 0 && input_tok->type < __INPUT_TOK_COUNT? __input_tok_type_str[input_tok->type]: "Invalid InputInputToken Type")

    Array arr = (void *)input_tok->arr;
    // do {
    //     String Cleanup(string_drop) fmt_string = string_new();
    //     string_fmt(fmt_string, "input_tok_type: '%s'(%d), should_peek: %d, should_consume: %d, peek: ",
    //                get_type_str, input_tok->type, status->should_peek, status->should_consume);
    //     string_fmt_func(fmt_string, (void *)ch_deb_dprint, peek);
    //     string_fmt(fmt_string, "\n");
    //     string_dprint(__logfd, fmt_string);
    // } while(0);

    switch(input_tok->type) {
        case INPUT_TOK_NONE: {
            if (!peek) {
                input_tok->type = INPUT_TOK_EOF, status->should_consume = 0;
            } else if (ch_is(peek, "\n", 1)) {
                input_tok->type = INPUT_TOK_LF, status->should_consume = 0;
            } else if (ch_is_text(peek)) {
                input_tok->type = INPUT_TOK_TEXT, status->should_consume = 0;
            } else {
                invalid_char("Unhandled character occured at the beginning of input_tokenization"), status->should_stop = 1, status->should_consume = 0;
            }
        } break;

        case INPUT_TOK_EOF: {
            status->should_stop = 1, status->should_consume = status->should_peek, status->should_peek = 0;
        } break;

        case INPUT_TOK_TEXT: {
            if (!peek) {
                status->should_stop = 1, status->should_consume = status->should_peek, status->should_peek = 0;
            } else if (!status->should_peek && ch_is(peek, "\n", 1)) {
                status->should_peek = 1, status->should_consume = 0;
            } else if (status->should_peek && !ch_stream_cmp(ch_stream, EQ_SPLITTER, status->should_peek)) {
                if ((size_t)status->should_peek + 1 < sizeof EQ_SPLITTER) {
                    ++status->should_peek, status->should_consume = 0;
                } else if (!arr_len(arr)) {
                    status->should_consume = status->should_peek, status->should_peek = 0, input_tok->type = INPUT_TOK_EQ_SPLIT;
                } else {
                    status->should_stop = 1, status->should_consume = 0;
                }
            } else if (status->should_peek && !ch_stream_cmp(ch_stream, MINUS_SPLITTER, status->should_peek)) {
                if ((size_t)status->should_peek + 1 < sizeof MINUS_SPLITTER) {
                    ++status->should_peek, status->should_consume = 0;
                } else if (!arr_len(arr)) {
                    status->should_consume = status->should_peek, status->should_peek = 0, input_tok->type = INPUT_TOK_MINUS_SPLIT;
                } else {
                    status->should_stop = 1, status->should_consume = 0;
                }
            } else if (ch_is_text(peek)) {
                status->should_consume = 1, status->should_peek = 0;
            } else if (status->should_peek) {
                status->should_consume = 1, --status->should_peek;
            } else {
                invalid_char("Unhandled character occured in text input_tokenization"), status->should_stop = 1, status->should_consume = 0;
            }
        } break;

        case INPUT_TOK_LF: {
            if (ch_is(peek, "\n", 1)) {
                status->should_peek = 0, status->should_consume = 1;
            } else {
                status->should_stop = 1, status->should_consume = 0;
            }
        } break;

        case INPUT_TOK_EQ_SPLIT: {
            if (!peek) {
                status->should_stop = 1, status->should_consume = 0;
            } else if (ch_is(peek, "\n", 1)) {
                status->should_stop = 1, status->should_consume = 0;
            } else if (ch_is_text(peek)) {
                status->should_consume = 1;
            } else {
                invalid_char("Unhandled character occured in eq_split input_tokenization"), status->should_stop = 1, status->should_consume = 0;
            }
        } break;

        case INPUT_TOK_MINUS_SPLIT: {
            if (!peek) {
                status->should_stop = 1, status->should_consume = 0;
            } else if (ch_is(peek, "\n", 1)) {
                status->should_stop = 1, status->should_consume = 0;
            } else if (ch_is_text(peek)) {
                status->should_consume = 1;
            } else {
                invalid_char("Unhandled character occured in minus_split input_tokenization"), status->should_stop = 1, status->should_consume = 0;
            }
        } break;

        default: not_implemented("invalid input_tok type"), status->should_stop = 1, status->should_consume = 0;
    }
}

#undef get_type_str
#undef ch_is_text
#undef invalid_char
#undef not_implemented

static int ch_stream_cmp(Stream stream, const char *splitter, int len) {
    for (int i = 0; i < len; ++i) {
        void *peek = stream_peek(stream, i);
        int cmp = memcmp(peek, splitter + i, 1);
        if (cmp) return cmp;
    }
    return 0;
}

#endif // _INPUT_TOK_C

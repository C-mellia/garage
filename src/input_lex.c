#include <stdlib.h>
#include <string.h>

#include <garage/input_lex.h>
#include <garage/ascii.h>
#include <garage/log.h>
#include <garage/string.h>
#include <garage/slice.h>
#include <garage/scanner.h>

static inline void __input_lexer_init(InputLexer input_lexer, void *mem, size_t align, size_t len);

void input_lexer_init(InputLexer input_lexer, void *mem, size_t align, size_t len) {
    nul_check(InputLexer, input_lexer);
    __input_lexer_init(input_lexer, mem, align, len);
}

InputLexer input_lexer_new(void *mem, size_t align, size_t len) {
    InputLexer input_lexer = malloc(sizeof *input_lexer);
    alloc_check(malloc, input_lexer, sizeof *input_lexer);
    return __input_lexer_init(input_lexer, mem, align, len), input_lexer;
}

void input_lexer_cleanup(InputLexer input_lexer) {
    Scanner scnr = (void *)input_lexer->scnr;
    scnr_cleanup(scnr);
}

void input_lexer_drop(InputLexer *input_lexer) {
    if (input_lexer && *input_lexer) input_lexer_cleanup(*input_lexer), free(*input_lexer), *input_lexer = 0;
}

int input_lexer_deb_dprint(int fd, InputLexer input_lexer) {
    if (!input_lexer) return dprintf(fd, "(nil)");
    Scanner scnr = (void *)input_lexer->scnr;
    String Cleanup(string_drop) string = string_new();
    string_fmt(string, "{ln: %zu, col: %zu, scnr: ", input_lexer->ln, input_lexer->col);
    string_fmt_func(string, (void *)scnr_deb_dprint, scnr);
    string_fmt(string, "}");
    return string_dprint(fd, string);
}

int input_lexer_deb_print(InputLexer input_lexer) {
    return input_lexer_deb_dprint(1, input_lexer);
}

void *input_lexer_peek(InputLexer input_lexer, size_t off) {
    nul_check(InputLexer, input_lexer);
    Slice slice = (void *)input_lexer->src;
    return slice_get(slice, input_lexer->_pos + off);
}

size_t input_lexer_consume(InputLexer input_lexer, size_t count) {
    nul_check(InputLexer, input_lexer);
    Slice slice = (void *)input_lexer->src;
    for (;; --count, ++input_lexer->_pos) {
        void *peek = slice_get(slice, input_lexer->_pos);
        if (!peek || !count) {
            return count;
        } else if (ch_is(peek, "\n", 1)) {
            ++input_lexer->ln, input_lexer->col = 1;
        } else {
            ++input_lexer->col;
        }
    }
}

InputTok input_lexer_produce(InputLexer input_lexer) {
#define not_implemented(FMT, ...) ({\
    String string = string_new();\
    string_fmt(string, "%s:%d:%s: Not implemented: " FMT ": with ch: ", \
               __FILE__, __LINE__, __func__, ##__VA_ARGS__);\
    string_fmt_func(string, (void *)ch_deb_dprint, peek);\
})
#define invalid_char(FMT, ...) ({\
    String string = string_new();\
    string_fmt(string, "%s:%d:%s: Invalid Character: " FMT ": with ch: ",\
               __FILE__, __LINE__, __func__, ##__VA_ARGS__);\
    string_fmt_func(string, (void *)ch_deb_dprint, peek);\
})
    // alphabetics, digits, punctuators, whitespaces
#define ch_is_text(peek) (ch_is_alpha(peek) || ch_is_num(peek) || ch_is_pun(peek) || ch_is_ws(peek))

    nul_check(InputLexer, input_lexer);
    InputTok input_tok = input_tok_new(TOK_NONE, input_lexer->ln, input_lexer->col, input_lexer_peek(input_lexer, 0), 0);
    Slice slice = (void *)input_tok->slice;
    int should_stop = 0, should_consume = 0, should_peek = 0;

    for (; !should_stop;) {
        void *front = input_lexer_peek(input_lexer, 0), *peek = input_lexer_peek(input_lexer, should_peek);
        switch(input_tok->type) {
            case TOK_NONE: {
                if (!peek) {
                    input_tok->type = TOK_EOF, should_consume = 0;
                } else if (ch_is_text(peek)) {
                    input_tok->type = TOK_TEXT, should_consume = 0;
                } else {
                    invalid_char("Unhandled character occured at the beginning of input_tokenization"), should_stop = 1, should_consume = 0;
                }
            } break;

            case TOK_EOF: {
                should_stop = 1, should_consume = should_peek, should_peek = 0;
            } break;

            case TOK_TEXT: {
                // ch_deb_print(peek), printf("\n");
                // printf("%d: %.*s\n", should_peek, should_peek, (char *)front);
                if (!peek) {
                    should_stop = 1, should_consume = should_peek, should_peek = 0;
                } else if (!should_peek && ch_is(peek, "\n", 1)) {
                    ++should_peek, should_consume = 0;
                } else if (should_peek && !memcmp(front, eq_splitter, should_peek)) {
                    if ((size_t)should_peek + 1 < sizeof eq_splitter) {
                        // x: input_lexer->pos, ^: peek
                        //
                        // should_peek = 1:
                        // '\n', ?
                        //   x   ^
                        //
                        // should_peek = 2:
                        // '\n', '=', ?
                        //   x        ^
                        //
                        // ...
                        //
                        // should_peek = sizeof eq_splitter:
                        // '\n', '=', '=', '=', (don't care)
                        //   x                   ^
                        ++should_peek, should_consume = 0;
                    } else if (!slice->len) {
                        should_consume = should_peek, should_peek = 0, input_tok->type = TOK_EQ_SPLIT;
                    } else {
                        should_stop = 1, should_consume = 0;
                    }
                } else if (should_peek && !memcmp(front, minus_splitter, should_peek)) {
                    if ((size_t)should_peek + 1 < sizeof minus_splitter) {
                        ++should_peek, should_consume = 0;
                    } else if (!slice->len) {
                        should_consume = should_peek, should_peek = 0, input_tok->type = TOK_MINUS_SPLIT;
                    } else {
                        should_stop = 1, should_consume = 0;
                    }
                } else if (ch_is_text(peek)) {
                    should_consume = 1, should_peek = 0;
                } else if (should_peek) {
                    should_consume = 1, --should_peek;
                } else {
                    invalid_char("Unhandled character occured in text input_tokenization"), should_stop = 1, should_consume = 0;
                }
            } break;

            case TOK_EQ_SPLIT: {
                if (!peek) {
                    should_stop = 1, should_consume = 0;
                } else if (ch_is(peek, "\n", 1)) {
                    should_stop = 1, should_consume = 1;
                } else if (ch_is_text(peek)) {
                    should_consume = 1;
                } else {
                    invalid_char("Unhandled character occured in eq_split input_tokenization"), should_stop = 1, should_consume = 0;
                }
            } break;

            case TOK_MINUS_SPLIT: {
                if (!peek) {
                    should_stop = 1, should_consume = 0;
                } else if (ch_is(peek, "\n", 1)) {
                    should_stop = 1, should_consume = 1;
                } else if (ch_is_text(peek)) {
                    should_consume = 1;
                } else {
                    invalid_char("Unhandled character occured in minus_split input_tokenization"), should_stop = 1, should_consume = 0;
                }
            } break;

            default: not_implemented("invalid input_tok type"), should_stop = 1, should_consume = 0;
        }
        input_lexer_consume(input_lexer, should_consume), slice->len += should_consume;
    }
    return input_tok;
}

#undef ch_is_text
#undef not_implemented

static void __input_lexer_init(InputLexer input_lexer, void *mem, size_t align, size_t len) {
    Scanner scnr = (void *)input_lexer->scnr;
    input_lexer->ln = input_lexer->col = 1, scnr_init(scnr, mem, align, len);
}

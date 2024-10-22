#include <stdlib.h>

#include <garage/log.h>
#include <garage/slice.h>
#include <garage/string.h>
#include <garage/input_tok.h>

static inline void __input_tok_init(InputTok input_tok, InputTokType type, size_t ln, size_t col, void *mem, size_t len);

extern const char *const input_tok_type_str[__INPUT_TOK_COUNT];

void input_tok_init(InputTok input_tok, InputTokType type, size_t ln, size_t col, void *mem, size_t len) {
    nul_check(InputTok, input_tok);
    __input_tok_init(input_tok, type, ln, col, mem, len);
}

InputTok input_tok_new(InputTokType type, size_t ln, size_t col, void *mem, size_t len) {
    InputTok input_tok = malloc(sizeof *input_tok);
    alloc_check(malloc, input_tok, sizeof *input_tok);
    return __input_tok_init(input_tok, type, ln, col, mem, len), input_tok;
}

void input_tok_cleanup(InputTok input_tok) {
    (void) input_tok;
}

void *input_tok_drop(InputTok *input_tok) {
    if (input_tok) input_tok_cleanup(*input_tok), free(*input_tok), *input_tok = 0;
    return input_tok;
}

int input_tok_deb_dprint(int fd, InputTok input_tok) {
    #define get_type_str (input_tok_type_str[input_tok->type]? : "Invalid InputToken Type")
    if (!input_tok) return dprintf(fd, "(nil)");
    Slice slice = (void *)input_tok->slice;
    if (input_tok->type == INPUT_TOK_NONE || input_tok->type == INPUT_TOK_EOF) {
        return dprintf(fd, "{type: '%s', ln: %zu, col: %zu, (none)}",
                       get_type_str, input_tok->ln, input_tok->col);
    } else if (input_tok->type == INPUT_TOK_TEXT) {
        String Cleanup(string_drop) string = string_new();
        string_fmt(string, "{type: '%s', ln: %zu, col: %zu, slice: ",
                   get_type_str, input_tok->ln, input_tok->col);
        string_fmt_func(string, (void *)slice_deb_dprint, slice);
        string_fmt(string, "}");
        return string_dprint(fd, string);
    } else if (input_tok->type == INPUT_TOK_EQ_SPLIT) {
        String Cleanup(string_drop) string = string_new();
        string_fmt(string, "{type: '%s', ln: %zu, col: %zu, slice: '%.*s'}",
                   get_type_str, input_tok->ln, input_tok->col, (int)(slice->len - sizeof eq_splitter), (char *)slice_get(slice, sizeof eq_splitter - 1));
        return string_dprint(fd, string);
    } else if (input_tok->type == INPUT_TOK_MINUS_SPLIT) {
        String Cleanup(string_drop) string = string_new();
        string_fmt(string, "{type: '%s', ln: %zu, col: %zu, slice: '%.*s'}",
                   get_type_str, input_tok->ln, input_tok->col, (int)(slice->len - sizeof minus_splitter), (char *)slice_get(slice, sizeof minus_splitter - 1));
        return string_dprint(fd, string);
    } else {
        return dprintf(fd, "(inval)");
    }
    #undef get_type_str
}

int input_tok_deb_print(InputTok input_tok) {
    return input_tok_deb_dprint(1, input_tok);
}

static void __input_tok_init(InputTok input_tok, InputTokType type, size_t ln, size_t col, void *mem, size_t len) {
    Slice slice = (void *)input_tok->slice;
    input_tok->type = type, input_tok->ln = ln, input_tok->col = col, slice_init(slice, mem, 1, len);
}

#include <stdlib.h>
#include <string.h>

#include <garage/prelude.h>
#include <garage/stream.h>
#include <garage/ascii.h>
#include <garage/input_tok.h>

#include "./.input_tok.c"

extern const char *const __input_tok_type_str[__INPUT_TOK_COUNT];

void input_tok_init(InputTok input_tok, InputTokType type, size_t ln, size_t col) {
    nul_check(InputTok, input_tok), __input_tok_init(input_tok, type, ln, col);
}

InputTok input_tok_new(InputTokType type, size_t ln, size_t col) {
    InputTok tok = malloc(sizeof *tok);
    alloc_check(malloc, tok, sizeof *tok);
    return __input_tok_init(tok, type, ln, col), tok;
}

InputTok input_tok_clone(InputTok input_tok) {
    if (!input_tok) return 0;
    InputTok clone = input_tok_new(input_tok->type, input_tok->ln, input_tok->col);
    arr_copy((void *)clone->arr, (void *)input_tok->arr);
    return clone;
}

void input_tok_cleanup(InputTok input_tok) {
    if (!input_tok) return;
    arr_cleanup((void *)input_tok->arr);
}

void *input_tok_drop(InputTok *input_tok) {
    if (input_tok) input_tok_cleanup(*input_tok), free(*input_tok), *input_tok = 0;
    return input_tok;
}

int input_tok_deb_dprint(int fd, InputTok input_tok) {
#define get_type_str (input_tok->type < __INPUT_TOK_COUNT? __input_tok_type_str[input_tok->type]: "Invalid InputToken Type")
    if (!input_tok) return dprintf(fd, "(nil)");
    Array arr = (void *)input_tok->arr;
    if (input_tok->type == INPUT_TOK_NONE || input_tok->type == INPUT_TOK_EOF) {
        return dprintf(fd, "{type: '%s', ln: %zu, col: %zu, (none)}",
                       get_type_str, input_tok->ln, input_tok->col);
    } else if (input_tok->type == INPUT_TOK_TEXT) {
        String Cleanup(string_drop) string = string_new();
        string_fmt(string, "{type: '%s', ln: %zu, col: %zu, string: ",
                   get_type_str, input_tok->ln, input_tok->col);
        string_fmt_func(string, (void *)arr_deb_dprint, arr);
        string_fmt(string, "}");
        return string_dprint(fd, string);
    } else if (input_tok->type == INPUT_TOK_EQ_SPLIT) {
        String Cleanup(string_drop) string = string_new();
        string_fmt(string, "{type: '%s', ln: %zu, col: %zu, string: '%.*s'}",
                   get_type_str, input_tok->ln, input_tok->col, (int)(arr->len - sizeof EQ_SPLITTER), (char *)arr_get(arr, sizeof EQ_SPLITTER - 1));
        return string_dprint(fd, string);
    } else if (input_tok->type == INPUT_TOK_MINUS_SPLIT) {
        String Cleanup(string_drop) string = string_new();
        string_fmt(string, "{type: '%s', ln: %zu, col: %zu, slice: '%.*s'}",
                   get_type_str, input_tok->ln, input_tok->col, (int)(arr->len - sizeof MINUS_SPLITTER), (char *)arr_get(arr, sizeof MINUS_SPLITTER - 1));
        return string_dprint(fd, string);
    } else {
        return dprintf(fd, "(inval)");
    }
}
#undef get_type_str

int input_tok_deb_print(InputTok input_tok) {
    return fflush(stdout), input_tok_deb_dprint(1, input_tok);
}

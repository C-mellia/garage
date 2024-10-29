#include <stdlib.h>
#include <string.h>

#include <garage/prelude.h>

#include <input_tok.h>
#include "./.input_tok.c"

void input_tok_init(InputTok input_tok, InputTokType type, size_t ln, size_t col) {
    nul_check(InputTok, input_tok), __input_tok_init(input_tok, type, ln, col);
}

InputTok input_tok_new(InputTokType type, size_t ln, size_t col) {
    InputTok input_tok = malloc(sizeof *input_tok);
    alloc_check(malloc, input_tok, sizeof *input_tok);
    return __input_tok_init(input_tok, type, ln, col), input_tok;
}

void input_tok_cleanup(InputTok input_tok) {
    arr_cleanup((void *)input_tok->arr);
}

void *input_tok_drop(InputTok *input_tok) {
    if (input_tok) input_tok_cleanup(*input_tok), free(*input_tok), *input_tok = 0;
    return input_tok;
}

int input_tok_deb_dprint(int fd, InputTok input_tok) {
#define get_type_str (input_tok->type < __INPUT_TOK_COUNT? input_tok_type_str[input_tok->type]: "Invalid Input Token Type")
    if (!input_tok) return dprintf(fd, "(nil)");
    String Cleanup(string_drop) string = string_new();
    string_fmt(string, "{type: '%s', ln: %zu, col: %zu, arr: ",
               get_type_str, input_tok->ln, input_tok->col);
    string_fmt_func(string, (void *)arr_deb_dprint, (void *)input_tok->arr);
    string_fmt(string, "}");
    return string_dprint(fd, string);
}
#undef get_type_str

int input_tok_deb_print(InputTok input_tok) {
    return fflush(stdout), input_tok_deb_dprint(1, input_tok);
}

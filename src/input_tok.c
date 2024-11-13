#include <stdlib.h>
#include <string.h>

#include <garage/prelude.h>

#include <garage/input_tok.h>
#include "./.input_tok.c"

void input_tok_init(InputTok input_tok, InputTokType type) {
    nul_check(InputTok, input_tok), __input_tok_init(input_tok, type);
}

InputTok input_tok_new(InputTokType type) {
    InputTok input_tok = malloc(sizeof *input_tok);
    alloc_check(malloc, input_tok, sizeof *input_tok);
    return __input_tok_init(input_tok, type), input_tok;
}

void input_tok_cleanup(InputTok input_tok) {
    if (!input_tok) return;
    arr_cleanup((void *)input_tok->arr);
}

void *input_tok_drop(InputTok *input_tok) {
    if (input_tok) input_tok_cleanup(*input_tok), free(*input_tok), *input_tok = 0;
    return input_tok;
}

InputTok input_tok_clone(InputTok input_tok) {
    nul_check(InputTok, input_tok);
    InputTok clone = input_tok_new(input_tok->type);
    arr_cat((void *)clone->arr, (void *)input_tok->arr);
    return clone;
}

int input_tok_deb_dprint(int fd, InputTok input_tok) {
#define get_type_str (input_tok->type < __INPUT_TOK_COUNT? __input_tok_type_str[input_tok->type]: "Invalid Input Token Type")
    if (!input_tok) return dprintf(fd, "(nil)");
    String Cleanup(string_drop) string = string_new();
    string_fmt(string, "{type: '%s'(%d), arr: ",
               get_type_str, input_tok->type);
    string_fmt_func(string, (void *)arr_deb_dprint, (void *)input_tok->arr);
    string_fmt(string, "}");
    return string_dprint(fd, string);
}
#undef get_type_str

int input_tok_deb_print(InputTok input_tok) {
    return fflush(stdout), input_tok_deb_dprint(1, input_tok);
}

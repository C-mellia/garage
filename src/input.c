#include <stdlib.h>
#include <string.h>

#include <garage/prelude.h>
#include <garage/stream.h>
#include <garage/input.h>

#include "./.input.c"

// extern const char *const __input_tok_type_str[__INPUT_TOK_COUNT];
extern const char *const __input_status_str[__INPUT_STATUS_COUNT];

void input_init(Input input) {
    nul_check(Input, input), __input_init(input);
}

Input input_new(void) {
    Input input = malloc(sizeof *input);
    alloc_check(malloc, input, sizeof *input);
    return __input_init(input), input;
}

void input_cleanup(Input input) {
    if (!input) return;
    input_tok_cleanup(input->in), input_tok_cleanup(input->minus_splitter), input_tok_cleanup(input->res), input_tok_cleanup(input->eq_splitter);
}

void input_drop(Input *input) {
    if (input) input_cleanup(*input), free(*input), *input= 0;
}

int input_deb_dprint(int fd, Input input) {
#define get_type_str (input->status < __INPUT_STATUS_COUNT? __input_status_str[input->status]: "Invalid InputStatus")
    if (!input) return dprintf(fd, "(nil)");
    String Cleanup(string_drop) string = string_new();
    string_fmt(string, "{status: '%s'(%d), in: ", get_type_str, input->status);
    string_fmt_func(string, (void *)input_tok_deb_dprint, input->in);
    string_fmt(string, ", minus_splitter: ");
    string_fmt_func(string, (void *)input_tok_deb_dprint, input->minus_splitter);
    string_fmt(string, ", res: ");
    string_fmt_func(string, (void *)input_tok_deb_dprint, input->res);
    string_fmt(string, ", eq_splitter: ");
    string_fmt_func(string, (void *)input_tok_deb_dprint, input->eq_splitter);
    string_fmt(string, "}");
    return string_dprint(fd, string);
}
#undef get_type_str

int input_deb_print(Input input) {
    return fflush(stdout), input_deb_dprint(1, input);
}

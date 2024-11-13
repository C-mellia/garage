#include <input_tok.h>
#include <input.h>

const char *const __input_tok_type_str[] = {
    [INPUT_TOK_EOF] = "INPUT_TOK_EOF",
    [INPUT_TOK_TEXT] = "INPUT_TOK_TEXT",
    [INPUT_TOK_EQ_SPLITTER] = "INPUT_TOK_EQ_SPLITTER",
    [INPUT_TOK_MINUS_SPLITTER] = "INPUT_TOK_MINUS_SPLITTER",
};

const char *const __input_status_str[] = {
    [INPUT_EMPTY] = "INPUT_EMPTY",
    [INPUT_IN] = "INPUT_IN",
    [INPUT_IN_RES] = "INPUT_IN_RES",
    [INPUT_FULL] = "INPUT_FULL",
};

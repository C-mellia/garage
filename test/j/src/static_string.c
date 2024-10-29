#include <input_tok.h>

const char *const input_tok_type_str[] = {
    [INPUT_TOK_EOF] = "INPUT_TOK_EOF",
    [INPUT_TOK_TEXT] = "INPUT_TOK_TEXT",
    [INPUT_TOK_LF] = "INPUT_TOK_LF",
    [INPUT_TOK_EQ_SPLITTER] = "INPUT_TOK_EQ_SPLITTER",
    [INPUT_TOK_MINUS_SPLITTER] = "INPUT_TOK_MINUS_SPLITTER",
};

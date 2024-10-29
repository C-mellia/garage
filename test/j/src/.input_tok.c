#ifndef _INPUT_TOK_C
#define _INPUT_TOK_C 1

static inline __attribute__((unused))
void __input_tok_init(InputTok input_tok, InputTokType type, size_t ln, size_t col);

extern const char *const input_tok_type_str[__INPUT_TOK_COUNT];

static void __input_tok_init(InputTok input_tok, InputTokType type, size_t ln, size_t col) {
    input_tok->type = type, input_tok->ln = ln, input_tok->col = col;
    arr_init((void *)input_tok->arr, 1);
}

#endif // _INPUT_TOK_C

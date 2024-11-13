#ifndef _GARAGE_INPUT_TOK_C
#define _GARAGE_INPUT_TOK_C 1

static inline __attribute__((unused))
void __input_tok_init(InputTok input_tok, InputTokType type);

extern const char *const __input_tok_type_str[__INPUT_TOK_COUNT];

static void __input_tok_init(InputTok input_tok, InputTokType type) {
    input_tok->type = type;
    arr_init((void *)input_tok->arr, 1);
}

#endif // _GARAGE_INPUT_TOK_C

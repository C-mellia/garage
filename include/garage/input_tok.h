#ifndef GARAGE_INPUT_TOK_H
#define GARAGE_INPUT_TOK_H 1

#include <stddef.h>
#include <garage/garage.h>

#define eq_splitter "\n======"
#define minus_splitter "\n------"

typedef enum input_tok_type {
    INPUT_TOK_NONE=0,
    INPUT_TOK_EOF,
    INPUT_TOK_TEXT,
    INPUT_TOK_EQ_SPLIT,
    INPUT_TOK_MINUS_SPLIT,
    __INPUT_TOK_COUNT,
} InputTokType;

typedef struct input_tok {
    InputTokType type;
    size_t ln, col;

    Phantom slice;

    void *_mem;
    size_t _align, _len;
} *InputTok;

void input_tok_init(InputTok input_tok, InputTokType type, size_t ln, size_t col, void *mem, size_t len);
InputTok input_tok_new(InputTokType type, size_t ln, size_t col, void *mem, size_t len);
void input_tok_cleanup(InputTok input_tok);
void *input_tok_drop(InputTok *input_tok);

int input_tok_deb_dprint(int fd, InputTok input_tok);
int input_tok_deb_print(InputTok input_tok);

#endif // GARAGE_INPUT_TOK_H

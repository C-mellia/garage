#ifndef GARAGE_INPUT_TOK_H
#define GARAGE_INPUT_TOK_H 1

#include <stddef.h>
#include <garage/garage.h>

#define eq_splitter "\n======"
#define minus_splitter "\n------"

typedef enum InputTokType {
    TOK_NONE=0,
    TOK_EOF,
    TOK_TEXT,
    TOK_EQ_SPLIT,
    TOK_MINUS_SPLIT,
    __TOK_COUNT,
} InputTokType;

typedef struct InputTok {
    InputTokType type;
    size_t ln, col;

    Phantom slice;

    void *_mem;
    size_t _align, _len;
} *InputTok;

void input_tok_init(InputTok input_tok, InputTokType type, size_t ln, size_t col, void *mem, size_t len);
InputTok input_tok_new(InputTokType type, size_t ln, size_t col, void *mem, size_t len);
void input_tok_cleanup(InputTok input_tok);
void input_tok_drop(InputTok *input_tok);

int input_tok_deb_dprint(int fd, InputTok input_tok);
int input_tok_deb_print(InputTok input_tok);

#endif // GARAGE_INPUT_TOK_H

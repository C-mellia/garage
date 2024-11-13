#ifndef INPUT_TOK_H
#define INPUT_TOK_H 1

#include <garage/types.h>

typedef enum InputTokType {
    INPUT_TOK_NONE=-1,

    INPUT_TOK_EOF,
    INPUT_TOK_TEXT,

    __INPUT_TOK_EQ_SPLITTER0,
    __INPUT_TOK_EQ_SPLITTER1,
    __INPUT_TOK_EQ_SPLITTER2,
    __INPUT_TOK_EQ_SPLITTER3,
    __INPUT_TOK_EQ_SPLITTER4,
    __INPUT_TOK_EQ_SPLITTER5,
    INPUT_TOK_EQ_SPLITTER,

    __INPUT_TOK_MINUS_SPLITTER0,
    __INPUT_TOK_MINUS_SPLITTER1,
    __INPUT_TOK_MINUS_SPLITTER2,
    __INPUT_TOK_MINUS_SPLITTER3,
    __INPUT_TOK_MINUS_SPLITTER4,
    __INPUT_TOK_MINUS_SPLITTER5,
    INPUT_TOK_MINUS_SPLITTER,
    __INPUT_TOK_COUNT,
} InputTokType;

typedef struct input_tok {
    InputTokType type;
    size_t ln, col;

    Phantom arr;
    struct {
        size_t arr_len;

        Phantom arr_slice;
        struct {
            void *arr_slice_mem;
            size_t arr_slice_align, arr_slice_len;
        };
    };
} *InputTok;

void input_tok_init(InputTok input_tok, InputTokType type, size_t ln, size_t col);
InputTok input_tok_new(InputTokType type, size_t ln, size_t col);
void input_tok_cleanup(InputTok input_tok);
void *input_tok_drop(InputTok *input_tok);
InputTok input_tok_clone(InputTok input_tok);

int input_tok_deb_dprint(int fd, InputTok input_tok);
int input_tok_deb_print(InputTok input_tok);

#endif // INPUT_TOK_H

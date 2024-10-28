#ifndef TOK_H
#define TOK_H 1

#include <stddef.h>
#include <garage/garage.h>

#define EQ_SPLITTER "\n======"
#define MINUS_SPLITTER "\n------"

struct stream;

typedef struct coord {
    size_t ln, col;
} *Coord;

typedef enum input_tok_type {
    INPUT_TOK_NONE=-1,
    INPUT_TOK_EOF,
    INPUT_TOK_TEXT,
    INPUT_TOK_EQ_SPLIT,
    INPUT_TOK_MINUS_SPLIT,
    __INPUT_TOK_COUNT,
} InputTokType;

typedef struct input_tok {
    InputTokType type;
    size_t ln, col;

    Phantom arr;
    struct {
        size_t arr_cap;

        Phantom arr_slice;
        struct {
            void *arr_slice_mem;
            size_t arr_slice_align, arr_slice_len;
        };
    };
} *InputTok;

void input_tok_init(InputTok input_tok, InputTokType type, size_t ln, size_t col);
InputTok input_tok_new(InputTokType type, size_t ln, size_t col);
InputTok input_tok_clone(InputTok input_tok);
void input_tok_cleanup(InputTok input_tok);
void *input_tok_drop(InputTok *input_tok);

int input_tok_deb_dprint(int fd, InputTok input_tok);
int input_tok_deb_print(InputTok input_tok);

#endif // INPUT_TOK_H

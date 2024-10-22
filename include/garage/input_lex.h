#ifndef GARAGE_INPUT_LEX_H
#define GARAGE_INPUT_LEX_H 1

#include <garage/garage.h>
#include <garage/input_tok.h>

typedef struct input_lexer {
    size_t ln, col;

    Phantom scnr;

    size_t _pos;

    Phantom /* Array<String> */errs;
    void *_errs_mem;
    size_t _errs_len, _errs_cap, _errs_align;

    Phantom /* Slice<char> */src;
    void *_src_mem;
    size_t _src_align, _src_len;
} *InputLexer;

void input_lexer_init(InputLexer input_lexer, void *mem, size_t align, size_t len);
InputLexer input_lexer_new(void *mem, size_t align, size_t len);
void input_lexer_cleanup(InputLexer input_lexer);
void *input_lexer_drop(InputLexer *input_lexer);

int input_lexer_deb_dprint(int fd, InputLexer input_lexer);
int input_lexer_deb_print(InputLexer input_lexer);

void *input_lexer_peek(InputLexer input_lexer, size_t off);
size_t input_lexer_consume(InputLexer input_lexer, size_t count);
InputTok input_lexer_produce(InputLexer input_lexer);

#endif // GARAGE_INPUT_LEX_H

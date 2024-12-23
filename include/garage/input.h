#ifndef GARAGE_INPUT_H
#define GARAGE_INPUT_H 1

#include <garage/input_tok.h>

typedef enum InputStatus {
    INPUT_EMPTY, // (nil)
    INPUT_IN, // in
    __INPUT_IN_RES, // in, minus_split
    INPUT_IN_RES, // in, minus_split, res
    INPUT_FULL, // in, (minus_split), (res), eq_splitter
    __INPUT_COUNT,
} InputStatus;

typedef struct input {
    InputStatus status;
    // reference(or reference to a clone of the token) because the tokens can
    // be null, and will be null for some cases
    InputTok in, minus_splitter, res, eq_splitter;
} *Input;

void input_init(Input input, InputStatus status);
Input input_new(InputStatus status);
void input_cleanup(Input input);
void *input_drop(Input *input);

int input_deb_dprint(int fd, Input input);
int input_deb_print(Input input);

#endif // GARAGE_INPUT_H

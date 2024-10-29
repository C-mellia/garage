#ifndef INPUT_H
#define INPUT_H 1

#include <garage/types.h>
#include <stddef.h>
#include <garage/input_tok.h>
#include <garage/input_stream.h>

typedef enum InputStatus {
    INPUT_EMPTY, // (nil)
    INPUT_IN, // in
    INPUT_EQ, // eq
    __INPUT_IN_MINUS, // in, minus
    INPUT_IN_RES, // in, minus, res
    INPUT_IN_EQ, // in, eq
    INPUT_FULL, // in, minus, res, eq
    __INPUT_STATUS_COUNT,
} InputStatus;

typedef struct input {
    InputStatus status;
    InputTok in, minus_splitter, res, eq_splitter;
} *Input;

void input_init(Input input);
Input input_new(void);
void input_cleanup(Input input);
void input_drop(Input *input);

int input_deb_dprint(int fd, Input input);
int input_deb_print(Input input);

#endif // INPUT_H

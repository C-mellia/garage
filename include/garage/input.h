#ifndef INPUT_H
#define INPUT_H 1

#include <garage/types.h>
#include <stddef.h>
#include <garage/input_tok.h>
#include <garage/input_stream.h>

typedef enum InputStatus {
    INPUT_EMPTY, // (nil)
    INPUT_IN, // in
    INPUT_IN_RES, // in, minus_split, res
    INPUT_FULL, // in, minus_split, res, eq_splitter
    __INPUT_COUNT,
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

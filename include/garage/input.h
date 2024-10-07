#ifndef GARAGE_INPUT_H
#define GARAGE_INPUT_H 1

#include <garage/garage.h>

// A text file format to seperate different test cases and expected output
// respectively
//
// `======` is used to seperate test cases
// `------` is used to seperate input and expected output
// splitters should be at the beginning of the line, and any character appeared
// before the end of line is ommited
//
// e.g.
//
// ```
// Foo bar
// ------
// Foo bar
// aosijdoi
// ======[test 0]
// Foo bar
// ======[test 1]
// ```
//
// for two test cases, the first one will have an input of
//
// ```
// Foo bar
// ```
//
// and an expected output of
//
// ```
// Foo bar
// aosijdoi
// ```
//
// and the second one will have an input of
//
// ```
// Foo bar
// ```
//
// however no expected output

struct slice;
struct array;

typedef struct test {
    Phantom input;

    void *_input_mem;
    size_t _input_align, _input_len;

    Phantom res;

    void *_res_mem;
    size_t _res_align, _res_len;
} *Test;

void test_init(Test test, size_t align);
Test test_new(size_t align);
void test_cleanup(Test test);
void test_drop(Test *test);

int test_deb_dprint(int fd, Test test);
int test_deb_print(Test test);
void tests_from_file(struct slice *file, struct array /* Test */*tests);

#endif // GARAGE_INPUT_H

#include <string.h>
#include <stdlib.h>

#include <garage/input.h>
#include <garage/string.h>
#include <garage/slice.h>
#include <garage/log.h>
#include <garage/scanner.h>
#include <garage/ascii.h>
#include <garage/input_lex.h>

static inline void __test_init(Test test, size_t align);

void test_init(Test test, size_t align) {
    nul_check(Test, test), __test_init(test, align);
}

Test test_new(size_t align) {
    Test test = malloc(sizeof *test);
    alloc_check(malloc, test, sizeof *test);
    return __test_init(test, align), test;
}

void test_cleanup(Test test) {
    if (!test) return;
    Slice input = (void *)test->input, res = (void *)test->res;
    slice_cleanup(input), slice_cleanup(res);
}

void test_drop(Test *test) {
    if (test && *test) test_cleanup(*test), free(*test), *test = 0;
}

int test_deb_dprint(int fd, Test test) {
    if (!test) return dprintf(fd, "(nil)");
    Slice input = (void *)test->input, res = (void *)test->res;
    String Cleanup(string_drop) string = string_new();
    string_fmt(string, "{input: ");
    string_fmt_func(string, (void *)slice_deb_dprint, input);
    string_fmt(string, ", res: ");
    string_fmt_func(string, (void *)slice_deb_dprint, res);
    string_fmt(string, "}");
    return string_dprint(fd, string);
}

int test_deb_print(Test test) {
    return test_deb_dprint(1, test);
}

void tests_from_file(Slice file, Array /* Test */tests) {
    nul_check(Array, tests), nul_check(Slice, file);
    InputLexer Cleanup(input_lexer_drop) input_lexer = input_lexer_new(file->mem, file->align, file->len);
}

static void __test_init(Test test, size_t align) {
    Slice input = (void *)test->input, res = (void *)test->res;
    slice_init(input, 0, align, 0), slice_init(res, 0, align, 0);
}

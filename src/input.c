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
    Array errs = (void *)input_lexer->errs;
    Test Cleanup(test_drop) test = test_new(file->align);
    int should_stop = 0, should_be_res = 0, should_push_back = 0;

    for (; !should_stop;) {
        InputTok Cleanup(input_tok_drop) input_tok = input_lexer_produce(input_lexer);
        // input_tok_deb_print(input_tok), printf("\n");
        Slice input = (void *)test->input, res = (void *)test->res, slice = (void *)input_tok->slice;

        if (arr_len(errs)) return;
        switch(input_tok->type) {
            case INPUT_TOK_EOF: {
                should_stop = 1;
                if (input->mem) should_push_back = 1;
            } break;
            case INPUT_TOK_TEXT: {
                should_be_res
                    ? should_be_res = 0, slice_copy(res, slice)
                    : slice_copy(input, slice);
            } break;
            case INPUT_TOK_EQ_SPLIT: {
                if (input->mem) should_push_back = 1;
            } break;
            case INPUT_TOK_MINUS_SPLIT: {
                should_be_res = 1;
            } break;
            default: return;
        }

        if (should_push_back) {
            arr_push_back(tests, &test), test = test_new(file->align);
            should_push_back = 0, should_be_res = 0;
        }
    }
}

static void __test_init(Test test, size_t align) {
    Slice input = (void *)test->input, res = (void *)test->res;
    slice_init(input, 0, align, 0), slice_init(res, 0, align, 0);
}

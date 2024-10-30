#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

#include <unistd.h>
#include <fcntl.h>

#include <garage/prelude.h>
#include <garage/stream.h>
#include <garage/engine.h>
#include <garage/deque.h>

#include "./.engine.c"

#define __BUF_LEN 8

extern const char *const __engine_type_str[__ENGINE_COUNT];

void engine_init(Engine engine, EngineType type, ...) {
    nul_check(Engine, engine);
    va_list Cleanup(va_list_drop) args = {0};
    va_start(args, type), __engine_vinit(engine, type, args);
}

Engine engine_new(EngineType type, ...) {
    va_list Cleanup(va_list_drop) args = {0};
    va_start(args, type);
    Engine engine = malloc(sizeof *engine);
    alloc_check(malloc, engine, sizeof *engine);
    return __engine_vinit(engine, type, args), engine;
}

void engine_cleanup(Engine engine) {
#define invalid_type(FMT, ...) ({\
    const char *type_str = engine->type < __ENGINE_COUNT? __engine_type_str[engine->type]: "INVALID_ENGINE_TYPE";\
    panic("Engine Type: '%s'(%d) is not valid" FMT, type_str, engine->type, ##__VA_ARGS__);\
})
    if (!engine) return;
    switch(engine->type) {
        case ENGINE_FILE_DESCRIPTOR: {
            FdHolder fd_holder = engine->data;
            fd_drop(&fd_holder->fd);
        } break;

        case ENGINE_NESTED_STREAM: {
            free(engine->data), engine->data = 0;
        } break;

        case ENGINE_RANGE: {
            free(engine->data), engine->data = 0;
        } break;

        case ENGINE_FUNCTIONAL: {
            free(engine->data), engine->data = 0;
        } break;

        default: invalid_type("");
    }
}
#undef invalid_type

void *engine_drop(Engine *engine) {
    if (engine) engine_cleanup(*engine), free(*engine), *engine = 0;
    return engine;
}

int engine_deb_dprint(int fd, Engine engine) {
    if (!engine) return dprintf(fd, "(nil)");
    String Cleanup(string_drop) string = string_new();
    const char *const type_str = engine->type < __ENGINE_COUNT? __engine_type_str[engine->type]: "INVALID_ENGINE_TYPE";
    string_fmt(string, "{type: '%s'(%d), align: %zu, data: ", type_str, engine->type, engine->align);
    engine_data_fmt(string, engine->type, engine->data);
    string_fmt(string, "}");
    return string_dprint(fd, string);
}

int engine_deb_print(Engine engine) {
    return fflush(stdout), engine_deb_dprint(1, engine);
}

ssize_t engine_next(struct deque *deq, Engine engine) {
    nul_check(Engine, engine);
    switch(engine->type) {
        case ENGINE_FILE_DESCRIPTOR: {
            uint8_t buf[__BUF_LEN];
            int len = fd_produce(engine->data, buf, sizeof buf);
            deq_reserve(deq, deq->len + len);
            for (int i = 0; i < len; ++i) deq_push_back(deq, buf + i);
            return len;
        } break;

        case ENGINE_NESTED_STREAM: {
            StreamHolder stream_holder = engine->data;
            return stream_holder->stream_next(deq, stream_holder->stream, stream_holder->data);
        } break;

        case ENGINE_RANGE: {
            ssize_t buf[__BUF_LEN];
            int len = range_produce(engine->data, buf, sizeof buf);
            for (int i = 0; i < len; ++i) deq_push_back(deq, buf + i);
            return len;
        } break;

        case ENGINE_FUNCTIONAL: {
            FuncHolder func_holder = engine->data;
            return func_holder->func_next(deq, func_holder->data);
        } break;

        default: return -1;
    }
}

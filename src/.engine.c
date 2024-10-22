#ifndef _GARAGE_ENGINE_C
#define _GARAGE_ENGINE_C 1

static inline __attribute__((unused))
void __engine_vinit(Engine engine, EngineType type, va_list args);
static inline __attribute__((unused))
void va_list_drop(va_list *list);
static inline __attribute__((unused))
int fd_produce(FdHolder fd_holder, void *buf, size_t len);
static inline __attribute__((unused))
int range_produce(RangeHolder range_holder, ssize_t *buf, size_t len);
static inline __attribute__((unused))
int stream_produce(StreamHolder stream_holder, void *buf, size_t len, size_t align);
static inline __attribute__((unused))
int func_produce(FuncHolder func_holder, void *buf, size_t len, size_t align);
static inline __attribute__((unused))
void engine_data_fmt(String string, EngineType type, void *engine_data);
static inline __attribute__((unused))
int should_end_at_null(void *item);

extern const char *const engine_type_str[__ENGINE_COUNT];

static inline void __engine_vinit(Engine engine, EngineType type, va_list args) {
#define not_implemented(FMT, ...) ({\
    const char *type_str = type < __ENGINE_COUNT? engine_type_str[type]: "INVALID_ENGINE_TYPE";\
    panic("Engine Type: '%s'(%d) Not Implemented: " FMT, type_str, type, ##__VA_ARGS__);\
})
    #define holder_get_field(holder, field) .field = va_arg(args, typeof(holder->field))
    #define assign_arg(var) ({ var = va_arg(args, typeof(var)); })

    engine->type = type;
    switch(type) {
        case ENGINE_FILE_DESCRIPTOR: {
            FdHolder fd_holder = malloc(sizeof *fd_holder);
            alloc_check(malloc, fd_holder, sizeof *fd_holder);
            *fd_holder = (struct fd_holder) {
                holder_get_field(fd_holder, fd),
            };
            engine->produce = (void *)fd_produce, engine->align = 1, engine->data = fd_holder;
        } break;

        case ENGINE_NESTED_STREAM: {
            StreamHolder stream_holder = malloc(sizeof *stream_holder);
            alloc_check(malloc, stream_holder, sizeof *stream_holder);
            *stream_holder = (struct stream_holder) {
                holder_get_field(stream_holder, stream),
                holder_get_field(stream_holder, stream_next),
                holder_get_field(stream_holder, should_end),
                .done = 0,
                holder_get_field(stream_holder, data),
            };
            engine->produce = (void *)stream_produce, engine->data = stream_holder;
            assign_arg(engine->drop), assign_arg(engine->align);
        } break;

        case ENGINE_RANGE: {
            RangeHolder range_holder = malloc(sizeof *range_holder);
            alloc_check(malloc, range_holder, sizeof *range_holder);
            *range_holder = (struct range_holder) {
                holder_get_field(range_holder, step),
                holder_get_field(range_holder, begin),
                holder_get_field(range_holder, end),
            };
            engine->produce = (void *)range_produce, engine->align = sizeof range_holder->begin, engine->data = range_holder;
        } break;

        case ENGINE_FUNCTIONAL: {
            FuncHolder func_holder = malloc(sizeof *func_holder);
            alloc_check(malloc, func_holder, sizeof *func_holder);
            *func_holder = (struct func_holder) {
                holder_get_field(func_holder, func_next),
                holder_get_field(func_holder, should_end),
                .done = 0,
                holder_get_field(func_holder, data),
            };
            engine->produce = (void *)func_produce, engine->data = func_holder;
            assign_arg(engine->drop), assign_arg(engine->align);
        } break;

        default: not_implemented("");
    }
}
#undef not_implemented

static void va_list_drop(va_list *list) {
    if (list) va_end(*list);
}

static int fd_produce(FdHolder fd_holder, void *buf, size_t len) {
    return fd_holder? read(fd_holder->fd, buf, len): -1;
}

static int range_produce(RangeHolder range_holder, ssize_t *buf, size_t len) {
    void *__buf = buf;
    if (range_holder->begin > range_holder->end) return -1;
    while (len && range_holder->begin != range_holder->end) *buf++ = range_holder->begin, --len, range_holder->begin += range_holder->step;
    return (void *)buf - __buf;
}

static int stream_produce(StreamHolder stream_holder, void *buf, size_t len, size_t align) {
    void *__buf = buf;
    int (*should_end)(void *item) = stream_holder->should_end? : should_end_at_null;
    while(len && !stream_holder->done) {
        void *val = stream_holder->stream_next(stream_holder->stream, stream_holder->data);
        stream_holder->done = should_end(val);
        memcpy(buf, &val, align), buf += align, --len;
    }
    return !len || __buf != buf? buf - __buf: -1;
}

static int func_produce(FuncHolder func_holder, void *buf, size_t len, size_t align) {
    void *__buf = buf;
    int (*should_end)(void *item) = func_holder->should_end? : should_end_at_null;
    while(len && !func_holder->done) {
        void *val = func_holder->func_next(func_holder->data);
        func_holder->done = should_end(val);
        memcpy(buf, &val, align), buf += align, --len;
    }
    return !len || __buf != buf? buf - __buf: -1;
}

static void engine_data_fmt(String string, EngineType type, void *engine_data) {
    if (!engine_data) return string_fmt(string, "(nil)");
    switch(type) {
        case ENGINE_FILE_DESCRIPTOR: {
            FdHolder fd_holder = engine_data;
            string_fmt(string, "{fd: %d}", fd_holder->fd);
        } break;

        case ENGINE_NESTED_STREAM: {
            StreamHolder stream_holder = engine_data;
            string_fmt(string, "{stream: ");
            string_fmt_func(string, (void *)stream_deb_dprint, stream_holder->stream);
            string_fmt(string, ", done: %s, data: %p}",
                       stream_holder->done? "true": "false", stream_holder->data);
        } break;

        case ENGINE_RANGE: {
            RangeHolder range_holder = engine_data;
            string_fmt(string, "{step: %ld, begin: %ld, end: %ld}",
                       range_holder->step, range_holder->begin, range_holder->end);
        } break;

        case ENGINE_FUNCTIONAL: {
            FuncHolder func_holder = engine_data;
            string_fmt(string, "{done: %s, data: %p}",
                       func_holder->done? "true": "false", func_holder->data);
        } break;

        default: string_fmt(string, "(inval)");
    }
}

static int should_end_at_null(void *item) {
    return !item;
}

#endif // _GARAGE_ENGINE_C
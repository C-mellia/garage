#ifndef _GARAGE_STREAM_C
#define _GARAGE_STREAM_C 1

#include "./.engine.c"

static inline __attribute__((unused))
void __stream_vinit(Stream stream, EngineType engine_type, va_list args);
static inline __attribute__((unused))
int deq_buffer_read(Deque deq, Engine engine, ssize_t count);
static inline __attribute__((unused))
int deq_buffer_check_len(Deque deq, Engine engine, size_t count);

static void __stream_vinit(Stream stream, EngineType engine_type, va_list args) {
    __engine_vinit((void *)stream->engine, engine_type, args);
    deq_init((void *)stream->deq, stream->engine_align);
}

static int deq_buffer_read(Deque deq, Engine engine, ssize_t count) {
    __label__ L0;
    ssize_t __count = count;
L0:
    int cnt = engine_next(deq, engine);
    if (cnt == -1) return -1;
    count -= cnt;
    if (cnt && count > 0) goto L0;
    return engine->align * (__count - count);
}

static int deq_buffer_check_len(Deque deq, Engine engine, size_t count) {
    size_t len = deq->len;
    // printf("%zu\n", count);
    while (len < count) len = len? len << 1: 8;
    if (len == deq->len) return -1;
    return deq_buffer_read(deq, engine, len - deq->len);
}

#endif // _GARAGE_STREAM_C

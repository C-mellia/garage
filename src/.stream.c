#ifndef _GARAGE_STREAM_C
#define _GARAGE_STREAM_C 1

#include "./.engine.c"

#define BUF_LEN 8

static inline __attribute__((unused))
void __stream_vinit(Stream stream, EngineType engine_type, va_list args);
static inline __attribute__((unused))
int deq_buffer_read(Deque deq, Engine engine, size_t count);
static inline __attribute__((unused))
int deq_buffer_check_len(Deque deq, Engine engine, size_t count);

static void __stream_vinit(Stream stream, EngineType engine_type, va_list args) {
    __engine_vinit((void *)stream->engine, engine_type, args);
    deq_init((void *)stream->deq, stream->engine_align);
}

static int deq_buffer_read(Deque deq, Engine engine, size_t count) {
    __label__ L0;
    char buf[engine->align * BUF_LEN];
    size_t __count = count;
L0:
    int cnt = engine_produce(engine, buf, BUF_LEN < count? BUF_LEN: count);
    if (cnt == -1) return -1;
    for(int i = 0; count && i < cnt; i += engine->align, --count) memcpy(deq_push_back(deq, 0), buf + i, deq->slice_align);
    if ((size_t)cnt == sizeof buf) goto L0;
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

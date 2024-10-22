#ifndef GARAGE_ENGINE_H
#define GARAGE_ENGINE_H 1

#include <sys/types.h>
#include <garage/types.h>

struct stream;

typedef enum engine_type {
    ENGINE_NONE=-1,
    ENGINE_FILE_DESCRIPTOR,
    ENGINE_NESTED_STREAM,
    ENGINE_RANGE,
    ENGINE_FUNCTIONAL,
    __ENGINE_COUNT,
} EngineType;

/**
* The `produce` field can be defined and assigned with different function signature via casting with `(void *)`
*
* `produce`: outputs are wrote to a buffer
*     `align` parameter will be passed but can be ignored
* `data`:
*      a. `int *` (fd_drop)
*      b. `struct stream *` (stream_drop)
*      c. `struct { void (*foo_drop)(void *); struct foo *foo; }` (pair_drop)
* `engine_new(EngineType type, ...)`
*  ```
*     fd_engine_new(int fd); [1]
*     range_engine_new(ssize_t step, ssize_t begin, ssize_t end); [3]
*     stream_engine_new(
*         Stream stream,
*         void *(*stream_next)(Stream stream, void *data),
*         int (*should_end)(void *item),
*         void *data,
*         void *(*drop)(void **item),
*         size_t align,
*     ); [6]
*     func_engine_new(
*         void *(*func_next)(void *data),
*         int (*should_end)(void *item),
*         void *data,
*         void *(*drop)(void **item),
*         size_t align,
*     ); [5]
* ```
*/
typedef struct engine {
    EngineType type;
    int (*produce) (void *engine_data, void *buf, size_t len, size_t align);
    void *(*drop)(void **item);
    size_t align;
    void *data;
} *Engine;

typedef struct fd_holder {
    int fd;
} *FdHolder;

typedef struct range_holder {
    ssize_t step, begin, end;
} *RangeHolder;

typedef struct stream_holder {
    struct stream *stream; // reference
    void *(*stream_next)(struct stream *stream, void *data);
    int (*should_end)(void *item);
    int done;
    void *data; // reference
} *StreamHolder;

typedef struct func_holder {
    void *(*func_next)(void *data);
    int (*should_end)(void *item);
    int done;
    void *data;
} *FuncHolder;

void engine_init(Engine engine, EngineType type, ...);
Engine engine_new(EngineType type, ...);
void engine_cleanup(Engine engine);
void *engine_drop(Engine *engine);

int engine_deb_dprint(int fd, Engine engine);
int engine_deb_print(Engine engine);

// accepts the number of items which is aligned by `engine->align` and returns the bytes written to `buf`
int engine_produce(Engine engine, void *buf, size_t len);

#endif // GARAGE_ENGINE_H

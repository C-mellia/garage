#ifndef GARAGE_TYPES_H
#define GARAGE_TYPES_H 1

#include <stddef.h>
#include <stdint.h>

typedef uint8_t Phantom[0] __attribute__((aligned(8)));
typedef int (*Dprint)(int fd, void *obj);
typedef void *Void;

typedef struct stream_status3 {
    int should_stop, should_peek, should_consume;
} *StreamStatus3;

#endif // GARAGE_TYPES_H

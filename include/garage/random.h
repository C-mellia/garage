#ifndef RANDOM_H
#define RANDOM_H 1

#include <stdint.h>

typedef struct {
	int fd;
} *RandomEngine;

RandomEngine re_new(void);
uint8_t re_get_u8(RandomEngine re);
float re_get_f32(RandomEngine re);
double re_get_f64(RandomEngine re);
uint32_t re_get_u32(RandomEngine re);
uint64_t re_get_u64(RandomEngine re);

#endif // RANDOM_H

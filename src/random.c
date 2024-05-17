#include "garage.h"
#include "random.h"
#include <unistd.h>
#include <fcntl.h>

extern StackAllocator sa;

RandomEngine re_new(void) {
	RandomEngine re = sa_alloc(sa, sizeof *re);
	if (re->fd = open("/dev/random", O_RDONLY), re->fd < 0) {
		perror("open");
		panic("");
	}
	return re;
}

uint8_t re_get_u8(RandomEngine re) {
	uint8_t buf[1] = {0};
	int count = 0;
	if (count = read(re->fd, buf, 1), count < 0) {
		perror("read");
		panic("");
	}
	return *buf;
}

float re_get_f32(RandomEngine re) {
	uint8_t buf[4] = {0};
	int count = 0;
	if (count = read(re->fd, buf, 4), count < 0) {
		perror("read");
		panic("");
	}
	return (float) *(uint32_t *) &buf / (float) 0x100000000;
}

double re_get_f64(RandomEngine re) {
	uint8_t buf[4] = {0};
	int count = 0;
	if (count = read(re->fd, buf, 4), count < 0) {
		perror("read");
		panic("");
	}
	return (double) *(uint32_t *) &buf / (double) 0x100000000;
}

uint32_t re_get_u32(RandomEngine re) {
	uint8_t buf[4] = {0};
	int count = 0;
	if (count = read(re->fd, buf, 4), count < 0) {
		perror("read");
		panic("");
	}
	return *(uint32_t *) &buf;
}

uint64_t re_get_u64(RandomEngine re) {
	uint8_t buf[8] = {0};
	int count = 0;
	if (count = read(re->fd, buf, 8), count < 0) {
		perror("read");
		panic("");
	}
	return *(uint64_t *) &buf;
}

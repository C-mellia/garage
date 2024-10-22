#include <alloca.h>
#include <stdlib.h>

#include <unistd.h>
#include <fcntl.h>

#include <garage/garage.h>
#include <garage/random.h>
#include <garage/log.h>

static const char *const random_device = "/dev/random";

static void __re_init(RandomEngine re);

void re_init(RandomEngine re) {
    if (!re) return;
    __re_init(re);
}

RandomEngine re_new(void) {
    RandomEngine re = malloc(sizeof *re);
    alloc_check(malloc, re, sizeof *re);
    return __re_init(re), re;
}

void re_cleanup(RandomEngine re) {
    if (!re) return;
    close(re->fd), re->fd = -1;
}

uint8_t re_get_u8(RandomEngine re) {
    void *buf = alloca(1);
    return read(re->fd, buf, 1) < 1? 0: *(uint8_t *) buf;
}

float re_get_f32(RandomEngine re) {
    void *buf = alloca(4);
    return read(re->fd, buf, 4) < 4? 0.0: (float) *(uint32_t *) buf / (float) (1L << 32);
}

double re_get_f64(RandomEngine re) {
    void *buf = alloca(4);
    return read(re->fd, buf, 4) < 4? 0.0: (double) *(uint32_t *) buf / (double) (float) (1L << 32);
}

uint32_t re_get_u32(RandomEngine re) {
    void *buf = alloca(4);
    return read(re->fd, buf, 4) < 4? 0: *(uint32_t *) buf;
}

uint64_t re_get_u64(RandomEngine re) {
    void *buf = alloca(8);
    return read(re->fd, buf, 4) < 4? 0: *(uint64_t *) buf;
}

void *re_drop(RandomEngine *re) {
    if (re) re_cleanup(*re), *re = 0;
    return re;
}

static void __re_init(RandomEngine re) {
    re->fd = open(random_device, O_RDONLY);
    assert(re->fd != -1, "re_init: failed to open file '%s'\n", random_device);
}

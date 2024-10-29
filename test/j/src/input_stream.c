#include <stdlib.h>
#include <string.h>

#include <garage/prelude.h>
#include <garage/deque.h>

#include <input_stream.h>
#include "./.input_stream.c"

void input_stream_init(InputStream input_stream, int fd) {
}

InputStream input_stream_new(int fd) {
}

void input_stream_cleanup(InputStream input_stream) {
}

void *input_stream_drop(InputStream *input_stream) {
}

int input_stream_deb_dprint(int fd, InputStream stream) {
}

int input_stream_deb_print(InputStream stream) {
}

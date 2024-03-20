#ifndef GLOBAL_ALLOCATOR_H
#define GLOBAL_ALLOCATOR_H 1

typedef struct Alloc {
	uint8_t *mem, *top;
	size_t cap;
} *Alloc;

Alloc global_allocator_init(size_t cap);
void *global_alloc(Alloc a, size_t size);
void global_allocator_deinit(Alloc a);

Alloc global_allocator = 0;

#endif // GLOBAL_ALLOCATOR_H

Alloc global_allocator_init(size_t cap) {
	__label__ cleanup;
	Alloc a = malloc(sizeof *a);
	if (!a) goto cleanup;
	a->mem = malloc(cap);
	if (!a->mem) goto cleanup;
	a->top = a->mem + cap;
	a->cap = cap;
	return a;
cleanup:
	if (a) free(a);
	return 0;
}

void *global_alloc(Alloc a, size_t size) {
	if (a->mem + size > a->top) return 0;
	a->top -= size;
	return a->top;
}

void global_allocator_deinit(Alloc a) {
	if (a) {
		if (a->mem) free(a->mem);
		free(a);
	}
}

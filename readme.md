# GARAGE

A library and also a design pattern provided with multiple useful utils and
macros to help build well structured and maintainable applications in C, and
hopefully speed up the development process.

## Design Pattern

### struct Composition

The idea is that using a zero-sized type(usually zero-length array) to insert a
anchor point before a certain field that act as a constant pointer to that
field; Objects de facto are merely datas located at a certain position on
memory, and if you know the position of the object, you can pass the address to
the functions(or methods) that access the object via a pointer to act on the
fields of the object.

Since the pointer is always effective, it can be casted to any pointer type of
any size reasonable; And therefore provides to the functions that act on that
specific type of object without necessarily nul checking.

Example:

```c
typedef struct Foo {
   int val;
} *Foo;

typedef struct Bar {
   char *str;
} *Bar;

typedef struct Baz {
    Phantom foo;
    int val;

    Phantom bar;
    char *str;
} *Baz;

Foo foo_new(int val);
void foo_cleanup(A val);
int foo_get(Foo foo);

Bar bar_new(char *str);
void bar_cleanup(Bar bar);
char *bar_get(Bar bar);

Baz baz_new(int val, char *str);
void baz_cleanup(Baz baz);

...

Baz baz = baz_new(10, "hello");
int val = foo_get((void *) baz->foo); // expected `10`
char *str = bar_get((void *) baz->bar); // expected `"hello"`
```

However, there will be a issue regarding the position of the `Phantom` field
because struct alignements. To give an example:

```c
typedef struct Foo {
    int val;
    Phantom bar;
    char *_str;
} *Foo;
```

The value of `_str` will not match up with the `str` field of the `Bar` struct
when the `bar` field is casted to `Bar` type. One way to find out why mismatch
happens is by running:

```c
printf("%lu, %lu, %lu\n", offsetof(Foo, bar), offsetof(Foo, _str), offsetof(Bar, str));
```

Normally c-compiler will pad the struct among its fields, instead of
packing the fields densely, which helps to avoid 2 cycles of memory access to
fetch the field by packing fields, whose size summed together is less than the
number of memory banks.

Field `bar` will be positioned by the end of field `val` because `sizeof val +
sizeof bar` is less than 8 which is a common alignment for 64-bit systems, and,
field `_str` will positioned at the 8th byte of the struct, which causes the
mismatch.

Adding an additional attribute `__attribute__((packed))` to the struct will
solve the issue, or by forcefully specifying the `aligned` attribute of the
`bar` field to 8 also solves the issue.

```c

### <a name="cleanup"></a> `cleanup` functions

- `void <type>_cleanup(<type> obj);`

The `cleanup` functions are used to release additional resources that were
allocated during the lifetime of a object, without deallocating the object
itself.

Some objects may not require any additional resources to be released, or only
contains a reference to other objects that are responsible for their own
cleanup. In such cases, the `cleanup` function might as well be a dummy
function.

If a object is allocated on the stack, the `cleanup` function can also be used
as a cleanup function instead of [`drop`](#drop) function, to deallocate
additional resources when the object goes out of scope automatically.

### <a name="drop"></a> `drop` functions

- `void <type>_drop(<type> *obj);`

### <a name="pop_front/back"></a> `pop_front` and `pop_back` functions

- `void *<type>_pop_front(<type> *obj);`
- `void *<type>_pop_back(<type> *obj);`

These functions will return the pointer to the address where the block of
memory that is supposed to be popped located, however, this block of memory
will soon be invalidated after several other actions of pushings and poppings,
but it will be the user's responsibilities to copy the memory to which the
pointer points before they got invalidated.

## Structure

### include/alc.h
### include/array.h
### include/ascii.h
### include/deque.h
### include/error.h
### include/garage.h:
### include/http.h
### include/log.h
### include/random.h
### include/sa.h
### include/slice.h
### include/string.h
### include/vec.h

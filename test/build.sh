#!/bin/bash

CC=gcc
CC_FLAG='-Wall -Wextra -std=gnu11 -I../include/'
LD_FLAG='-L../ -lgarage -lpthread'
CFILES='main'
SRC='src'

if [[ ! -d target ]]; then
    mkdir target
fi

build() {
    echo "$CC -c $1 -o $2 -static $CC_FLAG"
    $CC -c $1 -o $2 -static $CC_FLAG
}

link_obj() {
    targets=""
    for fname in $*; do
        targets+="./target/$fname.o "
    done
    echo "$CC $targets -o main $LD_FLAG"
    $CC $targets -o main $LD_FLAG
}

for fname in $CFILES; do
    if [[ ! -f src/$fname.c ]]; then
        echo "WARNING: File $fname not found in dir $SRC, skipping..."
        continue
    fi
    build src/$fname.c target/$fname.o
    if [[ ! $? -eq 0 ]]; then
        echo "ERROR: Failed to build $fname, exiting..."
        exit 1
    fi
done

link_obj $CFILES
if [[ ! $? -eq 0 ]]; then
    echo "ERROR: Failed to link"
    exit 1
fi

echo "Build process completed!"

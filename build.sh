#!/bin/bash
CC=gcc
CC_FLAG='-Wall -Wextra -std=gnu11'
FD_FLAG=''
CFILES='garage'

if [[ ! -d target ]]; then
    mkdir target
fi

function build {
    echo "$CC -c $1.c -o ./target/$1.o $CC_FLAG"
    $CC -c $1.c -o ./target/$1.o $CC_FLAG
}

function link_library {
    targets=""
    for fname in $*; do
        targets+="./target/$fname.o "
    done
    echo "ar -rcs libgarage.a $targets"
    ar -rcs libgarage.a $targets
    chmod +x libgarage.a
}

for fname in $CFILES; do
    if [[ ! -f $fname.c ]]; then
        echo "File $fname.c not found"
        exit 1
    fi
    build $fname
    if [[ ! $? -eq 0 ]]; then
        echo "Build $fname failure"
        exit 1
    fi
done

link_library $CFILES
if [[ ! $? -eq 0 ]]; then
    echo "Link failure"
    # rm -rf ./target
    exit 1
fi

echo "Build process completed"

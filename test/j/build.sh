#!/bin/bash

step_error() {
    echo $* && exit 1
}

step_exec() {
    echo $1 && eval $1 || step_error $2
}

export srcs=$(while read -r file; do basename ${file%.c} | /bin/grep -vE '^\.'; done <<< $(find src/ -type f | /bin/grep -E '\.c$'))
export -f step_error step_exec

[[ -d ./target/ ]] || mkdir target/

./.compile && ./.link && echo "Build process completed!"

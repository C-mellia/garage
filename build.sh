#!/bin/bash

function step_error {
	echo $* && exit 1
}

function step_exec {
    echo $1 && eval $1 || step_error $2
}

export srcs=$(while read -r file; do basename ${file%.c} | env grep -vE '^\.'; done <<< $(find src/ -type f | env grep -E '\.c$'))
export -f step_error step_exec

[[ -d target/ ]] || mkdir target

./.compile && ./.link && echo "Building steps succeeded!"

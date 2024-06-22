#!/bin/bash

function step_error {
	echo $* && exit 1
}

function step_exec {
    echo $1 && eval $1 || step_error $2
}

SRC_FILES='garage
random
array
http
deque'

export SRC_FILES
export -f step_error
export -f step_exec

[[ -d target/ ]] || mkdir target

./compile_step &&
    ./link_step &&
    echo "Building steps succeeded!"

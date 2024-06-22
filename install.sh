#!/bin/bash

function step_failed {
    echo $* && exit 1
}

function step_exec {
    echo $1 && eval $1 || step_failed $2
}

export -f step_failed
export -f step_exec

[[ -d /usr/include/garage ]] || sudo mkdir -p /usr/include/garage

step_exec "sudo cp -r --update include/garage /usr/include/" "Failed to install directory and inside which the headers 'include/garage'"

step_exec "sudo cp --update ./libgarage.a /usr/lib/"

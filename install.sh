#!/bin/bash

function echo_cp {
    echo "cp --update $1 $2"
    sudo cp --update $1 $2
}

[[ -d /usr/include/garage ]] || sudo mkdir /usr/include/garage

echo_cp src/garage.h /usr/include/garage/
echo_cp src/random.h /usr/include/garage/
echo_cp src/array.h /usr/include/garage/
echo_cp libgarage.a /usr/lib/

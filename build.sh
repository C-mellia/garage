#!/bin/bash

[[ -d target/ ]] || mkdir target

./compile_step && ./link_step && echo "Building steps succeeded!"

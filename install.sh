#!/bin/bash
if [[ ! -d /usr/include/garage ]]; then
    sudo mkdir /usr/include/garage
fi
if [[ $(sudo -v) -eq 1 ]]; then
    echo "Root permission required"
    exit 1
fi
sudo cp binode.h /usr/include/garage/
sudo cp garage.h /usr/include/garage/
sudo cp libgarage.a /usr/lib/

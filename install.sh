#!/bin/sh
if [ ! -d /usr/include/garage ]; then
    sudo mkdir /usr/include/garage
fi
sudo cp binode.h /usr/include/garage/
sudo cp garage.h /usr/include/garage/

#!/bin/bash
[[ -d /usr/include/garage ]] || sudo mkdir /usr/include/garage
echo "cp --update src/garage.h /usr/include/garage/"
sudo cp --update src/garage.h /usr/include/garage/
echo "cp --update src/random.h /usr/include/garage/"
sudo cp --update src/random.h /usr/include/garage/
echo "cp --update libgarage.a /usr/lib/"
sudo cp --update libgarage.a /usr/lib/

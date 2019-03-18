#!/bin/bash
git submodule update --init --recursive
DIR=`dirname "$(readlink -f "$0")"`
cd linear-rpc
./bootstrap
./configure --prefix="$DIR/build" install
make install

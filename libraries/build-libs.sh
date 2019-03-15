#!/bin/bash
DIR=`dirname "$(readlink -f "$0")"`
cd linear-rpc
./bootstrap
./configure --prefix="$DIR/build"
#./configure --prefix="$DIR/build" --enable-shared=no --enable-static=yes
make install

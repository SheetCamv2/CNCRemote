#!/bin/bash
git submodule update --init --recursive
DIR=`dirname "$(readlink -f "$0")"`
cd $DIR/linear-rpc
./bootstrap
pwd
./configure --prefix="$DIR/build"
make install

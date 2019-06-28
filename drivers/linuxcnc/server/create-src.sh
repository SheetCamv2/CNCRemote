#!/bin/bash
BASEDIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null 2>&1 && pwd )"
cd "$BASEDIR"
rm -rf src
rm -rf include
mkdir -p src
mkdir -p include
cp ../../../../include/*.h include
cp ../../../../drivers/linuxcnc/server/*.h include
cp -r ../../../../libraries/build/include/* include
cp ../../../../drivers/linuxcnc/server/*.cpp src
cp ../../../../src/*.cpp src



#!/bin/bash

#Build a driver for the given branch of LinuxCNC
#e.g build.sh v2.7.3
#Note: this takes a LONG time

#Path to LinuxCNC source code. Must be a Git repository.
lcnc=/home/gw/Documents/src/linuxcnc


home=$(dirname $(readlink -f "$0"))
cd $lcnc
git checkout $1
if [ $? -ne 0 ] 
then
   echo "Failed to checkout branch"
   exit 1
fi
cd src
./autogen.sh
if [ $? -ne 0 ] 
then
   exit 1
fi
./configure --with-realtime=uspace
# --without-libmodbus --without-libusb-1.0
if [ $? -ne 0 ] 
then
   exit 1
fi
make
if [ $? -ne 0 ] 
then
   exit 1
fi

. ../scripts/rip-environment
cd $home
codeblocks --rebuild --target=Release-Linux --no-splash-screen linuxcncserver.cbp


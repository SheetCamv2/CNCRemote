#!/bin/bash

#Build a driver for the given branch of LinuxCNC
#e.g build.sh v2.7.3
#Note: this takes a LONG time

if [ -z "$LINUXCNC" ]
then
	echo "You must define a variable called LINUXCNC with the path to your LinuxCNC source. See readme.txt for more details"
	exit -1
fi
echo "using LinuxCNC directory: $LINUXCNC"

home=$(dirname $(readlink -f "$0"))
cd "$LINUXCNC"
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


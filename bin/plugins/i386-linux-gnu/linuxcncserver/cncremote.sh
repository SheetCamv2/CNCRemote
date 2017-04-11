#!/bin/bash

drvname=cncremote-linuxcnc

type -P $drvname > /dev/null

if [ $? -eq 0 ] 
then
   if [ "$1" != "-c" ]
   then
      echo Starting system LinuxCNC driver
      $drvname &
   fi
   exit 0
fi

ver=`linuxcnc_var LINUXCNCVERSION`
ver=`expr $ver : '\([0-9]*\.[0-9]*\.[0-9]*\)'`
if [ $? -ne 0 ]
then 
   echo LinuxCNC not found
   exit 1
fi
exe=$(dirname $(readlink -f "$0"))/$drvname
if [ -f "$exe" ]
then
   if [ "$1" != "-c" ]
   then
      echo "Starting builtin driver for LinuxCNC"
      $exe &
   fi
   exit 0
fi
echo LinuxCNC version = $ver
exe=$exe\_$ver
if [ -f "$exe" ]
then
   if [ "$1" != "-c" ]
   then
      echo "Starting builtin driver for LinuxCNC V$ver"
      $exe &
   fi
   exit 0
fi

echo Driver not found
exit 1


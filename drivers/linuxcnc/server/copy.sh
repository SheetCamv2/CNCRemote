#!/bin/bash

ver=`linuxcnc_var LINUXCNCVERSION`
ver=`expr $ver : '\([0-9]*\.[0-9]*\.[0-9]*\)'`
if [ $? -ne 0 ]
then 
   echo LinuxCNC not found
   exit 1
fi
echo LinuxCNC version = $ver
\cp -rf $2 $2_$ver$3
exit 0

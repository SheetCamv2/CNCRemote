#!/bin/bash

if [ "$1" == "-h" ] || [ "$1" == "--help" ] ; then
   echo `basename "$0"` [-h][-c][-b]
   echo "Check for a suitable server and run it. The server is built if needed"
   echo "-h,--help See this help"
   echo "-c Check and build only. Does not run server"
   echo "-b Build a pre-built server"
   echo "Exit code is 0 if successful or 1 on error"
   exit 0
fi


function run_builtin() #last resort - try built in server
{
#   echo "Looking for pre-built server as a last resort"
   exe=./$drvname\_$ver\_`uname -r`
echo $exe
   if [ -f "$exe" ] ; then
      if [ "$1" != "-c" ] ; then
         echo "Starting pre-built server for LinuxCNC V$ver"
         $exe
      else
         echo "Pre-built server found"
      fi
      exit 0
   fi
   echo "Pre-built server not found"
}


function run_system()
{
   echo "Checking for LinuxCNC server..."
   type -P $drvname > /dev/null

   if [ $? -eq 0 ] ; then
      if [ "$1" != "-c" ] ; then
         echo "Starting system LinuxCNC server"
         $drvname
      else
         echo "System server found"
      fi
      exit 0
   fi
}


BASEDIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null 2>&1 && pwd )"
cd "$BASEDIR"

drvname=cncremote-linuxcnc

if [ "$1" != "-b" ]; then
   run_system
fi
ver=`linuxcnc_var LINUXCNCVERSION`
if [ $? -ne 0 ] ; then 
   if [ "$1" == "-c" ] ; then
      echo "LinuxCNC not found"
      echo "If you are using run-in-place you must run this from a rip-environment."
   fi
   exit 1
fi
ver=`expr $ver : '\([0-9]*\.[0-9]*\.[0-9]*\)'`

export LD_LIBRARY_PATH=../../../lib:../../../../libraries/build/lib

exe="./$drvname"

if [ -n "$1" ] ; then
   echo "LinuxCNC version is $ver"
fi
type -P halcompile > /dev/null
if [ $? -ne 0 ]; then
   run_builtin
   >&2 echo "Cannot find LinuxCNC development files." 
   echo "Try installing the development files for LinuxCNC using your package manager." 
   echo "This will probably be the package 'linuxcnc-dev' or 'linuxcnc-uspace-dev'."
   echo "If you are using run-in-place you must run this from a rip-environment."
   exit 1
fi

if [ "$1" == "-b" ]; then
   if [ -z "$EMC2_HOME" ]; then
      echo "Looks like we are using an installed version of LinuxCNC"
   else
      echo "Looks like we are using run-in-place"
   fi
   rm -rf obj
fi
mkdir -p obj
cd include
../version.sh
cd ..
make -j -f makefile 
if [ $? -ne 0 ]; then
   echo "Failed to build the server"
   run_builtin
   exit 1
fi

if [ "$1" == "-b" ]; then
   echo "Build succeeded"
   mv $exe $exe\_$ver\_`uname -r`
   exit 0
fi


if [ "$1" != "-c" ] ; then
   echo "Starting LinuxCNC server"
   ./$drvname
fi

exit 0


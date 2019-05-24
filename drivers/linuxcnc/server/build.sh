#!/bin/bash

function FindVersion
{
  git checkout master
  readarray vers < <(git log --follow --oneline --date-order -- VERSION)
  nlines=${#vers[@]}
  count=0
  if [ $nlines -gt 40 ]; then
    nlines=40
  fi

  while [ $count -lt $nlines ];do
    line=(${vers[count]})
    git checkout ${line[0]} VERSION
    tmp=`cat VERSION`
    if [ $tmp = $1 ]; then
      branch=${line[0]}
      break
    fi
    let count+=1
  done
  git checkout master VERSION
  if [ -z $branch ]; then
    echo "Version $1 not found"
    exit 1
  fi
  git checkout $branch
  if [ $? -ne 0 ]; then
    echo "Failed to checkout branch"
    exit 1
  fi
}


home=$(dirname $(readlink -f "$0"))
if [ -z $1 ] || [ -z $2 ]; then
  echo "Usage: build.sh version /path/to/linuxcnc"
  echo "where the version is a LinuxCNC release version or Git commit hash and the path points to a LinuxCNC Git repository"
  echo "NOTE: the Git repository must not have any uncommitted changes"
  echo "e.g. build.sh 2.7.1 ~/Documents/linuxcnc"
  echo "e.g. build.sh c328dfc ~/Documents/linuxcnc"
  exit 1
fi
cd $2
git checkout "$1"
if [ $? -ne 0 ]; then
  echo "Not a commit hash - trying to find by version"
  FindVersion $1
fi

cd src
./autogen.sh
if [ $? -ne 0 ]; then
   exit 1
fi
./configure --with-realtime=uspace
# --without-libmodbus --without-libusb-1.0
if [ $? -ne 0 ]; then
   exit 1
fi
make
if [ $? -ne 0 ]; then
   exit 1
fi

. ../scripts/rip-environment
cd $home
codeblocks --rebuild --target=Release-Linux --no-splash-screen linuxcncserver.cbp




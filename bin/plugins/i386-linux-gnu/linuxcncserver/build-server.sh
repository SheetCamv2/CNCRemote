#!/bin/bash
set -e

DownloadLCNC () {
   if [ ! -d linuxcnc/.git ] ; then
      echo "Downloading LinuxCNC source"
      git clone https://github.com/LinuxCNC/linuxcnc.git
   else
      echo "LinuxCNC source exists"
   fi
   cd linuxcnc
   git checkout $ver
   cd ..
   export LINUXCNC=$(readlink -f "linuxcnc")
}



ver=`linuxcnc_var LINUXCNCVERSION`
verfull=`expr $ver : '\([0-9]*\.[0-9]*\.[0-9]*\)'`
ver=`expr $ver : '\([0-9]*\.[0-9]*\)'`
if [ $? -ne 0 ]; then 
   echo "LinuxCNC not found"
   echo "If you are using run-in-place you must run this script from a rip-environment"
   exit 1
fi

if hash codeblocks 2>/dev/null; then
   echo "Code::Blocks found"
else
   echo "Code::Blocks is not installed."
   echo "Please install Code::Blocks. e.g 'sudo apt-get install codeblocks'"
   exit 1
fi

if [[ `linuxcnc_var REALTIME` == /etc/* ]]; then
   echo "Targeting installed version $ver"
   if [! -d /usr/include/linuxcnc ] ; then
      echo "LinuxCNC headers not found."
      echo "Try sudo apt-get install linuxcnc-dev"
      exit 1
   fi
   DownloadLCNC
else
   echo "Building from current source"
   LINUXCNC=$(dirname $(dirname `linuxcnc_var REALTIME`))
fi

BASEDIR=$(dirname "$0")

if [ -d CNCRemote/.git ] ; then
   echo "CNCRemote source exists"
else
   echo "Downloading CNCRemote source"
   git clone --recursive https://github.com/sheetcam/CNCRemote.git
fi

(cd CNCRemote; git checkout linear-rpc; git submodule update --init --recursive)

if [ ! -d CNCRemote/libraries/build ] ; then
   echo "Building CNCRemote libraries"
   (cd CNCRemote/libraries; ./build-libs.sh)
fi

echo "LinuxCNC is at $LINUXCNC"
codeblocks --rebuild --target=Release-Linux --no-splash-screen CNCRemote/drivers/linuxcnc/server/linuxcncserver.cbp
cp "CNCRemote/bin/plugins/i386-linux-gnu/linuxcncserver/cncremote-linuxcnc_$verfull" ./

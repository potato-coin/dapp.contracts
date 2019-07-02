#! /bin/bash

printf "\t=========== Building potato.contracts ===========\n\n"

RED='\033[0;31m'
NC='\033[0m'

unamestr=`uname`
if [[ "${unamestr}" == 'Darwin' ]]; then
   BOOST=/usr/local
   CXX_COMPILER=g++
   export ARCH="Darwin"
else
   OS_NAME=$( cat /etc/os-release | grep ^NAME | cut -d'=' -f2 | sed 's/\"//gI' )

   case "$OS_NAME" in
      "Amazon Linux AMI")
         export ARCH="Amazon Linux AMI"
         ;;
      "CentOS Linux")
         export ARCH="Centos"
         export CMAKE=${HOME}/opt/cmake/bin/cmake
         ;;
      "elementary OS")
         export ARCH="elementary OS"
         ;;
      "Fedora")
         export ARCH="Fedora"
         ;;
      "Linux Mint")
         export ARCH="Linux Mint"
         ;;
      "Ubuntu")
         export ARCH="Ubuntu"
         ;;
      "Debian GNU/Linux")
         export ARCH="Debian"
	 ;;
      *)
         printf "\\n\\tUnsupported Linux Distribution. Exiting now.\\n\\n"
         exit 1
   esac
fi

CORES=`getconf _NPROCESSORS_ONLN`
mkdir -p build
pushd build &> /dev/null
if [ -z "$CMAKE" ]; then
  CMAKE=$( command -v cmake )
fi
"$CMAKE" ../
if [ $? -ne 0 ]; then
   exit -1;
fi
make -j${CORES}
if [ $? -ne 0 ]; then
   exit -1;
fi
popd &> /dev/null

printf "\n\n${bldred}"
printf '\t  _____    ____  _______      _______  ____  \n' 
printf '\t |  __ \  / __ \|__   __| /\ |__   __|/ __ \ \n'
printf '\t | |__) || |  | |  | |   /  \   | |  | |  | |\n'
printf '\t |  ___/ | |  | |  | |  / /\ \  | |  | |  | |\n'
printf '\t | |     | |__| |  | | / ____ \ | |  | |__| |\n'
printf '\t |_|      \____/   |_|/_/    \_\|_|   \____/ \n'
printf "\t                                             \n${txtrst}"

printf "\\tFor more information:\\n"
printf "\\tPOTATO website: https://www.potatocoin.com\\n"
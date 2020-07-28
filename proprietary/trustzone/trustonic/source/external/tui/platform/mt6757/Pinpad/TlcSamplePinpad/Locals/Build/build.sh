#!/bin/bash

# Source setup.sh
setup_script=""
search_dir=$(dirname $(readlink -f ${BASH_SOURCE[0]}))
while [[ ${setup_script} == "" ]]
do
  setup_script=$( find $search_dir -name "setup.sh" )
  search_dir=$(dirname $search_dir)
done
if [[ ${setup_script} == "" ]]; then
  echo "ERROR: setup.sh not found"
  exit 1
fi
source $setup_script
t_sdk_root=$( dirname $setup_script )

if [ "$MODE" == "Release" ]; then
  echo -e "Mode\t\t: Release"
  OUT_DIR=Out/Bin/$APP_ABI/Release
  OPTIM=release
else
  echo -e "Mode\t\t: Debug"
  OUT_DIR=Out/Bin/$APP_ABI/Debug
  OPTIM=debug
fi

# go to project root
cd $(dirname $(readlink -f $0))/../..

### ---------------- Generic Build Command ----------------

# run NDK build
${NDK_BUILD} \
    -B \
    NDK_DEBUG=1 \
    NDK_PROJECT_PATH=Locals/Code \
    NDK_APPLICATION_MK=Locals/Code/Application.mk \
    NDK_MODULE_PATH=${t_sdk_root} \
    NDK_APP_OUT=Out/_build \
    APP_BUILD_SCRIPT=Locals/Code/Android.mk \
    APP_OPTIM=$OPTIM \
    APP_ABI=$APP_ABI

mkdir -p $OUT_DIR

cp -r $PWD/Out/_build/local/$APP_ABI/* $OUT_DIR

echo
echo Output directory of build is $PWD/$OUT_DIR


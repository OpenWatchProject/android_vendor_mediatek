#!/bin/bash

set -e

export TEE=trustonic

#debug,release
#export TARGET_MODE=debug
if [ x$TARGET_MODE == xrelease ]; then
    export TARGET_MODE=release
else
    export TARGET_MODE=debug
fi

#Please do not remove below line, it's used for release scripts to do auto replace!!
#export CHIP_TYPE=GF

if [  -a ./../prebuilts/tee/${TEE}/${TEE_VERSION}/t-base/setup.sh ]; then
source ./../prebuilts/tee/${TEE}/${TEE_VERSION}/t-base/setup.sh
fi

if [ ! -d Locals ]; then
  exit 1
fi

export TASDK_DIR_SRC=${COMP_PATH_TlSdk}
export TASDK_DIR=${COMP_PATH_TlSdk}
export TEE_TRUSTLET_OUTPUT_PATH=./../out

echo "Running make..."
make -f Locals/Code/makefile.mk "$@"

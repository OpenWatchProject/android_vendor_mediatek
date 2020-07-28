#!/bin/bash

source ./../prebuilts/tee/${TEE}/${TEE_VERSION}/t-base/setup.sh

if [ ! -d Locals ]; then
  exit 1
fi

export TLSDK_DIR=${COMP_PATH_TlSdk}
export DRSDK_DIR=${COMP_PATH_DrSdk}

export TEE_DRIVER_OUTPUT_PATH=./../out

echo "Running make..."
make -f Locals/Code/makefile.mk "$@"

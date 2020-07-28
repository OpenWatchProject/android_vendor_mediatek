#!/bin/bash

if [  -a ./../prebuilts/tee/${TEE}/${TEE_VERSION}/t-base/setup.sh ]; then
    source ./../prebuilts/tee/${TEE}/${TEE_VERSION}/t-base/setup.sh
    export TEE_DRIVER_OUTPUT_PATH=./../out
else
    # is only for Android L
    source ${TBASE_BUILD_ROOT}/setup.sh
    cd $(dirname $(readlink -f $0))
    cd ../..
fi #[  -a ./../prebuilts/tee/${TEE}/${TEE_VERSION}/t-base/setup.sh ]

if [ ! -d Locals ]; then
    exit 1
fi #[ ! -d Locals ]

export TLSDK_DIR=${COMP_PATH_TlSdk}
export DRSDK_DIR=${COMP_PATH_DrSdk}

echo "Running make..."
make -f Locals/Code/makefile.mk "$@"

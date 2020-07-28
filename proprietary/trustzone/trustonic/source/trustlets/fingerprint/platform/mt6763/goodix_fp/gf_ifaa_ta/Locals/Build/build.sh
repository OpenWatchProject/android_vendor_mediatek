#!/bin/bash
#
# Copyright (C) 2013-2016, Shenzhen Huiding Technology Co., Ltd.
# All Rights Reserved.
#

set -e

if [  -a ./../prebuilts/tee/${TEE}/${TEE_VERSION}/t-base/setup.sh ]; then
    source ./../prebuilts/tee/${TEE}/${TEE_VERSION}/t-base/setup.sh
    export TEE_TRUSTLET_OUTPUT_PATH=./../out
else
    # is only for Android L
    source ${TBASE_BUILD_ROOT}/setup.sh
    cd $(dirname $(readlink -f $0))
    cd ../..
fi #[  -a ./../prebuilts/tee/${TEE}/${TEE_VERSION}/t-base/setup.sh ]

if [ ! -d Locals ]; then
    exit 1
fi #[ ! -d Locals ]

export TASDK_DIR_SRC=${COMP_PATH_TlSdk}
export TASDK_DIR=${COMP_PATH_TlSdk}
export TLSDK_DIR=${COMP_PATH_TlSdk}

echo "Running make..."
make -f Locals/Code/makefile.mk all

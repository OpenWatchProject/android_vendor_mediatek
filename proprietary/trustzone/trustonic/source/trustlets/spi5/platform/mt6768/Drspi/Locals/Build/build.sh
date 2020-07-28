#!/bin/bash

t_sdk_root=${TBASE_BUILD_ROOT}

DRV_NAME=drspi

if [ "$TEE_MODE" == "Release" ]; then
  echo -e "Mode\t\t: Release"
  OUT_DIR=${TEE_DRIVER_OUTPUT_PATH}/${DRV_NAME}/Release
  OPTIM=release
else
  echo -e "Mode\t\t: Debug"
  OUT_DIR=${TEE_DRIVER_OUTPUT_PATH}/${DRV_NAME}/Debug
  OPTIM=debug
fi

export DRSEC_LIB_PATH=${TEE_DRIVER_OUTPUT_PATH}/drsec/${TEE_MODE}/drsec.lib
# For customer release - mode check
if [ "$TEE_MODE" == "Debug" ] && [ ! -f ${TEE_DRIVER_OUTPUT_PATH}/drsec/${TEE_MODE}/drsec.lib ] ; then
    echo "${TEE_DRIVER_OUTPUT_PATH}/drsec/${TEE_MODE}/drsec.lib is not existed in 'Debug' mode, changing to 'Release' mode"
    DRSEC_LIB_PATH=${TEE_DRIVER_OUTPUT_PATH}/drsec/Release/drsec.lib
elif [ "$TEE_MODE" == "Release" ] && [ ! -f ${TEE_DRIVER_OUTPUT_PATH}/drsec/${TEE_MODE}/drsec.lib ] ; then
    echo "${TEE_DRIVER_OUTPUT_PATH}/drsec/${TEE_MODE}/drsec.lib is not existed in 'Release' mode, chaning to 'Debug' mode"
    DRSEC_LIB_PATH=${TEE_DRIVER_OUTPUT_PATH}/drsec/Debug/drsec.lib
fi

# For customer release - file check
if [ ! -f ${DRSEC_LIB_PATH} ] ; then
    echo "[ERROR] '${DRSEC_LIB_PATH}' is still not existed! Please check it!"
    exit 0
fi

source ${t_sdk_root}/setup.sh

cd $(dirname $(readlink -f $0))
cd ../..

if [ ! -d Locals ]; then
  exit 1
fi

export TLSDK_DIR_SRC=${COMP_PATH_TlSdk}
export DRSDK_DIR_SRC=${COMP_PATH_DrSdk}
export TLSDK_DIR=${COMP_PATH_TlSdk}
export DRSDK_DIR=${COMP_PATH_DrSdk}

echo "Running Spi make..."
make -f Locals/Code/makefile.mk "$@"

if [ $? -ne 0 ]; then
	echo "[ERROR] <t-driver build failed!"
	exit 1;
fi


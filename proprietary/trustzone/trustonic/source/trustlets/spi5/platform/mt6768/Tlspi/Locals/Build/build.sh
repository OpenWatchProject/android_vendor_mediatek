#!/bin/bash

t_sdk_root=${TBASE_BUILD_ROOT}

export COMP_PATH_ROOT=$(dirname $(readlink -f ${BASH_SOURCE[0]}))
export DRSPI_OUT_DIR=${TEE_DRIVER_OUTPUT_PATH}/drspi
export DRSPI_DIR=${COMP_PATH_ROOT}/../../../Drspi/
export DRUTILS_OUT_DIR=${TEE_DRIVER_OUTPUT_PATH}/drutils
export DRUTILS_DIR=${COMP_PATH_ROOT}/../../../Drutils/

TL_NAME=tlspi

if [ "$TEE_MODE" == "Release" ]; then
  echo -e "Mode\t\t: Release"
  OUT_DIR=${TEE_TRUSTLET_OUTPUT_PATH}/${TL_NAME}/Release
  OPTIM=release
else
  echo -e "Mode\t\t: Debug"
  OUT_DIR=${TEE_TRUSTLET_OUTPUT_PATH}/${TL_NAME}/Debug
  OPTIM=debug
fi

export DRUTILS_LIB_PATH=${DRUTILS_OUT_DIR}/${TEE_MODE}/drutils.lib
# For customer release - mode check
if [ "$TEE_MODE" == "Debug" ] && [ ! -f ${DRUTILS_OUT_DIR}/${TEE_MODE}/drutils.lib ] ; then
    echo "${DRUTILS_OUT_DIR}/${TEE_MODE}/drutils.lib is not existed in 'Debug' mode, changing to 'Release' mode"
    DRUTILS_LIB_PATH=${DRUTILS_OUT_DIR}/Release/drutils.lib
elif [ "$TEE_MODE" == "Release" ] && [ ! -f ${DRUTILS_OUT_DIR}/${TEE_MODE}/drutils.lib ] ; then
    echo "${DRUTILS_OUT_DIR}/${TEE_MODE}/drutils.lib is not existed in 'Release' mode, chaning to 'Debug' mode"
    DRUTILS_LIB_PATH=${DRUTILS_OUT_DIR}/Debug/drutils.lib
fi

# For customer release - file check
if [ ! -f ${DRUTILS_LIB_PATH} ] ; then
    echo "[ERROR] '${DRUTILS_LIB_PATH}' is still not existed! Please check it!"
    exit 0
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
export TLSDK_DIR=${COMP_PATH_TlSdk}

echo "Running tlspi make..."	
make -f Locals/Code/makefile.mk "$@"

if [ $? -ne 0 ]; then
	echo "[ERROR] Trustlet build failed!"
	exit 1;
fi


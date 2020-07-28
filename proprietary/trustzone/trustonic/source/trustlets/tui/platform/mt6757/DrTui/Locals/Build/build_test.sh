#!/bin/bash

t_sdk_root=${TBASE_BUILD_ROOT}

DRV_NAME=drTui

if [ "$TEE_MODE" == "Release" ]; then
  echo -e "Mode\t\t: Release"
  OUT_DIR=${TEE_DRIVER_OUTPUT_PATH}/${DRV_NAME}/Release
  OPTIM=release
else
  echo -e "Mode\t\t: Debug"
  OUT_DIR=${TEE_DRIVER_OUTPUT_PATH}/${DRV_NAME}/Debug
  OPTIM=debug
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
mkdir -p Out/Public
cp -f \
	Locals/Code/public/* \
	Out/Public/
echo "Running make..."	
make -f Locals/Code/makefile_test.mk "$@"

if [ $? -ne 0 ]; then
	echo "[ERROR] <t-driver build failed!"
	exit 1;
fi

mkdir -p ${MKTOPDIR}/vendor/trustonic/platform/${ARCH_MTK_PLATFORM}/trustlets/tui/DrTui/${OPTIM}/
mkdir -p ${MKTOPDIR}/vendor/trustonic/platform/${ARCH_MTK_PLATFORM}/trustlets/tui/DrTui/public/

cp -f \
        Locals/Code/public/* \
        ${MKTOPDIR}/vendor/trustonic/platform/${ARCH_MTK_PLATFORM}/trustlets/tui/DrTui/public/

#cp -f ${OUT_DIR}/drTui.lib  ${MKTOPDIR}/vendor/trustonic/platform/${ARCH_MTK_PLATFORM}/trustlets/tui/DrTui/${OPTIM}/
cp -f ${OUT_DIR}/drTui.axf  ${MKTOPDIR}/vendor/trustonic/platform/${ARCH_MTK_PLATFORM}/trustlets/tui/DrTui/${OPTIM}/

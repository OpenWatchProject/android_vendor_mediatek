#!/bin/bash

t_sdk_root=${TBASE_BUILD_ROOT}

export COMP_PATH_ROOT=$(dirname $(readlink -f ${BASH_SOURCE[0]}))

TL_NAME=tlutils

if [ "$TEE_MODE" == "Release" ]; then
  echo -e "Mode\t\t: Release"
  OUT_DIR=${TEE_TRUSTLET_OUTPUT_PATH}/${TL_NAME}/Release
  OPTIM=release
else
  echo -e "Mode\t\t: Debug"
  OUT_DIR=${TEE_TRUSTLET_OUTPUT_PATH}/${TL_NAME}/Debug
  OPTIM=debug
fi

source ${t_sdk_root}/setup.sh

cd $(dirname $(readlink -f $0))
cd ../..

if [ ! -d Locals ]; then
  exit 1
fi

export TLSDK_DIR_SRC=${COMP_PATH_TlSdk}
export TLSDK_DIR=${COMP_PATH_TlSdk}
#mkdir -p Out/Public
#cp -f \
#	Locals/Code/public/* \
#	Out/Public/
echo "Running make..."	
make -f Locals/Code/makefile.mk "$@"

if [ $? -ne 0 ]; then
	echo "[ERROR] Trustlet build failed!"
	exit 1;
fi

#mkdir -p ${MKTOPDIR}/vendor/trustonic/platform/${ARCH_MTK_PLATFORM}/trustlets/utils/Tlutils/${OPTIM}/
#mkdir -p ${MKTOPDIR}/vendor/trustonic/platform/${ARCH_MTK_PLATFORM}/trustlets/utils/Tlutils/public/

#cp -f \
#        Locals/Code/public/* \
#        ${MKTOPDIR}/vendor/trustonic/platform/${ARCH_MTK_PLATFORM}/trustlets/utils/Tlutils/public/

#cp -f ${OUT_DIR}/tlutils.axf  ${MKTOPDIR}/vendor/trustonic/platform/${ARCH_MTK_PLATFORM}/trustlets/utils/Tlutils/${OPTIM}/

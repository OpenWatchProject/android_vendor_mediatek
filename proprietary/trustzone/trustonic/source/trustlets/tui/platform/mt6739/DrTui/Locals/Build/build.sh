#!/bin/bash

echo "Only TOOLCHAIN=ARM is currently supported"
export TOOLCHAIN=ARM

t_base_root=$(readlink -f $(dirname $(readlink -f $0))/../../../../..)

source ${t_base_root}/setup.sh

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
	Locals/Code/Public/* \
	Out/Public/
echo "Running make..."
make -f Locals/Code/makefile_plat.mk "$@"

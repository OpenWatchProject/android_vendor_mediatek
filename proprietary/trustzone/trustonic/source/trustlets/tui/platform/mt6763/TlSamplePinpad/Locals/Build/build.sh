#!/bin/bash -eu

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

cd $(dirname $(readlink -f $0))
cd ../..

if [ ! -d Locals ]; then
  exit 1
fi

export TLSDK_DIR_SRC=${COMP_PATH_TlSdk}
export TLSDK_DIR=${COMP_PATH_TlSdk}
export TASDK_DIR_SRC=${COMP_PATH_TlSdk}
export TASDK_DIR=${COMP_PATH_TlSdk}
mkdir -p Out/Public
cp -f \
	Locals/Code/public/* \
	Out/Public/
echo "Running make..."

for i in 3 6
do
    if [ "$TOOLCHAIN" == "GNU" ]; then
        rm -rf Out/Bin/GNU/${MODE}/obj-local
    else
        rm -rf Out/Bin/${MODE}/obj-local
    fi
    export TBASE_API_LEVEL=$i
    make -f Locals/Code/makefile.mk "$@"
done

#!/bin/bash -eu

# Source setup.sh
setup_script=""
search_dir=$(dirname $(readlink -f ${BASH_SOURCE[0]}))
build_dir=${search_dir}
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

cd ${build_dir}/..

rm -rf Locals/Code/libs
rm -rf Locals/Code/gen
rm -rf Locals/Code/assets/*.tlbin
rm -rf Locals/Code/obj
rm -rf Out/Bin/$MODE
rm -rf Out/Doc
mkdir -p Locals/Code/libs

cd $build_dir/../Code

if [ "$MODE" == "Release" ]; then
  echo -e "Mode\t\t: Release"
  OPTIM=release
else
  echo -e "Mode\t\t: Debug"
  OPTIM=debug
fi

mkdir -p assets
if [ $TOOLCHAIN == "ARM" ]; then
    cp -f ${COMP_PATH_TlSamplePinpad}/Bin/$MODE/0789000000000000000000000000000?.tlbin assets
else
    cp -f ${COMP_PATH_TlSamplePinpad}/Bin/GNU/$MODE/0789000000000000000000000000000?.tlbin assets
fi

mkdir -p libs
cp -f ${COMP_PATH_TeeClient_module}/Bin/java/$MODE/TeeClient.jar libs

mkdir -p libs/armeabi-v7a
cp -f ${COMP_PATH_TlcSamplePinpad}/Bin/armeabi-v7a/$MODE/libTlcPinpad.so    libs/armeabi-v7a
cp -f ${COMP_PATH_TeeClient_module}/Bin/armeabi-v7a/$MODE/libTeeClient.so   libs/armeabi-v7a
mkdir -p libs/arm64-v8a
cp -f ${COMP_PATH_TlcSamplePinpad}/Bin/arm64-v8a/$MODE/libTlcPinpad.so      libs/arm64-v8a
cp -f ${COMP_PATH_TeeClient_module}/Bin/arm64-v8a/$MODE/libTeeClient.so     libs/arm64-v8a

ant $OPTIM
if [ $? -ne 0 ]; then
    echo "ERROR: Build failed."
    exit 1
fi

mkdir -p $build_dir/../../Out/Bin/${MODE}
cp -f $build_dir/../Code/out/*${OPTIM}*.apk $build_dir/../../Out/Bin/$MODE/

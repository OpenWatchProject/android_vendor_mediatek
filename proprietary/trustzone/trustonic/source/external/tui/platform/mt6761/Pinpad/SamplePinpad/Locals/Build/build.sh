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
source ${setup_script}

cd ${build_dir}/../..
if [ -d Out/_build ]; then
    rm -r Out/_build
fi

# Create build environment in Out
mkdir -p Out/_build/SamplePinpad
cp -rfa Locals/Build/resources/*    Out/_build
cp -rfa Locals/Code/*               Out/_build/SamplePinpad/src/main
cd Out/_build

if [ "${MODE}" == "Release" ]; then
  echo -e "Mode\t\t: Release"
  lower_MODE=release
else
  echo -e "Mode\t\t: Debug"
  lower_MODE=debug
fi

# Import dependecies
mkdir -p src/main/assets
if [ ${TOOLCHAIN} == "ARM" ]; then
    cp -f ${COMP_PATH_TlSamplePinpad}/Bin/$MODE/0789000000000000000000000000000?.tlbin      SamplePinpad/src/main/assets
else
    cp -f ${COMP_PATH_TlSamplePinpad}/Bin/GNU/$MODE/0789000000000000000000000000000?.tlbin  SamplePinpad/src/main/assets
fi

# Create local.properties according to ANDROID_HOME set in setup.sh
echo "sdk.dir=${ANDROID_HOME}" >> SamplePinpad/local.properties

# Compile
export CORES=`cat /proc/cpuinfo | grep -c processor`
export lower_MODE
if [[ ${MODE} == "Debug" ]]; then
    ./gradlew clean lint assembleDebug
else
    ./gradlew clean lint assembleRelease
fi
if [ $? -ne 0 ]; then
    echo "ERROR: Build failed."
    exit 1
fi

# Copy binaries in Out directory
mkdir -p ${build_dir}/../../Out/Bin/${MODE}
cp -f \
    SamplePinpad/build/outputs/apk/SamplePinpad-signed-${lower_MODE}.apk \
    ${build_dir}/../../Out/Bin/${MODE}/SamplePinpad-${lower_MODE}.apk
if [ -f SamplePinpad/build/outputs/apk/SamplePinpad-unsigned-release.apk ]; then
    cp -f \
        SamplePinpad/build/outputs/apk/SamplePinpad-unsigned-release.apk \
        ${build_dir}/../../Out/Bin/${MODE}/SamplePinpad-release-unsigned.apk
fi

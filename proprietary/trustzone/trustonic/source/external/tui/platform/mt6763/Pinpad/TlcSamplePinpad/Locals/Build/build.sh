#!/bin/bash -e
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
t_sdk_root=$( dirname $setup_script )

if [ "$MODE" == "Release" ]; then
  echo -e "Mode\t\t: Release"
  OUT_DIR=Out/Bin/$APP_ABI/Release
  OPTIM=release
else
  echo -e "Mode\t\t: Debug"
  OUT_DIR=Out/Bin/$APP_ABI/Debug
  OPTIM=debug
fi

# go to project root
cd $(dirname $(readlink -f $0))/../..

if [[ ${TARGET_OS} == "Linux" ]]; then
    if [ -f Locals/Code/CMakeLists.txt ]; then
        ### ---------------- Linux Build Command ----------------
        CORES=`cat /proc/cpuinfo | grep -c processor`
        BASE=$PWD
        BuildDir=Out/_build/$MODE/$APP_ABI
        mkdir -p $BuildDir
        mkdir -p $OUT_DIR/LinuxBinaries

cat > $PWD/$BuildDir/toolchain.cmake <<EOF
# CMake system name must be something like "Linux".
# This is important for cross-compiling.
set( CMAKE_SYSTEM_NAME "`echo ${OS_REL_PATH_LINAROGCC} | sed -e 's/^./\u&/' -e 's/^\(Linux\).*/\1/'`" )
set( CMAKE_SYSTEM_PROCESSOR ${SYSTEM_PROCESSOR} )
set( CMAKE_C_COMPILER ${CROSS_GCC_PATH_GCC} )
set( CMAKE_CXX_COMPILER ${CROSS_GCC_PATH_GPP} )
set( CMAKE_ASM_COMPILER ${CROSS_GCC_PATH_GCC} )
set( CMAKE_AR ${CROSS_GCC_PATH_AR} CACHE FILEPATH "Archiver" )
set( CMAKE_C_FLAGS_RELEASE "${ARCH_FLAGS} -O2 -pipe -g -feliminate-unused-debug-types -DNDEBUG" CACHE STRING "CFLAGS for release" )
set( CMAKE_CXX_FLAGS_RELEASE "${ARCH_FLAGS} -O2 -pipe -g -feliminate-unused-debug-types -fvisibility-inlines-hidden -DNDEBUG" CACHE STRING "CXXFLAGS for release" )
set( CMAKE_ASM_FLAGS_RELEASE "${ARCH_FLAGS} -O2 -pipe -g -feliminate-unused-debug-types -DNDEBUG" CACHE STRING "ASM FLAGS for release" )
set( CMAKE_C_FLAGS_DEBUG "${ARCH_FLAGS} -O2 -pipe -g -feliminate-unused-debug-types  " CACHE STRING "CFLAGS for debug" )
set( CMAKE_CXX_FLAGS_DEBUG "${ARCH_FLAGS} -O2 -pipe -g -feliminate-unused-debug-types -fvisibility-inlines-hidden " CACHE STRING "CXXFLAGS for debug" )
set( CMAKE_ASM_FLAGS_DEBUG "${ARCH_FLAGS} -O2 -pipe -g -feliminate-unused-debug-types " CACHE STRING "ASM FLAGS for debug" )
set( CMAKE_FIND_ROOT_PATH "${CROSS_GCC_PATH_DIR} ")
set( CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ONLY )
set( CMAKE_FIND_ROOT_PATH_MODE_PROGRAM BOTH )
set( CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY )
set( CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY )
set(COMP_PATH_TlAes ${COMP_PATH_TlAes})
set(COMP_PATH_TlAsyncExample ${COMP_PATH_TlAsyncExample})
set(COMP_PATH_TlRsa ${COMP_PATH_TlRsa})
set(COMP_PATH_TlFloat ${COMP_PATH_TlFloat})
set(COMP_PATH_TlCryptoCatalog ${COMP_PATH_TlCryptoCatalog})
set(COMP_PATH_DrTplayDrmSkeleton ${COMP_PATH_DrTplayDrmSkeleton})
set(COMP_PATH_TlPlay ${COMP_PATH_TlPlay})
set(COMP_PATH_TlSampleRot13 ${COMP_PATH_TlSampleRot13})
set(COMP_PATH_TlSampleSha256 ${COMP_PATH_TlSampleSha256})
set(COMP_PATH_TACryptoCatalog ${COMP_PATH_TACryptoCatalog})
set(COMP_PATH_TASampleRot13 ${COMP_PATH_TASampleRot13})
set(COMP_PATH_TlSdk ${COMP_PATH_TlSdk})
EOF
        cd ${BuildDir}

        cmake \
            $BASE/Locals/Code/ \
            -DCMAKE_INSTALL_PREFIX=$BASE/$OUT_DIR/LinuxBinaries \
            -DCMAKE_TOOLCHAIN_FILE=toolchain.cmake \
            -DCOMMON_DEP_INCLUDE_DIRS=$COMP_PATH_MobiCoreClientLib_module/Public/ \
            -DCOMMON_DEP_LIBRARIES=$COMP_PATH_MobiCoreClientLib_module/Bin/$APP_ABI/Debug/ \
            -DCMAKE_BUILD_TYPE=$MODE \
            -Wno-dev

        make  -j $CORES

        make install
        binaries=$(find $BASE/$OUT_DIR/LinuxBinaries -type f)
        cp $binaries $BASE/$OUT_DIR
        rm -rf $BASE/$OUT_DIR/LinuxBinaries
        rm -rf *.cmake CMake* *.txt

        cd $BASE
    else
        echo "CMakeLists.txt not present"
    fi
elif [[ ${TARGET_OS} == "Android" ]]; then

### ---------------- Android Build Command ----------------
# run NDK build
${NDK_BUILD} \
    -B \
    NDK_DEBUG=1 \
    NDK_PROJECT_PATH=Locals/Code \
    NDK_APPLICATION_MK=Locals/Code/Application.mk \
    NDK_MODULE_PATH=${t_sdk_root} \
    NDK_APP_OUT=Out/_build \
    APP_BUILD_SCRIPT=Locals/Code/Android.mk \
    APP_OPTIM=$OPTIM \
    APP_ABI=$APP_ABI

mkdir -p $OUT_DIR

cp -r $PWD/Out/_build/local/$APP_ABI/* $OUT_DIR

else
  echo "ERROR: TARGET_OS [$TARGET_OS] not supported"
  exit 1
fi

echo
echo Output directory of build is $PWD/$OUT_DIR

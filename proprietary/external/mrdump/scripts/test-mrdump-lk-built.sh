#!/bin/bash

function test_project() {
    local product=$1
    local variant=$2
    local flags=$3
    local md5flags=`echo -n ${flags}  |  md5sum | awk -e '{ print $1; }'`

    local bootloader
    if [ -z "${flags}" ]; then
	echo "Test build ${product} ${variant} [${flags}]"
	bootloader_out=${android_top}/mrdump-out/${product}-${variant}
    else
	echo "Test build ${product} ${variant} [${flags} - $md5flags]"
	bootloader_out=${android_top}/mrdump-out/${product}-${variant}-$md5flags
    fi

    lunch full_${product}-${variant}
    rm -rf ${bootloader_out}
    mkdir -p ${bootloader_out}

    if ! ./prebuilts/build-tools/linux-x86/bin/make -j16 -C vendor/mediatek/proprietary/bootable/bootloader/lk TOOLCHAIN_PREFIX=${android_top}/prebuilts/gcc/linux-x86/arm/arm-linux-androideabi-4.9/bin/arm-linux-androideabi- BOOTLOADER_OUT=${bootloader_out}/ ROOTDIR=${android_top} LCM_WIDTH=720 LCM_HEIGHT=1280 ${flags} COMPILER=${android_top}/prebuilts/clang/host/linux-x86/clang-r353983c/bin/clang ${product} >& ${bootloader_out}/build.log; then
	echo "Test build ${product} ${variant} [${flags}] - Failed"
    else
	echo "Test build ${product} ${variant} [${flags}] - Passed"
    fi
};

set -e
android_top=$(gettop)

rm -rf ${android_top}/mrdump-out/
pushd ${android_top}

if [ -z "$1" ]; then
    test_project k37mv1_bsp eng
    test_project k37mv1_bsp user "KEDUMP_MINI=no"

    test_project k80hd_bsp_ago eng
    test_project k80hd_bsp_ago user

    test_project k39tv1_bsp_1g user

    test_project k50sv1_64_bsp eng

    test_project k61v1_64_bsp eng
    test_project k61v1_64_bsp user
    test_project k61v1_64_bsp userdebug
    test_project k61v1_64_bsp eng "KEDUMP_MINI=no"
    test_project k61v1_64_bsp eng "KEDUMP_MINI=yes MTK_MRDUMP_ENABLE=dconfig"
    test_project k61v1_64_bsp user "KEDUMP_MINI=no"
    test_project k61v1_64_bsp user "KEDUMP_MINI=yes MTK_MRDUMP_ENABLE=yes"
    test_project k61v1_64_bsp user "KEDUMP_MINI=yes MTK_MRDUMP_ENABLE=dconfig"

    test_project tb8167p3_bsp eng
    test_project tb8167p3_bsp user

    test_project k79v1_64 eng
    test_project k79v1_64 user
    test_project k79v1_64 userdebug

    test_project k85v1_64 eng
else
    test_project $1 user
fi

popd

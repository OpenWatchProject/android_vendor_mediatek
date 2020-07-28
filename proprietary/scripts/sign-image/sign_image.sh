#!/bin/bash

#######################################
# Initialize variables
#######################################
set -e
D_CURR=`pwd`

#######################################
# Specify temporarily folder path
#######################################

function usage() {

	#######################################
	# Dump usage howto
	#######################################

	echo "sign image ..."
	echo "Please source ./build/envsetup.sh and ./mbldenv.sh first, and select correct project"
	echo "Command: ./sign_image.sh <BASE_PROJECT>"
}


#######################################
# Check arguments
#######################################
source ./build/envsetup.sh
MTK_BASE_PROJECT=$(get_build_var MTK_BASE_PROJECT)
PRODUCT_OUT=$(get_build_var PRODUCT_OUT)
MTK_PLATFORM=$(get_build_var MTK_PLATFORM)
MTK_PLATFORM_DIR=${MTK_PLATFORM,,}

if [ "$1" != "" ]; then
	SIGN_FOR_SDL_ONLY=$1
else
	SIGN_FOR_SDL_ONLY=
fi

v2_file="vendor/mediatek/proprietary/custom/${MTK_PLATFORM_DIR}/security/cert_config/img_list.txt"

# sign-image-nodeps
if [ -f "$v2_file" ]; then
	echo "v2 sign flow"
	echo python vendor/mediatek/proprietary/scripts/sign-image_v2/sign_flow.py "${MTK_PLATFORM_DIR}" "${MTK_BASE_PROJECT}"
	PYTHONDONTWRITEBYTECODE=True PRODUCT_OUT=$(get_build_var PRODUCT_OUT) BOARD_AVB_ENABLE=$(get_build_var BOARD_AVB_ENABLE) python vendor/mediatek/proprietary/scripts/sign-image_v2/sign_flow.py -env_cfg vendor/mediatek/proprietary/scripts/sign-image_v2/env.cfg "${MTK_PLATFORM_DIR}" "${MTK_BASE_PROJECT}"
else
	echo "v1 sign flow"
	MTK_PROJECT_NAME=$(get_build_var MTK_PROJECT_NAME)
	MTK_PATH_CUSTOM=$(get_build_var MTK_PATH_CUSTOM)
	MTK_SEC_SECRO_AC_SUPPORT=$(get_build_var MTK_SEC_SECRO_AC_SUPPORT)
	MTK_NAND_PAGE_SIZE=$(get_build_var MTK_NAND_PAGE_SIZE)
	BOARD_AVB_ENABLE=$(get_build_var BOARD_AVB_ENABLE)
	echo perl vendor/mediatek/proprietary/scripts/sign-image/SignTool.pl "${MTK_BASE_PROJECT}" "${MTK_PROJECT_NAME}" "${MTK_PATH_CUSTOM}" "${MTK_SEC_SECRO_AC_SUPPORT}" "${MTK_NAND_PAGE_SIZE}" "${BOARD_AVB_ENABLE}" "${PRODUCT_OUT}" "all" "${SIGN_FOR_SDL_ONLY}"
	perl vendor/mediatek/proprietary/scripts/sign-image/SignTool.pl "${MTK_BASE_PROJECT}" "${MTK_PROJECT_NAME}" "${MTK_PATH_CUSTOM}" "${MTK_SEC_SECRO_AC_SUPPORT}" "${MTK_NAND_PAGE_SIZE}" "${BOARD_AVB_ENABLE}" "${PRODUCT_OUT}" "all" "${SIGN_FOR_SDL_ONLY}"
fi

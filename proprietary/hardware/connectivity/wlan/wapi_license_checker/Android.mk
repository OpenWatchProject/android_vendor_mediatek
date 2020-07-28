# This module is dedicated to check WAPI license, customer should enable MTK_WAPI_SUPPORT FO
# after you get WAPI license or it will encounter a build break!
# Following is a build error message you might see in build error message:
# =======================================================================
# ninja: error:
# 'out/target/product/{proj_name}/obj/SHARED_LIBRARIES/libwapi_intermediates/link_type', needed by
# 'out/target/product/{proj_name}/obj/SHARED_LIBRARIES/NO_WAPI_LICENSE_intermediates/link_type',
# missing and no known rule to make it
# =======================================================================
# To solve this license issue, please contact CPM to apply WAPI license directly.

ifeq ($(MTK_WAPI_SUPPORT), yes)
LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)
LOCAL_MODULE := NO_WAPI_LICENSE
LOCAL_PROPRIETARY_MODULE := true
LOCAL_MODULE_OWNER := mtk
LOCAL_SHARED_LIBRARIES := libwapi
include $(BUILD_SHARED_LIBRARY)
endif

LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)
LOCAL_SRC_FILES := meta_gsensor.c
LOCAL_C_INCLUDES := $(MTK_PATH_SOURCE)/hardware/meta/common/inc $(MTK_PATH_SOURCE)/external/sensor-tools $(MTK_PATH_SOURCE)/kernel/include/
LOCAL_SHARED_LIBRARIES := liblog libnvram libhwm
LOCAL_STATIC_LIBRARIES := libft
LOCAL_MODULE := libmeta_gsensor
LOCAL_PROPRIETARY_MODULE := true
LOCAL_MODULE_OWNER := mtk
LOCAL_PRELINK_MODULE := false
include $(MTK_STATIC_LIBRARY)

###############################################################################
# SELF TEST
###############################################################################
BUILD_SELF_TEST := false
ifeq ($(BUILD_SELF_TEST), true)
include $(CLEAR_VARS)
LOCAL_SRC_FILES := meta_gsensor_test.c
LOCAL_C_INCLUDES := $(MTK_PATH_SOURCE)/hardware/meta/common/inc $(MTK_PATH_SOURCE)/external/sensor-tools $(MTK_PATH_SOURCE)/kernel/include/
LOCAL_MODULE := meta_gs_test 
LOCAL_PROPRIETARY_MODULE := true
LOCAL_MODULE_OWNER := mtk
LOCAL_ALLOW_UNDEFINED_SYMBOLS := true
LOCAL_SHARED_LIBRARIES := libhwm
LOCAL_STATIC_LIBRARIES := libmeta_gsensor libft
LOCAL_MODULE_PATH := $(TARGET_OUT_VENDOR_OPTIONAL_EXECUTABLES)
LOCAL_UNSTRIPPED_PATH := $(TARGET_ROOT_OUT_SBIN_UNSTRIPPED)
include $(MTK_EXECUTABLE)
endif

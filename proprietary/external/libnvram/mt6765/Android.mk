LOCAL_PATH:=$(call my-dir)
include $(CLEAR_VARS)
#LOCAL_ARM_MODE:=arm
LOCAL_SHARED_LIBRARIES:= libc libcutils liblog
LOCAL_SRC_FILES:=nvram_platform.c
LOCAL_C_INCLUDES:= \
    $(MTK_PATH_SOURCE)/external/nvram/libnvram
LOCAL_MODULE:=libnvram_platform
LOCAL_PROPRIETARY_MODULE := true
LOCAL_MODULE_OWNER := mtk
LOCAL_MODULE_TAGS := optional
LOCAL_PRELINK_MODULE:=false
include $(MTK_SHARED_LIBRARY)

###############################################################################
# SELF TEST
###############################################################################
BUILD_SELF_TEST := false
# BUILD_SELF_TEST := true
ifeq ($(BUILD_SELF_TEST), true)
include $(CLEAR_VARS)
LOCAL_SRC_FILES := nvram_layout_test.c
LOCAL_C_INCLUDES := $(MTK_PATH_SOURCE)/external/nvram/libnvram
LOCAL_MODULE := nvram_layout_test
LOCAL_PROPRIETARY_MODULE := true
LOCAL_MODULE_OWNER := mtk
LOCAL_ALLOW_UNDEFINED_SYMBOLS := true
#LOCAL_SHARED_LIBRARIES := libc libnvram libft
LOCAL_SHARED_LIBRARIES := libc libnvram
LOCAL_MODULE_PATH := $(TARGET_OUT_VENDOR_OPTIONAL_EXECUTABLES)
LOCAL_UNSTRIPPED_PATH := $(TARGET_ROOT_OUT_SBIN_UNSTRIPPED)
include $(MTK_EXECUTABLE)
endif

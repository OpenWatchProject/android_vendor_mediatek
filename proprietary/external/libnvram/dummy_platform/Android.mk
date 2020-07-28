ifeq ($(strip $(MTK_PLATFORM)),)

LOCAL_PATH:=$(call my-dir)
include $(CLEAR_VARS)
#LOCAL_ARM_MODE:=arm
LOCAL_SHARED_LIBRARIES:= libc libcutils
LOCAL_SRC_FILES:=nvram_platform.c
LOCAL_C_INCLUDES:= \
    $(MTK_PATH_SOURCE)/external/nvram/libnvram
LOCAL_MODULE:=libnvram_platform
LOCAL_PROPRIETARY_MODULE := true
LOCAL_MODULE_OWNER := mtk
LOCAL_MODULE_TAGS := optional
LOCAL_PRELINK_MODULE:=false
include $(MTK_SHARED_LIBRARY)

endif

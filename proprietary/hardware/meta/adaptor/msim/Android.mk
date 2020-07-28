ifeq ($(MTK_TELEPHONY_FEATURE_SWITCH_DYNAMICALLY), yes)
##### MSIM library #####
BUILD_MSIM_META_LIB := true

ifeq ($(BUILD_MSIM_META_LIB), true)
LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)
LOCAL_SRC_FILES := meta_msim.c
LOCAL_C_INCLUDES := $(MTK_PATH_SOURCE)/hardware/meta/common/inc
LOCAL_C_INCLUDES += $(MTK_PATH_SOURCE)/external/libsysenv
LOCAL_MODULE := libmeta_msim
LOCAL_PROPRIETARY_MODULE := true
LOCAL_MODULE_OWNER := mtk
LOCAL_SHARED_LIBRARIES := libcutils libutils libsysenv
LOCAL_STATIC_LIBRARIES := libft
LOCAL_PRELINK_MODULE := false
include $(MTK_STATIC_LIBRARY)
endif
endif

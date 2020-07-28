ifeq ($(MTK_TELEPHONY_FEATURE_SWITCH_DYNAMICALLY), yes)
##### FM RAT library #####
BUILD_RAT_META_LIB := true

ifeq ($(BUILD_RAT_META_LIB), true)
LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)
LOCAL_SRC_FILES := meta_rat.c
LOCAL_C_INCLUDES := $(MTK_PATH_SOURCE)/hardware/meta/common/inc
LOCAL_C_INCLUDES += $(MTK_PATH_SOURCE)/external/libsysenv
LOCAL_MODULE := libmeta_rat
LOCAL_PROPRIETARY_MODULE := true
LOCAL_MODULE_OWNER := mtk
LOCAL_SHARED_LIBRARIES := libcutils libutils libsysenv_system
LOCAL_STATIC_LIBRARIES := libft
LOCAL_PRELINK_MODULE := false
include $(MTK_STATIC_LIBRARY)
endif
endif

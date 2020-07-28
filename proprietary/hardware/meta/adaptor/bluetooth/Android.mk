ifeq ($(MTK_BT_SUPPORT), yes)
LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)

LOCAL_C_INCLUDES := \
  $(MTK_PATH_SOURCE)/hardware/meta/common/inc

LOCAL_SRC_FILES := meta_bt.c

LOCAL_MODULE := libmeta_bluetooth

LOCAL_PROPRIETARY_MODULE := true
LOCAL_MODULE_OWNER := mtk

LOCAL_SHARED_LIBRARIES := libcutils libnetutils libc liblog
LOCAL_STATIC_LIBRARIES := libft
LOCAL_PRELINK_MODULE := false
include $(MTK_STATIC_LIBRARY)

###############################################################################
# SELF TEST
###############################################################################
BUILD_SELF_TEST := false
ifeq ($(BUILD_SELF_TEST), true)
include $(CLEAR_VARS)

LOCAL_C_INCLUDES := \
  $(MTK_PATH_SOURCE)/hardware/meta/common/inc

LOCAL_SRC_FILES := meta_bt_test.c

LOCAL_MODULE := meta_bt_test
LOCAL_PROPRIETARY_MODULE := true
LOCAL_MODULE_OWNER := mtk
LOCAL_ALLOW_UNDEFINED_SYMBOLS := true
LOCAL_SHARED_LIBRARIES := libc
LOCAL_STATIC_LIBRARIES := libmeta_bluetooth libft
LOCAL_MODULE_PATH := $(TARGET_OUT_VENDOR_OPTIONAL_EXECUTABLES)
LOCAL_UNSTRIPPED_PATH := $(TARGET_ROOT_OUT_SBIN_UNSTRIPPED)
include $(MTK_EXECUTABLE)
endif
endif
LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)
LOCAL_MODULE := wifitesttool
LOCAL_PROPRIETARY_MODULE := true
LOCAL_MODULE_OWNER := mtk
LOCAL_MODULE_TAGS := optional
LOCAL_SHARED_LIBRARIES := libwifitest libcutils
LOCAL_CFLAGS += -Wall
LOCAL_SRC_FILES := main.c lib.c
LOCAL_C_INCLUDES = $(LOCAL_PATH)/../libwifitest
include $(MTK_EXECUTABLE)

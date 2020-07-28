LOCAL_PATH := $(call my-dir)
#
# libdisp_dejitter.so
#
include $(CLEAR_VARS)

LOCAL_MODULE := libdisp_dejitter
LOCAL_PROPRIETARY_MODULE := false
LOCAL_MODULE_OWNER := mtk

LOCAL_MODULE_TAGS := optional

LOCAL_CFLAGS := -DLOG_TAG=\"DispDeJitter\"

LOCAL_SRC_FILES := \
	DispDeJitter.cpp

LOCAL_C_INCLUDES := \
	$(TOP)/$(MTK_ROOT)/hardware/libgem/inc \
	$(TOP)/$(MTK_ROOT)/hardware/include \
	$(TOP)/$(MTK_ROOT)/hardware/gralloc_extra/include \
	frameworks/native/libs/nativebase/include \
	frameworks/native/libs/arect/include \
	frameworks/native/services/surfaceflinger

LOCAL_SHARED_LIBRARIES := \
	libcutils \
	libutils \
	liblog \
	libgralloc_extra_sys \
	libnativewindow

LOCAL_CFLAGS += -Wall -Werror -Wunused -Wunreachable-code
include $(BUILD_SHARED_LIBRARY)

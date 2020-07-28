LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_SRC_FILES := \
	AudioMTKDcRemoval.cpp

LOCAL_C_INCLUDES := \
	vendor/mediatek/proprietary/external/audiodcremoveflt

LOCAL_SHARED_LIBRARIES := \
    libaudiodcrflt \
    libnativehelper \
    libcutils \
    libutils

LOCAL_MODULE := libaudiomtkdcremoval
LOCAL_PROPRIETARY_MODULE := true
LOCAL_MODULE_OWNER := mtk

LOCAL_MODULE_TAGS := optional
LOCAL_MULTILIB := both
include $(MTK_SHARED_LIBRARY)

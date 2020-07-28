LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

ifeq ($(MTK_CAM_HDR_SUPPORT), yes)

-include $(MTK_PATH_SOURCE)/hardware/mtkcam/mtkcam.mk

LOCAL_MODULE := libcam.camadapter.scenario.shot.hdrshot
LOCAL_PROPRIETARY_MODULE := true
LOCAL_MODULE_OWNER := mtk

LOCAL_ADDITIONAL_DEPENDENCIES := $(LOCAL_PATH)/Android.mk

LOCAL_SRC_FILES := \
	HDRShot.cpp \
	VHDRShot.cpp \
	utils/WorkerThread.cpp

LOCAL_C_INCLUDES := \
	system/media/camera/include \
	$(MTKCAM_C_INCLUDES) \
	$(MTK_PATH_SOURCE)/hardware/mtkcam/include \
	$(LOCAL_PATH)/../inc \
	$(LOCAL_PATH)/../../../inc/Scenario/Shot \
	$(MTK_PATH_SOURCE)/hardware/gralloc_extra/include

LOCAL_MODULE_TAGS := optional

LOCAL_SHARED_LIBRARIES += libui
LOCAL_CFLAGS := -DLOG_TAG=\"hdrshot\"
LOCAL_CFLAGS += $(MTKCAM_CFLAGS)
LOCAL_CFLAGS += -DUSE_SYSTRACE

include $(MTK_STATIC_LIBRARY)

endif # MTK_CAM_HDR_SUPPORT

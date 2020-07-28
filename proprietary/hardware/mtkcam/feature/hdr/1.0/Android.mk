LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

ifeq ($(MTK_CAM_HDR_SUPPORT), yes)

ifeq ($(TARGET_BOARD_PLATFORM), $(filter $(TARGET_BOARD_PLATFORM), mt6735m mt6737m mt6580 mt6570 mt8163 mt8167))

-include $(TOP)/$(MTK_PATH_SOURCE)/hardware/mtkcam/mtkcam.mk

LOCAL_MODULE := libhdrproc
LOCAL_PROPRIETARY_MODULE := true
LOCAL_MODULE_OWNER := mtk

LOCAL_ADDITIONAL_DEPENDENCIES := $(LOCAL_PATH)/Android.mk

MTK_PATH_CAM := $(MTK_PATH_SOURCE)/hardware/mtkcam
MTK_PATH_CAM_LEGACY := $(MTK_PATH_SOURCE)/hardware/mtkcam/legacy

LOCAL_HEADER_LIBRARIES += libmtkcam_headers

LOCAL_SRC_FILES := \
	HdrProc.cpp \
	Hdr.cpp \
	HDRAlgo.cpp \
	HDRUtils.cpp \
	utils/ImageBufferUtils.cpp

LOCAL_C_INCLUDES := \
	$(TOP)/system/media/camera/include \
	$(MTK_PATH_CAM)/include \
	$(MTK_PATH_SOURCE)/hardware/gralloc_extra/include

ifeq ($(TARGET_BOARD_PLATFORM), $(filter $(TARGET_BOARD_PLATFORM), mt6735m mt6737m mt6580 mt6570 mt8163 mt8167))
LOCAL_C_INCLUDES += \
	$(MTK_PATH_CAM_LEGACY)/include \
	$(MTK_PATH_CAM_LEGACY)/include/mtkcam \
	$(MTK_MTKCAM_PLATFORM)/include \
	$(MTK_MTKCAM_PLATFORM)/v1/hal/adapter/inc
endif

ifeq ($(TARGET_BOARD_PLATFORM), $(filter $(TARGET_BOARD_PLATFORM), mt6735m mt6737m))
LOCAL_C_INCLUDES += \
	$(MTK_PATH_CUSTOM_PLATFORM)/hal/D2/inc
endif

ifeq ($(TARGET_BOARD_PLATFORM), $(filter $(TARGET_BOARD_PLATFORM), mt6580 mt6570 mt8163 mt8167))
LOCAL_C_INCLUDES += \
	$(MTK_PATH_CUSTOM_PLATFORM)/hal/inc
endif

#External
LOCAL_C_INCLUDES += $(TOP)/vendor/mediatek/proprietary/external

LOCAL_SHARED_LIBRARIES := \
	liblog \
	libutils \
	libcutils \
	libcamalgo \
	libfeatureio \
	libmtkcam_stdutils libmtkcam_imgbuf \
	libcam.iopipe \
	libcameracustom \
	libcamdrv \
	libm4u \
	libcam.camshot \

LOCAL_MODULE_TAGS := optional

LOCAL_CFLAGS := $(MTKCAM_CFLAGS)

include $(MTK_SHARED_LIBRARY)

endif # TARGET_BOARD_PLATFORM

endif # MTK_CAM_HDR_SUPPORT

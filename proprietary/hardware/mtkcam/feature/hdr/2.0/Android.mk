LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

ifeq ($(MTK_CAM_HDR_SUPPORT), yes)

ifeq ($(TARGET_BOARD_PLATFORM), $(filter $(TARGET_BOARD_PLATFORM), mt6750 mt6755 mt6761))

-include $(MTK_PATH_SOURCE)/hardware/mtkcam/mtkcam.mk

LOCAL_MODULE := libhdrproc
LOCAL_PROPRIETARY_MODULE := true
LOCAL_MODULE_OWNER := mtk

LOCAL_ADDITIONAL_DEPENDENCIES := $(LOCAL_PATH)/Android.mk

MTK_PATH_CAM := $(MTK_PATH_SOURCE)/hardware/mtkcam
MTK_PATH_CAM_LEGACY := $(MTK_PATH_SOURCE)/hardware/mtkcam/legacy
MTK_PATH_HDR_PLATFORM := ../$(MTK_PLATFORM_DIR)

LOCAL_SRC_FILES := \
	HDRProc2.cpp \
	HDRAlgo.cpp \
	HDRUtils.cpp \
	$(MTK_PATH_HDR_PLATFORM)/Platform.cpp \
	utils/ImageBufferUtils.cpp

LOCAL_C_INCLUDES := \
	$(MTK_PATH_CAM)/include \
	$(MTK_PATH_SOURCE)/hardware/gralloc_extra/include

ifeq ($(TARGET_BOARD_PLATFORM), $(filter $(TARGET_BOARD_PLATFORM), mt6750 mt6755))
LOCAL_C_INCLUDES += \
	$(MTK_PATH_CAM_LEGACY)/include \
	$(MTK_PATH_CAM_LEGACY)/include/mtkcam \
	$(MTK_PATH_CAM_LEGACY)/platform/$(MTK_PLATFORM_DIR) \
	$(MTK_PATH_CAM_LEGACY)/platform/$(MTK_PLATFORM_DIR)/include \
	$(MTK_PATH_CUSTOM_PLATFORM)/hal/inc \
	$(MTK_PATH_CUSTOM_PLATFORM)/hal/inc/isp_tuning
endif

ifeq ($(TARGET_BOARD_PLATFORM), $(filter $(TARGET_BOARD_PLATFORM), mt6761))
LOCAL_C_INCLUDES += \
	$(MTK_PATH_CAM_LEGACY)/include \
	$(MTK_PATH_CAM_LEGACY)/include/mtkcam \
	$(MTK_PATH_CAM_LEGACY)/platform/$(MTK_PLATFORM_DIR) \
	$(MTK_PATH_CAM_LEGACY)/platform/$(MTK_PLATFORM_DIR)/include

ifeq ($(strip $(MTK_CAM_HAL_VERSION)),3)
LOCAL_C_INCLUDES += \
	$(MTK_PATH_CUSTOM_PLATFORM)/hal/inc \
	$(MTK_PATH_CUSTOM_PLATFORM)/hal/inc/isp_tuning
else
LOCAL_C_INCLUDES += \
	$(MTK_PATH_CUSTOM_PLATFORM)/hal1/hal/inc \
	$(MTK_PATH_CUSTOM_PLATFORM)/hal1/hal/inc/isp_tuning
endif
endif

#external
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
	libcam.hal3a.v3 \
	libcam.halsensor

LOCAL_HEADER_LIBRARIES := libcutils_headers libutils_headers libsystem_headers libhardware_headers

LOCAL_MODULE_TAGS := optional

LOCAL_CFLAGS := -DLOG_TAG=\"hdrproc\"
LOCAL_CFLAGS += $(MTKCAM_CFLAGS)
LOCAL_CFLAGS += -DUSE_SYSTRACE

include $(MTK_SHARED_LIBRARY)

endif # TARGET_BOARD_PLATFORM

endif # MTK_CAM_HDR_SUPPORT
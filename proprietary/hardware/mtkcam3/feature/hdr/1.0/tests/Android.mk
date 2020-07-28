LOCAL_PATH:= $(call my-dir)

include $(CLEAR_VARS)

ifeq ($(MTK_CAM_HDR_SUPPORT), yes)

ifeq ($(TARGET_BOARD_PLATFORM), $(filter $(TARGET_BOARD_PLATFORM), mt6735m mt6580 mt6570))

-include $(TOP)/$(MTK_PATH_SOURCE)/hardware/mtkcam/mtkcam.mk

LOCAL_MODULE := test-hdrproc
LOCAL_MODULE_OWNER := mtk

LOCAL_MODULE_TAGS := tests

LOCAL_ADDITIONAL_DEPENDENCIES := $(LOCAL_PATH)/Android.mk

MTK_PATH_CAM := $(MTK_PATH_SOURCE)/hardware/mtkcam
MTK_PATH_CAM_LEGACY := $(MTK_PATH_SOURCE)/hardware/mtkcam/legacy

# camera3test_fixtures.cpp add for start camera3 preview
LOCAL_SRC_FILES := \
	HdrEffectHal_test.cpp \
	main.cpp \
	camera3test_fixtures.cpp

# libhardware,libcamera_metadata, libdl add for start camera3 preview
LOCAL_SHARED_LIBRARIES := \
	liblog \
    libcutils \
    libutils \
    libhdrproc  \
    libmtkcam_stdutils libcam_utils \
    libcamdrv \
    libhardware \
    libcamera_metadata \
    libdl

LOCAL_C_INCLUDES := \
	$(TOP)/system/media/camera/include \
	$(MTK_PATH_CAM)/feature/include \
	$(MTK_PATH_SOURCE)/hardware/gralloc_extra/include \
	$(MTK_PATH_CUSTOM_PLATFORM)/hal/inc

ifeq ($(TARGET_BOARD_PLATFORM), mt6580 mt6570)
LOCAL_C_INCLUDES += \
	$(MTK_PATH_CAM_LEGACY)/include \
	$(MTK_PATH_CAM_LEGACY)/include/mtkcam \
	$(MTK_PATH_CAM_LEGACY)/platform/$(TARGET_BOARD_PLATFORM)/include
endif

LOCAL_CFLAGS := $(MTKCAM_CFLAGS)

# Build the binary to $(TARGET_OUT_DATA_NATIVE_TESTS)/$(LOCAL_MODULE)
# to integrate with auto-test framework.
include $(BUILD_NATIVE_TEST)

endif # TARGET_BOARD_PLATFORM

endif # MTK_CAM_HDR_SUPPORT

LOCAL_PATH:= $(call my-dir)

include $(CLEAR_VARS)

LOCAL_HEADER_LIBRARIES := libcutils_headers libutils_headers libhardware_headers

LOCAL_MODULE := libfeatureiopipe_hdr
LOCAL_PROPRIETARY_MODULE := true
LOCAL_MODULE_OWNER := mtk

ifeq ($(BUILD_MTK_LDVT), true)
	LOCAL_CFLAGS += -DUSING_MTK_LDVT
endif

LOCAL_ADDITIONAL_DEPENDENCIES := $(LOCAL_PATH)/Android.mk
LOCAL_SRC_FILES += \
	hdr_hal_base.cpp \
	hdr_hal.cpp

-include $(TOP)/$(MTK_PATH_SOURCE)/hardware/mtkcam/mtkcam.mk

LOCAL_C_INCLUDES:= \
	$(TOP)/system/media/camera/include \
	$(TOP)/$(MTK_PATH_SOURCE)/hardware/mtkcam/legacy/include \
	$(TOP)/$(MTK_PATH_SOURCE)/hardware/mtkcam/legacy/include/mtkcam \
	$(TOP)/$(MTK_PATH_SOURCE)/hardware/mtkcam/include \
	$(TOP)/$(MTK_MTKCAM_PLATFORM)/include \
	$(TOP)/$(MTK_PATH_CUSTOM_PLATFORM_HAL1)/hal/inc \
	$(TOP)/$(MTK_PATH_SOURCE)/hardware/gralloc_extra/include \
	$(TOP)/$(MTK_PATH_PLATFORM)/hardware/include/mtkcam/algorithm/libmav

include $(MTK_STATIC_LIBRARY)

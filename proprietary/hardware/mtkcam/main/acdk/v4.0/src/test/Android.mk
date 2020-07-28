#
# camshottest
#
LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)
-include $(TOP)/$(MTK_PATH_SOURCE)/hardware/mtkcam/mtkcam.mk

#
LOCAL_SRC_FILES := \
    main.cpp \
    test_mdk.cpp

#
#-----------------------------------------------------------
LOCAL_C_INCLUDES += $(MTKCAM_C_INCLUDES)
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_SOURCE)/hardware/mtkcam/include
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_SOURCE)/hardware/gralloc_extra/include

LOCAL_C_INCLUDES += $(TOP)/system/media/camera/include
LOCAL_C_INCLUDES += \
    $(LOCAL_PATH)/../../inc/acdk \
    $(TOP)/$(MTK_PATH_SOURCE)/kernel/drivers/video \
    $(TOP)/$(MTK_PATH_SOURCE)/kernel/include \
    $(TOP)/$(MTK_PATH_COMMON)/kernel/imgsensor/inc \
    $(TOP)/mediatek/hardware \

ifeq ($(MTK_M4U_SUPPORT),yes)
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_SOURCE)/hardware/m4u/$(TARGET_BOARD_PLATFORM)
endif

LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_SOURCE)/frameworks/av/include

#-----------------------------------------------------------
LOCAL_CFLAGS += $(MTKCAM_CFLAGS)

#-----------------------------------------------------------
LOCAL_SHARED_LIBRARIES += libcutils
LOCAL_SHARED_LIBRARIES += libimageio
LOCAL_SHARED_LIBRARIES += libcam.iopipe
LOCAL_SHARED_LIBRARIES += libmtkcam_stdutils libmtkcam_imgbuf
LOCAL_SHARED_LIBRARIES += libcam.halsensor
LOCAL_SHARED_LIBRARIES += libmtkcam_metadata

ifeq ($(MTK_M4U_SUPPORT),yes)
LOCAL_SHARED_LIBRARIES += libm4u
endif

LOCAL_SHARED_LIBRARIES += libhardware
LOCAL_SHARED_LIBRARIES += libutils
LOCAL_SHARED_LIBRARIES += libbinder
LOCAL_SHARED_LIBRARIES += libdl
LOCAL_SHARED_LIBRARIES += libmtkcam_fwkutils
#
LOCAL_STATIC_LIBRARIES := \

#
CAMSHOT_TEST_USE_DYN_ENTRY := yes

ifeq ($(CAMSHOT_TEST_USE_DYN_ENTRY),yes)
LOCAL_SHARED_LIBRARIES += libacdk
LOCAL_CFLAGS += -DUSE_DYN_ENTRY
else
LOCAL_WHOLE_STATIC_LIBRARIES := libacdk_entry_mdk
endif

#-----------------------------------------------------------
LOCAL_MODULE := acdk_camshottest
LOCAL_PROPRIETARY_MODULE := true
LOCAL_MODULE_OWNER := mtk
LOCAL_PRELINK_MODULE := false

#-----------------------------------------------------------
include $(MTK_EXECUTABLE)

#
#include $(call all-makefiles-under,$(LOCAL_PATH))

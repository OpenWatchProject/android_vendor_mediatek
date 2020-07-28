#
# iopipeFrmBtest
#
LOCAL_PATH := $(call my-dir)
SENINF_COMMON_SRC = ../../common/v1_1
SENINF_COMMON_SRC_TEST = $(SENINF_COMMON_SRC)/test
SENINF_COMMON_INC = $(LOCAL_PATH)/$(SENINF_COMMON_SRC)
SENINF_COMMON_TEST_INC = $(SENINF_COMMON_INC)/test


include $(CLEAR_VARS)
#-----------------------------------------------------------
LOCAL_SRC_FILES += $(SENINF_COMMON_SRC_TEST)/main_sensor.cpp
LOCAL_C_INCLUDES += $(SENINF_COMMON_TEST_INC)/
LOCAL_C_INCLUDES += $(SENINF_COMMON_INC)/
#
# Note: "/bionic" and "/external/stlport/stlport" is for stlport.
#LOCAL_C_INCLUDES += $(TOP)/bionic
#LOCAL_C_INCLUDES += $(TOP)/external/stlport/stlport
#
#Thread Priority
LOCAL_C_INCLUDES += $(TOP)/system/core/include
#utility

#
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_COMMON)/kernel/imgsensor/inc
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_CUSTOM)/kernel/sensor/inc
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_CUSTOM_PLATFORM)/hal/inc
#for camera_vendor_tags.h:
LOCAL_C_INCLUDES += $(TOP)/system/media/camera/include

#
LOCAL_SHARED_LIBRARIES := \
    libcutils \
    libc++ \
    libutils \
    libmtkcam_stdutils \
    libcam.iopipe \
    libcam.halsensor \
    libmtkcam_metadata

LOCAL_SHARED_LIBRARIES +=
LOCAL_SHARED_LIBRARIES  += libcamdrv_isp
LOCAL_SHARED_LIBRARIES  += libcamdrv_imem
LOCAL_SHARED_LIBRARIES  += libcamdrv_tuning_mgr

#
LOCAL_STATIC_LIBRARIES := \
#    libcam.iopipe.camio

#
LOCAL_WHOLE_STATIC_LIBRARIES := \

#
LOCAL_MODULE := sentest
LOCAL_PROPRIETARY_MODULE := true
LOCAL_MODULE_OWNER := mtk

#
LOCAL_PRELINK_MODULE := false

#
# Start of common part ------------------------------------
-include $(TOP)/$(MTK_PATH_SOURCE)/hardware/mtkcam/mtkcam.mk

MTKCAM_CFLAGS += -DL1_CACHE_BYTES=32
#-----------------------------------------------------------
LOCAL_CFLAGS += $(MTKCAM_CFLAGS)

#-----------------------------------------------------------
LOCAL_C_INCLUDES += $(MTKCAM_C_INCLUDES)

LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_SOURCE)/hardware/gralloc_extra/include
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_SOURCE)/hardware/mtkcam/include

LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_SOURCE)/hardware/mtkcam/drv/include/$(TARGET_BOARD_PLATFORM)
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_SOURCE)/hardware/mtkcam/drv/include/$(TARGET_BOARD_PLATFORM)/drv
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_SOURCE)/hardware/mtkcam/drv/include/$(TARGET_BOARD_PLATFORM)/imageio





LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_SOURCE)/hardware/mtkcam/drv/src/isp/$(TARGET_BOARD_PLATFORM)/inc
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_SOURCE)/hardware/mtkcam/drv/src/isp/$(TARGET_BOARD_PLATFORM)/iopipe/CamIO

LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_COMMON)/kernel/imgsensor/inc
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_SOURCE)/hardware/mtkcam/drv/src/sensor/$(TARGET_BOARD_PLATFORM)
#-----------------------------------------------------------
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_SOURCE)/hardware/include

# End of common part ---------------------------------------
#
include $(MTK_EXECUTABLE)


#
include $(call all-makefiles-under,$(LOCAL_PATH))

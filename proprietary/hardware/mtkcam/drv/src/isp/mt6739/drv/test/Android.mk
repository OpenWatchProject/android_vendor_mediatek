################################################################################
#
################################################################################

LOCAL_PATH := $(call my-dir)

################################################################################
#
################################################################################
include $(CLEAR_VARS)

# Start of common part ------------------------------------
-include $(TOP)/$(MTK_PATH_SOURCE)/hardware/mtkcam/mtkcam.mk

#-----------------------------------------------------------
LOCAL_SRC_FILES += main.cpp
LOCAL_SRC_FILES += main_IspDrv.cpp
#LOCAL_SRC_FILES += $(TOP)/$(MTK_PATH_SOURCE)/hardware/mtkcam/legacy/platform/$(TARGET_BOARD_PLATFORM)/core/imageio/drv/cam/isp_function.cpp

#-----------------------------------------------------------
LOCAL_C_INCLUDES += $(MTKCAM_C_INCLUDES)
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_SOURCE)/hardware/gralloc_extra/include
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_SOURCE)/hardware/mtkcam/legacy/include
#LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_SOURCE)/hardware/dpframework/include
LOCAL_C_INCLUDES += $(TOP)/$(MTK_MTKCAM_PLATFORM)/include

LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_SOURCE)/external/libion_mtk/include
#LOCAL_C_INCLUDES += $(TOP)/$(LOCAL_PATH)/../inc
#LOCAL_C_INCLUDES += $(TOPDIR)/system/core/include
LOCAL_C_INCLUDES += $(TOP)/system/core/libion/include
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_SOURCE)/kernel/include
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_PLATFORM)/external/ldvt/include
#LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_PLATFORM)/kernel/core/include/mach
#LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_CUSTOM)/hal/inc/isp_tuning

LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_SOURCE)/hardware/mtkcam/legacy/platform/$(TARGET_BOARD_PLATFORM)/include/mtkcam/drv
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_SOURCE)/hardware/mtkcam/legacy/platform/$(TARGET_BOARD_PLATFORM)/core/imageio
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_SOURCE)/hardware/mtkcam/legacy/platform/$(TARGET_BOARD_PLATFORM)/core/drv/isp
#LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_SOURCE)/hardware/mtkcam/legacy/platform/$(TARGET_BOARD_PLATFORM)/core/imageio/inc

LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_SOURCE)/hardware/m4u/$(TARGET_BOARD_PLATFORM)

LOCAL_HEADER_LIBRARIES := libhardware_headers

#-----------------------------------------------------------
LOCAL_CFLAGS += $(MTKCAM_CFLAGS)
#
# Add a define value that can be used in the code to indicate that it's using LDVT now.
# For print message function when using LDVT.
# Note: It will be cleared by "CLEAR_VARS", so if this line needed in other module, it
# have to be set in other module again.
ifeq ($(BUILD_MTK_LDVT),true)
    LOCAL_CFLAGS += -DUSING_MTK_LDVT
endif

#-----------------------------------------------------------
LOCAL_WHOLE_STATIC_LIBRARIES +=
#
LOCAL_STATIC_LIBRARIES +=
LOCAL_SHARED_LIBRARIES  += \
    libcamdrv \
    libcutils \
    libutils \

#-----------------------------------------------------------
#LOCAL_MODULE := IspDrv_Test //Temp Marked by Ryan due to HA3 IP-Based build error

LOCAL_PROPRIETARY_MODULE := true
LOCAL_MODULE_OWNER := mtk

LOCAL_PRELINK_MODULE := false

#-----------------------------------------------------------
#include $(MTK_EXECUTABLE) //Temp Marked by Ryan due to HA3 IP-Based build error


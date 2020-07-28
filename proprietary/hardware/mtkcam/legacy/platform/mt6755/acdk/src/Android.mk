#
# libacdk
#
LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)
#-----------------------------------------------------------
-include $(TOP)/$(MTK_PATH_SOURCE)/hardware/mtkcam/mtkcam.mk

#-----------------------------------------------------------
#
LOCAL_SRC_FILES := \
    $(subst $(LOCAL_PATH)/,,$(sort $(wildcard $(LOCAL_PATH)/acdk/*.cpp)))  \
    $(subst $(LOCAL_PATH)/,,$(sort $(wildcard $(LOCAL_PATH)/surfaceview/*.cpp)))

#
LOCAL_C_INCLUDES += \
    $(TOP)/$(MTK_PATH_SOURCE)/kernel/drivers/video \
    $(TOP)/$(MTK_PATH_PLATFORM)/kernel/core/include/mach \
    $(TOP)/$(MTK_PATH_SOURCE)/hardware/m4u/$(PLATFORM)\
    $(TOP)/$(MTK_PATH_SOURCE)/kernel/include \
    $(TOP)/$(MTK_PATH_PLATFORM)/external/ldvt/include \
    $(TOP)/$(MTK_PATH_SOURCE)/hardware/mtkcam/inc \
    $(TOP)/$(MTK_PATH_SOURCE)/hardware/mtkcam/inc/acdk \
    $(TOP)/$(MTK_PATH_SOURCE)/hardware/mtkcam/inc/common \
    $(TOP)/$(MTK_PATH_SOURCE)/hardware/mtkcam/inc/drv \
    $(TOP)/$(MTK_MTKCAM_PLATFORM) \
    $(TOP)/$(MTK_MTKCAM_PLATFORM)/acdk/inc/acdk \
    $(TOP)/$(MTK_PATH_PLATFORM)/hardware \
    $(TOP)/$(MTK_MTKCAM_PLATFORM)/core/drv/imgsensor \
    $(TOP)/$(MTK_MTKCAM_PLATFORM)/core/drv/res_mgr \
    $(TOP)/$(MTK_MTKCAM_PLATFORM)/core/imageio/inc \
    $(TOP)/$(MTK_MTKCAM_PLATFORM)/core/imageio/pipe/inc \
    $(TOP)/$(MTK_MTKCAM_PLATFORM)/v1/hal/adapter/inc \
    $(TOP)/$(MTK_MTKCAM_PLATFORM)/core/featureio/pipe/aaa/isp_tuning \
    $(TOP)/$(MTK_MTKCAM_PLATFORM)/core/featureio/pipe/aaa/ae_mgr \
    $(TOP)/$(MTK_MTKCAM_PLATFORM)/core/featureio/pipe/aaa \
    $(TOP)/$(MTK_PATH_SOURCE)/hardware/m4u/$(PLATFORM)\
    $(TOP)/$(MTK_PATH_PLATFORM)/kernel/core/include/mach \
    $(TOP)/$(MTK_PATH_PLATFORM)/external/ldvt/include \
    $(TOP)/$(MTK_PATH_COMMON)/kernel/imgsensor/inc \
    $(TOP)/$(MTK_PATH_CUSTOM)/hal/inc/isp_tuning \
    $(TOP)/$(MTK_PATH_CUSTOM_PLATFORM)/hal/inc/isp_tuning \
    $(TOP)/$(MTK_PATH_CUSTOM)/hal/inc/aaa \
    $(TOP)/$(MTK_PATH_CUSTOM)/hal/inc/debug_exif/aaa \
    $(TOP)/$(MTK_PATH_SOURCE)/hardware/mtkcam/inc/common/camexif \
    $(TOP)/mediatek/hardware \
    $(TOP)/mediatek/hardware/include \
    $(TOP)/$(MTKCAM_C_INCLUDES) \
    $(TOP)/$(MTK_MTKCAM_PLATFORM)/include//mtkcam \
    $(TOP)/$(MTK_MTKCAM_PLATFORM)/include/ \
    $(TOP)/$(MTK_MTKCAM_PLATFORM)/include//mtkcam/featureio \
    $(TOP)/$(MTK_PATH_CUSTOM_PLATFORM)/hal/inc/aaa \
    $(TOP)/$(MTK_PATH_CUSTOM_PLATFORM)/hal/inc \
    $(TOP)/$(MTK_PATH_CUSTOM_PLATFORM)/hal/inc/debug_exif/aaa \
    $(TOP)/$(MTK_PATH_CUSTOM)/hal/inc \
    $(TOP)/$(MTK_PATH_CUSTOM)/hal/imgsensor \



LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_SOURCE)/hardware/mtkcam/ext/include
# MM DVFS
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_SOURCE)/hardware/bwc/inc

# Add a define value that can be used in the code to indicate that it's using LDVT now.
# For print message function when using LDVT.
# Note: It will be cleared by "CLEAR_VARS", so if it is needed in other module, it
# has to be set in other module again.
LOCAL_WHOLE_STATIC_LIBRARIES := \
    libcct \
#
ifeq ($(BUILD_MTK_LDVT),true)
    LOCAL_CFLAGS += -DUSING_MTK_LDVT
    LOCAL_WHOLE_STATIC_LIBRARIES += libuvvf
endif

LOCAL_SHARED_LIBRARIES := \
    libcutils \
    libimageio \
    libcamdrv \
    libcam.iopipe \
    libmtkcam_stdutils libmtkcam_imgbuf\
    libcam.halsensor \
    libmtkcam_metadata \
    libm4u

LOCAL_SHARED_LIBRARIES += libhardware
LOCAL_SHARED_LIBRARIES += libutils
LOCAL_SHARED_LIBRARIES += libbinder
LOCAL_SHARED_LIBRARIES += libdl

LOCAL_SHARED_LIBRARIES += libmtkcam_fwkutils

ifneq ($(BUILD_MTK_LDVT),true)
LOCAL_SHARED_LIBRARIES += libcamalgo
LOCAL_SHARED_LIBRARIES += libcam.exif
LOCAL_SHARED_LIBRARIES += libfeatureio
LOCAL_SHARED_LIBRARIES += libfeatureiodrv
LOCAL_SHARED_LIBRARIES += libcameracustom
LOCAL_SHARED_LIBRARIES += libcam.camshot
endif
LOCAL_SHARED_LIBRARIES += liblog

# MM DVFS
LOCAL_SHARED_LIBRARIES += libbwc

# libccap.so
LOCAL_SHARED_LIBRARIES += libccap


# Camera Device
include $(TOP)/$(MTK_MTKCAM_PLATFORM)/main/hal/device/1.x/device/device.mk
LOCAL_CFLAGS += -DMTKCAM_HAVE_SENSOR_HAL="$(MTKCAM_HAVE_SENSOR_HAL)"
LOCAL_CFLAGS += -DMTKCAM_HAVE_3A_HAL="$(MTKCAM_HAVE_3A_HAL)"
LOCAL_CFLAGS += -DMTKCAM_HAVE_CAMDRV="$(MTKCAM_HAVE_CAMDRV)"
LOCAL_CFLAGS += -DMTKCAM_HAVE_CPUCTRL="$(MTKCAM_HAVE_CPUCTRL)"
LOCAL_CFLAGS += -DMTKCAM_HAVE_DEVMETAINFO="$(MTKCAM_HAVE_DEVMETAINFO)"
LOCAL_CFLAGS += -DMTKCAM_HAVE_CAM_MANAGER="$(MTKCAM_HAVE_CAM_MANAGER)"
LOCAL_CFLAGS += -DMTKCAM_HR_MONITOR_SUPPORT="$(MTKCAM_HR_MONITOR_SUPPORT)"

LOCAL_C_INCLUDES += $(TOP)/vendor/mediatek/proprietary/hardware/mtkcam/legacy/main/hal/device/1.x
LOCAL_C_INCLUDES += $(MTKCAM_C_INCLUDES)
LOCAL_C_INCLUDES += $(MTK_PATH_SOURCE)/hardware/mtkcam/include
LOCAL_C_INCLUDES += $(MTK_PATH_SOURCE)/hardware/mtkcam/legacy/include
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../../include
LOCAL_C_INCLUDES += $(TOP)/hardware/interfaces/camera/common/1.0/default/include
LOCAL_C_INCLUDES += $(TOP)/system/media/camera/include
LOCAL_C_INCLUDES += $(TOP)/frameworks/native/include/media/openmax
LOCAL_C_INCLUDES += $(TOP)/system/core/include/utils/

LOCAL_SHARED_LIBRARIES += libmtkcam_device1

LOCAL_STATIC_LIBRARIES += libmtkcam_device1_hidlcommon
LOCAL_STATIC_LIBRARIES += android.hardware.camera.common@1.0-helper

LOCAL_SHARED_LIBRARIES += liblog
LOCAL_SHARED_LIBRARIES += libutils
LOCAL_SHARED_LIBRARIES += libcutils
LOCAL_SHARED_LIBRARIES += libhidlbase
LOCAL_SHARED_LIBRARIES += libhidlmemory
LOCAL_SHARED_LIBRARIES += libmtkcam_metadata
LOCAL_SHARED_LIBRARIES += libhidltransport
LOCAL_SHARED_LIBRARIES += android.hardware.camera.device@1.0
LOCAL_SHARED_LIBRARIES += android.hardware.camera.common@1.0

LOCAL_SHARED_LIBRARIES += android.hardware.graphics.mapper@2.0
LOCAL_SHARED_LIBRARIES += android.hardware.graphics.mapper@3.0
LOCAL_SHARED_LIBRARIES += android.hidl.allocator@1.0
LOCAL_SHARED_LIBRARIES += android.hidl.memory@1.0

LOCAL_SHARED_LIBRARIES += vendor.mediatek.hardware.camera.device@1.1

# from DefaultCameraDevice
LOCAL_SHARED_LIBRARIES += libmtkcam_fwkutils
LOCAL_SHARED_LIBRARIES += libcam_hwutils

LOCAL_SHARED_LIBRARIES += libcam.paramsmgr
LOCAL_SHARED_LIBRARIES += libcam.client
LOCAL_SHARED_LIBRARIES += libcam.camadapter


#
LOCAL_PRELINK_MODULE := false

#
LOCAL_MODULE := libacdk
LOCAL_PROPRIETARY_MODULE := true
LOCAL_MODULE_OWNER := mtk

#

#
# Start of common part ------------------------------------
sinclude $(TOP)/$(MTK_PATH_SOURCE)/hardware/mtkcam/mtkcam.mk

#-----------------------------------------------------------
LOCAL_CFLAGS += $(MTKCAM_CFLAGS)

#-----------------------------------------------------------
LOCAL_C_INCLUDES += $(MTKCAM_C_INCLUDES)
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_SOURCE)/hardware/gralloc_extra/include
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_SOURCE)/hardware/mtkcam/ext/include
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_SOURCE)/hardware/mtkcam/include


#-----------------------------------------------------------
LOCAL_C_INCLUDES += $(TOP)/$(MTKCAM_C_INCLUDES)/..
LOCAL_C_INCLUDES += $(TOP)/$(MTK_MTKCAM_PLATFORM)/include/
LOCAL_C_INCLUDES += $(TOP)/system/media/camera/include
# End of common part ---------------------------------------
#
include $(MTK_SHARED_LIBRARY)

#
include $(call all-makefiles-under, $(LOCAL_PATH))

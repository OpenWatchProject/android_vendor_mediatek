#
# libcamdrv_isp
#
LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)
-include $(TOP)/$(MTK_PATH_SOURCE)/hardware/mtkcam/mtkcam.mk
#
LOCAL_SRC_FILES := isp_drv.cpp

# camutils: For CameraProfile APIs.
#
LOCAL_C_INCLUDES := \
    $(TOP)/$(MTK_MTKCAM_PLATFORM)/inc \
    $(TOP)/$(MTK_MTKCAM_PLATFORM)/inc/drv \
    $(TOP)/$(MTK_MTKCAM_PLATFORM)/core/ \
    $(TOP)/$(MTK_MTKCAM_PLATFORM)/core/drv_common/inc \
    $(TOP)/$(MTK_PATH_PLATFORM)/external/ldvt/include \
    $(TOP)/$(MTK_PATH_PLATFORM)/kernel/core/include/mach \
    $(TOP)/$(MTK_PATH_CUSTOM)/hal/inc/isp_tuning \
    $(TOP)/mediatek/source/external/mhal/src/core/drv/inc \
    $(MTK_PATH_PLATFORM)hardware/m4u \
    $(TOP)/$(MTK_MTKCAM_PLATFORM)/inc/common/camutils \
    $(TOP)/device/mediatek/sprout/kernel-headers

#-----------------------------------------------------------
ifeq ($(TARGET_BUILD_VARIANT), user)
MTKCAM_LOGENABLE_DEFAULT   := 0
else
MTKCAM_LOGENABLE_DEFAULT   := 1
endif
LOCAL_CFLAGS += -DMTKCAM_LOGENABLE_DEFAULT=$(MTKCAM_LOGENABLE_DEFAULT)
#-----------------------------------------------------------

# Add a define value that can be used in the code to indicate that it's using LDVT now.
# For print message function when using LDVT.
# Note: It will be cleared by "CLEAR_VARS", so if this line needed in other module, it
# have to be set in other module again.
ifeq ($(BUILD_MTK_LDVT),yes)
    LOCAL_CFLAGS += -DUSING_MTK_LDVT
endif

#
LOCAL_STATIC_LIBRARIES := \

#
LOCAL_WHOLE_STATIC_LIBRARIES := \

#
#LOCAL_SHARED_LIBRARIES := \
#    libcutils \

#
LOCAL_MODULE := libcamdrv_isp
LOCAL_PROPRIETARY_MODULE := true
LOCAL_MODULE_OWNER := mtk

#
LOCAL_PRELINK_MODULE := false

#

#
#LOCAL_MULTILIB := 32
#
# Start of common part ------------------------------------

LOCAL_HEADER_LIBRARIES := libutils_headers libhardware_headers
#-----------------------------------------------------------
LOCAL_CFLAGS += $(MTKCAM_CFLAGS)

#-----------------------------------------------------------
LOCAL_C_INCLUDES += $(MTKCAM_C_INCLUDES)
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_SOURCE)/hardware/gralloc_extra/include
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_SOURCE)/hardware/mtkcam/ext/include


#-----------------------------------------------------------
LOCAL_C_INCLUDES += $(TOP)/$(MTKCAM_C_INCLUDES)/..
LOCAL_C_INCLUDES += $(TOP)/$(MTKCAM_C_INCLUDES)
LOCAL_C_INCLUDES += $(TOP)/$(MTK_MTKCAM_PLATFORM)/include

# End of common part ---------------------------------------
#
include $(MTK_STATIC_LIBRARY)



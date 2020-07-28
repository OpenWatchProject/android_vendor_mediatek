#
# libacdk
#
LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)
-include $(TOP)/$(MTK_PATH_SOURCE)/hardware/mtkcam/mtkcam.mk
-include $(TOP)/$(MTK_PATH_SOURCE)/hardware/mtkcam/drv/driver.mk

#
ACDK_BUILD_PURE_SMT := yes
ifeq ($(TARGET_BOARD_PLATFORM), none)
    ifeq ($(MTK_ACDK_BUILD_DUMMY_ENG), yes)
        ACDK_BUILD_DUMMY_ENG := yes
    else
        ACDK_BUILD_DUMMY_ENG := no
    endif
else
    ACDK_BUILD_DUMMY_ENG := no
endif

LOCAL_SRC_FILES += \
    acdk/AcdkBase.cpp \
    acdk/AcdkIF.cpp \
    acdk/AcdkMain.cpp \
    acdk/AcdkMhalBase.cpp \
    acdk/AcdkMhalEng.cpp \
    acdk/AcdkMhalPure.cpp \
    acdk/AcdkMhalVC.cpp \
    acdk/AcdkUtility.cpp \

LOCAL_SRC_FILES += \
    surfaceview/AcdkSurfaceView.cpp \
    surfaceview/surfaceView.cpp \

$(info MTK_PATH_SOURCE      : $(MTK_PATH_SOURCE))
$(info MTK_PATH_COMMON      : $(MTK_PATH_COMMON))
$(info MTK_PATH_CUSTOM      : $(MTK_PATH_CUSTOM))
$(info MTK_PATH_CUSTOM_PLATFORM : $(MTK_PATH_CUSTOM_PLATFORM))
$(info PLATFORM_SDK_VERSION  : $(PLATFORM_SDK_VERSION))

# Note: "/bionic" and "/external/stlport/stlport" is for stlport.
LOCAL_C_INCLUDES += \
    $(LOCAL_PATH)/../inc/acdk \
    $(TOP)/$(MTK_PATH_SOURCE)/kernel/drivers/video \
    $(TOP)/$(MTK_PATH_SOURCE)/kernel/include \
    $(TOP)/hardware/libhardware/include/ \
    $(TOP)/$(MTK_PATH_SOURCE)/hardware/ldvt/$(TARGET_BOARD_PLATFORM)/include \
    $(TOP)/$(MTK_PATH_CUSTOM_PLATFORM)/hal/inc/isp_tuning \
    $(TOP)/$(MTK_PATH_CUSTOM_PLATFORM)/hal/inc/aaa \
    $(TOP)/$(MTK_PATH_CUSTOM_PLATFORM)/hal/inc \
    $(TOP)/$(MTK_PATH_CUSTOM_PLATFORM)/hal/inc/debug_exif/aaa \
    $(TOP)/$(MTK_PATH_COMMON)/kernel/imgsensor/inc \

LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_SOURCE)/hardware/mtkcam/drv/include/$(MTKCAM_DRV_PLATFORM)/drv

ifeq ($(MTK_M4U_SUPPORT),yes)
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_SOURCE)/hardware/m4u/$(TARGET_BOARD_PLATFORM)
endif
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_SOURCE)/hardware/jpeg/include
LOCAL_C_INCLUDES += $(MTK_PATH_SOURCE)/frameworks/av/include

ifeq ($(MTK_ION_SUPPORT), yes)
LOCAL_CFLAGS += -DUSING_MTK_ION
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_SOURCE)/external/libion_mtk/include

endif

# Add a define value that can be used in the code to indicate that it's using LDVT now.
# For print message function when using LDVT.
# Note: It will be cleared by "CLEAR_VARS", so if it is needed in other module, it
# has to be set in other module again.

ifeq ($(BUILD_MTK_LDVT),yes)
    LOCAL_CFLAGS += -DUSING_MTK_LDVT
    LOCAL_WHOLE_STATIC_LIBRARIES += libuvvf
endif

LOCAL_SHARED_LIBRARIES := \
    libcutils \
    libimageio \
    libcam.iopipe \
    libmtkcam_stdutils libmtkcam_imgbuf \
    libcam.halsensor \
    libmtkcam_metadata

ifeq ($(MTK_M4U_SUPPORT),yes)
LOCAL_SHARED_LIBRARIES += libm4u
endif

LOCAL_SHARED_LIBRARIES += libhardware
LOCAL_SHARED_LIBRARIES += libutils
LOCAL_SHARED_LIBRARIES += libdl

LOCAL_SHARED_LIBRARIES += libmtkcam_fwkutils
LOCAL_SHARED_LIBRARIES += libmtkcam_modulehelper
ifeq ($(MTK_ION_SUPPORT),yes)
LOCAL_SHARED_LIBRARIES += libion libion_mtk
endif

LOCAL_SHARED_LIBRARIES += libmtkcam_imem

ifeq ($(ACDK_BUILD_DUMMY_ENG),no)
LOCAL_SHARED_LIBRARIES += libmtkcam_metastore
LOCAL_SHARED_LIBRARIES += libJpgEncPipe
endif

LOCAL_SHARED_LIBRARIES += liblog
LOCAL_SHARED_LIBRARIES += libmtkcam_ulog
#
LOCAL_PRELINK_MODULE := false

#
LOCAL_MODULE := libacdk
LOCAL_PROPRIETARY_MODULE := true
LOCAL_MODULE_OWNER := mtk
LOCAL_MULTILIB := first
#

#
# Start of common part ------------------------------------
-include $(TOP)/$(MTK_PATH_SOURCE)/hardware/mtkcam/mtkcam.mk

#-----------------------------------------------------------
LOCAL_CFLAGS += $(MTKCAM_CFLAGS)

#LOCAL_CFLAGS += -DACDK_CAMERA_3A
#LOCAL_CFLAGS += -DACDK_FAKE_SENSOR
#LOCAL_CFLAGS += -DACDK_BYPASS_P2

ifeq ($(ACDK_BUILD_DUMMY_ENG),yes)
    LOCAL_CFLAGS += -DACDK_DUMMY_ENG
endif
ifeq ($(ACDK_BUILD_PURE_SMT),yes)
    LOCAL_CFLAGS += -DACDK_PURE_SMT
endif

ifneq (,$(filter $(strip $(TARGET_BOARD_PLATFORM)), mt6739 mt6761))
    LOCAL_CFLAGS += -DACDK_IPBASE_ISP30_SUPPORT
endif

ifneq (,$(filter $(strip $(TARGET_BOARD_PLATFORM)), mt6789))
    LOCAL_CFLAGS += -DACDK_ENABLE_VC_OUTPUT
endif
#-----------------------------------------------------------
LOCAL_C_INCLUDES += $(MTKCAM_C_INCLUDES)

LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_SOURCE)/hardware/gralloc_extra/include
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_SOURCE)/hardware/mtkcam/include


#-----------------------------------------------------------
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_SOURCE)/hardware/include
LOCAL_C_INCLUDES += $(TOP)/system/media/camera/include
# End of common part ---------------------------------------
#
include $(MTK_SHARED_LIBRARY)

#
ifeq ($(ACDK_BUILD_DUMMY_ENG),no)
include $(call all-makefiles-under, $(LOCAL_PATH))
else
include $(LOCAL_PATH)/test/Android.mk
endif

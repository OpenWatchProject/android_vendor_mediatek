################################################################################
#
################################################################################

LOCAL_PATH := $(call my-dir)

################################################################################
#
################################################################################
include $(CLEAR_VARS)

#-----------------------------------------------------------
-include $(TOP)/$(MTK_PATH_SOURCE)/hardware/mtkcam/mtkcam.mk

#-----------------------------------------------------------
LOCAL_SRC_FILES += \
    CamShotImp.cpp \
    SingleShot/ISingleShotBridge.cpp \
    SingleShot/SingleShot.cpp \
    BurstShot/BurstShot.cpp \
    BurstShot/IBurstShotBridge.cpp

ifneq ($(MTK_BASIC_PACKAGE), yes)
LOCAL_SRC_FILES += \
    MultiShot/IMultiShotBridge.cpp \
    MultiShot/ImageCreateThread.cpp \
    MultiShot/MultiShot.cpp \
    MultiShot/MultiShotCc.cpp \
    MultiShot/MultiShotNcc.cpp
endif

#-----------------------------------------------------------
LOCAL_HEADER_LIBRARIES += libmtkcam_headers
LOCAL_C_INCLUDES += $(MTKCAM_C_INCLUDES)
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_SOURCE)/hardware/gralloc_extra/include
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_SOURCE)/hardware/mtkcam/ext/include

#
LOCAL_C_INCLUDES +=$(TOP)/$(MTK_MTKCAM_PLATFORM)/include
LOCAL_C_INCLUDES +=$(TOP)/$(MTKCAM_C_INCLUDES)
LOCAL_C_INCLUDES +=$(TOP)/$(MTKCAM_C_INCLUDES)/..

LOCAL_C_INCLUDES += $(TOP)/$(MTK_MTKCAM_PLATFORM)/include/
#
# camera Hardware
LOCAL_C_INCLUDES += $(TOP)/$(MTK_MTKCAM_PLATFORM)
LOCAL_C_INCLUDES += $(TOP)/$(MTK_MTKCAM_PLATFORM)/v1/hal/adapter/inc
# ifneq "'1'" "$(strip $(MTKCAM_REMOVE_LIBMTKJPEG))"
# LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_SOURCE)/external/mtkjpeg/inc
# LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_SOURCE)/external/mtkjpeg/include
# endif
LOCAL_C_INCLUDES += $(TOP)/system/media/camera/include

LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_SOURCE)/hardware/mtkcam/include

# m4u
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_SOURCE)/hardware/m4u/$(PLATFORM)

# for SwNR
LOCAL_C_INCLUDES += $(TOP)/$(MTK_MTKCAM_PLATFORM)/include/mtkcam/featureio
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_CUSTOM_PLATFORM)/hal/D2/inc

#-----------------------------------------------------------
LOCAL_CFLAGS += $(MTKCAM_CFLAGS)

#-----------------------------------------------------------
LOCAL_WHOLE_STATIC_LIBRARIES += libcam.camshot.simager
LOCAL_WHOLE_STATIC_LIBRARIES += libcam.camshot.utils
#
LOCAL_STATIC_LIBRARIES +=

#-----------------------------------------------------------
LOCAL_SHARED_LIBRARIES += liblog
LOCAL_SHARED_LIBRARIES += libutils
LOCAL_SHARED_LIBRARIES += libcutils
LOCAL_SHARED_LIBRARIES += libion
#
LOCAL_SHARED_LIBRARIES += libcam.campipe
LOCAL_SHARED_LIBRARIES += libcamdrv
#
LOCAL_SHARED_LIBRARIES += libdpframework
#
# ifneq "'1'" "$(strip $(MTKCAM_REMOVE_LIBMTKJPEG))"
# LOCAL_SHARED_LIBRARIES += libmtkjpeg
# endif
#
# for jpeg enc use
LOCAL_SHARED_LIBRARIES += libm4u libJpgEncPipe
#
# for 3A
LOCAL_SHARED_LIBRARIES += libfeatureio
#
# camUtils
LOCAL_SHARED_LIBRARIES += libcam_mmp
LOCAL_SHARED_LIBRARIES += libcam.utils
LOCAL_SHARED_LIBRARIES += libcam.halsensor
#
# for SwNR
LOCAL_SHARED_LIBRARIES += libmtkcam_stdutils libmtkcam_imgbuf
LOCAL_SHARED_LIBRARIES += libcameracustom

#-----------------------------------------------------------
LOCAL_MODULE := libcam.camshot
LOCAL_PROPRIETARY_MODULE := true
LOCAL_MODULE_OWNER := mtk

#-----------------------------------------------------------
include $(MTK_SHARED_LIBRARY)


################################################################################
#
################################################################################
include $(CLEAR_VARS)
include $(call all-makefiles-under,$(LOCAL_PATH))


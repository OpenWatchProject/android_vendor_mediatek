#
# libcam.campipe
#
LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

-include $(TOP)/$(MTK_PATH_SOURCE)/hardware/mtkcam/mtkcam.mk

LOCAL_SRC_FILES := \
    PipeImp.cpp \
    CamIOPipe/CamIOPipe.cpp \
    CamIOPipe/ICamIOPipeBridge.cpp \
    XdpPipe/IXdpPipeBridge.cpp \
    XdpPipe/XdpPipe.cpp \
    PostProcPipe/IPostProcPipeBridge.cpp \
    PostProcPipe/PostProcPipe.cpp \
    utils/CampipeImgioPipeMapper.cpp

# camera Hardware
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_COMMON)/kernel/imgsensor/inc
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_SOURCE)/frameworks/base/include
LOCAL_C_INCLUDES += $(TOP)/$(MTK_MTKCAM_PLATFORM)/
LOCAL_C_INCLUDES += $(TOP)/$(MTK_MTKCAM_PLATFORM)/inc
LOCAL_C_INCLUDES += $(TOP)/$(MTK_MTKCAM_PLATFORM)/inc/common
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_SOURCE)/hardware/mtkcam/
LOCAL_C_INCLUDES += $(TOP)/$(MTK_MTKCAM_PLATFORM)/include/mtkcam/drv

LOCAL_C_INCLUDES += $(TOP)/$(MTK_MTKCAM_PLATFORM)/inc/drv
#

LOCAL_SHARED_LIBRARIES := \
    libcutils \
    liblog \
    libimageio \
    libcamdrv \

#use MDP
LOCAL_SHARED_LIBRARIES += libdpframework
LOCAL_SHARED_LIBRARIES += libcam.halsensor
#
LOCAL_STATIC_LIBRARIES := \

#
LOCAL_WHOLE_STATIC_LIBRARIES := \
    libcampipe_plat_pipe_mgr \




LOCAL_MODULE := libcam.campipe
LOCAL_PROPRIETARY_MODULE := true
LOCAL_MODULE_OWNER := mtk
LOCAL_MULTILIB := 32
#

#

#
# Start of common part ------------------------------------
sinclude $(TOP)/$(MTK_PATH_SOURCE)/hardware/mtkcam/mtkcam.mk

LOCAL_HEADER_LIBRARIES := libutils_headers libcutils_headers libhardware_headers
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
LOCAL_C_INCLUDES += $(TOP)/$(MTKCAM_C_INCLUDES)

# End of common part ---------------------------------------
#
include $(MTK_SHARED_LIBRARY)
include $(call all-makefiles-under,$(LOCAL_PATH))

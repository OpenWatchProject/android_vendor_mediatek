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
LOCAL_SRC_FILES += imem_drv.cpp
#LOCAL_SRC_FILES += hwutils.cpp

#-----------------------------------------------------------
LOCAL_C_INCLUDES += $(MTKCAM_C_INCLUDES)

LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_SOURCE)/hardware/mtkcam/include

LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_SOURCE)/hardware/mtkcam/aaa/include
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_SOURCE)/external/include
#
LOCAL_C_INCLUDES += $(TOPDIR)system/core/include

#
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_SOURCE)/kernel/include
#
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_SOURCE)/hardware/m4u/$(MTKCAM_MTK_PLATFORM)
#-----------------------------------------------------------
ifeq ($(TARGET_BUILD_VARIANT), user)
MTKCAM_LOGENABLE_DEFAULT   := 0
else
MTKCAM_LOGENABLE_DEFAULT   := 1
endif

#-----------------------------------------------------------
#-----------------------------------------------------------
LOCAL_CFLAGS += $(MTKCAM_CFLAGS)
#
# Add a define value that can be used in the code to indicate that it's using LDVT now.
# For print message function when using LDVT.
# Note: It will be cleared by "CLEAR_VARS", so if it is needed in other module, it
# has to be set in other module again.
ifeq ($(BUILD_MTK_LDVT),yes)
    LOCAL_CFLAGS += -DUSING_MTK_LDVT
    LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_SOURCE)/hardware/ldvt/$(TARGET_BOARD_PLATFORM)/include
    LOCAL_WHOLE_STATIC_LIBRARIES += libuvvf
endif

#-----------------------------------------------------------
LOCAL_CFLAGS += -DUSING_MTK_ION
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_SOURCE)/external/libion_mtk/include

#-----------------------------------------------------------
LOCAL_SHARED_LIBRARIES += liblog
LOCAL_SHARED_LIBRARIES += libcutils

ifeq ($(MTK_ION_SUPPORT),yes)
	LOCAL_SHARED_LIBRARIES += libion libion_mtk
endif

ifneq ($(BUILD_MTK_LDVT),yes)
    LOCAL_SHARED_LIBRARIES += libmtkcam_stdutils
endif

#-----------------------------------------------------------
LOCAL_MODULE := libcam3a_imem
LOCAL_PROPRIETARY_MODULE := true
LOCAL_MODULE_OWNER := mtk


#-----------------------------------------------------------
include $(MTK_SHARED_LIBRARY)


################################################################################
#
################################################################################
#include $(CLEAR_VARS)
#include $(call all-makefiles-under,$(LOCAL_PATH))


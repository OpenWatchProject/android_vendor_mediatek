#
# camshottest
#
LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

# don't build this under ldvt
ifeq "'1'" "$(strip $(MTKCAM_USE_LEGACY_SIMAGER))"
#-----------------------------------------------------------
-include $(TOP)/$(MTK_PATH_SOURCE)/hardware/mtkcam/mtkcam.mk
-include $(TOP)/$(MTK_PATH_SOURCE)/hardware/mtkcam/drv/driver.mk
#
LOCAL_SRC_FILES := \
    test_simager.cpp \

#-----------------------------------------------------------
LOCAL_C_INCLUDES += $(MTKCAM_C_INCLUDES)

LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_SOURCE)/hardware/gralloc_extra/include
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_SOURCE)/hardware/mtkcam/include

LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_SOURCE)/hardware/include

#
#
# Note: "/bionic" and "/external/stlport/stlport" is for stlport.
#LOCAL_C_INCLUDES += $(TOP)/bionic
#LOCAL_C_INCLUDES += $(TOP)/external/stlport/stlport
#
# camera Hardware

#-----------------------------------------------------------
LOCAL_CFLAGS += $(MTKCAM_CFLAGS)
#
# vector
LOCAL_SHARED_LIBRARIES += libcutils

# log
LOCAL_SHARED_LIBRARIES += liblog

LOCAL_SHARED_LIBRARIES += libc++

# Simager
LOCAL_SHARED_LIBRARIES += \
    libcam.iopipe \

#utils
LOCAL_SHARED_LIBRARIES += \
    libmtkcam_stdutils libmtkcam_imgbuf

LOCAL_HEADER_LIBRARIES += \
    libutils_headers libhardware_headers

## Imem

#
LOCAL_STATIC_LIBRARIES := \

#
LOCAL_WHOLE_STATIC_LIBRARIES := \

#
LOCAL_MODULE := simagertest
LOCAL_PROPRIETARY_MODULE := true
LOCAL_MODULE_OWNER := mtk

#
LOCAL_MODULE_TAGS := optional
#
LOCAL_PRELINK_MODULE := false

#
include $(MTK_EXECUTABLE)

#
#include $(call all-makefiles-under,$(LOCAL_PATH))
endif

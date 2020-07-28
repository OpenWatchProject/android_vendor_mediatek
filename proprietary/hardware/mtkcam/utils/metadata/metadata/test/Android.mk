
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
LOCAL_SRC_FILES += test_metadata.cpp

#-----------------------------------------------------------
LOCAL_C_INCLUDES += $(MTKCAM_C_INCLUDES)
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_SOURCE)/hardware/mtkcam/include
LOCAL_C_INCLUDES += $(TOP)/external/googletest/googletest/include
LOCAL_C_INCLUDES += $(TOPDIR)vendor/mediatek/proprietary/external/libudf/libladder
#
#-----------------------------------------------------------
LOCAL_CFLAGS += $(MTKCAM_CFLAGS)
#
#-----------------------------------------------------------
LOCAL_WHOLE_STATIC_LIBRARIES +=
#
#-----------------------------------------------------------
LOCAL_STATIC_LIBRARIES +=
LOCAL_STATIC_LIBRARIES += libgtest
#
#-----------------------------------------------------------
LOCAL_SHARED_LIBRARIES += liblog
LOCAL_SHARED_LIBRARIES += libmtkcam_ulog
LOCAL_SHARED_LIBRARIES += libutils
LOCAL_SHARED_LIBRARIES += libladder
LOCAL_SHARED_LIBRARIES += libcutils
LOCAL_SHARED_LIBRARIES += libmtkcam_stdutils

#LOCAL_SHARED_LIBRARIES += libmtkcam_metadata
# use static library for download testing#################################################
LOCAL_SHARED_LIBRARIES += libaedv
LOCAL_SHARED_LIBRARIES += libhidlbase
LOCAL_SHARED_LIBRARIES += libcamera_metadata
LOCAL_STATIC_LIBRARIES += libmtkcam_metadata.container
LOCAL_STATIC_LIBRARIES += libmtkcam_metadata.converter
LOCAL_STATIC_LIBRARIES += libmtkcam_metadata.vendortag
LOCAL_STATIC_LIBRARIES += libmtkcam_metadata.getDumpFilenamePrefix
# use static library for download testing#################################################

#-----------------------------------------------------------
LOCAL_MODULE := camtest_metadata
LOCAL_PROPRIETARY_MODULE := true
LOCAL_MODULE_OWNER := mtk
#
#-----------------------------------------------------------
LOCAL_PRELINK_MODULE := false

#-----------------------------------------------------------
include $(MTK_EXECUTABLE)

#-----------------------------------------------------------
#include $(call all-makefiles-under,$(LOCAL_PATH))



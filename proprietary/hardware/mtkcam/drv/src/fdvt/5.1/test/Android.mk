#
# FDVT_Test
#
LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

#
LOCAL_SRC_FILES := \
    main.cpp \
    main_fdvtstream.cpp \

-include $(TOP)/$(MTK_PATH_SOURCE)/hardware/mtkcam/mtkcam.mk
-include $(TOP)/$(MTK_PATH_SOURCE)/hardware/mtkcam/drv/driver.mk

# vector
LOCAL_SHARED_LIBRARIES := \
    libcutils \
    libutils

#LOCAL_SHARED_LIBRARIES += libstdc++

LOCAL_SHARED_LIBRARIES += libmtkcam_fdvt
LOCAL_SHARED_LIBRARIES += libdip_imem
LOCAL_SHARED_LIBRARIES += libmtkcam_imgbuf
LOCAL_SHARED_LIBRARIES += liblog

ifneq ($(BUILD_MTK_LDVT),yes)
    LOCAL_SHARED_LIBRARIES += libmtkcam_stdutils
endif

ifeq ($(BUILD_MTK_LDVT),yes)
    LOCAL_WHOLE_STATIC_LIBRARIES += libuvvf
    LOCAL_CFLAGS += -DUSING_MTK_LDVT
endif

LOCAL_STATIC_LIBRARIES := \

LOCAL_WHOLE_STATIC_LIBRARIES := \

LOCAL_MODULE := Fdvt_Test
LOCAL_PROPRIETARY_MODULE := true
LOCAL_MODULE_OWNER := mtk

LOCAL_PRELINK_MODULE := false

LOCAL_HEADER_LIBRARIES :=

LOCAL_HEADER_LIBRARIES += $(MTKCAM_INCLUDE_HEADER_LIB)
LOCAL_HEADER_LIBRARIES += libmtkcam_headers

#-----------------------------------------------------------
ifeq ($(TARGET_BUILD_VARIANT), user)
MTKCAM_LOGENABLE_DEFAULT   := 0
else
MTKCAM_LOGENABLE_DEFAULT   := 1
endif

#-----------------------------------------------------------
LOCAL_CFLAGS += -DMTKCAM_LOGENABLE_DEFAULT=$(MTKCAM_LOGENABLE_DEFAULT)
# End of common part ---------------------------------------
#
include $(BUILD_EXECUTABLE)

#
#include $(call all-makefiles-under,$(LOCAL_PATH))

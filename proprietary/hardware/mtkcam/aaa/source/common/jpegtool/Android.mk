LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)
#-----------------------------------------------------------
LOCAL_SRC_FILES += jpegtool.cpp
LOCAL_SHARED_LIBRARIES += \
    liblog \
    libcutils \
    libutils
#-----------------------------------------------------------
#LOCAL_SHARED_LIBRARIES += libjpeg
#LOCAL_C_INCLUDES += external/jpeg
LOCAL_SHARED_LIBRARIES += libjpeg-alpha_vendor
#-----------------------------------------------------------
LOCAL_PRELINK_MODULE := false

LOCAL_MODULE := jpegtool
LOCAL_MODULE_OWNER := mtk
LOCAL_PROPRIETARY_MODULE := true
#-----------------------------------------------------------
include $(MTK_EXECUTABLE)

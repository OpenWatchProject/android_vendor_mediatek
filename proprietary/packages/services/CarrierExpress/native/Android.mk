LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)


LOCAL_SRC_FILES := src/usp_main.c

LOCAL_SHARED_LIBRARIES := libcutils liblog

LOCAL_MODULE := usp_service
LOCAL_MULTILIB := 32

LOCAL_MODULE_TAGS := optional

# For usp service init.rc
LOCAL_INIT_RC := usp_service.rc

include $(BUILD_EXECUTABLE)

###################
#LOCAL_PATH := $(call my-dir)

#include $(CLEAR_VARS)


#LOCAL_SRC_FILES := src/usp_native.cpp

#LOCAL_SHARED_LIBRARIES := libcutils liblog

#LOCAL_C_INCLUDES := $(MTK_PATH_SOURCE)/hardware/ccci/include/

#LOCAL_MODULE := libusp_native

#include $(BUILD_SHARED_LIBRARY)

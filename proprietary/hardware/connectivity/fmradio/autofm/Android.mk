$(warning [FM]starting to build autofm)  

BUILD_FMTEST  := true
FM_TOOL_BUILD_DEAMON := false

LOCAL_PATH := $(call my-dir)

ifeq ($(BUILD_FMTEST), true)
include $(CLEAR_VARS)

LOCAL_SRC_FILES := \
	autofm.c

LOCAL_STATIC_LIBRARIES := 

#LOCAL_SHARED_LIBRARIES := libfm

LOCAL_C_INCLUDES := 

LOCAL_PRELINK_MODULE := yes
LOCAL_PROPRIETARY_MODULE := true
LOCAL_MODULE_ONWER := mtk

LOCAL_CFLAGS += -Wno-date-time

ifeq ($(FM_TOOL_BUILD_DEAMON), true)
LOCAL_CFLAGS += -DFM_TOOL_BUILD_DEAMON

LOCAL_MODULE := autofmd
else
LOCAL_MODULE := autofm
endif

include $(BUILD_EXECUTABLE)
endif

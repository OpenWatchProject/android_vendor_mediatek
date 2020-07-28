LOCAL_PATH := $(call my-dir)

# =============================================================================
# Module: libTEECommon, alias of libMcClient.so
# =============================================================================

include $(CLEAR_VARS)

LOCAL_MODULE := libTEECommon
LOCAL_MODULE_OWNER := mtk
LOCAL_MODULE_TAGS := optional
LOCAL_PROPRIETARY_MODULE := true

LOCAL_CFLAGS := -fvisibility=hidden
LOCAL_CFLAGS += -DTBASE_API_LEVEL=9
LOCAL_CFLAGS += -Wall -Wextra
LOCAL_CFLAGS += -std=c++11

LOCAL_C_INCLUDES := \
	$(LOCAL_PATH)/include \
	$(LOCAL_PATH)/include/GP

ifeq ($(APP_PROJECT_PATH),)
LOCAL_SHARED_LIBRARIES := \
	liblog

ifdef TRUSTONIC_ANDROID_LEGACY_SUPPORT
include external/stlport/libstlport.mk

LOCAL_C_INCLUDES += \
	external/stlport/stlport

LOCAL_SHARED_LIBRARIES += \
	libstlport
endif # !TRUSTONIC_ANDROID_LEGACY_SUPPORT
else # !NDK
LOCAL_LDLIBS := -llog

endif # NDK

LOCAL_SRC_FILES := \
	src/client_log.cpp \
	src/common.cpp \
	src/driver.cpp \
	src/native_interface.cpp

LOCAL_HEADER_LIBRARIES += liblog_headers
LOCAL_EXPORT_C_INCLUDES := $(LOCAL_C_INCLUDES)
LOCAL_EXPORT_C_INCLUDE_DIRS := $(LOCAL_EXPORT_C_INCLUDES)

include $(BUILD_SHARED_LIBRARY)

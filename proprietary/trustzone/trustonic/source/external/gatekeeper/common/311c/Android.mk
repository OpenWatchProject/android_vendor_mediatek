# =============================================================================
#
# HAL implementation of Android Gatekeeper
#
# =============================================================================

ifndef TRUSTONIC_ANDROID_LEGACY_SUPPORT

LOCAL_PATH := $(call my-dir)

# =============================================================================
#
# Module: libMcGatekeeper.so - Client library for Android authorization
# framework
#
# =============================================================================
include $(CLEAR_VARS)

LOCAL_MODULE := libMcGatekeeper
LOCAL_PROPRIETARY_MODULE := true
LOCAL_MODULE_OWNER := mtk
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_RELATIVE_PATH := hw

LOCAL_CFLAGS := -Wall -Werror -Wmissing-field-initializers -Wunused-parameter
# Software implementation is written in C++11
LOCAL_CPPFLAGS := -std=c++11
LOCAL_CPPFLAGS += -fpermissive -Wall -Werror -D__STDC_LIMIT_MACROS
LOCAL_CPPFLAGS += -D__FAILURE_RECORD_STORAGE_IN_SECURE_OBJECT__
LOCAL_LDLIBS := -llog

ALL_SRC_FILES := $(wildcard ${LOCAL_PATH}/src/*.c ${LOCAL_PATH}/src/*.cpp)
LOCAL_SRC_FILES := $(ALL_SRC_FILES:$(LOCAL_PATH)/%=%)

LOCAL_C_INCLUDES := \
    $(LOCAL_PATH)/include \
    $(MOBICORE_LIB_PATH)

LOCAL_SHARED_LIBRARIES := libMcClient
LOCAL_SHARED_LIBRARIES += libgatekeeper

LOCAL_HEADER_LIBRARIES := libhardware_headers

LOCAL_POST_INSTALL_CMD = \
    $(hide) ln -sf $(notdir $(LOCAL_INSTALLED_MODULE)) $(dir $(LOCAL_INSTALLED_MODULE))gatekeeper.$(TARGET_DEVICE).so ; \
            ln -sf $(notdir $(LOCAL_INSTALLED_MODULE)) $(dir $(LOCAL_INSTALLED_MODULE))gatekeeper.$(TARGET_BOARD_PLATFORM).so

include $(BUILD_SHARED_LIBRARY)

endif # !TRUSTONIC_ANDROID_LEGACY_SUPPORT

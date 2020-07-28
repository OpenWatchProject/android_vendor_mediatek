############################################################################
# Native APM HIDL server, runs in libdmi_core
# Native static library
############################################################################
LOCAL_PATH:= $(call my-dir)

include $(CLEAR_VARS)
LOCAL_ARM_MODE := arm
LOCAL_MODULE := libapmonitor_server
LOCAL_C_INCLUDES := $(LOCAL_PATH)/include
LOCAL_HEADER_LIBRARIES := dmc_core_public_headers
LOCAL_EXPORT_C_INCLUDE_DIRS += $(LOCAL_PATH)/include

LOCAL_SRC_FILES := $(subst $(LOCAL_PATH)/,,$(wildcard $(LOCAL_PATH)/src/*.cpp))
LOCAL_SHARED_LIBRARIES := libcutils \
                          libtranslator_utils \
                          libhidlbase \
                          libhidltransport \
                          libhwbinder

LOCAL_SHARED_LIBRARIES += \
    vendor.mediatek.hardware.apmonitor@2.0

LOCAL_PROPRIETARY_MODULE := true
LOCAL_MODULE_TAGS := optional
LOCAL_LDLIBS := -llog
include $(MTK_STATIC_LIBRARY)

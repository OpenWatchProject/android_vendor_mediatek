LOCAL_PATH:= $(call my-dir)

######[Header Libraries]######
include $(CLEAR_VARS)
LOCAL_MODULE:= libmdmi_core_headers
LOCAL_EXPORT_C_INCLUDE_DIRS +=  $(LOCAL_PATH)/include
include $(BUILD_HEADER_LIBRARY)

######[Implementation]######
include $(CLEAR_VARS)
LOCAL_ARM_MODE := arm
LOCAL_MODULE := libmdmi_core
LOCAL_HEADER_LIBRARIES := \
    mdmi_public_headers \
    libmdmi_core_headers

LOCAL_SRC_FILES := $(subst $(LOCAL_PATH)/,,$(wildcard $(LOCAL_PATH)/src/*.cpp))
LOCAL_SHARED_LIBRARIES := \
    vendor.mediatek.hardware.dmc@1.0 \
    libhidlbase \
    libhidltransport \
    libhwbinder \
    liblog \
    libutils

LOCAL_MODULE_TAGS := optional
include $(BUILD_SHARED_LIBRARY)
############################################################################
# Diagnostic Monitoring Collector core implementation
# Vendor executable
############################################################################
LOCAL_PATH:= $(call my-dir)

######[Header Libraries]######
include $(CLEAR_VARS)
LOCAL_MODULE:= dmc_core_public_headers
LOCAL_EXPORT_C_INCLUDE_DIRS += $(LOCAL_PATH)/include/public
include $(BUILD_HEADER_LIBRARY)

######[Implementation]######
include $(CLEAR_VARS)
LOCAL_MODULE := dmc_core
LOCAL_ARM_MODE := arm
LOCAL_MODULE_OWNER := mtk

LOCAL_C_INCLUDES := \
    $(LOCAL_PATH)/include/public \
    $(LOCAL_PATH)/include

LOCAL_SRC_FILES := $(subst $(LOCAL_PATH)/,,$(wildcard $(LOCAL_PATH)/src/*.cpp))

LOCAL_STATIC_LIBRARIES := \
    libdmc_server \
    libapmonitor_server \
    libmdmonitor

LOCAL_SHARED_LIBRARIES := \
    libtranslator_utils \
    libicd_decoder \
    vendor.mediatek.hardware.dmc@1.1 \
    vendor.mediatek.hardware.mdmonitor@1.0 \
    libutils \
    libcutils \
    libc++ \
    libhidlbase \
    libhidltransport \
    libhwbinder

LOCAL_SHARED_LIBRARIES += \
    vendor.mediatek.hardware.apmonitor@2.0

LOCAL_PROPRIETARY_MODULE := true
LOCAL_MODULE_TAGS := optional
LOCAL_LDLIBS := -llog
LOCAL_CPPFLAGS += -fexceptions
LOCAL_INIT_RC := dmc_core.rc
include $(MTK_EXECUTABLE)

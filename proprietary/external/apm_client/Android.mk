LOCAL_PATH:= $(call my-dir)

############################################################################
# Native APM HIDL client connect to libapmonitor_server, APM HIDL server.
# Vendor native shared library
############################################################################

########[ libapmonitor_vendor ]########
include $(CLEAR_VARS)
LOCAL_ARM_MODE := arm
LOCAL_MODULE := libapmonitor_vendor
LOCAL_C_INCLUDES := \
    $(LOCAL_PATH)/include/public \
    $(LOCAL_PATH)/include/public/msg_defs \
    $(LOCAL_PATH)/include

LOCAL_EXPORT_C_INCLUDE_DIRS := \
    $(LOCAL_PATH)/include/public \
    $(LOCAL_PATH)/include/public/msg_defs \
    $(LOCAL_PATH)/include/public/msg_defs/kpi

LOCAL_SRC_FILES := $(subst $(LOCAL_PATH)/,,$(wildcard $(LOCAL_PATH)/src/*.cpp))
LOCAL_SHARED_LIBRARIES := \
    libhidlbase \
    libhidltransport \
    libhwbinder

# APM HIDL interface
LOCAL_SHARED_LIBRARIES += \
    vendor.mediatek.hardware.apmonitor@2.0

#libcutils for property_get/set
LOCAL_SHARED_LIBRARIES += libcutils

LOCAL_PROPRIETARY_MODULE := true

LOCAL_MODULE_TAGS := optional
LOCAL_LDLIBS := -llog -lutils

include $(MTK_SHARED_LIBRARY)

#########[ APM messages definition header library ]########
include $(CLEAR_VARS)
LOCAL_MODULE:= libapm_msg_defs_headers
LOCAL_EXPORT_C_INCLUDE_DIRS += \
    $(LOCAL_PATH)/include/public/msg_defs \
    $(LOCAL_PATH)/include/public/msg_defs/kpi

LOCAL_PROPRIETARY_MODULE := true
include $(BUILD_HEADER_LIBRARY)

########[ APM interface header library ]########
include $(CLEAR_VARS)
LOCAL_MODULE:= libapmonitor_vendor_headers
LOCAL_EXPORT_C_INCLUDE_DIRS += $(LOCAL_PATH)/include/public
LOCAL_PROPRIETARY_MODULE := true
include $(BUILD_HEADER_LIBRARY)

########[ APM messages definition JAVA library ]########
include $(CLEAR_VARS)
LOCAL_SRC_FILES := src/java/com/mediatek/apmonitor/ApmMsgDefs.java
LOCAL_MODULE := apm-msg-defs
LOCAL_PROGUARD_ENABLED := disabled
LOCAL_PROPRIETARY_MODULE := false
include $(BUILD_STATIC_JAVA_LIBRARY)

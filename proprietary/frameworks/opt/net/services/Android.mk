ifneq ($(strip $(MSSI_MTK_TELEPHONY_ADD_ON_POLICY)), 1)

LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_SRC_FILES := $(call all-subdir-java-files)

LOCAL_SRC_FILES += \
    java/com/mediatek/net/connectivity/IMtkIpConnectivityMetrics.aidl

LOCAL_MODULE_TAGS := optional

LOCAL_MODULE := mediatek-framework-net

LOCAL_JAVA_LIBRARIES := services framework wifi-service mediatek-telephony-common
LOCAL_JAVA_LIBRARIES += mediatek-telephony-base
LOCAL_JAVA_LIBRARIES += mediatek-framework

LOCAL_STATIC_JAVA_LIBRARIES += \
    vendor.mediatek.hardware.netdagent-V1.0-java

include $(BUILD_JAVA_LIBRARY)

# Include subdirectory makefiles
# ============================================================
include $(call all-makefiles-under,$(LOCAL_PATH))

endif
LOCAL_PATH:= $(call my-dir)

# merge all required services into one jar
# ============================================================
include $(CLEAR_VARS)

LOCAL_SRC_FILES := $(call all-java-files-under, java)
LOCAL_MODULE_TAGS := optional

LOCAL_MODULE := services.omadmservice
LOCAL_JAVA_LIBRARIES := services.core telephony-common ims-common voip-common
LOCAL_JAVA_LIBRARIES += mediatek-telephony-base mediatek-common
#LOCAL_JAVA_LIBRARIES += mediatek-telephony-common mediatek-telephony-base mediatek-ims-common
#LOCAL_JAVA_LIBRARIES += mediatek-framework mediatek-common
LOCAL_STATIC_JAVA_LIBRARIES += vendor.mediatek.hardware.omadm-V1.0-java
LOCAL_MODULE_OWNER := mtk

# Include AIDL files from mediatek-common.
LOCAL_AIDL_INCLUDES += vendor/mediatek/proprietary/frameworks/common/src

include $(BUILD_STATIC_JAVA_LIBRARY)



include $(CLEAR_VARS)

LOCAL_PROPRIETARY_MODULE := true

include $(call all-makefiles-under, $(LOCAL_PATH))

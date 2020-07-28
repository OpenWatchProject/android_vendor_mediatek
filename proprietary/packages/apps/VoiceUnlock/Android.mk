LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)

LOCAL_JAVA_LIBRARIES := bouncycastle
LOCAL_JAVA_LIBRARIES += mediatek-common
LOCAL_JAVA_LIBRARIES += mediatek-framework

LOCAL_MODULE_TAGS := optional

LOCAL_SRC_FILES := $(call all-java-files-under, src)

LOCAL_PACKAGE_NAME := VoiceUnlock
LOCAL_PRIVATE_PLATFORM_APIS := true
LOCAL_CERTIFICATE := platform

# Secure setting access need a priv app.
LOCAL_PRIVILEGED_MODULE := true
LOCAL_PROPRIETARY_MODULE := false
LOCAL_MODULE_OWNER := mtk

include $(BUILD_PACKAGE)

# Use the folloing include to make our test apk.
include $(call all-makefiles-under,$(LOCAL_PATH))

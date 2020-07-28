ifneq ($(strip $(MTK_DISABLE_ATCIJ)), yes)
LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)

LOCAL_SRC_FILES := $(call all-java-files-under, src)


LOCAL_PACKAGE_NAME := AtciService
LOCAL_PRIVATE_PLATFORM_APIS := true
LOCAL_MULTILIB := both
LOCAL_CERTIFICATE := platform
LOCAL_JAVA_LIBRARIES += telephony-common ims-common

LOCAL_PRIVILEGED_MODULE := true

LOCAL_PROGUARD_ENABLED := disabled
LOCAL_PROGUARD_FLAG_FILES := proguard.flags
LOCAL_STATIC_JAVA_LIBRARIES := android-support-v4 \
    vendor.mediatek.hardware.netdagent-V1.0-java \
    vendor.mediatek.hardware.atci-V1.0-java

include $(BUILD_PACKAGE)

# Use the folloing include to make our test apk.
include $(call all-makefiles-under,$(LOCAL_PATH))
endif

LOCAL_PATH:= $(call my-dir)

# Build the Ims OEM implementation including imsservice, imsadapter, imsriladapter.
include $(CLEAR_VARS)

LOCAL_SRC_FILES := $(call all-java-files-under, src)
LOCAL_SRC_FILES += $(call all-java-files-under, ext/src)

ifneq ($(TARGET_DEVICE), gobo)
LOCAL_RESOURCE_DIR := $(LOCAL_PATH)/../../../../../../device/mediatek/config/ImsSSConfig/ImsResource/res
else
LOCAL_RESOURCE_DIR := $(LOCAL_PATH)/../../../../../../device/huawei/gobo/config/ImsSSConfig/ImsResource/res
endif

LOCAL_JAVA_LIBRARIES += telephony-common
LOCAL_JAVA_LIBRARIES += ims-common
LOCAL_JAVA_LIBRARIES += mediatek-common
LOCAL_JAVA_LIBRARIES += mediatek-ims-base

LOCAL_STATIC_JAVA_LIBRARIES += android-support-v4
LOCAL_STATIC_JAVA_LIBRARIES += WfoService
LOCAL_STATIC_JAVA_LIBRARIES += wfo-common
LOCAL_STATIC_JAVA_LIBRARIES += android.hardware.radio-V1.0-java
LOCAL_STATIC_JAVA_LIBRARIES += android.hardware.radio-V1.4-java
LOCAL_STATIC_JAVA_LIBRARIES += vendor.mediatek.hardware.mtkradioex-V1.0-java
LOCAL_STATIC_JAVA_LIBRARIES += vendor.mediatek.hardware.mtkradioex-V1.6-java
LOCAL_STATIC_JAVA_LIBRARIES += vendor.mediatek.hardware.radio-V3.0-java
LOCAL_STATIC_JAVA_LIBRARIES += vendor.mediatek.hardware.radio-V3.6-java
LOCAL_STATIC_JAVA_LIBRARIES += vendor.mediatek.hardware.radio-V3.11-java
LOCAL_STATIC_JAVA_LIBRARIES += vendor.mediatek.hardware.imsa-V1.0-java
LOCAL_STATIC_JAVA_LIBRARIES += vendor.mediatek.hardware.netdagent-V1.0-java

LOCAL_PACKAGE_NAME := ImsService
LOCAL_PRIVATE_PLATFORM_APIS := true
LOCAL_PROPRIETARY_MODULE := false
LOCAL_CERTIFICATE := platform
LOCAL_PRIVILEGED_MODULE := true

LOCAL_PROGUARD_ENABLED := disabled
LOCAL_PROGUARD_FLAGS := $(proguard.flags)

include $(BUILD_PACKAGE)

# Build java lib for accessing ImsConfigProvider API
include $(CLEAR_VARS)

LOCAL_MODULE := ims-config
LOCAL_SRC_FILES:= src/com/mediatek/ims/config/ConfigRegister.java \
                  src/com/mediatek/ims/config/FeatureRegister.java \
                  src/com/mediatek/ims/config/ImsConfigContract.java \
                  src/com/mediatek/ims/config/ImsConfigSettings.java \
                  src/com/mediatek/ims/config/Register.java \

LOCAL_JAVA_LIBRARIES := ims-common mediatek-ims-base

include $(BUILD_STATIC_JAVA_LIBRARY)

# Build ext package
include $(call all-makefiles-under,$(LOCAL_PATH))

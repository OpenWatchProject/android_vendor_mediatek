

LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)

# For portable build start
ifeq ($(strip $(PRODUCT_CHARACTERISTICS)),tablet)
MY_BUILD_PORTABLE := no
else
MY_BUILD_PORTABLE := no
endif

ifeq ($(strip $(MY_BUILD_PORTABLE)),yes)
# Module name should match apk name to be installed
LOCAL_MODULE := MiraVision
LOCAL_MODULE_TAGS := optional
ifeq ($(MTK_GMO_RAM_OPTIMIZE),yes)
LOCAL_SRC_FILES := $(LOCAL_MODULE)-slim-release-unsigned.apk
else
LOCAL_SRC_FILES := $(LOCAL_MODULE)-release-unsigned.apk
endif
LOCAL_MODULE_CLASS := APPS
LOCAL_MODULE_SUFFIX := $(COMMON_ANDROID_PACKAGE_SUFFIX)
LOCAL_CERTIFICATE := platform

LOCAL_PROPRIETARY_MODULE := false
LOCAL_MODULE_OWNER := mtk
include $(BUILD_PREBUILT)
else
# For portable build end

LOCAL_JAVA_LIBRARIES := bouncycastle
LOCAL_JAVA_LIBRARIES += mediatek-framework

LOCAL_STATIC_JAVA_LIBRARIES := android-support-v4

# for projects can't support 1080p video, so replaced with 720p resource
ifeq ($(MTK_GMO_RAM_OPTIMIZE),yes)
    LOCAL_ASSET_DIR := $(LOCAL_PATH)/assets_slim
endif

LOCAL_MODULE_TAGS := optional

LOCAL_SRC_FILES := $(call all-java-files-under, src)

LOCAL_PACKAGE_NAME := MiraVision
LOCAL_PRIVATE_PLATFORM_APIS := true
LOCAL_PROPRIETARY_MODULE := false
LOCAL_MODULE_OWNER := mtk
LOCAL_CERTIFICATE := platform

LOCAL_PROGUARD_FLAG_FILES := proguard.flags

include $(BUILD_PACKAGE)
endif

# Use the folloing include to make our test apk.
include $(call all-makefiles-under,$(LOCAL_PATH))



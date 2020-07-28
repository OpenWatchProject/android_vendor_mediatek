LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)

LOCAL_MODULE := OP07Entitlement-Fcm
LOCAL_MODULE_TAGS := optional
LOCAL_SRC_FILES := app/build/app-release-unsigned.apk
LOCAL_MODULE_CLASS := APPS
LOCAL_MODULE_SUFFIX := $(COMMON_ANDROID_PACKAGE_SUFFIX)
LOCAL_CERTIFICATE := platform
include $(BUILD_PREBUILT)


# Other makefiles
include $(call all-makefiles-under, $(LOCAL_PATH))


# Only trying to build it in case of telephony add on is supported
ifeq ($(strip $(MSSI_MTK_TELEPHONY_ADD_ON_POLICY)), 0)

LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)

# We only want this apk build for tests.
LOCAL_MODULE_TAGS := tests
LOCAL_CERTIFICATE := platform

LOCAL_JAVA_LIBRARIES := \
    android.test.runner \
    telephony-common \
    ims-common \
    android.test.base \
    android.test.mock \


LOCAL_STATIC_JAVA_LIBRARIES := \
    androidx.test.rules \
    androidx.test.espresso.core \
    androidx.test.espresso.contrib-nodeps \
    androidx.test.espresso.intents-nodeps \
    mockito-target-minus-junit4 \
    platform-test-annotations \
    truth-prebuilt \
    ub-uiautomator

# Include all test java files.
LOCAL_SRC_FILES := $(call all-java-files-under, src)

LOCAL_PACKAGE_NAME := MtkSettingsUnitTests
LOCAL_PRIVATE_PLATFORM_APIS := true
LOCAL_COMPATIBILITY_SUITE := device-tests

LOCAL_INSTRUMENTATION_FOR := MtkSettings

include $(BUILD_PACKAGE)

endif

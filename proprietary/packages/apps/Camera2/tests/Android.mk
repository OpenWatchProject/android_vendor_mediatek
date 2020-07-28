ifeq ($(MTK_AUTO_TEST) , yes)

LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)

# We only want this apk build for tests.
LOCAL_MODULE_TAGS := tests

LOCAL_CERTIFICATE := platform

LOCAL_JAVA_LIBRARIES := android.test.runner

LOCAL_STATIC_JAVA_LIBRARIES := ub-uiautomator
LOCAL_STATIC_JAVA_LIBRARIES += mockito-target
LOCAL_STATIC_JAVA_LIBRARIES += hamcrest-library
LOCAL_STATIC_JAVA_LIBRARIES += android-support-test

# Include all test java files.
LOCAL_SRC_FILES := $(call all-java-files-under, src/com/mediatek/camera/tests/v3)
LOCAL_SRC_FILES += $(call all-java-files-under, src/com/mediatek/camera/tests/helper)
LOCAL_SRC_FILES += src/com/mediatek/camera/tests/helper/ILoggerService.aidl
LOCAL_SRC_FILES += src/com/mediatek/camera/tests/helper/ILoggerCallback.aidl

# Include all test assets files.
LOCAL_ASSET_DIR += $(LOCAL_PATH)/assets

# Include all unit test java files, exclude tests running with RobolectricTestRunner
UNIT_TEST_ROOT := ../unittests/src/com/mediatek/camera
LOCAL_SRC_FILES += $(call all-java-files-under, $(UNIT_TEST_ROOT))
LOCAL_SRC_FILES := $(filter-out $(UNIT_TEST_ROOT)/common/relation/tests/DataStoreTest.java, $(LOCAL_SRC_FILES))
LOCAL_SRC_FILES := $(filter-out $(UNIT_TEST_ROOT)/feature/setting/picturesize/PictureSizeSettingViewTest.java, $(LOCAL_SRC_FILES))

LOCAL_PACKAGE_NAME := CameraTests
LOCAL_PRIVATE_PLATFORM_APIS := true

LOCAL_INSTRUMENTATION_FOR := Camera

include $(BUILD_PACKAGE)

endif

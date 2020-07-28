LOCAL_PATH:= $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE_TAGS := optional
LOCAL_AAPT_INCLUDE_ALL_RESOURCES := true

LOCAL_SRC_FILES := $(call all-subdir-java-files)

LOCAL_PACKAGE_NAME := LPPeService

# from Android P0
ifeq (1,$(filter 1,$(shell echo "$$(( $(PLATFORM_SDK_VERSION) >= 28 ))" )))
    LOCAL_PRIVATE_PLATFORM_APIS := true
    LOCAL_STATIC_JAVA_LIBRARIES += vendor.mediatek.hardware.lbs-V1.0-java
else
    # Before P0 Android version
    LOCAL_PROPRIETARY_MODULE := true
    LOCAL_MODULE_OWNER := mtk
    LOCAL_STATIC_JAVA_LIBRARIES += vendor.mediatek.hardware.lbs-V1.0-java-static
endif

# from Android Q0
ifeq (1,$(filter 1,$(shell echo "$$(( $(PLATFORM_SDK_VERSION) >= 29 ))" )))
    LOCAL_PRIVILEGED_MODULE := true
endif

LOCAL_CERTIFICATE := platform
include $(BUILD_PACKAGE)

include $(call all-makefiles-under,$(LOCAL_PATH))


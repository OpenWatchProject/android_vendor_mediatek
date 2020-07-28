
LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)

LOCAL_MODULE_TAGS := optional

LOCAL_SRC_FILES := $(call all-subdir-java-files)

LOCAL_MODULE := com.mediatek.lbs.em2.utils

# from Android P0
ifeq (1,$(filter 1,$(shell echo "$$(( $(PLATFORM_SDK_VERSION) >= 28 ))" )))
   LOCAL_STATIC_JAVA_LIBRARIES += vendor.mediatek.hardware.lbs-V1.0-java
else
   # Android O0
   LOCAL_STATIC_JAVA_LIBRARIES += vendor.mediatek.hardware.lbs-V1.0-java-static
endif

#LOCAL_JACK_ENABLED := disabled

include $(BUILD_STATIC_JAVA_LIBRARY)

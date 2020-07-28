#ifeq ($(MTK_GPS_SUPPORT),yes)
#
#LOCAL_PATH:= $(call my-dir)
#
#include $(CLEAR_VARS)
#
#LOCAL_MODULE_TAGS := optional
#LOCAL_SRC_FILES := $(call all-subdir-java-files)
#
#LOCAL_PACKAGE_NAME := NlpService
#LOCAL_PRIVATE_PLATFORM_APIS := true
#LOCAL_CERTIFICATE := platform
#LOCAL_STATIC_JAVA_LIBRARIES += vendor.mediatek.hardware.lbs-V1.0-java
#
#include $(BUILD_PACKAGE)
#include $(call all-makefiles-under,$(LOCAL_PATH))
#
#endif
#
ifneq ($(strip $(MTK_DISABLE_BSPTEL_TOOL)), yes)
LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)

LOCAL_MODULE_TAGS := optional

LOCAL_SRC_FILES := $(call all-subdir-java-files)

LOCAL_PRIVATE_PLATFORM_APIS := true
LOCAL_PACKAGE_NAME := SimRecoveryTestTool
LOCAL_MODULE_OWNER := mtk
LOCAL_CERTIFICATE := platform
LOCAL_JAVA_LIBRARIES += mediatek-framework telephony-common mediatek-telephony-common

include $(BUILD_PACKAGE)

# Use the folloing include to make our test apk.
include $(call all-makefiles-under,$(LOCAL_PATH))
endif

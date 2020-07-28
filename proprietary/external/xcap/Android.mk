LOCAL_PATH := $(call my-dir)

build_xcap := true

# No need to build XCAP if no telephony add on
ifeq ($(strip $(MSSI_MTK_TELEPHONY_ADD_ON_POLICY)), 1)
build_xcap := false
endif

# No need to build XCAP on tc1
ifeq ($(strip $(MSSI_MTK_TC1_COMMON_SERVICE)), yes)
build_xcap := false
endif

ifeq ($(build_xcap),true)
include $(CLEAR_VARS)

LOCAL_JAVA_LIBRARIES := okhttp
LOCAL_JAVA_LIBRARIES += mediatek-framework
LOCAL_JAVA_LIBRARIES += mediatek-ims-base
LOCAL_JAVA_LIBRARIES += mediatek-telephony-base

# Include all the java files.
LOCAL_SRC_FILES := $(call all-java-files-under, src)

LOCAL_MODULE_TAGS := optional

# The name of the jar file to create.
LOCAL_MODULE := xcap
LOCAL_PROPRIETARY_MODULE := false


# Build a static jar file.
include $(BUILD_STATIC_JAVA_LIBRARY)
endif

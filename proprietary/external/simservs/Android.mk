LOCAL_PATH := $(call my-dir)

build_simservs := true

# No need to build Simservs if no telephony add on
ifeq ($(strip $(MSSI_MTK_TELEPHONY_ADD_ON_POLICY)), 1)
build_simservs := false
endif

# No need to build XCAP on tc1
ifeq ($(strip $(MSSI_MTK_TC1_COMMON_SERVICE)), yes)
build_simservs := false
endif

ifeq ($(build_simservs),true)
include $(CLEAR_VARS)

LOCAL_JAVA_LIBRARIES := okhttp
LOCAL_STATIC_JAVA_LIBRARIES := xcap

# Include all the java files.
LOCAL_SRC_FILES := $(call all-java-files-under, src)

LOCAL_MODULE_TAGS := optional

# The name of the jar file to create.
LOCAL_MODULE := Simservs
LOCAL_PROPRIETARY_MODULE := false

# Build a static jar file.
include $(BUILD_STATIC_JAVA_LIBRARY)
include $(call all-makefiles-under,$(LOCAL_PATH))
endif

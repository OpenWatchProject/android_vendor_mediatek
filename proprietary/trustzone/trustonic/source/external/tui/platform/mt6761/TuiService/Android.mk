#
# build libTui
#

LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

# Module name (sets name of output binary / library)
LOCAL_MODULE := libTui
LOCAL_PROPRIETARY_MODULE := true

# Add your folders with header files here
LOCAL_C_INCLUDES := \
	$(JNI_H_INCLUDE)

# Add your source files here (relative paths)
LOCAL_SRC_FILES += \
	jni/tlcTui.cpp \
	jni/tlcTuiJni.cpp

# Needed to use Trustonic logging macros
LOCAL_SHARED_LIBRARIES := \
	libMcClient

# Undefine NDEBUG to enable LOG_D in log
LOCAL_CFLAGS += -UNDEBUG
# Enable logging to logcat per default
LOCAL_CFLAGS += -DLOG_ANDROID
ifeq ($(APP_PROJECT_PATH),)
LOCAL_SHARED_LIBRARIES += \
	liblog
else # !NDK
LOCAL_LDLIBS += \
	-llog
endif # NDK

include $(BUILD_SHARED_LIBRARY)

#
# build TuiService
#

include $(CLEAR_VARS)

LOCAL_PROGUARD_ENABLED := disabled
LOCAL_SRC_FILES := $(call all-java-files-under, java)
LOCAL_JNI_SHARED_LIBRARIES := libTui

LOCAL_PACKAGE_NAME := TuiService
LOCAL_CERTIFICATE := platform
LOCAL_DEX_PREOPT := false
LOCAL_PROPRIETARY_MODULE := true

include $(BUILD_PACKAGE)

# =============================================================================

# adding the root folder to the search path appears to make absolute paths
# work for import-module - lets see how long this works and what surprises
# future developers get from this.
$(call import-add-path,/)
$(call import-module,$(COMP_PATH_MobiCoreClientLib_module))

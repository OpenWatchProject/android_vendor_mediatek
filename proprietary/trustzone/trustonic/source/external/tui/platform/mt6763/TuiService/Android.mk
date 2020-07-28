#
# build TuiService
#

LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

# Module name (sets name of output binary / library)
LOCAL_MODULE := libTui
LOCAL_PROPRIETARY_MODULE := true
LOCAL_MODULE_OWNER := mtk

# Add your source files here (relative paths)
LOCAL_SRC_FILES += \
	jni/tlcTui.cpp \
	jni/tlcTuiJni.cpp

# Enable logging to logcat per default
LOCAL_CFLAGS += -DLOG_ANDROID
LOCAL_LDLIBS += -llog

# Undefine NDEBUG to enable LOG_D in log
LOCAL_CFLAGS += -UNDEBUG

# Needed to use Trustonic logging macros
LOCAL_SHARED_LIBRARIES := libMcClient

ifeq (STD,)
LOCAL_SRC_FILES += \
	jni/tlcNativeWindow.cpp

# Add libs for native UI
LOCAL_SHARED_LIBRARIES += \
     libandroid \
     libui \
     libutils \
     libcutils \
     libgui
endif

LOCAL_HEADER_LIBRARIES := jni_headers

include $(BUILD_SHARED_LIBRARY)


include $(CLEAR_VARS)

LOCAL_SRC_FILES := $(call all-java-files-under, src)
LOCAL_JNI_SHARED_LIBRARIES := libTui

LOCAL_PACKAGE_NAME := TuiService
LOCAL_PROPRIETARY_MODULE := true
LOCAL_MODULE_OWNER := mtk
LOCAL_MODULE_TAGS := optional
LOCAL_CERTIFICATE := platform
LOCAL_DEX_PREOPT := false

LOCAL_PROGUARD_FLAGS := -include $(LOCAL_PATH)/proguard-project.txt

include $(BUILD_PACKAGE)

# =============================================================================

# adding the root folder to the search path appears to make absolute paths
# work for import-module - lets see how long this works and what surprises
# future developers get from this.
$(call import-add-path,/)
$(call import-module,$(COMP_PATH_MobiCoreClientLib_module))

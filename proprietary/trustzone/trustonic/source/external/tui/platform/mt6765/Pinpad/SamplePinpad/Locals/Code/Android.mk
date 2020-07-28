LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

# Module name (sets name of output binary / library)
LOCAL_MODULE	:= libSamplePinpadJni
#LOCAL_PROPRIETARY_MODULE := true
LOCAL_PRIVATE_PLATFORM_APIS := true
LOCAL_MODULE_OWNER := mtk

# Add your folders with header files here
LOCAL_C_INCLUDES := \
	$(LOCAL_PATH)/jni/ \
	$(LOCAL_PATH)/jni/pictures \
	$(COMP_PATH_MobiCoreDriverLib)/Public \
	$(COMP_PATH_MobiCoreDriverLib)/Public/GP \
	$(COMP_PATH_ClientLib)/include \
	$(JNI_H_INCLUDE) \
	$(JNI_H_INCLUDE)/linux \
	vendor/mediatek/proprietary/trustzone/trustonic/source/trustlets/tui/platform/$(MTK_PLATFORM_DIR)/TlSamplePinpad/Locals/Code/public

# Add your source files here (relative paths)
LOCAL_SRC_FILES	+= \
	jni/tlcPinpad.cpp \
	jni/tlcPinpadJni.cpp

# Need the TeeClient library
LOCAL_SHARED_LIBRARIES := libTeeClient

# Enable logging to logcat per default
LOCAL_CFLAGS += -DLOG_ANDROID
LOCAL_CFLAGS += -DTBASE_API_LEVEL=9
ifeq ($(APP_PROJECT_PATH),)
LOCAL_SHARED_LIBRARIES += \
	liblog
else # !NDK
LOCAL_LDLIBS += \
	-llog
endif # NDK

include $(BUILD_SHARED_LIBRARY)

include $(CLEAR_VARS)

LOCAL_SRC_FILES := $(call all-java-files-under, java)
LOCAL_JNI_SHARED_LIBRARIES := libSamplePinpadJni

LOCAL_PACKAGE_NAME := SamplePinpad
#LOCAL_PROPRIETARY_MODULE := true
LOCAL_PRIVATE_PLATFORM_APIS := true
LOCAL_MODULE_OWNER := mtk
LOCAL_DEX_PREOPT := false

LOCAL_STATIC_JAVA_LIBRARIES := TeeClient
LOCAL_PROGUARD_ENABLED := disabled

include $(BUILD_PACKAGE)

$(call import-add-path,$(LOCAL_PATH))
$(call import-module, ../../build/intermediates/exploded-aar/TeeClient/jni/)

ifndef TRUSTONIC_ANDROID_8

LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

# Module name (sets name of output binary / library)
LOCAL_MODULE := libTeeServiceJni
LOCAL_CERTIFICATE := platform

# Add your source files here (relative paths)
LOCAL_SRC_FILES += \
	jni/native-lib-jni.cpp \
	jni/TeeCallback.cpp \

LOCAL_C_INCLUDES := \
	$(LOCAL_PATH)/../TeeService \
	$(LOCAL_PATH)/../hardware/interfaces/tee/1.0/default

ifeq ($(QC_SOURCE_TREE),yes)
LOCAL_C_INCLUDES += \
        frameworks/native/include \
        hardware/qcom/display/gralloc

LOCAL_CFLAGS += -DQC_GRALLOC
else ifeq ($(TARGET_PRODUCT),hikey960)
LOCAL_C_INCLUDES += device/linaro/hikey/gralloc960/
endif

# Undefine NDEBUG to enable LOG_D in log
LOCAL_CFLAGS += -UNDEBUG
# Enable logging to logcat per default
LOCAL_CFLAGS += -DLOG_ANDROID
ifeq ($(APP_PROJECT_PATH),)
LOCAL_SHARED_LIBRARIES := \
	liblog \
	libutils \
	libcutils \
	liblog \
	libbinder \
	libdl \
	libgui \
	libui \
	libselinux \
	libhidlbase \
	libhidlmemory \
	libhidltransport \
	android.hidl.allocator@1.0 \
	android.hidl.memory@1.0 \
	vendor.trustonic.tee@1.0

LOCAL_STATIC_LIBRARIES := \
	libteeservice_server
else # !NDK
LOCAL_LDLIBS += \
	-llog
endif # NDK

include $(BUILD_SHARED_LIBRARY)

include $(CLEAR_VARS)

LOCAL_PROGUARD_ENABLED := disabled
LOCAL_SRC_FILES := $(call all-java-files-under, java)
LOCAL_JNI_SHARED_LIBRARIES := libTeeServiceJni
LOCAL_STATIC_JAVA_LIBRARIES := android.hidl.base-V1.0-java
LOCAL_STATIC_JAVA_LIBRARIES += vendor.trustonic.tee.tui-V1.0-java
# Add your source files here (relative paths)
LOCAL_PACKAGE_NAME := TeeService
LOCAL_PRIVILEGED_MODULE := true
LOCAL_CERTIFICATE := platform
LOCAL_DEX_PREOPT := false
LOCAL_PRIVATE_PLATFORM_APIS := true

include $(BUILD_PACKAGE)

endif # TRUSTONIC_ANDROID_8

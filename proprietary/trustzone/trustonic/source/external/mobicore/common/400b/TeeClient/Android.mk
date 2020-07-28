# we provide the libraries with debug symbols. The symbols are stripped
# by the build system for the release version. Using generic build, as
# lib APIs are not platform dependent

LOCAL_PATH := $(call my-dir)

ifeq ($(TARGET_BUILD_VARIANT),eng)
  LOCAL_INSTALL_MODE ?= Debug
else
  LOCAL_INSTALL_MODE ?= Release
endif

#-------------------------------------------------------------------------------
# Module: "TeeClient" shared library (libTeeClient.so)
#-------------------------------------------------------------------------------
include $(CLEAR_VARS)

LOCAL_MODULE := libTeeClient
LOCAL_MODULE_TAGS := optional

LOCAL_CFLAGS := -fvisibility=hidden
LOCAL_CFLAGS += -DTBASE_API_LEVEL=9
LOCAL_CFLAGS += -Wall -Wextra -Werror
LOCAL_CFLAGS += -std=c++11
LOCAL_CFLAGS += -DLOG_ANDROID
LOCAL_CFLAGS += -DGOOGLE_PROTOBUF_NO_RTTI
LOCAL_CFLAGS += -static-libstdc++

LOCAL_C_INCLUDES := \
	$(LOCAL_PATH)/include \
	$(LOCAL_PATH)/include/GP \
	$(LOCAL_PATH)/jni

LOCAL_SRC_FILES := \
	jni/common.cpp \
	jni/dynamic_log.cpp \
	jni/dummy.cpp \
	jni/native_interface.cpp \
	jni/system.cpp \
	jni/trustonic_tee.cpp \
	jni/proxy_client.cpp \
	jni/proxy_common.cpp \
	jni/proxy.cpp \
	jni/proxy_gp.cpp \
	jni/proxy_mc.cpp \
	jni/tee_bind_jni.cpp \
	jni/tee_client_jni.cpp \
	jni/kinibi_info.cpp \
	jni/tlCmInfo.cpp \
	jni/tlCmInfo_2_0.cpp

LOCAL_HEADER_LIBRARIES += jni_headers

LOCAL_LDLIBS := -llog

LOCAL_STATIC_LIBRARIES := \
	libteeproxy_proto

ifeq ($(APP_PROJECT_PATH),)
LOCAL_SHARED_LIBRARIES += \
	libprotobuf-cpp-full
else
LOCAL_STATIC_LIBRARIES += \
	libprotobuf-cpp-lite-ndk
endif


LOCAL_EXPORT_C_INCLUDE_DIRS := $(LOCAL_C_INCLUDES)

include $(BUILD_SHARED_LIBRARY)

include $(CLEAR_VARS)
LOCAL_PROGUARD_ENABLED := disabled
LOCAL_SRC_FILES := $(call all-java-files-under, java)

LOCAL_MODULE := TeeClient
LOCAL_MODULE_TAGS := optional
LOCAL_CERTIFICATE := platform
LOCAL_DEX_PREOPT := false
LOCAL_PLATFORM_PRIVATE_APIS := true

include $(BUILD_STATIC_JAVA_LIBRARY)

# adding the root folder to the search path appears to make absolute paths
# work for import-module - lets see how long this works and what surprises
# future developers get from this.
$(call import-add-path,/)
$(call import-module,$(COMP_PATH_AndroidProtoBuf))
$(call import-module,$(COMP_PATH_TeeProxyProto_module))

# Google Android makefile for curl and libcurl
#
# This file is an updated version of Dan Fandrich's Android.mk, meant to build
# curl for ToT android with the android build system.

# This curl ends up in external/mobicore/curl, but must not collide with an
# existing one in external/curl
ifeq ($(wildcard external/curl),)

LOCAL_PATH:= $(call my-dir)

# Curl needs a version string.
# As this will be compiled on multiple platforms, generate a version string from
# the build environment variables.
version_string := "Android $(PLATFORM_VERSION) $(TARGET_ARCH_VARIANT)"

curl_CFLAGS := -Wpointer-arith -Wwrite-strings -Wunused -Winline \
	-Wnested-externs -Wmissing-declarations -Wmissing-prototypes -Wno-long-long \
	-Wfloat-equal -Wno-multichar -Wsign-compare -Wno-format-nonliteral \
	-Wendif-labels -Wstrict-prototypes -Wdeclaration-after-statement \
	-Wno-system-headers -DHAVE_CONFIG_H -DBUILDING_LIBCURL -DOS='$(version_string)'

curl_includes := \
	$(LOCAL_PATH)/include/ \
	$(LOCAL_PATH)/lib

#########################
# Build the libcurl shared library

include $(CLEAR_VARS)
include $(LOCAL_PATH)/lib/Makefile.inc

LOCAL_SRC_FILES := $(addprefix lib/,$(CSOURCES))
LOCAL_C_INCLUDES := $(curl_includes)
LOCAL_C_INCLUDES += external/openssl/include
LOCAL_C_INCLUDES += external/zlib
LOCAL_CFLAGS := $(curl_CFLAGS)
LOCAL_EXPORT_C_INCLUDE_DIRS := $(LOCAL_PATH)/include

LOCAL_MODULE:= libcurl
LOCAL_MODULE_OWNER := mtk
LOCAL_MODULE_TAGS := optional
LOCAL_PROPRIETARY_MODULE := true

LOCAL_SHARED_LIBRARIES := libcrypto libssl

ifneq ($(APP_PROJECT_PATH),)
LOCAL_LDLIBS += -lz
else
LOCAL_SHARED_LIBRARIES += libz
endif

include $(BUILD_SHARED_LIBRARY)

#########################
# Add openssl path for NDK build

$(call import-add-path,/)
$(call import-module,$(COMP_PATH_AndroidOpenSsl))

endif

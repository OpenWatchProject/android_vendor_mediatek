LOCAL_PATH:= $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE := kmsetkey.trustonic

LOCAL_PROPRIETARY_MODULE := true
LOCAL_MODULE_OWNER := mtk
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_RELATIVE_PATH := hw

LOCAL_CPPFLAGS := -Wall
LOCAL_CPPFLAGS += -Wextra
LOCAL_CPPFLAGS += -Werror
LOCAL_CPPFLAGS += -std=c++11

# Enable logging to logcat per default
LOCAL_CPPFLAGS += -DLOG_ANDROID

# Store attestation key in RPMB or persist partition
ifeq ($(KEYMASTER_RPMB),yes)
LOCAL_CPPFLAGS += -DRPMB_SUPPORT=1
else
LOCAL_CPPFLAGS += -DRPMB_SUPPORT=0
endif

LOCAL_LDLIBS := -llog

LOCAL_SRC_FILES := \
	src/KeyboxInject.cpp \
	src/module.c \
	src/HalAdaptationLayer.cpp

LOCAL_C_INCLUDES:= \
	$(LOCAL_PATH)/include \
	$(LOCAL_PATH)/../../../keymaster/common/$(TRUSTONIC_TEE_VERSION)/$(KEYMASTER_VERSION)/include

LOCAL_SHARED_LIBRARIES := \
	libMcClient

LOCAL_HEADER_LIBRARIES := \
	libhardware_headers

LOCAL_EXPORT_C_INCLUDE_DIRS := $(LOCAL_PATH)
LOCAL_ADDITIONAL_DEPENDENCIES := $(LOCAL_PATH)/Android.mk

LOCAL_POST_INSTALL_CMD = \
    $(hide) ln -sf $(notdir $(LOCAL_INSTALLED_MODULE)) $(dir $(LOCAL_INSTALLED_MODULE))kmsetkey.default.so

include $(BUILD_SHARED_LIBRARY)

# =============================================================================
# adding the root folder to the search path appears to make absolute paths
# work for import-module - lets see how long this works and what surprises
# future developers get from this.
$(call import-add-path,/)
$(call import-module,$(COMP_PATH_TeeClient_module))
$(call import-module,$(COMP_PATH_MobiCoreClientLib_module))

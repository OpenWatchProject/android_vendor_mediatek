# =============================================================================
#
# HAL implementation of Android Gatekeeper
#
# =============================================================================

LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE := libSoftGatekeeper
LOCAL_PROPRIETARY_MODULE := true
LOCAL_MODULE_OWNER := mtk
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_RELATIVE_PATH := hw

LOCAL_CFLAGS := -Wall -Werror -Wmissing-field-initializers -Wunused-parameter
# Software implementation is written in C++11
LOCAL_CPP_STD := c++11
LOCAL_CPPFLAGS += -fpermissive -Wall -Werror -D__STDC_LIMIT_MACROS

LOCAL_SRC_FILES := $(call all-c-files-under,src) $(call all-cpp-files-under,src)

LOCAL_CFLAGS += -DPLATFORM_VERSION=$(PLATFORM_VERSION)

LOCAL_C_INCLUDES := \
    $(LOCAL_PATH)/include \
    external/scrypt/lib/crypto \
    $(TOP)/system/gatekeeper/include/gatekeeper \

LOCAL_SHARED_LIBRARIES += \
    libgatekeeper \
    libhardware \
    libbase \
    libutils \
    libcrypto \
    liblog

LOCAL_STATIC_LIBRARIES := libscrypt_static

LOCAL_MODULE_SYMLINKS := gatekeeper.default.so

include $(BUILD_SHARED_LIBRARY)

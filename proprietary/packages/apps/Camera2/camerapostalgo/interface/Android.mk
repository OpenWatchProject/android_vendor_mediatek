LOCAL_PATH:= $(call my-dir)
aidl_common_files:= $(call all-Iaidl-files-under, aidl)

include $(CLEAR_VARS)

LOCAL_MODULE_TAGS := optional
LOCAL_SDK_VERSION := current

LOCAL_AIDL_INCLUDES := $(LOCAL_PATH)/aidl \
    $(LOCAL_PATH)/java \
    frameworks/native/aidl/gui \

LOCAL_SRC_FILES := \
    $(call all-java-files-under, java) \
    $(call all-Iaidl-files-under, java) \
    $(aidl_common_files) \

LOCAL_STATIC_JAVA_LIBRARIES += \

LOCAL_MODULE := libcampostalgo_api

$(info ========= CamPostAlgo building $(LOCAL_MODULE))


LOCAL_MODULE_OWNER := mtk
LOCAL_CERTIFICATE := platform

include $(BUILD_STATIC_JAVA_LIBRARY)


include $(CLEAR_VARS)

LOCAL_MODULE_TAGS := optional

LOCAL_SRC_FILES := $(aidl_common_files) \
    $(call all-Iaidl-files-under, cpp) \
    cpp/FeatureConfig.cpp \
    cpp/FeatureResult.cpp \
    cpp/FeatureParam.cpp \
    cpp/FeaturePipeConfig.cpp \
    cpp/Stream.cpp \
    cpp/StreamInfo.cpp \

LOCAL_AIDL_INCLUDES := \
    frameworks/native/aidl/gui \
    $(LOCAL_PATH)/aidl \
    $(LOCAL_PATH)/cpp \

LOCAL_HEADER_LIBRARIES += libcutils_headers
LOCAL_HEADER_LIBRARIES += camerapostalgo_headers

LOCAL_EXPORT_C_INCLUDES := $(LOCAL_PATH)
LOCAL_EXPORT_C_INCLUDE_DIRS := $(LOCAL_EXPORT_C_INCLUDES)

LOCAL_MODULE := libcampostalgo_interface

LOCAL_CFLAGS += -Wall -Werror
# suppress existing non-critical warnings
LOCAL_CFLAGS += \
        -Wno-gnu-static-float-init \
        -Wno-non-literal-null-conversion \
        -Wno-self-assign \
        -Wno-unused-parameter \
        -Wno-unused-variable \
        -Wno-unused-function \
        -Wno-macro-redefined \
        -Wno-unused-private-field \

$(info ========= CamPostAlgo building $(LOCAL_MODULE))

LOCAL_SHARED_LIBRARIES += \
    libc \
    liblog \
    libui \
    libcutils \
    libutils \
    libbinder \
    libgui \
    libbase \

include $(BUILD_SHARED_LIBRARY)

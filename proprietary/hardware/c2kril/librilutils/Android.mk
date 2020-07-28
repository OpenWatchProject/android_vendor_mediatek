# Copyright 2013 The Android Open Source Project

ifneq ($(MTK_RIL_MODE), c6m_1rild)

LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)

LOCAL_SRC_FILES:= \
    librilutils.c \
    record_stream.c \
    proto/sap-api.proto \

LOCAL_C_INCLUDES += \
    external/nanopb-c/ \
    $(LOCAL_PATH)/../include \
    $(TOP)/$(MTK_PATH_SOURCE)/hardware/ril/platformlib/include/mtkrilutils \
    $(TOP)/$(MTK_PATH_SOURCE)/hardware/ril/include

LOCAL_EXPORT_C_INCLUDE_DIRS := \
    $(LOCAL_PATH)/../../ril/include

LOCAL_PROTOC_OPTIMIZE_TYPE := nanopb-c-enable_malloc

LOCAL_MODULE:= libviarilutils
LOCAL_PROPRIETARY_MODULE := true
LOCAL_MODULE_OWNER := mtk
LOCAL_MULTILIB := first


include $(MTK_SHARED_LIBRARY)


# Create static library for those that want it
# =========================================
include $(CLEAR_VARS)

LOCAL_SRC_FILES:= \
    librilutils.c \
    record_stream.c \
    proto/sap-api.proto \

LOCAL_C_INCLUDES += \
    external/nanopb-c/ \
    $(LOCAL_PATH)/../include \
    $(TOP)/$(MTK_PATH_SOURCE)/hardware/ril/platformlib/include/mtkrilutils \
    $(TOP)/$(MTK_PATH_SOURCE)/hardware/ril/include

LOCAL_EXPORT_C_INCLUDE_DIRS := \
    $(LOCAL_PATH)/../../ril/include

LOCAL_PROTOC_OPTIMIZE_TYPE := nanopb-c-enable_malloc

LOCAL_MODULE:= libviarilutils_static

include $(BUILD_STATIC_LIBRARY)

# Create java protobuf code

include $(CLEAR_VARS)

src_proto := $(LOCAL_PATH)
LOCAL_MODULE := sap-api-java-static
LOCAL_SRC_FILES := proto/sap-api.proto
LOCAL_PROTOC_OPTIMIZE_TYPE := micro

LOCAL_PROPRIETARY_MODULE := true
LOCAL_MODULE_OWNER := mtk
LOCAL_MULTILIB := 32
#LOCAL_LDLIBS += -lpthread

include $(MTK_STATIC_LIBRARY)
endif

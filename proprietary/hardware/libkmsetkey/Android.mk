LOCAL_PATH:= $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE := libkmsetkey

LOCAL_PROPRIETARY_MODULE := true
LOCAL_MODULE_OWNER := mtk
LOCAL_MODULE_TAGS := optional

LOCAL_SRC_FILES := \
	kmsetkey.cpp

LOCAL_C_INCLUDES:= \
	$(LOCAL_PATH)

LOCAL_SHARED_LIBRARIES := \
	liblog \
	libutils \
	libhidlbase \
	libhidltransport \
	vendor.mediatek.hardware.keymaster_attestation@1.1

LOCAL_CFLAGS := -Wall -Werror -g
LOCAL_LDFLAGS := -Wl,--unresolved-symbols=ignore-all

LOCAL_EXPORT_C_INCLUDE_DIRS := $(LOCAL_PATH)
LOCAL_ADDITIONAL_DEPENDENCIES := $(LOCAL_PATH)/Android.mk

include $(BUILD_SHARED_LIBRARY)

include $(CLEAR_VARS)

LOCAL_MODULE := kmsetkey_ca

LOCAL_PROPRIETARY_MODULE := true
LOCAL_MODULE_OWNER := mtk
LOCAL_MODULE_TAGS := optional

LOCAL_SRC_FILES := \
	kmsetkey_ca.cpp

LOCAL_C_INCLUDES:= \
	$(LOCAL_PATH)/1.1

LOCAL_SHARED_LIBRARIES := \
	libdl \
	libhardware

LOCAL_CFLAGS := -Wall -Werror -g
LOCAL_LDFLAGS := -Wl,--unresolved-symbols=ignore-all

include $(BUILD_EXECUTABLE)

include $(call all-makefiles-under, $(LOCAL_PATH))

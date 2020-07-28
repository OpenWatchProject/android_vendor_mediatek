LOCAL_PATH:= $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MULTILIB := both
LOCAL_ARM_MODE := arm

LOCAL_SRC_FILES := \
	./eap-aka.c \
	./crypto/fips_prf_openssl.c

LOCAL_SHARED_LIBRARIES := \
	libnativehelper \
	libutils \
	libcutils \
	liblog \
	libcrypto \
	libssl \
	libkeystore_binder

LOCAL_MODULE_TAGS := optional
LOCAL_MODULE := libeap-aka
LOCAL_PROPRIETARY_MODULE := false
LOCAL_MODULE_OWNER := mtk

include $(BUILD_SHARED_LIBRARY)

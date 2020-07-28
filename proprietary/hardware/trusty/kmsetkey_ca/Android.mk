LOCAL_PATH:= $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE := libkmsetkey_ca.trusty

LOCAL_PROPRIETARY_MODULE := true
LOCAL_MODULE_OWNER := mtk
LOCAL_MODULE_TAGS := optional

LOCAL_SRC_FILES := \
	kmsetkey.cpp \
	kmsetkey_ipc.c

LOCAL_C_INCLUDES:= \
	$(LOCAL_PATH)

LOCAL_SHARED_LIBRARIES := \
	libtrusty \
	liblog \
	libcutils

LOCAL_HEADER_LIBRARIES := \
	libhardware_headers

LOCAL_CLFAGS = -fvisibility=hidden -Wall -Werror

LOCAL_EXPORT_C_INCLUDE_DIRS := $(LOCAL_PATH)

# Symlink libkmsetkey_ca.trusty.so -> ree_import_attest_keybox_ca.so
#LOCAL_POST_INSTALL_CMD = \
#	$(hide) ln -sf $(notdir $(LOCAL_INSTALLED_MODULE)) $(dir $(LOCAL_INSTALLED_MODULE))ree_import_attest_keybox_ca.so

include $(BUILD_SHARED_LIBRARY)

include $(CLEAR_VARS)

LOCAL_MODULE := kmsetkey_ca.trusty

LOCAL_PROPRIETARY_MODULE := true
LOCAL_MODULE_OWNER := mtk
LOCAL_MODULE_TAGS := optional

LOCAL_SRC_FILES := \
        main.cpp

LOCAL_C_INCLUDES:= \
        $(LOCAL_PATH)

LOCAL_SHARED_LIBRARIES := \
	libtrusty \
	liblog \
	libcutils \
	libkmsetkey_ca.trusty

LOCAL_HEADER_LIBRARIES := \
	libhardware_headers

LOCAL_CLFAGS = -fvisibility=hidden -Wall -Werror

include $(BUILD_EXECUTABLE)

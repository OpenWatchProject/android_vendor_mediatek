LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_SRC_FILES:= mntl_link.cpp

LOCAL_SHARED_LIBRARIES := \
    libc

LOCAL_STATIC_LIBRARIES := \
    libbase \
    libcutils \
    libselinux \
    libcrypto_utils \
    libcrypto

LOCAL_MODULE:= mntl_link
LOCAL_REQUIRED_MODULES := mntl_link_recovery
LOCAL_PROPRIETARY_MODULE := true
LOCAL_MODULE_OWNER := mtk
include $(BUILD_EXECUTABLE)

#############################################

include $(CLEAR_VARS)

LOCAL_SRC_FILES:= mntl_link.cpp

LOCAL_STATIC_LIBRARIES := \
    libbase \
    libcutils \
    libc \
    libselinux \
    libcrypto_utils \
    libcrypto

LOCAL_FORCE_STATIC_EXECUTABLE := true
LOCAL_MODULE:= mntl_link_recovery
LOCAL_MODULE_STEM := mntl_link
LOCAL_MODULE_PATH := $(TARGET_RECOVERY_ROOT_OUT)/sbin
include $(BUILD_EXECUTABLE)

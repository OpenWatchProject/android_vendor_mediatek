LOCAL_PATH := $(call my-dir)

ifeq (,$(wildcard vendor/mediatek/proprietary/frameworks/opt/mdlogger))
include $(CLEAR_VARS)
LOCAL_MODULE := mdlogger
LOCAL_MODULE_CLASS := EXECUTABLES
LOCAL_MODULE_TAGS := optional
LOCAL_SHARED_LIBRARIES := libcutils libselinux libmdloggerrecycle libmemoryDumpEncoder liblog
LOCAL_INIT_RC := mdlogger.rc
LOCAL_MULTILIB := 32
LOCAL_SRC_FILES_32 := arm_$(TARGET_ARCH_VARIANT)_$(TARGET_CPU_VARIANT)/mdlogger
include $(BUILD_PREBUILT)
endif

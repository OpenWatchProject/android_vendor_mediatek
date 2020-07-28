LOCAL_PATH:= $(call my-dir)

include $(CLEAR_VARS)

ifeq ($(MTK_GAUGE_VERSION), 30)
	LOCAL_PROPRIETARY_MODULE := true
	LOCAL_MODULE_OWNER := mtk
	LOCAL_MODULE_CLASS := SHARED_LIBRARIES
	LOCAL_MODULE := libfgauge_gm30
ifeq ($(LINUX_KERNEL_VERSION), kernel-4.4)
	LOCAL_SRC_FILES_arm := kernel-4.4/libfgauge_gm30.so
endif
ifeq ($(LINUX_KERNEL_VERSION), kernel-4.9)
	LOCAL_SRC_FILES_arm := kernel-4.9/libfgauge_gm30.so
endif
ifeq ($(LINUX_KERNEL_VERSION), kernel-4.9-lc)
	LOCAL_SRC_FILES_arm := kernel-4.9-lc/libfgauge_gm30.so
endif
ifeq ($(LINUX_KERNEL_VERSION), kernel-4.14)
	LOCAL_SRC_FILES_arm := kernel-4.14/libfgauge_gm30.so
endif
ifeq ($(LINUX_KERNEL_VERSION), kernel-4.19)
	LOCAL_SRC_FILES_arm := kernel-4.19/libfgauge_gm30.so
endif
	LOCAL_MULTILIB := 32
	LOCAL_MODULE_SUFFIX := .so
	include $(BUILD_PREBUILT)
else
	LOCAL_PROPRIETARY_MODULE := true
	LOCAL_MODULE_OWNER := mtk
	LOCAL_MODULE_CLASS := SHARED_LIBRARIES
	LOCAL_MODULE := libfgauge
	LOCAL_SRC_FILES_arm := libfgauge.so
	LOCAL_MULTILIB := 32
	LOCAL_MODULE_SUFFIX := .so
	include $(BUILD_PREBUILT)
endif

LOCAL_PATH := $(call my-dir)
#
# libvsync_enhance.so
#
include $(CLEAR_VARS)

LOCAL_MODULE := libvsync_enhance
LOCAL_PROPRIETARY_MODULE := false
LOCAL_MODULE_OWNER := mtk

LOCAL_MODULE_TAGS := optional

LOCAL_CFLAGS := -DLOG_TAG=\"SurfaceFlinger\"

ifneq ($(MTK_LCM_PHYSICAL_ROTATION), 0)
	LOCAL_CFLAGS += -DMTK_SF_HW_ROTATION_SUPPORT
endif

LOCAL_CFLAGS += -DMTK_SF_DEBUG_SUPPORT
LOCAL_CPPFLAGS += -DMTK_SF_DEBUG_SUPPORT
LOCAL_CFLAGS += -DMTK_AOSP_DISPLAY_BUGFIX
LOCAL_CPPFLAGS += -DMTK_AOSP_DISPLAY_BUGFIX
LOCAL_CFLAGS += -DMTK_SF_WATCHDOG_SUPPORT
LOCAL_CPPFLAGS += -DMTK_SF_WATCHDOG_SUPPORT
LOCAL_CFLAGS += -DMTK_GPU_DVFS_SUPPORT
LOCAL_CPPFLAGS += -DMTK_GPU_DVFS_SUPPORT
LOCAL_CFLAGS += -DMTK_VSYNC_ENHANCEMENT_SUPPORT
LOCAL_CPPFLAGS += -DMTK_VSYNC_ENHANCEMENT_SUPPORT
LOCAL_CFLAGS += -DMTK_DISPLAY_DEJITTER
LOCAL_CPPFLAGS += -DMTK_DISPLAY_DEJITTER
LOCAL_CFLAGS += -DMTK_BOOT_PROF
LOCAL_CPPFLAGS += -DMTK_BOOT_PROF

LOCAL_SRC_FILES := \
	DispSyncEnhancement.cpp

LOCAL_C_INCLUDES := \
	$(TOP)/$(MTK_ROOT)/hardware/libgem/inc \
	$(TOP)/$(MTK_ROOT)/hardware/include \
	frameworks/native/services/surfaceflinger \
	frameworks/native/services/surfaceflinger/Scheduler \
	$(TOP)/$(MTK_ROOT)/hardware/libdfps/inc

LOCAL_SHARED_LIBRARIES := \
	libcutils \
	libutils \
	liblog \
	libdl \
	libfmq \
	libgui \
	libbase \
	vendor.mediatek.hardware.gpu@1.0 \
	libhidlbase

LOCAL_CFLAGS += -Wall -Werror -Wunused -Wunreachable-code
include $(BUILD_SHARED_LIBRARY)

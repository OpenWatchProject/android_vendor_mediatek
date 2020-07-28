# Copyright 2006 The Android Open Source Project
###############################################################################
LOCAL_PATH:= $(call my-dir)
###############################################################################
include $(CLEAR_VARS)
LOCAL_SRC_FILES := libhwm.c
LOCAL_MODULE := libhwm
LOCAL_PROPRIETARY_MODULE := true
LOCAL_MODULE_OWNER := mtk
LOCAL_SHARED_LIBRARIES += libnvram liblog libfile_op
#bobule workaround pdk build error, needing review
#LOCAL_UNSTRIPPED_PATH := $(TARGET_ROOT_OUT_SBIN_UNSTRIPPED)
LOCAL_C_INCLUDES := $(LOCAL_PATH)/include $(MTK_PATH_SOURCE)/external/audio_utils/common_headers/cgen
LOCAL_HEADER_LIBRARIES := \
	libnvram_headers \
	libfile_op_headers
LOCAL_EXPORT_C_INCLUDE_DIRS := $(LOCAL_PATH)/include
ifneq (,$(filter $(strip $(TARGET_BOARD_PLATFORM)), mt6735 mt6737t mt6735m mt6737m mt6753 mt6570 mt6580 mt6752 mt6755 mt6757 mt6750 mt6797 mt6799 mt6759 mt6758 mt6763 mt6739 mt8163 mt8167 mt8173))
    LOCAL_CFLAGS += -DSUPPORT_SENSOR_ACCESS_NVRAM
endif
include $(MTK_SHARED_LIBRARY)
###############################################################################

ifneq ($(BOARD_MTK_LIBSENSORS_NAME),)
ifneq ($(BOARD_MTK_LIB_SENSOR),)
ifneq ($(BOARD_MTK_LIB_SENSOR_NO),)
###############################################################################
include $(CLEAR_VARS)
LOCAL_SRC_FILES := cmdtool.c
LOCAL_MODULE := sensor_cmd
LOCAL_PROPRIETARY_MODULE := true
LOCAL_MODULE_OWNER := mtk
LOCAL_MODULE_PATH := $(TARGET_OUT_VENDOR_OPTIONAL_EXECUTABLES)	#install to system/xbin
LOCAL_UNSTRIPPED_PATH := $(TARGET_ROOT_OUT_SBIN_UNSTRIPPED)
LOCAL_STATIC_LIBRARIES += libm
LOCAL_SHARED_LIBRARIES += libnvram libhwm libfile_op libc libcutils
include $(MTK_EXECUTABLE)
endif
endif
endif

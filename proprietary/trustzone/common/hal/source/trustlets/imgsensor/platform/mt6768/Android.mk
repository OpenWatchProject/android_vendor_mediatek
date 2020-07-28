#
# Copyright (c) 2018 MediaTek Inc.
#
# Permission is hereby granted, free of charge, to any person obtaining
# a copy of this software and associated documentation files
# (the "Software"), to deal in the Software without restriction,
# including without limitation the rights to use, copy, modify, merge,
# publish, distribute, sublicense, and/or sell copies of the Software,
# and to permit persons to whom the Software is furnished to do so,
# subject to the following conditions:
#
# The above copyright notice and this permission notice shall be
# included in all copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
# EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
# MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
# IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
# CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
# TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
# SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
#

ifeq ($(strip $(MTK_CAM_SECURITY_SUPPORT)), yes)
ifeq ($(strip $(MTK_TEE_GP_SUPPORT)), yes)

LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

IMGSENSOR_COMMON_SRC = ../../common

LOCAL_MODULE := libimgsensor_sec_custom
LOCAL_PROPRIETARY_MODULE := true

#include folder
LOCAL_C_INCLUDES += \
    $(TOP)/device/mediatek/common/kernel-headers/ \
    $(TOP)/vendor/mediatek/proprietary/hardware/mtkcam/drv/src/sensor/$(TARGET_BOARD_PLATFORM) \
    $(TOP)/vendor/mediatek/proprietary/custom/$(TARGET_BOARD_PLATFORM)/hal/inc \
    $(TOP)/$(MTK_PATH_CUSTOM_PLATFORM)/hal/inc \
    $(TOP)/$(MTK_PATH_CUSTOM)/hal/inc \
    $(TOP)/$(MTK_PATH_SOURCE)/hardware/mtkcam/include

LOCAL_SRC_FILES += \
    $(IMGSENSOR_COMMON_SRC)/sensor_cfg_sec.cpp \
    $(IMGSENSOR_COMMON_SRC)/seninf_drv_csi_info.cpp

IMGSENSOR_CUSTOM_PATH := ../../../../../../../../custom
ifeq ($(wildcard $(IMGSENSOR_CUSTOM_PATH)/$(MTK_PROJECT)/hal/imgsensor_src),)
    LOCAL_SRC_FILES += $(IMGSENSOR_CUSTOM_PATH)/$(TARGET_BOARD_PLATFORM)/hal/imgsensor_src/cfg_setting_imgsensor.cpp
else
    LOCAL_SRC_FILES += $(IMGSENSOR_CUSTOM_PATH)/$(MTK_PROJECT)/hal/imgsensor_src/cfg_setting_imgsensor.cpp
endif

LOCAL_SHARED_LIBRARIES := libTEECommon
LOCAL_SHARED_LIBRARIES += liblog

LOCAL_CPPFLAGS += -fno-exceptions

include vendor/mediatek/proprietary/trustzone/custom/build/tee_static_library.mk

endif # ifeq ($(strip $(MTK_TEE_GP_SUPPORT)), yes)
endif

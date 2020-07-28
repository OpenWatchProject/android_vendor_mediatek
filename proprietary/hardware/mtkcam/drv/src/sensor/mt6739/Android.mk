# Copyright Statement:
#
# This software/firmware and related documentation ("MediaTek Software") are
# protected under relevant copyright laws. The information contained herein
# is confidential and proprietary to MediaTek Inc. and/or its licensors.
# Without the prior written permission of MediaTek inc. and/or its licensors,
# any reproduction, modification, use or disclosure of MediaTek Software,
# and information contained herein, in whole or in part, shall be strictly prohibited.

# MediaTek Inc. (C) 2010. All rights reserved.
#
# BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
# THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
# RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER ON
# AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES,
# EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
# MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
# NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
# SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
# SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES TO LOOK ONLY TO SUCH
# THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. RECEIVER EXPRESSLY ACKNOWLEDGES
# THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES
# CONTAINED IN MEDIATEK SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK
# SOFTWARE RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
# STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND
# CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,
# AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE,
# OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY RECEIVER TO
# MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
#
# The following software/firmware and/or related documentation ("MediaTek Software")
# have been modified by MediaTek Inc. All revisions are subject to any receiver's
# applicable license agreements with MediaTek Inc.


################################################################################
#
################################################################################

LOCAL_PATH := $(call my-dir)

################################################################################
#
################################################################################
include $(CLEAR_VARS)

#-----------------------------------------------------------
-include $(TOP)/$(MTK_PATH_SOURCE)/hardware/mtkcam/mtkcam.mk

#-----------------------------------------------------------
MTKCAM_USE_LEGACY_SENSOR_HAL    := '0'

SENINF_COMMON_SRC = ../common/v1
SENINF_COMMON_INC = $(LOCAL_PATH)/$(SENINF_COMMON_SRC)

#-----------------------------------------------------------
LOCAL_SRC_FILES := \
    $(SENINF_COMMON_SRC)/HalSensor.control.cpp \
    $(SENINF_COMMON_SRC)/HalSensor.cpp \
    $(SENINF_COMMON_SRC)/HalSensorList.buildStaticInfo.cpp \
    $(SENINF_COMMON_SRC)/HalSensorList.cpp \
    $(SENINF_COMMON_SRC)/HalSensorList.enumList.cpp \
    $(SENINF_COMMON_SRC)/HalSensorList.getInfo.cpp \
    $(SENINF_COMMON_SRC)/HalSensorList.openList.cpp \
    $(SENINF_COMMON_SRC)/imgsensor_drv.cpp \
    seninf_drv.cpp

ifeq ($(strip $(MTK_CAM_HAL_VERSION)),3)
LOCAL_SRC_FILES += ../common/v1_1/hwsync_drv.cpp
else
LOCAL_SRC_FILES += hwsync_drv.cpp
endif

#-----------------------------------------------------------

LOCAL_C_INCLUDES += $(SENINF_COMMON_INC)
LOCAL_C_INCLUDES += $(MTKCAM_C_INCLUDES)

LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_SOURCE)/hardware/gralloc_extra/include

ifeq ($(MTKCAM_IP_BASE),0)
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_SOURCE)/hardware/mtkcam/legacy/include
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_SOURCE)/hardware/mtkcam/legacy/platform/$(TARGET_BOARD_PLATFORM)/include/mtkcam/drv
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_SOURCE)/hardware/mtkcam/legacy/platform/$(TARGET_BOARD_PLATFORM)/include
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_SOURCE)/hardware/mtkcam/legacy/platform/$(TARGET_BOARD_PLATFORM)/core/drv
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_SOURCE)/hardware/mtkcam/legacy/platform/$(TARGET_BOARD_PLATFORM)/include/mtkcam/imageio
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_CUSTOM_PLATFORM_HAL1)/hal/inc
else
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_SOURCE)/hardware/mtkcam/include
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_SOURCE)/hardware/mtkcam/drv/include/$(TARGET_BOARD_PLATFORM)/drv
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_SOURCE)/hardware/mtkcam/drv/include/$(TARGET_BOARD_PLATFORM)/imageio
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_SOURCE)/hardware/mtkcam/drv/src/isp/$(TARGET_BOARD_PLATFORM)/drv/inc
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_SOURCE)/hardware/mtkcam/drv/src/isp/$(TARGET_BOARD_PLATFORM)
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_CUSTOM_PLATFORM)/hal/inc
endif

#LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_SOURCE)/hardware/include

#

LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_CUSTOM)/kernel/sensor/inc
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_COMMON)/kernel/imgsensor/inc
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_COMMON)/hal/inc
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_SOURCE)/external/cameraprofile
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_SOURCE)/external/camera/cameraprofile
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_SOURCE)/hardware/ldvt/$(TARGET_BOARD_PLATFORM)/include
#
#Thread Priority
LOCAL_C_INCLUDES += $(TOP)/system/core/include
#

LOCAL_HEADER_LIBRARIES := libhardware_headers

#-----------------------------------------------------------
ifeq ($(TARGET_BUILD_VARIANT), user)
MTKCAM_LOGENABLE_DEFAULT   := 0
else
MTKCAM_LOGENABLE_DEFAULT   := 1
endif

#-----------------------------------------------------------
ifeq ($(BUILD_MTK_LDVT),yes)
    LOCAL_CFLAGS += -DUSING_MTK_LDVT
endif

ifneq ($(CUSTOM_KERNEL_MAIN2_IMGSENSOR),)
    LOCAL_CFLAGS += -DMTK_MAIN2_IMGSENSOR
ifeq ($(MTK_CAM_MAX_NUMBER_OF_CAMERA),)
    LOCAL_CFLAGS += -DMTK_CAM_MAX_NUMBER_OF_CAMERA=3
endif
endif

ifneq ($(CUSTOM_KERNEL_SUB_IMGSENSOR),)
    LOCAL_CFLAGS += -DMTK_SUB_IMGSENSOR
endif

ifeq ($(strip $(MTK_CAM_MAX_NUMBER_OF_CAMERA)), 4)
    MTKCAM_CFLAGS += -DMTK_SUB2_IMGSENSOR
endif

ifneq ($(MTK_CAM_MAX_NUMBER_OF_CAMERA),)
    LOCAL_CFLAGS += -DMTK_CAM_MAX_NUMBER_OF_CAMERA=$(MTK_CAM_MAX_NUMBER_OF_CAMERA)
endif

#-----------------------------------------------------------
LOCAL_CFLAGS += $(MTKCAM_CFLAGS)
LOCAL_CFLAGS += -DMTKCAM_LOGENABLE_DEFAULT=$(MTKCAM_LOGENABLE_DEFAULT)
#-----------------------------------------------------------
LOCAL_WHOLE_STATIC_LIBRARIES += libcam.halsensor.custom
ifeq ($(BUILD_MTK_LDVT),yes)
    LOCAL_WHOLE_STATIC_LIBRARIES += libuvvf
endif
#
LOCAL_STATIC_LIBRARIES :=

#-----------------------------------------------------------
LOCAL_SHARED_LIBRARIES += liblog
LOCAL_SHARED_LIBRARIES += libcutils
LOCAL_SHARED_LIBRARIES += libdl
LOCAL_SHARED_LIBRARIES += libutils
LOCAL_SHARED_LIBRARIES += libcameracustom
LOCAL_SHARED_LIBRARIES += libc++

LOCAL_SHARED_LIBRARIES += libmtkcam_modulehelper
LOCAL_SHARED_LIBRARIES += libmtkcam_stdutils
LOCAL_SHARED_LIBRARIES += libmtkcam_metadata
#LOCAL_SHARED_LIBRARIES += libcamdrv_isp
LOCAL_SHARED_LIBRARIES += libmtkcam_ulog

# hardware sync
#LOCAL_SHARED_LIBRARIES += libcamdrv

#
#LOCAL_MULTILIB := 64

#-----------------------------------------------------------
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE := libcam.halsensor
LOCAL_PROPRIETARY_MODULE := true
LOCAL_MODULE_OWNER := mtk
#-----------------------------------------------------------
include $(MTK_SHARED_LIBRARY)


################################################################################
#
################################################################################
include $(CLEAR_VARS)
include $(call all-makefiles-under,$(LOCAL_PATH))


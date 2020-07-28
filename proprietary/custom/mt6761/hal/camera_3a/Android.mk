# Copyright Statement:
#
# This software/firmware and related documentation ("MediaTek Software") are
# protected under relevant copyright laws. The information contained herein
# is confidential and proprietary to MediaTek Inc. and/or its licensors.
# Without the prior written permission of MediaTek inc. and/or its licensors,
# any reproduction, modification, use or disclosure of MediaTek Software,
# and information contained herein, in whole or in part, shall be strictly prohibited.

# MediaTek Inc. (C) 2017. All rights reserved.
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

ifeq ($(wildcard $(MTK_PATH_CUSTOM)/hal/camera_3a),)
################################################################################
#
################################################################################

LOCAL_PATH := $(call my-dir)


include $(CLEAR_VARS)

LOCAL_MODULE:= lib_camera_special_headers

#-----------------------------------------------------------
LOCAL_EXPORT_C_INCLUDE_DIRS += $(TOP)/$(MTK_PATH_CUSTOM)/hal/inc \
                    $(TOP)/$(MTK_PATH_CUSTOM)/hal/inc/isp_tuning \
                    $(TOP)/$(MTK_PATH_CUSTOM_PLATFORM)/hal/inc/isp_tuning \
                    $(TOP)/$(MTK_PATH_CUSTOM)/hal/inc/aaa \
                    $(TOP)/$(MTK_PATH_CUSTOM)/hal/camera/inc \

#
LOCAL_EXPORT_C_INCLUDE_DIRS += $(TOP)/$(MTK_MTKCAM_PLATFORM)/include
LOCAL_EXPORT_C_INCLUDE_DIRS += $(MTKCAM_C_INCLUDES)
LOCAL_EXPORT_C_INCLUDE_DIRS += $(MTKCAM_C_INCLUDES)/..
$(info MTK_MTKCAM_PLATFORM = $(MTK_MTKCAM_PLATFORM))
LOCAL_EXPORT_C_INCLUDE_DIRS += $(TOP)/$(MTK_MTKCAM_PLATFORM)/include
LOCAL_EXPORT_C_INCLUDE_DIRS += $(TOP)/$(MTK_PATH_CUSTOM_PLATFORM)/hal/inc/aaa
LOCAL_EXPORT_C_INCLUDE_DIRS += $(TOP)/$(MTK_PATH_CUSTOM_PLATFORM)/hal/inc/lomo
LOCAL_EXPORT_C_INCLUDE_DIRS += $(TOP)/$(MTK_PATH_CUSTOM_PLATFORM)/hal/inc/lomo_jni
LOCAL_EXPORT_C_INCLUDE_DIRS += $(TOP)/$(MTK_PATH_CUSTOM_PLATFORM)/hal/inc/mtkcam
LOCAL_EXPORT_C_INCLUDE_DIRS += $(TOP)/$(MTK_PATH_CUSTOM_PLATFORM)/hal/inc
LOCAL_EXPORT_C_INCLUDE_DIRS += $(TOP)/$(MTK_PATH_COMMON)/kernel/imgsensor/inc
LOCAL_EXPORT_C_INCLUDE_DIRS += $(TOP)/$(MTK_PATH_COMMON)/hal/inc/camera_feature
LOCAL_EXPORT_C_INCLUDE_DIRS += $(TOP)/$(MTK_PATH_COMMON)/hal/inc
LOCAL_EXPORT_C_INCLUDE_DIRS += $(TOP)/$(MTK_PATH_SOURCE)/hardware/include
LOCAL_EXPORT_C_INCLUDE_DIRS += $(TOP)/$(MTK_PATH_PLATFORM)/hardware/include
LOCAL_EXPORT_C_INCLUDE_DIRS += $(TOP)/system/media/camera/include
LOCAL_EXPORT_C_INCLUDE_DIRS += $(TOP)/$(MTK_PATH_SOURCE)/hardware/mtkcam/include/algorithm/$(shell echo $(MTK_PLATFORM) | tr A-Z a-z)
LOCAL_EXPORT_C_INCLUDE_DIRS += $(TOP)/$(MTK_PATH_SOURCE)/hardware/mtkcam/common/include/
LOCAL_EXPORT_C_INCLUDE_DIRS += $(TOP)/$(MTK_PATH_SOURCE)/hardware/mtkcam/aaa/include/
LOCAL_EXPORT_C_INCLUDE_DIRS += $(TOP)/$(MTK_PATH_SOURCE)/external/aee/binary/inc
LOCAL_EXPORT_C_INCLUDE_DIRS += $(TOP)/$(MTK_PATH_CUSTOM_PLATFORM)/hal/inc/aaa
LOCAL_EXPORT_C_INCLUDE_DIRS += $(TOP)/$(MTK_PATH_SOURCE)/hardware/libcamera_3a/libgma_lib/lib3a.gma/$(PLATFORM)/include
include $(BUILD_HEADER_LIBRARY)


################################################################################
#
################################################################################
include $(CLEAR_VARS)

sinclude $(TOP)/$(MTK_PATH_SOURCE)/hardware/mtkcam/mtkcam.mk
#-----------------------------------------------------------
#$(call config-custom-folder,hal:hal)

#-----------------------------------------------------------
#====== FLASHLIGHT_OFFSET_CALI ======
ifeq ($(MTK_CAM_FLASHLIGHT_OFFSET_CALI_SUPPORT),yes)
    LOCAL_CFLAGS += -DFLASHLIGHT_OFFSET_CALI
endif
#-----------------------------------------------------------
LOCAL_SRC_FILES += \
  af_flow_custom.cpp \
  af_tuning_custom.cpp \
  af_tuning_custom_main.cpp \
  af_tuning_custom_main2.cpp \
  af_tuning_custom_main3.cpp \
  af_tuning_custom_sub.cpp \
  af_tuning_custom_sub2.cpp \
  aaa_common_custom.cpp \
  aaa_scheduling_custom_main.cpp \
  aaa_scheduling_custom_main2.cpp \
  aaa_scheduling_custom_sub.cpp \
  aaa_scheduling_param_custom.cpp \
  aaa_yuv_tuning_custom.cpp \
  ae_tuning_custom_main.cpp \
  ae_tuning_custom_main2.cpp \
  ae_tuning_custom_main3.cpp \
  ae_tuning_custom_sub.cpp \
  ae_tuning_custom_sub2.cpp \
  ae_flow_custom.cpp \
  awb_tuning_custom_main3.cpp \
  awb_tuning_custom_sub2.cpp \
  awb_tuning_custom_main2.cpp \
  awb_tuning_custom_sub.cpp \
  awb_tuning_custom_main.cpp \
  camera_custom_flicker.cpp \
  isp_tuning_custom.cpp \
  isp_tuning_effect.cpp \
  isp_tuning_idx.cpp \
  isp_tuning_idx_capture.cpp \
  isp_tuning_idx_preview.cpp \
  isp_tuning_idx_video.cpp \
  isp_tuning_user.cpp \
  shading_tuning_custom.cpp \
  tsf_tuning_custom.cpp \
  flashawb_tuning_custom.cpp \
  camera_custom_msdk.cpp \
  camera_custom_if_yuv.cpp \
  camera_custom_if.cpp \
  n3d_sync2a_tuning_param.cpp \
  nvram_custom.cpp



#-------------------------------------------------------------------------------
LOCAL_HEADER_LIBRARIES += liblog_headers libcutils_headers libutils_headers libhardware_headers

LOCAL_HEADER_LIBRARIES += lib_camera_special_headers
#-------------------------------------------------------------------------------
# If sensor list contains IMX377, set ISO support mode to 12800 to main camera
#-------------------------------------------------------------------------------
ifneq (,$(findstring imx377_mipi_raw,$(CUSTOM_HAL_IMGSENSOR)))
LOCAL_CFLAGS += -DISO_TOP_SUPPORT_MODE_main=2
endif

# for Stereo Feature
ifeq ($(MTK_CAM_STEREO_CAMERA_SUPPORT),yes)
LOCAL_CFLAGS += -DCAM3_STEREO_FEATURE_EN=1
endif

#-----------------------------------------------------------
LOCAL_WHOLE_STATIC_LIBRARIES += libcam.hal3a_utils
#
LOCAL_WHOLE_STATIC_LIBRARIES +=
LOCAL_SHARED_LIBRARIES += libaedv
#LOCAL_SHARED_LIBRARIES += lib3a.gma

#-----------------------------------------------------------
LOCAL_MODULE := libcameracustom.camera.3a
LOCAL_PROPRIETARY_MODULE := true
LOCAL_MODULE_OWNER := mtk
#-----------------------------------------------------------
include $(MTK_STATIC_LIBRARY)

################################################################################
#
################################################################################
#include $(CLEAR_VARS)
#include $(call all-makefiles-under,$(LOCAL_PATH))

endif

################################################################################
#
################################################################################
include $(CLEAR_VARS)

#-----------------------------------------------------------
LOCAL_SRC_FILES += aaa_log.cpp

#-----------------------------------------------------------

#-----------------------------------------------------------
# MTKCAM_LOG_LEVEL_DEFAULT for compile-time loglevel control
MTKCAM_LOG_LEVEL_DEFAULT   := 4
LOG_EXCEPTION_LIST :=
ifneq ($(filter $(TARGET_BOARD_PLATFORM),$(LOG_EXCEPTION_LIST)),$(TARGET_BOARD_PLATFORM))
ifeq ($(TARGET_BUILD_VARIANT), user)
    MTKCAM_LOG_LEVEL_DEFAULT   := 2
else ifeq ($(TARGET_BUILD_VARIANT), userdebug)
# for user debug load & MTKCAM_USER_DBG_LOG_OFF (depends on special customer's request)
# change default log level to ALOGI
ifeq ($(MTK_CAM_USER_DBG_LOG_OFF), yes)
    MTKCAM_LOG_LEVEL_DEFAULT   := 2
else
    MTKCAM_LOG_LEVEL_DEFAULT   := 3
endif
endif
endif
LOCAL_CFLAGS += -DMTKCAM_LOG_LEVEL_DEFAULT=$(MTKCAM_LOG_LEVEL_DEFAULT)

#-----------------------------------------------------------
LOCAL_SHARED_LIBRARIES += liblog
LOCAL_SHARED_LIBRARIES += libutils
LOCAL_SHARED_LIBRARIES += libcutils
#LOCAL_STATIC_LIBRARIES += libc
#
LOCAL_WHOLE_STATIC_LIBRARIES +=

LOCAL_HEADER_LIBRARIES += lib_camera_special_headers
#-----------------------------------------------------------
LOCAL_MODULE := libcam.hal3a.log
LOCAL_PROPRIETARY_MODULE := true
LOCAL_MODULE_OWNER := mtk
#-----------------------------------------------------------
include $(MTK_SHARED_LIBRARY)

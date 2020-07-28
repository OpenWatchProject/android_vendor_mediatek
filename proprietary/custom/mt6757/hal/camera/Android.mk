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

# Use project first
# ifeq ($(wildcard $(MTK_PATH_CUSTOM)/hal/camera),)

################################################################################
#
################################################################################

LOCAL_PATH := $(call my-dir)

################################################################################
#
################################################################################
include $(CLEAR_VARS)

#-----------------------------------------------------------
#$(call config-custom-folder,hal:hal)

#-----------------------------------------------------------
define add_src_file
	ifeq ($(wildcard $(MTK_PATH_CUSTOM)/hal/camera/$1),)
		LOCAL_SRC_FILES += $1
	else
		LOCAL_SRC_FILES += ../../../$(MTK_PROJECT)/hal/camera/$1
	endif
endef

SRC_FILE_LIST += \
  camera_custom_3dnr.cpp \
  camera_custom_asd.cpp \
  camera_custom_capture_nr.cpp \
  camera_custom_fb.cpp \
  camera_custom_fd.cpp \
  camera_custom_gesture.cpp \
  camera_custom_hdr.cpp \
  camera_custom_n3d.cpp \
  camera_custom_ot.cpp \
  camera_custom_stereo.cpp \
  camera_custom_vfb.cpp \
  camera_custom_vt.cpp \
  camera_custom_zsd.cpp \
  camera_custom_hrd.cpp \
  camera_custom_smvr.cpp \
  camera_custom_isp_limitation.cpp \
  camera_custom_scenario_control.cpp \
  camera_custom_feature_table.cpp \

ifeq ($(strip $(MTK_CAM_DUAL_ZOOM_SUPPORT)), yes)
SRC_FILE_LIST += camera_custom_dualzoom.cpp
endif

#MTK_CAM_SW_VERSION: ver1 for Kibo, ver2 for Kibo+
SRC_FILE_LIST += \
  $(MTK_CAM_SW_VERSION)/camera_custom_ivhdr.cpp \
  $(MTK_CAM_SW_VERSION)/camera_custom_mvhdr.cpp \
  $(MTK_CAM_SW_VERSION)/camera_custom_zvhdr.cpp \

$(foreach file,$(SRC_FILE_LIST),$(eval $(call add_src_file,$(file))))

#-----------------------------------------------------------
PLATFORM := $(shell echo $(MTK_PLATFORM) | tr A-Z a-z)
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_CUSTOM)/hal/inc \
                    $(TOP)/$(MTK_PATH_CUSTOM)/hal/inc/isp_tuning \
                    $(TOP)/$(MTK_PATH_CUSTOM)/hal/inc/aaa \
                    $(TOP)/$(MTK_PATH_CUSTOM)/hal/camera/inc \

#
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_CUSTOM_PLATFORM)/hal/inc/$(MTK_CAM_SW_VERSION)
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_CUSTOM_PLATFORM)/hal/inc/aaa
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_CUSTOM_PLATFORM)/hal/inc/aaa/$(MTK_CAM_SW_VERSION)
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_CUSTOM_PLATFORM)/hal/inc/lomo
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_CUSTOM_PLATFORM)/hal/inc/lomo_jni
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_CUSTOM_PLATFORM)/hal/inc/mtkcam
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_CUSTOM_PLATFORM)/hal/inc
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_COMMON)/kernel/imgsensor/inc
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_COMMON)/hal/inc/camera_feature
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_COMMON)/hal/inc
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_SOURCE)/hardware/include
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_SOURCE)/hardware/mtkcam/include
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_SOURCE)/hardware/mtkcam/common/include
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_SOURCE)/hardware/mtkcam
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_SOURCE)/hardware/perfservice
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_SOURCE)/hardware/mtkcam/include/algorithm/$(PLATFORM)
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_SOURCE)/hardware/gralloc_extra/include

#-----------------------------------------------------------
# MTKCAM_LOG_LEVEL_DEFAULT for compile-time loglevel control
MTKCAM_LOG_LEVEL_DEFAULT   := 4

LOG_EXCEPTION_LIST :=
ifneq ($(filter $(TARGET_BOARD_PLATFORM),$(LOG_EXCEPTION_LIST)),$(TARGET_BOARD_PLATFORM))
ifeq ($(TARGET_BUILD_VARIANT), user)
    MTKCAM_LOG_LEVEL_DEFAULT   := 2
else ifeq ($(TARGET_BUILD_VARIANT), userdebug)
    MTKCAM_LOG_LEVEL_DEFAULT   := 3
endif
endif

LOCAL_CFLAGS := -DMTKCAM_LOG_LEVEL_DEFAULT=$(MTKCAM_LOG_LEVEL_DEFAULT)

ifeq ($(TARGET_BUILD_VARIANT), eng)
LOCAL_CFLAGS += -DMTK_EIS_IS_ENG_LOAD
endif

ifeq ($(MTK_GMO_RAM_OPTIMIZE), yes)
LOCAL_CFLAGS += -DMTK_EIS_IS_GMO_PROJ
endif

#-----------------------------------------------------------
# MTK_BASIC_PACKAGE
ifeq ($(MTK_BASIC_PACKAGE), yes)
LOCAL_CFLAGS += -DMTKCAM_BASIC_PACKAGE
endif
#-----------------------------------------------------------
LOCAL_STATIC_LIBRARIES += libbase libcutils
LOCAL_HEADER_LIBRARIES += libutils_headers
LOCAL_HEADER_LIBRARIES += libmtkcam3_headers
#-----------------------------------------------------------
LOCAL_MODULE := libcameracustom.camera
LOCAL_PROPRIETARY_MODULE := true
LOCAL_MODULE_OWNER := mtk
#-----------------------------------------------------------
include $(MTK_STATIC_LIBRARY)

################################################################################
#
################################################################################
include $(CLEAR_VARS)
include $(call all-makefiles-under,$(LOCAL_PATH))

# endif

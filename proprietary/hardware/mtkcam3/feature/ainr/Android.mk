## Copyright Statement:
#
# This software/firmware and related documentation ("MediaTek Software") are
# protected under relevant copyright laws. The information contained herein
# is confidential and proprietary to MediaTek Inc. and/or its licensors.
# Without the prior written permission of MediaTek inc. and/or its licensors,
# any reproduction, modification, use or disclosure of MediaTek Software,
# and information contained herein, in whole or in part, shall be strictly prohibited.

# MediaTek Inc. (C) 2018. All rights reserved.
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
##
LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)

# default disable AINR, we will check if enable later
AINR_ENABLE := no

# describes platform information
AINR_PLATFORM := default

#------------------------------------------------------------------------------
# Default AINR Configurations
#------------------------------------------------------------------------------
# AINR core version, AINR core is platform-independent
AINR_CORE_VERSION_MAJOR                 := 1
AINR_CORE_VERSION_MINOR                 := 0

# AINR default version, for build pass usage
AINR_MODULE_VERSION_EXIFINFO            := default
AINR_MODULE_VERSION_NVRAM               := default
AINR_MODULE_VERSION_STRATEGY            := default
AINR_MODULE_VERSION_FEFM                := default
AINR_MODULE_VERSION_WPE                 := default
AINR_MODULE_VERSION_AIALGO              := default
#AINR_MODULE_VERSION_BUFPOOL            := default

# Describes which MF TAG version to use. 0 is to disable MF tag
# If for a new project early porting, to set this value to 0 to avoid
# compile error(s).
AINR_MF_TAG_VERSION                     := 0

# Describes which MF TAG sub version.
AINR_MF_TAG_SUBVERSION                  := 0

# Max available capture frame number
AINR_MAX_FRAMES                         := 10

# Describes blending number
# Note: This is a default value and will be updated by NVRAM
AINR_BLEND_FRAME                        := 6

# Describes capture frame number
# Note: This is a default value and will be updated by NVRAM
AINR_CAPTURE_FRAME                      := $(AINR_BLEND_FRAME)

# Describes MEMC type, if to use full size MC or not
# Note: This is a default value and will be updated by NVRAM
AINR_FULL_SIZE_MC                       := 0

# Describes the size of GYRO information queue
# Note: Phased-out after MFNR v2.0
AINR_GYRO_QUEUE_SIZE                    := 30

# Enable/Disable post-NR refine function or not. Post-NR refine is a method to
# rescure the noise level if some frames are dropped by BSS.
AINR_POST_NR_REFINE_ENABLE              := 1

# Enable/Disable content-award AIS feature, if this option is enable, AIS may
# refer to the information of image to calculate the final result which
# improves stabilization quality.
# Note: The option is available since MfllStrate v1.3,
AINR_CONTENT_AWARE_AIS_ENABLE           := 1

# Algorithm goes to ip-based, include path and .so maybe different
# Note: This option is for compiler usage, usually vendor never changes this
#       option
AINR_ALGO_FEATURE_DECOUPLING            := 0

# This value controls the priority of algorithm threads.
# The priority of threads will be 120 + this value.
# The smaller value has higher priority.
AINR_ALGO_THREADS_PRIORITY              := 0

# NVRAM 2.0 is coupling with isp_runing and aaa
# This value must set to 1 when NVRAM 2.0 is support
# and isp_tuning and aaa is not decoupling yet.
AINR_NVRAM_3A_TUNING_COUPLING           := 0

#------------------------------------------------------------------------------
# Platform Configurations
#------------------------------------------------------------------------------

# MT6779
# {{{
ifeq ($(TARGET_BOARD_PLATFORM), $(filter $(TARGET_BOARD_PLATFORM), mt6779))
  AINR_PLATFORM                 := mt6779
  AINR_ALGO_FEATURE_DECOUPLING  := 1

  AINR_MF_TAG_VERSION           := 9
  AINR_MF_TAG_SUBVERSION        := 4

  AINR_CORE_VERSION_MAJOR       := 1
  AINR_CORE_VERSION_MINOR       := 0

  AINR_MODULE_VERSION_EXIFINFO  := 1.0
  AINR_MODULE_VERSION_NVRAM     := 1.0
  AINR_MODULE_VERSION_STRATEGY  := 1.0
  AINR_MODULE_VERSION_FEFM      := 1.0
  AINR_MODULE_VERSION_WPE       := 1.0
#  AINR_MODULE_VERSION_AIALGO    := 1.0

  AINR_MC_THREAD_NUM            := 8

  AINR_USING_IMAGEBUFFER_QUEUE  := 0

  AINR_NVRAM_3A_TUNING_COUPLING := 1
endif
# }}}

# MT6885
# {{{
ifeq ($(TARGET_BOARD_PLATFORM), $(filter $(TARGET_BOARD_PLATFORM), mt6885))
  AINR_PLATFORM                 := mt6885
  AINR_ALGO_FEATURE_DECOUPLING  := 1

  AINR_MF_TAG_VERSION           := 9
  AINR_MF_TAG_SUBVERSION        := 4

  AINR_CORE_VERSION_MAJOR       := 2
  AINR_CORE_VERSION_MINOR       := 0

  AINR_MODULE_VERSION_EXIFINFO  := 1.0
  AINR_MODULE_VERSION_NVRAM     := 2.0
  AINR_MODULE_VERSION_STRATEGY  := 2.0
  AINR_MODULE_VERSION_FEFM      := 1.0
  AINR_MODULE_VERSION_WPE       := 1.0
  AINR_MODULE_VERSION_AIALGO    := 1.0

  AINR_MC_THREAD_NUM            := 8

  AINR_USING_IMAGEBUFFER_QUEUE  := 0

  AINR_NVRAM_3A_TUNING_COUPLING := 1
endif
# }}}

# re-check module version
# for LDVT, do just for compile pass, hence we set AINR_MODULE_VERSION to default
ifeq ($(BUILD_MTK_LDVT),yes)
  AINR_MODULE_VERSION_EXIFINFO      := default
  AINR_MODULE_VERSION_NVRAM         := default
  AINR_MODULE_VERSION_STRATEGY      := default
  AINR_MODULE_VERSION_FEFM          := default
  AINR_MODULE_VERSION_WPE           := default
  AINR_MODULE_VERSION_AIALGO        := default
endif

# check if enable AINR by ProjectConfig.mk
ifneq ($(strip $(MTKCAM_HAVE_AINR_SUPPORT)),0)
  AINR_ENABLE := yes
endif


#------------------------------------------------------------------------------
# AINR config start from here.
#------------------------------------------------------------------------------
ifeq ($(strip $(AINR_ENABLE)), yes)
# Define AINR core (algorithm) version and dependent middleware version
AINR_CORE_VERSION := $(AINR_CORE_VERSION_MAJOR).$(AINR_CORE_VERSION_MINOR)
$(warning AINR_CORE_VERSION: $(AINR_CORE_VERSION))

# AINR common interface location
AINR_HEADER_LIBRARIES := $(MTKCAM_INCLUDE_HEADER_LIB)
AINR_HEADER_LIBRARIES += libmtkcam_headers
AINR_HEADER_LIBRARIES += libmtkcam3_headers
AINR_HEADER_LIBRARIES += libcameracustom_headers
AINR_HEADER_LIBRARIES += libutils_headers liblog_headers libhardware_headers

# AINR libcamera_feature decoupling
AINR_INCLUDE_PATH_ALGO            += $(MTK_PATH_SOURCE)/hardware/libcamera_feature/libainr_lib/libcamalgo.ainr/$(AINR_PLATFORM)/include

# AINR licamera_feature for DNG
AINR_INCLUDE_PATH_ALGO            += $(MTK_PATH_SOURCE)/hardware/libcamera_feature/libdngop_lib/libcamalgo.dngop/include

AINR_LIB_NAME_ALGO                = libcamalgo.ainr

#
# AINR core library name
#
AINR_CORE_LIB_NAME := libainrcore


#------------------------------------------------------------------------------
# Define share library.
#------------------------------------------------------------------------------
AINR_SHARED_LIBS := libutils
AINR_SHARED_LIBS += libcutils
#AINR_SHARED_LIBS += libstdc++ # M toolchain
# MTKCAM standard
AINR_SHARED_LIBS += libmtkcam_stdutils # MTK cam standard library
AINR_SHARED_LIBS += libmtkcam_imgbuf
AINR_SHARED_LIBS += liblog # log library since Android O
AINR_SHARED_LIBS += $(AINR_LIB_NAME_ALGO)
AINR_SHARED_LIBS += libmtkcam_metadata
AINR_SHARED_LIBS += libmtkcam_ulog
AINR_SHARED_LIBS += libmtkcam_hwutils
# nvram
AINR_SHARED_LIBS += libcameracustom
AINR_SHARED_LIBS += libmtkcam_modulehelper
AINR_SHARED_LIBS += libmtkcam_mapping_mgr
# FEFM
ifeq ($(strip $(AINR_MODULE_VERSION_FEFM)),1.0)
  AINR_SHARED_LIBS += libcam.iopipe
endif
# AI-HDR/AI-NR 2.0
AINR_SHARED_LIBS += libcamalgo.ainr
ifeq ($(strip $(AINR_MODULE_VERSION_AIALGO)),1.0)
AINR_SHARED_LIBS += libcamalgo.aibc
AINR_SHARED_LIBS += libcamalgo.aihdr
endif
AINR_SHARED_LIBS += libcamalgo.fdft
# aee
ifeq ($(HAVE_AEE_FEATURE),yes)
  AINR_SHARED_LIBS += libaedv
endif

# ION
#ifeq ($(MTK_ION_SUPPORT), yes)
#  AINR_SHARED_LIBS += libion
#  AINR_SHARED_LIBS += libion_mtk
#endif

AINR_SHARED_LIBS += libladder
#
AINR_SHARED_LIBS += libutils
# For wokring buffer allocate
AINR_SHARED_LIBS += libcam.feature_utils
# Tuning file dump
AINR_SHARED_LIBS += libmtkcam_tuning_utils
# Thermal
AINR_SHARED_LIBS += libmtkcam_sysutils
# DNG
AINR_SHARED_LIBS += libcamalgo.dngop
#Debug exif utils
AINR_SHARED_LIBS += libmtkcam_exif
#
#------------------------------------------------------------------------------
# AINR module static link lib name, usually you don't need to modify this.
#------------------------------------------------------------------------------
AINR_LIB_NAME_NVRAM     := libainrnvram
AINR_LIB_NAME_STRATEGY  := libainrstrategy
AINR_LIB_NAME_FEFM      := libainrfefm
AINR_LIB_NAME_WPE       := libainrwarping
AINR_LIB_NAME_AIBC      := libaibcalgo
#------------------------------------------------------------------------------
# defines
#------------------------------------------------------------------------------
AINR_CFLAGS += -DAINR_CORE_VERSION_MAJOR=$(AINR_CORE_VERSION_MAJOR)
AINR_CFLAGS += -DAINR_CORE_VERSION_MINOR=$(AINR_CORE_VERSION_MINOR)
AINR_CFLAGS += -DAINR_MC_THREAD_NUM=$(AINR_MC_THREAD_NUM)
AINR_CFLAGS += -DAINR_MAX_FRAMES=$(AINR_MAX_FRAMES)
AINR_CFLAGS += -DAINR_BLEND_FRAME=$(AINR_BLEND_FRAME)
AINR_CFLAGS += -DAINR_CAPTURE_FRAME=$(AINR_CAPTURE_FRAME)
AINR_CFLAGS += -DAINR_FULL_SIZE_MC=$(AINR_FULL_SIZE_MC)
AINR_CFLAGS += -DAINR_PLATFORM_$(shell echo $(AINR_PLATFORM)|tr a-z A-Z)
AINR_CFLAGS += -DAINR_GYRO_QUEUE_SIZE=$(AINR_GYRO_QUEUE_SIZE)
AINR_CFLAGS += -DAINR_MF_TAG_VERSION=$(AINR_MF_TAG_VERSION)
AINR_CFLAGS += -DAINR_MF_TAG_SUBVERSION=$(AINR_MF_TAG_SUBVERSION)
AINR_CFLAGS += -DAINR_POST_NR_REFINE_ENABLE=$(AINR_POST_NR_REFINE_ENABLE)
AINR_CFLAGS += -DAINR_ALGO_FEATURE_DECOUPLING=$(AINR_ALGO_FEATURE_DECOUPLING)
AINR_CFLAGS += -DAINR_ALGO_THREADS_PRIORITY=$(AINR_ALGO_THREADS_PRIORITY)
AINR_CFLAGS += -DAINR_CONTENT_AWARE_AIS_ENABLE=$(AINR_CONTENT_AWARE_AIS_ENABLE)


include $(call all-makefiles-under, $(LOCAL_PATH))
endif # AINR_ENABLE

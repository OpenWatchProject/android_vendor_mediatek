## Copyright Statement:
#
# This software/firmware and related documentation ("MediaTek Software") are
# protected under relevant copyright laws. The information contained herein
# is confidential and proprietary to MediaTek Inc. and/or its licensors.
# Without the prior written permission of MediaTek inc. and/or its licensors,
# any reproduction, modification, use or disclosure of MediaTek Software,
# and information contained herein, in whole or in part, shall be strictly prohibited.

# MediaTek Inc. (C) 2016. All rights reserved.
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

# default disable MFLL, we will check if enable later
MFLL_ENABLE := no

# describes platform information
MFLL_PLATFORM := default

#------------------------------------------------------------------------------
# Default MFLL Configurations
#------------------------------------------------------------------------------
# MFLL core version, MFLL core is platform-independent
MFLL_CORE_VERSION_MAJOR                 := 1
MFLL_CORE_VERSION_MINOR                 := 5

# MFLL default version, for build pass usage
MFLL_MODULE_VERSION_BSS                 := default
MFLL_MODULE_VERSION_CAPTURER            := default
MFLL_MODULE_VERSION_EXIFINFO            := default
MFLL_MODULE_VERSION_PERFSERV            := default
MFLL_MODULE_VERSION_IMAGEBUF            := default
MFLL_MODULE_VERSION_MEMC                := default
MFLL_MODULE_VERSION_MFB                 := default
MFLL_MODULE_VERSION_NVRAM               := default
MFLL_MODULE_VERSION_STRATEGY            := default
MFLL_MODULE_VERSION_GYRO                := default

# Describes which MF TAG version to use. 0 is to disable MF tag
# If for a new project early porting, to set this value to 0 to avoid
# compile error(s).
MFLL_MF_TAG_VERSION                     := 0

# Describes which MF TAG sub version.
MFLL_MF_TAG_SUBVERSION                  := 0

# To use parallel computing for MEMC or single thread
# 0: sequential MEMC
# 1: parallel MEMC
MFLL_MEMC_SUPPORT_MODE                  := 1

# MFLL core will create N threads to do ME and MC if MFLL_MEMC_SUPPORT_MODE = 1
MFLL_MEMC_THREADS_NUM                   := 3

# The MC algorithm also supports (if does) multi-threads computing. This define
# is describes how many threads that MC algorithm can create
MFLL_MC_THREAD_NUM                      := 4

# MfllImageBuffer 1.0 has a mechanism to handle image buffer for ZSD + DualPhse.
# This define describes if to enable or disable(set to 0) this feature. If to
# enable the feature, this define also describes the maximum number of image buffer
# that MfllImageBuffer can create.
MFLL_USING_IMAGEBUFFER_QUEUE            := 0

# Define is to 1 to always enable Multi Frame Blending
MFLL_MFB_ALWAYS_ON                      := 0

# Max available capture frame number
MFLL_MAX_FRAMES                         := 10

# Describes blending number
# Note: This is a default value and will be updated by NVRAM
MFLL_BLEND_FRAME                        := 4

# Describes capture frame number
# Note: This is a default value and will be updated by NVRAM
MFLL_CAPTURE_FRAME                      := $(MFLL_BLEND_FRAME)

# Describes MEMC type, if to use full size MC or not
# Note: This is a default value and will be updated by NVRAM
MFLL_FULL_SIZE_MC                       := 0

# Describes the size of GYRO information queue
# Note: Phased-out after MFNR v2.0
MFLL_GYRO_QUEUE_SIZE                    := 30

# Enable/Disable post-NR refine function or not. Post-NR refine is a method to
# rescure the noise level if some frames are dropped by BSS.
MFLL_POST_NR_REFINE_ENABLE              := 1

# Enable/Disable content-award AIS feature, if this option is enable, AIS may
# refer to the information of image to calculate the final result which
# improves stabilization quality.
# Note: The option is available since MfllStrate v1.3,
MFLL_CONTENT_AWARE_AIS_ENABLE           := 1

# Algorithm goes to ip-based, include path and .so maybe different
# Note: This option is for compiler usage, usually vendor never changes this
#       option
MFLL_ALGO_FEATURE_DECOUPLING            := 0

# This value controls the priority of algorithm threads.
# The priority of threads will be 120 + this value.
# The smaller value has higher priority.
MFLL_ALGO_THREADS_PRIORITY              := 0

# NVRAM 2.0 is coupling with isp_runing and aaa
# This value must set to 1 when NVRAM 2.0 is support
# and isp_tuning and aaa is not decoupling yet.
MFLL_NVRAM_3A_TUNING_COUPLING           := 0

#------------------------------------------------------------------------------
# Platform Configurations
#------------------------------------------------------------------------------
# MT6797
# {{{
ifeq ($(TARGET_BOARD_PLATFORM), $(filter $(TARGET_BOARD_PLATFORM), mt6797))
  MFLL_PLATFORM := mt6797
  MFLL_MF_TAG_VERSION := 2

  MFLL_CORE_VERSION_MAJOR       := 1
  MFLL_CORE_VERSION_MINOR       := 5

  MFLL_MODULE_VERSION_BSS       := 1.0
  MFLL_MODULE_VERSION_CAPTURER  := 1.0
  MFLL_MODULE_VERSION_EXIFINFO  := 1.0
  MFLL_MODULE_VERSION_PERFSERV  := default
  MFLL_MODULE_VERSION_IMAGEBUF  := 1.0
  MFLL_MODULE_VERSION_MEMC      := 1.0
  MFLL_MODULE_VERSION_MFB       := 1.0
  MFLL_MODULE_VERSION_NVRAM     := 1.0
  MFLL_MODULE_VERSION_STRATEGY  := 1.0

  MFLL_MC_THREAD_NUM            := 10

  MFLL_USING_IMAGEBUFFER_QUEUE  := 12 # 12 full size YUV

  # workaroud to judge whether libperfservicenative exists nor not
  # remove this condition after MTK_PERFSERVICE_SUPPORT works as expected
  ifeq ($(MTK_BASIC_PACKAGE), yes)
    MFLL_MODULE_VERSION_PERFSERV := default
  endif
endif
# }}}

# MT6757
# {{{
ifeq ($(TARGET_BOARD_PLATFORM), $(filter $(TARGET_BOARD_PLATFORM), mt6757))
  MFLL_PLATFORM := mt6757

# detect SW version
ifeq        ($(strip $(MTK_CAM_SW_VERSION)),ver1)
  MFLL_MF_TAG_VERSION := 3
else ifeq   ($(strip $(MTK_CAM_SW_VERSION)),ver2)
  MFLL_MF_TAG_VERSION := 5
else
  $(error Not support MTK_CAM_SW_VERSION:$(MTK_CAM_SW_VERSION))
endif

  # to use MFNR Core Library v2.0 for MT6757.
  # v1.6 is included in v2.0, hence MT6757 is still using v1.6.
  MFLL_CORE_VERSION_MAJOR       := 2
  MFLL_CORE_VERSION_MINOR       := 0

  MFLL_MODULE_VERSION_BSS       := 1.1
  MFLL_MODULE_VERSION_CAPTURER  := 1.0
  MFLL_MODULE_VERSION_EXIFINFO  := 1.0
  MFLL_MODULE_VERSION_PERFSERV  := default
  MFLL_MODULE_VERSION_IMAGEBUF  := 1.1
  MFLL_MODULE_VERSION_MEMC      := 1.0
  MFLL_MODULE_VERSION_MFB       := 1.0
  MFLL_MODULE_VERSION_NVRAM     := 1.0
  MFLL_MODULE_VERSION_STRATEGY  := 1.1

  MFLL_MC_THREAD_NUM            := 8

  MFLL_USING_IMAGEBUFFER_QUEUE  := 12 # 12 full size YUV

  # workaroud to judge whether libperfservicenative exists nor not
  # remove this condition after MTK_PERFSERVICE_SUPPORT works as expected
  ifeq ($(MTK_BASIC_PACKAGE), yes)
    MFLL_MODULE_VERSION_PERFSERV := default
  endif
endif
# }}}

# MT6799
# {{{
ifeq ($(TARGET_BOARD_PLATFORM), $(filter $(TARGET_BOARD_PLATFORM), mt6799))
  MFLL_PLATFORM := mt6799

# detect SW version
ifeq ($(strip $(MTK_CAM_SW_VERSION)),ver1)
  MFLL_MF_TAG_VERSION := 4

  MFLL_CORE_VERSION_MAJOR       := 2
  MFLL_CORE_VERSION_MINOR       := 0

  MFLL_MODULE_VERSION_BSS       := 1.1
  MFLL_MODULE_VERSION_CAPTURER  := 1.0
  MFLL_MODULE_VERSION_EXIFINFO  := 1.0
  MFLL_MODULE_VERSION_PERFSERV  := default
  MFLL_MODULE_VERSION_IMAGEBUF  := 1.1
  MFLL_MODULE_VERSION_MEMC      := 1.1
  MFLL_MODULE_VERSION_MFB       := 1.0
  MFLL_MODULE_VERSION_NVRAM     := 1.0
  MFLL_MODULE_VERSION_STRATEGY  := 1.1

  MFLL_MC_THREAD_NUM            := 8

  MFLL_USING_IMAGEBUFFER_QUEUE  := 12 # 12 full size YUV

else ifeq ($(strip $(MTK_CAM_SW_VERSION)),ver2)

  MFLL_MF_TAG_VERSION := 6

  MFLL_CORE_VERSION_MAJOR       := 2
  MFLL_CORE_VERSION_MINOR       := 0

  MFLL_MODULE_VERSION_BSS       := 1.1
  MFLL_MODULE_VERSION_CAPTURER  := 1.0
  MFLL_MODULE_VERSION_EXIFINFO  := 1.0
  MFLL_MODULE_VERSION_PERFSERV  := default
  MFLL_MODULE_VERSION_IMAGEBUF  := 1.1
  MFLL_MODULE_VERSION_MEMC      := 1.1
  MFLL_MODULE_VERSION_MFB       := 1.0
  MFLL_MODULE_VERSION_NVRAM     := 1.0
  MFLL_MODULE_VERSION_STRATEGY  := 1.2

  MFLL_MC_THREAD_NUM            := 8

  MFLL_USING_IMAGEBUFFER_QUEUE  := 12 # 12 full size YUV

else
  $(error Not support MTK_CAM_SW_VERSION:$(MTK_CAM_SW_VERSION))
endif

  # workaroud to judge whether libperfservicenative exists nor not
  # remove this condition after MTK_PERFSERVICE_SUPPORT works as expected
  ifeq ($(MTK_BASIC_PACKAGE), yes)
    MFLL_MODULE_VERSION_PERFSERV := default
  endif
endif
# }}}

# MT6763
# {{{
ifeq ($(TARGET_BOARD_PLATFORM), $(filter $(TARGET_BOARD_PLATFORM), mt6763))
  MFLL_PLATFORM                 := mt6763
  MFLL_ALGO_FEATURE_DECOUPLING  := 1

  MFLL_MF_TAG_VERSION           := 7

  MFLL_CORE_VERSION_MAJOR       := 2
  MFLL_CORE_VERSION_MINOR       := 0

  MFLL_MODULE_VERSION_BSS       := 1.1
  MFLL_MODULE_VERSION_CAPTURER  := 1.0
  MFLL_MODULE_VERSION_EXIFINFO  := 1.0
  MFLL_MODULE_VERSION_PERFSERV  := default
  MFLL_MODULE_VERSION_IMAGEBUF  := 1.1
  MFLL_MODULE_VERSION_MEMC      := 1.1
  MFLL_MODULE_VERSION_MFB       := 1.0
  MFLL_MODULE_VERSION_NVRAM     := 1.0
  MFLL_MODULE_VERSION_STRATEGY  := 1.3

  MFLL_MC_THREAD_NUM            := 8

  MFLL_USING_IMAGEBUFFER_QUEUE  := 12 # 12 full size YUV

  # workaroud to judge whether libperfservicenative exists nor not
  # remove this condition after MTK_PERFSERVICE_SUPPORT works as expected
  ifeq ($(MTK_BASIC_PACKAGE), yes)
    MFLL_MODULE_VERSION_PERFSERV := default
  endif
endif
# }}}

# MT6758
# {{{
ifeq ($(TARGET_BOARD_PLATFORM), $(filter $(TARGET_BOARD_PLATFORM), mt6758))
  MFLL_PLATFORM                 := mt6758
  MFLL_ALGO_FEATURE_DECOUPLING  := 1

  MFLL_MF_TAG_VERSION           := 8

  MFLL_CORE_VERSION_MAJOR       := 2
  MFLL_CORE_VERSION_MINOR       := 0

  MFLL_MODULE_VERSION_BSS       := 1.1
  MFLL_MODULE_VERSION_CAPTURER  := 1.0
  MFLL_MODULE_VERSION_EXIFINFO  := 1.0
  MFLL_MODULE_VERSION_PERFSERV  := default
  MFLL_MODULE_VERSION_IMAGEBUF  := 1.1
  MFLL_MODULE_VERSION_MEMC      := 1.1
  MFLL_MODULE_VERSION_MFB       := 1.0
  MFLL_MODULE_VERSION_NVRAM     := 1.0
  MFLL_MODULE_VERSION_STRATEGY  := 1.3

  MFLL_MC_THREAD_NUM            := 8

  MFLL_USING_IMAGEBUFFER_QUEUE  := 12 # 12 full size YUV

  # workaroud to judge whether libperfservicenative exists nor not
  # remove this condition after MTK_PERFSERVICE_SUPPORT works as expected
  ifeq ($(MTK_BASIC_PACKAGE), yes)
    MFLL_MODULE_VERSION_PERFSERV := default
  endif
endif
# }}}

# MT6771
# {{{
ifeq ($(TARGET_BOARD_PLATFORM), $(filter $(TARGET_BOARD_PLATFORM), mt6771))
  MFLL_PLATFORM                 := mt6771
  MFLL_ALGO_FEATURE_DECOUPLING  := 1

  MFLL_MF_TAG_VERSION           := 9
  MFLL_MF_TAG_SUBVERSION        := 4

  MFLL_CORE_VERSION_MAJOR       := 2
  MFLL_CORE_VERSION_MINOR       := 5

  MFLL_MODULE_VERSION_BSS       := 1.2
  MFLL_MODULE_VERSION_CAPTURER  := 1.2
  MFLL_MODULE_VERSION_EXIFINFO  := 1.0
  MFLL_MODULE_VERSION_PERFSERV  := default
  MFLL_MODULE_VERSION_IMAGEBUF  := 1.1
  MFLL_MODULE_VERSION_MEMC      := 1.2
  MFLL_MODULE_VERSION_MFB       := 2.0
  MFLL_MODULE_VERSION_NVRAM     := 2.0
  MFLL_MODULE_VERSION_STRATEGY  := 1.4

  MFLL_MC_THREAD_NUM            := 8

  MFLL_USING_IMAGEBUFFER_QUEUE  := 0

  MFLL_NVRAM_3A_TUNING_COUPLING := 1

  # workaroud to judge whether libperfservicenative exists nor not
  # remove this condition after MTK_PERFSERVICE_SUPPORT works as expected
  ifeq ($(MTK_BASIC_PACKAGE), yes)
    MFLL_MODULE_VERSION_PERFSERV := default
  endif
endif
# }}}

# MT6775
# {{{
ifeq ($(TARGET_BOARD_PLATFORM), $(filter $(TARGET_BOARD_PLATFORM), mt6775))
  MFLL_PLATFORM                 := mt6775
  MFLL_ALGO_FEATURE_DECOUPLING  := 1

  MFLL_MF_TAG_VERSION           := 9
  MFLL_MF_TAG_SUBVERSION        := 2

  MFLL_CORE_VERSION_MAJOR       := 2
  MFLL_CORE_VERSION_MINOR       := 5

  MFLL_MODULE_VERSION_BSS       := 1.2
  MFLL_MODULE_VERSION_CAPTURER  := 1.2
  MFLL_MODULE_VERSION_EXIFINFO  := 1.0
  MFLL_MODULE_VERSION_PERFSERV  := default
  MFLL_MODULE_VERSION_IMAGEBUF  := 1.1
  MFLL_MODULE_VERSION_MEMC      := 1.2
  MFLL_MODULE_VERSION_MFB       := 2.0
  MFLL_MODULE_VERSION_NVRAM     := 2.0
  MFLL_MODULE_VERSION_STRATEGY  := 1.4

  MFLL_MC_THREAD_NUM            := 8

  MFLL_USING_IMAGEBUFFER_QUEUE  := 0

  MFLL_NVRAM_3A_TUNING_COUPLING := 1

  # workaroud to judge whether libperfservicenative exists nor not
  # remove this condition after MTK_PERFSERVICE_SUPPORT works as expected
  ifeq ($(MTK_BASIC_PACKAGE), yes)
    MFLL_MODULE_VERSION_PERFSERV := default
  endif
endif
# }}}

# MT6765
# {{{
ifeq ($(TARGET_BOARD_PLATFORM), $(filter $(TARGET_BOARD_PLATFORM), mt6765))
  MFLL_PLATFORM                 := mt6765
  MFLL_ALGO_FEATURE_DECOUPLING  := 1

  MFLL_MF_TAG_VERSION           := 10

  MFLL_CORE_VERSION_MAJOR       := 2
  MFLL_CORE_VERSION_MINOR       := 0

  MFLL_MODULE_VERSION_BSS       := 1.1
  MFLL_MODULE_VERSION_CAPTURER  := 1.0
  MFLL_MODULE_VERSION_EXIFINFO  := 1.0
  MFLL_MODULE_VERSION_PERFSERV  := default
  MFLL_MODULE_VERSION_IMAGEBUF  := 1.1
  MFLL_MODULE_VERSION_MEMC      := 1.1
  MFLL_MODULE_VERSION_MFB       := 1.0
  MFLL_MODULE_VERSION_NVRAM     := 1.0
  MFLL_MODULE_VERSION_STRATEGY  := 1.3

  MFLL_MC_THREAD_NUM            := 8

  MFLL_USING_IMAGEBUFFER_QUEUE  := 12 # 12 full size YUV

  # workaroud to judge whether libperfservicenative exists nor not
  # remove this condition after MTK_PERFSERVICE_SUPPORT works as expected
  ifeq ($(MTK_BASIC_PACKAGE), yes)
    MFLL_MODULE_VERSION_PERFSERV := default
  endif
endif
# }}}


# re-check module version
# for LDVT, do just for compile pass, hence we set MFLL_MODULE_VERSION to default
ifeq ($(BUILD_MTK_LDVT),yes)
  MFLL_MODULE_VERSION_BSS           := default
  MFLL_MODULE_VERSION_CAPTURER      := default
  MFLL_MODULE_VERSION_EXIFINFO      := default
  MFLL_MODULE_VERSION_PERFSERV      := default
  MFLL_MODULE_VERSION_IMAGEBUF      := default
  MFLL_MODULE_VERSION_MEMC          := default
  MFLL_MODULE_VERSION_MFB           := default
  MFLL_MODULE_VERSION_NVRAM         := default
  MFLL_MODULE_VERSION_STRATEGY      := default
  MFLL_MODULE_VERSION_GYRO          := default
endif

# check if enable MFLL by ProjectConfig.mk
ifneq ($(strip $(MTK_CAM_MFB_SUPPORT)),0)
  MFLL_ENABLE := yes
endif


#------------------------------------------------------------------------------
# MFLL config start from here.
#------------------------------------------------------------------------------
ifeq ($(strip $(MFLL_ENABLE)), yes)
# Define MFLL core (algorithm) version and dependent middleware version
MFLL_CORE_VERSION := $(MFLL_CORE_VERSION_MAJOR).$(MFLL_CORE_VERSION_MINOR)
$(warning MFLL_CORE_VERSION: $(MFLL_CORE_VERSION))

# MFNR common interface location
MFLL_INCLUDE_PATH := $(MTK_PATH_SOURCE)/hardware/mtkcam/include # MTKCAM include path
MFLL_INCLUDE_PATH += $(MTK_PATH_SOURCE)/hardware/mtkcam/include/mtkcam/feature/mfnr
MFLL_INCLUDE_PATH += $(TOP)/$(MTK_PATH_SOURCE)/external/aee/binary/inc
MFLL_INCLUDE_PATH += $(LOCAL_PATH)/core/inc
MFLL_INCLUDE_PATH += $(LOCAL_PATH)/core/$(MFLL_CORE_VERSION)
MFLL_INCLUDE_PATH += $(LOCAL_PATH)/modules/bss/$(MFLL_MODULE_VERSION_BSS)
MFLL_INCLUDE_PATH += $(LOCAL_PATH)/modules/capturer/$(MFLL_MODULE_VERSION_CAPTURER)
MFLL_INCLUDE_PATH += $(LOCAL_PATH)/modules/exifinfo/$(MFLL_MODULE_VERSION_EXIFINFO)
MFLL_INCLUDE_PATH += $(LOCAL_PATH)/modules/imagebuffer/$(MFLL_MODULE_VERSION_IMAGEBUF)
MFLL_INCLUDE_PATH += $(LOCAL_PATH)/modules/memc/$(MFLL_MODULE_VERSION_MEMC)
MFLL_INCLUDE_PATH += $(LOCAL_PATH)/modules/mfb/$(MFLL_MODULE_VERSION_MFB)
MFLL_INCLUDE_PATH += $(LOCAL_PATH)/modules/nvram/$(MFLL_MODULE_VERSION_NVRAM)
MFLL_INCLUDE_PATH += $(LOCAL_PATH)/modules/perfserv/$(MFLL_MODULE_VERSION_PERFSERV)
MFLL_INCLUDE_PATH += $(LOCAL_PATH)/modules/strategy/$(MFLL_MODULE_VERSION_STRATEGY)
MFLL_INCLUDE_PATH += $(LOCAL_PATH)/modules/gyro/$(MFLL_MODULE_VERSION_GYRO)

MFLL_INCLUDE_PATH += $(TOPDIR)vendor/mediatek/proprietary/external/libudf/libladder

# MFNR libcamera_feature decoupling
ifeq ($(strip $(MFLL_ALGO_FEATURE_DECOUPLING)),1)
  MFLL_INCLUDE_PATH_ALGO            := $(MTK_PATH_SOURCE)/hardware/libcamera_feature/libmfnr_lib/$(MFLL_PLATFORM)/include
  MFLL_LIB_NAME_ALGO                := libcamalgo.mfnr
else
  MFLL_INCLUDE_PATH_ALGO            := $(MTK_PATH_SOURCE)/hardware/mtkcam/include/algorithm/$(MFLL_PLATFORM)/libais
  MFLL_INCLUDE_PATH_ALGO            += $(MTK_PATH_SOURCE)/hardware/mtkcam/include/algorithm/$(MFLL_PLATFORM)/libbss
  MFLL_INCLUDE_PATH_ALGO            += $(MTK_PATH_SOURCE)/hardware/mtkcam/include/algorithm/$(MFLL_PLATFORM)/libmfbll
  MFLL_LIB_NAME_ALGO                := libcamalgo
endif

# NVRAM coupling with aaa/ isp_tuning
ifeq ($(strip $(MFLL_NVRAM_3A_TUNING_COUPLING)),1)
  MFLL_INCLUDE_PATH += $(MTK_PATH_CUSTOM_PLATFORM)/hal/inc/aaa
  MFLL_INCLUDE_PATH += $(MTK_PATH_CUSTOM_PLATFORM)/hal/inc/isp_tuning
endif

#
# MFLL core library name
#
MFLL_CORE_LIB_NAME := libmfllcore


#------------------------------------------------------------------------------
# Define share library.
#------------------------------------------------------------------------------
MFLL_SHARED_LIBS := libutils
MFLL_SHARED_LIBS += libcutils
#MFLL_SHARED_LIBS += libstdc++ # M toolchain
# MTKCAM standard
MFLL_SHARED_LIBS += libmtkcam_stdutils # MTK cam standard library
MFLL_SHARED_LIBS += liblog # log library since Android O

# bss
ifeq ($(strip $(MFLL_MODULE_VERSION_BSS)),1.0)
  MFLL_SHARED_LIBS += $(MFLL_LIB_NAME_ALGO)
endif
ifeq ($(strip $(MFLL_MODULE_VERSION_BSS)),1.1)
  MFLL_SHARED_LIBS += $(MFLL_LIB_NAME_ALGO)
endif
ifeq ($(strip $(MFLL_MODULE_VERSION_BSS)),1.2)
  MFLL_SHARED_LIBS += $(MFLL_LIB_NAME_ALGO)
  MFLL_SHARED_LIBS += libmtkcam_hwutils
  MFLL_INCLUDE_PATH += $(MTK_PATH_SOURCE)/hardware/libcamera_feature/libfdft_lib/include
endif

# capturer
ifeq ($(strip $(MFLL_MODULE_VERSION_CAPTURER)),1.0)
  # no need any
endif
ifeq ($(strip $(MFLL_MODULE_VERSION_CAPTURER)),1.2)
  # no need any
endif

# exifInfo
ifeq ($(strip $(MFLL_MODULE_VERSION_EXIFINFO)),1.0)
  MFLL_SHARED_LIBS += libcameracustom
endif

# imagebuffer
ifeq ($(strip $(MFLL_MODULE_VERSION_IMAGEBUF)),1.0)
  MFLL_SHARED_LIBS += libmtkcam_imgbuf
endif
ifeq ($(strip $(MFLL_MODULE_VERSION_IMAGEBUF)),1.1)
  MFLL_SHARED_LIBS += libmtkcam_imgbuf
endif

# memc
ifeq ($(strip $(MFLL_MODULE_VERSION_MEMC)),1.0)
  MFLL_SHARED_LIBS += $(MFLL_LIB_NAME_ALGO)
endif
ifeq ($(strip $(MFLL_MODULE_VERSION_MEMC)),1.2)
  MFLL_SHARED_LIBS += $(MFLL_LIB_NAME_ALGO)
endif

# mfb
ifeq ($(strip $(MFLL_MODULE_VERSION_MFB)),1.0)
  MFLL_SHARED_LIBS += libcam.iopipe
  MFLL_SHARED_LIBS += libmtkcam_metadata
  MFLL_SHARED_LIBS += libmtkcam_imgbuf
  MFLL_SHARED_LIBS += libmtkcam_modulehelper
endif
ifeq ($(strip $(MFLL_MODULE_VERSION_MFB)),2.0)
  MFLL_SHARED_LIBS += libcam.iopipe
  MFLL_SHARED_LIBS += libmtkcam_metadata
  MFLL_SHARED_LIBS += libmtkcam_imgbuf
  MFLL_SHARED_LIBS += libmtkcam_modulehelper
  MFLL_SHARED_LIBS += libmtkcam_mfb
  MFLL_SHARED_LIBS += libdpframework
endif

# nvram
ifeq ($(strip $(MFLL_MODULE_VERSION_NVRAM)),1.0)
  MFLL_SHARED_LIBS += libcameracustom
  MFLL_SHARED_LIBS += libmtkcam_modulehelper
endif
ifeq ($(strip $(MFLL_MODULE_VERSION_NVRAM)),2.0)
  MFLL_SHARED_LIBS += libcameracustom
  MFLL_SHARED_LIBS += libmtkcam_modulehelper
  MFLL_SHARED_LIBS += libmtkcam_mapping_mgr
endif


# perfserv
ifneq ($(strip $(MFLL_MODULE_VERSION_PERFSERV)),default)
  MFLL_SHARED_LIBS += libperfservicenative
endif

# strategy
ifeq ($(strip $(MFLL_MODULE_VERSION_STRATEGY)),1.0)
  MFLL_SHARED_LIBS += $(MFLL_LIB_NAME_ALGO)
endif
ifeq ($(strip $(MFLL_MODULE_VERSION_STRATEGY)),1.1)
  MFLL_SHARED_LIBS += $(MFLL_LIB_NAME_ALGO)
  MFLL_SHARED_LIBS += libmtkcam_sysutils
  MFLL_SHARED_LIBS += libmtkcam_hwutils
endif
ifeq ($(strip $(MFLL_MODULE_VERSION_STRATEGY)),1.2)
  MFLL_SHARED_LIBS += $(MFLL_LIB_NAME_ALGO)
  MFLL_SHARED_LIBS += libmtkcam_sysutils
  MFLL_SHARED_LIBS += libmtkcam_hwutils
endif
ifeq ($(strip $(MFLL_MODULE_VERSION_STRATEGY)),1.3)
  MFLL_SHARED_LIBS += $(MFLL_LIB_NAME_ALGO)
  MFLL_SHARED_LIBS += libmtkcam_sysutils
  MFLL_SHARED_LIBS += libmtkcam_hwutils
endif

# gyro
ifeq ($(strip $(MFLL_MODULE_VERSION_GYRO)),1.0)
  MFLL_SHARED_LIBS += libmtkcam_sysutils
endif

# aee
ifeq ($(HAVE_AEE_FEATURE),yes)
  MFLL_SHARED_LIBS += libaedv
endif

MFLL_SHARED_LIBS += libladder
MFLL_SHARED_LIBS += libcamalgo.mfnr
#------------------------------------------------------------------------------
# MFLL module static link lib name, usually you don't need to modify this.
#------------------------------------------------------------------------------
MFLL_LIB_NAME_BSS       := libmfllbss
MFLL_LIB_NAME_CAPTURER  := libmfllcaptuer
MFLL_LIB_NAME_EXIFINFO  := libmfllexifinfo
MFLL_LIB_NAME_PERFSERV  := libmfllperfserv
MFLL_LIB_NAME_IMAGEBUF  := libmfllimagebuf
MFLL_LIB_NAME_MEMC      := libmfllmemc
MFLL_LIB_NAME_MFB       := libmfllmfb
MFLL_LIB_NAME_NVRAM     := libmfllnvram
MFLL_LIB_NAME_STRATEGY  := libmfllstrategy
MFLL_LIB_NAME_GYRO      := libmfllgyro


#------------------------------------------------------------------------------
# defines
#------------------------------------------------------------------------------
MFLL_CFLAGS += -DMFLL_CORE_VERSION_MAJOR=$(MFLL_CORE_VERSION_MAJOR)
MFLL_CFLAGS += -DMFLL_CORE_VERSION_MINOR=$(MFLL_CORE_VERSION_MINOR)
MFLL_CFLAGS += -DMFLL_MEMC_SUPPORT_MODE=$(MFLL_MEMC_SUPPORT_MODE)
MFLL_CFLAGS += -DMFLL_MEMC_THREADS_NUM=$(MFLL_MEMC_THREADS_NUM)
MFLL_CFLAGS += -DMFLL_MC_THREAD_NUM=$(MFLL_MC_THREAD_NUM)
MFLL_CFLAGS += -DMFLL_USING_IMAGEBUFFER_QUEUE=$(MFLL_USING_IMAGEBUFFER_QUEUE)
MFLL_CFLAGS += -DMFLL_MFB_ALWAYS_ON=$(MFLL_MFB_ALWAYS_ON)
MFLL_CFLAGS += -DMFLL_MAX_FRAMES=$(MFLL_MAX_FRAMES)
MFLL_CFLAGS += -DMFLL_BLEND_FRAME=$(MFLL_BLEND_FRAME)
MFLL_CFLAGS += -DMFLL_CAPTURE_FRAME=$(MFLL_CAPTURE_FRAME)
MFLL_CFLAGS += -DMFLL_FULL_SIZE_MC=$(MFLL_FULL_SIZE_MC)
MFLL_CFLAGS += -DMFLL_PLATFORM_$(shell echo $(MFLL_PLATFORM)|tr a-z A-Z)
MFLL_CFLAGS += -DMFLL_GYRO_QUEUE_SIZE=$(MFLL_GYRO_QUEUE_SIZE)
MFLL_CFLAGS += -DMFLL_MF_TAG_VERSION=$(MFLL_MF_TAG_VERSION)
MFLL_CFLAGS += -DMFLL_MF_TAG_SUBVERSION=$(MFLL_MF_TAG_SUBVERSION)
MFLL_CFLAGS += -DMFLL_POST_NR_REFINE_ENABLE=$(MFLL_POST_NR_REFINE_ENABLE)
MFLL_CFLAGS += -DMFLL_ALGO_FEATURE_DECOUPLING=$(MFLL_ALGO_FEATURE_DECOUPLING)
MFLL_CFLAGS += -DMFLL_ALGO_THREADS_PRIORITY=$(MFLL_ALGO_THREADS_PRIORITY)
MFLL_CFLAGS += -DMFLL_CONTENT_AWARE_AIS_ENABLE=$(MFLL_CONTENT_AWARE_AIS_ENABLE)


include $(call all-makefiles-under, $(LOCAL_PATH))
endif # MFLL_ENABLE

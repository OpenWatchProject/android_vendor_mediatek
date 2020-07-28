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
-include $(TOP)/$(MTK_PATH_CUSTOM)/hal/mtkcam/mtkcam.mk
PLATFORM := $(shell echo $(MTK_PLATFORM) | tr A-Z a-z)

-include $(TOP)/$(MTK_PATH_SOURCE)/hardware/mtkcam/mtkcam.mk
-include $(TOP)/$(MTK_PATH_SOURCE)/hardware/mtkcam/drv/driver.mk

INDEP_STEREO_PROVIDER := true
STEREO_HAL_VER := common

ifeq ($(PLATFORM), $(filter $(PLATFORM),mt6797))
else ifeq ($(PLATFORM), $(filter $(PLATFORM),kiboplus mt6757))
else ifeq ($(PLATFORM), $(filter $(PLATFORM),mt6758 mt6763))
else ifeq ($(PLATFORM), $(filter $(PLATFORM),mt6765))
else ifeq ($(PLATFORM), $(filter $(PLATFORM),mt6799))
LOCAL_CFLAGS+=-DHAS_WPE=1
HAS_WPE := 1
else ifeq ($(PLATFORM), $(filter $(PLATFORM),mt6771 mt6775 mt6785))
LOCAL_CFLAGS+=-DHAS_WPE=1
LOCAL_CFLAGS+=-DHAS_HW_OCC=1
HAS_WPE := 1
HAS_HW_OCC := 1
else ifeq ($(PLATFORM), $(filter $(PLATFORM),mt6779))
LOCAL_CFLAGS+=-DHAS_P1YUV=1
LOCAL_CFLAGS+=-DHAS_WPE=1
LOCAL_CFLAGS+=-DHAS_HW_DPE=2
LOCAL_CFLAGS+=-DHAS_AIDEPTH=1
LOCAL_CFLAGS+=-DHAS_ISP_FE_TUNING=1
LOCAL_CFLAGS+=-DHAS_P1_RESIZE_QUALITY=1
HAS_WPE := 1
HAS_HW_DPE := 2
HAS_AIDEPTH := 1
HAS_P1YUV := 1
else

endif

HAL_MET_PROFILE := false

VSDOF_COMMON_INC := $(MTKCAM_C_INCLUDES)
VSDOF_COMMON_INC += $(MTKCAM_DRV_INCLUDE)
VSDOF_COMMON_INC += $(MTKCAM_DRV_INCLUDE)/drv
VSDOF_COMMON_INC += $(TOP)/$(MTK_PATH_SOURCE)/hardware/mtkcam/drv/include
VSDOF_COMMON_INC += $(TOP)/$(MTK_PATH_SOURCE)/hardware/mtkcam
VSDOF_COMMON_INC += $(TOP)/$(MTK_PATH_SOURCE)/hardware/mtkcam3/include $(MTK_PATH_SOURCE)/hardware/mtkcam/include
VSDOF_COMMON_INC += $(TOP)/$(MTK_PATH_SOURCE)/hardware/gralloc_extra/include
VSDOF_COMMON_INC += $(TOP)/$(MTK_PATH_SOURCE)/hardware/mtkcam3/feature/include/common
VSDOF_COMMON_INC += $(TOP)/$(MTK_PATH_CUSTOM_PLATFORM)/hal/inc
VSDOF_COMMON_INC += $(TOP)/$(MTK_PATH_SOURCE)/hardware/mtkcam/include/$(PLATFORM)
VSDOF_COMMON_INC += $(TOP)/system/core/libsync/include/sync
VSDOF_COMMON_INC += $(TOP)/system/core/libsync
VSDOF_COMMON_INC += $(TOP)/vendor/mediatek/proprietary/hardware/libcamera_feature/libfdft_lib/include/
# For AHardwareBuffer
VSDOF_COMMON_INC += $(TOP)/$(MTK_PATH_SOURCE)/hardware/gralloc_extra/include
VSDOF_COMMON_INC += $(TOP)/frameworks/native/libs/nativewindow/include
#For libladder(dump call stack)
VSDOF_COMMON_INC += $(TOPDIR)vendor/mediatek/proprietary/external/libudf/libladder

#For DPE
VSDOF_COMMON_INC += $(TOP)/$(MTK_PATH_SOURCE)/hardware/mtkcam/drv/include/dpe/$(PLATFORM)

ifeq ($(PLATFORM), $(filter $(PLATFORM), mt6757 kiboplus))
#3A has IP base change for mt6757p
AAA_VERSION := ver2
endif

ifeq ($(MTKCAM_IP_BASE),0)
VSDOF_COMMON_INC += $(MTK_MTKCAM_PLATFORM)/include
VSDOF_COMMON_INC += $(MTK_MTKCAM_PLATFORM)/include/mtkcam
VSDOF_COMMON_INC += $(MTKCAM_C_INCLUDES)/..
VSDOF_COMMON_INC += $(MTK_PATH_SOURCE)/hardware/mtkcam/drv/include/$(PLATFORM)/drv
VSDOF_COMMON_INC += $(MTK_PATH_SOURCE)/hardware/mtkcam/include/algorithm/$(PLATFORM)
VSDOF_COMMON_INC += $(MTK_PATH_SOURCE)/hardware/mtkcam/include/algorithm/$(PLATFORM)/libutility
else
VSDOF_COMMON_INC += $(TOP)/$(MTK_PATH_SOURCE)/hardware/mtkcam3/feature/include/common/vsdof/hal/$(STEREO_HAL_VER)
VSDOF_COMMON_INC += $(TOP)/$(MTK_PATH_SOURCE)/hardware/mtkcam3/feature/include/common/vsdof/providers
VSDOF_COMMON_INC += $(TOP)/$(MTK_PATH_SOURCE)/hardware/mtkcam3/feature/include/common/vsdof/providers/tuning-provider/
ifeq ($(wildcard $(TOP)/$(MTK_PATH_SOURCE)/hardware/mtkcam3/feature/include/common/vsdof/providers/tuning-provider/$(PLATFORM)),)
	VSDOF_COMMON_INC += $(TOP)/$(MTK_PATH_SOURCE)/hardware/mtkcam3/feature/include/common/vsdof/providers/tuning-provider/default
else
	VSDOF_COMMON_INC += $(TOP)/$(MTK_PATH_SOURCE)/hardware/mtkcam3/feature/include/common/vsdof/providers/tuning-provider/$(PLATFORM)
endif
VSDOF_COMMON_INC += $(TOP)/$(MTK_PATH_SOURCE)/hardware/mtkcam3/feature/include/common/vsdof/providers/fefm-setting-provider
endif

LOCAL_CFLAGS += $(MTKCAM_CFLAGS)

LOCAL_HEADER_LIBRARIES := libutils_headers liblog_headers libhardware_headers libmtkcam_headers

########## Dual cam FO check ##########
ifeq ($(MTK_CAM_STEREO_CAMERA_SUPPORT), yes)
LOCAL_CFLAGS += -DSTEREO_CAMERA_SUPPORTED=1
endif

ifeq ($(MTK_CAM_STEREO_DENOISE_SUPPORT), yes)
LOCAL_CFLAGS += -DSTEREO_DENOISE_SUPPORTED=1
endif

ifeq ($(MTK_CAM_VSDOF_SUPPORT), yes)
LOCAL_CFLAGS += -DVSDOF_SUPPORTED=1
endif

ifeq ($(MTK_CAM_DUAL_ZOOM_SUPPORT), yes)
LOCAL_CFLAGS += -DDUAL_ZOOM_SUPPORTED=1
endif

ifeq ($(MTK_CAM_IMAGE_REFOCUS_SUPPORT), yes)
LOCAL_CFLAGS += -DIMAGE_REFOCUS_SUPPORTED=1
endif

ifeq ($(MTK_CAM_DEPTH_AF_SUPPORT), yes)
LOCAL_CFLAGS += -DDEPTH_AF_SUPPORTED=1
endif

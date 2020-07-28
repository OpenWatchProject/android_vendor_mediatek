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
-include $(TOP)/$(MTK_PATH_SOURCE)/hardware/mtkcam/aaa/aaa.mk
-include $(TOP)/$(MTK_PATH_SOURCE)/hardware/mtkcam/aaa/source/$(MTKCAM_LSC_PLATFORM)/lsc_mgr/lsc_tbl.mk
-include $(TOP)/$(MTK_PATH_SOURCE)/hardware/mtkcam/aaa/source/$(MTKCAM_NVRAM_PLATFORM)/nvram/nvram.mk

################################################################################
ifneq (,$(filter $(strip $(TARGET_BOARD_PLATFORM)), mt6797 mt6757 mt6799 mt6763 mt6771 mt6775 mt6765 mt6739 mt6761 mt6779 mt6768 mt6785))

#-----------------------------------------------------------
# Hal3A
#-----------------------------------------------------------
PLATFORMS_USING_HAL3A_SIMULATOR := # mt6797
ifeq (,$(filter $(strip $(TARGET_BOARD_PLATFORM)), $(PLATFORMS_USING_HAL3A_SIMULATOR)))
MY_LOCAL_SHARED_LIBRARIES += libcam.hal3a.v3
MY_LOCAL_SRC_FILES += register_Hal3A.cpp
MY_LOCAL_C_INCLUDES +=
MY_LOCAL_CFLAGS +=
else
MY_LOCAL_SHARED_LIBRARIES += libcam.hal3a.v3
MY_LOCAL_SRC_FILES += register_Hal3ASimulator.cpp
MY_LOCAL_C_INCLUDES +=
MY_LOCAL_CFLAGS +=
endif

#-----------------------------------------------------------
# HalIsp
#-----------------------------------------------------------
MY_LOCAL_SHARED_LIBRARIES += libcam.halisp
MY_LOCAL_SRC_FILES += register_HalISP.cpp
MY_LOCAL_C_INCLUDES +=
MY_LOCAL_CFLAGS +=

#-----------------------------------------------------------
# HalFlash
#-----------------------------------------------------------
MY_LOCAL_SHARED_LIBRARIES += libcam.hal3a.v3
MY_LOCAL_SRC_FILES += register_HalFlash.cpp
MY_LOCAL_C_INCLUDES +=
MY_LOCAL_CFLAGS +=

#-----------------------------------------------------------
# IspMgrIf
#-----------------------------------------------------------
MY_LOCAL_SHARED_LIBRARIES += libcam.hal3a.v3
MY_LOCAL_SRC_FILES += register_IspMgrIf.cpp
MY_LOCAL_C_INCLUDES += $(MTK_PATH_CUSTOM_PLATFORM)/hal/inc/isp_tuning
MY_LOCAL_CFLAGS +=

#-----------------------------------------------------------
# SwNR
#-----------------------------------------------------------
MY_LOCAL_SHARED_LIBRARIES += libcam.hal3a.v3
MY_LOCAL_SRC_FILES += register_SwNR.cpp
MY_LOCAL_C_INCLUDES += $(MTK_PATH_CUSTOM_PLATFORM)/hal/inc
MY_LOCAL_CFLAGS +=

#-----------------------------------------------------------
# DngInfo
#-----------------------------------------------------------
MY_LOCAL_SHARED_LIBRARIES += libcam.hal3a.v3.dng
MY_LOCAL_SRC_FILES += register_DngInfo.cpp
MY_LOCAL_C_INCLUDES +=
MY_LOCAL_CFLAGS +=

#-----------------------------------------------------------
# NvBufUtil
#-----------------------------------------------------------
MY_LOCAL_SHARED_LIBRARIES += $(LIBCAM_HAL3A_V3_NVRAM)
#MY_LOCAL_SHARED_LIBRARIES += libcam.hal3a.v3.nvram
MY_LOCAL_SRC_FILES += register_NvBufUtil.cpp
MY_LOCAL_C_INCLUDES += $(MTK_PATH_CUSTOM_PLATFORM)/hal/inc
MY_LOCAL_CFLAGS +=

#-----------------------------------------------------------
# LscTable
#-----------------------------------------------------------
MY_LOCAL_SHARED_LIBRARIES += $(LIBCAM_HAL3A_V3_LSCTBL)
#MY_LOCAL_SHARED_LIBRARIES += libcam.hal3a.v3.lsctbl
MY_LOCAL_SRC_FILES += register_LscTable.cpp
MY_LOCAL_C_INCLUDES +=
MY_LOCAL_CFLAGS +=

#-----------------------------------------------------------
# Lcs Hal
#-----------------------------------------------------------
ifeq (,$(filter $(strip $(TARGET_BOARD_PLATFORM)), mt6739 mt6761 mt6779))
MY_LOCAL_SHARED_LIBRARIES += libcam.lcs
MY_LOCAL_SRC_FILES += register_lcs_hal.cpp
MY_LOCAL_C_INCLUDES +=
MY_LOCAL_CFLAGS +=
endif

#-----------------------------------------------------------
#
#-----------------------------------------------------------

endif ### (,$(filter $(strip $(TARGET_BOARD_PLATFORM)), ...))

################################################################################
#
################################################################################

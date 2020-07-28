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

#Split build wrap
ifdef MTK_TARGET_PROJECT

-include $(TOP)/$(MTK_PATH_SOURCE)/hardware/mtkcam/main/acdk/acdk.mk
-include $(TOP)/$(MTK_PATH_SOURCE)/hardware/mtkcam/mtkcam.mk
$(info "TARGET_BOARD_PLATFORM=$(TARGET_BOARD_PLATFORM)")

################################################################################
# Build camera modules in normal case
################################################################################
$(info "MTKCAM_IP_BASE=$(MTKCAM_IP_BASE)")
$(info "IS_SMT_PURE=$(IS_SMT_PURE)")
$(info "IS_LEGACY=$(IS_LEGACY)")
$(info "CAMERA_HAL_VERSION=$(CAMERA_HAL_VERSION)")




ifeq ($(MTKCAM_IP_BASE),1)

# IS_SMT_PURE defined in: acdk/acdk.mk
ifeq ($(IS_SMT_PURE), 1)
CAMERA_BUILD_FOLDER := $(SMT_PURE_BUILD_FOLDER)
$(info "build mtkcam for smt pure $(CAMERA_BUILD_FOLDER)")

else

ifeq ($(CAMERA_HAL_VERSION), 3)
# feature middleware/v3 pipeline is moved to mtkcam3
CAMERA_BUILD_FOLDER := utils aaa drv main
$(info "build as HAL3 mtkcam ip base (for version ISP3.x/ISP4.0 and after ISP4.0)")
else
CAMERA_BUILD_FOLDER := utils feature middleware pipeline aaa drv main
$(info "build as HAL1 mtkcam ip base (for version ISP3.x/ISP4.0 and after ISP4.0)")
endif

endif

else
CAMERA_BUILD_FOLDER := utils feature legacy
$(info "build as mtkcam legacy (for version before ISP4.0)")

# Specify halsensor location. Only for legacy HAL1
ifneq (,$(filter $(strip $(TARGET_BOARD_PLATFORM)), mt6739 mt6761))
CAMERA_BUILD_FOLDER += drv/src/sensor
$(info "CAMERA_BUILD_FOLDER=$(CAMERA_BUILD_FOLDER)")
endif

endif

ifeq (,$(filter $(strip $(TARGET_BOARD_PLATFORM)), ))
ifeq (,$(filter $(strip $(PRODUCT_MODEL)), ))
include $(call all-named-subdir-makefiles,$(CAMERA_BUILD_FOLDER))
endif #enable camera for all projects
endif #enable camera for all platforms

endif #MTK_TARGET_PROJECT

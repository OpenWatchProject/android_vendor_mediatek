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
$(info mtkcam3 HAL service: HAL Version=$(CAMERA_HAL_VERSION))


LOCAL_PATH := $(call my-dir)

################################################################################
#
################################################################################
include $(CLEAR_VARS)

#-----------------------------------------------------------
-include $(TOP)/$(MTK_PATH_SOURCE)/hardware/mtkcam/mtkcam.mk

#-----------------------------------------------------------
ifeq ($(MTK_CAM_LAZY_HAL), yes)
LOCAL_SRC_FILES += serviceLazy.cpp
else
LOCAL_SRC_FILES += service.cpp
endif

#-----------------------------------------------------------
LOCAL_C_INCLUDES += $(MTKCAM_C_INCLUDES)
LOCAL_C_INCLUDES += $(MTK_PATH_SOURCE)/hardware/mtkcam/include
LOCAL_C_INCLUDES += $(MTK_PATH_SOURCE)/hardware/mtkcam3/main/hal
#

#-----------------------------------------------------------
LOCAL_CFLAGS += $(MTKCAM_CFLAGS)
#
#-----------------------------------------------------------
LOCAL_SHARED_LIBRARIES += liblog
LOCAL_SHARED_LIBRARIES += libutils
LOCAL_SHARED_LIBRARIES += libcutils
#
LOCAL_SHARED_LIBRARIES += libbinder # to communicate with other vendor components over /dev/vndbinder.
LOCAL_SHARED_LIBRARIES += libhwbinder
LOCAL_SHARED_LIBRARIES += libhidlbase
LOCAL_SHARED_LIBRARIES += libhidltransport
LOCAL_SHARED_LIBRARIES += libmtkcam_stdutils
LOCAL_SHARED_LIBRARIES += libmtkcam_ulog
#
# HIDL HAL interface
LOCAL_SHARED_LIBRARIES += android.hardware.camera.provider@2.4

ifeq ($(MTKCAM_ADV_CAM_SUPPORT), 1)
LOCAL_SHARED_LIBRARIES += vendor.mediatek.hardware.camera.advcam@1.0
endif

ifeq ($(MTKCAM_LOMO_SUPPORT), 1)
#lomoEffect will be removed after android P only support HAL3
LOCAL_SHARED_LIBRARIES += vendor.mediatek.hardware.camera.lomoeffect@1.0
endif

ifneq (,$(filter $(strip $(TARGET_BOARD_PLATFORM)), mt6739 mt6757 mt6761 mt6763 mt6765 mt6768 mt6771 mt6779 mt6785 mt6885 mt8168))
$(info $(TARGET_BOARD_PLATFORM) don't support ccap hidl)
else
LOCAL_SHARED_LIBRARIES += vendor.mediatek.hardware.camera.ccap@1.0
LOCAL_CFLAGS += -DMTK_CCAP_HIDL_SUPPORT=1
endif

LOCAL_SHARED_LIBRARIES += vendor.mediatek.hardware.camera.frhandler@1.0
LOCAL_SHARED_LIBRARIES += vendor.mediatek.hardware.camera.postproc@1.0

ifneq (,$(filter $(strip $(TARGET_BOARD_PLATFORM)), mt6779))
LOCAL_SHARED_LIBRARIES += vendor.mediatek.hardware.camera.isphal@1.0
LOCAL_CFLAGS += -DMTK_CAM_ISP_HIDL_SUPPORT=1
endif

ifeq ($(MTK_CAM_BGSERVICE_SUPPORT), yes)
LOCAL_CFLAGS += -DMTKCAM_BGSERVICE_SUPPORT
LOCAL_SHARED_LIBRARIES += vendor.mediatek.hardware.camera.bgservice@1.0
LOCAL_SHARED_LIBRARIES += vendor.mediatek.hardware.camera.bgservice@1.1
#LOCAL_SHARED_LIBRARIES += vendor.mediatek.hardware.camera.bgservice@1.0-impl
endif

LOCAL_SHARED_LIBRARIES += libhardware
LOCAL_SHARED_LIBRARIES += libmtkcam_grallocutils
LOCAL_SHARED_LIBRARIES += vendor.mediatek.hardware.camera.atms@1.0

#-----------------------------------------------------------
ifeq ($(MTK_CAM_LAZY_HAL), yes)
LOCAL_INIT_RC := camerahalserverLazy.rc
else
LOCAL_INIT_RC := camerahalserver.rc
endif

LOCAL_MODULE := camerahalserver
LOCAL_MODULE_RELATIVE_PATH := hw
LOCAL_MODULE_OWNER := mtk
LOCAL_PROPRIETARY_MODULE := true
LOCAL_MULTILIB := first

#-----------------------------------------------------------
include $(MTK_EXECUTABLE)


################################################################################
#
################################################################################
include $(call all-makefiles-under,$(LOCAL_PATH))

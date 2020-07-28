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
-include $(TOP)/$(MTK_PATH_SOURCE)/hardware/mtkcam/drv/driver.mk
#-----------------------------------------------------------
LOCAL_C_INCLUDES += $(call include-path-for, camera)

LOCAL_SRC_FILES +=  \
    NormalStream.cpp \
    HalPipeWrapper.cpp \
    HalPipeEnqueNode.cpp \
    HalPipeDequeNode.cpp \
    DipUtility.cpp

#
#
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_SOURCE)/hardware/mtkcam/drv/include/dip/$(MTKCAM_DRV_PLATFORM)
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_SOURCE)/hardware/mtkcam/drv/src/isp/dip/$(MTKCAM_DRV_DIP_VERSION_FOLDER)/inc
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_SOURCE)/hardware/mtkcam/drv/src/isp/dip/$(MTKCAM_DRV_DIP_VERSION_FOLDER)/$(MTKCAM_DRV_PLATFORM)/drv
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_SOURCE)/hardware/mtkcam/drv/src/isp/dip/$(MTKCAM_DRV_DIP_VERSION_FOLDER)/$(MTKCAM_DRV_PLATFORM)/postproc
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_SOURCE)/hardware/mtkcam/include
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_SOURCE)/hardware/ldvt/$(TARGET_BOARD_PLATFORM)/include
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_SOURCE)/external/aee/binary/inc
#
# because IPipe.h has <vector>
#LOCAL_C_INCLUDES += $(TOP)/bionic
#LOCAL_C_INCLUDES += $(TOP)/external/stlport/stlport
#
#Thread Priority
LOCAL_C_INCLUDES += $(TOP)/system/core/include
#
#-----------------------------------------------------------

-include $(TOP)/$(MTK_PATH_SOURCE)/hardware/mtkcam/mtkcam.mk
-include $(TOP)/$(MTK_PATH_SOURCE)/hardware/mtkcam/drv/driver.mk

#-----------------------------------------------------------
# Wpe support control
#-----------------------------------------------------------
ifneq (,$(filter $(strip $(TARGET_BOARD_PLATFORM)), mt6775 mt6771 mt6779))
    MTKWPE_SUPPORT := 1
    LOCAL_SRC_FILES += WpeStream.Thread.cpp \
                       WpeUtility.cpp \
                       WpeStream.cpp
else
    MTKWPE_SUPPORT := 0
endif
#-----------------------------------------------------------
LOCAL_CFLAGS += -DMTKWPE_SUPPORT=$(MTKWPE_SUPPORT)
#-----------------------------------------------------------
#-----------------------------------------------------------
#-----------------------------------------------------------

ifeq ($(HAVE_AEE_FEATURE),yes)
    LOCAL_CFLAGS += -DHAVE_AEE_FEATURE
endif

ifeq ($(BUILD_MTK_LDVT),yes)
    LOCAL_CFLAGS += -DUSING_MTK_LDVT
    LOCAL_WHOLE_STATIC_LIBRARIES += libuvvf
endif

#HALPOSTPROC_USING_TEST := '1'
#LOCAL_CFLAGS += -DHALPOSTPROC_USING_TEST
#-----------------------------------------------------------
LOCAL_CFLAGS += -DMTKCAM_LOGENABLE_DEFAULT=$(MTKCAM_LOGENABLE_DEFAULT)
#-----------------------------------------------------------
LOCAL_CFLAGS += $(MTKCAM_CFLAGS)
#
LOCAL_STATIC_LIBRARIES +=
#
LOCAL_SHARED_LIBRARIES += libdip_postproc
LOCAL_HEADER_LIBRARIES += libhardware_headers
#-----------------------------------------------------------
LOCAL_MODULE := libdip.iopipe.postproc
LOCAL_PROPRIETARY_MODULE := true
LOCAL_MODULE_OWNER := mtk

#-----------------------------------------------------------
include $(MTK_STATIC_LIBRARY)


################################################################################
#
################################################################################
include $(CLEAR_VARS)
include $(call all-makefiles-under,$(LOCAL_PATH))


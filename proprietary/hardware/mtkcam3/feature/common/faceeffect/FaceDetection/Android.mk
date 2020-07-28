# Copyright Statement:
#
# This software/firmware and related documentation ("MediaTek Software") are
# protected under relevant copyright laws. The information contained herein
# is confidential and proprietary to MediaTek Inc. and/or its licensors.
# Without the prior written permission of MediaTek inc. and/or its licensors,
# any reproduction, modification, use or disclosure of MediaTek Software,
# and information contained herein, in whole or in part, shall be strictly prohibited.
#
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
#===============================================================================


LOCAL_PATH:= $(call my-dir)

include $(CLEAR_VARS)

LOCAL_ARM_MODE := arm

#
# Start of common part ------------------------------------
-include $(TOP)/$(MTK_PATH_SOURCE)/hardware/mtkcam/mtkcam.mk
-include $(TOP)/$(MTK_PATH_SOURCE)/hardware/mtkcam3/feature/common/faceeffect/facefeature.mk

ifeq ($(BUILD_MTK_LDVT),true)
    LOCAL_CFLAGS += -DUSING_MTK_LDVT
endif

LOCAL_SRC_FILES += \
    $(FDFT_DIR)/fd_hal_base.cpp \
    $(FDFT_DIR)/fdvt_hal.cpp

ifeq ($(FDFT_PSD_FLOW),1)
LOCAL_CFLAGS += -DMTK_ALGO_PSD_MODE=1
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_CUSTOM)/hal/inc
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_CUSTOM_PLATFORM)/hal/inc
ifeq ($(FDFT_USE_HW),'1')
LOCAL_SHARED_LIBRARIES += libmtkcam_fdvt
endif
LOCAL_SHARED_LIBRARIES += libcamalgo.fdft
else
LOCAL_CFLAGS += -DMTK_ALGO_PSD_MODE=0
LOCAL_C_INCLUDES:= \
     $(TOP)/$(MTK_PATH_CUSTOM)/hal/inc \
     $(TOP)/$(MTK_PATH_CUSTOM_PLATFORM)/hal/inc
endif

#-----------------------------------------------------------
LOCAL_C_INCLUDES += $(MTKCAM_C_INCLUDES)
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_SOURCE)/hardware/gralloc_extra/include
#LOCAL_C_INCLUDES += $(TOP)/libhardware/include
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_SOURCE)/hardware/mtkcam3/include $(MTK_PATH_SOURCE)/hardware/mtkcam/include
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_SOURCE)/hardware/libdnn_feature/include
LOCAL_HEADER_LIBRARIES += libmtkcam_headers
#-----------------------------------------------------------
ifeq ($(MTK_CAM_SD_SUPPORT),yes)
LOCAL_CFLAGS += -DSMILE_DETECT_SUPPORT=1
else
LOCAL_CFLAGS += -DSMILE_DETECT_SUPPORT=0
endif

LOCAL_CFLAGS += -DMTKCAM_HWFD_MAIN_VERSION=$(FDVER)
LOCAL_CFLAGS += $(MTKCAM_CFLAGS)
LOCAL_CFLAGS += -DMTKCAM_FDFT_USE_HW="$(FDFT_USE_HW)"
LOCAL_CFLAGS += -DMTKCAM_FDFT_SUB_VERSION="$(FDFT_SUB_VERSION)"
LOCAL_CFLAGS += -DUSE_PORTRAIT=$(FDFT_SUPPORT_PORTRAIT)
ifeq ($(MTK_CAM_DLFD_SUPPORT), yes)
LOCAL_CFLAGS += -DUSE_DL_FD=1
else
LOCAL_CFLAGS += -DUSE_DL_FD=0
endif

LOCAL_SHARED_LIBRARIES += libdpframework libutils libmtkcam_imgbuf
LOCAL_SHARED_LIBRARIES += libmtkcam_ulog
LOCAL_SHARED_LIBRARIES += libcamalgo.fdft
LOCAL_STATIC_LIBRARIES := \

LOCAL_WHOLE_STATIC_LIBRARIES := \

LOCAL_MODULE := libfeature.fdft
LOCAL_PROPRIETARY_MODULE := true
LOCAL_MODULE_OWNER := mtk

# End of common part ---------------------------------------
#
include $(MTK_STATIC_LIBRARY)
include $(CLEAR_VARS)
include $(call all-makefiles-under,$(LOCAL_PATH))

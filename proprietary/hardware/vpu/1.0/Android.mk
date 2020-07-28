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



ifneq (,$(filter $(strip $(TARGET_BOARD_PLATFORM)), mt6799, mt6758))

LOCAL_PATH:= $(call my-dir)


include $(CLEAR_VARS)

LOCAL_SRC_FILES := vpuImpl.cpp
LC_MTK_PLATFORM := $(shell echo $(MTK_PLATFORM) | tr A-Z a-z )

LOCAL_C_INCLUDES:= \
  $(TOP)/kernel-4.4/drivers/misc/mediatek/vpu/1.0/ \
  $(TOP)/$(MTK_PATH_SOURCE)/hardware/m4u/$(LC_MTK_PLATFORM) \
  $(TOP)/$(MTK_PATH_SOURCE)/hardware/vpu/include/ \
  $(TOP)/$(MTK_PATH_SOURCE)/hardware/vpu/1.0/platform/$(TARGET_BOARD_PLATFORM)/ \

# ion/ion.h : standard ion api
#LOCAL_C_INCLUDES += $(TOP)/system/core/libion/include

# libion_mtk/include/ion.h : path for mtk ion api
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_SOURCE)/external

# linux/ion.h : ioctl define for ion
LOCAL_C_INCLUDES += $(TOP)/system/core/libion/kernel-headers

# linux/ion_drv.h :define for ion_mm_data_t
LOCAL_C_INCLUDES += $(TOP)/device/mediatek/common/kernel-headers

# mt_iommu_port.h :define for m4u_lib_port
LOCAL_C_INCLUDES += $(TOP)/device/mediatek/$(LC_MTK_PLATFORM)/kernel-headers

#-----------------------------------------------------------

LOCAL_SHARED_LIBRARIES := \
    libstdc++ \
    libion \
    libion_mtk \
    liblog \
    libcutils \
    libbwc \

LOCAL_STATIC_LIBRARIES := \

LOCAL_WHOLE_STATIC_LIBRARIES := \

ifneq (,$(filter $(strip $(TARGET_BOARD_PLATFORM)), mt6758))
LOCAL_CFLAGS += -DHAVE_EFUSE_BOND
else
LOCAL_CFLAGS := 
endif

#-----------------------------------------------------------
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE := libvpu
LOCAL_PROPRIETARY_MODULE := true
LOCAL_MODULE_OWNER := mtk
#-----------------------------------------------------------
include $(BUILD_SHARED_LIBRARY)


################################################################################
#
################################################################################
include $(CLEAR_VARS)
include $(call all-makefiles-under,$(LOCAL_PATH))




endif



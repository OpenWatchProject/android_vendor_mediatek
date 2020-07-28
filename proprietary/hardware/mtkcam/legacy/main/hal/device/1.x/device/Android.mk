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
-include $(TOP)/$(MTK_PATH_SOURCE)/hardware/mtkcam/legacy/v1/client.mk

#-----------------------------------------------------------
LOCAL_SRC_FILES += CameraDevice.cpp
LOCAL_SRC_FILES += CameraDevice1Factory.cpp
LOCAL_SRC_FILES += CameraDevice1Base.cpp
LOCAL_SRC_FILES += CameraDevice1Base.dump.cpp
LOCAL_SRC_FILES += CameraDevice1Base.wrapper.cpp

LOCAL_SRC_FILES += MtkCameraDevice.cpp
#-----------------------------------------------------------
LOCAL_HEADER_LIBRARIES += libmtkcam_headers
LOCAL_C_INCLUDES += $(MTKCAM_C_INCLUDES)
LOCAL_C_INCLUDES += $(MTK_PATH_SOURCE)/hardware/mtkcam/include
LOCAL_C_INCLUDES += $(MTK_PATH_SOURCE)/hardware/mtkcam/legacy/include
LOCAL_C_INCLUDES += $(TOP)/$(MTK_MTKCAM_PLATFORM)/include
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../../include
LOCAL_C_INCLUDES += $(TOP)/hardware/interfaces/camera/common/1.0/default/include
LOCAL_C_INCLUDES += $(TOP)/hardware/interfaces/graphics/mapper/3.0

LOCAL_C_INCLUDES += $(TOP)/system/media/camera/include

# for AOSP HardwareAPI library link in o0
# should be phased after o1 by using media_plugin_headers instead
LOCAL_C_INCLUDES += $(TOP)/frameworks/native
LOCAL_C_INCLUDES += $(TOP)/frameworks/native/include/media/openmax

# for AOSP HardwareAPI library link in o1 (adaptive)
# should be phased after o1 by using media_plugin_headers instead
LOCAL_C_INCLUDES += $(TOP)/frameworks/native/headers/media_plugin

# LOCAL_HEADER_LIBRARIES += media_plugin_headers

LOCAL_C_INCLUDES += $(TOP)/system/core/include/utils/
#

#-----------------------------------------------------------
LOCAL_CFLAGS += $(MTKCAM_CFLAGS)
#
LOCAL_CFLAGS += -DLOG_TAG='"mtkcam-dev1"'
LOCAL_CFLAGS += -DMTKCAM_HAVE_DISPLAY_CLIENT="$(MTKCAM_HAVE_DISPLAY_CLIENT)"
#
LOCAL_CFLAGS += -fexceptions

#-----------------------------------------------------------
LOCAL_WHOLE_STATIC_LIBRARIES +=
#
LOCAL_STATIC_LIBRARIES += libmtkcam_device1_hidlcommon
LOCAL_STATIC_LIBRARIES += android.hardware.camera.common@1.0-helper
#
#-----------------------------------------------------------
LOCAL_SHARED_LIBRARIES += liblog
LOCAL_SHARED_LIBRARIES += libutils
LOCAL_SHARED_LIBRARIES += libcutils
LOCAL_SHARED_LIBRARIES += libhidlbase
LOCAL_SHARED_LIBRARIES += libhidlmemory
LOCAL_SHARED_LIBRARIES += libmtkcam_metadata
LOCAL_SHARED_LIBRARIES += libhidltransport
LOCAL_SHARED_LIBRARIES += android.hardware.camera.device@1.0
LOCAL_SHARED_LIBRARIES += android.hardware.camera.common@1.0

LOCAL_SHARED_LIBRARIES += android.hardware.graphics.mapper@2.0
LOCAL_SHARED_LIBRARIES += android.hardware.graphics.mapper@3.0
LOCAL_SHARED_LIBRARIES += android.hidl.allocator@1.0
LOCAL_SHARED_LIBRARIES += android.hidl.memory@1.0
LOCAL_SHARED_LIBRARIES += libcam.halsensor
LOCAL_SHARED_LIBRARIES += vendor.mediatek.hardware.camera.device@1.1
#

#-----------------------------------------------------------
LOCAL_EXPORT_C_INCLUDE_DIRS := $(LOCAL_PATH)/../include

#-----------------------------------------------------------
LOCAL_MODULE := libmtkcam_device1_base
LOCAL_MODULE_OWNER := mtk
LOCAL_PROPRIETARY_MODULE := true

#-----------------------------------------------------------
include $(MTK_STATIC_LIBRARY)


################################################################################
#
################################################################################
include $(call all-makefiles-under,$(LOCAL_PATH))

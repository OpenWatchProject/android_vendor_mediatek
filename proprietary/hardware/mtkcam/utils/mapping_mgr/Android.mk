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
-include $(TOP)/$(MTK_PATH_SOURCE)/hardware/mtkcam/aaa/aaa.mk
#-----------------------------------------------------------


ifneq (,$(filter $(strip $(TARGET_BOARD_PLATFORM)), mt6779 mt6883 mt6885))
	BUILD_VERSION = 2.0
else ifneq (,$(filter $(strip $(TARGET_BOARD_PLATFORM)), mt6771 mt6775))
	BUILD_VERSION = 1.0
else
	BUILD_VERSION = 1.0
endif

LOCAL_SRC_FILES += $(BUILD_VERSION)/cam_idx_mgr.cpp
LOCAL_SRC_FILES += nvram_ut.cpp

ifneq (,$(filter $(strip $(MTKCAM_AAA_PLATFORM)), isp_6s))
LOCAL_SRC_FILES += idx_cache/idx_cache.cpp
endif
#------------------ip base new include path-----------------------
LOCAL_C_INCLUDES += $(MTK_PATH_SOURCE)/hardware/mtkcam/include
LOCAL_C_INCLUDES += $(MTK_PATH_CUSTOM_PLATFORM)/hal/inc

#-----------------------------------------------------------
LOCAL_C_INCLUDES += $(MTKCAM_C_INCLUDES)
LOCAL_C_INCLUDES += $(MTK_PATH_SOURCE)/custom
LOCAL_C_INCLUDES += $(MTK_PATH_CUSTOM_PLATFORM)/hal/inc
LOCAL_C_INCLUDES += $(MTK_PATH_CUSTOM_PLATFORM)/hal/inc/aaa
LOCAL_C_INCLUDES += $(MTK_PATH_CUSTOM_PLATFORM)/hal/inc/isp_tuning
LOCAL_C_INCLUDES += $(MTK_PATH_CUSTOM_PLATFORM)/hal/inc/isp_tuning/ver1
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_SOURCE)/hardware/mtkcam/include/mtkcam
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_SOURCE)/hardware/mtkcam/aaa/include
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_SOURCE)/hardware/mtkcam/include
#LOCAL_C_INCLUDES += $(MTK_PATH_CUSTOM_PLATFORM)/hal/inc/aaa
#LOCAL_C_INCLUDES += $(MTK_PATH_CUSTOM_PLATFORM)/hal/inc/pd_buf_mgr
#LOCAL_C_INCLUDES += $(MTK_PATH_CUSTOM_PLATFORM)/hal/inc/mtkcam
#LOCAL_C_INCLUDES += $(MTK_PATH_CUSTOM_PLATFORM)/hal/inc/debug_exif/aaa
#LOCAL_C_INCLUDES += $(MTK_PATH_CUSTOM_PLATFORM)/hal/inc/debug_exif/cam
#LOCAL_C_INCLUDES += $(MTK_PATH_CUSTOM_PLATFORM)/hal/camera_3a
#LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_SOURCE)/hardware/mtkcam/include/mtkcam

ifneq (,$(filter $(strip $(MTKCAM_AAA_PLATFORM)), isp_6s))
LOCAL_HEADER_LIBRARIES += aaa_utils_common_includes
endif
#-----------------------------------------------------------
LOCAL_CFLAGS += $(MTKCAM_CFLAGS)

#-----------------------------------------------------------
ifneq (,$(filter $(strip $(TARGET_BOARD_PLATFORM)), mt6771 mt6779 mt6785 mt6883 mt6885))
LOCAL_STATIC_LIBRARIES += libcam.hal3a_utils
LOCAL_SHARED_LIBRARIES += libmtkcam_modulehelper
endif
LOCAL_STATIC_LIBRARIES +=

#-----------------------------------------------------------
LOCAL_SHARED_LIBRARIES += liblog
LOCAL_SHARED_LIBRARIES += libutils
LOCAL_SHARED_LIBRARIES += libcutils
LOCAL_SHARED_LIBRARIES += libmtkcam_stdutils
LOCAL_SHARED_LIBRARIES += libmtkcam_debugutils
LOCAL_SHARED_LIBRARIES += libmtkcam_grallocutils
LOCAL_SHARED_LIBRARIES += libmtkcam_sysutils
LOCAL_SHARED_LIBRARIES += libutilscallstack
LOCAL_SHARED_LIBRARIES += libaedv
LOCAL_SHARED_LIBRARIES += lib3a.awb.core
LOCAL_SHARED_LIBRARIES += lib3a.af.core
#For AE
LOCAL_SHARED_LIBRARIES += lib3a.ae.core

ifneq (,$(filter $(strip $(MTKCAM_AAA_PLATFORM)), isp_50 isp_60 isp_6s))

LOCAL_SHARED_LIBRARIES += libcam.hal3a.v3.nvram.50
endif
LOCAL_HEADER_LIBRARIES += libhardware_headers
LOCAL_SHARED_LIBRARIES += lib3a.flash
#-----------------------------------------------------------
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE := libmtkcam_mapping_mgr
LOCAL_MODULE_CLASS := SHARED_LIBRARIES
LOCAL_PROPRIETARY_MODULE := true
LOCAL_MODULE_OWNER := mtk
#-----------------------------------------------------------
ifeq ($(MTK_CAM_NEW_NVRAM_SUPPORT),1)
LOCAL_INTERMEDIATES         := $(call local-intermediates-dir)
LOCAL_GEN_TOOL              := $(LOCAL_PATH)/enum2str
LOCAL_MODULE_STR_ARRAY := $(LOCAL_INTERMEDIATES)/EModule_string.h
LOCAL_APP_STR_ARRAY := $(LOCAL_INTERMEDIATES)/EApp_string.h
LOCAL_ISP_PROFILE_STR_ARRAY := $(LOCAL_INTERMEDIATES)/EIspProfile_string.h
LOCAL_SENSOR_MODE_STR_ARRAY := $(LOCAL_INTERMEDIATES)/ESensorMode_string.h

    LOCAL_MODULE_HEADER    := $(MTK_PATH_CUSTOM_PLATFORM)/hal/inc/tuning_mapping/cam_idx_struct_ext.h

$(LOCAL_MODULE_STR_ARRAY): $(LOCAL_MODULE_HEADER) $(LOCAL_GEN_TOOL)
	$(LOCAL_GEN_TOOL) $(LOCAL_MODULE_HEADER) EModule_ strEModule > $@ || { rm $@; exit -1; }
$(LOCAL_APP_STR_ARRAY): $(LOCAL_MODULE_HEADER) $(LOCAL_GEN_TOOL)
	$(LOCAL_GEN_TOOL) $(LOCAL_MODULE_HEADER) EApp_ strEApp > $@ || { rm $@; exit -1; }
$(LOCAL_ISP_PROFILE_STR_ARRAY): $(LOCAL_MODULE_HEADER) $(LOCAL_GEN_TOOL)
	$(LOCAL_GEN_TOOL) $(LOCAL_MODULE_HEADER) EIspProfile_ strEIspProfile > $@ || { rm $@; exit -1; }
$(LOCAL_SENSOR_MODE_STR_ARRAY): $(LOCAL_MODULE_HEADER) $(LOCAL_GEN_TOOL)
	$(LOCAL_GEN_TOOL) $(LOCAL_MODULE_HEADER) ESensorMode_ strESensorMode > $@ || { rm $@; exit -1; }

LOCAL_GENERATED_SOURCES += $(LOCAL_MODULE_STR_ARRAY)
LOCAL_GENERATED_SOURCES += $(LOCAL_APP_STR_ARRAY)
LOCAL_GENERATED_SOURCES += $(LOCAL_ISP_PROFILE_STR_ARRAY)
LOCAL_GENERATED_SOURCES += $(LOCAL_SENSOR_MODE_STR_ARRAY)
LOCAL_C_INCLUDES += $(LOCAL_INTERMEDIATES)
endif

#-----------------------------------------------------------
include $(MTK_SHARED_LIBRARY)
include $(call all-makefiles-under,$(LOCAL_PATH))

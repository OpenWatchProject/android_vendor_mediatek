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

MTK_CAM_NEW_NVRAM_SUPPORT := 1

#-----------------------------------------------------------
LOCAL_SRC_FILES +=

#-----------------------------------------------------------

LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_SOURCE)/custom


define find-all-my-module
$(shell cd $(LOCAL_PATH) ; \
    find ./imgsensor/ -name $(1)*)
endef

all_sensor_modules := $(patsubst ./imgsensor/ver1/%,%_tuning,$(foreach f, $(CUSTOM_HAL_IMGSENSOR), $(call find-all-my-module, $(strip $(f)))))
all_sensor_idxMgr := $(patsubst ./imgsensor/ver1/%,%_IdxMgr,$(foreach f, $(CUSTOM_HAL_IMGSENSOR), $(call find-all-my-module, $(strip $(f)))))

#-----------------------------------------------------------
#
LOCAL_WHOLE_STATIC_LIBRARIES += libcameracustom.camera
LOCAL_WHOLE_STATIC_LIBRARIES += libcameracustom.camera.3a
LOCAL_WHOLE_STATIC_LIBRARIES += libcameracustom.camera_exif
LOCAL_WHOLE_STATIC_LIBRARIES += libcameracustom.flashlight
LOCAL_WHOLE_STATIC_LIBRARIES += libcameracustom.lens
LOCAL_WHOLE_STATIC_LIBRARIES += libcameracustom.imgsensor.core
LOCAL_WHOLE_STATIC_LIBRARIES += libcameracustom.eeprom
LOCAL_WHOLE_STATIC_LIBRARIES += libcameracustom.pd_buf_mgr

#-----------------------------------------------------------
LOCAL_SHARED_LIBRARIES += liblog
LOCAL_SHARED_LIBRARIES += libutils
LOCAL_SHARED_LIBRARIES += libcutils
LOCAL_SHARED_LIBRARIES += libaedv
LOCAL_SHARED_LIBRARIES += libcam.hal3a.log
LOCAL_SHARED_LIBRARIES += libmtkcam_stdutils
LOCAL_SHARED_LIBRARIES += libSonyIMX230PdafLibraryWrapper
LOCAL_SHARED_LIBRARIES += libSonyIMX386PdafLibraryWrapper
LOCAL_SHARED_LIBRARIES += libSonyIMX338PdafLibraryWrapper
LOCAL_SHARED_LIBRARIES += libSonyIMX519PdafLibraryWrapper
#LOCAL_SHARED_LIBRARIES += libcameracustom.imgsensor
#LOCAL_SHARED_LIBRARIES += $(all_sensor_modules)
LOCAL_SHARED_LIBRARIES += $(all_sensor_idxMgr)

LOCAL_SHARED_LIBRARIES += libmtkcam_modulehelper
#For AE
LOCAL_SHARED_LIBRARIES += lib3a.ae.core

#-----------------------------------------------------------
#LOCAL_MODULE_TAGS := optional
LOCAL_MODULE := libcameracustom
LOCAL_PROPRIETARY_MODULE := true
LOCAL_MODULE_OWNER := mtk
#-----------------------------------------------------------
include $(MTK_SHARED_LIBRARY)


################################################################################
#
################################################################################

include $(CLEAR_VARS)
include $(call all-makefiles-under,$(LOCAL_PATH))


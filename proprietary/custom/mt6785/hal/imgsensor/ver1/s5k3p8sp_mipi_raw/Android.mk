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

# Use project imgsensor first
ifeq ($(wildcard $(MTK_PATH_CUSTOM)/hal/imgsensor),)

#function definition
define all-c-cpp-under
$(patsubst ./%,%, \
$(shell cd $(LOCAL_PATH) ; \
find -L $(1) -maxdepth 1 \( -name "*.c" -or -name "*.cpp" \) -and -not -name ".*") \
)
endef

define current_module_name
$(shell basename $(LOCAL_PATH))
endef

define find-all-sub-module
$(shell cd $(LOCAL_PATH) ; \
    find ./*/ -name Android.mk)
endef

################################################################################
#
################################################################################
include $(CLEAR_VARS)


##read Module ID from folder
LOCAL_MODULE_INDEX=$(wordlist 4,4,$(subst _, ,$(call current_module_name)))
#set default moduleID
ifeq ($(LOCAL_MODULE_INDEX),)
LOCAL_MODULE_INDEX=0
endif

LOCAL_CFLAGS += -DMODULE_INDEX=$(LOCAL_MODULE_INDEX)

#get module name
LOCAL_MODULE_NAME=$(call current_module_name)
$(info LOCAL_MODULE_NAME=$(LOCAL_MODULE_NAME), LOCAL_MODULE_ID=$(LOCAL_MODULE_INDEX))

all_sub_modules := $(patsubst ./%/Android.mk,%,$(call find-all-sub-module))
LOCAL_filter += $(patsubst %,%/$(subst _,,$(LOCAL_MODULE_NAME))_%, $(all_sub_modules))
LOCAL_filter += AE_Tuning_Para/%
LOCAL_filter += camera_indexmgr_api.cpp
#$(info Find sub modules = $(all_sub_modules))
#$(info LOCAL_filters = $(LOCAL_filter))


#find all cpp and make code
cppall_t := $(call all-cpp-files-under)
LOCAL_cppall_t := $(call all-cpp-files-under)
LOCAL_cppall := $(filter-out $(LOCAL_filter), $(LOCAL_cppall_t))

#$(info All my cpp files = $(LOCAL_cppall))

#-----------------------------------------------------------
LOCAL_SRC_FILES += $(LOCAL_cppall)

#-----------------------------------------------------------
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_CUSTOM)/hal/inc/aaa
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_CUSTOM)/hal/inc/camera_feature
#
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_CUSTOM)/hal/camera
#
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_COMMON)/kernel/imgsensor/inc
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_CUSTOM)/kernel/eeprom/inc
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_CUSTOM)/kernel/cam_cal/inc
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_CUSTOM)/hal/inc \

LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_COMMON)/hal/inc
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_CUSTOM_PLATFORM)/hal/inc
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_CUSTOM_PLATFORM)/hal/inc/aaa

LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_CUSTOM_PLATFORM)/hal/inc/isp_tuning


LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_CUSTOM_PLATFORM)/hal/inc/imgsensor \

LOCAL_C_INCLUDES += $(LOCAL_PATH)

#include all sub folder
LOCAL_C_INCLUDES += $(patsubst %,$(LOCAL_PATH)/%,$(all_sub_modules))
#$(info local LOCAL_C_INCLUDES = $(LOCAL_C_INCLUDES))


#-----------------------------------------------------------
LOCAL_SHARED_LIBRARIES += liblog
LOCAL_SHARED_LIBRARIES += $(patsubst %, libCamera_$(subst _,,$(LOCAL_MODULE_NAME))_%, $(all_sub_modules))
#$(info LOCAL_SHARED_LIBRARIES = $(LOCAL_SHARED_LIBRARIES))
LOCAL_HEADER_LIBRARIES += libutils_headers
#seanlin 120920 for adding camera_clibration_eeprom.cpp
#seanlin 121005 for adding camera_clibration_cam_cal.cpp
#-----------------------------------------------------------
LOCAL_STATIC_LIBRARIES +=
#
LOCAL_WHOLE_STATIC_LIBRARIES +=



#-----------------------------------------------------------
LOCAL_MODULE := $(LOCAL_MODULE_NAME)_tuning
LOCAL_PROPRIETARY_MODULE := true
LOCAL_MODULE_OWNER := mtk
#-----------------------------------------------------------

#
# Start of common part ------------------------------------
sinclude $(TOP)/$(MTK_PATH_PLATFORM)/hardware/mtkcam/mtkcam.mk

#-----------------------------------------------------------
LOCAL_CFLAGS += $(MTKCAM_CFLAGS)
LOCAL_CFLAGS += -DMTK_CAM_NEW_NVRAM_SUPPORT=1

#-----------------------------------------------------------
LOCAL_C_INCLUDES += $(MTKCAM_C_INCLUDES)

#-----------------------------------------------------------
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_SOURCE)/hardware/include
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_PLATFORM)/hardware/include
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_SOURCE)/hardware/mtkcam
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_SOURCE)/hardware/mtkcam/include
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_SOURCE)/hardware/mtkcam/common/include
# End of common part ---------------------------------------
include $(MTK_SHARED_LIBRARY)

################################################################################
#
################################################################################

include $(CLEAR_VARS)

LOCAL_CFLAGS += -DMODULE_INDEX=$(LOCAL_MODULE_INDEX)
LOCAL_CFLAGS += -DMTK_CAM_NEW_NVRAM_SUPPORT=1

LOCAL_SRC_FILES += camera_indexmgr_api.cpp

LOCAL_C_INCLUDES += $(MTKCAM_C_INCLUDES)

#-----------------------------------------------------------
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_SOURCE)/hardware/include
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_PLATFORM)/hardware/include
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_SOURCE)/hardware/mtkcam
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_SOURCE)/hardware/mtkcam/include
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_SOURCE)/hardware/mtkcam/common/include
#-----------------------------------------------------------
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_CUSTOM)/hal/inc/aaa
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_CUSTOM)/hal/inc/camera_feature
#
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_CUSTOM)/hal/camera
#
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_COMMON)/kernel/imgsensor/inc
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_CUSTOM)/kernel/eeprom/inc
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_CUSTOM)/kernel/cam_cal/inc
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_CUSTOM)/hal/inc \

LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_COMMON)/hal/inc
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_CUSTOM_PLATFORM)/hal/inc
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_CUSTOM_PLATFORM)/hal/inc/aaa
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_CUSTOM_PLATFORM)/hal/inc/isp_tuning
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_CUSTOM_PLATFORM)/hal/inc/imgsensor

LOCAL_C_INCLUDES += $(LOCAL_PATH)
#include all sub folder
LOCAL_C_INCLUDES += $(patsubst %,$(LOCAL_PATH)/%,$(all_sub_modules))


LOCAL_SHARED_LIBRARIES += liblog
LOCAL_HEADER_LIBRARIES += libutils_headers
LOCAL_MODULE := $(LOCAL_MODULE_NAME)_IdxMgr
LOCAL_PROPRIETARY_MODULE := true
LOCAL_MODULE_OWNER := mtk

include $(MTK_SHARED_LIBRARY)

################################################################################
#
################################################################################

include $(CLEAR_VARS)
include $(call all-makefiles-under,$(LOCAL_PATH))
endif



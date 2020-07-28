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


ifneq ($(PLATFORM_VERSION),4.4.4)
################################################################################
#
################################################################################
include $(CLEAR_VARS)

#-----------------------------------------------------------
-include $(TOP)/$(MTK_PATH_SOURCE)/hardware/mtkcam/mtkcam.mk
-include $(TOP)/$(MTK_PATH_SOURCE)/hardware/mtkcam/drv/driver.mk
-include $(TOP)/$(MTK_PATH_SOURCE)/hardware/mtkcam/aaa/source/common/lsc_mgr/lsc.mk
-include $(TOP)/$(MTK_PATH_SOURCE)/hardware/mtkcam/aaa/aaa.mk
#--------------------------------------------

#ifeq ($(MTKCAM_AAA_PLATFORM), isp_50)
ifneq (,$(filter $(strip $(MTKCAM_AAA_PLATFORM)), isp_50 isp_60 isp_6s))

LOCAL_SRC_FILES += LscBufImp.cpp
LOCAL_SRC_FILES += LscNvramImp.cpp
LOCAL_SRC_FILES += LscNvramImp_dep.cpp
LOCAL_SRC_FILES += LscTsf.cpp
LOCAL_SRC_FILES += LscMgr.cpp

PLATFORM_FOLDER=$(TARGET_BOARD_PLATFORM)
MY_TARGET_PLATFORM=$(TARGET_BOARD_PLATFORM)

ifeq ($(TARGET_BOARD_PLATFORM), mt6757)
ifeq ($(strip $(MTK_CAM_SW_VERSION)),ver2)
PLATFORM_FOLDER=$(TARGET_BOARD_PLATFORM)p
endif
endif

ifeq ($(MTKCAM_AAA_PLATFORM), isp_50)
PLATFORM_FOLDER=isp_50
MY_TARGET_PLATFORM=isp_50
else ifeq ($(MTKCAM_AAA_PLATFORM), isp_60)
PLATFORM_FOLDER=isp_60
MY_TARGET_PLATFORM=isp_60
else ifeq ($(MTKCAM_AAA_PLATFORM), isp_6s)
PLATFORM_FOLDER=isp_6s
MY_TARGET_PLATFORM=isp_6s
endif

ifeq ($(MTKCAM_AAA_PLATFORM), isp_30)
PLATFORM_FOLDER=isp_30
endif

LOCAL_SRC_FILES += ../../$(PLATFORM_FOLDER)/lsc_mgr/LscMgrDefault.cpp
LOCAL_SRC_FILES += ../../$(PLATFORM_FOLDER)/lsc_mgr/LscMgrDefault_dep.cpp
LOCAL_SRC_FILES += ../../$(PLATFORM_FOLDER)/lsc_mgr/OpenShading.cpp
LOCAL_SRC_FILES += ../../$(PLATFORM_FOLDER)/lsc_mgr/OpenShading_dep.cpp
LOCAL_SRC_FILES += ../../$(PLATFORM_FOLDER)/lsc_mgr/TsfDft.cpp
LOCAL_SRC_FILES += ../../$(PLATFORM_FOLDER)/lsc_mgr/TsfDft_dep.cpp
LOCAL_SRC_FILES += ../../$(PLATFORM_FOLDER)/lsc_mgr/LscMgrDefault.misc.cpp
LOCAL_SRC_FILES += ../../$(PLATFORM_FOLDER)/sensor_mgr/aaa_sensor_mgr.cpp

ifeq ($(MTKCAM_AAA_PLATFORM), isp_30)
LOCAL_SRC_FILES += ../../$(PLATFORM_FOLDER)/isp_mgr/$(TARGET_BOARD_PLATFORM)/isp_mgr.cpp
LOCAL_SRC_FILES += ../../$(PLATFORM_FOLDER)/isp_mgr/$(TARGET_BOARD_PLATFORM)/isp_mgr_lsc.cpp
LOCAL_SRC_FILES += ../../$(PLATFORM_FOLDER)/isp_mgr/$(TARGET_BOARD_PLATFORM)/isp_mgr_ctl.cpp
else
ifeq ($(MTKCAM_AAA_PLATFORM), isp_60)
LOCAL_SRC_FILES += ../../$(PLATFORM_FOLDER)/lsc_mgr/LogHelper.cpp
$(info LscMgr isp60 decouple)
else
LOCAL_SRC_FILES += ../../$(PLATFORM_FOLDER)/isp_mgr/isp_mgr.cpp
LOCAL_SRC_FILES += ../../$(PLATFORM_FOLDER)/isp_mgr/isp_mgr_lsc.cpp
LOCAL_SRC_FILES += ../../$(PLATFORM_FOLDER)/isp_mgr/isp_mgr_ctl.cpp
endif
endif

ifneq ("$(wildcard $(MTK_PATH_SOURCE)/hardware/mtkcam/aaa/source/$(PLATFORM_FOLDER)/isp_tuning/isp_tuning_buf.cpp)","")
LOCAL_SRC_FILES += ../../$(PLATFORM_FOLDER)/isp_tuning/isp_tuning_buf.cpp
endif

ifneq ("$(wildcard $(MTK_PATH_SOURCE)/hardware/mtkcam/aaa/source/$(PLATFORM_FOLDER)/isp_tuning/$(TARGET_BOARD_PLATFORM)/isp_tuning_buf.cpp)","")
LOCAL_SRC_FILES += ../../$(PLATFORM_FOLDER)/isp_tuning/$(TARGET_BOARD_PLATFORM)/isp_tuning_buf.cpp
endif

#-----------------------------------------------------------
LOCAL_C_INCLUDES += $(MTKCAM_C_INCLUDES)
#For BuildInTypes.h
LOCAL_C_INCLUDES += $(MTK_PATH_SOURCE)/hardware/mtkcam/include/mtkcam/def
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_SOURCE)/hardware/mtkcam/include
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_SOURCE)/hardware/mtkcam/aaa/include
LOCAL_C_INCLUDES += $(MTKCAM_ALGO_LSC_INCLUDE)
LOCAL_C_INCLUDES += $(MTKCAM_ALGO_INCLUDE)
LOCAL_C_INCLUDES += $(MTKCAM_DRV_INCLUDE)
LOCAL_C_INCLUDES += $(MTK_PATH_SOURCE)/hardware/mtkcam/aaa/source/$(PLATFORM_FOLDER)
LOCAL_C_INCLUDES += $(MTK_PATH_SOURCE)/hardware/mtkcam/aaa/source/$(PLATFORM_FOLDER)/isp_mgr
LOCAL_C_INCLUDES += $(MTK_PATH_SOURCE)/hardware/mtkcam/aaa/source/$(PLATFORM_FOLDER)/af_mgr
LOCAL_C_INCLUDES += $(MTK_PATH_SOURCE)/hardware/mtkcam/aaa/source/$(PLATFORM_FOLDER)/isp_mgr/$(TARGET_BOARD_PLATFORM)
LOCAL_C_INCLUDES += $(MTK_PATH_SOURCE)/hardware/mtkcam/aaa/source/$(PLATFORM_FOLDER)/drv
LOCAL_C_INCLUDES += $(MTK_PATH_SOURCE)/hardware/mtkcam/drv/include/$(MTKCAM_DRV_PLATFORM)
LOCAL_C_INCLUDES += $(MTK_PATH_SOURCE)/hardware/mtkcam/include/mtkcam/
LOCAL_C_INCLUDES += $(MTK_PATH_SOURCE)/hardware/mtkcam/drv/include/$(MTKCAM_DRV_PLATFORM)/drv
LOCAL_C_INCLUDES += $(MTK_PATH_SOURCE)/hardware/mtkcam/aaa/source/common
LOCAL_C_INCLUDES += $(MTK_PATH_SOURCE)/hardware/mtkcam/aaa/source/common/lsc_tbl

ifneq (,$(filter $(strip $(MTKCAM_AAA_PLATFORM)), isp_60 isp_6s))
LOCAL_C_INCLUDES += $(MTK_PATH_SOURCE)/hardware/mtkcam/aaa/source/common/hal3a/$(MTKCAM_COMMON_HAL3A_VERSION)/ResultPool
LOCAL_C_INCLUDES += $(MTK_PATH_SOURCE)/hardware/mtkcam/aaa/source/$(PLATFORM_FOLDER)/isp_tuning_buf
endif

LOCAL_C_INCLUDES += $(MTK_PATH_SOURCE)/hardware/mtkcam/aaa/source/$(PLATFORM_FOLDER)/isp_tuning
LOCAL_C_INCLUDES += $(MTK_PATH_SOURCE)/hardware/mtkcam/aaa/source/$(PLATFORM_FOLDER)/lsc_mgr
LOCAL_C_INCLUDES += $(MTK_PATH_SOURCE)/hardware/mtkcam/aaa/source/$(PLATFORM_FOLDER)/isp_tuning/$(TARGET_BOARD_PLATFORM)
LOCAL_C_INCLUDES += $(MTK_PATH_SOURCE)/hardware/mtkcam/aaa/source/$(PLATFORM_FOLDER)/sensor_mgr
LOCAL_C_INCLUDES += $(MTK_PATH_CUSTOM_PLATFORM)/hal/inc
LOCAL_C_INCLUDES += $(MTK_PATH_CUSTOM_PLATFORM)/hal/inc/aaa
LOCAL_C_INCLUDES += $(MTK_PATH_CUSTOM_PLATFORM)/hal/inc/isp_tuning
LOCAL_C_INCLUDES += $(MTK_PATH_CUSTOM_PLATFORM)/hal/inc/$(MTK_CAM_SW_VERSION)
LOCAL_C_INCLUDES += $(MTK_PATH_CUSTOM_PLATFORM)/hal/inc/$(MTK_CAM_SW_VERSION)/debug_exif/cam
LOCAL_C_INCLUDES += $(MTK_PATH_CUSTOM_PLATFORM)/hal/inc/debug_exif/aaa
LOCAL_C_INCLUDES += $(MTK_PATH_CUSTOM_PLATFORM)/hal/inc/debug_exif/aaa/$(MTK_CAM_SW_VERSION)
LOCAL_C_INCLUDES += $(MTK_PATH_CUSTOM_PLATFORM)/hal/inc/isp_tuning/$(MTK_CAM_SW_VERSION)
LOCAL_C_INCLUDES += $(MTK_PATH_CUSTOM_PLATFORM)/hal/inc/aaa/debug_exif/aaa/$(MTK_CAM_SW_VERSION)
LOCAL_C_INCLUDES += $(MTK_PATH_CUSTOM_PLATFORM)/hal/inc/aaa/$(MTK_CAM_SW_VERSION)
LOCAL_C_INCLUDES += $(MTK_PATH_CUSTOM_PLATFORM)/hal/inc/ver2
LOCAL_C_INCLUDES += $(MTK_PATH_SOURCE)/custom

#For AEPlineTable.h
LOCAL_C_INCLUDES += $(MTK_PATH_COMMON)/hal/inc/custom/aaa

#-----------------------------------------------------------
LOCAL_CFLAGS += $(MTKCAM_CFLAGS)

#ISP 5.0
LOCAL_CFLAGS += -DCAM3_3A_ISP_50_EN=$(MTKCAM_IS_ISP_50_PLATFORM)

LOCAL_CFLAGS += -DCAM3_3A_IP_BASE=$(MTKCAM_IS_IP_BASE)
#
#-----------------------------------------------------------
#
LOCAL_STATIC_LIBRARIES += lib3a.ccu.headers
#-----------------------------------------------------------
LOCAL_SHARED_LIBRARIES += libmtkcam_ulog
LOCAL_SHARED_LIBRARIES += liblog
LOCAL_SHARED_LIBRARIES += libutils
LOCAL_SHARED_LIBRARIES += libcutils
LOCAL_SHARED_LIBRARIES += lib3a.awb.core
LOCAL_SHARED_LIBRARIES += libcamalgo.ispfeature
LOCAL_SHARED_LIBRARIES += libcam3a_imem
LOCAL_SHARED_LIBRARIES += libcam.hal3a.v3.lsctbl.50
LOCAL_SHARED_LIBRARIES += libcam.hal3a.v3.nvram.50
LOCAL_SHARED_LIBRARIES += libmtkcam_stdutils
LOCAL_SHARED_LIBRARIES += libcameracustom
LOCAL_SHARED_LIBRARIES += libmtkcam_imgbuf
LOCAL_SHARED_LIBRARIES += libmtkcam_modulehelper
LOCAL_SHARED_LIBRARIES += libfeatureiodrv_mem
LOCAL_STATIC_LIBRARIES += libcam.hal3a_utils
LOCAL_STATIC_LIBRARIES += lib3a.ccu.headers
LOCAL_SHARED_LIBRARIES += libmtkcam_tuning_utils
LOCAL_SHARED_LIBRARIES += libmtkcam_mapping_mgr
LOCAL_SHARED_LIBRARIES += lib3a.flash
LOCAL_SHARED_LIBRARIES += lib3a.af.core
#For AE
LOCAL_SHARED_LIBRARIES += lib3a.ae.core
ifneq ("$(wildcard $(TOP)/$(MTK_PATH_SOURCE)/hardware/libcamera_feature/liblsc_lib/libcamalgo.lsc/$(TARGET_BOARD_PLATFORM)/arm/libcamalgo.lsc.so)","")
    LOCAL_SHARED_LIBRARIES += libcamalgo.lsc
else
    LOCAL_SHARED_LIBRARIES += libcamalgo
endif

LOCAL_HEADER_LIBRARIES += libhardware_headers
#-----------------------------------------------------------
LOCAL_MODULE := libcam.hal3a.v3.lscMgr
LOCAL_PROPRIETARY_MODULE := true
LOCAL_MODULE_OWNER := mtk

#-----------------------------------------------------------
include $(MTK_SHARED_LIBRARY)



################################################################################
#
################################################################################
include $(CLEAR_VARS)
#include $(call all-makefiles-under,$(LOCAL_PATH))

endif

endif

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
-include $(TOP)/$(MTK_PATH_SOURCE)/hardware/mtkcam/aaa/aaa.mk

MTKCAM_3A_PATH        := $(MTK_PATH_SOURCE)/hardware/mtkcam/aaa/source/$(MTKCAM_AAA_PLATFORM)
MTKCAM_3A_COMMON_PATH := $(MTK_PATH_SOURCE)/hardware/mtkcam/aaa/source/common
RELATIVE_PATH_MTKCAM_3A_COMMON  := ../../common
$(info "HALISP MTKCAM_AAA_PLATFORM=$(MTKCAM_AAA_PLATFORM)")
$(info "HALISP MTKCAM_3A_PATH=$(MTKCAM_3A_PATH)")
$(info "HALISP MTKCAM_DRV_INCLUDE=$(MTKCAM_DRV_INCLUDE)")

#-----------------------------------------------------------
LOCAL_CFLAGS += $(MTKCAM_CFLAGS)

#-----------------------------------------------------------
LOCAL_SRC_FILES += HalIspImp.cpp
LOCAL_SRC_FILES += HalIspAttributes.cpp
LOCAL_SRC_FILES += ../../common/isp_hal/IHalISP.cpp
LOCAL_SRC_FILES += ../IspMgrIf.cpp

LOCAL_SRC_FILES += ../isp_tuning/capturenr/capturenr.cpp
LOCAL_SRC_FILES += ../../common/hal3a/$(MTKCAM_COMMON_HAL3A_VERSION)/Hal3ARefBase/debug/IDebugEntry.cpp
LOCAL_SRC_FILES += ../../common/hal3a/$(MTKCAM_COMMON_HAL3A_VERSION)/Hal3ARefBase/debug/DebugUtil.cpp
LOCAL_SRC_FILES += $(RELATIVE_PATH_MTKCAM_3A_COMMON)/iopipe/NormalPipeUtils.cpp
#LOCAL_SRC_FILES += ../../common/hal3a/$(MTKCAM_COMMON_HAL3A_VERSION)/ResultPool/ResultPool.cpp
#LOCAL_SRC_FILES += ../../common/hal3a/$(MTKCAM_COMMON_HAL3A_VERSION)/ResultPool/ResultPoolImpWrapper.cpp
#LOCAL_SRC_FILES += ../../common/hal3a/$(MTKCAM_COMMON_HAL3A_VERSION)/ResultPool/Entry.cpp
#LOCAL_SRC_FILES += ../../common/hal3a/$(MTKCAM_COMMON_HAL3A_VERSION)/ResultPool/ThreadResultPool.cpp

#-----------------------------------------------------------
LOCAL_C_INCLUDES += $(MTKCAM_INCLUDE)
LOCAL_C_INCLUDES += $(MTKCAM_DRV_INCLUDE)
LOCAL_C_INCLUDES += $(MTK_PATH_SOURCE)/hardware/mtkcam/include
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_SOURCE)/hardware/mtkcam/drv/include/$(MTKCAM_DRV_PLATFORM)/drv
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_SOURCE)/hardware/mtkcam/include/mtkcam
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_SOURCE)/hardware/mtkcam/include/mtkcam/aaa
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_SOURCE)/hardware/mtkcam3/include
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_SOURCE)/hardware/mtkcam/aaa/include
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_SOURCE)/hardware/mtkcam/aaa/source/common/isp_hal

Var=$(TOP)/$(MTK_PATH_SOURCE)/hardware/mtkcam/aaa/include

LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_SOURCE)/hardware/mtkcam/aaa/source/common/lsc_mgr
LOCAL_C_INCLUDES += $(MTKCAM_3A_PATH)
LOCAL_C_INCLUDES += $(MTKCAM_3A_PATH)/isp_hal
LOCAL_C_INCLUDES += $(MTKCAM_3A_PATH)/isp_mgr
LOCAL_C_INCLUDES += $(MTKCAM_3A_COMMON_PATH)/af_hal
LOCAL_C_INCLUDES += $(MTKCAM_3A_COMMON_PATH)/af_hal/af_mgr
LOCAL_C_INCLUDES += $(MTKCAM_3A_PATH)/isp_tuning
LOCAL_C_INCLUDES += $(MTKCAM_3A_PATH)/isp_tuning_buf
LOCAL_C_INCLUDES += $(MTKCAM_3A_PATH)/isp_tuning/paramctrl/inc
LOCAL_C_INCLUDES += $(MTKCAM_3A_PATH)/isp_tuning/paramctrl/ccm_mgr
LOCAL_C_INCLUDES += $(MTKCAM_3A_PATH)/isp_tuning/paramctrl/ggm_mgr
LOCAL_C_INCLUDES += $(MTKCAM_3A_PATH)/isp_tuning/paramctrl/gma_mgr
LOCAL_C_INCLUDES += $(MTKCAM_3A_PATH)/isp_tuning/paramctrl/lce_mgr
LOCAL_C_INCLUDES += $(MTKCAM_3A_PATH)/isp_tuning/paramctrl/dce_mgr
LOCAL_C_INCLUDES += $(MTKCAM_3A_PATH)/isp_tuning/paramctrl/idx_cache
#debug/DebugUtil.cpp
LOCAL_C_INCLUDES += $(MTKCAM_3A_COMMON_PATH)/hal3a/$(MTKCAM_COMMON_HAL3A_VERSION)/

LOCAL_C_INCLUDES += $(MTKCAM_3A_COMMON_PATH)/utils/aaa_utils
LOCAL_C_INCLUDES += $(MTKCAM_3A_COMMON_PATH)/hal3a/$(MTKCAM_COMMON_HAL3A_VERSION)/Hal3ARefBase
LOCAL_C_INCLUDES += $(MTKCAM_3A_COMMON_PATH)/hal3a/$(MTKCAM_COMMON_HAL3A_VERSION)/ResultPool

LOCAL_C_INCLUDES += $(MTK_PATH_COMMON)/hal/inc/camera_feature
LOCAL_C_INCLUDES += $(MTK_PATH_CUSTOM_PLATFORM)/hal/inc/isp_tuning
LOCAL_C_INCLUDES += $(MTK_PATH_CUSTOM_PLATFORM)/hal/inc/isp_tuning/$(MTK_CAM_SW_VERSION)
LOCAL_C_INCLUDES += $(MTK_PATH_CUSTOM_PLATFORM)/hal/inc
LOCAL_C_INCLUDES += $(MTK_PATH_CUSTOM_PLATFORM)/hal/inc/aaa
LOCAL_C_INCLUDES += $(MTK_PATH_CUSTOM_PLATFORM)/hal/inc/debug_exif/aaa
LOCAL_C_INCLUDES += $(MTK_PATH_CUSTOM_PLATFORM)/hal/camera_3a


#-----------------------------------------------------------
#for CCU

#For AE
LOCAL_C_INCLUDES += $(MTK_PATH_SOURCE)/custom
LOCAL_C_INCLUDES += $(MTK_PATH_COMMON)/hal/inc/custom/aaa

#algo
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_SOURCE)/hardware/libcamera_3a/libgma_lib/$(TARGET_BOARD_PLATFORM)/include
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_SOURCE)/hardware/libcamera_3a/liblce_lib/$(TARGET_BOARD_PLATFORM)/include
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_SOURCE)/hardware/libcamera_3a/libdce_lib/$(TARGET_BOARD_PLATFORM)/include
#feature
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_SOURCE)/hardware/libcamera_feature/libfdft_lib/include
#capturenr
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_SOURCE)/hardware/perfservice/perfservicenative
LOCAL_C_INCLUDES += $(MTK_PATH_SOURCE)/hardware/libcamera_feature/libnr_lib/include

#for P1 Callback
LOCAL_SRC_FILES += HalISPCbHub.cpp

LOCAL_SHARED_LIBRARIES += lib3a.ccudrv
LOCAL_SHARED_LIBRARIES += lib3a.ccuif

ifeq ($(HAVE_AEE_FEATURE),yes)
    LOCAL_CFLAGS += -DHAVE_AEE_FEATURE
endif

LOCAL_CFLAGS += -DMTKCAM_CCU_AF_SUPPORT
ifeq ($(TARGET_BOARD_PLATFORM), mt6771)
LOCAL_CFLAGS += -DCCU_AF_FEATURE_EN=1
else
LOCAL_CFLAGS += -DCCU_AF_FEATURE_EN=0
endif

#ISP 5.0
LOCAL_CFLAGS += -DCAM3_3A_ISP_30_EN=$(MTKCAM_IS_ISP_30_PLATFORM)
LOCAL_CFLAGS += -DCAM3_3A_ISP_40_EN=$(MTKCAM_IS_ISP_40_PLATFORM)
LOCAL_CFLAGS += -DCAM3_3A_ISP_50_EN=$(MTKCAM_IS_ISP_50_PLATFORM)
LOCAL_CFLAGS += -DCAM3_3A_IP_BASE=$(MTKCAM_IS_IP_BASE)
LOCAL_CFLAGS += -DCAM3_LSC_FEATURE_EN=1

#-----------------------------------------------------------
LOCAL_SHARED_LIBRARIES += liblog
LOCAL_SHARED_LIBRARIES += libutils
LOCAL_SHARED_LIBRARIES += libcutils
LOCAL_SHARED_LIBRARIES += libmtkcam_stdutils libmtkcam_imgbuf
LOCAL_SHARED_LIBRARIES += libmtkcam_sysutils
LOCAL_SHARED_LIBRARIES += libmtkcam_hwutils
LOCAL_SHARED_LIBRARIES += libmtkcam_modulehelper
LOCAL_SHARED_LIBRARIES += libcamdrv_tuning_mgr
LOCAL_SHARED_LIBRARIES += libcam.pdtblgen
ifeq ($(HAVE_AEE_FEATURE),yes)
    LOCAL_SHARED_LIBRARIES += libaedv
endif

#capturenr
LOCAL_SHARED_LIBRARIES += libhidlbase
LOCAL_SHARED_LIBRARIES += libhidltransport
LOCAL_SHARED_LIBRARIES += libhwbinder
LOCAL_SHARED_LIBRARIES += android.hardware.power@1.0
LOCAL_SHARED_LIBRARIES += vendor.mediatek.hardware.power@2.0
LOCAL_SHARED_LIBRARIES += libcamalgo.nr
#LOCAL_SHARED_LIBRARIES += libperfservicenative
LOCAL_SHARED_LIBRARIES += libui
LOCAL_SHARED_LIBRARIES += libmtkcam_metadata
#for CCU
LOCAL_STATIC_LIBRARIES += lib3a.ccu.headers

#libispfeature
LOCAL_STATIC_LIBRARIES += libispfeature_mtkcam
LOCAL_SHARED_LIBRARIES += libcamalgo.ispfeature

LOCAL_STATIC_LIBRARIES += libcam.hal3a_utils
LOCAL_SHARED_LIBRARIES += libcam.hal3a.v3.nvram.50
LOCAL_SHARED_LIBRARIES += libcam.hal3a.v3.lsctbl.50
LOCAL_SHARED_LIBRARIES += libcam.hal3a.v3.lscMgr
LOCAL_SHARED_LIBRARIES += lib3a.gma lib3a.lce lib3a.dce
LOCAL_SHARED_LIBRARIES += libcam.hal3a.v3.resultpool

LOCAL_SHARED_LIBRARIES += libmtkcam_tuning_utils
LOCAL_SHARED_LIBRARIES += libmtkcam_mapping_mgr
LOCAL_SHARED_LIBRARIES += libcam.halisp.buf
LOCAL_SHARED_LIBRARIES += libcam.halisp.common
#For AE
LOCAL_SHARED_LIBRARIES += lib3a.ae.core
ifeq ($(MTK_CAM_NEW_NVRAM_SUPPORT),1)
    LOCAL_SHARED_LIBRARIES += libmtkcam_mapping_mgr
endif
LOCAL_SHARED_LIBRARIES += libcam.isptuning

#ifneq ($(BUILD_MTK_LDVT),yes)
   #LOCAL_SHARED_LIBRARIES += lib3a
   #LOCAL_SHARED_LIBRARIES += lib3a_core1
   #LOCAL_SHARED_LIBRARIES += lib3a_sample
    LOCAL_SHARED_LIBRARIES += libcameracustom
   LOCAL_SHARED_LIBRARIES += lib3a.af.core
   #LOCAL_SHARED_LIBRARIES += lib3a.af
   #LOCAL_SHARED_LIBRARIES += lib3a.awb
   #LOCAL_SHARED_LIBRARIES += lib3a.awb.core
   #LOCAL_SHARED_LIBRARIES += lib3a.flash
   #LOCAL_SHARED_LIBRARIES += lib3a.ae
   #LOCAL_SHARED_LIBRARIES += lib3a.ae.core
#endif
#-----------------------------------------------------------
#LOCAL_STATIC_LIBRARIES :=

#LOCAL_WHOLE_STATIC_LIBRARIES :=

#-----------------------------------------------------------
LOCAL_MODULE := libcam.halisp
LOCAL_MODULE_CLASS := SHARED_LIBRARIES
LOCAL_PROPRIETARY_MODULE := true
LOCAL_MODULE_OWNER := mtk

ifeq ($(BUILD_MTK_LDVT),yes)
    LOCAL_CFLAGS += -DUSING_MTK_LDVT
    LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_SOURCE)/hardware/ldvt/$(TARGET_BOARD_PLATFORM)/include
    LOCAL_C_INCLUDES += $(TOP)/$(MTKCAM_INCLUDE)
    LOCAL_WHOLE_STATIC_LIBRARIES += libuvvf
endif
#-----------------------------------------------------------

#-----------------------------------------------------------
LOCAL_INTERMEDIATES         := $(call local-intermediates-dir)
LOCAL_GEN_TOOL              := $(MTKCAM_3A_PATH)/isp_tuning/enum2str
ifeq ($(MTK_CAM_NEW_NVRAM_SUPPORT),1)
    LOCAL_ISP_PROFILE_HEADER    := $(MTK_PATH_CUSTOM_PLATFORM)/hal/inc/tuning_mapping/cam_idx_struct_ext.h
else
    LOCAL_ISP_PROFILE_HEADER    := $(MTK_PATH_CUSTOM_PLATFORM)/hal/inc/isp_tuning/isp_tuning.h
endif
LOCAL_ISP_PROFILE_STR_ARRAY := $(LOCAL_INTERMEDIATES)/EIspProfile_string.h

$(LOCAL_ISP_PROFILE_STR_ARRAY): $(LOCAL_ISP_PROFILE_HEADER) $(LOCAL_GEN_TOOL)
	$(LOCAL_GEN_TOOL) $(LOCAL_ISP_PROFILE_HEADER) EIspProfile_ strEIspProfile > $@ || { rm $@; exit -1; }

LOCAL_GENERATED_SOURCES += $(LOCAL_ISP_PROFILE_STR_ARRAY)
LOCAL_C_INCLUDES += $(LOCAL_INTERMEDIATES)

#-----------------------------------------------------------
ifeq ($(MTK_CAM_NEW_NVRAM_SUPPORT),1)
#LOCAL_INTERMEDIATES         := $(call local-intermediates-dir)
#LOCAL_GEN_TOOL              := $(LOCAL_PATH)/enum2str
LOCAL_MODULE_STR_ARRAY := $(LOCAL_INTERMEDIATES)/EModule_string.h
LOCAL_APP_STR_ARRAY := $(LOCAL_INTERMEDIATES)/EApp_string.h
#LOCAL_ISP_PROFILE_STR_ARRAY := $(LOCAL_INTERMEDIATES)/EIspProfile_string.h
LOCAL_SENSOR_MODE_STR_ARRAY := $(LOCAL_INTERMEDIATES)/ESensorMode_string.h

LOCAL_MODULE_HEADER    := $(MTK_PATH_CUSTOM_PLATFORM)/hal/inc/tuning_mapping/cam_idx_struct_ext.h

$(LOCAL_MODULE_STR_ARRAY): $(LOCAL_MODULE_HEADER) $(LOCAL_GEN_TOOL)
	$(LOCAL_GEN_TOOL) $(LOCAL_MODULE_HEADER) EModule_ strEModule > $@ || { rm $@; exit -1; }
$(LOCAL_APP_STR_ARRAY): $(LOCAL_MODULE_HEADER) $(LOCAL_GEN_TOOL)
	$(LOCAL_GEN_TOOL) $(LOCAL_MODULE_HEADER) EApp_ strEApp > $@ || { rm $@; exit -1; }
#$(LOCAL_ISP_PROFILE_STR_ARRAY): $(LOCAL_MODULE_HEADER) $(LOCAL_GEN_TOOL)
#	$(LOCAL_GEN_TOOL) $(LOCAL_MODULE_HEADER) EIspProfile_ strEIspProfile > $@ || { rm $@; exit -1; }
$(LOCAL_SENSOR_MODE_STR_ARRAY): $(LOCAL_MODULE_HEADER) $(LOCAL_GEN_TOOL)
	$(LOCAL_GEN_TOOL) $(LOCAL_MODULE_HEADER) ESensorMode_ strESensorMode > $@ || { rm $@; exit -1; }

LOCAL_GENERATED_SOURCES += $(LOCAL_MODULE_STR_ARRAY)
LOCAL_GENERATED_SOURCES += $(LOCAL_APP_STR_ARRAY)
#LOCAL_GENERATED_SOURCES += $(LOCAL_ISP_PROFILE_STR_ARRAY)
LOCAL_GENERATED_SOURCES += $(LOCAL_SENSOR_MODE_STR_ARRAY)
#LOCAL_C_INCLUDES += $(LOCAL_INTERMEDIATES)
endif
#-----------------------------------------------------------

include $(MTK_SHARED_LIBRARY)

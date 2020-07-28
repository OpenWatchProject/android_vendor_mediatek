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

ifeq ($(MTK_TC1_FEATURE),yes)
    LOCAL_CFLAGS += -DMTK_TC1_FEATURE
    LOCAL_CPPFLAGS += -DMTK_TC1_FEATURE
    LOCAL_CFLAGS += -DMTK_A3MAC
    LOCAL_CFLAGS += -DMTK_MAC_EBORTS2
    LOCAL_CPPFLAGS += -DMTK_MAC_EBORTS2
endif



#-----------------------------------------------------------
sinclude $(TOP)/$(MTK_PATH_SOURCE)/hardware/mtkcam/mtkcam.mk

#-----------------------------------------------------------
#LOCAL_SRC_FILES += $(subst $(LOCAL_PATH)/,,$(sort $(wildcard $(LOCAL_PATH)/./*.cpp)))
LOCAL_SRC_FILES += \
  Hal3AAdapter1.cpp \
  IHal3A.cpp \
  aaa_hal_if.cpp \
  aaa_hal.cpp \
  aaa_hal.thread.cpp \
  aaa_scheduler.cpp \
  aaa_hal_yuv.cpp \
  aaa_hal_yuv.thread.cpp \
  ResultBufMgr/ResultBufMgr.cpp \
  state_mgr/aaa_state.cpp \
  state_mgr/aaa_state_camera_preview.cpp \
  state_mgr/aaa_state_camcorder_preview.cpp \
  state_mgr/aaa_state_recording.cpp \
  state_mgr/aaa_state_af.cpp \
  state_mgr/aaa_state_precapture.cpp \
  state_mgr/aaa_state_capture.cpp \
  nvram_mgr/nvram_drv_mgr.cpp \
  state_mgr/aaa_state_mgr.cpp \
  awb_mgr/awb_mgr_if.cpp \
  awb_mgr/awb_mgr.cpp \
  awb_mgr/awb_cct_feature.cpp \
  awb_mgr/awb_state.cpp \
  ae_mgr/ae_mgr.cpp \
  ae_mgr/ae_mgr_pline.cpp \
  ae_mgr/ae_cct_feature.cpp \
  ae_mgr/ae_mgr_if.cpp \
  af_mgr/af_mgr.cpp \
  af_mgr/af_mgr_if.cpp \
  af_mgr/af_cct_feature.cpp \
  af_mgr/af_platform_based.cpp \
  aaa_util/LogCollector.cpp \
  pd_mgr/pd_mgr.cpp \
  pd_mgr/pd_mgr_if.cpp \
  isp_mgr/isp_mgr.cpp \
  isp_mgr/isp_mgr_ctl.cpp \
  isp_mgr/isp_mgr_dbs.cpp \
  isp_mgr/isp_mgr_obc.cpp \
  isp_mgr/isp_mgr_bnr.cpp \
  isp_mgr/isp_mgr_lsc.cpp \
  isp_mgr/isp_mgr_rpg.cpp \
  isp_mgr/isp_mgr_pgn.cpp \
  isp_mgr/isp_mgr_nsl2.cpp \
  isp_mgr/isp_mgr_udm.cpp \
  isp_mgr/isp_mgr_ccm.cpp \
  isp_mgr/isp_mgr_ggm.cpp \
  isp_mgr/isp_mgr_g2c.cpp \
  isp_mgr/isp_mgr_nbc.cpp \
  isp_mgr/isp_mgr_nbc2.cpp \
  isp_mgr/isp_mgr_pca.cpp \
  isp_mgr/isp_mgr_seee.cpp \
  isp_mgr/isp_mgr_nr3d.cpp \
  isp_mgr/isp_mgr_mfb.cpp \
  isp_mgr/isp_mgr_mixer3.cpp \
  isp_mgr/isp_mgr_lce.cpp \
  isp_mgr/isp_mgr_rmg.cpp \
  isp_mgr/isp_mgr_awb_stat.cpp \
  isp_mgr/isp_mgr_ae_stat.cpp \
  isp_mgr/isp_mgr_af_stat.cpp \
  isp_mgr/isp_mgr_flk.cpp \
  isp_mgr/isp_mgr_helper.cpp \
  isp_mgr/isp_debug.cpp \
  buf_mgr/aao_buf_mgr.cpp \
  buf_mgr/afo_buf_mgr.cpp \
  ispdrv_mgr/ispdrv_mgr.cpp \
  isp_tuning/isp_tuning_mgr.cpp \
  isp_tuning/paramctrl/paramctrl_lifetime.cpp \
  isp_tuning/paramctrl/paramctrl_user.cpp \
  isp_tuning/paramctrl/paramctrl_attributes.cpp \
  isp_tuning/paramctrl/paramctrl_validate.cpp \
  isp_tuning/paramctrl/paramctrl_per_frame.cpp \
  isp_tuning/paramctrl/paramctrl_frameless.cpp \
  isp_tuning/paramctrl/paramctrl_exif.cpp \
  sensor_mgr/aaa_sensor_mgr.cpp \
  sensor_mgr/aaa_sensor_buf_mgr.cpp \
  isp_tuning/paramctrl/pca_mgr/pca_mgr.cpp \
  isp_tuning/paramctrl/ccm_mgr/ccm_mgr.cpp \
  isp_tuning/paramctrl/gma_mgr/gma_mgr.cpp \
  lsc_mgr/lsc_mgr2.cpp \
  lsc_mgr/lsc_mgr2.thread.cpp \
  lsc_mgr/lsc_mgr2_rto.cpp\
  lsc_mgr/lsc_mgr2_rto.thread.cpp\
  lsc_mgr/lsc_mgr2.misc.cpp \
  flash_mgr/flash_mgr.cpp \
  flash_mgr/flash_mgr_m.cpp \
  flash_mgr/flash_util.cpp \
  flash_mgr/flash_pline_tool.cpp \
  flash_mgr/flash_cct.cpp \
  flash_mgr/flash_cct_quick.cpp \
  flash_mgr/flash_cct_quick2.cpp \
  state_mgr_af/af_state_mgr.cpp \
  state_mgr_af/af_state.cpp \
  state_mgr_af/af_state_caf.cpp \
  state_mgr_af/af_state_taf.cpp \
  debug/IDebugEntry.cpp \
  debug/DebugUtil.cpp \
  laser_mgr/laser_mgr_if.cpp \
  laser_mgr/laser_drv.cpp \




LOCAL_SRC_FILES += \
  Sync3A.cpp \
  state_mgr_n3d/aaa_state_n3d.cpp \
  state_mgr_n3d/aaa_state_camera_preview_n3d.cpp \
  state_mgr_n3d/aaa_state_capture_n3d.cpp \
  state_mgr_n3d/aaa_state_precapture_n3d.cpp \
  state_mgr_n3d/aaa_state_af_n3d.cpp

$(warning $(MTK_PATH_COMMON))
#-----------------------------------------------------------
LOCAL_C_INCLUDES:= \
    $(TOP)/external/stlport/stlport \
    $(TOP)/$(MTK_PATH_COMMON)/hal/inc  \
    $(MTK_PATH_COMMON)/kernel/imgsensor/inc \
    $(MTK_PATH_CUSTOM_PLATFORM_HAL1)/hal/inc \
    $(MTK_PATH_CUSTOM_PLATFORM_HAL1)/hal/inc/aaa \
    $(MTK_PATH_CUSTOM)/hal/inc \
    $(MTK_PATH_CUSTOM)/hal/inc/isp_tuning \
    $(MTK_PATH_CUSTOM_PLATFORM_HAL1)/hal/inc/isp_tuning \
    $(MTK_PATH_COMMON)/hal/inc/camera_feature \
    $(MTK_PATH_COMMON)/ \
    $(MTK_PATH_CUSTOM)/hal/inc/debug_exif/aaa \
    $(MTK_PATH_CUSTOM)/hal/inc/debug_exif/cam \
    $(MTK_PATH_CUSTOM)/hal/inc/debug_exif/aaa \
    $(MTK_PATH_CUSTOM_PLATFORM_HAL1)/hal/inc/pd_buf_mgr \
    $(MTK_PATH_CUSTOM_PLATFORM_HAL1)/hal/inc/debug_exif/aaa \
    $(MTK_MTKCAM_PLATFORM)/core/featureio/drv/inc \
    $(MTK_MTKCAM_PLATFORM)/core/featureio/drv/cam_cal \
    $(MTK_MTKCAM_PLATFORM)/core/featureio/pipe/aaa/ \
    $(MTK_MTKCAM_PLATFORM)/core/featureio/pipe/aaa/awb_mgr \
    $(MTK_MTKCAM_PLATFORM)/core/featureio/pipe/aaa/nvram_mgr \
    $(MTK_MTKCAM_PLATFORM)/core/featureio/pipe/aaa/isp_mgr \
    $(MTK_MTKCAM_PLATFORM)/core/featureio/pipe/aaa/buf_mgr \
    $(MTK_MTKCAM_PLATFORM)/core/featureio/pipe/aaa/ispdrv_mgr \
    $(MTK_MTKCAM_PLATFORM)/core/featureio/pipe/aaa/isp_tuning \
    $(MTK_MTKCAM_PLATFORM)/core/featureio/pipe/aaa/isp_tuning/paramctrl/inc \
    $(MTK_MTKCAM_PLATFORM)/core/featureio/pipe/aaa/isp_tuning/paramctrl/pca_mgr/ \
    $(MTK_MTKCAM_PLATFORM)/core/featureio/pipe/aaa/isp_tuning/paramctrl/ccm_mgr/ \
    $(MTK_MTKCAM_PLATFORM)/core/featureio/pipe/aaa/isp_tuning/paramctrl/gma_mgr/ \
    $(MTK_MTKCAM_PLATFORM)/core/featureio/pipe/aaa/lsc_mgr \
    $(MTK_MTKCAM_PLATFORM)/core/featureio/pipe/aaa/ae_mgr \
    $(MTK_MTKCAM_PLATFORM)/core/featureio/pipe/aaa/flash_mgr \
    $(MTK_MTKCAM_PLATFORM)/core/featureio/pipe/aaa/af_mgr \
	$(MTK_MTKCAM_PLATFORM)/core/featureio/pipe/aaa/aaa_util \
    $(MTK_MTKCAM_PLATFORM)/core/featureio/pipe/aaa/pd_mgr \
    $(MTK_MTKCAM_PLATFORM)/core/featureio/pipe/aaa/laser_mgr \
    $(MTK_MTKCAM_PLATFORM)/core/featureio/pipe/aaa/sensor_mgr \
    $(MTK_MTKCAM_PLATFORM)/core/featureio/pipe/aaa/ResultBufMgr \
    $(MTK_MTKCAM_PLATFORM)/core/featureio/pipe/aaa/state_mgr \
    $(MTK_MTKCAM_PLATFORM)/core/featureio/pipe/aaa/state_mgr_af \
    $(MTK_MTKCAM_PLATFORM)/include/mtkcam/algorithm/lib3a \
    $(MTK_MTKCAM_PLATFORM)/include/mtkcam/algorithm/liblsctrans \
    $(MTK_MTKCAM_PLATFORM)/include/mtkcam/algorithm/libtsf \
    $(MTK_MTKCAM_PLATFORM)/include/mtkcam/algorithm/libgma \
    $(MTK_MTKCAM_PLATFORM)/include/mtkcam/algorithm/libsync3a \
    $(TOP)/$(MTK_MTKCAM_PLATFORM)/ \
    $(TOP)/$(MTK_PATH_PLATFORM)/hardware \
    $(MTK_MTKCAM_PLATFORM)/core/drv/imgsensor \
    $(MTK_MTKCAM_PLATFORM)/core/imageio/inc \
    $(MTK_MTKCAM_PLATFORM)/include/mtkcam/camnode \
    $(MTK_MTKCAM_PLATFORM)/include

LOCAL_C_INCLUDES += $(MTKCAM_C_INCLUDES)
LOCAL_C_INCLUDES += $(TOP)/system/core/base/include
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_SOURCE)/hardware/gralloc_extra/include
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_SOURCE)/hardware/mtkcam/ext/include
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_SOURCE)/hardware/mtkcam/include
LOCAL_C_INCLUDES += $(TOP)/frameworks/hardware/interfaces/sensorservice/libsensorndkbridge
LOCAL_C_INCLUDES += $(TOP)/hardware/interfaces/sensors/1.0/default/include

LOCAL_C_INCLUDES +=$(TOP)/$(MTK_MTKCAM_PLATFORM)/include
LOCAL_C_INCLUDES +=$(TOP)/$(MTKCAM_C_INCLUDES)
LOCAL_C_INCLUDES +=$(TOP)/$(MTKCAM_C_INCLUDES)/..

LOCAL_C_INCLUDES += $(MTK_PATH_CUSTOM_PLATFORM_HAL1)/hal/camera_3a
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_SOURCE)/hardware/camera/inc
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_SOURCE)/hardware/camera/inc/featureio
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_SOURCE)/hardware/camera/inc/drv
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_SOURCE)/hardware/camera/inc/acdk
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_SOURCE)/hardware/camera/inc/common
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_SOURCE)/hardware/camera/inc/common/camexif

LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_SOURCE)/external/aee/binary/inc
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_SOURCE)/external
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_PLATFORM)/hardware/camera/inc/common/camutils
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_PLATFORM)/kernel/core/include/mach
LOCAL_C_INCLUDES += $(TOP)/bionic
#
LOCAL_C_INCLUDES += $(TOP)/$(MTKCAM_C_INCLUDES)
LOCAL_C_INCLUDES += $(TOP)/$(MTK_MTKCAM_PLATFORM)/include/mtkcam/featureio
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_SOURCE)/hardware/libcamera_3a/libaf_core_lib/$(MTK_PLATFORM_DIR)/include

LOCAL_HEADER_LIBRARIES := libcutils_headers libutils_headers libsystem_headers libhardware_headers libandroid_sensor_headers

# For stereo_hal.h
LOCAL_C_INCLUDES += \
    $(TOP)/$(MTK_ROOT)/frameworks/av/include \
    $(TOP)/$(MTK_MTKCAM_PLATFORM)/include/mtkcam/algorithm/libcore \
    $(TOP)/$(MTK_MTKCAM_PLATFORM)/include/mtkcam/algorithm/libwarp \
    $(TOP)/$(MTK_MTKCAM_PLATFORM)/include/mtkcam/algorithm/libutility \
    $(TOP)/$(MTK_MTKCAM_PLATFORM)/include/mtkcam/algorithm/libstereocam \
    $(TOP)/$(MTK_MTKCAM_PLATFORM)/core/featureio/pipe/stereo
LOCAL_C_INCLUDES += \
  $(MTK_PATH_SOURCE)/kernel/include \
#-----------------------------------------------------------
#For hwsync_drv.h
LOCAL_C_INCLUDES += \
    $(TOP)/$(MTK_MTKCAM_PLATFORM)/hal/sensor
#-----------------------------------------------------------

LOCAL_CFLAGS += $(MTKCAM_CFLAGS)
#

ifeq ($(HAVE_AEE_FEATURE),yes)
    LOCAL_CFLAGS += -DHAVE_AEE_FEATURE
endif

#====== FLASHLIGHT_OFFSET_CALI ======
ifeq ($(MTK_CAM_FLASHLIGHT_OFFSET_CALI_SUPPORT),yes)
    LOCAL_CFLAGS += -DFLASHLIGHT_OFFSET_CALI
endif

#-----------------------------------------------------------
LOCAL_WHOLE_STATIC_LIBRARIES += libfeatureio.aaa.custom
#
LOCAL_STATIC_LIBRARIES += libispfeature_mtkcam

#-----------------------------------------------------------
LOCAL_SHARED_LIBRARIES += liblog
LOCAL_SHARED_LIBRARIES += libutils
LOCAL_SHARED_LIBRARIES += libcutils
#
#
LOCAL_SHARED_LIBRARIES += libmtkcam_stdutils libcam_utils libmtkcam_imgbuf
LOCAL_SHARED_LIBRARIES += android.frameworks.sensorservice@1.0

ifneq ($(MTK_BASIC_PACKAGE), yes)
LOCAL_STATIC_LIBRARIES += libsched
endif
#-----------------------------------------------------------
LOCAL_MODULE := libfeatureiopipe_aaa
LOCAL_PROPRIETARY_MODULE := true
LOCAL_MODULE_OWNER := mtk
#-----------------------------------------------------------
include $(MTK_STATIC_LIBRARY)


################################################################################
#
################################################################################
include $(CLEAR_VARS)
include $(call all-makefiles-under,$(LOCAL_PATH))


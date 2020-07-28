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


#
# libfeatureiodrv_comm
#
LOCAL_PATH:= $(call my-dir)

include $(CLEAR_VARS)

ifeq ($(BUILD_MTK_LDVT),true)
    LOCAL_CFLAGS += -DUSING_MTK_LDVT
endif

LOCAL_SRC_FILES:= \
  tdri_mgr/tdri_mgr.cpp \
  tuning_mgr/tuning_mgr.cpp \
  cam_cal/cam_cal_drv.cpp \
  cam_cal/camera_common_calibration.cpp \

LOCAL_SRC_FILES += nvram/nvram_drv.cpp
LOCAL_SRC_FILES += nvram/nvbuf_util.cpp


LOCAL_SRC_FILES += strobe/strobe_drv.cpp
LOCAL_SRC_FILES += strobe/strobe_global_driver.cpp
LOCAL_SRC_FILES += strobe/flashlight_drv.cpp


ifneq ($(BUILD_MTK_LDVT),true)
    LOCAL_SRC_FILES += \
        eis/eis_drv.cpp \
        eis/eis_drv_r.cpp
endif



LOCAL_SRC_FILES +=  lens/mcu_drv.cpp
  LOCAL_SRC_FILES += lens/lens_drv.cpp
  LOCAL_SRC_FILES += lens/gaflens_drv.cpp
  LOCAL_SRC_FILES += lens/lens_sensor_drv.cpp



LOCAL_C_INCLUDES:= \
  $(MTK_PATH_COMMON)/kernel/imgsensor/inc \
  $(MTK_PATH_SOURCE)/external/nvram/libnvram \
  $(MTK_PATH_CUSTOM)/hal/inc/aaa \
  $(MTK_PATH_CUSTOM)/hal/inc \
  $(MTK_PATH_CUSTOM_PLATFORM)/hal/inc \
  $(MTK_PATH_CUSTOM)/hal/camera \
  $(MTK_PATH_CUSTOM)/kernel/lens/inc \
  $(TOP)/device/mediatek/common/kernel-headers \
  $(MTK_PATH_CUSTOM_PLATFORM)/kernel/lens/inc \
  $(MTK_PATH_CUSTOM_PLATFORM)/hal/inc/aaa \
  $(TOP)/$(MTK_MTKCAM_PLATFORM)/core/featureio/drv/inc \
  $(TOP)/$(MTK_MTKCAM_PLATFORM)/core/drv/imem \
  $(TOP)/$(MTK_MTKCAM_PLATFORM)/core/imageio_common/inc \
  $(TOP)/$(MTK_MTKCAM_PLATFORM)/inc/common \
  $(TOP)/mediatek/hardware/mtkcam/inc/drv\
  $(TOP)/$(MTK_MTKCAM_PLATFORM)/inc/drv \
  $(TOP)/$(MTK_MTKCAM_PLATFORM)/inc/featureio \
  $(TOP)/$(MTK_PATH_PLATFORM)/external/ldvt/include \
  $(TOP)/$(MTK_PATH_SOURCE)/hardware/m4u/$(PLATFORM) \
  $(TOP)/$(MTK_MTKCAM_PLATFORM)/include \
  $(TOP)/$(MTK_MTKCAM_PLATFORM)/include/mtkcam/featureio \
  $(TOP)/$(MTK_MTKCAM_PLATFORM)/inc/common/camutils \
  $(TOP)/$(MTK_PATH_CUSTOM)/cgen/inc \
  $(TOP)/$(MTK_PATH_CUSTOM)/cgen/cfgfileinc

#-----------------------------------------------------------
ifeq ($(TARGET_BUILD_VARIANT), user)
MTKCAM_LOGENABLE_DEFAULT   := 0
else
MTKCAM_LOGENABLE_DEFAULT   := 1
endif
LOCAL_CFLAGS += -DMTKCAM_LOGENABLE_DEFAULT=$(MTKCAM_LOGENABLE_DEFAULT)
#-----------------------------------------------------------

LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_SOURCE)/hardware/mtkcam/inc/drv

#endif

LOCAL_STATIC_LIBRARIES := \

LOCAL_WHOLE_STATIC_LIBRARIES := \

LOCAL_MODULE:= libfeatureiodrv
LOCAL_PROPRIETARY_MODULE := true
LOCAL_MODULE_OWNER := mtk
LOCAL_MULTILIB := 32

#
# Start of common part ------------------------------------
sinclude $(TOP)/$(MTK_PATH_SOURCE)/hardware/mtkcam/mtkcam.mk

#-----------------------------------------------------------
LOCAL_CFLAGS += $(MTKCAM_CFLAGS)

#-----------------------------------------------------------
LOCAL_HEADER_LIBRARIES += libmtkcam_headers
LOCAL_C_INCLUDES += $(MTKCAM_C_INCLUDES)
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_SOURCE)/hardware/gralloc_extra/include
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_SOURCE)/hardware/mtkcam/ext/include


#-----------------------------------------------------------
LOCAL_C_INCLUDES += $(TOP)/$(MTKCAM_C_INCLUDES)/..
LOCAL_C_INCLUDES += $(TOP)/$(MTKCAM_C_INCLUDES)
LOCAL_C_INCLUDES += $(TOP)/$(MTK_MTKCAM_PLATFORM)/include
#LOCAL_SHARED_LIBRARIES += libcam.utils.sensorlistener
#-----------------------------------------------------------
LOCAL_SHARED_LIBRARIES += liblog
LOCAL_SHARED_LIBRARIES += libcutils
#
LOCAL_SHARED_LIBRARIES += libcamdrv
LOCAL_SHARED_LIBRARIES += libcam.halsensor
LOCAL_SHARED_LIBRARIES += libcam.iopipe_FrmB
LOCAL_SHARED_LIBRARIES += libmtkcam_stdutils libcam_utils
LOCAL_SHARED_LIBRARIES += libcam.utils
LOCAL_SHARED_LIBRARIES += libcamdrv_FrmB
LOCAL_SHARED_LIBRARIES += libnvramagentclient
#
#ifeq ($(HAVE_AEE_FEATURE),yes)
#    LOCAL_SHARED_LIBRARIES += libaedv
#endif
#
ifneq ($(BUILD_MTK_LDVT),true)
#    LOCAL_SHARED_LIBRARIES += lib3a
    LOCAL_SHARED_LIBRARIES += libnvram
    LOCAL_SHARED_LIBRARIES += libcamdrv

#    LOCAL_SHARED_LIBRARIES += libcam.exif
    LOCAL_SHARED_LIBRARIES += libcameracustom
#    LOCAL_SHARED_LIBRARIES += libcam_mmp
#    LOCAL_SHARED_LIBRARIES += libdpframework
endif

# End of common part ---------------------------------------
#
#-----------------------------------------------------------
LOCAL_MODULE := libfeatureiodrv
LOCAL_PROPRIETARY_MODULE := true
LOCAL_MODULE_OWNER := mtk
#-----------------------------------------------------------
include $(MTK_SHARED_LIBRARY)



include $(call all-makefiles-under,$(LOCAL_PATH))

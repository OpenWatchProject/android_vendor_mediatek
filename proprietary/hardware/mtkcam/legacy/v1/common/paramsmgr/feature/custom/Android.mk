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

LOCAL_PATH := $(call my-dir)

################################################################################
#
################################################################################
include $(CLEAR_VARS)
-include $(TOP)/$(MTK_PATH_SOURCE)/hardware/mtkcam/mtkcam.mk
#sinclude $(TOP)/$(MTK_MTKCAM_PLATFORM)/mtkcam.mk

#-----------------------------------------------------------
#For detecting gmo projects
$(info "MTK_GMO_RAM_OPTIMIZE = $(MTK_GMO_RAM_OPTIMIZE)")
ifeq ($(findstring yes, $(MTK_GMO_RAM_OPTIMIZE)), yes)
    LOCAL_CPPFLAGS += -DGMO_PROJECT
endif

#For detecting D2 projects
ifeq ($(findstring 35m, $(MTK_TARGET_PROJECT)), 35m)
    LOCAL_CPPFLAGS += -DD2_PROJECT
endif

#For detecting the blow projects
ifeq ($(MTK_TARGET_PROJECT),tb6735p1_wsvga)
    LOCAL_CPPFLAGS += -DPROJECT_TB6735P1_WSVGA
endif

ifeq ($(MTK_TARGET_PROJECT),tb6735p1_lttg)
    LOCAL_CPPFLAGS += -DPROJECT_TB6735P1_WSVGA_LTTG
endif

ifeq ($(MTK_TARGET_PROJECT),tb6735p1_wsvga_om_svlte)
    LOCAL_CPPFLAGS += -DPROJECT_TB6735P1_WSVGA_OM_SVLTE
endif

ifeq ($(MTK_TARGET_PROJECT),tb6735p1_wsvga_wifi)
    LOCAL_CPPFLAGS += -DPROJECT_TB6735P1_WSVGA_WIFI
endif

ifeq ($(MTK_TARGET_PROJECT),tb6735p1_wsvga_worldphone)
    LOCAL_CPPFLAGS += -DPROJECT_TB6735P1_WSVGA_WORLDPHONE
endif

#-----------------------------------------------------------
MY_CUST_FTABLE_PATH := $(LOCAL_PATH)/config/matv
#
#ifeq ($(wildcard $(MTK_PATH_CUSTOM)/hal/imgsensor),)
#MY_CUST_FTABLE_PATH += $(MTK_PATH_CUSTOM_PLATFORM)/hal/imgsensor
#MY_CUST_FTABLE_PATH += $(MTK_PATH_CUSTOM_PLATFORM)/hal/imgsensor_src
#else
#MY_CUST_FTABLE_PATH += $(MTK_PATH_CUSTOM)/hal/imgsensor
#MY_CUST_FTABLE_PATH += $(MTK_PATH_CUSTOM)/hal/imgsensor_src
#endif
#
#For flavor project to read its own feature table
ifneq ($(wildcard vendor/mediatek/proprietary/custom/$(strip $(MTK_TARGET_PROJECT))/hal/sendepfeature),)
MY_CUST_FTABLE_PATH += vendor/mediatek/proprietary/custom/$(strip $(MTK_TARGET_PROJECT))/hal/sendepfeature
else
ifneq ($(wildcard $(MTK_PATH_CUSTOM)/hal/sendepfeature),)
MY_CUST_FTABLE_PATH += $(MTK_PATH_CUSTOM)/hal/sendepfeature
else
ifeq ($(strip $(TARGET_BOARD_PLATFORM)),mt6739)
ifneq ($(wildcard $(MTK_PATH_CUSTOM_PLATFORM_HAL1)/hal/sendepfeature),)
MY_CUST_FTABLE_PATH += $(MTK_PATH_CUSTOM_PLATFORM_HAL1)/hal/sendepfeature
endif
else
ifneq ($(wildcard $(MTK_PATH_CUSTOM_PLATFORM)/hal/sendepfeature),)
MY_CUST_FTABLE_PATH += $(MTK_PATH_CUSTOM_PLATFORM)/hal/sendepfeature
endif
endif
endif
#
ifneq (,$(filter $(strip $(TARGET_BOARD_PLATFORM)), mt6735 mt6737t))
ifeq ($(wildcard $(MTK_PATH_CUSTOM)/hal/sendepfeature),)
MY_CUST_FTABLE_PATH += $(MTK_PATH_CUSTOM_PLATFORM)/hal/D1/sendepfeature
else
MY_CUST_FTABLE_PATH += $(MTK_PATH_CUSTOM)/hal/sendepfeature
endif
endif

ifeq ($(strip $(TARGET_BOARD_PLATFORM)),mt6753)
ifeq ($(wildcard $(MTK_PATH_CUSTOM)/hal/sendepfeature),)
MY_CUST_FTABLE_PATH += $(MTK_PATH_CUSTOM_PLATFORM)/hal/D1/sendepfeature
else
MY_CUST_FTABLE_PATH += $(MTK_PATH_CUSTOM)/hal/sendepfeature
endif
endif
#
ifneq (,$(filter $(strip $(TARGET_BOARD_PLATFORM)), mt6735m mt6737m))
ifeq ($(wildcard $(MTK_PATH_CUSTOM)/hal/sendepfeature),)
MY_CUST_FTABLE_PATH += $(MTK_PATH_CUSTOM_PLATFORM)/hal/D2/sendepfeature
else
MY_CUST_FTABLE_PATH += $(MTK_PATH_CUSTOM)/hal/sendepfeature
endif
endif

ifeq ($(strip $(TARGET_BOARD_PLATFORM)),mt8173)
ifeq ($(wildcard $(MTK_PATH_CUSTOM)/hal/sendepfeature),)
MY_CUST_FTABLE_PATH += $(MTK_PATH_CUSTOM_PLATFORM)/hal/sendepfeature
else
MY_CUST_FTABLE_PATH += $(MTK_PATH_CUSTOM)/hal/sendepfeature
endif
endif

endif
#
ifneq ($(wildcard $(MTK_PATH_CUSTOM)/hal/flashlight),)
MY_CUST_FTABLE_PATH += $(MTK_PATH_CUSTOM)/hal/flashlight
else
ifeq ($(strip $(TARGET_BOARD_PLATFORM)),mt6739)
ifneq ($(wildcard $(MTK_PATH_CUSTOM_PLATFORM_HAL1)/hal/flashlight),)
MY_CUST_FTABLE_PATH += $(MTK_PATH_CUSTOM_PLATFORM_HAL1)/hal/flashlight
endif
else
ifneq ($(wildcard $(MTK_PATH_CUSTOM_PLATFORM)/hal/flashlight),)
MY_CUST_FTABLE_PATH += $(MTK_PATH_CUSTOM_PLATFORM)/hal/flashlight
endif
endif
endif
#
ifneq (,$(filter $(strip $(TARGET_BOARD_PLATFORM)), mt6735 mt6737t))
ifeq ($(wildcard $(MTK_PATH_CUSTOM)/hal/flashlight),)
MY_CUST_FTABLE_PATH += $(MTK_PATH_CUSTOM_PLATFORM)/hal/D1/flashlight
else
MY_CUST_FTABLE_PATH += $(MTK_PATH_CUSTOM)/hal/flashlight
endif
endif
#
ifeq ($(strip $(TARGET_BOARD_PLATFORM)),mt6753)
ifeq ($(wildcard $(MTK_PATH_CUSTOM)/hal/flashlight),)
MY_CUST_FTABLE_PATH += $(MTK_PATH_CUSTOM_PLATFORM)/hal/D1/flashlight
else
MY_CUST_FTABLE_PATH += $(MTK_PATH_CUSTOM)/hal/flashlight
endif
endif
#
ifneq (,$(filter $(strip $(TARGET_BOARD_PLATFORM)), mt6735m mt6737m))
ifeq ($(wildcard $(MTK_PATH_CUSTOM)/hal/flashlight),)
MY_CUST_FTABLE_PATH += $(MTK_PATH_CUSTOM_PLATFORM)/hal/D2/flashlight
else
MY_CUST_FTABLE_PATH += $(MTK_PATH_CUSTOM)/hal/flashlight
endif
endif
#
ifeq ($(strip $(TARGET_BOARD_PLATFORM)),mt8173)
ifeq ($(wildcard $(MTK_PATH_CUSTOM)/hal/flashlight),)
MY_CUST_FTABLE_PATH += $(MTK_PATH_CUSTOM_PLATFORM)/hal/flashlight
else
MY_CUST_FTABLE_PATH += $(MTK_PATH_CUSTOM)/hal/flashlight
endif
endif
#
ifneq ($(wildcard $(MTK_PATH_CUSTOM)/hal/camerashot),)
MY_CUST_FTABLE_PATH += $(MTK_PATH_CUSTOM)/hal/camerashot
else
ifeq ($(strip $(TARGET_BOARD_PLATFORM)),mt6739)
ifneq ($(wildcard $(MTK_PATH_CUSTOM_PLATFORM_HAL1)/hal/camerashot),)
MY_CUST_FTABLE_PATH += $(MTK_PATH_CUSTOM_PLATFORM_HAL1)/hal/camerashot
endif
else
ifneq ($(wildcard $(MTK_PATH_CUSTOM_PLATFORM)/hal/camerashot),)
MY_CUST_FTABLE_PATH += $(MTK_PATH_CUSTOM_PLATFORM)/hal/camerashot
endif
endif
endif
#
ifneq (,$(filter $(strip $(TARGET_BOARD_PLATFORM)), mt6735 mt6737t))
ifeq ($(wildcard $(MTK_PATH_CUSTOM)/hal/camerashot),)
MY_CUST_FTABLE_PATH += $(MTK_PATH_CUSTOM_PLATFORM)/hal/D1/camerashot
else
MY_CUST_FTABLE_PATH += $(MTK_PATH_CUSTOM)/hal/camerashot
endif
endif
#
ifeq ($(strip $(TARGET_BOARD_PLATFORM)),mt6753)
ifeq ($(wildcard $(MTK_PATH_CUSTOM)/hal/camerashot),)
MY_CUST_FTABLE_PATH += $(MTK_PATH_CUSTOM_PLATFORM)/hal/D1/camerashot
else
MY_CUST_FTABLE_PATH += $(MTK_PATH_CUSTOM)/hal/camerashot
endif
endif
#
ifneq (,$(filter $(strip $(TARGET_BOARD_PLATFORM)), mt6735m mt6737m))
ifeq ($(wildcard $(MTK_PATH_CUSTOM)/hal/camerashot),)
MY_CUST_FTABLE_PATH += $(MTK_PATH_CUSTOM_PLATFORM)/hal/D2/camerashot
else
MY_CUST_FTABLE_PATH += $(MTK_PATH_CUSTOM)/hal/camerashot
endif
endif
#
ifeq ($(strip $(TARGET_BOARD_PLATFORM)),mt8173)
ifeq ($(wildcard $(MTK_PATH_CUSTOM)/hal/camerashot),)
MY_CUST_FTABLE_PATH += $(MTK_PATH_CUSTOM_PLATFORM)/hal/camerashot
else
MY_CUST_FTABLE_PATH += $(MTK_PATH_CUSTOM)/hal/camerashot
endif
endif
#
ifneq ($(wildcard $(MTK_PATH_CUSTOM)/hal/senindepfeature),)
MY_CUST_FTABLE_PATH += $(MTK_PATH_CUSTOM)/hal/senindepfeature
else
ifeq ($(strip $(TARGET_BOARD_PLATFORM)),mt6739)
ifneq ($(wildcard $(MTK_PATH_CUSTOM_PLATFORM_HAL1)/hal/senindepfeature),)
MY_CUST_FTABLE_PATH += $(MTK_PATH_CUSTOM_PLATFORM_HAL1)/hal/senindepfeature
endif
else
ifneq ($(wildcard $(MTK_PATH_CUSTOM_PLATFORM)/hal/senindepfeature),)
MY_CUST_FTABLE_PATH += $(MTK_PATH_CUSTOM_PLATFORM)/hal/senindepfeature
endif
endif
endif
#
ifneq (,$(filter $(strip $(TARGET_BOARD_PLATFORM)), mt6735 mt6737t))
ifeq ($(wildcard $(MTK_PATH_CUSTOM)/hal/senindepfeature),)
MY_CUST_FTABLE_PATH += $(MTK_PATH_CUSTOM_PLATFORM)/hal/D1/senindepfeature
else
MY_CUST_FTABLE_PATH += $(MTK_PATH_CUSTOM)/hal/senindepfeature
endif
endif
#
ifeq ($(strip $(TARGET_BOARD_PLATFORM)),mt6753)
ifeq ($(wildcard $(MTK_PATH_CUSTOM)/hal/senindepfeature),)
MY_CUST_FTABLE_PATH += $(MTK_PATH_CUSTOM_PLATFORM)/hal/D1/senindepfeature
else
MY_CUST_FTABLE_PATH += $(MTK_PATH_CUSTOM)/hal/senindepfeature
endif
endif
#
ifneq (,$(filter $(strip $(TARGET_BOARD_PLATFORM)), mt6735m mt6737m))
ifeq ($(wildcard $(MTK_PATH_CUSTOM)/hal/senindepfeature),)
MY_CUST_FTABLE_PATH += $(MTK_PATH_CUSTOM_PLATFORM)/hal/D2/senindepfeature
else
MY_CUST_FTABLE_PATH += $(MTK_PATH_CUSTOM)/hal/senindepfeature
endif
endif
#
ifeq ($(strip $(TARGET_BOARD_PLATFORM)),mt8173)
ifeq ($(wildcard $(MTK_PATH_CUSTOM)/hal/senindepfeature),)
MY_CUST_FTABLE_PATH += $(MTK_PATH_CUSTOM_PLATFORM)/hal/senindepfeature
else
MY_CUST_FTABLE_PATH += $(MTK_PATH_CUSTOM)/hal/senindepfeature
endif
endif

#-----------------------------------------------------------
#
define my-all-config.ftbl-under
$(patsubst ./%,%, \
  $(shell if [ -d $(1) ] ; then find $(1) -maxdepth 2 \( -name 'config.ftbl.*.h' \) -and -not -name '.*' ; fi) \
)
endef
#
# custom feature table file list
MY_CUST_FTABLE_FILE_LIST := $(foreach d,$(MY_CUST_FTABLE_PATH),$(call my-all-config.ftbl-under,$(d)))

#-----------------------------------------------------------
LOCAL_SRC_FILES += $(subst $(LOCAL_PATH)/,,$(sort $(wildcard $(LOCAL_PATH)/./*.cpp)))

#-----------------------------------------------------------
LOCAL_HEADER_LIBRARIES += libmtkcam_headers
LOCAL_C_INCLUDES += $(MTKCAM_C_INCLUDES)
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_SOURCE)/hardware/gralloc_extra/include
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_SOURCE)/hardware/mtkcam/ext/include

LOCAL_C_INCLUDES += $(PARAMSMGR_C_INCLUDES)
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_SOURCE)/hardware/include
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_SOURCE)/hardware/mtkcam/legacy/include
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_SOURCE)/hardware/mtkcam/legacy/include/mtkcam/v1
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_SOURCE)/hardware/mtkcam/ext/include
LOCAL_C_INCLUDES += $(TOP)/$(MTKCAM_C_INCLUDES)
#
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_SOURCE)/hardware/mtkcam/include
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_SOURCE)/frameworks/av
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../../inc
LOCAL_C_INCLUDES += $(TOP)
LOCAL_C_INCLUDES += $(LOCAL_PATH)/config/matv
LOCAL_C_INCLUDES += $(MTK_PATH_CUSTOM)/hal/inc/aaa
LOCAL_C_INCLUDES += $(MTK_PATH_CUSTOM)/hal/inc

ifeq ($(strip $(TARGET_BOARD_PLATFORM)),mt6739)
LOCAL_C_INCLUDES += $(MTK_PATH_CUSTOM_PLATFORM_HAL1)/hal/inc/aaa
LOCAL_C_INCLUDES += $(MTK_PATH_CUSTOM_PLATFORM_HAL1)/hal/inc
else
LOCAL_C_INCLUDES += $(MTK_PATH_CUSTOM_PLATFORM)/hal/inc/aaa
LOCAL_C_INCLUDES += $(MTK_PATH_CUSTOM_PLATFORM)/hal/inc
endif

#-----------------------------------------------------------
LOCAL_CFLAGS += $(MTKCAM_CFLAGS)

#====== FACEBEAUTY ======
ifeq ($(MTK_FACEBEAUTY_SUPPORT),yes)
    LOCAL_CFLAGS += -DFACEBEAUTY_SUPPORTED=1
endif

#====== VSS ======
ifeq ($(MTK_CAM_VSS_SUPPORT),yes)
    LOCAL_CFLAGS += -DVSS_SUPPORTED=1
endif

#====== EIS ======
ifeq ($(MTK_CAM_EIS_SUPPORT),yes)
    LOCAL_CFLAGS += -DEIS_SUPPORTED=1
endif

#====== VHDR ======
ifeq ($(MTK_CAM_VHDR_SUPPORT),yes)
    LOCAL_CFLAGS += -DVHDR_SUPPORTED=1
endif

#====== HDR ======
ifeq ($(MTK_CAM_HDR_SUPPORT),yes)
    LOCAL_CFLAGS += -DHDR_SUPPORTED=1
endif

#====== MFB ======
ifeq ($(MTK_CAM_MFB_SUPPORT),)
    LOCAL_CFLAGS += -DMFB_SUPPORTED=0
endif
ifeq ($(MTK_CAM_MFB_SUPPORT),0)
    LOCAL_CFLAGS += -DMFB_SUPPORTED=0
endif
ifeq ($(MTK_CAM_MFB_SUPPORT),1)
    LOCAL_CFLAGS += -DMFB_SUPPORTED=1
endif
ifeq ($(MTK_CAM_MFB_SUPPORT),2)
    LOCAL_CFLAGS += -DMFB_SUPPORTED=2
endif
ifeq ($(MTK_CAM_MFB_SUPPORT),3)
    LOCAL_CFLAGS += -DMFB_SUPPORTED=3
endif
ifeq ($(MTK_CAM_MFB_SUPPORT),4)
    LOCAL_CFLAGS += -DMFB_SUPPORTED=4
endif

#====== AUTORAMA ======
ifeq ($(MTK_CAM_AUTORAMA_SUPPORT),yes)
    LOCAL_CFLAGS += -DAUTORAMA_SUPPORTED=1
endif

#====== MAV ======
ifeq ($(MTK_CAM_MAV_SUPPORT),yes)
    LOCAL_CFLAGS += -DMAV_SUPPORTED=1
endif

#====== Video Facebeauty ======
ifeq ($(MTK_CAM_VIDEO_FACEBEAUTY_SUPPORT),yes)
    LOCAL_CFLAGS += -DVIDEO_FACEBEAUTY_SUPPORTED=1
endif

#====== Slow Motion Video ======
ifeq ($(MTK_SLOW_MOTION_VIDEO_SUPPORT),yes)
    LOCAL_CFLAGS += -DSLOW_MOTION_VIDEO_SUPPORTED=1
endif

#====== 3DNR ======
ifeq ($(MTK_CAM_NR3D_SUPPORT),yes)
    LOCAL_CFLAGS += -DNR3D_SUPPORTED=1
endif

#====== cFB ======
ifeq ($(MTK_CAM_FACEBEAUTY_SUPPORT),yes)
    LOCAL_CFLAGS += -DFACEBEAUTY_SUPPORTED=1
endif

#====== Gesture Shot ======
ifeq ($(MTK_CAM_GESTURE_SUPPORT),yes)
    LOCAL_CFLAGS += -DGESTURE_SHOT_SUPPORTED=1
endif

#====== Native PIP ======
ifeq ($(MTK_CAM_NATIVE_PIP_SUPPORT),yes)
    LOCAL_CFLAGS += -DPIP_SUPPORTED=1
endif

#====== ASD Shot ======
ifeq ($(MTK_CAM_ASD_SUPPORT),yes)
    LOCAL_CFLAGS += -DASD_SHOT_SUPPORTED=1
endif

#====== Depth-based AF ======
ifeq ($(MTK_CAM_DEPTH_AF_SUPPORT),yes)
    LOCAL_CFLAGS += -DDEPTH_AF_SUPPORTED=1
endif

#====== Distance Measurement ======
ifeq ($(MTK_CAM_DISTANCE_MEASURE_SUPPORT),yes)
    LOCAL_CFLAGS += -DDISTANCE_MEASURE_SUPPORTED=1
endif

#====== Image Refocus ======
ifeq ($(MTK_CAM_IMAGE_REFOCUS_SUPPORT),yes)
    LOCAL_CFLAGS += -DIMAGE_REFOCUS_SUPPORTED=1
endif

#====== Stereo Camera ======
ifeq ($(MTK_CAM_STEREO_CAMERA_SUPPORT),yes)
    LOCAL_CFLAGS += -DSTEREO_CAMERA_SUPPORTED=1
endif

#====== Continuous Shot ======
LOCAL_CFLAGS += -DCONTINUOUS_SHOT_SUPPORTED=1

#====== ZSD ======
$(info "MTK_CAM_ZSD_SUPPORT = $(MTK_CAM_ZSD_SUPPORT)")
ifeq ($(MTK_CAM_ZSD_SUPPORT),yes)
    $(info "Support zsd")
    LOCAL_CPPFLAGS += -DZSD_MODE_SUPPORT
endif

#====== LOMO ======
ifeq ($(MTK_CAM_LOMO_SUPPORT),yes)
    LOCAL_CFLAGS += -DLOMO_SUPPORTED=1
endif

ifeq ($(MTK_TARGET_PROJECT),tb8765ap1_bsp)
    LOCAL_CFLAGS += -DMTKCAM_SUB_FLASHLIGHT_SUPPORT=1
endif

ifeq ($(MTK_TARGET_PROJECT),tb8765ap1_64_bsp)
    LOCAL_CFLAGS += -DMTKCAM_SUB_FLASHLIGHT_SUPPORT=1
endif

ifeq ($(MTK_TARGET_PROJECT),tb8765ap1_bsp_1g)
    LOCAL_CFLAGS += -DMTKCAM_SUB_FLASHLIGHT_SUPPORT=1
endif

LOCAL_CFLAGS += -DCUSTOM_FLASHLIGHT_TYPE_$(CUSTOM_KERNEL_FLASHLIGHT)

PLATFORM_VERSION_MAJOR := $(word 1,$(subst .,$(space),$(PLATFORM_VERSION)))
LOCAL_CFLAGS += -DPLATFORM_VERSION_MAJOR=$(PLATFORM_VERSION_MAJOR)

#-----------------------------------------------------------
LOCAL_WHOLE_STATIC_LIBRARIES +=
#
LOCAL_STATIC_LIBRARIES +=

#-----------------------------------------------------------
LOCAL_MODULE := libcam.paramsmgr.feature.custom
#LOCAL_CFLAGS += -E
LOCAL_PROPRIETARY_MODULE := true
LOCAL_MODULE_OWNER := mtk

LOCAL_MODULE_CLASS := STATIC_LIBRARIES
#bobule workaround pdk build error, needing review
ifeq ($(TARGET_ARCH), arm)
INTERMEDIATES := $(call local-intermediates-dir)
else
INTERMEDIATES := $(call local-intermediates-dir)
INTERMEDIATES2 := $(call local-intermediates-dir,,$(TARGET_2ND_ARCH_VAR_PREFIX))
endif
# custom feature table all-in-one file
ifeq ($(TARGET_ARCH), arm)
MY_CUST_FTABLE_FINAL_FILE := $(INTERMEDIATES)/custgen.config.ftbl.h
else
MY_CUST_FTABLE_FINAL_FILE := $(INTERMEDIATES)/custgen.config.ftbl.h $(INTERMEDIATES2)/custgen.config.ftbl.h
endif
LOCAL_GENERATED_SOURCES += $(MY_CUST_FTABLE_FINAL_FILE)
$(MY_CUST_FTABLE_FINAL_FILE): $(MY_CUST_FTABLE_FILE_LIST)
	@mkdir -p $(dir $@)
	@echo '//this file is auto-generated; do not modify it!' > $@
	@echo '#define MY_CUST_VERSION "$(PARAMSMGR_VERSION)"' >> $@
	@echo '#define MY_CUST_FTABLE_FILE_LIST "$(MY_CUST_FTABLE_FILE_LIST)"' >> $@
	@echo '#define MY_CUST_FTABLE_FINAL_FILE "$(MY_CUST_FTABLE_FINAL_FILE)"' >> $@
	@for x in $(MY_CUST_FTABLE_FILE_LIST); do echo "#include <$$x>" >> $@; done

#-----------------------------------------------------------
include $(MTK_STATIC_LIBRARY)


################################################################################
#
################################################################################
#include $(CLEAR_VARS)
#include $(call all-makefiles-under,$(LOCAL_PATH))


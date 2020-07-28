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

ifneq (,$(filter $(strip $(TARGET_BOARD_PLATFORM)), mt6757))
ifneq (,$(filter $(strip $(MTK_CAM_SW_VERSION)), ver2))
 		MTKCAM_DRV_PLATFORM := mt6757p
 		MTKCAM_DRV_P1_PLATFORM := mt6757p
 		$(info This is $(MTKCAM_DRV_PLATFORM) ver2)
else
 		MTKCAM_DRV_PLATFORM := $(TARGET_BOARD_PLATFORM)
 		MTKCAM_DRV_P1_PLATFORM := $(TARGET_BOARD_PLATFORM)
 		$(info This is $(MTKCAM_DRV_PLATFORM) ver1)
endif
 		MTKCAM_ISP_INCLUDE := $(TOP)/$(MTK_PATH_SOURCE)/hardware/mtkcam/drv/src/isp/$(MTKCAM_DRV_PLATFORM)
 		MTKCAM_DRV_INCLUDE := $(TOP)/$(MTK_PATH_SOURCE)/hardware/mtkcam/drv/include/$(TARGET_BOARD_PLATFORM)
else ifneq (,$(filter $(strip $(TARGET_BOARD_PLATFORM)), mt6799))
ifneq (,$(filter $(strip $(MTK_CAM_SW_VERSION)), ver2))
 		MTKCAM_DRV_PLATFORM := mt6799p
 		MTKCAM_DRV_P1_PLATFORM := mt6799p
 		$(info This is $(MTKCAM_DRV_PLATFORM) ver2)
else
 		MTKCAM_DRV_PLATFORM := $(TARGET_BOARD_PLATFORM)
 		MTKCAM_DRV_P1_PLATFORM := $(TARGET_BOARD_PLATFORM)
 		$(info This is $(MTKCAM_DRV_PLATFORM) ver1)
endif
 		MTKCAM_ISP_INCLUDE := $(TOP)/$(MTK_PATH_SOURCE)/hardware/mtkcam/drv/src/isp/$(MTKCAM_DRV_PLATFORM)
 		MTKCAM_DRV_INCLUDE := $(TOP)/$(MTK_PATH_SOURCE)/hardware/mtkcam/drv/include/$(MTKCAM_DRV_PLATFORM)
else ifneq (,$(filter $(strip $(TARGET_BOARD_PLATFORM)), mt6885))
		MTKCAM_P2_DECOUPLE := 1
		MTKCAM_DRV_P1_PLATFORM := mt6885
		MTKCAM_DRV_PLATFORM := mt6885
		MTKCAM_DRV_DIP_VERSION_FOLDER := 6s
 		MTKCAM_ISP_INCLUDE := $(TOP)/$(MTK_PATH_SOURCE)/hardware/mtkcam/drv/src/isp/$(MTKCAM_DRV_PLATFORM)
 		MTKCAM_DRV_INCLUDE := $(TOP)/$(MTK_PATH_SOURCE)/hardware/mtkcam/drv/include/$(MTKCAM_DRV_PLATFORM)
 		MTKCAM_DRV_INCLUDE += $(TOP)/$(MTK_PATH_SOURCE)/hardware/mtkcam/drv/include/dip/$(MTKCAM_DRV_PLATFORM)
		MTKCAM_DRV_INCLUDE += $(TOP)/$(MTK_PATH_SOURCE)/hardware/mtkcam/drv/include/mfb/$(MTKCAM_DRV_PLATFORM)
		MTKCAM_DRV_INCLUDE += $(TOP)/system/core/libutils/include/utils
else ifneq (,$(filter $(strip $(TARGET_BOARD_PLATFORM)), mt6771 mt6775 mt3967 mt6785))
		MTKCAM_P2_DECOUPLE := 1
#{
ifeq ($(TARGET_BOARD_PLATFORM), mt6785)
    MTKCAM_DRV_P1_PLATFORM := $(TARGET_BOARD_PLATFORM)
else
    MTKCAM_DRV_P1_PLATFORM := isp_50
endif
#}
		MTKCAM_DRV_PLATFORM := isp_50
		MTKCAM_DRV_DIP_VERSION_FOLDER := 5.0
 		MTKCAM_ISP_INCLUDE := $(TOP)/$(MTK_PATH_SOURCE)/hardware/mtkcam/drv/src/isp/$(MTKCAM_DRV_P1_PLATFORM)
 		MTKCAM_DRV_INCLUDE := $(TOP)/$(MTK_PATH_SOURCE)/hardware/mtkcam/drv/include/$(MTKCAM_DRV_P1_PLATFORM)
 		MTKCAM_DRV_INCLUDE += $(TOP)/$(MTK_PATH_SOURCE)/hardware/mtkcam/drv/include/dip/$(MTKCAM_DRV_PLATFORM)
		MTKCAM_DRV_INCLUDE += $(TOP)/$(MTK_PATH_SOURCE)/hardware/mtkcam/drv/include/mfb/$(MTKCAM_DRV_PLATFORM)
else ifneq (,$(filter $(strip $(TARGET_BOARD_PLATFORM)), mt6779))
		MTKCAM_P2_DECOUPLE := 1
		MTKCAM_DRV_PLATFORM := mt6779
		MTKCAM_DRV_P1_PLATFORM := mt6779
		MTKCAM_DRV_DIP_VERSION_FOLDER := 6.0
 		MTKCAM_ISP_INCLUDE := $(TOP)/$(MTK_PATH_SOURCE)/hardware/mtkcam/drv/src/isp/$(MTKCAM_DRV_PLATFORM)
 		MTKCAM_DRV_INCLUDE := $(TOP)/$(MTK_PATH_SOURCE)/hardware/mtkcam/drv/include/$(MTKCAM_DRV_PLATFORM)
 		MTKCAM_DRV_INCLUDE += $(TOP)/$(MTK_PATH_SOURCE)/hardware/mtkcam/drv/include/dip/$(MTKCAM_DRV_PLATFORM)
		MTKCAM_DRV_INCLUDE += $(TOP)/$(MTK_PATH_SOURCE)/hardware/mtkcam/drv/include/mfb/$(MTKCAM_DRV_PLATFORM)
		MTKCAM_DRV_INCLUDE += $(TOP)/system/core/libutils/include/utils
else    ifneq (,$(filter $(strip $(TARGET_BOARD_PLATFORM)), mt6739 mt6755 mt6750 mt6761 mt6735 mt6753))
		MTKCAM_DRV_ISP_VERSION := 30
		MTKCAM_P2_DECOUPLE := 0
		MTKCAM_DRV_PLATFORM := $(TARGET_BOARD_PLATFORM)
		MTKCAM_DRV_P1_PLATFORM := $(TARGET_BOARD_PLATFORM)
		MTKCAM_ISP_INCLUDE := $(TOP)/$(MTK_PATH_SOURCE)/hardware/mtkcam/drv/src/isp/$(MTKCAM_DRV_PLATFORM)
 		MTKCAM_DRV_INCLUDE := $(TOP)/$(MTK_PATH_SOURCE)/hardware/mtkcam/drv/include/$(MTKCAM_DRV_PLATFORM)
else
		MTKCAM_DRV_PLATFORM := $(TARGET_BOARD_PLATFORM)
		MTKCAM_DRV_P1_PLATFORM := $(TARGET_BOARD_PLATFORM)
		MTKCAM_ISP_INCLUDE := $(TOP)/$(MTK_PATH_SOURCE)/hardware/mtkcam/drv/src/isp/$(MTKCAM_DRV_PLATFORM)
 		MTKCAM_DRV_INCLUDE := $(TOP)/$(MTK_PATH_SOURCE)/hardware/mtkcam/drv/include/$(MTKCAM_DRV_PLATFORM)
endif
################################################################################
#TSF Part Start
################################################################################
MTKCAM_TSF_PATH := $(TOP)/$(MTK_PATH_SOURCE)/hardware/mtkcam/drv/src/tsf/1.0
MTKCAM_TSF_VERSION := 1.0

MTKTSF_SUPPORT_PLATFORM := none

ifneq (,$(filter $(strip $(TARGET_BOARD_PLATFORM)), mt6757))
ifneq (,$(filter $(strip $(MTK_CAM_SW_VERSION)), ver2))
 		MTKTSF_SUPPORT := 0
 		$(info This is ver2)
else
 		MTKTSF_SUPPORT := 1
 		$(info This is ver1)
endif
else ifneq (,$(filter $(strip $(TARGET_BOARD_PLATFORM)), mt6799))
ifneq (,$(filter $(strip $(MTK_CAM_SW_VERSION)), ver2))
 		MTKTSF_SUPPORT := 1
 		$(info This is ver2)
else
 		MTKTSF_SUPPORT := 0
 		$(info This is ver1)
endif
else ifneq (, $(findstring $(strip $(TARGET_BOARD_PLATFORM)), $(MTKTSF_SUPPORT_PLATFORM)))
 		MTKTSF_SUPPORT := 1
else
 		MTKTSF_SUPPORT := 0
endif

LOCAL_CFLAGS += -DMTKTSF_SUPPORT=$(MTKTSF_SUPPORT)

################################################################################
#
################################################################################

################################################################################
#DPE Part Start
################################################################################
MTKDPE_SUPPORT_PLATFORM := mt6759 mt6799 mt6797 mt6758 mt6775 mt6771 mt6765 mt6762 mt6779 mt6768 mt6785

DPE_10_PLATFORM := mt6759 mt6799 mt6797 mt6758 mt6775 mt6771 mt6765 mt6762 mt6768 mt6785
DPE_20_PLATFORM := mt6779

ifneq (, $(findstring $(strip $(TARGET_BOARD_PLATFORM)), $(DPE_10_PLATFORM)))
		MTKCAM_DPE_PATH := $(TOP)/$(MTK_PATH_SOURCE)/hardware/mtkcam/drv/src/dpe/1.0
		MTKCAM_DPE_VERSION := 1.0
else ifneq (, $(findstring $(strip $(TARGET_BOARD_PLATFORM)), $(DPE_20_PLATFORM)))
        MTKCAM_DPE_PATH := $(TOP)/$(MTK_PATH_SOURCE)/hardware/mtkcam/drv/src/dpe/2.0
        MTKCAM_DPE_VERSION := 2.0
else
endif

ifneq (,$(filter $(strip $(TARGET_BOARD_PLATFORM)), mt6757))
ifneq (,$(filter $(strip $(MTK_CAM_SW_VERSION)), ver2))
 		MTKDPE_SUPPORT := 1
 		$(info This is ver2)
else
 		MTKDPE_SUPPORT := 0
 		$(info This is ver1)
endif
else ifneq (, $(findstring $(strip $(TARGET_BOARD_PLATFORM)), $(MTKDPE_SUPPORT_PLATFORM)))
 		MTKDPE_SUPPORT := 1
else
 		MTKDPE_SUPPORT := 0
endif

LOCAL_CFLAGS += -DMTKDPE_SUPPORT=$(MTKDPE_SUPPORT)

################################################################################
#OWE Part Start
################################################################################
MTKCAM_OWE_PATH := $(TOP)/$(MTK_PATH_SOURCE)/hardware/mtkcam/drv/src/owe/
MTKCAM_OWE_VERSION := 1.0

MTKOWE_SUPPORT_PLATFORM := isp_50
MTKOWE_EGNSTREAM_PLATFORMS := isp_50

ifneq (,$(filter $(strip $(MTKCAM_DRV_PLATFORM)), none))
ifneq (,$(filter $(strip $(MTK_CAM_SW_VERSION)), ver2))
MTKOWE_SUPPORT := 1
$(info This is ver2)
else
MTKOWE_SUPPORT := 0
$(info This is ver1)
endif
else ifneq (, $(findstring $(strip $(MTKCAM_DRV_PLATFORM)), $(MTKOWE_SUPPORT_PLATFORM)))
MTKOWE_SUPPORT := 1
else
MTKOWE_SUPPORT := 0
endif

ifneq (, $(findstring $(strip $(MTKCAM_DRV_PLATFORM)), $(MTKOWE_EGNSTREAM_PLATFORMS)))
MTKCAM_OWE_VERSION := 1.1
else
MTKCAM_OWE_VERSION := 1.0
endif

LOCAL_CFLAGS += -DMTKOWE_SUPPORT=$(MTKOWE_SUPPORT)

################################################################################
#
################################################################################

################################################################################
#RSC platfrom and version control
################################################################################
MTKCAM_RSC_PATH := $(TOP)/$(MTK_PATH_SOURCE)/hardware/mtkcam/drv/src/rsc/1.0
MTKCAM_RSC_VERSION := 1.0

MTKRSC_SUPPORT_PLATFORM := mt6799 mt6799p mt6759 mt6758 mt6771 mt6775 mt6779 mt6785

ifneq (,$(filter $(strip $(TARGET_BOARD_PLATFORM)), none))
ifneq (,$(filter $(strip $(MTK_CAM_SW_VERSION)), ver2))
 		MTKRSC_SUPPORT := 1
 		$(info This is ver2)
else
 		MTKRSC_SUPPORT := 0
 		$(info This is ver1)
endif
else ifneq (, $(findstring $(strip $(TARGET_BOARD_PLATFORM)), $(MTKRSC_SUPPORT_PLATFORM)))
 		MTKRSC_SUPPORT := 1
    ifneq (, $(findstring $(strip $(TARGET_BOARD_PLATFORM)), mt6771 mt6775 mt6779 mt6785))
                RSC_DRV_PLATFORM := isp_50
    else
                RSC_DRV_PLATFORM := $(TARGET_BOARD_PLATFORM)
    endif
else
 		MTKRSC_SUPPORT := 0
endif

LOCAL_CFLAGS += -DMTKRSC_SUPPORT=$(MTKRSC_SUPPORT)

################################################################################
#MFB platfrom and version control
################################################################################
MTKCAM_MFB_PATH := $(TOP)/$(MTK_PATH_SOURCE)/hardware/mtkcam/drv/src/mfb/1.0
MTKCAM_MFB_VERSION := 1.0

MTKMFB_SUPPORT_PLATFORM := mt6775 mt6771 mt6779 mt6785

ifneq (, $(findstring $(strip $(TARGET_BOARD_PLATFORM)), $(MTKMFB_SUPPORT_PLATFORM)))
                MTKMFB_SUPPORT := 1
else
                MTKMFB_SUPPORT := 0
endif

ifneq (,$(filter $(strip $(TARGET_BOARD_PLATFORM)), mt6775 mt6771 mt6785))
                MTKCAM_DRV_MFB_PLATFORM := isp_50
else ifneq (,$(filter $(strip $(TARGET_BOARD_PLATFORM)), mt6779))
                MTKCAM_DRV_MFB_PLATFORM := mt6779
endif

LOCAL_CFLAGS += -DMTKMFB_SUPPORT=$(MTKMFB_SUPPORT)

################################################################################
#FDVT Part Start
################################################################################
#$(info FDVT: makefile start)

FDVT_LEGACY_PLATFORM = mt6735 mt6755
FDVT_35_PLATFORM = mt6761
FDVT_35P_PLATFORM = mt6763
FDVT_40_PLATFORM = mt6771 mt6775 mt3886 mt6758 mt6757 mt6799 mt6759 mt6797 mt6765 mt6768
FDVT_50_PLATFORM = mt6779 mt6785
FDVT_51_PLATFORM = mt6885
MTKFDVT_SUPPORT_PLATFORM := mt6779 mt6785 mt6885
FDVT_CURRENT_PLATFORM = $(subst ",,$(TARGET_BOARD_PLATFORM))

#$(info FDVT_CURRENT_PLATFORM= $(FDVT_CURRENT_PLATFORM))
ifeq (, $(findstring $(FDVT_CURRENT_PLATFORM), $(FDVT_LEGACY_PLATFORM)))

ifneq (, $(findstring $(FDVT_CURRENT_PLATFORM), $(FDVT_35P_PLATFORM)))
#$(info FDVT: Drv use 3.5P folder)
MTKCAM_FDVT_PATH := $(TOP)/$(MTK_PATH_SOURCE)/hardware/mtkcam/drv/src/fdvt/3.5
MTKCAM_FDVT_VERSION := 3.5
LOCAL_CFLAGS += -DFDVT_VERSION_35P
else ifneq (, $(findstring $(FDVT_CURRENT_PLATFORM), $(FDVT_35_PLATFORM)))
#$(info FDVT: Drv use 3.5 folder)
MTKCAM_FDVT_PATH := $(TOP)/$(MTK_PATH_SOURCE)/hardware/mtkcam/drv/src/fdvt/3.5
MTKCAM_FDVT_VERSION := 3.5
else ifneq (, $(findstring $(FDVT_CURRENT_PLATFORM), $(FDVT_40_PLATFORM)))
#$(info FDVT: Drv use 4.0 folder)
MTKCAM_FDVT_PATH := $(TOP)/$(MTK_PATH_SOURCE)/hardware/mtkcam/drv/src/fdvt/4.0
MTKCAM_FDVT_VERSION := 4.0
else ifneq (, $(findstring $(FDVT_CURRENT_PLATFORM), $(FDVT_50_PLATFORM)))
#$(info FDVT: Drv use 5.0 folder)
MTKCAM_FDVT_PATH := $(TOP)/$(MTK_PATH_SOURCE)/hardware/mtkcam/drv/src/fdvt/5.0
MTKCAM_FDVT_VERSION := 5.0

MTKFDVT_SUPPORT_PLATFORM := mt3967 mt6779 mt6785

ifneq (, $(findstring $(strip $(TARGET_BOARD_PLATFORM)), $(MTKFDVT_SUPPORT_PLATFORM)))
                MTKFDVT_SUPPORT := 1
else
                MTKFDVT_SUPPORT := 0
endif

LOCAL_CFLAGS += -DMTKFDVT_SUPPORT=$(MTKFDVT_SUPPORT)

else ifneq (, $(findstring $(FDVT_CURRENT_PLATFORM), $(FDVT_51_PLATFORM)))
#$(info FDVT: Drv use 5.1 folder)
MTKCAM_FDVT_PATH := $(TOP)/$(MTK_PATH_SOURCE)/hardware/mtkcam/drv/src/fdvt/5.1
MTKCAM_FDVT_VERSION := 5.1

MTKFDVT_SUPPORT_PLATFORM := mt6885

ifneq (, $(findstring $(strip $(TARGET_BOARD_PLATFORM)), $(MTKFDVT_SUPPORT_PLATFORM)))
                MTKFDVT_SUPPORT := 1
else
                MTKFDVT_SUPPORT := 0
endif

LOCAL_CFLAGS += -DMTKFDVT_SUPPORT=$(MTKFDVT_SUPPORT)
endif

endif
#$(info FDVT: makefile end)

################################################################################
#
################################################################################


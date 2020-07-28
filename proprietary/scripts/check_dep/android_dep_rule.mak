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

# note: modem makefile operation is moved from mediatek/build/libs/custom.mk for AOSP use
# get modem feature option
ifneq ($(strip $(CUSTOM_MODEM)),)
  mtk_modem_file := $(wildcard $(foreach m,$(CUSTOM_MODEM),$(MTK_ROOT)/modem/$(m)/modem_*_*_*.mak))
  mtk_modem_feature_md :=
  mtk_modem_feature_ap :=
  ifeq ($(mtk_modem_file),)
    $(warning Warning: Invalid CUSTOM_MODEM = $(CUSTOM_MODEM) in ProjectConfig.mk)
    $(warning Cannot find $(foreach m,$(CUSTOM_MODEM),$(MTK_ROOT)/modem/$(m)) for modem_*_*_*.mak)
  endif
  $(info mtk_modem_file = $(mtk_modem_file))
  $(foreach modem_make,$(mtk_modem_file),\
	$(eval modem_name_x_yy_z := $(subst modem,MODEM,$(notdir $(basename $(modem_make)))))\
	$(eval modem_name_x := $(word 1,$(subst _, ,$(modem_name_x_yy_z)))_$(word 2,$(subst _, ,$(modem_name_x_yy_z))))\
	$(eval modem_text := $(shell cat $(modem_make) | sed -e '/^\s*#/'d | awk -F# '{print $$1}' | sed -n '/^\S\+\s*=\s*\S\+/'p | sed -e 's/\s/\?/g' ))\
	$(foreach modem_line,$(modem_text),\
		$(eval modem_option := $(subst ?, ,$(modem_line)))\
		$(eval $(modem_name_x_yy_z)_$(modem_option))\
	)\
	$(foreach modem_line,$(modem_text),\
		$(eval modem_option := $(subst ?, ,$(modem_line)))\
		$(eval modem_feature := $(word 1,$(subst =, ,$(modem_option))))\
		$(eval MODEM_$(modem_feature) := $(sort $(MODEM_$(modem_feature)) $($(modem_name_x_yy_z)_$(modem_feature))))\
		$(eval $(modem_name_x)_$(modem_feature) := $(sort $($(modem_name_x)_$(modem_feature)) $($(modem_name_x_yy_z)_$(modem_feature))))\
		$(eval mtk_modem_feature_md := $(sort $(mtk_modem_feature_md) $(modem_feature)))\
		$(eval mtk_modem_feature_ap := $(sort $(mtk_modem_feature_ap) MODEM_$(modem_feature) $(modem_name_x)_$(modem_feature) $(modem_name_x_yy_z)_$(modem_feature)))\
	)\
  )
  modem_name_x_yy_z :=
  modem_text :=
  modem_option :=
  modem_feature :=
endif

#######################################################
# dependency check between AP side & modem side

ifeq (yes,$(strip $(MTK_TTY_SUPPORT)))
  ifneq ($(filter FALSE,$(strip $(MODEM_CTM_SUPPORT))),)
    $(call dep-err-ona-or-offb,CTM_SUPPORT,MTK_TTY_SUPPORT)
  endif
endif

ifneq ($(strip $(MODEM_MTK_BT_CHIP)),)
  ifneq ($(strip $(MODEM_MTK_BT_CHIP)),NONE)
    ifneq ($(filter-out $(strip $(MTK_BT_CHIP)),$(strip $(MODEM_MTK_BT_CHIP))),)
      $(call dep-err-common, Please change MODEM project makefile's MTK_BT_CHIP to be the same with ProjectConfig.mk's and rebuild MODEM binary. Or change ProjectConfig.mk's MTK_BT_CHIP to be the same with $(mtk_modem_file).)
    endif
  endif
endif

ifeq ($(strip $(MTK_PROTOCOL2_RAT_CONFIG)),L/W/G)
  ifneq ($(strip $(MODEM_GEMINI_L)),)
    ifneq ($(strip $(MODEM_GEMINI_L)),2)
      $(call dep-err-common, Please set MODEM project makefile's GEMINI_L as 2 or set MTK_PROTOCOL2_RAT_CONFIG as not L/W/G)
    endif
  endif
endif

ifneq (,$(filter G W/G,$(strip $(MTK_PROTOCOL2_RAT_CONFIG))))
  ifeq (yes,$(strip $(MTK_TEMPORARY_DATA_SUPPORT)))
    $(call dep-err-common, Please turn off Temp Data Swtich MTK_TEMPORARY_DATA_SUPPORT on non L+L project MTK_PROTOCOL2_RAT_CONFIG = G or W/G)
  endif
endif
#######################################################
# UFS related dependency check
ifeq (yes,$(strip $(MTK_UFS_SUPPORT)))
  ifeq (yes,$(strip $(MTK_MLC_NAND_SUPPORT)))
    $(call dep-err-offa-or-offb,MTK_UFS_SUPPORT,MTK_MLC_NAND_SUPPORT)
  endif
  ifeq (yes,$(strip $(MTK_COMBO_NAND_SUPPORT)))
    $(call dep-err-offa-or-offb,MTK_UFS_SUPPORT,MTK_COMBO_NAND_SUPPORT)
  endif
  ifeq (no,$(strip $(MTK_GPT_SCHEME_SUPPORT)))
    $(call dep-err-ona-or-offb,MTK_GPT_SCHEME_SUPPORT,MTK_UFS_SUPPORT)
  endif
endif

############################################################
# Platforms in the blacklist must set MTK_HEADER_SUPPORT to yes. Other platforms
# must NOT enable it (can be no or empty).
mtkplatformblacklist:= MT8163
ifneq ($(MTK_PLATFORM),)
    ifeq ($(filter $(strip $(MTK_PLATFORM)),$(mtkplatformblacklist)),)
        ifeq (yes,$(strip $(MTK_HEADER_SUPPORT)))
           $(call dep-err-common,Please trun off MTK_HEADER_SUPPORT when MTK_PLATFORM is $(MTK_PLATFORM))
        endif
    else
        ifneq (yes,$(strip $(MTK_HEADER_SUPPORT)))
        $(call dep-err-common,Please trun on MTK_HEADER_SUPPORT when MTK_PLATFORM is $(MTK_PLATFORM))
        endif
    endif
endif

###########################################################
# for wapi feature

ifeq (yes,$(strip $(MTK_WAPI_SUPPORT)))
  ifneq (yes, $(strip $(MTK_WLAN_SUPPORT)))
    $(call dep-err-ona-or-offb, MTK_WLAN_SUPPORT, MTK_WAPI_SUPPORT)
  endif
endif

ifeq (yes,$(strip $(MTK_CTA_SUPPORT)))
  ifneq (yes, $(strip $(MTK_WAPI_SUPPORT)))
    $(call dep-err-ona-or-offb, MTK_WAPI_SUPPORT, MTK_CTA_SUPPORT)
  endif
endif

##############################################################
# for gps feature

ifeq (yes,$(strip $(MTK_AGPS_APP)))
  ifeq (no,$(strip $(MTK_GPS_SUPPORT)))
     $(call dep-err-ona-or-offb, MTK_GPS_SUPPORT, MTK_AGPS_APP)
  endif
endif

#############################################################
# for BT

ifeq (no,$(strip $(MTK_BT_SUPPORT)))
  ifeq (yes,$(strip $(MTK_BT_FM_OVER_BT_VIA_CONTROLLER)))
    $(call dep-err-ona-or-offb, MTK_BT_SUPPORT, MTK_BT_FM_OVER_BT_VIA_CONTROLLER)
  endif
endif

##############################################################
# for emmc feature
ifneq (yes,$(strip $(MTK_EMMC_SUPPORT)))
  ifeq (yes,$(strip $(MTK_FSCK_TUNE)))
    $(call dep-err-ona-or-offb, MTK_EMMC_SUPPORT, MTK_FSCK_TUNE)
  endif
endif
##############################################################
# for otp
ifeq (yes,$(strip $(MTK_EMMC_SUPPORT_OTP)))
  ifneq (yes,$(strip $(MTK_EMMC_SUPPORT)))
    $(call dep-err-ona-or-offb, MTK_EMMC_SUPPORT, MTK_EMMC_SUPPORT_OTP)
  endif
endif

ifeq (yes,$(strip $(MTK_UFS_OTP)))
  ifneq (yes,$(strip $(MTK_UFS_SUPPORT)))
    $(call dep-err-ona-or-offb, MTK_UFS_SUPPORT, MTK_UFS_OTP)
  endif
endif

ifeq (TRUE,$(strip $(MODEM_OTP_SUPPORT)))
  ifneq (yes,$(strip $(MTK_EMMC_SUPPORT_OTP)))
    ifneq (yes,$(strip $(NAND_OTP_SUPPORT)))
      ifneq (yes,$(strip $(MTK_UFS_OTP)))
        $(call dep-err-common, Please turn on MTK_UFS_OTP, MTK_EMMC_SUPPORT_OTP or NAND_OTP_SUPPORT when MODEM_OTP_SUPPORT is TRUE)
      endif
    endif
  endif
endif

ifeq (yes, $(strip $(MTK_COMBO_NAND_SUPPORT)))
  ifeq (yes, $(strip $(MTK_EMMC_SUPPORT)))
    $(call dep-err-common, Please turn off MTK_COMBO_NAND_SUPPORT or turn off MTK_EMMC_SUPPORT)
  endif
endif
##############################################################
# for NFC feature
ifeq (yes,$(strip $(MTK_NFC_ADDON_SUPPORT)))
  ifneq (yes,$(strip $(NFC_CHIP_SUPPORT)))
    $(call dep-err-ona-or-offb,NFC_CHIP_SUPPORT,MTK_NFC_ADDON_SUPPORT)
  endif
endif

##############################################################
ifeq ($(strip $(MTK_COMBO_CHIP)),MT6628)
  ifneq ($(strip $(MTK_BT_FM_OVER_BT_VIA_CONTROLLER)),no)
    $(call dep-err-seta-or-setb,MTK_BT_FM_OVER_BT_VIA_CONTROLLER,no,MTK_COMBO_CHIP,none MT6628)
  endif
endif
ifeq ($(strip $(MTK_BT_CHIP)),MTK_MT6628)
  ifneq ($(strip $(MTK_COMBO_CHIP)),MT6628)
     $(call dep-err-seta-or-setb,MTK_BT_CHIP, none MTK_MT6628,MTK_COMBO_CHIP,MT6628)
  endif
endif
ifeq ($(strip $(MTK_WLAN_CHIP)),MT6628)
  ifneq ($(strip $(MTK_COMBO_CHIP)),MT6628)
    $(call dep-err-seta-or-setb,MTK_WLAN_CHIP, none MT6628,MTK_COMBO_CHIP,MT6628)
  endif
endif
ifeq ($(strip $(MTK_GPS_CHIP)),MTK_GPS_MT6628)
  ifneq ($(strip $(MTK_COMBO_CHIP)),MT6628)
    $(call dep-err-seta-or-setb,MTK_GPS_CHIP, none MTK_GPS_MT6628,MTK_COMBO_CHIP,MT6628)
  endif
endif

############################################################
ifeq ($(strip $(MTK_AP_SPEECH_ENHANCEMENT)),yes)
  ifneq ($(strip $(MTK_AUDIO_HD_REC_SUPPORT)),yes)
    $(call dep-err-ona-or-offb, MTK_AUDIO_HD_REC_SUPPORT, MTK_AP_SPEECH_ENHANCEMENT)
  endif
endif
############################################################
ifeq ($(strip $(MTK_INTERNAL_HDMI_SUPPORT)),yes)
  ifeq ($(strip $(MTK_INTERNAL_MHL_SUPPORT)),yes)
    $(call dep-err-offa-or-offb, MTK_INTERNAL_HDMI_SUPPORT, MTK_INTERNAL_MHL_SUPPORT)
  endif
  ifneq ($(strip $(MTK_HDMI_SUPPORT)),yes)
    $(call dep-err-ona-or-offb, MTK_HDMI_SUPPORT, MTK_INTERNAL_HDMI_SUPPORT)
  endif
else
  ifeq ($(strip $(MTK_INTERNAL_MHL_SUPPORT)),yes)
    ifneq ($(strip $(MTK_HDMI_SUPPORT)),yes)
      $(call dep-err-ona-or-offb, MTK_HDMI_SUPPORT, MTK_INTERNAL_MHL_SUPPORT)
    endif
  endif
endif

############################################################
ifeq ($(strip $(MTK_MT8193_HDCP_SUPPORT)),yes)
  ifneq ($(strip $(MTK_MT8193_HDMI_SUPPORT)),yes)
    $(call dep-err-ona-or-offb, MTK_MT8193_HDMI_SUPPORT, MTK_MT8193_HDCP_SUPPORT)
  endif
endif
ifeq ($(strip $(MTK_MT8193_HDMI_SUPPORT)),yes)
  ifneq ($(strip $(MTK_MT8193_SUPPORT)),yes)
    $(call dep-err-ona-or-offb, MTK_MT8193_SUPPORT, MTK_MT8193_HDMI_SUPPORT)
  endif
endif
############################################################
ifeq (yes, $(strip $(MTK_SIM_HOT_SWAP)))
  ifneq (no, $(strip $(MTK_RADIOOFF_POWER_OFF_MD)))
    $(call dep-err-ona-or-offb, MTK_RADIOOFF_POWER_OFF_MD, MTK_SIM_HOT_SWAP)
  endif
endif
############################################################
ifeq (yes, $(strip $(MTK_WVDRM_L1_SUPPORT)))
  ifneq (yes , $(strip $(MTK_IN_HOUSE_TEE_SUPPORT)))
    ifneq (yes, $(strip $(TRUSTONIC_TEE_SUPPORT)))
      ifneq (yes, $(strip $(MTK_GOOGLE_TRUSTY_SUPPORT)))
        ifneq (yes, $(strip $(MICROTRUST_TEE_SUPPORT)))
          $(call dep-err-common, Please turn on MTK_IN_HOUSE_TEE_SUPPORT or TRUSTONIC_TEE_SUPPORT or MTK_GOOGLE_TRUSTY_SUPPORT or MICROTRUST_TEE_SUPPORT when MTK_WVDRM_L1_SUPPORT set as yes)
        endif
      endif
    endif
  endif
  ifneq (yes , $(strip $(MTK_DRM_KEY_MNG_SUPPORT)))
    #$(call dep-err-ona-or-offb,MTK_DRM_KEY_MNG_SUPPORT,MTK_WVDRM_L1_SUPPORT)
  endif
  ifneq (yes , $(strip $(MTK_SEC_VIDEO_PATH_SUPPORT)))
    $(call dep-err-ona-or-offb,MTK_SEC_VIDEO_PATH_SUPPORT,MTK_WVDRM_L1_SUPPORT)
  endif
endif
ifeq (yes, $(strip $(MTK_DRM_KEY_MNG_SUPPORT)))
  ifneq (yes, $(strip $(MTK_IN_HOUSE_TEE_SUPPORT)))
    ifneq (yes, $(strip $(TRUSTONIC_TEE_SUPPORT)))
      ifneq (yes, $(strip $(MTK_GOOGLE_TRUSTY_SUPPORT)))
        ifneq (yes, $(strip $(MICROTRUST_TEE_SUPPORT)))
          $(call dep-err-common, Please turn on MTK_IN_HOUSE_TEE_SUPPORT or TRUSTONIC_TEE_SUPPORT or MTK_GOOGLE_TRUSTY_SUPPORT or MICROTRUST_TEE_SUPPORT when MTK_DRM_KEY_MNG_SUPPORT set as yes)
        endif
      endif
    endif
  endif
endif
ifeq (yes , $(strip $(MTK_SEC_VIDEO_PATH_SUPPORT)))
  ifneq (yes, $(strip $(MTK_IN_HOUSE_TEE_SUPPORT)))
    ifneq (yes, $(strip $(TRUSTONIC_TEE_SUPPORT)))
      ifneq (yes, $(strip $(MTK_GOOGLE_TRUSTY_SUPPORT)))
        ifneq (yes, $(strip $(MICROTRUST_TEE_SUPPORT)))
          $(call dep-err-common, Please turn on MTK_IN_HOUSE_TEE_SUPPORT or TRUSTONIC_TEE_SUPPORT or MTK_GOOGLE_TRUSTY_SUPPORT or MICROTRUST_TEE_SUPPORT when MTK_SEC_VIDEO_PATH_SUPPORT set as yes)
        endif
      endif
    endif
  endif
endif

ifneq (yes,$(strip $(MTK_AUDIO_HD_REC_SUPPORT)))
  ifeq (yes,$(strip $(MTK_VOIP_ENHANCEMENT_SUPPORT)))
    $(call dep-err-ona-or-offb, MTK_AUDIO_HD_REC_SUPPORT, MTK_VOIP_ENHANCEMENT_SUPPORT)
  endif
  ifeq (yes,$(strip $(MTK_HANDSFREE_DMNR_SUPPORT)))
    $(call dep-err-ona-or-offb, MTK_AUDIO_HD_REC_SUPPORT, MTK_HANDSFREE_DMNR_SUPPORT)
  endif
endif

###############################################################
#for mp release  check release package OPTR_SPEC_SEG_DEF
#ifneq ($(filter OP01%,$(strip $(OPTR_SPEC_SEG_DEF))),)
#  ifneq ($(filter rel_customer_operator_cmcc, $(MTK_RELEASE_PACKAGE)),rel_customer_operator_cmcc)
#    $(call dep-err-common, please use rel_customer_operator_cmcc as optr release package in MTK_RELEASE_PACKAGE When OPTR_SEPEC_SEG_DEF set as OP01)
#  endif
#endif
#ifneq ($(filter OP02%,$(strip $(OPTR_SPEC_SEG_DEF))),)
#  ifneq ($(filter rel_customer_operator_cu, $(MTK_RELEASE_PACKAGE)),rel_customer_operator_cu)
#    $(call dep-err-common, please use rel_customer_operator_cu as optr release package in to MTK_RELEASE_PACKAGE When OPTR_SEPEC_SEG_DEF set as OP02)
#  endif
#endif
#ifneq ($(filter OP03%,$(strip $(OPTR_SPEC_SEG_DEF))),)
#  ifneq ($(filter rel_customer_operator_orange, $(MTK_RELEASE_PACKAGE)),rel_customer_operator_orange)
#    $(call dep-err-common, please use rel_customer_operator_orange as optr release package in to MTK_RELEASE_PACKAGE When OPTR_SEPEC_SEG_DEF set as OP03)
#  endif
#endif

#ifneq ($(filter OP06%,$(strip $(OPTR_SPEC_SEG_DEF))),)
#  ifneq ($(filter rel_customer_operator_vodafone, $(MTK_RELEASE_PACKAGE)),rel_customer_operator_vodafone)
#    $(call dep-err-common, please use rel_customer_operator_vodafone as optr release package in to MTK_RELEASE_PACKAGE When OPTR_SEPEC_SEG_DEF set as OP06)
#  endif
#endif
#ifneq ($(filter OP07%,$(strip $(OPTR_SPEC_SEG_DEF))),)
#  ifneq ($(filter rel_customer_operator_att, $(MTK_RELEASE_PACKAGE)),rel_customer_operator_att)
#    $(call dep-err-common, please use rel_customer_operator_att as optr release package in to MTK_RELEASE_PACKAGE When OPTR_SEPEC_SEG_DEF set as OP07)
#  endif
#endif
#ifneq ($(filter OP08%,$(strip $(OPTR_SPEC_SEG_DEF))),)
#  ifneq ($(filter rel_customer_operator_tmo_us, $(MTK_RELEASE_PACKAGE)),rel_customer_operator_tmo_us)
#    $(call dep-err-common, please use rel_customer_operator_tmo_us as optr release package in to MTK_RELEASE_PACKAGE When OPTR_SEPEC_SEG_DEF set as OP08)
#  endif
#endif
#ifneq ($(filter OP09%,$(strip $(OPTR_SPEC_SEG_DEF))),)
#  ifneq ($(filter rel_customer_operator_ct, $(MTK_RELEASE_PACKAGE)),rel_customer_operator_ct)
#    $(call dep-err-common, please use rel_customer_operator_ct as optr release package in to MTK_RELEASE_PACKAGE When OPTR_SEPEC_SEG_DEF set as OP09)
#  endif
#endif


#ifeq ($(strip $(MTK_BSP_PACKAGE)),yes)
#  ifneq ($(filter rel_customer_bsp, $(MTK_RELEASE_PACKAGE)),rel_customer_bsp)
#     $(call dep-err-common, please add rel_customer_bsp in to MTK_RELEASE_PACKAGE When MTK_BSP_PACKAGE is yes)
#  endif
#endif
#ifneq (,$(strip $(MTK_PLATFORM)))
#  ifeq ($(filter rel_customer_platform%,$(MTK_RELEASE_PACKAGE)),)
#     $(call dep-err-common, please add rel_customer_platform_xxx in to MTK_RELEASE_PACKAGE)
#  endif
#endif
#############################################################
ifeq (yes,$(strip $(MTK_AIV_SUPPORT)))
  ifneq (yes, $(strip $(MTK_DRM_PLAYREADY_SUPPORT)))
    $(call dep-err-ona-or-offb,MTK_AIV_SUPPORT,MTK_DRM_PLAYREADY_SUPPORT)
  endif
endif
################################################################
ifneq (yes, $(strip $(MTK_VOIP_ENHANCEMENT_SUPPORT)))
   ifeq (yes, $(strip $(MTK_VOIP_NORMAL_DMNR)))
       $(call dep-err-ona-or-offb,MTK_VOIP_ENHANCEMENT_SUPPORT,MTK_VOIP_NORMAL_DMNR)
    endif
   ifeq (yes, $(strip $(MTK_VOIP_HANDSFREE_DMNR)))
       $(call dep-err-ona-or-offb,MTK_VOIP_ENHANCEMENT_SUPPORT,MTK_VOIP_HANDSFREE_DMNR)
    endif
endif
################################################################
ifeq (yes,$(strip $(MTK_WFD_HDCP_TX_SUPPORT)))
  ifneq (yes, $(strip $(MTK_DRM_KEY_MNG_SUPPORT)))
    $(call dep-err-ona-or-offb,MTK_DRM_KEY_MNG_SUPPORT,MTK_WFD_HDCP_TX_SUPPORT)
  endif
  ifneq (yes, $(strip $(MTK_IN_HOUSE_TEE_SUPPORT)))
    $(call dep-err-ona-or-offb,MTK_IN_HOUSE_TEE_SUPPORT,MTK_WFD_HDCP_TX_SUPPORT)
  endif
endif

ifeq (yes,$(strip $(MTK_WFD_SINK_SUPPORT)))
  ifneq (yes,$(strip $(MTK_WFD_SUPPORT)))
    $(call dep-err-ona-or-offb,MTK_WFD_SUPPORT,MTK_WFD_SINK_SUPPORT)
  endif
else
  ifeq (yes,$(strip $(MTK_WFD_SINK_UIBC_SUPPORT)))
    $(call dep-err-ona-or-offb,MTK_WFD_SINK_SUPPORT,MTK_WFD_SINK_UIBC_SUPPORT)
  endif
endif
#######################################################################
ifeq (yes, $(strip $(MTK_SIM_HOT_SWAP_COMMON_SLOT)))
  ifneq (yes, $(strip $(MTK_SIM_HOT_SWAP)))
    $(call dep-err-ona-or-offb,MTK_SIM_HOT_SWAP,MTK_SIM_HOT_SWAP_COMMON_SLOT)
  endif
endif
################################################
ifeq (yes,$(strip $(MTK_EMMC_SUPPORT)))
  ifeq (yes,$(strip $(MTK_COMBO_NAND_SUPPORT)))
    $(call dep-err-offa-or-offb,MTK_EMMC_SUPPORT,MTK_COMBO_NAND_SUPPORT)
  endif
endif
#############################################################
ifeq (yes,$(strip $(MTK_DX_HDCP_SUPPORT)))
  ifneq (yes, $(strip $(MTK_PERSIST_PARTITION_SUPPORT)))
    $(call dep-err-ona-or-offb,MTK_PERSIST_PARTITION_SUPPORT,MTK_DX_HDCP_SUPPORT)
  endif
endif
#############################################################
ifeq (yes,$(strip $(MTK_EMMC_SUPPORT)))
  ifeq (yes,$(strip $(MTK_MLC_NAND_SUPPORT)))
    $(call dep-err-offa-or-offb,MTK_EMMC_SUPPORT,MTK_MLC_NAND_SUPPORT)
  endif
endif
############################################################
ifeq ($(strip $(SIM_ME_LOCK)),1)
  ifneq (yes,$(strip $(TRUSTONIC_TEE_SUPPORT)))
    $(call dep-err-common, please set SIM_ME_LOCK=0 or turn on TRUSTONIC_TEE_SUPPORT)
  endif
endif
###############################################################
ifneq (,$(filter 1 2,$(strip $(MTK_C2K_LTE_MODE))))
  ifeq (,$(filter C%, $(MTK_PROTOCOL1_RAT_CONFIG)))
    $(call dep-err-common, Please set MTK_PROTOCOL1_RAT_CONFIG as C% or set MTK_C2K_LTE_MODE as not 1 or not 2)
  endif
endif
###############################################################
RAT_CONFIG := Lf/Lt/W/T/G Lf/Lt/W/G Lf/W/G Lt/T/G Lf/Lt/T/G W/T/G W/G T/G G \
              C/Lf C/Lf/Lt/W/T/G C/Lf/Lt/W/G C/Lf/W/G C/Lf/Lt/G C/W/G Lf Lf/Lt no
ifeq (,$(strip $(MTK_PROTOCOL1_RAT_CONFIG)))
  $(call dep-err-common, MTK_PROTOCOL1_RAT_CONFIG must be set.)
else
  ifeq (,$(filter $(strip $(RAT_CONFIG)),$(strip $(MTK_PROTOCOL1_RAT_CONFIG))))
    $(call dep-err-common, MTK_PROTOCOL1_RAT_CONFIG = $(MTK_PROTOCOL1_RAT_CONFIG) is invalid, \
      the value should be Lf/Lt/W/T/G, Lf/Lt/W/G, Lf/W/G, Lt/T/G, Lf/Lt/T/G, W/T/G, W/G, T/G, G, \
      C/Lf, C/Lf/Lt/W/T/G, C/Lf/Lt/W/G, C/Lf/W/G, C/Lf/Lt/G, C/W/G Lf Lf/Lt or set to no.)
  endif
endif
###############################################################
ifeq ($(strip $(MTK_CAM_DEPTH_AF_SUPPORT)),yes)
  ifneq ($(strip $(MTK_CAM_STEREO_CAMERA_SUPPORT)),yes)
    $(call dep-err-ona-or-offb, MTK_CAM_STEREO_CAMERA_SUPPORT, MTK_CAM_DEPTH_AF_SUPPORT)
  endif
endif
ifeq ($(strip $(MTK_CAM_VSDOF_SUPPORT)),yes)
  ifneq ($(strip $(MTK_CAM_STEREO_CAMERA_SUPPORT)),yes)
    $(call dep-err-ona-or-offb, MTK_CAM_STEREO_CAMERA_SUPPORT, MTK_CAM_VSDOF_SUPPORT)
  endif
endif

ifeq (yes,$(strip $(MTK_CAM_STEREO_DENOISE_SUPPORT)))
  ifneq (yes,$(strip $(MTK_CAM_STEREO_CAMERA_SUPPORT)))
    $(call dep-err-ona-or-offb, MTK_CAM_STEREO_CAMERA_SUPPORT,MTK_CAM_STEREO_DENOISE_SUPPORT)
  endif
endif

###############################################################
ifeq (OP12_SPEC0200_SEGDEFAULT,$(OPTR_SPEC_SEG_DEF))
  ifneq (yes,$(strip $(MTK_IPV6_VZW)))
     $(call dep-err-common,Please set OPTR_SPEC_SEG_DEF as non OP12_SPEC0200_SEGDEFAULT or turn on MTK_IPV6_VZW)
  endif
endif

ifeq (OP12_SPEC0200_SEGTYPE3,$(OPTR_SPEC_SEG_DEF))
  ifneq (yes,$(strip $(MTK_IPV6_VZW)))
     $(call dep-err-common,Please set OPTR_SPEC_SEG_DEF as non OP12_SPEC0200_SEGTYPE3 or turn on MTK_IPV6_VZW)
  endif
endif
##############################################################
ifeq (yes,$(strip $(MTK_VOLTE_SUPPORT)))
  ifneq (yes,$(strip $(MTK_IMS_SUPPORT)))
    $(call dep-err-ona-or-offb,MTK_IMS_SUPPORT,MTK_VOLTE_SUPPORT)
  endif
endif
ifeq (yes,$(strip $(MTK_CT_VOLTE_SUPPORT)))
  ifneq (yes,$(strip $(MTK_IMS_SUPPORT)))
    $(call dep-err-ona-or-offb,MTK_IMS_SUPPORT,MTK_CT_VOLTE_SUPPORT)
  endif
endif
ifeq (yes,$(strip $(MTK_CT_VOLTE_SUPPORT)))
  ifneq (yes,$(strip $(MTK_VOLTE_SUPPORT)))
    $(call dep-err-ona-or-offb,MTK_VOLTE_SUPPORT,MTK_CT_VOLTE_SUPPORT)
  endif
endif
##############################################################
ifneq (yes,$(strip $(MTK_AAL_SUPPORT)))
  ifeq (yes,$(strip $(MTK_ULTRA_DIMMING_SUPPORT)))
    $(call dep-err-ona-or-offb, MTK_AAL_SUPPORT, MTK_ULTRA_DIMMING_SUPPORT)
  endif
endif

ifeq (yes,$(strip $(MTK_DRE30_SUPPORT)))
  ifneq (yes,$(strip $(MTK_AAL_SUPPORT)))
    $(call dep-err-ona-or-offb, MTK_AAL_SUPPORT,MTK_DRE30_SUPPORT)
  endif
endif

ifeq (mt8321,$(strip $(MTK_TABLET_HARDWARE)))
  ifneq (yes,$(strip $(MTK_PLATFORM_OPTIMIZE)))
    $(call dep-err-common,Please turn on MTK_PLATFORM_OPTIMIZE)
  endif
  ifneq (yes,$(strip $(MTK_HW_ENHANCE)))
    $(call dep-err-common,Please turn on MTK_HW_ENHANCE)
  endif
endif

###############################################################
#                                MODEM_MTK_AUDIO_HIERARCHICAL_PARAM_SUPPORT
#                               +-----------------------------------------+
#                               |                  | FALSE | TRUE | EMPTY |
#                               |=========================================|
#                               |        V1        |   V   |   X  |   V   |
#                               |------------------+-------+------+-------|
#                               | Starting with V  |   X   |   V  |   X   |
#                               | but not V1       |       |      |       |
#                               |------------------+-------+------+-------|
# MTK_AUDIO_TUNING_TOOL_VERSION |       Empty      |   V   |   X  |   V   |
#                               |------------------+-------+------+-------+
#                               |       Others     |   X   |   X  |   X   |
#                               +-----------------------------------------+
#
ifneq ($(mtk_modem_file),)
ifeq (,$(strip $(MTK_AUDIO_TUNING_TOOL_VERSION)))
  ifneq ($(filter TRUE,$(strip $(MODEM_MTK_AUDIO_HIERARCHICAL_PARAM_SUPPORT))),)
    $(call dep-err-common,MODEM_MTK_AUDIO_HIERARCHICAL_PARAM_SUPPORT cannot be TRUE when MTK_AUDIO_TUNING_TOOL_VERSION is not defined)
  endif
else
  ifeq (V1,$(strip $(MTK_AUDIO_TUNING_TOOL_VERSION)))
    ifneq ($(filter TRUE,$(strip $(MODEM_MTK_AUDIO_HIERARCHICAL_PARAM_SUPPORT))),)
      $(call dep-err-common,MODEM_MTK_AUDIO_HIERARCHICAL_PARAM_SUPPORT cannot be TRUE when MTK_AUDIO_TUNING_TOOL_VERSION is $(strip $(MTK_AUDIO_TUNING_TOOL_VERSION)))
    endif
  else
    ifneq (,$(shell echo '$(strip $(MTK_AUDIO_TUNING_TOOL_VERSION))' | egrep '^V[0-9]+(\.[0-9]+)?'))
      ifeq ($(filter TRUE,$(strip $(MODEM_MTK_AUDIO_HIERARCHICAL_PARAM_SUPPORT))),)
        $(call dep-err-common,MODEM_MTK_AUDIO_HIERARCHICAL_PARAM_SUPPORT must be TRUE instead of "$(MODEM_MTK_AUDIO_HIERARCHICAL_PARAM_SUPPORT)" when MTK_AUDIO_TUNING_TOOL_VERSION is $(strip $(MTK_AUDIO_TUNING_TOOL_VERSION)))
      endif
    else
        $(call dep-err-common,Invalid MTK_AUDIO_TUNING_TOOL_VERSION $(strip $(MTK_AUDIO_TUNING_TOOL_VERSION)))
    endif
  endif
endif
endif # ifneq ($(mtk_modem_file),)
###############################################################
ifeq (yes,$(strip $(MTK_BLULIGHT_DEFENDER_SUPPORT)))
  ifeq (,$(filter DISP DISP_MDP,$(strip $(MTK_PQ_COLOR_MODE))))
    $(call dep-err-common,MTK_PQ_COLOR_MODE must be DISP or DISP_MDP instead of "$(strip $(MTK_PQ_COLOR_MODE))" when MTK_BLULIGHT_DEFENDER_SUPPORT is yes)
  endif
endif
###############################################################
ifeq (yes,$(strip $(MTK_WFC_SUPPORT)))
  ifneq (yes,$(strip $(MTK_IMS_SUPPORT)))
    $(call dep-err-ona-or-offb,MTK_IMS_SUPPORT,MTK_WFC_SUPPORT)
  endif
endif
###############################################################
ifeq (,$(strip $(MTK_MULTI_SIM_SUPPORT)))
  $(call dep-err-common,MTK_MULTI_SIM_SUPPORT cannot be empty)
endif

ifeq (,$(strip $(MTK_NUM_MODEM_PROTOCOL)))
  $(call dep-err-common,MTK_NUM_MODEM_PROTOCOL cannot be empty)
endif
###############################################################
ifneq (,$(strip $(MTK_MULTIPLE_IMS_SUPPORT)))
  ifneq (1,$(strip $(MTK_MULTIPLE_IMS_SUPPORT)))
    ifneq (yes,$(strip $(MTK_IMS_SUPPORT)))
      $(call dep-err-ona-or-offb,MTK_IMS_SUPPORT,MTK_MULTIPLE_IMS_SUPPORT)
    endif
    ifneq (yes,$(strip $(MTK_MULTI_PS_SUPPORT)))
      $(call dep-err-ona-or-offb,MTK_MULTI_PS_SUPPORT,MTK_MULTIPLE_IMS_SUPPORT)
    endif
  endif
endif
ifeq (yes,$(strip $(MTK_EXTERNAL_SIM_RSIM_ENHANCEMENT)))
  ifneq (yes,$(strip $(MTK_MULTI_PS_SUPPORT)))
    $(call dep-err-ona-or-offb,MTK_MULTI_PS_SUPPORT,MTK_EXTERNAL_SIM_RSIM_ENHANCEMENT)
  endif
  ifneq (yes,$(strip $(MTK_EXTERNAL_SIM_SUPPORT)))
    $(call dep-err-ona-or-offb,MTK_EXTERNAL_SIM_SUPPORT,MTK_EXTERNAL_SIM_RSIM_ENHANCEMENT)
  endif
  ifdef MODEM_RSIM_SINGLE_RF_SUPPORT
  ifneq (TRUE,$(strip $(MODEM_RSIM_SINGLE_RF_SUPPORT)))
    $(call dep-err-common,MTK_EXTERNAL_SIM_RSIM_ENHANCEMENT cannot be enabled when MODEM RSIM_SINGLE_RF_SUPPORT disabled)
  endif
  endif
endif
###############################################################
ifeq (yes,$(strip $(MTK_MULTI_PS_SUPPORT)))
  ifneq (,$(filter $(strip MT6291%),$(strip $(MODEM_MTK_MODEM_ARCH))))
    ifneq (MT6291P,$(filter $(strip MT6291%),$(strip $(MODEM_MTK_MODEM_ARCH))))
      ifneq (Lf/Lt/W/G, $(strip $(MTK_PROTOCOL1_RAT_CONFIG)))
        $(call dep-err-common, [91 MPS] Only support 4M project please check MTK_PROTOCOL1_RAT_CONFIG.)
      endif
      ifneq (TRUE, $(strip $(MODEM_MULTIPLE_PS)))
        $(call dep-err-common, [91 MPS] Modem MPS option is off.)
      endif
    endif
  endif

  ifneq (,$(filter $(strip MT6291P) $(strip MT6292%),$(strip $(MODEM_MTK_MODEM_ARCH))))
    ifeq (1, $(strip $(MODEM_GEMINI)),)
      $(call dep-err-common, [92 MPS] Gemini must greater than 1)
    endif
  endif

  ifneq (,$(filter $(strip MT6293%),$(strip $(MODEM_MTK_MODEM_ARCH))))
    ifneq (TRUE, $(strip $(MODEM_MULTIPLE_PS)))
      $(call dep-err-common, [93 MPS] Modem Multiple PS is off)
     endif
  endif
endif

ifneq (yes,$(strip $(MTK_MULTI_PS_SUPPORT)))
  ifneq (,$(filter $(strip MT6293%),$(strip $(MODEM_MTK_MODEM_ARCH))))
    ifeq (TRUE, $(strip $(MODEM_MULTIPLE_PS)))
      $(call dep-err-common, [93 MPS] Should always turn on MPS in 93 R2 project)
    endif
  endif
endif
###############################################################
ifeq (yes,$(strip $(MTK_GMO_RAM_OPTIMIZE)))
  ifeq (no,$(strip $(MTK_HIDL_PROCESS_CONSOLIDATION_ENABLED)))
    $(call dep-err-ona-or-offb,MTK_HIDL_PROCESS_CONSOLIDATION_ENABLED,MTK_GMO_RAM_OPTIMIZE)
  endif
endif
###############################################################
ifeq (yes,$(strip $(MTK_VOW_BARGE_IN_SUPPORT)))
  ifneq (yes,$(strip $(MTK_VOW_SUPPORT)))
    $(call dep-err-ona-or-offb,MTK_VOW_SUPPORT,MTK_VOW_BARGE_IN_SUPPORT)
  endif
endif

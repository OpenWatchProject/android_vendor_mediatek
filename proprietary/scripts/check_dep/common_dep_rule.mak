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

#################################################################
# dependency between AST TD modem & MMSYS1
##################################################################
ifeq (AST_TL1_TDD, $(strip $(MODEM_L1_3GSOLUTION)))
  ifneq (yes,$(strip $(MTK_ASTBSP_SUPPORT)))
    # MMSYS1 is required to be powered on when using AST TD modem
    # We, here, should reminder customer to change modem
    # NOT confuse them to change the option setting in modem.mk
    $(call dep-err-seta-or-onb,CUSTOM_MODEM,non $(CUSTOM_MODEM),MTK_ASTBSP_SUPPORT)
  endif
endif

##############################################################
# for resolution check

ifeq (MT6573,$(strip $(MTK_PLATFORM)))
  ifneq (,$(strip $(LCM_WIDTH)))
    ifeq ($(call gt,$(LCM_WIDTH),320),T)
      ifneq (,$(strip $(LCM_HEIGHT)))
        ifeq ($(call gt,$(LCM_HEIGHT),480),T)
          ifeq (2G,$(strip $(CUSTOM_DRAM_SIZE)))
            $(call dep-err-common, resolution should not be higher than HVGA(320*480) when CUSTOM_DRAM_SIZE=$(CUSTOM_DRAM_SIZE))
          endif
        endif
      endif
    endif
  endif
endif

##############################################################
# for mtk sec modem

ifeq (yes,$(strip $(MTK_SEC_MODEM_AUTH)))
  ifeq (no,$(strip $(MTK_SEC_MODEM_ENCODE)))
    $(call dep-err-ona-or-offb, MTK_SEC_MODEM_ENCODE, MTK_SEC_MODEM_AUTH)
  endif
endif


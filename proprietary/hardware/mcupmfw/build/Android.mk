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


# Copyright (C) 2008 The Android Open Source Project
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#      http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

LOCAL_PATH := $(call my-dir)
MCUPMFW_BUILD_DIR := $(LOCAL_PATH)

include $(call all-makefiles-under,$(LOCAL_PATH))

ifeq (yes,$(strip $(MCUPM_FW_USE_PARTITION)))
# Restore LOCAL_PATH
include $(CLEAR_VARS)
LOCAL_PATH := $(MCUPMFW_BUILD_DIR)

MCUPM_FW_IMAGE_BASENAME := mcupmfw
MCUPM_FW_IMAGE_BIN_NH   := $(MCUPM_FW_IMAGE_BASENAME)-nh.img
MCUPM_FW_IMAGE_BIN      := $(MCUPM_FW_IMAGE_BASENAME).img
MCUPM_FW_IMAGE_BUILT    := $(PRODUCT_OUT)/$(MCUPM_FW_IMAGE_BIN)
MCUPM_FW_LIST_MK        := $(MCUPMFW_ROOT_DIR)/$(MTK_PLATFORM_DIR)/$(MCUPM_FW_IMAGE_BASENAME).mk
MCUPM_FW_MKSPMIMAGE     := $(LOCAL_PATH)/common/mkspmimage
MCUPM_FW_MKSPMIMAGE_OPT := pack
MCUPM_FW_MKIMAGE        := $(LOCAL_PATH)/common/mkimage

ifeq (,$(wildcard $(MCUPM_FW_LIST_MK)))
  $(error $(TINYSYS_SSPM): MCUPM firmware definition file $(MCUPM_FW_LIST_MK) is missing)
endif

include $(MCUPM_FW_LIST_MK)

LOCAL_MODULE := $(MCUPM_FW_IMAGE_BIN)
LOCAL_MODULE_CLASS := ETC
LOCAL_MODULE_TAGS := optional
LOCAL_MULTILIB := first
LOCAL_MODULE_PATH := $(PRODUCT_OUT)
LOCAL_SRC_FILES := $(addprefix $(dir $(MCUPM_FW_LIST_MK)),$(MCUPM_FIRMWARE_LIST))

include $(BUILD_SYSTEM)/base_rules.mk

LOCAL_GENERATED_SOURCES := $(addprefix $(intermediates)/$(MTK_PLATFORM_DIR)/,$(MCUPM_FIRMWARE_LIST))

$(LOCAL_GENERATED_SOURCES): $(intermediates)/% : $(MCUPMFW_ROOT_DIR)/% | $(ACP)
	@echo "Copy: $@"
	$(copy-file-to-target)

MCUPM_FW_BIN_HEADER_CFG   := $(intermediates)/img_hdr_$(MCUPM_FW_IMAGE_BASENAME).cfg
$(MCUPM_FW_BIN_HEADER_CFG): BASENAME := $(MCUPM_FW_IMAGE_BASENAME)
$(MCUPM_FW_BIN_HEADER_CFG): $(MCUPM_FW_LIST_MK) $(LOCAL_PATH)/Android.mk
	@echo 'MCUPM_FW: GEN   $@'
	@mkdir -p $(dir $@)
	$(hide)echo 'NAME = $(BASENAME)' > $@

$(LOCAL_BUILT_MODULE): BIN_NH := $(intermediates)/$(MCUPM_FW_IMAGE_BIN_NH)
$(LOCAL_BUILT_MODULE): SRCS   := $(LOCAL_GENERATED_SOURCES)
$(LOCAL_BUILT_MODULE): HCFG   := $(MCUPM_FW_BIN_HEADER_CFG)
$(LOCAL_BUILT_MODULE): \
  $(MCUPM_FW_LIST_MK) $(MCUPM_FW_BIN_HEADER_CFG) $(LOCAL_GENERATED_SOURCES) \
  | $(MCUPM_FW_MKSPMIMAGE) $(MCUPM_FW_MKIMAGE)
	@echo 'MCUPM_FW: Build $@'
	@mkdir -p $(dir $@)
	$(hide)$(MCUPM_FW_MKSPMIMAGE) $(MCUPM_FW_MKSPMIMAGE_OPT) $(SRCS) > $(BIN_NH)
	$(hide)$(MCUPM_FW_MKIMAGE) $(BIN_NH) $(HCFG) > $@

endif # MCUPM_FW_USE_PARTITION is yes

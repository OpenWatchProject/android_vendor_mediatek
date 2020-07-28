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
SPMFW_BUILD_DIR := $(LOCAL_PATH)

include $(call all-makefiles-under,$(LOCAL_PATH))

ifeq (yes,$(strip $(SPM_FW_USE_PARTITION)))
include $(CLEAR_VARS)
LOCAL_PATH := $(SPMFW_BUILD_DIR)

SPM_FW_IMAGE_BASENAME := spmfw
SPM_FW_IMAGE_BIN_NH   := $(SPM_FW_IMAGE_BASENAME)-nh.img
SPM_FW_IMAGE_BIN      := $(SPM_FW_IMAGE_BASENAME).img
SPM_FW_IMAGE_BUILT    := $(PRODUCT_OUT)/$(SPM_FW_IMAGE_BIN)
SPM_FW_LIST_MK        := $(SPMFW_ROOT_DIR)/$(MTK_PLATFORM_DIR)/$(SPM_FW_IMAGE_BASENAME).mk
SPM_FW_MKSPMIMAGE     := $(LOCAL_PATH)/common/mkspmimage
SPM_FW_MKSPMIMAGE_OPT := pack
SPM_FW_MKIMAGE        := $(LOCAL_PATH)/common/mkimage

ifeq (,$(wildcard $(SPM_FW_LIST_MK)))
  $(error SPM_FW: SPM firmware definition file $(SPM_FW_LIST_MK) is missing)
endif

include $(SPM_FW_LIST_MK)

LOCAL_MODULE := $(SPM_FW_IMAGE_BIN)
LOCAL_MODULE_CLASS := ETC
LOCAL_MODULE_TAGS := optional
LOCAL_MULTILIB := first
LOCAL_MODULE_PATH := $(PRODUCT_OUT)
LOCAL_SRC_FILES := $(addprefix $(dir $(SPM_FW_LIST_MK)),$(SPM_FIRMWARE_LIST))

include $(BUILD_SYSTEM)/base_rules.mk

LOCAL_GENERATED_SOURCES := $(addprefix $(intermediates)/$(MTK_PLATFORM_DIR)/,$(SPM_FIRMWARE_LIST))

$(LOCAL_GENERATED_SOURCES): $(intermediates)/% : $(SPMFW_ROOT_DIR)/% | $(ACP)
	@echo "Copy: $@"
	$(copy-file-to-target)

SPM_FW_BIN_HEADER_CFG   := $(intermediates)/img_hdr_$(SPM_FW_IMAGE_BASENAME).cfg
$(SPM_FW_BIN_HEADER_CFG): BASENAME := $(SPM_FW_IMAGE_BASENAME)
$(SPM_FW_BIN_HEADER_CFG): $(SPM_FW_LIST_MK) $(LOCAL_PATH)/Android.mk
	@echo 'SPM_FW: GEN   $@'
	@mkdir -p $(dir $@)
	$(hide)echo 'NAME = $(BASENAME)' > $@

$(LOCAL_BUILT_MODULE): BIN_NH := $(intermediates)/$(SPM_FW_IMAGE_BIN_NH)
$(LOCAL_BUILT_MODULE): SRCS   := $(LOCAL_GENERATED_SOURCES)
$(LOCAL_BUILT_MODULE): HCFG   := $(SPM_FW_BIN_HEADER_CFG)
$(LOCAL_BUILT_MODULE): \
  $(SPM_FW_LIST_MK) $(SPM_FW_BIN_HEADER_CFG) $(LOCAL_GENERATED_SOURCES) \
  | $(SPM_FW_MKSPMIMAGE) $(SPM_FW_MKIMAGE)
	@echo 'SPM_FW: Build $@'
	@mkdir -p $(dir $@)
	$(hide)$(SPM_FW_MKSPMIMAGE) $(SPM_FW_MKSPMIMAGE_OPT) $(SRCS) > $(BIN_NH)
	$(hide)$(SPM_FW_MKIMAGE) $(BIN_NH) $(HCFG) > $@

endif # SPM_FW_USE_PARTITION is yes

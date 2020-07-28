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

ifneq ($(strip $(MTK_GPU_SUPPORT)),no)

LOCAL_PATH := $(my-dir)

define GPU_INSTALL_FIRM
include $$(CLEAR_VARS)
LOCAL_MULTILIB := $(1)
LOCAL_MODULE := $$(notdir $(2))
LOCAL_MODULE := $$(notdir $$(LOCAL_MODULE:.22.68.54.30=))
LOCAL_SRC_FILES := $(2)
LOCAL_MODULE_CLASS := EXECUTABLES
LOCAL_PROPRIETARY_MODULE := true
LOCAL_MODULE_OWNER := mtk
LOCAL_MODULE_PATH := $(TARGET_OUT_VENDOR)/firmware
include $$(BUILD_PREBUILT)
endef

define HAL_INSTALL_LIB
include $$(CLEAR_VARS)
LOCAL_MULTILIB := $(1)
LOCAL_MODULE := $$(notdir $(2:.so=))
LOCAL_MODULE := $$(notdir $$(LOCAL_MODULE:.rogue=.$(TARGET_BOARD_PLATFORM)))
LOCAL_SRC_FILES := $(2)
LOCAL_MODULE_CLASS := SHARED_LIBRARIES
LOCAL_MODULE_SUFFIX := .so
LOCAL_PROPRIETARY_MODULE := true
LOCAL_MODULE_OWNER := mtk
LOCAL_MODULE_RELATIVE_PATH := $(3)
LOCAL_POST_INSTALL_CMD = $(4)
include $$(BUILD_PREBUILT)
endef

define GPU_INSTALL_LIB
include $$(CLEAR_VARS)
LOCAL_MULTILIB := $(1)
LOCAL_MODULE := $$(notdir $(2:.so=))
LOCAL_SRC_FILES := $(2)
LOCAL_MODULE_CLASS := SHARED_LIBRARIES
LOCAL_MODULE_SUFFIX := .so
LOCAL_PROPRIETARY_MODULE := true
LOCAL_MODULE_OWNER := mtk
LOCAL_MODULE_RELATIVE_PATH := $(3)
LOCAL_POST_INSTALL_CMD = $(4)
include $$(BUILD_PREBUILT)
endef

define MEMTRACK_INSTALL_LIB
include $$(CLEAR_VARS)
LOCAL_MULTILIB := $(1)
LOCAL_MODULE := $$(notdir $(2:.so=))
LOCAL_SRC_FILES := $(2)
LOCAL_MODULE_CLASS := SHARED_LIBRARIES
LOCAL_MODULE_SUFFIX := .so
LOCAL_PROPRIETARY_MODULE := true
LOCAL_MODULE_OWNER := mtk
LOCAL_MODULE_RELATIVE_PATH := $(3)
LOCAL_POST_INSTALL_CMD = $(4)
include $$(BUILD_PREBUILT)
endef

define GPU_INSTALL_LIBS
$(call HAL_INSTALL_LIB,$(1),$(2)/gralloc.rogue.so,hw)
$(call MEMTRACK_INSTALL_LIB,$(1),$(2)/libmemtrack_GL.so)
$(call HAL_INSTALL_LIB,$(1),$(2)/vulkan.rogue.so,hw)
ifeq ($(MTK_HWC_SUPPORT),no)
#if MTK_HWC is disable, we install the IMG's HWC.
endif
$(call GPU_INSTALL_FIRM,$(1),$(2)/rgx.fw.22.68.54.30)
$(call GPU_INSTALL_LIB,$(1),$(2)/libEGL_mtk.so,egl)
$(call GPU_INSTALL_LIB,$(1),$(2)/libGLESv1_CM_mtk.so,egl)
$(call GPU_INSTALL_LIB,$(1),$(2)/libGLESv2_mtk.so,egl)
$(call GPU_INSTALL_LIB,$(1),$(2)/libIMGegl.so)
$(call GPU_INSTALL_LIB,$(1),$(2)/libglslcompiler.so)
$(call GPU_INSTALL_LIB,$(1),$(2)/libPVRScopeServices.so)
$(call GPU_INSTALL_LIB,$(1),$(2)/libsrv_um.so)
$(call GPU_INSTALL_LIB,$(1),$(2)/libusc.so)
$(call GPU_INSTALL_LIB,$(1),$(2)/libtqvalidate.so)
$(call GPU_INSTALL_LIB,$(1),$(2)/libmpvr.so)
# For OCL
$(call GPU_INSTALL_LIB,$(1),$(2)/libPVROCL.so)
$(call GPU_INSTALL_LIB,$(1),$(2)/libufwriter.so)
endef

# Setup GPU profile
VER := $(if $(filter eng,$(TARGET_BUILD_VARIANT)),eng,user)

# Sanity check: Modify the following mapping data to support more ARCHs if need.
$(if $(filter-out arm arm64,$(TARGET_ARCH) $(TARGET_2ND_ARCH)),$(error "GPU binary only support arm or arm64 currently"))

# Map ARCH to value /(32/64)/ for LOCAL_MULTILIB
GPU_ARCH_arm := 32
GPU_ARCH_arm64 := 64
# Map ARCH to /(eng|user)(64)?/ folder
GPU_VER_arm := $(VER)
GPU_VER_arm64 := $(VER)64

# Install shared libraries for the primary arch
$(eval $(call GPU_INSTALL_LIBS,$(GPU_ARCH_$(TARGET_ARCH)),$(GPU_VER_$(TARGET_ARCH))))

# Install only shared libraries for the 2nd arch
ifneq ($(TARGET_2ND_ARCH),)
$(eval $(call GPU_INSTALL_LIBS,$(GPU_ARCH_$(TARGET_2ND_ARCH)),$(GPU_VER_$(TARGET_2ND_ARCH))))
endif

endif # MTK_GPU_SUPPORT

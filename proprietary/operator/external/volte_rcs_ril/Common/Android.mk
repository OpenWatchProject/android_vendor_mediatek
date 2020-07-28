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


LOCAL_PATH:= $(call my-dir)

###############################
# librcs_interface_includes library
###############################

include $(CLEAR_VARS)

LOCAL_MODULE := librcs_interface_includes
LOCAL_EXPORT_C_INCLUDE_DIRS := $(LOCAL_PATH)/include

include $(BUILD_HEADER_LIBRARY)

###############################
# librcs_interface library
###############################

include $(CLEAR_VARS)
LOCAL_MODULE            := librcs_interface
LOCAL_PROPRIETARY_MODULE := true
LOCAL_MODULE_OWNER := mtk
LOCAL_MODULE_TAGS       := optional

# Build for 32-bits
LOCAL_MULTILIB := 32

API_COMMON_SRC_FILES    := src/common/api_string.c     \

API_OS_SRC_FILES        := src/os/api_btimer.c         \
                           src/os/api_channel.c        \
                           src/os/api_mem.c            \
                           src/os/api_msgq.c           \
                           src/os/api_mutex.c          \
                           src/os/api_task.c           \

INTERFACE_SRC_FILES     := src/interface/volte_cap.c   \
                           src/interface/volte_stack.c \

LOCAL_SRC_FILES         := $(API_COMMON_SRC_FILES)     \
                           $(API_OS_SRC_FILES)         \
                           $(INTERFACE_SRC_FILES)      \

LOCAL_ARM_MODE          := arm

LOCAL_HEADER_LIBRARIES  += librcs_interface_includes

LOCAL_CFLAGS            += -D __ANDROID__ -O0 -g

# if it is userdebug and user load, add the flag
ifneq (,$(filter user,$(TARGET_BUILD_VARIANT)))
    LOCAL_CFLAGS += -D__PRODUCTION_RELEASE__
endif

LOCAL_SHARED_LIBRARIES  += libc libcutils libutils libdl liblog
LOCAL_PRELINK_MODULE    := false

LOCAL_SHARED_LIBRARIES  += libhardware libhardware_legacy

include $(MTK_SHARED_LIBRARY)


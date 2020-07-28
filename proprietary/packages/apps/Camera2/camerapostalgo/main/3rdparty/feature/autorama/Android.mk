# Copyright Statement:
#
# This software/firmware and related documentation ("MediaTek Software") are
# protected under relevant copyright laws. The information contained herein
# is confidential and proprietary to MediaTek Inc. and/or its licensors.
# Without the prior written permission of MediaTek inc. and/or its licensors,
# any reproduction, modification, use or disclosure of MediaTek Software,
# and information contained herein, in whole or in part, shall be strictly prohibited.

# MediaTek Inc. (C) 2018. All rights reserved.
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

#-----------------------------------------------------------
LOCAL_HEADER_LIBRARIES += postalgo_featurepipe_headers
LOCAL_HEADER_LIBRARIES += postalgo_main_headers
LOCAL_HEADER_LIBRARIES += camerapostalgo_headers
LOCAL_HEADER_LIBRARIES += 3rdparty_plugin_headers
LOCAL_HEADER_LIBRARIES += feature_utils_headers
LOCAL_HEADER_LIBRARIES += libpostalgo_autorama_headers
LOCAL_HEADER_LIBRARIES += libpostalgo_motion_headers

#-----------------------------------------------------------
LOCAL_SRC_FILES += $(call all-cpp-files-under, )
LOCAL_SRC_FILES += $(call all-cpp-files-under, ../utils)
$(info "LOCAL_SRC_FILES = $(LOCAL_SRC_FILES))

#-----------------------------------------------------------
LOCAL_CFLAGS += -Wall -Werror -Wno-unused-parameter -DGL_GLEXT_PROTOTYPES -DEGL_EGLEXT_PROTOTYPES

#-----------------------------------------------------------
LOCAL_STATIC_LIBRARIES +=

LOCAL_WHOLE_STATIC_LIBRARIES +=

#-----------------------------------------------------------
LOCAL_SHARED_LIBRARIES += liblog
LOCAL_SHARED_LIBRARIES += libutils
LOCAL_SHARED_LIBRARIES += libcutils
LOCAL_SHARED_LIBRARIES += libhidlbase
LOCAL_SHARED_LIBRARIES += libandroid
LOCAL_SHARED_LIBRARIES += vendor.mediatek.hardware.mms@1.0
LOCAL_SHARED_LIBRARIES += vendor.mediatek.hardware.mms@1.1
LOCAL_SHARED_LIBRARIES += vendor.mediatek.hardware.mms@1.2
LOCAL_SHARED_LIBRARIES += libEGL
LOCAL_SHARED_LIBRARIES += libGLESv2
LOCAL_SHARED_LIBRARIES += libpostalgo_stdutils
LOCAL_SHARED_LIBRARIES += libpostalgo_imgbuf
LOCAL_SHARED_LIBRARIES += libpostalgo_metadata
LOCAL_SHARED_LIBRARIES += libnativewindow
LOCAL_SHARED_LIBRARIES += libpostalgo_3rdparty.core
LOCAL_SHARED_LIBRARIES += libcamalgo_sys.autorama
$(info "LOCAL_SHARED_LIBRARIES = $(LOCAL_SHARED_LIBRARIES))

#-----------------------------------------------------------
LOCAL_MODULE := libpostalgo.plugin.autorama
#LOCAL_MULTILIB := 64
LOCAL_PROPRIETARY_MODULE := false
LOCAL_MODULE_TAGS := optional
include $(BUILD_SHARED_LIBRARY)

################################################################################
#
#################################################################################
include $(call all-makefiles-under,$(LOCAL_PATH))

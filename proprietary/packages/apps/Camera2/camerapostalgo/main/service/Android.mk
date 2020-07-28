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
# MERCHANTABILITY, FTNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
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
#-----------------------------------------------------------

include $(CLEAR_VARS)

LOCAL_HEADER_LIBRARIES += postalgo_main_headers
LOCAL_HEADER_LIBRARIES += camerapostalgo_headers
LOCAL_HEADER_LIBRARIES += postalgo_main_core_headers
LOCAL_HEADER_LIBRARIES += postalgo_featurepipe_headers
LOCAL_HEADER_LIBRARIES += 3rdparty_plugin_headers

LOCAL_SRC_FILES += service.cpp

LOCAL_CFLAGS += -Wall -Werror
# suppress existing non-critical warnings
LOCAL_CFLAGS += \
        -Wno-gnu-static-float-init \
        -Wno-non-literal-null-conversion \
        -Wno-self-assign \
        -Wno-unused-parameter \
        -Wno-unused-variable \
        -Wno-unused-function \
        -Wno-macro-redefined \
        -Wno-unused-private-field \

#-----------------------------------------------------------
LOCAL_SHARED_LIBRARIES += \
    liblog \
    libui \
    libgui \
    libutils \
    libcutils \
    libbinder \
    libcampostalgo \
    libcampostalgo_interface \


#-----------------------------------------------------------
LOCAL_INIT_RC := camerapostalgo.rc
LOCAL_MODULE := camerapostalgo
LOCAL_MODULE_OWNER := mtk
LOCAL_MULTILIB := first
$(info ========= CamPostAlgo building $(LOCAL_MODULE))
#-----------------------------------------------------------
include $(BUILD_EXECUTABLE)

include $(call all-makefiles-under,$(LOCAL_PATH))


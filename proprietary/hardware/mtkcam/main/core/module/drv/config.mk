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
ifneq (,$(filter $(strip $(TARGET_BOARD_PLATFORM)), none))
# if not support modify 'none' to platform name
else
#-----------------------------------------------------------
# HalSensor
#-----------------------------------------------------------
PLATFORMS_USING_FAKE_SENSOR := # mt6797
ifeq (,$(filter $(strip $(TARGET_BOARD_PLATFORM)), $(PLATFORMS_USING_FAKE_SENSOR)))
MY_LOCAL_SHARED_LIBRARIES += libcam.halsensor
MY_LOCAL_SRC_FILES += register_HalSensor.cpp
MY_LOCAL_C_INCLUDES +=
MY_LOCAL_CFLAGS +=
else
MY_LOCAL_SHARED_LIBRARIES += libcam.iopipe
MY_LOCAL_SRC_FILES += register_FakeSensor.cpp
MY_LOCAL_C_INCLUDES +=
MY_LOCAL_CFLAGS +=
endif

#-----------------------------------------------------------
# HWSyncDrv
#-----------------------------------------------------------
MY_LOCAL_SHARED_LIBRARIES += libcam.halsensor
MY_LOCAL_SRC_FILES += register_HwSyncDrv.cpp
MY_LOCAL_C_INCLUDES +=
MY_LOCAL_CFLAGS +=

#-----------------------------------------------------------
# INormalPipeModule
#-----------------------------------------------------------
MY_LOCAL_SHARED_LIBRARIES += libcam.iopipe
MY_LOCAL_SRC_FILES += register_iopipe_CamIO_NormalPipe.cpp
MY_LOCAL_C_INCLUDES +=
MY_LOCAL_CFLAGS +=

#-----------------------------------------------------------
#
#-----------------------------------------------------------

endif ### (,$(filter $(strip $(TARGET_BOARD_PLATFORM)), ...))

################################################################################
#
################################################################################

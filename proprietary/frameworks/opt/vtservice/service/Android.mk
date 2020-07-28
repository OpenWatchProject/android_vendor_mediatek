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

############################################################

build_vilte =
ifeq ($(strip $(MSSI_MTK_VILTE_SUPPORT)),yes)
    build_vilte = yes
endif

ifeq ($(strip $(MSSI_MTK_VIWIFI_SUPPORT)),yes)
    build_vilte = yes
endif

ifdef build_vilte

LOCAL_PATH:= $(call my-dir)

############################################################
include $(CLEAR_VARS)

LOCAL_PRELINK_MODULE := false

FRAMEWORKS_NATIVE_INLCUDE_GUI_PATH=frameworks/native/include/gui
FRAMEWORKS_NATIVE_INLCUDE_PATH=frameworks/native/include
FRAMEWORKS_MEDIA_INLCUDE_PATH=frameworks/av/include/media/stagefright/foundation
FRAMEWORKS_PATH=frameworks/av

AVSYNC_PATH:= vendor/mediatek/proprietary/frameworks/opt/vtservice/avsync/include

IMS_MD_INC=$(TOP)/vendor/mediatek/proprietary/hardware/ims/include/volte_header/volte_imcb/sap

HAL_PATH:= vendor/mediatek/proprietary/frameworks/opt/vtservice/hal/inc

IMSMA_INC1=vendor/mediatek/proprietary/frameworks/opt/libimsma/imsma
IMSMA_INC2=frameworks/native/include/binder
RTP_INC=vendor/mediatek/proprietary/frameworks/opt/libimsma/rtp/include
SOURCE_INC=vendor/mediatek/proprietary/frameworks/opt/libimsma/source
SINK_INC=vendor/mediatek/proprietary/frameworks/opt/libimsma/sink

RIL_INC_PATH:= vendor/mediatek/proprietary/hardware/ril/include
RIL_INC_PATH2:= vendor/mediatek/proprietary/hardware/ril/fusion/include

SERVICE_INC_PATH:= $(LOCAL_PATH)/inc
SERVICE_SRC_PATH:= src

LOCAL_LDLIBS :=  -L$(SYSROOT)/usr/lib -llog

LOCAL_C_INCLUDES += \
  $(SERVICE_INC_PATH) \
  $(JNI_H_INCLUDE) \
  $(FRAMEWORKS_MEDIA_INLCUDE_PATH) \
  $(FRAMEWORKS_NATIVE_INLCUDE_GUI_PATH) \
  $(FRAMEWORKS_NATIVE_INLCUDE_PATH) \
  $(FRAMEWORKS_MEDIA_INLCUDE_PATH) \
  $(FRAMEWORKS_PATH)/include/media \
  $(AVSYNC_PATH) \
  $(IMS_MD_INC) \
  $(HAL_PATH) \
  $(IMSMA_INC1) \
  $(IMSMA_INC2) \
  $(SOURCE_INC) \
  $(SINK_INC) \
  $(RTP_INC) \
  $(RIL_INC_PATH)\
  $(RIL_INC_PATH2)

LOCAL_SRC_FILES:= \
  $(SERVICE_SRC_PATH)/IVTClient.cpp \
  $(SERVICE_SRC_PATH)/IVTService.cpp \
  $(SERVICE_SRC_PATH)/VTMALStub.cpp \
  $(SERVICE_SRC_PATH)/VTCore.cpp \
  $(SERVICE_SRC_PATH)/VTCoreHelper.cpp \
  $(SERVICE_SRC_PATH)/VTService.cpp \
  $(SERVICE_SRC_PATH)/VTMsgDispatcher.cpp

LOCAL_SHARED_LIBRARIES:= \
  libc \
  libandroid_runtime \
  libbinder \
  libutils \
  libcutils \
  libnativehelper \
  libui \
  libgui \
  libmedia \
  libandroid_runtime \
  libbase \
  libfmq \
  vendor.mediatek.hardware.videotelephony@1.0 \
  libhidlbase \
  libhidltransport \
  libimsma \
  libsource \
  libimsma_rtp \
  libvt_avsync \
  libstagefright_foundation \
  libvcodec_cap

LOCAL_MODULE:= libmtk_vt_service
LOCAL_MULTILIB := 32

# for imcb needed
LOCAL_CFLAGS += -D __IMCF_MTK_VA__=1 -D __IMCF_NO_UA__=1

ifeq ($(TARGET_SIMULATOR),true)
LOCAL_CFLAGS += -DSINGLE_PROCESS
endif

include $(MTK_SHARED_LIBRARY)

endif

############################################################

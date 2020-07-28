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


#
# libmjpeg
#
LOCAL_PATH:= $(call my-dir)

include $(CLEAR_VARS)

#LOCAL_SRC_FILES:= \
   test_sk_decoder.cpp

#LOCAL_SRC_FILES:= \
   test_sw_skia.cpp

#LOCAL_SRC_FILES:= \
   jpeg_dec_test_mhal_regression.cpp \

#LOCAL_SRC_FILES:= \
   jpeg_dec_test_mhal.cpp \

#LOCAL_SRC_FILES:= \
   jpeg_dec_test_mhal_earlyPorting_regression.cpp 

#LOCAL_SRC_FILES:= \
   webp_dec_test_comp_tile.cpp \

#LOCAL_SRC_FILES:= \
   jpeg_dec_test_pipe.cpp \

#LOCAL_SRC_FILES:= \
   jpeg_dec_test_comp_tile.cpp \

#LOCAL_SRC_FILES:= \
   jpeg_dec_test_comp.cpp \

LOCAL_SRC_FILES:= \
   jpeg_enc_test.cpp \

#LOCAL_SRC_FILES:= \
   jpeg_enc_test_3p_sw.cpp \

#LOCAL_SRC_FILES:= \
   jpeg_enc_test_ion.cpp \

#LOCAL_SRC_FILES:= \
   webp_dec_test_comp_tile_ion.cpp \


LOCAL_SHARED_LIBRARIES := \
  libcutils \
  liblog \
  libJpgEncPipe \
  libJpgDecPipe \
  libjpeg \
  libion \
  libion_mtk \

ifeq ($(MTK_M4U_SUPPORT), yes)
LOCAL_SHARED_LIBRARIES += \
  libm4u
endif

LOCAL_SHARED_LIBRARIES += \
  libmhalImageCodec

LOCAL_C_INCLUDES:= \
  $(TOP)/system/core/include/cutils \
  $(TOP)/bionic \
  $(TOP)/external/jpeg \
  $(TOP)/external/skia/include/images \
  $(TOP)/external/skia/include/core \
  $(TOP)/external/stlport/stlport \
  $(TOP)/$(MTK_PATH_PLATFORM)/hardware/m4u \
  $(TOP)/$(MTK_PATH_SOURCE)/protect-bsp/platform/mt6752/hardware/m4u \
  $(TOP)/$(MTK_PATH_PLATFORM)/kernel/core/include/mach \
  $(MTK_PATH_SOURCE)external/mhal/src/core/drv/inc \
  $(TOP)/$(MTK_PATH_PLATFORM)/kernel/core/include/mach \
  $(TOP)/$(MTK_PATH_PLATFORM)/hardware/camera \
  $(TOP)/$(MTK_PATH_PLATFORM)/hardware/camera/inc \
  $(TOP)/$(MTK_PATH_PLATFORM)/hardware/camera/inc/imageio \
  $(TOP)/$(MTK_PATH_PLATFORM)/hardware/camera/inc/drv \
  $(TOP)/$(MTK_PATH_PLATFORM)/exteranl/ldvt/include \
  $(TOP)/$(MTK_PATH_PLATFORM)/hardware/camera/inc \
  $(TOP)/$(MTK_PATH_PLATFORM)/exteranl/ldvt/include \
  $(TOP)/$(MTK_PATH_PLATFORM)/hardware/camera/inc/scenario \
  $(TOP)/$(MTK_PATH_PLATFORM)/hardware/camera/core/inc/scenario \
  $(TOP)/$(MTK_PATH_PLATFORM)/hardware/camera/inc/campipe \
  $(TOP)/$(MTK_PATH_PLATFORM)/exteranl/ldvt/ts/camera/mhal/src/core/drv/common/imgsensor \
  $(MTK_PATH_SOURCE)kernel/include/linux \
  $(TOP)/$(MTK_PATH_PLATFORM)/kernel/core/include/mach \
  $(MTK_PATH_SOURCE)external/mhal/src/core/drv/inc \
  $(TOP)/$(MTK_PATH_PLATFORM)/kernel/drivers/jpeg \
  $(TOP)/$(MTK_PATH_SOURCE)/hardware/jpeg/mt6735/inc \
  $(TOP)/$(MTK_PATH_SOURCE)/hardware/jpeg/mt6735/utils \
  $(TOP)/$(MTK_PATH_SOURCE)/hardware/jpeg/mt6735/hal \
  $(TOP)/$(MTK_PATH_SOURCE)/protect-bsp/platform/mt6752/hardware/webp/inc \
  $(TOP)/$(MTK_PATH_SOURCE)/hardware/jpeg/inc/platform/$(MTK_PLATFORM_DIR) \
  $(TOP)/$(MTK_PATH_PLATFORM)/hardware/jpeg/utils \
  $(TOP)/$(MTK_PATH_PLATFORM)/hardware/gdma/inc \
  $(TOP)/$(MTK_PATH_PLATFORM)/hardware/gdma/hal \
  $(TOP)/$(MTK_PATH_PLATFORM)/hardware/webp/inc \
  $(TOP)/$(MTK_PATH_SOURCE)/hardware/jpeg/inc/mhal \
  $(TOP)/$(MTK_PATH_SOURCE)/hardware/jpeg/include \
  $(TOP)/$(MTK_PATH_SOURCE)/hardware/m4u/$(MTK_PLATFORM_DIR) \
  $(TOP)/$(MTK_PATH_SOURCE)/platform/inc \
  $(TOP)/$(MTK_PATH_PLATFORM)/hardware/vcodec/inc \
  $(TOP)/$(MTK_PATH_PLATFORM)/hardware/dpframework/inc \
  $(TOP)/$(MTK_PATH_SOURCE)/hardware/dpframework/inc \
  $(TOP)/$(MTK_PATH_SOURCE)/hardware/libvcodec/mt6735/common \
  $(TOP)/$(MTK_PATH_SOURCE)/hardware/libvcodec/mt6735/common/vp8_dec \
  $(TOP)/$(MTK_PATH_SOURCE)/protect-bsp/external/mhal/src/core/drv/inc \
  $(TOP)/$(MTK_PATH_SOURCE)/protect-bsp/external/mhal/src/core/drv/6752/vcodec/include \
  $(TOP)/$(MTK_PATH_SOURCE)/protect-bsp/external/mhal/src/core/drv/6752/vcodec/common/ \
  $(TOP)/$(MTK_PATH_SOURCE)/protect-bsp/external/mhal/src/core/drv/6752/vcodec/common/vp8_dec \
  $(TOP)/system/core/libion/include/ \
  $(TOP)/vendor/mediatek/proprietary/external/libion_mtk/include/ \
  

LOCAL_CFLAGS += -Wno-multichar

LOCAL_MODULE:= JpgUnitTest
LOCAL_PROPRIETARY_MODULE := true
LOCAL_MODULE_OWNER := mtk

LOCAL_MODULE_TAGS := tests

LOCAL_PRELINK_MODULE:=false

#LOCAL_PRELINK_MODULE:=true

include $(MTK_EXECUTABLE)





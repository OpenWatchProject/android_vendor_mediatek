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
ifneq ($(MTK_EMULATOR_SUPPORT), yes)
LOCAL_PATH:= $(call my-dir)


ifneq (,$(filter $(strip $(TARGET_BOARD_PLATFORM)), mt2601 mt6572 mt8127 mt8163 mt7623 mt8168))

include $(LOCAL_PATH)/$(MTK_PLATFORM_DIR)/Android.mk

endif

#for new chips, use v2 driver with both HW jpeg encoder/decoder supported
ifneq (,$(filter $(strip $(TARGET_BOARD_PLATFORM)), mt6735 mt6737t mt6735m mt6737m mt6753 mt6752 mt6755 mt6750 mt6757 mt6595 mt6795 mt6797 mt6799 mt8173 mt8167 elbrus kiboplus))

LOCAL_PATH:= $(call my-dir)

################################################################################
# libJpgEncDrv_plat
################################################################################
include $(CLEAR_VARS)

LOCAL_SRC_FILES:= \
  v2/hal/jpeg_enc_hal.cpp \

LOCAL_C_INCLUDES:= \
  $(TOP)/$(MTK_PATH_SOURCE)/kernel/core/include/mach \
  $(TOP)/$(MTK_PATH_SOURCE)/kernel/drivers/jpeg \
  $(TOP)/$(MTK_PATH_SOURCE)/hardware/jpeg/v2/hal \

LOCAL_SHARED_LIBRARIES += \
  libcutils

LOCAL_HEADER_LIBRARIES := liblog_headers

LOCAL_STATIC_LIBRARIES := \

LOCAL_MODULE:= libJpgEncDrv_plat
LOCAL_PROPRIETARY_MODULE := true
LOCAL_MODULE_OWNER := mtk

include $(MTK_STATIC_LIBRARY)

################################################################################
# libJpgDecDrv_plat
################################################################################
include $(CLEAR_VARS)

LOCAL_SRC_FILES:= \
  v2/hal/jpeg_dec_hal.cpp \
  v2/hal/jpeg_dec_parse.cpp \

LOCAL_C_INCLUDES:= \
  $(TOP)/system/core/include/utils \
  $(TOP)/$(MTK_PATH_SOURCE)/kernel/core/include/mach \
  $(TOP)/$(MTK_PATH_SOURCE)/kernel/drivers/jpeg \
  $(TOP)/$(MTK_PATH_SOURCE)/hardware/jpeg/v2/hal \

LOCAL_SHARED_LIBRARIES += \
  liblog \
  libcutils

LOCAL_HEADER_LIBRARIES := libutils_headers

LOCAL_CFLAGS += -DJPEG_DEC_CPU_GO

LOCAL_STATIC_LIBRARIES := \

LOCAL_MODULE:= libJpgDecDrv_plat
LOCAL_PROPRIETARY_MODULE := true
LOCAL_MODULE_OWNER := mtk

include $(MTK_STATIC_LIBRARY)

################################################################################
# libSwJpgCodec.so
################################################################################

include $(CLEAR_VARS)

LOCAL_SRC_FILES:= \
  v2/sw_codec/jpeg_sw_dec.cpp \
  v2/sw_codec/SwJpegUtility.cpp \

LOCAL_C_INCLUDES:= \
  $(TOP)/bionic \
  $(TOP)/$(MTK_PATH_SOURCE)/hardware/jpeg/v2/inc \
  #$(TOP)/$(MTK_PATH_SOURCE)/hardware/jpeg/inc/platform/$(MTK_PLATFORM_DIR) \


LOCAL_SHARED_LIBRARIES += \
  liblog \
  libutils \
  libcutils \
  libjpeg-alpha_vendor

LOCAL_STATIC_LIBRARIES := \

LOCAL_WHOLE_STATIC_LIBRARIES := \

LOCAL_MODULE := libSwJpgCodec
LOCAL_PROPRIETARY_MODULE := true
LOCAL_MODULE_OWNER := mtk

LOCAL_MULTILIB := both

LOCAL_MODULE_TAGS := optional
LOCAL_PRELINK_MODULE := false

include $(MTK_SHARED_LIBRARY)

################################################################################
# libJpgEnc_pipe
################################################################################

include $(CLEAR_VARS)

LOCAL_SRC_FILES:= \
  v2/jpegio/jpeg_enc_pipe.cpp \

LOCAL_SRC_FILES += \
  v2/utils/img_mmu.cpp \

LOCAL_C_INCLUDES:= \
  $(TOP)/$(MTK_PATH_SOURCE)/hardware/m4u/$(MTK_PLATFORM_DIR) \
  $(TOP)/device/mediatek/$(MTK_PLATFORM_DIR)/kernel-headers \
  $(TOP)/$(MTK_PATH_SOURCE)/hardware/jpeg/v2/inc \
  $(TOP)/$(MTK_PATH_SOURCE)/hardware/jpeg/v2/hal \
  $(TOP)/$(MTK_PATH_SOURCE)/hardware/jpeg/v2/utils \
  $(TOP)/$(MTK_PATH_SOURCE)/hardware/jpeg/include/enc \
  $(TOP)/system/core/include

LOCAL_C_INCLUDES += \
  $(TOP)/$(MTK_PATH_SOURCE)/external/libion_mtk/include \

LOCAL_SHARED_LIBRARIES := \
  liblog \
  libutils \
  libcutils \
  libjpeg-alpha_vendor \
  libc

LOCAL_SHARED_LIBRARIES += \
  libion \
  libion_mtk

ifeq ($(MTK_M4U_SUPPORT), yes)
LOCAL_SHARED_LIBRARIES += \
  libm4u
LOCAL_CFLAGS += -DMTK_M4U_SUPPORT
endif

LOCAL_WHOLE_STATIC_LIBRARIES := \
 libJpgEncDrv_plat \

#LOCAL_CFLAGS += -DJENC_LOCK_VIDEO
LOCAL_CFLAGS += -DJENC_LOCK_HWENC

ifneq (,$(filter $(strip $(TARGET_BOARD_PLATFORM)), mt6757))
LOCAL_CFLAGS += -DJENC_PLATFORM_MT6757
endif


#JPEG HE encoder support flag
LOCAL_CFLAGS += -DJENC_HW_SUPPORT

LOCAL_MODULE_TAGS := optional

LOCAL_MODULE := libJpgEncPipe
LOCAL_PROPRIETARY_MODULE := true
LOCAL_MODULE_OWNER := mtk
LOCAL_MULTILIB := both

LOCAL_EXPORT_C_INCLUDE_DIRS := \
  $(LOCAL_PATH)/include

include $(MTK_SHARED_LIBRARY)

################################################################################
# libJpgDecPipe.so
################################################################################

include $(CLEAR_VARS)

LOCAL_SRC_FILES:= \
  v2/jpegio/jpeg_dec_pipe.cpp \
  v2/gdmaio/img_mdp_pipe.cpp \

LOCAL_SRC_FILES += \
  v2/utils/img_mmu.cpp \
  v2/jpegio/jpeg_dec_comp.cpp

LOCAL_C_INCLUDES:= \
  $(TOP)/system/core/include/utils \
  $(TOP)/$(MTK_PATH_SOURCE)/hardware/m4u/$(MTK_PLATFORM_DIR) \
  $(TOP)/device/mediatek/$(MTK_PLATFORM_DIR)/kernel-headers \
  $(TOP)/$(MTK_PATH_SOURCE)/hardware/jpeg/v2/hal \
  $(TOP)/$(MTK_PATH_SOURCE)/hardware/jpeg/v2/inc \
  $(TOP)/$(MTK_PATH_SOURCE)/hardware/jpeg/v2/utils \
  $(TOP)/$(MTK_PATH_SOURCE)/hardware/jpeg/include/mhal \

LOCAL_C_INCLUDES += \
  $(TOP)/$(MTK_PATH_SOURCE)/external/libion_mtk/include \


LOCAL_SHARED_LIBRARIES += \
  liblog \
  libutils \
  libcutils \
  libSwJpgCodec \
  libdpframework \
  libjpeg-alpha_vendor \
  libc

LOCAL_SHARED_LIBRARIES += \
  libion \
  libion_mtk

ifeq ($(MTK_M4U_SUPPORT), yes)
LOCAL_SHARED_LIBRARIES += \
  libm4u
LOCAL_CFLAGS += -DMTK_M4U_SUPPORT
endif

LOCAL_STATIC_LIBRARIES := \

LOCAL_WHOLE_STATIC_LIBRARIES := \
  libJpgDecDrv_plat \

LOCAL_CFLAGS += -DJPEG_HW_DECODE_COMP

LOCAL_MODULE := libJpgDecPipe
LOCAL_PROPRIETARY_MODULE := true
LOCAL_MODULE_OWNER := mtk
LOCAL_MULTILIB := both

LOCAL_MODULE_TAGS := optional
LOCAL_PRELINK_MODULE := false

include $(MTK_SHARED_LIBRARY)

################################################################################
# libmHalImgCodec.so
################################################################################

include $(CLEAR_VARS)

LOCAL_SRC_FILES:= \
  v2/imagecodec/mhal_image_codec.cpp \
  v2/imagecodec/mhal_jpeg_dec.cpp \
  v2/imagecodec/mhal_jpeg_enc.cpp \

LOCAL_C_INCLUDES += \
  $(TOP)/$(MTK_PATH_SOURCE)/hardware/m4u/$(MTK_PLATFORM_DIR) \
  $(TOP)/$(MTK_PATH_SOURCE)/hardware/jpeg/v2/inc \
  $(TOP)/$(MTK_PATH_SOURCE)/hardware/jpeg/include/mhal \
  $(TOP)/$(MTK_PATH_SOURCE)/hardware/jpeg/include/enc \

LOCAL_STATIC_LIBRARIES := \

LOCAL_SHARED_LIBRARIES:= \
	liblog \
	libcutils \
	libutils \
	libJpgDecPipe \
	libJpgEncPipe \
	libdpframework \
    libion

LOCAL_MODULE:= libmhalImageCodec
LOCAL_PROPRIETARY_MODULE := true
LOCAL_MODULE_OWNER := mtk
LOCAL_MULTILIB := both
LOCAL_MODULE_TAGS := optional

LOCAL_EXPORT_C_INCLUDE_DIRS := \
  $(LOCAL_PATH)/include

include $(MTK_SHARED_LIBRARY)

endif

#for new chips, use v2 driver with only HW jpeg encoder supported
ifneq (,$(filter $(strip $(TARGET_BOARD_PLATFORM)), mt6759 mt6763 mt6758 mt6739 mt3886 mt6771 mt6775 mt6765 mt6761 mt3967 mt6779 mt6768 mt6785 mt6885 mt6789))

LOCAL_PATH:= $(call my-dir)

################################################################################
# libJpgEncDrv_plat
################################################################################
include $(CLEAR_VARS)

LOCAL_SRC_FILES:= \
  v2/hal/jpeg_enc_hal.cpp \

LOCAL_C_INCLUDES:= \
  $(TOP)/$(MTK_PATH_SOURCE)/kernel/core/include/mach \
  $(TOP)/$(MTK_PATH_SOURCE)/kernel/drivers/jpeg \
  $(TOP)/$(MTK_PATH_SOURCE)/hardware/jpeg/v2/hal \

LOCAL_SHARED_LIBRARIES += \
  libcutils

LOCAL_STATIC_LIBRARIES := \

LOCAL_HEADER_LIBRARIES := liblog_headers

LOCAL_MODULE:= libJpgEncDrv_plat
LOCAL_PROPRIETARY_MODULE := true
LOCAL_MODULE_OWNER := mtk

include $(MTK_STATIC_LIBRARY)

################################################################################
# libSwJpgCodec.so
################################################################################

include $(CLEAR_VARS)

LOCAL_SRC_FILES:= \
  v2/sw_codec/jpeg_sw_dec.cpp \
  v2/sw_codec/SwJpegUtility.cpp \

LOCAL_C_INCLUDES:= \
  $(TOP)/bionic \
  $(TOP)/$(MTK_PATH_SOURCE)/hardware/jpeg/v2/inc \
  #$(TOP)/$(MTK_PATH_SOURCE)/hardware/jpeg/inc/platform/$(MTK_PLATFORM_DIR) \


LOCAL_SHARED_LIBRARIES += \
  liblog \
  libutils \
  libcutils \
  libjpeg-alpha_vendor

LOCAL_STATIC_LIBRARIES := \

LOCAL_WHOLE_STATIC_LIBRARIES := \

LOCAL_MODULE := libSwJpgCodec
LOCAL_PROPRIETARY_MODULE := true
LOCAL_MODULE_OWNER := mtk

LOCAL_MULTILIB := both

LOCAL_MODULE_TAGS := optional
LOCAL_PRELINK_MODULE := false

include $(MTK_SHARED_LIBRARY)

################################################################################
# libJpgEnc_pipe
################################################################################

include $(CLEAR_VARS)

LOCAL_SRC_FILES:= \
  v2/jpegio/jpeg_enc_pipe.cpp \

LOCAL_SRC_FILES += \
  v2/utils/img_mmu.cpp \

LOCAL_C_INCLUDES:= \
  $(TOP)/$(MTK_PATH_SOURCE)/hardware/m4u/$(MTK_PLATFORM_DIR) \
  $(TOP)/device/mediatek/common/kernel-headers \
  $(TOP)/$(MTK_PATH_SOURCE)/hardware/jpeg/v2/inc \
  $(TOP)/$(MTK_PATH_SOURCE)/hardware/jpeg/v2/hal \
  $(TOP)/$(MTK_PATH_SOURCE)/hardware/jpeg/v2/utils \
  $(TOP)/$(MTK_PATH_SOURCE)/hardware/jpeg/include/enc \
  $(TOP)/$(MTK_PATH_SOURCE)/hardware/libvcodec/include \
  $(TOP)/system/core/include

LOCAL_C_INCLUDES += \
  $(TOP)/$(MTK_PATH_SOURCE)/external/libion_mtk/include \

LOCAL_SHARED_LIBRARIES := \
  liblog \
  libutils \
  libcutils \
  libjpeg-alpha_vendor \
  libc

LOCAL_SHARED_LIBRARIES += \
  libion \
  libion_mtk

ifeq ($(MTK_M4U_SUPPORT), yes)
LOCAL_SHARED_LIBRARIES += \
  libm4u
LOCAL_CFLAGS += -DMTK_M4U_SUPPORT
endif

ifneq (,$(filter $(strip $(TARGET_BOARD_PLATFORM)), mt6763 mt6739 mt6765 mt6761))
LOCAL_SHARED_LIBRARIES += \
  libvcodec_utility

LOCAL_CFLAGS += -DJENC_LOCK_VIDEO
endif

ifneq (,$(filter $(strip $(TARGET_BOARD_PLATFORM)), mt6771 mt6762 mt6762m mt6765))
LOCAL_CFLAGS += -DJENC_CORRECT_ALIGNMENT_CHECK
endif


ifneq (,$(filter $(strip $(TARGET_BOARD_PLATFORM)), mt6779 mt6785))
LOCAL_CFLAGS += -DJENC_PLATFORM_MT6779_PORT
endif


LOCAL_WHOLE_STATIC_LIBRARIES := \
 libJpgEncDrv_plat \

LOCAL_CFLAGS += -DJENC_LOCK_HWENC

#JPEG HE encoder support flag
LOCAL_CFLAGS += -DJENC_HW_SUPPORT

LOCAL_MODULE_TAGS := optional

LOCAL_MODULE := libJpgEncPipe
LOCAL_PROPRIETARY_MODULE := true
LOCAL_MODULE_OWNER := mtk
LOCAL_MULTILIB := both

LOCAL_EXPORT_C_INCLUDE_DIRS := \
  $(LOCAL_PATH)/include

include $(MTK_SHARED_LIBRARY)

################################################################################
# libJpgDecPipe.so
################################################################################

include $(CLEAR_VARS)

LOCAL_SRC_FILES:= \
  v2/jpegio/jpeg_dec_pipe.cpp \
  v2/gdmaio/img_mdp_pipe.cpp \

LOCAL_C_INCLUDES:= \
  $(TOP)/system/core/include/utils \
  $(TOP)/$(MTK_PATH_SOURCE)/hardware/m4u/$(MTK_PLATFORM_DIR) \
  $(TOP)/$(MTK_PATH_SOURCE)/hardware/jpeg/v2/hal \
  $(TOP)/$(MTK_PATH_SOURCE)/hardware/jpeg/v2/inc \
  $(TOP)/$(MTK_PATH_SOURCE)/hardware/jpeg/v2/utils \
  $(TOP)/$(MTK_PATH_SOURCE)/hardware/jpeg/include/mhal \

LOCAL_SHARED_LIBRARIES += \
  liblog \
  libutils \
  libcutils \
  libSwJpgCodec \
  libjpeg-alpha_vendor \
  libdpframework

LOCAL_STATIC_LIBRARIES := \

LOCAL_MODULE := libJpgDecPipe
LOCAL_PROPRIETARY_MODULE := true
LOCAL_MODULE_OWNER := mtk
LOCAL_MULTILIB := both

LOCAL_MODULE_TAGS := optional
LOCAL_PRELINK_MODULE := false

include $(MTK_SHARED_LIBRARY)

################################################################################
# libmHalImgCodec.so
################################################################################

include $(CLEAR_VARS)

LOCAL_SRC_FILES:= \
  v2/imagecodec/mhal_image_codec.cpp \
  v2/imagecodec/mhal_jpeg_dec.cpp \
  v2/imagecodec/mhal_jpeg_enc.cpp \

LOCAL_C_INCLUDES += \
  $(TOP)/$(MTK_PATH_SOURCE)/hardware/m4u/$(MTK_PLATFORM_DIR) \
  $(TOP)/$(MTK_PATH_SOURCE)/hardware/jpeg/v2/inc \
  $(TOP)/$(MTK_PATH_SOURCE)/hardware/jpeg/include/mhal \
  $(TOP)/$(MTK_PATH_SOURCE)/hardware/jpeg/include/enc \

LOCAL_STATIC_LIBRARIES := \

LOCAL_SHARED_LIBRARIES:= \
	liblog \
	libcutils \
	libutils \
	libJpgDecPipe \
	libJpgEncPipe \
	libdpframework \
    libion

LOCAL_MODULE:= libmhalImageCodec
LOCAL_PROPRIETARY_MODULE := true
LOCAL_MODULE_OWNER := mtk
LOCAL_MULTILIB := both
LOCAL_MODULE_TAGS := optional

LOCAL_EXPORT_C_INCLUDE_DIRS := \
  $(LOCAL_PATH)/include

include $(MTK_SHARED_LIBRARY)

endif

#for new chips, use v2 driver with only SW jpeg supported
ifneq (,$(filter $(strip $(TARGET_BOARD_PLATFORM)), mt6580 mt6570))

LOCAL_PATH:= $(call my-dir)

################################################################################
# libSwJpgCodec.so
################################################################################

include $(CLEAR_VARS)

LOCAL_SRC_FILES:= \
  v2/sw_codec/jpeg_sw_dec.cpp \
  v2/sw_codec/SwJpegUtility.cpp \

LOCAL_C_INCLUDES:= \
  $(TOP)/bionic \
  $(TOP)/$(MTK_PATH_SOURCE)/hardware/jpeg/v2/inc \
  #$(TOP)/$(MTK_PATH_SOURCE)/hardware/jpeg/inc/platform/$(MTK_PLATFORM_DIR) \


LOCAL_SHARED_LIBRARIES += \
  liblog \
  libutils \
  libcutils \
  libjpeg-alpha_vendor

LOCAL_STATIC_LIBRARIES := \

LOCAL_WHOLE_STATIC_LIBRARIES := \

LOCAL_MODULE := libSwJpgCodec
LOCAL_PROPRIETARY_MODULE := true
LOCAL_MODULE_OWNER := mtk

LOCAL_MULTILIB := both

LOCAL_MODULE_TAGS := optional
LOCAL_PRELINK_MODULE := false

include $(MTK_SHARED_LIBRARY)

################################################################################
# libJpgEnc_pipe
################################################################################

include $(CLEAR_VARS)

LOCAL_SRC_FILES:= \
  v2/jpegio/jpeg_enc_pipe.cpp \

LOCAL_C_INCLUDES:= \
  $(TOP)/$(MTK_PATH_SOURCE)/hardware/m4u/$(MTK_PLATFORM_DIR) \
  $(TOP)/device/mediatek/$(MTK_PLATFORM_DIR)/kernel-headers \
  $(TOP)/$(MTK_PATH_SOURCE)/hardware/jpeg/v2/inc \
  $(TOP)/$(MTK_PATH_SOURCE)/hardware/jpeg/v2/hal \
  $(TOP)/$(MTK_PATH_SOURCE)/hardware/jpeg/v2/utils \
  $(TOP)/$(MTK_PATH_SOURCE)/hardware/jpeg/include/enc \

LOCAL_SHARED_LIBRARIES := \
  liblog \
  libutils \
  libcutils \
  libjpeg-alpha_vendor

LOCAL_SHARED_LIBRARIES += \
  libion \
  libion_mtk

ifeq ($(MTK_M4U_SUPPORT), yes)
LOCAL_SHARED_LIBRARIES += \
  libm4u
LOCAL_CFLAGS += -DMTK_M4U_SUPPORT
endif

LOCAL_MODULE_TAGS := optional

LOCAL_MODULE := libJpgEncPipe
LOCAL_PROPRIETARY_MODULE := true
LOCAL_MODULE_OWNER := mtk
LOCAL_MULTILIB := both

LOCAL_EXPORT_C_INCLUDE_DIRS := \
  $(LOCAL_PATH)/include

include $(MTK_SHARED_LIBRARY)

################################################################################
# libJpgDecPipe.so
################################################################################

include $(CLEAR_VARS)

LOCAL_SRC_FILES:= \
  v2/jpegio/jpeg_dec_pipe.cpp \
  v2/gdmaio/img_mdp_pipe.cpp \

LOCAL_C_INCLUDES:= \
  $(TOP)/system/core/include/utils \
  $(TOP)/$(MTK_PATH_SOURCE)/hardware/m4u/$(MTK_PLATFORM_DIR) \
  $(TOP)/device/mediatek/$(MTK_PLATFORM_DIR)/kernel-headers \
  $(TOP)/$(MTK_PATH_SOURCE)/hardware/jpeg/v2/hal \
  $(TOP)/$(MTK_PATH_SOURCE)/hardware/jpeg/v2/inc \
  $(TOP)/$(MTK_PATH_SOURCE)/hardware/jpeg/v2/utils \
  $(TOP)/$(MTK_PATH_SOURCE)/hardware/jpeg/include/mhal \


LOCAL_SHARED_LIBRARIES += \
  liblog \
  libutils \
  libcutils \
  libSwJpgCodec \
  libjpeg-alpha_vendor \
  libdpframework

LOCAL_STATIC_LIBRARIES := \

LOCAL_MODULE := libJpgDecPipe
LOCAL_PROPRIETARY_MODULE := true
LOCAL_MODULE_OWNER := mtk
LOCAL_MULTILIB := both

LOCAL_MODULE_TAGS := optional
LOCAL_PRELINK_MODULE := false

include $(MTK_SHARED_LIBRARY)

################################################################################
# libmHalImgCodec.so
################################################################################

include $(CLEAR_VARS)

LOCAL_SRC_FILES:= \
  v2/imagecodec/mhal_image_codec.cpp \
  v2/imagecodec/mhal_jpeg_dec.cpp \
  v2/imagecodec/mhal_jpeg_enc.cpp \

LOCAL_C_INCLUDES += \
  $(TOP)/$(MTK_PATH_SOURCE)/hardware/m4u/$(MTK_PLATFORM_DIR) \
  $(TOP)/$(MTK_PATH_SOURCE)/hardware/jpeg/v2/inc \
  $(TOP)/$(MTK_PATH_SOURCE)/hardware/jpeg/include/mhal \
  $(TOP)/$(MTK_PATH_SOURCE)/hardware/jpeg/include/enc \

LOCAL_STATIC_LIBRARIES := \

LOCAL_SHARED_LIBRARIES:= \
	liblog \
	libcutils \
	libutils \
	libJpgDecPipe \
	libJpgEncPipe \
	libdpframework \
    libion

LOCAL_MODULE:= libmhalImageCodec
LOCAL_PROPRIETARY_MODULE := true
LOCAL_MODULE_OWNER := mtk
LOCAL_MULTILIB := both
LOCAL_MODULE_TAGS := optional

LOCAL_EXPORT_C_INCLUDE_DIRS := \
  $(LOCAL_PATH)/include/

include $(MTK_SHARED_LIBRARY)

endif

endif


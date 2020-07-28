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

#
# libui_ext.so always in system partition
#

include $(CLEAR_VARS)

LOCAL_MODULE := libui_ext
LOCAL_PROPRIETARY_MODULE := false
LOCAL_MODULE_OWNER := mtk

LOCAL_MODULE_TAGS := optional


# for bring up, please unmark this line
# LOCAL_CFLAGS += -DMTK_DO_NOT_USE_GUI_EXT

ifneq ($(strip $(TARGET_BUILD_VARIANT)), eng)
LOCAL_CFLAGS += -DMTK_USER_BUILD
endif

ifeq ($(MTK_MIRAVISION_SUPPORT),yes)
LOCAL_CFLAGS += -DCONFIG_FOR_SOURCE_PQ
endif

ifeq ($(FPGA_EARLY_PORTING), yes)
LOCAL_CFLAGS += -DFPGA_EARLY_PORTING
endif

ifeq ($(MTK_DO_NOT_USE_GPU_EXT),true)
LOCAL_CFLAGS += -DMTK_DO_NOT_USE_GPU_EXT
endif

LOCAL_SRC_FILES := \
	lib/ui_ext/FpsCounter.cpp \
	lib/ui_ext/GraphicBufferUtil.cpp \
	lib/ui_ext/SWWatchDog.cpp \
	lib/ui_ext/IDumpTunnel.cpp \
	lib/ui_ext/FpsCounterWraper.cpp \

LOCAL_C_INCLUDES := \
	$(TOP)/$(MTK_ROOT)/hardware/libgem/inc \
	$(TOP)/$(MTK_ROOT)/hardware/include \
	$(TOP)/$(MTK_ROOT)/hardware/gralloc_extra/include \
	external/libpng \
	external/zlib \
	external/skia/src/images \
	external/skia/include/core \
	external/skia/include/config \
	external/skia/include/config/android \
	external/skia/src/core \
	external/skia/include/private

LOCAL_SHARED_LIBRARIES := \
	libutils \
	libcutils \
	liblog \
	libbinder \
	libhardware \
	libdl \
	libui \
	libpng \
	libgralloc_extra_sys \
	libnativewindow

ifneq ($(MTK_BASIC_PACKAGE), yes)
	LOCAL_CFLAGS += -DMTK_AOSP_ENHANCEMENT

	LOCAL_CPPFLAGS += -DMTK_AOSP_ENHANCEMENT
endif

include $(BUILD_SHARED_LIBRARY)

#
# libgui_ext.so
#

include $(CLEAR_VARS)

LOCAL_MODULE := libgui_ext
LOCAL_PROPRIETARY_MODULE := false
LOCAL_MODULE_OWNER := mtk

LOCAL_MODULE_TAGS := optional


# for bring up, please unmark this line
# LOCAL_CFLAGS += -DMTK_DO_NOT_USE_GUI_EXT

ifneq ($(strip $(TARGET_BUILD_VARIANT)), eng)
LOCAL_CFLAGS += -DMTK_USER_BUILD
endif

ifeq ($(MTK_MIRAVISION_SUPPORT),yes)
LOCAL_CFLAGS += -DCONFIG_FOR_SOURCE_PQ
endif

LOCAL_SRC_FILES := \
	lib/gui_ext/IGuiExtService.cpp \
	lib/gui_ext/GuiExtService.cpp \
	lib/gui_ext/GuiExtClient.cpp \

LOCAL_C_INCLUDES := \
	$(TOP)/$(MTK_ROOT)/hardware/libgem/inc \
	$(TOP)/$(MTK_ROOT)/hardware/include

LOCAL_SHARED_LIBRARIES := \
	libutils \
	libcutils \
	liblog \
	libbinder \
	libhardware \
	libgui \
	libui \
	libui_ext

ifneq ($(MTK_BASIC_PACKAGE), yes)
	LOCAL_CFLAGS += -DMTK_AOSP_ENHANCEMENT

	LOCAL_CPPFLAGS += -DMTK_AOSP_ENHANCEMENT

	LOCAL_SRC_FILES += \
		lib/gui_ext/RefBaseDump.cpp
endif

include $(BUILD_SHARED_LIBRARY)



#
# libgui_debug.so
#

include $(CLEAR_VARS)

LOCAL_MODULE := libgui_debug
LOCAL_PROPRIETARY_MODULE := false
LOCAL_MODULE_OWNER := mtk

LOCAL_MODULE_TAGS := optional

ifeq ($(MTK_DO_NOT_USE_GPU_EXT),true)
LOCAL_CFLAGS += -DMTK_DO_NOT_USE_GPU_EXT
endif

LOCAL_CFLAGS += -DMTK_LIBUI_DEBUG_SUPPORT

LOCAL_SRC_FILES := \
	lib/gui_debug/BufferQueueDump.cpp

LOCAL_C_INCLUDES := \
	$(TOP)/$(MTK_ROOT)/hardware/libgem/inc \
	$(TOP)/$(MTK_ROOT)/hardware/include

LOCAL_SHARED_LIBRARIES := \
	libutils \
	libcutils \
	liblog \
	libhardware \
	libui \
	libnativewindow \
	libgralloc_extra_sys \
	libui_ext

include $(BUILD_SHARED_LIBRARY)

#
# libgui_debug.so
#

include $(CLEAR_VARS)

LOCAL_MODULE := libsf_debug
LOCAL_PROPRIETARY_MODULE := false
LOCAL_MODULE_OWNER := mtk

LOCAL_MODULE_TAGS := optional

LOCAL_SRC_FILES := \
	lib/sf_debug/SurfaceFlingerWatchDog.cpp \
	lib/sf_debug/SFDebug.cpp

LOCAL_C_INCLUDES := \
	$(TOP)/$(MTK_ROOT)/hardware/libgem/inc \
	$(TOP)/$(MTK_ROOT)/hardware/include \
	$(TOP)/$(MTK_ROOT)/external/aee/binary/inc

LOCAL_SHARED_LIBRARIES := \
	libutils \
	libcutils \
	liblog \
	libnativewindow \
	libui_ext \
	libcrypto \
	libselinux \
	libutilscallstack \
	libgralloc_extra_sys

include $(BUILD_SHARED_LIBRARY)



#
# libcorrect_hw_rotation
#
include $(CLEAR_VARS)

LOCAL_MODULE := libcorrect_hw_rotation
LOCAL_PROPRIETARY_MODULE := false
LOCAL_MODULE_OWNER := mtk

LOCAL_MODULE_TAGS := optional

LOCAL_SRC_FILES := \
	lib/hw_rotation/CorrectHwRotation.cpp

LOCAL_C_INCLUDES := \
	$(TOP)/$(MTK_ROOT)/hardware/libgem/inc

LOCAL_SHARED_LIBRARIES := \
	libcutils \
	liblog \
	libui \
	libgui

include $(BUILD_SHARED_LIBRARY)



#
# libvsync_hint.so
#
include $(CLEAR_VARS)

LOCAL_MODULE := libvsync_hint
LOCAL_PROPRIETARY_MODULE := false
LOCAL_MODULE_OWNER := mtk

LOCAL_MODULE_TAGS := optional

LOCAL_SRC_FILES := \
	lib/vsync_hint/VsyncHint.cpp

LOCAL_C_INCLUDES := \
	$(TOP)/$(MTK_ROOT)/hardware/libgem/inc \
	$(TOP)/$(MTK_ROOT)/hardware/include

LOCAL_SHARED_LIBRARIES := \
	libutils \
	libcutils \
	liblog \

include $(BUILD_SHARED_LIBRARY)

include $(call all-makefiles-under,$(LOCAL_PATH))

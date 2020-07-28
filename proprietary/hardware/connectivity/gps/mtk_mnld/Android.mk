# Copyright Statement:
#
# This software/firmware and related documentation ("MediaTek Software") are
# protected under relevant copyright laws. The information contained herein
# is confidential and proprietary to MediaTek Inc. and/or its licensors.
# Without the prior written permission of MediaTek inc. and/or its licensors,
# any reproduction, modification, use or disclosure of MediaTek Software,
# and information contained herein, in whole or in part, shall be strictly prohibited.

# MediaTek Inc. (C) 2016. All rights reserved.
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


# Copyright 2005 The Android Open Source Project

###############################################################################
# Configuration
###############################################################################
#MTK_GPS_CHIP = MTK_GPS_MT6582
###############################################################################
# build start
###############################################################################
ifeq ($(MTK_GPS_SUPPORT), yes)
LOCAL_PATH := $(call my-dir)

MY_MNL_PATH := mnl/MNL

include $(CLEAR_VARS)
LOCAL_MODULE:= libmnl_headers
LOCAL_EXPORT_C_INCLUDE_DIRS := $(LOCAL_PATH)/$(MY_MNL_PATH)/inc
include $(BUILD_HEADER_LIBRARY)
$(warning libmnl_headers:$(LOCAL_EXPORT_C_INCLUDE_DIRS))

include $(CLEAR_VARS)
MY_LOCAL_PATH := $(LOCAL_PATH)
$(warning feature_option=$(MTK_GPS_CHIP))

MY_MNL_PATH := mnl/MNL

LOCAL_C_INCLUDES += \
  $(LOCAL_PATH)/utility/inc \
  $(LOCAL_PATH)/mnl_agps_interface/inc \
  $(LOCAL_PATH)/mnl_at_cmd_interface/inc \
  $(LOCAL_PATH)/mnl_flp_interface/inc \
  $(LOCAL_PATH)/mnl_nlp_interface/inc \
  $(LOCAL_PATH)/mnl_meta_interface/inc \
  $(LOCAL_PATH)/mnl_debug_interface/inc \
  $(LOCAL_PATH)/mnl_log_interface/inc \
  $(LOCAL_PATH)/mnl_geofence_interface/inc \
  $(LOCAL_PATH)/mnld_entity/inc \
  $(LOCAL_PATH)/curl/inc \
  $(MTK_PATH_SOURCE)/external/nvram/libnvram \
  external/libxml2/include \
  $(TOP)/system/core/libcutils/include_vndk \
  $(MTK_PATH_SOURCE)/external/libudf/libladder \

# MPE HIDL #
LOCAL_C_INCLUDES += $(TOP)/frameworks/native/include
LOCAL_C_INCLUDES += $(TOP)/frameworks/hardware/interfaces/sensorservice/libsensorndkbridge
LOCAL_C_INCLUDES += $(TOP)/hardware/interfaces/sensors/1.0/default/include

LOCAL_C_INCLUDES += $(LOCAL_PATH)/mnl_mpe_interface/inc
LOCAL_CFLAGS += -DGPS_SUSPEND_SUPPORT

LOCAL_SRC_FILES := \
	mnld_entity/src/mnl2hal_interface.c \
	utility/src/data_coder.c \
	utility/src/mtk_lbs_utility.c \
	utility/src/mtk_socket_data_coder.c \
	utility/src/mtk_socket_utils.c \
	mnl_agps_interface/src/mnl_agps_interface.c \
	mnl_agps_interface/src/mnl2agps_interface.c \
	mnl_agps_interface/src/agps2mnl_interface.c \
	mnl_flp_interface/src/mtk_flp_controller.c \
	mnl_flp_interface/src/mtk_flp_main.c \
	mnl_flp_interface/src/mtk_flp_mnl_interface.c \
	mnl_flp_interface/src/mtk_flp_screen_monitor.c \
	mnl_flp_interface/src/mnl_flp_test_interface.c \
	mnl_geofence_interface/src/mtk_geofence_controller.c \
	mnl_geofence_interface/src/mtk_geofence_main.c \
	mnl_nlp_interface/src/Mnld2NlpUtilsInterface.c \
	mnl_meta_interface/src/Meta2MnldInterface.c \
	mnl_debug_interface/src/Debug2MnldInterface.c \
	mnl_debug_interface/src/Mnld2DebugInterface.c \
	mnl_log_interface/src/LbsLogInterface.c \
	mnl_at_cmd_interface/src/mnl_at_interface.c \
	mnld_entity/src/mnld.c \
	mnld_entity/src/mnld_uti.c \
	mnld_entity/src/gps_controller.c \
	mnld_entity/src/nmea_parser.c \
	mnld_entity/src/epo.c \
	mnld_entity/src/qepo.c \
	mnld_entity/src/mtknav.c \
	mnld_entity/src/mnl_common.c \
	mnld_entity/src/op01_log.c \
	mnld_entity/src/gps_dbg_log.c \
	mnld_entity/src/mpe.c \
	mnl/src/pseudo_mnl.c \
	utility/src/mtk_mnld_dump.cpp \

LOCAL_SRC_FILES += mnl_mpe_interface/src/mpe_main.c
LOCAL_SRC_FILES += mnl_mpe_interface/src/mpe_logger.c
LOCAL_SRC_FILES += mnl_mpe_interface/src/mpe_sensor.cpp

LOCAL_MODULE := mnld
LOCAL_PROPRIETARY_MODULE := true
LOCAL_MODULE_OWNER := mtk
LOCAL_MULTILIB := 32
LOCAL_MODULE_PATH := $(TARGET_OUT_VENDOR_EXECUTABLES)
# LOCAL_UNSTRIPPED_PATH := $(TARGET_ROOT_OUT_SBIN_UNSTRIPPED)
#ifeq ($(MTK_TC1_FEATURE), yes)
#ifeq ($(GPS_CO_CLOCK_DATA_IN_MD), yes)
LOCAL_CFLAGS += -DMTK_GPS_CO_CLOCK_DATA_IN_MD
#endif
#endif
ifeq ($(TARGET_BUILD_VARIANT), user)
LOCAL_CFLAGS += -DCONFIG_GPS_USER_LOAD
endif
ifeq ($(TARGET_BUILD_VARIANT), userdebug)
LOCAL_CFLAGS += -DCONFIG_GPS_USER_DBG_LOAD
endif
ifeq ($(TARGET_BUILD_VARIANT), eng)
LOCAL_CFLAGS += -DCONFIG_GPS_ENG_LOAD
endif
ifeq ($(MTK_AGPS_APP), yes)
LOCAL_CFLAGS += -DMTK_AGPS_SUPPORT
endif
LOCAL_STATIC_LIBRARIES +=  libsupl
LOCAL_SHARED_LIBRARIES +=  libmnl libgeofence libcurl libcutils libc libm libnvram libcrypto libssl libz liblog libhardware libladder
LOCAL_HEADER_LIBRARIES +=  libcutils_headers libmnl_headers
LOCAL_EXPORT_HEADER_LIBRARY_HEADERS += libhardware_headers
# MPE HIDL #
LOCAL_SHARED_LIBRARIES +=  libDR libutils android.frameworks.sensorservice@1.0 android.hardware.sensors@1.0 libhidlbase libhidltransport libsensorndkbridge
LOCAL_MODULE_TAGS := optional
LOCAL_REQUIRED_MODULES := libmnl.so libgeofence.so
include $(MTK_EXECUTABLE)
include $(MY_LOCAL_PATH)/mnl/bin/Android.mk
include $(MY_LOCAL_PATH)/curl/libs/Android.mk
include $(MY_LOCAL_PATH)/$(MY_MNL_PATH)/libs/Android.mk
$(warning $(MY_LOCAL_PATH)/$(MY_MNL_PATH)/libs/Android.mk)
endif
###############################################################################

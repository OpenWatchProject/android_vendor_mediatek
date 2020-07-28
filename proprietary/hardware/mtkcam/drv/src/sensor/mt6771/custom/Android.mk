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

LOCAL_PATH := $(call my-dir)

################################################################################
#
################################################################################
include $(CLEAR_VARS)

SENINF_COMMON_SRC = ../../common/v1_1/custom
SENINF_COMMON_INC = $(LOCAL_PATH)/$(SENINF_COMMON_SRC)
#-----------------------------------------------------------
-include $(TOP)/$(MTK_PATH_SOURCE)/hardware/mtkcam/mtkcam.mk

#-----------------------------------------------------------
# version
MY_CUST_SENSOR_VERSION := rev:1.0.0

#-----------------------------------------------------------
MY_CUST_SENSOR_METADATA_PATH_COMMON := $(MTK_PATH_SOURCE)/custom/common/hal/imgsensor_metadata
MY_CUST_SENSOR_METADATA_PATH_COMMON_SENSOR   := $(MTK_PATH_SOURCE)/custom/common/hal/imgsensor_metadata
MY_CUST_SENSOR_METADATA_PATH_PLATFORM := $(MTK_PATH_CUSTOM_PLATFORM)/hal/imgsensor_metadata
# MY_CUST_SENSOR_METADATA_PATH_PROJECT := $(MTK_PATH_SOURCE)/custom/$(MTK_TARGET_PROJECT)/hal/imgsensor_metadata

#-----------------------------------------------------------
#
define my-all-config.sensor-metadata-under
$(patsubst ./%,%, \
  $(shell find $(1)  -maxdepth 2 \( -name "config_static_metadata.*.h" \) -and -not -name ".*") \
 )
endef
#
# custom feature table file list
MY_CUST_SENSOR_METADATA_FILE_LIST := $(call my-all-config.sensor-metadata-under, $(MY_CUST_SENSOR_METADATA_PATH_COMMON))
MY_CUST_SENSOR_METADATA_FILE_LIST += $(call my-all-config.sensor-metadata-under, $(MY_CUST_SENSOR_METADATA_PATH_PLATFORM))
MY_CUST_SENSOR_METADATA_FILE_LIST += $(call my-all-config.sensor-metadata-under, $(MY_CUST_SENSOR_METADATA_PATH_PROJECT))

define my-all-config.sensordrv-metadata-under
$(patsubst ./%,%, \
  $(shell find $(1)  -maxdepth 3 \( -name "config_static_metadata.sensor*.h" \) -and -not -name ".*") \
 )
endef
MY_CUST_SENSORDRV_METADATA_FILE_LIST := $(call my-all-config.sensordrv-metadata-under, $(MY_CUST_SENSOR_METADATA_PATH_COMMON))


#-----------------------------------------------------------
LOCAL_SRC_FILES += $(SENINF_COMMON_SRC)/custom.cpp
LOCAL_C_INCLUDES += $(SENINF_COMMON_INC)/
#
#-----------------------------------------------------------
LOCAL_C_INCLUDES += $(MTKCAM_C_INCLUDES)

LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_SOURCE)/hardware/gralloc_extra/include
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_SOURCE)/hardware/mtkcam/include


LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_SOURCE)/hardware/mtkcam/include


LOCAL_C_INCLUDES += $(call include-path-for, camera)
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_SOURCE)/custom
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_SOURCE)/hardware/include

#-----------------------------------------------------------
LOCAL_CFLAGS += $(MTKCAM_CFLAGS)
LOCAL_CFLAGS += -DCUSTOM_FLASHLIGHT_TYPE_$(CUSTOM_KERNEL_FLASHLIGHT)
$(info "CUSTOM_FLASHLIGHT_TYPE_$(CUSTOM_KERNEL_FLASHLIGHT)")

#-----------------------------------------------------------
LOCAL_WHOLE_STATIC_LIBRARIES +=
#
LOCAL_STATIC_LIBRARIES +=
LOCAL_SHARED_LIBRARIES += liblog
LOCAL_HEADER_LIBRARIES += libutils_headers
#-----------------------------------------------------------
LOCAL_MODULE := libcam.halsensor.custom
LOCAL_PROPRIETARY_MODULE := true
LOCAL_MODULE_OWNER := mtk
LOCAL_MODULE_CLASS := STATIC_LIBRARIES

#bobule workaround pdk build error, needing review
ifeq ($(TARGET_ARCH), arm)
INTERMEDIATES := $(call local-intermediates-dir)
MY_CUST_SENSOR_METADATA_FINAL_FILE := $(INTERMEDIATES)/custgen.config_metadata.h
else
INTERMEDIATES := $(call local-intermediates-dir)
INTERMEDIATES2 := $(call local-intermediates-dir,,$(TARGET_2ND_ARCH_VAR_PREFIX))
MY_CUST_SENSOR_METADATA_FINAL_FILE := $(INTERMEDIATES)/custgen.config_metadata.h $(INTERMEDIATES2)/custgen.config_metadata.h
endif

# custom feature table all-in-one file
# MY_CUST_METADATA_FINAL_FILE := $(INTERMEDIATES)/custgen.config_metadata.h
LOCAL_GENERATED_SOURCES += $(MY_CUST_SENSOR_METADATA_FINAL_FILE)
$(MY_CUST_SENSOR_METADATA_FINAL_FILE): $(MY_CUST_SENSOR_METADATA_FILE_LIST) $(MY_CUST_SENSORDRV_METADATA_FILE_LIST)
	@mkdir -p $(dir $@)
	@echo '//this file is auto-generated; do not modify it!' > $@
	@echo '#define MY_CUST_SENSOR_VERSION "$(MY_CUST_SENSOR_VERSION)"' >> $@
	@echo '#define MY_CUST_SENSOR_FTABLE_FILE_LIST1 "$(MY_CUST_SENSORDRV_METADATA_FILE_LIST)"' >> $@
	@for x in $(MY_CUST_SENSORDRV_METADATA_FILE_LIST); do echo $$x | awk -F/ '{print "#include<"$$(NF-5) "/" $$(NF-4) "/" $$(NF-3) "/" $$(NF-2) "/" $$(NF-1) "/" $$NF ">";}' >> $@; done
	@echo '#define MY_CUST_SENSOR_FTABLE_FILE_LIST2 "$(MY_CUST_SENSOR_METADATA_FILE_LIST)"' >> $@
	@for x in $(MY_CUST_SENSOR_METADATA_FILE_LIST); do echo $$x | awk -F/ '{print "#include<"$$(NF-4) "/" $$(NF-3) "/" $$(NF-2) "/" $$(NF-1) "/" $$NF ">";}' >> $@; done
	@echo '#define MY_CUST_SENSOR_FTABLE_FINAL_FILE "$@"' >> $@
#-----------------------------------------------------------
include $(MTK_STATIC_LIBRARY)

################################################################################
#
################################################################################
include $(CLEAR_VARS)
include $(call all-makefiles-under,$(LOCAL_PATH))


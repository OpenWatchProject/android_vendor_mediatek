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

#-----------------------------------------------------------
-include $(TOP)/$(MTK_PATH_SOURCE)/hardware/mtkcam/mtkcam.mk

#-----------------------------------------------------------
# version
MY_CUST_VERSION := rev:0.3.0

#-----------------------------------------------------------
MY_CUST_METASTORE_REQUEST_PATH_COMMON   := $(MTK_PATH_SOURCE)/custom/common/hal/imgsensor_metadata
MY_CUST_METASTORE_REQUEST_PATH_PLATFORM := $(MTK_PATH_CUSTOM_PLATFORM)/hal/imgsensor_metadata
#MY_CUST_METASTORE_REQUEST_PATH_PROJECT  := $(MTK_PATH_CUSTOM)/hal/imgsensor_metadata
MY_CUST_METASTORE_REQUEST_PATH_PROJECT := $(MTK_PATH_SOURCE)/custom/$(MTK_TARGET_PROJECT)/hal/imgsensor_metadata

#-----------------------------------------------------------
#
define my-all-config.request-under
$(patsubst ./%,%, \
  $(shell if [ -d $(1) ] ; then find $(1) -maxdepth 4 \( -name 'config_request_metadata_*.h' \) -and -not -name '.*' ; fi) \
)
endef
#
# custom feature table file list
MY_CUST_METASTORE_REQUEST_FILE_LIST := $(call my-all-config.request-under, $(MY_CUST_METASTORE_REQUEST_PATH_COMMON))
MY_CUST_METASTORE_REQUEST_FILE_LIST += $(call my-all-config.request-under, $(MY_CUST_METASTORE_REQUEST_PATH_PLATFORM))
MY_CUST_METASTORE_REQUEST_FILE_LIST += $(call my-all-config.request-under, $(MY_CUST_METASTORE_REQUEST_PATH_PROJECT))

#-----------------------------------------------------------
LOCAL_SRC_FILES += custom.cpp

#-----------------------------------------------------------
LOCAL_HEADER_LIBRARIES += libmtkcam_headers
LOCAL_C_INCLUDES += $(MTKCAM_C_INCLUDES)
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_SOURCE)/hardware/mtkcam/include
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_SOURCE)/custom
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_COMMON)/hal/inc
#
LOCAL_C_INCLUDES += $(call include-path-for, camera)

#-----------------------------------------------------------
LOCAL_CFLAGS += $(MTKCAM_CFLAGS)
LOCAL_CFLAGS += -DCUSTOM_FLASHLIGHT_TYPE_$(CUSTOM_KERNEL_FLASHLIGHT)

#-----------------------------------------------------------
LOCAL_WHOLE_STATIC_LIBRARIES +=
#
LOCAL_STATIC_LIBRARIES +=

#-----------------------------------------------------------
# custom request metadata all-in-one file
LOCAL_MODULE := libmtkcam_metastore.request.custom
LOCAL_PROPRIETARY_MODULE := true
LOCAL_MODULE_OWNER := mtk
LOCAL_MODULE_CLASS := STATIC_LIBRARIES

#bobule workaround pdk build error, needing review
ifeq ($(TARGET_ARCH), arm)
INTERMEDIATES := $(call local-intermediates-dir)
MY_CUST_METASTORE_REQUEST_FINAL_FILE := $(INTERMEDIATES)/custgen.config_request.h
else
INTERMEDIATES := $(call local-intermediates-dir)
INTERMEDIATES2 := $(call local-intermediates-dir,,$(TARGET_2ND_ARCH_VAR_PREFIX))
MY_CUST_METASTORE_REQUEST_FINAL_FILE := $(INTERMEDIATES)/custgen.config_request.h $(INTERMEDIATES2)/custgen.config_request.h
endif

# custom feature table all-in-one file
# MY_CUST_METASTORE_REQUEST_FINAL_FILE := $(INTERMEDIATES)/custgen.config_request.h
LOCAL_GENERATED_SOURCES += $(MY_CUST_METASTORE_REQUEST_FINAL_FILE)
$(MY_CUST_METASTORE_REQUEST_FINAL_FILE): $(MY_CUST_METASTORE_REQUEST_FILE_LIST)
	@mkdir -p $(dir $@)
	@echo '//this file is auto-generated; do not modify it!' > $@
	@echo '#define MY_CUST_VERSION "$(MY_CUST_VERSION)"' >> $@
	@echo '#define MY_CUST_FTABLE_FILE_LIST "$(MY_CUST_METASTORE_REQUEST_FILE_LIST)"' >> $@
	@echo '#define MY_CUST_FTABLE_FINAL_FILE "$@"' >> $@
	@for x in $(MY_CUST_METASTORE_REQUEST_FILE_LIST); do echo $$x | awk -F/ '{print "#include<"$$(NF-4) "/" $$(NF-3) "/" $$(NF-2) "/" $$(NF-1) "/" $$NF ">";}' >> $@; done

#-----------------------------------------------------------
include $(MTK_STATIC_LIBRARY)

################################################################################
#
################################################################################
#include $(CLEAR_VARS)
#include $(call all-makefiles-under,$(LOCAL_PATH))


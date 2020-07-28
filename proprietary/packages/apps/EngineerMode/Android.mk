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

ifeq ($(MTK_BSP_PACKAGE),yes)

LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)

LOCAL_PACKAGE_NAME := EngineerMode

LOCAL_PRIVATE_PLATFORM_APIS := true
LOCAL_PRIVILEGED_MODULE := true
LOCAL_MODULE_OWNER := mtk
LOCAL_CERTIFICATE := platform
LOCAL_PROGUARD_ENABLED := disabled
LOCAL_MODULE_TAGS := optional
LOCAL_STATIC_JAVA_LIBRARIES := android-support-v4
LOCAL_JAVA_LIBRARIES += telephony-common ims-common mediatek-common
LOCAL_SRC_FILES := $(call all-java-files-under, src)
JAVA_SRC_DIR := src/com/mediatek/engineermode

#MSSI_MTK_TELEPHONY_ADD_ON_POLICY  empty or 0 : enable MTK BSP+
#MSSI_MTK_TELEPHONY_ADD_ON_POLICY           1 : disable MTK bsp+

#tc1 branch
ifeq ($(MSSI_MTK_TC1_COMMON_SERVICE), yes)
MSSI_MTK_TELEPHONY_ADD_ON_POLICY := 1
LOCAL_JAVA_LIBRARIES += vendor.mediatek.hardware.interfaces_tc1.mtkradioex_tc1-V1.0-java
else
LOCAL_JAVA_LIBRARIES += vendor.mediatek.hardware.mtkradioex-V1.0-java
endif

# Files only for eng/user_debug load
ifeq ($(TARGET_BUILD_VARIANT), user)
    ENG_ONLY_TEL_FILE := $(call all-java-files-under, $(JAVA_SRC_DIR)/amrwb) \
                         $(call all-java-files-under, $(JAVA_SRC_DIR)/sbp) \
                         $(call all-java-files-under, $(JAVA_SRC_DIR)/u3phy) \
                         $(call all-java-files-under, $(JAVA_SRC_DIR)/simswitch) \
                         $(call all-java-files-under, $(JAVA_SRC_DIR)/modemfilter) \
                         $(call all-java-files-under, $(JAVA_SRC_DIR)/iatype) \
                         $(call all-java-files-under, $(JAVA_SRC_DIR)/lte) \
                         $(call all-java-files-under, $(JAVA_SRC_DIR)/mdmdiagnosticinfo) \
                         $(call all-java-files-under, $(JAVA_SRC_DIR)/epdgconfig) \
                         $(call all-java-files-under, $(JAVA_SRC_DIR)/fastdormancy) \
                         $(call all-java-files-under, $(JAVA_SRC_DIR)/hspainfo) \
                         $(call all-java-files-under, $(JAVA_SRC_DIR)/spc) \
                         $(call all-java-files-under, $(JAVA_SRC_DIR)/npt) \
                         $(JAVA_SRC_DIR)/GPRS.java \
                         $(call all-java-files-under, $(JAVA_SRC_DIR)/gwsdconfigure)

    LOCAL_SRC_FILES := $(filter-out $(ENG_ONLY_TEL_FILE), $(LOCAL_SRC_FILES))

    ENG_ONLY_CONN_FILE := $(call all-java-files-under, $(JAVA_SRC_DIR)/wfdsettings) \
                          $(JAVA_SRC_DIR)/wifi/WiFiMcr.java \
                          $(JAVA_SRC_DIR)/wifi/WiFiEeprom.java \
                          $(JAVA_SRC_DIR)/wifi/WiFiRFCR.java

    LOCAL_SRC_FILES := $(filter-out $(ENG_ONLY_CONN_FILE), $(LOCAL_SRC_FILES))

    ENG_ONLY_HARDWARE_FILE := $(call all-java-files-under, $(JAVA_SRC_DIR)/desense) \
                              $(call all-java-files-under, $(JAVA_SRC_DIR)/io) \
                              $(call all-java-files-under, $(JAVA_SRC_DIR)/memory) \
                              $(call all-java-files-under, $(JAVA_SRC_DIR)/power) \
                              $(call all-java-files-under, $(JAVA_SRC_DIR)/usb) \
                              $(JAVA_SRC_DIR)/UsbSuperSpeedSwitch.java \
                              $(JAVA_SRC_DIR)/AalSetting.java \
                              $(JAVA_SRC_DIR)/UartUsbSwitch.java

    LOCAL_SRC_FILES := $(filter-out $(ENG_ONLY_HARDWARE_FILE), $(LOCAL_SRC_FILES))

    ENG_ONLY_LOCATION_FILE := $(call all-java-files-under, $(JAVA_SRC_DIR)/cwtest) \
                              $(call all-java-files-under, $(JAVA_SRC_DIR)/desenseat) \
                              $(call all-java-files-under, $(JAVA_SRC_DIR)/clkqualityat)

    LOCAL_SRC_FILES := $(filter-out $(ENG_ONLY_LOCATION_FILE), $(LOCAL_SRC_FILES))

    ENG_ONLY_LOG_DEBUG_FILE := $(JAVA_SRC_DIR)/BatteryLog.java \
                               $(call all-java-files-under, $(JAVA_SRC_DIR)/modemwarning) \
                               $(call all-java-files-under, $(JAVA_SRC_DIR)/modemresetdelay) \
                               $(call all-java-files-under, $(JAVA_SRC_DIR)/wcncoredump)

    LOCAL_SRC_FILES := $(filter-out $(ENG_ONLY_LOG_DEBUG_FILE), $(LOCAL_SRC_FILES))

    ENG_ONLY_OTHERS_FILE := $(call all-java-files-under, $(JAVA_SRC_DIR)/swla) \
                            $(call all-java-files-under, $(JAVA_SRC_DIR)/usbacm) \
                            $(JAVA_SRC_DIR)/carrierexpress/CarrierExpressActivity.java \
                            $(JAVA_SRC_DIR)/cip/CipPropContentActivity.java

    LOCAL_SRC_FILES := $(filter-out $(ENG_ONLY_OTHERS_FILE), $(LOCAL_SRC_FILES))
endif

ifeq (,$(filter $(MSSI_MTK_TELEPHONY_ADD_ON_POLICY),1))

MTK_FRAMEWORK_LOCATION_FILE  :=  $(call all-java-files-under, $(JAVA_SRC_DIR)/mtkradioextc1)
LOCAL_SRC_FILES := $(filter-out $(MTK_FRAMEWORK_LOCATION_FILE), $(LOCAL_SRC_FILES))

LOCAL_STATIC_JAVA_LIBRARIES += \
    com.mediatek.mdml \
    vendor.mediatek.hardware.netdagent-V1.0-java \
    vendor.mediatek.hardware.engineermode-V1.2-java \
    vendor.mediatek.hardware.lbs-V1.0-java \
    wfo-common \

LOCAL_JAVA_LIBRARIES += mediatek-framework mediatek-telephony-base mediatek-telephony-common mediatek-telecom-common mediatek-ims-base
endif


ifneq (,$(filter $(MSSI_MTK_TELEPHONY_ADD_ON_POLICY),1))

MTK_FRAMEWORK_LOCATION_FILE := $(call all-java-files-under, $(JAVA_SRC_DIR)/worldphone) \
                                $(call all-java-files-under, $(JAVA_SRC_DIR)/siminfo) \
                                $(call all-java-files-under, $(JAVA_SRC_DIR)/iatype) \
                                $(call all-java-files-under, $(JAVA_SRC_DIR)/rttn) \
                                $(call all-java-files-under, $(JAVA_SRC_DIR)/apc) \
                                $(call all-java-files-under, $(JAVA_SRC_DIR)/networkinfo) \
                                $(call all-java-files-under, $(JAVA_SRC_DIR)/cfu) \
                                $(call all-java-files-under, $(JAVA_SRC_DIR)/voicesettings) \
                                $(call all-java-files-under, $(JAVA_SRC_DIR)/wfdsettings) \
                                $(call all-java-files-under, $(JAVA_SRC_DIR)/mdmdiagnosticinfo) \
                                $(call all-java-files-under, $(JAVA_SRC_DIR)/iotconfig) \
                                $(call all-java-files-under, $(JAVA_SRC_DIR)/gwsdconfigure) \
                                $(JAVA_SRC_DIR)/misc/PresenceSet.java \
                                $(JAVA_SRC_DIR)/misc/PresenceActivity.java \
                                $(JAVA_SRC_DIR)/GPRS.java

ifeq ($(MSSI_MTK_TC1_COMMON_SERVICE), yes)
  MTK_FRAMEWORK_LOCATION_FILE +=  $(call all-java-files-under, $(JAVA_SRC_DIR)/mtkradioex)
else
  MTK_FRAMEWORK_LOCATION_FILE +=  $(call all-java-files-under, $(JAVA_SRC_DIR)/mtkradioextc1)
endif

LOCAL_SRC_FILES := $(filter-out $(MTK_FRAMEWORK_LOCATION_FILE), $(LOCAL_SRC_FILES))

LOCAL_STATIC_JAVA_LIBRARIES += \
    vendor.mediatek.hardware.engineermode-V1.2-java \
    vendor.mediatek.hardware.lbs-V1.0-java \
    vendor.mediatek.hardware.netdagent-V1.0-java \
    wfo-common \
    com.mediatek.mdml
endif

include $(BUILD_PACKAGE)
include $(call all-makefiles-under,$(LOCAL_PATH))
endif

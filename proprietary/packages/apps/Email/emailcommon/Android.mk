#
# Copyright (C) 2014 MediaTek Inc.
# Modification based on code covered by the mentioned copyright
# and/or permission notice(s).
#
# Copyright 2011, The Android Open Source Project
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

ifneq ($(strip $(MSSI_MTK_TC1_COMMON_SERVICE)), yes)

LOCAL_PATH := $(call my-dir)

# Build the com.android.emailcommon static library. At the moment, this includes
# the emailcommon files themselves plus everything under src/org (apache code).  All of our
# AIDL files are also compiled into the static library

include $(CLEAR_VARS)

unified_email_unified_src_dir := ../UnifiedEmail/unified_src
unified_email_src_dir := ../UnifiedEmail/src
apache_src_dir := ../UnifiedEmail/src/org

imported_unified_email_files := \
        $(unified_email_src_dir)/com/android/mail/utils/LogTag.java \
        $(unified_email_src_dir)/com/android/mail/utils/LogUtils.java \
        $(unified_email_src_dir)/com/android/mail/providers/UIProvider.java

LOCAL_MODULE := com.android.emailcommon.mtk
LOCAL_STATIC_JAVA_LIBRARIES := \
    guava \
    android-common \
    com.mediatek.protect.email \
    androidx.annotation_annotation \
    com.mediatek.email.ext

# M: add mediatek framework lib.
LOCAL_JAVA_LIBRARIES += mediatek-framework
# M: add java library org.apache.http.legacy.boot
LOCAL_JAVA_LIBRARIES += org.apache.http.legacy

LOCAL_STATIC_ANDROID_LIBRARIES += androidx.core_core

LOCAL_SRC_FILES := $(call all-java-files-under, src/com/android/emailcommon)
# M: add mediatek emailcommon
LOCAL_SRC_FILES += $(call all-java-files-under, src/com/mediatek/emailcommon)
LOCAL_SRC_FILES += \
    src/com/android/emailcommon/service/IEmailService.aidl \
    src/com/android/emailcommon/service/IEmailServiceCallback.aidl \
    src/com/android/emailcommon/service/IPolicyService.aidl \
    src/com/android/emailcommon/service/IAccountService.aidl
LOCAL_SRC_FILES += $(call all-java-files-under, $(apache_src_dir))
LOCAL_SRC_FILES += $(imported_unified_email_files)
LOCAL_SRC_FILES += $(call all-java-files-under, $(unified_email_src_dir)/com/android/emailcommon)
LOCAL_SRC_FILES += $(call all-java-files-under, $(unified_email_src_dir)/com/ibm)

# M: If set LOCAL_SDK_VERSION like this,MTK dependence code will can't successful compile.
#LOCAL_SDK_VERSION := current

LOCAL_RESOURCE_DIR := $(LOCAL_PATH)/res
LOCAL_USE_AAPT2 := true

include $(BUILD_STATIC_JAVA_LIBRARY)
endif

#
# Copyright (C) 2014 MediaTek Inc.
# Modification based on code covered by the mentioned copyright
# and/or permission notice(s).
#
# Copyright 2008, The Android Open Source Project
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

ifneq ($(strip $(MSSI_MTK_TC1_COMMON_SERVICE)),yes)

LOCAL_PATH := $(call my-dir)

# Build the Email application itself, along with its tests and tests for the emailcommon
# static library.  All tests can be run via runtest email

include $(CLEAR_VARS)

# Choose the appropriate AndroidManifest.xml
ifeq ($(strip $(MSSI_MTK_EXCHANGE_SUPPORT)),yes)
    LOCAL_MANIFEST_FILE := AndroidManifest.xml
else
    LOCAL_MANIFEST_FILE := easMnfst/AndroidManifest.xml
endif

# M: replace with mtk-ex
chips_dir := ../../../frameworks/ex/chips/res
unified_email_dir := UnifiedEmail
emailcommon_dir := emailcommon
exchange_dir := Exchange
res_dir := res $(unified_email_dir)/res $(chips_dir) $(exchange_dir)/res $(emailcommon_dir)/res

LOCAL_MODULE_TAGS := optional

LOCAL_SRC_FILES := $(call all-java-files-under, $(unified_email_dir)/src)
LOCAL_SRC_FILES += $(call all-java-files-under, src/com/android)
LOCAL_SRC_FILES += $(call all-java-files-under, provider_src/com/android)
LOCAL_SRC_FILES += $(call all-java-files-under, src/com/beetstra)
# M: add Exchnage files
LOCAL_SRC_FILES += $(call all-java-files-under, $(exchange_dir)/src)
LOCAL_SRC_FILES += $(call all-java-files-under, $(exchange_dir)/build/src)
# M: add mediatek email package
LOCAL_SRC_FILES += $(call all-java-files-under, src/com/mediatek/email)
# M: add mediatek extension package
LOCAL_SRC_FILES += $(call all-java-files-under, ext/src)

LOCAL_RESOURCE_DIR := $(addprefix $(LOCAL_PATH)/, $(res_dir))
LOCAL_USE_AAPT2 := true

# Use assets dir from UnifiedEmail
# (the default package target doesn't seem to deal with multiple asset dirs)
LOCAL_ASSET_DIR := $(LOCAL_PATH)/$(unified_email_dir)/assets

# M: replace with mtk-ex
LOCAL_AAPT_FLAGS := --auto-add-overlay
LOCAL_AAPT_FLAGS += --extra-packages com.android.mtkex.chips:com.android.mail:com.android.email:com.android.exchange:com.android.emailcommon

LOCAL_STATIC_ANDROID_LIBRARIES := \
    com.android.emailcommon.mtk  \
    libphotoviewer_appcompat \
    android-opt-bitmap \
    android-opt-datetimepicker \
    androidx.core_core \
    androidx.media_media \
    androidx.legacy_legacy-support-core-utils \
    androidx.legacy_legacy-support-core-ui \
    androidx.fragment_fragment \
    androidx.appcompat_appcompat \
    androidx.gridlayout_gridlayout \
    androidx.legacy_legacy-support-v13

LOCAL_STATIC_JAVA_LIBRARIES := \
    androidx.annotation_annotation \
    android-common \
    calendar-common \
    androidx.legacy_legacy-support-v4 \
    android-common-chips \
    guava \
    owasp-html-sanitizer

# M: add java library org.apache.http.legacy.boot
LOCAL_JAVA_LIBRARIES := org.apache.http.legacy
# M: add mediatek framework lib.
LOCAL_JAVA_LIBRARIES += mediatek-framework
# M: add mediatek common lib.
LOCAL_JAVA_LIBRARIES += mediatek-common

# M: For read/write secondary storage
LOCAL_CERTIFICATE := platform
LOCAL_PRIVATE_PLATFORM_APIS := true
LOCAL_PACKAGE_NAME := MtkEmail
LOCAL_OVERRIDES_PACKAGES := Email

LOCAL_PRODUCT_MODULE := true

LOCAL_PROGUARD_FLAG_FILES := proguard.flags $(unified_email_dir)/proguard.flags $(exchange_dir)/proguard.flags
ifeq (eng,$(TARGET_BUILD_VARIANT))
  LOCAL_PROGUARD_FLAG_FILES += proguard-test.flags
endif

LOCAL_PRIVATE_PLATFORM_APIS := true
LOCAL_EMMA_COVERAGE_FILTER += +com.android.exchange.*

include $(BUILD_PACKAGE)

# only include rules to build other stuff for the original package, not the derived package.
ifeq ($(strip $(LOCAL_PACKAGE_OVERRIDES)),)
# additionally, build unit tests in a separate .apk
include $(call all-makefiles-under,$(LOCAL_PATH))
endif

endif

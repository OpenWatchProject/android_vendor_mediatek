#
# Copyright (C) 2014 MediaTek Inc.
# Modification based on code covered by the mentioned copyright
# and/or permission notice(s).
#
# Copyright 2007-2008 The Android Open Source Project 

# Only trying to build it in case of telephony add on is supported
ifeq ($(strip $(MSSI_MTK_TELEPHONY_ADD_ON_POLICY)), 0)

#ifeq ($(strip $(MTK_MIGRATION_DONE)),yes)
LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)
# Include res dir from chips
chips_dir := ../../../frameworks/ex/chips/res
contacts_common_dir := src/com/mediatek/Contacts/common
phone_common_dir := ../../../../../../packages/apps/PhoneCommon

src_dirs := src \
    $(phone_common_dir)/src

res_dirs := res \
    $(phone_common_dir)/res \
    $(contacts_common_dir)/res \
    $(chips_dir)

$(shell rm -f $(LOCAL_PATH)/chips)

LOCAL_MODULE_TAGS := optional

LOCAL_SRC_FILES := $(call all-java-files-under, $(src_dirs)) \
    src/com/mediatek/mms/appcallback/IMmsCallbackService.aidl \
    src/com/mediatek/mms/appcallback/SmsEntry.aidl \
    src/com/mediatek/mms/appserviceproxy/IAppServiceProxy.aidl \
    src/com/mediatek/mms/appserviceproxy/AppServiceProxyParam.aidl

LOCAL_RESOURCE_DIR := $(addprefix $(LOCAL_PATH)/, $(res_dirs))

LOCAL_PACKAGE_NAME := MtkMms
#LOCAL_PROPRIETARY_MODULE := true
#LOCAL_MODULE_OWNER := mtk
LOCAL_OVERRIDES_PACKAGES := messaging

# Builds against the public SDK
LOCAL_PRIVATE_PLATFORM_APIS := true
LOCAL_CERTIFICATE := platform

LOCAL_JAVA_LIBRARIES += mediatek-framework
LOCAL_JAVA_LIBRARIES += telephony-common
LOCAL_JAVA_LIBRARIES += org.apache.http.legacy
LOCAL_JAVA_LIBRARIES += voip-common
LOCAL_JAVA_LIBRARIES += ims-common
LOCAL_JAVA_LIBRARIES += mediatek-common
LOCAL_JAVA_LIBRARIES += mediatek-telephony-base
LOCAL_JAVA_LIBRARIES += mediatek-telephony-common
LOCAL_JAVA_LIBRARIES += mediatek-ims-common
LOCAL_STATIC_JAVA_LIBRARIES += android-common jsr305
LOCAL_STATIC_JAVA_LIBRARIES += libchips
LOCAL_STATIC_JAVA_LIBRARIES += android-common-chips
LOCAL_STATIC_JAVA_LIBRARIES += com.android.vcard
LOCAL_STATIC_JAVA_LIBRARIES += com.mediatek.mms.ext
LOCAL_STATIC_JAVA_LIBRARIES += com.mediatek.mms.callback \
    guava \
    android-support-v13 \
    android-support-v4 \
    libphonenumber \
    glide \
    com.android.browser.provider \
    com.mediatek.omadrm.common


LOCAL_SRC_FILES := $(call all-java-files-under, $(src_dirs))
LOCAL_RESOURCE_DIR := $(addprefix $(LOCAL_PATH)/, $(res_dirs))
LOCAL_AAPT_FLAGS := --auto-add-overlay
LOCAL_AAPT_FLAGS += --extra-packages com.android.phone.common
LOCAL_AAPT_FLAGS += --extra-packages com.android.mtkex.chips
LOCAL_AAPT_FLAGS += --extra-packages com.android.contacts.common

ifeq ($(strip $(MTK_RCS_SUPPORT)),yes)
LOCAL_PROGUARD_ENABLED := disabled
#LOCAL_PROGUARD_FLAG_FILES := proguard.flags
else
#LOCAL_PROGUARD_ENABLED := disabled
LOCAL_PROGUARD_FLAG_FILES := proguard.flags
endif

LOCAL_PRIVILEGED_MODULE := true

include $(BUILD_PACKAGE)
include $(CLEAR_VARS)
LOCAL_PREBUILT_STATIC_JAVA_LIBRARIES := apachecommons:libs/commons-io.jar
include $(BUILD_MULTI_PREBUILT)

# This finds and builds the test apk as well, so a single make does both.
include $(call all-makefiles-under,$(LOCAL_PATH))
#endif
endif

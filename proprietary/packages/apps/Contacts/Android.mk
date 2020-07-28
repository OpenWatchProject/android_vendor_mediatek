# Only trying to build it in case of telephony add on is supported
ifeq ($(strip $(MSSI_MTK_TELEPHONY_ADD_ON_POLICY)), 0)

LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)
ifeq ($(strip $(MTK_TB_WIFI_3G_MODE)), WIFI_ONLY)
    LOCAL_MANIFEST_FILE := wifionly/AndroidManifest.xml
endif
LOCAL_MODULE_TAGS := optional

# M: modify source path for mtk contacts repo.
phone_common_dir := ../../../../../../packages/apps/PhoneCommon

# M: add ext for Plugin interface
src_dirs := src src-bind $(phone_common_dir)/src ext
res_dirs := res $(phone_common_dir)/res
asset_dirs := assets

LOCAL_SRC_FILES := $(call all-java-files-under, $(src_dirs))
LOCAL_RESOURCE_DIR := $(addprefix $(LOCAL_PATH)/, $(res_dirs))
# M: add for MTK resource
LOCAL_RESOURCE_DIR += $(LOCAL_PATH)/res_ext
LOCAL_ASSET_DIR := $(addprefix $(LOCAL_PATH)/, $(asset_dirs))

LOCAL_STATIC_ANDROID_LIBRARIES := \
    com.google.android.material_material \
    androidx.transition_transition \
    androidx.legacy_legacy-support-v13 \
    androidx.appcompat_appcompat \
    androidx.cardview_cardview \
    androidx.recyclerview_recyclerview \
    androidx.palette_palette \
    androidx.legacy_legacy-support-v4

LOCAL_STATIC_JAVA_LIBRARIES := \
    android-common \
    com.android.vcard.mtk \
    guava \
    libphonenumber

LOCAL_USE_AAPT2 := true

LOCAL_AAPT_FLAGS := \
    --auto-add-overlay \
    --extra-packages com.android.phone.common

# M: for using framework @{
# add for AOSP
LOCAL_JAVA_LIBRARIES := voip-common
# add for mtk add-on functions.
LOCAL_JAVA_LIBRARIES += telephony-common
LOCAL_JAVA_LIBRARIES += framework
LOCAL_JAVA_LIBRARIES += mediatek-framework
LOCAL_JAVA_LIBRARIES += mediatek-common
LOCAL_JAVA_LIBRARIES += mediatek-telephony-common
LOCAL_JAVA_LIBRARIES += mediatek-telephony-base
LOCAL_JAVA_LIBRARIES += mediatek-telecom-common
# [ConfCall]
LOCAL_JAVA_LIBRARIES += ims-common
LOCAL_JAVA_LIBRARIES += mediatek-ims-common
LOCAL_JAVA_LIBRARIES += mediatek-ims-base
# @}

# M: change for MTK customization @{
LOCAL_PACKAGE_NAME := MtkContacts
LOCAL_CERTIFICATE := platform
LOCAL_OVERRIDES_PACKAGES := Contacts
# @}
LOCAL_PRODUCT_MODULE := true
LOCAL_PRIVILEGED_MODULE := true
LOCAL_REQUIRED_MODULES := privapp_whitelist_com.android.contacts

LOCAL_PROGUARD_FLAG_FILES := proguard.flags

LOCAL_PRIVATE_PLATFORM_APIS := true
LOCAL_MIN_SDK_VERSION := 21

include $(BUILD_PACKAGE)

# Use the following include to make our test apk.
include $(call all-makefiles-under,$(LOCAL_PATH))

endif

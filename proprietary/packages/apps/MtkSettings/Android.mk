# Only trying to build it in case of telephony add on is supported
ifeq ($(strip $(MSSI_MTK_TELEPHONY_ADD_ON_POLICY)), 0)

LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)

LOCAL_SRC_FILES := \
        $(call all-logtags-files-under, src)

LOCAL_MODULE := mtksettings-logtags

include $(BUILD_STATIC_JAVA_LIBRARY)

# Build the Settings APK
include $(CLEAR_VARS)

LOCAL_PACKAGE_NAME := MtkSettings
LOCAL_OVERRIDES_PACKAGES := Settings
LOCAL_PRIVATE_PLATFORM_APIS := true
LOCAL_CERTIFICATE := platform
LOCAL_PRODUCT_MODULE := true
LOCAL_PRIVILEGED_MODULE := true
LOCAL_REQUIRED_MODULES := privapp_whitelist_com.android.settings
LOCAL_MODULE_TAGS := optional
LOCAL_USE_AAPT2 := true

LOCAL_SRC_FILES := $(call all-java-files-under, src)

LOCAL_RESOURCE_DIR := $(LOCAL_PATH)/res
LOCAL_RESOURCE_DIR += $(LOCAL_PATH)/res_ext

LOCAL_STATIC_ANDROID_LIBRARIES := \
    androidx-constraintlayout_constraintlayout \
    androidx.slice_slice-builders \
    androidx.slice_slice-core \
    androidx.slice_slice-view \
    androidx.core_core \
    androidx.appcompat_appcompat \
    androidx.cardview_cardview \
    androidx.preference_preference \
    androidx.recyclerview_recyclerview \
    com.google.android.material_material \
    setupcompat \
    setupdesign

LOCAL_JAVA_LIBRARIES := \
    telephony-common \
    ims-common \
    mediatek-common \
    mediatek-framework \
    mediatek-ims-common \
    mediatek-telephony-base \
    mediatek-telephony-common

LOCAL_STATIC_JAVA_LIBRARIES := \
    nfc_settings_adapter \
    androidx-constraintlayout_constraintlayout-solver \
    androidx.lifecycle_lifecycle-runtime \
    androidx.lifecycle_lifecycle-extensions \
    guava \
    jsr305 \
    com.mediatek.lbs.em2.utils \
    Mtksettings-contextual-card-protos-lite \
    Mtkcontextualcards \
    Mtksettings-log-bridge-protos-lite \
    settings-logtags \
    zxing-core-1.7 \
    com.mediatek.settings.ext
ifeq ($(strip $(MTK_HDMI_SUPPORT)), yes)
ifeq ($(MTK_PLATFORM),$(filter $(MTK_PLATFORM),MT8173 MT8167 MT6735 MT8163 MT6771))
LOCAL_JAVA_LIBRARIES += hdmimanager
endif
endif

LOCAL_PROGUARD_FLAG_FILES := proguard.flags

ifneq ($(INCREMENTAL_BUILDS),)
    LOCAL_PROGUARD_ENABLED := disabled
    LOCAL_JACK_ENABLED := incremental
    LOCAL_JACK_FLAGS := --multi-dex native
endif

include vendor/mediatek/proprietary/packages/apps/SettingsLib/common.mk
include frameworks/base/packages/SettingsLib/common.mk
include frameworks/base/packages/SettingsLib/search/common.mk

include $(BUILD_PACKAGE)

# ====  prebuilt library  ========================
include $(CLEAR_VARS)

LOCAL_PREBUILT_STATIC_JAVA_LIBRARIES := \
    Mtkcontextualcards:libs/contextualcards.aar
include $(BUILD_MULTI_PREBUILT)

# Use the following include to make our test apk.
ifeq (,$(ONE_SHOT_MAKEFILE))
include $(call all-makefiles-under,$(LOCAL_PATH))
endif

endif

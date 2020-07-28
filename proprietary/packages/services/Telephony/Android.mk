ifeq ($(strip $(MSSI_MTK_TELEPHONY_ADD_ON_POLICY)), 0)

LOCAL_PATH:= $(call my-dir)

# Build the Phone app which includes the emergency dialer. See Contacts
# for the 'other' dialer.
include $(CLEAR_VARS)

phone_common_dir := ../../../../../../packages/apps/PhoneCommon

src_dirs := src $(phone_common_dir)/src sip/src
res_dirs := res res_ext $(phone_common_dir)/res sip/res
asset_dirs := assets ecc/output

LOCAL_JAVA_LIBRARIES := \
        telephony-common \
        voip-common \
        ims-common \
        org.apache.http.legacy \
        libprotobuf-java-lite \
        mediatek-common \
        mediatek-framework \
        mediatek-telephony-base \
        mediatek-telephony-common \
        mediatek-ims-common \
        mediatek-telecom-common

LOCAL_STATIC_ANDROID_LIBRARIES := \
        androidx.appcompat_appcompat \
        androidx.preference_preference \
        androidx.recyclerview_recyclerview \
        androidx.legacy_legacy-preference-v14

LOCAL_STATIC_JAVA_LIBRARIES := \
        guava \
        volley \
        android-support-annotations \
        com.mediatek.phone.ext

LOCAL_SRC_FILES := $(call all-java-files-under, $(src_dirs))
LOCAL_SRC_FILES += $(call all-proto-files-under, ecc/proto)
LOCAL_SRC_FILES += \
        src/com/android/phone/EventLogTags.logtags \
        src/com/android/phone/INetworkQueryService.aidl \
        src/com/android/phone/INetworkQueryServiceCallback.aidl
LOCAL_RESOURCE_DIR := $(addprefix $(LOCAL_PATH)/, $(res_dirs))
LOCAL_ASSET_DIR := $(addprefix $(LOCAL_PATH)/, $(asset_dirs))
LOCAL_USE_AAPT2 := true

LOCAL_AAPT_FLAGS := \
    --extra-packages com.android.phone.common \
    --extra-packages com.android.services.telephony.sip

LOCAL_OVERRIDES_PACKAGES := TeleService
LOCAL_PACKAGE_NAME := MtkTeleService
LOCAL_PRIVATE_PLATFORM_APIS := true

LOCAL_CERTIFICATE := platform
LOCAL_PRIVILEGED_MODULE := true

LOCAL_PROGUARD_FLAG_FILES := proguard.flags sip/proguard.flags

include frameworks/base/packages/SettingsLib/common.mk

include $(BUILD_PACKAGE)

# Build the test package
include $(call all-makefiles-under,$(LOCAL_PATH))

endif
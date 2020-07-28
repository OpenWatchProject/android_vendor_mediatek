
# Only trying to build it in case of telephony add on is supported
ifeq ($(strip $(MSSI_MTK_TELEPHONY_ADD_ON_POLICY)), 0)

LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)

LOCAL_MODULE_TAGS := optional

# Only compile source java files in this apk.
LOCAL_SRC_FILES := $(call all-java-files-under, src)
LOCAL_SRC_FILES += \
        src/com/android/providers/contacts/EventLogTags.logtags

LOCAL_JAVA_LIBRARIES := ext \
        telephony-common \
        voip-common \
        mediatek-telephony-base \
        mediatek-telephony-common

LOCAL_STATIC_JAVA_LIBRARIES += android-common com.android.vcard.mtk guava

# The Jacoco tool analyzes code coverage when running unit tests on the
# application. This configuration line selects which packages will be analyzed,
# leaving out code which is tested by other means (e.g. static libraries) that
# would dilute the coverage results. These options do not affect regular
# production builds.
LOCAL_JACK_COVERAGE_INCLUDE_FILTER := com.android.providers.contacts.*

LOCAL_PACKAGE_NAME := MtkContactsProvider
LOCAL_OVERRIDES_PACKAGES := ContactsProvider
LOCAL_PRIVATE_PLATFORM_APIS := true
LOCAL_CERTIFICATE := shared
LOCAL_PRIVILEGED_MODULE := true

LOCAL_PROGUARD_FLAG_FILES := proguard.flags

# M: Resource overlay for GTS.
# GMS will overlay some resources of ContactsProvider for GTS test. Since we
# override AOSP ContactsProvider with MtkContactsProvider, we have to overlay
# these resources by ourselves.
ifeq ($(strip $(BUILD_GMS)),yes)
LOCAL_AAPT_FLAGS += --auto-add-overlay
# Order matters. Directories at left takes precedence over directories at
# right.
LOCAL_RESOURCE_DIR := $(LOCAL_PATH)/res_gms $(LOCAL_PATH)/res
endif

include $(BUILD_PACKAGE)

# Use the following include to make our test apk.
include $(call all-makefiles-under,$(LOCAL_PATH))

endif

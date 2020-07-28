ifneq ($(strip $(MSSI_MTK_TC1_COMMON_SERVICE)), yes)
LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)

LOCAL_MODULE_TAGS := optional
LOCAL_CERTIFICATE := platform

LOCAL_STATIC_JAVA_LIBRARIES := \
        android-common \
        guava \
        android-support-v13 \
        android-support-v4 \
        com.mediatek.browser.ext \
        com.android.browser.provider

LOCAL_JAVA_LIBRARIES += mediatek-framework telephony-common mediatek-common
LOCAL_SRC_FILES := \
        $(call all-java-files-under, src) \
        src/com/android/browser/EventLogTags.logtags

LOCAL_PRODUCT_MODULE := true
LOCAL_PRIVATE_PLATFORM_APIS := true
LOCAL_OVERRIDES_PACKAGES := Browser2
LOCAL_PACKAGE_NAME := MtkBrowser
LOCAL_MODULE_OWNER := mtk

LOCAL_PROGUARD_FLAG_FILES := proguard.flags
#EMMA_INSTRUMENT := true
LOCAL_EMMA_COVERAGE_FILTER := @$(LOCAL_PATH)/browser-emma-filter.txt

include $(BUILD_PACKAGE)

# additionally, build tests in sub-folders in a separate .apk
include $(call all-makefiles-under,$(LOCAL_PATH))
endif

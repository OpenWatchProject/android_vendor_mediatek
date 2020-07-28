LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)
res_dirs := res
LOCAL_MODULE_TAGS := optional

LOCAL_SRC_FILES := $(call all-java-files-under, src)
LOCAL_RESOURCE_DIR := $(addprefix $(LOCAL_PATH)/, $(res_dirs))

#LOCAL_PACKAGE_NAME := DownloadProvider

LOCAL_PACKAGE_NAME := MtkDownloadProvider
LOCAL_PRIVATE_PLATFORM_APIS := true
LOCAL_OVERRIDES_PACKAGES := DownloadProvider
LOCAL_USE_AAPT2 := true

LOCAL_CERTIFICATE := media
LOCAL_PRIVILEGED_MODULE := true
LOCAL_STATIC_JAVA_LIBRARIES := guava \
                  com.mediatek.mtkdownloadmanager.ext \
                  com.mediatek.omadrm.common

LOCAL_JAVA_LIBRARIES += mediatek-framework mediatek-common
LOCAL_JAVA_LIBRARIES += org.apache.http.legacy
#LOCAL_RESOURCE_DIR = $(LOCAL_PATH)/res
# M: add for emma
LOCAL_EMMA_COVERAGE_FILTER := @$(LOCAL_PATH)/downloadprovider-emma-filter.txt
LOCAL_JACK_COVERAGE_INCLUDE_FILTER := com.android.providers.downloads.*
LOCAL_JAVA_LIBRARIES += mediatek-common
#LOCAL_RES_LIBRARIES += mediatek-res

include frameworks/base/packages/SettingsLib/common.mk

include $(BUILD_PACKAGE)

# build UI + tests
include $(call all-makefiles-under,$(LOCAL_PATH))

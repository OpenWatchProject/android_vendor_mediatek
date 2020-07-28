LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)

LOCAL_MODULE_TAGS := optional

LOCAL_SRC_FILES := $(call all-java-files-under, src) \
    ../src/com/android/providers/downloads/OpenHelper.java \
    ../src/com/android/providers/downloads/MediaStoreDownloadsHelper.java \
    ../src/com/android/providers/downloads/Constants.java \
    ../src/com/android/providers/downloads/MtkDownloads.java \
    ../src/com/android/providers/downloads/DownloadDrmHelper.java \
    ../src/com/android/providers/downloads/PluginFactory.java \
    ../src/com/android/providers/downloads/RawDocumentsHelper.java

LOCAL_STATIC_JAVA_LIBRARIES := com.mediatek.mtkdownloadmanager.ext \
                  com.mediatek.omadrm.common
LOCAL_JAVA_LIBRARIES += mediatek-framework
#LOCAL_PACKAGE_NAME := DownloadProviderUi
LOCAL_PACKAGE_NAME := MtkDownloadProviderUi
LOCAL_PRIVATE_PLATFORM_APIS := true
LOCAL_OVERRIDES_PACKAGES := DownloadProviderUi

LOCAL_CERTIFICATE := media
LOCAL_PRIVILEGED_MODULE := true
LOCAL_JAVA_LIBRARIES += mediatek-common
#LOCAL_RES_LIBRARIES += mediatek-res

include $(BUILD_PACKAGE)

LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)

LOCAL_MODULE_TAGS := optional

LOCAL_SRC_FILES := $(call all-java-files-under, src) \
	src/com/android/music/IMediaPlaybackService.aidl \
	src/com/mediatek/bluetooth/avrcp/IBTAvrcpMusic.aidl \
	src/com/mediatek/bluetooth/avrcp/IBTAvrcpMusicCallback.aidl
LOCAL_STATIC_JAVA_LIBRARIES := \
    android-support-v4 \
    com.mediatek.omadrm.common

LOCAL_JAVA_LIBRARIES += mediatek-framework
LOCAL_JAVA_LIBRARIES += voip-common
LOCAL_PACKAGE_NAME := MusicBspPlus
LOCAL_OVERRIDES_PACKAGES := Music

LOCAL_PRIVILEGED_MODULE := true
#LOCAL_PROPRIETARY_MODULE := true
#LOCAL_MODULE_OWNER := mtk
#LOCAL_MODULE_PATH := $(TARGET_OUT_VENDOR)/app



LOCAL_CERTIFICATE := platform
LOCAL_PRIVATE_PLATFORM_APIS := true
LOCAL_PROGUARD_FLAG_FILES := proguard.flags

include $(BUILD_PACKAGE)

# Use the folloing include to make our test apk.
include $(call all-makefiles-under,$(LOCAL_PATH))

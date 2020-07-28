LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)

LOCAL_JAVA_LIBRARIES := mediatek-common mediatek-framework

LOCAL_MODULE_TAGS := optional

LOCAL_SRC_FILES := $(call all-java-files-under, src)

LOCAL_PACKAGE_NAME := VoiceCommand
LOCAL_PRIVATE_PLATFORM_APIS := true
# Enroll sound model to Native, must need a priv app.
LOCAL_PRIVILEGED_MODULE := true

LOCAL_JNI_SHARED_LIBRARIES := libvoicerecognition_jni
LOCAL_MULTILIB := both
#LOCAL_CERTIFICATE := media
LOCAL_CERTIFICATE := platform

LOCAL_PROPRIETARY_MODULE := false
LOCAL_MODULE_OWNER := mtk

#LOCAL_STATIC_JAVA_LIBRARIES := junit legacy-android-test

LOCAL_PROGUARD_FLAG_FILES := proguard.flags

LOCAL_EMMA_COVERAGE_FILTER := +com.mediatek.voicecommand.adapter.*\
                              +com.mediatek.voicecommand.business.*\
                              +com.mediatek.voicecommand.cfg.*\
                              +com.mediatek.voicecommand.data.*\
                              +com.mediatek.voicecommand.mgr.*\
                              +com.mediatek.voicecommand.service.*\
                              +com.mediatek.voicecommand.voicesettings.*

#EMMA_INSTRUMENT := true
include $(BUILD_PACKAGE)

# Use the following include to make our test apk.
include $(call all-makefiles-under,$(LOCAL_PATH))

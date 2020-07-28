LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)

LOCAL_MODULE_TAGS := optional

LOCAL_SRC_FILES := $(call all-java-files-under,src)

#LOCAL_STATIC_JAVA_LIBRARIES := android-common
LOCAL_JAVA_LIBRARIES += mediatek-common

LOCAL_MODULE := com.mediatek.protect.email

#JPE start#
LOCAL_JAVASSIST_ENABLED := true
LOCAL_JAVASSIST_OPTIONS := $(LOCAL_PATH)/jpe.config
#JPE end#

#PROGUARD start#
#LOCAL_PROGUARD_ENABLED := custom
#LOCAL_PROGUARD_FLAG_FILES := proguard.flags
#LOCAL_PROGUARD_SOURCE := javaclassfile
#PROGUARD end#

include $(BUILD_STATIC_JAVA_LIBRARY)

#link Android.mk in tests folder, when use mma command.
include $(call all-makefiles-under,$(LOCAL_PATH))

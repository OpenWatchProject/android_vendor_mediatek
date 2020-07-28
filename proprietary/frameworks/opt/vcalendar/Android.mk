LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)

LOCAL_MODULE_TAGS := optional

LOCAL_SRC_FILES := $(call all-java-files-under,src)

LOCAL_STATIC_JAVA_LIBRARIES := android-common

LOCAL_MODULE := com.mediatek.vcalendar

#PROGUARD start#
#LOCAL_PROGUARD_ENABLED := custom
#LOCAL_PROGUARD_FLAG_FILES := proguard.flags
#PROGUARD end#

include $(BUILD_STATIC_JAVA_LIBRARY)

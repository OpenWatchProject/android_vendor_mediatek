LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)

LOCAL_MODULE_TAGS := optional

LOCAL_SRC_FILES := $(call all-java-files-under,src) $(call all-Iaidl-files-under, src)

LOCAL_AIDL_INCLUDES := $(LOCAL_PATH)/src

LOCAL_MODULE := com.mediatek.tatf.common.plugin
LOCAL_PROPRIETARY_MODULE := true
LOCAL_MODULE_OWNER := mtk
LOCAL_CERTIFICATE := platform
#LOCAL_JAVA_LIBRARIES := legacy-test

include $(BUILD_STATIC_JAVA_LIBRARY)
include $(call all-makefiles-under,$(LOCAL_PATH))

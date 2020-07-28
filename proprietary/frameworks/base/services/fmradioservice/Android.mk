ifneq ($(wildcard vendor/mediatek/proprietary/frameworks/opt/fm/Android.bp),)
LOCAL_PATH:= $(call my-dir)

# merge all required services into one jar
# ============================================================
include $(CLEAR_VARS)

LOCAL_SRC_FILES := $(call all-java-files-under, java)
LOCAL_MODULE_TAGS := optional

LOCAL_MODULE := services.fmradioservice
LOCAL_JAVA_LIBRARIES := services.core mediatek-framework mediatek-common mediatek-fm-framework
LOCAL_MODULE_OWNER := mtk

include $(BUILD_STATIC_JAVA_LIBRARY)
include $(CLEAR_VARS)

include $(call all-makefiles-under, $(LOCAL_PATH))
endif
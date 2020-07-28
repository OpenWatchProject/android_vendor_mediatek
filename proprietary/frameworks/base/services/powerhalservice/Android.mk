LOCAL_PATH:= $(call my-dir)

# merge all required services into one jar
# ============================================================
include $(CLEAR_VARS)

LOCAL_SRC_FILES := $(call all-java-files-under, java)
LOCAL_MODULE_TAGS := optional

LOCAL_MODULE := services.powerhalmgrservice
LOCAL_JAVA_LIBRARIES := services.core mediatek-framework mediatek-common vendor.mediatek.hardware.power-V2.0-java
LOCAL_MODULE_OWNER := mtk
LOCAL_SHARED_LIBRARIES := libpowerhalmgrserv_jni

include $(BUILD_STATIC_JAVA_LIBRARY)

include $(CLEAR_VARS)
LOCAL_SHARED_LIBRARIES := libcutils libpowerhalwrap
LOCAL_LDLIBS := -llog
LOCAL_SRC_FILES := \
        jni/com_mediatek_powerhalservice_PowerHalMgrServiceImpl.cpp
LOCAL_C_INCLUDES += \
    $(JNI_H_INCLUDE) \
    frameworks/base/core/jni \
    $(LOCAL_PATH)/inc
LOCAL_MODULE := libpowerhalmgrserv_jni
LOCAL_MODULE_TAGS := optional
LOCAL_PRELINK_MODULE := false
include $(BUILD_SHARED_LIBRARY)

include $(CLEAR_VARS)

include $(call all-makefiles-under, $(LOCAL_PATH))

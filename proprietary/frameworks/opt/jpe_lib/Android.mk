LOCAL_PATH:= $(call my-dir)

# ====== 64/32 bit library =====
include $(CLEAR_VARS)
LOCAL_MODULE := libnativecheck-jni
#LOCAL_PROPRIETARY_MODULE := true
LOCAL_MODULE_OWNER := mtk
LOCAL_SRC_FILES_arm64 := arm64/libnativecheck-jpi.so
LOCAL_SRC_FILES_arm := arm/libnativecheck-jpi.so
LOCAL_MODULE_CLASS := SHARED_LIBRARIES
LOCAL_MODULE_SUFFIX := .so
LOCAL_MULTILIB := both
include $(BUILD_PREBUILT)

# ====== jpe_tool =====
include $(CLEAR_VARS)
LOCAL_IS_HOST_MODULE := true
LOCAL_MODULE := jpe_tool
LOCAL_PROPRIETARY_MODULE := true
LOCAL_MODULE_OWNER := mtk
LOCAL_MODULE_SUFFIX := .jar
LOCAL_SRC_FILES := jpe_tool.jar
LOCAL_MODULE_CLASS := JAVA_LIBRARIES
include $(BUILD_PREBUILT)

LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)

LOCAL_MODULE_TAGS := optional

LOCAL_AAPT_INCLUDE_ALL_RESOURCES := true

LOCAL_SRC_FILES := $(call all-java-files-under, src)

LOCAL_PACKAGE_NAME := WapiCertManager
#LOCAL_SDK_VERSION := current

LOCAL_PRIVATE_PLATFORM_APIS := true
LOCAL_PRIVILEGED_MODULE := true
LOCAL_USE_AAPT2 := true

LOCAL_MODULE_OWNER := mtk
#LOCAL_PROPRIETARY_MODULE := true

LOCAL_MULTILIB := both

LOCAL_CERTIFICATE := platform

LOCAL_JNI_SHARED_LIBRARIES := libwapi_cert_jni

LOCAL_STATIC_JAVA_LIBRARIES += WapiCertStore

include $(BUILD_PACKAGE)

include $(call all-makefiles-under,$(LOCAL_PATH))

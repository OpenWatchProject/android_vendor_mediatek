LOCAL_PATH:= $(call my-dir)

include $(CLEAR_VARS)

LOCAL_SRC_FILES := $(call all-java-files-under, src)
LOCAL_AIDL_INCLUDES := $(LOCAL_PATH)/aidl

# aidl
LOCAL_SRC_FILES += aidl/com/mediatek/entitlement/ISesServiceListener.aidl \
                   aidl/com/mediatek/entitlement/ISesService.aidl \

LOCAL_JAVA_LIBRARIES += telephony-common
LOCAL_JAVA_LIBRARIES += okhttp
LOCAL_JAVA_LIBRARIES += ims-common

# Add for Plug-in, include the plug-in framework
LOCAL_JAVA_LIBRARIES += mediatek-framework mediatek-telephony-base mediatek-ims-common

LOCAL_STATIC_JAVA_LIBRARIES += entitlement-utils

LOCAL_RESOURCE_DIR := $(LOCAL_PATH)/res


LOCAL_MODULE_TAGS := optional
LOCAL_PACKAGE_NAME := OP07Entitlement
LOCAL_PRIVATE_PLATFORM_APIS := true
LOCAL_MODULE_OWNER := mtk
LOCAL_CERTIFICATE := platform

LOCAL_PROGUARD_ENABLED := disabled
LOCAL_PROGUARD_FLAGS := $(proguard.flags)


include $(BUILD_PACKAGE)

# Other makefiles
include $(call all-makefiles-under, $(LOCAL_PATH))

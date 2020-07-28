LOCAL_PATH:= $(call my-dir)

include $(CLEAR_VARS)

LOCAL_SRC_FILES := $(call all-java-files-under, src)
                   
#LOCAL_SRC_FILES += ../src/com/mediatek/entitlement/EapAka.java
#LOCAL_SRC_FILES += ../src/com/mediatek/entitlement/Utils.java

LOCAL_JAVA_LIBRARIES += telephony-common
LOCAL_JAVA_LIBRARIES += okhttp
LOCAL_JAVA_LIBRARIES += ims-common

LOCAL_STATIC_JAVA_LIBRARIES += entitlement-utils


# Add for Plug-in, include the plug-in framework
LOCAL_JAVA_LIBRARIES += mediatek-framework mediatek-telephony-base

LOCAL_MODULE_TAGS := optional
LOCAL_PACKAGE_NAME := OP15Entitlement
LOCAL_MODULE_OWNER := mtk
LOCAL_CERTIFICATE := platform
LOCAL_PRIVILEGED_MODULE := true
LOCAL_PRIVATE_PLATFORM_APIS := true

LOCAL_PROGUARD_ENABLED := disabled
LOCAL_PROGUARD_FLAGS := $(proguard.flags)

ifeq ($(strip $(MTK_CIP_SUPPORT)),yes)
LOCAL_MODULE_PATH := $(TARGET_CUSTOM_OUT)/app
else
LOCAL_MODULE_PATH := $(TARGET_OUT)/app
endif

include $(BUILD_PACKAGE)

# Other makefiles
include $(call all-makefiles-under, $(LOCAL_PATH))

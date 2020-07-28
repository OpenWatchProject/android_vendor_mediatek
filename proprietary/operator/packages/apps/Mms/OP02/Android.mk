LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)

#This is the target being built.(Name of Jar)
LOCAL_PACKAGE_NAME :=  OP02Mms
#LOCAL_SDK_VERSION := system_current

LOCAL_SRC_FILES := $(call all-java-files-under, src)

#LOCAL_PROPRIETARY_MODULE := true
#LOCAL_MODULE_OWNER := mtk
LOCAL_PRIVATE_PLATFORM_APIS := true
LOCAL_CERTIFICATE := platform

# Specify install path for MTK CIP solution
ifeq ($(strip $(MTK_CIP_SUPPORT)),yes)
LOCAL_MODULE_PATH := $(TARGET_CUSTOM_OUT)/app
else
LOCAL_MODULE_PATH := $(PRODUCT_OUT)/system/app
endif

LOCAL_APPT_FLAGS := --auto-add-overlay
LOCAL_APPT_FLAGS += -- extra-packages com.android.mms

LOCAL_APK_LIBRARIES += MtkMms

LOCAL_JAVA_LIBRARIES += telephony-common \
                        mediatek-telephony-common \
                        mediatek-telephony-base \
                        mediatek-framework \
                        mediatek-common \


LOCAL_PROGUARD_ENABLED := disabled

#Tell it to build a Jar
include $(BUILD_PACKAGE)

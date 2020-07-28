LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)

LOCAL_SRC_FILES := $(call all-java-files-under, src)
LOCAL_SRC_FILES += \
      src/com/mediatek/nfcsettingsadapter/INfcSettingsAdapter.aidl
LOCAL_PROPRIETARY_MODULE := false
LOCAL_MODULE_OWNER := mtk

LOCAL_MODULE := nfc_settings_adapter

# Add for Proguard start
LOCAL_PROGUARD_ENABLED := custom
LOCAL_PROGUARD_FLAG_FILES := proguard.flags
LOCAL_PROGUARD_SOURCE := javaclassfile
LOCAL_EXCLUDED_JAVA_CLASSES := com/mediatek/apm/test/service/**.class
# Add for Proguard end

include $(BUILD_STATIC_JAVA_LIBRARY)

##################################################
include $(CLEAR_VARS)

include $(call all-makefiles-under,$(LOCAL_PATH))

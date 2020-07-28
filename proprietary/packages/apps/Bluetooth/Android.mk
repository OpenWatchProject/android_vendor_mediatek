LOCAL_PATH:= $(call my-dir)

# MAP API module

# MTK internal solution, only for BSP load
ifeq ($(MTK_BSP_PACKAGE) , yes)
include $(CLEAR_VARS)
LOCAL_MODULE_TAGS := optional
LOCAL_SRC_FILES := $(call all-java-files-under, lib/mapapi)
LOCAL_MODULE := mtkbluetooth.mapsapi
include $(BUILD_STATIC_JAVA_LIBRARY)

# Bluetooth APK

include $(CLEAR_VARS)
LOCAL_MODULE_TAGS := optional
LOCAL_SRC_FILES := $(call all-java-files-under, src)
LOCAL_PACKAGE_NAME := MtkBluetooth
LOCAL_OVERRIDES_PACKAGES := Bluetooth
LOCAL_PRIVATE_PLATFORM_APIS := true
LOCAL_CERTIFICATE := platform
LOCAL_USE_AAPT2 := true
LOCAL_JNI_SHARED_LIBRARIES := libmtkbluetooth_jni
LOCAL_JAVA_LIBRARIES := javax.obex telephony-common services.net
LOCAL_STATIC_JAVA_LIBRARIES := \
        com.android.vcard.mtk \
        mtkbluetooth.mapsapi \
        sap-api-java-static \
        services.net \
        libprotobuf-java-lite \
        bluetooth-protos-lite \

LOCAL_STATIC_ANDROID_LIBRARIES := \
        androidx.core_core \
        androidx.lifecycle_lifecycle-livedata \
        androidx.room_room-runtime \

LOCAL_ANNOTATION_PROCESSORS := \
        bt-androidx-annotation-nodeps \
        bt-androidx-room-common-nodeps \
        bt-androidx-room-compiler-nodeps \
        bt-androidx-room-migration-nodeps \
        bt-antlr4-nodeps \
        bt-apache-commons-codec-nodeps \
        bt-auto-common-nodeps \
        bt-javapoet-nodeps \
        bt-kotlin-metadata-nodeps \
        bt-sqlite-jdbc-nodeps \
        bt-jetbrain-nodeps \
        guava-21.0 \
        kotlin-stdlib

LOCAL_ANNOTATION_PROCESSOR_CLASSES := \
        androidx.room.RoomProcessor

# Add for feature: log in data/, should build with READ_LOGS permission
# In user load, MSSI_MTK_LOG_CUSTOMER_SUPPORT=yes should be set to get this permission
ifeq ($(wildcard vendor/mediatek/internal/mtklog_enable),)
  ifeq ($(strip $(TARGET_BUILD_VARIANT)),user)
    ifneq ($(strip $(MSSI_MTK_LOG_CUSTOMER_SUPPORT)),yes)
      LOCAL_MANIFEST_FILE := customer/AndroidManifest.xml
    endif
  endif
endif

LOCAL_REQUIRED_MODULES := libbluetooth
LOCAL_JAVA_LIBRARIES += mediatek-framework
LOCAL_PROGUARD_ENABLED := disabled
include $(BUILD_PACKAGE)

include $(call all-makefiles-under,$(LOCAL_PATH))

include $(CLEAR_VARS)

COMMON_LIBS_PATH := ../../../../../prebuilts/tools/common/m2/repository
ROOM_LIBS_PATH := ../../lib/room

include $(BUILD_HOST_PREBUILT)
endif

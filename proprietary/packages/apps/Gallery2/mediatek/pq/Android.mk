ifneq ($(strip $(MSSI_MTK_TC1_COMMON_SERVICE)), yes)
LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)

LOCAL_MODULE_PATH := $(TARGET_OUT_PRODUCT)/app/MtkGallery2/galleryfeature
LOCAL_MODULE_TAGS := optional
LOCAL_STATIC_JAVA_LIBRARIES := com.mediatek.gallerybasic
LOCAL_JAVA_LIBRARIES += mediatek-framework

LOCAL_SRC_FILES := $(call all-java-files-under, src)
LOCAL_PACKAGE_NAME := Gallery2Pq
LOCAL_PRODUCT_MODULE := true
LOCAL_PRIVATE_PLATFORM_APIS := true
LOCAL_PROGUARD_FLAG_FILES := proguard.flags

include $(BUILD_PACKAGE)

endif

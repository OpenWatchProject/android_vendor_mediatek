ifneq ($(strip $(MSSI_MTK_TC1_COMMON_SERVICE)), yes)
$(info "MTK_OMADRM_SUPPORT=$(MTK_OMADRM_SUPPORT)")

LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)

LOCAL_MODULE_PATH := $(TARGET_OUT_PRODUCT)/app/MtkGallery2/galleryfeature
LOCAL_MODULE_TAGS := optional

LOCAL_SRC_FILES := $(call all-java-files-under, src)

LOCAL_JAVA_LIBRARIES := com.mediatek.gallerybasic
LOCAL_JAVA_LIBRARIES += mediatek-framework
LOCAL_STATIC_JAVA_LIBRARIES += com.mediatek.omadrm.common

LOCAL_PACKAGE_NAME := Gallery2Drm
LOCAL_PRODUCT_MODULE := true
LOCAL_PRIVATE_PLATFORM_APIS := true

LOCAL_PROGUARD_FLAG_FILES := proguard.flags

include $(BUILD_PACKAGE)

endif

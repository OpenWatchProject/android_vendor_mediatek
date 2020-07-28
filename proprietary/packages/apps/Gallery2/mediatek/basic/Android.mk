ifneq ($(strip $(MSSI_MTK_TC1_COMMON_SERVICE)), yes)
LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE := com.mediatek.gallerybasic
LOCAL_STATIC_JAVA_LIBRARIES += com.mediatek.galleryportable
LOCAL_STATIC_JAVA_LIBRARIES += com.mediatek.gallery3d.common2
LOCAL_SRC_FILES := $(call all-java-files-under, src)

include $(BUILD_STATIC_JAVA_LIBRARY)

endif

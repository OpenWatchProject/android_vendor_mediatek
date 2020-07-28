ifneq ($(strip $(MSSI_MTK_TC1_COMMON_SERVICE)), yes)
LOCAL_PATH := $(call my-dir)

ifneq ($(strip $(MTK_BSP_PACKAGE)),yes)

$(info "##### build galleryportable module on tk load #####")

include $(CLEAR_VARS)

LOCAL_MODULE := com.mediatek.galleryportable

LOCAL_JAVA_LIBRARIES += mediatek-framework
LOCAL_JAVA_LIBRARIES += mediatek-common

LOCAL_SRC_FILES := $(call all-java-files-under, src)

include $(BUILD_STATIC_JAVA_LIBRARY)

else
$(info "##### prebuilt galleryportable jar on bsp load #####")
include $(CLEAR_VARS)

LOCAL_PREBUILT_STATIC_JAVA_LIBRARIES := com.mediatek.galleryportable:../../lib/com.mediatek.galleryportable.jar

include $(BUILD_MULTI_PREBUILT)

endif

endif
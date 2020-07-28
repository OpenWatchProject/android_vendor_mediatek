ifneq ($(strip $(MSSI_MTK_TC1_COMMON_SERVICE)), yes)
LOCAL_PATH:= $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE_TAGS := optional

LOCAL_STATIC_JAVA_LIBRARIES := \
    android-support-v4 \
    android-support-v13
LOCAL_STATIC_JAVA_LIBRARIES += com.mediatek.gallery3d.common2
LOCAL_STATIC_JAVA_LIBRARIES += xmp_toolkit
LOCAL_STATIC_JAVA_LIBRARIES += mp4parser

# Add for video drm @{
LOCAL_STATIC_JAVA_LIBRARIES += com.mediatek.omadrm.common
# @}
LOCAL_STATIC_JAVA_LIBRARIES += com.mediatek.gallerybasic
LOCAL_STATIC_JAVA_LIBRARIES += com.mediatek.galleryportable
LOCAL_STATIC_JAVA_LIBRARIES += appluginmanager

LOCAL_JAVA_LIBRARIES += telephony-common

LOCAL_SRC_FILES := \
    $(call all-java-files-under, src) \
    $(call all-renderscript-files-under, src)
LOCAL_SRC_FILES += $(call all-java-files-under, src_pd)

# make plugin @{
LOCAL_SRC_FILES += $(call all-java-files-under, ext/src)
# @}

LOCAL_RESOURCE_DIR += $(LOCAL_PATH)/res \
    $(LOCAL_PATH)/../../../../../../frameworks/support/compat/res

LOCAL_AAPT_FLAGS := --auto-add-overlay
LOCAL_AAPT_FLAGS += --extra-packages android.support.compat

LOCAL_PACKAGE_NAME := MtkGallery2

LOCAL_PRODUCT_MODULE := true
LOCAL_PRIVILEGED_MODULE := true
LOCAL_OVERRIDES_PACKAGES := Gallery Gallery3D GalleryNew3D Gallery2

LOCAL_PRIVATE_PLATFORM_APIS := true

# If this is an unbundled build (to install seprately) then include
# the libraries in the APK, otherwise just put them in /system/lib and
# leave them out of the APK
ifneq (,$(TARGET_BUILD_APPS))
  LOCAL_JNI_SHARED_LIBRARIES := libjni_eglfence_mtk libjni_filtershow_filters_mtk libjni_jpegstream_mtk
else
  LOCAL_REQUIRED_MODULES := libjni_eglfence_mtk libjni_filtershow_filters_mtk libjni_jpegstream_mtk
endif

LOCAL_PROGUARD_FLAG_FILES := proguard.flags

LOCAL_JAVA_LIBRARIES += org.apache.http.legacy

include $(BUILD_PACKAGE)

include $(call all-makefiles-under, jni)

ifeq ($(strip $(LOCAL_PACKAGE_OVERRIDES)),)
# Use the following include to make gallery sub module
include $(call all-makefiles-under, $(LOCAL_PATH))

endif
endif
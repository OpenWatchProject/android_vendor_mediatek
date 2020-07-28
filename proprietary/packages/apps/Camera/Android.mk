#ifneq ($(MTK_BSP_PACKAGE) , yes)
ifneq ($(strip $(MSSI_MTK_TC1_COMMON_SERVICE)), yes)
LOCAL_PATH:= $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE_TAGS := optional
LOCAL_CERTIFICATE := platform

LOCAL_STATIC_JAVA_LIBRARIES := android-support-v13
LOCAL_STATIC_JAVA_LIBRARIES += mp4parser
#LOCAL_STATIC_JAVA_LIBRARIES += stereo_info_accessor
LOCAL_STATIC_JAVA_LIBRARIES += android-support-v7-appcompat
LOCAL_STATIC_JAVA_LIBRARIES += android-support-v8-renderscript
LOCAL_STATIC_JAVA_LIBRARIES += android-ex-camera2
LOCAL_STATIC_JAVA_LIBRARIES += junit


LOCAL_JAVA_LIBRARIES += mediatek-framework
LOCAL_JAVA_LIBRARIES += telephony-common
LOCAL_JAVA_LIBRARIES += mediatek-common

LOCAL_RENDERSCRIPT_TARGET_API := 18
LOCAL_RENDERSCRIPT_COMPATIBILITY := 18
LOCAL_RENDERSCRIPT_FLAGS := -rs-package-name=android.support.v8.renderscript

# Keep track of previously compiled RS files too (from bundled GalleryGoogle).
prev_compiled_rs_files := $(call all-renderscript-files-under, src)

# We already have these files from GalleryGoogle, so don't install them.
LOCAL_RENDERSCRIPT_SKIP_INSTALL := $(prev_compiled_rs_files)

LOCAL_SRC_FILES := $(call all-java-files-under, src) $(prev_compiled_rs_files)
LOCAL_SRC_FILES += ../Camera/src/com/mediatek/camera/addition/remotecamera/service/ICameraClientCallback.aidl
LOCAL_SRC_FILES += ../Camera/src/com/mediatek/camera/addition/remotecamera/service/IMtkCameraService.aidl
LOCAL_AIDL_INCLUDES += $(LOCAL_PATH)/src

LOCAL_RESOURCE_DIR += $(LOCAL_PATH)/res $(LOCAL_PATH)/res_ext $(LOCAL_PATH)/res_v2

#LOCAL_AAPT_FLAGS := --auto-add-overlay
LOCAL_AAPT_FLAGS := --auto-add-overlay --extra-packages com.android.camera

ifeq ($(MTK_CAM_NATIVE_PIP_SUPPORT), yes)
    LOCAL_JNI_SHARED_LIBRARIES := libjni_jpegdecoder
    LOCAL_JNI_SHARED_LIBRARIES += libjni_jpegencoder
endif

LOCAL_PROGUARD_FLAG_FILES := proguard.flags

LOCAL_MIN_SDK_VERSION := 14

LOCAL_PACKAGE_NAME := Camera0
LOCAL_PRIVATE_PLATFORM_APIS := true
LOCAL_PRODUCT_MODULE := true

#overrides aosp camera
LOCAL_OVERRIDES_PACKAGES := Camera2

include $(BUILD_PACKAGE)

include $(call all-makefiles-under, jni)
include $(call all-makefiles-under, packages/apps/Camera/jni_jpegdecoder)
include $(call all-makefiles-under, packages/apps/Camera/jni_jpegencoder)

ifeq ($(strip $(LOCAL_PACKAGE_OVERRIDES)),)

# Use the following include to make gallery test apk
include $(call all-makefiles-under, $(LOCAL_PATH))

endif
endif
#endif

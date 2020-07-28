LOCAL_PATH:= $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE_TAGS := optional
LOCAL_CERTIFICATE := platform

LOCAL_STATIC_JAVA_LIBRARIES := com.mediatek.camera.portability
LOCAL_STATIC_JAVA_LIBRARIES += guava
LOCAL_STATIC_JAVA_LIBRARIES += android-support-v4
LOCAL_STATIC_JAVA_LIBRARIES += android-support-v7-appcompat
LOCAL_STATIC_JAVA_LIBRARIES += appluginmanager
LOCAL_STATIC_JAVA_LIBRARIES += android-support-v7-recyclerview
LOCAL_STATIC_JAVA_LIBRARIES += vendor.mediatek.hardware.camera.bgservice-V1.0-java
LOCAL_STATIC_JAVA_LIBRARIES += libcampostalgo_api

LOCAL_RENDERSCRIPT_TARGET_API := 18
LOCAL_RENDERSCRIPT_COMPATIBILITY := 18


LOCAL_RESOURCE_DIR := $(LOCAL_PATH)/res $(LOCAL_PATH)/../feature/setting/cameraswitcher/res
LOCAL_RESOURCE_DIR += prebuilts/sdk/current/support/v7/appcompat/res
LOCAL_RESOURCE_DIR += $(LOCAL_PATH)/../feature/setting/continuousshot/res
LOCAL_RESOURCE_DIR += $(LOCAL_PATH)/../feature/setting/hdr/res
LOCAL_RESOURCE_DIR += $(LOCAL_PATH)/../feature/setting/flash/res
LOCAL_RESOURCE_DIR += $(LOCAL_PATH)/../feature/setting/focus/res
LOCAL_RESOURCE_DIR += $(LOCAL_PATH)/../feature/setting/exposure/res
LOCAL_RESOURCE_DIR += $(LOCAL_PATH)/../feature/setting/zoom/res
LOCAL_RESOURCE_DIR += $(LOCAL_PATH)/../feature/setting/dng/res
LOCAL_RESOURCE_DIR += $(LOCAL_PATH)/../feature/setting/dualcamerazoom/res
LOCAL_RESOURCE_DIR += $(LOCAL_PATH)/../feature/setting/selftimer/res
LOCAL_RESOURCE_DIR += $(LOCAL_PATH)/../feature/setting/facedetection/res
LOCAL_RESOURCE_DIR += $(LOCAL_PATH)/../feature/setting/picturesize/res
LOCAL_RESOURCE_DIR += $(LOCAL_PATH)/../feature/setting/previewmode/res
LOCAL_RESOURCE_DIR += $(LOCAL_PATH)/../feature/setting/microphone/res
LOCAL_RESOURCE_DIR += $(LOCAL_PATH)/../feature/setting/videoquality/res
LOCAL_RESOURCE_DIR += $(LOCAL_PATH)/../feature/setting/noisereduction/res
LOCAL_RESOURCE_DIR += $(LOCAL_PATH)/../feature/setting/fps60/res
LOCAL_RESOURCE_DIR += $(LOCAL_PATH)/../feature/setting/eis/res
LOCAL_RESOURCE_DIR += $(LOCAL_PATH)/../feature/setting/ais/res
LOCAL_RESOURCE_DIR += $(LOCAL_PATH)/../feature/setting/scenemode/res
LOCAL_RESOURCE_DIR += $(LOCAL_PATH)/../feature/setting/whitebalance/res
LOCAL_RESOURCE_DIR += $(LOCAL_PATH)/../feature/setting/antiflicker/res
LOCAL_RESOURCE_DIR += $(LOCAL_PATH)/../feature/setting/zsd/res
LOCAL_RESOURCE_DIR += $(LOCAL_PATH)/../feature/setting/iso/res
LOCAL_RESOURCE_DIR += $(LOCAL_PATH)/../feature/setting/aaaroidebug/res
LOCAL_RESOURCE_DIR += $(LOCAL_PATH)/../feature/mode/panorama/res
LOCAL_RESOURCE_DIR += $(LOCAL_PATH)/../feature/setting/shutterspeed/res
LOCAL_RESOURCE_DIR += $(LOCAL_PATH)/../feature/mode/longexposure/res
LOCAL_RESOURCE_DIR += $(LOCAL_PATH)/../feature/mode/hdr/res
LOCAL_RESOURCE_DIR += $(LOCAL_PATH)/../feature/mode/slowmotion/res
LOCAL_RESOURCE_DIR += $(LOCAL_PATH)/../feature/mode/vsdof/res
LOCAL_RESOURCE_DIR += $(LOCAL_PATH)/../feature/setting/formats/res
LOCAL_RESOURCE_DIR += $(LOCAL_PATH)/../feature/setting/slowmotionquality/res
LOCAL_RESOURCE_DIR += $(LOCAL_PATH)/../feature/mode/dof/res
LOCAL_RESOURCE_DIR += $(LOCAL_PATH)/../feature/mode/matrixdisplay/res
LOCAL_RESOURCE_DIR += $(LOCAL_PATH)/../feature/mode/facebeauty/res
LOCAL_RESOURCE_DIR += $(LOCAL_PATH)/../feature/mode/vfacebeauty/res


LOCAL_SRC_FILES := $(call all-java-files-under, src) $(call all-renderscript-files-under, src)
LOCAL_SRC_FILES += $(call all-java-files-under, ../feature/setting/cameraswitcher/src)
LOCAL_SRC_FILES += $(call all-java-files-under, ../feature/setting/continuousshot/src)
LOCAL_SRC_FILES += $(call all-java-files-under, ../feature/setting/hdr/src)
LOCAL_SRC_FILES += $(call all-java-files-under, ../feature/setting/flash/src)
LOCAL_SRC_FILES += $(call all-java-files-under, ../feature/setting/focus/src)
LOCAL_SRC_FILES += $(call all-java-files-under, ../feature/setting/exposure/src)
LOCAL_SRC_FILES += $(call all-java-files-under, ../feature/setting/zoom/src)
LOCAL_SRC_FILES += $(call all-java-files-under, ../feature/setting/facedetection/src)
LOCAL_SRC_FILES += $(call all-java-files-under, ../feature/setting/dng/src)
LOCAL_SRC_FILES += $(call all-java-files-under, ../feature/setting/dualcamerazoom/src)
LOCAL_SRC_FILES += $(call all-java-files-under, ../feature/setting/selftimer/src)
LOCAL_SRC_FILES += $(call all-java-files-under, ../feature/setting/picturesize/src)
LOCAL_SRC_FILES += $(call all-java-files-under, ../feature/setting/previewmode/src)
LOCAL_SRC_FILES += $(call all-java-files-under, ../feature/setting/microphone/src)
LOCAL_SRC_FILES += $(call all-java-files-under, ../feature/setting/videoquality/src)
LOCAL_SRC_FILES += $(call all-java-files-under, ../feature/setting/noisereduction/src)
LOCAL_SRC_FILES += $(call all-java-files-under, ../feature/setting/fps60/src)
LOCAL_SRC_FILES += $(call all-java-files-under, ../feature/setting/eis/src)
LOCAL_SRC_FILES += $(call all-java-files-under, ../feature/setting/ais/src)
LOCAL_SRC_FILES += $(call all-java-files-under, ../feature/setting/scenemode/src)
LOCAL_SRC_FILES += $(call all-java-files-under, ../feature/setting/whitebalance/src)
LOCAL_SRC_FILES += $(call all-java-files-under, ../feature/setting/antiflicker/src)
LOCAL_SRC_FILES += $(call all-java-files-under, ../feature/setting/zsd/src)
LOCAL_SRC_FILES += $(call all-java-files-under, ../feature/setting/iso/src)
LOCAL_SRC_FILES += $(call all-java-files-under, ../feature/setting/aaaroidebug/src)
LOCAL_SRC_FILES += $(call all-java-files-under, ../feature/mode/panorama/src)
LOCAL_SRC_FILES += $(call all-java-files-under, ../feature/setting/shutterspeed/src)
LOCAL_SRC_FILES += $(call all-java-files-under, ../feature/mode/longexposure/src)
LOCAL_SRC_FILES += $(call all-java-files-under, ../feature/mode/hdr/src)
LOCAL_SRC_FILES += $(call all-java-files-under, ../feature/mode/slowmotion/src)
LOCAL_SRC_FILES += $(call all-java-files-under, ../feature/mode/vsdof/src)
LOCAL_SRC_FILES += $(call all-java-files-under, ../feature/setting/formats/src)
LOCAL_SRC_FILES += $(call all-java-files-under, ../feature/setting/slowmotionquality/src)
LOCAL_SRC_FILES += $(call all-java-files-under, ../feature/setting/postview/src)
LOCAL_SRC_FILES += $(call all-java-files-under, ../feature/mode/dof/src)
LOCAL_SRC_FILES += $(call all-java-files-under, ../feature/mode/matrixdisplay/src)
LOCAL_SRC_FILES += $(call all-java-files-under, ../feature/mode/facebeauty/src)
LOCAL_SRC_FILES += $(call all-java-files-under, ../feature/mode/vfacebeauty/src)

LOCAL_SRC_FILES += $(call all-java-files-under, ../common/src)
#LOCAL_RENDERSCRIPT_FLAGS := -rs-package-name=android.support.v8.renderscript
#LOCAL_JNI_SHARED_LIBRARIES := librsjni

LOCAL_AAPT_FLAGS := --auto-add-overlay
LOCAL_AAPT_FLAGS += --no-version-vectors
LOCAL_AAPT_FLAGS += --extra-packages android.support.v7.appcompat

LOCAL_MIN_SDK_VERSION := 21

LOCAL_PROGUARD_FLAG_FILES := proguard.flags
LOCAL_PACKAGE_NAME := Camera
LOCAL_PRIVATE_PLATFORM_APIS := true
LOCAL_PRODUCT_MODULE := true
#overrides aosp camera
LOCAL_OVERRIDES_PACKAGES := Camera2
include $(BUILD_PACKAGE)

LOCAL_PATH := $(call my-dir)

ifneq ($(filter $(TARGET_BUILD_VARIANT),eng userdebug),)
gpu_src_path := prebuilt_eng
else
gpu_src_path := prebuilt_user
endif

include $(CLEAR_VARS)
LOCAL_MODULE := libGLES_mali
LOCAL_PROPRIETARY_MODULE := true
LOCAL_MODULE_OWNER := mtk
LOCAL_SRC_FILES_64 := $(gpu_src_path)/arm64/libGLES_mali.so
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_CLASS := SHARED_LIBRARIES
LOCAL_MODULE_SUFFIX := .so
LOCAL_MODULE_RELATIVE_PATH := egl
LOCAL_PROPRIETARY_MODULE := true
LOCAL_MODULE_OWNER := mtk
LOCAL_MULTILIB := 64

#symbol link for vulkan.$(TARGET_BOARD_PLATFORM).so
#LOCAL_POST_INSTALL_CMD := \
#	mkdir -p $(TARGET_OUT)/vendor/lib64/hw; \
#	ln -sf /system/vendor/lib64/egl/libGLES_mali.so $(TARGET_OUT)/vendor/lib64/hw/vulkan.$(TARGET_BOARD_PLATFORM).so
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE := libGLES_mali
LOCAL_PROPRIETARY_MODULE := true
LOCAL_MODULE_OWNER := mtk
LOCAL_SRC_FILES_32 := $(gpu_src_path)/arm/libGLES_mali.so
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_CLASS := SHARED_LIBRARIES
LOCAL_MODULE_SUFFIX := .so
LOCAL_MODULE_RELATIVE_PATH := egl
LOCAL_PROPRIETARY_MODULE := true
LOCAL_MODULE_OWNER := mtk
LOCAL_MULTILIB := 32
#symbol link for vulkan.$(TARGET_BOARD_PLATFORM).so
#LOCAL_POST_INSTALL_CMD := \
#	mkdir -p $(TARGET_OUT)/vendor/lib/hw; \
#	ln -sf /system/vendor/lib/egl/libGLES_mali.so $(TARGET_OUT)/vendor/lib/hw/vulkan.$(TARGET_BOARD_PLATFORM).so
include $(BUILD_PREBUILT)

# NOTE for gralloc #
# prebuild-tool always rename the galloc by the format gralloc.mali.so
# we need to rename to $(TARGET_BOARD_PLATFORM) again

include $(CLEAR_VARS)
LOCAL_MODULE := gralloc.$(TARGET_BOARD_PLATFORM)
LOCAL_PROPRIETARY_MODULE := true
LOCAL_MODULE_OWNER := mtk
LOCAL_SRC_FILES_64 := $(gpu_src_path)/arm64/gralloc.mali.so
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_CLASS := SHARED_LIBRARIES
LOCAL_MODULE_SUFFIX := .so
LOCAL_MODULE_RELATIVE_PATH := hw
LOCAL_PROPRIETARY_MODULE := true
LOCAL_MODULE_OWNER := mtk
LOCAL_MULTILIB := 64
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE := gralloc.$(TARGET_BOARD_PLATFORM)
LOCAL_PROPRIETARY_MODULE := true
LOCAL_MODULE_OWNER := mtk
LOCAL_SRC_FILES_32 := $(gpu_src_path)/arm/gralloc.mali.so
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_CLASS := SHARED_LIBRARIES
LOCAL_MODULE_SUFFIX := .so
LOCAL_MODULE_RELATIVE_PATH := hw
LOCAL_PROPRIETARY_MODULE := true
LOCAL_MODULE_OWNER := mtk
LOCAL_MULTILIB := 32
include $(BUILD_PREBUILT)


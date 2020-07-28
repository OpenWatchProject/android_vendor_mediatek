ifeq ($(strip $(MTK_TEE_GP_SUPPORT)), yes)

LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)
LOCAL_MODULE := drinfo_core
LOCAL_MULTILIB := 32
LOCAL_MODULE_CLASS = STATIC_LIBRARIES
LOCAL_MODULE_SUFFIX = .lib
LOCAL_SRC_FILES := ../plat/$(MTK_PLATFORM_DIR)/lib/drinfo_core.lib
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE := drinfo_api
LOCAL_MULTILIB := 32
LOCAL_MODULE_CLASS = STATIC_LIBRARIES
LOCAL_MODULE_SUFFIX = .lib
LOCAL_SRC_FILES := lib/drinfo_api.lib
include $(BUILD_PREBUILT)

endif # ifeq ($(strip $(MTK_TEE_GP_SUPPORT)), yes)

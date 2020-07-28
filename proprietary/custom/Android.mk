LOCAL_PATH := $(call my-dir)

LOCAL_BASE_MODULES := $(call all-makefiles-under,$(LOCAL_PATH)/$(MTK_BASE_PROJECT))
LOCAL_PLATFORM_MODULES := $(call all-makefiles-under, $(LOCAL_PATH)/$(shell echo $(MTK_PLATFORM) | tr '[A-Z]' '[a-z]'))
LOCAL_COMMON_MODULES := $(call all-makefiles-under,$(LOCAL_PATH)/common)

# ISP3 chip HAL1 use 'hal1' subfolder
ifneq (,$(filter $(strip $(TARGET_BOARD_PLATFORM)), mt6739))
ifeq ($(strip $(MTK_CAM_LEGACY_HAL_SUPPORT)),yes)
ifeq ($(strip $(MTK_CAM_HAL_VERSION)),1)
    LOCAL_PLATFORM_MODULES := $(call all-makefiles-under, $(LOCAL_PATH)/$(shell echo $(MTK_PLATFORM)/hal1 | tr '[A-Z]' '[a-z]'))
endif
endif
endif

ifneq (,$(filter $(strip $(TARGET_BOARD_PLATFORM)), mt6761))
ifneq ($(strip $(MTK_CAM_HAL_VERSION)),3)
    LOCAL_PLATFORM_MODULES += $(call all-makefiles-under, $(LOCAL_PATH)/$(shell echo $(MTK_PLATFORM)/hal1 | tr '[A-Z]' '[a-z]'))
endif
endif

define all-c-cpp-files-under
$(patsubst ./%,%, \
  $(shell cd $(LOCAL_PATH) ; \
          find $(1) -maxdepth 1 \( -name "*.c" -or -name "*.cpp" \) -and -not -name ".*") \
 )
endef

ifneq ($(MTK_EMULATOR_SUPPORT), yes)
include $(LOCAL_COMMON_MODULES) $(LOCAL_BASE_MODULES) $(LOCAL_PLATFORM_MODULES)
else
include $(LOCAL_COMMON_MODULES) $(LOCAL_BASE_MODULES)
endif

include $(CLEAR_VARS)
LOCAL_MODULE := libcameracustom_headers
LOCAL_EXPORT_C_INCLUDE_DIRS +=$(MTK_PATH_COMMON)/hal/inc
LOCAL_EXPORT_C_INCLUDE_DIRS +=$(MTK_PATH_CUSTOM_PLATFORM)/hal/inc
LOCAL_EXPORT_C_INCLUDE_DIRS +=$(MTK_PATH_CUSTOM_PLATFORM)/hal/inc/$(MTK_CAM_SW_VERSION)
LOCAL_EXPORT_C_INCLUDE_DIRS +=$(MTK_PATH_CUSTOM_PLATFORM)/hal/inc/aaa
LOCAL_EXPORT_C_INCLUDE_DIRS +=$(MTK_PATH_CUSTOM_PLATFORM)/hal/inc/aaa/$(MTK_CAM_SW_VERSION)
LOCAL_EXPORT_C_INCLUDE_DIRS +=$(MTK_PATH_CUSTOM_PLATFORM)/hal/inc/isp_tuning
LOCAL_EXPORT_C_INCLUDE_DIRS +=$(MTK_PATH_CUSTOM_PLATFORM)/hal/inc/isp_tuning/$(MTK_CAM_SW_VERSION)
LOCAL_EXPORT_C_INCLUDE_DIRS +=$(MTK_PATH_SOURCE)/custom
LOCAL_EXPORT_C_INCLUDE_DIRS +=$(MTK_PATH_COMMON)/hal/inc/custom/aaa
include $(BUILD_HEADER_LIBRARY)

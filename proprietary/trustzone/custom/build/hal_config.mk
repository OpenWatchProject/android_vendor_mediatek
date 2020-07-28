
TEE_HAL_EXCLUDE_PATTERNS :=

ifeq ($(strip $(MTK_TEE_SUPPORT)),yes)
#TEE_HAL_EXCLUDE_PATTERNS += sec/
#TEE_HAL_EXCLUDE_PATTERNS += secmem/
#TEE_HAL_EXCLUDE_PATTERNS += pmem/
TEE_HAL_EXCLUDE_PATTERNS += devinfo/
#TEE_HAL_EXCLUDE_PATTERNS += rpmb/
#TEE_HAL_EXCLUDE_PATTERNS += mpu/
#TEE_HAL_EXCLUDE_PATTERNS += tee_sanity/
endif

# MICROTRUST
ifeq ($(strip $(MICROTRUST_TEE_SUPPORT)),yes)
TEE_HAL_EXCLUDE_PATTERNS += rpmb/
endif

# SVP
ifneq ($(strip $(MTK_SEC_VIDEO_PATH_SUPPORT)),yes)
TEE_HAL_EXCLUDE_PATTERNS += modular_drm/
TEE_HAL_EXCLUDE_PATTERNS += video/
TEE_HAL_EXCLUDE_PATTERNS += drm_hdcp_common/
ifneq ($(strip $(MTK_CAM_SECURITY_SUPPORT)),yes)
TEE_HAL_EXCLUDE_PATTERNS += cmdq/
TEE_HAL_EXCLUDE_PATTERNS += m4u/
endif
endif

# SEC_CAM
ifneq ($(strip $(MTK_CAM_SECURITY_SUPPORT)),yes)
TEE_HAL_EXCLUDE_PATTERNS += camera/
TEE_HAL_EXCLUDE_PATTERNS += imgsensor/
endif

# DRM_KEY_MNG
ifneq ($(strip $(MTK_DRM_KEY_MNG_SUPPORT)),yes)
TEE_HAL_EXCLUDE_PATTERNS += keyinstall/
endif

TEE_HAL_PROTECT_PATH := $(MTK_PATH_SOURCE)/trustzone/common/hal/secure
ifeq ($(strip $(MTK_TEE_GP_SUPPORT)),yes)
ifneq ($(wildcard $(TEE_HAL_PROTECT_PATH)),)
TEE_HAL_PROTECT_PLATFORM_PATH := $(addsuffix /$(MTK_PLATFORM_DIR),$(shell find $(TEE_HAL_PROTECT_PATH) -maxdepth 4 -type d -name platform))
TEE_HAL_PROTECT_COMMON_PATH := $(shell find $(TEE_HAL_PROTECT_PATH) -maxdepth 4 -type d -name common)
TEE_HAL_PROTECT_MAKEFILES := $(sort $(wildcard $(foreach d,$(TEE_HAL_PROTECT_PLATFORM_PATH) $(TEE_HAL_PROTECT_COMMON_PATH),$(foreach t,drv drv_api ta ca,$(d)/$(t)/Android.mk))))
TEE_HAL_EXCLUDE_MAKEFILES := $(foreach f,$(TEE_HAL_PROTECT_MAKEFILES),\
	$(foreach p,$(TEE_HAL_EXCLUDE_PATTERNS),\
		$(if $(findstring $(p),$(f)),$(f))\
	)\
)
TEE_HAL_PROTECT_MAKEFILES := $(filter-out $(TEE_HAL_EXCLUDE_MAKEFILES),$(TEE_HAL_PROTECT_MAKEFILES))
$(warning TEE_HAL_PROTECT_MAKEFILES=$(TEE_HAL_PROTECT_MAKEFILES))

ifeq ($(strip $(MTK_CAM_SECURITY_SUPPORT)),yes)
TEE_HAL_CAMERA_MAKEFILES := $(TEE_HAL_PROTECT_PATH)/trustlets/camera/Android.mk
endif

endif
else # MTK_TEE_GP_SUPPORT = no
ifeq ($(strip $(MTK_SEC_VIDEO_PATH_SUPPORT)),yes)
ifneq ($(wildcard $(TEE_HAL_PROTECT_PATH)),)
TEE_HAL_PROTECT_MAKEFILES += $(TEE_HAL_PROTECT_PATH)/trustlets/modular_drm/common/ca/Android.mk
TEE_HAL_PROTECT_MAKEFILES += $(TEE_HAL_PROTECT_PATH)/trustlets/modular_drm/common/ta/Android.mk
TEE_HAL_PROTECT_MAKEFILES += $(TEE_HAL_PROTECT_PATH)/trustlets/modular_drm/common/drv/Android.mk
TEE_HAL_PROTECT_MAKEFILES += $(TEE_HAL_PROTECT_PATH)/trustlets/drm_hdcp_common/common/drv/Android.mk
endif
endif
ifeq ($(strip $(TRUSTONIC_TEE_SUPPORT)),yes)
ifneq ($(wildcard $(TEE_HAL_PROTECT_PATH)),)
TEE_HAL_PROTECT_MAKEFILES += $(TEE_HAL_PROTECT_PATH)/trustlets/secmem/common/ta/Android.mk
TEE_HAL_PROTECT_MAKEFILES += $(TEE_HAL_PROTECT_PATH)/trustlets/secmem/common/drv/Android.mk
TEE_HAL_PROTECT_MAKEFILES += $(TEE_HAL_PROTECT_PATH)/trustlets/secmem/common/drv_api/Android.mk
endif
endif
$(warning TEE_HAL_PROTECT_MAKEFILES=$(TEE_HAL_PROTECT_MAKEFILES))
endif

TEE_HAL_SOURCE_PATH := $(MTK_PATH_SOURCE)/trustzone/common/hal/source
ifeq ($(strip $(MTK_TEE_GP_SUPPORT)),yes)
ifneq ($(wildcard $(TEE_HAL_SOURCE_PATH)),)
TEE_HAL_SOURCE_PLATFORM_PATH := $(addsuffix /$(MTK_PLATFORM_DIR),$(shell find $(TEE_HAL_SOURCE_PATH) -maxdepth 4 -type d -name platform))
TEE_HAL_SOURCE_COMMON_PATH := $(shell find $(TEE_HAL_SOURCE_PATH) -maxdepth 4 -type d -name common)
TEE_HAL_SOURCE_MAKEFILES := $(sort $(wildcard $(foreach d,$(TEE_HAL_SOURCE_PLATFORM_PATH) $(TEE_HAL_SOURCE_COMMON_PATH),$(foreach t,drv drv_api ta ca,$(d)/$(t)/Android.mk) $(d)/Android.mk)))
TEE_HAL_EXCLUDE_MAKEFILES := $(foreach f,$(TEE_HAL_SOURCE_MAKEFILES),\
	$(foreach p,$(TEE_HAL_EXCLUDE_PATTERNS),\
		$(if $(findstring $(p),$(f)),$(f))\
	)\
)
TEE_HAL_SOURCE_MAKEFILES := $(filter-out $(TEE_HAL_EXCLUDE_MAKEFILES),$(TEE_HAL_SOURCE_MAKEFILES))
$(warning TEE_HAL_SOURCE_MAKEFILES=$(TEE_HAL_SOURCE_MAKEFILES))
endif
else # MTK_TEE_GP_SUPPORT = no
TEE_HAL_SOURCE_MAKEFILES :=
ifeq ($(strip $(TRUSTONIC_TEE_SUPPORT)),yes)
ifneq ($(wildcard $(TEE_HAL_SOURCE_PATH)),)
TEE_HAL_SOURCE_MAKEFILES += $(TEE_HAL_SOURCE_PATH)/trustlets/mpu/platform/$(MTK_PLATFORM_DIR)/drv/Android.mk
endif
endif
$(warning TEE_HAL_SOURCE_MAKEFILES=$(TEE_HAL_SOURCE_MAKEFILES))
endif

TEE_HAL_EXCLUDE_MODULES := $(ALL_MODULES)
$(foreach mk,$(TEE_HAL_PROTECT_MAKEFILES) $(TEE_HAL_SOURCE_MAKEFILES) $(TEE_HAL_CAMERA_MAKEFILES),\
	$(info including $(mk) ...)\
	$(eval include $(mk))\
)

TEE_HAL_SRC_MODULES := $(filter-out $(TEE_HAL_EXCLUDE_MODULES),$(ALL_MODULES))
TEE_HAL_modules_to_check := $(call module-built-files,$(TEE_HAL_SRC_MODULES))
TEE_HAL_modules_to_install := $(call module-installed-files,$(TEE_HAL_SRC_MODULES))

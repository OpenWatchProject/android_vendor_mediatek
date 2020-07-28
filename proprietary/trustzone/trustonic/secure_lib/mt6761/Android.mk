# TEE_RELEASE_TRUSTLET_PATH was defined vendor/mediatek/proprietary/trustzone/custom/build/tee_config.mk in the past
TRUSTZONE_CUSTOM_BUILD_PATH := $(MTK_PATH_SOURCE)/trustzone/custom/build
ifeq ($(strip $(TRUSTONIC_TEE_SUPPORT)),yes)
ifneq ($(wildcard $(TRUSTZONE_CUSTOM_BUILD_PATH)/tee_config.mk),)
ifeq ($(strip $(shell grep TEE_RELEASE_TRUSTLET_PATH $(TRUSTZONE_CUSTOM_BUILD_PATH)/tee_config.mk)),)

LOCAL_PATH := $(call my-dir)
TRUSTZONE_ROOT_DIR := $(PWD)
TRUSTZONE_OUTPUT_PATH := $(PRODUCT_OUT)/trustzone

include $(TRUSTZONE_CUSTOM_BUILD_PATH)/common_config.mk

TEE_BUILD_MODE ?= Debug Release
ifeq ($(TARGET_BUILD_VARIANT),eng)
  TEE_INSTALL_MODE ?= Debug
else
  TEE_INSTALL_MODE ?= Release
endif
TEE_TOOLCHAIN ?= GNU


TEE_RELEASE_TRUSTLET_PATH := $(LOCAL_PATH)
TEE_CRYPTO_TRUSTLET_PATH := $(MTK_PATH_SOURCE)/trustzone/trustonic/crypto/trustlets
TEE_PRIVATE_TRUSTLET_PATH := $(MTK_PATH_SOURCE)/trustzone/trustonic/private/trustlets
TEE_INTERNAL_TRUSTLET_PATH := $(MTK_PATH_SOURCE)/trustzone/trustonic/internal/trustlets
TEE_PROTECT_TRUSTLET_PATH := $(MTK_PATH_SOURCE)/trustzone/trustonic/secure/trustlets
TEE_SOURCE_TRUSTLET_PATH := $(MTK_PATH_SOURCE)/trustzone/trustonic/source/trustlets

TEE_ALL_MODULE_MAKEFILE :=
ifneq ($(wildcard vendor/mediatek/proprietary/trustzone/trustonic/internal/bsp),)
define mtk_tee_find_module_makefile
$(firstword \
  $(wildcard \
    $(TEE_CRYPTO_TRUSTLET_PATH)/$(strip $(1))/$(if $(filter platform,$(2)),platform/$(ARCH_MTK_PLATFORM),$(strip $(2)))/$(strip $(3))/Locals/Code/makefile.mk \
    $(TEE_PRIVATE_TRUSTLET_PATH)/$(strip $(1))/$(if $(filter platform,$(2)),platform/$(ARCH_MTK_PLATFORM),$(strip $(2)))/$(strip $(3))/Locals/Code/makefile.mk \
    $(TEE_INTERNAL_TRUSTLET_PATH)/$(strip $(1))/$(if $(filter platform,$(2)),platform/$(ARCH_MTK_PLATFORM),$(strip $(2)))/$(strip $(3))/Locals/Code/makefile.mk \
    $(TEE_INTERNAL_TRUSTLET_PATH)/$(strip $(1))/$(if $(filter platform,$(2)),platform/$(ARCH_MTK_PLATFORM),$(strip $(2)))/$(strip $(3))/makefile.mk \
    $(TEE_PROTECT_TRUSTLET_PATH)/$(strip $(1))/$(if $(filter platform,$(2)),platform/$(ARCH_MTK_PLATFORM),$(strip $(2)))/$(strip $(3))/Locals/Code/makefile.mk \
    $(TEE_SOURCE_TRUSTLET_PATH)/$(strip $(1))/$(if $(filter platform,$(2)),platform/$(ARCH_MTK_PLATFORM),$(strip $(2)))/$(strip $(3))/Locals/Code/makefile.mk \
    $(TEE_SOURCE_TRUSTLET_PATH)/$(strip $(1))/$(if $(filter platform,$(2)),platform/$(ARCH_MTK_PLATFORM),$(strip $(2)))/$(strip $(3))/makefile.mk \
    $(TEE_RELEASE_TRUSTLET_PATH)/$(strip $(1))/$(if $(filter platform,$(2)),platform/$(ARCH_MTK_PLATFORM),$(strip $(2)))/$(strip $(3))/makefile.mk \
  )\
)
endef
else
define mtk_tee_find_module_makefile
$(firstword \
  $(wildcard \
    $(TEE_RELEASE_TRUSTLET_PATH)/$(strip $(1))/$(if $(filter platform,$(2)),platform/$(ARCH_MTK_PLATFORM),$(strip $(2)))/$(strip $(3))/makefile.mk \
  )\
)
endef
endif
include $(MTK_PATH_SOURCE)/trustzone/trustonic/source/build/platform/$(ARCH_MTK_PLATFORM)/tee_config.mk
TEE_ALL_MODULE_MAKEFILE := $(filter $(TEE_RELEASE_TRUSTLET_PATH)/%,$(TEE_ALL_MODULE_MAKEFILE))

TEE_MACH_TYPE := $(MTK_MACH_TYPE)
TEE_GLOBAL_PLATFORM_OPTION :=
TEE_GLOBAL_PLATFORM_OPTION += TEE_MACH_TYPE=$(TEE_MACH_TYPE)
TEE_GLOBAL_PLATFORM_OPTION += MTK_TEE_GP_SUPPORT=$(MTK_TEE_GP_SUPPORT)

# SDK path
TLSDK_DIR := $(TRUSTZONE_ROOT_DIR)/$(TEE_SOURCE_SDK_PATH)/TlSdk/Out
INTERNAL_TLSDK_DIR := $(TRUSTZONE_ROOT_DIR)/$(TEE_INTERNAL_SDK_PATH)/TlSdk/Out
DRSDK_DIR := $(TRUSTZONE_ROOT_DIR)/$(TEE_SOURCE_SDK_PATH)/DrSdk/Out
INTERNAL_DRSDK_DIR := $(TRUSTZONE_ROOT_DIR)/$(TEE_INTERNAL_SDK_PATH)/DrSdk/Out

TEE_GLOBAL_PLATFORM_OPTION += TLSDK_DIR=$(TLSDK_DIR)
TEE_GLOBAL_PLATFORM_OPTION += INTERNAL_TLSDK_DIR=$(INTERNAL_TLSDK_DIR)
TEE_GLOBAL_PLATFORM_OPTION += DRSDK_DIR=$(DRSDK_DIR)
TEE_GLOBAL_PLATFORM_OPTION += INTERNAL_DRSDK_DIR=$(INTERNAL_DRSDK_DIR)


TEE_DRIVER_OUTPUT_PATH := $(TRUSTZONE_OUTPUT_PATH)/driver
TEE_TRUSTLET_OUTPUT_PATH := $(TRUSTZONE_OUTPUT_PATH)/trustlet
TEE_TLC_OUTPUT_PATH := $(TRUSTZONE_OUTPUT_PATH)/tlc
TEE_APP_INSTALL_PATH := $(TARGET_OUT_VENDOR_APPS)/mcRegistry
# TEE only need to support and share 32-bit currently
ifeq ($(strip $(TARGET_ARCH)),arm64)
TEE_HAL_OUTPUT_PATH := $($(TARGET_2ND_ARCH_VAR_PREFIX)TARGET_OUT_INTERMEDIATES)/STATIC_LIBRARIES
else
TEE_HAL_OUTPUT_PATH := $(TARGET_OUT_INTERMEDIATES)/STATIC_LIBRARIES
endif
ANDROID_STATIC_LIBRARIES_OUT_DIR := $(abspath $(TEE_HAL_OUTPUT_PATH))
TEE_ADDITIONAL_DEPENDENCIES := $(abspath $(TRUSTZONE_PROJECT_MAKEFILE) $(TRUSTZONE_CUSTOM_BUILD_PATH)/common_config.mk $(TRUSTZONE_CUSTOM_BUILD_PATH)/tee_config.mk $(MTK_PATH_SOURCE)/trustzone/trustonic/source/build/platform/$(ARCH_MTK_PLATFORM)/tee_config.mk)

TEE_GLOBAL_PLATFORM_OPTION += ANDROID_STATIC_LIBRARIES_OUT_DIR=$(ANDROID_STATIC_LIBRARIES_OUT_DIR)
TEE_GLOBAL_PLATFORM_OPTION += TEE_ADDITIONAL_DEPENDENCIES=$(TEE_ADDITIONAL_DEPENDENCIES)


TEE_CLEAR_VARS := $(TRUSTZONE_CUSTOM_BUILD_PATH)/tee_clear_vars.mk
TEE_BASE_RULES := $(TRUSTZONE_CUSTOM_BUILD_PATH)/tee_base_rules.mk
TEE_SRC_MODULES := $(TEE_ALL_MODULES)
TEE_ALL_MODULES :=
TEE_modules_to_install :=
TEE_modules_to_check :=
ifeq ($(strip $(SHOW_COMMANDS)),)
  TEE_GLOBAL_MAKE_OPTION += --silent
endif
ifneq ($(TRUSTZONE_ROOT_DIR),)
  TEE_GLOBAL_MAKE_OPTION += ROOTDIR=$(TRUSTZONE_ROOT_DIR)
endif
TEE_GLOBAL_MAKE_OPTION += MTK_PROJECT=$(MTK_PROJECT)
TEE_DUMP_MAKEFILE_ONLY := true
$(foreach p,$(sort $(TEE_ALL_MODULE_MAKEFILE)),\
	$(eval include $(TEE_CLEAR_VARS))\
	$(eval LOCAL_MAKEFILE := $(p))\
	$(info including $(LOCAL_MAKEFILE) ...)\
	$(eval include $(LOCAL_MAKEFILE))\
	$(foreach n,$(TEE_BUILD_MODE),\
		$(eval TEE_MODE := $(n))\
		$(eval include $(TEE_BASE_RULES))\
	)\
)
TEE_DUMP_MAKEFILE_ONLY :=
# library and include path dependency between modules
ifeq ($(strip $(MTK_TEE_GP_SUPPORT)), yes)
  TEE_HEADER_REQUIRED_MODULES := drutils.drbin
else
  TEE_HEADER_REQUIRED_MODULES := drutils.lib
endif
$(foreach m,$(sort $(TEE_ALL_MODULES) $(TEE_SRC_MODULES)),\
	$(foreach n,$(TEE_BUILD_MODE),\
		$(foreach r,$(TEE_ALL_MODULES.$(m).$(n).REQUIRED),\
			$(eval $(TEE_ALL_MODULES.$(m).$(n).BUILT): $(TEE_ALL_MODULES.$(r).$(n).BUILT))\
		)\
		$(foreach r,$(TEE_HEADER_REQUIRED_MODULES) $(TEE_ALL_MODULES.$(m).$(n).REQUIRED),\
			$(if $(TEE_ALL_MODULES.$(r).PATH),\
				$(eval s := $(call UpperCase,$(basename $(r))))\
				$(eval $(TEE_ALL_MODULES.$(m).$(n).BUILT): PRIVATE_MAKE_OPTION += COMP_PATH_$(basename $(r))=$(TRUSTZONE_ROOT_DIR)/$(TEE_ALL_MODULES.$(r).PATH))\
				$(eval $(TEE_ALL_MODULES.$(m).$(n).BUILT): PRIVATE_MAKE_OPTION += $(s)_DIR=$(TRUSTZONE_ROOT_DIR)/$(TEE_ALL_MODULES.$(r).PATH))\
				$(eval $(TEE_ALL_MODULES.$(m).$(n).BUILT): PRIVATE_MAKE_OPTION += $(s)_OUT_DIR=$(TRUSTZONE_ROOT_DIR)/$(TEE_ALL_MODULES.$(r).OUTPUT_ROOT))\
			)\
		)\
	)\
)
ifeq ($(PLATFORM_VERSION),R)
else
# copy headers
$(foreach m,$(sort $(TEE_ALL_MODULES)),\
	$(eval TEE_public_header_path := $(TEE_ALL_MODULES.$(m).PATH)/Locals/Code/public)\
	$(if $(wildcard $(TEE_public_header_path)),\
		$(eval TEE_copy_headers := $(patsubst ./%,%,$(shell cd $(TEE_public_header_path); find . -name "*.h" -and -not -name ".*")))\
		$(if $(strip $(TEE_copy_headers)),\
			$(eval include $(CLEAR_VARS))\
			$(eval LOCAL_PATH := $(TEE_public_header_path))\
			$(eval LOCAL_COPY_HEADERS := $(TEE_copy_headers))\
			$(eval LOCAL_COPY_HEADERS_TO := $(m)/Locals/Code/public)\
			$(eval LOCAL_USE_VNDK := true)\
			$(eval include $(BUILD_COPY_HEADERS))\
		)\
	)\
)
endif


$(PRODUCT_OUT)/recovery.img: $(TEE_modules_to_install)
trustzone: $(TEE_modules_to_install) $(TEE_modules_to_check)
ALL_DEFAULT_INSTALLED_MODULES += $(TEE_modules_to_install) $(TEE_modules_to_check)

endif
endif
endif

include $(TRUSTZONE_CUSTOM_BUILD_PATH)/common_config.mk

MICROTRUST_TEE_VERSION ?= 300
KEYMASTER_VERSION ?= 4.0

$(info "build\microtrust_config.mk:MICROTRUST_TEE_SUPPORT=$(MICROTRUST_TEE_SUPPORT)")
$(info "build\microtrust_config.mk:MICROTRUST_TEE_VERSION=$(MICROTRUST_TEE_VERSION)")
$(info "build\microtrust_config.mk:MICROTRUST_TEE_LITE_SUPPORT=$(MICROTRUST_TEE_LITE_SUPPORT)")
ATF_GLOBAL_MAKE_OPTION += MICROTRUST_TEE_VERSION=$(MICROTRUST_TEE_VERSION)

ifeq ($(TARGET_BUILD_VARIANT),eng)
  MICROTRUST_INSTALL_MODE ?= Debug
else
  MICROTRUST_INSTALL_MODE ?= Release
endif
ifeq ($(MICROTRUST_INSTALL_MODE),Debug)
  MICROTRUST_INSTALL_MODE_LC := debug
else
  MICROTRUST_INSTALL_MODE_LC := release
endif

MICROTRUST_UT_SDK_HOME := vendor/mediatek/proprietary/trustzone/microtrust/ut_sdk/$(MICROTRUST_TEE_VERSION)

UT_SDK_HOME := $(abspath $(MICROTRUST_UT_SDK_HOME))
UT_SDK_DIR := $(MICROTRUST_UT_SDK_HOME)
$(info Microtrust SDK PATH $(MICROTRUST_UT_SDK_HOME) $(UT_SDK_DIR))

ifneq ($(wildcard $(UT_SDK_HOME)),)

ifeq ($(strip $(MICROTRUST_TEE_SUPPORT)), yes)

MICROTRUST_PRIVATE_TRUSTLET_PATH := $(MTK_PATH_SOURCE)/trustzone/trustonic/private/trustlets
MICROTRUST_PROTECT_TRUSTLET_PATH := $(MTK_PATH_SOURCE)/trustzone/microtrust/secure/trustlets
MICROTRUST_SOURCE_TRUSTLET_PATH := $(MTK_PATH_SOURCE)/trustzone/microtrust/source/trustlets
MICROTRUST_ALL_MODULE_MAKEFILE :=
# $(1): path
# $(2): common or platform
# $(3): sub-path
define mtk_microtrust_find_module_makefile
$(firstword \
  $(wildcard \
    $(MICROTRUST_PRIVATE_TRUSTLET_PATH)/$(strip $(1))/$(if $(filter platform,$(2)),platform/$(ARCH_MTK_PLATFORM),$(strip $(2)))/$(strip $(3))/Makefile \
    $(MICROTRUST_PROTECT_TRUSTLET_PATH)/$(strip $(1))/$(if $(filter platform,$(2)),platform/$(ARCH_MTK_PLATFORM),$(strip $(2)))/$(strip $(3))/Makefile \
    $(MICROTRUST_SOURCE_TRUSTLET_PATH)/$(strip $(1))/$(if $(filter platform,$(2)),platform/$(ARCH_MTK_PLATFORM),$(strip $(2)))/$(strip $(3))/Makefile \
  )\
)
endef
include $(MTK_PATH_SOURCE)/trustzone/microtrust/source/build/platform/$(ARCH_MTK_PLATFORM)/microtrust_config.mk

MICROTRUST_OUTPUT_PATH ?= $(TARGET_OUT_INTERMEDIATES)/MICROTRUST_OBJ
MICROTRUST_INSTALL_PATH := $(TARGET_OUT_VENDOR)/thh/ta
ifeq ($(strip $(TARGET_ARCH)),arm64)
MICROTRUST_HAL_OUTPUT_PATH := $($(TARGET_2ND_ARCH_VAR_PREFIX)TARGET_OUT_INTERMEDIATES)/STATIC_LIBRARIES
else
MICROTRUST_HAL_OUTPUT_PATH := $(TARGET_OUT_INTERMEDIATES)/STATIC_LIBRARIES
endif
ANDROID_STATIC_LIBRARIES_OUT_DIR := $(abspath $(MICROTRUST_HAL_OUTPUT_PATH))
MICROTRUST_ADDITIONAL_DEPENDENCIES := $(abspath $(TRUSTZONE_PROJECT_MAKEFILE) $(TRUSTZONE_CUSTOM_BUILD_PATH)/common_config.mk $(TRUSTZONE_CUSTOM_BUILD_PATH)/microtrust_config.mk)

### Replace of expert
MICROTRUST_GLOBAL_PLATFORM_OPTION :=
MICROTRUST_GLOBAL_PLATFORM_OPTION += UT_SDK_HOME=$(UT_SDK_HOME)
MICROTRUST_GLOBAL_PLATFORM_OPTION += MICROTRUST_ADDITIONAL_DEPENDENCIES=$(MICROTRUST_ADDITIONAL_DEPENDENCIES)
MICROTRUST_GLOBAL_PLATFORM_OPTION += ANDROID_STATIC_LIBRARIES_OUT_DIR=$(ANDROID_STATIC_LIBRARIES_OUT_DIR)
MICROTRUST_GLOBAL_PLATFORM_OPTION += ARCH_MTK_PLATFORM=$(ARCH_MTK_PLATFORM)

MICROTRUST_CLEAR_VARS := $(TRUSTZONE_CUSTOM_BUILD_PATH)/microtrust_clear_vars.mk
MICROTRUST_BASE_RULES := $(TRUSTZONE_CUSTOM_BUILD_PATH)/microtrust_base_rules.mk
MICROTRUST_LIB_MODULES := $(MICROTRUST_ALL_MODULES)
MICROTRUST_ALL_MODULES :=
MICROTRUST_modules_to_install :=
MICROTRUST_modules_to_check :=
ifeq ($(strip $(SHOW_COMMANDS)),)
  MICROTRUST_GLOBAL_MAKE_OPTION += --silent
endif
MICROTRUST_GLOBAL_MAKE_OPTION += --no-print-directory
ifneq ($(TRUSTZONE_ROOT_DIR),)
  MICROTRUST_GLOBAL_MAKE_OPTION += ROOTDIR=$(TRUSTZONE_ROOT_DIR)
endif
MICROTRUST_GLOBAL_MAKE_OPTION += MTK_PROJECT=$(MTK_PROJECT)
TEE_DUMP_MAKEFILE_ONLY := true
$(foreach p,$(sort $(MICROTRUST_ALL_MODULE_MAKEFILE)),\
	$(eval include $(MICROTRUST_CLEAR_VARS))\
	$(eval LOCAL_MAKEFILE := $(p))\
	$(info including $(LOCAL_MAKEFILE) ...)\
	$(eval include $(LOCAL_MAKEFILE))\
	$(eval include $(MICROTRUST_BASE_RULES))\
)
TEE_DUMP_MAKEFILE_ONLY :=
$(foreach m,$(sort $(MICROTRUST_ALL_MODULES)),\
	$(foreach r,$(filter-out $(MICROTRUST_ALL_MODULES),$(MICROTRUST_ALL_MODULES.$(m).REQUIRED)),\
		$(info Ignore $(m).REQUIRED = $(r))\
	)\
	$(eval l := $(foreach r,$(filter $(MICROTRUST_ALL_MODULES),$(MICROTRUST_ALL_MODULES.$(m).REQUIRED)),$(abspath $(MICROTRUST_ALL_MODULES.$(r).OUTPUT_ROOT))))\
	$(if $(strip $(l)),\
		$(eval $(MICROTRUST_ALL_MODULES.$(m).BUILT): PRIVATE_MAKE_OPTION += EXTERNAL_LIB_DIR="$(l)")\
	)\
	$(foreach r,$(filter $(MICROTRUST_ALL_MODULES),$(MICROTRUST_ALL_MODULES.$(m).REQUIRED)),\
		$(eval $(MICROTRUST_ALL_MODULES.$(m).BUILT): $(MICROTRUST_ALL_MODULES.$(r).BUILT))\
	)\
)
ifeq ($(PLATFORM_VERSION),R)
else
# copy headers
$(foreach m,$(filter-out $(TEE_ALL_MODULES) $(TEE_LIB_MODULES),tlutils.tlbin $(TEE_HEADER_REQUIRED_MODULES) msee_fwk_drv.lib msee_fwk_ta.lib),\
	$(if $(TEE_ALL_MODULES.$(m).PATH),,\
		$(eval TEE_ALL_MODULES.$(m).PATH := $(TARGET_OUT_HEADERS)/$(m))\
	)\
)
endif

MICROTRUST_TUI_SUPPORT ?= no
$(info MICROTRUST_TUI_SUPPORT=$(MICROTRUST_TUI_SUPPORT))
ATF_GLOBAL_MAKE_OPTION += \
	MICROTRUST_TUI_SUPPORT=$(MICROTRUST_TUI_SUPPORT)

endif # ifeq ($(strip $(MICROTRUST_TEE_SUPPORT)), yes)

else

$(info MICROTRUST SDK is removed for DrvFwk)

endif # ifneq ($(wildcard $(UT_SDK_HOME)),)

TEE_modules_to_install := $(TEE_modules_to_install) $(MICROTRUST_modules_to_install)
TEE_modules_to_check := $(TEE_modules_to_check) $(MICROTRUST_modules_to_check)

$(shell mkdir -p $(TARGET_OUT_VENDOR)/thh/ta)
$(shell mkdir -p $(TARGET_OUT_VENDOR)/bin)
$(shell mkdir -p $(TARGET_OUT_VENDOR)/etc/init)
$(info "atf compile copy ta MICROTRUST_TEE_VERSION=$(MICROTRUST_TEE_VERSION)")

ifeq ($(strip $(MICROTRUST_TEE_LITE_SUPPORT)), yes)

MICROTRUST_ORI_IMAGE_NAME := $(MTK_PATH_SOURCE)/trustzone/microtrust/source/common/lite/teei/teei.raw
$(shell cp -f vendor/mediatek/proprietary/trustzone/microtrust/source/common/$(MICROTRUST_TEE_VERSION)/teei/isee_product.cfg $(TARGET_OUT_VENDOR)/thh/ta/isee_product.cfg)
$(shell cp -f vendor/mediatek/proprietary/trustzone/microtrust/source/common/$(MICROTRUST_TEE_VERSION)/teei/TAs_list $(TARGET_OUT_VENDOR)/thh/ta/TAs_list)
$(shell cp -f vendor/mediatek/proprietary/trustzone/microtrust/source/common/$(MICROTRUST_TEE_VERSION)/teei_loader/teei_loader $(TARGET_OUT_VENDOR)/bin/teei_loader)
$(shell cp -f vendor/mediatek/proprietary/trustzone/microtrust/source/common/$(MICROTRUST_TEE_VERSION)/teei_loader/microtrust.rc $(TARGET_OUT_VENDOR)/etc/init/microtrust.rc)

else

MICROTRUST_ORI_IMAGE_NAME := $(MTK_PATH_SOURCE)/trustzone/microtrust/source/common/$(MICROTRUST_TEE_VERSION)/teei/teei.raw
ifeq ($(strip $(KEYMASTER_VERSION)), 4.0)
$(shell cp -f $(MTK_PATH_SOURCE)/trustzone/microtrust/source/common/$(MICROTRUST_TEE_VERSION)/teei/keymaster_server4 $(TARGET_OUT_VENDOR)/thh/ta/c09c9c5daa504b78b0e46eda61556c3a.ta)
else
$(shell cp -f $(MTK_PATH_SOURCE)/trustzone/microtrust/source/common/$(MICROTRUST_TEE_VERSION)/teei/keymaster_server3 $(TARGET_OUT_VENDOR)/thh/ta/c09c9c5daa504b78b0e46eda61556c3a.ta)
endif
$(shell cp -f $(MTK_PATH_SOURCE)/trustzone/microtrust/source/common/$(MICROTRUST_TEE_VERSION)/teei/gatekeeper_server $(TARGET_OUT_VENDOR)/thh/ta/c1882f2d885e4e13a8c8e2622461b2fa.ta)
$(shell cp -f $(MTK_PATH_SOURCE)/trustzone/microtrust/source/common/$(MICROTRUST_TEE_VERSION)/teei/km_key_manager_ta $(TARGET_OUT_VENDOR)/thh/ta/d91f322ad5a441d5955110eda3272fc0.ta)
endif #MICROTRUST_TEE_LITE_SUPPORT

MICROTRUST_RAW_IMAGE_NAME := $(TRUSTZONE_IMAGE_OUTPUT_PATH)/bin/$(ARCH_MTK_PLATFORM)_microtrust_$(MICROTRUST_INSTALL_MODE_LC)_raw.img
MICROTRUST_TEMP_PADDING_FILE := $(TRUSTZONE_IMAGE_OUTPUT_PATH)/bin/$(ARCH_MTK_PLATFORM)_microtrust_$(MICROTRUST_INSTALL_MODE_LC)_pad.txt
MICROTRUST_TEMP_CFG_FILE := $(TRUSTZONE_IMAGE_OUTPUT_PATH)/bin/img_hdr_microtrust.cfg
MICROTRUST_SIGNED_IMAGE_NAME := $(TRUSTZONE_IMAGE_OUTPUT_PATH)/bin/$(ARCH_MTK_PLATFORM)_microtrust_$(MICROTRUST_INSTALL_MODE_LC)_signed.img
MICROTRUST_PADDING_IMAGE_NAME := $(TRUSTZONE_IMAGE_OUTPUT_PATH)/bin/$(ARCH_MTK_PLATFORM)_microtrust_$(MICROTRUST_INSTALL_MODE_LC)_pad.img
MICROTRUST_COMP_IMAGE_NAME := $(TRUSTZONE_IMAGE_OUTPUT_PATH)/bin/$(ARCH_MTK_PLATFORM)_microtrust.img

$(MICROTRUST_RAW_IMAGE_NAME): $(MICROTRUST_ORI_IMAGE_NAME) $(MICROTRUST_ADDITIONAL_DEPENDENCIES)
	@echo Microtrust build: $@
	$(hide) mkdir -p $(dir $@)
	$(hide) cp -f $(MICROTRUST_ORI_IMAGE_NAME) $@

$(MICROTRUST_TEMP_PADDING_FILE): ALIGNMENT=512
$(MICROTRUST_TEMP_PADDING_FILE): MKIMAGE_HDR_SIZE=512
$(MICROTRUST_TEMP_PADDING_FILE): RSA_SIGN_HDR_SIZE=576
$(MICROTRUST_TEMP_PADDING_FILE): $(MICROTRUST_RAW_IMAGE_NAME) $(MICROTRUST_ADDITIONAL_DEPENDENCIES)
	@echo Microtrust build: $@
	$(hide) mkdir -p $(dir $@)
	$(hide) rm -f $@
	$(hide) FILE_SIZE=$$(($$(wc -c < "$(MICROTRUST_RAW_IMAGE_NAME)")+$(MKIMAGE_HDR_SIZE)+$(RSA_SIGN_HDR_SIZE)));\
	REMAINDER=$$(($${FILE_SIZE} % $(ALIGNMENT)));\
	if [ $${REMAINDER} -ne 0 ]; then dd if=/dev/zero of=$@ bs=$$(($(ALIGNMENT)-$${REMAINDER})) count=1; else touch $@; fi

$(MICROTRUST_TEMP_CFG_FILE): $(TEE_DRAM_SIZE_CFG) $(MICROTRUST_ADDITIONAL_DEPENDENCIES)
	@echo Microtrust build: $@
	$(hide) mkdir -p $(dir $@)
	$(hide) rm -f $@
	@echo "LOAD_MODE = 0" > $@
	@echo "NAME = tee" >> $@
	@echo "LOAD_ADDR =" $(TEE_TOTAL_DRAM_SIZE) >> $@

$(MICROTRUST_PADDING_IMAGE_NAME): $(MICROTRUST_RAW_IMAGE_NAME) $(MICROTRUST_TEMP_PADDING_FILE) $(MICROTRUST_ADDITIONAL_DEPENDENCIES)
	@echo Microtrust build: $@
	$(hide) mkdir -p $(dir $@)
	$(hide) cat $(MICROTRUST_RAW_IMAGE_NAME) $(MICROTRUST_TEMP_PADDING_FILE) > $@

$(MICROTRUST_SIGNED_IMAGE_NAME): ALIGNMENT=512
$(MICROTRUST_SIGNED_IMAGE_NAME): $(MICROTRUST_PADDING_IMAGE_NAME) $(TRUSTZONE_SIGN_TOOL) $(TRUSTZONE_IMG_PROTECT_CFG) $(TEE_DRAM_SIZE_CFG) $(MICROTRUST_ADDITIONAL_DEPENDENCIES)
	@echo Microtrust build: $@
	$(hide) mkdir -p $(dir $@)
	$(hide) $(TRUSTZONE_SIGN_TOOL) $(TRUSTZONE_IMG_PROTECT_CFG) $(MICROTRUST_PADDING_IMAGE_NAME) $@ $(TEE_DRAM_SIZE)
	$(hide) FILE_SIZE=$$(wc -c < "$(MICROTRUST_SIGNED_IMAGE_NAME)");REMAINDER=$$(($${FILE_SIZE} % $(ALIGNMENT)));\
	if [ $${REMAINDER} -ne 0 ]; then echo "[ERROR] File $@ size $${FILE_SIZE} is not $(ALIGNMENT) bytes aligned";exit 1; fi

$(MICROTRUST_COMP_IMAGE_NAME): ALIGNMENT=512
$(MICROTRUST_COMP_IMAGE_NAME): $(MICROTRUST_SIGNED_IMAGE_NAME) $(MTK_MKIMAGE_TOOL) $(MICROTRUST_TEMP_CFG_FILE)  $(MICROTRUST_ADDITIONAL_DEPENDENCIES)
	@echo Microtrust build: $@
	$(hide) mkdir -p $(dir $@)
	$(hide) $(MTK_MKIMAGE_TOOL) $(MICROTRUST_SIGNED_IMAGE_NAME) $(MICROTRUST_TEMP_CFG_FILE) > $@
	$(hide) FILE_SIZE=$$(wc -c < "$(MICROTRUST_COMP_IMAGE_NAME)");REMAINDER=$$(($${FILE_SIZE} % $(ALIGNMENT)));\
	if [ $${REMAINDER} -ne 0 ]; then echo "[ERROR] File $@ size $${FILE_SIZE} is not $(ALIGNMENT) bytes aligned";exit 1; fi


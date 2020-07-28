# vendor/mediatek/proprietary/trustzone/Android.mk
SHELL=/bin/bash

hide := @
#===========================
# 1. Execute Andoroid build ATF once to get options value
#    $mosesq make -j24 trustzone 2>&1 | tee atf.log
# 2. find options in atf.log
#    $vim atf.log and search(/) "ATF_Copy_Start" as keyword
# 3. Copy and paste the options to Makefile
# 4. build ATF
#    $make all
#===========================

#=========== ATF_Copy_Start ===========
MTK_EMMC_SUPPORT := yes
MTK_ACAO_SUPPORT := yes
TARGET_BUILD_VARIANT := eng
MTK_PLATFORM := MT6771
MTK_BASE_PROJECT := k71v1_64_bsp
MTK_TARGET_PROJECT := k71v1_64_bsp_tee
MTK_ATF_SUPPORT := yes
MTK_TEE_SUPPORT := yes
MTK_TEE_GP_SUPPORT := yes
TRUSTONIC_TEE_SUPPORT := yes
WATCHDATA_TEE_SUPPORT := no
MICROTRUST_TEE_SUPPORT := no
MICROTRUST_TEE_LITE_SUPPORT := no
TRUSTKERNEL_TEE_SUPPORT := no
MTK_GOOGLE_TRUSTY_SUPPORT := no
MTK_IN_HOUSE_TEE_SUPPORT := no
MTK_SOTER_SUPPORT := no
MTK_SEC_VIDEO_PATH_SUPPORT := yes
MTK_WFD_HDCP_TX_SUPPORT := no
MTK_CAM_SECURITY_SUPPORT := no
RELEASE_BRM :=
#============= ATF_Copy_End ===========

LOCAL_PATH := .
PRODUCT_OUT := $(LOCAL_PATH)/out/${MTK_TARGET_PROJECT}
TRUSTZONE_ROOT_DIR := $(PWD)
TRUSTZONE_CUSTOM_BUILD_PATH := $(LOCAL_PATH)
TRUSTZONE_OUTPUT_PATH := $(PRODUCT_OUT)/trustzone
TRUSTZONE_IMAGE_OUTPUT_PATH := $(TRUSTZONE_OUTPUT_PATH)

# used in TEE projects
MTK_PATH_SOURCE :=  ../../../../../../vendor/mediatek/proprietary/

# define streq(), ATF standalone building system do not define it yet
define streq
$(strip $(if $(strip $(1)),\
  $(if $(strip $(2)),\
    $(if $(filter-out __,_$(subst $(strip $(1)),,$(strip $(2)))$(subst $(strip $(2)),,$(strip $(1)))_),,true), \
    ),\
  $(if $(strip $(2)),\
    ,\
    true)\
 ))
endef

#get MTK_ATF_VERSION through common_config.mk, then setting ATF_BUILD_PATH
include $(TRUSTZONE_CUSTOM_BUILD_PATH)/common_config.mk
ATF_BUILD_PATH := ../../../../../../vendor/mediatek/proprietary/trustzone/atf/$(MTK_ATF_VERSION)

$(info MTK_EMMC_SUPPORT := $(MTK_EMMC_SUPPORT))
$(info MTK_ACAO_SUPPORT := $(MTK_ACAO_SUPPORT))
$(info TARGET_BUILD_VARIANT := $(TARGET_BUILD_VARIANT))
$(info MTK_PLATFORM := $(MTK_PLATFORM))
$(info MTK_BASE_PROJECT := $(MTK_BASE_PROJECT))
$(info MTK_TARGET_PROJECT := $(MTK_TARGET_PROJECT))
$(info MTK_ATF_SUPPORT := $(MTK_ATF_SUPPORT))
$(info MTK_TEE_SUPPORT := $(MTK_TEE_SUPPORT))
$(info MTK_TEE_GP_SUPPORT := $(MTK_TEE_GP_SUPPORT))
$(info TRUSTONIC_TEE_SUPPORT := $(TRUSTONIC_TEE_SUPPORT))
$(info WATCHDATA_TEE_SUPPORT := $(WATCHDATA_TEE_SUPPORT))
$(info MICROTRUST_TEE_SUPPORT := $(MICROTRUST_TEE_SUPPORT))
$(info MICROTRUST_TEE_LITE_SUPPORT := $(MICROTRUST_TEE_LITE_SUPPORT))
$(info TRUSTKERNEL_TEE_SUPPORT := $(TRUSTKERNEL_TEE_SUPPORT))
$(info MTK_GOOGLE_TRUSTY_SUPPORT := $(MTK_GOOGLE_TRUSTY_SUPPORT))
$(info MTK_IN_HOUSE_TEE_SUPPORT := $(MTK_IN_HOUSE_TEE_SUPPORT))
$(info MTK_SOTER_SUPPORT := $(MTK_SOTER_SUPPORT))
$(info MTK_SEC_VIDEO_PATH_SUPPORT := $(MTK_SEC_VIDEO_PATH_SUPPORT))
$(info MTK_WFD_HDCP_TX_SUPPORT := $(MTK_WFD_HDCP_TX_SUPPORT))
$(info MTK_CAM_SECURITY_SUPPORT := $(MTK_CAM_SECURITY_SUPPORT))
$(info RELEASE_BRM := $(RELEASE_BRM))

ifneq ($(strip $(MTK_IN_HOUSE_TEE_SUPPORT)),yes)
ifneq (,$(strip $(wildcard $(MTK_PATH_SOURCE)/trustzone/trustonic/source/build/platform/$(strip $(call LowerCase,$(MTK_PLATFORM)))/tee_config.mk)))
ifneq (,$(strip $(wildcard $(MTK_PATH_SOURCE)/trustzone/trustonic/source/build/platform/$(strip $(call LowerCase,$(MTK_PLATFORM)))/brm_config.mk)))
  $(info RELEASE_BRM = $(RELEASE_BRM))
  ifeq ($(strip $(RELEASE_BRM)),yes)
    include $(MTK_PATH_SOURCE)/trustzone/trustonic/source/build/platform/$(strip $(call LowerCase,$(MTK_PLATFORM)))/brm_config.mk
    $(info MTK_TEE_SUPPORT = $(MTK_TEE_SUPPORT))
    $(info MTK_TEE_GP_SUPPORT = $(MTK_TEE_GP_SUPPORT))
    $(info TRUSTONIC_TEE_SUPPORT = $(TRUSTONIC_TEE_SUPPORT))
    $(info MICROTRUST_TEE_SUPPORT = $(MICROTRUST_TEE_SUPPORT))
    $(info MICROTRUST_TEE_LITE_SUPPORT = $(MICROTRUST_TEE_LITE_SUPPORT))
    $(info WATCHDATA_TEE_SUPPORT = $(WATCHDATA_TEE_SUPPORT))
    $(info TRUSTKERNEL_TEE_SUPPORT = $(TRUSTKERNEL_TEE_SUPPORT))
    $(info MTK_GOOGLE_TRUSTY_SUPPORT = $(MTK_GOOGLE_TRUSTY_SUPPORT))
    $(info MTK_IN_HOUSE_TEE_SUPPORT = $(MTK_IN_HOUSE_TEE_SUPPORT))
    $(info MTK_SOTER_SUPPORT = $(MTK_SOTER_SUPPORT))
    $(info MTK_SEC_VIDEO_PATH_SUPPORT = $(MTK_SEC_VIDEO_PATH_SUPPORT))
    $(info MTK_WFD_HDCP_TX_SUPPORT = $(MTK_WFD_HDCP_TX_SUPPORT))
    $(info MTK_CAM_SECURITY_SUPPORT = $(MTK_CAM_SECURITY_SUPPORT))
  endif
endif
endif
endif

ifeq ($(strip $(MTK_IN_HOUSE_TEE_SUPPORT)),yes)
  ifneq ($(wildcard vendor/mediatek/proprietary/trustzone/mtee/source/Android.mk),)
    # source release
    # include vendor/mediatek/proprietary/trustzone/mtee/protect/Android.mk
    INSTALLED_TRUSTZONE_TARGET := $(PRODUCT_OUT)/tee.img
    BUILT_TRUSTZONE_TARGET := $(TRUSTZONE_IMAGE_OUTPUT_PATH)/bin/tz.img
  else
    # binary release
    INSTALLED_TRUSTZONE_TARGET := $(PRODUCT_OUT)/tee.img
    PREBUILT_TRUSTZONE_TARGET := $(PRODUCT_OUT)/tee.img
  endif
else
  ifneq ($(filter yes,$(MTK_ATF_SUPPORT) $(TRUSTONIC_TEE_SUPPORT) $(WATCHDATA_TEE_SUPPORT) $(MICROTRUST_TEE_SUPPORT) $(TRUSTKERNEL_TEE_SUPPORT) $(MTK_GOOGLE_TRUSTY_SUPPORT)),)
    INSTALLED_TRUSTZONE_TARGET := $(PRODUCT_OUT)/tee.img
    BUILT_TRUSTZONE_TARGET := $(TRUSTZONE_IMAGE_OUTPUT_PATH)/bin/tee.img
  endif
endif

SIGN_TRUSTZONE_TARGET := $(addsuffix -verified$(suffix $(INSTALLED_TRUSTZONE_TARGET)),$(basename $(INSTALLED_TRUSTZONE_TARGET)))

.PHONY: trustzone clean
# ATF
  ifeq ($(strip $(MTK_ATF_SUPPORT)),yes)
include $(TRUSTZONE_CUSTOM_BUILD_PATH)/atf_config.mk
$(BUILT_TRUSTZONE_TARGET): $(ATF_COMP_IMAGE_NAME)
  endif
  # MTEE
  ifeq ($(strip $(MTK_IN_HOUSE_TEE_SUPPORT)),yes)
  ifeq ($(strip $(MTK_TEE_SUPPORT)),yes)
include $(TRUSTZONE_CUSTOM_BUILD_PATH)/mtee_config.mk
$(BUILT_TRUSTZONE_TARGET): $(MTEE_COMP_IMAGE_NAME)
  else
MTEE_INTERMEDIATES_IMAGE_NAME := $(PRODUCT_OUT)/obj/EXECUTABLES/tz.img_intermediates/tz.img
$(BUILT_TRUSTZONE_TARGET): $(MTEE_INTERMEDIATES_IMAGE_NAME)
  endif
  endif
  #ifeq ($(MTK_TEE_SUPPORT),yes)
  # Trustonic
  ifeq ($(strip $(TRUSTONIC_TEE_SUPPORT)),yes)
include $(TRUSTZONE_CUSTOM_BUILD_PATH)/tee_config.mk
$(BUILT_TRUSTZONE_TARGET): $(TEE_COMP_IMAGE_NAME)
#trustzone: mcDriverDaemon
$(PRODUCT_OUT)/recovery.img: \
	$(TEE_modules_to_install) \
	$(call intermediates-dir-for,EXECUTABLES,mcDriverDaemon)/mcDriverDaemon
  endif
  # Watchdata
  ifeq ($(strip $(WATCHDATA_TEE_SUPPORT)),yes)
include $(TRUSTZONE_CUSTOM_BUILD_PATH)/watchdata_config.mk
$(BUILT_TRUSTZONE_TARGET): $(WATCHDATA_COMP_IMAGE_NAME)
  endif
  # Microtrust
  ifeq ($(strip $(MICROTRUST_TEE_SUPPORT)),yes)
include $(TRUSTZONE_CUSTOM_BUILD_PATH)/microtrust_config.mk
$(BUILT_TRUSTZONE_TARGET): $(MICROTRUST_COMP_IMAGE_NAME)
  endif
  ifeq ($(strip $(MICROTRUST_TEE_LITE_SUPPORT)),yes)
include $(TRUSTZONE_CUSTOM_BUILD_PATH)/microtrust_config.mk
$(BUILT_TRUSTZONE_TARGET): $(MICROTRUST_COMP_IMAGE_NAME)
  endif
# TrustKernel
  ifeq ($(strip $(TRUSTKERNEL_TEE_SUPPORT)),yes)
include $(TRUSTZONE_CUSTOM_BUILD_PATH)/trustkernel_config.mk
$(BUILT_TRUSTZONE_TARGET): $(TEE_COMP_IMAGE_NAME)
  endif
  # Trusty
  ifeq ($(strip $(MTK_GOOGLE_TRUSTY_SUPPORT)),yes)
include $(TRUSTZONE_CUSTOM_BUILD_PATH)/trusty_config.mk
$(BUILT_TRUSTZONE_TARGET): $(TRUSTY_COMP_IMAGE_NAME)
  endif
  #endif
  # tee.img/trustzone.bin
$(BUILT_TRUSTZONE_TARGET):
	@echo "Trustzone build: $@ <= $^"
	$(hide) mkdir -p $(dir $@)
	$(hide) cat $^ > $@

$(INSTALLED_TRUSTZONE_TARGET): $(BUILT_TRUSTZONE_TARGET)
	@echo Copying: $@
	$(hide) mkdir -p $(dir $@)
	$(hide) cp -f $< $@

# Trustonic
ifeq ($(strip $(TRUSTONIC_TEE_SUPPORT)),yes)
ifneq ($(wildcard vendor/mediatek/proprietary/trustzone/trustonic/internal/bsp),)
include $(TRUSTZONE_CUSTOM_BUILD_PATH)/hal_config.mk
endif
endif

# Microtrust
ifeq ($(strip $(MICROTRUST_TEE_SUPPORT)),yes)
ifeq ($(strip $(MTK_TEE_GP_SUPPORT)),yes)
ifneq ($(wildcard $(UT_SDK_HOME)),)
include $(TRUSTZONE_CUSTOM_BUILD_PATH)/hal_config.mk
else
$(info MICROTRUST SDK is removed for HAL)
endif
endif
endif

ifneq ($(INSTALLED_TRUSTZONE_TARGET),)
$(TARGET_OUT_ETC)/tee.img: $(INSTALLED_TRUSTZONE_TARGET) $(LOCAL_PATH)/Android.mk
	@echo Copying: $@
	$(hide) mkdir -p $(dir $@)
	$(hide) cp -f $< $@

all: $(INSTALLED_TRUSTZONE_TARGET) $(TEE_modules_to_install) $(TEE_modules_to_check) $(TEE_HAL_modules_to_check) $(TEE_HAL_modules_to_install) $(SIGN_TRUSTZONE_TARGET)

ALL_DEFAULT_INSTALLED_MODULES += $(INSTALLED_TRUSTZONE_TARGET) $(TEE_modules_to_install) $(TEE_modules_to_check) $(TEE_HAL_modules_to_check) $(TEE_HAL_modules_to_install) $(TARGET_OUT_ETC)/tee.img $(SIGN_TRUSTZONE_TARGET)
$(SIGN_TRUSTZONE_TARGET):
endif

FORCE:
	@echo "in Force, do nothing"

clean:
	rm -rf out


# vendor/mediatek/proprietary/trustzone/Android.mk
UpperCase = $(subst a,A,$(subst b,B,$(subst c,C,$(subst d,D,$(subst e,E,$(subst f,F,$(subst g,G,$(subst h,H,$(subst i,I,$(subst j,J,$(subst k,K,$(subst l,L,$(subst m,M,$(subst n,N,$(subst o,O,$(subst p,P,$(subst q,Q,$(subst r,R,$(subst s,S,$(subst t,T,$(subst u,U,$(subst v,V,$(subst w,W,$(subst x,X,$(subst y,Y,$(subst z,Z,$(1)))))))))))))))))))))))))))
LowerCase = $(subst A,a,$(subst B,b,$(subst C,c,$(subst D,d,$(subst E,e,$(subst F,f,$(subst G,g,$(subst H,h,$(subst I,i,$(subst J,j,$(subst K,k,$(subst L,l,$(subst M,m,$(subst N,n,$(subst O,o,$(subst P,p,$(subst Q,q,$(subst R,r,$(subst S,s,$(subst T,t,$(subst U,u,$(subst V,v,$(subst W,w,$(subst X,x,$(subst Y,y,$(subst Z,z,$(1)))))))))))))))))))))))))))

LOCAL_PATH := $(call my-dir)
TRUSTZONE_ROOT_DIR := $(PWD)
TRUSTZONE_CUSTOM_BUILD_PATH := $(LOCAL_PATH)
TRUSTZONE_OUTPUT_PATH := $(PRODUCT_OUT)/trustzone
TRUSTZONE_IMAGE_OUTPUT_PATH := $(TRUSTZONE_OUTPUT_PATH)

ifdef MTK_TARGET_PROJECT
$(info #=========== ATF_Copy_Start ===========)
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
$(info #============= ATF_Copy_End ===========)
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
  ifneq ($(wildcard vendor/mediatek/proprietary/trustzone/mtee/release/Android.mk),)
  ifeq ($(MTK_IN_HOUSE_TEE_DEV),yes)
    # licensed source release
    INSTALLED_TRUSTZONE_TARGET := $(PRODUCT_OUT)/tee.img
    BUILT_TRUSTZONE_TARGET := $(TRUSTZONE_IMAGE_OUTPUT_PATH)/bin/tz.img
  endif
  endif
  endif
  ifeq ($(BUILT_TRUSTZONE_TARGET),)
    # binary release
    INSTALLED_TRUSTZONE_TARGET := $(PRODUCT_OUT)/tee.img
    PREBUILT_TRUSTZONE_TARGET := $(PRODUCT_OUT)/tee.img
  endif
else
  ifneq ($(filter yes,$(MTK_ATF_SUPPORT) $(TRUSTONIC_TEE_SUPPORT) $(WATCHDATA_TEE_SUPPORT) $(MICROTRUST_TEE_SUPPORT) $(MICROTRUST_TEE_LITE_SUPPORT) $(TRUSTKERNEL_TEE_SUPPORT) $(MTK_GOOGLE_TRUSTY_SUPPORT)),)
    INSTALLED_TRUSTZONE_TARGET := $(PRODUCT_OUT)/tee.img
    BUILT_TRUSTZONE_TARGET := $(TRUSTZONE_IMAGE_OUTPUT_PATH)/bin/tee.img
  endif
endif

SIGN_TRUSTZONE_TARGET := $(addsuffix -verified$(suffix $(INSTALLED_TRUSTZONE_TARGET)),$(basename $(INSTALLED_TRUSTZONE_TARGET)))

.PHONY: trustzone
ifneq ($(PREBUILT_TRUSTZONE_TARGET),)

else ifneq ($(BUILT_TRUSTZONE_TARGET),)

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

$(INSTALLED_TRUSTZONE_TARGET): $(BUILT_TRUSTZONE_TARGET) $(LOCAL_PATH)/Android.mk
	@echo Copying: $@
	$(hide) mkdir -p $(dir $@)
	$(hide) cp -f $< $@

endif

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

trustzone: $(INSTALLED_TRUSTZONE_TARGET) $(TEE_modules_to_install) $(TEE_modules_to_check) $(TEE_HAL_modules_to_check) $(TEE_HAL_modules_to_install) $(SIGN_TRUSTZONE_TARGET)
ALL_DEFAULT_INSTALLED_MODULES += $(TEE_modules_to_install) $(TEE_modules_to_check) $(TEE_HAL_modules_to_check) $(TEE_HAL_modules_to_install)
droidcore: $(INSTALLED_TRUSTZONE_TARGET) $(SIGN_TRUSTZONE_TARGET)
$(SIGN_TRUSTZONE_TARGET):

endif
endif#MTK_TARGET_PROJECT

.PHONY: MODULES-IN-vendor-mediatek-proprietary-trustzone-custom-build
MODULES-IN-vendor-mediatek-proprietary-trustzone-custom-build: trustzone

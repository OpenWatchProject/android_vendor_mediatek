UpperCase = $(subst a,A,$(subst b,B,$(subst c,C,$(subst d,D,$(subst e,E,$(subst f,F,$(subst g,G,$(subst h,H,$(subst i,I,$(subst j,J,$(subst k,K,$(subst l,L,$(subst m,M,$(subst n,N,$(subst o,O,$(subst p,P,$(subst q,Q,$(subst r,R,$(subst s,S,$(subst t,T,$(subst u,U,$(subst v,V,$(subst w,W,$(subst x,X,$(subst y,Y,$(subst z,Z,$(1)))))))))))))))))))))))))))
LowerCase = $(subst A,a,$(subst B,b,$(subst C,c,$(subst D,d,$(subst E,e,$(subst F,f,$(subst G,g,$(subst H,h,$(subst I,i,$(subst J,j,$(subst K,k,$(subst L,l,$(subst M,m,$(subst N,n,$(subst O,o,$(subst P,p,$(subst Q,q,$(subst R,r,$(subst S,s,$(subst T,t,$(subst U,u,$(subst V,v,$(subst W,w,$(subst X,x,$(subst Y,y,$(subst Z,z,$(1)))))))))))))))))))))))))))

# stash-product-vars
$(foreach v,MTK_ATF_SUPPORT MTK_TEE_SUPPORT MTK_IN_HOUSE_TEE_SUPPORT TRUSTONIC_TEE_SUPPORT MICROTRUST_TEE_SUPPORT MICROTRUST_TEE_LITE_SUPPORT MTK_GOOGLE_TRUSTY_SUPPORT WATCHDATA_TEE_SUPPORT TRUSTKERNEL_TEE_SUPPORT,\
  $(eval __STASHED_$(v) := $($(v)))\
)

TRUSTZONE_PROJECT_MAKEFILE := $(wildcard $(addprefix $(TRUSTZONE_CUSTOM_BUILD_PATH)/project/,common.mk $(call LowerCase,$(MTK_PLATFORM)).mk $(MTK_BASE_PROJECT).mk $(MTK_TARGET_PROJECT).mk))
TRUSTZONE_PROJECT_MAKEFILE_FULL_PATH := $(addprefix $(PWD)/,$(TRUSTZONE_PROJECT_MAKEFILE))

$(info include $(TRUSTZONE_PROJECT_MAKEFILE))
include $(TRUSTZONE_PROJECT_MAKEFILE)

ifneq ($(strip $(MTK_IN_HOUSE_TEE_SUPPORT)),yes)
ifneq (,$(strip $(wildcard $(MTK_PATH_SOURCE)/trustzone/trustonic/source/build/platform/$(strip $(call LowerCase,$(MTK_PLATFORM)))/tee_config.mk)))
ifneq (,$(strip $(wildcard $(MTK_PATH_SOURCE)/trustzone/trustonic/source/build/platform/$(strip $(call LowerCase,$(MTK_PLATFORM)))/brm_config.mk)))
  $(info RELEASE_BRM = $(RELEASE_BRM))
  ifeq ($(strip $(RELEASE_BRM)),yes)
    include $(MTK_PATH_SOURCE)/trustzone/trustonic/source/build/platform/$(strip $(call LowerCase,$(MTK_PLATFORM)))/brm_config.mk
    $(info MTK_TEE_SUPPORT = $(MTK_TEE_SUPPORT))
    $(info MTK_TEE_GP_SUPPORT = $(MTK_TEE_GP_SUPPORT))
    $(info TRUSTONIC_TEE_SUPPORT = $(TRUSTONIC_TEE_SUPPORT))
    $(info WATCHDATA_TEE_SUPPORT = $(WATCHDATA_TEE_SUPPORT))
    $(info MICROTRUST_TEE_SUPPORT = $(MICROTRUST_TEE_SUPPORT))
    $(info MICROTRUST_TEE_LITE_SUPPORT = $(MICROTRUST_TEE_LITE_SUPPORT))
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

# assert-product-vars
changed_variables :=
$(foreach v,MTK_ATF_SUPPORT MTK_TEE_SUPPORT MTK_IN_HOUSE_TEE_SUPPORT TRUSTONIC_TEE_SUPPORT MICROTRUST_TEE_SUPPORT MICROTRUST_TEE_LITE_SUPPORT MTK_GOOGLE_TRUSTY_SUPPORT WATCHDATA_TEE_SUPPORT TRUSTKERNEL_TEE_SUPPORT,\
  $(if $(call streq,$($(v)),$(__STASHED_$(v))),,\
    $(eval $(warning $(v) has been modified: $($(v))))\
    $(eval $(warning previous value: $(__STASHED_$(v))))\
    $(eval changed_variables := $(changed_variables) $(v)))\
)
ifneq ($(strip $(changed_variables)),)
  $(error The following variables have been changed: $(changed_variables))
endif


##
# vendor/mediatek/proprietary/trustzone/build.sh
##

### MTK_FILE_PATH ###
MTK_BUILD_TOOL_PATH := device/mediatek/build/build/tools
ifeq ($(HOST_OS),darwin)
MTK_MKIMAGE_TOOL := $(TRUSTZONE_CUSTOM_BUILD_PATH)/tools/mkimage.$(HOST_OS)
TRUSTZONE_SIGN_TOOL := $(TRUSTZONE_CUSTOM_BUILD_PATH)/tools/TeeImgSignEncTool.$(HOST_OS)
else
MTK_MKIMAGE_TOOL := $(TRUSTZONE_CUSTOM_BUILD_PATH)/tools/mkimage
TRUSTZONE_SIGN_TOOL := $(TRUSTZONE_CUSTOM_BUILD_PATH)/tools/TeeImgSignEncTool
endif
MTK_PRELOADER_PATH_CUSTOM := vendor/mediatek/proprietary/bootable/bootloader/preloader/custom/$(PRELOADER_TARGET_PRODUCT)

# FIXME: bootable/bootloader/preloader/platform/$(ARCH_MTK_PLATFORM)/src/security/trustzone/inc is used in makefile.mk
ifndef MTK_PLATFORM
  $(error MTK_PLATFORM is not defined)
endif
ARCH_MTK_PLATFORM := $(call LowerCase,$(MTK_PLATFORM))


### COMMON SETTINGS ###
ifeq ($(MTK_IN_HOUSE_TEE_SUPPORT),yes)
  ifeq ($(MTK_IN_HOUSE_TEE_FORCE_32_SUPPORT),yes)
    TRUSTZONE_IMPL := mtee32
  else
    TRUSTZONE_IMPL := mtee
  endif
else ifeq ($(TRUSTONIC_TEE_SUPPORT),yes)
    TRUSTZONE_IMPL := tbase
else ifeq ($(MICROTRUST_TEE_SUPPORT),yes)
    TRUSTZONE_IMPL := teeid
else ifeq ($(MICROTRUST_TEE_LITE_SUPPORT),yes)
    TRUSTZONE_IMPL := teeid
else ifeq ($(MTK_GOOGLE_TRUSTY_SUPPORT),yes)
    TRUSTZONE_IMPL := trusty
else ifeq ($(TRUSTKERNEL_TEE_SUPPORT),yes)
    TRUSTZONE_IMPL := tkcored
else ifeq ($(WATCHDATA_TEE_SUPPORT),yes)
    TRUSTZONE_IMPL := watchdata
else
    TRUSTZONE_IMPL := no
endif


### CUSTOMIZTION FILES ###
TRUSTZONE_IMG_PROTECT_CFG := $(MTK_PRELOADER_PATH_CUSTOM)/inc/TRUSTZONE_IMG_PROTECT_CFG.ini
ifeq ($(wildcard $(TRUSTZONE_IMG_PROTECT_CFG)),)
  ifeq ($(MTK_IN_HOUSE_TEE_SUPPORT),yes)
    TRUSTZONE_IMG_PROTECT_CFG := $(MTK_PATH_SOURCE)/trustzone/mtee/protect/common/cfg/$(ARCH_MTK_PLATFORM)/TRUSTZONE_IMG_PROTECT_CFG.ini
    ifeq ($(wildcard $(TRUSTZONE_IMG_PROTECT_CFG)),)
      TRUSTZONE_IMG_PROTECT_CFG := $(MTK_PATH_SOURCE)/trustzone/mtee/build/test/$(ARCH_MTK_PLATFORM)/TRUSTZONE_IMG_PROTECT_CFG.ini
    endif
  else
    TRUSTZONE_IMG_PROTECT_CFG := $(TRUSTZONE_CUSTOM_BUILD_PATH)/cfg/TRUSTZONE_IMG_PROTECT_CFG.ini
  endif
endif
TEE_DRAM_SIZE_CFG := $(MTK_PRELOADER_PATH_CUSTOM)/inc/SECURE_DRAM_SIZE_CFG.ini
ifeq ($(wildcard $(TEE_DRAM_SIZE_CFG)),)
  TEE_DRAM_SIZE_CFG := $(TRUSTZONE_CUSTOM_BUILD_PATH)/cfg/SECURE_DRAM_SIZE_CFG.ini
endif
ifneq ($(MTK_TEE_DRAM_SIZE),)
  TEE_DRAM_SIZE := $(MTK_TEE_DRAM_SIZE)
else
  TEE_DRAM_SIZE := $(shell cat $(TEE_DRAM_SIZE_CFG))
endif
ifeq ($(MTK_IN_HOUSE_TEE_SUPPORT),yes)
  TEE_TOTAL_DRAM_SIZE := $(TEE_DRAM_SIZE)
else
# MKIMAGE_HDR_SIZE := 0x240
  TEE_TOTAL_DRAM_SIZE := 0x$(shell echo "obase=16; $$(($(TEE_DRAM_SIZE) + 0x240))" | bc)
endif

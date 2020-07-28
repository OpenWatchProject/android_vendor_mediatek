
###############################################################################
# TBASE
###############################################################################
TEE_ALL_MODULE_MAKEFILE += $(call mtk_tee_find_module_makefile,cm,common,$(strip $(TRUSTONIC_TEE_VERSION))/tlCm)
TEE_ALL_MODULE_MAKEFILE += $(call mtk_tee_find_module_makefile,android,common,$(strip $(TRUSTONIC_TEE_VERSION))/DrAndroid)
TEE_ALL_MODULE_MAKEFILE += $(call mtk_tee_find_module_makefile,keymaster,common,$(strip $(TRUSTONIC_TEE_VERSION))/TlTeeKeymaster)
TEE_ALL_MODULE_MAKEFILE += $(call mtk_tee_find_module_makefile,gatekeeper,common,$(strip $(TRUSTONIC_TEE_VERSION))/TlTeeGatekeeper)
TEE_ALL_MODULE_MAKEFILE += $(call mtk_tee_find_module_makefile,mpu,platform,Drmpu)
TEE_ALL_MODULE_MAKEFILE += $(call mtk_tee_find_module_makefile,secmem,common,secmem_drbin)
TEE_ALL_MODULE_MAKEFILE += $(call mtk_tee_find_module_makefile,secmem,common,secmem_tabin)
TEE_ALL_MODULE_MAKEFILE += $(call mtk_tee_find_module_makefile,spi1,platform,Drspi)
TEE_ALL_MODULE_MAKEFILE += $(call mtk_tee_find_module_makefile,spi1,platform,Tlspi)
TEE_ALL_MODULE_MAKEFILE += $(call mtk_tee_find_module_makefile,rda,platform,Drrda)
TEE_ALL_MODULE_MAKEFILE += $(call mtk_tee_find_module_makefile,rda,platform,Tlrda)
ifeq ($(strip $(MTK_TEE_GP_SUPPORT)), yes)
  TEE_ALL_MODULE_MAKEFILE += $(call mtk_tee_find_module_makefile,msee_fwk,common,drv)
  TEE_ALL_MODULE_MAKEFILE += $(call mtk_tee_find_module_makefile,msee_fwk,common,ta)
  TEE_ALL_MODULE_MAKEFILE += $(call mtk_tee_find_module_makefile,rpmb,common,drv)
  #TEE_ALL_MODULE_MAKEFILE += $(call mtk_tee_find_module_makefile,rpmb,common,ta)
  TEE_ALL_MODULE_MAKEFILE += $(call mtk_tee_find_module_makefile,sec,common,drv)
  #TEE_ALL_MODULE_MAKEFILE += $(call mtk_tee_find_module_makefile,sec,common,ta)
else
  TEE_ALL_MODULE_MAKEFILE += $(call mtk_tee_find_module_makefile,rpmb,common,Drrpmb)
  #TEE_ALL_MODULE_MAKEFILE += $(call mtk_tee_find_module_makefile,rpmb,common,Tlrpmb)
  TEE_ALL_MODULE_MAKEFILE += $(call mtk_tee_find_module_makefile,sec,platform,Drsec)
  #TEE_ALL_MODULE_MAKEFILE += $(call mtk_tee_find_module_makefile,sec,platform,Tlsec)
endif

ifeq ($(strip $(MTK_FIPS_SUPPORT)), yes)
TEE_ALL_MODULE_MAKEFILE += $(call mtk_tee_find_module_makefile,fips,platform,DrFips)
TEE_ALL_MODULE_MAKEFILE += $(call mtk_tee_find_module_makefile,fips,platform,TlFips)
endif

###############################################################################
# TUI
###############################################################################
ifeq ($(strip $(MTK_TEE_TRUSTED_UI_SUPPORT)), yes)
  TEE_ALL_MODULE_MAKEFILE += $(call mtk_tee_find_module_makefile,tui,platform,DrTui)
  ifeq ($(TARGET_BUILD_VARIANT),eng)
    TEE_ALL_MODULE_MAKEFILE += $(call mtk_tee_find_module_makefile,tui,platform,DrTuiTest)
  endif
  TEE_ALL_MODULE_MAKEFILE += $(call mtk_tee_find_module_makefile,tui,platform,TlSamplePinpad)
endif

###############################################################################
# Keyinstall
###############################################################################
ifeq ($(strip $(MTK_DRM_KEY_MNG_SUPPORT)), yes)
TEE_ALL_MODULE_MAKEFILE += $(call mtk_tee_find_module_makefile,keyinstall,common,DrKeyInstall)
TEE_ALL_MODULE_MAKEFILE += $(call mtk_tee_find_module_makefile,keyinstall,common,TlKeyInstall)
TEE_ALL_MODULE_MAKEFILE += $(TEE_SOURCE_TRUSTLET_PATH)/keyinstall/common/TlKeyInstall/makefile.mk
endif

###############################################################################
# SVP
###############################################################################
ifeq ($(strip $(MTK_SEC_VIDEO_PATH_SUPPORT)), yes)
  TEE_ALL_MODULE_MAKEFILE += $(call mtk_tee_find_module_makefile,widevine,common,modular_drm/DrWidevineModularDrm)
  TEE_ALL_MODULE_MAKEFILE += $(call mtk_tee_find_module_makefile,widevine,common,modular_drm/TlWidevineModularDrm)
  TEE_ALL_MODULE_MAKEFILE += $(call mtk_tee_find_module_makefile,m4u,common,drv)
  TEE_ALL_MODULE_MAKEFILE += $(call mtk_tee_find_module_makefile,m4u,common,m4u_tl)
#TEE_ALL_MODULE_MAKEFILE += $(call mtk_tee_find_module_makefile,video,common,MtkH264Venc/MtkH264SecVencDrv)
#TEE_ALL_MODULE_MAKEFILE += $(call mtk_tee_find_module_makefile,video,common,MtkH264Venc/MtkH264SecVencTL)
  TEE_ALL_MODULE_MAKEFILE += $(call mtk_tee_find_module_makefile,video,common,MtkH264Vdec/MtkH264SecVdecDrv)
  TEE_ALL_MODULE_MAKEFILE += $(call mtk_tee_find_module_makefile,video,common,MtkH264Vdec/MtkH264SecVdecTL)
  #TEE_ALL_MODULE_MAKEFILE += $(call mtk_tee_find_module_makefile,video,common,MtkH265Vdec/MtkH265SecVdecDrv)
  #TEE_ALL_MODULE_MAKEFILE += $(call mtk_tee_find_module_makefile,video,common,MtkH265Vdec/MtkH265SecVdecTL)
  TEE_ALL_MODULE_MAKEFILE += $(call mtk_tee_find_module_makefile,video,common,MtkVP9Vdec/MtkVP9SecVdecDrv)
  TEE_ALL_MODULE_MAKEFILE += $(call mtk_tee_find_module_makefile,video,common,MtkVP9Vdec/MtkVP9SecVdecTL)
  TEE_ALL_MODULE_MAKEFILE += $(call mtk_tee_find_module_makefile,cmdq,common,drv)
  TEE_ALL_MODULE_MAKEFILE += $(call mtk_tee_find_module_makefile,cmdq,common,ta)
endif

###############################################################################
# IRIS Secure Camera
###############################################################################
ifeq ($(strip $(MTK_CAM_SECURITY_SUPPORT)), yes)
TEE_ALL_MODULE_MAKEFILE += $(call mtk_tee_find_module_makefile,camera/isp,common,drv)
TEE_ALL_MODULE_MAKEFILE += $(call mtk_tee_find_module_makefile,camera/isp,common,ta)
TEE_ALL_MODULE_MAKEFILE += $(call mtk_tee_find_module_makefile,imgsensor,common,drv)
TEE_ALL_MODULE_MAKEFILE += $(call mtk_tee_find_module_makefile,imgsensor,common,ta)
TEE_ALL_MODULE_MAKEFILE += $(call mtk_tee_find_module_makefile,m4u,common,drv)
TEE_ALL_MODULE_MAKEFILE += $(call mtk_tee_find_module_makefile,m4u,common,m4u_tl)
TEE_ALL_MODULE_MAKEFILE += $(call mtk_tee_find_module_makefile,cmdq,common,drv)
TEE_ALL_MODULE_MAKEFILE += $(call mtk_tee_find_module_makefile,cmdq,common,ta)
endif
$(warning TEE_ALL_MODULE_MAKEFILE=$(TEE_ALL_MODULE_MAKEFILE))

ifeq ($(strip $(MTK_M4U_SECURITY_TEE_SUPPORT)), yes)
TEE_ALL_MODULE_MAKEFILE += $(call mtk_tee_find_module_makefile,m4u,common,drv)
TEE_ALL_MODULE_MAKEFILE += $(call mtk_tee_find_module_makefile,m4u,common,m4u_tl)
endif

###############################################################################
# GENIEZONE
###############################################################################
ifeq ($(strip $(MTK_ENABLE_GENIEZONE)),yes)
TEE_ALL_MODULE_MAKEFILE += $(call mtk_tee_find_module_makefile,pmem,common,drv)
TEE_ALL_MODULE_MAKEFILE += $(call mtk_tee_find_module_makefile,pmem,common,ta)
ifeq ($(strip $(CFG_GZ_SECURE_DSP)),yes)
TEE_ALL_MODULE_MAKEFILE += $(call mtk_tee_find_module_makefile,fod_sample,common,drv)
TEE_ALL_MODULE_MAKEFILE += $(call mtk_tee_find_module_makefile,fod_sample,common,ta)
endif
endif

###############################################################################
# FP
###############################################################################
ifeq ($(strip $(MTK_FINGERPRINT_SUPPORT)),yes)
ifeq ($(MTK_FINGERPRINT_SELECT), $(filter $(MTK_FINGERPRINT_SELECT), GF316M GF318M GF3118M GF518M GF5118M GF516M GF816M GF3208 GF3206 GF3266 GF3288 GF5206 GF5216 GF5208))
TEE_ALL_MODULE_MAKEFILE += $(call mtk_tee_find_module_makefile,fingerprint,platform,goodix_fp/gf_dr)
TEE_ALL_MODULE_MAKEFILE += $(call mtk_tee_find_module_makefile,fingerprint,platform,goodix_fp/gf_ta)
#TEE_ALL_MODULE_MAKEFILE += $(call mtk_tee_find_module_makefile,fingerprint,platform,goodix_fp/gf_alipay_ta)
#TEE_ALL_MODULE_MAKEFILE += $(call mtk_tee_find_module_makefile,fingerprint,platform,goodix_fp/gf_ifaa_ta)
endif
endif

##
# vendor/trustonic/platform/mtXXXX/t-base/setup.h
# toolchain and SDK path
##
TEE_CROSS_GCC_PATH := prebuilts/gcc/linux-x86/arm/gcc-arm-none-eabi-4_8-2014q3
TEE_GLOBAL_MAKE_OPTION :=
TEE_GLOBAL_MAKE_OPTION += CROSS_GCC_PATH_INC=$(CURDIR)/$(TEE_CROSS_GCC_PATH)/arm-none-eabi/include
TEE_GLOBAL_MAKE_OPTION += CROSS_GCC_PATH_LIB=$(CURDIR)/$(TEE_CROSS_GCC_PATH)/arm-none-eabi/lib
TEE_GLOBAL_MAKE_OPTION += CROSS_GCC_PATH_LGCC=$(CURDIR)/$(TEE_CROSS_GCC_PATH)/lib/gcc/arm-none-eabi/4.8.4
TEE_GLOBAL_MAKE_OPTION += CROSS_GCC_PATH_BIN=$(CURDIR)/$(TEE_CROSS_GCC_PATH)/bin

TEE_SOURCE_BSP_PATH := $(MTK_PATH_SOURCE)/trustzone/trustonic/source/bsp
TEE_INTERNAL_BSP_PATH := $(MTK_PATH_SOURCE)/trustzone/trustonic/internal/bsp
TEE_SOURCE_SDK_PATH := $(TEE_SOURCE_BSP_PATH)/common/$(TRUSTONIC_TEE_VERSION)/t-sdk
TEE_INTERNAL_SDK_PATH := $(TEE_INTERNAL_BSP_PATH)/common/$(TRUSTONIC_TEE_VERSION)/t-sdk
TEE_MOBICORE_TOOL_ROOT := $(TEE_SOURCE_BSP_PATH)/common/$(TRUSTONIC_TEE_VERSION)/tools
TEE_HAL_SOURCE_TRUSTLETS_PATH := $(MTK_PATH_SOURCE)/trustzone/common/hal/source/trustlets
TEE_HAL_SECURE_TRUSTLETS_PATH := $(MTK_PATH_SOURCE)/trustzone/common/hal/secure/trustlets

COMP_PATH_MobiConfig := $(CURDIR)/$(TEE_MOBICORE_TOOL_ROOT)/MobiConfig
TEE_GLOBAL_MAKE_OPTION += COMP_PATH_MobiConfig=$(COMP_PATH_MobiConfig)

# m4u
TEE_GLOBAL_MAKE_OPTION += COMP_PATH_M4U_ROOT=$(CURDIR)/$(TEE_PROTECT_TRUSTLET_PATH)/m4u/common/
TEE_GLOBAL_MAKE_OPTION += COMP_PATH_M4U_CROSS_INC=$(CURDIR)/$(strip $(LINUX_KERNEL_VERSION))/drivers/misc/mediatek/include/mt-plat/$(ARCH_MTK_PLATFORM)/include/trustzone/m4u
TEE_GLOBAL_MAKE_OPTION += COMP_PATH_Drsec_Export=$(CURDIR)/$(TEE_SOURCE_TRUSTLET_PATH)/sec/platform/$(ARCH_MTK_PLATFORM)/Drsec

# secmem driver API header
TEE_GLOBAL_MAKE_OPTION += COMP_PATH_Secmem_Export=$(CURDIR)/vendor/mediatek/proprietary/trustzone/common/hal/source/trustlets/secmem/common/drv_api_inc

# video
TEE_GLOBAL_MAKE_OPTION += COMP_PATH_ION_H_INC=$(CURDIR)/bionic/libc/kernel/uapi
TEE_GLOBAL_MAKE_OPTION += COMP_PATH_IOCTRL_H_INC=$(CURDIR)/bionic/libc/kernel/uapi/asm-arm
TEE_GLOBAL_MAKE_OPTION += COMP_PATH_HW_VCODEC_COMM=$(CURDIR)/vendor/mediatek/proprietary/hardware/libvcodec_v2/common/hardware
TEE_GLOBAL_MAKE_OPTION += COMP_PATH_HW_VCODEC_HDR=$(CURDIR)/device/mediatek/common/kernel-headers
TEE_GLOBAL_MAKE_OPTION += COMP_PATH_HW_VCODEC=$(CURDIR)/vendor/mediatek/proprietary/hardware/libvcodec_v2
TEE_GLOBAL_MAKE_OPTION += COMP_PATH_DRSEC=$(CURDIR)/$(TEE_SOURCE_TRUSTLET_PATH)/sec/platform/$(ARCH_MTK_PLATFORM)/Drsec

# dapc and devinfo
ifeq ($(strip $(MTK_TEE_GP_SUPPORT)), yes)
  TEE_GLOBAL_MAKE_OPTION += COMP_PATH_drsec_drv=$(CURDIR)/$(TEE_HAL_SECURE_TRUSTLETS_PATH)/sec/common/drv
  TEE_GLOBAL_MAKE_OPTION += COMP_PATH_drsec_plat=$(CURDIR)/$(TEE_HAL_SECURE_TRUSTLETS_PATH)/sec/common/plat/$(ARCH_MTK_PLATFORM)
endif # MTK_TEE_GP_SUPPORT=yes

# widevine
#TEE_GLOBAL_MAKE_OPTION += COMP_PATH_drtplay=$(CURDIR)/$(TEE_SOURCE_TRUSTLET_PATH)/tplay/platform/$(ARCH_MTK_PLATFORM)/Drtplay

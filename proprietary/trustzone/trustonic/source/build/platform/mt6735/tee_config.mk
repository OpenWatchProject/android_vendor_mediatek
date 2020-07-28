
################################################################################
# TBASE
################################################################################
TEE_ALL_MODULE_MAKEFILE += $(call mtk_tee_find_module_makefile,cm,common/$(strip $(TRUSTONIC_TEE_VERSION)),tlCm)
TEE_ALL_MODULE_MAKEFILE += $(call mtk_tee_find_module_makefile,android,common,$(strip $(TRUSTONIC_TEE_VERSION))/DrAndroid)
TEE_ALL_MODULE_MAKEFILE += $(call mtk_tee_find_module_makefile,keymaster,common,$(strip $(TRUSTONIC_TEE_VERSION))/TlTeeKeymaster)
TEE_ALL_MODULE_MAKEFILE += $(call mtk_tee_find_module_makefile,gatekeeper,common,302d/TlTeeGatekeeper)
TEE_ALL_MODULE_MAKEFILE += $(call mtk_tee_find_module_makefile,secmem,common,secmem_drbin)
TEE_ALL_MODULE_MAKEFILE += $(call mtk_tee_find_module_makefile,secmem,common,secmem_tabin)
TEE_ALL_MODULE_MAKEFILE += $(call mtk_tee_find_module_makefile,spi,platform,Drspi)
TEE_ALL_MODULE_MAKEFILE += $(call mtk_tee_find_module_makefile,spi,platform,Tlspi)
TEE_ALL_MODULE_MAKEFILE += $(call mtk_tee_find_module_makefile,sec,platform,Drsec)
TEE_ALL_MODULE_MAKEFILE += $(call mtk_tee_find_module_makefile,sec,platform,Tlsec)
TEE_ALL_MODULE_MAKEFILE += $(call mtk_tee_find_module_makefile,msee_fwk,common,drv)
TEE_ALL_MODULE_MAKEFILE += $(call mtk_tee_find_module_makefile,msee_fwk,common,ta)
TEE_ALL_MODULE_MAKEFILE += $(call mtk_tee_find_module_makefile,rpmb,common,Drrpmb)

################################################################################
# TBASE
################################################################################
ifeq ($(MTK_MACH_TYPE),mt6753)
TEE_ALL_MODULE_MAKEFILE += $(call mtk_tee_find_module_makefile,tui,platform,DrTui)
ifeq ($(TARGET_BUILD_VARIANT),eng)
TEE_ALL_MODULE_MAKEFILE += $(call mtk_tee_find_module_makefile,tui,platform,DrTuiTest)
endif
TEE_ALL_MODULE_MAKEFILE += $(call mtk_tee_find_module_makefile,tui,platform,TlSamplePinpad)
endif

################################################################################
# Keyinstall
################################################################################
ifeq ($(strip $(MTK_DRM_KEY_MNG_SUPPORT)), yes)
TEE_ALL_MODULE_MAKEFILE += $(call mtk_tee_find_module_makefile,keyinstall,common,DrKeyInstall)
TEE_ALL_MODULE_MAKEFILE += $(call mtk_tee_find_module_makefile,keyinstall,common,TlKeyInstall)
TEE_ALL_MODULE_MAKEFILE += $(TEE_SOURCE_TRUSTLET_PATH)/keyinstall/common/TlKeyInstall/makefile.mk
endif

################################################################################
# SVP
################################################################################
ifeq ($(strip $(MTK_SEC_VIDEO_PATH_SUPPORT)), yes)
TEE_ALL_MODULE_MAKEFILE += $(call mtk_tee_find_module_makefile,widevine,common,classic_drm/DrWidevineClassicDrm)
TEE_ALL_MODULE_MAKEFILE += $(call mtk_tee_find_module_makefile,widevine,common,classic_drm/TlWidevineClassicDrm)
TEE_ALL_MODULE_MAKEFILE += $(call mtk_tee_find_module_makefile,m4u,platform,m4u_drv)
TEE_ALL_MODULE_MAKEFILE += $(call mtk_tee_find_module_makefile,m4u,platform,m4u_tl)
TEE_ALL_MODULE_MAKEFILE += $(call mtk_tee_find_module_makefile,cmdq,platform,cmdq_sec_dr)
TEE_ALL_MODULE_MAKEFILE += $(call mtk_tee_find_module_makefile,cmdq,platform,cmdq_sec_tl) 
#TEE_ALL_MODULE_MAKEFILE += $(call mtk_tee_find_module_makefile,video,platform,MtkH264Venc/MtkH264SecVencDrv)
#TEE_ALL_MODULE_MAKEFILE += $(call mtk_tee_find_module_makefile,video,platform,MtkH264Venc/MtkH264SecVencTL)
#TEE_ALL_MODULE_MAKEFILE += $(call mtk_tee_find_module_makefile,video,platform,MtkH264Vdec/MtkH264SecVdecDrv)
#TEE_ALL_MODULE_MAKEFILE += $(call mtk_tee_find_module_makefile,video,platform,MtkH264Vdec/MtkH264SecVdecTL)
endif

################################################################################
# HDCP TX
################################################################################
ifeq ($(strip $(MTK_WFD_HDCP_TX_SUPPORT)), yes)
TEE_ALL_MODULE_MAKEFILE += $(call mtk_tee_find_module_makefile,hdcptx,common,TlHdcpTx)
TEE_ALL_MODULE_MAKEFILE += $(call mtk_tee_find_module_makefile,hdcptx,common,DrHdcpTx)
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

COMP_PATH_MobiConfig := $(CURDIR)/$(TEE_MOBICORE_TOOL_ROOT)/MobiConfig
TEE_GLOBAL_MAKE_OPTION += COMP_PATH_MobiConfig=$(COMP_PATH_MobiConfig)

# secmem driver API header
TEE_GLOBAL_MAKE_OPTION += COMP_PATH_Secmem_Export=$(CURDIR)/vendor/mediatek/proprietary/trustzone/common/hal/source/trustlets/secmem/common/drv_api_inc

# m4u
TEE_GLOBAL_MAKE_OPTION += COMP_PATH_M4U_ROOT=$(CURDIR)/$(TEE_PROTECT_TRUSTLET_PATH)/m4u/platform/$(ARCH_MTK_PLATFORM)
TEE_GLOBAL_MAKE_OPTION += COMP_PATH_M4U_CROSS_INC=$(CURDIR)/device/mediatek/$(ARCH_MTK_PLATFORM)/kernel-headers
TEE_GLOBAL_MAKE_OPTION += COMP_PATH_Drsec_Export=$(CURDIR)/$(TEE_SOURCE_TRUSTLET_PATH)/sec/platform/$(ARCH_MTK_PLATFORM)/Drsec
TEE_GLOBAL_MAKE_OPTION += TEE_DRIVER_OUTPUT_PATH=${CURDIR}/${TRUSTZONE_OUTPUT_PATH}/driver

#HDCP TX
ifeq ($(strip $(MTK_WFD_HDCP_TX_SUPPORT)), yes)
TEE_GLOBAL_MAKE_OPTION += COMP_PATH_HDCPTX_ROOT=$(CURDIR)/$(TEE_PROTECT_TRUSTLET_PATH)/hdcptx/common
TEE_GLOBAL_MAKE_OPTION += COMP_PATH_HDCPTX_SOURCE=$(CURDIR)/$(TEE_SOURCE_TRUSTLET_PATH)
endif

# video
TEE_GLOBAL_MAKE_OPTION += COMP_PATH_ION_H_INC=$(CURDIR)/bionic/libc/kernel/uapi/
TEE_GLOBAL_MAKE_OPTION += COMP_PATH_IOCTRL_H_INC=$(CURDIR)/bionic/libc/kernel/uapi/asm-arm
TEE_GLOBAL_MAKE_OPTION += COMP_PATH_HW_VCODEC_COMM=$(CURDIR)/vendor/mediatek/proprietary/hardware/libvcodec/common/hardware
TEE_GLOBAL_MAKE_OPTION += COMP_PATH_HW_VCODEC_HDR=$(CURDIR)/device/mediatek/common/kernel-headers
TEE_GLOBAL_MAKE_OPTION += COMP_PATH_DRSEC=$(CURDIR)/$(TEE_SOURCE_TRUSTLET_PATH)/sec/platform/$(ARCH_MTK_PLATFORM)/Drsec

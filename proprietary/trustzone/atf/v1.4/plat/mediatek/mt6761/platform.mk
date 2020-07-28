#
# Copyright (c) 2013-2017, ARM Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

MTK_PLAT		:=	plat/mediatek
MTK_PLAT_SOC		:=	${MTK_PLAT}/${PLAT}
include ${MTK_PLAT}/common/mtk_common.mk

# Add OEM customized codes
OEMS				:= true

# Enable dynamic memory mapping
PLAT_XLAT_TABLES_DYNAMIC :=    1
$(eval $(call add_define,PLAT_XLAT_TABLES_DYNAMIC))

include lib/xlat_tables_v2/xlat_tables.mk

ifneq (${OEMS},none)
  OEMS_INCLUDES		:= -I${MTK_PLAT}/common/custom/
  OEMS_SOURCES		:=	${MTK_PLAT}/common/custom/oem_svc.c
endif

PLAT_INCLUDES		:=	\
				-Iinclude/plat/arm/common/					\
				-Iinclude/common/tbbr/						\
				-Idrivers/arm/gic/v3/						\
				-I${MTK_PLAT}/common/						\
				-I${MTK_PLAT}/common/drivers/uart			\
				-I${MTK_PLAT}/common/drivers/emi		\
				-I${MTK_PLAT}/common/drivers/fiq_cache		\
				-I${MTK_PLAT}/common/drivers/mcsi			\
				-I${MTK_PLAT_SOC}/drivers/emi			\
				-I${MTK_PLAT_SOC}/drivers/dbgtool/			\
				-I${MTK_PLAT_SOC}/drivers/gpio/				\
				-I${MTK_PLAT_SOC}/drivers/l2c/				\
				-I${MTK_PLAT_SOC}/drivers/i2c/				\
				-I${MTK_PLAT_SOC}/drivers/wfifo/			\
				-I${MTK_PLAT_SOC}/drivers/mcdi/				\
				-I${MTK_PLAT_SOC}/drivers/pmic/				\
				-I${MTK_PLAT_SOC}/drivers/rtc/				\
				-I${MTK_PLAT_SOC}/drivers/spm/				\
				-I${MTK_PLAT_SOC}/drivers/sec/			\
				-I${MTK_PLAT_SOC}/drivers/spmc/				\
				-I${MTK_PLAT_SOC}/drivers/timer/			\
				-I${MTK_PLAT_SOC}/drivers/devapc/			\
                                -I${MTK_PLAT_SOC}/drivers/crypto/                       \
                                -I${MTK_PLAT_SOC}/drivers/cm_mgr/			\
				-I${MTK_PLAT_SOC}/drivers/pccif/			\
				-I${MTK_PLAT_SOC}/drivers/amms/				\
				-I${MTK_PLAT_SOC}/drivers/md/				\
				-I${MTK_PLAT_SOC}/include/					\
				 -I${MTK_PLAT_SOC}/drivers/sec/                   \
				-I${MTK_PLAT_SOC}/drivers/cmdq/				\
				${OEMS_INCLUDES}

PLAT_BL_COMMON_SOURCES	:= ${XLAT_TABLES_LIB_SRCS}  \
                plat/common/aarch64/plat_common.c	\
				plat/common/plat_gic.c

BL31_SOURCES    	+= 	drivers/arm/cci/cci.c					\
				drivers/arm/gic/arm_gic.c							\
				drivers/arm/gic/gic_v2.c							\
				drivers/arm/gic/gic_v3.c							\
				drivers/arm/gic/v3/gicv3_helpers.c					\
				drivers/console/console.S							\
				drivers/delay_timer/delay_timer.c					\
				lib/cpus/aarch64/aem_generic.S						\
				lib/cpus/aarch64/cortex_a35.S						\
				lib/cpus/aarch64/cortex_a53.S						\
				lib/cpus/aarch64/cortex_a57.S						\
				lib/cpus/aarch64/cortex_a72.S						\
				lib/cpus/aarch64/cortex_a73.S						\
				${MTK_PLAT}/common/drivers/fiq_cache/fiq_cache_v1.c	\
				${MTK_PLAT_SOC}/aarch64/plat_helpers.S				\
				${MTK_PLAT_SOC}/bl31_plat_setup.c					\
                   ${MTK_PLAT_SOC}/drivers/sec/rpmb_hmac.c        \
                   ${MTK_PLAT_SOC}/drivers/sec/rng.c              \
				${MTK_PLAT_SOC}/plat_sip_svc.c						\
				${MTK_PLAT_SOC}/plat_delay_timer.c					\
				${MTK_PLAT_SOC}/plat_mt_gic.c						\
				${MTK_PLAT_SOC}/plat_debug.c						\
				${MTK_PLAT_SOC}/plat_topology.c						\
				${MTK_PLAT_SOC}/power_tracer.c						\
				${MTK_PLAT_SOC}/scu.c								\
				${MTK_PLAT_SOC}/drivers/amms/amms.c					\
				${MTK_PLAT_SOC}/drivers/pccif/pccif.c				\
				${MTK_PLAT_SOC}/drivers/sec/crypto_util.c				\
				${MTK_PLAT_SOC}/drivers/cmdq/cmdq.c

ifeq ($(ATF_BYPASS_DRAM), yes)
BL31_SOURCE_DRAM := ${MTK_PLAT_SOC}/plat_pm.c
else
BL31_SOURCE_DRAM :=		\
				${MTK_PLAT}/common/drivers/emi/mpu_v1.c		\
				${MTK_PLAT_SOC}/plat_dcm.c							\
				${MTK_PLAT_SOC}/plat_pm.c							\
				${MTK_PLAT_SOC}/drivers/dbgtool/latch.c				\
				${MTK_PLAT_SOC}/drivers/gpio/gpio.c					\
				${MTK_PLAT_SOC}/drivers/l2c/l2c.c					\
				${MTK_PLAT_SOC}/drivers/i2c/mtk_i2c.c					\
				${MTK_PLAT_SOC}/drivers/wfifo/wfifo.c					\
				${MTK_PLAT_SOC}/drivers/mcdi/mtk_mcdi.c       		\
				${MTK_PLAT_SOC}/drivers/pmic/pmic_wrap_init.c		\
				${MTK_PLAT_SOC}/drivers/rtc/rtc.c					\
				${MTK_PLAT_SOC}/drivers/spm/mt_spm.c				\
				${MTK_PLAT_SOC}/drivers/spm/mt_spm_sleep.c			\
				${MTK_PLAT_SOC}/drivers/spm/mt_spm_dpidle.c			\
                                ${MTK_PLAT_SOC}/drivers/crypto/crypto.c                         \
                                ${MTK_PLAT_SOC}/drivers/crypto/msdc_crypto.c                    \
				${MTK_PLAT_SOC}/drivers/spm/mt_spm_sodi.c			\
				${MTK_PLAT_SOC}/drivers/spm/mt_spm_idle.c			\
				${MTK_PLAT_SOC}/drivers/spm/mt_spm_internal.c		\
				${MTK_PLAT_SOC}/drivers/spm/mt_spm_vcorefs.c		\
				${MTK_PLAT_SOC}/drivers/spm/mt_spm_pmic_wrap.c		\
                    ${MTK_PLAT_SOC}/drivers/md/md.c               \
				${MTK_PLAT_SOC}/drivers/spmc/mtspmc.c				\
				${MTK_PLAT_SOC}/drivers/timer/mt_cpuxgpt.c			\
				${MTK_PLAT_SOC}/drivers/timer/mt_systimer.c			\
				${MTK_PLAT_SOC}/drivers/devapc/devapc.c			\
				${MTK_PLAT_SOC}/drivers/cm_mgr/mtk_cm_mgr.c			\
				${OEMS_SOURCES}
endif

STATIC_LIBS += ${MTK_PLAT_SOC}/drivers/sec/lib/sha2.a

CFLAGS      += -D__MTK_RPMB

ifeq (${SPD}, tbase)
-include ${MTK_PLAT_SOC}/plat_tbase.mk
TBASE_AARCH ?= kinibi_aarch32
PLAT_INCLUDES += -I${MTK_PLAT_SOC}/include/${TBASE_AARCH}
BL31_SOURCE_DRAM += ${MTK_PLAT_SOC}/${TBASE_AARCH}/plat_tbase.c
ifeq (${TBASE_AARCH}, kinibi_aarch64)
BL31_SOURCE_DRAM += ${MTK_PLAT_SOC}/${TBASE_AARCH}/plat_tbase_pm.c
endif
endif

ifeq (${SPD}, teeid)
MICROTRUST_TEE_VERSION ?= 300
$(info "platform.mk:MICROTRUST_TEE_VERSION=$(MICROTRUST_TEE_VERSION)")
ATF_GLOBAL_MAKE_OPTION += MICROTRUST_TEE_VERSION=$(MICROTRUST_TEE_VERSION)
BL31_SOURCE_DRAM += ${MTK_PLAT_SOC}/plat_teei.c
CFLAGS += -DMICROTRUST_TEE_VERSION=$(MICROTRUST_TEE_VERSION)
CFLAGS += -DCFG_MICROTRUST_TEE_SUPPORT=1
CFLAGS += -DMICROTRUST_ATF_VERSION=$(VERSION_MAJOR)$(VERSION_MINOR)
ASFLAGS += -DMICROTRUST_ATF_VERSION=$(VERSION_MAJOR)$(VERSION_MINOR)
NS_TIMER_SWITCH := 1
endif

ifeq (${SPD}, tkcored)
BL31_SOURCE_DRAM += ${MTK_PLAT_SOC}/plat_tkcore.c
CFLAGS += -DCFG_TRUSTKERNEL_TEE_SUPPORT=1
endif

# Flag used by the MTK_platform port to determine the version of ARM GIC
# architecture to use for interrupt management in EL3.
ARM_GIC_ARCH		:=	2
$(eval $(call add_define,ARM_GIC_ARCH))

# Enable workarounds for selected Cortex-A53 erratas.
ERRATA_A53_826319	:=	0
ERRATA_A53_836870	:=	1
ERRATA_A53_855873	:=	1

# indicate the reset vector address can be programmed
PROGRAMMABLE_RESET_ADDRESS	:=	1

# Disable the PSCI platform compatibility layer
ENABLE_PLAT_COMPAT	:=	1

# Enable SRAM_FLAG support for DFD/LASTPC
CFG_PLAT_SRAM_FLAG	:=	1

ifeq (${ATF_BYPASS_DRAM},yes)
$(eval $(call add_define,ATF_BYPASS_DRAM))
endif

#Full ATF ram dump
ifeq (${MTK_ATF_RAM_DUMP},yes)
MTK_ATF_RAM_DUMP := 1
$(eval $(call add_define,MTK_ATF_RAM_DUMP))
endif

ifeq (${MTK_ATF_LOG_BUF_SLIM},yes)
MTK_ATF_LOG_BUF_SLIM := 1
$(eval $(call add_define,MTK_ATF_LOG_BUF_SLIM))
endif

# Enable CM_MGR support
ifeq (${MTK_CM_MGR},yes)
$(eval $(call add_define,MTK_CM_MGR))
endif

ifeq (${CFG_PLAT_SRAM_FLAG},1)
$(eval $(call add_define,CFG_PLAT_SRAM_FLAG))
endif

ifeq (${DRAM_EXTENSION_SUPPORT},yes)
$(eval $(call add_define,DRAM_EXTENSION_SUPPORT))
endif

ifeq (${MTK_EMMC_SUPPORT},yes)
$(eval $(call add_define,MTK_MSDC_HW_FDE))
endif

ifeq (${MTK_FPGA_EARLY_PORTING},yes)
$(eval $(call add_define,MTK_FPGA_EARLY_PORTING))
endif

ifeq (${MTK_FIQ_CACHE_SUPPORT},yes)
$(eval $(call add_define,MTK_FIQ_CACHE_SUPPORT))
endif

ifeq (${MTK_DEBUGSYS_LOCK},yes)
$(eval $(call add_define,MTK_DEBUGSYS_LOCK))
endif


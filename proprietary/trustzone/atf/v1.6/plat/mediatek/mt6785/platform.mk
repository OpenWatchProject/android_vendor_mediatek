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

# Add MTK GICv3 common driver
MTK_GICV3_COMMON	:= true
MTK_SIP_KERNEL_BOOT_ENABLE := 1

# Enable dynamic memory mapping
PLAT_XLAT_TABLES_DYNAMIC :=    1

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
				-I${MTK_PLAT}/common/drivers/mcsi			\
				-I${MTK_PLAT}/common/drivers/emi/			\
				-I${MTK_PLAT}/common/drivers/tee/			\
				-I${MTK_PLAT_SOC}/drivers/emi/				\
				-I${MTK_PLAT_SOC}/drivers/dbgtool/			\
				-I${MTK_PLAT_SOC}/drivers/devapc/			\
				-I${MTK_PLAT_SOC}/drivers/gpio/				\
				-I${MTK_PLAT_SOC}/drivers/l2c/				\
				-I${MTK_PLAT_SOC}/drivers/i2c/				\
				-I${MTK_PLAT_SOC}/drivers/m4u/				\
				-I${MTK_PLAT_SOC}/drivers/drcc/				\
				-I${MTK_PLAT_SOC}/drivers/mcdi/				\
				-I${MTK_PLAT_SOC}/drivers/sec/				\
				-I${MTK_PLAT_SOC}/drivers/crypto/			\
				-I${MTK_PLAT_SOC}/drivers/pmic/				\
				-I${MTK_PLAT_SOC}/drivers/rtc/				\
				-I${MTK_PLAT_SOC}/drivers/spm/				\
				-I${MTK_PLAT_SOC}/drivers/sec/				\
				-I${MTK_PLAT_SOC}/drivers/spmc/				\
				-I${MTK_PLAT_SOC}/drivers/ildo/				\
				-I${MTK_PLAT_SOC}/drivers/timer/			\
				-I${MTK_PLAT_SOC}/drivers/cm_mgr/			\
				-I${MTK_PLAT_SOC}/drivers/md/    			\
				-I${MTK_PLAT_SOC}/drivers/pccif/            \
				-I${MTK_PLAT_SOC}/drivers/amms/             \
				-I${MTK_PLAT_SOC}/drivers/ccci/             \
				-I${MTK_PLAT_SOC}/drivers/udi/              \
				-I${MTK_PLAT_SOC}/drivers/apu/             \
				-I${MTK_PLAT_SOC}/drivers/cmdq/				\
				-I${MTK_PLAT_SOC}/include/				\
				${OEMS_INCLUDES}

PLAT_BL_COMMON_SOURCES	:= ${XLAT_TABLES_LIB_SRCS}			\
				plat/common/plat_gic.c

PLAT_BL_COMMON_SOURCES	+=	\
				drivers/arm/gic/arm_gic.c							\
				drivers/arm/gic/gic_v2.c							\
				drivers/arm/gic/gic_v3.c							\
				drivers/arm/gic/v3/gicv3_helpers.c					\
				drivers/console/console.S							\
				drivers/delay_timer/delay_timer.c					\
				lib/cpus/aarch64/aem_generic.S						\
				lib/cpus/aarch64/cortex_a55.S						\
				lib/cpus/aarch64/cortex_a76.S


ifeq ($(ATF_BYPASS_DRAM), yes)
PLAT_BL_COMMON_SOURCES	+=	${MTK_PLAT_SOC}/drivers/timer/mt_cpuxgpt.c			\
					${MTK_PLAT_SOC}/drivers/timer/mt_systimer.c	\
					${MTK_PLAT_SOC}/plat_pm.c
else
PLAT_BL_COMMON_SOURCES += ${MTK_PLAT}/common/drivers/mcsi/mcsi.c				\
				${MTK_PLAT_SOC}/aarch64/plat_helpers.S				\
				${MTK_PLAT_SOC}/bl31_plat_setup.c					\
				${MTK_PLAT_SOC}/plat_delay_timer.c					\
				${MTK_PLAT_SOC}/plat_debug.c						\
				${MTK_PLAT_SOC}/plat_topology.c						\
				${MTK_PLAT_SOC}/power_tracer.c						\
				${MTK_PLAT_SOC}/scu.c								\
				${MTK_PLAT}/common/drivers/emi/mpu_v1.c				\
				${MTK_PLAT}/common/drivers/tee/tee.c				\
				${MTK_PLAT_SOC}/plat_sip_svc.c						\
				${MTK_PLAT_SOC}/plat_dcm.c							\
				${MTK_PLAT_SOC}/plat_dfd.c							\
				${MTK_PLAT_SOC}/plat_pm.c							\
				${MTK_PLAT_SOC}/drivers/dbgtool/latch.c				\
				${MTK_PLAT_SOC}/drivers/gpio/gpio.c					\
				${MTK_PLAT_SOC}/drivers/l2c/l2c.c					\
				${MTK_PLAT_SOC}/drivers/drcc/mtk_drcc.c					\
				${MTK_PLAT_SOC}/drivers/i2c/mt_i2c.c					\
				${MTK_PLAT_SOC}/drivers/m4u/mt_m4u.c					\
				${MTK_PLAT_SOC}/drivers/mcdi/mtk_mcdi.c       		\
				${MTK_PLAT_SOC}/drivers/pmic/pmic_wrap_init.c		\
				${MTK_PLAT_SOC}/drivers/rtc/rtc.c					\
				${MTK_PLAT_SOC}/drivers/spm/mt_spm.c				\
				${MTK_PLAT_SOC}/drivers/spm/mt_spm_sleep.c			\
				${MTK_PLAT_SOC}/drivers/spm/mt_spm_idle.c			\
				${MTK_PLAT_SOC}/drivers/spm/mt_spm_internal.c		\
				${MTK_PLAT_SOC}/drivers/spm/mt_spm_vcorefs.c		\
				${MTK_PLAT_SOC}/drivers/spm/mt_spm_pmic_wrap.c		\
				${MTK_PLAT_SOC}/drivers/crypto/crypto.c			\
				${MTK_PLAT_SOC}/drivers/crypto/ufs_crypto.c		\
				${MTK_PLAT_SOC}/drivers/crypto/msdc_crypto.c		\
				${MTK_PLAT_SOC}/drivers/spmc/mtspmc.c				\
				${MTK_PLAT_SOC}/drivers/ildo/mt_ildo.c				\
				${MTK_PLAT_SOC}/drivers/timer/mt_cpuxgpt.c			\
				${MTK_PLAT_SOC}/drivers/timer/mt_systimer.c			\
				${MTK_PLAT_SOC}/drivers/cm_mgr/mtk_cm_mgr.c			\
				${MTK_PLAT_SOC}/drivers/devapc/devapc.c			\
				${MTK_PLAT_SOC}/drivers/devapc/devapc_hal.c		\
                ${MTK_PLAT_SOC}/drivers/md/md.c               \
				${MTK_PLAT_SOC}/drivers/sec/rpmb_hmac.c					\
				${MTK_PLAT_SOC}/drivers/sec/rng.c					\
				${MTK_PLAT_SOC}/drivers/amms/amms.c					\
				${MTK_PLAT_SOC}/drivers/pccif/pccif.c               \
				${MTK_PLAT_SOC}/drivers/ccci/ccci.c               \
				${MTK_PLAT_SOC}/drivers/udi/mtk_udi_api.c           \
				${MTK_PLAT_SOC}/plat_mtk_gic.c						\
				${MTK_PLAT_SOC}/drivers/spm/mt_spm_conservation.c	\
				${MTK_PLAT_SOC}/drivers/spm/mt_spm_dram_bus26m.c	\
				${MTK_PLAT_SOC}/drivers/spm/mt_spm_syspll.c			\
				${MTK_PLAT_SOC}/drivers/apu/apu.c			\
				${MTK_PLAT_SOC}/drivers/sec/crypto_util.c	\
				${MTK_PLAT_SOC}/drivers/cmdq/cmdq.c			\
				${OEMS_SOURCES}
endif

ifneq (${MTK_GICV3_COMMON},none)
# for using mtk gicv3 common driver
PLAT_BL_COMMON_SOURCES +=	${MTK_PLAT}/common/mtk_gic_v3_main.c
endif

STATIC_LIBS += ${MTK_PLAT_SOC}/drivers/sec/lib/sha2.a
CFLAGS      += -D__MTK_RPMB

ifeq (${SPD}, tbase)
-include ${MTK_PLAT_SOC}/plat_tbase.mk
TBASE_AARCH ?= kinibi_aarch32
PLAT_INCLUDES += -I${MTK_PLAT_SOC}/include/${TBASE_AARCH}
PLAT_BL_COMMON_SOURCES += ${MTK_PLAT_SOC}/${TBASE_AARCH}/plat_tbase.c
ifeq (${TBASE_AARCH}, kinibi_aarch64)
PLAT_BL_COMMON_SOURCES += ${MTK_PLAT_SOC}/${TBASE_AARCH}/plat_tbase_pm.c
endif
endif

ifeq (${SPD}, teeid)
MICROTRUST_TEE_VERSION ?= 300
$(info "platform.mk:MICROTRUST_TEE_VERSION=$(MICROTRUST_TEE_VERSION)")
ATF_GLOBAL_MAKE_OPTION += MICROTRUST_TEE_VERSION=$(MICROTRUST_TEE_VERSION)
PLAT_BL_COMMON_SOURCES += ${MTK_PLAT_SOC}/spd/teeid/plat_teei.c
PLAT_INCLUDES          += -I${MTK_PLAT_SOC}/spd/teeid/
CFLAGS += -DMICROTRUST_TEE_VERSION=$(MICROTRUST_TEE_VERSION)
CFLAGS += -DCFG_MICROTRUST_TEE_SUPPORT=1
CFLAGS += -DMICROTRUST_ATF_VERSION=$(VERSION_MAJOR)$(VERSION_MINOR)
ASFLAGS += -DMICROTRUST_ATF_VERSION=$(VERSION_MAJOR)$(VERSION_MINOR)
NS_TIMER_SWITCH := 1
endif

# Flag used by the MTK_platform port to determine the version of ARM GIC
# architecture to use for interrupt management in EL3.
ARM_GIC_ARCH		:=	2
$(eval $(call add_define,ARM_GIC_ARCH))

# Enable workarounds for selected Cortex-A53 erratas.
ERRATA_A53_826319	:=	0
ERRATA_A53_836870	:=	0

WORKAROUND_CVE_2018_3639	:=	1
DYNAMIC_WORKAROUND_CVE_2018_3639	:=	1
WORKAROUND_CVE_2017_5715	:=	0
# indicate the reset vector address can be programmed
PROGRAMMABLE_RESET_ADDRESS	:=	1

# Disable the PSCI platform compatibility layer
ENABLE_PLAT_COMPAT	:=	1

# Build flag to include AArch32 registers in cpu context save and restore during
# world switch. This flag must be set to 0 for AArch64-only platforms.
CTX_INCLUDE_AARCH32_REGS :=	0

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

# Enable ACAO support
ifeq (${MTK_ACAO_SUPPORT},yes)
$(eval $(call add_define,MTK_ACAO_SUPPORT))
endif

ifeq (${MTK_UFS_SUPPORT},yes)
$(eval $(call add_define,MTK_UFS_SUPPORT))
endif

ifeq (${MTK_EMMC_SUPPORT},yes)
$(eval $(call add_define,MTK_MSDC_HW_FDE))
endif

# Enable CM_MGR support
ifeq (${MTK_CM_MGR},yes)
$(eval $(call add_define,MTK_CM_MGR))
endif

ifeq (${CFG_PLAT_SRAM_FLAG},1)
$(eval $(call add_define,CFG_PLAT_SRAM_FLAG))
endif

ifeq (${MTK_FPGA_LDVT},yes)
$(eval $(call add_define,MTK_FPGA_LDVT))
endif

ifeq (${MTK_FPGA_EARLY_PORTING},yes)
$(eval $(call add_define,MTK_FPGA_EARLY_PORTING))
endif

$(eval $(call add_define,MTK_SIP_KERNEL_BOOT_ENABLE))

# Enable dynamic memory mapping
$(eval $(call add_define,PLAT_XLAT_TABLES_DYNAMIC))

ifeq ($(MTK_ENABLE_GENIEZONE),yes)
$(eval $(call add_define,MTK_ENABLE_GENIEZONE))
ERRATA_A76_1262888 := 1
endif

ifeq ($(CATB_ERRATA_SUPPORT),yes)
$(eval $(call add_define,CATB_ERRATA_SUPPORT))
ERRATA_A76_1257314 := 1
ERRATA_A76_1262606 := 1
ERRATA_A76_1406411 := 1
endif

ifeq (${MTK_ENABLE_MPU_HAL_SUPPORT},yes)
PLAT_BL_COMMON_SOURCES += ${BL31_MPU_SOURCE}
endif

ifeq ($(MTK_DEVMPU_SUPPORT),yes)
$(eval $(call add_define,MTK_DEVMPU_SUPPORT))
PLAT_INCLUDES += \
		-I${MTK_PLAT}/common/drivers/devmpu/ \
		-I${MTK_PLAT_SOC}/drivers/devmpu/
PLAT_BL_COMMON_SOURCES += \
		${MTK_PLAT}/common/drivers/devmpu/devmpu.c
endif

ifeq (${MTK_FPGA_LDVT},yes)
PLAT_BL_COMMON_SOURCES           +=      ${MTK_PLAT_SOC}/drivers/uart/uart.c
$(eval $(call add_define,MTK_FPGA_LDVT))
endif

ifeq (${MTK_DEBUGSYS_LOCK},yes)
$(eval $(call add_define,MTK_DEBUGSYS_LOCK))
endif

DYNAMIC_WORKAROUND_CVE_2018_3639    :=      1

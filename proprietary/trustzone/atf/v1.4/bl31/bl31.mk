#
# Copyright (c) 2013-2017, ARM Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

include lib/psci/psci_lib.mk

BL31_SOURCES		+=	bl31/bl31_main.c				\
				bl31/interrupt_mgmt.c				\
				bl31/aarch64/bl31_entrypoint.S			\
				bl31/aarch64/runtime_exceptions.S		\
				bl31/aarch64/crash_reporting.S			\
				bl31/bl31_context_mgmt.c			\
				common/runtime_svc.c				\
				services/arm_arch_svc/arm_arch_svc_setup.c \
				plat/common/aarch64/platform_mp_stack.S		\
				services/std_svc/std_svc_setup.c		\
				${PSCI_LIB_SOURCES}

ifeq (${ENABLE_PMF}, 1)
BL31_SOURCES		+=	lib/pmf/pmf_main.c
endif

MTK_PLATFORM_LC := $(shell echo ${PLAT} | tr A-Z a-z )
MTK_ATF_PLATFORM_FOLDER := plat
ifeq ($(DRAM_EXTENSION_SUPPORT), yes)
CHIP_LD_S := $(MTK_ATF_PLATFORM_FOLDER)/mediatek/common/bl31_dram_ext.ld.S
else
CHIP_LD_S := $(MTK_ATF_PLATFORM_FOLDER)/mediatek/common/bl31.ld.S
endif
ifneq ($(wildcard $(CHIP_LD_S)),)
BL31_LINKERFILE		:=	$(CHIP_LD_S)
else
BL31_LINKERFILE		:=	bl31/bl31.ld.S
endif

# Flag used to indicate if Crash reporting via console should be included
# in BL31. This defaults to being present in DEBUG builds only
ifndef CRASH_REPORTING
CRASH_REPORTING		:=	$(DEBUG)
endif

$(eval $(call assert_boolean,CRASH_REPORTING))
$(eval $(call add_define,CRASH_REPORTING))

#
# Copyright (c) 2013-2017, ARM Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

ifeq (${MTK_ENABLE_MPU_HAL_SUPPORT},yes)
$(eval $(call add_define,MTK_ENABLE_MPU_HAL_SUPPORT))
$(info include ${MTK_PLAT}/mpu_ctrl/rules.mk)
include ${MTK_PLAT}/common/mpu_ctrl/rules.mk
endif

BL31_SOURCES		+=	${MTK_PLAT}/common/mtk_sip_svc.c               \
                   ${MTK_PLAT}/common/bl31_fiq_handler.c          \
                   ${MTK_PLAT}/common/fiq_smp_call.c              \
                   ${MTK_PLAT}/common/mtk_aee_debug.c            \
                   ${MTK_PLAT}/common/log.c                       \
                   ${MTK_PLAT}/common/mtk_plat_common.c           \
                   ${MTK_PLAT}/common/mtk_rot.c                   \
                   ${MTK_PLAT}/common/mtk_rgu.c                   \
                   ${MTK_PLAT}/common/drivers/uart/8250_console.S \
                   ${MTK_PLAT}/common/mtk_helpers.S \

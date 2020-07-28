#
# Copyright (c) from 2019, ARM Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

CUR_MT_LPM_PATH=${MTK_PLAT}/common/lpm/

BL31_MT_LPM_COMMON_CFLAGS := -I${CUR_MT_LPM_PATH}/

BL31_MT_LPM_COMMON_SOURCE := \
	${CUR_MT_LPM_PATH}/mt_lp_rm.c \
	${CUR_MT_LPM_PATH}/mt_lpm_dispatch.c \
	${CUR_MT_LPM_PATH}/mt_lp_rq.c


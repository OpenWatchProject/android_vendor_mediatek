/*
 * Copyright (c) 2018 MediaTek Inc.
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files
 * (the "Software"), to deal in the Software without restriction,
 * including without limitation the rights to use, copy, modify, merge,
 * publish, distribute, sublicense, and/or sell copies of the Software,
 * and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
 * CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#ifndef __PLAT_MPU_DYNAMIC_H__
#define __PLAT_MPU_DYNAMIC_H__

/* clang-format off */
#include <mpu_ctrl/mpu_def.h>

#include <mpu_v1.h>
#if defined(MTK_DEVMPU_SUPPORT)
#include <devmpu.h>
#endif

/* Region does not have to be in order. */
struct mpu_feat_perm_entry mpu_feat_permission_table[] = {
	{
		.feat_id = MPU_FEAT_AMMS_POS_OWNER_AP,
		.perm = { .u_perm.emimpu = {
			.d15 = FORBIDDEN, .d14 = FORBIDDEN,
			.d13 = FORBIDDEN, .d12 = FORBIDDEN,
			.d11 = FORBIDDEN, .d10 = FORBIDDEN,
			.d9  = FORBIDDEN, .d8  = FORBIDDEN,
			.d7  = FORBIDDEN, .d6  = SEC_R_NSEC_RW,
			.d5  = FORBIDDEN, .d4  = SEC_R_NSEC_RW,
			.d3  = FORBIDDEN, .d2  = FORBIDDEN,
			.d1  = SEC_R_NSEC_R, .d0  = SEC_R_NSEC_RW,
		}, },
	},
	{
		.feat_id = MPU_FEAT_AMMS_POS_OWNER_MD,
		.perm = { .u_perm.emimpu = {
			.d15 = FORBIDDEN, .d14 = FORBIDDEN,
			.d13 = FORBIDDEN, .d12 = FORBIDDEN,
			.d11 = FORBIDDEN, .d10 = NO_PROTECTION,
			.d9  = FORBIDDEN, .d8  = FORBIDDEN,
			.d7  = NO_PROTECTION, .d6  = FORBIDDEN,
			.d5  = FORBIDDEN, .d4  = FORBIDDEN,
			.d3  = FORBIDDEN, .d2  = FORBIDDEN,
			.d1  = NO_PROTECTION, .d0  = SEC_R_NSEC_R,
		}, },
	},
	{
		.feat_id = MPU_FEAT_AMMS_STATIC_FREE_TO_AP,
		.perm = { .u_perm.emimpu = {
			.d15 = FORBIDDEN, .d14 = FORBIDDEN,
			.d13 = FORBIDDEN, .d12 = FORBIDDEN,
			.d11 = FORBIDDEN, .d10 = FORBIDDEN,
			.d9  = NO_PROTECTION, .d8  = NO_PROTECTION,
			.d7  = SEC_R_NSEC_R, .d6  = SEC_R_NSEC_RW,
			.d5  = FORBIDDEN, .d4  = NO_PROTECTION,
			.d3  = FORBIDDEN, .d2  = FORBIDDEN,
			.d1  = SEC_R_NSEC_R, .d0  = NO_PROTECTION,
		}, },
	},
	{
		.feat_id = MPU_FEAT_AMMS_STATIC_MD_READ_ONLY,
		.perm = { .u_perm.emimpu = {
			.d15 = FORBIDDEN, .d14 = FORBIDDEN,
			.d13 = FORBIDDEN, .d12 = FORBIDDEN,
			.d11 = FORBIDDEN, .d10 = FORBIDDEN,
			.d9  = FORBIDDEN, .d8  = FORBIDDEN,
			.d7  = SEC_R_NSEC_R, .d6  = FORBIDDEN,
			.d5  = FORBIDDEN, .d4  = FORBIDDEN,
			.d3  = FORBIDDEN, .d2  = FORBIDDEN,
			.d1  = SEC_R_NSEC_R, .d0  = FORBIDDEN,
		}, },
	},
	{
		.feat_id = MPU_FEAT_AMMS_STATIC_MD_READ_WRITE,
		.perm = { .u_perm.emimpu = {
			.d15 = FORBIDDEN, .d14 = FORBIDDEN,
			.d13 = FORBIDDEN, .d12 = FORBIDDEN,
			.d11 = FORBIDDEN, .d10 = FORBIDDEN,
			.d9  = FORBIDDEN, .d8  = FORBIDDEN,
			.d7  = SEC_R_NSEC_RW, .d6  = FORBIDDEN,
			.d5  = FORBIDDEN, .d4  = FORBIDDEN,
			.d3  = FORBIDDEN, .d2  = FORBIDDEN,
			.d1  = SEC_R_NSEC_RW, .d0  = SEC_R_NSEC_R,
		}, },
	},
	{
		.feat_id = MPU_FEAT_AMMS_STATIC_MD_DISABLE,
		.perm = { .u_perm.emimpu = {
			.d15 = FORBIDDEN, .d14 = FORBIDDEN,
			.d13 = FORBIDDEN, .d12 = FORBIDDEN,
			.d11 = FORBIDDEN, .d10 = FORBIDDEN,
			.d9  = FORBIDDEN, .d8  = FORBIDDEN,
			.d7  = NO_PROTECTION, .d6  = NO_PROTECTION,
			.d5  = NO_PROTECTION, .d4  = NO_PROTECTION,
			.d3  = NO_PROTECTION, .d2  = NO_PROTECTION,
			.d1  = NO_PROTECTION, .d0  = NO_PROTECTION,
		}, },
	},
	{
		.feat_id = MPU_FEAT_EMIMPU_ALL_NO_PROTECTION,
		.perm = { .u_perm.emimpu = {
			.d15 = NO_PROTECTION, .d14 = NO_PROTECTION,
			.d13 = NO_PROTECTION, .d12 = NO_PROTECTION,
			.d11 = NO_PROTECTION, .d10 = NO_PROTECTION,
			.d9  = NO_PROTECTION, .d8  = NO_PROTECTION,
			.d7  = NO_PROTECTION, .d6  = NO_PROTECTION,
			.d5  = NO_PROTECTION, .d4  = NO_PROTECTION,
			.d3  = NO_PROTECTION, .d2  = NO_PROTECTION,
			.d1  = NO_PROTECTION, .d0  = NO_PROTECTION,
		}, },
	},
#if defined(MTK_DEVMPU_SUPPORT)
	{
		.feat_id = MPU_FEAT_DEVMPU_ALL_NO_PROTECTION,
		.perm = { .u_perm.devmpu = {
			.d3_rd = DEVMPU_PERM_NS, .d2_rd = DEVMPU_PERM_NS,
			.d1_rd = DEVMPU_PERM_NS, .d0_rd = DEVMPU_PERM_NS,
			.d3_wr = DEVMPU_PERM_NS, .d2_wr = DEVMPU_PERM_NS,
			.d1_wr = DEVMPU_PERM_NS, .d0_wr = DEVMPU_PERM_NS,
		}, },
	},
#endif /* #if defined(MTK_DEVMPU_SUPPORT) */
};

#define MPU_FEATURE_ENTRY_SIZE ARRAY_SIZE(mpu_feat_permission_table)

/* clang-format on */

#endif  /* __PLAT_MPU_DYNAMIC_H__ */

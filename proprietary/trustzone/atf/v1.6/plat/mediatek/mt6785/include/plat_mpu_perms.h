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

#ifndef __PLAT_MPU_PERMS_H__
#define __PLAT_MPU_PERMS_H__

/* clang-format off */
#include <mpu_ctrl/mpu_def.h>

#include <mpu_v1.h>
#if defined(MTK_DEVMPU_SUPPORT)
#include <devmpu.h>
#endif

#define DOMAIN_ENTRY(ID, NAME) \
	{ .id = ID,  .name = NAME, }

struct mpu_domain_entry mpu_domain_table[] = {
	DOMAIN_ENTRY(MPU_DOMAIN_0,  "AP"),
	DOMAIN_ENTRY(MPU_DOMAIN_1,  "MD1"),
	DOMAIN_ENTRY(MPU_DOMAIN_2,  "CONN"),
	DOMAIN_ENTRY(MPU_DOMAIN_3,  "SCP"),
	DOMAIN_ENTRY(MPU_DOMAIN_4,  "MM"),
	DOMAIN_ENTRY(MPU_DOMAIN_5,  "RESERVE"),
	DOMAIN_ENTRY(MPU_DOMAIN_6,  "MFG"),
	DOMAIN_ENTRY(MPU_DOMAIN_7,  "MDHW"),
	DOMAIN_ENTRY(MPU_DOMAIN_8,  "SSPM"),
	DOMAIN_ENTRY(MPU_DOMAIN_9,  "SPM"),
	DOMAIN_ENTRY(MPU_DOMAIN_10, "ADSP"),
	DOMAIN_ENTRY(MPU_DOMAIN_11, "GZ"),
	DOMAIN_ENTRY(MPU_DOMAIN_12, "sMM"),
	DOMAIN_ENTRY(MPU_DOMAIN_13, "sAPU"),
	DOMAIN_ENTRY(MPU_DOMAIN_14, "RESERVE"),
	DOMAIN_ENTRY(MPU_DOMAIN_15, "RESERVE"),
};

struct mpu_domain_entry devmpu_domain_table[] = {
	DOMAIN_ENTRY(MPU_DOMAIN_0,  "AP"),
	DOMAIN_ENTRY(MPU_DOMAIN_1,  "GZ"),
	DOMAIN_ENTRY(MPU_DOMAIN_2,  "sMM/sAPU/MM"),
	DOMAIN_ENTRY(MPU_DOMAIN_3,  "UNIVERSE"),
};

/* Region does not have to be in order. */
struct mpu_perm_entry mpu_permission_table[] = {
	{
		.region_type = MPU_REGION_TYPE_EMIMPU,
		.unique_id = MPU_UNIQUE_ID_0,
		.set_perm = { .u_perm.emimpu = {
			.phy_region_id = 0,
			.d15 = FORBIDDEN, .d14 = FORBIDDEN,
			.d13 = FORBIDDEN, .d12 = SEC_RW,
			.d11 = FORBIDDEN, .d10 = FORBIDDEN,
			.d9  = FORBIDDEN, .d8  = FORBIDDEN,
			.d7  = FORBIDDEN, .d6  = FORBIDDEN,
			.d5  = FORBIDDEN, .d4  = SEC_RW,
			.d3  = FORBIDDEN, .d2  = FORBIDDEN,
			.d1  = FORBIDDEN, .d0  = SEC_RW,
		}, },
		.allowed_origins = MPU_REQ_ORIGIN_ATF,
		.flags = MPU_FLAGS_LOCK_AFTER_SET,
		.active = true,
		.name = "SECURE_OS",
	},
	{
		.region_type = MPU_REGION_TYPE_EMIMPU,
		.unique_id = MPU_UNIQUE_ID_1,
		.set_perm = { .u_perm.emimpu = {
			.phy_region_id = 1,
			.d15 = FORBIDDEN, .d14 = FORBIDDEN,
			.d13 = FORBIDDEN, .d12 = SEC_RW,
			.d11 = FORBIDDEN, .d10 = FORBIDDEN,
			.d9  = FORBIDDEN, .d8  = FORBIDDEN,
			.d7  = FORBIDDEN, .d6  = FORBIDDEN,
			.d5  = FORBIDDEN, .d4  = SEC_RW,
			.d3  = FORBIDDEN, .d2  = FORBIDDEN,
			.d1  = FORBIDDEN, .d0  = SEC_RW,
		}, },
		.allowed_origins = MPU_REQ_ORIGIN_TEE,
		.flags = MPU_FLAGS_NONE,
		.active = true,
		.name = "SECURE_MEM",
	},
	{
		.region_type = MPU_REGION_TYPE_EMIMPU,
		.unique_id = MPU_UNIQUE_ID_2,
		.set_perm = { .u_perm.emimpu = {
			.phy_region_id = 2,
			.d15 = FORBIDDEN, .d14 = FORBIDDEN,
			.d13 = FORBIDDEN, .d12 = FORBIDDEN,
			.d11 = FORBIDDEN, .d10 = FORBIDDEN,
			.d9  = FORBIDDEN, .d8  = FORBIDDEN,
			.d7  = FORBIDDEN, .d6  = FORBIDDEN,
			.d5  = FORBIDDEN, .d4  = SEC_RW,
			.d3  = FORBIDDEN, .d2  = FORBIDDEN,
			.d1  = FORBIDDEN, .d0  = SEC_RW,
		}, },
		.allowed_origins = MPU_REQ_ORIGIN_TEE,
		.flags = MPU_FLAGS_NONE,
		.active = true,
		.name = "TRUSTED_UI",
	},
	{
		.region_type = MPU_REGION_TYPE_EMIMPU,
		.unique_id = MPU_UNIQUE_ID_3,
		.set_perm = { .u_perm.emimpu = {
			.phy_region_id = 2,
			.d15 = FORBIDDEN, .d14 = FORBIDDEN,
			.d13 = FORBIDDEN, .d12 = FORBIDDEN,
			.d11 = FORBIDDEN, .d10 = FORBIDDEN,
			.d9  = FORBIDDEN, .d8  = FORBIDDEN,
			.d7  = FORBIDDEN, .d6  = FORBIDDEN,
			.d5  = FORBIDDEN, .d4  = SEC_RW,
			.d3  = FORBIDDEN, .d2  = FORBIDDEN,
			.d1  = FORBIDDEN, .d0  = SEC_RW,
		}, },
		.allowed_origins = MPU_REQ_ORIGIN_TEE,
		.flags = MPU_FLAGS_NONE,
		.active = true,
		.name = "SECURE_WFD",
	},
	{
		.region_type = MPU_REGION_TYPE_EMIMPU,
		.unique_id = MPU_UNIQUE_ID_4,
		.set_perm = { .u_perm.emimpu = {
			.phy_region_id = 2,
			.d15 = FORBIDDEN, .d14 = FORBIDDEN,
			.d13 = FORBIDDEN, .d12 = SEC_RW,
			.d11 = SEC_R_NSEC_RW, .d10 = FORBIDDEN,
			.d9  = FORBIDDEN, .d8  = FORBIDDEN,
			.d7  = FORBIDDEN, .d6  = FORBIDDEN,
			.d5  = FORBIDDEN, .d4  = SEC_RW,
			.d3  = FORBIDDEN, .d2  = FORBIDDEN,
			.d1  = FORBIDDEN, .d0  = SEC_RW,
		}, },
		.allowed_origins = MPU_REQ_ORIGIN_GZ,
		.flags = MPU_FLAGS_NONE,
		.active = true,
		.name = "PROT_SHARED",
	},
	{
		.region_type = MPU_REGION_TYPE_EMIMPU,
		.unique_id = MPU_UNIQUE_ID_5,
		.set_perm = { .u_perm.emimpu = {
			.phy_region_id = 3,
			.d15 = FORBIDDEN,     .d14 = FORBIDDEN,
			.d13 = FORBIDDEN,     .d12 = FORBIDDEN,
			.d11 = NO_PROTECTION, .d10 = FORBIDDEN,
			.d9  = FORBIDDEN,     .d8  = FORBIDDEN,
			.d7  = FORBIDDEN,     .d6  = FORBIDDEN,
			.d5  = FORBIDDEN,     .d4  = FORBIDDEN,
			.d3  = FORBIDDEN,     .d2  = FORBIDDEN,
			.d1  = NO_PROTECTION, .d0  = FORBIDDEN,
		}, },
		.allowed_origins = MPU_REQ_ORIGIN_GZ,
		.flags = MPU_FLAGS_LOCK_AFTER_SET,
		.active = true,
		.name = "MD_SHARED",
	},
	{
		.region_type = MPU_REGION_TYPE_EMIMPU,
		.unique_id = MPU_UNIQUE_ID_6,
		.set_perm = { .u_perm.emimpu = {
			.phy_region_id = 8,
		}, },
		.allowed_origins = MPU_REQ_ORIGIN_ATF,
		.flags = MPU_FLAGS_ALLOW_DOMAIN_PERMS_CHANGE_DYN,
		.active = true,
		.name = "AMMS_DYN_STATIC",
	},
	{
		.region_type = MPU_REGION_TYPE_EMIMPU,
		.unique_id = MPU_UNIQUE_ID_7,
		.set_perm = { .u_perm.emimpu = {
			.phy_region_id = 9,
		}, },
		.allowed_origins = MPU_REQ_ORIGIN_ATF,
		.flags = MPU_FLAGS_ALLOW_DOMAIN_PERMS_CHANGE_DYN,
		.active = true,
		.name = "AMMS_DYN_POS",
	},
#if defined(MTK_DEVMPU_SUPPORT)
	{
		.region_type = MPU_REGION_TYPE_DEVMPU,
		.unique_id = MPU_UNIQUE_ID_50,
		.set_perm = { .u_perm.devmpu = {
			.d3_rd = DEVMPU_PERM_BLOCK, .d2_rd = DEVMPU_PERM_BLOCK,
			.d1_rd = DEVMPU_PERM_BLOCK, .d0_rd = DEVMPU_PERM_S,
			.d3_wr = DEVMPU_PERM_BLOCK, .d2_wr = DEVMPU_PERM_BLOCK,
			.d1_wr = DEVMPU_PERM_BLOCK, .d0_wr = DEVMPU_PERM_S,
		}, },
		.allowed_origins = MPU_REQ_ORIGIN_ATF,
		.flags = MPU_FLAGS_LOCK_AFTER_SET,
		.active = true,
		.name = "ATF",
	},
	{
		.region_type = MPU_REGION_TYPE_DEVMPU,
		.unique_id = MPU_UNIQUE_ID_51,
		.set_perm = { .u_perm.devmpu = {
			.d3_rd = DEVMPU_PERM_BLOCK, .d2_rd = DEVMPU_PERM_BLOCK,
			.d1_rd = DEVMPU_PERM_NS,    .d0_rd = DEVMPU_PERM_BLOCK,
			.d3_wr = DEVMPU_PERM_BLOCK, .d2_wr = DEVMPU_PERM_BLOCK,
			.d1_wr = DEVMPU_PERM_NS,    .d0_wr = DEVMPU_PERM_BLOCK,
		}, },
		.allowed_origins = MPU_REQ_ORIGIN_ATF,
		.flags = MPU_FLAGS_LOCK_AFTER_SET,
		.active = true,
		.name = "GZ",
	},
	{
		.region_type = MPU_REGION_TYPE_DEVMPU,
		.unique_id = MPU_UNIQUE_ID_52,
		.set_perm = { .u_perm.devmpu = {
			.d3_rd = DEVMPU_PERM_BLOCK, .d2_rd = DEVMPU_PERM_S,
			.d1_rd = DEVMPU_PERM_NS,    .d0_rd = DEVMPU_PERM_BLOCK,
			.d3_wr = DEVMPU_PERM_BLOCK, .d2_wr = DEVMPU_PERM_S,
			.d1_wr = DEVMPU_PERM_NS,    .d0_wr = DEVMPU_PERM_BLOCK,
		}, },
		.allowed_origins = MPU_REQ_ORIGIN_GZ,
		.flags = MPU_FLAGS_LOCK_AFTER_SET,
		.active = true,
		.name = "SDSP_FIRMWARE",
	},
	{
		.region_type = MPU_REGION_TYPE_DEVMPU,
		.unique_id = MPU_UNIQUE_ID_53,
		.set_perm = { .u_perm.devmpu = {
			.d3_rd = DEVMPU_PERM_BLOCK, .d2_rd = DEVMPU_PERM_BLOCK,
			.d1_rd = DEVMPU_PERM_NS,    .d0_rd = DEVMPU_PERM_S,
			.d3_wr = DEVMPU_PERM_BLOCK, .d2_wr = DEVMPU_PERM_BLOCK,
			.d1_wr = DEVMPU_PERM_NS,    .d0_wr = DEVMPU_PERM_S,
		}, },
		.allowed_origins = MPU_REQ_ORIGIN_GZ,
		.flags = MPU_FLAGS_NONE,
		.active = true,
		.name = "MTEE_TEE_SHARED",
	},
	{
		.region_type = MPU_REGION_TYPE_DEVMPU,
		.unique_id = MPU_UNIQUE_ID_54,
		.set_perm = { .u_perm.devmpu = {
			.d3_rd = DEVMPU_PERM_BLOCK, .d2_rd = DEVMPU_PERM_S,
			.d1_rd = DEVMPU_PERM_NS,    .d0_rd = DEVMPU_PERM_S,
			.d3_wr = DEVMPU_PERM_BLOCK, .d2_wr = DEVMPU_PERM_S,
			.d1_wr = DEVMPU_PERM_NS,    .d0_wr = DEVMPU_PERM_S,
		}, },
		.allowed_origins = MPU_REQ_ORIGIN_GZ,
		.flags = MPU_FLAGS_NONE,
		.active = true,
		.name = "VPU_MTEE_TEE_S",
	},
	{
		.region_type = MPU_REGION_TYPE_DEVMPU,
		.unique_id = MPU_UNIQUE_ID_55,
		.set_perm = { .u_perm.devmpu = {
			.d3_rd = DEVMPU_PERM_BLOCK, .d2_rd = DEVMPU_PERM_S,
			.d1_rd = DEVMPU_PERM_BLOCK, .d0_rd = DEVMPU_PERM_S,
			.d3_wr = DEVMPU_PERM_BLOCK, .d2_wr = DEVMPU_PERM_S,
			.d1_wr = DEVMPU_PERM_BLOCK, .d0_wr = DEVMPU_PERM_S,
		}, },
		.allowed_origins = MPU_REQ_ORIGIN_TEE,
		.flags = MPU_FLAGS_NONE,
		.active = true,
		.name = "VPU_TEE_SHARED",
	},
	{
		.region_type = MPU_REGION_TYPE_DEVMPU,
		.unique_id = MPU_UNIQUE_ID_56,
		.set_perm = { .u_perm.devmpu = {
			.d3_rd = DEVMPU_PERM_BLOCK, .d2_rd = DEVMPU_PERM_S,
			.d1_rd = DEVMPU_PERM_BLOCK, .d0_rd = DEVMPU_PERM_S,
			.d3_wr = DEVMPU_PERM_BLOCK, .d2_wr = DEVMPU_PERM_S,
			.d1_wr = DEVMPU_PERM_BLOCK, .d0_wr = DEVMPU_PERM_S,
		}, },
		.allowed_origins = MPU_REQ_ORIGIN_ATF,
		.flags = MPU_FLAGS_LOCK_AFTER_SET,
		.active = true,
		.name = "MTEE_TEE_SHARED",
	},
	{
		.region_type = MPU_REGION_TYPE_DEVMPU,
		.unique_id = MPU_UNIQUE_ID_57,
		.set_perm = { .u_perm.devmpu = {
			.d3_rd = DEVMPU_PERM_BLOCK, .d2_rd = DEVMPU_PERM_BLOCK,
			.d1_rd = DEVMPU_PERM_NS,    .d0_rd = DEVMPU_PERM_S,
			.d3_wr = DEVMPU_PERM_BLOCK, .d2_wr = DEVMPU_PERM_BLOCK,
			.d1_wr = DEVMPU_PERM_NS,    .d0_wr = DEVMPU_PERM_S,
		}, },
		.allowed_origins = MPU_REQ_ORIGIN_ATF,
		.flags = MPU_FLAGS_LOCK_AFTER_SET,
		.active = true,
		.name = "MTEE_TEE_STATIC",
	},
	{
		.region_type = MPU_REGION_TYPE_DEVMPU,
		.unique_id = MPU_UNIQUE_ID_58,
		.set_perm = { .u_perm.devmpu = {
			.d3_rd = DEVMPU_PERM_BLOCK, .d2_rd = DEVMPU_PERM_BLOCK,
			.d1_rd = DEVMPU_PERM_NS,    .d0_rd = DEVMPU_PERM_BLOCK,
			.d3_wr = DEVMPU_PERM_BLOCK, .d2_wr = DEVMPU_PERM_BLOCK,
			.d1_wr = DEVMPU_PERM_NS,    .d0_wr = DEVMPU_PERM_BLOCK,
		}, },
		.allowed_origins = MPU_REQ_ORIGIN_GZ,
		.flags = MPU_FLAGS_NONE,
		.active = true,
		.name = "HAPP_EXTRA",
	},
#endif /* #if defined(MTK_DEVMPU_SUPPORT) */
};

#define MPU_DOMAIN_ENTRY_SIZE ARRAY_SIZE(mpu_domain_table)
#define DEVMPU_DOMAIN_ENTRY_SIZE ARRAY_SIZE(devmpu_domain_table)
#define MPU_PERMISSION_ENTRY_SIZE ARRAY_SIZE(mpu_permission_table)
/* clang-format on */

#endif  /* __PLAT_MPU_PERMS_H__ */

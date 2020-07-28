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
	DOMAIN_ENTRY(MPU_DOMAIN_10, "sMM"),
	DOMAIN_ENTRY(MPU_DOMAIN_11, "GZ"),
	DOMAIN_ENTRY(MPU_DOMAIN_12, "RESERVE"),
	DOMAIN_ENTRY(MPU_DOMAIN_13, "sAPU"),
	DOMAIN_ENTRY(MPU_DOMAIN_14, "RESERVE"),
	DOMAIN_ENTRY(MPU_DOMAIN_15, "RESERVE"),
};

/* Region does not have to be in order. */
struct mpu_perm_entry mpu_permission_table[] = {
	{
		.region_type = MPU_REGION_TYPE_EMIMPU,
		.unique_id = MPU_UNIQUE_ID_0,
		.set_perm = { .u_perm.emimpu = {
			.phy_region_id = 2,
			.d15 = FORBIDDEN, .d14 = FORBIDDEN,
			.d13 = FORBIDDEN, .d12 = FORBIDDEN,
			.d11 = FORBIDDEN, .d10 = SEC_RW,
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
		.unique_id = MPU_UNIQUE_ID_1,
		.set_perm = { .u_perm.emimpu = {
			.phy_region_id = 7,
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
		.unique_id = MPU_UNIQUE_ID_2,
		.set_perm = { .u_perm.emimpu = {
			.phy_region_id = 7,
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
		.unique_id = MPU_UNIQUE_ID_3,
		.set_perm = { .u_perm.emimpu = {
			.phy_region_id = 7,
			.d15 = FORBIDDEN,     .d14 = FORBIDDEN,
			.d13 = FORBIDDEN,     .d12 = FORBIDDEN,
			.d11 = SEC_R_NSEC_RW, .d10 = SEC_RW,
			.d9  = FORBIDDEN,     .d8  = FORBIDDEN,
			.d7  = FORBIDDEN,     .d6  = FORBIDDEN,
			.d5  = FORBIDDEN,     .d4  = SEC_RW,
			.d3  = FORBIDDEN,     .d2  = FORBIDDEN,
			.d1  = FORBIDDEN,     .d0  = SEC_RW,
		}, },
		.allowed_origins = MPU_REQ_ORIGIN_GZ,
		.flags = MPU_FLAGS_NONE,
		.active = true,
		.name = "PROT_SHARED",
	},
	{
		.region_type = MPU_REGION_TYPE_EMIMPU,
		.unique_id = MPU_UNIQUE_ID_4,
		.set_perm = { .u_perm.emimpu = {
			.phy_region_id = 10,
			.d15 = FORBIDDEN, .d14 = FORBIDDEN,
			.d13 = SEC_RW, .d12 = FORBIDDEN,
			.d11 = SEC_RW_NSEC_R, .d10 = FORBIDDEN,
			.d9  = FORBIDDEN, .d8  = FORBIDDEN,
			.d7  = FORBIDDEN, .d6  = FORBIDDEN,
			.d5  = FORBIDDEN, .d4  = FORBIDDEN,
			.d3  = FORBIDDEN, .d2  = FORBIDDEN,
			.d1  = FORBIDDEN, .d0  = FORBIDDEN,
		}, },
		.allowed_origins = MPU_REQ_ORIGIN_GZ,
		.flags = MPU_FLAGS_LOCK_AFTER_SET,
		.active = true,
		.name = "SDSP_FIRMWARE",
	},
	{
		.region_type = MPU_REGION_TYPE_EMIMPU,
		.unique_id = MPU_UNIQUE_ID_5,
		.set_perm = { .u_perm.emimpu = {
			.phy_region_id = 29,
			.d15 = FORBIDDEN,     .d14 = FORBIDDEN,
			.d13 = FORBIDDEN,     .d12 = FORBIDDEN,
			.d11 = SEC_R_NSEC_RW, .d10 = FORBIDDEN,
			.d9  = FORBIDDEN,     .d8  = FORBIDDEN,
			.d7  = FORBIDDEN,     .d6  = FORBIDDEN,
			.d5  = FORBIDDEN,     .d4  = FORBIDDEN,
			.d3  = FORBIDDEN,     .d2  = FORBIDDEN,
			.d1  = FORBIDDEN,     .d0  = SEC_RW,
		}, },
		.allowed_origins = MPU_REQ_ORIGIN_GZ,
		.flags = MPU_FLAGS_NONE,
		.active = true,
		.name = "MTEE_TEE_SHARED",
	},
	{
		.region_type = MPU_REGION_TYPE_EMIMPU,
		.unique_id = MPU_UNIQUE_ID_6,
		.set_perm = { .u_perm.emimpu = {
			.phy_region_id = 22,
			.d15 = FORBIDDEN,     .d14 = FORBIDDEN,
			.d13 = FORBIDDEN,     .d12 = FORBIDDEN,
			.d11 = SEC_R_NSEC_RW, .d10 = FORBIDDEN,
			.d9  = FORBIDDEN,     .d8  = FORBIDDEN,
			.d7  = FORBIDDEN,     .d6  = FORBIDDEN,
			.d5  = FORBIDDEN,     .d4  = FORBIDDEN,
			.d3  = FORBIDDEN,     .d2  = FORBIDDEN,
			.d1  = SEC_R_NSEC_RW, .d0  = FORBIDDEN,
		}, },
		.allowed_origins = MPU_REQ_ORIGIN_GZ,
		.flags = MPU_FLAGS_LOCK_AFTER_SET,
		.active = true,
		.name = "MD_SHARED",
	},
	{
		.region_type = MPU_REGION_TYPE_EMIMPU,
		.unique_id = MPU_UNIQUE_ID_7,
		.set_perm = { .u_perm.emimpu = {
			.phy_region_id = 1,
			.d15 = FORBIDDEN, .d14 = FORBIDDEN,
			.d13 = FORBIDDEN, .d12 = FORBIDDEN,
			.d11 = FORBIDDEN, .d10 = FORBIDDEN,
			.d9  = FORBIDDEN, .d8  = FORBIDDEN,
			.d7  = FORBIDDEN, .d6  = FORBIDDEN,
			.d5  = FORBIDDEN, .d4  = FORBIDDEN,
			.d3  = FORBIDDEN, .d2  = FORBIDDEN,
			.d1  = FORBIDDEN, .d0  = SEC_RW,
		}, },
		.allowed_origins = MPU_REQ_ORIGIN_ATF,
		.flags = MPU_FLAGS_LOCK_AFTER_SET,
		.active = true,
		.name = "ATF",
	},
	{
		.region_type = MPU_REGION_TYPE_EMIMPU,
		.unique_id = MPU_UNIQUE_ID_8,
		.set_perm = { .u_perm.emimpu = {
			.phy_region_id = 0,
			.d15 = FORBIDDEN, .d14 = FORBIDDEN,
			.d13 = FORBIDDEN, .d12 = FORBIDDEN,
			.d11 = FORBIDDEN, .d10 = SEC_RW,
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
		.unique_id = MPU_UNIQUE_ID_9,
		.set_perm = { .u_perm.emimpu = {
			.phy_region_id = 9,
			.d15 = FORBIDDEN,     .d14 = FORBIDDEN,
			.d13 = FORBIDDEN,     .d12 = FORBIDDEN,
			.d11 = SEC_R_NSEC_RW, .d10 = FORBIDDEN,
			.d9  = FORBIDDEN,     .d8  = FORBIDDEN,
			.d7  = FORBIDDEN,     .d6  = FORBIDDEN,
			.d5  = FORBIDDEN,     .d4  = FORBIDDEN,
			.d3  = FORBIDDEN,     .d2  = FORBIDDEN,
			.d1  = FORBIDDEN,     .d0  = FORBIDDEN,
		}, },
		.allowed_origins = MPU_REQ_ORIGIN_ATF,
		.flags = MPU_FLAGS_LOCK_AFTER_SET,
		.active = true,
		.name = "GZ",
	},
	{
		.region_type = MPU_REGION_TYPE_EMIMPU,
		.unique_id = MPU_UNIQUE_ID_10,
		.set_perm = { .u_perm.emimpu = {
			.phy_region_id = 29,
			.d15 = FORBIDDEN,     .d14 = FORBIDDEN,
			.d13 = SEC_RW,        .d12 = FORBIDDEN,
			.d11 = SEC_R_NSEC_RW, .d10 = FORBIDDEN,
			.d9  = FORBIDDEN,     .d8  = FORBIDDEN,
			.d7  = FORBIDDEN,     .d6  = FORBIDDEN,
			.d5  = FORBIDDEN,     .d4  = FORBIDDEN,
			.d3  = FORBIDDEN,     .d2  = FORBIDDEN,
			.d1  = FORBIDDEN,     .d0  = SEC_RW,
		}, },
		.allowed_origins = MPU_REQ_ORIGIN_GZ,
		.flags = MPU_FLAGS_NONE,
		.active = true,
		.name = "VPU_MTEE_TEE_S",
	},
	{
		.region_type = MPU_REGION_TYPE_EMIMPU,
		.unique_id = MPU_UNIQUE_ID_11,
		.set_perm = { .u_perm.emimpu = {
			.phy_region_id = 29,
			.d15 = FORBIDDEN, .d14 = FORBIDDEN,
			.d13 = SEC_RW,    .d12 = FORBIDDEN,
			.d11 = FORBIDDEN, .d10 = FORBIDDEN,
			.d9  = FORBIDDEN, .d8  = FORBIDDEN,
			.d7  = FORBIDDEN, .d6  = FORBIDDEN,
			.d5  = FORBIDDEN, .d4  = FORBIDDEN,
			.d3  = FORBIDDEN, .d2  = FORBIDDEN,
			.d1  = FORBIDDEN, .d0  = SEC_RW,
		}, },
		.allowed_origins = MPU_REQ_ORIGIN_TEE,
		.flags = MPU_FLAGS_NONE,
		.active = true,
		.name = "VPU_TEE_SHARED",
	},
	{
		.region_type = MPU_REGION_TYPE_EMIMPU,
		.unique_id = MPU_UNIQUE_ID_12,
		.set_perm = { .u_perm.emimpu = {
			.phy_region_id = 29,
			.d15 = FORBIDDEN,     .d14 = FORBIDDEN,
			.d13 = SEC_RW,        .d12 = FORBIDDEN,
			.d11 = FORBIDDEN,     .d10 = FORBIDDEN,
			.d9  = FORBIDDEN,     .d8  = FORBIDDEN,
			.d7  = FORBIDDEN,     .d6  = FORBIDDEN,
			.d5  = FORBIDDEN,     .d4  = FORBIDDEN,
			.d3  = FORBIDDEN,     .d2  = FORBIDDEN,
			.d1  = FORBIDDEN,     .d0  = SEC_RW,
		}, },
		.allowed_origins = MPU_REQ_ORIGIN_ATF,
		.flags = MPU_FLAGS_LOCK_AFTER_SET,
		.active = true,
		.name = "MTEE_TEE_SHARED",
	},
};

#define MPU_DOMAIN_ENTRY_SIZE ARRAY_SIZE(mpu_domain_table)
#define MPU_PERMISSION_ENTRY_SIZE ARRAY_SIZE(mpu_permission_table)
/* clang-format on */

#endif  /* __PLAT_MPU_PERMS_H__ */

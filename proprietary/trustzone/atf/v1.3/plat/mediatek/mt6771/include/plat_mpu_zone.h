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

#ifndef __PLAT_MPU_ZONE_H__
#define __PLAT_MPU_ZONE_H__

/* clang-format off */
#include <mpu_ctrl/mpu_def.h>

/* Zone does not have to be in order. */
struct mpu_origin_zone_entry mpu_origin_zone_gz_mappings[] = {
	{
		.zone_id = MPU_REQ_ORIGIN_GZ_ZONE_PROT_SHARED,
		.unique_id = MPU_UNIQUE_ID_3,
		.active = true,
		.name = "PROT_SHARED",
	},
	{
		.zone_id = MPU_REQ_ORIGIN_GZ_ZONE_HAPP,
		.unique_id = MPU_UNIQUE_ID_INVALID,
		.active = false,
		.name = "HAPP",
	},
	{
		.zone_id = MPU_REQ_ORIGIN_GZ_ZONE_HAPP_EXTRA,
		.unique_id = MPU_UNIQUE_ID_INVALID,
		.active = false,
		.name = "HAPP_EXTRA",
	},
	{
		.zone_id = MPU_REQ_ORIGIN_GZ_ZONE_SDSP,
		.unique_id = MPU_UNIQUE_ID_4,
		.active = true,
		.name = "SDSP_FIRMWARE",
	},
	{
		.zone_id = MPU_REQ_ORIGIN_GZ_ZONE_SDSP_SHARED_MTEE_TEE,
		.unique_id = MPU_UNIQUE_ID_5,
		.active = true,
		.name = "MTEE_TEE_SHARED",
	},
	{
		.zone_id = MPU_REQ_ORIGIN_GZ_ZONE_MD_SHARED,
		.unique_id = MPU_UNIQUE_ID_6,
		.active = true,
		.name = "MD_SHARED",
	},
	{
		.zone_id = MPU_REQ_ORIGIN_GZ_ZONE_SDSP_SHARED_VPU_MTEE_TEE,
		.unique_id = MPU_UNIQUE_ID_10,
		.active = true,
		.name = "VPU_MTEE_TEE_S",
	},
};

/* Zone does not have to be in order. */
struct mpu_origin_zone_entry mpu_origin_zone_tee_mappings[] = {
	{
		.zone_id = MPU_REQ_ORIGIN_TEE_ZONE_SVP,
		.unique_id = MPU_UNIQUE_ID_0,
		.active = true,
		.name = "SVP",
	},
	{
		.zone_id = MPU_REQ_ORIGIN_TEE_ZONE_TUI,
		.unique_id = MPU_UNIQUE_ID_1,
		.active = true,
		.name = "TUI",
	},
	{
		.zone_id = MPU_REQ_ORIGIN_TEE_ZONE_WFD,
		.unique_id = MPU_UNIQUE_ID_2,
		.active = true,
		.name = "WFD",
	},
	{
		.zone_id = MPU_REQ_ORIGIN_TEE_ZONE_SDSP_SHARED_VPU_TEE,
		.unique_id = MPU_UNIQUE_ID_11,
		.active = true,
		.name = "SDSP_VPU_TEE",
	},
};

/* Zone does not have to be in order. */
struct mpu_origin_zone_entry mpu_origin_zone_atf_mappings[] = {
	{
		.zone_id = MPU_REQ_ORIGIN_ATF_ZONE_AMMS_STATIC,
		.unique_id = MPU_UNIQUE_ID_INVALID,
		.active = false,
		.name = "AMMS_STATIC",
	},
	{
		.zone_id = MPU_REQ_ORIGIN_ATF_ZONE_AMMS_POS,
		.unique_id = MPU_UNIQUE_ID_INVALID,
		.active = false,
		.name = "AMMS_POS",
	},
	{
		.zone_id = MPU_REQ_ORIGIN_ATF_ZONE_ATF_EMIMPU,
		.unique_id = MPU_UNIQUE_ID_7,
		.active = true,
		.name = "ATF",
	},
	{
		.zone_id = MPU_REQ_ORIGIN_ATF_ZONE_TEE,
		.unique_id = MPU_UNIQUE_ID_8,
		.active = true,
		.name = "TEE",
	},
	{
		.zone_id = MPU_REQ_ORIGIN_ATF_ZONE_GZ,
		.unique_id = MPU_UNIQUE_ID_9,
		.active = true,
		.name = "GZ",
	},
	{
		.zone_id = MPU_REQ_ORIGIN_ATF_ZONE_SDSP_SHARED_VPU_TEE,
		.unique_id = MPU_UNIQUE_ID_12,
		.active = true,
		.name = "MTEE_TEE_SHARED",
	},
	{
		.zone_id = MPU_REQ_ORIGIN_ATF_ZONE_MTEE_TEE_STATIC_SHARED,
		.unique_id = MPU_UNIQUE_ID_INVALID,
		.active = false,
		.name = "MTEE_TEE_STATIC",
	},
};

#define MPU_ORIGIN_ZONE_GZ_ENTRY_SIZE ARRAY_SIZE(mpu_origin_zone_gz_mappings)
#define MPU_ORIGIN_ZONE_TEE_ENTRY_SIZE ARRAY_SIZE(mpu_origin_zone_tee_mappings)
#define MPU_ORIGIN_ZONE_ATF_ENTRY_SIZE ARRAY_SIZE(mpu_origin_zone_atf_mappings)
/* clang-format on */

#endif  /* __PLAT_MPU_ZONE_H__ */

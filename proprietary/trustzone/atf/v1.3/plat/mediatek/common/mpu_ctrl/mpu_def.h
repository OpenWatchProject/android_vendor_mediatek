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

#ifndef __MPU_DEF_H__
#define __MPU_DEF_H__

#include <stdint.h>
#include <utils.h>

#ifdef HAVE_STDBOOL_H
#include <stdbool.h>
#else
#define false 0
#define true 1
#define bool uint32_t
#endif

enum MPU_REGION_TYPE {
	MPU_REGION_TYPE_EMIMPU,
	MPU_REGION_TYPE_DEVMPU,

	MPU_REGION_TYPE_MAX = 0x7FFFFFFF,
};

enum MPU_REQ_ORIGIN {
	MPU_REQ_ORIGIN_LK = 0x1,
	MPU_REQ_ORIGIN_GZ = 0x2,
	MPU_REQ_ORIGIN_TEE = 0x4,
	MPU_REQ_ORIGIN_LINUX = 0x8,
	MPU_REQ_ORIGIN_ATF = 0x10,

	MPU_REQ_ORIGIN_INVALID = 0x7FFFFFFF,
};

enum MPU_REQ_ORIGIN_ZONE_ID {
	/* MPU Request ORIGIN Zone - GZ Specific */
	MPU_REQ_ORIGIN_GZ_ZONE_PROT_SHARED = 0,
	MPU_REQ_ORIGIN_GZ_ZONE_HAPP = 1,
	MPU_REQ_ORIGIN_GZ_ZONE_HAPP_EXTRA = 2,
	MPU_REQ_ORIGIN_GZ_ZONE_SDSP = 3,
	MPU_REQ_ORIGIN_GZ_ZONE_SDSP_SHARED_MTEE_TEE = 4,
	MPU_REQ_ORIGIN_GZ_ZONE_SDSP_SHARED_VPU_MTEE_TEE = 5,
	MPU_REQ_ORIGIN_GZ_ZONE_MD_SHARED = 6,
	MPU_REQ_ORIGIN_GZ_ZONE_MAX = 7,

	/* MPU Request ORIGIN Zone - TEE Specific */
	MPU_REQ_ORIGIN_TEE_ZONE_SVP = 0,
	MPU_REQ_ORIGIN_TEE_ZONE_TUI = 1,
	MPU_REQ_ORIGIN_TEE_ZONE_WFD = 2,
	MPU_REQ_ORIGIN_TEE_ZONE_SDSP_SHARED_VPU_TEE = 3,
	MPU_REQ_ORIGIN_TEE_ZONE_MAX = 4,

	/* MPU Request ORIGIN Zone - ATF Specific */
	MPU_REQ_ORIGIN_ATF_ZONE_AMMS_STATIC = 0,
	MPU_REQ_ORIGIN_ATF_ZONE_AMMS_POS = 1,
	MPU_REQ_ORIGIN_ATF_ZONE_ATF_EMIMPU = 2,
	MPU_REQ_ORIGIN_ATF_ZONE_ATF_DEVMPU = 3,
	MPU_REQ_ORIGIN_ATF_ZONE_TEE = 4,
	MPU_REQ_ORIGIN_ATF_ZONE_GZ = 5,
	MPU_REQ_ORIGIN_ATF_ZONE_SDSP_SHARED_VPU_TEE = 6,
	MPU_REQ_ORIGIN_ATF_ZONE_MTEE_TEE_STATIC_SHARED = 7,
	MPU_REQ_ORIGIN_ATF_ZONE_MAX = 8,

	MPU_REQ_ORIGIN_ZONE_INVALID = 0x7FFFFFFF
};

enum MPU_UNIQUE_ID {
	MPU_UNIQUE_ID_0 = 0,
	MPU_UNIQUE_ID_1,
	MPU_UNIQUE_ID_2,
	MPU_UNIQUE_ID_3,
	MPU_UNIQUE_ID_4,
	MPU_UNIQUE_ID_5,
	MPU_UNIQUE_ID_6,
	MPU_UNIQUE_ID_7,
	MPU_UNIQUE_ID_8,
	MPU_UNIQUE_ID_9,
	MPU_UNIQUE_ID_10,
	MPU_UNIQUE_ID_11,
	MPU_UNIQUE_ID_12,
	MPU_UNIQUE_ID_13,
	MPU_UNIQUE_ID_14,
	MPU_UNIQUE_ID_15,
	MPU_UNIQUE_ID_16,
	MPU_UNIQUE_ID_17,
	MPU_UNIQUE_ID_18,
	MPU_UNIQUE_ID_19,
	MPU_UNIQUE_ID_20,
	MPU_UNIQUE_ID_21,
	MPU_UNIQUE_ID_22,
	MPU_UNIQUE_ID_23,
	MPU_UNIQUE_ID_24,
	MPU_UNIQUE_ID_25,
	MPU_UNIQUE_ID_26,
	MPU_UNIQUE_ID_27,
	MPU_UNIQUE_ID_28,
	MPU_UNIQUE_ID_29,
	MPU_UNIQUE_ID_30,
	MPU_UNIQUE_ID_31,
	MPU_UNIQUE_ID_32,
	MPU_UNIQUE_ID_33,
	MPU_UNIQUE_ID_34,
	MPU_UNIQUE_ID_35,
	MPU_UNIQUE_ID_36,
	MPU_UNIQUE_ID_37,
	MPU_UNIQUE_ID_38,
	MPU_UNIQUE_ID_39,
	MPU_UNIQUE_ID_40,
	MPU_UNIQUE_ID_41,
	MPU_UNIQUE_ID_42,
	MPU_UNIQUE_ID_43,
	MPU_UNIQUE_ID_44,
	MPU_UNIQUE_ID_45,
	MPU_UNIQUE_ID_46,
	MPU_UNIQUE_ID_47,
	MPU_UNIQUE_ID_48,
	MPU_UNIQUE_ID_49,
	MPU_UNIQUE_ID_50,
	MPU_UNIQUE_ID_51,
	MPU_UNIQUE_ID_52,
	MPU_UNIQUE_ID_53,
	MPU_UNIQUE_ID_54,
	MPU_UNIQUE_ID_55,
	MPU_UNIQUE_ID_56,
	MPU_UNIQUE_ID_57,
	MPU_UNIQUE_ID_58,
	MPU_UNIQUE_ID_59,
	MPU_UNIQUE_ID_60,
	MPU_UNIQUE_ID_61,
	MPU_UNIQUE_ID_62,
	MPU_UNIQUE_ID_63,

	MPU_UNIQUE_ID_MAX,
	MPU_UNIQUE_ID_INVALID = 0x7FFFFFFF
};

enum MPU_DOMAIN_ID {
	MPU_DOMAIN_0 = 0,
	MPU_DOMAIN_1,
	MPU_DOMAIN_2,
	MPU_DOMAIN_3,
	MPU_DOMAIN_4,
	MPU_DOMAIN_5,
	MPU_DOMAIN_6,
	MPU_DOMAIN_7,
	MPU_DOMAIN_8,
	MPU_DOMAIN_9,
	MPU_DOMAIN_10,
	MPU_DOMAIN_11,
	MPU_DOMAIN_12,
	MPU_DOMAIN_13,
	MPU_DOMAIN_14,
	MPU_DOMAIN_15,

	MPU_DOMAIN_MAX,
	MPU_DOMAIN_INVALID = 0x7FFFFFFF
};

enum MPU_FEAT_ID {
	MPU_FEAT_AMMS_POS_OWNER_AP,
	MPU_FEAT_AMMS_POS_OWNER_MD,
	MPU_FEAT_AMMS_STATIC_FREE_TO_AP,
	MPU_FEAT_AMMS_STATIC_MD_READ_ONLY,
	MPU_FEAT_AMMS_STATIC_MD_READ_WRITE,
	MPU_FEAT_AMMS_STATIC_MD_DISABLE,

	MPU_FEAT_EMIMPU_ALL_NO_PROTECTION,
	MPU_FEAT_DEVMPU_ALL_NO_PROTECTION,

	MPU_FEAT_MAX,
	MPU_FEAT_INVALID = 0x7FFFFFFF
};

struct mpu_origin_zone_entry {
	enum MPU_REQ_ORIGIN_ZONE_ID zone_id;
	enum MPU_UNIQUE_ID unique_id;
	bool active;
	char name[16];
};

struct perm_cfg_emimpu {
	uint32_t phy_region_id;
	uint32_t d15 : 4;
	uint32_t d14 : 4;
	uint32_t d13 : 4;
	uint32_t d12 : 4;
	uint32_t d11 : 4;
	uint32_t d10 : 4;
	uint32_t d9 : 4;
	uint32_t d8 : 4;
	uint32_t d7 : 4;
	uint32_t d6 : 4;
	uint32_t d5 : 4;
	uint32_t d4 : 4;
	uint32_t d3 : 4;
	uint32_t d2 : 4;
	uint32_t d1 : 4;
	uint32_t d0 : 4;
};

struct perm_cfg_devmpu {
	uint32_t d3_rd : 2;
	uint32_t d2_rd : 2;
	uint32_t d1_rd : 2;
	uint32_t d0_rd : 2;
	uint32_t d3_wr : 2;
	uint32_t d2_wr : 2;
	uint32_t d1_wr : 2;
	uint32_t d0_wr : 2;
};

struct perm_cfg {
	union {
		struct perm_cfg_emimpu emimpu;
		struct perm_cfg_devmpu devmpu;
	} u_perm;
};

/* clang-format off */
#define MPU_FLAGS_NONE                          (0 << 0)
#define MPU_FLAGS_LOCK_AFTER_SET                (1 << 0)
#define MPU_FLAGS_ALLOW_DOMAIN_PERMS_CHANGE_DYN (1 << 1)
/* clang-format on */

/* unique_id: Different scenarios may use the same physical region.
 *            Therefore, we use virt ids for different scenario users
 *            to be able to have different permission settings on the
 *            same physical region. But they won't use at the same time.
 * active: whether set permission to hardware or not.
 *         > set 0 to dump log only
 *         > set 1 commit setting to hardware
 *
 */
struct mpu_perm_entry {
	enum MPU_REGION_TYPE region_type;
	enum MPU_UNIQUE_ID unique_id;
	struct perm_cfg set_perm;
	uint32_t allowed_origins;
	uint32_t flags;
	bool active;
	char name[16];
};

struct mpu_feat_perm_entry {
	enum MPU_FEAT_ID feat_id;
	struct perm_cfg perm;
};

struct mpu_domain_entry {
	enum MPU_DOMAIN_ID id;
	char name[16];
};

#endif /* __MPU_DEF_H__ */

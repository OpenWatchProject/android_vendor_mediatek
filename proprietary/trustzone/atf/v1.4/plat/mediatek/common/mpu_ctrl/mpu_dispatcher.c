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

#include <spinlock.h>
#include <mpu_ctrl/mpu_api.h>
#include <mpu_ctrl/mpu_def.h>
#include <mpu_ctrl/mpu_error.h>

#define LOG_TAG "mpu_disp"
#include <mpu_ctrl/mpu_dbg.h>

#include <plat_mpu_perms.h>
#include <plat_mpu_feats.h>

struct mpu_control {
	bool is_set;
	bool is_locked;

	enum MPU_REQ_ORIGIN origin;

	uint64_t pa;
	uint64_t sz;

	struct mpu_perm_entry *perm_ent;
};
static struct mpu_control mpu_ctrl_tbl[MPU_UNIQUE_ID_MAX];
static spinlock_t mpu_ctrl_lock;

#define ALIGN_MASK_EMIMPU ((0x1ULL < EMI_MPU_ALIGN_BITS) - 1)
#if defined(MTK_DEVMPU_SUPPORT)
#define ALIGN_MASK_DEVMPU ((0x1ULL < DEVMPU_ALIGN_BITS) - 1)
#endif

/**
 * Sanity check
 */
static inline bool is_uid_valid(uint32_t uid)
{
	return (uid < MPU_UNIQUE_ID_MAX);
}

static inline bool is_uid_inited(uint32_t uid)
{
	return (mpu_ctrl_tbl[uid].perm_ent != NULL);
}

static inline bool is_type_valid(uint32_t type)
{
	return (type < MPU_REGION_TYPE_MAX);
}

static inline bool is_feat_valid(uint32_t feat)
{
	return (feat < MPU_FEAT_MAX);
}

static inline bool is_region_valid(uint64_t pa, uint64_t sz)
{
	return ((UINT64_MAX - pa) >= sz);
}

static inline bool is_region_locked(uint32_t uid)
{
	return mpu_ctrl_tbl[uid].is_locked;
}

static inline bool is_region_aligned(uint64_t pa, uint64_t sz,
				     uint32_t mpu_type)
{
	uint64_t pa_sz_or = pa | sz;

	switch (mpu_type) {
	case MPU_REGION_TYPE_EMIMPU:
		return ((pa_sz_or & ALIGN_MASK_EMIMPU) == 0);
#if defined(MTK_DEVMPU_SUPPORT)
	case MPU_REGION_TYPE_DEVMPU:
		return ((pa_sz_or & ALIGN_MASK_DEVMPU) == 0);
#endif
	default:
		return false;
	}

	return false;
}

static inline bool is_region_overlapped(enum MPU_UNIQUE_ID req_uid,
					bool is_rewritable, uint64_t pa,
					uint64_t sz)
{
	int i;
	struct mpu_control *mpu = &mpu_ctrl_tbl[0];

	for (i = 0; i < MPU_UNIQUE_ID_MAX; ++i) {
		if (!mpu->is_set)
			continue;
		if (is_rewritable && (req_uid == mpu->perm_ent->unique_id))
			continue;
		if ((pa < mpu->pa) && ((pa + sz - 1) < mpu->pa))
			continue;
		if ((mpu->pa < pa) && ((mpu->pa + mpu->sz - 1) < pa))
			continue;

		return true;
	}

	return false;
}

static inline bool is_region_lockable(struct mpu_control *mpu)
{
	return (mpu->perm_ent->flags & MPU_FLAGS_LOCK_AFTER_SET);
}

static inline bool is_perm_changeable(uint32_t flags)
{
	return (flags & MPU_FLAGS_ALLOW_DOMAIN_PERMS_CHANGE_DYN);
}

static inline bool is_origin_allowed(uint32_t requester, uint32_t allows)
{
	if (allows == MPU_REQ_ORIGIN_INVALID)
		return false;
	return (requester & allows);
}

static struct mpu_feat_perm_entry *get_feat_entry(enum MPU_FEAT_ID feat_id)
{
	if (MPU_FEATURE_ENTRY_SIZE == 0)
		return NULL;

	if (!is_feat_valid(feat_id))
		return NULL;

	for (int idx = 0; idx < MPU_FEATURE_ENTRY_SIZE; idx++) {
		if (feat_id == mpu_feat_permission_table[idx].feat_id)
			return &mpu_feat_permission_table[idx];
	}

	return NULL;
}

#if (1 == TARGET_BUILD_VARIANT_ENG)
static const char *get_origin_name(enum MPU_REQ_ORIGIN origin)
{
	switch (origin) {
	case MPU_REQ_ORIGIN_LK:
		return "MPU_REQ_ORIGIN_LK";
	case MPU_REQ_ORIGIN_GZ:
		return "MPU_REQ_ORIGIN_GZ";
	case MPU_REQ_ORIGIN_TEE:
		return "MPU_REQ_ORIGIN_TEE";
	case MPU_REQ_ORIGIN_LINUX:
		return "MPU_REQ_ORIGIN_LINUX";
	case MPU_REQ_ORIGIN_ATF:
		return "MPU_REQ_ORIGIN_ATF";
	case MPU_REQ_ORIGIN_INVALID:
		return "MPU_REQ_ORIGIN_INVALID";
	default:
		return "MPU_REQ_ORIGIN_UNKNOWN";
	}
}
#endif

/**
 * EMI MPU handler
 */
static uint64_t emimpu_set(uint64_t pa, uint64_t sz, bool is_lock,
			   struct perm_cfg_emimpu *on_perm)
{
	uint64_t rc;

	struct emi_region_info_t region_info;

	region_info.start = pa;
	region_info.end = pa + sz - 1;
	region_info.region = on_perm->phy_region_id;

	SET_ACCESS_PERMISSION(region_info.apc, is_lock, on_perm->d15,
			      on_perm->d14, on_perm->d13, on_perm->d12,
			      on_perm->d11, on_perm->d10, on_perm->d9,
			      on_perm->d8, on_perm->d7, on_perm->d6,
			      on_perm->d5, on_perm->d4, on_perm->d3,
			      on_perm->d2, on_perm->d1, on_perm->d0);

	rc = emi_mpu_set_protection(&region_info);
	if (rc) {
		MPU_ERR("failed to set emimpu permission, rc=0x%lx\n", rc);
		return MPU_ERROR_DISP_EMIMPU_PERM_SET_FAIL;
	}

	return MPU_OK;
}

static uint64_t emimpu_clr(struct mpu_control *mpu,
			   struct perm_cfg_emimpu *off_perm)
{
	uint64_t rc;

	UNUSED(mpu);

	rc = sip_emi_mpu_clear_protection(off_perm->phy_region_id);
	if (rc) {
		MPU_ERR("failed to clear emimpu permission, rc=0x%lx\n", rc);
		return MPU_ERROR_DISP_EMIMPU_PERM_CLR_FAIL;
	}

	return MPU_OK;
}

#if defined(MTK_DEVMPU_SUPPORT)
/**
 * Device MPU handler
 */
static uint64_t devmpu_set(uint64_t pa, uint64_t sz,
			   struct perm_cfg_devmpu *perm)
{
	uint64_t rc;

	enum DEVMPU_PERM vmd_rd_perm[DEVMPU_VMD_NUM];
	enum DEVMPU_PERM vmd_wr_perm[DEVMPU_VMD_NUM];

	vmd_rd_perm[3] = perm->d3_rd;
	vmd_rd_perm[2] = perm->d2_rd;
	vmd_rd_perm[1] = perm->d1_rd;
	vmd_rd_perm[0] = perm->d0_rd;

	vmd_wr_perm[3] = perm->d3_wr;
	vmd_wr_perm[2] = perm->d2_wr;
	vmd_wr_perm[1] = perm->d1_wr;
	vmd_wr_perm[0] = perm->d0_wr;

	rc = devmpu_rw_perm_set(pa, sz, vmd_rd_perm, false);
	if (rc) {
		MPU_ERR("failed to set devmpu read permission, rc=0x%lx\n", rc);
		return MPU_ERROR_DISP_DEVMPU_READ_PERM_SET_FAIL;
	}

	rc = devmpu_rw_perm_set(pa, sz, vmd_wr_perm, true);
	if (rc) {
		MPU_ERR("failed to set devmpu write permission, rc=0x%lx\n",
			rc);
		return MPU_ERROR_DISP_DEVMPU_WRITE_PERM_SET_FAIL;
	}

	return MPU_OK;
}

static uint64_t devmpu_clr(struct mpu_control *mpu,
			   struct perm_cfg_devmpu *off_perm)
{
	uint64_t rc;

	rc = devmpu_set(mpu->pa, mpu->sz, off_perm);
	switch (rc) {
	case MPU_OK:
		return MPU_OK;
	case MPU_ERROR_DISP_DEVMPU_READ_PERM_SET_FAIL:
		return MPU_ERROR_DISP_DEVMPU_READ_PERM_CLR_FAIL;
	case MPU_ERROR_DISP_DEVMPU_WRITE_PERM_SET_FAIL:
		return MPU_ERROR_DISP_DEVMPU_WRITE_PERM_CLR_FAIL;
	default:
		MPU_ERR("unknown devmpu clear return code, rc=0x%lx\n", rc);
		return rc;
	}
	return MPU_ERROR_DISP_GENERIC;
}
#endif

/**
 * Only check for EMI MPU regions.
 * Different unique IDs may use the same physical regions.
 */
static bool is_phys_region_used(struct mpu_control *req_mpu)
{
	int i;
	struct mpu_control *mpu;

	for (i = 0; i < MPU_UNIQUE_ID_MAX; ++i) {
		if (!is_uid_inited(i))
			continue;
		mpu = &mpu_ctrl_tbl[i];

		if (!mpu->is_set)
			continue;
		if (mpu->perm_ent->region_type != MPU_REGION_TYPE_EMIMPU)
			continue;
		if (req_mpu->perm_ent->unique_id == i)
			continue;
		if (mpu->perm_ent->set_perm.u_perm.emimpu.phy_region_id
		    == req_mpu->perm_ent->set_perm.u_perm.emimpu
			       .phy_region_id) {
			MPU_ERR("EMIMPU phy_region_id=%u is already used by other uid=%d\n",
				req_mpu->perm_ent->set_perm.u_perm.emimpu
					.phy_region_id,
				i);
			return true;
		}
	}

	return false;
}

/**
 * Dispatcher
 */
static uint64_t mpu_set(struct mpu_control *mpu,
			struct mpu_feat_perm_entry *feat,
			enum MPU_REQ_ORIGIN origin, uint64_t pa, uint64_t sz,
			bool is_lock)
{
	uint64_t rc;
	struct mpu_perm_entry *perm_ent = mpu->perm_ent;

	if (!perm_ent->active) {
		MPU_DBG("%s is not active\n", perm_ent->name);
		goto mpu_set_success;
	}

	switch (perm_ent->region_type) {
	case MPU_REGION_TYPE_EMIMPU:
		if (is_phys_region_used(mpu)) {
			rc = MPU_ERROR_DISP_EMIMPU_PHYS_REGION_ALREADY_OCCUPY;
			break;
		}
		if (feat)
			feat->perm.u_perm.emimpu.phy_region_id =
				mpu->perm_ent->set_perm.u_perm.emimpu
					.phy_region_id;
		rc = emimpu_set(pa, sz, is_lock,
				(feat ? &feat->perm.u_perm.emimpu
				      : &perm_ent->set_perm.u_perm.emimpu));
		break;
	case MPU_REGION_TYPE_DEVMPU:
#if defined(MTK_DEVMPU_SUPPORT)
		rc = devmpu_set(pa, sz,
				(feat ? &feat->perm.u_perm.devmpu
				      : &perm_ent->set_perm.u_perm.devmpu));
#else
		rc = MPU_ERROR_DISP_DEVMPU_SET_OPERATION_NOT_SUPPORTED;
#endif
		break;
	default:
		MPU_ERR("invalid region type=%u\n", perm_ent->region_type);
		return MPU_ERROR_DISP_TYPE_INVALID;
	}

	if (rc)
		return rc;

mpu_set_success:
	mpu->is_set = true;
	mpu->is_locked = is_lock;

	mpu->origin = origin;

	mpu->pa = pa;
	mpu->sz = sz;

	return MPU_OK;
}

/* clang-format off */
static struct perm_cfg emimpu_off_perm = {
	.u_perm.emimpu = {
		.d15 = NO_PROTECTION, .d14 = NO_PROTECTION,
		.d13 = NO_PROTECTION, .d12 = NO_PROTECTION,
		.d11 = NO_PROTECTION, .d10 = NO_PROTECTION,
		.d9  = NO_PROTECTION, .d8  = NO_PROTECTION,
		.d7  = NO_PROTECTION, .d6  = NO_PROTECTION,
		.d5  = NO_PROTECTION, .d4  = NO_PROTECTION,
		.d3  = NO_PROTECTION, .d2  = NO_PROTECTION,
		.d1  = NO_PROTECTION, .d0  = NO_PROTECTION,
	}
};

#if defined(MTK_DEVMPU_SUPPORT)
static struct perm_cfg devmpu_off_perm = {
	.u_perm.devmpu = {
		.d3_rd = DEVMPU_PERM_NS, .d2_rd = DEVMPU_PERM_NS,
		.d1_rd = DEVMPU_PERM_NS, .d0_rd = DEVMPU_PERM_NS,
		.d3_wr = DEVMPU_PERM_NS, .d2_wr = DEVMPU_PERM_NS,
		.d1_wr = DEVMPU_PERM_NS, .d0_wr = DEVMPU_PERM_NS,
	}
};
#endif
/* clang-format on */

static uint64_t mpu_clr(struct mpu_control *mpu)
{
	uint64_t rc;
	struct mpu_perm_entry *perm_ent;

	perm_ent = mpu->perm_ent;
	if (!perm_ent->active) {
		MPU_DBG("%s is not active\n", perm_ent->name);
		goto mpu_clr_success;
	}

	switch (perm_ent->region_type) {
	case MPU_REGION_TYPE_EMIMPU:
		if (is_phys_region_used(mpu)) {
			rc = MPU_ERROR_DISP_EMIMPU_PHYS_REGION_ALREADY_OCCUPY;
			break;
		}
		emimpu_off_perm.u_perm.emimpu.phy_region_id =
			perm_ent->set_perm.u_perm.emimpu.phy_region_id;
		rc = emimpu_clr(mpu, &emimpu_off_perm.u_perm.emimpu);
		break;
	case MPU_REGION_TYPE_DEVMPU:
#if defined(MTK_DEVMPU_SUPPORT)
		rc = devmpu_clr(mpu, &devmpu_off_perm.u_perm.devmpu);
#else
		rc = MPU_ERROR_DISP_DEVMPU_CLEAR_OPERATION_NOT_SUPPORTED;
#endif
		break;
	default:
		MPU_ERR("invalid region type=%u\n", perm_ent->region_type);
		return MPU_ERROR_DISP_TYPE_INVALID;
	}

	if (rc)
		return rc;

mpu_clr_success:
	mpu->is_set = false;

	mpu->pa = 0x0ULL;
	mpu->sz = 0x0ULL;

	return MPU_OK;
}

static uint64_t mpu_dump(void)
{
#ifdef MPU_DUMP_ENABLE
	int i;
	struct mpu_control *mpu;

	for (i = 0; i < MPU_UNIQUE_ID_MAX; ++i) {
		mpu = &mpu_ctrl_tbl[i];
		if (mpu->perm_ent == NULL)
			continue;

		MPU_DBG("MPU_R%d:\n", i);

		MPU_DBG("	MPU Type: %u\n", mpu->perm_ent->region_type);
		MPU_DBG("	Flags: 0x%x\n", mpu->perm_ent->flags);
		MPU_DBG("	AllowedOrigin: 0x%x\n",
			mpu->perm_ent->allowed_origins);

		MPU_DBG("	Status: %s\n", (mpu->is_set) ? "SET" : "UNSET");
		if (mpu->is_set) {
			MPU_DBG("	Origin: %u\n", mpu->origin);
			MPU_DBG("	Base: 0x%lx\n", mpu->pa);
			MPU_DBG("	Size: 0x%lx\n", mpu->sz);
			MPU_DBG("	Lock: %s\n",
				(mpu->is_locked) ? "TRUE" : "FALSE");
			MPU_DBG("	Name: %s\n",
				mpu->perm_ent->name);
		}
	}
#endif
	return MPU_OK;
}

/**
 * Init
 */
uint64_t mpu_init(void)
{
	int i;
	struct mpu_perm_entry *perm_ent = &mpu_permission_table[0];

	MPU_TRACE_FN();

	for (i = 0; i < MPU_PERMISSION_ENTRY_SIZE; ++i, ++perm_ent) {

		if (!is_uid_valid(perm_ent->unique_id)) {
			MPU_ERR("invalid region uid=%u\n", perm_ent->unique_id);
			return MPU_ERROR_DISP_UID_INVALID;
		}

		if (is_uid_inited(perm_ent->unique_id)) {
			MPU_ERR("duplicated region uid=%u\n",
				perm_ent->unique_id);
			return MPU_ERROR_DISP_UID_DUPLICATED;
		}

		if (!is_type_valid(perm_ent->region_type)) {
			MPU_ERR("invalid region type=%u\n",
				perm_ent->region_type);
			return MPU_ERROR_DISP_TYPE_INVALID;
		}

		mpu_ctrl_tbl[perm_ent->unique_id].is_set = false;
		mpu_ctrl_tbl[perm_ent->unique_id].perm_ent = perm_ent;
	}

	mpu_dump();
	return MPU_OK;
}

/**
 * SiP interface
 */
uint64_t sip_mpu_request(enum MPU_UNIQUE_ID unique_id,
			 enum MPU_REQ_ORIGIN origin, enum MPU_FEAT_ID feat_id,
			 uint64_t addr, uint64_t size, bool is_set,
			 bool is_lock)
{
	uint64_t rc;
	struct mpu_control *mpu;
	struct mpu_feat_perm_entry *feat = NULL;
	bool is_rewrite = false;

	MPU_TRACE_FN();
	MPU_DBG_ENG("addr: 0x%lx, sz: 0x%lx, uid: %d, op: %s, lock: %s\n", addr,
		    size, unique_id, is_set ? "SET" : "CLEAR",
		    is_lock ? "LOCK" : "NO LOCK");

	if (!is_uid_valid(unique_id)) {
		MPU_ERR("invalid region uid=%u\n", unique_id);
		return MPU_ERROR_DISP_UID_INVALID;
	}

	if (!is_uid_inited(unique_id)) {
		MPU_ERR("uninited region uid=%u\n", unique_id);
		return MPU_ERROR_DISP_UID_UNINITED;
	}

	if (is_region_locked(unique_id)) {
		MPU_ERR("locked region uid=%u\n", unique_id);
		return MPU_ERROR_DISP_REGION_LOCKED;
	}

	if (!is_region_valid(addr, size)) {
		MPU_ERR("invalid region addr=%lx, size=%lx\n", addr, size);
		return MPU_ERROR_DISP_REGION_INVALID;
	}

	mpu = &mpu_ctrl_tbl[unique_id];
	is_rewrite = is_perm_changeable(mpu->perm_ent->flags);
	feat = get_feat_entry(feat_id);

	if (is_set && is_rewrite && !is_feat_valid(feat_id)) {
		MPU_ERR("invalid feat id=0x%x\n", feat_id);
		return MPU_ERROR_DISP_FEAT_ID_INVALID;
	}

	if (is_set && is_rewrite && (feat == NULL)) {
		MPU_ERR("feat entry not found=0x%x\n", feat_id);
		return MPU_ERROR_DISP_FEAT_ENTRY_NOT_FOUND;
	}

	if (is_set && is_region_overlapped(unique_id, is_rewrite, addr, size)) {
		MPU_ERR("overlapped addr=0x%lx, size=0x%lx, uid=%u, rw=0x%x\n",
			addr, size, unique_id, is_rewrite);
		return MPU_ERROR_DISP_REGION_OVERLAPPED;
	}

	if (!is_origin_allowed(origin, mpu->perm_ent->allowed_origins)) {
		MPU_ERR("origin req deny req=0x%x, allows=0x%x\n", origin,
			mpu->perm_ent->allowed_origins);
		return MPU_ERROR_DISP_ORIGIN_REQUEST_NOT_ALLOWED;
	}

	if (is_set
	    && !is_region_aligned(addr, size, mpu->perm_ent->region_type)) {
		MPU_ERR("unaligned region addr=0x%lx, size=0x%lx, type=%u\n",
			addr, size, mpu->perm_ent->region_type);
		return MPU_ERROR_DISP_ALIGNMENT;
	}

	if (is_set && is_lock && !is_region_lockable(mpu)) {
		MPU_ERR("lock-unsupported region uid=%u\n", unique_id);
		return MPU_ERROR_DISP_REGION_LOCK_NOT_SUPPORTED;
	}

	spin_lock(&mpu_ctrl_lock);

	rc = (is_set) ? mpu_set(mpu, feat, origin, addr, size, is_lock)
		      : mpu_clr(mpu);

	if (rc)
		MPU_ERR("failed to %s MPU permission, rc=0x%lx\n",
			(is_set) ? "set" : "clear", rc);
	else
		mpu_dump();

	MPU_DBG_ENG("requester: %s, uid: %d, feat: %d, attr: %s\n",
		    get_origin_name(origin), unique_id, feat_id,
		    is_rewrite ? "DYN" : "STATIC");
	MPU_DBG_ENG("addr: 0x%lx, sz: 0x%lx, op: %s, lock: %s (%s)\n", addr,
		    size, is_set ? "SET" : "CLEAR",
		    is_lock ? "LOCK" : "NO LOCK", rc ? "FAILED" : "PASSED");

	spin_unlock(&mpu_ctrl_lock);

	return rc;
}

uint64_t sip_mpu_request_ree_perm_check(uint32_t phy_id)
{
	int i;
	struct mpu_perm_entry *perm_ent;
	enum MPU_REQ_ORIGIN origin = (MPU_REQ_ORIGIN_LK | MPU_REQ_ORIGIN_LINUX);

	MPU_TRACE_FN();

	for (i = 0; i < MPU_PERMISSION_ENTRY_SIZE; ++i) {
		perm_ent = &mpu_permission_table[i];

		if (perm_ent->region_type != MPU_REGION_TYPE_EMIMPU)
			continue;

		if (phy_id == perm_ent->set_perm.u_perm.emimpu.phy_region_id) {
			if (is_origin_allowed(origin,
					      perm_ent->allowed_origins)) {
				MPU_DBG_ENG(
					"phy id=%d is allowed, allows=0x%x!\n",
					phy_id, perm_ent->allowed_origins);
				return MPU_OK;
			}

			MPU_ERR("phy id req deny phy=%d, forbid=%d, allows=0x%x\n",
				phy_id,
				perm_ent->set_perm.u_perm.emimpu.phy_region_id,
				perm_ent->allowed_origins);
			return MPU_ERROR_DISP_REE_REQUEST_PHY_ID_NOT_ALLOWED;
		}
	}

	MPU_DBG_ENG("phy id=%d is allowed!\n", phy_id);
	return MPU_OK;
}

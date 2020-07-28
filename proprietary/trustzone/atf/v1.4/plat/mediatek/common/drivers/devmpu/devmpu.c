/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein is
 * confidential and proprietary to MediaTek Inc. and/or its licensors. Without
 * the prior written permission of MediaTek inc. and/or its licensors, any
 * reproduction, modification, use or disclosure of MediaTek Software, and
 * information contained herein, in whole or in part, shall be strictly
 * prohibited.
 *
 * MediaTek Inc. (C) 2018. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER
 * ON AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL
 * WARRANTIES, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR
 * NONINFRINGEMENT. NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH
 * RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY,
 * INCORPORATED IN, OR SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES
 * TO LOOK ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO.
 * RECEIVER EXPRESSLY ACKNOWLEDGES THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO
 * OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES CONTAINED IN MEDIATEK
 * SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE
 * RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S
 * ENTIRE AND CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE
 * RELEASED HEREUNDER WILL BE, AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE
 * MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE
 * CHARGE PAID BY RECEIVER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek
 * Software") have been modified by MediaTek Inc. All revisions are subject to
 * any receiver's applicable license agreements with MediaTek Inc.
 */

#include <mmio.h>
#include <debug.h>
#include <errno.h>
#include <string.h>
#include <console.h>
#include <spinlock.h>
#include <platform_def.h>
#include <mtk_plat_common.h>

#include <devmpu.h>

#define LOG_TAG "DEVMPU"

/* debug flag */
#define DEVMPU_DEBUG (0)

#define DEVMPU_ERR(fmt, ...) ERROR("[%s] %s: " fmt, LOG_TAG, __func__, ##__VA_ARGS__)

#if DEVMPU_DEBUG
#define DEVMPU_DBG(fmt, ...) NOTICE("[%s] %s:%d: " fmt, LOG_TAG, __func__, __LINE__, ##__VA_ARGS__)
#else
#define DEVMPU_DBG(fmt, ...) do {} while (0)
#endif

/* alias ATF HW register read/write function */
#define reg_write8(r, v)    mmio_write_8(r, v)
#define reg_write16(r, v)   mmio_write_16(r, v)
#define reg_write32(r, v)   mmio_write_32(r, v)
#define reg_read8(r)        mmio_read_8(r)
#define reg_read16(r)       mmio_read_16(r)
#define reg_read32(r)       mmio_read_32(r)

/* misc constant */
#define MAX_RETRY_CHECKS    5

/* misc macro function */
#define likely(x)               __builtin_expect(!!(x), 1)
#define unlikely(x)             __builtin_expect(!!(x), 0)
#define get_bit_field(x, m, o)  ((x & (m << o)) >> o)
#define is_align_devmpu(x)      ((x & (DEVMPU_PAGE_SIZE-1)) ? false : true)

static spinlock_t devmpu_lock;

#if DEVMPU_DEBUG
static int get_invd_tbl_by_entry(uint32_t entry, uint8_t perm_out[DEVMPU_ENT_SIZE], bool is_write)
{
	int i;
	uint32_t debug_ctl;

	bool enable_stat;

	DEVMPU_DBG("entry=%u, is_write=%d\n", entry, is_write);

	if (unlikely(entry >= DEVMPU_INVD_ENT_NUM)) {
		DEVMPU_ERR("invalid %s invalidate table entry=%u\n",
				(is_write) ? "write" : "read", entry);
		return -1;
	}

	enable_stat = devmpu_is_enabled();

	spin_lock(&devmpu_lock);

	/* disable devmpu */
	if (enable_stat)
		devmpu_disable();

	/* backup original debug control */
	debug_ctl = reg_read32(DEVMPU_DEBUG_CON);

	/* config the permission entry to read */
	reg_write32(DEVMPU_DEBUG_CON, (is_write) ? 0x7 : 0x5);
	reg_write32(DEVMPU_DEBUG_SRAM_ADDR, entry);

	/* 4 bytes per read to get 128-bit written permission */
	for (i = 0; i < DEVMPU_ENT_SIZE / sizeof(uint32_t); ++i)
		*((uint32_t *)perm_out + i) = reg_read32(DEVMPU_DEBUG_SRAM_DATA);

	/* restore debug control */
	reg_write32(DEVMPU_DEBUG_CON, debug_ctl);

	/* re-enable devmpu if previously enabled */
	if (enable_stat)
		devmpu_enable();

	spin_unlock(&devmpu_lock);

	return 0;
}
#endif

static int get_perm_tbl_by_entry(uint32_t entry, uint8_t perm_out[DEVMPU_ENT_SIZE], bool is_write)
{
	int i;
	uint32_t debug_ctl;

	bool enable_stat;

	DEVMPU_DBG("entry=%u, is_write=%d\n", entry, is_write);

	if (unlikely(entry >= DEVMPU_PERM_ENT_NUM)) {
		DEVMPU_ERR("invalid %s permission table entry=%u\n",
				(is_write) ? "write" : "read", entry);
		return -1;
	}

	enable_stat = devmpu_is_enabled();

	spin_lock(&devmpu_lock);

	/* disable devmpu */
	if (enable_stat)
		devmpu_disable();

	/* backup original debug control */
	debug_ctl = reg_read32(DEVMPU_DEBUG_CON);

	/* config the permission entry to read */
	reg_write32(DEVMPU_DEBUG_CON, (is_write) ? 0x3 : 0x1);
	reg_write32(DEVMPU_DEBUG_SRAM_ADDR, entry);

	/* 4 bytes per read to get 128-bit written permission */
	for (i = 0; i < DEVMPU_ENT_SIZE / sizeof(uint32_t); ++i)
		*((uint32_t *)perm_out + i) = reg_read32(DEVMPU_DEBUG_SRAM_DATA);

	/* restore debug control */
	reg_write32(DEVMPU_DEBUG_CON, debug_ctl);

	/* re-enable devmpu if previously enabled */
	if (enable_stat)
		devmpu_enable();

	spin_unlock(&devmpu_lock);


	return 0;
}

static int set_perm_tbl_by_entry(uint32_t entry, uint8_t perm_in[DEVMPU_ENT_SIZE], bool is_write)
{
	int i;
	uint32_t perm;

	uintptr_t reg_perm_sram_addr = 0;
	uintptr_t reg_perm_sram_data = 0;

	DEVMPU_DBG("entry=%u, is_write=%d\n", entry, is_write);

	if (unlikely(entry >= DEVMPU_PERM_ENT_NUM)) {
		DEVMPU_ERR("invalid %s permission table entry=%u\n", (is_write) ? "write" : "read", entry);
		return -1;
	}

	reg_perm_sram_addr = (is_write) ? DEVMPU_WR_PERM_SRAM_ADDR : DEVMPU_RD_PERM_SRAM_ADDR;
	reg_perm_sram_data = (is_write) ? DEVMPU_WR_PERM_SRAM_DATA : DEVMPU_RD_PERM_SRAM_DATA;

	spin_lock(&devmpu_lock);

	/* enable APB interface */
	reg_write32(DEVMPU_LOAD_TABLE_USING_APB, 0x5);

	/* set table entry to update */
	reg_write32(reg_perm_sram_addr, entry);

	/* 4 bytes per write to update 128-bit permission control */
	for (i = 0; i < DEVMPU_ENT_SIZE / sizeof(uint32_t); ++i) {
		perm = *((uint32_t *)perm_in + i);
		reg_write32(reg_perm_sram_data, perm);
	}

	spin_unlock(&devmpu_lock);

#if DEVMPU_DEBUG
	{
		uint8_t perm_cmp[DEVMPU_ENT_SIZE];

		/* read the permission table entry just updated */
		if (get_perm_tbl_by_entry(entry, perm_cmp, is_write)) {
			DEVMPU_ERR("failed to get %s permission table entry %u\n",
					(is_write) ? "write" : "read", entry);
			return -1;
		}

		/* compare permission with the given one */
		for (i = 0; i < DEVMPU_ENT_SIZE; ++i) {
			if (perm_cmp[i] != perm_in[i]) {
				DEVMPU_ERR("inconsistent %s permssion in entry[%u][%d]=0x%x, expected 0x%x\n",
						(is_write) ? "write" : "read",
						entry,
						i,
						perm_cmp[i],
						perm_in[i]);
				return -1;
			}
		}
		DEVMPU_DBG("consistency check for permission entry[%u]: PASSED\n", entry);
	}
#endif

	return 0;
}

static int get_perm_tbl_by_page(uint32_t page, enum DEVMPU_PERM *perm_out, bool is_write)
{
	uint8_t perm_ent[DEVMPU_ENT_SIZE];

	uint32_t entry		   = page / DEVMPU_ENT_SIZE;
	uint32_t page_in_entry = page % DEVMPU_ENT_SIZE;

	DEVMPU_DBG("page=%u, is_write=%d\n", page, is_write);

	if (unlikely(page >= DEVMPU_PAGE_NUM)) {
		DEVMPU_ERR("invalid %s permission table page %u\n",
				(is_write) ? "write" : "read", page);
		return -1;
	}

	if (get_perm_tbl_by_entry(entry, perm_ent, is_write)) {
		DEVMPU_ERR("failed to get %s permission table entry %u\n",
				(is_write) ? "write" : "read",  entry);
		return -1;
	}

	*perm_out = perm_ent[page_in_entry];

	DEVMPU_DBG("perm_out=0x%02x\n", *perm_out);
	return 0;
}

static int set_perm_tbl_by_page(uint32_t page, enum DEVMPU_PERM perm_in, bool is_write)
{
	uintptr_t reg_perm_sram_addr = 0;
	uintptr_t reg_perm_sram_data = 0;

	DEVMPU_DBG("page=%u, perm_in=0x%x, is_write=%d\n", page, perm_in, is_write);

	if (unlikely(page >= DEVMPU_PAGE_NUM)) {
		DEVMPU_ERR("invalid %s permission table page %u\n", (is_write) ? "write" : "read", page);
		return -1;
	}

	reg_perm_sram_addr = (is_write) ? DEVMPU_WR_PERM_SRAM_ADDR : DEVMPU_RD_PERM_SRAM_ADDR;
	reg_perm_sram_data = (is_write) ? DEVMPU_WR_PERM_SRAM_DATA : DEVMPU_RD_PERM_SRAM_DATA;

	/* enable APB interface */
	reg_write32(DEVMPU_LOAD_TABLE_USING_APB, 0x19);

	/* set table entry to update */
	reg_write32(reg_perm_sram_addr, page);

	/* 1 byte write to update permission control */
	reg_write32(reg_perm_sram_data, perm_in);

#if DEVMPU_DEBUG
	{
		enum DEVMPU_PERM perm_cmp;

		/* get the permission of the page just updated */
		if (get_perm_tbl_by_page(page, &perm_cmp, is_write)) {
			DEVMPU_ERR("failed to get %s permission table page %u\n",
					(is_write) ? "write" : "read", page);
			return -1;
		}

		/* compare permission with the given one */
		if (perm_cmp != perm_in) {
			DEVMPU_ERR("inconsistent %s permission in page[%u]=0x%x, expected 0x%x\n",
					(is_write) ? "write" : "read",
					page,
					perm_cmp,
					perm_in);
			return -1;
		}
		DEVMPU_DBG("consistency check for permission page[%u]: PASSED\n", page);
	}
#endif

	return 0;
}


int devmpu_vio_get(struct devmpu_vio_stat *vio_stat)
{
	uint32_t isr;

	uint32_t vio_id;
	uint32_t vio_domain;
	uint32_t vio_is_ns;

	uint64_t vio_addr;

#if DEVMPU_VIO_FULL_INFO
	/* TODO */
#else
	uint32_t msk_off;

	/* identify read or write violation */
	isr = reg_read32(DEVMPU_ISR);
	switch ((isr >> 1) & 0x3) {
	case 1:
		vio_stat->is_write = false;
		break;
	case 2:
		vio_stat->is_write = true;
		break;
	default:
		DEVMPU_ERR("unknown violation status, ISR=%u\n", isr);
		return -1;
	}

	/* read violation registers */
	vio_id = reg_read32(DEVMPU_VIO_ID);
	vio_addr = reg_read32(DEVMPU_VIO_ADDR);
	vio_domain = reg_read32(DEVMPU_VIO_DOMAIN);
	vio_is_ns = reg_read32(DEVMPU_VIO_SECURE_BIT);

	msk_off = (vio_stat->is_write) ? DEVMPU_VIO_MASK_OFFSET_WR : DEVMPU_VIO_MASK_OFFSET_RD;

	vio_stat->id = get_bit_field(vio_id, DEVMPU_VIO_MASK_ID, msk_off);
	vio_stat->addr = get_bit_field(vio_addr, DEVMPU_VIO_MASK_ADDR, msk_off);
	vio_stat->domain = get_bit_field(vio_domain, DEVMPU_VIO_MASK_DOMAIN, msk_off);
	vio_stat->is_ns = get_bit_field(vio_is_ns, DEVMPU_VIO_MASK_SEC_B, msk_off);
#endif

	return 0;
}

void devmpu_vio_clr(void)
{
	reg_write32(DEVMPU_ICR, 0x1F);
}

int devmpu_domain_remap_get(uint32_t emi_domain, uint32_t *vm_domain)
{
	uintptr_t reg_domain_remap = 0;

	uint32_t remap_off;
	uint32_t emi_domain_in_ent;

	DEVMPU_DBG("emi_domain=%u\n", emi_domain);

	/* check if valid EMI domain */
	if (unlikely(emi_domain >= DEVMPU_EMI_DOMAIN_NUM)) {
		DEVMPU_ERR("invalid argument: EMI domain=%u\n", emi_domain);
		return -1;
	}

	/* check output pointer */
	if (unlikely(vm_domain == NULL)) {
		DEVMPU_ERR("output pointer is NULL\n");
		return -1;
	}

	/* locate VMD remap bits for the EMI domain */
	emi_domain_in_ent = emi_domain % DEVMPU_EMI_DOMAIN_PER_ENT;
	switch (emi_domain / DEVMPU_EMI_DOMAIN_PER_ENT) {
	case 0:
		reg_domain_remap = DEVMPU_DOMAIN_REMAP_0;
		break;
	case 1:
		reg_domain_remap = DEVMPU_DOMAIN_REMAP_1;
		break;
	default:
		return -1;
	}
	remap_off = DEVMPU_VMD_ENC_BITS * emi_domain_in_ent;

	*vm_domain = ((reg_read32(reg_domain_remap) & (DEVMPU_VMD_ENC_MASK << remap_off)) >> remap_off);

	return 0;
}

int devmpu_domain_remap_set(uint32_t emi_domain, uint32_t vm_domain)
{
	uintptr_t reg_domain_remap = 0;

	uint32_t remap_off;
	uint32_t emi_domain_in_ent;

	uint32_t remap_cfg = 0;

	DEVMPU_DBG("emi_domain=%u, vm_domain=%u\n", emi_domain, vm_domain);

	/* check if valid EMI/VMD domain */
	if (unlikely(emi_domain >= DEVMPU_EMI_DOMAIN_NUM || vm_domain >= DEVMPU_VMD_NUM)) {
		DEVMPU_ERR("invalid domain argument: EMI domain=%u, VM domain=%u\n", emi_domain, vm_domain);
		return -1;
	}

	/* locate VMD remap bits for the EMI domain */
	emi_domain_in_ent = emi_domain % DEVMPU_EMI_DOMAIN_PER_ENT;
	switch (emi_domain / DEVMPU_EMI_DOMAIN_PER_ENT) {
	case 0:
		reg_domain_remap = DEVMPU_DOMAIN_REMAP_0;
		break;
	case 1:
		reg_domain_remap = DEVMPU_DOMAIN_REMAP_1;
		break;
	default:
		return -1;
	}
	remap_off = DEVMPU_VMD_ENC_BITS * emi_domain_in_ent;

	spin_lock(&devmpu_lock);

	/* get the original remap setting */
	remap_cfg = reg_read32(reg_domain_remap);

	/* prepare updated remap config */
	remap_cfg &= (~(DEVMPU_VMD_ENC_MASK << remap_off));
	remap_cfg |= ((vm_domain & DEVMPU_VMD_ENC_MASK) << remap_off);

	/* update remap register with the updated config */
	reg_write32(reg_domain_remap, remap_cfg);

	spin_unlock(&devmpu_lock);

#if DEVMPU_DEBUG
	{
		uint32_t vmd_cmp = -1;

		devmpu_domain_remap_get(emi_domain, &vmd_cmp);
		DEVMPU_DBG("emi_domain=%u -> vm_domain=%u (expected %u), verification: %s\n",
				emi_domain,
				vmd_cmp,
				vm_domain,
				(vmd_cmp == vm_domain) ? "PASSED" : "FAILED");
	}
#endif
	return 0;
}

int devmpu_rw_perm_get(uint64_t pa, struct devmpu_vmd_perm *perm_out)
{
	int i;
	int rc;

	uint32_t page  = (pa - DEVMPU_DRAM_BASE) >> DEVMPU_ALIGN_BITS;

	enum DEVMPU_PERM pg_rd_perm;
	enum DEVMPU_PERM pg_wr_perm;

	DEVMPU_DBG("pa=%lx\n", pa);

	/* check for valid DRAM address range */
	if (unlikely(pa < DEVMPU_DRAM_BASE || pa >= (DEVMPU_DRAM_BASE + DEVMPU_DRAM_SIZE))) {
		DEVMPU_ERR("Invalid DRAM physical address: 0x%lx\n", pa);
		return -1;
	}

	/* validate output pointer */
	if (unlikely(perm_out == NULL)) {
		DEVMPU_ERR("output pointer is NULL\n");
		return -1;
	}

	/* get page read permission */
	rc = get_perm_tbl_by_page(page, &pg_rd_perm, false);
	if (rc) {
		DEVMPU_ERR("failed to get read permission of page %u, rc=%d\n", page, rc);
		return -1;
	}

	/* get page write permission */
	rc = get_perm_tbl_by_page(page, &pg_wr_perm, true);
	if (rc) {
		DEVMPU_ERR("failed to get write permission of page %u, rc=%d\n", page, rc);
		return -1;
	}

	/* output permission */
	for (i = 0; i < DEVMPU_VMD_NUM; ++i) {
		perm_out->rd_perm[i] = get_bit_field(pg_rd_perm, DEVMPU_VMD_ENC_MASK, i * DEVMPU_VMD_ENC_BITS);
		perm_out->wr_perm[i] = get_bit_field(pg_wr_perm, DEVMPU_VMD_ENC_MASK, i * DEVMPU_VMD_ENC_BITS);
	}

	return 0;
}

int devmpu_rw_perm_set(uint64_t pa, uint64_t sz, enum DEVMPU_PERM vmd_perm[DEVMPU_VMD_NUM], bool is_write)
{
	int i;
	int rc = 0;

	uint8_t perm_bmp[DEVMPU_ENT_SIZE];
	uint8_t perm_page = 0;

	uint32_t page_st;
	uint32_t page_ed;

	DEVMPU_DBG("pa=0x%lx, sz=0x%lx, is_write=%d\n", pa, sz, is_write);

	/* check for alignment */
	if (unlikely(is_align_devmpu(pa) == false || is_align_devmpu(sz) == false)) {
		DEVMPU_ERR("Unaligned (alignment=0x%x) physical address=0x%lx or size=0x%lx\n",
				DEVMPU_PAGE_SIZE, pa, sz);
		return -1;
	}

	/* check for valid VM domain permission */
	if (unlikely(vmd_perm == NULL)) {
		DEVMPU_ERR("VM domain permission is NULL\n");
		return -1;
	}

	/* check for valid DRAM address range */
	if (unlikely(
		pa <  DEVMPU_DRAM_BASE
	||	pa >= (DEVMPU_DRAM_BASE + DEVMPU_DRAM_SIZE - sz)
	)) {
		DEVMPU_ERR("Invalid DRAM physical address range: pa=0x%lx, sz=0x%lx\n", pa, sz);
		return -1;
	}

	/* do nothing if zero size */
	if (unlikely(sz == 0))
		return 0;

	/* we are doing align here anyway, in case the implementation needs flexibility */
	page_st = DEVMPU_ALIGN_DOWN(pa - DEVMPU_DRAM_BASE) >> DEVMPU_ALIGN_BITS;
	page_ed = DEVMPU_ALIGN_DOWN(pa - DEVMPU_DRAM_BASE + sz - 1) >> DEVMPU_ALIGN_BITS;

	DEVMPU_DBG("update permission for DRAM pages, page_st=%u, page_ed=%u\n", page_st, page_ed);

	/* assemble permission bitmap for pages/entries */
	for (i = 0; i < DEVMPU_VMD_NUM; ++i)
		perm_page |= ((vmd_perm[i] & DEVMPU_VMD_ENC_MASK) << (DEVMPU_VMD_ENC_BITS * i));

	memset(perm_bmp, perm_page, sizeof(perm_bmp));

	/* update permission control table */
	while (page_st <= page_ed) {
		/* perform entry-based (128 bits) update if aligned to entry number */
		if ((page_st % DEVMPU_ENT_SIZE == 0) && (page_ed - page_st + 1) >= DEVMPU_ENT_SIZE) {
			rc = set_perm_tbl_by_entry(page_st / DEVMPU_ENT_SIZE, perm_bmp, is_write);
			page_st += DEVMPU_ENT_SIZE;
		}
		/* otherwise, update page-by-page */
		else {
			rc = set_perm_tbl_by_page(page_st, perm_page, is_write);
			page_st++;
		}

		/* return immediately if any error */
		if (rc) {
			DEVMPU_ERR("failed to update %s permission for pa=0x%lx, size=0x%lx\n",
					(is_write) ? "write" : "read", pa, sz);
			return rc;
		}
	}

	return 0;
}

int devmpu_reset(void)
{
	int i;

#if DEVMPU_DEBUG
	uint8_t invd_perm[DEVMPU_ENT_SIZE];
#endif
	uint8_t clear_perm[DEVMPU_ENT_SIZE];

	/* prepare zero-clean permission settings */
	memset(clear_perm, 0x0, sizeof(clear_perm));

#if !(DEVMPU_VIO_FULL_INFO)
	/* mask all interrupt as the violation info. is incomplete */
	reg_write32(DEVMPU_IMR, 0x1F);
#endif

	/* clear violation */
	devmpu_vio_clr();

	/* clear debug control */
	reg_write32(DEVMPU_DEBUG_CON, 0x0);

	/* set all EMI domain to VM domain 0 */
	for (i = 0; i < DEVMPU_EMI_DOMAIN_NUM; ++i) {
		if (devmpu_domain_remap_set(i, 0))
			goto devmpu_reset_fail;
	}

	/* clear R/W permission tables (auto-clear invalidate table) */
	for (i = 0; i < DEVMPU_PERM_ENT_NUM; ++i) {
		if (set_perm_tbl_by_entry(i, clear_perm, false))
			goto devmpu_reset_fail;

		if (set_perm_tbl_by_entry(i, clear_perm, true))
			goto devmpu_reset_fail;
	}

#if DEVMPU_DEBUG
	/* check auto-cleared invalidate table */
	for (i = 0; i < DEVMPU_INVD_ENT_NUM; ++i) {
		get_invd_tbl_by_entry(i, invd_perm, false);
		if (memcmp(clear_perm, invd_perm, sizeof(clear_perm))) {
			DEVMPU_ERR("unclean read invalidate table entry %u\n", i);
			goto devmpu_reset_fail;
		}
		DEVMPU_DBG("clean check for read invalidate table entry[%u]: PASSED\n", i);

		get_invd_tbl_by_entry(i, invd_perm, true);
		if (memcmp(clear_perm, invd_perm, sizeof(clear_perm))) {
			DEVMPU_ERR("unclean write invalidate table entry %u\n", i);
			goto devmpu_reset_fail;
		}
		DEVMPU_DBG("clean check for write invalidate table entry[%u]: PASSED\n", i);
	}
#endif

	return 0;

devmpu_reset_fail:
	DEVMPU_ERR("failed to reset DeviceMPU\n");
	return -1;
}

bool devmpu_is_enabled(void)
{
	if (
		((reg_read32(EMI_HYP_MOD) & EMI_HYP_MOD_BITMASK) == 0)
	||	(reg_read32(DEVMPU_DISABLE) & 0x1))
		return false;

	return true;
}

int devmpu_disable(void)
{
	uint32_t retry_chk;
	uint32_t emi_hyp_mod;

	/* disable DeviceMPU */
	reg_write32(DEVMPU_DISABLE, 0x1);

	/* disable hypervior mode MPU (aka DevMPU) of EMI */
	emi_hyp_mod  = reg_read32(EMI_HYP_MOD);
	emi_hyp_mod &= ~(EMI_HYP_MOD_BITMASK);
	reg_write32(EMI_HYP_MOD, emi_hyp_mod);

    /*
	 * double check register value as well as to increase latency
	 * for SW workaround dealing the HW data race issues between
	 * Device MPU permission check and permission dump.
	 */
	for (retry_chk = 0; retry_chk < MAX_RETRY_CHECKS; ++retry_chk) {
		if (!devmpu_is_enabled())
			break;
	}

	if (MAX_RETRY_CHECKS && retry_chk == MAX_RETRY_CHECKS) {
		DEVMPU_ERR("DeviceMPU disable exceed max-retry count\n");
		return -1;
	}

	return 0;
}

int devmpu_enable(void)
{
	uint32_t retry_chk;
	uint32_t emi_hyp_mod;

	/* enable hypervisor mode MPU (aka DevMPU) of EMI */
	emi_hyp_mod  = reg_read32(EMI_HYP_MOD);
	emi_hyp_mod |= EMI_HYP_MOD_BITMASK;
	reg_write32(EMI_HYP_MOD, emi_hyp_mod);

	/* enable DeviceMPU */
	reg_write32(DEVMPU_DISABLE, 0x0);

	/*
	 * double check register value as well as to increase latency
	 * for SW workaround dealing the HW data race issues between
	 * Device MPU permission check and permission dump.
	 */
	for (retry_chk = 0; retry_chk < MAX_RETRY_CHECKS; ++retry_chk) {
		if (devmpu_is_enabled())
			break;
	}

	if (MAX_RETRY_CHECKS && retry_chk == MAX_RETRY_CHECKS) {
		DEVMPU_ERR("DeviceMPU enable exceed max-retry count\n");
		return -1;
	}

	return 0;
}

uint64_t sip_devmpu_vio_get(uint64_t x1, uint64_t *o1, uint64_t *o2)
{
	int rc;
	struct devmpu_vio_stat vio;

	bool do_clear;

	if (unlikely(o1 == NULL || o2 == NULL)) {
		DEVMPU_ERR("output pointer is NULL\n");
		return -EINVAL;
	}

	/* decode input */
	do_clear = (bool)x1;

	/* process */
	rc = devmpu_vio_get(&vio);

	if (do_clear)
		devmpu_vio_clr();

	if (rc) {
		DEVMPU_ERR("failed to parse violation, rc=%d\n", rc);
		return -EPERM;
	}

	/* encode output */
	*o1 = 0x0ULL;
	*o2 = 0x0ULL;

	*o1 = vio.addr;
	*o2 = ((vio.id & 0xFFFF) << 16)
		| ((vio.domain & 0xFF) << 8)
		| ((vio.is_ns & 0x1) << 1)
		| ((vio.is_write & 0x1));

	return 0;
}

uint64_t sip_devmpu_vio_clr(void)
{
	uint32_t emi_conm = reg_read32(EMI_CONM);
	uint32_t devmpu_con = reg_read32(DEVMPU_CON);

	/*
	 * Workaround for HW clock issue:
	 *
	 * force EMI clock ON to ensure that
	 * violation status is cleared.
	 */
	reg_write32(EMI_CONM, (emi_conm | (0x1 << 30)));
	reg_write32(DEVMPU_CON, (devmpu_con | 0x1));

	/* clear violation */
	reg_write32(EMI_MPUT_2ND, (0x1 << 30));

	/* restore HW config. */
	reg_write32(EMI_CONM, emi_conm);
	reg_write32(DEVMPU_CON, devmpu_con);

	return 0;
}

uint64_t sip_devmpu_rw_perm_get(uint64_t x1, uint64_t *o1, uint64_t *o2)
{
	int i;
	int rc;

	uint64_t pa;

	struct devmpu_vmd_perm perm;

	if (unlikely(o1 == NULL || o2 == NULL)) {
		DEVMPU_ERR("output pointer is NULL\n");
		return -EINVAL;
	}

	/* decode input */
	pa = x1;

	/* process */
	rc = devmpu_rw_perm_get(pa, &perm);
	if (rc) {
		DEVMPU_ERR("failed to get permission, rc=%d\n", rc);
		return -EPERM;
	}

	/* encode output */
	*o1 = 0x0ULL;
	*o2 = 0x0ULL;

	for (i = 0; i < DEVMPU_VMD_NUM; ++i) {
		*o1 |= ((perm.rd_perm[i] & DEVMPU_VMD_ENC_MASK) << (i * DEVMPU_VMD_ENC_BITS));
		*o2 |= ((perm.wr_perm[i] & DEVMPU_VMD_ENC_MASK) << (i * DEVMPU_VMD_ENC_BITS));
	}

	return 0;
}

uint64_t sip_devmpu_rw_perm_set(uint64_t x1, uint64_t x2, uint64_t x3)
{
	int i;
	int rc;

	uint64_t pa;
	uint64_t sz;
	enum DEVMPU_PERM vmd_perm[DEVMPU_VMD_NUM];
	bool is_write;

	/* decode input */
	pa = x1 & ~(0x1);
	sz = x2;
	is_write = x1 & (0x1);
	for (i = 0; i < DEVMPU_VMD_NUM; ++i) {
		vmd_perm[i] = x3 & DEVMPU_VMD_ENC_MASK;
		x3 >>= DEVMPU_VMD_ENC_BITS;
	}

	/* process */
	rc = devmpu_rw_perm_set(pa, sz, vmd_perm, is_write);
	if (rc) {
		DEVMPU_ERR("failed to set permission, rc=%d\n", rc);
		return -EPERM;
	}

	return 0;
}

uint64_t sip_devmpu_domain_remap_set(uint64_t x1, uint64_t x2)
{
	int rc;

	uint64_t emi_domain;
	uint64_t vm_domain;

	/* decode input */
	emi_domain = x1;
	vm_domain = x2;

	/* process */
	rc = devmpu_domain_remap_set(emi_domain, vm_domain);
	if (rc) {
		DEVMPU_ERR("failed to set domain, rc=%d\n", rc);
		return -EPERM;
	}

	return 0;
}

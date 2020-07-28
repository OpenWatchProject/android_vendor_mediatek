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
 * MediaTek Inc. (C) 2019. All rights reserved.
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

#include "tui_m4u.h"
#include "m4u_reg.h"
#include "m4u_pgtable.h"
#include "DrApi/DrApi.h"
#include "log.h"


#define LOG_TAG "TUI_M4U"
#define M4ULOGD(fmt, args...)	TUI_LOGW("["LOG_TAG"]"fmt, ##args)
#define M4ULOGE(fmt, args...)	TUI_LOGE("["LOG_TAG"]error:"fmt, ##args)
#define M4ULOGV(fmt, args...)   TUI_LOGW("["LOG_TAG"]"fmt, ##args)
/*depend on CMDQ api for config port*/
#define DISP_REG_MASK_EXT(handle, reg_va, reg_pa, val, mask) 	\
	do { \
	   if(handle==NULL) \
	   { \
			 M4U_REG_SET((unsigned int)(M4U_REG_GET(reg_va)&~(mask))|(val),(volatile void*)(reg_va) );\
	   } \
	   else \
	   { \
			 cmdqRecWrite(handle, reg_pa, val, mask); \
	   }  \
	} while (0)

struct m4u_pt_base {
	unsigned long pagetable_base;
	unsigned long pagetable_base_aligned;
};

static short mvaGraph[MVA_MAX_BLOCK_NR+1];
static unsigned char moduleGraph[MVA_MAX_BLOCK_NR+1];
static unsigned int g_m4u_reg_base_original_val = 0;
static unsigned int g_m4u_secure_status = 0;
volatile unsigned long g_m4u_reg_base_va[1] = {0};
struct m4u_pt_base g_sec_pt_base;

#define SMI_LARB_NR		4
#define SMI_LARB_SZ		(0x1000)

#define LARB0_BASE		0x14003000
#define LARB1_BASE		0x17010000
#define LARB2_BASE		0x15021000
#define LARB3_BASE		0x1a002000

static unsigned long smiLarbBaseAddr[SMI_LARB_NR];
static unsigned long smiLarbBasePA[SMI_LARB_NR] = {LARB0_BASE, LARB1_BASE,
						   LARB2_BASE, LARB3_BASE};

static unsigned int m4u_port0_in_larbx[SMI_LARB_NR] = {M4U_PORT_DISP_OVL0, M4U_PORT_CAM_IMGI,
					M4U_PORT_CAM_IMGO, M4U_PORT_VENC_RCPU};

static inline int m4u_port_2_larb_port(enum M4U_PORT_ID port)
{
	int i;
	for(i = SMI_LARB_NR-1; i>=0; i--)
	{
		if(port >= m4u_port0_in_larbx[i])
			return (port - m4u_port0_in_larbx[i]);
	}
	return 0;
}


static inline int m4u_port_2_larb_id(enum M4U_PORT_ID port)
{
	int i;
	for(i = SMI_LARB_NR-1; i>=0; i--)
	{
		if(port >= m4u_port0_in_larbx[i])
			return i;
	}
	return 0;
}

static int tui_m4u_pagetable_init(void)
{
	g_sec_pt_base.pagetable_base = (unsigned long)drApiMalloc(IMU_PGD_SIZE + IMU_PGD_ALIGN,0);
	if (!g_sec_pt_base.pagetable_base) {
		M4ULOGE("Physical memory not available.\n");
		return -1;
	}
	g_sec_pt_base.pagetable_base_aligned =
		((g_sec_pt_base.pagetable_base + IMU_PGD_ALIGN) & (~IMU_PGD_ALIGN));
	memset((void *)g_sec_pt_base.pagetable_base, 0, IMU_PGD_SIZE + IMU_PGD_ALIGN);

	return 0;
}

static void tui_m4u_mvaGraph_init(void)
{
	memset(mvaGraph, 0, sizeof(short)*(MVA_MAX_BLOCK_NR+1));
	memset(moduleGraph, 0, sizeof(unsigned char)*(MVA_MAX_BLOCK_NR+1));
	mvaGraph[0] = 1|MVA_BUSY_MASK;
	moduleGraph[0] = M4U_PORT_UNKNOWN;
	mvaGraph[1] = MVA_MAX_BLOCK_NR;
	moduleGraph[1] = M4U_PORT_UNKNOWN;
	mvaGraph[MVA_MAX_BLOCK_NR] = MVA_MAX_BLOCK_NR;
	moduleGraph[MVA_MAX_BLOCK_NR] = M4U_PORT_UNKNOWN;
}

static int tui_m4u_hw_init(void)
{
	unsigned int ret, i;

	if(g_m4u_secure_status)
		return 0;

	ret = drApiMapPhysicalBuffer(M4U0_REG_BASE,
				     0x1000,
				     MAP_HARDWARE,
				     (void **)&g_m4u_reg_base_va[0]);
	if (ret) {
		M4ULOGE("map reg addr to virtual addr failed.\n");
		return -1;
	}

#if 0 //debug of the m4u translation fualt at normal MVA, switch to sec mapping table ,after display frame done.
	tui_m4u_switch_to_sec();
#endif

	/*map larb 1 base address*/
	//map LARB registers

	for (i = 0; i < SMI_LARB_NR; i ++) {
		ret = drApiMapPhysicalBuffer(smiLarbBasePA[i], SMI_LARB_SZ, MAP_HARDWARE, (void **)&smiLarbBaseAddr[i]);

		if(ret) {
			M4ULOGE("map larb %d register fail: pa=0x%x, size=0x%x, ret=%d\n", i,  smiLarbBasePA[i], SMI_LARB_SZ, ret);
			return -1;
		}
		M4ULOGV("smi larb %d map register: va=0x%x, pa=0x%x, size=0x%x\n", i, smiLarbBaseAddr[i], smiLarbBasePA[i], SMI_LARB_SZ);

	}

	return 0;
}

static unsigned int tui_m4u_do_alloc_mva(enum M4U_PORT_ID port_id,
						const uint64_t buf_addr,
						const unsigned int buf_size)
{
	short s,end;
	short new_start, new_end;
	short required_block_cnt = 0;
	unsigned int mva_start;
	unsigned long start_required, end_required, size_required;
	enum M4U_PORT_ID original_port_id;

	if (buf_size == 0)
		return 0;

	/*calculate mva block number*/
	start_required = buf_addr & (~M4U_VA_PAGE_MASK);
	end_required = (buf_addr + buf_size - 1)| M4U_VA_PAGE_MASK;
	size_required = end_required - start_required + 1;
	required_block_cnt = (size_required + MVA_BLOCK_ALIGN_MASK) >> MVA_BLOCK_SIZE_ORDER;

	/*find first match free region*/
	for(s = 1; (s < (MVA_MAX_BLOCK_NR + 1)) && (mvaGraph[s] < required_block_cnt);
	    s += (mvaGraph[s] & MVA_BLOCK_NR_MASK))
		;
	if (s > MVA_MAX_BLOCK_NR) {
		M4ULOGE("mva_alloc error: no available MVA region for %d blocks!\n",
		       required_block_cnt);
		return 0;
	}

	/*get a total mva region(from s to end) to insert our required mva region*/
	end = s + mvaGraph[s] - 1;

	if ((required_block_cnt == mvaGraph[s])) {
		MVA_SET_BUSY(s);
		MVA_SET_BUSY(end);
		moduleGraph[s] = port_id;
		moduleGraph[end] = port_id;
	} else {
		/*new_end is the end of our required mva region */
		new_end = s + required_block_cnt - 1;
		/*new_start is the start of remained mva region (total - required)*/
		new_start = new_end + 1;
		//note: new_start may equals to end
		/*update the remained region*/
		mvaGraph[new_start] = (mvaGraph[s] - required_block_cnt);
		mvaGraph[end] = mvaGraph[new_start];
		/*our required region is from s to new_end*/
		mvaGraph[s] = required_block_cnt | MVA_BUSY_MASK;
		mvaGraph[new_end] = mvaGraph[s];

		/*update module*/
		original_port_id = moduleGraph[s];
		moduleGraph[s] = port_id;
		moduleGraph[new_end] = port_id;
		moduleGraph[new_start] = original_port_id;
		moduleGraph[end] = original_port_id;
	}

	mva_start = (unsigned int)s;
	return (mva_start << MVA_BLOCK_SIZE_ORDER) + MVA_PAGE_OFFSET(buf_addr);
}

static int m4u_fill_section(unsigned int mva, uint64_t pa, unsigned int prot)
{
	unsigned int pgprot;
	unsigned int padscpt;
	imu_pgd_t *pgd;
	/*write section into page table buffer directly*/
	imu_pgd_t *g_pgd = (imu_pgd_t *)g_sec_pt_base.pagetable_base_aligned;

	if ((mva & (~F_PGD_PA_SECTION_MSK)) != ((unsigned int)pa & (~F_PGD_PA_SECTION_MSK))) {
		M4ULOGE("error to map section: mva=0x%lx, pa=0x%lx.\n", mva, pa);
		return -1;
	}

	mva &= F_PGD_PA_SECTION_MSK;
	/*set 32bit*/
	if (pa > 0xffffffffUL)
		padscpt = ((unsigned int)pa & F_PGD_PA_SECTION_MSK) | F_PGD_BIT32_BIT;
	else
		padscpt = (unsigned int)pa & F_PGD_PA_SECTION_MSK;

	pgprot = __m4u_get_pgd_attr_1M(prot);
	/*compute offset from page table buffer base address*/
	pgd = imu_pgd_offset(g_pgd, mva);

	if ((imu_pgd_val(*pgd))) {
		M4ULOGE("%s: mva=0x%lx, pgd=0x%x\n", __func__, mva, imu_pgd_val(*pgd));
		return -1;
	}

	/*write section*/
	imu_pgd_val(*pgd) = pa | padscpt | pgprot;

	return 0;
}

static int tui_m4u_fill_pagetable(uint64_t phy_buf_addr,
				       unsigned int buf_size,
				       unsigned int mva_start,
				       unsigned int prot)
{
	int ret;
	unsigned int mva;
	unsigned int mva_end = mva_start + buf_size - 1;
	unsigned int required_desc_cnt = 0;

	required_desc_cnt = M4U_GET_PAGE_NUM(mva_start, buf_size);
	M4ULOGV("mva_start = 0x%x, mva_end = 0x%x, pa=0x%x, buf size = %u, required_desc_cnt = %u\n",
		mva_start, mva_end,phy_buf_addr, buf_size, required_desc_cnt);
	/*align to 1M*/
	mva_start &= F_PGD_PA_SECTION_MSK;
	phy_buf_addr &= F_PGD_PA_SECTION_MSK;
	for(mva=mva_start; required_desc_cnt != 0; required_desc_cnt--) {
		m4u_fill_section(mva, phy_buf_addr, prot);
		mva += MMU_SECTION_SIZE;
		phy_buf_addr+=MMU_SECTION_SIZE;
	}
	/*flush pt buffer in cpu cache to dram*/
	ret = drApiCacheDataCleanAll();
	if (ret) {
		M4ULOGE("flush pt buffer in cpu cache to dram failed,\n");
		return -1;
	}
	return 0;
}

static int tui_m4u_invalid_tlb(int m4u_id,
				   int L2_en,
				   int isInvAll,
				   unsigned int mva_start,
				   unsigned int mva_end)
{
    unsigned int reg = 0;
    unsigned long m4u_base = g_m4u_reg_base_va[0];

    if(L2_en)
        reg = F_MMU_INV_EN_L2;

    reg |= F_MMU_INV_EN_L1;

    /*invalid high 4G tlb*/
    //reg |= F_MMU_INV_VA_32;

    M4U_WriteReg32(m4u_base, REG_INVLID_SEL, reg);


    if(isInvAll)
    {
        M4U_WriteReg32(m4u_base, REG_MMU_INVLD, F_MMU_INV_ALL);
    }
    else
    {
        M4U_WriteReg32(m4u_base, REG_MMU_INVLD_SA ,mva_start);
        M4U_WriteReg32(m4u_base, REG_MMU_INVLD_EA, mva_end);
        M4U_WriteReg32(m4u_base, REG_MMU_INVLD, F_MMU_INV_RANGE);
    }

    if(!isInvAll)
    {
        while(!M4U_ReadReg32(m4u_base, REG_MMU_CPE_DONE));
        M4U_WriteReg32(m4u_base, REG_MMU_CPE_DONE, 0);
    }


    return 0;

}

static void tui_m4u_invalid_tlb_all(int m4u_id, int L2_en)
{
    tui_m4u_invalid_tlb(m4u_id, L2_en, 1, 0, 0);
}

void dump_mva_graph()
{
	int index = 1, i;
	int count;
	enum M4U_PORT_ID owner;
	char *status;

	M4ULOGV("***************dump mva graph****************\n");
	M4ULOGD("graph[%04s]%8s%8s%8s\n", "index", "count", "status", "owner");
	for (index = 1; index <= MVA_MAX_BLOCK_NR;) {
		if (MVA_IS_BUSY(index))
			status= "busy";
		else
			status= "free";
		owner = (enum M4U_PORT_ID)moduleGraph[index];
		count = MVA_GET_NR(index);
		M4ULOGD("graph[%04u]%8u%8s%8u\n", index, count, status, owner);
		index += count;
	}

	M4ULOGV("*************dump mva graph done*************\n\n");
}
/*dump page table by mva status. only print one at busy status.*/
void dump_pagetable_in_use(void)
{
	/*our mva graph start with 1 not 0, so the index should start with 16th pgd.*/
	unsigned int pt_start_idx = 0x10, pt_offset = 0x10;
	unsigned int pt_jump_count = 0;
	unsigned int* p =
		(unsigned int*)g_sec_pt_base.pagetable_base_aligned + pt_start_idx;
	unsigned int* tmp_end;
	unsigned int block_count = 0;
	int graph_index;

	int ret = drApiCacheDataCleanAll();
	if (ret) {
		M4ULOGE("flush pt buffer in cpu cache to dram failed,\n");
		return;
	}
	M4ULOGV("*************dump busy page table*************\n");
	M4ULOGV("%12s %20s\n","graph index", "descriptor");
	for (graph_index = 1; graph_index <= MVA_MAX_BLOCK_NR; graph_index++) {
		/*get block count at current graph index*/
		block_count = MVA_GET_NR(graph_index);
		/*get pt pointer next start address*/
		pt_jump_count = block_count * pt_offset;
		tmp_end = p + pt_jump_count;

		/*ignore free blocks*/
		if (!MVA_IS_BUSY(graph_index)) {
			p = tmp_end;
			//M4ULOGD("ignore mva graph %u\n", graph_index);
			continue;
		}
		for (; p < tmp_end; p++) {
			M4ULOGE("%12u 0x%20x\n", graph_index, *p);
		}
		p = tmp_end;
	}
	M4ULOGV("***********dump busy page table done***********\n\n");

}
static int tui_m4u_dealloc_mva(unsigned int mva, unsigned int size)
{
	unsigned int startRequire, endRequire, sizeRequire;
	short nrRequire;
	unsigned long irq_flags;
	/*compute mva graph start/end index and the block count needed to dealloc*/
	short startIdx = mva >> MVA_BLOCK_SIZE_ORDER;
	short dealloc_block_cnt = mvaGraph[startIdx] & MVA_BLOCK_NR_MASK;
	short endIdx = startIdx + dealloc_block_cnt - 1;

	/* -------------------------------- */
	/* check the input arguments */
	/* right condition: startIdx is not NULL && region is busy && right module && right size */
	startRequire = mva & (unsigned int)(~M4U_VA_PAGE_MASK);
	endRequire = (mva + size - 1) | (unsigned int)M4U_VA_PAGE_MASK;
	sizeRequire = endRequire - startRequire + 1;
	nrRequire = (sizeRequire + MVA_BLOCK_ALIGN_MASK) >> MVA_BLOCK_SIZE_ORDER;
	/* (sizeRequire>>MVA_BLOCK_SIZE_ORDER) + ((sizeRequire&MVA_BLOCK_ALIGN_MASK)!=0); */
	if (!(startIdx != 0	/* startIdx is not NULL */
		&& MVA_IS_BUSY(startIdx)
		&& (dealloc_block_cnt == nrRequire))) {
		M4ULOGE("error to free mva========================>\n");
		M4ULOGE("BufSize=%d(unit:0x%xBytes) (expect %d)\n",
		       nrRequire, MVA_BLOCK_SIZE, dealloc_block_cnt);
		M4ULOGE("mva=0x%x, (IsBusy?)=%d (expect %d)\n",
		       mva, MVA_IS_BUSY(startIdx), 1);
		return -1;
	}

	/*revert to defualt value*/
	moduleGraph[startIdx] = M4U_PORT_UNKNOWN;
	moduleGraph[endIdx] = M4U_PORT_UNKNOWN;

	/* -------------------------------- */
	/* merge with followed region */
	if ((endIdx + 1 <= MVA_MAX_BLOCK_NR) && (!MVA_IS_BUSY(endIdx + 1))) {
		dealloc_block_cnt += mvaGraph[endIdx + 1];
		mvaGraph[endIdx] = 0;
		mvaGraph[endIdx + 1] = 0;
	}
	/* -------------------------------- */
	/* merge with previous region */
	if ((startIdx - 1 > 0) && (!MVA_IS_BUSY(startIdx - 1))) {
		int pre_nr = mvaGraph[startIdx - 1];

		mvaGraph[startIdx] = 0;
		mvaGraph[startIdx - 1] = 0;
		startIdx -= pre_nr;
		dealloc_block_cnt += pre_nr;
	}
	/* -------------------------------- */
	/* set region flags */
	mvaGraph[startIdx] = dealloc_block_cnt;
	mvaGraph[startIdx + dealloc_block_cnt - 1] = dealloc_block_cnt;

	return 0;
}


static int tui_m4u_clean_section(unsigned int mva)
{
	imu_pgd_t *pgd;
	/*write section into page table buffer directly*/
	imu_pgd_t *g_pgd = (imu_pgd_t *)g_sec_pt_base.pagetable_base_aligned;
	/*compute offset from page table buffer base address*/
	pgd = imu_pgd_offset(g_pgd, mva);
	if (!imu_pgd_val(*pgd)) {
		M4ULOGE("the section need to clean is NULL\n");
		return -1;
	} else
		imu_pgd_val(*pgd) = 0;
	return 0;
}


static int tui_m4u_clean_pagetable(unsigned int mva_start,
					 unsigned int buf_size)
{
	unsigned int mva;
	unsigned int mva_end = mva_start + buf_size - 1;

	for(mva = mva_start; mva <= mva_end; mva += MMU_SECTION_SIZE) {
		if(tui_m4u_clean_section(mva))
			return -1;
	}
	return 0;
}

int tui_m4u_Init(void)
{
	int ret;

	ret = tui_m4u_pagetable_init();
	if (ret) {
		M4ULOGE("init page table failed.\n");
		return -1;
	}

	tui_m4u_mvaGraph_init();

	/*restore original secure page table address to avoid corruption after quite tui*/
	ret = tui_m4u_hw_init();
	if (ret) {
		M4ULOGE("init hw failed.\n");
		return -1;
	}
	return 0;
}

int tui_m4u_switch_to_sec(void)
{
	void *pt_base_pa = NULL;
	int ret = 0;
	
	if(g_m4u_secure_status)
		return 0;
	
	g_m4u_reg_base_original_val = M4U_REG_GET(g_m4u_reg_base_va[0] + REG_MMUg_PT_BASE_SEC);
	M4ULOGV("%s, %d: remove the normal mva mapping table at: 0x%x\n", __func__, __LINE__, g_m4u_reg_base_original_val);

	/*check if pt buffer exists*/
	if (!g_sec_pt_base.pagetable_base_aligned) {
		M4ULOGE("page table buffer was not initailized.\n");
		return -1;
	}

	/*before fill pt buffer start address to sec base register, convert virtual address
	 * to physical address is required.*/
	ret = drApiVirt2Phys(0,
			     (void *)g_sec_pt_base.pagetable_base_aligned,
			     (void **)&pt_base_pa);
	if (ret) {
		M4ULOGE("convert pt buffer va to pa failed.\n");
		tui_m4u_deinit();
		return -1;
	}

	/*fill TUI secure page table address*/
	M4U_REG_SET(g_m4u_reg_base_va[0] + REG_MMUg_PT_BASE_SEC,
		    (unsigned int)pt_base_pa);
	M4ULOGV("%s, %d: add the secure mva mapping table at: 0x%x\n", __func__, __LINE__, M4U_REG_GET(g_m4u_reg_base_va[0] + REG_MMUg_PT_BASE_SEC));
	g_m4u_secure_status = 1;

	return 0;

}
void tui_m4u_config_port_sec(cmdqRecHandle cmdq_handle, uint32_t port, int mmu_en, int sec)
{
	unsigned int larbid = m4u_port_2_larb_id(port);
	unsigned int larb_port = m4u_port_2_larb_port(port);
	unsigned long larb_base = smiLarbBaseAddr[larbid];
	unsigned int regval;

	m4uHw_set_field_by_mask(larb_base, SMI_LARB_SEC_CONx(larb_port),\
				F_SMI_SEC_MMU_EN(1), F_SMI_SEC_MMU_EN(!!(mmu_en)));
	
	m4uHw_set_field_by_mask(larb_base, SMI_LARB_SEC_CONx(larb_port),\
				F_SMI_SEC_EN(1), F_SMI_SEC_EN(!!(sec)));

	//debug use
	mmu_en = m4uHw_get_field_by_mask(larb_base, SMI_LARB_SEC_CONx(larb_port), F_SMI_SEC_MMU_EN(1));
	if(!!(mmu_en) != mmu_en) {
		M4ULOGV("m4u_config_port_sec error, port=%d, Virtuality=%d, mmu_en=%x (%x, %x)\n",
			   port, mmu_en, larb_port,M4U_ReadReg32(larb_base, SMI_LARB_SEC_CONx(larb_port)), F_SMI_SEC_MMU_EN(1));
	}
	M4ULOGV("%s, port=%d, Virtuality=%d, sec=0x%x, mmu_en=%x reg0x%x, value(%x, %x)\n", __func__, port, larb_port, sec,
			mmu_en, (larb_base+SMI_LARB_SEC_CONx(larb_port)), M4U_ReadReg32(larb_base, SMI_LARB_SEC_CONx(larb_port)), F_SMI_SEC_MMU_EN(1));

}


unsigned int tui_m4u_alloc_mva(enum M4U_PORT_ID port_id,
				const uint64_t phy_buf_addr,
				const unsigned int size)
{
	unsigned int mva_start;
	int ret;
	/*page attribute: uncacheable/secure/unshared*/
	unsigned int attribute = F_PGD_TYPE_SECTION | M4U_PROT_SEC;

	if (port_id != M4U_PORT_DISP_2L_OVL0_LARB0 &&
	    port_id != M4U_PORT_DISP_RDMA0) {
		M4ULOGE("%s: invalid port id\n", __func__);
		return 0;
	}
	/*use virtual start address to get mva*/
	mva_start = tui_m4u_do_alloc_mva(port_id, phy_buf_addr, size);

	if (!mva_start) {
		M4ULOGE("%s: m4u alloc mva failed\n", __func__);
		return 0;
	}

	/*use mva to fill section page table*/
	ret = tui_m4u_fill_pagetable(phy_buf_addr, size, mva_start, attribute);
	if (ret) {
		M4ULOGE("%s: m4u fill page table failed\n", __func__);
		return 0;
	}

	/*invalid high 4G tlb*/
	tui_m4u_invalid_tlb_all(0,1);
	return mva_start;
}

int tui_m4u_free(unsigned int mva_start, unsigned int buf_size)
{
	int ret;
	
	M4ULOGV("dealloc mva start, at mva 0x%x, size=0x%x, ret=%d\n", mva_start, buf_size, ret);
	ret = tui_m4u_clean_pagetable(mva_start, buf_size);
	if (ret) {
		M4ULOGE("clean page table error at mva 0x%x\n", mva_start);
		return -1;
	}
	ret = tui_m4u_dealloc_mva(mva_start, buf_size);
	if (ret) {
		M4ULOGE("dealloc mva graph error at mva 0x%x\n", mva_start);
		dump_mva_graph();
		return -1;
	}
	
	M4ULOGV("dealloc mva done, at mva 0x%x, size=%d, ret=%d\n", mva_start, buf_size, ret);
	return 0;
}

void tui_m4u_deinit(void)
{
	int i =0;
	int ret = 0;

	/*restore secure pgd*/
	M4U_REG_SET(g_m4u_reg_base_va[0] + REG_MMUg_PT_BASE_SEC,
		    g_m4u_reg_base_original_val);
	g_m4u_secure_status = 0;	
	M4ULOGV("%s, %d: restore the normal mva mapping table at: 0x%x\n", __func__, __LINE__, M4U_REG_GET(g_m4u_reg_base_va[0] + REG_MMUg_PT_BASE_SEC));
	/*free page table buffer*/
	drApiFree((void *)g_sec_pt_base.pagetable_base);
	/*unmap register to virtual address*/
#if 1
	drApiUnmapBuffer((void *)g_m4u_reg_base_va[0]);
	for (i = 0; i < SMI_LARB_NR; i ++) {
		ret = drApiUnmapBuffer((void *)smiLarbBaseAddr[i]);
		if (ret)
			M4ULOGE("%s, %d, failed to unmap larb%d, base addr\n", __func__, __LINE__);
	}
#endif
	M4ULOGV("%s, %d\n", __func__, __LINE__);
}

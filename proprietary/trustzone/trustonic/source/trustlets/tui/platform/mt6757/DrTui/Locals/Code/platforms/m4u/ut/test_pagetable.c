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

#include "stdio.h"
#include "stdlib.h"
#include "memory.h"
#include "tui_m4u.h"
#include "m4u_reg.h"
#include "m4u_pgtable.h"
#include "test_raw_data.h"


#define LOG_TAG "TUI_M4U"
#ifdef _DEBUG_
#define M4ULOGD(fmt, args...)	printf("[M4U debug]"fmt, ##args)
#else
#define M4ULOGD(fmt, args...)
#endif
#define M4ULOGV(fmt, args...)	printf("[M4U info]"fmt, ##args)
#define M4ULOGE(fmt, args...)	printf("["LOG_TAG"]error:"fmt, ##args)
#define ARRAY_NUMBER(array) (sizeof(array) / sizeof(array[0]))

struct m4u_pt_base {
	unsigned long pagetable_base;
	unsigned long pagetable_base_aligned;
};

static short mvaGraph[MVA_MAX_BLOCK_NR+1];
static unsigned char moduleGraph[MVA_MAX_BLOCK_NR+1];
static unsigned int g_original_reg_val = 0;

volatile unsigned long g_m4u_reg_base_va[1] = {0};
struct m4u_pt_base g_sec_pt_base;

static int test_tui_m4u_pagetable_init(void)
{
	g_sec_pt_base.pagetable_base = (unsigned long long)malloc(IMU_PGD_SIZE + IMU_PGD_ALIGN);
	if (!g_sec_pt_base.pagetable_base) {        
		M4ULOGE("Physical memory not available.\n");
		return -1;
	}
	g_sec_pt_base.pagetable_base_aligned =
		((g_sec_pt_base.pagetable_base + IMU_PGD_ALIGN) & (~IMU_PGD_ALIGN));
	memset((void *)g_sec_pt_base.pagetable_base, 0, IMU_PGD_SIZE + IMU_PGD_ALIGN);
	return 0;
}

static void test_tui_m4u_deinit(void)
{
	/*free page table buffer*/
	free((void *)g_sec_pt_base.pagetable_base);
	/*unmap register to virtual address*/
	//drApiUnmap((void *)g_m4u_reg_base_va[0]);
}

static void test_tui_m4u_mvaGraph_init(void)
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

static unsigned int test_tui_m4u_alloc_mva(ENUM_M4U_PORT_ID port_id, 
					   const unsigned long buf_addr, 
					   const unsigned int buf_size)
{
	short s,end;
	short new_start, new_end;
	short required_block_cnt = 0;
	unsigned long mva_start;
	unsigned long start_required, end_required, size_required; 
	ENUM_M4U_PORT_ID original_port_id;

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

static int test_m4u_fill_section(unsigned int mva, unsigned long pa, unsigned int prot)
{
	unsigned int pgprot;
	unsigned int padscpt;
	imu_pgd_t *pgd;
	/*write section into page table buffer directly*/
	imu_pgd_t *g_pgd = (imu_pgd_t *)g_sec_pt_base.pagetable_base_aligned;

	M4ULOGD("input attribute = %x\n", prot);
	if ((mva & (~F_PGD_PA_SECTION_MSK)) != ((unsigned int)pa & (~F_PGD_PA_SECTION_MSK))) {
		M4ULOGE("error to map section: mva=0x%x, pa=0x%lx.\n", mva, pa);
		return -1;
	}

	mva &= F_PGD_PA_SECTION_MSK;
	/*set 32bit*/
	if (pa > 0xffffffffUL) {
		padscpt = ((unsigned int)pa & F_PTE_PA_SMALL_MSK) | F_PGD_BIT32_BIT;
		M4ULOGV("set bit32 pgd seccess!\n");
	}
	else
		padscpt = (unsigned int)pa & F_PGD_PA_SECTION_MSK;

	pgprot = __m4u_get_pgd_attr_1M(prot);
	M4ULOGD("g_pgd = %p imu_pgd_index(mva) = %x\n", g_pgd, imu_pgd_index(mva));
	/*compute offset from page table buffer base address*/
	pgd = imu_pgd_offset(g_pgd, mva);
	M4ULOGD("pgd = %p\n", pgd);

	if ((imu_pgd_val(*pgd))) {
		M4ULOGE("%s: mva=0x%lx, pgd=0x%x\n", __func__, mva, imu_pgd_val(*pgd));
		return -1;
	}

	M4ULOGV("padscpt = %x pgprot = %x\n", padscpt, pgprot);
	/*write section*/
	imu_pgd_val(*pgd) = pa | padscpt | pgprot;
	return 0;
}

static int is_aligned(unsigned long addr)
{
	if (addr & M4U_VA_PAGE_MASK)
		return 0;
	return 1;
}

static void test_tui_m4u_fill_pagetable(unsigned long phy_buf_addr, 
					     unsigned int buf_size,
					     unsigned int mva_start,
					     unsigned int prot)
{
	unsigned int mva;
	unsigned int mva_end = mva_start + buf_size - 1;
	unsigned int loop = 0;

	loop = buf_size / MMU_SECTION_SIZE;
	if ((buf_size + (mva_start & M4U_VA_PAGE_MASK)) % MMU_SECTION_SIZE)
		loop += 1;
	
	M4ULOGV("mva_start = %x, mva_end = %x, buf size = %u, loop = %u\n",
		mva_start, mva_end, buf_size, loop);
	for(mva=mva_start; loop != 0; loop--) {
		M4ULOGV("mva = %x\n", mva);
		/*align to 1M*/
		mva &= F_PGD_PA_SECTION_MSK;
		phy_buf_addr &= F_PGD_PA_SECTION_MSK;
		test_m4u_fill_section(mva, phy_buf_addr, prot);
		mva += MMU_SECTION_SIZE;
		phy_buf_addr+=MMU_SECTION_SIZE;
	}
}

int test_tui_m4u_Init(void)
{
	int ret;
	
	ret = test_tui_m4u_pagetable_init();
	if (ret) {
		M4ULOGE("init page table failed.\n");
		return -1;
	}

	test_tui_m4u_mvaGraph_init();

	return 0;
}

unsigned int test_tui_m4u_translate_64_to_32(ENUM_M4U_PORT_ID port_id,
					      const unsigned long virt_addr_start,
					      const unsigned long phy_addr_start,
					      const unsigned int buf_size)
{
	int ret;
	unsigned int mva_start;
	/*page attribute: uncacheable/secure/unshared*/
	unsigned int attribute = F_PGD_TYPE_SECTION | M4U_PROT_SEC;

	M4ULOGV("*****start to translate with page type of section*****\n");	
	M4ULOGV("%s%16s%16s%16s\n", "port", "virt_start", "phy_start", "buf_size");
	M4ULOGV("%d%20p%17p%10u\n", port_id, (void *)virt_addr_start,
		(void *)phy_addr_start, buf_size);

	/*use virtual start address to get mva*/
	mva_start = test_tui_m4u_alloc_mva(port_id, virt_addr_start, buf_size);
	if (!mva_start) {
		M4ULOGE("%s: m4u alloc mva failed\n", __func__);
		return 0;
	}
	M4ULOGV("mva start = 0x%x, page attribute = 0x%x\n", mva_start, attribute);

	/*use mva to fill section page table*/
	test_tui_m4u_fill_pagetable(phy_addr_start, buf_size, mva_start, attribute);
	M4ULOGV("*****finish to translate with page type of section*****\n\n");
	return mva_start;
}

void test_tui_m4u_exit(void)
{
	test_tui_m4u_deinit();
}

static void dump_mva_graph()
{
	int index = 1, i;
	int count;
	ENUM_M4U_PORT_ID owner;
	char *status;

	M4ULOGV("***************dump mva graph****************\n");
	M4ULOGV("graph[%4s]%8s%8s%8s\n", "index", "count", "status", "owner");
	for (index = 1; index <= MVA_MAX_BLOCK_NR;) {
		if (MVA_IS_BUSY(index))
			status= "busy";
		else
			status= "free";
		owner = (ENUM_M4U_PORT_ID)moduleGraph[index];
		count = MVA_GET_NR(index);
		M4ULOGV("graph[%04u]%8u%8s%8u\n", index, count, status, owner);
		index += count;
	}

	M4ULOGV("*************dump mva graph done*************\n\n");
}

/*dump page table by mva status. only print one at busy status.*/
static void dump_pagetable_in_use(void)
{
	/*our mva graph start with 1 not 0, so the index should start with 16th pgd.*/
	unsigned int pt_start_idx = 0x10, pt_offset = 0x10;
	unsigned int pt_jump_count = 0;
	unsigned int* p =
		(unsigned int*)g_sec_pt_base.pagetable_base_aligned + pt_start_idx;
	unsigned int* tmp_end;
	unsigned int block_count = 0;
	int graph_index;

	M4ULOGV("*************dump busy page table*************\n");
	M4ULOGV("%12s%20s%20s\n","graph index","pt address","descriptor");
	for (graph_index = 1; graph_index <= MVA_MAX_BLOCK_NR; graph_index++) {
		/*get block count at current graph index*/
		block_count = MVA_GET_NR(graph_index);
		/*get pt pointer next start address*/
		pt_jump_count = block_count * pt_offset;
		tmp_end = p + pt_jump_count;

		/*ignore free blocks*/
		if (!MVA_IS_BUSY(graph_index)) {
			p = tmp_end;
			M4ULOGD("ignore mva graph %u\n", graph_index);
			continue;
		}
		for (; p < tmp_end; p++) {
			M4ULOGV("%12u%20p%20x\n", graph_index, p, *p);
		}
		p = tmp_end;
	}
	M4ULOGV("***********dump busy page table done***********\n\n");

}

static void dump_pagetable_by_interval_16M()
{
	/*our mva graph start with 1 not 0, so the index should start with 16th pgd.
	 * mva = mva graph start index << 24 + VA & 0xfff
	 * pgd = g_pgd + mva >> 20
	 * so, the interval of pgd is 16, and pointer offset is 16 * 4 = 64
	 * the interval of mva graph elements is 16M
	 */
	int i = 0x10, offset = 0x10;
	unsigned int* p = (unsigned int*)g_sec_pt_base.pagetable_base_aligned;

	M4ULOGV("*****dump page table*****\n");
	for (; i < (IMU_PGD_SIZE / 4);) {
		if (!(p[i]))
			break;
		M4ULOGV("PGD[%d] = 0x%x  ", i, p[i]);
		i += offset;
		if (i % (4 * offset))
			M4ULOGV("\n");
	}
	M4ULOGV("*****dump page table done*****\n");
}

static void dump_pagetable_from_16_to_33()
{
	/*our mva graph start with 1 not 0, so the index should start with 16th pgd.*/

	int i = 0x10;
	unsigned int* p = (unsigned int*)g_sec_pt_base.pagetable_base_aligned;

	M4ULOGV("*****dump page table*****\n");
	for (; i < 33;) {
		M4ULOGV("PGD[%d] = 0x%x  \n", i, p[i]);
		i++;
	}
	M4ULOGV("*****dump page table done*****\n");

}

static int test_tui_m4u_dealloc_mva(unsigned int mva, unsigned int size)
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


static int test_tui_m4u_clean_section(unsigned int mva)
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


static int test_tui_m4u_clean_pagetable(unsigned int mva_start, unsigned int buf_size)
{
	unsigned int mva;
	unsigned int mva_end = mva_start + buf_size - 1;
	
	for(mva = mva_start; mva <= mva_end; mva += MMU_SECTION_SIZE) {
		if(test_tui_m4u_clean_section(mva))
			return -1;
	}
	return 0;
}

static int test_tui_m4u_recycle(unsigned int mva_start, unsigned int buf_size)
{
	int ret;
	ret = test_tui_m4u_clean_pagetable(mva_start, buf_size);
	if (ret) {
		M4ULOGE("clean page table error at mva 0x%x\n", mva_start);
		return -1;
	}
	ret = test_tui_m4u_dealloc_mva(mva_start, buf_size);
	if (ret) {
		M4ULOGE("dealloc mva graph error at mva 0x%x\n", mva_start);
		dump_mva_graph();
		return -1;
	}
	return 0;
}

/*translate 1 buffer which size is more than 1M. (5M)*/
static int test_case_1()
{
	int ret;
	unsigned int mva_start = 0;
	unsigned int size = 0x500000 + 0x10;
	mva_start = test_tui_m4u_translate_64_to_32(M4U_PORT_DISP_2L_OVL0_LARB1,
					g_array_virtual_addr[0],
					g_array_physical_addr[0],
					size);
	dump_mva_graph();
	dump_pagetable_in_use();

	ret = test_tui_m4u_recycle(mva_start, size);
	if (ret) {
		M4ULOGE("recycle error: mva = 0x%x", mva_start);
		return -1;
	}

	return 0;
}


/*translate 17 buffers which size is less than 1M*/
static int test_case_2()
{
	int ret;
	unsigned int mva_start_array[ARRAY_NUMBER(g_array_virtual_addr)] = {0};
	unsigned int size = 0x20000, i;
	for (i = 0; i < ARRAY_NUMBER(g_array_virtual_addr); i++) {
		mva_start_array[i] = test_tui_m4u_translate_64_to_32(M4U_PORT_DISP_2L_OVL0_LARB1,
								     g_array_virtual_addr[i],
								     g_array_physical_addr[i],
								     0x20000);
	}
	dump_mva_graph();
	dump_pagetable_in_use();

	for (i = 0; i < ARRAY_NUMBER(g_array_virtual_addr); i++) {
		M4ULOGV("recycle mva = %x\n", mva_start_array[i]);
		ret = test_tui_m4u_recycle(mva_start_array[i], size);
		if (ret) {
			M4ULOGE("recycle error: mva[%u] = 0x%x\n", i, mva_start_array[i]);
			return -1;
		}
			
	}
	return 0;
}

static void help(char *bin_name)
{
	M4ULOGV("***************TUI M4U UT usage*****************\n", bin_name);
	M4ULOGV("%s [case id]\n", bin_name);
	M4ULOGV("case id: 1\n%60s\n", "translate 1 buffer which size is more than 1M. (5M)");
	M4ULOGV("case id: 2\n%56s\n", "translate 17 buffers which size is less than 1M");
}
int main(int argc, char **argv)
{
	int ret;
	int i = 0;

	if (argc != 2 || (atoi(argv[1]) > 2 || atoi(argv[1]) < 1)) {
		help(argv[0]);
		return -1;
	}
	
	M4ULOGV("===================init m4u====================\n");
	ret = test_tui_m4u_Init();
	if (ret)
		return -1;
	dump_mva_graph();
	M4ULOGV("============start case verification============\n");
	/*init complete, start test case*/
	switch(atoi(argv[1])) {
	case 1:
		test_case_1();
		break;
	case 2:
		test_case_2();
		break;
	default:
		break;
	}
	
	M4ULOGV("==========check if resouce is recycled==========\n");
	dump_mva_graph();
	dump_pagetable_in_use();
	/*test case over, recycle resource*/
	test_tui_m4u_exit();
	M4ULOGV("===================test over====================\n");
}


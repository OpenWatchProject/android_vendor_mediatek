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

#ifndef __M4U_PGTABLE_H__
#define __M4U_PGTABLE_H__

/*define mva graph*/
#define M4U_VA_PAGE_MASK 0xfff
#define M4U_VA_PAGE_SIZE   0x1000 //4KB
/*need to add page offset before return mva start address*/
#define MVA_PAGE_OFFSET(mva) ((mva)&0xfffff)


#define MVA_BLOCK_SIZE_ORDER     24     //16M
#define MVA_MAX_BLOCK_NR        255    //4GB

#define MVA_BLOCK_SIZE      (1<<MVA_BLOCK_SIZE_ORDER)  //0x1000000 
#define MVA_BLOCK_ALIGN_MASK (MVA_BLOCK_SIZE-1)        //0xffffff
#define MVA_BLOCK_NR_MASK   (MVA_MAX_BLOCK_NR)      //0xff
#define MVA_BUSY_MASK       (1<<15)                 //0x8000

#define MVA_IS_BUSY(index) ((mvaGraph[index]& MVA_BUSY_MASK)!=0)
#define MVA_SET_BUSY(index) (mvaGraph[index] |= MVA_BUSY_MASK)
#define MVA_SET_FREE(index) (mvaGraph[index] & (~MVA_BUSY_MASK))
#define MVA_GET_NR(index)   (mvaGraph[index] & MVA_BLOCK_NR_MASK)
#define MVAGRAPH_INDEX(mva) (mva>>MVA_BLOCK_SIZE_ORDER)

/*define pgd*/
#define IMU_PGD_SIZE  (16*1024)
#define IMU_PGD_ALIGN  ((16*1024)-1)


/* ================================================================= */
/* 2 level pagetable: pgd -> pte */

#define F_PTE_TYPE_MSK          F_MSK(1, 0)
#define F_PTE_TYPE_SET(val)     F_VAL(val, 1, 0)
#define F_PTE_TYPE_GET(regval)  F_MSK_SHIFT(regval, 1, 0)
#define F_PTE_TYPE_LARGE        (0x1)
#define F_PTE_TYPE_SMALL        (0x2)
#define F_PTE_B_BIT             F_BIT_SET(2)
#define F_PTE_C_BIT             F_BIT_SET(3)
#define F_PTE_BIT33_BIT         F_BIT_SET(4)
#define F_PTE_TEX_MSK            F_MSK(8, 6)
#define F_PTE_TEX_SET(val)       F_VAL(val, 8, 6)
#define F_PTE_TEX_GET(regval)    F_MSK_SHIFT(regval, 8, 6)
#define F_PTE_BIT32_BIT         F_BIT_SET(9)
#define F_PTE_S_BIT             F_BIT_SET(10)
#define F_PTE_NG_BIT            F_BIT_SET(11)
#define F_PTE_PA_LARGE_MSK            F_MSK(31, 16)
#define F_PTE_PA_LARGE_SET(val)       F_VAL(val, 31, 16)
#define F_PTE_PA_LARGE_GET(regval)    F_MSK_SHIFT(regval, 31, 16)
#define F_PTE_PA_SMALL_MSK            F_MSK(31, 12)
#define F_PTE_PA_SMALL_SET(val)       F_VAL(val, 31, 12)
#define F_PTE_PA_SMALL_GET(regval)    F_MSK_SHIFT(regval, 31, 12)
#define F_PTE_TYPE_IS_LARGE_PAGE(pte) ((imu_pte_val(pte)&0x3) == F_PTE_TYPE_LARGE)
#define F_PTE_TYPE_IS_SMALL_PAGE(pte)   ((imu_pte_val(pte)&0x3) == F_PTE_TYPE_SMALL)


#define F_PGD_TYPE_PAGE         (0x1)
#define F_PGD_TYPE_PAGE_MSK     (0x3)
#define F_PGD_TYPE_SECTION      (0x2)
#define F_PGD_TYPE_SUPERSECTION   (0x2|(1<<18))
#define F_PGD_TYPE_SECTION_MSK  (0x3|(1<<18))
#define F_PGD_TYPE_IS_PAGE(pgd) ((imu_pgd_val(pgd)&3) == 1)
#define F_PGD_TYPE_IS_SECTION(pgd) \
	(F_PGD_TYPE_IS_PAGE(pgd) ? 0 : ((imu_pgd_val(pgd)&F_PGD_TYPE_SECTION_MSK) == F_PGD_TYPE_SECTION))
#define F_PGD_TYPE_IS_SUPERSECTION(pgd) \
	(F_PGD_TYPE_IS_PAGE(pgd) ? 0 : ((imu_pgd_val(pgd)&F_PGD_TYPE_SECTION_MSK) == F_PGD_TYPE_SUPERSECTION))

#define F_PGD_B_BIT             F_BIT_SET(2)
#define F_PGD_C_BIT             F_BIT_SET(3)
#define F_PGD_BIT33_BIT         F_BIT_SET(4)
#define F_PGD_AP_MSK            F_MSK(11, 10)
#define F_PGD_AP_SET(val)       F_VAL(val, 11, 10)
#define F_PGD_AP_GET(regval)    F_MSK_SHIFT(regval, 11, 10)
#define F_PGD_TEX_MSK            F_MSK(14, 12)
#define F_PGD_TEX_SET(val)       F_VAL(val, 14, 12)
#define F_PGD_TEX_GET(regval)    F_MSK_SHIFT(regval, 14, 12)
#define F_PGD_BIT32_BIT         F_BIT_SET(9)
#define F_PGD_S_BIT             F_BIT_SET(16)
#define F_PGD_NG_BIT            F_BIT_SET(17)
#define F_PGD_NS_BIT_PAGE(ns)   F_BIT_VAL(ns, 3)
#define F_PGD_NS_BIT_SECTION(ns)    F_BIT_VAL(ns, 19)
#define F_PGD_NS_BIT_SUPERSECTION(ns)    F_BIT_VAL(ns, 19)

#define F_PGD_PA_PAGETABLE_MSK            F_MSK(31, 10)
#define F_PGD_PA_PAGETABLE_SET(val)       F_VAL(val, 31, 10)
#define F_PGD_PA_SECTION_MSK            F_MSK(31, 20)
#define F_PGD_PA_SECTION_SET(val)       F_VAL(val, 31, 20)
#define F_PGD_PA_SUPERSECTION_MSK            F_MSK(31, 24)
#define F_PGD_PA_SUPERSECTION_SET(val)       F_VAL(val, 31, 24)

/* pagetable walk */
#define IMU_PGDIR_SHIFT   20
#define IMU_PAGE_SHIFT   12
#define IMU_PTRS_PER_PGD 4096
#define IMU_PTRS_PER_PTE 256
#define IMU_BYTES_PER_PTE (IMU_PTRS_PER_PTE*sizeof(imu_pteval_t))

#define MMU_PT_TYPE_SUPERSECTION    (1<<4)
#define MMU_PT_TYPE_SECTION         (1<<3)
#define MMU_PT_TYPE_LARGE_PAGE      (1<<2)
#define MMU_PT_TYPE_SMALL_PAGE      (1<<1)

#define MMU_SMALL_PAGE_SIZE     0x1000 //4KB
#define MMU_LARGE_PAGE_SIZE     (16*MMU_SMALL_PAGE_SIZE) //64KB
#define MMU_SECTION_SIZE        (1*1024*1024) //1M
#define MMU_SUPERSECTION_SIZE  (16*MMU_SECTION_SIZE) //16M

#define M4U_GET_PAGE_NUM(va, size) ((((va)&(MMU_SECTION_SIZE-1))+(size)+(MMU_SECTION_SIZE-1))>>20)
//#define MMU_SMALL_PAGE_SIZE     (SZ_4K)
//#define MMU_LARGE_PAGE_SIZE     (SZ_64K)
//#define MMU_SECTION_SIZE        (SZ_1M)
//#define MMU_SUPERSECTION_SIZE  (SZ_16M)

/*page attribute*/
#define M4U_PROT_READ	(1<<0)  /* buffer can be read by engine */
#define M4U_PROT_WRITE	(1<<1)  /* buffer can be write by engine */
#define M4U_PROT_CACHE	(1<<2)  /* buffer access will goto CCI to do cache snoop */
#define M4U_PROT_SHARE	(1<<3)  /* buffer access will goto CCI, but don't do cache snoop*/
							/*	(just for engines who wants to use CCI bandwidth) */
#define M4U_PROT_SEC    (1<<4)  /* buffer can only be accessed by secure engine. */


typedef unsigned int imu_pteval_t;
typedef struct {imu_pteval_t imu_pte; } imu_pte_t;
typedef struct {imu_pteval_t imu_pgd; } imu_pgd_t;

#define imu_pte_val(x)      ((x).imu_pte)
#define imu_pgd_val(x)      ((x).imu_pgd)

#define __imu_pte(x)    ((imu_pte_t){(x)})
#define __imu_pgd(x)    ((imu_pgd_t){(x)})

#define imu_pte_none(pte)   (!imu_pte_val(pte))
#define imu_pte_type(pte)   (imu_pte_val(pte)&0x3)

#define imu_pgd_index(addr)		((addr) >> IMU_PGDIR_SHIFT)
#define imu_pgd_offset(gpgd, addr) (gpgd + imu_pgd_index(addr))

#define imu_pte_index(addr)		(((addr)>>IMU_PAGE_SHIFT)&(IMU_PTRS_PER_PTE - 1))
#define imu_pte_offset_map(pgd, addr) (imu_pte_map(pgd) + imu_pte_index(addr))

static inline unsigned int __m4u_get_pgd_attr_1M(unsigned int prot)
{
	unsigned int pgprot;

	pgprot = F_PGD_TYPE_SECTION;
	pgprot |= (prot & M4U_PROT_SEC) ? 0 : F_PGD_NS_BIT_SECTION(1);
	pgprot |= (prot & M4U_PROT_SHARE) ? F_PGD_S_BIT : 0;
	pgprot |= (prot & M4U_PROT_CACHE) ? (F_PGD_C_BIT | F_PGD_B_BIT) : 0;
	return pgprot;
}
#endif

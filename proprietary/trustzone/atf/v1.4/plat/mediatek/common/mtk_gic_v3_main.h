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
 * MediaTek Inc. (C) 2010. All rights reserved.
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

/******************************************************************************
 *
 *  Copyright Statement:
 *  --------------------
 *  This software is protected by Copyright and the information contained
 *  herein is confidential. The software may not be copied and the information
 *  contained herein may not be used or disclosed except with the written
 *  permission of MediaTek Inc. (C) 2001
 *
 *******************************************************************************/

#ifndef __MTK_GIC_V3_HELPERS_H__
#define __MTK_GIC_V3_HELPERS_H__
#include <mmio.h>
#ifndef MAX_GIC_NR
#define MAX_GIC_NR			(1)
#endif

#define MAX_RDIST_NR			(64)

#define DIV_ROUND_UP(n, d)		(((n) + (d) - 1) / (d))

#define SZ_64K				(0x00010000)

/* since gcc not support most ARMv8 ICC sysreg in asm,
 * we learn Linux's way to encode them
 */
#define sys_reg(op0, op1, crn, crm, op2) \
				((((op0)-2)<<19)|((op1)<<16)|((crn)<<12)|((crm)<<8)|((op2)<<5))


#define ICC_GRPEN0_EL1                  sys_reg(3, 0, 12, 12, 6)
#define ICC_GRPEN1_EL1                  sys_reg(3, 0, 12, 12, 7)
#define ICC_GRPEN1_EL3                  sys_reg(3, 6, 12, 12, 7)
#define ICC_IAR0_EL1_			sys_reg(3, 0, 12,  8, 0)
#define ICC_HPPIR0_EL1_			sys_reg(3, 0, 12,  8, 2)
#define ICC_HPPIR1_EL1_			sys_reg(3, 0, 12, 12, 2)
#define ICC_SGI0R_EL1			sys_reg(3, 0, 12, 11, 7)
#define ICC_EOIR0_EL1_			sys_reg(3, 0, 12,  8, 1)
/* end of ICC sysreg encode macros */

/* gic600 */
#define GICR_PWRR		0x0024

/* GICR_PWRR fields */
#define PWRR_RDPD_SHIFT		0
#define PWRR_RDGPD_SHIFT	2
#define PWRR_RDGPO_SHIFT	3

#define PWRR_RDGPD		(1 << PWRR_RDGPD_SHIFT)
#define PWRR_RDGPO		(1 << PWRR_RDGPO_SHIFT)

/* Values to write to GICR_PWRR register to power redistributor */
#define PWRR_ON			(0 << PWRR_RDPD_SHIFT)
#define PWRR_OFF		(1 << PWRR_RDPD_SHIFT)


/* distributor registers & their field definitions, in secure world */

#define GICD_V3_CTLR                       0x0000
#define GICD_V3_TYPER                      0x0004
#define GICD_V3_IIDR                       0x0008
#define GICD_V3_STATUSR                    0x0010
#define GICD_V3_SETSPI_NSR                 0x0040
#define GICD_V3_CLRSPI_NSR                 0x0048
#define GICD_V3_SETSPI_SR                  0x0050
#define GICD_V3_CLRSPI_SR                  0x0058
#define GICD_V3_SEIR                       0x0068
#define GICD_V3_ISENABLER                  0x0100
#define GICD_V3_ICENABLER                  0x0180
#define GICD_V3_ISPENDR                    0x0200
#define GICD_V3_ICPENDR                    0x0280
#define GICD_V3_ISACTIVER                  0x0300
#define GICD_V3_ICACTIVER                  0x0380
#define GICD_V3_IPRIORITYR                 0x0400
#define GICD_V3_ICFGR                      0x0C00
#define GICD_V3_IROUTER                    0x6000
#define GICD_V3_PIDR2                      0xFFE8

#define GICD_V3_CTLR_RWP                   (1U << 31)
#define GICD_V3_CTLR_E1NWF			(1U << 7)
#define GICD_V3_CTLR_DS			(1U << 6)
#define GICD_V3_CTLR_ARE_NS                (1U << 5)
#define GICD_V3_CTLR_ARE_S                 (1U << 4)
#define GICD_V3_CTLR_ENABLE_G1S            (1U << 2)
#define GICD_V3_CTLR_ENABLE_G1NS           (1U << 1)
#define GICD_V3_CTLR_ENABLE_G0		(1U << 0)

#define GICD_V3_TYPER_ID_BITS(typer)       ((((typer) >> 19) & 0x1f) + 1)
#define GICD_V3_TYPER_IRQS(typer)          ((((typer) & 0x1f) + 1) * 32)
#define GICD_V3_TYPER_LPIS                 (1U << 17)

#define GICD_V3_IIDR_PROD_ID_SHIFT	(24)
#define GICD_V3_IIDR_GIC500	(0x0)
#define GICD_V3_IIDR_GIC600	(0x2)

#define GICD_V3_IROUTER_SPI_MODE_ONE       (0U << 31)
#define GICD_V3_IROUTER_SPI_MODE_ANY       (1U << 31)

#define GIC_V3_PIDR2_ARCH_MASK             0xf0
#define GIC_V3_PIDR2_ARCH_GICv3            0x30
#define GIC_V3_PIDR2_ARCH_GICv4            0x40

/*
 * Re-Distributor registers, offsets from RD_base
 */
#define GICR_V3_CTLR                       GICD_V3_CTLR
#define GICR_V3_IIDR                       0x0004
#define GICR_V3_TYPER                      0x0008
#define GICR_V3_STATUSR                    GICD_V3_STATUSR
#define GICR_V3_WAKER                      0x0014
#define GICR_V3_SETLPIR                    0x0040
#define GICR_V3_CLRLPIR                    0x0048
#define GICR_V3_SEIR                       GICD_V3_SEIR
#define GICR_V3_PROPBASER                  0x0070
#define GICR_V3_PENDBASER                  0x0078
#define GICE_V3_IGROUP0			   0x0080
#define GICR_V3_INVLPIR                    0x00A0
#define GICR_V3_INVALLR                    0x00B0
#define GICR_V3_SYNCR                      0x00C0
#define GICR_V3_MOVLPIR                    0x0100
#define GICR_V3_MOVALLR                    0x0110
#define GICE_V3_IGRPMOD0		   0x0d00
#define GICR_V3_PIDR2                      GICD_V3_PIDR2

#define GICR_V3_CTLR_ENABLE_LPIS           (1UL << 0)
#define GICR_V3_CTLR_DPG1NS                (1UL << 25)
#define GICR_V3_CTLR_UWP                   (1UL << 31)
#define GICR_V3_CTLR_RWP                   (1UL << 3)

#define GICR_V3_TYPER_CPU_NUMBER(r)        (((r) >> 8) & 0xffff)

#define GICR_V3_WAKER_Sleep		   (1U << 0)
#define GICR_V3_WAKER_ProcessorSleep       (1U << 1)
#define GICR_V3_WAKER_ChildrenAsleep       (1U << 2)
#define GICR_V3_WAKER_Quiescent		   (1U << 31)

#define GICR_V3_PROPBASER_NonShareable     (0U << 10)
#define GICR_V3_PROPBASER_InnerShareable   (1U << 10)
#define GICR_V3_PROPBASER_OuterShareable   (2U << 10)
#define GICR_V3_PROPBASER_SHAREABILITY_MASK (3UL << 10)
#define GICR_V3_PROPBASER_nCnB             (0U << 7)
#define GICR_V3_PROPBASER_nC               (1U << 7)
#define GICR_V3_PROPBASER_RaWt             (2U << 7)
#define GICR_V3_PROPBASER_RaWb             (3U << 7)
#define GICR_V3_PROPBASER_WaWt             (4U << 7)
#define GICR_V3_PROPBASER_WaWb             (5U << 7)
#define GICR_V3_PROPBASER_RaWaWt           (6U << 7)
#define GICR_V3_PROPBASER_RaWaWb           (7U << 7)
#define GICR_V3_PROPBASER_IDBITS_MASK      (0x1f)

/*
 * Re-Distributor registers, offsets from SGI_base
 */
#define GICR_V3_ISENABLER0                 GICD_V3_ISENABLER
#define GICR_V3_ICENABLER0                 GICD_V3_ICENABLER
#define GICR_V3_ISPENDR0                   GICD_V3_ISPENDR
#define GICR_V3_ICPENDR0                   GICD_V3_ICPENDR
#define GICR_V3_ISACTIVER0                 GICD_V3_ISACTIVER
#define GICR_V3_ICACTIVER0                 GICD_V3_ICACTIVER
#define GICR_V3_IPRIORITYR0                GICD_V3_IPRIORITYR
#define GICR_V3_ICFGR0                     GICD_V3_ICFGR

#define GICR_V3_TYPER_PLPIS                (1U << 0)
#define GICR_V3_TYPER_VLPIS                (1U << 1)
#define GICR_V3_TYPER_LAST                 (1U << 4)
#define GICR_V3_TYPER_DPGS                 (1U << 5)

/*
 * CPU interface registers
 */
#define ICC_V3_CTLR_EL1_EOImode_drop_dir   (0U << 1)
#define ICC_V3_CTLR_EL1_EOImode_drop       (1U << 1)
#define ICC_V3_SRE_EL1_SRE                 (1U << 0)

/*
 * Hypervisor interface registers (SRE only)
 */
#define ICH_V3_LR_VIRTUAL_ID_MASK          ((1UL << 32) - 1)

#define ICH_V3_LR_EOI                      (1UL << 41)
#define ICH_V3_LR_GROUP                    (1UL << 60)
#define ICH_V3_LR_STATE                    (3UL << 62)
#define ICH_V3_LR_PENDING_BIT              (1UL << 62)
#define ICH_V3_LR_ACTIVE_BIT               (1UL << 63)

#define ICH_V3_MISR_EOI                    (1 << 0)
#define ICH_V3_MISR_U                      (1 << 1)

#define ICH_V3_HCR_EN                      (1 << 0)
#define ICH_V3_HCR_UIE                     (1 << 1)

#define ICH_V3_VMCR_CTLR_SHIFT             0
#define ICH_V3_VMCR_CTLR_MASK              (0x21f << ICH_VMCR_CTLR_SHIFT)
#define ICH_V3_VMCR_BPR1_SHIFT             18
#define ICH_V3_VMCR_BPR1_MASK              (7 << ICH_VMCR_BPR1_SHIFT)
#define ICH_V3_VMCR_BPR0_SHIFT             21
#define ICH_V3_VMCR_BPR0_MASK              (7 << ICH_VMCR_BPR0_SHIFT)
#define ICH_V3_VMCR_PMR_SHIFT              24
#define ICH_V3_VMCR_PMR_MASK               (0xffUL << ICH_VMCR_PMR_SHIFT)

/* TODO: replace sys_reg macro */
#define ICC_V3_EOIR1_EL1                   sys_reg(3, 0, 12, 12, 1)
#define ICC_V3_IAR1_EL1                    sys_reg(3, 0, 12, 12, 0)
#define ICC_V3_SGI1R_EL1                   sys_reg(3, 0, 12, 11, 5)
#define ICC_V3_PMR_EL1                     sys_reg(3, 0, 4, 6, 0)
#define ICC_V3_CTLR_EL1                    sys_reg(3, 0, 12, 12, 4)
#define ICC_V3_SRE_EL1                     sys_reg(3, 0, 12, 12, 5)
#define ICC_V3_GRPEN1_EL1                  sys_reg(3, 0, 12, 12, 7)

#define ICC_V3_IAR1_EL1_SPURIOUS           0x3ff

#define ICC_V3_SRE_EL2                     sys_reg(3, 4, 12, 9, 5)

#define ICC_V3_SRE_EL2_SRE                 (1 << 0)
#define ICC_V3_SRE_EL2_ENABLE              (1 << 3)

static inline unsigned int gicd_v3_read_ctlr(unsigned int base)
{
	return mmio_read_32(base + GICD_V3_CTLR);
}

static inline void gicd_v3_write_ctlr(unsigned int base, unsigned int val)
{
	mmio_write_32(base + GICD_V3_CTLR, val);
}

static inline unsigned int gicd_v3_read_pidr2(unsigned int base)
{
	return mmio_read_32(base + GICD_V3_PIDR2);
}

static inline void gicd_v3_set_irouter(unsigned int base, unsigned int id, uint64_t aff)
{
	unsigned int reg = base + GICD_V3_IROUTER + (id*8);

	mmio_write_64(reg, aff);
}
enum irq_schedule_mode {
	SW_MODE,
	HW_MODE,
};


#define MPIDR_LEVEL_BITS_SHIFT  3
#define MPIDR_LEVEL_BITS        (1 << MPIDR_LEVEL_BITS_SHIFT)
#define MPIDR_LEVEL_MASK ((1 << MPIDR_LEVEL_BITS) - 1)

#define MPIDR_LEVEL_SHIFT(level) \
			(((1 << level) >> 1) << MPIDR_LEVEL_BITS_SHIFT)

#define MPIDR_AFFINITY_LEVEL(mpidr, level) \
			((mpidr >> MPIDR_LEVEL_SHIFT(level)) & MPIDR_LEVEL_MASK)

#define CPU_LOGIC_MAP_RESET_VAL	(-1L)

#define SGI_MASK 0xffff

struct gic_chip_data {
	unsigned int saved_enable[DIV_ROUND_UP(1020, 32)];
	unsigned int saved_conf[DIV_ROUND_UP(1020, 16)];
	unsigned int saved_priority[DIV_ROUND_UP(1020, 4)];
	uint64_t saved_target[DIV_ROUND_UP(1020, 1)];
	unsigned int saved_group[DIV_ROUND_UP(1020, 32)];
	unsigned int saved_grpmod[DIV_ROUND_UP(1020, 32)];
	unsigned int rdist_base[MAX_RDIST_NR];
#if CFG_MICROTRUST_TEE_SUPPORT
	unsigned int saved_spi_pending[DIV_ROUND_UP(1020, 32)];
#endif
	unsigned int saved_active_sel;
	unsigned int saved_sgi[PLATFORM_CORE_COUNT];
};

extern uint64_t cpu_logical_map[PLATFORM_CORE_COUNT];
extern struct gic_chip_data gic_data[MAX_GIC_NR];

extern int gic_cpuif_init(void);
extern void gic_distif_init(unsigned int gicd_base);
extern void gic600_rdistif_init(void);
extern void gic_rdist_restore_all(void);
extern void gic600_rdistif_init(void);
extern void gic_rdist_save(void);
extern void gic_rdist_restore(void);
extern void gic_cpuif_deactivate(unsigned int gicc_base);
extern void gic_dist_save(void);
extern void gic_dist_restore(void);
extern void gic_setup(void);
extern void gic_sgi_save_all(void);
extern void gic_sgi_restore_all(void);
extern void gic600_rdistif_deactivate(void);

extern void setup_int_schedule_mode(enum irq_schedule_mode mode, unsigned int active_cpu);
extern void int_schedule_mode_save(void);
extern void int_schedule_mode_restore(void);
extern void gicd_set_ispendr(uintptr_t, unsigned int);

extern unsigned int gicd_read_iidr(unsigned int gicd_base);
extern void gic_dpg_cfg_set(void);
extern void gic_dpg_cfg_clr(void);

extern void mt_trigger_md_wdt_irq(void);

#if CFG_MICROTRUST_TEE_SUPPORT
extern void teei_gic_setup(void);
#endif

#endif

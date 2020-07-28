/*
 * Copyright (c) 2014-2015, ARM Limited and Contributors. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * Redistributions of source code must retain the above copyright notice, this
 * list of conditions and the following disclaimer.
 *
 * Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation
 * and/or other materials provided with the distribution.
 *
 * Neither the name of ARM nor the names of its contributors may be used
 * to endorse or promote products derived from this software without specific
 * prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef __PLATFORM_DEF_H__
#define __PLATFORM_DEF_H__

#include <arch.h>

#include <memory_layout.h>
#define PLAT_PRIMARY_CPU	0x0
#define PLAT_USE_EL1_AARCH32

/* Special value used to verify platform parameters from BL2 to BL3-1 */
#define MT_BL31_PLAT_PARAM_VAL	0x0f1e2d3c4b5a6978ULL

#define IO_PHYS          (0x10000000)
#define INFRACFG_AO_BASE (IO_PHYS + 0x1000)
#define PMIC_WRAP_BASE   (IO_PHYS + 0xD000)
#define RGU_BASE         (IO_PHYS + 0x7000)
#define SECURE_AO_BASE   (IO_PHYS + 0x0001a000)
#define I2C7_BASE_SE     (IO_PHYS + 0x101a000)
#define I2C4_BASE_SE     (IO_PHYS + 0x1011000)
#define I2C6_BASE_SE     (IO_PHYS + 0x1005000)
#define I2C2_BASE_SE     (IO_PHYS + 0x1009000)
#define GPIO_BASE        (IO_PHYS + 0x5000)
#define EMI_CEN_BASE	 (IO_PHYS + 0x219000)
#define EMI_MPU_BASE     (IO_PHYS + 0x226000)
#define SPM_BASE         (IO_PHYS + 0x6000)
#define DVFSRC_BASE      (IO_PHYS + 0x12000)

#define PERI_BASE		(IO_PHYS + 0x3000)
#define TRNG_base		(IO_PHYS + 0x20F000)
#define UART0_BASE		(IO_PHYS + 0x01002000)

#define MT_GIC_BASE			(0x0C000000)
#define MCUCFG_BASE			(0x0C530000)
#define PLAT_MT_CCI_BASE (0x10390000)
#define CCI_SIZE         (0x00010000)

#define APMIXEDSYS		 (IO_PHYS + 0xC000)
#define ARMPLL_LL_CON0   (APMIXEDSYS + 0x200)
#define ARMPLL_L_CON0    (APMIXEDSYS + 0x210)
#define MAINPLL_CON0     (APMIXEDSYS + 0x220)
#define CCIPLL_CON0      (APMIXEDSYS + 0x290)

#define M4U_SECURE_BANK_MM   (IO_PHYS + 0x224000)
#define M4U_SECURE_BANK_VPU  (IO_PHYS + 0x253000)

#define M4U_VPU_BANK1    (IO_PHYS + 0x250000) /* protect 1 */
#define M4U_VPU_BANK2    (IO_PHYS + 0x251000) /* protect 2 */
#define M4U_VPU_BANK3    (IO_PHYS + 0x252000) /* protect 3 */
#define M4U_VPU_BANK4    (IO_PHYS + 0x253000) /* secure */

#define M4U_MM_BANK1    (IO_PHYS + 0x221000) /* protect 1 */
#define M4U_MM_BANK2    (IO_PHYS + 0x222000) /* protect 2 */
#define M4U_MM_BANK3    (IO_PHYS + 0x223000) /* protect 3 */
#define M4U_MM_BANK4    (IO_PHYS + 0x224000) /* secure */

#define armpll_mux1_sel_big_mask	(0xF<<4)
#define armpll_mux1_sel_big_ARMSPLL	(0x1<<4)
#define armpll_mux1_sel_sml_mask	(0xF<<8)
#define armpll_mux1_sel_sml_ARMSPLL	(0x1<<8)


/* Aggregate of all devices in the first GB */
#define MTK_DEV_RNG0_BASE	IO_PHYS
#define MTK_DEV_RNG0_SIZE	0x400000
#define MTK_DEV_RNG1_BASE	(IO_PHYS + 0x1000000)
#define MTK_DEV_RNG1_SIZE	0x4000000
#define MTK_DEV_RNG2_BASE	(MT_GIC_BASE)
#define MTK_DEV_RNG2_SIZE	0x600000
#define MTK_DEV_RNG3_BASE	SPM_BASE
#define MTK_DEV_RNG3_SIZE	0x100000
#define MTK_MCDI_SRAM_BASE	0x11B000
#define MTK_MCDI_SSPM_SIZE	0x1000
#define MTK_SSPM_BASE		(IO_PHYS + 0x400000)
#define MTK_SSPM_SIZE		0x50000
#define MTK_MD_WDT_BASE         (0x0D0D2000)
#define MTK_MD_WDT_SIZE         0x1000

/*******************************************************************************
 * Timer
 ******************************************************************************/
#define SYSTIMER_BASE		(IO_PHYS + 0x17000)

/*******************************************************************************
 * DBGSYS
 ******************************************************************************/
#define DBGSYS_BASE			0x0D000000
#define DBGSYS_DEM_BASE		(DBGSYS_BASE + 0xA0000)
#define DBGSYS_DEM_SIZE		0x1000
#define DBGSYS_PCMONITOR_BASE			(DBGSYS_BASE + 0xD9000)
#define DBGSYS_PCMONITOR_SIZE			0x2000
#define DBGSYS_MDMCU_BUSMON_BASE		(DBGSYS_BASE + 0xC6000)
#define DBGSYS_MDMCU_BUSMON_SIZE		0x1000
#define DBGSYS_MDINFRA_BUSMON_BASE		(DBGSYS_BASE + 0xC8000)
#define DBGSYS_MDINFRA_BUSMON_SIZE		0x1000
#define DBGSYS_USIP_BASE			(DBGSYS_BASE + 0xC4000)
#define DBGSYS_USIP_SIZE			0x4000
#define MD_WDTCR                (MTK_MD_WDT_BASE + 0x100)
#define MD_WDTSR                (MTK_MD_WDT_BASE + 0x134)
/*******************************************************************************
 * MSDC
 ******************************************************************************/
#define MSDC0_BASE          (IO_PHYS + 0x01230000)

/*******************************************************************************
 * UFS
 ******************************************************************************/
#define PERICFG_BASE        (IO_PHYS + 0x00003000)

/*******************************************************************************
 * UFS
 ******************************************************************************/
#define UFSHCI_BASE         (IO_PHYS + 0x01270000)

/*******************************************************************************
 * APUSYS
 ******************************************************************************/
#define APUSYS_VCORE_CONFIG_BASE	0x19020000
#define APUSYS_VCORE_CONFIG_SIZE	0x1000
#define APUSYS_VCORE_MDOM_CTRL		(APUSYS_VCORE_CONFIG_BASE + 0x0048)

/*******************************************************************************
 * GIC related constants
 ******************************************************************************/
#define MT_IRQ_SEC_SGI_0	8
#define MT_IRQ_SEC_SGI_1	9
#define MT_IRQ_SEC_SGI_2	10
#define MT_IRQ_SEC_SGI_3	11
#define MT_IRQ_SEC_SGI_4	12
#define MT_IRQ_SEC_SGI_5	13
#define MT_IRQ_SEC_SGI_6	14
#define MT_IRQ_SEC_SGI_7	15

#define MT_POLARITY_LOW		0
#define MT_POLARITY_HIGH	1
#define MT_EDGE_SENSITIVE	1
#define MT_LEVEL_SENSITIVE	0

#define WDT_IRQ_BIT_ID		(104)
#define MD_WDT_IRQ_BIT_ID	(109)
#define FIQ_SMP_CALL_SGI	13

#define ATF_AMMS_IRQ_ID		(366)
#define PCCIF1_IRQ0_BIT_ID	(208)
#define PCCIF1_IRQ1_BIT_ID	(209)

/*******************************************************************************
 * UART related constants
 ******************************************************************************/
#define UART_BAUDRATE		(921600)
#define UART_CLOCK			(12000000)

/*******************************************************************************
 * System counter frequency related constants
 ******************************************************************************/
#define SYS_COUNTER_FREQ_IN_TICKS	13000000
#define SYS_COUNTER_FREQ_IN_MHZ		13

/*******************************************************************************
 * GIC-400 & interrupt handling related constants
 ******************************************************************************/

/* Base MTK_platform compatible GIC memory map */
#define BASE_GICD_BASE		(MT_GIC_BASE)
#define BASE_GICC_BASE		(MT_GIC_BASE + 0x400000)
#define MT_GIC_RDIST_BASE	(MT_GIC_BASE + 0x40000)
#define BASE_GICR_BASE		(MT_GIC_RDIST_BASE)
#define BASE_GICH_BASE		(MT_GIC_BASE + 0x4000)
#define BASE_GICV_BASE		(MT_GIC_BASE + 0x6000)
#define INT_POL_CTL0		(MCUCFG_BASE + 0xa650)
#define SEC_POL_CTL_EN0		(MCUCFG_BASE + 0xa600)
#define INT_MSK_CTL0		(MCUCFG_BASE + 0xa6a0)
#define INT_MSK_ALL		(MCUCFG_BASE + 0xa6f4)
#define INT_CFG_INDIRECT_ACCESS	(MCUCFG_BASE + 0xa6f8)
#define INT_CFG_DIRECT_ACCESS_EN	(MCUCFG_BASE + 0xa6fc)

#define NR_INT_POL_CTL		(20)

#define GIC_PRIVATE_SIGNALS	(32)

/*******************************************************************************
 * CCI-400 related constants
 ******************************************************************************/
#define PLAT_MT_CCI_CLUSTER0_SL_IFACE_IX	4
#define PLAT_MT_CCI_CLUSTER1_SL_IFACE_IX	3

#define DEBUG_XLAT_TABLE	0

/*******************************************************************************
 * Platform binary types for linking
 ******************************************************************************/
#define PLATFORM_LINKER_FORMAT		"elf64-littleaarch64"
#define PLATFORM_LINKER_ARCH		aarch64

/*******************************************************************************
 * Generic platform constants
 ******************************************************************************/

/* Size of cacheable stacks */
#if DEBUG_XLAT_TABLE
#define PLATFORM_STACK_SIZE	0x800
#elif IMAGE_BL1
#define PLATFORM_STACK_SIZE	0x440
#elif IMAGE_BL2
#define PLATFORM_STACK_SIZE	0x400
#elif IMAGE_BL31
#define PLATFORM_STACK_SIZE	0x800
#elif IMAGE_BL32
#define PLATFORM_STACK_SIZE	0x440
#endif

#define FIRMWARE_WELCOME_STR		"Booting Trusted Firmware\n"
#if ENABLE_PLAT_COMPAT
#define PLATFORM_MAX_AFFLVL		MPIDR_AFFLVL1
#else
#define PLAT_MAX_PWR_LVL		MPIDR_AFFLVL1
#endif

#define PLATFORM_CACHE_LINE_SIZE      64
#define PLATFORM_SYSTEM_COUNT         0
#define PLATFORM_CLUSTER_COUNT        1
#define PLATFORM_CLUSTER0_CORE_COUNT  8
#define PLATFORM_CLUSTER1_CORE_COUNT  0
#define PLATFORM_CORE_COUNT		(PLATFORM_CLUSTER1_CORE_COUNT +	\
					 PLATFORM_CLUSTER0_CORE_COUNT)
#define PLATFORM_MAX_CPUS_PER_CLUSTER	8
#define PLATFORM_NUM_AFFS		(PLATFORM_SYSTEM_COUNT +	\
					 PLATFORM_CLUSTER_COUNT +	\
					 PLATFORM_CORE_COUNT)

#define PLATFORM_CACHE_LEVEL_PER_CORE		2
#define PLATFORM_CACHE_LEVEL_PER_CLUSTER	2
#define PLATFORM_CACHE_LEVEL_ALL		3
/*******************************************************************************
 * Platform memory map related constants
 ******************************************************************************/
/* ATF Argument */
#define ATF_ARG_SIZE		(0x800)
#if defined(MTK_DEVMPU_SUPPORT)
#define ATF_DRAM_TOTAL_SIZE 0x200000
#else
#define ATF_DRAM_TOTAL_SIZE 0x40000
#endif

/* TF txet, ro, rw, internal SRAM, Size: release: 80KB, debug: 92KB */
#define TZRAM_BASE		(0x100000)
#define TZRAM_SIZE		(0x1B000)

#define TZRAM2_BASE		(ATF_DRAM_BASE)
#define TZRAM2_SIZE		(ATF_DRAM_TOTAL_SIZE)

#define DRAM_EMI_BASE		0x40000000

/*******************************************************************************
 * BL31 specific defines.
 ******************************************************************************/
/*
 * Put BL3-1 at the top of the Trusted SRAM (just below the shared memory, if
 * present). BL31_BASE is calculated using the current BL3-1 debug size plus a
 * little space for growth.
 */
#define BL31_IMG_SIZE		0x13A00
#define BL31_BASE		(TZRAM_BASE + ATF_ARG_SIZE)
#if defined(SPD_no)
#define BL31_IMG_LIMIT		(TZRAM_BASE + BL31_IMG_SIZE - 0x2000)	/* 0x0011_3A00 */
#else
#define BL31_IMG_LIMIT		(TZRAM_BASE + BL31_IMG_SIZE)	/* 0x0011_3A00 */
#endif
#define BL31_LIMIT		(TZRAM_BASE + TZRAM_SIZE)			/* 0x0011_C000 */
#define TZRAM2_LIMIT		(TZRAM2_BASE + TZRAM2_SIZE)
#define BL31_TZRAM_SIZE		(TZRAM_SIZE - ATF_ARG_SIZE)

/*******************************************************************************
 * Platform specific page table and MMU setup constants
 ******************************************************************************/
#define ADDR_SPACE_SIZE		(1ull << 32)
#if defined(ATF_BYPASS_DRAM)
#define MAX_XLAT_TABLES		1
#else
#define MAX_XLAT_TABLES     20
#endif
#define MAX_MMAP_REGIONS	30

/*******************************************************************************
 * Declarations and constants to access the mailboxes safely. Each mailbox is
 * aligned on the biggest cache line size in the platform. This is known only
 * to the platform as it might have a combination of integrated and external
 * caches. Such alignment ensures that two maiboxes do not sit on the same cache
 * line at any cache level. They could belong to different cpus/clusters &
 * get written while being protected by different locks causing corruption of
 * a valid mailbox address.
 ******************************************************************************/
#define CACHE_WRITEBACK_SHIFT	6
#define CACHE_WRITEBACK_GRANULE	(1 << CACHE_WRITEBACK_SHIFT)

#define BL32_BASE (0x0)

/******************************************************************************
 * MODEM REGISTERs
 ******************************************************************************/
#define LTE_SBC_PUBK_HASH0		((SECURE_AO_BASE + 0x0800))
#define LTE_SBC_PUBK_HASH1		((SECURE_AO_BASE + 0x0804))
#define LTE_SBC_PUBK_HASH2		((SECURE_AO_BASE + 0x0808))
#define LTE_SBC_PUBK_HASH3		((SECURE_AO_BASE + 0x080C))
#define LTE_SBC_PUBK_HASH4		((SECURE_AO_BASE + 0x0810))
#define LTE_SBC_PUBK_HASH5		((SECURE_AO_BASE + 0x0814))
#define LTE_SBC_PUBK_HASH6		((SECURE_AO_BASE + 0x0818))
#define LTE_SBC_PUBK_HASH7		((SECURE_AO_BASE + 0x081C))
#define LTE_SBC_LOCK			((SECURE_AO_BASE + 0x0820))

/******************************************************************************
 * M4U REGISTERs
 ******************************************************************************/
#define MM_IOMMU_SEC_DEBUG		(SECURE_AO_BASE + 0x610)
#define VPU_IOMMU_SEC_DEBUG		(SECURE_AO_BASE + 0x618)

/*******************************************************************************
 * TRNG Registers
 ******************************************************************************/
#define TRNG_BASE_ADDR			TRNG_base
#define TRNG_BASE_SIZE			(0x1000)
#define TRNG_CTRL			(TRNG_base+0x0000)
#define TRNG_TIME			(TRNG_base+0x0004)
#define TRNG_DATA			(TRNG_base+0x0008)
#define TRNG_PDN_base			(0x10001000)
#define TRNG_PDN_BASE_ADDR		TRNG_PDN_BASE_ADDR
#define TRNG_PDN_BASE_SIZE		(0x1000)
#define TRNG_PDN_SET			(TRNG_PDN_base+0x0088)
#define TRNG_PDN_CLR			(TRNG_PDN_base+0x008C)
#define TRNG_PDN_STATUS			(TRNG_PDN_base+0x0094)
#define TRNG_CTRL_RDY			0x80000000
#define TRNG_CTRL_START			0x00000001
#define TRNG_PDN_VALUE			0x200


#define PLAT_MTK_DFD_SETUP_MAGIC		(0x99716150)
#define PLAT_MTK_DFD_READ_MAGIC			(0x99716151)
#define PLAT_MTK_DFD_WRITE_MAGIC		(0x99716152)

#ifdef MTK_FPGA_LDVT
#define KERNEL_ENTRYPOINT	0x40080000
#define KERNEL_DTB_ADDR		0x40000300
#endif

/*******************************************************************************
 * MMSYS
 ******************************************************************************/
#define MMSYS_BASE			(IO_PHYS + 0x4000000)
#define TO_SPM_CG_MASK_MCDSR_0		(MMSYS_BASE + 0x0160)
#define TO_SPM_CG_MASK_MCDSR_1		(MMSYS_BASE + 0x0164)
#define TO_SPM_CG_MASK_SODI2_0		(MMSYS_BASE + 0x0168)
#define TO_SPM_CG_MASK_SODI2_1		(MMSYS_BASE + 0x016C)

/*******************************************************************************
 * ARMv8.2 Related Registers
 ******************************************************************************/
#define CLUSTERPWRDN_EL1	S3_0_C15_C3_6
#define CPUPWRCTLR_EL1		S3_0_C15_C2_7
#define RET_CTRL_MASK		0x7
#define WFE_RET_CTRL_SHIFT	7
#define WFI_RET_CTRL_SHIFT	4

#endif /* __PLATFORM_DEF_H__ */

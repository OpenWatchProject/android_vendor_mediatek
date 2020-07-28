/*
 * Copyright (c) 2016, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#include <arch_helpers.h>
#include <arm_gic.h>
#include <bl_common.h>
#include <bl31.h>
#include <cci.h>
#include <console.h>
#include <debug.h>
#include <mmio.h>
#include <mtk_aee_debug.h>
#include <mtk_plat_common.h>
#include <mtk_sip_svc.h>
#include <mtspmc.h>
#include <platform.h>
#include <plat_private.h>
#include <xlat_tables_v2.h>

#if defined(MTK_DEVMPU_SUPPORT)
#include <mpu_v1.h>
#include <devmpu.h>
#endif

unsigned long __RW_START__;
unsigned long __RW_END__;
uintptr_t __NONCACHE_DRAM_START__;
uintptr_t __NONCACHE_DRAM_END__;


#define BL31_RO_BASE (unsigned long)(&__RO_START__)
#define BL31_RO_LIMIT (unsigned long)(&__RO_END__)
#define BL31_COHERENT_RAM_BASE (unsigned long)(&__COHERENT_RAM_START__)
#define BL31_COHERENT_RAM_LIMIT (unsigned long)(&__COHERENT_RAM_END__)
#define MTK_NONCACHE_DRAM_BASE (unsigned long)(&__NONCACHE_DRAM_START__)
#define MTK_NONCACHE_DRAM_LIMIT (unsigned long)(&__NONCACHE_DRAM_END__)

#define BL31_RW_BASE (unsigned long)(&__RW_START__)
#define BL31_RW_LIMIT (unsigned long)(&__RW_END__)

#pragma weak check_wdt_sgi_routing

#ifdef DRAM_EXTENSION_SUPPORT

#define DRAM_RO_BASE (unsigned long)(&__DRAM_RO_START__)
#define DRAM_RO_END (unsigned long)(&__DRAM_RO_END__)
#define DRAM_RW_BASE (unsigned long)(&__DRAM_RW_START__)
#define DRAM_RW_END (unsigned long)(&__DRAM_RW_END__)
#endif

struct atf_arg_t gteearg;
uint8_t percpu_plat_suspend_state[PLATFORM_CORE_COUNT]
#if USE_COHERENT_MEM
__section("tzfw_coherent_mem")
#endif
;
void clean_top_32b_of_param(uint32_t smc_fid,
				uint64_t *px1,
				uint64_t *px2,
				uint64_t *px3,
				uint64_t *px4)
{
	/* if parameters from SMC32. Clean top 32 bits */
	if (0 == (smc_fid & SMC_AARCH64_BIT)) {
		*px1 = *px1 & SMC32_PARAM_MASK;
		*px2 = *px2 & SMC32_PARAM_MASK;
		*px3 = *px3 & SMC32_PARAM_MASK;
		*px4 = *px4 & SMC32_PARAM_MASK;
	}
}

static struct kernel_info k_info;
static void save_kernel_info(uint64_t pc,
			uint64_t r0,
			uint64_t r1,
			uint64_t k32_64)
{
	k_info.k32_64 = k32_64;
	k_info.pc = pc;

	if (k32_64 == LINUX_KERNEL_32) {
		/* for 32 bits kernel */
		k_info.r0 = 0;
		/* machtype */
		k_info.r1 = r0;
		/* tags */
		k_info.r2 = r1;
	} else {
		/* for 64 bits kernel */
		k_info.r0 = r0;
		k_info.r1 = r1;
	}
}

uint64_t get_kernel_info_pc(void)
{
	return k_info.pc;
}

uint64_t get_kernel_info_r0(void)
{
	return k_info.r0;
}

uint64_t get_kernel_info_r1(void)
{
	return k_info.r1;
}

uint64_t get_kernel_info_r2(void)
{
	return k_info.r2;
}

void set_kernel_k32_64(uint64_t k32_64)
{
	k_info.k32_64 = k32_64;
}

uint64_t get_kernel_k32_64(void)
{
	return k_info.k32_64;
}

void boot_to_kernel(uint64_t x1, uint64_t x2, uint64_t x3, uint64_t x4)
{
	static uint8_t kernel_boot_once_flag;
	/* only support in booting flow */
	if (kernel_boot_once_flag == 0) {
		kernel_boot_once_flag = 1;
		console_init(gteearg.atf_log_port, UART_CLOCK, UART_BAUDRATE);

		INFO("save kernel info\n");
		save_kernel_info(x1, x2, x3, x4);
		bl31_prepare_kernel_entry(x4);

		INFO("el3_exit\n");
		console_uninit();
	}
}

#if defined(MTK_ENABLE_GENIEZONE)
#define GZ_KERNEL_LOAD_OFFSET (0x38000000)
#define GZ_INVALID_MPU_DOMAIN (0xFF)
#define EL2_BOOT_DISABLE (1 << 0)
static uint32_t gz_configs = EL2_BOOT_DISABLE;
static uint64_t gz_exec_start_offset;
static uint32_t gz_reserved_mem_size;
uint32_t is_el2_enabled(void)
{
	if (gz_configs & EL2_BOOT_DISABLE)
		return 0; /* el2 is disabled */

	return 1; /* el2 is enabled */
}
uint64_t get_el2_exec_start_offset(void)
{
	return gz_exec_start_offset;
}
uint32_t get_el2_reserved_mem_size(void)
{
	return gz_reserved_mem_size;
}
void configure_el2_info(struct boot_tag_gz_info *gz_info)
{
	gz_configs = gz_info->gz_configs;
	INFO("GZ CONFIGS = 0x%x\n", gz_configs);
}
#include <string.h>
void configure_el2_plat(struct boot_tag_gz_platform *gz_plat)
{
	gz_reserved_mem_size = gz_plat->reserve_mem_size;
	INFO("GZ RESERVE MEM SIZE = 0x%x\n", gz_plat->reserve_mem_size);

	/* do copy here to prevent un-aligned access for 64-bit memory */
	memcpy(&gz_exec_start_offset, &gz_plat->exec_start_offset,
		sizeof(uint64_t));
	INFO("GZ EXEC START OFFSET = 0x%lx\n", gz_exec_start_offset);
}
#endif

#if defined(MTK_DEVMPU_SUPPORT)
void configure_devmpu(void)
{
	int i;
	int rc;

	struct emi_region_info_t region_info;

	enum DEVMPU_VMD_MAPPING vmd_mapping[EMI_MPU_DOMAIN_NUM] = {
		DEVMPU_VMD_MAPPING_D0,  DEVMPU_VMD_MAPPING_D1,
		DEVMPU_VMD_MAPPING_D2,  DEVMPU_VMD_MAPPING_D3,
		DEVMPU_VMD_MAPPING_D4,  DEVMPU_VMD_MAPPING_D5,
		DEVMPU_VMD_MAPPING_D6,  DEVMPU_VMD_MAPPING_D7,
		DEVMPU_VMD_MAPPING_D8,  DEVMPU_VMD_MAPPING_D9,
		DEVMPU_VMD_MAPPING_D10, DEVMPU_VMD_MAPPING_D11,
		DEVMPU_VMD_MAPPING_D12, DEVMPU_VMD_MAPPING_D13,
		DEVMPU_VMD_MAPPING_D14, DEVMPU_VMD_MAPPING_D15,
#if EMI_MPU_DOMAIN_NUM >= 32
		/* extend VM domain mapping if EMI domain num increase ... */
#endif
	};

	/* disable for safety */
	devmpu_disable();

	/* reset device mpu for clean permission control state */
	if (devmpu_reset()) {
		WARN("%s: failed to reset Device MPU\n", __func__);
		return;
	}

	/* set VM domain mapping for each EMI domain */
	for (i = 0; i < EMI_MPU_DOMAIN_NUM; ++i) {
		rc = devmpu_domain_remap_set(i, vmd_mapping[i]);
		if (rc) {
			WARN("%s: failed to remap EMI domain=%d to VM domain=%d, rc=%d\n",
					__func__, i, vmd_mapping[i], rc);
			return;
		}
	}

	/*
	 * Device MPU protection relies on at least one region
	 * covered by EMI MPU to become fully functional. Thus,
	 * we need setup EMI MPU AP region (31) at early stage.
	 */
	region_info.start  = DEVMPU_DRAM_BASE;
	region_info.end    = DEVMPU_DRAM_BASE + DEVMPU_DRAM_SIZE - 1;
	region_info.region = EMI_MPU_REGION_ID_AP;
	SET_ACCESS_PERMISSION(region_info.apc, UNLOCK,
			NO_PROTECTION, NO_PROTECTION, NO_PROTECTION, NO_PROTECTION,
			NO_PROTECTION, NO_PROTECTION, NO_PROTECTION, NO_PROTECTION,
			NO_PROTECTION, NO_PROTECTION, NO_PROTECTION, NO_PROTECTION,
			NO_PROTECTION, NO_PROTECTION, NO_PROTECTION, NO_PROTECTION);
	if (emi_mpu_set_protection(&region_info)) {
		WARN("%s: failed to set EMI MPU AP region for Device MPU\n", __func__);
		return;
	}

	/* enable DeviceMPU protection */
	devmpu_enable();
}
#endif

uint32_t plat_get_spsr_for_bl33_entry(void)
{
	unsigned int mode;
	uint32_t spsr;
	unsigned int ee;
	unsigned long daif;

#if defined(MTK_ENABLE_GENIEZONE)
	if (is_el2_enabled()) {
		INFO("Booting from EL2!\n");
		spsr = SPSR_64(MODE_EL2, MODE_SP_ELX, DISABLE_ALL_EXCEPTIONS);
		return spsr;
	}
#endif

	INFO("Secondary bootloader is AArch32\n");
	mode = MODE32_svc;
	ee = 0;
	/*
	 * TODO: Choose async. exception bits if HYP mode is not
	 * implemented according to the values of SCR.{AW, FW} bits
	 */
	daif = DAIF_ABT_BIT | DAIF_IRQ_BIT | DAIF_FIQ_BIT;

	spsr = SPSR_MODE32(mode, 0, ee, daif);
	return spsr;
}
/*******************************************************************************
 * common function setting up the pagetables of the platform memory
 * map & initialize the mmu, for the given exception level.
 * Use plat_mmp_tbl as platform customized page table mapping.
 ******************************************************************************/
void configure_mmu_el3(const mmap_region_t *plat_mmap_tbl)
{
	unsigned long total_base = TZRAM_BASE;
#ifdef DRAM_EXTENSION_SUPPORT
	unsigned long total_size = (TZRAM_SIZE) & ~(PAGE_SIZE_MASK);
#else
	unsigned long total_size = (TZRAM_SIZE + TZRAM2_SIZE) & ~(PAGE_SIZE_MASK);
#endif
	unsigned long ro_start = BL31_RO_BASE & ~(PAGE_SIZE_MASK);
	unsigned long ro_size = BL31_RO_LIMIT - ro_start;
#if (USE_COHERENT_MEM) || (USE_MTK_NONCACHE_MEM)
	unsigned long coh_start, coh_size;
#endif
	/* add memory regions */
	mmap_add_region(total_base, total_base,
			total_size,
			MT_MEMORY | MT_RW | MT_SECURE);
	mmap_add_region(ro_start, ro_start,
			ro_size,
			MT_MEMORY | MT_RO | MT_SECURE);

#if USE_COHERENT_MEM
	coh_start = BL31_COHERENT_RAM_BASE;
	coh_size = BL31_COHERENT_RAM_LIMIT - BL31_COHERENT_RAM_BASE;

	mmap_add_region(coh_start, coh_start,
			coh_size,
			MT_DEVICE | MT_RW | MT_SECURE);
#endif
#if	USE_MTK_NONCACHE_MEM
	coh_start = MTK_NONCACHE_DRAM_BASE;
	coh_size = MTK_NONCACHE_DRAM_LIMIT - MTK_NONCACHE_DRAM_BASE;
	mmap_add_region(coh_start, coh_start,
				coh_size,
				MT_DEVICE | MT_RW | MT_SECURE);
#endif
#ifdef DRAM_EXTENSION_SUPPORT
	mmap_add_region(DRAM_RO_BASE, DRAM_RO_BASE,
			DRAM_RO_END - DRAM_RO_BASE,
			MT_MEMORY | MT_RO | MT_SECURE);
	mmap_add_region(DRAM_RW_BASE, DRAM_RW_BASE,
			DRAM_RW_END - DRAM_RW_BASE,
			MT_MEMORY | MT_RW | MT_SECURE);
#endif
	/* add mmap table */
	if (plat_mmap_tbl)
		mmap_add(plat_mmap_tbl);
	else
		WARN("platform mmap table is not available\n");

	/* set up translation tables */
	init_xlat_tables();

	/* enable the MMU */
	enable_mmu_el3(0);
}

void check_wdt_sgi_routing(void)
{
}

void bl31_plat_runtime_setup(void)
{
	/* check WDT/SGI routing after S-OS init */
	check_wdt_sgi_routing();
#ifdef MTK_FPGA_LDVT
	boot_to_kernel(KERNEL_ENTRYPOINT, KERNEL_DTB_ADDR, 0, LINUX_KERNEL_64);
	bl31_prepare_next_image_entry();
#endif
	/*
	 * Finish the use of console driver in BL31 so that any runtime logs
	 * from BL31 will be suppressed.
	 */
	console_uninit();
}

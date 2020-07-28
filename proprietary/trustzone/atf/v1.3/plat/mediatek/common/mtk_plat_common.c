/*
 * Copyright (c) 2016, ARM Limited and Contributors. All rights reserved.
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
#include <arch_helpers.h>
#include <arm_gic.h>
#include <bl_common.h>
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
#include <xlat_tables.h>

unsigned long __RW_START__;
unsigned long __RW_END__;


#define BL31_RO_BASE (unsigned long)(&__RO_START__)
#define BL31_RO_LIMIT (unsigned long)(&__RO_END__)
#define BL31_COHERENT_RAM_BASE (unsigned long)(&__COHERENT_RAM_START__)
#define BL31_COHERENT_RAM_LIMIT (unsigned long)(&__COHERENT_RAM_END__)

#define BL31_RW_BASE (unsigned long)(&__RW_START__)
#define BL31_RW_LIMIT (unsigned long)(&__RW_END__)

#pragma weak check_wdt_sgi_routing
#pragma weak picachu_cfg_default_boot_addr

#ifdef DRAM_EXTENSION_SUPPORT
extern unsigned long __DRAM_RO_START__;
extern unsigned long __DRAM_RO_END__;
extern unsigned long __DRAM_RW_START__;
extern unsigned long __DRAM_RW_END__;

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

void picachu_cfg_default_boot_addr(void)
{
}

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

#if MTK_SIP_KERNEL_BOOT_ENABLE
static struct kernel_info k_info;

static void save_kernel_info(uint64_t pc,
			uint64_t r0,
			uint64_t r1,
			uint64_t k32_64)
{
	k_info.k32_64 = k32_64;
	k_info.pc = pc;

	if (LINUX_KERNEL_32 ==  k32_64) {
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
	if (0 == kernel_boot_once_flag) {
		kernel_boot_once_flag = 1;

		picachu_cfg_default_boot_addr();

		console_init(gteearg.atf_log_port,
			UART_CLOCK, UART_BAUDRATE);
		INFO("save kernel info\n");
		save_kernel_info(x1, x2, x3, x4);
		bl31_prepare_kernel_entry(x4);
#if CONFIG_SPMC_MODE != 0
#if !defined(ATF_BYPASS_DRAM)
		spmc_init();
#endif
#endif
		INFO("el3_exit\n");
		console_uninit();
	}
}
#endif

#if defined(MTK_ENABLE_GENIEZONE)
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
 * common function setting up the pagetables of the platform memory map & initialize the mmu, for the given exception level.
 * Use plat_mmp_tbl as platform customized page table mapping.
 ******************************************************************************/
void configure_mmu_el3(const mmap_region_t *plat_mmap_tbl) {
	unsigned long total_base = TZRAM_BASE;
#ifdef DRAM_EXTENSION_SUPPORT
	unsigned long total_size = (TZRAM_SIZE) & ~(PAGE_SIZE_MASK);
#else
	unsigned long total_size = (TZRAM_SIZE + TZRAM2_SIZE) & ~(PAGE_SIZE_MASK);
#endif
	unsigned long ro_start = BL31_RO_BASE & ~(PAGE_SIZE_MASK);
	unsigned long ro_size = BL31_RO_LIMIT - ro_start;
#if USE_COHERENT_MEM
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

	/*
	 * Finish the use of console driver in BL31 so that any runtime logs
	 * from BL31 will be suppressed.
	 */
	console_uninit();
}

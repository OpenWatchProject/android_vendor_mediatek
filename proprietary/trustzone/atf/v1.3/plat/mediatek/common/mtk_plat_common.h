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
#ifndef __MTK_PLAT_COMMON_H__
#define __MTK_PLAT_COMMON_H__

#ifndef __ASSEMBLY__
#include <stdint.h>
#include <xlat_tables.h>

#if defined(MTK_ENABLE_MPU_HAL_SUPPORT)
#include <mpu_ctrl/mpu_mblock.h>
#endif
#endif /*__ASSEMBLY__*/
/*******************************************************************************
 * Function and variable prototypes
 ******************************************************************************/
#define DEVINFO_SIZE 4
#define LINUX_KERNEL_32 0
#define SMC32_PARAM_MASK		(0xFFFFFFFF)

#ifndef __ASSEMBLY__
struct atf_arg_t{
	uint32_t atf_magic;
	uint32_t tee_support;
	uint32_t tee_entry;
	uint64_t tee_boot_arg_addr;
	uint32_t hwuid[4];     /* HW Unique id for t-base used */
	uint32_t atf_hrid_size; /* Check this atf_hrid_size to read from HRID array */
	uint32_t HRID[8];      /* HW random id for t-base used */
	uint32_t atf_log_port;
	uint32_t atf_log_baudrate;
	uint64_t atf_log_buf_start;
	uint32_t atf_log_buf_size;
	uint32_t atf_irq_num;
	uint32_t devinfo[DEVINFO_SIZE];
	uint64_t atf_aee_debug_buf_start;
	uint32_t atf_aee_debug_buf_size;
	uint32_t msg_fde_key[4]; /* size of message auth key is 16bytes(128 bits) */
#if CFG_TEE_SUPPORT
	uint32_t tee_rpmb_size;
#endif
};

struct kernel_info {
	uint64_t pc;
	uint64_t r0;
	uint64_t r1;
	uint64_t r2;
	uint64_t k32_64;
};

struct mtk_bl_param_t {
	uint64_t bootarg_loc;
	uint64_t bootarg_size;
	uint64_t bl33_start_addr;
	uint64_t tee_info_addr;
};

/* boot reason */
#define BOOT_TAG_BOOT_REASON	0x88610001
struct boot_tag_boot_reason {
	uint32_t boot_reason;
};

struct boot_tag_plat_dbg_info {
	uint32_t info_max;
};

/* charger type info */
#define BOOT_TAG_IS_VOLT_UP      0x8861001A
struct boot_tag_is_volt_up {
	uint32_t is_volt_up;
};

#define NO_ABNORMAL_BOOT_TAG 0xffffffff
/* Boot tag */
#define BOOT_TAG_IS_ABNORMAL_BOOT	0x8861001B
struct boot_tag_is_abnormal_boot {
	uint32_t is_abnormal_boot;
};

#define BOOT_TAG_RAM_CONSOLE_INFO	0x8861001C
struct boot_tag_ram_console_info {
	uint32_t sram_addr;
	uint32_t sram_size;
	uint32_t def_type;
	uint32_t memory_info_offset;
};

#define BOOT_TAG_SPMFW_BASE	0x88610022
struct boot_tag_spmfw_base {
	uint32_t spmfw_base_addr;
	uint32_t spmfw_base_size;
};

#if defined(MTK_ENABLE_MPU_HAL_SUPPORT)
#define BOOT_TAG_MEM    0x88610005
struct boot_tag_mem {
	uint32_t dram_rank_num;
	uint64_t dram_rank_size[4];
	struct mblock_info mblock_info;
	struct dram_info orig_dram_info;
	struct mem_desc lca_reserved_mem;
	struct mem_desc tee_reserved_mem;
};
#endif

#if defined(MTK_ENABLE_GENIEZONE)
#define BOOT_TAG_GZ_INFO  (0x88610023)
#define BOOT_TAG_GZ_PARAM (0x88610024)
#define BOOT_TAG_GZ_PLAT  (0x88610025)
struct boot_tag_gz_info {
	uint32_t gz_configs;
	uint32_t lk_addr;
	uint32_t build_variant;
};
struct boot_tag_gz_platform {

	/* GZ platform */
	uint64_t flags;
	uint64_t exec_start_offset;
	uint32_t reserve_mem_size;

	/* GZ remap offset */
	struct {
		uint64_t offset_ddr;
		uint64_t offset_io;
		uint64_t offset_sec_io;
	} __packed remap;

	/* HW reg base */
	struct {
		uint32_t uart;
		uint32_t cpuxgpt;
		uint32_t gicd;
		uint32_t gicr;
		uint32_t pwrap;
		uint32_t rtc;
		uint32_t mcucfg;
		uint32_t res[8];
	} __packed reg_base;

	/* Hardware version */
	struct {
		uint16_t hw_code;
		uint16_t hw_sub_code;
	} __packed plat_ver;

	/* VM info */
	uint32_t vm_mem_size;

	/* gz log info */
	uint32_t log_addr;
	uint32_t log_size;

	/* total detected DRAM size */
	uint32_t dram_size_1mb_cnt;

	/* device info area 30 */
	uint32_t dev_info_area30;

	/* RAZ */
	uint32_t reserved[32];

} __packed;
#endif

struct boot_tag_header {
	uint32_t size;
	uint32_t tag;
};

struct boot_tag{
	struct boot_tag_header hdr;
	union {
		struct boot_tag_boot_reason boot_reason;
		struct boot_tag_plat_dbg_info plat_dbg_info;
		struct boot_tag_is_volt_up volt_info;
		struct boot_tag_is_abnormal_boot is_abnormal_boot;
		struct boot_tag_ram_console_info ram_console_info;
		struct boot_tag_spmfw_base spmfw_base;
#if defined(MTK_ENABLE_MPU_HAL_SUPPORT)
		struct boot_tag_mem mem;
#endif
#if defined(MTK_ENABLE_GENIEZONE)
		struct boot_tag_gz_info gz_info;
		struct boot_tag_gz_platform gz_plat;
#endif
	} u;
};

typedef enum {
	BR_POWER_KEY = 0,
	BR_USB,
	BR_RTC,
	BR_WDT,
	BR_WDT_BY_PASS_PWK,
	BR_TOOL_BY_PASS_PWK,
	BR_2SEC_REBOOT,
	BR_UNKNOWN,
	BR_KERNEL_PANIC,
	BR_WDT_SW,
	BR_WDT_HW
} boot_reason_t;

#define boot_tag_next(t)    ((struct boot_tag *)((uint32_t *)(t) + (t)->hdr.size))
#define boot_tag_size(type)	((sizeof(struct boot_tag_header) + sizeof(struct type)) >> 2)

/* bit operations */
#define SET_BIT(_arg_, _bits_)					(uint32_t)((_arg_) |=  (uint32_t)(1 << (_bits_)))
#define CLEAR_BIT(_arg_, _bits_)				((_arg_) &= ~(1 << (_bits_)))
#define TEST_BIT(_arg_, _bits_)					((uint32_t)(_arg_) & (uint32_t)(1 << (_bits_)))
#define EXTRACT_BIT(_arg_, _bits_)				((_arg_ >> (_bits_)) & 1)
#define MASK_BITS(_msb_, _lsb_)					(((1U << ((_msb_) - (_lsb_) + 1)) - 1) << (_lsb_))
#define MASK_FIELD(_field_)						MASK_BITS(_field_##_MSB, _field_##_LSB)
#define EXTRACT_BITS(_arg_, _msb_, _lsb_)		((_arg_ & MASK_BITS(_msb_, _lsb_)) >> (_lsb_))
#define EXTRACT_FIELD(_arg_, _field_)			EXTRACT_BITS(_arg_, _field_##_MSB, _field_##_LSB)
#define INSERT_BIT(_arg_, _bits_, _value_)		((_value_) ? ((_arg_) |= (1 << (_bits_))) : ((_arg_) &= ~(1 << (_bits_))))
#define INSERT_BITS(_arg_, _msb_, _lsb_, _value_) \
				((_arg_) = ((_arg_) & ~MASK_BITS(_msb_, _lsb_)) | (((_value_) << (_lsb_)) & MASK_BITS(_msb_, _lsb_)))
#define INSERT_FIELD(_arg_, _field_, _value_)	INSERT_BITS(_arg_, _field_##_MSB, _field_##_LSB, _value_)

#define typeof __typeof__
#define ALIGN(x,a)              __ALIGN_MASK(x,(typeof(x))(a)-1)
#define __ALIGN_MASK(x,mask)    (((x)+(mask))&~(mask))

#define set_cpu_suspend_state(linearid) \
{ \
  assert(linearid < PLATFORM_CORE_COUNT); \
  percpu_plat_suspend_state[linearid] = 1; \
}
#define clear_cpu_suspend_state(linearid) \
{ \
  assert(linearid < PLATFORM_CORE_COUNT); \
  percpu_plat_suspend_state[linearid] = 0; \
}
#define get_cpu_suspend_list() (percpu_plat_suspend_state)

/* Declarations for mtk_plat_common.c */
uint32_t plat_get_spsr_for_bl32_entry(void);
uint32_t plat_get_spsr_for_bl33_entry(void);
void clean_top_32b_of_param(uint32_t smc_fid, uint64_t *x1,
				uint64_t *x2,
				uint64_t *x3,
				uint64_t *x4);
void bl31_prepare_kernel_entry(uint64_t k32_64);
void boot_to_kernel(uint64_t x1, uint64_t x2, uint64_t x3, uint64_t x4);
uint64_t get_kernel_info_pc(void);
uint64_t get_kernel_info_r0(void);
uint64_t get_kernel_info_r1(void);
uint64_t get_kernel_info_r2(void);

extern struct atf_arg_t gteearg;
extern uint64_t mtk_lk_stage;
extern uint64_t wdt_kernel_cb_addr;

extern void configure_mmu_el3(const mmap_region_t *plat_mmap_tbl);
extern uint8_t percpu_plat_suspend_state[];

#if defined(MTK_ENABLE_GENIEZONE)
uint32_t is_el2_enabled(void);
uint64_t get_el2_exec_start_offset(void);
uint32_t get_el2_reserved_mem_size(void);
void configure_el2_info(struct boot_tag_gz_info *gz_info);
void configure_el2_plat(struct boot_tag_gz_platform *gz_plat);
#endif

#endif /*__ASSEMBLY__*/

#endif

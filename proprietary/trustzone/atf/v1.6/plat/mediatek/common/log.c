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

#include <arch.h>
#include <arch_helpers.h>
#include <debug.h>
#include <gic_common.h>
#include <gicv3.h>
#include <log.h>
#include <mmio.h>
#include <mt_cpuxgpt.h>	/* sched_clock() */
#include <mtk_plat_common.h>
#include <plat_private.h>
#include <platform.h>
#include <platform_def.h>
#include <string.h>
#include <stdint.h>

#define DEBUG_LOG_SERVICE 1
#if DEBUG_LOG_SERVICE
#define debug_print(...) NOTICE(__VA_ARGS__)
#else
#define debug_print(...) ((void)0)
#endif

extern void gicd_set_ispendr(uintptr_t, unsigned int);

static spinlock_t atf_buf_lock;
static unsigned int atf_crash_write_offset;
static unsigned int mt_log_buf_start;
static unsigned int mt_log_buf_size;
static unsigned int mt_log_buf_end;
static unsigned int mt_log_sanity;
/* Record if the ATF log overwritten, if yes, print overwrtten log */
static unsigned int is_log_overwritten;
static uint64_t except_write_pos_per_cpu[PLATFORM_CORE_COUNT];

unsigned int *mt_exception_buf_end;
atf_log_ctrl_t *p_atf_log_ctrl;
unsigned int mt_log_ktime_sync;

/* Set ptr_atf_crash_flg to ATF_BAD_PTR instead of NULL pointer, this
 *  could prevent any unexpect access before .bss initialization. */
unsigned int *ptr_atf_crash_flag = (unsigned int *)ATF_BAD_PTR;
unsigned long *ptr_except_write_pos_per_cpu;
uintptr_t atf_crash_buf_upper_bound_addr;

#ifdef MTK_ATF_RAM_DUMP
uint64_t	atf_ram_dump_base;
uint64_t	atf_ram_dump_size;
spinlock_t	atf_ram_dump_lock;
#endif

#if !LOG_USE_SPIN_LOCK
DEFINE_BAKERY_LOCK(log_lock);
#endif
unsigned int get_is_log_overwritten(void)
{
	return is_log_overwritten;
}

void clr_is_log_overwritten(void)
{
	is_log_overwritten = 0;
}

void mt_log_suspend_flush(void)
{
	if (mt_log_buf_size != 0)
	{
		flush_dcache_range((uint64_t)mt_log_buf_start, (uint64_t)mt_log_buf_size);
	}
}

/* don't use any print function here in this function */
int mt_log_lock_acquire(void)
{
#if LOG_USE_SPIN_LOCK
	spin_lock(&atf_buf_lock);
#else
	bakery_lock_get(&log_lock);
#endif
	return 0;
}

/* don't use any print function here in this function */
int mt_log_write(unsigned char c)
{
	/* p_atf_log_ctrl->info.atf_log_addr = start + ATF_LOG_CTRL_BUF_SIZE; */
	*(unsigned char *)(uintptr_t)(p_atf_log_ctrl->info.atf_log_addr +
								p_atf_log_ctrl->info.atf_write_offset) = c;

	if (p_atf_log_ctrl->info.atf_total_write_count <= ATF_CRASH_LAST_LOG_SIZE)
		p_atf_log_ctrl->info.atf_total_write_count++;

	/*  */
	/* |--- Ctrl  ---|--- Ring Buffer ---|--- Crash Log -|-Except-|--- AEE buffer --| */
	/* clean		  clean			  keep			keep		clean */
	/*  */
	/*	p_atf_log_ctrl->info.atf_log_size = size -
			ATF_LOG_CTRL_BUF_SIZE - aee_buf_size -
			ATF_CRASH_LAST_LOG_SIZE - ATF_EXCEPT_BUF_SIZE;*/

	p_atf_log_ctrl->info.atf_write_offset++;
	p_atf_log_ctrl->info.atf_write_offset = p_atf_log_ctrl->info.atf_write_offset %
										p_atf_log_ctrl->info.atf_log_size;

	if (p_atf_log_ctrl->info.atf_write_offset == p_atf_log_ctrl->info.atf_read_offset) {
		/* write_pos cross read_pos */
		is_log_overwritten = 1;
	}

	return 0;
}

static int mt_crash_log_write(unsigned char c)
{
	/* p_atf_log_ctrl->info.atf_log_addr = start + ATF_LOG_CTRL_BUF_SIZE; */
	*(unsigned char *)(uintptr_t)(p_atf_log_ctrl->info.atf_crash_log_addr + atf_crash_write_offset) = c;

	/*  */
	/* |- Ctrl -|- Ring Buffer -|- Crash Log -|-Except-|- AEE buffer -| */
	/*    clean       clean           keep       keep       clean */
	/*  */
	/*	p_atf_log_ctrl->info.atf_log_size = size - */
	/*	ATF_LOG_CTRL_BUF_SIZE - aee_buf_size - */
	/*	ATF_CRASH_LAST_LOG_SIZE - ATF_EXCEPT_BUF_SIZE; */

	atf_crash_write_offset++;
	atf_crash_write_offset = atf_crash_write_offset % p_atf_log_ctrl->info.atf_crash_log_size;

	return 0;
}

static void show_atf_wdt_footprint(void)
{
	int cpu;

	atf_crash_write_offset = 0;
	/* direct atf log write crash_log_write() */
	bl31_log_service_register(NULL, &mt_crash_log_write, NULL);
	for (cpu = 0; cpu < PLATFORM_CORE_COUNT; cpu++) {
		/* cannot merge in a line, */
		/* it will caused the print buffer overflow */
		NOTICE("[%s] CPU[%d] IRQ[%u]",
			/* check it timestamp sync with kernel wall time */
		((p_atf_log_ctrl->info.enter_atf_fiq_irq_num[cpu] >>
			MT_LOG_KTIME_FLAG_SHIFT) & 0x1)?"K":"'A",
			cpu,
			/* keep IRQ numeber only, remove MT_LOG_KTIME flag */
			(p_atf_log_ctrl->info.enter_atf_fiq_irq_num[cpu] & IAR0_EL1_INTID_MASK));

		NOTICE("enter(%llu)", p_atf_log_ctrl->info.fiq_irq_enter_timestamp[cpu]);
		NOTICE("quit(%llu)\n", p_atf_log_ctrl->info.fiq_irq_quit_timestamp[cpu]);
	}
	/* unregister crash_log_write() */
	bl31_log_service_register(NULL, NULL, NULL);
}

static int mt_crash_log_dump_sanity(atf_log_ctrl_t *ctrl, uint32_t start, uint32_t size, uint32_t aee_buf_size)
{
	uint32_t atf_log_size_sanity = size - ATF_LOG_CTRL_BUF_SIZE - aee_buf_size -
					ATF_CRASH_LAST_LOG_SIZE - ATF_EXCEPT_BUF_SIZE;
	uint32_t atf_log_addr_sanity = start + ATF_LOG_CTRL_BUF_SIZE;

	if (ATF_LAST_LOG_MAGIC_NO != ctrl->info.atf_crash_flag && ATF_CRASH_MAGIC_NO != ctrl->info.atf_crash_flag)
		goto FAIL;
	if (ctrl->info.atf_log_addr != atf_log_addr_sanity)
		goto FAIL;
	if (ctrl->info.atf_log_size != atf_log_size_sanity)
		goto FAIL;
	if (ctrl->info.atf_crash_log_addr != (start + ATF_LOG_CTRL_BUF_SIZE) +
				(size - ATF_LOG_CTRL_BUF_SIZE - aee_buf_size -
				ATF_CRASH_LAST_LOG_SIZE - ATF_EXCEPT_BUF_SIZE))
		goto FAIL;
	if (ctrl->info.atf_write_offset > atf_log_size_sanity)
		goto FAIL;
	if (ctrl->info.atf_read_offset > atf_log_size_sanity)
		goto FAIL;

	return 1;
FAIL:
	INFO("atf RAW dump\n");
	return 0;

}

static int mt_crash_log_dump(uint8_t *crash_log_addr, uint32_t crash_log_size, uint32_t start, uint32_t size, uint32_t aee_buf_size)
{
	uint64_t ret = 0;
	uint64_t read_count;
	uint64_t offset;
	uint64_t write_count = p_atf_log_ctrl->info.atf_total_write_count;
	uint8_t *r_ptr = NULL;
	uint8_t *w_ptr = NULL;
	uint8_t *start_ptr = (uint8_t *)(uintptr_t)(mt_log_buf_start + ATF_LOG_CTRL_BUF_SIZE);

	/* Sanity check */
	if (!mt_crash_log_dump_sanity(p_atf_log_ctrl, start, size, aee_buf_size))
		return 0;
	if (write_count == 0)
		return 0;
	if (write_count > crash_log_size) {
		offset = crash_log_size-1;
		read_count = crash_log_size;
	} else {
		offset = write_count-1;
		read_count = write_count;
	}

	/* p_atf_log_ctrl->info.atf_log_addr = start + ATF_LOG_CTRL_BUF_SIZE; */
	r_ptr = (uint8_t *)((uintptr_t)p_atf_log_ctrl->info.atf_log_addr +
						(uintptr_t)p_atf_log_ctrl->info.atf_write_offset);/*offset*/

	w_ptr = crash_log_addr + offset;
	while (read_count) {
		*(w_ptr--) = *(r_ptr--);
		if (r_ptr < start_ptr)
			r_ptr = (uint8_t *)(uintptr_t)mt_log_buf_end;
		read_count--;
		ret++;
	}

	return ret;
}


/* don't use any print function here in this function */
int mt_log_lock_release(void)
{
	/* release the lock */
#if LOG_USE_SPIN_LOCK
	spin_unlock(&atf_buf_lock);
#else
	bakery_lock_release(&log_lock);
#endif
	return 0;
}

/*uint32_t is_power_on_boot(void)
{
	uint32_t wdt_sta, wdt_interval;
	wdt_sta = mmio_read_32(MTK_WDT_STATUS);
	wdt_interval = mmio_read_32(MTK_WDT_INTERVAL);
	NOTICE("sta=0x%x int=0x%x\r\n", wdt_sta, wdt_interval);

	// Bit 2: IS_POWER_ON_RESET.
	// (bit2 will be set in preloader when reboot_from power on)
	if(wdt_interval & (1<<2))
		return 1;
	return 0;
}*/

void mt_log_set_crash_flag(void)
{
	p_atf_log_ctrl->info.atf_crash_flag = ATF_CRASH_MAGIC_NO;
}

uint64_t mt_log_get_crash_log_addr(void)
{
	if (!mt_log_sanity) {
		return (uint64_t) gteearg.atf_log_buf_start;
	} else {
		return p_atf_log_ctrl->info.atf_crash_log_addr;
	}
}

uint64_t mt_log_get_crash_log_size(void)
{
	if (!mt_log_sanity) {
		return (uint64_t) gteearg.atf_log_buf_size;
	} else{
		return p_atf_log_ctrl->info.atf_crash_log_size;
	}
}

unsigned int *mt_log_get_crash_flag_addr(void)
{
	return ptr_atf_crash_flag;
}

void mt_log_setup(uint64_t start, unsigned int size,
	unsigned int aee_buf_size, unsigned int is_abnormal_boot)
{
	uint32_t dump_ret = 0;
	int i;
	uint32_t local_flag = 0;

	mt_log_buf_start = start;
	mt_log_buf_size = size;
	mt_log_buf_end = start + size - 1 - aee_buf_size -
				ATF_CRASH_LAST_LOG_SIZE - ATF_EXCEPT_BUF_SIZE;
	mt_exception_buf_end =
		(unsigned int *)(uintptr_t)(start + size - 1 - aee_buf_size);

	p_atf_log_ctrl = (atf_log_ctrl_t *)(uintptr_t)start;
#if !LOG_USE_SPIN_LOCK
	bakery_lock_init(&log_lock);
#endif
	/* Initialize those two pointers for mt_console_core_putc */
	ptr_except_write_pos_per_cpu =
		(unsigned long *) &except_write_pos_per_cpu[0];
	INFO("abnormal_boot: 0x%x, cflag: 0x%x\n",
		is_abnormal_boot, p_atf_log_ctrl->info.atf_crash_flag);

	if (!is_abnormal_boot || ATF_DUMP_DONE_MAGIC_NO ==
					p_atf_log_ctrl->info.atf_crash_flag) {
		memset((void *)(uintptr_t)start, 0x0, size);
		p_atf_log_ctrl->info.atf_crash_flag = ATF_LAST_LOG_MAGIC_NO;
		p_atf_log_ctrl->info.atf_crash_log_size = ATF_CRASH_LAST_LOG_SIZE;
	} else {
		dump_ret = mt_crash_log_dump(
		(uint8_t *)(uintptr_t)p_atf_log_ctrl->info.atf_crash_log_addr,
		ATF_CRASH_LAST_LOG_SIZE, start, size, aee_buf_size);

		if (dump_ret) {
			/* dump ATF footprint, when atf_log_ctrl_buf correct */
			show_atf_wdt_footprint();

			local_flag = p_atf_log_ctrl->info.atf_crash_flag;

			/*  */
			/* |--- Ctrl  ---|--- Ring Buffer ---|--- Crash Log -|-Except-|--- AEE buffer --| */
			/* clean         clean               keep          keep        clean */
			/*  */
			memset((void *)(uintptr_t)start, 0x0,
					size-aee_buf_size-ATF_CRASH_LAST_LOG_SIZE - ATF_EXCEPT_BUF_SIZE);
			memset((void *)(uintptr_t)(start+(size-aee_buf_size)), 0x0, aee_buf_size);

			/* write crash info back to control buffer */
			if (local_flag == ATF_LAST_LOG_MAGIC_NO) {
				p_atf_log_ctrl->info.atf_crash_log_size = ATF_CRASH_LAST_LOG_SIZE;
				p_atf_log_ctrl->info.atf_crash_flag = ATF_LAST_LOG_MAGIC_NO;
			} else if (local_flag == ATF_CRASH_MAGIC_NO) {
				p_atf_log_ctrl->info.atf_crash_log_size = ATF_CRASH_LAST_LOG_SIZE + ATF_EXCEPT_BUF_SIZE;
				p_atf_log_ctrl->info.atf_crash_flag = ATF_CRASH_MAGIC_NO;
				INFO("atf_crash_flag = ATF_CRASH_MAGIC_NO\n");
			}
		} else{
			/* Can't pass ATF log sanity, dump entire atf log buffer */
			mt_log_sanity = 0;
			ptr_atf_crash_flag = &p_atf_log_ctrl->info.atf_crash_flag;
			flush_dcache_range(start, size);

			/* Don't modify Buffer and register information, just dump */
			return;
		}
	}

	atf_buf_lock.lock = 0;

	p_atf_log_ctrl->info.atf_crash_log_addr = (start + ATF_LOG_CTRL_BUF_SIZE) +
			(size - ATF_LOG_CTRL_BUF_SIZE - aee_buf_size -
			ATF_CRASH_LAST_LOG_SIZE - ATF_EXCEPT_BUF_SIZE);

	p_atf_log_ctrl->info.atf_log_addr = start + ATF_LOG_CTRL_BUF_SIZE;
	p_atf_log_ctrl->info.atf_log_size = size - ATF_LOG_CTRL_BUF_SIZE - aee_buf_size -
					ATF_CRASH_LAST_LOG_SIZE - ATF_EXCEPT_BUF_SIZE;
	p_atf_log_ctrl->info.atf_write_offset = 0;
	p_atf_log_ctrl->info.atf_read_offset = 0;

	INFO("mt_log_setup\n");
	INFO(" -atf_buf_lock: 0x%x\n", atf_buf_lock.lock);
	INFO(" -mt_log_buf_start: 0x%x\n", mt_log_buf_start);
	INFO(" -mt_log_buf_size: 0x%x\n", mt_log_buf_size);
	INFO(" -log_addr: 0x%llx\n", p_atf_log_ctrl->info.atf_log_addr);
	INFO(" -log_size: 0x%llx\n", p_atf_log_ctrl->info.atf_log_size);
	INFO(" -write_offset: 0x%x\n", p_atf_log_ctrl->info.atf_write_offset);
	INFO(" -read_offset: 0x%x\n", p_atf_log_ctrl->info.atf_read_offset);
	INFO(" -log_buf_end : 0x%x\n", mt_log_buf_end);
	INFO(" -ATF_EXCEPT_BUF_SIZE_PER_CPU : 0x%x\n", ATF_EXCEPT_BUF_SIZE_PER_CPU);
	INFO(" -ATF_EXCEPT_BUF_SIZE : 0x%x\n", ATF_EXCEPT_BUF_SIZE);
	INFO(" -PLATFORM_CORE_COUNT : 0x%x\n", PLATFORM_CORE_COUNT);
	atf_crash_buf_upper_bound_addr = start + (size - aee_buf_size) - 1;

	for (i = 0; i < PLATFORM_CORE_COUNT; i++) {
		except_write_pos_per_cpu[i] =
			(start+(size-aee_buf_size)-ATF_EXCEPT_BUF_SIZE) +
			(i * ATF_EXCEPT_BUF_SIZE_PER_CPU);
		INFO(" -except_write_pos_per_cpu[%d]: 0x%llx\n", i,
			except_write_pos_per_cpu[i]);
	}

	INFO(" -crash_flag : 0x%x\n", p_atf_log_ctrl->info.atf_crash_flag);
	INFO(" -crash_log_addr : 0x%llx\n", p_atf_log_ctrl->info.atf_crash_log_addr);
	INFO(" -crash_log_size : 0x%llx\n", p_atf_log_ctrl->info.atf_crash_log_size);

	bl31_log_service_register(&mt_log_lock_acquire, &mt_log_write,
		&mt_log_lock_release);
	mt_log_sanity = 1;
	/* Record if the ATF log overwritten */
	is_log_overwritten = 0;
	ptr_atf_crash_flag = &p_atf_log_ctrl->info.atf_crash_flag;
	flush_dcache_range(start, size);
}


#define MT_LOG_SECURE_OS_BUFFER_MAX_LENGTH 120
#define TBASE_TAG "TBASE"
static unsigned char mt_log_secure_os_buf[MT_LOG_SECURE_OS_BUFFER_MAX_LENGTH+1] = {0};
static unsigned int mt_log_secure_os_pos;

void mt_log_secure_os_print(int c)
{
	mt_log_secure_os_buf[mt_log_secure_os_pos] = c;

	/* Force to flush the buffer if find end of line */
	if (c == '\n')
	{
		mt_log_secure_os_buf[mt_log_secure_os_pos+1] = '\0';
#if CFG_MICROTRUST_TEE_SUPPORT
	NOTICE("[%s]%s", "uTos", mt_log_secure_os_buf);
#elif CFG_TRUSTKERNEL_TEE_SUPPORT
	NOTICE("[%s]%s", "TKCore", mt_log_secure_os_buf);
#else
	NOTICE("[%s]%s", TBASE_TAG, mt_log_secure_os_buf);
#endif
		mt_log_secure_os_pos = 0;
		return;
	}

	mt_log_secure_os_pos++;

	/* Already reach the end of buffer, force to flush the buffer */
	if (mt_log_secure_os_pos == MT_LOG_SECURE_OS_BUFFER_MAX_LENGTH)
	{
		mt_log_secure_os_buf[mt_log_secure_os_pos] = '\0';
#if CFG_MICROTRUST_TEE_SUPPORT
	NOTICE("[%s]%s", "uTos", mt_log_secure_os_buf);
#elif CFG_TRUSTKERNEL_TEE_SUPPORT
	NOTICE("[%s]%s", "TKCore", mt_log_secure_os_buf);
#else
	NOTICE("[%s]%s", TBASE_TAG, mt_log_secure_os_buf);
#endif
		mt_log_secure_os_pos = 0;
	}
}

static void flush_log_ctrl_cache(void)
{
	flush_dcache_range((unsigned long)p_atf_log_ctrl,
			ATF_LOG_CTRL_BUF_SIZE);
}

void atf_enter_footprint(unsigned int irq)
{
	uint64_t mpidr = 0;
	uint32_t cpu = 0;
	unsigned long long cur_time;

	mpidr = read_mpidr();
	cpu = plat_core_pos_by_mpidr(mpidr);

	/* in ATF boot time, tiemr for cntpct_el0 is not initialized
	 * so it will not count now.
	 */
#if !defined(ATF_BYPASS_DRAM)
	cur_time = sched_clock();
#else
	cur_time = 0;
#endif

	p_atf_log_ctrl->info.fiq_irq_enter_timestamp[cpu] = cur_time;
	p_atf_log_ctrl->info.enter_atf_fiq_irq_num[cpu] = irq;

	/* if timestamp is not sync. to kernel, or a bit 31 as flag */
	if (MT_LOG_KTIME) {
		p_atf_log_ctrl->info.enter_atf_fiq_irq_num[cpu] = p_atf_log_ctrl->info.enter_atf_fiq_irq_num[cpu] |
						(1 << MT_LOG_KTIME_FLAG_SHIFT);
	}
}

void atf_exit_footprint(void)
{
	uint64_t mpidr = 0;
	uint32_t cpu = 0;
	unsigned long long cur_time;

	mpidr = read_mpidr();
	cpu = plat_core_pos_by_mpidr(mpidr);

#if !defined(ATF_BYPASS_DRAM)
	cur_time = sched_clock();
#else
	cur_time = 0;
#endif

	/* only WDT or SGI will record the exit timestamp */
	if ((p_atf_log_ctrl->info.enter_atf_fiq_irq_num[cpu] & IAR0_EL1_INTID_MASK)
			== WDT_IRQ_BIT_ID ||
		(p_atf_log_ctrl->info.enter_atf_fiq_irq_num[cpu] & IAR0_EL1_INTID_MASK)
			== FIQ_SMP_CALL_SGI) {
		p_atf_log_ctrl->info.fiq_irq_quit_timestamp[cpu] = cur_time;
		flush_log_ctrl_cache();
	}
}


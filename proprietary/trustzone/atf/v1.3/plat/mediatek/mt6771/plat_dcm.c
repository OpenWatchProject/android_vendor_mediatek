/*
 * Copyright (c) 2013-2015, ARM Limited and Contributors. All rights reserved.
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

#include <arch.h>
#include <bakery_lock.h>
#include <console.h>
#include <debug.h>
#include <mmio.h>
#include <mt_spm_reg.h>
#include <plat_dcm.h>
#include <plat_private.h>
#include <plat_dcm.h>
#include <platform.h>
#include <platform_def.h>
#include <mtk_plat_common.h>

/* #define DCM_DEBUG */

uint64_t plat_dcm_mcsi_a_addr;
uint32_t plat_dcm_mcsi_a_val;
static int plat_dcm_init_type;
static unsigned int dcm_big_core_cnt;
int plat_dcm_initiated = 0;

#define PWR_STA_BIG_MP_MASK	(0x1 << 15)

DEFINE_BAKERY_LOCK(dcm_lock);

void dcm_lock_init(void)
{
	bakery_lock_init(&dcm_lock);
}

void dcm_lock_get(void)
{
	bakery_lock_get(&dcm_lock);
}

void dcm_lock_release(void)
{
	bakery_lock_release(&dcm_lock);
}

void plat_dcm_mcsi_a_backup(void)
{
}

void plat_dcm_mcsi_a_restore(void)
{
}

void plat_dcm_rgu_enable(void)
{
}

void plat_dcm_big_core_sync(short on)
{
	/* Check if Big cluster power is existed */
#ifdef DCM_DEBUG
	console_init(gteearg.atf_log_port, UART_CLOCK, UART_BAUDRATE);
	ERROR("%s: PWR_STATUS=0x%x\n",
	       __func__, mmio_read_32(PWR_STATUS));
	console_uninit();
#endif
	if (!(mmio_read_32(PWR_STATUS) & PWR_STA_BIG_MP_MASK))
		return;

	if (on) {
		mmio_write_32(MP2_SYNC_DCM,
			      (mmio_read_32(MP2_SYNC_DCM) & ~MP2_SYNC_DCM_MASK)
			      | MP2_SYNC_DCM_ON);
		dcm_big_core_cnt++;
	} else
		mmio_write_32(MP2_SYNC_DCM,
			      (mmio_read_32(MP2_SYNC_DCM) & ~MP2_SYNC_DCM_MASK)
			      | MP2_SYNC_DCM_OFF);
#ifdef DCM_DEBUG
	console_init(gteearg.atf_log_port, UART_CLOCK, UART_BAUDRATE);
	ERROR("%s: plat_dcm_init_type=0x%x, MP2_SYNC_DCM=0x%x\n",
	       __func__, plat_dcm_init_type, mmio_read_32(MP2_SYNC_DCM));
	console_uninit();
#endif
}

void plat_dcm_restore_cluster_on(unsigned long mpidr)
{
	unsigned long cluster_id =
		(mpidr & MPIDR_CLUSTER_MASK) >> MPIDR_AFFINITY_BITS;
#ifdef DCM_DEBUG
	console_init(gteearg.atf_log_port, UART_CLOCK, UART_BAUDRATE);
	ERROR("%s: cluster_id=0x%lx\n", __func__, cluster_id);
	console_uninit();
#endif
#ifdef DCM_SKIP_RESTORE
	return;
#endif

	switch (cluster_id) {
	case 0x1:
		dcm_lock_get();
		if (plat_dcm_init_type & BIG_CORE_DCM_TYPE)
			plat_dcm_big_core_sync(1);
		else
			plat_dcm_big_core_sync(0); /* do clear to prevent if buck is not off */
		dcm_lock_release();
		break;
	default:
		break;
	}
}

void plat_dcm_msg_handler(uint64_t x1)
{
	plat_dcm_init_type = x1 & ALL_DCM_TYPE;
#ifdef DCM_DEBUG
	console_init(gteearg.atf_log_port, UART_CLOCK, UART_BAUDRATE);
	ERROR("%s: plat_dcm_init_type=0x%x\n", __func__, plat_dcm_init_type);
	console_uninit();
#endif
}

unsigned long plat_dcm_get_enabled_cnt(uint64_t type)
{
#ifdef DCM_DEBUG
	console_init(gteearg.atf_log_port, UART_CLOCK, UART_BAUDRATE);
	ERROR("%s: plat_dcm_init_type=0x%x, type=0x%lx\n", __func__, plat_dcm_init_type, type);
	console_uninit();
#endif
	switch (type) {
	case BIG_CORE_DCM_TYPE:
		return dcm_big_core_cnt;
	default:
		return 0;
	}
}

void plat_dcm_init(void)
{
#ifdef DCM_DEBUG
	console_init(gteearg.atf_log_port, UART_CLOCK, UART_BAUDRATE);
	ERROR("%s: plat_dcm_init_type=0x%x\n", __func__, plat_dcm_init_type);
	console_uninit();
#endif
	dcm_lock_init();
}

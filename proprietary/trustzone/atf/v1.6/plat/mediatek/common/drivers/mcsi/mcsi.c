/*
 * Copyright (c) 2013-2014, ARM Limited and Contributors. All rights reserved.
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
#include <arch_helpers.h>
#include <assert.h>
#include <debug.h>
#include <mcsi.h>
#include <mmio.h>
#include <scu.h>
#include <mcucfg.h>
#include <delay_timer.h>
#define MAX_CLUSTERS		5

static unsigned long cci_base_addr;
static unsigned int cci_cluster_ix_to_iface[MAX_CLUSTERS];

void mcsi_init(unsigned long cci_base,
		unsigned int num_cci_masters)
{
	int i;

	assert(cci_base);
	assert(num_cci_masters < MAX_CLUSTERS);

	cci_base_addr = cci_base;

	for (i = 0; i < num_cci_masters; i++) {
		cci_cluster_ix_to_iface[i] = SLAVE_IFACE_OFFSET(i);
	}
	/* mmio_write_32(cci_base_addr, config); */
}

static inline unsigned long get_slave_iface_base(unsigned long mpidr)
{
	/*
	 * We assume the TF topology code allocates affinity instances
	 * consecutively from zero.
	 * It is a programming error if this is called without initializing
	 * the slave interface to use for this cluster.
	 */
	unsigned int cluster_id =
		(mpidr >> MPIDR_AFF1_SHIFT) & MPIDR_AFFLVL_MASK;

	assert(cluster_id < MAX_CLUSTERS);
	assert(cci_cluster_ix_to_iface[cluster_id] != 0);

	return cci_base_addr + cci_cluster_ix_to_iface[cluster_id];
}

void cci_enable_cluster_coherency(unsigned long mpidr)
{
	unsigned long slave_base;
	unsigned int support_ability;
	unsigned int config = 0;

	assert(cci_base_addr);
	slave_base  = get_slave_iface_base(mpidr);
	support_ability = mmio_read_32(slave_base);

#ifdef MTK_ICCS_SUPPORT
	if (((mpidr >> MPIDR_AFF1_SHIFT) & MPIDR_AFFLVL_MASK) == 0x2)
		mmio_write_32(slave_base + BD_CTRL_REG,
			mmio_read_32(slave_base + BD_CTRL_REG) | USE_SHARED_CACHE | CACHE_INSTR_EN);
	else
		mmio_write_32(slave_base + BD_CTRL_REG,
			mmio_read_32(slave_base + BD_CTRL_REG) | USE_SHARED_CACHE);
#endif
	while (mmio_read_32(cci_base_addr + SNP_PENDING_REG) >> SNP_PENDING);

	if (support_ability & SNP_SUPPORT)
		config = SNOOP_EN_BIT;
	if (support_ability & DVM_SUPPORT)
		config = config | DVM_EN_BIT;

	mmio_write_32(slave_base, support_ability | config);

	/* Wait for the dust to settle down */
	while (mmio_read_32(cci_base_addr + SNP_PENDING_REG) >> SNP_PENDING);

#ifdef MTK_ICCS_SUPPORT
	/* FIXME: Should be removed before submitting*/
	*(volatile unsigned int *)(cci_base_addr + 0x0) |= 0x14; /* Unlock MCSI Regs*/
#endif
	return;

}

#ifdef ERRATA_MCSIB_SW
extern int mcsib_sw_workaround_main(void);
#endif
void cci_disable_cluster_coherency(unsigned long mpidr)
{
	unsigned long slave_base;
	unsigned int config = 0;

	assert(cci_base_addr);
	slave_base = get_slave_iface_base(mpidr);

	while (mmio_read_32(cci_base_addr + SNP_PENDING_REG) >> SNP_PENDING);


	config = mmio_read_32(slave_base);
#ifdef MTK_ICCS_SUPPORT
	if (((mpidr >> MPIDR_AFF1_SHIFT) & MPIDR_AFFLVL_MASK) != 0x2)
		config &= ~(DVM_EN_BIT);
	else
#endif
		config &= ~(DVM_EN_BIT | SNOOP_EN_BIT);

	/* Disable Snoops and DVM messages */
	mmio_write_32(slave_base, config);

#ifdef ERRATA_MCSIB_SW
	mcsib_sw_workaround_main();
#endif

	/* Wait for the dust to settle down */
	while (mmio_read_32(cci_base_addr + SNP_PENDING_REG) >> SNP_PENDING);
	return;
}

void cci_secure_switch(unsigned int ns)
{
	unsigned int config;

	assert(ns >= 0);
	config = mmio_read_32(cci_base_addr + CENTRAL_CTRL_REG);
	if (ns == NS_ACC)
		config |= SECURE_ACC_EN;
	else
		config &= ~SECURE_ACC_EN;
	mmio_write_32(cci_base_addr + CENTRAL_CTRL_REG, config);
}

void cci_pmu_secure_switch(unsigned int ns)
{
	unsigned int config;

	assert(ns >= 0);
	config = mmio_read_32(cci_base_addr + CENTRAL_CTRL_REG);
	if (ns == NS_ACC)
		config |= PMU_SECURE_ACC_EN;
	else
		config &= ~PMU_SECURE_ACC_EN;
	mmio_write_32(cci_base_addr + CENTRAL_CTRL_REG, config);
}

void cci_init_sf(void)
{
	while (mmio_read_32(cci_base_addr + SNP_PENDING_REG) >> SNP_PENDING);
	/* init sf1 */
	mmio_write_32(cci_base_addr + SF_INIT_REG, TRIG_SF1_INIT);
	while (mmio_read_32(cci_base_addr + SF_INIT_REG) & TRIG_SF1_INIT);
	while (!(mmio_read_32(cci_base_addr + SF_INIT_REG) & SF1_INIT_DONE));
	/* init sf2 */
	mmio_write_32(cci_base_addr + SF_INIT_REG, TRIG_SF2_INIT);
	while (mmio_read_32(cci_base_addr + SF_INIT_REG) & TRIG_SF2_INIT);
	while (!(mmio_read_32(cci_base_addr + SF_INIT_REG) & SF2_INIT_DONE));
}

void cci_interrupt_en(void)
{
	unsigned int config;

	config = mmio_read_32(cci_base_addr + CENTRAL_CTRL_REG);
	config |= INT_EN;
	mmio_write_32(cci_base_addr + CENTRAL_CTRL_REG, config);
}

unsigned long cci_reg_access(unsigned int op, unsigned long offset, unsigned long val)
{
	unsigned long ret = 0;

	if (!cci_base_addr)
		goto err;

	if (offset > MSCI_MEMORY_SZ)
		goto err;

	switch (op) {
		case MCSI_REG_ACCESS_READ:
			ret = mmio_read_32(cci_base_addr + offset);
			break;
		case MCSI_REG_ACCESS_WRITE:
			mmio_write_32(cci_base_addr + offset, val);
			dsb();
			break;
		case MCSI_REG_ACCESS_SET_BITMASK:
			ret = mmio_read_32(cci_base_addr + offset);
			mmio_write_32(cci_base_addr + offset, ret|val);
			dsb();
			break;
		case MCSI_REG_ACCESS_CLEAR_BITMASK:
			ret = mmio_read_32(cci_base_addr + offset);
			mmio_write_32(cci_base_addr + offset, ret&~(val));
			dsb();
			break;
		default:
			break;
	}
	return ret;
err:
	return 0;
}

#ifdef MTK_ICCS_SUPPORT

unsigned char iccs_next_cache_shared_state;

void iccs_cache_shared_enable(unsigned char cluster_id)
{
	INFO("iccs_cache_shared_enable, cluster_id-%d\n", cluster_id);
	unsigned long slave_base = cci_base_addr + SLAVE_IFACE_OFFSET(cluster_id);

	if (cluster_id > NR_MAX_SLV)
		return;

	enable_scu(cluster_id << 8);

	mmio_write_32(slave_base + SNOOP_CTRL_REG,
		(mmio_read_32(slave_base + SNOOP_CTRL_REG) & ~DVM_EN_BIT) | SNOOP_EN_BIT);

	while (mmio_read_32(cci_base_addr + SNP_PENDING_REG) >> SNP_PENDING);

	mmio_write_32(slave_base + BD_CTRL_REG,
		mmio_read_32(slave_base + BD_CTRL_REG) & ~(USE_SHARED_CACHE | CACHE_INSTR_EN));

	mmio_write_32(slave_base + BD_CTRL_REG,
		(mmio_read_32(slave_base + BD_CTRL_REG) | CACHE_SHARED_PRE_EN | IDLE_CACHE));

	mmio_write_32(slave_base + BD_CTRL_REG,
		(mmio_read_32(slave_base + BD_CTRL_REG) | CACHE_SHARED_POST_EN));

}

#define L2C_FLUSH_DONE          (1 << 18)
#define L2C_FLUSH_REQ           (1 << 19)

void iccs_cache_shared_disable(unsigned char cluster_id)
{

	INFO("iccs_cache_shared_disable, cluster_id-%d\n", cluster_id);
	unsigned long slave_base = cci_base_addr + SLAVE_IFACE_OFFSET(cluster_id);
	unsigned long mcucfg_base = MCUCFG_BASE + 0x200 * cluster_id;
	unsigned long mpx_axi_base = MP0_AXI_CONFIG  + 0x200 * cluster_id;

	if (cluster_id > NR_MAX_SLV)
		return;

	mmio_write_32(slave_base + BD_CTRL_REG,
		mmio_read_32(slave_base + BD_CTRL_REG) & ~CACHE_SHARED_PRE_EN);

	/* Waiting until no any ACP pending */
	while (mmio_read_32(cci_base_addr + ACP_PENDING_REG) & ACP_PENDING_MASK);

	mmio_write_32(slave_base + BD_CTRL_REG,
		mmio_read_32(slave_base + BD_CTRL_REG) & ~(CACHE_SHARED_POST_EN | IDLE_CACHE));

	while (mmio_read_32(cci_base_addr + SNP_PENDING_REG) >> SNP_PENDING);

	mmio_write_32(slave_base + SNOOP_CTRL_REG,
		(mmio_read_32(slave_base + SNOOP_CTRL_REG) & ~(DVM_EN_BIT | SNOOP_EN_BIT)));

	while (mmio_read_32(cci_base_addr + SNP_PENDING_REG) >> SNP_PENDING);


	/*HW Flush L2$*/
	mmio_write_32(mpx_axi_base,
		(mmio_read_32(mpx_axi_base) & ~MPx_AXI_CONFIG_ACINACTM) | MPx_AXI_CONFIG_AINACTS);

	mmio_write_32(mcucfg_base, mmio_read_32(mcucfg_base) | L2C_FLUSH_REQ);

	while (!(mmio_read_32(mcucfg_base) & L2C_FLUSH_DONE));
	mmio_write_32(mcucfg_base, mmio_read_32(mcucfg_base) & (~L2C_FLUSH_REQ));

	mmio_write_32(mpx_axi_base,
		(mmio_read_32(mpx_axi_base) | MPx_AXI_CONFIG_ACINACTM | MPx_AXI_CONFIG_AINACTS));
	INFO(" exit iccs_cache_shared_disable , cluster_id-%d\n", cluster_id);
/* disable_scu(cluster_id << 8); */

}

unsigned char iccs_is_cluster_cache_shared(unsigned char cluster_id)
{
	if (cluster_id > NR_MAX_SLV)
		return 0;

	if (mmio_read_32(cci_base_addr + SLAVE_IFACE_OFFSET(cluster_id) + BD_CTRL_REG) & (CACHE_SHARED_POST_EN))
		return 1;

	return 0;
}

unsigned char iccs_get_curr_cache_shared_state(void)
{
	unsigned char i;
	unsigned char ret = 0;

	assert(PLATFORM_CLUSTER_COUNT <= NR_MAX_SLV);

	for (i = 0; i <= PLATFORM_CLUSTER_COUNT-1; ++i)
		if (mmio_read_32(cci_base_addr + SLAVE_IFACE_OFFSET(i) + BD_CTRL_REG) & (CACHE_SHARED_POST_EN))
			ret |= 1 << i;

	return ret;
}

unsigned char iccs_get_next_cache_shared_state(void)
{
	return iccs_next_cache_shared_state;
}

void iccs_set_next_cache_shared_state(unsigned char state)
{
	iccs_next_cache_shared_state = state;
}

unsigned char iccs_state(unsigned int op, unsigned char state, unsigned char cluster)
{
	unsigned char ret = 22;

	switch (op) {
	case ICCS_GET_CURR_STATE:
		ret = iccs_get_curr_cache_shared_state();
		break;
	case ICCS_GET_NEXT_STATE:
		ret = iccs_get_next_cache_shared_state();
		break;
	case ICCS_SET_NEXT_STATE:
		iccs_set_next_cache_shared_state(state);
		break;
	case ICCS_SET_CACHE_SHARED:
		if (state == 0)
			iccs_cache_shared_disable(cluster);
		else
			iccs_cache_shared_enable(cluster);

		break;
	default:
		break;
	}

	return ret;
}
#endif

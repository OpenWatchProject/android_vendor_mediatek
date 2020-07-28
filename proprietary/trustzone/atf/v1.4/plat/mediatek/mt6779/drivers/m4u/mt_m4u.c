/*
 * Copyright (c) 2015, ARM Limited and Contributors. All rights reserved.
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
#include <debug.h>
#include <mmio.h>
#include <mtk_sip_svc.h>
#include <platform_def.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <mt_m4u.h>
#include <mt_m4u_platform.h>
#include <xlat_tables_v2.h>
#include <mtk_plat_common.h>
#include <platform.h>
#include <console.h>
#include <spinlock.h>


static int __m4u_print_active_port_sec(int port)
{
	uint64_t larb_base_pa;

	int mmu_en, sec;
	int larb, larb_port;
	int ret = 0, reg = 0;


	INFO("m4u secure ports: ====>\n");

	larb = m4u_port_2_larb_id(port);
	larb_port = m4u_port_2_larb_port(port);

	larb_base_pa = smiLarbBasePA[larb];

	ret = mmap_add_dynamic_region((larb_base_pa & ~(PAGE_SIZE_MASK)), (larb_base_pa & ~(PAGE_SIZE_MASK)),
		SMI_LARB_SZ, MT_DEVICE | MT_RW | MT_NS);
	if (ret) {
		INFO("%s dynamic mmap fail, ret = %d\n", __func__, ret);
		return 0;
	}

	INFO("m4u secure larb = %d pa = 0x%lx, va = 0x%lx\n", larb, smiLarbBasePA[larb], larb_base_pa);

	if (larb_base_pa == 0) {
		INFO("m4u secure larb base address invalid!!\n");
		return 0;
	}
	reg = mmio_read_32(larb_base_pa + SMI_LARB_SEC_CONx(larb_port));
	mmu_en = mmio_read_32(larb_base_pa + SMI_LARB_SEC_CONx(larb_port)) &  F_SMI_SEC_MMU_EN(1);
	sec = mmio_read_32(larb_base_pa + SMI_LARB_SEC_CONx(larb_port)) & F_SMI_SEC_EN(1);

	INFO("reg:0x%x, %s(%s,%s)\n", reg, m4u_get_port_name(port),
		(mmu_en) ? "enable" : "disable",
		(sec) ? "sec" : "non-sec"
		);

	ret = mmap_remove_dynamic_region((larb_base_pa & ~(PAGE_SIZE_MASK)), SMI_LARB_SZ);
	if (ret)
		INFO("%s dynamic unmap fail, ret = %d\n", __func__, ret);

	return 0;

}

static inline void m4u_clear_intr_sec(unsigned int m4u_base)
{
	unsigned int temp = mmio_read_32(m4u_base) | F_INT_L2_CLR_BIT_SEC;

	mmio_write_32((m4u_base + REG_MMU_INT_L2_CONTROL_SEC), temp);
	INFO("m4u secure clear irq, ,m4u_base=0x%x\n", m4u_base);
}

int32_t m4u_dump_secure_reg(uint64_t m4u_id, uint32_t *val)
{
	int32_t ret = 0;
	int m4u_slave_id;
	unsigned int regval, IntrSrc;
	int layer, write, m4u_port;
	unsigned int fault_mva, fault_pa;
	unsigned int m4u_base = 0;

	INFO("m4u in secure world, m4u_id=%lu\n", m4u_id);

	if (val == NULL)
		return MTK_SIP_E_INVALID_PARAM;

	if (m4u_id == 0) {
		m4u_base = M4U_SECURE_BANK_MM;
	} else if (m4u_id == 1) {
		m4u_base = M4U_SECURE_BANK_VPU;
	} else if (m4u_id == M4U_ATF_SECURITY_DEBUG_EN) {
		INFO("m4u enable mm_iommu and vpu_iommu security_debug\n");
		m4u_security_debug_en();
		return ret;
	} else if (m4u_id == M4U_ATF_BANK1_4_TF) {
		INFO("m4u enable mm_iommu and vpu_iommu tf\n");
		m4u_bank1_4_tf_en();
		return ret;
	} else if (m4u_id == M4U_ATF_DUMP_INFO) {
		INFO("m4u dump hw info\n");
		m4u_dump_bank1_4_info();
		return ret;
	} else {
		return MTK_SIP_E_INVALID_PARAM;
	}

	/* secure L2 interrupt */
	regval = mmio_read_32(m4u_base + REG_MMU_L2_FAULT_ST_SEC);

	INFO("m4u secure L2 interrupt m4u_base=0x%x, m4u_id=%lu, sta=0x%x\n", m4u_base, m4u_id, regval);

	if (regval & F_INT_L2_TABLE_WALK_FAULT_SEC) {
		unsigned int fault_va, layer;

		fault_va = mmio_read_32(m4u_base + REG_MMU_TBWALK_FAULT_VA_SEC);
		layer = fault_va & 1;
		fault_va &= (~1);
		INFO("m4u secure L2 table walk fault: mva=0x%x, layer=%d, m4u_id=%lu\n", fault_va, layer, m4u_id);
	}

	if (regval & F_INT_L2_INVALD_DONE_SEC)
		INFO("m4u secure iommu%lu_%d L2_INVALD_DONE int happens!!\n", m4u_id, 0);

	/* secure main interrupt */
	IntrSrc = mmio_read_32(m4u_base + REG_MMU_MAIN_FAULT_ST_SEC);

	INFO("m4u secure main interrupt happened: sta=0x%x, m4u_id=%lu\n", IntrSrc, m4u_id);

	if (IntrSrc & F_INT_MMU0_MSK_SEC)
		m4u_slave_id = 0;
	else if (IntrSrc & F_INT_MMU1_MSK_SEC)
		m4u_slave_id = 1;
	else {
		INFO("m4u secure interrupt error: status = 0x%x\n", IntrSrc);
		m4u_clear_intr_sec(m4u_base);
		return 0;
	}

	/* read error info from registers */
	fault_mva = mmio_read_32(m4u_base + REG_MMU_FAULT_VA_SEC(m4u_slave_id));
	layer = !!(fault_mva & F_MMU_FAULT_VA_LAYER_BIT_SEC);
	write = !!(fault_mva & F_MMU_FAULT_VA_WRITE_BIT_SEC);
	fault_mva &= F_MMU_FAULT_VA_MSK_SEC;
	fault_pa = mmio_read_32(m4u_base + REG_MMU_INVLD_PA_SEC(m4u_slave_id));
	regval = mmio_read_32(m4u_base + REG_MMU_INT_ID_SEC(m4u_slave_id));
	m4u_port = m4u_get_port_id(regval);

	if (IntrSrc & F_INT_TRANSLATION_FAULT_SEC(m4u_slave_id)) {
		INFO(
			"m4u secure fault: m4u_id=%lu, slave_id = %d, mva=0x%x, pa=0x%x, layer=%d, wr=%d, fault_id=0x%x\n",
				m4u_id,
				m4u_slave_id,
				fault_mva, fault_pa,
				layer, write, regval);

		__m4u_print_active_port_sec(m4u_port);

		if (m4u_id == 0)
			INFO("m4u mm secure CRDISPATCH_KEY:M4U_%s translation fault: port=%s, mva=0x%x, pa=0x%x\n",
			 m4u_get_port_name(m4u_port),
			m4u_get_port_name(m4u_port),
			fault_mva, fault_pa);
		else if (m4u_id == 1)
			INFO("m4u vpu secure CRDISPATCH_KEY:M4U_%s translation fault: port=%s, mva=0x%x, pa=0x%x\n",
			 m4u_get_vpu_port_name(regval),
			m4u_get_vpu_port_name(regval),
			fault_mva, fault_pa);

		ret = 1;
		*val = m4u_port;
	}
	if (IntrSrc & F_INT_INVALID_PA_FAULT_SEC(m4u_slave_id)) {
		if (!(IntrSrc & F_INT_TRANSLATION_FAULT_SEC(m4u_slave_id)))
			INFO("m4u secure iommu%lu_%d INVALID_PA_FAULT int happens!!\n", m4u_id, m4u_slave_id);
	}
	m4u_clear_intr_sec(m4u_base);

	INFO("m4u leave secure world\n");

	return ret;
}

void m4u_security_debug_en(void)
{
	uint32_t regVal;

	/* enabel MM and VPU security debug */
	regVal = mmio_read_32(MM_IOMMU_SEC_DEBUG);
	SET_BIT(regVal, 0);
	mmio_write_32(MM_IOMMU_SEC_DEBUG, regVal);
	regVal = mmio_read_32(VPU_IOMMU_SEC_DEBUG);
	SET_BIT(regVal, 0);
	mmio_write_32(VPU_IOMMU_SEC_DEBUG, regVal);
	INFO("m4u set security_debug MM:0x%x, VPU:0x%x\n",
		mmio_read_32(MM_IOMMU_SEC_DEBUG),
		mmio_read_32(VPU_IOMMU_SEC_DEBUG)
		);
}

void m4u_bank1_4_tf_en(void)
{
	/*enable tf for project and secure world */
	mmio_write_32((M4U_MM_BANK1 + REG_MMU_INT_MAIN_CONTROL_SEC), 0xffffffff);
	mmio_write_32((M4U_MM_BANK2 + REG_MMU_INT_MAIN_CONTROL_SEC), 0xffffffff);
	mmio_write_32((M4U_MM_BANK3 + REG_MMU_INT_MAIN_CONTROL_SEC), 0xffffffff);
	mmio_write_32((M4U_MM_BANK4 + REG_MMU_INT_MAIN_CONTROL_SEC), 0xffffffff);

	mmio_write_32((M4U_VPU_BANK1 + REG_MMU_INT_MAIN_CONTROL_SEC), 0xffffffff);
	mmio_write_32((M4U_VPU_BANK2 + REG_MMU_INT_MAIN_CONTROL_SEC), 0xffffffff);
	mmio_write_32((M4U_VPU_BANK3 + REG_MMU_INT_MAIN_CONTROL_SEC), 0xffffffff);
	mmio_write_32((M4U_VPU_BANK4 + REG_MMU_INT_MAIN_CONTROL_SEC), 0xffffffff);

	INFO("m4u mm intr_en P1:0x%x, P2:0x%x, P3:0x%x, S:0x%x\n",
		mmio_read_32(M4U_MM_BANK1 + REG_MMU_INT_MAIN_CONTROL_SEC),
		mmio_read_32(M4U_MM_BANK2 + REG_MMU_INT_MAIN_CONTROL_SEC),
		mmio_read_32(M4U_MM_BANK3 + REG_MMU_INT_MAIN_CONTROL_SEC),
		mmio_read_32(M4U_MM_BANK4 + REG_MMU_INT_MAIN_CONTROL_SEC));

	INFO("m4u vpu intr_en P1:0x%x, P2:0x%x, P3:0x%x, S:0x%x\n",
		mmio_read_32(M4U_VPU_BANK1 + REG_MMU_INT_MAIN_CONTROL_SEC),
		mmio_read_32(M4U_VPU_BANK2 + REG_MMU_INT_MAIN_CONTROL_SEC),
		mmio_read_32(M4U_VPU_BANK3 + REG_MMU_INT_MAIN_CONTROL_SEC),
		mmio_read_32(M4U_VPU_BANK4 + REG_MMU_INT_MAIN_CONTROL_SEC));
}

void m4u_dump_bank1_4_info(void)
{
	INFO("m4u dump hw info start\n");

	INFO("m4u dump security_debug MM:0x%x, VPU:0x%x\n",
		mmio_read_32(MM_IOMMU_SEC_DEBUG),
		mmio_read_32(VPU_IOMMU_SEC_DEBUG)
		);

	INFO("m4u dump mm domian\n");

	INFO("m4u mm intr_en P1:0x%x, P2:0x%x, P3:0x%x, S:0x%x\n",
		mmio_read_32(M4U_MM_BANK1 + REG_MMU_INT_MAIN_CONTROL_SEC),
		mmio_read_32(M4U_MM_BANK2 + REG_MMU_INT_MAIN_CONTROL_SEC),
		mmio_read_32(M4U_MM_BANK3 + REG_MMU_INT_MAIN_CONTROL_SEC),
		mmio_read_32(M4U_MM_BANK4 + REG_MMU_INT_MAIN_CONTROL_SEC));

	INFO("m4u mm intr_sta P1:0x%x, P2:0x%x, P3:0x%x, S:0x%x\n",
		mmio_read_32(M4U_MM_BANK1 + REG_MMU_MAIN_FAULT_ST_SEC),
		mmio_read_32(M4U_MM_BANK2 + REG_MMU_MAIN_FAULT_ST_SEC),
		mmio_read_32(M4U_MM_BANK3 + REG_MMU_MAIN_FAULT_ST_SEC),
		mmio_read_32(M4U_MM_BANK4 + REG_MMU_MAIN_FAULT_ST_SEC));

	INFO("m4u mm repalce_pa P1:0x%x, P2:0x%x, P3:0x%x, S:0x%x\n",
		mmio_read_32(M4U_MM_BANK1 + REG_MMU_IVRP_PADDR_SEC),
		mmio_read_32(M4U_MM_BANK2 + REG_MMU_IVRP_PADDR_SEC),
		mmio_read_32(M4U_MM_BANK3 + REG_MMU_IVRP_PADDR_SEC),
		mmio_read_32(M4U_MM_BANK4 + REG_MMU_IVRP_PADDR_SEC));

	INFO("m4u m fault_id0 P1:0x%x, P2:0x%x, P3:0x%x, S:0x%x\n",
		mmio_read_32(M4U_MM_BANK1 + REG_MMU_INT_ID_SEC(0)),
		mmio_read_32(M4U_MM_BANK2 + REG_MMU_INT_ID_SEC(0)),
		mmio_read_32(M4U_MM_BANK3 + REG_MMU_INT_ID_SEC(0)),
		mmio_read_32(M4U_MM_BANK4 + REG_MMU_INT_ID_SEC(0)));

	INFO("m4u mm fault_id1 P1:0x%x, P2:0x%x, P3:0x%x, S:0x%x\n",
		mmio_read_32(M4U_MM_BANK1 + REG_MMU_INT_ID_SEC(1)),
		mmio_read_32(M4U_MM_BANK2 + REG_MMU_INT_ID_SEC(1)),
		mmio_read_32(M4U_MM_BANK3 + REG_MMU_INT_ID_SEC(1)),
		mmio_read_32(M4U_MM_BANK4 + REG_MMU_INT_ID_SEC(1)));

	INFO("m4u mm fault_va0 P1:0x%x, P2:0x%x, P3:0x%x, S:0x%x\n",
			mmio_read_32(M4U_MM_BANK1 + REG_MMU_FAULT_VA_SEC(0)),
			mmio_read_32(M4U_MM_BANK2 + REG_MMU_FAULT_VA_SEC(0)),
			mmio_read_32(M4U_MM_BANK3 + REG_MMU_FAULT_VA_SEC(0)),
			mmio_read_32(M4U_MM_BANK4 + REG_MMU_FAULT_VA_SEC(0)));

	INFO("m4u mm fault_va1 P1:0x%x, P2:0x%x, P3:0x%x, S:0x%x\n",
			mmio_read_32(M4U_MM_BANK1 + REG_MMU_FAULT_VA_SEC(1)),
			mmio_read_32(M4U_MM_BANK2 + REG_MMU_FAULT_VA_SEC(1)),
			mmio_read_32(M4U_MM_BANK3 + REG_MMU_FAULT_VA_SEC(1)),
			mmio_read_32(M4U_MM_BANK4 + REG_MMU_FAULT_VA_SEC(1)));

	INFO("m4u dump vpu domian\n");

	INFO("m4u vpu intr_en P1:0x%x, P2:0x%x, P3:0x%x, S:0x%x\n",
		mmio_read_32(M4U_VPU_BANK1 + REG_MMU_INT_MAIN_CONTROL_SEC),
		mmio_read_32(M4U_VPU_BANK2 + REG_MMU_INT_MAIN_CONTROL_SEC),
		mmio_read_32(M4U_VPU_BANK3 + REG_MMU_INT_MAIN_CONTROL_SEC),
		mmio_read_32(M4U_VPU_BANK4 + REG_MMU_INT_MAIN_CONTROL_SEC));

	INFO("m4u vpu intr_sta P1:0x%x, P2:0x%x, P3:0x%x, S:0x%x\n",
		mmio_read_32(M4U_VPU_BANK1 + REG_MMU_MAIN_FAULT_ST_SEC),
		mmio_read_32(M4U_VPU_BANK2 + REG_MMU_MAIN_FAULT_ST_SEC),
		mmio_read_32(M4U_VPU_BANK3 + REG_MMU_MAIN_FAULT_ST_SEC),
		mmio_read_32(M4U_VPU_BANK4 + REG_MMU_MAIN_FAULT_ST_SEC));

	INFO("m4u vpu repalce_pa P1:0x%x, P2:0x%x, P3:0x%x, S:0x%x\n",
		mmio_read_32(M4U_VPU_BANK1 + REG_MMU_IVRP_PADDR_SEC),
		mmio_read_32(M4U_VPU_BANK2 + REG_MMU_IVRP_PADDR_SEC),
		mmio_read_32(M4U_VPU_BANK3 + REG_MMU_IVRP_PADDR_SEC),
		mmio_read_32(M4U_VPU_BANK4 + REG_MMU_IVRP_PADDR_SEC));

	INFO("m4u vpu fault_id0 P1:0x%x, P2:0x%x, P3:0x%x, S:0x%x\n",
		mmio_read_32(M4U_VPU_BANK1 + REG_MMU_INT_ID_SEC(0)),
		mmio_read_32(M4U_VPU_BANK2 + REG_MMU_INT_ID_SEC(0)),
		mmio_read_32(M4U_VPU_BANK3 + REG_MMU_INT_ID_SEC(0)),
		mmio_read_32(M4U_VPU_BANK4 + REG_MMU_INT_ID_SEC(0)));

	INFO("m4u vpu fault_id1 P1:0x%x, P2:0x%x, P3:0x%x, S:0x%x\n",
		mmio_read_32(M4U_VPU_BANK1 + REG_MMU_INT_ID_SEC(1)),
		mmio_read_32(M4U_VPU_BANK2 + REG_MMU_INT_ID_SEC(1)),
		mmio_read_32(M4U_VPU_BANK3 + REG_MMU_INT_ID_SEC(1)),
		mmio_read_32(M4U_VPU_BANK4 + REG_MMU_INT_ID_SEC(1)));

	INFO("m4u vpu fault_va0 P1:0x%x, P2:0x%x, P3:0x%x, S:0x%x\n",
			mmio_read_32(M4U_VPU_BANK1 + REG_MMU_FAULT_VA_SEC(0)),
			mmio_read_32(M4U_VPU_BANK2 + REG_MMU_FAULT_VA_SEC(0)),
			mmio_read_32(M4U_VPU_BANK3 + REG_MMU_FAULT_VA_SEC(0)),
			mmio_read_32(M4U_VPU_BANK4 + REG_MMU_FAULT_VA_SEC(0)));

	INFO("m4u vpu fault_va1 P1:0x%x, P2:0x%x, P3:0x%x, S:0x%x\n",
			mmio_read_32(M4U_VPU_BANK1 + REG_MMU_FAULT_VA_SEC(1)),
			mmio_read_32(M4U_VPU_BANK2 + REG_MMU_FAULT_VA_SEC(1)),
			mmio_read_32(M4U_VPU_BANK3 + REG_MMU_FAULT_VA_SEC(1)),
			mmio_read_32(M4U_VPU_BANK4 + REG_MMU_FAULT_VA_SEC(1)));

	INFO("m4u dump hw info end\n");
}



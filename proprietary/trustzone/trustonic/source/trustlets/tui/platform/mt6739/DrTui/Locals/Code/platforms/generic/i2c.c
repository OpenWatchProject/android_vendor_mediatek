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

#define TAG "[I2C-TEE]"

#include "drStd.h"
#include "DrApi/DrApi.h"
#include "drError.h"
#include "mt_typedefs.h"
#include "DrApi/DrApiMm.h"
#include "drTuiHal.h"
#include "dapc.h"
#include "tlDapcIndex.h"

#include "log.h"
#include "i2c.h"

//#define SECURE_I2C_TEST
#define SECURE_LOCK
extern void set_module_apc(unsigned int module, unsigned int domain_num , unsigned int permission_control);
extern int start_devapc();

#ifdef NO_I2C_LTIMING
volatile U32 I2C_TIMING_REG_BACKUP[I2C_MAX_CHANNEL] = {0};
#endif
static struct i2c_dma_info g_dma_regs[I2C_MAX_CHANNEL];
uint8_t *i2c0_va = 0;
uint8_t *i2c1_va = 0;
uint8_t *i2c2_va = 0;
uint8_t *i2c3_va = 0;
uint8_t *i2c4_va = 0;
uint8_t *i2c5_va = 0;
uint8_t *clock_gate = 0;
uint8_t *DMA_BASE_VA = 0;

static inline void i2c_writel(mt_i2c * i2c, U8 offset, U16 value)
{
	I2C_WRITE((i2c->base) + (offset), value);
}

static inline U32 i2c_readl(mt_i2c * i2c, U8 offset)
{
	return I2C_READ((i2c->base) + (offset));
}

static void record_i2c_dma_info(mt_i2c *i2c)
{
	g_dma_regs[i2c->id].base = (unsigned long)i2c->pdmabase;
	g_dma_regs[i2c->id].int_flag = I2C_READ(i2c->pdmabase + OFFSET_INT_FLAG);
	g_dma_regs[i2c->id].int_en = I2C_READ(i2c->pdmabase + OFFSET_INT_EN);
	g_dma_regs[i2c->id].en = I2C_READ(i2c->pdmabase + OFFSET_EN);
	g_dma_regs[i2c->id].rst = I2C_READ(i2c->pdmabase + OFFSET_RST);
	g_dma_regs[i2c->id].stop = I2C_READ(i2c->pdmabase + OFFSET_STOP);
	g_dma_regs[i2c->id].flush = I2C_READ(i2c->pdmabase + OFFSET_FLUSH);
	g_dma_regs[i2c->id].con = I2C_READ(i2c->pdmabase + OFFSET_CON);
	g_dma_regs[i2c->id].tx_mem_addr = I2C_READ(i2c->pdmabase + OFFSET_TX_MEM_ADDR);
	g_dma_regs[i2c->id].rx_mem_addr = I2C_READ(i2c->pdmabase + OFFSET_RX_MEM_ADDR);
	g_dma_regs[i2c->id].tx_len = I2C_READ(i2c->pdmabase + OFFSET_TX_LEN);
	g_dma_regs[i2c->id].rx_len = I2C_READ(i2c->pdmabase + OFFSET_RX_LEN);
	g_dma_regs[i2c->id].int_buf_size = I2C_READ(i2c->pdmabase + OFFSET_INT_BUF_SIZE);
	g_dma_regs[i2c->id].debug_sta = I2C_READ(i2c->pdmabase + OFFSET_DEBUG_STA);
	g_dma_regs[i2c->id].tx_mem_addr2 = I2C_READ(i2c->pdmabase + OFFSET_TX_MEM_ADDR2);
	g_dma_regs[i2c->id].rx_mem_addr2 = I2C_READ(i2c->pdmabase + OFFSET_RX_MEM_ADDR2);
}

static inline U32 i2c_clock_enable(mt_i2c * i2c)
{
#ifdef TEE_I2C_CLOCK
	switch (i2c->id) {
		case 0:
			I2C_WRITE(clock_gate + OFFSET_SW_CG_0_CLR, 1 << I2C0_CG);
			break;
		case 1:
			I2C_WRITE(clock_gate + OFFSET_SW_CG_0_CLR, 1 << I2C1_CG);
			break;
		case 2:
			I2C_WRITE(clock_gate + OFFSET_SW_CG_0_CLR, 1 << I2C2_CG);
			break;
		case 3:
			I2C_WRITE(clock_gate + OFFSET_SW_CG_0_CLR, 1 << I2C3_CG);
			break;
		case 4:
			I2C_WRITE(clock_gate + OFFSET_SW_CG_2_CLR, 1 << I2C4_CG);
			break;
		case 5:
			I2C_WRITE(clock_gate + OFFSET_SW_CG_2_CLR, 1 << I2C5_CG);
			break;
		default:
			I2CERR("invalid para: i2c->id=%d\n", i2c->id);
	}
#endif
	return 0;
}

static inline U32 i2c_clock_disable(mt_i2c * i2c)
{
#ifdef TEE_I2C_CLOCK
	switch(i2c->id) {
		case 0:
			I2C_WRITE(clock_gate + OFFSET_SW_CG_0_SET, 1 << I2C0_CG);
			break;
		case 1:
			I2C_WRITE(clock_gate + OFFSET_SW_CG_0_SET, 1 << I2C1_CG);
			break;
		case 2:
			I2C_WRITE(clock_gate + OFFSET_SW_CG_0_SET, 1 << I2C2_CG);
			break;
		case 3:
			I2C_WRITE(clock_gate + OFFSET_SW_CG_0_SET, 1 << I2C3_CG);
			break;
		case 4:
			I2C_WRITE(clock_gate + OFFSET_SW_CG_2_SET, 1 << I2C4_CG);
			break;
		case 5:
			I2C_WRITE(clock_gate + OFFSET_SW_CG_2_SET, 1 << I2C5_CG);
			break;
		default:
			I2CERR("invalid para: i2c->id=%d\n", i2c->id);
	}
#endif
	return 0;
}

static inline U32 ap_dma_clock_enable(void)
{
#ifdef TEE_I2C_CLOCK
	I2C_WRITE(clock_gate + OFFSET_SW_DMA_CLR, 1 << AP_DMA_CG);
#endif
	return 0;
}

static inline U32 ap_dma_clock_disable(void)
{
#ifdef TEE_I2C_CLOCK
	I2C_WRITE(clock_gate + OFFSET_SW_DMA_SET, 1 << AP_DMA_CG);
#endif
	return 0;
}

static inline void i2c_secure_enable(mt_i2c * i2c)
{
	/* Important Note: ALWAYS USE THE DEVICE APC MODULE INDEX IN "tlDapcIndex.h" HEADER */

	switch(i2c->id) {
		case I2C0:
			set_module_apc(DAPC_INDEX_I2C0, DOMAIN_AP, SECURE_TRANSACTION);
			break;
		case I2C1:
			set_module_apc(DAPC_INDEX_I2C1_IMM, DOMAIN_AP, SECURE_TRANSACTION);
			break;
		case I2C2:
			set_module_apc(DAPC_INDEX_I2C2_IMM, DOMAIN_AP, SECURE_TRANSACTION);
			break;
		case I2C3: /*off chip, on chip, remap*/
			set_module_apc(DAPC_INDEX_I2C4, DOMAIN_AP, SECURE_TRANSACTION);
			break;
		case I2C4: /*off chip, on chip, remap*/
			set_module_apc(DAPC_INDEX_I2C5_IMM, DOMAIN_AP, SECURE_TRANSACTION);
			break;
		case I2C5: /*off chip, on chip, remap*/
			set_module_apc(DAPC_INDEX_I2C3, DOMAIN_AP, SECURE_TRANSACTION);
			break;
		default:
			I2CERR("invalid para: i2c->id=%d\n", i2c->id);
	}
}

static inline void i2c_secure_disable(mt_i2c * i2c)
{
	/* Important Note: ALWAYS USE THE DEVICE APC MODULE INDEX IN "tlDapcIndex.h" HEADER */

	switch(i2c->id) {
		case I2C0:
			set_module_apc(DAPC_INDEX_I2C0, DOMAIN_AP, NON_SECURE_TRANSACTION);
			break;
		case I2C1:
			set_module_apc(DAPC_INDEX_I2C1_IMM, DOMAIN_AP, NON_SECURE_TRANSACTION);
			break;
		case I2C2:
			set_module_apc(DAPC_INDEX_I2C2_IMM, DOMAIN_AP, NON_SECURE_TRANSACTION);
			break;
		case I2C3: /*off chip, on chip, remap*/
			set_module_apc(DAPC_INDEX_I2C4, DOMAIN_AP, NON_SECURE_TRANSACTION);
			break;
		case I2C4: /*off chip, on chip, remap*/
			set_module_apc(DAPC_INDEX_I2C5_IMM, DOMAIN_AP, NON_SECURE_TRANSACTION);
			break;
		case I2C5: /*off chip, on chip, remap*/
			set_module_apc(DAPC_INDEX_I2C3, DOMAIN_AP, NON_SECURE_TRANSACTION);
			break;
		default:
			I2CERR("invalid para: i2c->id=%d\n", i2c->id);
	}
}

static inline U32 ap_dma_secure_enable(mt_i2c * i2c)
{
	I2C_WRITE(DMA_BASE_VA + OFFSET_AP_DMA_GLOBAL_GSEC_EN,
			(I2C_READ(DMA_BASE_VA + OFFSET_AP_DMA_GLOBAL_GSEC_EN) | 0x1));

	switch(i2c->id) {
		case I2C0:
			I2C_WRITE(DMA_BASE_VA + OFFSET_AP_DMA_I2C0_SEC_EN,
					(I2C_READ(DMA_BASE_VA + OFFSET_AP_DMA_I2C0_SEC_EN) | 0x1));
			break;
		case I2C1:
			I2C_WRITE(DMA_BASE_VA + OFFSET_AP_DMA_I2C1_SEC_EN,
					(I2C_READ(DMA_BASE_VA + OFFSET_AP_DMA_I2C1_SEC_EN) | 0x1));
			break;
		case I2C2:
			I2C_WRITE(DMA_BASE_VA + OFFSET_AP_DMA_I2C2_SEC_EN,
					(I2C_READ(DMA_BASE_VA + OFFSET_AP_DMA_I2C2_SEC_EN) | 0x1));
			break;
		case I2C3:
			I2C_WRITE(DMA_BASE_VA + OFFSET_AP_DMA_I2C3_SEC_EN,
					(I2C_READ(DMA_BASE_VA + OFFSET_AP_DMA_I2C3_SEC_EN) | 0x1));
			break;
		case I2C4:
			I2C_WRITE(DMA_BASE_VA + OFFSET_AP_DMA_I2C4_SEC_EN,
					(I2C_READ(DMA_BASE_VA + OFFSET_AP_DMA_I2C4_SEC_EN) | 0x1));
			break;
		case I2C5:
			I2C_WRITE(DMA_BASE_VA + OFFSET_AP_DMA_I2C5_SEC_EN,
					(I2C_READ(DMA_BASE_VA + OFFSET_AP_DMA_I2C5_SEC_EN) | 0x1));
			break;
		default:
			I2CERR("invalid para: i2c->id=%d\n",i2c->id);
			I2C_WRITE(DMA_BASE_VA + OFFSET_AP_DMA_GLOBAL_SEC_EN,
					(I2C_READ(DMA_BASE_VA + OFFSET_AP_DMA_GLOBAL_SEC_EN) | 0x0));
	}
	return 0;
}

static inline U32 ap_dma_secure_disable(mt_i2c * i2c)
{
	switch(i2c->id) {
		case I2C0:
			I2C_WRITE(DMA_BASE_VA + OFFSET_AP_DMA_I2C0_SEC_EN,
					(I2C_READ(DMA_BASE_VA + OFFSET_AP_DMA_I2C0_SEC_EN) & ~0x1));
			break;
		case I2C1:
			I2C_WRITE(DMA_BASE_VA + OFFSET_AP_DMA_I2C1_SEC_EN,
					(I2C_READ(DMA_BASE_VA + OFFSET_AP_DMA_I2C1_SEC_EN) & ~0x1));
			break;
		case I2C2:
			I2C_WRITE(DMA_BASE_VA + OFFSET_AP_DMA_I2C2_SEC_EN,
					(I2C_READ(DMA_BASE_VA + OFFSET_AP_DMA_I2C2_SEC_EN) & ~0x1));
			break;
		case I2C3:
			I2C_WRITE(DMA_BASE_VA + OFFSET_AP_DMA_I2C3_SEC_EN,
					(I2C_READ(DMA_BASE_VA + OFFSET_AP_DMA_I2C3_SEC_EN) & ~0x1));
			break;
		case I2C4:
			I2C_WRITE(DMA_BASE_VA + OFFSET_AP_DMA_I2C4_SEC_EN,
					(I2C_READ(DMA_BASE_VA + OFFSET_AP_DMA_I2C4_SEC_EN) & ~0x1));
			break;
		case I2C5:
			I2C_WRITE(DMA_BASE_VA + OFFSET_AP_DMA_I2C5_SEC_EN,
					(I2C_READ(DMA_BASE_VA + OFFSET_AP_DMA_I2C5_SEC_EN) & ~0x1));
			break;
		default:
			I2CERR("invalid para: i2c->id=%d\n", i2c->id);
	}

	I2C_WRITE(DMA_BASE_VA + OFFSET_AP_DMA_GLOBAL_GSEC_EN,
			(I2C_READ(DMA_BASE_VA + OFFSET_AP_DMA_GLOBAL_GSEC_EN) & ~0x1));
	return 0;
}

void _i2c_dump_info(mt_i2c *i2c)
{
	I2CLOG("+++++++++++++++++++++++++++++++++++++ _i2c_dump_info +++++++++++++++++++++++++++++++++++++\n");
	I2CLOG("I2C structure:\n"
			"Id=%d,Mode=%x,St_rs=%x,Dma_en=%x,Op=%x,Poll_en=%x,Irq_stat=%x\n"
			"Trans_len=%x,Trans_num=%x,Trans_auxlen=%x,Data_size=%x,speed=%d\n",
			i2c->id,i2c->mode,i2c->st_rs,i2c->dma_en,i2c->op,i2c->poll_en,i2c->irq_stat,
			i2c->trans_data.trans_len,i2c->trans_data.trans_num,i2c->trans_data.trans_auxlen,i2c->trans_data.data_size,i2c->speed);

	I2CLOG("base address 0x%x\n",i2c->base);
	I2CLOG("I2C register:\n"
			"SLAVE_ADDR=%x,INTR_MASK=%x,INTR_STAT=%x,CONTROL=%x,TRANSFER_LEN=%x\n"
			"TRANSAC_LEN=%x,DELAY_LEN=%x,TIMING=%x,START=%x,FIFO_STAT=%x\n"
			"IO_CONFIG=%x,HS=%x,DEBUGSTAT=%x,EXT_CONF=%x,TRANSFER_LEN_AUX=%x\n",
			(i2c_readl(i2c, OFFSET_SLAVE_ADDR)),
			(i2c_readl(i2c, OFFSET_INTR_MASK)),
			(i2c_readl(i2c, OFFSET_INTR_STAT)),
			(i2c_readl(i2c, OFFSET_CONTROL)),
			(i2c_readl(i2c, OFFSET_TRANSFER_LEN)),
			(i2c_readl(i2c, OFFSET_TRANSAC_LEN)),
			(i2c_readl(i2c, OFFSET_DELAY_LEN)),
			(i2c_readl(i2c, OFFSET_TIMING)),
			(i2c_readl(i2c, OFFSET_START)),
			(i2c_readl(i2c, OFFSET_FIFO_STAT)),
			(i2c_readl(i2c, OFFSET_IO_CONFIG)),
			(i2c_readl(i2c, OFFSET_HS)),
			(i2c_readl(i2c, OFFSET_DEBUGSTAT)),
			(i2c_readl(i2c, OFFSET_EXT_CONF)),
			(i2c_readl(i2c, OFFSET_TRANSFER_LEN_AUX)));

	I2CLOG("before enable DMA register(0x%x):\n"
			"INT_FLAG=%x,INT_EN=%x,EN=%x,RST=%x,\n"
			"STOP=%x,FLUSH=%x,CON=%x,TX_MEM_ADDR=%x, RX_MEM_ADDR=%x\n"
			"TX_LEN=%x,RX_LEN=%x,INT_BUF_SIZE=%x,DEBUG_STATUS=%x\n"
			"TX_MEM_ADDR2=%x, RX_MEM_ADDR2=%x\n",
			g_dma_regs[i2c->id].base,
			g_dma_regs[i2c->id].int_flag,
			g_dma_regs[i2c->id].int_en,
			g_dma_regs[i2c->id].en,
			g_dma_regs[i2c->id].rst,
			g_dma_regs[i2c->id].stop,
			g_dma_regs[i2c->id].flush,
			g_dma_regs[i2c->id].con,
			g_dma_regs[i2c->id].tx_mem_addr,
			g_dma_regs[i2c->id].rx_mem_addr,
			g_dma_regs[i2c->id].tx_len,
			g_dma_regs[i2c->id].rx_len,
			g_dma_regs[i2c->id].int_buf_size, g_dma_regs[i2c->id].debug_sta,
			g_dma_regs[i2c->id].tx_mem_addr2,
			g_dma_regs[i2c->id].rx_mem_addr2);
	I2CLOG("DMA register(0x%p):\n"
			"INT_FLAG=%x,INT_EN=%x,EN=%x,RST=%x,\n"
			"STOP=%x,FLUSH=%x,CON=%x,TX_MEM_ADDR=%x, RX_MEM_ADDR=%x\n"
			"TX_LEN=%x,RX_LEN=%x,INT_BUF_SIZE=%x,DEBUG_STATUS=%x\n"
			"TX_MEM_ADDR2=%x, RX_MEM_ADDR2=%x\n",
			i2c->pdmabase,
			(I2C_READ(i2c->pdmabase + OFFSET_INT_FLAG)),
			(I2C_READ(i2c->pdmabase + OFFSET_INT_EN)),
			(I2C_READ(i2c->pdmabase + OFFSET_EN)),
			(I2C_READ(i2c->pdmabase + OFFSET_RST)),
			(I2C_READ(i2c->pdmabase + OFFSET_STOP)),
			(I2C_READ(i2c->pdmabase + OFFSET_FLUSH)),
			(I2C_READ(i2c->pdmabase + OFFSET_CON)),
			(I2C_READ(i2c->pdmabase + OFFSET_TX_MEM_ADDR)),
			(I2C_READ(i2c->pdmabase + OFFSET_RX_MEM_ADDR)),
			(I2C_READ(i2c->pdmabase + OFFSET_TX_LEN)),
			(I2C_READ(i2c->pdmabase + OFFSET_RX_LEN)),
			(I2C_READ(i2c->pdmabase + OFFSET_INT_BUF_SIZE)),
			(I2C_READ(i2c->pdmabase + OFFSET_DEBUG_STA)),
			(I2C_READ(i2c->pdmabase + OFFSET_TX_MEM_ADDR2)),
			(I2C_READ(i2c->pdmabase + OFFSET_RX_MEM_ADDR2)));
	I2CLOG("------------------------------------- _i2c_dump_info -------------------------------------\n");

	return;
}

S32 _i2c_check_para(mt_i2c *i2c)
{
	S32 ret = 0;

	if (i2c->addr == 0) {
		I2CERR(" addr is invalid.\n");
		ret = -EINVAL_I2C;
		goto err;
	}

	if (i2c->msg_buf == NULL) {
		I2CERR(" data buffer is NULL.\n");
		ret = -EINVAL_I2C;
		goto err;
	}
err:
	return ret;

}

/*
 * off-chip <-> on-chip
 * i2c0     <-> i2c0
 * i2c1     <-> i2c1
 * i2c2     <-> i2c2
 * i2c-appm <-> i2c3 (APPM)
 * i2c3     <-> i2c4 (remap)
 * i2c4     <-> i2c5 (remap)
 * i2c6     <-> i2c6
 */
void _config_mt_i2c(mt_i2c *i2c)
{
	switch(i2c->id) {
		case I2C0:
			i2c->base = (unsigned long)I2C0_BASE;
			i2c->pdmabase = (unsigned long)(DMA_BASE_VA + 0x100);
			break;
		case I2C1:
			i2c->base = (unsigned long)I2C1_BASE;
			i2c->pdmabase = (unsigned long)(DMA_BASE_VA + 0x380);
			break;
		case I2C2:
			i2c->base = (unsigned long)I2C2_BASE;
			i2c->pdmabase = (unsigned long)(DMA_BASE_VA + 0x400);
			break;
		case I2C3: /*remap*/
			i2c->base = (unsigned long)I2C4_BASE;
			i2c->pdmabase = (unsigned long)(DMA_BASE_VA + 0x300);
			break;
		case I2C4: /*remap*/
			i2c->base = (unsigned long)I2C5_BASE;
			i2c->pdmabase = (unsigned long)(DMA_BASE_VA + 0x500);
			break;
		case I2C5: /*APPM*/
			i2c->base = (unsigned long)I2C3_BASE;
			i2c->pdmabase = (unsigned long)(DMA_BASE_VA + 0x280);
			break;
		default:
			I2CERR("invalid para: i2c id = %d\n",i2c->id);
			break;
	}

	if (i2c->st_rs == I2C_TRANS_REPEATED_START)
		i2c->st_rs = I2C_TRANS_REPEATED_START;
	else
		i2c->st_rs = I2C_TRANS_STOP;

	if (i2c->dma_en != TRUE)
		i2c->dma_en = FALSE;

	i2c->poll_en = TRUE;

	if (i2c->filter_msg != TRUE)
		i2c->filter_msg = FALSE;

	///*Set device speed,set it before set_control register
	if (0 == i2c->speed) {
		i2c->mode  = ST_MODE;
		i2c->speed = MAX_ST_MODE_SPEED;
	} else {
		if (i2c->mode  == HS_MODE)
			i2c->mode  = HS_MODE;
		else
			i2c->mode  = FS_MODE;
	}

	/*Set ioconfig*/
	if (i2c->pushpull != TRUE)
		i2c->pushpull=FALSE;
}


S32 _i2c_get_transfer_len(mt_i2c *i2c)
{
	S32 ret = I2C_OK;
	U16 trans_num = 0;
	U16 data_size = 0;
	U16 trans_len = 0;
	U16 trans_auxlen = 0;

	/* Get Transfer len and transaux len */
	if (FALSE == i2c->dma_en) { /*non-DMA mode*/
		if (I2C_MASTER_WRRD != i2c->op)	{
			trans_len = (i2c->msg_len) & 0xFFFF;
			trans_num = (i2c->msg_len >> 16) & 0xFF;
			if (0 == trans_num)
				trans_num = 1;
			trans_auxlen = 0;
			data_size = trans_len * trans_num;

			if (!trans_len || !trans_num || trans_len*trans_num > 8) {
				I2CERR(" non-WRRD transfer length is not right. trans_len=%x, tans_num=%x, trans_auxlen=%x\n", trans_len, trans_num, trans_auxlen);
				I2C_BUG_ON(!trans_len || !trans_num || trans_len*trans_num > 8);
				ret = -EINVAL_I2C;
			}
		} else {
			trans_len = (i2c->msg_len) & 0xFFFF;
			trans_auxlen = (i2c->msg_len >> 16) & 0xFFFF;
			trans_num = 2;
			data_size = trans_len;
			if (!trans_len || !trans_auxlen || trans_len > 8 || trans_auxlen > 8) {
				I2CERR(" WRRD transfer length is not right. trans_len=%x, tans_num=%x, trans_auxlen=%x\n", trans_len, trans_num, trans_auxlen);
				I2C_BUG_ON(!trans_len || !trans_auxlen || trans_len > 8 || trans_auxlen > 8);
				ret = -EINVAL_I2C;
			}
		}
	} else { /* DMA mode */
		if (I2C_MASTER_WRRD != i2c->op) {
			trans_len = (i2c->msg_len) & 0xFFFF;
			trans_num = (i2c->msg_len >> 16) & 0xFF;
			if(0 == trans_num)
				trans_num = 1;
			trans_auxlen = 0;
			data_size = trans_len * trans_num;

			if (!trans_len || !trans_num || trans_len > MAX_DMA_TRANS_SIZE || trans_num > MAX_DMA_TRANS_NUM)	{
				I2CERR(" DMA non-WRRD transfer length is not right. trans_len=%x, tans_num=%x, trans_auxlen=%x\n", trans_len, trans_num, trans_auxlen);
				I2C_BUG_ON(!trans_len || !trans_num || trans_len > MAX_DMA_TRANS_SIZE || trans_num > MAX_DMA_TRANS_NUM);
				ret = -EINVAL_I2C;
			}
		} else {
			trans_len = (i2c->msg_len) & 0xFFFF;
			trans_auxlen = (i2c->msg_len >> 16) & 0xFFFF;
			trans_num = 2;
			data_size = trans_len;
			if (!trans_len || !trans_auxlen || trans_len > MAX_DMA_TRANS_SIZE || trans_auxlen > MAX_DMA_TRANS_NUM) {
				I2CERR(" DMA WRRD transfer length is not right. trans_len=%x, tans_num=%x, trans_auxlen=%x\n", trans_len, trans_num, trans_auxlen);
				I2C_BUG_ON(!trans_len || !trans_auxlen || trans_len > MAX_DMA_TRANS_SIZE || trans_auxlen > MAX_DMA_TRANS_NUM);
				ret = -EINVAL_I2C;
			}
		}
	}

	i2c->trans_data.trans_num = trans_num;
	i2c->trans_data.trans_len = trans_len;
	i2c->trans_data.data_size = data_size;
	i2c->trans_data.trans_auxlen = trans_auxlen;

	return ret;
}

S32 i2c_set_speed(mt_i2c *i2c)
{
	S32 ret = 0;
	static S32 mode = 0;
	static U32 khz = 0;
	U16 step_cnt_div = 0;
	U16 sample_cnt_div = 0;
	U32 tmp, sclk, hclk = i2c->clk;
	U16 max_step_cnt_div = 0;
	U32 diff, min_diff = i2c->clk;
	U16 sample_div = MAX_SAMPLE_CNT_DIV;
	U16 step_div = 0;

	mode = i2c->mode;
	khz = i2c->speed;

	max_step_cnt_div = (mode == HS_MODE) ? MAX_HS_STEP_CNT_DIV : MAX_STEP_CNT_DIV;
	step_div = max_step_cnt_div;

	if ((mode == FS_MODE && khz > MAX_FS_MODE_SPEED) || (mode == HS_MODE && khz > MAX_HS_MODE_SPEED)) {
		I2CERR(" the speed is too fast for this mode.\n");
		I2C_BUG_ON((mode == FS_MODE && khz > MAX_FS_MODE_SPEED) || (mode == HS_MODE && khz > MAX_HS_MODE_SPEED));
		ret = -EINVAL_I2C;
		goto end;
	}

	/* Find the best combination */
	for (sample_cnt_div = 1; sample_cnt_div <= MAX_SAMPLE_CNT_DIV; sample_cnt_div++) {
		for (step_cnt_div = 1; step_cnt_div <= max_step_cnt_div; step_cnt_div++) {
			sclk = (hclk >> 1) / (sample_cnt_div * step_cnt_div);
			if (sclk > khz)
				continue;
			diff = khz - sclk;
			if (diff < min_diff) {
				min_diff = diff;
				sample_div = sample_cnt_div;
				step_div   = step_cnt_div;
			}
		}
	}
	sample_cnt_div = sample_div;
	step_cnt_div   = step_div;
	sclk = hclk / (2 * sample_cnt_div * step_cnt_div);

	if (sclk > khz) {
		I2CERR("%s mode: unsupported speed (%dkhz)\n",(mode == HS_MODE) ? "HS" : "ST/FT", khz);
		I2C_BUG_ON(sclk > khz);
		ret = -ENOTSUPP_I2C;
		goto end;
	}

	step_cnt_div--;
	sample_cnt_div--;

	if (mode == HS_MODE) {
#ifdef NO_I2C_LTIMING
		/* Set the hignspeed timing control register */
		tmp = i2c_readl(i2c, OFFSET_TIMING) & ~((0x7 << 8) | (0x3f << 0));
		tmp = (0 & 0x7) << 8 | (16 & 0x3f) << 0 | tmp;
		i2c->timing_reg = tmp;
		I2C_TIMING_REG_BACKUP[i2c->id] = tmp;

		/*Set the hign speed mode register*/
		tmp = i2c_readl(i2c, OFFSET_HS) & ~((0x7 << 12) | (0x7 << 8));
		tmp = (sample_cnt_div & 0x7) << 12 | (step_cnt_div & 0x7) << 8 | tmp;
		/* Enable the hign speed transaction */
		tmp |= 0x0001;
		i2c->high_speed_reg = tmp;
#else
		/* Set the hign speed mode register */
		i2c->timing_reg = 0x1303;
		i2c->ltiming_reg= (0x3 << 6) | (0x3 << 0) |
			(sample_cnt_div & 0x7) << 12 |
			(step_cnt_div & 0x7) << 9;
		i2c->high_speed_reg = 0x3 |
			(sample_cnt_div & 0x7) << 12 |
			(step_cnt_div & 0x7) << 8;
#endif
	} else {
#ifdef NO_I2C_LTIMING
		/*Set non-highspeed timing*/
		tmp = i2c_readl(i2c, OFFSET_TIMING) & ~((0x7 << 8) | (0x3f << 0));
		tmp = (sample_cnt_div & 0x7) << 8 | (step_cnt_div & 0x3f) << 0 | tmp;
		i2c->timing_reg=tmp;
		I2C_TIMING_REG_BACKUP[i2c->id] = tmp;
		/* Disable the high speed transaction */
		tmp = i2c_readl(i2c, OFFSET_HS) & ~(0x0001);
		i2c->high_speed_reg = tmp;
#else
	    i2c->timing_reg =
	        (sample_cnt_div & 0x7) << 8 |
	        (step_cnt_div & 0x3f) << 0;
	     i2c->ltiming_reg =
	         (sample_cnt_div & 0x7) << 6 |
	         (step_cnt_div & 0x3f) << 0;
	    /* Disable the high speed transaction */
	    i2c->high_speed_reg = 0x0;
#endif
	}

end:
	return ret;
}

void _i2c_write_reg(mt_i2c *i2c)
{
	U8 *ptr = i2c->msg_buf;
	U32 data_size = i2c->trans_data.data_size;
	U32 addr_reg = 0;

	i2c_writel(i2c, OFFSET_CONTROL, i2c->control_reg);

	/*set start condition */
	if (i2c->speed <= 100)
		i2c_writel(i2c, OFFSET_EXT_CONF, 0x8001);

	//set timing reg
	i2c_writel(i2c, OFFSET_TIMING, i2c->timing_reg);
#ifndef NO_I2C_LTIMING
	i2c_writel(i2c, OFFSET_LTIMING, i2c->ltiming_reg);
#endif
	i2c_writel(i2c, OFFSET_HS, i2c->high_speed_reg);

	if (0 == i2c->delay_len)
		i2c->delay_len = 2;

	if (~i2c->control_reg & I2C_CONTROL_RS)
		i2c_writel(i2c, OFFSET_DELAY_LEN, i2c->delay_len);

	/* Set ioconfig */
	if (i2c->pushpull)
		i2c_writel(i2c, OFFSET_IO_CONFIG, 0x0);
	else
		i2c_writel(i2c, OFFSET_IO_CONFIG, 0x3);

	/* Set slave address */
	addr_reg = i2c->read_flag ? ((i2c->addr << 1) | 0x1) : ((i2c->addr << 1) & ~0x1);
	i2c_writel(i2c, OFFSET_SLAVE_ADDR, addr_reg);
	/* Clear interrupt status */
	i2c_writel(i2c, OFFSET_INTR_STAT, (I2C_HS_NACKERR | I2C_ACKERR | I2C_TRANSAC_COMP));
	/* Clear fifo address */
	i2c_writel(i2c, OFFSET_FIFO_ADDR_CLR, 0x1);
	/* Setup the interrupt mask flag */
	if (i2c->poll_en)
		i2c_writel(i2c, OFFSET_INTR_MASK, i2c_readl(i2c, OFFSET_INTR_MASK)
			& ~(I2C_HS_NACKERR | I2C_ACKERR | I2C_TRANSAC_COMP)); /* Disable interrupt */
	else
		i2c_writel(i2c, OFFSET_INTR_MASK, i2c_readl(i2c, OFFSET_INTR_MASK)
			| (I2C_HS_NACKERR | I2C_ACKERR | I2C_TRANSAC_COMP)); /* Enable interrupt */
	/* Set transfer len */
	i2c_writel(i2c, OFFSET_TRANSFER_LEN, i2c->trans_data.trans_len & 0xFFFF);
	i2c_writel(i2c, OFFSET_TRANSFER_LEN_AUX, i2c->trans_data.trans_auxlen & 0xFFFF);
	/* Set transaction len */
	i2c_writel(i2c, OFFSET_TRANSAC_LEN, i2c->trans_data.trans_num & 0xFF);

	/* Prepare buffer data to start transfer */
	if (i2c->dma_en) {
		if (I2C_MASTER_RD == i2c->op) {
			I2C_WRITE(i2c->pdmabase + OFFSET_INT_FLAG, 0x0000);
			I2C_WRITE(i2c->pdmabase + OFFSET_CON, 0x0001);
			I2C_WRITE(i2c->pdmabase + OFFSET_RX_MEM_ADDR, lower_32_bits(i2c->msg_buf));
			I2C_WRITE(i2c->pdmabase + OFFSET_RX_MEM_ADDR2, upper_32_bits(i2c->msg_buf));
			I2C_WRITE(i2c->pdmabase + OFFSET_RX_LEN, (U32)i2c->trans_data.data_size);
		} else if (I2C_MASTER_WR == i2c->op) {
			I2C_WRITE(i2c->pdmabase + OFFSET_INT_FLAG, 0x0000);
			I2C_WRITE(i2c->pdmabase + OFFSET_CON, 0x0000);
			I2C_WRITE(i2c->pdmabase + OFFSET_TX_MEM_ADDR, lower_32_bits(i2c->msg_buf));
			I2C_WRITE(i2c->pdmabase + OFFSET_TX_MEM_ADDR2, upper_32_bits(i2c->msg_buf));
			I2C_WRITE(i2c->pdmabase + OFFSET_TX_LEN, (U32)i2c->trans_data.data_size);
		} else {
			I2C_WRITE(i2c->pdmabase + OFFSET_INT_FLAG, 0x0000);
			I2C_WRITE(i2c->pdmabase + OFFSET_CON, 0x0000);
			I2C_WRITE(i2c->pdmabase + OFFSET_RX_MEM_ADDR, lower_32_bits(i2c->msg_buf));
			I2C_WRITE(i2c->pdmabase + OFFSET_RX_MEM_ADDR2, upper_32_bits(i2c->msg_buf));
			I2C_WRITE(i2c->pdmabase + OFFSET_TX_MEM_ADDR, lower_32_bits(i2c->msg_buf));
			I2C_WRITE(i2c->pdmabase + OFFSET_TX_MEM_ADDR2, upper_32_bits(i2c->msg_buf));
			I2C_WRITE(i2c->pdmabase + OFFSET_TX_LEN, (U32)i2c->trans_data.trans_len);
			I2C_WRITE(i2c->pdmabase + OFFSET_RX_LEN, (U32)i2c->trans_data.trans_auxlen);
		}
		record_i2c_dma_info(i2c);
		I2C_MB();
		I2C_WRITE(i2c->pdmabase + OFFSET_EN, 0x0001);
	} else {
		/* Set fifo mode data */
		if (I2C_MASTER_RD == i2c->op) {
			/* do not need set fifo data */
		} else { /* both write && write_read mode */
			while (data_size--) {
				i2c_writel(i2c, OFFSET_DATA_PORT, *ptr);
				ptr++;
			}
		}
	}
	/* Set trans_data */
	i2c->trans_data.data_size = data_size;
}

static S32 _i2c_deal_result(mt_i2c *i2c)
{
	long tmo = 1;
	U16 data_size = 0;
	U8 *ptr = i2c->msg_buf;
	bool TRANSFER_ERROR = FALSE;
	S32 ret = i2c->msg_len;
	long tmo_poll = 0xffff;

#ifdef SECURE_I2C_TEST
	I2CERR("I2C[%d]------------------------------------>_i2c_deal_result\n", i2c->id);
	_i2c_dump_info(i2c);
#endif

	if (i2c->poll_en) { /*master read && poll mode*/
		for (;;) { /* check the interrupt status register */
			i2c->irq_stat = i2c_readl(i2c, OFFSET_INTR_STAT);
			if (i2c->irq_stat & (I2C_HS_NACKERR | I2C_ACKERR)) {
				//transfer error
				/* Clear interrupt status,write 1 clear */
				TRANSFER_ERROR = TRUE;
				tmo = 1;
				break;
			} else if (i2c->irq_stat & I2C_TRANSAC_COMP) {
				//transfer complete
				tmo = 1;
				break;
			}
			tmo_poll--;
			if(tmo_poll == 0) {
				tmo = 0;
				break;
			}
		}
	}

	/* Check the transfer status */
	if (!(tmo == 0) && TRANSFER_ERROR == FALSE)	{
		/* Transfer success ,we need to get data from fifo */
		if ((!i2c->dma_en) && (i2c->op == I2C_MASTER_RD || i2c->op == I2C_MASTER_WRRD))
		{ /* only read mode or write_read mode and fifo mode need to get data */
			data_size = (i2c_readl(i2c, OFFSET_FIFO_STAT) >> 4) & 0xF;
			while (data_size--)	{
				*ptr = i2c_readl(i2c, OFFSET_DATA_PORT);
#ifdef I2C_EARLY_PORTING
				I2CLOG("addr %x read byte = 0x%x\n", i2c->addr, *ptr);
#endif
				ptr++;
			}
		}
	} else {
		/*Timeout or ACKERR*/
		if (tmo == 0) {
			I2CERR("i2c%d, addr: %x, transfer timeout\n", i2c->id, i2c->addr);
			ret = -ETIMEDOUT_I2C;
		} else {
			I2CERR("i2c%d,addr: %x, transfer error\n", i2c->id, i2c->addr);
			ret = -EREMOTEIO_I2C;
		}
		if (i2c->irq_stat & I2C_HS_NACKERR)
			I2CERR("I2C_HS_NACKERR\n");
		if (i2c->irq_stat & I2C_ACKERR)
			I2CERR("I2C_ACKERR\n");

		_i2c_dump_info(i2c);

		i2c_writel(i2c, OFFSET_SOFTRESET, 0x1);
		i2c_writel(i2c, OFFSET_SLAVE_ADDR, 0x0);
		i2c_writel(i2c, OFFSET_INTR_STAT, (I2C_HS_NACKERR|I2C_ACKERR|I2C_TRANSAC_COMP));
		i2c_writel(i2c, OFFSET_FIFO_ADDR_CLR, 0x1);
	}
	return ret;
}

S32 tui_lock_i2c(int id, int lock)
{
	mt_i2c i2c = {.poll_en = TRUE};
	i2c.id = id;

	I2CLOG("%s: tui_lock_i2c(%d, %d)\n", __func__, id, lock);

	if (!lock) {
		i2c_secure_disable(&i2c);
	} else {
		i2c_secure_enable(&i2c);
	}
}

S32 _i2c_transfer_interface(mt_i2c *i2c)
{
	S32 return_value=0;
	S32 ret=0;
	U8 *ptr = i2c->msg_buf;
#ifdef SECURE_I2C_TEST
	ap_dma_clock_enable();
	i2c_clock_enable(i2c);
#endif
#ifdef SECURE_LOCK
	i2c_secure_enable(i2c);
	ap_dma_secure_enable(i2c);
#endif

	if (i2c->dma_en) {
		if (i2c->pdmabase == 0) {
			I2CERR(" I2C%d doesnot support DMA mode!\n",i2c->id);
			I2C_BUG_ON(i2c->pdmabase == NULL);
			ret = -EINVAL_I2C;
			goto err;
		}
	}

	i2c->irq_stat = 0;

	return_value = _i2c_get_transfer_len(i2c);
	if (return_value < 0) {
		I2CERR("_i2c_get_transfer_len fail(%d)\n", return_value);
		ret = -EINVAL_I2C;
		goto err;
	}

	/* get clock */
	i2c->clk = I2C_CLK_RATE;

	return_value = i2c_set_speed(i2c);
	if (return_value < 0) {
		I2CERR("i2c_set_speed fail(%d)\n", return_value);
		ret = -EINVAL_I2C;
		goto err;
	}

	/* Set Control Register */
	i2c->control_reg = I2C_CONTROL_ACKERR_DET_EN | I2C_CONTROL_CLK_EXT_EN | I2C_CONTROL_DMA_ACK_EN;
	if (i2c->dma_en)
		i2c->control_reg |= I2C_CONTROL_DMA_EN | I2C_CONTROL_DMA_ACK_EN | I2C_CONTROL_DMA_ASYNC_MODE;

	if (I2C_MASTER_WRRD == i2c->op)
		i2c->control_reg |= I2C_CONTROL_DIR_CHANGE;

	if (HS_MODE == i2c->mode || (i2c->trans_data.trans_num > 1
		&& I2C_TRANS_REPEATED_START == i2c->st_rs)) {
		i2c->control_reg |= I2C_CONTROL_RS;
	}

	_i2c_write_reg(i2c);

	/* All register must be prepared before setting the start bit [SMP] */
	I2C_MB();

	/* Start the transfer */
	i2c_writel(i2c, OFFSET_START, 0x0001);
	ret = _i2c_deal_result(i2c);
#ifdef SECURE_LOCK
	ap_dma_secure_disable(i2c);
	i2c_secure_disable(i2c);
#endif
#ifdef SECURE_I2C_TEST
	ap_dma_clock_disable();
	i2c_clock_disable(i2c);
#endif
err:

	return ret;
}

S32 i2c_write(mt_i2c *i2c, U8 *buffer, U32 len)
{
	int32_t ret = I2C_OK;

	//write
	i2c->read_flag = !I2C_M_RD;
	i2c->op = I2C_MASTER_WR;
	i2c->msg_buf = buffer;
	i2c->msg_len = len;

	ret = _i2c_check_para(i2c);
	if (ret < 0) {
		I2CERR(" _i2c_check_para fail\n");
		goto err;
	}

	_config_mt_i2c(i2c);
	ret = _i2c_transfer_interface(i2c);
	if ((int)i2c->msg_len != ret) {
		I2CERR("Write %d bytes fails,ret=%d.\n",i2c->msg_len,ret);
		ret = -1;
		return ret;
	} else {
		ret = I2C_OK;
	}
err:
	return ret;
}

S32 i2c_read(mt_i2c *i2c,U8 *buffer, U32 len)
{
	S32 ret = I2C_OK;

	//read
	i2c->read_flag |= I2C_M_RD;
	i2c->op = I2C_MASTER_RD;
	i2c->msg_buf = buffer;
	i2c->msg_len = len;

	ret = _i2c_check_para(i2c);
	if (ret < 0) {
		I2CERR(" _i2c_check_para fail\n");
		goto err;
	}

	_config_mt_i2c(i2c);
	//get the addr
	ret = _i2c_transfer_interface(i2c);
	if ((int)i2c->msg_len != ret) {
		I2CERR("read %d bytes fails,ret=%d.\n",i2c->msg_len,ret);
		ret = -1;
		return ret;
	} else {
		ret = I2C_OK;
	}
err:
	return ret;
}

S32 i2c_write_read(mt_i2c *i2c,U8 *buffer, U32 write_len, U32 read_len)
{
	S32 ret = I2C_OK;

	//write and read
	i2c->op = I2C_MASTER_WRRD;
	i2c->read_flag = !I2C_M_RD;
	i2c->msg_buf = buffer;
	i2c->msg_len = ((read_len & 0xFFFF) << 16) | (write_len & 0xFFFF);

	ret = _i2c_check_para(i2c);
	if (ret < 0) {
		I2CERR(" _i2c_check_para fail\n");
		goto err;
	}

	_config_mt_i2c(i2c);
	ret = _i2c_transfer_interface(i2c);
	if ((int)i2c->msg_len != ret) {
		I2CERR("write_read 0x%x bytes fails,ret=%d.\n", i2c->msg_len, ret);
		ret = -1;
		return ret;
	} else {
		ret = I2C_OK;
	}
err:
	return ret;
}

#ifdef I2C_EARLY_PORTING
int mt_i2c_test_eeprom(int id)
{
	U32 ret = 0;
	U32 len = 0;
	U32 delay_count = 0xff;
	unsigned char write_byte[2], read_byte[2];
	struct mt_i2c_t i2c;

	I2CLOG("i2c %d test start++++++++++++++++++++\n", id);

	i2c.id = id;
	i2c.addr = 0x50;
	i2c.mode = ST_MODE;
	i2c.speed = 100;
	//==================================================
	I2CLOG("\ntest i2c write\n\n");
	write_byte[0] = 0x20;
	write_byte[1] = 0x55;
	len = 2;
	ret = i2c_write(&i2c, write_byte, 2);
	if (I2C_OK != ret) {
		I2CERR("Write 2 bytes fails(%x).\n", ret);
		ret = -1;
		return ret;
	} else {
		I2CLOG("Write 2 bytes pass,these bytes are %x, %x.\n", write_byte[0], write_byte[1]);
	}
	// mdelay(1000);
	for (delay_count = 0xff; delay_count > 0; delay_count--);
	//==================================================
	I2CLOG("\ntest i2c read\n\n");
	//1st:write addree 00,1byte(0x0A)
	write_byte[0] = 0x20;
	write_byte[1] = 0x0A;
	len=2;
	ret = i2c_write(&i2c, write_byte, 2);

	//mdelay(1000);
	for (delay_count = 0xff; delay_count > 0; delay_count--);

	read_byte[0] = 0x20;
	len = 1;
	ret = i2c_read(&i2c, read_byte, len);
	if ((I2C_OK != ret) && read_byte[0] == write_byte[1]) {
		I2CERR("read 2 bytes fails(%x).\n",ret);
		ret = -1;
		return ret;
	} else {
		I2CLOG("read 2 bytes pass,read_byte=%x\n", write_byte[1]);
	}

	//mdelay(1000);
	for(delay_count=0xff; delay_count>0;delay_count--);
	//==================================================
	I2CLOG("\ntest i2c write_read\n\n");
	write_byte[0] = 0x20;
	// write_byte[1] = 0x34;
	len = (1 & 0xFF) << 8 | (1 & 0xFF);
	ret = i2c_write_read(&i2c, write_byte, 1, 1);
	if(I2C_OK != ret){
		I2CERR("Write 1 byte fails(ret=%x).\n", ret);
		ret = -1;
		return ret;
	}else{
		I2CLOG("Read 1 byte pass ,this byte is %x.\n", write_byte[0]);
		ret = 0;
	}
	I2CLOG("i2c %d test done-------------------\n", id);
	return ret;
}
int mt_i2c_test_eeprom1(int id)
{
	U32 ret = 0;
	U32 len = 0;
	U32 delay_count = 0xff;
	U8 write_byte[2], read_byte[2];
	struct mt_i2c_t i2c;

	I2CLOG("i2c %d test start++++++++++++++++++++\n", id);

	i2c.id = id;
	i2c.addr = 0x50;
	i2c.mode = ST_MODE;
	i2c.speed = 100;
	//==================================================
	I2CLOG("\ntest i2c write\n\n");
	write_byte[0] = 0x20;
	write_byte[1] = 0x55;
	len=2;
	ret = i2c_write(&i2c, write_byte, len);
	if(I2C_OK != ret){
		I2CERR("Write 2 bytes fails(%x).\n", ret);
		ret = -1;
		return ret;
	}else{
		I2CLOG("Write 2 bytes pass,these bytes are %x, %x.\n", write_byte[0], write_byte[1]);
	}
	// mdelay(1000);
	for (delay_count = 0xff; delay_count > 0; delay_count--);
	//==================================================
	I2CLOG("\ntest i2c read\n\n");
	//1st:write addree 00,1byte(0x0A)
	write_byte[0] = 0x20;
	len = 1;
	ret = i2c_write(&i2c, write_byte, len);
	if(I2C_OK != ret){
		I2CERR("Write 1 bytes fails(%x).\n", ret);
		ret = -1;
		return ret;
	}else{
		I2CLOG("Write 1 bytes pass,these bytes are %x.\n", write_byte[0]);
	}

	//mdelay(1000);
	for(delay_count=0xff; delay_count>0;delay_count--);
	//2rd:read back 1byte(0x0A)
	read_byte[0] = 0x33;
	len = 1;
	ret = i2c_read(&i2c, read_byte, len);
	if((I2C_OK != ret) || read_byte[0]!=write_byte[1]){
		I2CERR("read 1 bytes fails(%x).\n",ret);
		I2CLOG("Debug: ----> read 1 bytes ,read_byte=%x\n", read_byte[0]);
		ret = -1;
		return ret;
	}else{
		I2CLOG("read 1 bytes pass,read_byte=%x\n", read_byte[0]);
	}

	//mdelay(1000);
	for (delay_count = 0xff; delay_count > 0; delay_count--);
	//==================================================
	I2CLOG("\ntest i2c write_read\n\n");
	read_byte[0] = 0x20;
	//write_byte[1] = 0x34;
	len = ((1 & 0xFFFF)<< 16) | ( 1 & 0xFFFF);
	ret = i2c_write_read(&i2c, read_byte, 1, 1);
	if (I2C_OK != ret || read_byte[0] != write_byte[1]){
		I2CERR("write_read 1 byte fails(ret=%x).\n",ret);
		I2CLOG("Debug: ----> write_read 1 bytes ,read_byte=%x\n", read_byte[0]);
		ret = -1;
		return ret;
	} else {
		I2CLOG("Write_Read 1 byte pass ,this byte is %x.\n", read_byte[0]);
		ret = 0;
	}
	I2CLOG("i2c %d test done-------------------\n", id);
	return ret;
}

int mt_i2c_test_eeprom2(int id)
{
	U32 ret = 0;
	U32 len=0;
	U32 delay_count=0xff;
	U8 *write_byte, *read_byte;
	U8 *write_byte_pa = 0xf0000000, *read_byte_pa = 0xf0010000;
	U8 *write_byte_test, *write_byte_test_va;
	struct mt_i2c_t i2c = {.poll_en = TRUE};
	uint32_t mapFlag = MAP_READABLE | MAP_WRITABLE | MAP_IO | MAP_UNCACHED;

	I2CLOG("i2c %d test start++++++++++++++++++++\n", id);

	drApiMapPhysicalBuffer((uint64_t)read_byte_pa, 0x100, mapFlag, (void **)&read_byte);
	drApiMapPhysicalBuffer((uint64_t)write_byte_pa, 0x100, mapFlag, (void **)&write_byte);

	i2c.id = id;
	i2c.addr = 0x50;
	i2c.mode = ST_MODE;
	i2c.speed = 100;
	//i2c.dma_en = FALSE;
	i2c.dma_en = TRUE;
	//==================================================
	I2CLOG("\ntest i2c write\n\n");
	write_byte[0] = 0x20;
	write_byte[1] = 0x55;
	len=2;
	if (TRUE == i2c.dma_en)
		ret = i2c_write(&i2c, write_byte_pa, len);
	else
		ret = i2c_write(&i2c, write_byte, len);
	if(I2C_OK != ret){
		I2CERR("Write 2 bytes fails(%x).\n",ret);
		ret = -1;
		return ret;
	}else{
		I2CLOG("Write 2 bytes pass,these bytes are %x, %x.\n", write_byte[0], write_byte[1]);
	}

	//drApiThreadSleep(10);
	//==================================================
	I2CLOG("\ntest i2c read\n\n");
	//1st:write addree 00,1byte(0x0A)
	write_byte[0] = 0x20;
	len = 1;
	if (TRUE == i2c.dma_en)
		ret = i2c_write(&i2c, write_byte_pa, len);
	else
		ret = i2c_write(&i2c, write_byte, len);
	if(I2C_OK != ret){
		I2CERR("Write 1 bytes fails(%x).\n",ret);
		ret = -1;
		return ret;
	}else{
		I2CLOG("Write 1 bytes pass,these bytes are %x.\n", write_byte[0] );
	}

	//drApiThreadSleep(10);
	//2rd:read back 1byte(0x0A)
	read_byte[0] = 0x33;
	len = 1;
	if (TRUE == i2c.dma_en)
		ret = i2c_read(&i2c, read_byte_pa, len);
	else
		ret = i2c_read(&i2c, read_byte, len);
	if((I2C_OK != ret) || read_byte[0]!=write_byte[1]){
		I2CERR("read 1 bytes fails(%x).\n",ret);
		I2CLOG("Debug: ----> read 1 bytes ,read_byte=%x\n", read_byte[0]);
		ret = -1;
		return ret;
	}else{
		I2CLOG("read 1 bytes pass,read_byte=%x\n", read_byte[0]);
	}

	//drApiThreadSleep(10);
	//==================================================
	I2CLOG("\ntest i2c write_read\n\n");
	read_byte[0] = 0x20;
	// write_byte[1] = 0x34;
	len=(1 & 0xFF)<<8 |(1 & 0xFF);
	drApiVirt2Phys(0, read_byte, &read_byte_pa);
	if (TRUE == i2c.dma_en)
		ret = i2c_write_read(&i2c, read_byte_pa, 1, 1);
	else
		ret = i2c_write_read(&i2c, read_byte, 1, 1);
	if(I2C_OK != ret || read_byte[0] != write_byte[1]){
		I2CERR("write_read 1 byte fails(ret=%x).\n",ret);
		I2CLOG("Debug: ----> write_read 1 bytes ,read_byte=%x\n", read_byte[0]);
		ret = -1;
		return ret;
	}else{
		I2CLOG("Write_Read 1 byte pass ,this byte is %x.\n", read_byte[0]);
		ret = 0;
	}
	I2CLOG("i2c %d test done-------------------\n", id);
	return ret;
}

void test_i2c(void)
{
	int ret;
	int i=0;
	uint32_t mapFlag = MAP_READABLE | MAP_WRITABLE | MAP_IO;
	I2CFUC();
	I2CMSG("fwq test_i2c++++++++++++++ 4");

#ifdef SECURE_LOCK
	start_devapc();
#endif

	drApiMapPhysicalBuffer((uint64_t)I2C0_BASE_PA, 0x1000, mapFlag, (void **)&i2c0_va);
	drApiMapPhysicalBuffer((uint64_t)I2C1_BASE_PA, 0x1000, mapFlag, (void **)&i2c1_va);
	drApiMapPhysicalBuffer((uint64_t)I2C2_BASE_PA, 0x1000, mapFlag, (void **)&i2c2_va);
	drApiMapPhysicalBuffer((uint64_t)I2C3_BASE_PA, 0x1000, mapFlag, (void **)&i2c3_va);
	drApiMapPhysicalBuffer((uint64_t)I2C4_BASE_PA, 0x1000, mapFlag, (void **)&i2c4_va);
	drApiMapPhysicalBuffer((uint64_t)I2C5_BASE_PA, 0x1000, mapFlag, (void **)&i2c5_va);
	drApiMapPhysicalBuffer((uint64_t)AP_DMA_BASE, 0x1000, mapFlag, (void **)&DMA_BASE_VA);
	drApiMapPhysicalBuffer((uint64_t)INFRACFG_AO_BASE, 0x1000, mapFlag, (void **)&clock_gate);

	I2CMSG("i2c0_map_regiser CFG, hasMapped [%x]\n", i2c0_va);
	I2CMSG("i2c1_map_regiser CFG, hasMapped [%x]\n", i2c1_va);
	I2CMSG("i2c2_map_regiser CFG, hasMapped [%x]\n", i2c2_va);
	I2CMSG("i2c3_map_regiser CFG, hasMapped [%x]\n", i2c3_va);
	I2CMSG("i2c4_map_regiser CFG, hasMapped [%x]\n", i2c4_va);
	I2CMSG("i2c5_map_regiser CFG, hasMapped [%x]\n", i2c5_va);
	I2CMSG("ap_dma_va_regiser CFG, hasMapped [%x]\n", DMA_BASE_VA);
	I2CMSG("clock_map_regiser CFG, hasMapped [%x]\n", clock_gate);
	I2CERR("fwq _clockSTA[%x]\n", (*(volatile uint32_t*)(clock_gate+0x90)));
	I2CERR("fwq _clockSTA2[%x]\n", (*(volatile uint32_t*)(clock_gate+0x94)));
	I2CERR("fwq _clockSTA3[%x]\n", (*(volatile uint32_t*)(clock_gate+0xb0)));

#if 1
	for (i = 0; i <= 0; i++) {
		ret = mt_i2c_test_eeprom2(i);
		if(0==ret)
			I2CLOG("I2C%d,EEPROM test PASS!!!!!!!!! \n",i);
		else
			I2CLOG("I2C%d,EEPROM test FAIL!!!!! ( %d)\n",i,ret);
	}
#endif
	I2CMSG("fwq test_i2c--------- 4");
}
#endif

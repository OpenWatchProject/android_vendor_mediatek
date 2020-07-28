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

#include "drStd.h"
#include "DrApi/DrApi.h"
#include "DrApi/DrApiMm.h"
#include "DrApi/DrApiThread.h"
#include "drTuiCommon.h"
#include "DrApi/DrApiCommon.h"
#include "DrApi/DrApiLogging.h"
#include "drTuiHal.h"
#include "drSecMemApi.h"

#include "spi.h"

/* for secure transaction */
#include "dapc.h"
#include "tlDapcIndex.h"
#define SPI_SECURE_LOCK

/* SPI register virtual base address*/
uint8_t  *SpiBaseVA;
uint8_t  *SpiPmBaseVA;

uint8_t  *SpiTxDmaVA;
uint8_t  *SpiRxDmaVA;

/*use this flag to sync every data transfer*/
static int32_t irq_flag = IRQ_IDLE;

static bool SpiRegHasMapped = false;

/* if users forgot to set chip_config, use this */
struct mtk_spi_chip_conf chip_config_default = {
	.setuptime = 3,
	.holdtime = 3,
	.cs_idletime = 2,
	.speed_hz = SPI_SPEED_DEFAULT,
	.cpol = 0,
	.cpha = 0,
	.rx_mlsb = 1,
	.tx_mlsb = 1,
	.cs_pol = 0,
	.tx_endian = 0,
	.rx_endian = 0,
	.com_mod = FIFO_TRANSFER,
	.deassert = 0,
	.tckdly = 0
};

void SpiDumpChipConfig(struct mtk_spi_chip_conf *chip_config)
{
	SPI_DEBUG("setuptime=%d\n",chip_config->setuptime);
	SPI_DEBUG("holdtime=%d\n",chip_config->holdtime);
	SPI_DEBUG("cs_idletime=%d\n",chip_config->cs_idletime);
	SPI_DEBUG("cpol=%d\n",chip_config->cpol);
	SPI_DEBUG("cpha=%d\n",chip_config->cpha);
	SPI_DEBUG("tx_mlsb=%d\n",chip_config->tx_mlsb);
	SPI_DEBUG("rx_mlsb=%d\n",chip_config->rx_mlsb);
	SPI_DEBUG("tx_endian=%d\n",chip_config->tx_endian);
	SPI_DEBUG("rx_endian=%d\n",chip_config->rx_endian);
	SPI_DEBUG("com_mod=%d\n",chip_config->com_mod);
	SPI_DEBUG("deassert=%d\n",chip_config->deassert);
	SPI_DEBUG("tckdly=%d\n",chip_config->tckdly);
}

void SpiDumpRegister(void)
{
	SPI_DEBUG("SPI_REG_CFG0=0x%08x\n",SPI_READ32(SPI_REG_CFG0));
	SPI_DEBUG("SPI_REG_CFG1=0x%08x\n",SPI_READ32(SPI_REG_CFG1));
	SPI_DEBUG("SPI_REG_CFG2=0x%08x\n",SPI_READ32(SPI_REG_CFG2));
	SPI_DEBUG("SPI_REG_TX_SRC=0x%08x\n",SPI_READ32(SPI_REG_TX_SRC));
	SPI_DEBUG("SPI_REG_RX_DST=0x%08x\n",SPI_READ32(SPI_REG_RX_DST));
	SPI_DEBUG("SPI_REG_TX_SRC_64=0x%08x\n",SPI_READ32(SPI_REG_TX_SRC_64));
	SPI_DEBUG("SPI_REG_RX_DST_64=0x%08x\n",SPI_READ32(SPI_REG_RX_DST_64));
	SPI_DEBUG("SPI_REG_CMD=0x%08x\n",SPI_READ32(SPI_REG_CMD));
	SPI_DEBUG("SPI_REG_PAD_SEL=0x%08x\n",SPI_READ32(SPI_REG_PAD_SEL));
}

/*
 * SPI Registers Mapping
 * set SpiRegHasMapped to prevent duplicate calls
 */
static int SpiMapRegister(void)
{
	int status = 0;
	uint32_t mapFlag = MAP_READABLE | MAP_WRITABLE | MAP_IO;

	SPI_DEBUG("enter %s\n", __func__);

	// register map
	status = drApiMapPhysicalBuffer((uint64_t)SPI_CLK_PA_BASE, 0x100,
		mapFlag, (void **)&SpiPmBaseVA);
	if (status == DRAPI_OK){
		SPI_INF("SPI_CLK_PA_BASE, hasMapped [0x%08x]\n", SpiPmBaseVA);
	} else
		status = -1;

	status = drApiMapPhysicalBuffer((uint64_t)SPI_REG_PA_BASE, 0x100,
		mapFlag, (void **)&SpiBaseVA);
	if (status == DRAPI_OK) {
		SPI_INF("SPI_REG_PA_BASE, hasMapped [0x%08x]\n", SpiBaseVA);
	} else
		status = -2;

	if (status != DRAPI_OK) {
		SPI_ERR("SPI_PM_PA_BASE, Mapped err: status: %d\n", status);
	} else
		SpiRegHasMapped = true;

	SPI_INF("spi_map_regiser done[status = %d]\n", SpiRegHasMapped);
	return status;
}

static int SpiDmaPaQuery(struct spi_transfer *xfer)
{
	drApiResult_t ret = 0;

	xfer->tx_dma = PLRESERVED_TX;
	xfer->rx_dma = PLRESERVED_RX;
	if (!xfer->tx_dma || !xfer->rx_dma)
		return -1;

	SPI_INF("%s SUCCESS!!!\n", __func__);
	//spi measter use secure transaction
	ret = set_master_transaction(SECURE_TRANSFER_SPI, SECURE_TRANSACTION);
	if (ret == TLAPI_OK)
		SPI_INF("Spi UseSecure DMA!!\n");

	return 0;
}

static int SpiDmaVaMapping(struct spi_transfer *xfer)
{
	drApiResult_t ret;
	uint32_t mapFlag = MAP_READABLE | MAP_WRITABLE | MAP_UNCACHED ;
	uint8_t *temp = 0;

	if (xfer->len > 200) {
		SPI_ERR("tui SPI only support transfer len <= 200\n");
		return -1;
	}

	ret = drApiMapPhysicalBuffer((uint64_t)(xfer->rx_dma),
			200, mapFlag, (void **)&temp);
	if (ret != TLAPI_OK) {
		SPI_ERR("[SpiDMAMapping] drApiMapPhysical RX Buffer failed ret=%d\n",
			ret);
	} else
		SpiRxDmaVA = temp;

	ret = drApiMapPhysicalBuffer((uint64_t)(xfer->tx_dma),
				200, mapFlag, (void **)&temp);
	if (ret != TLAPI_OK) {
		SPI_ERR("[SpiDMAMapping] drApiMapPhysical TX Buffer failed ret=%d\n",
			ret);
	} else
		SpiTxDmaVA = temp;

	return 0;
}

static void SpiDmaVaUnMapping(void)
{
	drApiResult_t ret;

	ret = drApiUnmapBuffer(SpiTxDmaVA);
	if (ret != TLAPI_OK)
		SPI_ERR("[SpiDMAMapping] drApiMapPhysicalBuffer Tx failed ret=%d\n",
			ret);

	ret = drApiUnmapBuffer(SpiRxDmaVA);
	if (ret != TLAPI_OK)
		SPI_ERR("[SpiDMAMapping] drApiMapPhysicalBuffer Rx failed ret=%d\n",
			ret);

	SPI_INF("spi_dma_addr, UnMapped\n");
}

static void SpiIrqAttach(void)
{
    drApiResult_t attachedRet =
		drApiIntrAttach(MT6768_SPI_SECURE_IRQ_ID, INTR_MODE_LOW_LEVEL);
    //active low level-sensitive
    if (DRAPI_OK != attachedRet) {
		SPI_ERR("SpiIrqAttach thread failed IRQ[%d], res = [%d]\n",
			MT6768_SPI_SECURE_IRQ_ID, attachedRet);
    }
}

static void SpiIrqDetach(void)
{
	SPI_DEBUG("enter %s\n", __func__);
	//active low level-sensitive
    drApiResult_t attachedRet =
		drApiIntrDetach(MT6768_SPI_SECURE_IRQ_ID);

	if (DRAPI_OK != attachedRet) {
		SPI_ERR("SpiIrq_Detach failed IRQ[%d], res = [%d]\n",
			MT6768_SPI_SECURE_IRQ_ID, attachedRet);
    }
}

static void inline SpiDmaPrepare(struct spi_transfer *xfer)
{
	uint32_t reg_val = 0;

	/* enable DMA */
	if (xfer->is_last_xfer == 0) {
		reg_val = SPI_READ32 (SPI_REG_CMD);
		reg_val |= (DMA_TRANSFER << SPI_CMD_RX_DMA_OFFSET);
		reg_val |= (DMA_TRANSFER << SPI_CMD_TX_DMA_OFFSET);
		SPI_WRITE32 (SPI_REG_CMD, reg_val);
	}

#define SPI_4B_ALIGN 0x4
	/*set up the DMA bus address*/
	if(xfer->tx_dma & (SPI_4B_ALIGN - 1)) {
		SPI_ERR("Tx_DMA address should be 4Byte alignment,buf:%p,dma:%08x\n",
			xfer->tx_buf,xfer->tx_dma);
	}

	SPI_WRITE32(SPI_REG_TX_SRC_64, 0x0);
	SPI_WRITE32(SPI_REG_TX_SRC, xfer->tx_dma);
	SPI_DEBUG("SpiEnableDma, SpiTxDmaVA = [0x%08x]\n", SpiTxDmaVA);

	if(xfer->rx_dma & (SPI_4B_ALIGN - 1)) {
		SPI_ERR("Rx_DMA address should be 4Byte alignment,buf:%p,dma:%08x\n",
			xfer->rx_buf,xfer->rx_dma);
	}

	SPI_WRITE32(SPI_REG_RX_DST_64, 0x0);
	SPI_WRITE32(SPI_REG_RX_DST, xfer->rx_dma);
	SPI_DEBUG("SpiEnableDma, SpiRxDmaVA = [0x%08x]\n", SpiRxDmaVA);
}

/*
* Write chip configuration to HW register
*/
static int SpiSetup(struct mtk_spi_chip_conf *chip_config)
{
	uint32_t reg_val = 0;
	uint32_t div, sck_time;

	if (!chip_config->speed_hz)
		chip_config->speed_hz = SPI_SPEED_DEFAULT;

	if (chip_config->speed_hz < SPI_CLK_HZ / 2)
		div = DIV_ROUND_UP(SPI_CLK_HZ, chip_config->speed_hz);
	else
		div = 1;
	sck_time = (div + 1) / 2;

	reg_val = SPI_READ32(SPI_REG_CFG0);
	reg_val &= ~(SPI_CFG0_CS_HOLD_MASK | SPI_CFG0_CS_SETUP_MASK);

	if (chip_config->holdtime)
		reg_val |= ((chip_config->holdtime-1) << SPI_CFG0_CS_HOLD_OFFSET);
	else
		reg_val |= ((2 * sck_time) << SPI_CFG0_CS_HOLD_OFFSET);

	if (chip_config->setuptime)
		reg_val |= ((chip_config->setuptime-1) << SPI_CFG0_CS_SETUP_OFFSET);
	else
		reg_val |= ((2 * sck_time) << SPI_CFG0_CS_SETUP_OFFSET);
	SPI_WRITE32(SPI_REG_CFG0, reg_val);

	reg_val = SPI_READ32 (SPI_REG_CFG1);
	reg_val &= ~(SPI_CFG1_CS_IDLE_MASK);
	reg_val &= ~(SPI_CFG1_GET_TICK_DLY_MASK);

	if (chip_config->cs_idletime)
		reg_val |= ((chip_config->cs_idletime-1) << SPI_CFG1_CS_IDLE_OFFSET);
	else
		reg_val |= ((2 * sck_time) << SPI_CFG1_CS_IDLE_OFFSET);

	if (chip_config->tckdly)
		reg_val |= ((chip_config->tckdly) << SPI_CFG1_GET_TICK_DLY_OFFSET);
	else
		reg_val |= (TICK_DLY0 << SPI_CFG1_GET_TICK_DLY_OFFSET);
	SPI_WRITE32( SPI_REG_CFG1, reg_val );

	reg_val = SPI_READ32( SPI_REG_CFG2 );
	reg_val &= ~ (SPI_CFG0_SCK_HIGH_MASK |SPI_CFG0_SCK_LOW_MASK);
	reg_val |= ((sck_time-1) << SPI_CFG0_SCK_HIGH_OFFSET);
	reg_val |= ((sck_time-1) << SPI_CFG0_SCK_LOW_OFFSET);
	SPI_WRITE32(SPI_REG_CFG2, reg_val);

	/*set the mlsbx mlsbtx endian cpha cpol cs_pol deassert*/
	reg_val = SPI_READ32 (SPI_REG_CMD);
	reg_val &= ~ (SPI_CMD_TX_ENDIAN_MASK | SPI_CMD_RX_ENDIAN_MASK);
	reg_val &= ~ (SPI_CMD_TXMSBF_MASK| SPI_CMD_RXMSBF_MASK);
	reg_val &= ~ (SPI_CMD_CPHA_MASK | SPI_CMD_CPOL_MASK);
	reg_val &= ~ SPI_CMD_CS_POL_MASK;
	reg_val &= ~ SPI_CMD_DEASSERT_MASK;
	if (chip_config->tx_endian == 1)
		reg_val |= (SPI_BENDIAN << SPI_CMD_TX_ENDIAN_OFFSET);
	else
		reg_val |= (SPI_LENDIAN << SPI_CMD_TX_ENDIAN_OFFSET);

	if (chip_config->rx_endian == 1)
		reg_val |= (SPI_BENDIAN << SPI_CMD_RX_ENDIAN_OFFSET);
	else
		reg_val |= (SPI_LENDIAN << SPI_CMD_RX_ENDIAN_OFFSET);

	if (chip_config->tx_mlsb == 1)
		reg_val |= (SPI_MSB << SPI_CMD_TXMSBF_OFFSET);
	else
		reg_val |= (SPI_LSB << SPI_CMD_TXMSBF_OFFSET);

	if (chip_config->rx_mlsb == 1)
		reg_val |= (SPI_MSB << SPI_CMD_RXMSBF_OFFSET);
	else
		reg_val |= (SPI_LSB << SPI_CMD_RXMSBF_OFFSET);

	if (chip_config->cpha == 1)
		reg_val |= (SPI_CPHA_1 << SPI_CMD_CPHA_OFFSET);
	else
		reg_val |= (SPI_CPHA_0 << SPI_CMD_CPHA_OFFSET);

	if (chip_config->cpol == 1)
		reg_val |= (SPI_CPOL_1 << SPI_CMD_CPOL_OFFSET);
	else
		reg_val |= (SPI_CPOL_0 << SPI_CMD_CPOL_OFFSET);

	if (chip_config->cs_pol == 1)
		reg_val |= (CS_HIGH << SPI_CMD_CS_POL_OFFSET);
	else
		reg_val |= (CS_LOW << SPI_CMD_CS_POL_OFFSET);
	if (chip_config->deassert == 1)
		reg_val |= (DEASSERT_ENABLE << SPI_CMD_DEASSERT_OFFSET);
	else
		reg_val |= (DEASSERT_DISABLE << SPI_CMD_DEASSERT_OFFSET);
	SPI_WRITE32(SPI_REG_CMD, reg_val);

	/*set finish interrupt always enable*/
	reg_val = SPI_READ32 ( SPI_REG_CMD);
	reg_val &= ~ SPI_CMD_FINISH_IE_MASK;
	reg_val |= (1 << SPI_CMD_FINISH_IE_OFFSET);
	SPI_WRITE32 (SPI_REG_CMD, reg_val);

	reg_val = SPI_READ32 ( SPI_REG_CMD);
	reg_val &= (FIFO_TRANSFER << SPI_CMD_RX_DMA_OFFSET);
	reg_val &= (FIFO_TRANSFER << SPI_CMD_TX_DMA_OFFSET);
	SPI_WRITE32 (SPI_REG_CMD, reg_val);

#if 1
	reg_val = SPI_READ32 (SPI_REG_PAD_SEL);
	reg_val |= 0x7;
	SPI_WRITE32 (SPI_REG_PAD_SEL, reg_val);
#endif

	return 0;
}

static void inline ClearPauseBit(void)
{
	uint32_t reg_val;

	reg_val = SPI_READ32(SPI_REG_CMD);
	reg_val &= ~SPI_CMD_PAUSE_EN_MASK;
	SPI_WRITE32(SPI_REG_CMD, reg_val);
}

static void inline SetPauseBit(void)
{
	uint32_t reg_val;

	reg_val = SPI_READ32(SPI_REG_CMD);
	reg_val |= 1 << SPI_CMD_PAUSE_EN_OFFSET;
	SPI_WRITE32(SPI_REG_CMD, reg_val);
}

static void SpiReset(void)
{
	uint32_t reg_val;

	/*set the software reset bit in SPI_REG_CMD.*/
	reg_val = SPI_READ32(SPI_REG_CMD);
	reg_val &= ~SPI_CMD_RST_MASK;
	reg_val |= 1 << SPI_CMD_RST_OFFSET;
	SPI_WRITE32(SPI_REG_CMD, reg_val);

	reg_val = SPI_READ32(SPI_REG_CMD);
	reg_val &= ~SPI_CMD_RST_MASK;
	SPI_WRITE32(SPI_REG_CMD, reg_val);
}

/*
*  SpiSetupPacket: used to define per data length and loop count
* @ ptr : data structure from User
*/
static int inline SpiSetupPacket(struct spi_transfer *xfer)
{
	uint32_t  packet_size, packet_loop, reg_val;

	//set transfer packet and loop
	if(xfer->len < SPI_PACKET_SIZE)
		packet_size = xfer->len;
	else
		packet_size = SPI_PACKET_SIZE;

	if(xfer->len % packet_size){
		SPI_INF("The lens are not a multiple of %d, your len %u\n\n",
			SPI_PACKET_SIZE, xfer->len);
	}

	packet_loop = xfer->len / packet_size;

	reg_val = SPI_READ32(SPI_REG_CFG1);
	reg_val &= ~(SPI_CFG1_PACKET_LENGTH_MASK + SPI_CFG1_PACKET_LOOP_MASK);
	reg_val |= (packet_size - 1)<<SPI_CFG1_PACKET_LENGTH_OFFSET;
	reg_val |= (packet_loop - 1)<<SPI_CFG1_PACKET_LOOP_OFFSET;
	SPI_WRITE32(SPI_REG_CFG1, reg_val);

	return 0;
}

static int inline SpiFifoFillTx(struct spi_transfer *xfer)
{
	uint32_t cnt_quotient, cnt_remainder, i;
	uint32_t reg_val, value = 0;
	uint8_t *p8_tmp;

	SPI_DEBUG("FIFO start to transfer data, xfer->len:%d\n", xfer->len);
	cnt_quotient = xfer->len / 4;
	cnt_remainder = xfer->len % 4;
	/*Handling 4bytes data*/
	for(i = 0; i < cnt_quotient; i++) {
		SPI_WRITE32(SPI_REG_TX_DATA, *(xfer->tx_buf + i));
	}
	/*Handling less than 4bytes data*/
	if (cnt_remainder) {
	/*Convert the 32bits point(point to less than 4bytes data) to 8bits point*/
		p8_tmp = (uint8_t *)(xfer->tx_buf + cnt_quotient);

		for(i = 0; i < cnt_remainder; i++) {
			SPI_DEBUG("tx_buf[%d](None 4B Align):0x%02x,tx_buf addr is:0x%p\n",
				i, *(p8_tmp + i), p8_tmp + i);

			value |= ((uint32_t)(*(p8_tmp + i))) << (8 * i);
		}
		SPI_WRITE32(SPI_REG_TX_DATA, value);
		SPI_DEBUG("fifo tx_buf data(synthetic) is:0x%08x\n", value);
	}
}

static int inline SpiDmaFillTx(struct spi_transfer *xfer)
{
	uint32_t cnt_quotient, cnt_remainder, i;
	uint32_t reg_val, value = 0;
	uint8_t *p8_tmp;

	SPI_DEBUG("DMA start to transfer data, xfer->len:%d\n", xfer->len);

	cnt_quotient = xfer->len / 4;
	cnt_remainder = xfer->len % 4;

	SPI_DEBUG("tx_buf[%d](4B Align) is:0x%08x\n", cnt_quotient, cnt_remainder);

	/*Handling 4bytes data*/
	for(i = 0; i < cnt_quotient; i++) {
		SPI_WRITE32((SpiTxDmaVA + 4 * i) , *(xfer->tx_buf + i));
		if (i < 4)
			SPI_DEBUG("tx_buf[%d](4B Align) is:0x%08x\n",
			  i, *(xfer->tx_buf + i));
	}

	/*Handling less than 4bytes data*/
	if (cnt_remainder) {
	/*Convert the 32bits point(point to less than 4bytes data) to 8bits point*/
		p8_tmp = (uint8_t *)(xfer->tx_buf + cnt_quotient);
		for(i = 0; i < cnt_remainder; i++) {
			SPI_DEBUG("tx_buf[%d](No 4B Align) is:0x%02x\n",
				i, *(p8_tmp + i));
			SPI_WRITE8(((SpiTxDmaVA + 4 * cnt_quotient) +  i), *(p8_tmp + i));
		}
	}
}

#ifdef SPI_DMA_TWICE
static void SpiUpdateXfer(struct spi_transfer *xfer, uint32_t packet_loop,
	uint32_t rest_size)
{
	xfer->rx_buf += (1024 * packet_loop) / 4;
	xfer->tx_buf += (1024 * packet_loop) / 4;
	xfer->rx_dma += (1024 * packet_loop) / 4;
	xfer->tx_dma += (1024 * packet_loop) / 4;
	SpiRxDmaVA += 1024 * packet_loop;
	SpiTxDmaVA += 1024 * packet_loop;
	xfer->len = rest_size;

	SPI_DEBUG("rx_buf: %p; tx_buf: %p; len: %d; rx_dma %p; tx_dma %p\n",
		xfer->rx_buf, xfer->tx_buf, xfer->len,
		xfer->rx_dma, xfer->tx_dma);
}
#endif
static void SpiHandleIRQ(struct spi_transfer *xfer)
{
	unsigned long flags;
	uint32_t reg_val, i, cnt_quotient, cnt_remainder;
	uint8_t *p8_Tmp;
	SPI_DEBUG("------------SpiHandleIRQ Start read data------------\n");

	/*Using FIFO/DMA to receive data*/
	/*Default mode: LITTLE-ENDIAN*/
	cnt_quotient = xfer->len / 4;
	cnt_remainder = xfer->len % 4;

	SPI_DEBUG("cnt_quotient = %d, cnt_remainder = %d\n",
		cnt_quotient, cnt_remainder);

	if(xfer->chip_config->com_mod == FIFO_TRANSFER) {
		SPI_INF("FIFO start to read data xfer->len:%d!!\n", xfer->len);

		/*Handling 4bytes data*/
		for(i = 0; i < cnt_quotient; i++) {
				reg_val = SPI_READ32(SPI_REG_RX_DATA);

				if(i < 4)
					SPI_DEBUG("rx_buf[%d] data(4B Align) is:%08x\n", i, reg_val);

				*(xfer->rx_buf + i) = reg_val;
		}

		/*Handling less than 4bytes data*/
		if (cnt_remainder) {
			reg_val = SPI_READ32(SPI_REG_RX_DATA);

			SPI_DEBUG("rx_buf data(None 4B Align):%08x\n", reg_val);

			/*Convert the 32bits point(point to less than 4bytes data) to 8b p*/
			p8_Tmp = (uint8_t *)(xfer->rx_buf + cnt_quotient);

			for(i = 0; i < cnt_remainder; i++) {
				*(p8_Tmp + i) = (uint8_t)(reg_val >> (i * 8));

				SPI_DEBUG("rx_buf[%d] data(disintegrate):%02x\n",
					i, *(p8_Tmp + i));

			}
		}
	} else if (xfer->chip_config->com_mod == DMA_TRANSFER) {
		SPI_DEBUG("DMA start to read data xfer->len:%d!!\n", xfer->len);

		/*Handling 4bytes data*/
		for(i = 0; i < cnt_quotient; i++) {
			reg_val = SPI_READ32(SpiRxDmaVA + 4 * i);
			if(i > cnt_quotient - 4)
				SPI_DEBUG("rx_buf[%d] data(4B Align) is:%08x\n", i, reg_val);
				*(xfer->rx_buf + i) = reg_val;
		}

		/*Handling less than 4bytes data*/
		if (cnt_remainder) {
	/*Convert the 32bits point(point to less than 4bytes data) to 8bits point*/
			p8_Tmp = (uint8_t *)(xfer->rx_buf + cnt_quotient);
			for(i = 0; i < cnt_remainder; i++) {
				*(p8_Tmp + i) = SPI_READ8((SpiRxDmaVA + 4 * cnt_quotient) + i);
				SPI_DEBUG("rx_buf[%d](None 4B Align):%02x\n",
					i, *(p8_Tmp + i));
			}
		}
	}
	xfer->is_last_xfer = 1;
	SPI_INF("Pause set IDLE state & disable clk\n");
	SPI_INF("----------------SPI HandleIRQ/Transfer end--------------\n");
}

static void SpiIrqHandler(struct spi_transfer *xfer)
{
    uint32_t irqStatus;

	do {
		irqStatus = SPI_READ32(SPI_REG_STATUS1);
	} while (irqStatus == 0);

	irqStatus = SPI_READ32(SPI_REG_STATUS0) & 0x00000003;
	SPI_DEBUG("IRQ: irqStatus[0x%08x]\n", irqStatus);

	SpiHandleIRQ(xfer);
}

static void inline SpiStartTransfer(struct spi_transfer *xfer,
	uint32_t flag)
{
	uint32_t reg_val;

	SPI_DEBUG("%s!!!\n", __func__);
	reg_val = SPI_READ32(SPI_REG_CMD);

	if (flag == SPI_START_TRANSFER)
		reg_val |= 1 << SPI_CMD_ACT_OFFSET;
	else if (flag == SPI_RESUME_TRANSFER) {
		reg_val |= 1 << SPI_CMD_RESUME_OFFSET;
		reg_val &= ~SPI_CMD_PAUSE_EN_MASK;
	}
	/*All register must be prepared before setting the start bit [SMP]*/
	SPI_WRITE32(SPI_REG_CMD, reg_val);

	SpiIrqHandler(xfer);
}

int tui_spi_sync(void *rx_buf, const void *tx_buf,
	struct mtk_spi_chip_conf *chip_config, uint32_t len)
{
	struct spi_transfer xfer;
	uint32_t packet_loop, rest_size;
	int ret;

	SPI_DEBUG("%s: enter\n", __func__);
	if (!rx_buf || !tx_buf || !len) {
		ret = -1;
		SPI_ERR("Please check rx_buf tx_buf or len, ret = %d\n", ret);
		goto fail;
	}

	while(irq_flag == IRQ_BUSY) {
		/*need a pause instruction to avoid unknow exception*/
		SPI_INF("wait IRQ handle finish\n");
	}
	/*set flag to block next transfer*/
	irq_flag = IRQ_BUSY;

	/* Before start we should attach irq to tee */
	SpiIrqAttach();

#ifdef SPI_SECURE_LOCK
	start_devapc();
	/* enable secure transfer */
	set_module_apc(DAPC_LOCK_SPI, DOMAIN_AP, SECURE_TRANSACTION);
#endif

	/* Map register PA to VA */
	if(!SpiRegHasMapped)
		SpiMapRegister();

	/* Set clk in kernel */
	SPI_DEBUG("SPI_CLK CG = %d, STA = 0x%08x\n",
		SPI_CLK_CG_OFFSET, SPI_READ32(SPI_CLK_CG_STA));

	xfer.len = len;
	xfer.tx_buf = (uint32_t *)tx_buf;
	xfer.rx_buf = (uint32_t *)rx_buf;
	xfer.is_last_xfer = 0;

	for (ret = len - 4; ret < len; ret++)
		SPI_DEBUG("tx[%d] = 0x%08x\n", ret, *(xfer.tx_buf + ret));

	if (chip_config == NULL) {
		SPI_INF("use default chip_config\n");
		xfer.chip_config = &chip_config_default;
	} else
		xfer.chip_config = chip_config;

	if (xfer.len <= 32)
		xfer.chip_config->com_mod= FIFO_TRANSFER;
	else {
		xfer.chip_config->com_mod = DMA_TRANSFER;
		SpiDmaPaQuery(&xfer);
		if (SpiDmaVaMapping(&xfer)) {
			ret = -1;
			goto err_clk;
		}
	}
	SPI_DEBUG("xfer %p; xfer->len:%d; tx_buf/ %p; rx_buf %p; mode %d\n",
		&xfer, len, tx_buf, rx_buf, xfer.chip_config->com_mod);

	/* reset spi register */
	SpiReset();

	/* set spi register*/
	if (SpiSetup(xfer.chip_config)) {
		ret = -1;
		goto err_clk;
	}

	SpiDumpChipConfig(xfer.chip_config);
	packet_loop = xfer.len / 1024;
	rest_size = xfer.len % 1024;
	/* for onetime transfer */
	if (xfer.len <=1024 || rest_size == 0) {
		ClearPauseBit();
		SpiSetupPacket(&xfer);
		SpiDumpRegister();
        if (xfer.chip_config->com_mod == FIFO_TRANSFER)
			SpiFifoFillTx(&xfer);
		else if (xfer.chip_config->com_mod == DMA_TRANSFER) {
			SpiDmaPrepare(&xfer);
			SpiDmaFillTx(&xfer);
		}
		SpiStartTransfer(&xfer, SPI_START_TRANSFER);
		goto out;
	}
#ifdef SPI_DMA_TWICE
	else {/* for twice transfer*/
		SetPauseBit();
		SpiSetupPacket(&xfer);
		SpiDmaPrepare(&xfer);//prepare once
		SpiDmaFillTx(&xfer); //one transfer fill TX once
		SpiDumpRegister();
		SpiStartTransfer(&xfer, SPI_START_TRANSFER);
	}

	if (xfer.is_last_xfer == 1) {
		SPI_INF("One DMA transfer Done.Start Next!!!\n");
		SpiUpdateXfer(&xfer, packet_loop, rest_size);
		SpiDmaPrepare(&xfer);
		SpiSetupPacket(&xfer);
		SpiStartTransfer(&xfer, SPI_RESUME_TRANSFER);
	}

	for (ret = len - 4; ret < len; ret++)
		SPI_DEBUG(" rx[%d] = 0x%08x\n", ret, *(xfer.rx_buf + ret));
#endif
out:
err_clk:
	/* After transfer we should de-attach irq in tee */
	SpiIrqDetach();
	irq_flag = IRQ_IDLE;
	xfer.is_last_xfer = 0;
	if (xfer.chip_config->com_mod == DMA_TRANSFER)
		SpiDmaVaUnMapping();
fail:
	return ret;
}


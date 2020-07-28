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

#ifndef __MTK_SPI_H__
#define __MTK_SPI_H__

#include "spi_reg.h"

/* SPI LOG Define
 * SPI_INF:    SPI Key massage prompt
 * SPI_DEBUG:  SPI driver debug
 * SPI_ERR:    SPI err log
 * SPI_PERFORMANCE: SPI performance debug
 */
#define SPI_INF(string, args...) \
    if(0)                        \
    {                            \
        drApiLogPrintf("[SPI][INF]"string, ##args);\
    }
#define SPI_DEBUG(string, args...)\
    if(0)                         \
    {                             \
        drApiLogPrintf("[SPI][DEBUG]"string, ##args);\
    }

#define SPI_ERR(string, args...)  \
    if(1)                         \
    {                             \
        drApiLogPrintf("[SPI][ERR]"string, ##args);\
    }
/*---------------------SPI_PERFORMANCE--------------------*/
#define SPI_PERFORMANCE(string, args...) \
    if(0)                                \
    {                                    \
        drApiLogPrintf("[SPI][PERFORMANCE]"string, ##args);\
    }


#define DIV_ROUND_UP(n,d)  (((n) + (d) - 1) / (d))

/*
 * SPI_CHIP_CFG for slave device
 */
enum spi_cpol {
	SPI_CPOL_0,
	SPI_CPOL_1
};

enum spi_cpha {
	SPI_CPHA_0,
	SPI_CPHA_1
};

enum spi_mlsb {
	SPI_LSB,
	SPI_MSB
};

enum spi_cs_pol {
	CS_LOW,
	CS_HIGH
};

enum spi_endian {
	SPI_LENDIAN,
	SPI_BENDIAN
};

enum spi_transfer_mode {
	FIFO_TRANSFER,
	DMA_TRANSFER,
	OTHER1,
	OTHER2,
};

enum spi_deassert_mode {
	DEASSERT_DISABLE,
	DEASSERT_ENABLE
};

enum spi_tckdly {
	TICK_DLY0,
	TICK_DLY1,
	TICK_DLY2,
	TICK_DLY3
};

enum spi_irq_flag {
	IRQ_IDLE,
	IRQ_BUSY
};

struct mtk_spi_chip_conf {
		uint32_t setuptime;
		uint32_t holdtime;
		uint32_t cs_idletime;
		uint32_t speed_hz;
		enum spi_cpol cpol;
		enum spi_cpha cpha;
		enum spi_mlsb tx_mlsb;
		enum spi_mlsb rx_mlsb;
		enum spi_cs_pol cs_pol;
		enum spi_endian tx_endian;
		enum spi_endian rx_endian;
		enum spi_transfer_mode com_mod;
		enum spi_deassert_mode deassert;
		enum spi_tckdly tckdly;
};

struct spi_transfer {
	struct mtk_spi_chip_conf *chip_config;
	uint32_t *tx_buf;
	uint32_t *rx_buf;
	uint32_t len;
	/* dma_addr_t */
	uint32_t tx_dma;
	uint32_t rx_dma;
	/* for clear pause bit */
	uint32_t is_last_xfer;
};

int tui_spi_sync(void *rx_buf, const  void *tx_buf,
	struct mtk_spi_chip_conf *chip_config, uint32_t len);

extern int set_master_transaction(unsigned int master_index,
	unsigned int transaction_type);

#endif //__MTK_SPI_H__


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

#ifndef __MTK_SPI_REG_H__
#define __MTK_SPI_REG_H__

/*
 * SPI register define
 *
 * secure dirver map region (via drApiMapPhys)
 * 1. t-driver will own virtual address
 * 2. SPI used virtual address range is 0x80000 - 0xFEFFF
 */
#define SPI_REG_CFG0         (SpiBaseVA + 0x00)
#define SPI_REG_CFG1         (SpiBaseVA + 0x04)
#define SPI_REG_TX_SRC       (SpiBaseVA + 0x08)
#define SPI_REG_RX_DST       (SpiBaseVA + 0x0C)
#define SPI_REG_TX_DATA      (SpiBaseVA + 0x10)
#define SPI_REG_RX_DATA      (SpiBaseVA + 0x14)
#define SPI_REG_CMD          (SpiBaseVA + 0x18)
#define SPI_REG_STATUS0      (SpiBaseVA + 0x1C)
#define SPI_REG_STATUS1      (SpiBaseVA + 0x20)
#define SPI_REG_PAD_SEL      (SpiBaseVA + 0x24)
#define SPI_REG_CFG2         (SpiBaseVA + 0x28)
#define SPI_REG_TX_SRC_64    (SpiBaseVA + 0x2C)
#define SPI_REG_RX_DST_64    (SpiBaseVA + 0x30)

#define SPI_FIFO_MAX_SIZE           32
#define SPI_PACKET_SIZE             1024
#define SPI_CLK_HZ                  109200000
#define SPI_SPEED_DEFAULT           8000000
#define SPI_GIC_PRIVATE_SIGNALS     32

#define ENABLE_CLK                  0
#define DISABLE_CLK                 1
#define SPI_START_TRANSFER          0
#define SPI_RESUME_TRANSFER         1

/* should be change depend on diff platform */
#define PLRESERVED_RX               0xA3C00000
#define PLRESERVED_TX               0xA3C10000
#define SPI_REG_PA_BASE             0x11012000
#define SPI_CLK_PA_BASE             0x10001000
#define SPI_CLK_CG_OFFSET           9
#define SPI_CLK_CG_STA              (SpiPmBaseVA + 0xac)
#define SECURE_TRANSFER_SPI         DAPC_MASTER_INDEX_SPI2
#define DAPC_LOCK_SPI               DAPC_INDEX_PERISYS_SPI2
#define MT6768_SPI_SECURE_IRQ_ID    (SPI_GIC_PRIVATE_SIGNALS + 145)

/*
 * SPI register OFFSET
 */
#define SPI_CFG0_SCK_HIGH_OFFSET          0
#define SPI_CFG0_SCK_LOW_OFFSET           16
#define SPI_CFG0_CS_HOLD_OFFSET           0
#define SPI_CFG0_CS_SETUP_OFFSET          16

#define SPI_CFG0_SCK_HIGH_MASK            0xffff
#define SPI_CFG0_SCK_LOW_MASK             0xffff0000
#define SPI_CFG0_CS_HOLD_MASK             0xffff
#define SPI_CFG0_CS_SETUP_MASK            0xffff0000

#define SPI_CFG1_CS_IDLE_OFFSET           0
#define SPI_CFG1_PACKET_LOOP_OFFSET       8
#define SPI_CFG1_PACKET_LENGTH_OFFSET     16
#define SPI_CFG1_GET_TICK_DLY_OFFSET      29

#define SPI_CFG1_CS_IDLE_MASK             0xff
#define SPI_CFG1_PACKET_LOOP_MASK         0xff00
#define SPI_CFG1_PACKET_LENGTH_MASK       0x3fff0000
#define SPI_CFG1_GET_TICK_DLY_MASK        0xe0000000

#define SPI_CMD_ACT_OFFSET                0
#define SPI_CMD_RESUME_OFFSET             1
#define SPI_CMD_RST_OFFSET                2
#define SPI_CMD_PAUSE_EN_OFFSET           4
#define SPI_CMD_DEASSERT_OFFSET           5
#define SPI_CMD_SAMPLE_SEL_OFFSET         6
#define SPI_CMD_CS_POL_OFFSET             7
#define SPI_CMD_CPHA_OFFSET               8
#define SPI_CMD_CPOL_OFFSET               9
#define SPI_CMD_RX_DMA_OFFSET             10
#define SPI_CMD_TX_DMA_OFFSET             11
#define SPI_CMD_TXMSBF_OFFSET             12
#define SPI_CMD_RXMSBF_OFFSET             13
#define SPI_CMD_RX_ENDIAN_OFFSET          14
#define SPI_CMD_TX_ENDIAN_OFFSET          15
#define SPI_CMD_FINISH_IE_OFFSET          16
#define SPI_CMD_PAUSE_IE_OFFSET           17

#define SPI_CMD_ACT_MASK                  0x1
#define SPI_CMD_RESUME_MASK               0x2
#define SPI_CMD_RST_MASK                  0x4
#define SPI_CMD_PAUSE_EN_MASK             0x10
#define SPI_CMD_DEASSERT_MASK             0x20
#define SPI_CMD_CS_POL_MASK               0x80
#define SPI_CMD_CPHA_MASK                 0x100
#define SPI_CMD_CPOL_MASK                 0x200
#define SPI_CMD_RX_DMA_MASK               0x400
#define SPI_CMD_TX_DMA_MASK               0x800
#define SPI_CMD_TXMSBF_MASK               0x1000
#define SPI_CMD_RXMSBF_MASK               0x2000
#define SPI_CMD_RX_ENDIAN_MASK            0x4000
#define SPI_CMD_TX_ENDIAN_MASK            0x8000
#define SPI_CMD_FINISH_IE_MASK            0x10000
#define SPI_CMD_PAUSE_IE_MASK             0x20000

// REG API
#define SPI_READ32(addr)                (*(volatile uint32_t*)(addr))
#define SPI_WRITE32(addr, val)          (*(volatile uint32_t*)(addr) = val)

#define SPI_READ8(addr)                 (*(volatile uint8_t*)(addr))
#define SPI_WRITE8(addr, val)           (*(volatile uint8_t*)(addr) = val)

#endif // __MTK_SPI_REG_H__


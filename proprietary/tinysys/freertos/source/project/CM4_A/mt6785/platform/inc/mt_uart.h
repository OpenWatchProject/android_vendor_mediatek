/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 */
/* MediaTek Inc. (C) 2018. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER ON
 * AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
 * NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
 * SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
 * SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES TO LOOK ONLY TO SUCH
 * THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. RECEIVER EXPRESSLY ACKNOWLEDGES
 * THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES
 * CONTAINED IN MEDIATEK SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK
 * SOFTWARE RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND
 * CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,
 * AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE,
 * OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY RECEIVER TO
 * MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.

 *
 * The following software/firmware and/or related documentation ("MediaTek Software")
 * have been modified by MediaTek Inc. All revisions are subject to any receiver\'s
 * applicable license agreements with MediaTek Inc.
 */

#include <mt_reg_base.h>


#define NO_USE_UART

#ifdef CFG_MTK_APUART_SUPPORT
#undef NO_USE_UART
#undef USING_BCLK
#define USING_APUART_PORT
#endif

#ifdef CFG_MTK_SCPUART_SUPPORT
#undef NO_USE_UART
#define USING_BCLK
#undef USING_APUART_PORT
#endif

#ifdef USING_APUART_PORT
#undef UART0_BASE
#define UART0_BASE            (0xA1002000)
#endif


#ifndef CFG_FPGA_UART_SUPPORT
#ifdef USING_BCLK
//#define UART_SRC_CLK        (20833333)  //SCP ULPOSC=250M ,use ulposc_div12
#define UART_SRC_CLK          (26000000)  //SCP sysclk = 26M
#define UART_LOG_BAUD_RATE    (921600)
#else
#define UART_SRC_CLK          (26000000) //AP uart
#define UART_LOG_BAUD_RATE    (921600)
#endif
#else
#define UART_SRC_CLK          (10000000) //use in FPGA
#define UART_LOG_BAUD_RATE    (921600)
#endif

#ifdef CFG_SCP_UART1_SUPPORT
#define UART_LOG_PORT        UART1_BASE
#else
#define UART_LOG_PORT        UART0_BASE
#endif

#define UART_SET_BITS(REG, BS)       OUTREG32(REG, INREG32(REG) | (unsigned int)(BS))
#define UART_CLR_BITS(REG, BC)       OUTREG32(REG, INREG32(REG) & ~((unsigned int)(BC)))

#define UART_RBR(uart)                    ((uart)+0x0)  /* Read only */
#define UART_THR(uart)                    ((uart)+0x0)  /* Write only */
#define UART_IER(uart)                    ((uart)+0x4)
#define UART_IIR(uart)                    ((uart)+0x8)  /* Read only */
#define UART_FCR(uart)                    ((uart)+0x8)  /* Write only */
#define UART_LCR(uart)                    ((uart)+0xc)
#define UART_MCR(uart)                    ((uart)+0x10)
#define UART_LSR(uart)                    ((uart)+0x14)
#define UART_MSR(uart)                    ((uart)+0x18)
#define UART_SCR(uart)                    ((uart)+0x1c)
#define UART_DLL(uart)                    ((uart)+0x0)  /* Only when LCR.DLAB = 1 */
#define UART_DLH(uart)                    ((uart)+0x4)  /* Only when LCR.DLAB = 1 */
#define UART_EFR(uart)                    ((uart)+0x8)  /* Only when LCR = 0xbf */
#define UART_XON1(uart)                   ((uart)+0x10) /* Only when LCR = 0xbf */
#define UART_XON2(uart)                   ((uart)+0x14) /* Only when LCR = 0xbf */
#define UART_XOFF1(uart)                  ((uart)+0x18) /* Only when LCR = 0xbf */
#define UART_XOFF2(uart)                  ((uart)+0x1c) /* Only when LCR = 0xbf */
#define UART_AUTOBAUD_EN(uart)            ((uart)+0x20)
#define UART_HIGHSPEED(uart)              ((uart)+0x24)
#define UART_SAMPLE_COUNT(uart)           ((uart)+0x28)
#define UART_SAMPLE_POINT(uart)           ((uart)+0x2c)
#define UART_AUTOBAUD_REG(uart)           ((uart)+0x30)
#define UART_RATE_FIX_AD(uart)            ((uart)+0x34)
#define UART_AUTOBAUD_SAMPLE(uart)        ((uart)+0x38)
#define UART_GUARD(uart)                  ((uart)+0x3c)
#define UART_ESCAPE_DAT(uart)             ((uart)+0x40)
#define UART_ESCAPE_EN(uart)              ((uart)+0x44)
#define UART_SLEEP_EN(uart)               ((uart)+0x48)
#define UART_VFIFO_EN(uart)               ((uart)+0x4c)
#define UART_RXTRI_AD(uart)               ((uart)+0x50)

/* LSR */
#define UART_LSR_DR                 (1 << 0)
#define UART_LSR_OE                 (1 << 1)
#define UART_LSR_PE                 (1 << 2)
#define UART_LSR_FE                 (1 << 3)
#define UART_LSR_BI                 (1 << 4)
#define UART_LSR_THRE               (1 << 5)
#define UART_LSR_TEMT               (1 << 6)
#define UART_LSR_FIFOERR            (1 << 7)

/* LCR */
#define UART_LCR_BREAK              (1 << 6)
#define UART_LCR_DLAB               (1 << 7)

#define UART_WLS_5                  (0 << 0)
#define UART_WLS_6                  (1 << 0)
#define UART_WLS_7                  (2 << 0)
#define UART_WLS_8                  (3 << 0)
#define UART_WLS_MASK               (3 << 0)

#define UART_1_STOP                 (0 << 2)
#define UART_2_STOP                 (1 << 2)
#define UART_1_5_STOP               (1 << 2)    /* Only when WLS=5 */
#define UART_STOP_MASK              (1 << 2)

#define UART_NONE_PARITY            (0 << 3)
#define UART_ODD_PARITY             (0x1 << 3)
#define UART_EVEN_PARITY            (0x3 << 3)
#define UART_MARK_PARITY            (0x5 << 3)
#define UART_SPACE_PARITY           (0x7 << 3)
#define UART_PARITY_MASK            (0x7 << 3)

/* FCR */
#define UART_FCR_FIFOE              (1 << 0)
#define UART_FCR_CLRR               (1 << 1)
#define UART_FCR_CLRT               (1 << 2)
#define UART_FCR_DMA1               (1 << 3)
#define UART_FCR_RXFIFO_1B_TRI      (0 << 6)
#define UART_FCR_RXFIFO_6B_TRI      (1 << 6)
#define UART_FCR_RXFIFO_12B_TRI     (2 << 6)
#define UART_FCR_RXFIFO_RX_TRI      (3 << 6)
#define UART_FCR_TXFIFO_1B_TRI      (0 << 4)
#define UART_FCR_TXFIFO_4B_TRI      (1 << 4)
#define UART_FCR_TXFIFO_8B_TRI      (2 << 4)
#define UART_FCR_TXFIFO_14B_TRI     (3 << 4)
#define UART_FCR_FIFO_INIT          (UART_FCR_FIFOE|UART_FCR_CLRR|UART_FCR_CLRT)
#define UART_FCR_NORMAL             (UART_FCR_FIFO_INIT|UART_FCR_TXFIFO_4B_TRI|UART_FCR_RXFIFO_12B_TRI)

#define DEBUG_SERIAL_READ_NODATA   (0)
#ifdef USING_APUART_PORT
#define CLK_AP_CG_CTRL_BASE  0xA0001000
#define CLK_AP_CG_CTRL_SET   (CLK_AP_CG_CTRL_BASE + 0x80)
#define CLK_AP_CG_CTRL_CLR   (CLK_AP_CG_CTRL_BASE + 0x84)
#define CLK_AP_CG_CTRL_STA   (CLK_AP_CG_CTRL_BASE + 0x90)
#define CLK_AP_UART0_CLK     (1 << 22)
#endif

#define CLK_CG_CTRL (CLK_CTRL_BASE+0x30)
#ifdef USING_BCLK
#define UART0_CLK ((1<<8)|(1<<7))
#define UART1_CLK ((1<<11)|(1<<10))
#else
#define UART0_CLK ((1<<8)|(1<<7))
#define UART1_CLK ((1<<12)|(1<<11))
#endif
#define UART0_RST (1<<9)
#define UART1_RST (1<<12)


void uart_init(unsigned int g_uart, unsigned int speed, unsigned int uart_clk_changed);
void uart_platform_setting();
void uart_switch_clk(unsigned int uart_clk_changed);

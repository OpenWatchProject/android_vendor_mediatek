/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 */
/* MediaTek Inc. (C) 2017. All rights reserved.
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

#include "main.h"
#include "FreeRTOS.h"
#include <driver_api.h>
#include <mt_uart.h>



/* Uart Clk Setting */
#define UART0_CK_SEL_MSK    0x3
#define UART0_CK_SEL_SHFT   0
#define UART0_CK_STA_MSK    0xf
#define UART0_CK_STA_SHFT   8
#define UART1_CK_SEL_MSK    0x3
#define UART1_CK_SEL_SHFT   16
#define UART1_CK_STA_MSK    0xf
#define UART1_CK_STA_SHFT   24

enum UART_CTRL_BITS {
    UART_26M = 0,
    UART_32K = 1,
    UART_ULPOSC_DIV_26M = 2,
};

void uart_platform_setting(void)
{
#ifdef USING_APUART_PORT
    UART_SET_BITS(CLK_AP_CG_CTRL_CLR, CLK_AP_UART1_CLK);
#else

#ifdef CFG_SCP_UART1_SUPPORT
    /* scpuart1 150/151 */
    DRV_WriteReg32(0xA0005420, (DRV_Reg32(0xA0005420)&0x00FFFFFF)|0x66000000);
    UART_SET_BITS(CLK_CG_CTRL, UART1_CLK | UART1_RST);
#else
    /* scpuart0 GPIO 50-51 not used */
    DRV_WriteReg32(0xA0005360, (DRV_Reg32(0xA0005360)&0xFFFF00FF)|0x00006600);
    UART_SET_BITS(CLK_CG_CTRL, UART0_CLK | UART0_RST); /* enable clock and release reset*/
#endif
#endif

}

/* Uart Bringup use 26M Clock due to ulposc is not ready during chip bring up.
 * And when ulposc is ready, dvfs.c will call this api to switch uart  to
 * use ulposc clock. 26M clock will disapper when system enter sleep but ulposc
 * will not.
 */
void uart_switch_clk(unsigned int uart_clk_changed)
{
#if defined(CFG_UART_SUPPORT)
#ifdef CFG_SCP_UART1_SUPPORT
    /* Set UART_CK_SEL[17:16] = 2'b10: ulposc_div_to_26m for UART1 */
    DRV_ClrReg32(UART_CK_SEL, UART1_CK_SEL_MSK << UART1_CK_SEL_SHFT);
    DRV_SetReg32(UART_CK_SEL, UART_ULPOSC_DIV_26M << UART1_CK_SEL_SHFT);

    uart_init(UART_LOG_PORT, UART_LOG_BAUD_RATE, uart_clk_changed*1000000/10);

    /* Check UART_CK_SEL[27:24] = 4'b0100: ulposc_div_to_26m for UART1 */
    while((DRV_Reg32(UART_CK_SEL) & (UART1_CK_STA_MSK << UART1_CK_STA_SHFT)) !=
        (1 << (UART_ULPOSC_DIV_26M + UART1_CK_STA_SHFT)))
        PRINTF_E("uart src status = 0x%x\n", DRV_Reg32(UART_CK_SEL));
#else
    /* Set UART_CK_SEL[1:0] = 2'b10: ulposc_div_to_26m for UART0 */
    DRV_ClrReg32(UART_CK_SEL, UART0_CK_SEL_MSK << UART0_CK_SEL_SHFT);
    DRV_SetReg32(UART_CK_SEL, UART_ULPOSC_DIV_26M << UART0_CK_SEL_SHFT);

    uart_init(UART_LOG_PORT, UART_LOG_BAUD_RATE, uart_clk_changed*1000000/10);

    /* Check UART_CK_SEL[11:8] = 4'b0100: ulposc_div_to_26m for UART0 */
    while((DRV_Reg32(UART_CK_SEL) & (UART0_CK_STA_MSK << UART0_CK_STA_SHFT)) !=
        (1 << (UART_ULPOSC_DIV_26M + UART0_CK_STA_SHFT)))
        PRINTF_E("uart src status = 0x%x\n", DRV_Reg32(UART_CK_SEL));
#endif
#endif
}

/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 */
/* MediaTek Inc. (C) 2015. All rights reserved.
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
 */
/*
============================================================================================================
------------------------------------------------------------------------------------------------------------
||                        Header Files
------------------------------------------------------------------------------------------------------------
============================================================================================================
*/
#include <FreeRTOS.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <driver_api.h>
#include <mt_reg_base.h>
#include <scp_sem.h>
#ifdef CFG_PMIC_WRAP_SUPPORT
#include "pmic_wrap.h"
#endif
#include "vow_hw.h"
#include "audio_task_interface.h"
#ifdef CFG_VCORE_DVFS_SUPPORT
#include <dvfs.h>
#endif
/*
============================================================================================================
------------------------------------------------------------------------------------------------------------
||                        Global Variables
------------------------------------------------------------------------------------------------------------
============================================================================================================
*/
#if PMIC_ADD_DELAY
#ifdef CFG_VCORE_DVFS_SUPPORT
extern freq_enum get_cur_clk(void);
#endif
// extern volatile unsigned int *DWT_CYCCNT;
volatile unsigned int loop;
#endif

/*
============================================================================================================
------------------------------------------------------------------------------------------------------------
||                        PMIC WRAPPER ACCESS
------------------------------------------------------------------------------------------------------------
============================================================================================================
*/
#if PMIC_ADD_DELAY
void vow_busy_wait(void)
{
    unsigned int delay_cnt = 0;

    // busy wait about 2usec to let PMIC sleep

#ifdef CFG_VCORE_DVFS_SUPPORT
    // about 11 cycle cnt Per loop cnt
    switch (get_cur_clk()) {
        case FREQ_82MHZ:  // E1:71MHz, E2:82MHz
            // AUD_LOG_D("71MHz\n");
            // delay_cnt = 11;  // cycle cnt 141
            delay_cnt = 13;  // cycle cnt 163
            break;
        case FREQ_250MHZ:
            // AUD_LOG_D("286MHz\n");
            delay_cnt = 46;  // cycle cnt 570
            break;
        case FREQ_328MHZ:
            // AUD_LOG_D("330MHz\n");
            delay_cnt = 58;  // cycle cnt 658
            break;
        default:
            // AUD_LOG_D("Others\n");
            delay_cnt = 58;  // cycle cnt 658
            break;
    }
    // get_cur_clk() will cost about 33 cycles
    delay_cnt -= 3;
#endif
    loop = 0;
    while (loop < delay_cnt) {
        loop++;
    }
}
#endif

unsigned short vow_pwrapper_read(unsigned short addr)
{
    unsigned int rdata = 0;
    // unsigned int start_time, end_time;
#ifdef CFG_PMIC_WRAP_SUPPORT
    pwrap_scp_read((unsigned int)addr, (unsigned int *)&rdata);
#endif
#if PMIC_ADD_DELAY
    // start_time = *DWT_CYCCNT;
    vow_busy_wait();
    // end_time = *DWT_CYCCNT;
    // AUD_LOG_D("R:%d, %d, %d\n", end_time - start_time, start_time, end_time);
#endif
    return (unsigned short)rdata;
}

void vow_pwrapper_write(unsigned short addr, unsigned short data)
{
#ifdef CFG_PMIC_WRAP_SUPPORT
    pwrap_scp_write((unsigned int)addr, (unsigned int)data);
#endif
    // vow_pwrapper_read(addr);  // For sync bus data
}

unsigned short vow_pwrapper_read_bits(unsigned short addr, char bits, char len)
{
    unsigned short TargetBitField = ((0x1 << len) - 1) << bits;
    unsigned short CurrValue = vow_pwrapper_read(addr);
    return ((CurrValue & TargetBitField) >> bits);
}

void vow_pwrapper_write_bits(unsigned short addr, unsigned short value, char bits, char len)
{
    unsigned short TargetBitField = ((0x1 << len) - 1) << bits;
    unsigned short TargetValue = (value << bits) & TargetBitField;
    unsigned short CurrValue;
    CurrValue = vow_pwrapper_read(addr);
    vow_pwrapper_write(addr, (short)((CurrValue & (~TargetBitField)) | TargetValue));

    return;
}


/*
============================================================================================================
------------------------------------------------------------------------------------------------------------
||                        PMIC State Control Functions
------------------------------------------------------------------------------------------------------------
============================================================================================================
*/

/***********************************************************************************
** vow_pmic_idleTophase1 - set pmic from idle to phase1 mode
************************************************************************************/
void vow_pmic_idleTophase1(vowserv_t *vow)
{
    AUD_LOG_D("IdleToP1\n");

#if SMART_AAD_MIC_SUPPORT
    if (vow->smart_device_flag == true) {
        // Do not enable PMIC VOW phase 1 because smart device handle it.
        vow_pwrapper_write_bits(AFE_VOW_CFG4, 0x4e, 0, 9);  // vow txif sck divider = 4
        return;
    }
#endif

    // VOW phase1 enable
    // Power down VOW source clock: 0=normal, 1=power down
    vow_pwrapper_write_bits(AFE_VOW_TOP_CON0, 0, 15, 1);

    // vow_1p6m_800k_sel: 1.6MHz
    //vow_pwrapper_write_bits(AFE_VOW_TOP, 1, 14, 1);

    // VOW interrupt source select: 1=no bias irq source, 0=bias base irq source
    // vow_pwrapper_write_bits(AFE_VOW_TOP, 1, 4, 1);
    vow_pwrapper_write_bits(AFE_VOW_TOP_CON1, 1, 4, 1); // channel1

    // Power on VOW: 1=enable VOW
    //vow_pwrapper_write_bits(AFE_VOW_TOP, 1, 11, 1);
    vow_pwrapper_write_bits(AFE_VOW_TOP_CON1, 1, 0, 1); // channel1

    if (vow->channel == VOW_STEREO) {
        vow_pwrapper_write_bits(AFE_VOW_TOP_CON2, 1, 4, 1); // channel2
        vow_pwrapper_write_bits(AFE_VOW_TOP_CON2, 1, 0, 1); // channel2
    }
}

/***********************************************************************************
** vow_pmic_phase2Tophase1 - set pmic from phase2 to phase1 mode
************************************************************************************/
void vow_pmic_phase2Tophase1(vowserv_t *vow)
{
    unsigned int rdata;
    AUD_LOG_D("P2toP1\n");
    // Disable VOW Power
    //vow_pwrapper_write_bits(AFE_VOW_TOP, 0, 11, 1);
    vow_pwrapper_write_bits(AFE_VOW_TOP_CON1, 0, 0, 1); // channel1

    // Clear Interrupt flag
    //vow_pwrapper_write_bits(AFE_VOW_TOP, 1, 3, 1);
    vow_pwrapper_write_bits(AFE_VOW_TOP_CON1, 1, 3, 1); // channel1

    if (vow->channel == VOW_STEREO) {
        vow_pwrapper_write_bits(AFE_VOW_TOP_CON2, 0, 0, 1); // channel2
        vow_pwrapper_write_bits(AFE_VOW_TOP_CON2, 1, 3, 1); // channel2
    }

    // VOW disable FIFO
    rdata = DRV_Reg32(VOW_FIFO_EN);
    rdata &= (~0x3);
    DRV_WriteReg32(VOW_FIFO_EN, rdata);

    // VOW enable FIFO
    rdata = DRV_Reg32(VOW_FIFO_EN);
    rdata |= 0x3;
    DRV_WriteReg32(VOW_FIFO_EN, rdata);

    // Bit[5] vow_intr_sw_mode = 0; Bit[4] vow_intr_sw_val = 0
    //vow_pwrapper_write_bits(AFE_VOW_POSDIV_CFG0, 0, 4, 2);
    vow_pwrapper_write_bits(AFE_VOW_TOP_CON0, 0, 6, 2);

    // S,N value reset: 1=reset to 'h64
    //vow_pwrapper_write_bits(AFE_VOW_TOP, 1, 2, 1);
    vow_pwrapper_write_bits(AFE_VOW_TOP_CON1, 1, 2, 1); // channel1

    // S,N value reset: 0=keep last N
    //vow_pwrapper_write_bits(AFE_VOW_TOP, 0, 2, 1);
    vow_pwrapper_write_bits(AFE_VOW_TOP_CON1, 0, 2, 1); // channel1

    if (vow->channel == VOW_STEREO) {
        vow_pwrapper_write_bits(AFE_VOW_TOP_CON2, 1, 2, 1); // channel2
        vow_pwrapper_write_bits(AFE_VOW_TOP_CON2, 0, 2, 1); // channel2
    }

#if SMART_AAD_MIC_SUPPORT
    if (vow->smart_device_flag == true) {
        vow_pwrapper_write_bits(AUDENC_ANA_CON9, 0,  0, 3);  // DMIC enable.BIT0~2
        vow_pwrapper_write_bits(AUDENC_ANA_CON15, 0, 15, 1);  // PLL low power off.BIT:15
        vow_pwrapper_write_bits(AUDENC_ANA_CON14, 0,  2, 2);  // Set DCKO = 1/4 F_PLL off
        vow_pwrapper_write_bits(AUDENC_ANA_CON14, 0, 10, 1);  // Disable fbdiv relatch (low jitter).BIT:10
        vow_pwrapper_write_bits(AUDENC_ANA_CON14, 0,  0, 1);
        vow_pwrapper_write_bits(AFE_VOW_TOP,      0, 14, 1);
        vow_pwrapper_write_bits(AFE_VOW_TOP,      0,  4, 1);
        vow_pwrapper_write_bits(AFE_VOW_TOP,      1, 15, 1);
        return;
    }
#endif

    // Reset Interrupt flag
    //vow_pwrapper_write_bits(AFE_VOW_TOP, 0, 3, 1);
    vow_pwrapper_write_bits(AFE_VOW_TOP_CON1, 0, 3, 1); // channel1

    // Power on VOW: 1=enable VOW
    //vow_pwrapper_write_bits(AFE_VOW_TOP, 1, 11, 1);
    vow_pwrapper_write_bits(AFE_VOW_TOP_CON1, 1, 0, 1); // channel1

    if (vow->channel == VOW_STEREO) {
        vow_pwrapper_write_bits(AFE_VOW_TOP_CON2, 0, 3, 1); // channel2
        vow_pwrapper_write_bits(AFE_VOW_TOP_CON2, 1, 0, 1); // channel2
    }
}

/***********************************************************************************
** vow_force_phase2 - set pmic to phase2 mode by SW
************************************************************************************/
void vow_force_phase2(vowserv_t *vow)
{
    AUD_LOG_D("ForceP2\n");

    // Bit[5] vow_intr_sw_mode = 1; Bit[4] vow_intr_sw_val = 1
    //vow_pwrapper_write_bits(AFE_VOW_POSDIV_CFG0, 3, 4, 2);
    vow_pwrapper_write_bits(AFE_VOW_TOP_CON0, 3, 6, 2);

#if SMART_AAD_MIC_SUPPORT
    if (vow->smart_device_flag == true) {
        vow_pwrapper_write_bits(AUDENC_ANA_CON14, 2,  2, 2);   // Set DCKO = 1/4 F_PLL. bit:2,3
        vow_pwrapper_write_bits(AUDENC_ANA_CON14, 1, 10, 1);   // Enable fbdiv relatch (low jitter). bit:10
        vow_pwrapper_write_bits(AUDENC_ANA_CON14, 1,  0, 1);   // Enable VOWPLL CLK. bit:0
        vow_pwrapper_write_bits(AUDENC_ANA_CON16, 2,  3, 3);   // PLL VCOBAND. bit5:3
        // PLL devider ratio. bit9:4; 392KHz(0x2F)->376KHz(0x2D)
        vow_pwrapper_write_bits(AUDENC_ANA_CON14, 0x2D, 4, 6);
        vow_pwrapper_write_bits(AUDENC_ANA_CON15, 1, 15, 1);   // PLL low power. bit:15
        vow_pwrapper_write_bits(AUDENC_ANA_CON9,  5,  0, 3);   // DMIC enable.BIT0~2
        return;
    }
#endif
}

/***********************************************************************************
** vow_clear_force_phase2 - set pmic phase control by HW
************************************************************************************/
void vow_clear_force_phase2(void)
{
    AUD_LOG_D("Clear ForceP2\n");
    // Bit[5] vow_intr_sw_mode = 0; Bit[4] vow_intr_sw_val = 0
    //vow_pwrapper_write_bits(AFE_VOW_POSDIV_CFG0, 0, 4, 2);
    vow_pwrapper_write_bits(AFE_VOW_TOP_CON0, 0, 6, 2);
}

/***********************************************************************************
** vow_force_phase1 - set pmic to phase1 mode by SW
************************************************************************************/
void vow_force_phase1(void)
{
    AUD_LOG_D("ForceP1\n");
    // Bit[5] vow_intr_sw_mode = 1; Bit[4] vow_intr_sw_val = 0
    //vow_pwrapper_write_bits(AFE_VOW_POSDIV_CFG0, 2, 4, 2);
    vow_pwrapper_write_bits(AFE_VOW_TOP_CON0, 2, 6, 2);

    vow_pwrapper_write_bits(TOP_CKSEL_CON1_SET, 1, 9, 1);
}

/***********************************************************************************
** vow_pmic_idle - set pmic to idle status
************************************************************************************/
void vow_pmic_idle(vowserv_t *vow)
{
    AUD_LOG_D("ToIdle\n");
    if (1 == vow_pwrapper_read_bits(TOP_CKSEL_CON1, 9, 1)) {
        vow_pwrapper_write_bits(TOP_CKSEL_CON1_CLR, 1, 9, 1);
    }

#if SMART_AAD_MIC_SUPPORT
    if (vow->smart_device_flag == true) {
       vow_pwrapper_write_bits(AUDENC_ANA_CON9,  0,  0, 3);  // DMIC enable.BIT0~2
       vow_pwrapper_write_bits(AUDENC_ANA_CON15, 0, 15, 1);  // PLL low power off.BIT:15
       vow_pwrapper_write_bits(AUDENC_ANA_CON14, 0,  2, 2);  // Set DCKO = 1/4 F_PLL off
       vow_pwrapper_write_bits(AUDENC_ANA_CON14, 0, 10, 1);  // disable fbdiv relatch (low jitter).BIT:10
       vow_pwrapper_write_bits(AUDENC_ANA_CON14, 0,  0, 1);  // disable VOWPLL CLK. Bit0
       vow_pwrapper_write_bits(AFE_VOW_TOP, 0, 13, 1);  // VOW DMIC Off
       vow_pwrapper_write_bits(AFE_VOW_TOP, 0,  6, 2);  // bit 7: DMIC clock(800K/1.6m), bit6: select SDM 3-level mode
    }
#endif
    // VOW phase1 disable and intr clear
    // Power on VOW: 0=disable VOW
    //vow_pwrapper_write_bits(AFE_VOW_TOP, 0, 11, 1);
    vow_pwrapper_write_bits(AFE_VOW_TOP_CON1, 0, 0, 1); // channel1

    // vow_1p6m_800k_sel: 800kHz (set back to default)
    //vow_pwrapper_write_bits(AFE_VOW_TOP, 0, 14, 1);

    // VOW interrupt source select: 1=no bias irq source, 0=bias base irq source
    //vow_pwrapper_write_bits(AFE_VOW_TOP, 0, 4, 1);
    vow_pwrapper_write_bits(AFE_VOW_TOP_CON1, 0, 4, 1); // channel1

    if (vow->channel == VOW_STEREO) {
        vow_pwrapper_write_bits(AFE_VOW_TOP_CON2, 0, 0, 1); // channel2
        vow_pwrapper_write_bits(AFE_VOW_TOP_CON2, 0, 4, 1); // channel2
    }

    // Power down VOW source clock: 0=normal, 1=power down
    vow_pwrapper_write_bits(AFE_VOW_TOP_CON0, 1, 15, 1);
}

/***********************************************************************************
** vow_dc_removal - set dc removal function
************************************************************************************/
void vow_dc_removal(char enable, char order)
{
    short data;

    AUD_LOG_V("DC_remove, en:%d,order:%d\n", enable, order);
    if (enable == 0) {
        data = 0x0056;
    } else {
        data = 0x0005 | ((order & 0x0f) << 4);
    }
    vow_pwrapper_write(AFE_VOW_HPF_CFG0, data); // channel1
    vow_pwrapper_write_bits(AFE_VOW_VAD_CFG4, 1, 15, 1); // channel1

    //if (vow->channel == VOW_STEREO) {
    vow_pwrapper_write(AFE_VOW_HPF_CFG1, data); // channel2
    vow_pwrapper_write_bits(AFE_VOW_VAD_CFG5, 1, 15, 1); // channel2
    //}
}


/*
============================================================================================================
------------------------------------------------------------------------------------------------------------
||                        VOW Control Functions
------------------------------------------------------------------------------------------------------------
============================================================================================================
*/

/***********************************************************************************
** vow_enable_hw - enable VOW FIFO control
************************************************************************************/
void vow_enable_fifo(vowserv_t *vow)
{
    unsigned int Temp;

    AUD_LOG_D("enable_fifo, SR:%d, mtkif:%d, channel:%d\n", vow->samplerate,
                                                            vow->mtkif_type,
                                                            vow->channel);
    // Enable VOW FIFO
    DRV_WriteReg32(VOW_FIFO_EN, 0x0);
    Temp = 0x00000000;
    if (vow->channel == VOW_STEREO) {
        Temp |= 0x00030603;
    } else {
        Temp |= 0x27130703;
    }
    DRV_WriteReg32(VOW_RXIF_CFG0, Temp);
    DRV_WriteReg32(VOW_RXIF_CFG1, 0x33180014);
    DRV_WriteReg32(VOW_FIFO_EN,   0x00000003);
    DRV_WriteReg32(VOW_FIFO_DATA_THRES, (VOW_SAMPLE_NUM_IRQ + 1));
}

/***********************************************************************************
** vow_disable_fifo - disable VOW FIFO control
************************************************************************************/
void vow_disable_fifo(void)
{
    AUD_LOG_D("disable_fifo\n");
    DRV_WriteReg32(VOW_FIFO_EN, 0x0);
}

/***********************************************************************************
** vow_read_fifo_data - read fifo data in irq
************************************************************************************/
void vow_read_fifo_data(short *voice_buf_L, short *voice_buf_R, unsigned short read_length, vowserv_t *vow)
{
    int i;

    if (voice_buf_L == NULL) {
        int data;
        for (i = 0; i < read_length; i++) {
            data = DRV_Reg32(VOW_FIFO_DATA);
        }
        (void)data;
    } else {
        int rdata;

        for (i = 0; i < read_length; i++) {
            rdata = DRV_Reg32(VOW_FIFO_DATA);
            *voice_buf_L = (short)(rdata >> 16);
            voice_buf_L++;
            if (vow->channel == VOW_STEREO) {
                if (voice_buf_R != NULL) {
                    *voice_buf_R = (short)rdata;
                    voice_buf_R++;
                }
            }
        }
    }
}

/***********************************************************************************
** vow_reset_fifo_data - reset fifo data in irq
************************************************************************************/
void vow_reset_fifo_irq(void)
{
    int data = DRV_Reg32(VOW_FIFO_IRQ_ACK);
    (void)data;
}

/***********************************************************************************
** vow_get_smartdevice_eint - do in Smart device EINT handler
************************************************************************************/
#if SMART_AAD_MIC_SUPPORT
void vow_smartdevice_eint_handler(bool smart_device_flag)
{
    AUD_LOG_V("GET EINT\n\r");
    if (smart_device_flag == false) {
        return;
    }

    /* Enable phase 2 hardware. ex: PLL*/
    vow_pwrapper_write_bits(AUDENC_ANA_CON14, 2,  2, 2);    // Set DCKO = 1/4 F_PLL. bit:2,3
    vow_pwrapper_write_bits(AUDENC_ANA_CON14, 1, 10, 1);    // Enable fbdiv relatch (low jitter). bit:10
    vow_pwrapper_write_bits(AUDENC_ANA_CON14, 1,  0, 1);    // Enable VOWPLL CLK. bit:0
    vow_pwrapper_write_bits(AUDENC_ANA_CON16, 2,  3, 3);    // PLL VCOBAND. bit5:3, 0->2
    vow_pwrapper_write_bits(AUDENC_ANA_CON14, 0x2D, 4, 6);  // PLL devider ratio. bit9:4; 392KHz(0x2F)->376KHz(0x2D)

    vow_pwrapper_write_bits(AUDENC_ANA_CON15, 1, 15, 1);    // PLL low power. bit:15
    vow_pwrapper_write_bits(AUDENC_ANA_CON9, 5,  0, 3);    // DMIC enable.BIT0~2

    vow_pwrapper_write(AFE_VOW_POSDIV_CFG0, 0x0C3A);  // For 770KHz clock

    vow_pwrapper_write_bits(AFE_VOW_TOP, 1, 13, 1);   // VOW DMIC on
    vow_pwrapper_write_bits(AFE_VOW_TOP, 3,  6, 2);   // bit 7: DMIC clock(800K/1.6m), bit6: select SDM 3-level mode

    vow_pwrapper_write_bits(AFE_VOW_TOP, 0, 15, 1);
    vow_pwrapper_write_bits(AFE_VOW_TOP, 1, 14, 1);
    vow_pwrapper_write_bits(AFE_VOW_TOP, 1,  4, 1);
    vow_pwrapper_write_bits(AFE_VOW_TOP, 1, 11, 1);
}
#endif

/***********************************************************************************
** vow_pmic_low_power_control - 6358 PMIC low power settings
************************************************************************************/
void vow_pmic_low_power_control(vowserv_t *vow, vow_pmic_low_power_t lp_control)
{
#ifdef SUPPORT_PMIC_VOW_LP_MODE
    switch (lp_control) {
        case VOW_PMIC_LOW_POWER_INIT:
            AUD_LOG_V("==VOW_PMIC_LOW_POWER_INIT==\n");
            /* vow set vowen sw value to 0*/
            vow_pwrapper_write_bits(TOP_CKSEL_CON1_CLR, 1, 1, 1);
            break;
        case VOW_PMIC_LOW_POWER_ENTRY:
            AUD_LOG_V("==VOW_PMIC_LOW_POWER_ENTRY==\n");
            /* set VOWEN to SW mode */
            vow_pwrapper_write_bits(TOP_CKSEL_CON1_SET, 1, 9, 1);
            vow_pwrapper_write_bits(AFE_VOW_POSDIV_CFG0, 3, 4, 2);
            /* call freq_api_here */
            request_pmic_lp_mode_for_vow(SCP_ENABLE);
            break;
        case VOW_PMIC_LOW_POWER_EXIT:
            AUD_LOG_V("==VOW_PMIC_LOW_POWER_EXIT==\n");
            /* call fred_api_here */
            request_pmic_lp_mode_for_vow(SCP_DISABLE);
            /* set VOWEN to HW mode */
            vow_pwrapper_write_bits(TOP_CKSEL_CON1_CLR, 1, 9, 1);
            vow_pwrapper_write_bits(AFE_VOW_POSDIV_CFG0, 0, 4, 2);
            /* reset first irq flag when P2 to P1 */
            break;
        default:
            AUD_LOG_V("VOW PMIC low power setting error.\n\r");
            break;
    }
#endif  // #if SUPPORT_PMIC_VOW_LP_MODE
}

int32_t is_normal_src_additional_frame_accumulated(uint32_t *p_frm_cnt)
{
    // Alway return 0, beacuse MT6359 DOES NOT need SRC
    return 0;
}

#ifdef CFG_MTK_VOW_BARGE_IN_SUPPORT
int32_t is_accurate_src_additional_frame_accumulated(uint32_t *p_frm_cnt)
{
    // Assumption: 160 samples per frame for 1625/102kHz & increase exactly 1 frame every time, start from frame count "1"
    // formula: remainder sample per frame x = (160 * 1632 / 1625) - 160
    // formula: How many input frame can output 160 samples which have been SRCed already? => 160 / x = (160 * (1625 / 1120))
    // formula: (1625 * 160 / 1120) frame can output 1 frame (160 samples)
    // formula: 1625 frame can output 7 frames
    uint32_t frm_cnt = *p_frm_cnt;  // frame count can only be 1 ~ 1625
    int32_t result;
    if (frm_cnt == 1625) {
        result = 1;
        *p_frm_cnt = 0;
    } else if (frm_cnt % 233 == 0) {
        result = 1;
    } else {
        result = 0;
    }
    return result;
}
#endif // #ifdef CFG_MTK_VOW_BARGE_IN_SUPPORT

#ifdef CFG_MTK_VOW_DUAL_MIC_SUPPORT
void vow_headset_detection(vowserv_t *vow)
{
    bool is_amic;
    bool is_pga_l_ain1;
    unsigned short rdata1, rdata2;
    // if detect pmic setting: both amic and pga_L_Ain1, determine headset
    rdata1 = vow_pwrapper_read(AFE_VOW_TOP_CON0);
    rdata2 = vow_pwrapper_read(AUDENC_ANA_CON0);
    is_amic = vow_pwrapper_read_bits(AFE_VOW_TOP_CON0, 9, 1) == 0 ? true : false;
    is_pga_l_ain1 = vow_pwrapper_read_bits(AUDENC_ANA_CON0, 6, 2) == 2 ? true : false;

    if (is_amic && is_pga_l_ain1) {
        AUD_LOG_D("PMIC RG determine headset. 0x%x 0x%x\n\r", rdata1, rdata2);
        vow->is_headset = true;
    } else {
        AUD_LOG_D("PMIC RG determine handset. 0x%x 0x%x\n\r", rdata1, rdata2);
        vow->is_headset = false;
    }
}
#endif // #ifdef #ifdef CFG_MTK_VOW_DUAL_MIC_SUPPORT

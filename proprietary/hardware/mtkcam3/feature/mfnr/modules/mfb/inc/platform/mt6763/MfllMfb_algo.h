/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 *
 * MediaTek Inc. (C) 2017. All rights reserved.
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
 * have been modified by MediaTek Inc. All revisions are subject to any receiver's
 * applicable license agreements with MediaTek Inc.
 */
#ifndef __MFLLMFB_ALGO_H__
#define __MFLLMFB_ALGO_H__

#ifndef LOG_TAG
#define LOG_TAG "MfllCore/Mfb"
#include <mtkcam3/feature/mfnr/MfllLog.h>
#endif

// platform dependent headers
#include <drv/isp_reg.h> // dip_x_reg_t

// AOSP
#include <cutils/compiler.h>

// STL
#include <math.h>

// define this macro to 0 to disable refine function for avoiding compile errors
#define MFLL_COMPILE_POST_NR_REFINE     1

#define RF_CLIP(a,b,c)   (( (a) < (b) )?  (b) : (( (a) > (c) )? (c):(a) ))

namespace mfll
{

inline int reg_div_gain(
    int val,
    float gain,
    int min,
    int max
	)
{
	int ret = val >= 0 ? (int)((float(val) / gain) + 0.5) : (int)((float(val) / gain) - 0.5);
	ret = RF_CLIP(ret, min, max);
	return ret;
}

inline int reg_mul_gain(
    int val,
    float gain,
    int min,
    int max
	)
{
	int ret = val >= 0 ? (int)((float(val)*gain) + 0.5) : (int)((float(val)*gain) - 0.5);
	ret = RF_CLIP(ret, min, max);
	return ret;
}

inline int reg_gain_max(
    int val,
    int min,
    int max,
    float gain
	)
{
	float fDelta = float(max - val) / gain;
	int ret = (fDelta >= 0) ? max - (int)(fDelta + 0.5) : max - (int)(fDelta - 0.5);
	ret = RF_CLIP(ret, min, max);
	return ret;
}

inline int mix_m1_refine(int m1,int m0, int curFrames, int expFrames)
{
    int m1_out = 36;
    if (expFrames >= 2) {
        float fDelta = float(m1-m0)*float(curFrames-1)/float(expFrames-1);
        int i4Delta;
        if (fDelta>=0) { i4Delta=(int)(fDelta+0.5); }
        else { i4Delta=(int)(fDelta-0.5); }
        if (i4Delta<=0) i4Delta=0;
        m1_out = m0+i4Delta;
        if (m1_out<=36) m1_out=36;
    }

    return m1_out;
}

// PostNR refine function implementation.
//  @param [in/out] regs            Registers structure, see drv/isp_reg.h
//  @param [in]     real_cnt        Real blending count. If takes 6 frames, assumes
//                                  all frames are ok for blending, this value will
//                                  be 5 (5 times of blending).
//  @param [in]     expected_cnt    Expected blending count. If takes 6 frames,
//                                  this value will be 5 (5 times to blend).
//  @note                           Notice that, arguments real_cnt and expected_cnt
//                                  will be (CaptureFrameNum - 1).
inline void bss_refine_postnr_regs(
        void*   regs,
        int     real_cnt,
        int     expected_cnt
        )
{
    dip_x_reg_t* pRegs = static_cast<dip_x_reg_t*>(regs);

#if MFLL_COMPILE_POST_NR_REFINE
    mfllLogD("%s: blend frames count, real=%d, expected=%d",
            __FUNCTION__, real_cnt, expected_cnt);

    if (CC_UNLIKELY( real_cnt < 0 )) {
        mfllLogW("%s: read blend count < 0, ignore refine", __FUNCTION__);
        return;
    }

    if (CC_UNLIKELY( expected_cnt < 0 )) {
        mfllLogW("%s: expected blending count is < 0, ignore refine", __FUNCTION__);
        return;
    }

    mfllLogD("%s: refine postNR", __FUNCTION__);

    // increase real_cnt and expected_cnt as the domain of CaptureFrameNum
    real_cnt++;
    expected_cnt++;


    // ------------------------------------------------------------------------
    // implement refine function here
    // ------------------------------------------------------------------------

    float fSTDGain1 = (float)pow(double(expected_cnt)/double(real_cnt), 0.5);
    float fSTDGain2 = (float)pow(double(expected_cnt)/double(real_cnt), 0.33);

    int mix_m1,mix_m0,mix_dt;

    float f_mix_dt;

    if (real_cnt!=expected_cnt) {

        pRegs->DIP_X_ANR_ACTYL2.Bits.ANR_LUMA_BLD_BASE_Y_L = reg_div_gain(pRegs->DIP_X_ANR_ACTYL2.Bits.ANR_LUMA_BLD_BASE_Y_L, fSTDGain2, 0, 64);
        pRegs->DIP_X_ANR_ACTYHL2.Bits.ANR_LUMA_BLD_BASE_Y_HL = reg_div_gain(pRegs->DIP_X_ANR_ACTYHL2.Bits.ANR_LUMA_BLD_BASE_Y_HL,fSTDGain2, 0, 64);
        pRegs->DIP_X_ANR_ACTYHH2.Bits.ANR_LUMA_BLD_BASE_Y_HH = reg_div_gain(pRegs->DIP_X_ANR_ACTYHH2.Bits.ANR_LUMA_BLD_BASE_Y_HH,fSTDGain2, 0, 64);
        pRegs->DIP_X_ANR_ACTYL2.Bits.ANR_LUMA_BLD_TH_Y_L = reg_div_gain(pRegs->DIP_X_ANR_ACTYL2.Bits.ANR_LUMA_BLD_TH_Y_L,fSTDGain2, 0, 64);
        pRegs->DIP_X_ANR_ACTYHL2.Bits.ANR_LUMA_BLD_TH_Y_HL = reg_div_gain(pRegs->DIP_X_ANR_ACTYHL2.Bits.ANR_LUMA_BLD_TH_Y_HL,fSTDGain2, 0, 64);
        pRegs->DIP_X_ANR_ACTYHH2.Bits.ANR_LUMA_BLD_TH_Y_HH = reg_div_gain(pRegs->DIP_X_ANR_ACTYHH2.Bits.ANR_LUMA_BLD_TH_Y_HH,fSTDGain2, 0, 64);

        pRegs->DIP_X_ANR_ACTYL.Bits.ANR_ACT_BLD_BASE_Y_L     = reg_div_gain(pRegs->DIP_X_ANR_ACTYL.Bits.ANR_ACT_BLD_BASE_Y_L,fSTDGain2,0,64);
        pRegs->DIP_X_ANR_ACTYHL.Bits.ANR_ACT_BLD_BASE_Y_HL    = reg_div_gain(pRegs->DIP_X_ANR_ACTYHL.Bits.ANR_ACT_BLD_BASE_Y_HL,fSTDGain2,0,64);
        pRegs->DIP_X_ANR_ACTYHH.Bits.ANR_ACT_BLD_BASE_Y_HH    = reg_div_gain(pRegs->DIP_X_ANR_ACTYHH.Bits.ANR_ACT_BLD_BASE_Y_HH,fSTDGain2,0,64);
        pRegs->DIP_X_ANR_ACTYL.Bits.ANR_ACT_BLD_TH_Y_L       = reg_div_gain(pRegs->DIP_X_ANR_ACTYL.Bits.ANR_ACT_BLD_TH_Y_L,fSTDGain2,0,64);
        pRegs->DIP_X_ANR_ACTYHL.Bits.ANR_ACT_BLD_TH_Y_HL     = reg_div_gain(pRegs->DIP_X_ANR_ACTYHL.Bits.ANR_ACT_BLD_TH_Y_HL,fSTDGain2,0,64);
        pRegs->DIP_X_ANR_ACTYHH.Bits.ANR_ACT_BLD_TH_Y_HH     = reg_div_gain(pRegs->DIP_X_ANR_ACTYHH.Bits.ANR_ACT_BLD_TH_Y_HH,fSTDGain2,0,64);

        pRegs->DIP_X_ANR_PTY.Bits.ANR_PTY1 = reg_mul_gain(pRegs->DIP_X_ANR_PTY.Bits.ANR_PTY1,fSTDGain1,0,255);
        pRegs->DIP_X_ANR_PTY.Bits.ANR_PTY2 = reg_mul_gain(pRegs->DIP_X_ANR_PTY.Bits.ANR_PTY2,fSTDGain1,0,255);
        pRegs->DIP_X_ANR_PTY.Bits.ANR_PTY3 = reg_mul_gain(pRegs->DIP_X_ANR_PTY.Bits.ANR_PTY3,fSTDGain1,0,255);
        pRegs->DIP_X_ANR_PTY.Bits.ANR_PTY4 = reg_mul_gain(pRegs->DIP_X_ANR_PTY.Bits.ANR_PTY4,fSTDGain1,0,255);

        pRegs->DIP_X_ANR_PTYL.Bits.ANR_PTY1_LPF = reg_mul_gain(pRegs->DIP_X_ANR_PTYL.Bits.ANR_PTY1_LPF,fSTDGain1,0,255);
        pRegs->DIP_X_ANR_PTYL.Bits.ANR_PTY2_LPF = reg_mul_gain(pRegs->DIP_X_ANR_PTYL.Bits.ANR_PTY2_LPF,fSTDGain1,0,255);
        pRegs->DIP_X_ANR_PTYL.Bits.ANR_PTY3_LPF = reg_mul_gain(pRegs->DIP_X_ANR_PTYL.Bits.ANR_PTY3_LPF,fSTDGain1,0,255);
        pRegs->DIP_X_ANR_PTYL.Bits.ANR_PTY4_LPF = reg_mul_gain(pRegs->DIP_X_ANR_PTYL.Bits.ANR_PTY4_LPF,fSTDGain1,0,255);

        pRegs->DIP_X_ANR_PTC.Bits.ANR_PTC1 = reg_mul_gain(pRegs->DIP_X_ANR_PTC.Bits.ANR_PTC1,fSTDGain1,0,255);
        pRegs->DIP_X_ANR_PTC.Bits.ANR_PTC2 = reg_mul_gain(pRegs->DIP_X_ANR_PTC.Bits.ANR_PTC2,fSTDGain1,0,255);
        pRegs->DIP_X_ANR_PTC.Bits.ANR_PTC3 = reg_mul_gain(pRegs->DIP_X_ANR_PTC.Bits.ANR_PTC3,fSTDGain1,0,255);
        pRegs->DIP_X_ANR_PTC.Bits.ANR_PTC4 = reg_mul_gain(pRegs->DIP_X_ANR_PTC.Bits.ANR_PTC4,fSTDGain1,0,255);

        pRegs->DIP_X_ANR_YAD1.Bits.ANR_CEN_GAIN_LO_TH        = reg_gain_max(pRegs->DIP_X_ANR_YAD1.Bits.ANR_CEN_GAIN_LO_TH,0 ,16,fSTDGain2);
        pRegs->DIP_X_ANR_YAD1.Bits.ANR_CEN_GAIN_HI_TH        = reg_gain_max(pRegs->DIP_X_ANR_YAD1.Bits.ANR_CEN_GAIN_HI_TH,0 ,16,fSTDGain2);
        pRegs->DIP_X_ANR_YLAD.Bits.ANR_CEN_GAIN_LO_TH_LPF    = reg_gain_max(pRegs->DIP_X_ANR_YLAD.Bits.ANR_CEN_GAIN_LO_TH_LPF,0 ,16,fSTDGain2);
        pRegs->DIP_X_ANR_YLAD.Bits.ANR_CEN_GAIN_HI_TH_LPF    = reg_gain_max(pRegs->DIP_X_ANR_YLAD.Bits.ANR_CEN_GAIN_HI_TH_LPF,0 ,16,fSTDGain2);

        mix_m1 = mix_m1_refine(pRegs->DIP_X_MIX3_CTRL_1.Bits.MIX3_M1, pRegs->DIP_X_MIX3_CTRL_1.Bits.MIX3_M0, real_cnt, expected_cnt);
    }
    else {
        mix_m1 = pRegs->DIP_X_MIX3_CTRL_1.Bits.MIX3_M1;
    }

    mix_m0 = pRegs->DIP_X_MIX3_CTRL_1.Bits.MIX3_M0;

    // final m0 modification
    if (mix_m1<=(mix_m0+4)) mix_m1=(mix_m0+4);

    f_mix_dt = 256.0/float(mix_m1-mix_m0);
    if (f_mix_dt>=0) { mix_dt=(int)(f_mix_dt+0.5); }
    else { mix_dt=(int)(f_mix_dt-0.5); }

    mix_m1 = mix_m0 + 256/mix_dt;
    mix_m1 = RF_CLIP(mix_m1,(mix_m0+4),(real_cnt*32));

    pRegs->DIP_X_MIX3_CTRL_1.Bits.MIX3_M1 = mix_m1;
    pRegs->DIP_X_MIX3_CTRL_1.Bits.MIX3_M0 = mix_m0;
    pRegs->DIP_X_MIX3_CTRL_0.Bits.MIX3_DT = mix_dt;
#else
    mfllLogD("%s: disabled post NR refine func", __FUNCTION__);
#endif
}


// Software workaround for MIX3, but performance trade off.
//  @param a_pbyImg             Virtual address of the image.
//  @param a_i4W                Width of the image.
//  @param a_i4H                Height of the image.
//  @return                     True for performed OK, false not.
bool fix_mix_yuy2(
        unsigned char*  a_pbyImg    __attribute__((unused)),
        int             a_i4W       __attribute__((unused)),
        int             a_i4H       __attribute__((unused))
)
{
    mfllLogD("%s: no function", __FUNCTION__);
    return false;
}


};/* namespace mfll */
#endif//__MFLLMFB_ALGO_H__

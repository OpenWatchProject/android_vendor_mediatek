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


namespace mfll
{
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

    // TODO: implement refine function here
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

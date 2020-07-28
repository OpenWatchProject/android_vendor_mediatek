/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 */
/* MediaTek Inc. (C) 2010. All rights reserved.
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

/********************************************************************************************
 *     LEGAL DISCLAIMER
 *
 *     (Header of MediaTek Software/Firmware Release or Documentation)
 *
 *     BY OPENING OR USING THIS FILE, BUYER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 *     THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE") RECEIVED
 *     FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO BUYER ON AN "AS-IS" BASIS
 *     ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES, EXPRESS OR IMPLIED,
 *     INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR
 *     A PARTICULAR PURPOSE OR NONINFRINGEMENT. NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY
 *     WHATSOEVER WITH RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY,
 *     INCORPORATED IN, OR SUPPLIED WITH THE MEDIATEK SOFTWARE, AND BUYER AGREES TO LOOK
 *     ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. MEDIATEK SHALL ALSO
 *     NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE RELEASES MADE TO BUYER'S SPECIFICATION
 *     OR TO CONFORM TO A PARTICULAR STANDARD OR OPEN FORUM.
 *
 *     BUYER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND CUMULATIVE LIABILITY WITH
 *     RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE, AT MEDIATEK'S OPTION,
TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE
 *     FEES OR SERVICE CHARGE PAID BY BUYER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 *     THE TRANSACTION CONTEMPLATED HEREUNDER SHALL BE CONSTRUED IN ACCORDANCE WITH THE LAWS
 *     OF THE STATE OF CALIFORNIA, USA, EXCLUDING ITS CONFLICT OF LAWS PRINCIPLES.
 ************************************************************************************************/
#ifndef _LCE_MGR_H_
#define _LCE_MGR_H_

#include <MTKLce.h>
#include <ae_param.h>
#include <ispif.h>
#include <dbg_isp_param.h>

using namespace NSIspTuning;

namespace NSIspTuningv3
{

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// CCM Manager
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
class LceMgr
{
public:
    static LceMgr* createInstance(ESensorDev_T const eSensorDev, ISP_NVRAM_LCE_TUNING_PARAM_T* pIspNvramLcePara);
    virtual MVOID destroyInstance() = 0;


private:
    /*
    enum
    {
        CCM_IDX_D65 = 0,
        CCM_IDX_TL84,
        CCM_IDX_CWF,
        CCM_IDX_A,
        CCM_IDX_NUM
    };
    */

private:
/*
    inline
    MVOID
    setIfChange(MINT32 i4Idx)
    {
        if  ( i4Idx != m_i4Idx )
        {
            m_i4Idx = i4Idx;
            m_rCCMOutput = m_rCCMInput[m_i4Idx];
        }
    }
*/
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Index
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:     ////
/*
    inline
    MINT32
    getIdx() const
    {
        return m_i4Idx;
    }

    inline
    MBOOL
    setIdx(MINT32 const i4Idx)
    {
        if  (( CCM_IDX_NUM <= i4Idx ) || ( 0 > i4Idx ))
            return  MFALSE;
        setIfChange(i4Idx);
        return  MTRUE;
    }
*/
private:
    ISP_NVRAM_LCE_T m_rLCEOutput; // LCE output
    ESensorDev_T m_eSensorDev;
    ISP_NVRAM_LCE_TUNING_PARAM_T* m_pIspNvramLcePara;
    MTKLce* m_pLceAlgo;
    MTK_LCE_ENV_INFO_STRUCT mEnv;
    MUINT32 mLceScenario;
    MTK_LCE_PROC_INFO_STRUCT mInInfo;
    MTK_LCE_RESULT_INFO_STRUCT mOutLCE;
    MTK_LCE_EXIF_INFO_STRUCT mOutExif;
    AE_INFO_T mAEInfo;
    MINT32 mLogEn;
    MINT32 mAlgoEn;
    eMTKLCE_TUNING_SET_T m_ePrev_Tuning_Set;
    MBOOL m_bHLR2En;


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Ctor
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:     ////
    LceMgr(ESensorDev_T const eSensorDev, ISP_NVRAM_LCE_TUNING_PARAM_T* pIspNvramLcePara);

    virtual ~LceMgr();

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Operations.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:

    MVOID
    calculateLCE(ISP_LCS_OUT_INFO_T LCSO_info,
                       ISP_NVRAM_LCE_T* pLCEReg, NSIspExifDebug::IspExifDebugInfo_T::IspLceInfo* pLceExif, RAWIspCamInfo const& rRawIspCamInfo,
                       ISP_NVRAM_LCS_T const& p1Lcs, ISP_NVRAM_PGN_T const& p2Pgn, ISP_NVRAM_CCM_T const& p2CCM, MUINT16* pLcsBuffer);

    MVOID setHLRInfo(MBOOL HLR_Enable)
    {
        m_bHLR2En = HLR_Enable;
    }

    MVOID start();

    MVOID stop();

    inline MVOID setScenario(MUINT32 scenario)
    {
        mLceScenario = scenario;
    }

    MVOID printInfo() const;

    MVOID printAEInfo(AE_INFO_T const & rAEInfo, const char* username);

    inline
    ESensorDev_T
    getSensorDev() const
    {
        return m_eSensorDev;
    }

private:
    MVOID setAEInfo(AE_INFO_T const & rAEInfo);

    MVOID setCCMInfo(ISP_NVRAM_LCS_T const& p1_LCS, ISP_NVRAM_CCM_T const & p2_CCM);

    MVOID setPGNInfo(ISP_NVRAM_LCS_T const& p1_LCS, ISP_NVRAM_PGN_T const & p2_PGN);

};

};  //  NSIspTuning
#endif // _LCE_MGR_H_


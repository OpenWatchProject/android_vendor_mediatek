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
#ifndef _GMA_MGR_H_
#define _GMA_MGR_H_

#include <MTKGma.h>
#include <tuning/ae_flow_custom.h>
#include <ispif.h>
#include <dbg_isp_param.h>
#include <mutex>


using namespace NSIspTuning;

namespace NSIspTuning
{


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// CCM Manager
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
class GmaMgr
{
public:
    static GmaMgr* createInstance(ESensorDev_T const eSensorDev, ISP_NVRAM_GMA_STRUCT_T* pIspNvramGmaPara);
    virtual MVOID destroyInstance() = 0;

private:
    ESensorDev_T m_eSensorDev;
    MTKGma* m_pGmaAlgo;
    MTK_GMA_ENV_INFO_STRUCT mEnv;
    MTK_GMA_PROC_INFO_STRUCT mInInfo;
    MTK_GMA_RESULT_INFO_STRUCT mOutGGM;
    MTK_GMA_RESULT_INFO_STRUCT mOutGGM_D1;
    MTK_GMA_RESULT_INFO_STRUCT mOutIGGM_D1;
    MTK_GMA_EXIF_INFO_STRUCT mOutExif;
    ISP_NVRAM_GMA_STRUCT_T* m_pIspNvramGmaPara;
    ISP_NVRAM_GGM_T m_Last_GGM;
    AE_ISP_INFO_T mLast_AEInfo;
    MINT32 mLogEn;
    MINT32 mAlgoEn;
    MUINT16 m_PrvIdx;
    mutable std::mutex mLock;
    MBOOL m_bSameResult;  // Save YNR Performance



//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Ctor
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:     ////
    GmaMgr(ESensorDev_T const eSensorDev, ISP_NVRAM_GMA_STRUCT_T* pIspNvramGmaPara);

    virtual ~GmaMgr();

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Operations.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:
    MVOID
    calculateGGM(RAWIspCamInfo const& rRawIspCamInfo, MUINT16 Idx,
                       ISP_NVRAM_GGM_T* pGGMReg_D1,ISP_NVRAM_IGGM_T* pIGGMReg_D1, ISP_NVRAM_GGM_T* pGGMReg,
                       NSIspExifDebug::IspGmaInfo* pGmaExif);

    MVOID start();

    MVOID stop();

    MUINT16 getIDX() const
    {
        return m_PrvIdx;
    }

    MVOID getLastGGM(ISP_NVRAM_GGM_T& rGGMReg)
    {
        std::lock_guard<std::mutex> lock(mLock);
        rGGMReg = m_Last_GGM;
    }

    MVOID updateLastGGM(ISP_NVRAM_GGM_T& rGGMReg)
    {
        std::lock_guard<std::mutex> lock(mLock);
        m_Last_GGM = rGGMReg;
    }

    MBOOL isGGMSame()
    {
        return m_bSameResult;
    }

private:
    MVOID setAEInfo(AE_ISP_INFO_T const & rAEInfo);

    MVOID printAEInfo(AE_ISP_INFO_T const & rAEInfo, const char* username);
    MVOID printInfo() const;

};

};  //  NSIspTuning
#endif // _GMA_MGR_H_


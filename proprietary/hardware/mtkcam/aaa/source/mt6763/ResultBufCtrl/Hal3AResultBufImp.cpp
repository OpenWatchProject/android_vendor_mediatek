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
#define LOG_TAG "Hal3AResultBufImp"

#include <IHal3AResultBufImp.h>
#include <ResultBufMgr.h>
#include <aaa_result.h>
#include <mtkcam/utils/std/Log.h>
#include <lsc_mgr/ILscTsf.h>

using namespace NS3Av3;

/**
 * @brief 3A Hal Result Buffer Class
 */

class Hal3AResultBufImp : public IHal3AResultBufImp
{
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
protected:  //    Ctor/Dtor.
                                    Hal3AResultBufImp(MUINT32 const i4SensorDevId);
    virtual                         ~Hal3AResultBufImp(){}

private: // disable copy constructor and copy assignment operator
                                    Hal3AResultBufImp(const Hal3AResultBufImp&);
                                    Hal3AResultBufImp& operator=(const Hal3AResultBufImp&);
public:
    static IHal3AResultBufImp*      getInstance(MUINT32 const i4SensorDevId);

    virtual MVOID                   init(MUINT32 u4FrmId);//updateCapacity
    virtual MVOID                   uninit();//clearAll
    virtual MINT32                  updateResult(MUINT32 u4MagicNum, Result_T rResultNew);//u4MagicNum is request magic
    virtual MINT32                  updateResultCur(MUINT32 u4MagicNum, Result_T rResultNew);//u4MagicNum is AAO magic
    virtual MINT32                  getResult(MINT32 i4FrmId, Result_T& rResult);//i4FrmId is current magic
    virtual MINT32                  getResultCur(MINT32 i4FrmId, Result_T& rResult);
    virtual MVOID                   updateRepeatQueue(MINT32 i4MagicNum);//u4MagicNum is request magic
    virtual MVOID                   updateRepeatQueueCur(MINT32 i4MagicNum);
    virtual MBOOL                   queryRepeatQueue(MINT32 i4MagicNum);//i4FrmId is current magic
    virtual MBOOL                   queryRepeatQueueCur(MINT32 i4MagicNum);
    virtual MINT32                  getLast(Result_T& rResult);
    virtual MINT32                  getLastCur(Result_T& rResult);
    virtual MBOOL                   lockBuf();
    virtual MBOOL                   unlockBuf();
    virtual Result_T*               getResult(MINT32 i4FrmId); /*be careful to get refernce! must be protected by mutex*/ //i4FrmId is AAO magic
    virtual MBOOL                   getAAOInfo(MUINT32 u4GridPointNum, MINT32 i4MagicNum, AAO_PROC_INFO_T& rPSOInfo);//i4MagicNum is AAO magic

public:
    MINT32                          m_i4SensorDev;
    ResultBufMgr                    m_rResultBuf;
    ResultBufMgr                    m_rResultBufCur;
    Mutex                           m_Lock;
};

template <MUINT32 const sensorDevId>
class Hal3AResultBufImpDev : public Hal3AResultBufImp
{
public:
    static Hal3AResultBufImpDev* getInstance()
    {
        static Hal3AResultBufImpDev<sensorDevId> singleton;
        return &singleton;
    }
    Hal3AResultBufImpDev()
        : Hal3AResultBufImp(sensorDevId)
    {
    }

private:

};

IHal3AResultBufImp*
IHal3AResultBufImp::
getInstance(MUINT32 const i4SensorDevId)
{
    return Hal3AResultBufImp::getInstance(i4SensorDevId);
}

IHal3AResultBufImp*
Hal3AResultBufImp::
getInstance(MUINT32 const i4SensorDevId)
{
    switch (i4SensorDevId)
    {
        case NSIspTuning::ESensorDev_Main:
            return Hal3AResultBufImpDev<NSIspTuning::ESensorDev_Main>::getInstance();
        case NSIspTuning::ESensorDev_Sub:
            return Hal3AResultBufImpDev<NSIspTuning::ESensorDev_Sub>::getInstance();
        case NSIspTuning::ESensorDev_MainSecond:
            return Hal3AResultBufImpDev<NSIspTuning::ESensorDev_MainSecond>::getInstance();
        case NSIspTuning::ESensorDev_SubSecond:
            return Hal3AResultBufImpDev<NSIspTuning::ESensorDev_SubSecond>::getInstance();
        default:
            CAM_LOGW("Unsupport sensor device ID: %d\n", i4SensorDevId);
            return NULL;
    }
}

Hal3AResultBufImp::
Hal3AResultBufImp(MUINT32 const i4SensorDevId)
    : m_i4SensorDev(i4SensorDevId)
{
}

MVOID
Hal3AResultBufImp::
init(MUINT32 u4FrmId)//updateCapacity
{
    //Mutex::Autolock autoLock(m_Lock);
    CAM_LOGD("[%s] u4FrmId(%d)", __FUNCTION__, u4FrmId);
    m_rResultBuf.updateCapacity(u4FrmId);
    m_rResultBufCur.updateCapacity(u4FrmId);
}

MVOID
Hal3AResultBufImp::
uninit()//clearAll
{
    //Mutex::Autolock autoLock(m_Lock);
    m_rResultBuf.clearAll();
    m_rResultBufCur.clearAll();
}


MINT32
Hal3AResultBufImp::
updateResult(MUINT32 u4MagicNum, Result_T rResultNew)
{
    Mutex::Autolock autoLock(m_Lock);
    return m_rResultBuf.updateResult(u4MagicNum, rResultNew);
}

MINT32
Hal3AResultBufImp::
updateResultCur(MUINT32 u4MagicNum, Result_T rResultNew)
{
    Mutex::Autolock autoLock(m_Lock);
    return m_rResultBufCur.updateResult(u4MagicNum, rResultNew);
}

MINT32
Hal3AResultBufImp::
getResult(MINT32 i4FrmId, Result_T& rResult)
{
    Mutex::Autolock autoLock(m_Lock);
    return m_rResultBuf.getResult(i4FrmId, rResult);
}

MINT32
Hal3AResultBufImp::
getResultCur(MINT32 i4FrmId, Result_T& rResult)
{
    Mutex::Autolock autoLock(m_Lock);
    return m_rResultBufCur.getResult(i4FrmId, rResult);
}

MVOID
Hal3AResultBufImp::
updateRepeatQueue(MINT32 i4MagicNum)
{
    Mutex::Autolock autoLock(m_Lock);
    m_rResultBuf.updateRepeatQueue(i4MagicNum);
}

MVOID
Hal3AResultBufImp::
updateRepeatQueueCur(MINT32 i4MagicNum)
{
    Mutex::Autolock autoLock(m_Lock);
    m_rResultBufCur.updateRepeatQueue(i4MagicNum);
}

MBOOL
Hal3AResultBufImp::
queryRepeatQueue(MINT32 i4MagicNum)
{
    Mutex::Autolock autoLock(m_Lock);
    return m_rResultBuf.queryRepeatQueue(i4MagicNum);
}

MBOOL
Hal3AResultBufImp::
queryRepeatQueueCur(MINT32 i4MagicNum)
{
    Mutex::Autolock autoLock(m_Lock);
    return m_rResultBufCur.queryRepeatQueue(i4MagicNum);
}

MINT32
Hal3AResultBufImp::
getLast(Result_T& rResult)
{
    Mutex::Autolock autoLock(m_Lock);
    return m_rResultBuf.getLast(rResult);
}

MINT32
Hal3AResultBufImp::
getLastCur(Result_T& rResult)
{
    Mutex::Autolock autoLock(m_Lock);
    return m_rResultBufCur.getLast(rResult);
}


MBOOL
Hal3AResultBufImp::
lockBuf()
{
    m_rResultBuf.lockBuf();
    return MTRUE;
}
MBOOL
Hal3AResultBufImp::
unlockBuf()
{
    m_rResultBuf.unlockBuf();
    return MTRUE;
}

Result_T*
Hal3AResultBufImp::
getResult(MINT32 i4FrmId)
{
    //Mutex::Autolock autoLock(m_Lock);// SMVR preview stop, because m_Lock can't exist with queryRepeatQueue and getResult together
    return m_rResultBuf.getResult(i4FrmId);
}

MBOOL
Hal3AResultBufImp::
getAAOInfo(MUINT32 u4GridPointNum, MINT32 i4MagicNum, AAO_PROC_INFO_T& rPSOInfo)
{
    Mutex::Autolock autoLock(m_Lock);

    lockBuf();
    Result_T *rResult = NULL;

    rResult = getResult(i4MagicNum);

    if(rResult == NULL)
    {
        //CAM_LOGE("[%s] Get Result fail", __FUNCTION__);
        ::memset(&rPSOInfo.OB_Offset, 0, sizeof(rPSOInfo.OB_Offset));
        ::memset(&rPSOInfo.OB_Gain, 0, sizeof(rPSOInfo.OB_Gain));
        ::memset(&rPSOInfo.awb_gain, 0, sizeof(rPSOInfo.awb_gain));
        ::memset(&rPSOInfo.PreGain1, 0, sizeof(rPSOInfo.PreGain1));
        ::memset(&rPSOInfo.CCM, 0, sizeof(rPSOInfo.CCM));
        ::memset(&rPSOInfo.LSC_Gain_Grid_R, 0, sizeof(rPSOInfo.LSC_Gain_Grid_R));
        ::memset(&rPSOInfo.LSC_Gain_Grid_G, 0, sizeof(rPSOInfo.LSC_Gain_Grid_G));
        ::memset(&rPSOInfo.LSC_Gain_Grid_B, 0, sizeof(rPSOInfo.LSC_Gain_Grid_B));
        unlockBuf();
        return MFALSE;
    }
    else
    {
        //CAM_LOGD("[%s] rResult Addr(%p)", __FUNCTION__, rResult);

        // OB offset
        rPSOInfo.OB_Offset[0] = rResult->rCamInfo.rOBC1.offst0.val;
        rPSOInfo.OB_Offset[1] = rResult->rCamInfo.rOBC1.offst1.val;
        rPSOInfo.OB_Offset[2] = rResult->rCamInfo.rOBC1.offst2.val;
        rPSOInfo.OB_Offset[3] = rResult->rCamInfo.rOBC1.offst3.val;
        // OB gain
        rPSOInfo.OB_Gain[0] = rResult->rCamInfo.rOBC1.gain0.val;
        rPSOInfo.OB_Gain[1] = rResult->rCamInfo.rOBC1.gain1.val;
        rPSOInfo.OB_Gain[2] = rResult->rCamInfo.rOBC1.gain2.val;
        rPSOInfo.OB_Gain[3] = rResult->rCamInfo.rOBC1.gain3.val;

        // AWB preGain1
        rPSOInfo.PreGain1 = rResult->rCamInfo.rAWBInfo.rPregain1;
        // AWB awbGain
        rPSOInfo.awb_gain = rResult->rCamInfo.rAWBInfo.rCurrentAWBGain;
        // CCM
        //memset(&rPSOInfo.CCM, 0, sizeof(rPSOInfo.CCM));
        ::memcpy(&rPSOInfo.CCM, &rResult->rCamInfo.rAWBInfo.rCscCCM, sizeof(rPSOInfo.CCM));

        // LSC
        ILscTbl rLscTbl(ILscTable::GAIN_FIXED);
        ILscMgr* pLsc = ILscMgr::getInstance(static_cast<ESensorDev_T>(m_i4SensorDev));
        MBOOL ret = pLsc->getGainTable(u4GridPointNum, i4MagicNum, rLscTbl);

        if(ret && rLscTbl.getSize() != 0)
        {
        const MUINT16* rLSCGain = static_cast<const MUINT16*>(rLscTbl.getData());
        for(MUINT32 i=0; i<u4GridPointNum*u4GridPointNum; i++) // Gain table defined in lsc_mgr always starts with B pixel.
        {
            rPSOInfo.LSC_Gain_Grid_B[i] = *(rLSCGain++);
            rPSOInfo.LSC_Gain_Grid_G[i] = *(rLSCGain++);
            (rLSCGain++);
            rPSOInfo.LSC_Gain_Grid_R[i] = *(rLSCGain++);
        }
    }
        else
        {
            ::memset(&rPSOInfo.LSC_Gain_Grid_R, 0, sizeof(rPSOInfo.LSC_Gain_Grid_R));
            ::memset(&rPSOInfo.LSC_Gain_Grid_G, 0, sizeof(rPSOInfo.LSC_Gain_Grid_G));
            ::memset(&rPSOInfo.LSC_Gain_Grid_B, 0, sizeof(rPSOInfo.LSC_Gain_Grid_B));
        }

    }
    unlockBuf();
    return MTRUE;
}

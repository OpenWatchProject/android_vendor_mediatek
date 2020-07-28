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
#define LOG_TAG "aao_buf_mgr"

#ifndef ENABLE_MY_LOG
    #define ENABLE_MY_LOG       (1)
#endif

#include <mtkcam/utils/std/Trace.h>
#include <sys/stat.h>
#include <cutils/properties.h>
#include <string.h>

#include <aaa_types.h>
#include <aaa_error_code.h>
#include <mtkcam/utils/std/Log.h>
#include <debug/DebugUtil.h>
#include <mtkcam/drv/IHalSensor.h>

#if (CAM3_3A_ISP_40_EN)
#include "../common/hal3a/v1.0/buf_mgr/IBufMgr.h"
#include "IResultPool.h"
#else
#include "IBufMgr.h"
#endif

#if CAM3_FAKE_SENSOR_DRV
#include "fake_sensor_drv/IStatisticPipe.h"
#else
#include <mtkcam/drv/iopipe/CamIO/IStatisticPipe.h>
#endif

#include <mtkcam/drv/iopipe/CamIO/IHalCamIO.h>
#include <private/IopipeUtils.h>

#include <utils/List.h>

//AAOSeparation
#include <utils/Condition.h>
#include <ae_param.h>
#include <ae_tuning_custom.h>
#include "AAOSeparation.h"
#include "SttBufQ.h"
#include "string.h"
#undef MAX_STATISTIC_BUFFER_CNT
#define MAX_STATISTIC_BUFFER_CNT (4)

using namespace android;
using namespace NS3Av3;
using namespace NSCam;
using namespace NSCam::NSIoPipe;
using namespace NSCam::NSIoPipe::NSCamIOPipe;

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//AAOSeparation
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
struct AAOStatisticBufInfo : public StatisticBufInfo
{
    MUINT32                 m_u4AAOSepAWBsize;
    MUINT32                 m_u4AAOSepAEsize;
    MUINT32                 m_u4AAOSepHistsize;
    MUINT32                 m_u4AAOSepOverCntsize;
    MUINT32                 m_u4AAOSepLSCsize;
    MUINT32                 m_u4AAOSepsize;
    MVOID*                  m_pAAOSepBuf;
    AAO_BUF_CONFIG_T        m_rCfg;
    AAO_BUF_T               m_rBufs;
    virtual MVOID*          getPart(MUINT32 u4ParId) const;
    virtual MUINT32         getPartSize(MUINT32 u4ParId) const;
};

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//AAOSeparation
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
class AAOStatisticBuf
{
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Data member
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
protected:
    mutable Mutex           m_Lock;
    MBOOL                   m_fgLog;
    MBOOL                   m_fgCmd;
    MBOOL                   m_fgFlush;
    Condition               m_Cond;
    AAOStatisticBufInfo     m_rBufInfo;
    MUINT8                  m_ufgOpt;
    MUINT32                 m_u4width ;
    MUINT32                 m_u4height;
    MUINT32                 m_u4BlocksizeX;
    MUINT32                 m_u4BlocksizeY;
    ConfigInfo_T            m_rConfigInfo;
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Ctor/Dtor.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
private:    ////    Disallowed.
    //  Copy constructor is disallowed.
    AAOStatisticBuf(AAOStatisticBuf const&);
    //  Copy-assignment operator is disallowed.
    AAOStatisticBuf& operator=(AAOStatisticBuf const&);

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Operations.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:
    AAOStatisticBuf();
    virtual ~AAOStatisticBuf();

    virtual MBOOL allocateBuf(MUINT8 const ufgStatOpt, MBOOL bEnablePixelBaseHist,
                            MUINT32 const u4AAWidth,MUINT32 const u4AAHight, MUINT32 const i4WindowNumX, MUINT32 const i4WindowNumY,
                            const ConfigInfo_T& rConfigInfo);
    virtual MBOOL freeBuf();
    virtual MBOOL write(const BufInfo& rBufInfo, const MUINT32 u4FrameCount);
    virtual StatisticBufInfo* read();
    virtual MBOOL flush();
};
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
class AAOBufMgrImp : public AAOBufMgr
{
public:
    virtual MBOOL destroyInstance();
#if (CAM3_3A_ISP_50_EN)
                  AAOBufMgrImp(MINT32 i4SensorDev, MINT32 i4SensorIdx, const ConfigInfo_T& rConfigInfo);
#else
                  AAOBufMgrImp(MINT32 i4SensorDev, MINT32 i4SensorIdx, const ConfigInfo_T& rConfigInfo, I3ACallBack* cb);
#endif
    virtual      ~AAOBufMgrImp();

    virtual MBOOL dequeueHwBuf();
    virtual MBOOL enqueueHwBuf();
    virtual StatisticBufInfo* dequeueSwBuf();
    virtual MVOID abortDequeue();
    virtual MINT32 waitDequeue(){return 0;};
    virtual MVOID reset() {};
    virtual MVOID notifyPreStop();
    virtual MVOID updateInfo(StatisticBufInfo* pStatisticBufInfo);

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Data member
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
private:
    MINT32           const m_i4SensorDev;
    MINT32           const m_i4SensorIdx;
    MBOOL                  m_bDebugEnable;
    MBOOL                  m_bAAODumpEn;
    MBOOL                  m_bAAOHWDumpEn;
    IStatisticPipe*        m_pSttPipe;
    MUINT32                m_rBufIndex;  // the index of write buffers.
    PortID                 m_rPort;
    MBOOL                  m_bAbort;
    AAOStatisticBuf        m_rBufInfo[MAX_STATISTIC_BUFFER_CNT];
    MUINT8                 m_ufgStatOpt;
    MUINT32                m_u4WindowNumX;
    MUINT32                m_u4WindowNumY;
    MBOOL                  m_bEnablePixelBaseHist;
    MUINT32                m_u4AAWidth;
    MUINT32                m_u4AAHight;
    MUINT32                m_u4FrameCount;
    INormalPipe*           m_pCamIO;//NSCam::NSIoPipe::NSCamIOPipe::INormalPipe
    ConfigInfo_T           m_rConfigInfo;
    MBOOL                  m_bPreStop;
    List<QBufInfo>         m_QBufInfoList;
#if (!CAM3_3A_ISP_50_EN)
    I3ACallBack*           m_pCbSet;
#endif
};

#if (CAM3_3ATESTLVL > CAM3_3AUT)
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
#if (CAM3_3A_ISP_50_EN)
AAOBufMgr*
AAOBufMgr::
createInstance(MINT32 const i4SensorDev, MINT32 const i4SensorIdx, const ConfigInfo_T& rConfigInfo, STT_CFG_INFO_T const sttInfo)
{
    AAOBufMgrImp* pObj = new AAOBufMgrImp(i4SensorDev,i4SensorIdx,rConfigInfo);
    return pObj;
}
#else
AAOBufMgr*
AAOBufMgr::
createInstance(MINT32 const i4SensorDev, MINT32 const i4SensorIdx, const ConfigInfo_T& rConfigInfo, I3ACallBack* cb)
{
    AAOBufMgrImp* pObj = new AAOBufMgrImp(i4SensorDev,i4SensorIdx,rConfigInfo, cb);
    return pObj;
}
#endif

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL
AAOBufMgrImp::
destroyInstance()
{
    delete this;
    return MTRUE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
#if (CAM3_3A_ISP_50_EN)
AAOBufMgrImp::
AAOBufMgrImp(MINT32 i4SensorDev, MINT32 i4SensorIdx, const ConfigInfo_T& rConfigInfo)
#else
AAOBufMgrImp::
AAOBufMgrImp(MINT32 i4SensorDev, MINT32 i4SensorIdx, const ConfigInfo_T& rConfigInfo, I3ACallBack* cb)
#endif
    : m_i4SensorDev(i4SensorDev)
    , m_i4SensorIdx(i4SensorIdx)
    , m_bDebugEnable(MFALSE)
    , m_bAAODumpEn(MFALSE)
    , m_bAAOHWDumpEn(MFALSE)
    , m_pSttPipe(NULL)
    , m_rBufIndex(0)
    , m_rPort(PORT_AAO)
    , m_bAbort(MFALSE)
    , m_ufgStatOpt(0)
    , m_u4WindowNumX(0)
    , m_u4WindowNumY(0)
    , m_bEnablePixelBaseHist(0)
    , m_u4AAWidth(0)
    , m_u4AAHight(0)
    , m_u4FrameCount(0)
    , m_pCamIO(NULL)
    , m_rConfigInfo(rConfigInfo)
    , m_bPreStop(MFALSE)
#if (!CAM3_3A_ISP_50_EN)
    , m_pCbSet(cb)
#endif
{

    m_rConfigInfo = rConfigInfo;
    m_QBufInfoList.clear();

    const AE_PARAM_T* pAeParam;
    switch (m_i4SensorDev)
    {
        default:
        case ESensorDev_Main:
            pAeParam = &getAEParam<ESensorDev_Main>();
            break;
        case ESensorDev_Sub:
            pAeParam = &getAEParam<ESensorDev_Sub>();
            break;
        case ESensorDev_MainSecond:
            pAeParam = &getAEParam<ESensorDev_MainSecond>();
            break;
        case ESensorDev_SubSecond:
            pAeParam = &getAEParam<ESensorDev_SubSecond>();
            break;
    }

    m_ufgStatOpt = STAT_OPT(
            pAeParam->strAEParasetting.bEnableHDRLSB,
            pAeParam->strAEParasetting.bEnableAEOVERCNTconfig,
            pAeParam->strAEParasetting.bEnableTSFSTATconfig);
    m_u4WindowNumX = pAeParam->strAEParasetting.u4AEWinodwNumX; //rAwbStatParma.i4WindowNumX;
    m_u4WindowNumY = pAeParam->strAEParasetting.u4AEWinodwNumY; //rAwbStatParma.i4WindowNumY;
    m_bEnablePixelBaseHist = pAeParam->strAEParasetting.bEnablePixelBaseHist;

    // NormalIOPipe create instance
    if (m_pCamIO == NULL)
    {
        m_pCamIO = (INormalPipe*)INormalPipeUtils::get()->createDefaultNormalPipe(m_i4SensorIdx, LOG_TAG);
        if (m_pCamIO == NULL)
        {
            CAM_LOGE("Fail to create NormalPipe");
        }
    }

    if (m_pCamIO != NULL)
        m_pCamIO->sendCommand(NSCam::NSIoPipe::NSCamIOPipe::ENPipeCmd_GET_HBIN_INFO,
                            (MINTPTR)&m_u4AAWidth, (MINTPTR)&m_u4AAHight, 0);

    if(m_u4AAWidth <= 0 || m_u4AAHight <= 0)
        CAM_LOGE("AAWidth and AAHight val error");

    for(int i = 0; i < MAX_STATISTIC_BUFFER_CNT; i++)
    {
        CAM_LOGD("[%s]  i(%d), BitMode(%d) \n", __FUNCTION__, i, m_rConfigInfo.i4BitMode);
        m_rBufInfo[i].allocateBuf(m_ufgStatOpt,m_bEnablePixelBaseHist,m_u4AAWidth,m_u4AAHight,m_u4WindowNumX,m_u4WindowNumY,m_rConfigInfo);
    }

    m_bDebugEnable = ::property_get_int32("vendor.debug.aao_mgr.enable", 0);
    m_bAAODumpEn = ::property_get_int32("vendor.aao.dump.enable", 0);
    m_bAAOHWDumpEn = ::property_get_int32("vendor.aaohw.dump.enable", 0);

    m_pSttPipe = IStatisticPipe::createInstance(m_i4SensorIdx, LOG_TAG);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
AAOBufMgrImp::
~AAOBufMgrImp()
{
    for(int i = 0; i < MAX_STATISTIC_BUFFER_CNT; i++)
    {
        CAM_LOGD("[%s]  i = %d \n", __FUNCTION__, i);
        m_rBufInfo[i].freeBuf();
    }

    if(m_pSttPipe != NULL)
    {
        m_pSttPipe->destroyInstance(LOG_TAG);
        m_pSttPipe = NULL;
    }

    // NormalIOPipe destroy instance
    if (m_pCamIO != NULL)
    {
        m_pCamIO->destroyInstance(LOG_TAG);
        m_pCamIO = NULL;
    }
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
static MBOOL dumpHwBuf(char* fileName, char* folderName, BufInfo& rLastBuf, MBOOL isHwBuf = 0)
{
    FILE *fp = fopen(fileName, "w");
    if (NULL == fp)
    {
        CAM_LOGE("fail to open file to save img: %s", fileName);
        MINT32 err = mkdir(folderName, S_IRWXU | S_IRWXG | S_IRWXO);
        CAM_LOGD("err = %d", err);
        return MFALSE;
    } else
    {
        if(isHwBuf){
            fwrite(&(rLastBuf.mMetaData.mMagicNum_hal), sizeof(MUINT32), 1, fp);
            fwrite(&(rLastBuf.mStride), sizeof(MUINT32), 1, fp);
            fwrite(&(rLastBuf.mSize), sizeof(MUINT32), 1, fp);
        }
        fwrite(reinterpret_cast<void *>(rLastBuf.mVa), 1, rLastBuf.mSize, fp);
        fclose(fp);
        return MTRUE;
    }
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MINT32
AAOBufMgrImp::
dequeueHwBuf()
{
    CAM_LOGD_IF(m_bDebugEnable,"[%s] m_i4SensorDev(%d) + \n", __FUNCTION__, m_i4SensorDev);

    if(m_bAbort || m_bPreStop)
        return -1;

    if(m_pSttPipe != NULL)
    {
        DebugUtil::getInstance(m_i4SensorDev)->update(LOG_TAG, "dequeueHwBuf", 0);
        // deque HW buffer from driver.
        QBufInfo    rDQBuf;
        CAM_TRACE_BEGIN("AAO STT deque");
        MBOOL ret = m_pSttPipe->deque(m_rPort, rDQBuf);
        CAM_TRACE_END();
        int size = rDQBuf.mvOut.size();
        DebugUtil::getInstance(m_i4SensorDev)->update(LOG_TAG, "dequeueHwBuf", 1);
        if(m_bAbort || m_bPreStop || (size == 0 && ret == MTRUE)){
            DebugUtil::getInstance(m_i4SensorDev)->update(LOG_TAG, "dequeueHwBuf", -1);
            return -1;
        }

        if(ret == MFALSE)
        {
            CAM_LOGE("[%s] m_pSttPipe deque fail", __FUNCTION__);
            DebugUtil::getInstance(m_i4SensorDev)->update(LOG_TAG, "dequeueHwBuf", -1);
            return MFALSE;
        }
        // get the last HW buffer to SW Buffer.
        int index = m_rBufIndex;
        m_u4FrameCount++;
        BufInfo rLastBuf = rDQBuf.mvOut.at( size-1 );

        // copy the last HW buffer to SW Buffer.
        DebugUtil::getInstance(m_i4SensorDev)->update(LOG_TAG, "dequeueHwBuf", 2);
        CAM_TRACE_BEGIN("AAO STT clone");
        m_rBufInfo[index].write(rLastBuf, m_u4FrameCount);

        do {
            android::sp<ISttBufQ> pSttBufQ;
            android::sp<ISttBufQ::DATA> pData;
            int BufSize;

            pSttBufQ = NSCam::ISttBufQ::getInstance(m_i4SensorDev);
            if (pSttBufQ == NULL) break;

            pData = pSttBufQ->deque();
            if (pData == NULL) break;

            BufSize = rLastBuf.mSize;
            pData->AAO.resize(BufSize);
            pData->MagicNumberStt     = rLastBuf.mMetaData.mMagicNum_hal;
            pData->MagicNumberRequest = 0;
            ::memcpy(pData->AAO.editArray(), (const void*)rLastBuf.mVa, BufSize);

            pSttBufQ->enque(pData);
        } while(0);

        CAM_TRACE_END();
        DebugUtil::getInstance(m_i4SensorDev)->update(LOG_TAG, "dequeueHwBuf", 3);


        CAM_LOGD_IF(m_bDebugEnable, "port(%d), index(%d), va[%d]/pa[0x%d]/#(%d), Size(%d), timeStamp(%" PRId64 ")",
                          (MINT32)m_rPort.index, index, (MINT32)rLastBuf.mVa, (MINT32)rLastBuf.mPa, rLastBuf.mMetaData.mMagicNum_hal, rLastBuf.mSize, rLastBuf.mMetaData.mTimeStamp);

        // dump CmdQ info for debug
        if(m_bDebugEnable)
        {
            if(rLastBuf.mMetaData.mPrivateDataSize >= 8)//8 byte
            {
                MUINT32* pPrivateData = (MUINT32*)rLastBuf.mMetaData.mPrivateData;
                CAM_LOGD("CmdQ#(%d), Size(%d), u4Rto(0x%08x), OBCGain(%d)",
                    rLastBuf.mMetaData.mMagicNum_tuning, rLastBuf.mMetaData.mPrivateDataSize, pPrivateData[1], pPrivateData[0]);
            }
        }

        // dump aao data for debug
        if (m_bAAODumpEn) {
            static MUINT32 count = 0;
            char fileName[64];
            char folderName[64];
            strncpy(folderName, "/sdcard/aao", 12);
            sprintf(fileName, "/sdcard/aao/aao_%d_%d.raw", rLastBuf.mMetaData.mMagicNum_hal, count++);
            if(dumpHwBuf(fileName, folderName, rLastBuf))
            {
                CAM_LOGD_IF(m_bDebugEnable,"%s\n", fileName);
            }
        }
        // dump aao hw data for ut
        if(m_bAAOHWDumpEn){
            char fileName[64];
            char folderName[64];
            strncpy(folderName, "/sdcard/aaoHwBuf", 17);
            sprintf(fileName, "/sdcard/aaoHwBuf/aao_%d.hwbuf", rLastBuf.mMetaData.mMagicNum_hal);
            if(dumpHwBuf(fileName, folderName, rLastBuf, m_bAAOHWDumpEn))
            {
                CAM_LOGD_IF(m_bDebugEnable,"%s\n", fileName);
            }
        }

        if(m_bAbort || m_bPreStop)
            return -1;
        // enque HW buffer back driver
        CAM_LOGD_IF(m_bDebugEnable, "enque Hw buffer back driver. QBufInfoList size:%zu\n", m_QBufInfoList.size());
        CAM_TRACE_BEGIN("AAO STT enque");

        if(m_QBufInfoList.size())
        {
            List<QBufInfo>::iterator it = m_QBufInfoList.begin();
            for (MINT32 i = 0; it != m_QBufInfoList.end(); i++, it++)
            {
                BufInfo rLastBuf = it->mvOut.at( size-1 );
                CAM_LOGD("enque QBufInfoList MagicNum_hal:%d\n", rLastBuf.mMetaData.mMagicNum_hal);
                m_pSttPipe->enque((*it));
            }
            m_QBufInfoList.clear();
        }

        MINT32 i4Ret = m_pSttPipe->enque(rDQBuf);
        if(!i4Ret)
        {
            CAM_LOGD("Push pack enque fail buf to QBufInfoList\n");
            m_QBufInfoList.push_back(rDQBuf);
        }

        CAM_TRACE_END();
        CAM_TRACE_BEGIN("STTnotify");
#if (!CAM3_3A_ISP_50_EN)
        m_pCbSet->doNotifyCb(I3ACallBack::ECallBack_T::eID_NOTIFY_P1_DONE_FROMSTT, rLastBuf.mMetaData.mMagicNum_hal, NULL, NULL);
#endif
        CAM_TRACE_END();
        DebugUtil::getInstance(m_i4SensorDev)->update(LOG_TAG, "dequeueHwBuf", -1);
    }

    return MTRUE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL
AAOBufMgrImp::
enqueueHwBuf()
{
    return MTRUE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
StatisticBufInfo*
AAOBufMgrImp::
dequeueSwBuf()
{
    CAM_LOGD_IF(m_bDebugEnable,"[%s] index(%d) +\n", __FUNCTION__, m_rBufIndex);
    if(m_bAbort || m_bPreStop)
        return NULL;
    DebugUtil::getInstance(m_i4SensorDev)->update(LOG_TAG, "dequeueSwBuf", 0);
    int r_index = m_rBufIndex;
    StatisticBufInfo* pBuf = m_rBufInfo[r_index].read();
    DebugUtil::getInstance(m_i4SensorDev)->update(LOG_TAG, "dequeueSwBuf", 1);
    // switch buffer
    m_rBufIndex = ( r_index + 1 ) % MAX_STATISTIC_BUFFER_CNT;
    CAM_LOGD_IF(m_bDebugEnable,"[%s] m_i4SensorDev(%d), port(%d), r_index(%d), w_index(%d)\n", __FUNCTION__, m_i4SensorDev, (MINT32)m_rPort, r_index, m_rBufIndex);
    DebugUtil::getInstance(m_i4SensorDev)->update(LOG_TAG, "dequeueSwBuf", -1);
    if(pBuf)
        updateInfo(pBuf);
    return pBuf;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MVOID
AAOBufMgrImp::
abortDequeue()
{
    CAM_LOGD("[%s] m_rBufIndex(%d) +\n", __FUNCTION__, m_rBufIndex);
    m_bAbort = MTRUE;
    m_u4FrameCount = 0;
    if(m_pSttPipe != NULL)
        m_pSttPipe->abortDma(m_rPort,LOG_TAG);
    CAM_LOGD("[%s] abortDma done\n", __FUNCTION__);
    for(int i = 0; i < MAX_STATISTIC_BUFFER_CNT; i++)
        m_rBufInfo[i].flush();
    CAM_LOGD("[%s] m_rBufIndex(%d) -\n", __FUNCTION__, m_rBufIndex);
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MVOID
AAOBufMgrImp::
notifyPreStop()
{
    CAM_LOGD("[%s] m_rBufIndex(%d) +\n", __FUNCTION__, m_rBufIndex);
    m_bPreStop = MTRUE;
    CAM_LOGD("[%s] m_rBufIndex(%d) -\n", __FUNCTION__, m_rBufIndex);
}
MVOID
AAOBufMgrImp::
updateInfo(StatisticBufInfo* pStatisticBufInfo)
{
#if (CAM3_3A_ISP_40_EN)
    // update AE mode to statistic information.
    IResultPool* pResultPoolObj = IResultPool::getInstance(m_i4SensorDev);
    if(pResultPoolObj)
    {
        AEResultToMeta_T* pAEResult = (AEResultToMeta_T*)pResultPoolObj->getResult(pStatisticBufInfo->mMagicNumber,E_AE_RESULTTOMETA);
        if(pAEResult != NULL)
        {
            pStatisticBufInfo->mu1AeMode = pAEResult->u1AeMode;
            CAM_LOGD_IF(m_bDebugEnable,"[%s] AeMode(%d) \n", __FUNCTION__, pStatisticBufInfo->mu1AeMode);
        }
        else
        {
            CAM_LOGE("[%s]  pAEResult is NULL \n", __FUNCTION__);
            pStatisticBufInfo->mu1AeMode = MTK_CONTROL_AE_MODE_ON; // error control for batch mode of SMVR
        }

        FLASHResultToMeta_T* pFLASHResult = (FLASHResultToMeta_T*)pResultPoolObj->getResult(pStatisticBufInfo->mMagicNumber,E_FLASH_RESULTTOMETA);
        if (pFLASHResult != NULL) {
            pStatisticBufInfo->mFlashState = pFLASHResult->u1FlashState;
        } else {
            CAM_LOGE("[%s] Failed to get FLASHResult for MagicNum(%d)\n", __FUNCTION__, pStatisticBufInfo->mMagicNumber);
        }
    }
        else
            CAM_LOGE("[%s]  pResultPoolObj is NULL \n", __FUNCTION__);
#endif
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//AAOSeparation
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MVOID*
AAOStatisticBufInfo::
getPart(MUINT32 u4ParId) const
{
    switch (u4ParId)
    {
        case AAOSepAWBBuf:
            return (m_rBufs.m_pAAOSepAWBBuf);
            break;
        case AAOSepAEBuf:
            return (m_rBufs.m_pAAOSepAEBuf);
            break;
        case AAOSepHistBuf:
            return (m_rBufs.m_pAAOSepHistBuf);
            break;
        case AAOSepOverCntBuf:
            return (m_rBufs.m_pAAOSepOverCntBuf);
            break;
        case AAOSepLSCBuf:
            return (m_rBufs.m_pAAOSepLSCBuf);
            break;
        case AAOSepBuf:
            return (m_pAAOSepBuf);
            break;
        default:
            return NULL;
            break;
    }
}

MUINT32
AAOStatisticBufInfo::
getPartSize(MUINT32 u4ParId) const
{
    switch (u4ParId)
    {
        case AAOSepAWBBuf:
            return m_u4AAOSepAWBsize;
            break;
        case AAOSepAEBuf:
            return m_u4AAOSepAEsize;
            break;
        case AAOSepHistBuf:
            return m_u4AAOSepHistsize;
            break;
        case AAOSepOverCntBuf:
            return m_u4AAOSepOverCntsize;
            break;
        case AAOSepLSCBuf:
            return m_u4AAOSepLSCsize;
            break;
        case AAOSepBuf:
            return m_u4AAOSepsize;
            break;
        default:
            return MFALSE;
            break;
    }
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Function.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
AAOStatisticBuf::
AAOStatisticBuf()
    : m_fgLog(MFALSE)
    , m_fgCmd(MFALSE)
    , m_fgFlush(MFALSE)
    , m_ufgOpt(0)
    , m_u4width(0)
    , m_u4height(0)
    , m_u4BlocksizeX(0)
    , m_u4BlocksizeY(0)
{
    memset(&m_rBufInfo.m_rBufs, 0, sizeof(AAO_BUF_T));
    memset(&m_rBufInfo.m_rCfg, 0, sizeof(AAO_BUF_CONFIG_T));
    m_rBufInfo.mVa = (MUINTPTR)NULL;
    m_rBufInfo.mSize = 0;
    m_rBufInfo.m_pAAOSepBuf = NULL;
    m_rBufInfo.m_u4AAOSepAEsize = 0;
    m_rBufInfo.m_u4AAOSepAWBsize = 0;
    m_rBufInfo.m_u4AAOSepHistsize = 0;
    m_rBufInfo.m_u4AAOSepLSCsize = 0;
    m_rBufInfo.m_u4AAOSepOverCntsize = 0;
    m_rBufInfo.m_u4AAOSepsize = 0;

}

AAOStatisticBuf::
~AAOStatisticBuf()
{
    freeBuf();
}

MBOOL
AAOStatisticBuf::
allocateBuf(MUINT8 const ufgStatOpt,MBOOL bEnablePixelBaseHist,
            MUINT32 const u4AAWidth, MUINT32 const u4AAHight, MUINT32 const u4WindowNumX, MUINT32 const u4WindowNumY,
            const ConfigInfo_T& rConfigInfo)
{
    Mutex::Autolock lock(m_Lock);

    m_fgLog = property_get_int32("vendor.debug.statistic_buf.enable", 0);

    m_ufgOpt = ufgStatOpt;
    m_rConfigInfo = rConfigInfo;
    CAM_LOGD_IF(m_fgLog, "[%s] BitMode(%d) \n", __FUNCTION__, m_rConfigInfo.i4BitMode);

    m_rBufInfo.m_rCfg.m_i4BlkNumX = u4WindowNumX;//m_rTsfEnvInfo.ImgWidth;(120)
    m_rBufInfo.m_rCfg.m_i4BlkNumY = u4WindowNumY;//m_rTsfEnvInfo.ImgHeight;(90)
    m_rBufInfo.m_rCfg.m_bEnableHDRYConfig = (m_ufgOpt & 0x4);
    m_rBufInfo.m_rCfg.m_bEnableOverCntConfig = (m_ufgOpt & 0x2);
    m_rBufInfo.m_rCfg.m_bEnableTSFConfig = (m_ufgOpt & 0x1);
    m_rBufInfo.m_rCfg.m_bDisablePixelHistConfig = ((bEnablePixelBaseHist==1)?MFALSE:MTRUE);
    m_u4width = u4AAWidth;
    m_u4height = u4AAHight;
    m_u4BlocksizeX = ((m_u4width / m_rBufInfo.m_rCfg.m_i4BlkNumX)/2)*2;
    m_u4BlocksizeY = ((m_u4height / m_rBufInfo.m_rCfg.m_i4BlkNumY)/2)*2;
    m_rBufInfo.m_rCfg.m_u4AEOverExpCntShift = (((m_u4BlocksizeX/4)*(m_u4BlocksizeY/2)>255)?MTRUE:MFALSE);
    m_rBufInfo.m_rCfg.m_i4AwbLineSize = m_rBufInfo.m_rCfg.m_i4BlkNumX * 4;
    m_rBufInfo.m_rCfg.m_i4AeSize      = ((m_rBufInfo.m_rCfg.m_i4BlkNumX + 3)/4) * 4;                           // in byte

    if(m_rConfigInfo.i4BitMode == EBitMode_12Bit)
        m_rBufInfo.m_rCfg.m_i4HdrSize     = (m_ufgOpt & 0x4) ? ((m_rBufInfo.m_rCfg.m_i4BlkNumX*4 + 31)/32)*4 : 0;  // in byte
    else if(m_rConfigInfo.i4BitMode == EBitMode_14Bit)
        m_rBufInfo.m_rCfg.m_i4HdrSize     = (m_ufgOpt & 0x4) ? ((m_rBufInfo.m_rCfg.m_i4BlkNumX*8 + 31)/32)*4 : 0;  // in byte

    m_rBufInfo.m_rCfg.m_i4AeOverSize  = (m_ufgOpt & 0x2) ? ((m_rBufInfo.m_rCfg.m_i4BlkNumX*8 + 31)/32)*4 : 0;  // in byte
    m_rBufInfo.m_rCfg.m_i4TsfSize     = (m_ufgOpt & 0x1) ? ((m_rBufInfo.m_rCfg.m_i4BlkNumX*16 + 31)/32)*4 : 0; // in byte
    m_rBufInfo.m_rCfg.m_i4LineSize    = m_rBufInfo.m_rCfg.m_i4AwbLineSize + m_rBufInfo.m_rCfg.m_i4AeSize + m_rBufInfo.m_rCfg.m_i4HdrSize + m_rBufInfo.m_rCfg.m_i4AeOverSize + m_rBufInfo.m_rCfg.m_i4TsfSize;//lineByte
    m_rBufInfo.m_rCfg.m_i4SkipSize    = m_rBufInfo.m_rCfg.m_i4AwbLineSize + m_rBufInfo.m_rCfg.m_i4AeSize + m_rBufInfo.m_rCfg.m_i4HdrSize + m_rBufInfo.m_rCfg.m_i4AeOverSize;

    m_rBufInfo.m_u4AAOSepAWBsize = AAO_SEP_AWB_SIZE_COEF * u4WindowNumX * u4WindowNumY;
    m_rBufInfo.m_u4AAOSepAEsize = AAO_SEP_AE_SIZE_COEF * u4WindowNumX*u4WindowNumY;
    m_rBufInfo.m_u4AAOSepHistsize = AAO_SEP_HIST_SIZE_COEF * AAO_STT_HIST_BIN_NUM;
    m_rBufInfo.m_u4AAOSepOverCntsize = AAO_SEP_OC_SIZE_COEF * u4WindowNumX*u4WindowNumY;
    m_rBufInfo.m_u4AAOSepLSCsize = AAO_SEP_LSC_SIZE_COEF * u4WindowNumX*u4WindowNumY;
    m_rBufInfo.m_u4AAOSepsize = m_rBufInfo.m_u4AAOSepAWBsize+m_rBufInfo.m_u4AAOSepAEsize+m_rBufInfo.m_u4AAOSepHistsize
                                +m_rBufInfo.m_u4AAOSepOverCntsize+m_rBufInfo.m_u4AAOSepLSCsize;

    m_rBufInfo.m_pAAOSepBuf = (MVOID*)malloc(m_rBufInfo.m_u4AAOSepsize);
    m_rBufInfo.m_rBufs.m_pAAOSepAWBBuf = m_rBufInfo.m_pAAOSepBuf;
    m_rBufInfo.m_rBufs.m_pAAOSepAEBuf = (MUINT8 *)m_rBufInfo.m_pAAOSepBuf + m_rBufInfo.m_u4AAOSepAWBsize;
    m_rBufInfo.m_rBufs.m_pAAOSepHistBuf = (MUINT8 *)m_rBufInfo.m_pAAOSepBuf + m_rBufInfo.m_u4AAOSepAWBsize + m_rBufInfo.m_u4AAOSepAEsize;
    m_rBufInfo.m_rBufs.m_pAAOSepOverCntBuf = (MUINT8 *)m_rBufInfo.m_pAAOSepBuf + m_rBufInfo.m_u4AAOSepAWBsize+ m_rBufInfo.m_u4AAOSepAEsize+ m_rBufInfo.m_u4AAOSepHistsize;
    m_rBufInfo.m_rBufs.m_pAAOSepLSCBuf = (MUINT8 *)m_rBufInfo.m_pAAOSepBuf + m_rBufInfo.m_u4AAOSepAWBsize+ m_rBufInfo.m_u4AAOSepAEsize+ m_rBufInfo.m_u4AAOSepHistsize+ m_rBufInfo.m_u4AAOSepOverCntsize;

    if(m_rBufInfo.m_pAAOSepBuf != NULL && m_rBufInfo.m_u4AAOSepsize != 0){
        CAM_LOGD_IF(m_fgLog, "[%s] m_rBufInfo->m_pAAOSepBuf = 0x%p, \n", __FUNCTION__, m_rBufInfo.m_pAAOSepBuf);
        CAM_LOGD_IF(m_fgLog, "[%s] m_rBufInfo->m_u4AAOSepsize = 0x%d, \n", __FUNCTION__, (MINT32)m_rBufInfo.m_u4AAOSepsize);
    }

    if(m_rBufInfo.m_pAAOSepBuf == NULL)
    {
        CAM_LOGE("m_rBufInfo.m_pAAOSepBuf == NULL");
        return MFALSE;
    }

    return MTRUE;
}

MBOOL
AAOStatisticBuf::
freeBuf()
{
    Mutex::Autolock lock(m_Lock);
    if(m_rBufInfo.m_pAAOSepBuf != NULL && m_rBufInfo.m_u4AAOSepsize != 0){
        CAM_LOGD_IF(m_fgLog, "[%s] m_rBufInfo->m_pAAOSepBuf = 0x%p, \n", __FUNCTION__, m_rBufInfo.m_pAAOSepBuf);
        free(m_rBufInfo.m_pAAOSepBuf);
    }

    memset(&m_rBufInfo.m_rBufs, 0, sizeof(AAO_BUF_T));
    memset(&m_rBufInfo.m_rCfg, 0, sizeof(AAO_BUF_CONFIG_T));
    m_rBufInfo.m_pAAOSepBuf = NULL;
    m_rBufInfo.m_u4AAOSepAEsize = 0;
    m_rBufInfo.m_u4AAOSepAWBsize = 0;
    m_rBufInfo.m_u4AAOSepHistsize = 0;
    m_rBufInfo.m_u4AAOSepLSCsize = 0;
    m_rBufInfo.m_u4AAOSepOverCntsize = 0;
    m_rBufInfo.m_u4AAOSepsize = 0;

    return MTRUE;
}

MBOOL
AAOStatisticBuf::
write(const BufInfo& rBufInfo, const MUINT32 u4FrameCount)
{
    Mutex::Autolock lock(m_Lock);

    MINT32 AAOSepType;

#if CAM3_AAO_NEON_EN //NEON-version
    AAOSepType = AAONeonType;
#else
    AAOSepType = AAOCType;
#endif

    if(m_rBufInfo.m_pAAOSepBuf == NULL || m_rBufInfo.m_u4AAOSepsize ==0){
        CAM_LOGE("m_rBufInfo.m_pAAOSepBuf == NULL");
        CAM_LOGE("m_rBufInfo.m_u4AAOSepsize == 0");
        return MFALSE;
    }

    CAM_LOGD_IF(m_fgLog, "[%s] + m_fgCmd(%d) Magic(%d) BitMode(%d) AAOSepType(%d)\n", __FUNCTION__,
        m_fgCmd, rBufInfo.mMetaData.mMagicNum_hal, m_rConfigInfo.i4BitMode, AAOSepType);

    // copy buffer member
    m_rBufInfo.mMagicNumber = rBufInfo.mMetaData.mMagicNum_hal;
    m_rBufInfo.mSize = rBufInfo.mSize;
    m_rBufInfo.mStride = rBufInfo.mStride;
    m_rBufInfo.mFrameCount = u4FrameCount;
    m_rBufInfo.mTimeStamp = rBufInfo.mMetaData.mTimeStamp;

    MVOID* AAOSrc = reinterpret_cast<MVOID *>(rBufInfo.mVa);

    if(m_rConfigInfo.i4BitMode == EBitMode_12Bit)
    {
        if(aaoSeparation12(AAOSrc, m_rBufInfo.m_rCfg, m_rBufInfo.m_rBufs, AAOSepType) != MTRUE)
            CAM_LOGE("aaoSeparation12 fail!");
        else
            CAM_LOGD_IF(m_fgLog,"[%s] aaoSeparation12 success!", __FUNCTION__);
    }
    else if(m_rConfigInfo.i4BitMode == EBitMode_14Bit)
    {
        if(aaoSeparation14(AAOSrc, m_rBufInfo.m_rCfg, m_rBufInfo.m_rBufs, AAOSepType) != MTRUE)
            CAM_LOGE("aaoSeparation14 fail!");
        else
            CAM_LOGD_IF(m_fgLog,"[%s] aaoSeparation14 success!", __FUNCTION__);
    }

    m_fgCmd = MTRUE;
    // broadcast for writing buffer done.
    m_Cond.broadcast();
    CAM_LOGD_IF(m_fgLog, "[%s] - m_fgCmd(%d) Magic(%d)\n", __FUNCTION__, m_fgCmd, rBufInfo.mMetaData.mMagicNum_hal);
    return MTRUE;
}

StatisticBufInfo*
AAOStatisticBuf::
read()
{
    Mutex::Autolock lock(m_Lock);

    if(m_rBufInfo.m_pAAOSepBuf == NULL){
        CAM_LOGE("m_rBufInfo.m_pAAOSepBuf == NULL");
        return NULL;
    }

    if(m_fgFlush)
        return NULL;

    CAM_LOGD_IF(m_fgLog, "[%s] + m_fgCmd(%d)", __FUNCTION__, m_fgCmd);
    // wait for writing buffer done.
    if(!m_fgCmd)
    {
        CAM_LOGD_IF(m_fgLog, "[%s] wait\n", __FUNCTION__);
        m_Cond.wait(m_Lock);
    }
    m_fgCmd = MFALSE;
    CAM_LOGD_IF(m_fgLog, "[%s] - m_fgCmd(%d) Magic(%d)\n", __FUNCTION__, m_fgCmd, m_rBufInfo.mMagicNumber);
    if(m_fgFlush)
    {
        return NULL;
    }
    else
        return &m_rBufInfo;
}

MBOOL
AAOStatisticBuf::
flush()
{
    Mutex::Autolock lock(m_Lock);
    CAM_LOGD_IF(m_fgLog, "[%s] + m_fgCmd(%d)\n", __FUNCTION__, m_fgCmd);
    m_fgFlush = MTRUE;
    if(!m_fgCmd)
        m_Cond.broadcast();
    CAM_LOGD_IF(m_fgLog, "[%s] -\n", __FUNCTION__);
    return MTRUE;
}

#endif

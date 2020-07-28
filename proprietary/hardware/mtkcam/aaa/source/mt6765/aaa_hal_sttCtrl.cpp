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
#define LOG_TAG "aaa_hal_sttCtrl"

#include <mtkcam/utils/std/Trace.h>
#include <sys/stat.h>
#include <cutils/properties.h>
#include <string.h>

#include <isp_tuning.h>
#include <aaa_hal_sttCtrl.h>
#include <aaa_types.h>
#include <aaa_error_code.h>
#include <mtkcam/utils/std/Log.h>
#include <mtkcam/drv/IHalSensor.h>


#include <mtkcam/drv/iopipe/CamIO/IHalCamIO.h>
#include <private/IopipeUtils.h>

#include <debug/DebugUtil.h>
#include <cutils/atomic.h>

#include <ae_mgr/ae_mgr_if.h>

#include <af_param.h>
#include <af_algo_if.h>
#include <af_mgr/af_mgr_if.h>
#include <af_define.h>
#if defined(MTKCAM_CCU_AF_SUPPORT)
#include "ccu_ext_interface/ccu_af_reg.h"
#include "iccu_ctrl_af.h"
#include "iccu_mgr.h"
using namespace NSCcuIf;
#endif

#define PD_SEPARATE_MODE 0


#if defined(HAVE_AEE_FEATURE)
#include <aee.h>
#define AEE_ASSERT_3A_HAL(String) \
          do { \
              aee_system_exception( \
                  "Hal3ASttCtrl", \
                  NULL, \
                  DB_OPT_DEFAULT, \
                  String); \
          } while(0)
#else
#define AEE_ASSERT_3A_HAL(String)
#endif

using namespace NS3Av3;
using namespace NSCam;

Hal3ASttCtrl*
Hal3ASttCtrl::
createInstance(MINT32 i4SensorDevId, MINT32 i4SensorOpenIndex)
{
    CAM_LOGD("Hal3ASttFlow::createInstance i4SensorDevId(%d), i4SensorOpenIndex(%d)\n"
           , i4SensorDevId
           , i4SensorOpenIndex);

    switch (i4SensorDevId)
    {
        case ESensorDev_Main:
            return Hal3ASttCtrlDev<ESensorDev_Main>::getInstance();
        break;
        case ESensorDev_Sub:
            return Hal3ASttCtrlDev<ESensorDev_Sub>::getInstance();
        break;
        case ESensorDev_MainSecond:
            return Hal3ASttCtrlDev<ESensorDev_MainSecond>::getInstance();
        break;
        case ESensorDev_SubSecond:
            return Hal3ASttCtrlDev<ESensorDev_SubSecond>::getInstance();
        break;
        default:
            CAM_LOGE("Unsupport sensor device ID: %d\n", i4SensorDevId);
            AEE_ASSERT_3A_HAL("Unsupport sensor device.");
            return MNULL;
    }
}

Hal3ASttCtrl*
Hal3ASttCtrl::
getInstance(MINT32 i4SensorDevId)
{
    switch (i4SensorDevId)
    {
        case ESensorDev_Main:
            return Hal3ASttCtrlDev<ESensorDev_Main>::getInstance();
        case ESensorDev_Sub:
            return Hal3ASttCtrlDev<ESensorDev_Sub>::getInstance();
        case ESensorDev_MainSecond:
            return Hal3ASttCtrlDev<ESensorDev_MainSecond>::getInstance();
        case ESensorDev_SubSecond:
            return Hal3ASttCtrlDev<ESensorDev_SubSecond>::getInstance();
        default:
            CAM_LOGE("Unsupport sensor device ID: %d\n", i4SensorDevId);
            AEE_ASSERT_3A_HAL("Unsupport sensor device.");
            return MNULL;
    }

}

MBOOL
Hal3ASttCtrl::
setSensorDevInfo(MINT32 i4SensorDevId, MINT32 i4SensorOpenIndex)
{
    CAM_LOGD("[%s] i4SensorDevId(%d), i4SensorOpenIndex(%d)\n"
           , __FUNCTION__
           , i4SensorDevId
           , i4SensorOpenIndex);

    m_i4SensorDev = i4SensorDevId;
    m_i4SensorIdx = i4SensorOpenIndex;
    return MTRUE;
}

MBOOL
Hal3ASttCtrl::
getPdInfoForSttCtrl()
{
    IAfMgr::getInstance().getPdInfoForSttCtrl(m_i4SensorDev, m_i4SensorIdx, m_i4SensorMode,
        m_u4PDOSizeW, m_u4PDOSizeH, m_pPDAFStatus);

    CAM_LOGD("[%s] SensorDev(%d), SensorOpenIdx(%d), SensorMode(%d), PDAFStatus(%d), PDOSizeW(%d), PDOSizeH(%d)\n",
        __FUNCTION__, m_i4SensorDev, m_i4SensorIdx, m_i4SensorMode, m_pPDAFStatus, m_u4PDOSizeW, m_u4PDOSizeH);

    return MTRUE;
}

MVOID
Hal3ASttCtrl::
destroyInstance()
{
    CAM_LOGD("[%s] \n", __FUNCTION__);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
Hal3ASttCtrl::Hal3ASttCtrl()
    : m_Users(0)
    , m_Lock()
    , rAAOBufThread(NULL)
    , rAFOBufThread(NULL)
    , rPDOBufThread(NULL)
    , rAEOBufThread(NULL)
    , m_i4SensorDev(0)
    , m_i4SensorIdx(0)
    , m_i4SensorMode(0)
    , m_i4SttPortEnable(0)
    , m_u4TGSizeW(0)
    , m_u4TGSizeH(0)
    , m_u4PDOSizeW(0)
    , m_u4PDOSizeH(0)
    , m_pSttPipe(NULL)
    , m_pPDAFStatus(FEATURE_PDAF_UNSUPPORT)
    , m_pMvHDRStatus(FEATURE_MVHDR_UNSUPPORT)
    , m_bTgIntAEEn(MFALSE)
    , m_fTgIntAERatio(0)
    , m_bPreStopSttEnable(0)
    , m_pCbSet(NULL)
{
    for(int i = 0; i < EPIPE_CAMSV_FEATURE_NUM; i++)
        m_pCamsvSttPipe[i] = NULL;

    for(int i = 0; i < BUF_NUM; i++)
        m_pBufMgrList[i] = NULL;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
Hal3ASttCtrl::~Hal3ASttCtrl()
{}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL
Hal3ASttCtrl::
initStt(MINT32 i4SensorDevId, MINT32 i4SensorOpenIndex, const ConfigInfo_T& rConfigInfo)
{
    CAM_LOGD("[%s] i4SensorDevId(%d), i4SensorOpenIndex(%d)\n"
           , __FUNCTION__
           , i4SensorDevId
           , i4SensorOpenIndex);

    Mutex::Autolock lock(m_Lock);

    // check user count
    if (m_Users > 0)
    {
        CAM_LOGD_IF(1,"%d has created \n", m_Users);
        android_atomic_inc(&m_Users);
        return MTRUE;
    }
    m_i4SttPortEnable = property_get_int32("debug.stt_flow.enable", 7);
    //m_i4SensorDev = i4SensorDevId;
    //m_i4SensorIdx = i4SensorOpenIndex;
    m_rConfigInfo = rConfigInfo;

    IAeMgr::getInstance().getTgIntAEInfo(m_i4SensorDev,m_bTgIntAEEn,m_fTgIntAERatio);
    querySensorStaticInfo();
    CAM_LOGD("[%s:TgIntAEInfo] m_i4SensorDev:%d, m_u4TGSizeH:%d, m_bTgIntAEEn:%d, m_fTgIntAERatio:%f\n", __FUNCTION__, m_i4SensorDev, m_u4TGSizeH, (int)m_bTgIntAEEn, m_fTgIntAERatio);

    m_pBufMgrList[BUF_AAO] = IBufMgr::createInstance(BUF_AAO,m_i4SensorDev,m_i4SensorIdx,m_rConfigInfo, m_pCbSet);
    m_pBufMgrList[BUF_AFO] = IBufMgr::createInstance(BUF_AFO,m_i4SensorDev,m_i4SensorIdx,m_rConfigInfo, m_pCbSet);
    m_pBufMgrList[BUF_PSO] = IBufMgr::createInstance(BUF_PSO,m_i4SensorDev,m_i4SensorIdx,m_rConfigInfo, m_pCbSet);

    if(m_bTgIntAEEn)
    {
        MINT32 i4Tglineno = 0;
        i4Tglineno = m_fTgIntAERatio * m_u4TGSizeH;
        m_pBufMgrList[BUF_AEO] = AEOBufMgr::createInstance(m_i4SensorDev,m_i4SensorIdx,i4Tglineno);
    }

    m_pBufMgrList[BUF_FLKO] = IBufMgr::createInstance(BUF_FLKO,m_i4SensorDev,m_i4SensorIdx,m_rConfigInfo, m_pCbSet);
    m_pBufMgrList[BUF_PDO] = IBufMgr::createInstance(BUF_PDO,m_i4SensorDev,m_i4SensorIdx,m_rConfigInfo, m_pCbSet);
    m_pBufMgrList[BUF_MVHDR] = IBufMgr::createInstance(BUF_MVHDR,m_i4SensorDev,m_i4SensorIdx,m_rConfigInfo, m_pCbSet);

    /********************************
     * StatisticPipe init and config
     ********************************/
    {
        CAM_TRACE_BEGIN("3A STT init");
        CAM_LOGD("[%s] Statistic Pipe createInstance\n", __FUNCTION__);
        m_pSttPipe = IStatisticPipe::createInstance(m_i4SensorIdx, LOG_TAG);
        CAM_LOGD("[%s] Statistic Pipe init\n", __FUNCTION__);
        if (MFALSE == m_pSttPipe->init()) {
            CAM_LOGD("IStatisticPipe init fail");
            CAM_TRACE_END();
            return MFALSE;
        }
        CAM_TRACE_END();

        // Check FLK enable or not by normal pipe information
        MBOOL enable_FLK = isFlkEnable();
        CAM_LOGD("[%s] enable_FLK = %d, SensorIdx = %d\n", __FUNCTION__, (int)enable_FLK, m_i4SensorIdx);

        // Check AFO enable or not by normal pipe information
        MBOOL enable_AFO = isAFOEnable();
        CAM_LOGD("[%s] enable_AFO = %d, SensorIdx = %d\n", __FUNCTION__, (int)enable_AFO, m_i4SensorIdx);

        CAM_LOGD("[%s] Statistic Pipe config\n", __FUNCTION__);
        std::vector<statPortInfo> vp;
        QInitStatParam statParm(vp);
        if (m_i4SttPortEnable & ENABLE_STT_FLOW_AAO)
        {
            statParm.mStatPortInfo.push_back(statPortInfo(PORT_AAO));
            statParm.mStatPortInfo.push_back(statPortInfo(PORT_PSO));
        }
        if ((m_i4SttPortEnable & ENABLE_STT_FLOW_AFO) && enable_AFO)
            statParm.mStatPortInfo.push_back(statPortInfo(PORT_AFO));
        if ((m_i4SttPortEnable & ENABLE_STT_FLOW_FLKO) && enable_FLK)
            statParm.mStatPortInfo.push_back(statPortInfo(PORT_FLKO));
        if(m_pPDAFStatus == FEATURE_PDAF_SUPPORT_BNR_PDO)
            statParm.mStatPortInfo.push_back(statPortInfo(PORT_PDO, m_u4PDOSizeW, m_u4PDOSizeH));
        if(m_pPDAFStatus == FEATURE_PDAF_SUPPORT_PBN_PDO)
            statParm.mStatPortInfo.push_back(statPortInfo(PORT_PDO, m_u4PDOSizeW, m_u4PDOSizeH));

        IAfMgr::getInstance().setPdSeparateMode(m_i4SensorDev, PD_SEPARATE_MODE);

        CAM_TRACE_BEGIN("3A STT configPipe");
        m_pSttPipe->configPipe(statParm, m_rConfigInfo.i4SubsampleCount);
        CAM_TRACE_END();
    }

    /**********************************
     * virtual channal init and config
     **********************************/
    if(m_pPDAFStatus == FEATURE_PDAF_SUPPORT_VIRTUAL_CHANNEL ||
       m_pMvHDRStatus == FEATURE_MVHDR_SUPPORT_VIRTUAL_CHANNEL )
    {
        if(m_pPDAFStatus == FEATURE_PDAF_SUPPORT_VIRTUAL_CHANNEL)
            m_pCamsvSttPipe[EPIPE_CAMSV_FEATURE_PDAF] = ICamsvStatisticPipe::createInstance(m_i4SensorIdx, LOG_TAG, EPIPE_CAMSV_FEATURE_PDAF);
        if(m_pMvHDRStatus == FEATURE_MVHDR_SUPPORT_VIRTUAL_CHANNEL)
            m_pCamsvSttPipe[EPIPE_CAMSV_FEATURE_MVHDR] = ICamsvStatisticPipe::createInstance(m_i4SensorIdx, LOG_TAG, EPIPE_CAMSV_FEATURE_MVHDR);

        for(int i = 0; i < EPIPE_CAMSV_FEATURE_NUM; i++)
        {
            if(m_pCamsvSttPipe[i] == NULL)
            {
                CAM_LOGD("ICamsvStatisticPipe[%d] createInstance fail", i);
                continue;
            }

            CAM_TRACE_BEGIN("3A CamsvSTT init");
            CAM_LOGD("[%s] Virtual Channal Pipe(%d) init\n", __FUNCTION__, i);
            if (MFALSE == m_pCamsvSttPipe[i]->init()) {
                CAM_LOGD("ICamsvStatisticPipe[%d] init fail", i);
                m_pCamsvSttPipe[i]->destroyInstance(LOG_TAG);
                m_pCamsvSttPipe[i] = NULL;
                CAM_TRACE_END();
                continue;
            }
            CAM_TRACE_END();

            MBOOL bRet = m_pCamsvSttPipe[i]->sendCommand(ECAMSVSPipeCmd_SET_CAMSV_SENARIO_ID, (MINTPTR)&m_i4SensorMode, NULL,NULL);

            CAM_TRACE_BEGIN("3A CamsvSTT configPipe");
            std::vector<statPortInfo> vp;
            QInitStatParam statParm(vp);
            if(MFALSE == m_pCamsvSttPipe[i]->configPipe(statParm))
            {
                CAM_LOGD("ICamsvStatisticPipe[%d] configPipe fail", i);
                m_pCamsvSttPipe[i]->uninit();
                m_pCamsvSttPipe[i]->destroyInstance(LOG_TAG);
                m_pCamsvSttPipe[i] = NULL;
            }
            CAM_TRACE_END();
        }
    }
    android_atomic_inc(&m_Users);
    return MTRUE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL
Hal3ASttCtrl::
uninitStt()
{
    CAM_LOGD("[%s] \n", __FUNCTION__);

    Mutex::Autolock lock(m_Lock);

    // If no more users, return directly and do nothing.
    if (m_Users <= 0)
    {
        return MTRUE;
    }

    // More than one user, so decrease one User.
    android_atomic_dec(&m_Users);

    if(m_Users == 0)
    {
        // Statistic Pipe uninit
        if(m_pSttPipe != NULL)
        {
            CAM_TRACE_BEGIN("3A STT uninit");
            m_pSttPipe->uninit();
            m_pSttPipe->destroyInstance(LOG_TAG);
            m_pSttPipe = NULL;
            CAM_TRACE_END();
        }

        for(int i = 0; i < EPIPE_CAMSV_FEATURE_NUM; i++)
        {
            if(m_pCamsvSttPipe[i] != NULL)
            {
                // Virtaul channal uninit
                CAM_TRACE_BEGIN("3A CamsvSTT uninit");
                m_pCamsvSttPipe[i]->uninit();
                m_pCamsvSttPipe[i]->destroyInstance(LOG_TAG);
                m_pCamsvSttPipe[i] = NULL;
                CAM_TRACE_END();
            }
        }

        for(int i = 0; i < BUF_NUM; i++)
        {
            if(m_pBufMgrList[i] != NULL)
            {
                m_pBufMgrList[i]->destroyInstance();
                m_pBufMgrList[i] = NULL;
            }
        }
    }
    CAM_LOGD("[%s] done \n", __FUNCTION__);
    return MTRUE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL
Hal3ASttCtrl::
startStt()
{
    CAM_LOGW("[%s] m_i4SttPortEnable = %d \n", __FUNCTION__, m_i4SttPortEnable);

    m_bPreStopSttEnable = 0;

    if(m_pSttPipe != NULL)
        m_pSttPipe->start();

    for(int i = 0; i < EPIPE_CAMSV_FEATURE_NUM; i++)
    {
        if(m_pCamsvSttPipe[i] != NULL)
        {
            CAM_LOGW("[%s] ICamsvStatisticPipe[%d] start\n", __FUNCTION__, i);
            m_pCamsvSttPipe[i]->start();
        }
    }

    CAM_LOGW("[%s] SensorDev=%d\n", __FUNCTION__, m_i4SensorDev);
    char pThreadName[256] = {'\0'};
    if(m_i4SttPortEnable & ENABLE_STT_FLOW_AAO)
    {
        sprintf(pThreadName, "AAOBufThread_%d", m_i4SensorDev);
        std::vector<IBufMgr*> list;
        // add mvHDR
        if(m_pMvHDRStatus != FEATURE_MVHDR_UNSUPPORT && m_pCamsvSttPipe[EPIPE_CAMSV_FEATURE_MVHDR] != NULL)
            list.push_back(m_pBufMgrList[BUF_MVHDR]);
        // add AAO
        list.push_back(m_pBufMgrList[BUF_AAO]);
        // add PSO
        list.push_back(m_pBufMgrList[BUF_PSO]);
        // add FLKO
        if(m_i4SttPortEnable & ENABLE_STT_FLOW_FLKO)
            list.push_back(m_pBufMgrList[BUF_FLKO]);

        rAAOBufThread = ThreadStatisticBuf::createInstance(m_i4SensorDev, pThreadName, list);
    }

    // Check AFO enable or not by normal pipe information
    #if 0 // Doesn't create AFO dequen thread since ISP 5.0
    MBOOL enable_AFO = isAFOEnable();
    CAM_LOGD("[%s] enable_AFO = %d, SensorIdx = %d\n", __FUNCTION__, (int)enable_AFO, m_i4SensorIdx);
    if ((m_i4SttPortEnable & ENABLE_STT_FLOW_AFO) && enable_AFO)
    {
        sprintf(pThreadName, "AFOBufThread_%d", m_i4SensorDev);
        std::vector<IBufMgr*> list;
        // add AF
        list.push_back(m_pBufMgrList[BUF_AFO]);
        rAFOBufThread = ThreadStatisticBuf::createInstance(m_i4SensorDev, pThreadName, list);
    }
    #endif

    if((m_pPDAFStatus == FEATURE_PDAF_SUPPORT_VIRTUAL_CHANNEL && m_pCamsvSttPipe[EPIPE_CAMSV_FEATURE_PDAF] != NULL) || /* CamSV */
       ((m_pPDAFStatus == FEATURE_PDAF_SUPPORT_BNR_PDO || m_pPDAFStatus == FEATURE_PDAF_SUPPORT_PBN_PDO)) || /* PDO*/
       (m_pPDAFStatus == FEATURE_PDAF_SUPPORT_LEGACY)/*raw type PDAF*/)
    {
        sprintf(pThreadName, "PDOBufThread_%d", m_i4SensorDev);
        std::vector<IBufMgr*> list;
        // add PDAF
        list.push_back(m_pBufMgrList[BUF_PDO]);
        rPDOBufThread = ThreadStatisticBuf::createInstance(m_i4SensorDev, pThreadName, list);
    }
    if(m_bTgIntAEEn)
    {   // add AEO
        sprintf(pThreadName, "AEOBufThread_%d", m_i4SensorDev);
        std::vector<IBufMgr*> list;
        list.push_back(m_pBufMgrList[BUF_AEO]);
        rAEOBufThread = ThreadStatisticBuf::createInstance(m_i4SensorDev, pThreadName, list);
    }

    return MTRUE;
}


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL
Hal3ASttCtrl::
preStopStt()
{
    CAM_LOGD("[%s] \n", __FUNCTION__);

    m_bPreStopSttEnable = 1;

    if(rAFOBufThread != NULL)
        rAFOBufThread->waitFinished();
    if(rAAOBufThread != NULL)
        rAAOBufThread->waitFinished();
    if(rPDOBufThread != NULL)
        rPDOBufThread->waitFinished();
    if(rAEOBufThread != NULL)
        rAEOBufThread->waitFinished();

    for(int i = 0; i < BUF_NUM; i++)
    {
        if(m_pBufMgrList[i] != NULL)
        {
            if(i == BUF_MVHDR)
            {
                if(m_pMvHDRStatus != FEATURE_MVHDR_UNSUPPORT && m_pCamsvSttPipe[EPIPE_CAMSV_FEATURE_MVHDR] != NULL)
                    m_pBufMgrList[i]->notifyPreStop();
            } else if(i == BUF_PDO)
            {
                if(m_pPDAFStatus != FEATURE_PDAF_UNSUPPORT)
                    m_pBufMgrList[i]->notifyPreStop();
            } else
                m_pBufMgrList[i]->notifyPreStop();
        }
    }

    if(m_pSttPipe != NULL)
        m_pSttPipe->stop(MTRUE);
    for(int i = 0; i < EPIPE_CAMSV_FEATURE_NUM; i++)
    {
        if(m_pCamsvSttPipe[i] != NULL)
        m_pCamsvSttPipe[i]->stop(MTRUE);
    }

    return MTRUE;
}


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL
Hal3ASttCtrl::
stopStt()
{
    CAM_LOGW("[%s] \n", __FUNCTION__);
    DebugUtil::getInstance(m_i4SensorDev)->update(LOG_TAG, "stopStt", 0);
    if(rAFOBufThread != NULL){
        DebugUtil::getInstance(m_i4SensorDev)->update(LOG_TAG, "stopStt", 1);
        rAFOBufThread->destroyInstance();
        rAFOBufThread = NULL;
    }
    DebugUtil::getInstance(m_i4SensorDev)->update(LOG_TAG, "stopStt", 2);
    if(rAAOBufThread != NULL){
        DebugUtil::getInstance(m_i4SensorDev)->update(LOG_TAG, "stopStt", 3);
        rAAOBufThread->destroyInstance();
        rAAOBufThread = NULL;
    }
    if(rPDOBufThread != NULL){
        rPDOBufThread->destroyInstance();
        rPDOBufThread = NULL;
    }
    DebugUtil::getInstance(m_i4SensorDev)->update(LOG_TAG, "stopStt", 4);
    if(rAEOBufThread != NULL){
        DebugUtil::getInstance(m_i4SensorDev)->update(LOG_TAG, "stopStt", 5);
        rAEOBufThread->destroyInstance();
        rAEOBufThread = NULL;
    }
    DebugUtil::getInstance(m_i4SensorDev)->update(LOG_TAG, "stopStt", -1);
    return MTRUE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// Pasue
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
VOID
Hal3ASttCtrl::
pause()
{
    CAM_LOGW("[%s] \n", __FUNCTION__);
    if(rAFOBufThread != NULL)
        rAFOBufThread->pause();

    if(rAAOBufThread != NULL)
        rAAOBufThread->pause();

    if(rPDOBufThread != NULL)
        rPDOBufThread->pause();

    if(rAEOBufThread != NULL)
        rAEOBufThread->pause();

    for(int i = 0; i < EPIPE_CAMSV_FEATURE_NUM; i++)
    {
        if(m_pCamsvSttPipe[i] != NULL)
            m_pCamsvSttPipe[i]->suspend();
    }

}


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// Pasue
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
VOID
Hal3ASttCtrl::
resume()
{
    CAM_LOGW("[%s] \n", __FUNCTION__);

    if(!m_bPreStopSttEnable)
    {
        for(int i = 0; i < EPIPE_CAMSV_FEATURE_NUM; i++)
        {
            if(m_pCamsvSttPipe[i] != NULL)
                m_pCamsvSttPipe[i]->resume();
        }
    }

    if(rAFOBufThread != NULL)
        rAFOBufThread->resume();

    if(rAAOBufThread != NULL)
        rAAOBufThread->resume();

    if(rPDOBufThread != NULL)
        rPDOBufThread->resume();

    if(rAEOBufThread != NULL)
        rAEOBufThread->resume();
}


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL
Hal3ASttCtrl::
abortDeque()
{
    CAM_LOGW("[%s] \n", __FUNCTION__);
    for(int i = 0; i < BUF_NUM; i++)
    {
        if(m_pBufMgrList[i] != NULL)
        {
            if(i == BUF_MVHDR)
            {
                if(m_pMvHDRStatus != FEATURE_MVHDR_UNSUPPORT && m_pCamsvSttPipe[EPIPE_CAMSV_FEATURE_MVHDR] != NULL)
                    m_pBufMgrList[i]->abortDequeue();
            } else if(i == BUF_PDO)
            {
                if(m_pPDAFStatus != FEATURE_PDAF_UNSUPPORT)
                    m_pBufMgrList[i]->abortDequeue();
            } else
                m_pBufMgrList[i]->abortDequeue();
        }
    }
    return MTRUE;
}


MVOID
Hal3ASttCtrl::
setSensorMode(MINT32 i4SensorMode)
{
    CAM_LOGD("[%s] i4SensorMode(%d)", __FUNCTION__, i4SensorMode);
    m_i4SensorMode = i4SensorMode;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL
Hal3ASttCtrl::
isFlkEnable()
{
    MUINT disable = property_get_int32("debug.aaa_flk.disable", 0);

    if(disable)
        return 0;

    MBOOL uni_info = MFALSE;
    INormalPipe* pCamIO = (INormalPipe*)INormalPipeUtils::get()->createDefaultNormalPipe(m_i4SensorIdx, LOG_TAG);
    if (pCamIO == NULL)
    {
        CAM_LOGE("Fail to create NormalPipe");
        return MFALSE;
    }

    pCamIO->sendCommand(ENPipeCmd_GET_FLK_INFO,(MINTPTR)&uni_info, 0, 0);

    if (pCamIO != NULL)
    {
        pCamIO->destroyInstance(LOG_TAG);
        pCamIO = NULL;
    }

    return uni_info;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL
Hal3ASttCtrl::
isMvHDREnable()
{
    if((m_pMvHDRStatus != FEATURE_MVHDR_UNSUPPORT) &&
       (m_pCamsvSttPipe[EPIPE_CAMSV_FEATURE_MVHDR] != NULL))
        return MTRUE;
    else
        return MFALSE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL
Hal3ASttCtrl::
isAFOEnable()
{
    // Check AFO enable or not by normal pipe information
    MBOOL enable_AFO   = MFALSE;
    MINT32 IsAFSupport = MTRUE;

    IsAFSupport = IAfMgr::getInstance().getAFMaxAreaNum(m_i4SensorDev);

    if (IsAFSupport)
    {
        MUINT32 IsCCUSupport = 0;

        if (IAfMgr::getInstance().sendAFCtrl(m_i4SensorDev, EAFMgrCtrl_IsCCUSupport, m_i4SensorDev, m_i4SensorIdx) > 0)
        {
            IsCCUSupport = 1;
            #if defined(MTKCAM_CCU_AF_SUPPORT)
            NSCcuIf::ICcuCtrlAf* pICcuCtrlAf = NULL;
            pICcuCtrlAf = NSCcuIf::ICcuCtrlAf::createInstance((const uint8_t *)"AFOPort",(ESensorDev_T)m_i4SensorDev);
            if (pICcuCtrlAf->init((MUINT32)m_i4SensorIdx, (ESensorDev_T)m_i4SensorDev) != CCU_CTRL_SUCCEED)
            {
                pICcuCtrlAf->destroyInstance();
                IsCCUSupport = 0;
                pICcuCtrlAf  = NULL;
                CAM_LOGE("[%s] CCU AF init fail", __FUNCTION__);
            }
            else
            {
                pICcuCtrlAf->uninit();
                pICcuCtrlAf->destroyInstance();
                pICcuCtrlAf = NULL;
            }
            #endif
        }

        CAM_LOGD("[%s] IsCCUSupport(%d)", __FUNCTION__, IsCCUSupport);
        enable_AFO = (IsCCUSupport > 0) ? 0 : 1;
    }
    else
    {
        CAM_LOGW("[%s] disable AF", __FUNCTION__);
    }
    /*
        enable_AFO = MTRUE,  Enable AFO port for P1 driver
        enable_AFO = MFALSE, Don't support AF or Use CCU to control statistic data
    */
    return enable_AFO;
}

MVOID
Hal3ASttCtrl::
querySensorStaticInfo()
{
    IHalSensorList* const pIHalSensorList = MAKE_HalSensorList();

    //Before phone boot up (before opening camera), we can query IHalsensor for the sensor static info (EX: MONO or Bayer)
    SensorStaticInfo sensorStaticInfo;
    IHalSensorList*const pHalSensorList = MAKE_HalSensorList();
    IHalSensor* pIHalSensor = pIHalSensorList->createSensor( "sttCtrl", m_i4SensorDev);
    if (!pHalSensorList)
    {
        CAM_LOGE("MAKE_HalSensorList() == NULL");
        return;
    }
    pHalSensorList->querySensorStaticInfo(m_i4SensorDev,&sensorStaticInfo);

    /*0: NO HDR, 1: iHDR, 2:mvHDR, 3:zHDR*/
    if(sensorStaticInfo.HDR_Support == 2)
        m_pMvHDRStatus = FEATURE_MVHDR_SUPPORT_VIRTUAL_CHANNEL;

    if(m_bTgIntAEEn)
    {
        INormalPipe* pPipe = (INormalPipe*)INormalPipeUtils::get()->createDefaultNormalPipe(m_i4SensorIdx, LOG_TAG);
        pPipe->sendCommand( NSCam::NSIoPipe::NSCamIOPipe::ENPipeCmd_GET_TG_OUT_SIZE, (MINTPTR)(&m_u4TGSizeW), (MINTPTR)(&m_u4TGSizeH),0);
        CAM_LOGD( "%s: Dev 0x%04x, TGSZ: W %d, H %d", __FUNCTION__, m_i4SensorDev, m_u4TGSizeW, m_u4TGSizeH);
        pPipe->destroyInstance(LOG_TAG);
    }

    IAfMgr::getInstance().getPdInfoForSttCtrl(m_i4SensorDev, m_i4SensorIdx, m_i4SensorMode,
        m_u4PDOSizeW, m_u4PDOSizeH, m_pPDAFStatus);

    CAM_LOGD("[%s] SensorDev(%d), SensorOpenIdx(%d), mvHDR_Status(%d), SensorMode(%d), PDAFStatus(%d), PDOSizeW(%d PIXELS), PDOSizeH(%d)\n",
        __FUNCTION__, m_i4SensorDev, m_i4SensorIdx, m_pMvHDRStatus, m_i4SensorMode, m_pPDAFStatus, m_u4PDOSizeW, m_u4PDOSizeH);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
IBufMgr*
Hal3ASttCtrl::
getBufMgr(BUF_PORTS port)
{
    if(m_bPreStopSttEnable == 1)
        return NULL;
    else
        return m_pBufMgrList[port];
}

MINT32
Hal3ASttCtrl::
attachCb(I3ACallBack* cb)
{
    CAM_LOGD("[%s] m_pCbSet(0x%p), cb(0x%p)", __FUNCTION__, m_pCbSet, cb);
    m_pCbSet = cb;
    //m_p3AWrap->attachCb(cb);
    return 0;
}

MINT32
Hal3ASttCtrl::
detachCb(I3ACallBack* cb)
{
    CAM_LOGD("[%s] m_pCbSet(0x%p), cb(0x%p)", __FUNCTION__, m_pCbSet, cb);
    m_pCbSet = NULL;
    //m_p3AWrap->detachCb(cb);
    return 0;
}

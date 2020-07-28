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
#include <array>

#include <isp_tuning.h>
#include <aaa_hal_sttCtrl.h>
#include <aaa_types.h>
#include <aaa_error_code.h>
#include <mtkcam/utils/std/Log.h>
#include <mtkcam/drv/IHalSensor.h>
#include <private/aaa_utils.h>

#include <mtkcam/drv/iopipe/CamIO/IHalCamIO.h>
#include <private/IopipeUtils.h>

#include <debug/DebugUtil.h>
#include <cutils/atomic.h>

#include <ae_mgr/ae_mgr_if.h>
#include <flicker_hal_if.h>

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

#define MY_INST NS3Av3::INST_T<Hal3ASttCtrl>
static std::array<MY_INST, SENSOR_IDX_MAX> gMultiton;

Hal3ASttCtrl*
Hal3ASttCtrl::
createInstance(MINT32 i4SensorDevId, MINT32 i4SensorOpenIndex)
{
    CAM_LOGD("Hal3ASttFlow::createInstance i4SensorDevId(%d), i4SensorOpenIndex(%d)\n"
           , i4SensorDevId
           , i4SensorOpenIndex);

    auto i4SensorIdx = mapSensorDevToIdx(i4SensorDevId);

    if(i4SensorIdx >= SENSOR_IDX_MAX || i4SensorIdx < 0) {
        MY_LOGE("[%s] Unsupport sensor Index: %d\n", __FUNCTION__, i4SensorIdx);
        return nullptr;
    }

    MY_INST& rSingleton = gMultiton[i4SensorIdx];
    std::call_once(rSingleton.onceFlag, [&] {
        rSingleton.instance = std::make_unique<Hal3ASttCtrl>();
    } );

    return rSingleton.instance.get();
}

Hal3ASttCtrl*
Hal3ASttCtrl::
getInstance(MINT32 i4SensorDevId)
{
    auto i4SensorIdx = NS3Av3::mapSensorDevToIdx(i4SensorDevId);

    if(i4SensorIdx >= SENSOR_IDX_MAX || i4SensorIdx < 0) {
        MY_LOGE("[%s] Unsupport sensor Index: %d\n", __FUNCTION__, i4SensorIdx);
        return nullptr;
    }

    MY_INST& rSingleton = gMultiton[i4SensorIdx];
    std::call_once(rSingleton.onceFlag, [&] {
        rSingleton.instance = std::make_unique<Hal3ASttCtrl>();
    } );

    return rSingleton.instance.get();
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
getPdInfoForSttCtrl(const ConfigInfo_T& rConfigInfo)
{
    IAfMgr::getInstance().getPdInfoForSttCtrl(m_i4SensorDev, m_i4SensorIdx, m_i4SensorMode,
        m_u4PDOSizeW, m_u4PDOSizeH, m_pPDAFStatus, rConfigInfo);

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
    , m_pPDAFStatus(FEATURE_PDAF_UNSUPPORT)
    , m_pMvHDRStatus(FEATURE_MVHDR_UNSUPPORT)
    , m_bTgIntAEEn(MFALSE)
    , m_fTgIntAERatio(0)
    , m_bPreStopSttEnable(0)
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
    m_i4SttPortEnable = property_get_int32("debug.stt_flow.enable", 15);
    //m_i4SensorDev = i4SensorDevId;
    //m_i4SensorIdx = i4SensorOpenIndex;
    m_rConfigInfo = rConfigInfo;

    IAeMgr::getInstance().getTgIntAEInfo(m_i4SensorDev,m_bTgIntAEEn,m_fTgIntAERatio);
    querySensorStaticInfo(rConfigInfo);
    CAM_LOGD("[%s:TgIntAEInfo] m_i4SensorDev:%d, m_u4TGSizeH:%d, m_bTgIntAEEn:%d, m_fTgIntAERatio:%f\n", __FUNCTION__, m_i4SensorDev, m_u4TGSizeH, (int)m_bTgIntAEEn, m_fTgIntAERatio);


    /********************************
     * StatisticPipe init and config
     ********************************/
    {
        IAfMgr::getInstance().setPdSeparateMode(m_i4SensorDev, PD_SEPARATE_MODE);
    }

    /**********************************
     * virtual channal init and config
     **********************************/
    if(m_pPDAFStatus == FEATURE_PDAF_SUPPORT_VIRTUAL_CHANNEL ||
       m_pMvHDRStatus != FEATURE_MVHDR_UNSUPPORT)
    {

        if(m_pPDAFStatus == FEATURE_PDAF_SUPPORT_VIRTUAL_CHANNEL)
            m_pCamsvSttPipe[EPIPE_CAMSV_FEATURE_PDAF] = ICamsvStatisticPipe::createInstance(m_i4SensorIdx, LOG_TAG, EPIPE_CAMSV_FEATURE_PDAF);
        if(m_pMvHDRStatus == FEATURE_MVHDR_SUPPORT_VIRTUAL_CHANNEL)
            m_pCamsvSttPipe[EPIPE_CAMSV_FEATURE_MVHDR] = ICamsvStatisticPipe::createInstance(m_i4SensorIdx, LOG_TAG, EPIPE_CAMSV_FEATURE_MVHDR);
        else if(m_pMvHDRStatus == FEATURE_MVHDR_SUPPORT_3EXPO_VIRTUAL_CHANNEL)
        {
            m_pCamsvSttPipe[EPIPE_CAMSV_FEATURE_4CELL_MVHDR_EMB] = ICamsvStatisticPipe::createInstance(m_i4SensorIdx, LOG_TAG, EPIPE_CAMSV_FEATURE_4CELL_MVHDR_EMB);
            m_pCamsvSttPipe[EPIPE_CAMSV_FEATURE_4CELL_MVHDR_Y] = ICamsvStatisticPipe::createInstance(m_i4SensorIdx, LOG_TAG, EPIPE_CAMSV_FEATURE_4CELL_MVHDR_Y);
            m_pCamsvSttPipe[EPIPE_CAMSV_FEATURE_4CELL_MVHDR_AE] = ICamsvStatisticPipe::createInstance(m_i4SensorIdx, LOG_TAG, EPIPE_CAMSV_FEATURE_4CELL_MVHDR_AE);
            m_pCamsvSttPipe[EPIPE_CAMSV_FEATURE_4CELL_MVHDR_FLK] = ICamsvStatisticPipe::createInstance(m_i4SensorIdx, LOG_TAG, EPIPE_CAMSV_FEATURE_4CELL_MVHDR_FLK);
        }

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
                CAM_LOGD("ICamsvStatisticPipe[%d] configPipe fail, bRet(%d)", i, bRet);
                m_pCamsvSttPipe[i]->uninit();
                m_pCamsvSttPipe[i]->destroyInstance(LOG_TAG);
                m_pCamsvSttPipe[i] = NULL;
            }
            CAM_TRACE_END();
        }
    }

    // Query TG info, note that Middleware has configured sensor before
    SensorDynamicInfo senInfo;
    senInfo.TgInfo = CAM_TG_1;
    IHalSensorList*const pHalSensorList = MAKE_HalSensorList();
    if (!pHalSensorList)
    {
        CAM_LOGE("MAKE_HalSensorList() == NULL");
    }
    else
    {
        const char* const callerName = "Hal3ASttQueryTG";
        IHalSensor* pHalSensor = pHalSensorList->createSensor(callerName, m_i4SensorIdx);
        pHalSensor->querySensorDynamicInfo(m_i4SensorDev, &senInfo);
        pHalSensor->destroyInstance(callerName);
        CAM_LOGD("[%s] m_i4SensorDev = %d, senInfo.TgInfo = %d\n", __FUNCTION__, m_i4SensorDev, senInfo.TgInfo);
    }

    if ((senInfo.TgInfo != CAM_TG_1) && (senInfo.TgInfo != CAM_TG_2))
    {
        CAM_LOGE("RAW sensor is connected with TgInfo: %d\n", senInfo.TgInfo);
    }
    // Set statistic config info
    STT_CFG_INFO_T sttInfo;
    sttInfo.i4TgInfo = senInfo.TgInfo;
    // Create buffer after virtual channal init and config. Because FLKO need isMvHDREnable info
    m_pBufMgrList[BUF_AAO] = IBufMgr::createInstance(BUF_AAO,m_i4SensorDev,m_i4SensorIdx,m_rConfigInfo, sttInfo);
    m_pBufMgrList[BUF_AFO] = IBufMgr::createInstance(BUF_AFO,m_i4SensorDev,m_i4SensorIdx,m_rConfigInfo, sttInfo);
    m_pBufMgrList[BUF_FLKO] = IBufMgr::createInstance(BUF_FLKO,m_i4SensorDev,m_i4SensorIdx,m_rConfigInfo, sttInfo);
    m_pBufMgrList[BUF_PDO] = IBufMgr::createInstance(BUF_PDO,m_i4SensorDev,m_i4SensorIdx,m_rConfigInfo, sttInfo);

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
    CAM_LOGW("[%s] SensorDev = %d, m_i4SttPortEnable = %d \n", __FUNCTION__, m_i4SensorDev, m_i4SttPortEnable);

    m_bPreStopSttEnable = 0;

    for(int i = 0; i < EPIPE_CAMSV_FEATURE_NUM; i++)
    {
        if(m_pCamsvSttPipe[i] != NULL)
        {
            CAM_LOGW("[%s] ICamsvStatisticPipe[%d] start\n", __FUNCTION__, i);
            m_pCamsvSttPipe[i]->start();
        }
    }

    char pThreadName[256] = {'\0'};
    if(m_i4SttPortEnable & ENABLE_STT_FLOW_AAO)
    {
        sprintf(pThreadName, "AAOBufThread_%d", m_i4SensorDev);
        std::vector<IBufMgr*> list;

        // add AAO
        list.push_back(m_pBufMgrList[BUF_AAO]);
        // add FLKO
        if(m_i4SttPortEnable & ENABLE_STT_FLOW_FLKO)
            list.push_back(m_pBufMgrList[BUF_FLKO]);

        rAAOBufThread = ThreadStatisticBuf::createInstance(m_i4SensorDev, pThreadName, list);
    }

    if((m_pPDAFStatus == FEATURE_PDAF_SUPPORT_VIRTUAL_CHANNEL && m_pCamsvSttPipe[EPIPE_CAMSV_FEATURE_PDAF] != NULL) || /* CamSV */
       ((m_pPDAFStatus == FEATURE_PDAF_SUPPORT_BNR_PDO || m_pPDAFStatus == FEATURE_PDAF_SUPPORT_PBN_PDO)) || /* PDO */
       (m_pPDAFStatus == FEATURE_PDAF_SUPPORT_LEGACY)/*raw type PDAF*/)
    {
        sprintf(pThreadName, "PDOBufThread_%d", m_i4SensorDev);
        std::vector<IBufMgr*> list;
        // add PDAF
        list.push_back(m_pBufMgrList[BUF_PDO]);
        rPDOBufThread = ThreadStatisticBuf::createInstance(m_i4SensorDev, pThreadName, list);
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
    CAM_LOGI("[%s] \n", __FUNCTION__);

    m_bPreStopSttEnable = 1;

    if(rAFOBufThread != NULL)
        rAFOBufThread->waitFinished();
    if(rAAOBufThread != NULL)
        rAAOBufThread->waitFinished();
    if(rPDOBufThread != NULL)
        rPDOBufThread->waitFinished();

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
    DebugUtil::getInstance(m_i4SensorDev)->update(LOG_TAG, "stopStt", 6);
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
            if(i == BUF_PDO)
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
MINT32
Hal3ASttCtrl::
isMvHDREnable()
{
    if((m_pMvHDRStatus != FEATURE_MVHDR_UNSUPPORT) &&
       (m_pCamsvSttPipe[EPIPE_CAMSV_FEATURE_MVHDR] != NULL))
    {
        return FEATURE_MVHDR_SUPPORT_VIRTUAL_CHANNEL;
    }
    else if(m_pMvHDRStatus != FEATURE_MVHDR_UNSUPPORT && m_pCamsvSttPipe[EPIPE_CAMSV_FEATURE_4CELL_MVHDR_EMB] != NULL &&
            m_pCamsvSttPipe[EPIPE_CAMSV_FEATURE_4CELL_MVHDR_Y] != NULL && m_pCamsvSttPipe[EPIPE_CAMSV_FEATURE_4CELL_MVHDR_AE] != NULL &&
            m_pCamsvSttPipe[EPIPE_CAMSV_FEATURE_4CELL_MVHDR_FLK] != NULL)
    {
        return FEATURE_MVHDR_SUPPORT_3EXPO_VIRTUAL_CHANNEL;
    }
    else
    {
        return MFALSE;
    }
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL
Hal3ASttCtrl::
isAFOEnable()
{
    // Check AFO enable
    MBOOL enable_AFO   = MFALSE;
    MINT32 IsAFSupport = MTRUE;

    IsAFSupport = IAfMgr::getInstance().getAFMaxAreaNum(m_i4SensorDev);

    if (IsAFSupport)
    {
        enable_AFO = MTRUE;
        CAM_LOGW("[%s] enable AF", __FUNCTION__);
    }
    else
    {
        CAM_LOGW("[%s] disable AF", __FUNCTION__);
    }

    return enable_AFO;
}

MVOID
Hal3ASttCtrl::
querySensorStaticInfo(const ConfigInfo_T& rConfigInfo)
{
    //Before phone boot up (before opening camera), we can query IHalsensor for the sensor static info (EX: MONO or Bayer)
    IHalSensorList*const pHalSensorList = MAKE_HalSensorList();
    if (!pHalSensorList)
    {
        CAM_LOGE("MAKE_HalSensorList() == NULL");
        return;
    }
    SensorStaticInfo sensorStaticInfo;
    const char* const callerName = "sttCtrl";
    IHalSensor* pHalSensor = pHalSensorList->createSensor(callerName, m_i4SensorIdx);
    pHalSensorList->querySensorStaticInfo(m_i4SensorDev,&sensorStaticInfo);
    pHalSensor->destroyInstance(callerName);

    /*0: NO HDR, 1: iHDR, 2:mvHDR, 3:zHDR, 4:four-cell mVHDR*/
    if(sensorStaticInfo.HDR_Support == 2)
        m_pMvHDRStatus = FEATURE_MVHDR_SUPPORT_VIRTUAL_CHANNEL;
    else if(sensorStaticInfo.HDR_Support == 4)
        m_pMvHDRStatus = FEATURE_MVHDR_SUPPORT_3EXPO_VIRTUAL_CHANNEL;

    /* IAfMgr::getInstance().getPdInfoForSttCtrl(m_i4SensorDev, m_i4SensorIdx, m_i4SensorMode,
        m_u4PDOSizeW, m_u4PDOSizeH, m_pPDAFStatus, rConfigInfo); */

    CAM_LOGD("[%s] SensorDev(%d), SensorOpenIdx(%d), mvHDR_Status(%d), SensorMode(%d), PDAFStatus(%d), PDOSizeW(%d), PDOSizeH(%d)\n",
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


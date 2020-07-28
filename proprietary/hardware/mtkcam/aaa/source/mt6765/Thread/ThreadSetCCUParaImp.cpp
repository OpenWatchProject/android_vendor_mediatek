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
#define LOG_TAG "ThreadSetCCUPara"

#include <IThread.h>
#include <IEventIrq.h>

#include <mtkcam/def/PriorityDefs.h>
#include <pthread.h>
#include <semaphore.h>
#include <sys/prctl.h>
#include <sys/resource.h>

#include <cutils/properties.h>
#include <utils/threads.h>
#include <utils/List.h>
#include <mtkcam/utils/std/Log.h>
#include <aaa_error_code.h>

#include <ae_mgr/ae_mgr.h>
#include <isp_tuning/isp_tuning_mgr.h>
#include <isp_mgr/isp_mgr.h>
#include <debug/DebugUtil.h>
//#include <private/IopipeUtils.h> //isp_reg.h


#if CAM3_LSC_FEATURE_EN
#include <lsc/ILscMgr.h>
#endif

//CmdQ
#if defined(MTKCAM_CMDQ_AA_SUPPORT)
#include <list>
#include <drv/cq_tuning_mgr.h>
#endif


#define GET_PROP(prop, init, val)\
{\
    char value[PROPERTY_VALUE_MAX] = {'\0'};\
    property_get(prop, value, (init));\
    (val) = atoi(value);\
}

using namespace NSIspTuningv3;

namespace NS3Av3
{
using namespace android;

class ThreadSetCCUParaImp : public ThreadSetCCUPara
{
public:
    virtual MBOOL               destroyInstance();
    virtual MBOOL               postCmd(void* pArg);
    virtual MBOOL               waitFinished();

                                ThreadSetCCUParaImp(Hal3ARaw* pHal3A, MUINT32 u4SensorDev, MINT32 i4SensorIndex);
    virtual                     ~ThreadSetCCUParaImp();
private:
    static MVOID*               onThreadLoop(MVOID*);
    MVOID                       changeThreadSetting();
    MBOOL                       getCmd(ThreadSetCCUPara::Cmd_T& rCmd);
    MBOOL                       notifyResult4CCU(ThreadSetCCUParaImp::Cmd_T rCmd);
    MBOOL                       setCCUParams(MUINT32 i4PreFrmId);

    MBOOL                       m_fgLogEn;
    MBOOL                       m_fgdisableThread;
    MBOOL                       m_bDbgInfoEnable;
    MUINT32                     m_u4SensorDev;
    MINT32                      m_i4SensorIndex;
    IEventIrq*                  m_pEvent;
    pthread_t                   m_rThread;
    List<ThreadSetCCUPara::Cmd_T>  m_rCmdQ;
    Mutex                       m_ModuleMtx;
    Condition                   m_rCmdQCond;
    Hal3ARaw*                   m_pHal3A;
#if defined(MTKCAM_CMDQ_AA_SUPPORT)
    CqTuningMgr*                m_pCqTuningMgr;
#endif
};

ThreadSetCCUPara*
ThreadSetCCUPara::
createInstance(Hal3ARaw* pHal3A, MUINT32 u4SensorDev, MINT32 i4SensorIndex)
{
    ThreadSetCCUParaImp* pObj = new ThreadSetCCUParaImp(pHal3A, u4SensorDev, i4SensorIndex);
    return pObj;
}

MBOOL
ThreadSetCCUParaImp::
destroyInstance()
{
    m_fgdisableThread = 1;
    delete this;
    return MTRUE;
}

MBOOL
ThreadSetCCUParaImp::
postCmd(void* pArg)
{
    Mutex::Autolock autoLock(m_ModuleMtx);

    ThreadSetCCUPara::Cmd_T rCmd;
    // data clone
    if (pArg != NULL)
        rCmd = *reinterpret_cast<ThreadSetCCUPara::Cmd_T*>(pArg);

    m_rCmdQ.push_back(rCmd);
    m_rCmdQCond.broadcast();

    CAM_LOGD_IF(m_fgLogEn, "[%s] m_rCmdQ size(%d), PreMagicReq(%d), rPreResult Addr(%p)",
        __FUNCTION__, (MINT32)m_rCmdQ.size(), rCmd.u4MagicNumReq, rCmd.rPreResult);

    return MTRUE;
}

MBOOL
ThreadSetCCUParaImp::
getCmd(ThreadSetCCUPara::Cmd_T& rCmd)
{
    Mutex::Autolock autoLock(m_ModuleMtx);

    if (m_rCmdQ.size() == 0)
    {
        m_rCmdQCond.wait(m_ModuleMtx);
    }

    rCmd = *m_rCmdQ.begin();
    CAM_LOGD_IF(m_fgLogEn, "[%s] m_rCmdQ size(%d), PreMagicReqs(%d), rPreResult Addr(%p)",
        __FUNCTION__, (MINT32)m_rCmdQ.size(), rCmd.u4MagicNumReq, rCmd.rPreResult);

    m_rCmdQ.erase(m_rCmdQ.begin());

    if (m_fgdisableThread)
    {
        return MFALSE;
    }

    return MTRUE;
}

MBOOL
ThreadSetCCUParaImp::
waitFinished()
{
    return MTRUE;
}

ThreadSetCCUParaImp::
ThreadSetCCUParaImp(Hal3ARaw* pHal3A, MUINT32 u4SensorDev, MINT32 i4SensorIndex)
    : m_fgLogEn(0)
    , m_fgdisableThread(0)
    , m_u4SensorDev(u4SensorDev)
    , m_i4SensorIndex(i4SensorIndex)
    , m_pEvent(NULL)
    , m_rThread()
    , m_pHal3A(pHal3A)
{
    CAM_LOGD("[%s] sensor(%d), sensoridx(%d)", __FUNCTION__, u4SensorDev, i4SensorIndex);
    // init something
    MUINT32 u4LogEn = 0;
    GET_PROP("vendor.debug.ThreadCCU.log", "0", u4LogEn);
    m_fgLogEn = u4LogEn ? 1 : 0;

    MBOOL dbgInfoEnable = 1;
#if (!IS_BUILD_USER)
    dbgInfoEnable = 1;
#else
    dbgInfoEnable = 0;
#endif
    //GET_PROP("debug.camera.dbginfo", dbgInfoEnable, m_bDbgInfoEnable);
    m_bDbgInfoEnable = ::property_get_int32("debug.camera.dbginfo", dbgInfoEnable);

    // create thread
    ::pthread_create(&m_rThread, NULL, onThreadLoop, this);
}

ThreadSetCCUParaImp::
~ThreadSetCCUParaImp()
{
    CAM_LOGD("[%s +] sensor(%d)", __FUNCTION__, m_u4SensorDev);
    // destroy thread
    postCmd(NULL);
    ::pthread_join(m_rThread, NULL);
    CAM_LOGD("[%s -] sensor(%d)", __FUNCTION__, m_u4SensorDev);
}

MVOID
ThreadSetCCUParaImp::
changeThreadSetting()
{
    // (1) set name
    ::prctl(PR_SET_NAME, "ThreadSetCCUPara", 0, 0, 0);
    //
    struct sched_param sched_p;
    ::sched_getparam(0, &sched_p);

    // (2) set policy/priority
#if 0
    int const policy    = SCHED_RR;
    int const priority  = PRIO_RT_3A_THREAD;
    //  set
    sched_p.sched_priority = priority;  //  Note: "priority" is real-time priority.
    ::sched_setscheduler(0, policy, &sched_p);
    //  get
    ::sched_getparam(0, &sched_p);
#else
    int const policy    = SCHED_OTHER;
    int const priority  = NICE_CAMERA_CCU;
    //  set
    sched_p.sched_priority = priority;  //  Note: "priority" is nice value.
    ::sched_setscheduler(0, policy, &sched_p);
    ::setpriority(PRIO_PROCESS, 0, priority);
    //  get
    sched_p.sched_priority = ::getpriority(PRIO_PROCESS, 0);
#endif
    //
    CAM_LOGD(
        "[%s] sensor(%d), tid(%d), policy:(expect, result)=(%d, %d), priority:(expect, result)=(0x%08x, 0x%08x)", __FUNCTION__
        , m_u4SensorDev, ::gettid()
        , policy, ::sched_getscheduler(0)
        , priority, sched_p.sched_priority
    );
}

MVOID*
ThreadSetCCUParaImp::
onThreadLoop(MVOID* pArg)
{
    CAM_LOGD("[%s] +", __FUNCTION__);
    ThreadSetCCUParaImp* _this = reinterpret_cast<ThreadSetCCUParaImp*>(pArg);
    MUINT32 u4SensorDev = _this->m_u4SensorDev;
    ThreadSetCCUPara::Cmd_T rCmd;

    _this->changeThreadSetting();

    while (_this->getCmd(rCmd))
    {
        if(rCmd.u4MagicNumReq != 0)
        {
            //1. get AE info and set AE info to ISP(AE_mgr)
            //2. update TSF ratio
            NSIspTuningv3::IspTuningMgr::getInstance().sendIspTuningIOCtrl(u4SensorDev, NSIspTuningv3::IspTuningMgr::E_ISPTUNING_UPDATE_LSC_INFO, NSIspTuningv3::IspTuningMgr::E_SHADING_RATIO_FREE_RUN, 0);
            //3. upadte result
            _this->notifyResult4CCU(rCmd);
            //4. update driver
            _this->setCCUParams(rCmd.u4MagicNumReq);
        }
    }

    CAM_LOGD("[%s] -", __FUNCTION__);

    return NULL;
}

MBOOL
ThreadSetCCUParaImp::
notifyResult4CCU(ThreadSetCCUParaImp::Cmd_T rCmd)
{
    CAM_LOGD_IF(m_fgLogEn, "[%s] Previous FrmId(%d) rPreResult Addr(%p) +", __FUNCTION__, rCmd.u4MagicNumReq, rCmd.rPreResult);
    // set AE info to previous result Q

    rCmd.rPreResult->i8SensorExposureTime = rCmd.rAESensorInfo.u8ExposureTime;
    rCmd.rPreResult->i8SensorFrameDuration = rCmd.rAESensorInfo.u8FrameDuration;
    rCmd.rPreResult->i4SensorSensitivity = rCmd.rAESensorInfo.u4Sensitivity;//ISO value updated

    // set new LSC ratio to previous result Q
    NSIspTuning::ILscMgr* pLsc = NSIspTuning::ILscMgr::getInstance(static_cast<ESensorDev_T>(m_u4SensorDev));
    MUINT32 u4Rto = pLsc->getRatio();
    if(rCmd.rPreResult->vecLscData.size() != 0)
    {
        MUINT8* pDest = rCmd.rPreResult->vecLscData.editArray();
        *((MUINT32*)(pDest+sizeof(ILscTable::Config))) = u4Rto;
    }

    // set data to exif
    CAM_LOGD_IF(m_fgLogEn, "[%s] u4Rto(%d), i8SensorExposureTime(%ld), i8SensorFrameDuration(%ld), i4SensorSensitivity(%d)", __FUNCTION__,
        (MINT32)u4Rto, (long)rCmd.rPreResult->i8SensorExposureTime, (long)rCmd.rPreResult->i8SensorFrameDuration,rCmd.rPreResult->i4SensorSensitivity);

    CAM_LOGD_IF(m_fgLogEn, "[%s] vecExifInfo.size(%d), vecDbg3AInfo.size(%d), vecDbgIspInfo.size(%d), m_rParam.u1IsGetExif(%d), bIsCapEnd(%d)", __FUNCTION__,
        (MINT32)rCmd.rPreResult->vecExifInfo.size(), (MINT32)rCmd.rPreResult->vecDbg3AInfo.size(), (MINT32)rCmd.rPreResult->vecDbgIspInfo.size(), rCmd.u1IsGetExif, rCmd.bIsCapEnd);

    RAWIspCamInfo rTempCamInfo;

    if (!IspTuningMgr::getInstance().getCamInfo(m_u4SensorDev, rTempCamInfo))
    {
        CAM_LOGE("Fail to get CamInfo");
    }
    else
    {
        rCmd.rPreResult->rCamInfo.rAEInfo = rTempCamInfo.rAEInfo;
    }


    if (rCmd.u1IsGetExif || rCmd.bIsCapEnd)
    {
        if( rCmd.rPreResult->vecExifInfo.size() >= 1)// && rCmd.rPreResult->vecDbg3AInfo.size() >= sizeof(AAA_DEBUG_INFO1_T) && rCmd.rPreResult->vecDbgIspInfo.size() >= sizeof(AAA_DEBUG_INFO2_T))
        {
            // standard exif
            m_pHal3A->send3ACtrl(E3ACtrl_GetExifInfo, (MINTPTR)(&(rCmd.rPreResult->vecExifInfo.editTop())), NULL);
        }

        if (m_bDbgInfoEnable) {
            if (rCmd.rPreResult->vecDbg3AInfo.size() >= sizeof(AAA_DEBUG_INFO1_T) && rCmd.rPreResult->vecDbgIspInfo.size() >= sizeof(AAA_DEBUG_INFO2_T))
            {
                // debug exif
                AAA_DEBUG_INFO1_T& rDbg3AInfo1 = *reinterpret_cast<AAA_DEBUG_INFO1_T*>(rCmd.rPreResult->vecDbg3AInfo.editArray());
                AAA_DEBUG_INFO2_T& rDbg3AInfo2 = *reinterpret_cast<AAA_DEBUG_INFO2_T*>(rCmd.rPreResult->vecDbgIspInfo.editArray());
                NSIspExifDebug::IspExifDebugInfo_T& rDbgIspInfo = rDbg3AInfo2.rISPDebugInfo;

                AE_PLINE_DEBUG_INFO_T tmpBuff;
                // AE
                AE_MODE_CFG_T rPreviewInfo;
                IAeMgr::getInstance().getPreviewParams(m_u4SensorDev, rPreviewInfo);
                CAM_LOGD_IF(m_fgLogEn, "[%s] (EXP,Afe,Ispgain,ISO)(%d, %d, %d, %d) -> (%d, %d, %d, %d)", __FUNCTION__,
                            rDbg3AInfo1.rAEDebugInfo.Tag[AE_TAG_PRV_SHUTTER_TIME].u4FieldValue, rDbg3AInfo1.rAEDebugInfo.Tag[AE_TAG_PRV_SENSOR_GAIN].u4FieldValue, rDbg3AInfo1.rAEDebugInfo.Tag[AE_TAG_PRV_ISP_GAIN].u4FieldValue, rDbg3AInfo1.rAEDebugInfo.Tag[AE_TAG_PRV_ISO_REAL].u4FieldValue,
                            rPreviewInfo.u4Eposuretime, rPreviewInfo.u4AfeGain, rPreviewInfo.u4IspGain, rPreviewInfo.u4RealISO);
                rDbg3AInfo1.rAEDebugInfo.Tag[AE_TAG_PRV_SHUTTER_TIME].u4FieldValue = rPreviewInfo.u4Eposuretime;
                rDbg3AInfo1.rAEDebugInfo.Tag[AE_TAG_PRV_SENSOR_GAIN].u4FieldValue = rPreviewInfo.u4AfeGain;
                rDbg3AInfo1.rAEDebugInfo.Tag[AE_TAG_PRV_ISP_GAIN].u4FieldValue = rPreviewInfo.u4IspGain;
                rDbg3AInfo1.rAEDebugInfo.Tag[AE_TAG_PRV_ISO_REAL].u4FieldValue = rPreviewInfo.u4RealISO;

                //Avoid memory copy for AE debug info
                //IAeMgr::getInstance().getDebugInfo(m_u4SensorDev, rDbg3AInfo1.rAEDebugInfo, tmpBuff);

#if CAM3_LSC_FEATURE_EN
                // LSC
                //Avoid NE for LSC debug info
                //ILscMgr::getInstance(static_cast<ESensorDev_T>(m_u4SensorDev))->getDebugInfo(rDbg3AInfo1.rShadigDebugInfo);
                rDbg3AInfo1.rShadigDebugInfo.Tag[SHAD_TAG_CAM_LSC_RATIO].u4FieldValue = u4Rto;
#endif

                // P1 ISP
                MUINT32 u4RtoOffset0 = ((MUINT32)offsetof(cam_reg_t, CAM_LSC_RATIO_0))/4;
                *(rDbgIspInfo.P1RegInfo.regData+u4RtoOffset0) = (u4Rto<<24)|(u4Rto<<16)|(u4Rto<<8)|u4Rto;

                MUINT32 u4RtoOffset1 = ((MUINT32)offsetof(cam_reg_t, CAM_LSC_RATIO_1))/4;
                *(rDbgIspInfo.P1RegInfo.regData+u4RtoOffset1) = (u4Rto<<24)|(u4Rto<<16)|(u4Rto<<8)|u4Rto;

                CAM_LOGD_IF(m_fgLogEn, "[%s] Offset0 = %d, Offset1 = %d", __FUNCTION__, u4RtoOffset0, u4RtoOffset1);

            }
        }

        CAM_LOGD_IF(m_fgLogEn, "[%s] Condition for vecExifInfo.size %d, vecDbg3AInfo.size (%d,%d), vecDbgIspInfo.size (%d,%d)"
                             , __FUNCTION__
                             , rCmd.rPreResult->vecExifInfo.size()
                             , rCmd.rPreResult->vecDbg3AInfo.size(), sizeof(AAA_DEBUG_INFO1_T)
                             , rCmd.rPreResult->vecDbgIspInfo.size(), sizeof(AAA_DEBUG_INFO2_T));
    }

    CAM_LOGD_IF(m_fgLogEn, "[%s] Previous FrmId(%d) rPreResult Addr(%p) rPreResult i4FrmId(%d) -", __FUNCTION__, rCmd.u4MagicNumReq, rCmd.rPreResult, rCmd.rPreResult->i4FrmId);
    return MTRUE;
}

MBOOL
ThreadSetCCUParaImp::
setCCUParams(MUINT32 i4PreFrmId)
{
    CAM_LOGD_IF(m_fgLogEn, "[%s] +", __FUNCTION__);

    MUINT32 u4MagicNumReq = i4PreFrmId;

    // 1. get ratio
    NSIspTuning::ILscMgr* pLsc = NSIspTuning::ILscMgr::getInstance(static_cast<ESensorDev_T>(m_u4SensorDev));
    MUINT32 u4Rto = (MINT32)pLsc->getRatio();

    // 1.1 updateTSF
    CAM_LOGD_IF(m_fgLogEn, "[%s] PreMagicReq(%d), u4Rto(%d)", __FUNCTION__, u4MagicNumReq, u4Rto);
    pLsc->updateRatio(u4MagicNumReq, u4Rto);//request magicnum, ratio

    // 2. call normalpipe API to set obc gain & ratio
    // CmdQ
#if defined(MTKCAM_CMDQ_AA_SUPPORT)
    CAM_LOGD_IF(m_fgLogEn, "[%s] Set ratio to HW by CMDQ", __FUNCTION__);
    m_pCqTuningMgr = (CqTuningMgr*)CqTuningMgr::getInstance(m_i4SensorIndex);
    std::list<ECQ_TUNING_USER> module_en;
    CQ_TUNING_CMD_ST cmd;
    //Ratio
    CQ_TUNING_MGR_WRITE_REG(m_pCqTuningMgr, CAM_LSC_RATIO_0, ((u4Rto<<24)|(u4Rto<<16)|(u4Rto<<8)|u4Rto));
    CQ_TUNING_MGR_WRITE_REG(m_pCqTuningMgr, CAM_LSC_RATIO_1, ((u4Rto<<24)|(u4Rto<<16)|(u4Rto<<8)|u4Rto));

    module_en.push_back(EAA_TUNING_LSC);
    cmd.module = &module_en;
    cmd.serial = (u4MagicNumReq);
    m_pCqTuningMgr->update(cmd);
#endif

    CAM_LOGD_IF(m_fgLogEn, "[%s] -", __FUNCTION__);

    return MTRUE;
}



};

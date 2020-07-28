/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein is
 * confidential and proprietary to MediaTek Inc. and/or its licensors. Without
 * the prior written permission of MediaTek inc. and/or its licensors, any
 * reproduction, modification, use or disclosure of MediaTek Software, and
 * information contained herein, in whole or in part, shall be strictly
 * prohibited.
 *
 * MediaTek Inc. (C) 2010. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER
 * ON AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL
 * WARRANTIES, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR
 * NONINFRINGEMENT. NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH
 * RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY,
 * INCORPORATED IN, OR SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES
 * TO LOOK ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO.
 * RECEIVER EXPRESSLY ACKNOWLEDGES THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO
 * OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES CONTAINED IN MEDIATEK
 * SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE
 * RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S
 * ENTIRE AND CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE
 * RELEASED HEREUNDER WILL BE, AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE
 * MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE
 * CHARGE PAID BY RECEIVER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek
 * Software") have been modified by MediaTek Inc. All revisions are subject to
 * any receiver's applicable license agreements with MediaTek Inc.
 */

#ifdef LOG_TAG
#undef LOG_TAG
#endif
#define LOG_TAG     "NormalSvPipe"

#include <sys/prctl.h>
#include <utils/Vector.h>
#include <utils/KeyedVector.h>
using namespace android;

#include <mtkcam/def/common.h>
#include <mtkcam/utils/std/common.h>
#include <mtkcam/utils/metadata/client/mtk_metadata_tag.h>
using namespace NSCam;
using namespace NSCam::Utils;

// for thread priority
#include <mtkcam/def/PriorityDefs.h>

#include <utils/Thread.h>
#include <utils/Condition.h>
#include <cutils/properties.h>

#include <IPipe.h>
#include <ICamIOPipe.h>
#include <ispio_pipe_ports.h>
#include <Cam_Notify_datatype.h>
#include <Cam/ICam_capibility.h>

#include <mtkcam/drv/IHalSensor.h>

#include <mtkcam/drv/iopipe/CamIO/INormalPipe.h>

#include "IOPipeProfile.h"
#include "NormalSvPipe.h"
#include "camio_log.h"
#include "FakeSensor.h"
//for N3D
#include <mtkcam/drv/IHwSyncDrv.h>

#undef   DBG_LOG_TAG                        // Decide a Log TAG for current file.
#ifndef USING_MTK_LDVT   // Not using LDVT.
#define  DBG_LOG_TAG        ""
#else
#define  DBG_LOG_TAG        LOG_TAG
#endif
#define PTHREAD_CTRL_M      (1)

#define THIS_NAME           "NormalSvPipe"

using namespace NSCam;
using namespace NSIoPipe;
using namespace NSCamIOPipe;
using namespace NSImageio;
using namespace NSIspio;

DECLARE_DBG_LOG_VARIABLE(ionsvpipe);
#undef PIPE_VRB
#undef PIPE_DBG
#undef PIPE_INF
#undef PIPE_WRN
#undef PIPE_ERR
#undef PIPE_AST
#define PIPE_VRB(fmt, arg...)        do { if (ionsvpipe_DbgLogEnable_VERBOSE) { BASE_LOG_VRB(fmt, ##arg); } } while(0)
#define PIPE_DBG(fmt, arg...)        do { if (ionsvpipe_DbgLogEnable_DEBUG  ) { BASE_LOG_DBG(fmt, ##arg); } } while(0)
#define PIPE_INF(fmt, arg...)        do { if (ionsvpipe_DbgLogEnable_INFO   ) { BASE_LOG_INF(fmt, ##arg); } } while(0)
#define PIPE_WRN(fmt, arg...)        do { if (ionsvpipe_DbgLogEnable_WARN   ) { BASE_LOG_WRN(fmt, ##arg); } } while(0)
#define PIPE_ERR(fmt, arg...)        do { if (ionsvpipe_DbgLogEnable_ERROR  ) { BASE_LOG_ERR(fmt, ##arg); } } while(0)
#define PIPE_AST(cond, fmt, arg...)  do { if (ionsvpipe_DbgLogEnable_ASSERT ) { BASE_LOG_AST(cond, fmt, ##arg); } } while(0)

#include "NormalSvPipeDef.h"

/******************************************************************************
 *
 ******************************************************************************/

Mutex                     NormalSvPipe::NSvPipeGLock;
Mutex                     NormalSvPipe::NSvPipeCfgGLock;
Mutex                     NormalSvPipe::NSvPipeDbgLock;
NormalSvPipe*             NormalSvPipe::pNormalSvPipe[EPIPE_Sensor_RSVD] = {NULL, NULL, NULL, NULL};
platSensorsInfo*          NormalSvPipe::pAllSensorInfo = NULL;
MUINT32                   NormalSvPipe::mEnablePath = 0;
NormalSvPipe::_MemInfo_t  NormalSvPipe::mMemInfo;

#if (CONFIG_IOPIPE_PROFILING == 1)
MUINT32                   IOPipeTrace::m_traceEnable = MFALSE;
MUINT32                   IOPipeTrace::m_metPrfEnable = MFALSE;
MUINT32                   IOPipeTrace::m_userCount = 0;
Mutex                     IOPipeTrace::m_traceLock;
#endif

/******************************************************************************
 *
 ******************************************************************************/
NormalSvPipe::NormalSvPipe(
    MUINT32                         _SensorIdx,
    char const*                     _szCallerName
    )
    : mpHalSensor(NULL)
    , mBurstQNum(0)
    , mRawType(0)
    , mpCamIOPipe(NULL)
    #ifdef USE_IMAGEBUF_HEAP
    , mpFrameMgr(NULL)
    #endif
    , mpName(_szCallerName)
    , mTotalUserCnt(0)
    , mpSensorIdx(_SensorIdx)
    , mSenPixMode(ePixMode_NONE)
    , m_FSM(op_unknown)
    , mInitSettingApplied(MFALSE)
    , mDynSwtRawType(MTRUE)
    , mPureRaw(MFALSE)
    , m_bN3DEn(MFALSE)
    , mResetCount(0)
    , mOpenedPort(0)
    , mPrvEnqSOFIdx(0xFFFFFFFF)
    , mReqDepthPreStart(0)
    , m_b1stEnqLoopDone(MFALSE)
    , mpNSvPipeThread(NULL)
    , m_DropCB(NULL)
    , m_returnCookie(NULL)
    , m_SofCB(NULL)
    , m_SofCBCookie(NULL)
    , mpIMem(NULL)
{
    DBG_LOG_CONFIG(imageio, ionsvpipe);

    for (MUINT32 i = 0; i < ePort_max; i++) {
        PortImgFormat[i] = eImgFmt_UNKNOWN;
    }

    memset((void*)mpEnqueRequest, 0, sizeof(mpEnqueRequest));
    memset((void*)mpEnqueReserved, 0, sizeof(mpEnqueReserved));
    memset((void*)mpDeQueue, 0, sizeof(mpDeQueue));
    memset((void*)PortIndex, 0, sizeof(PortIndex));
    memset((void*)mUserCnt, 0, sizeof(mUserCnt));
    memset((void*)mUserName, 0, sizeof(mUserName));
    memset((void*)mImgHdrIdx, 0, sizeof(mImgHdrIdx));
    memset((void*)mImgHdrCnt, 0, sizeof(mImgHdrCnt));
    memset((void*)mCfgFmt, 0, sizeof(mCfgFmt));
    memset(&mMemInfo, 0, sizeof(mMemInfo));
}

static const NSImageio::NSIspio::EPortIndex m_Slot2PortIndex[] = {
    EPortIndex_IMGO
};

/******************************************************************************
 *
 ******************************************************************************/

extern "C"
INormalPipe* createInstance_INormalSvPipe(MUINT32 SensorIndex, char const* szCallerName)
{
    NormalSvPipe* pNSvPipe = NULL;

    PIPE_DBG("+");

    NormalSvPipe::NSvPipeGLock.lock();

    if (NormalSvPipe::pAllSensorInfo == NULL) {
        IHalSensorList *mSList = NULL;
        _sensorInfo_t  *pSInfo = NULL;
        MUINT32 SCnt = 0, i = 0;

        MINT32 retTrace = IOPipeTrace::createTrace();
        if (retTrace < 0) {
            PIPE_ERR("enable trace fail: ret: %d", retTrace);
        }
        else if (retTrace > 0) {
            PIPE_INF("enable trace: ret: 0x%x", retTrace);
        }

        NSVPIPE_MEM_NEW(NormalSvPipe::pAllSensorInfo, platSensorsInfo, sizeof(platSensorsInfo), NormalSvPipe::mMemInfo);
        PIPE_INF("N:%d,%s,pAllSensorInfo=0x%p", SensorIndex, szCallerName, NormalSvPipe::pAllSensorInfo);

        #if 1
        if (strcmp(szCallerName, "iopipeUseTM") == 0) {
            mSList = TS_FakeSensorList::getTestModel();
        }
        else {
            mSList = IHalSensorList::get();
        }
        #else
        mSList = IHalSensorList::get();
        #endif

        //#ifdef USING_MTK_LDVT
        //mSList->searchSensors();
        //#endif
        SCnt =  mSList->queryNumberOfSensors();

        NormalSvPipe::pAllSensorInfo->mSList          = mSList;
        NormalSvPipe::pAllSensorInfo->mExistedSCnt    = SCnt;
        NormalSvPipe::pAllSensorInfo->mUserCnt        = 0;
        if ((SCnt > IOPIPE_MAX_SENSOR_CNT) || (SCnt == 0)) {
            PIPE_ERR("Not support %d sensors", SCnt);
            NSVPIPE_MEM_DEL(NormalSvPipe::pAllSensorInfo, sizeof(platSensorsInfo), NormalSvPipe::mMemInfo);
            NormalSvPipe::pAllSensorInfo = NULL;
            NormalSvPipe::NSvPipeGLock.unlock();
            return MFALSE;
        }

        for (i = 0; i < SCnt; i++) {
            pSInfo = &NormalSvPipe::pAllSensorInfo->mSenInfo[i];

            pSInfo->mIdx       = i;
            pSInfo->mTypeforMw = mSList->queryType(i);
            pSInfo->mDevId     = mSList->querySensorDevIdx(i);
            mSList->querySensorStaticInfo(pSInfo->mDevId, &pSInfo->mSInfo);
            PIPE_INF("N:%d,SensorName=%s,Type=%d,DevId=%d", i, mSList->queryDriverName(i),\
                    pSInfo->mTypeforMw, pSInfo->mDevId);
        }

        PIPE_DBG("Attach seninf dump callback");
        SENINF_DBG::m_fp_Sen_Camsv = NormalSvPipe::dumpSeninfDebugCB;
    }
    NormalSvPipe::pAllSensorInfo->mUserCnt++;

    NormalSvPipe::NSvPipeGLock.unlock();

    if (SensorIndex >= EPIPE_Sensor_RSVD) {
        PIPE_ERR("InvalidSensorIdx = %d", SensorIndex);
        return MFALSE;
    }

    NormalSvPipe::NSvPipeGLock.lock();
    pNSvPipe = NormalSvPipe::pNormalSvPipe[SensorIndex];
    if (NULL == pNSvPipe) {
        NSVPIPE_MEM_NEW(NormalSvPipe::pNormalSvPipe[SensorIndex], NormalSvPipe(SensorIndex, THIS_NAME), sizeof(NormalSvPipe), NormalSvPipe::mMemInfo);
        pNSvPipe = NormalSvPipe::pNormalSvPipe[SensorIndex];

        PIPE_INF("%s NSvPipe[%d]=0x%p create", szCallerName, SensorIndex, pNSvPipe);

        if (pNSvPipe->mpNSvPipeThread == NULL){
            pNSvPipe->mpNSvPipeThread = NormalSvPipe_Thread::createInstance(pNSvPipe);
            if (NULL == pNSvPipe->mpNSvPipeThread){
                PIPE_ERR("error: Thread:createinstance fail");
                return NULL;
            }
        }
    }

    pNSvPipe->addUser(szCallerName); //pNSvPipe->mUserCnt++;

    NormalSvPipe::NSvPipeGLock.unlock();

    PIPE_DBG("- NSvPipe[%d](0x%p) mUserCnt(%d)", SensorIndex, pNSvPipe, pNSvPipe->mTotalUserCnt);

    return pNSvPipe;
}

/******************************************************************************
 *
 ******************************************************************************/

MVOID NormalSvPipe::dumpSeninfDebugCB(MUINT32 ispModule, MUINT32 tgStat)
{
#define NSVPIPE_DBG_USER      "NSvPipeDbg"
    MUINT32 idx = 0;
    MUINT32 camTg = CAM_TG_NONE;
    INormalPipe *pNSvPipeObj = NULL;

    NormalSvPipe::NSvPipeDbgLock.lock();

    switch (ispModule) {
    case CAMSV_0:
        camTg = CAM_SV_1;
        break;
    case CAMSV_1:
        camTg = CAM_SV_2;
        break;
    case CAMSV_2:
        camTg = CAM_SV_3;
        break;
    case CAMSV_3:
        camTg = CAM_SV_4;
        break;
    case CAMSV_4:
        camTg = CAM_SV_5;
        break;
    case CAMSV_5:
        camTg = CAM_SV_6;
        break;
    default:
        PIPE_ERR("ERROR: unknow isp hw module : %d, dump all", ispModule);
        NormalSvPipe::NSvPipeDbgLock.unlock();

        dumpSeninfDebugCB(CAMSV_0, tgStat);
        dumpSeninfDebugCB(CAMSV_1, tgStat);
        dumpSeninfDebugCB(CAMSV_2, tgStat);
        dumpSeninfDebugCB(CAMSV_3, tgStat);
        dumpSeninfDebugCB(CAMSV_4, tgStat);
        dumpSeninfDebugCB(CAMSV_5, tgStat);
        return;
    }

    for (idx = 0; idx < EPIPE_Sensor_RSVD; idx++) {
        if (_NORMALSVPIPE_GET_TG_IDX(idx) == camTg) {
            break;
        }
    }
    if (EPIPE_Sensor_RSVD == idx) {
        PIPE_ERR("ERROR: unknow tg index : %d", camTg);
        goto _EXIT_DUMP;
    }

    if (NormalSvPipe::pNormalSvPipe[idx]) {
        MINT32 seninfStatus = 0;

        PIPE_WRN("Start dump seninf debug info: sensor_idx:%d sensor_devid:x%x",
                    idx, NormalSvPipe::pAllSensorInfo->mSenInfo[idx].mDevId);
        seninfStatus = NormalSvPipe::pNormalSvPipe[idx]->mpHalSensor->sendCommand(NormalSvPipe::pAllSensorInfo->mSenInfo[idx].mDevId,
                                                                SENSOR_CMD_DEBUG_P1_DQ_SENINF_STATUS, 1, 0, 0);
        if ((1 == tgStat) || (0 != seninfStatus)) {
            NormalSvPipe::pNormalSvPipe[idx]->mResetCount = MAX_P1_RST_COUNT;
        }
    }
    else {
        PIPE_ERR("ERROR: NormalPipe[%d] object not create yet", idx);
        goto _EXIT_DUMP;
    }

_EXIT_DUMP:
    NormalSvPipe::NSvPipeDbgLock.unlock();
}


/******************************************************************************
 *
 ******************************************************************************/

MVOID NormalSvPipe::destroyInstance(char const* szCallerName)
{
    MUINT32 idx = 0;

    PIPE_DBG("+ : %s", szCallerName);

    NormalSvPipe::NSvPipeGLock.lock();
    for (idx = 0; idx < EPIPE_Sensor_RSVD; idx++){
        if (this == NormalSvPipe::pNormalSvPipe[idx]) {
            break;
        }
    }

    if (EPIPE_Sensor_RSVD == idx) {
        PIPE_ERR("Can't find the entry");
        NormalSvPipe::NSvPipeGLock.unlock();
        return;
    }

    if ((mTotalUserCnt <= 0)){
        PIPE_DBG("No instance now");
        NormalSvPipe::NSvPipeGLock.unlock();
        return;
    }

    if (!delUser(szCallerName)) {
        PIPE_ERR("No such user : %s", szCallerName);
        NormalSvPipe::NSvPipeGLock.unlock();
        return;
    }

    if (mTotalUserCnt == 0)  {
        if (mpHalSensor) {
            mpHalSensor->destroyInstance(THIS_NAME);
            mpHalSensor = NULL;
        }

        if (mpNSvPipeThread) {
            mpNSvPipeThread->destroyInstance();
            mpNSvPipeThread = NULL;
        }

        if (mpCamIOPipe) {
            mpCamIOPipe->destroyInstance();
            mpCamIOPipe = NULL;
        }

        if (mpIMem) {
            for (MUINT32 PlaneID = 0; PlaneID <= (MUINT32)ePlane_3rd; PlaneID++) {
                if (mImgHeaderPool.ImemBufInfo[PlaneID].virtAddr) {
                    if (mpIMem->unmapPhyAddr(&mImgHeaderPool.ImemBufInfo[PlaneID])) {
                        PIPE_ERR("imgheader unmap error %" PRIXPTR "", mImgHeaderPool.ImemBufInfo[PlaneID].phyAddr);
                    }
                    if (mpIMem->freeVirtBuf(&mImgHeaderPool.ImemBufInfo[PlaneID], MFALSE)) {
                        PIPE_ERR("imgheader free error %" PRIXPTR "", mImgHeaderPool.ImemBufInfo[PlaneID].virtAddr);
                    }
                    mImgHeaderPool.ImemBufInfo[PlaneID].virtAddr = 0;
                    mImgHeaderPool.ImemBufInfo[PlaneID].phyAddr = 0;
                }
            }
            mpIMem->uninit();
            mpIMem->destroyInstance();
            mpIMem = NULL;
        }

        PIPE_INF("Destroy NSvPipe[%d]=0x%p", idx, NormalSvPipe::pNormalSvPipe[idx]);
        NSVPIPE_MEM_DEL(NormalSvPipe::pNormalSvPipe[idx], sizeof(NormalSvPipe), NormalSvPipe::mMemInfo);
    }

    if (NormalSvPipe::pAllSensorInfo) {
       if (NormalSvPipe::pAllSensorInfo->mUserCnt > 0) {
           NormalSvPipe::pAllSensorInfo->mUserCnt--;
       }

       if (NormalSvPipe::pAllSensorInfo->mUserCnt == 0) {
           PIPE_INF("Destroy pAllSensorInfo. idx=%d", idx);
           NSVPIPE_MEM_DEL(NormalSvPipe::pAllSensorInfo, sizeof(platSensorsInfo), NormalSvPipe::mMemInfo);
           PIPE_DBG("Detach seninf dump callback (Alloc:0x%x, Free:0x%x)", mMemInfo.NSvPipeAllocMemSum, mMemInfo.NSvPipeFreedMemSum);
           SENINF_DBG::m_fp_Sen_Camsv = NULL;

           if (IOPipeTrace::destroyTrace() < 0) {
               PIPE_ERR("destroyTrace failed...");
           }
       }

    }

    NormalSvPipe::NSvPipeGLock.unlock();

    PIPE_DBG("-");
}

/******************************************************************************
 *
 ******************************************************************************/
MBOOL NormalSvPipe::start()
{
    IOPipeTrace _traceObj("NormalSvPipe::start", IOPIPE_TRACE_EN_NORMALSVPIPE);
    Mutex::Autolock lock(mCfgLock);

    MUINT32 _size = 0;

    PIPE_DBG("E:");

    if (this->FSM_CHECK(op_start, __FUNCTION__) == MFALSE) {
        return MFALSE;
    }

    /*
     * Consume 1st element, which alread applied in immediate request
     */
    if (MTRUE == mInitSettingApplied) {
        if (mpNSvPipeThread->m_ConsumedReqCnt >= mReqDepthPreStart) {
            PIPE_DBG("N:%d static loop done 0(%d)", mpSensorIdx, mpNSvPipeThread->m_ConsumedReqCnt);
            m_b1stEnqLoopDone = MTRUE;
        }
    }
    else {
        PIPE_ERR("no enque/p1_update before start");
        return MFALSE;
    }

    /* start: CamIo -> NPipeThread to prevent timing issue below */
    /* start: NPipeThread -> CamIo may happen enqueRequest run during CamIo start when performance low */
    if (mpCamIOPipe) {
        if (MFALSE == mpCamIOPipe->start()) {
            PIPE_ERR("N:%d start fail", mpSensorIdx);
            return MFALSE;
        }
    }

    if (mpNSvPipeThread->start() == MFALSE) {
        PIPE_ERR("error: NPipeThread start fail\n");
        return MFALSE;
    }

    PIPE_DBG("X:");

    this->FSM_UPDATE(op_start);

    return MTRUE;
}


/******************************************************************************
 *
 ******************************************************************************/
MBOOL NormalSvPipe::stop(MBOOL bNonblocking)
{
    IOPipeTrace _traceObj("NormalSvPipe::stop", IOPIPE_TRACE_EN_NORMALSVPIPE);
    Mutex::Autolock lock(mCfgLock);

    bool ret = MTRUE;

    if (this->FSM_CHECK(op_stop, __FUNCTION__) == MFALSE) {
        return MFALSE;
    }

#if 0
    if(this->m_bN3DEn == MTRUE){
        HWSyncDrv* pHwSync = NULL;
        pHwSync = HWSyncDrv::createInstance();
        if(pHwSync->sendCommand(HW_SYNC_CMD_DISABLE_SYNC, \
                HW_SYNC_BLOCKING_EXE, pAllSensorInfo->mSenInfo[mpSensorIdx].mDevId,0) != 0){
                PIPE_ERR("N3D stop fail\n");
        }
        pHwSync->destroyInstance();
        this->m_bN3DEn = MFALSE;
    }
#endif

    PIPE_INF("N:%d stop+ %s", mpSensorIdx, (bNonblocking? "Non-blocking": "Blocking"));

    if (bNonblocking) {
        NormalSvPipe_Thread::E_ThreadGrop thdGrp;

        switch(this->m_FSM) {
        case op_start:
            thdGrp = (NormalSvPipe_Thread::E_ThreadGrop)(NormalSvPipe_Thread::eThdGrp_Internal|NormalSvPipe_Thread::eThdGrp_External);
            break;
        case op_suspend:
            thdGrp = NormalSvPipe_Thread::eThdGrp_External;
            break;
        default:
            PIPE_ERR("Unknown state: %d\n", this->m_FSM);
            return MFALSE;;
        }

        /* stop isp hw first */
        if (mpCamIOPipe) {
            if (mpNSvPipeThread == 0) {
                PIPE_ERR("mpNPipeThread is null");
            } else {
                /* hold idle lock */
                mpNSvPipeThread->idleStateHold(1, thdGrp);
            }

            PIPE_TRACE_BEGIN("TAG_CAM_STOP");
            ret = mpCamIOPipe->stop(MFALSE);
            PIPE_TRACE_END("TAG_CAM_STOP");

            if (mpNSvPipeThread) {
                this->mpNSvPipeThread->prestop();

                /* release idle lock */
                mpNSvPipeThread->idleStateHold(0,
                    (NormalSvPipe_Thread::E_ThreadGrop)(NormalSvPipe_Thread::eThdGrp_Internal|NormalSvPipe_Thread::eThdGrp_External));
            }
        }

        if (mpNSvPipeThread) {
            if (mpNSvPipeThread->stop(MTRUE) == MFALSE) {
                PIPE_ERR("error: NPipeThread stop fail");
                return MFALSE;
            }
        }
    } else {
        if (this->m_FSM == op_suspend) {
            mCfgLock.unlock();

            PIPE_INF("Blocking stop at suspending, resume first");

            this->resume((MUINT64)0);

            mCfgLock.lock();
        }

        if (mpNSvPipeThread) {
            if (mpNSvPipeThread->stop() == MFALSE) {
                PIPE_ERR("error: NPipeThread stop fail");
                return MFALSE;
            }
        }

        if (mpCamIOPipe) {
            PIPE_TRACE_BEGIN("TAG_CAM_STOP");
            ret = mpCamIOPipe->stop(MFALSE);
            PIPE_TRACE_END("TAG_CAM_STOP");
        }
    }

    PIPE_DBG("-");

    this->FSM_UPDATE(op_stop);

    return ret;
}

/******************************************************************************
 *
 ******************************************************************************/
MBOOL NormalSvPipe::abort()
{
    IOPipeTrace _traceObj("NormalSvPipe::abort", IOPIPE_TRACE_EN_NORMALSVPIPE);
    Mutex::Autolock lock(mCfgLock);

    bool ret = MTRUE;

    if (this->FSM_CHECK(op_stop, __FUNCTION__) == MFALSE) {
        return MFALSE;
    }

    PIPE_INF("+ N:%d", mpSensorIdx);

    NormalSvPipe_Thread::E_ThreadGrop thdGrp;

    switch(this->m_FSM) {
    case op_start:
        thdGrp = (NormalSvPipe_Thread::E_ThreadGrop)(NormalSvPipe_Thread::eThdGrp_Internal|NormalSvPipe_Thread::eThdGrp_External);
        break;
    case op_suspend:
        thdGrp = NormalSvPipe_Thread::eThdGrp_External;
        break;
    default:
        PIPE_ERR("Unknown state: %d\n", this->m_FSM);
        return MFALSE;;
    }

    /* stop isp hw first */
    if (mpCamIOPipe) {
        if (mpNSvPipeThread == 0) {
            PIPE_ERR("mpNPipeThread is null");
        } else {
            /* hold idle lock */
            mpNSvPipeThread->idleStateHold(1, thdGrp);
        }

        PIPE_TRACE_BEGIN("TAG_CAM_STOP");
        ret = mpCamIOPipe->stop(MTRUE);/*force stop*/
        PIPE_TRACE_END("TAG_CAM_STOP");

        if (mpNSvPipeThread) {
            this->mpNSvPipeThread->prestop();

            /* release idle lock */
            mpNSvPipeThread->idleStateHold(0,
                (NormalSvPipe_Thread::E_ThreadGrop)(NormalSvPipe_Thread::eThdGrp_Internal|NormalSvPipe_Thread::eThdGrp_External));
        }
    }

    if (mpNSvPipeThread) {
        if (mpNSvPipeThread->stop(MTRUE) == MFALSE) {
            PIPE_ERR("error: NPipeThread stop fail");
            return MFALSE;
        }
    }

#if 0
    if(this->m_bN3DEn == MTRUE){
        HWSyncDrv* pHwSync = NULL;
        pHwSync = HWSyncDrv::createInstance();
        if(pHwSync->sendCommand(HW_SYNC_CMD_DISABLE_SYNC, \
                HW_SYNC_BLOCKING_EXE, pAllSensorInfo->mSenInfo[mpSensorIdx].mDevId,0) != 0){
                PIPE_ERR("N3D stop fail\n");
        }
        pHwSync->destroyInstance();
        this->m_bN3DEn = MFALSE;
    }
#endif

    PIPE_INF("- N:%d", mpSensorIdx);

    this->FSM_UPDATE(op_stop);

    return ret;
}


/******************************************************************************
 *
 ******************************************************************************/
MBOOL NormalSvPipe::init(MBOOL EnableSec)
{
    IOPipeTrace _traceObj("NormalSvPipe::init", IOPIPE_TRACE_EN_NORMALSVPIPE);
    MUINT32     i, j;
    MBOOL       ret = MTRUE;

    if (this->FSM_CHECK(op_init, __FUNCTION__) == MFALSE) {
        return MFALSE;
    }

    PIPE_INF("+ SenIdx=%d", mpSensorIdx);

    mpIMem = IMemDrv::createInstance();
    if (mpIMem) {
        ret = mpIMem->init();
    }
    if ((NULL == mpIMem) || (MFALSE == ret)) {
        PIPE_ERR("imem create/init fail %p, %d", mpIMem, ret);
    }

    /*
     * NormalSvPipe thread init
     */
    if (mpNSvPipeThread) {
        if (mpNSvPipeThread->init() == MFALSE) {
            PIPE_ERR("error: NPipeThread init fail\n");
            return MFALSE;
        }
    }

    #ifdef USE_IMAGEBUF_HEAP
    if (!mpFrameMgr) {
        NSVPIPE_MEM_NEW(mpFrameMgr,FrameMgr(),sizeof(FrameMgr), NormalSvPipe::mMemInfo);
    }
    mpFrameMgr->init();
    #endif

    for (i = 0; i < ePort_max; i++) {
        if (!mpEnqueRequest[i]) {
            NSVPIPE_MEM_NEW(mpEnqueRequest[i], QueueMgr<QBufInfo>(), sizeof(QueueMgr<QBufInfo>), NormalSvPipe::mMemInfo);
            mpEnqueRequest[i]->init();
        }
        if (!mpEnqueReserved[i]) {
            NSVPIPE_MEM_NEW(mpEnqueReserved[i], QueueMgr<QBufInfo>(), sizeof(QueueMgr<QBufInfo>), NormalSvPipe::mMemInfo);
            mpEnqueReserved[i]->init();
        }
        if (!mpDeQueue[i]) {
            NSVPIPE_MEM_NEW(mpDeQueue[i], QueueMgr<QBufInfo>(), sizeof(QueueMgr<QBufInfo>), NormalSvPipe::mMemInfo);
            mpDeQueue[i]->init();
        }
    }

    PIPE_INF("-");

    this->FSM_UPDATE(op_init);

    return ret;
}


/******************************************************************************
 *
 ******************************************************************************/
MBOOL NormalSvPipe::uninit()
{
    IOPipeTrace _traceObj("NormalSvPipe::uninit", IOPIPE_TRACE_EN_NORMALSVPIPE);
    MUINT32 i;
    MBOOL   ret = MTRUE;

    if (this->FSM_CHECK(op_uninit, __FUNCTION__) == MFALSE) {
        return MFALSE;
    }

    PIPE_INF("+ SenIdx=%d", mpSensorIdx);

    //Mutex::Autolock lock(mCfgLock);
    if (mpNSvPipeThread) {
        if(mpNSvPipeThread->uninit() == MFALSE){
            PIPE_ERR("error NPipeThread uninit fail\n");
            return MFALSE;
        }
    }

    if (mpCamIOPipe)  {
        ret = mpCamIOPipe->uninit();
    }

    if (mpCamIOPipe)  {
        Mutex::Autolock lock(NormalSvPipe::NSvPipeCfgGLock);

        switch (_NORMALSVPIPE_GET_TG_IDX(mpSensorIdx)) {
        case CAM_SV_1:
            NormalSvPipe::mEnablePath &= ~NSVPIPE_CAMSV_1;
            break;
        case CAM_SV_2:
            NormalSvPipe::mEnablePath &= ~NSVPIPE_CAMSV_2;
            break;
        case CAM_SV_3:
            NormalSvPipe::mEnablePath &= ~NSVPIPE_CAMSV_3;
            break;
        case CAM_SV_4:
            NormalSvPipe::mEnablePath &= ~NSVPIPE_CAMSV_4;
            break;
        case CAM_SV_5:
            NormalSvPipe::mEnablePath &= ~NSVPIPE_CAMSV_5;
            break;
        case CAM_SV_6:
            NormalSvPipe::mEnablePath &= ~NSVPIPE_CAMSV_6;
            break;
        default:
            PIPE_ERR("unknown TG");
            break;
        }
    }

    NormalSvPipe::NSvPipeGLock.lock();
    if (NormalSvPipe::pAllSensorInfo) {
        NormalSvPipe::pAllSensorInfo->mSenInfo[mpSensorIdx].mOccupiedOwner = (MUINT32)NULL;
    }
    else {
        PIPE_ERR("error:NormalPipe::uninit: pAllSensorInfo NULL");
    }
    NormalSvPipe::NSvPipeGLock.unlock();

    #ifdef USE_IMAGEBUF_HEAP
    if (mpFrameMgr)  {
        mpFrameMgr->uninit();
        NSVPIPE_MEM_DEL(mpFrameMgr,sizeof(FrameMgr), NormalSvPipe::mMemInfo);
    }
    #endif

    for (i = 0; i < ePort_max; i++) {
        if (mpEnqueRequest[i])  {
            mpEnqueRequest[i]->uninit();
            NSVPIPE_MEM_DEL(mpEnqueRequest[i],sizeof(QueueMgr<QBufInfo>), NormalSvPipe::mMemInfo);
        }
        if (mpEnqueReserved[i])  {
            mpEnqueReserved[i]->uninit();
            NSVPIPE_MEM_DEL(mpEnqueReserved[i],sizeof(QueueMgr<QBufInfo>), NormalSvPipe::mMemInfo);
        }
        if (mpDeQueue[i])  {
            mpDeQueue[i]->uninit();
            NSVPIPE_MEM_DEL(mpDeQueue[i],sizeof(QueueMgr<QBufInfo>), NormalSvPipe::mMemInfo);
        }

        mpImgHeaderMgr[i].uninit();
    }

    PIPE_DBG("-");

    this->FSM_UPDATE(op_uninit);

    return ret;
}


/******************************************************************************
 *
 ******************************************************************************/
MBOOL NormalSvPipe::enque(QBufInfo const& rQBuf)
{
    IOPipeTrace _traceObj("NormalSvPipe::enque", IOPIPE_TRACE_EN_NORMALSVPIPE);
    MBOOL       ret = MTRUE;
    MUINT32     i = 0, _port = 0, PlaneID = 0, PlaneMax = 0;

    Mutex::Autolock lock(mEnQLock);

    PIPE_DBG("+");

    mEnqContainerLock.lock();

    for (i = 0; i < rQBuf.mvOut.size(); i++) {
            if (NULL == rQBuf.mvOut.at(i).mBuffer->getBufPA(ePlane_1st)) {
                PIPE_ERR("Port(%d) Enque PA is NULL!", rQBuf.mvOut.at(i).mPortID.index);
                ret = MFALSE;
                goto _enque_exit;
            }
    }

    if (this->FSM_CHECK(op_streaming, __FUNCTION__) == MFALSE) {
        ret = MFALSE;
        goto _enque_exit;
    }

    ret = this->acceptEnqRequest(rQBuf);
    if (ret == MFALSE) {
        goto _enque_exit;
    }

    if (this->m_FSM == op_cfg) {
        MUINT32 minDepth = 0xFFFFFFFF, _size = 0, _OpenedPort = 0;

        _OpenedPort = GetOpenedPortInfo(NormalSvPipe::_GetPort_OpendNum);

        for (i = 0; i < _OpenedPort; i++) {
            _port = NSVPIPE_MAP_PORTID(GetOpenedPortInfo(NormalSvPipe::_GetPort_Index, i));

            mpEnqueRequest[_port]->sendCmd(QueueMgr<QBufInfo>::eCmd_getsize, (MUINTPTR)&_size, 0);

            if (minDepth > _size) {
                minDepth = _size;
            }
        }
        mReqDepthPreStart = minDepth;

        if (this->mInitSettingApplied == MFALSE) {
            PIPE_INF("N:%d immediateEnque", this->mpSensorIdx);
            mpNSvPipeThread->frameToEnque();

            this->mInitSettingApplied = MTRUE;
        }
    }

_enque_exit:
    mEnqContainerLock.unlock();

    PIPE_DBG("-");

    return ret;
}

/*****************************************************************************
*
******************************************************************************/
MBOOL NormalSvPipe::acceptEnqRequest(QBufInfo const& rQBuf)
{
    MUINT32     i = 0, _port = 0;
    QBufInfo    vPortQBufSlot[ePort_max];
    BufInfo*    pBuf;
    char        str[256] = {'\0'};
    MUINT32     strLeng = sizeof(str) - 1, totalStrLeng = 0;

    if (checkEnque(rQBuf) == MFALSE) {
        PIPE_ERR("enque fail\n");
        return MFALSE;
    }

    for (i = 0; i < rQBuf.mvOut.size(); i++) {
        QBufInfo *pSlot = NULL;

        _port = NSVPIPE_MAP_PORTID(rQBuf.mvOut.at(i).mPortID.index);

        pSlot = &(vPortQBufSlot[_port]);
        if (pSlot->mvOut.size() == 0) {
            pSlot->mvOut.reserve(mBurstQNum);
        }
        pSlot->mvOut.push_back(rQBuf.mvOut.at(i));
    }

    for (_port = 0; _port < ePort_max; _port++) { //check enque buf num for each port
        if (vPortQBufSlot[_port].mvOut.size() == 0) {
            continue;
        }
        else if (vPortQBufSlot[_port].mvOut.size() != mBurstQNum) {
            PIPE_ERR("enque buf num not sync: %d/%d", (MUINT32)vPortQBufSlot[_port].mvOut.size(), mBurstQNum);
            return MFALSE;
        }
        vPortQBufSlot[_port].mShutterTimeNs = rQBuf.mShutterTimeNs;
    }

    if (checkDropEnque(vPortQBufSlot) == MFALSE) {
        return MFALSE;
    }

    mpCamIOPipe->sendCommand(EPIPECmd_SET_FRM_TIME, (MINTPTR)(rQBuf.mShutterTimeNs/1000000), 1, 0);
    mpNSvPipeThread->updateFrameTime((MUINT32)(rQBuf.mShutterTimeNs/1000000), 1);

    for (_port = 0; _port < ePort_max; _port++) {
        if (vPortQBufSlot[_port].mvOut.size() == 0) {
            continue;
        }

        mpEnqueRequest[_port]->sendCmd(QueueMgr<QBufInfo>::eCmd_push, (MUINTPTR)&vPortQBufSlot[_port], 0);
        for (i = 0; i < vPortQBufSlot[_port].mvOut.size(); i++) {
            pBuf = &(vPortQBufSlot[_port].mvOut.at(i));

            char _tmp[128];
            MUINT32 curLeng = snprintf(_tmp,sizeof(_tmp),"dma:x%x pa(0x%" PRIXPTR ")io(%d_%d_%d_%d_%d_%d)M:x%x,RawType(%d)",
                    pBuf->mPortID.index, pBuf->mBuffer->getBufPA(ePlane_1st),
                    pBuf->FrameBased.mCropRect.p.x,pBuf->FrameBased.mCropRect.p.y,
                    pBuf->FrameBased.mCropRect.s.w,pBuf->FrameBased.mCropRect.s.h,
                    pBuf->FrameBased.mDstSize.w,pBuf->FrameBased.mDstSize.h,
                    pBuf->FrameBased.mMagicNum_tuning,pBuf->mRawOutFmt);
            if(strLeng - totalStrLeng < curLeng){
                PIPE_INF("MyEnq:%d: %s", mpSensorIdx, str);
                str[0] = '\0';
                totalStrLeng = 0;
            }
            strncat(str, _tmp, (strLeng - totalStrLeng));
            totalStrLeng += curLeng;
        }
    }
    /* for log reduction */
    PIPE_INF("MyEnq:%d: %s", mpSensorIdx, str);

    return MTRUE;
}

MBOOL NormalSvPipe::checkEnque(QBufInfo const& rQBuf)
{
    MBOOL                   ret = MTRUE;
    MUINT32                 i = 0, _size;
    MSize                   _tgsize;
    EImageFormat            _ImgFmt;
    E_CamPixelMode          _PixMode;
    ENPipeQueryCmd          _op = ENPipeQueryCmd_X_PIX;
    NormalPipe_InputInfo    qryInput;
    NormalPipe_QueryInfo    queryRst;
    PortInfo                PortInfo;

    PIPE_DBG("+");

    if (MTRUE == rQBuf.mvOut.empty())  {
        PIPE_ERR("error:queue empty");
        return MFALSE;
    }

    _size = rQBuf.mvOut.size();
    _tgsize = mTgOut_size[(MUINT32)NSVPIPE_MAP_CAM_TG(mpSensorIdx)];

    if (mBurstQNum && (_size % mBurstQNum)) {
        PIPE_ERR("enque buf cnt is not multiple of mBurstQNum %d/%d", _size, mBurstQNum);
        return MFALSE;
    }

    //IO checking
    for (i = 0; i < _size; i++) {
        MINT ImgFormat      = rQBuf.mvOut.at(i).mBuffer->getImgFormat();
        PortInfo.index      = rQBuf.mvOut.at(i).mPortID.index;
        qryInput.format     = (EImageFormat)NSVPIPE_MAP_FMT(PortInfo.index, (EImageFormat)GetOpenedPortInfo(NormalSvPipe::_GetPort_ImgFmt, NSVPIPE_MAP_PORTID(PortInfo.index)));
        qryInput.width      = rQBuf.mvOut.at(i).FrameBased.mDstSize.w;
        qryInput.pixelMode  = mSenPixMode;

        switch (PortInfo.index) {
            case EPortIndex_IMGO:
                _op = (_tgsize.w != qryInput.width)? (ENPipeQueryCmd_CROP_X_PIX): (ENPipeQueryCmd_X_PIX);
                break;
            default:
                PIPE_ERR("Unsupported port_%d", PortInfo.index);
                break;
        }

        if (0xFFFF != rQBuf.mvOut.at(i).mBufIdx) {
            PIPE_ERR("replace buffer with the same address r not supported, plz make sure mBufIdx=0xffff");
            return MFALSE;
        }
        if (0xFFFF == rQBuf.mvOut.at(i).FrameBased.mSOFidx) {
            PIPE_ERR("buf has invalid sof idx(%x)", rQBuf.mvOut.at(i).FrameBased.mSOFidx);
            return MFALSE;
        }

        if ((rQBuf.mvOut.at(i).FrameBased.mDstSize.w == 0) || (rQBuf.mvOut.at(i).FrameBased.mDstSize.h == 0)) {
            PIPE_ERR("error: Port(%d) out size is 0 (0x%x_0x%x)", PortInfo.index, rQBuf.mvOut.at(i).FrameBased.mDstSize.w,
                rQBuf.mvOut.at(i).FrameBased.mDstSize.h);
            return MFALSE;
        }
        if ((_tgsize.w < rQBuf.mvOut.at(i).FrameBased.mDstSize.w) || (_tgsize.h < rQBuf.mvOut.at(i).FrameBased.mDstSize.h)) {
            PIPE_ERR("error: Port(%d) out size(0x%x_0x%x) > tg size(0x%x_0x%x)", PortInfo.index,\
                    rQBuf.mvOut.at(i).FrameBased.mDstSize.w, rQBuf.mvOut.at(i).FrameBased.mDstSize.h, _tgsize.w, _tgsize.h);
            return MFALSE;
        }
        if ((rQBuf.mvOut.at(i).FrameBased.mDstSize.w > rQBuf.mvOut.at(i).FrameBased.mCropRect.s.w) ||\
                (rQBuf.mvOut.at(i).FrameBased.mDstSize.h > rQBuf.mvOut.at(i).FrameBased.mCropRect.s.h)) {
            PIPE_ERR("error: Port(%d) out size(0x%x_0x%x) > crop size(0x%x_0x%x)", PortInfo.index,\
                    rQBuf.mvOut.at(i).FrameBased.mDstSize.w, rQBuf.mvOut.at(i).FrameBased.mDstSize.h,\
                    rQBuf.mvOut.at(i).FrameBased.mCropRect.s.w, rQBuf.mvOut.at(i).FrameBased.mCropRect.s.h);
            return MFALSE;
        }

        if (NSVPIPE_GET_CAPIBILITY(_op, qryInput, PortInfo) == MFALSE) {
            return MFALSE;
        }
    }

    PIPE_DBG("-");

    return ret;
}

/******************************************************************************
 *
 ******************************************************************************/
MBOOL NormalSvPipe::checkDropEnque(QBufInfo *pQBuf)
{
    MUINT32     _port = 0, i = 0, j = 0, _sof_idx = 0, _size, PLANE_ID = 0;
    QBufInfo    _popQBuf;

    PIPE_DBG("+");

    if (this->m_FSM == op_cfg) {
        return MTRUE;;
    }

    if (MFALSE == m_b1stEnqLoopDone) {
        PIPE_ERR("N:%d static enque loop not done", mpSensorIdx);
        return MFALSE;
    }

    //this->sendCommand(EPIPECmd_GET_CUR_FRM_STATUS,(MINTPTR)&_drop_status,0,0);
    sendCommand(ENPipeCmd_GET_CUR_SOF_IDX, (MINTPTR)&_sof_idx, 0, 0);

    //to chk if enque twice within the the frame
    //drop previous enque request and push this request into enque container
    if ((this->m_FSM == op_start) && (_sof_idx == mPrvEnqSOFIdx)) {
        vector<MUINT32> vMnum;
        MBOOL bFirst = 1, nDrop = 0;
        for (_port = 0; _port < ePort_max; _port++) {
            if (pQBuf[_port].mvOut.size() == 0) {
                continue;
            }

            for (i = 0, nDrop = 0; i < mBurstQNum; i++) {
                mpEnqueRequest[_port]->sendCmd(QueueMgr<QBufInfo>::eCmd_getsize, (MUINTPTR)&_size, 0);
                if (_size) {
                    mpEnqueRequest[_port]->sendCmd(QueueMgr<QBufInfo>::eCmd_at, _size-1, (MUINTPTR)&_popQBuf);
                    mpEnqueRequest[_port]->sendCmd(QueueMgr<QBufInfo>::eCmd_pop_back, 0, 0);
                    if (bFirst) {
                        MUINT32 j;
                        for (j = 0; j < _popQBuf.mvOut.size(); j++) {
                            vMnum.push_back(_popQBuf.mvOut.at(j).FrameBased.mMagicNum_tuning);
                        }
                    }
                    nDrop++;
                }
                else {
                    PIPE_ERR("N:%d error: SOF=%d burstNum=%d nDrop=%d not enought to drop",
                        this->mpSensorIdx, this->mPrvEnqSOFIdx, this->mBurstQNum, nDrop);
                }
            }
            bFirst = 0;
        }
        for (i = 0; i < vMnum.size(); i++) {
            PIPE_INF("WARNING: enque drv > 1 in 1 frame, drop request(mag:0x%x)sof(%d/%d)",
                vMnum.at(i), mPrvEnqSOFIdx, _sof_idx);
            if (m_DropCB) {
                m_DropCB(vMnum.at(i), m_returnCookie);
            }
        }
    }
    mPrvEnqSOFIdx = _sof_idx;

    // check if input buf alread enqued before
    for (_port = 0; _port < ePort_max; _port++) {
        if (pQBuf[_port].mvOut.size() == 0) {
            continue;
        }

        #if 1//TBD check if buf alread enqued
        mpEnqueRequest[_port]->sendCmd(QueueMgr<QBufInfo>::eCmd_getsize, (MUINTPTR)&_size, 0);
        for (i = 0; i < _size; i++) {
            MUINT32 k;
            mpEnqueRequest[_port]->sendCmd(QueueMgr<QBufInfo>::eCmd_at, i, (MUINTPTR)&_popQBuf);
            for (j = 0; j < _popQBuf.mvOut.size(); j++) {
                for (k = 0; k < pQBuf[_port].mvOut.size(); k++) {
                    if (_NOMRALSVPIPE_BUF_GETPA(pQBuf[_port].mvOut.at(k),PLANE_ID) ==
                            _NOMRALSVPIPE_BUF_GETPA(_popQBuf.mvOut.at(j),PLANE_ID)) {
                        PIPE_ERR("error: buf already enqued before: x%" PRIXPTR "@%d/%d",\
                                _NOMRALSVPIPE_BUF_GETPA(_popQBuf.mvOut.at(j),PLANE_ID), _port, k);
                        return MFALSE;
                    }
                }
            }
        }
        #endif
    }

    PIPE_DBG("-");

    return MTRUE;
}


/******************************************************************************
 *
 ******************************************************************************/
MBOOL NormalSvPipe::deque(QPortID& rQPort, QBufInfo& rQBuf, MUINT32 u4TimeoutMs)
{
    IOPipeTrace _traceObj("NormalSvPipe::deque", IOPIPE_TRACE_EN_NORMALSVPIPE);
    MBOOL   ret = MTRUE, dequePortsBreak = MFALSE;
    MUINT32 ii = 0, j = 0;
    MINT32  waitStat, queueDepth = 0;
    vector<NSImageio::NSIspio::PortID>      QportID; //used for CamIOPipe
    NSImageio::NSIspio::QTimeStampBufInfo   rQTSBufInfo;
    NSImageio::NSIspio::PortID              portID;    //used for CamIOPipe
    IImageBuffer    *pframe= NULL;
    BufInfo         _buf;
    QBufInfo        _qBuf;

    PIPE_DBG("+");

    if (this->FSM_CHECK(op_streaming, __FUNCTION__) == MFALSE) {
        return MFALSE;
    }

    if (0 == rQPort.mvPortId.size()) {
        PIPE_ERR("deque port is empty");
        return MFALSE;
    }

    //

    rQBuf.mvOut.clear();
    for (ii = 0 ; ii < rQPort.mvPortId.size() ; ii++) {
        portID.index = rQPort.mvPortId.at(ii).index;
        PIPE_DBG("dma(x%x)", portID.index);

        if (u4TimeoutMs == 0xFFFFFFFF) {
            do {
                waitStat = ::sem_wait(&mpNSvPipeThread->m_semDeqDonePort[NSVPIPE_MAP_PORTID(portID.index)]);
                if ((waitStat == -1) && (errno == EINTR)) {
                    PIPE_INF("WARNING: wait again 1!");
                }
            } while ((waitStat == -1) && (errno == EINTR));
        }
        else {
            struct timespec ts;

            if (clock_gettime(CLOCK_REALTIME, &ts) == -1) {
                PIPE_ERR("wait clock_gettime error. wait with no timeout");
                do {
                    waitStat = ::sem_wait(&mpNSvPipeThread->m_semDeqDonePort[NSVPIPE_MAP_PORTID(portID.index)]);
                    if ((waitStat == -1) && (errno == EINTR)) {
                        PIPE_INF("WARNING: wait again 2!");
                    }
                } while ((waitStat == -1) && (errno == EINTR));
            }
            else {
                TIME_SPEC_CAL(ts, u4TimeoutMs);

                do {
                    waitStat = sem_timedwait(&mpNSvPipeThread->m_semDeqDonePort[NSVPIPE_MAP_PORTID(portID.index)], &ts);
                    if ((waitStat == -1) && (errno == EINTR)) {
                        PIPE_INF("WARNING: wait again 3!");
                    }
                } while ((waitStat == -1) && (errno == EINTR));
            }
        }
        if (0 != waitStat) {
            PIPE_ERR("N:%d dma:x%x pass1 deque fail %d", mpSensorIdx, portID.index, waitStat);
            return MFALSE;
        }

        if (NormalSvPipe_Thread::_ThreadErrExit == mpNSvPipeThread->m_DeqThrdState) {
            PIPE_ERR("N:%d dma:x%x deque fail", mpSensorIdx, portID.index);
            return MFALSE;
        }
        else if (NormalSvPipe_Thread::_ThreadStop == mpNSvPipeThread->m_DeqThrdState) {
            rQBuf.mvOut.resize(0);
            PIPE_WRN("N:%d dma:x%x deque, already stopped", mpSensorIdx, portID.index);
            return MTRUE;
        }

        mDeqContainerLock.lock();
        mpDeQueue[NSVPIPE_MAP_PORTID(portID.index)]->sendCmd(QueueMgr<QBufInfo>::eCmd_getsize, (MUINTPTR)&queueDepth, 0);
        if (queueDepth) {
            mpDeQueue[NSVPIPE_MAP_PORTID(portID.index)]->sendCmd(QueueMgr<QBufInfo>::eCmd_front, (MUINTPTR)&_qBuf, 0);
        }
        else {
            dequePortsBreak = MTRUE;
            _qBuf.mvOut.resize(0);
            rQBuf.mvOut.resize(0);
            PIPE_INF("N:%d dma:x%x break depth %d", mpSensorIdx, portID.index, (MUINT32)rQBuf.mvOut.size());
        }
        mDeqContainerLock.unlock();

        for (j = 0; j < _qBuf.mvOut.size(); j++) {
            rQBuf.mvOut.push_back(_qBuf.mvOut.at(j));
        }

        if (_qBuf.mvOut.size()) {
            PIPE_DBG("MyDeq:%d: dma:x%x M:x%x", mpSensorIdx, portID.index, _qBuf.mvOut.at(0).mMetaData.mMagicNum_tuning);
        }
    }

    if (dequePortsBreak == MTRUE) {
        PIPE_INF("MYDEQ:%d not ready for all port,, clear out queue", mpSensorIdx);
        rQBuf.mvOut.resize(0);
    }
    else {
        for (ii = 0 ; ii < rQPort.mvPortId.size() ; ii++) {
            portID.index = rQPort.mvPortId.at(ii).index;

            mDeqContainerLock.lock();
            /* pop from deque container */
            mpDeQueue[NSVPIPE_MAP_PORTID(portID.index)]->sendCmd(QueueMgr<QBufInfo>::eCmd_getsize, (MUINTPTR)&queueDepth, 0);
            if (queueDepth) {
                mpDeQueue[NSVPIPE_MAP_PORTID(portID.index)]->sendCmd(QueueMgr<QBufInfo>::eCmd_pop, 0, 0);
            }
            mDeqContainerLock.unlock();
        }
    }

    PIPE_DBG("-");

    return ret;
}

MBOOL NormalSvPipe::addUser(char const* szUserName)
{
    /* search user names to find if exist
     * if not found, check max user num
     *      ok => set idx to the 1st empty loc
     *      ng => return err
     * else set idx to found loc
     * add user cnt @idx
     **/
    MINT32 i = 0, found_idx = -1, len = 0;

    len = strlen(szUserName);
    if ((len == 0) || (len >= IOPIPE_MAX_USER_NAME_LEN)) {
        PIPE_ERR("Ivalide user name length: %d", len);
        return MFALSE;
    }

    for (i = 0; i < IOPIPE_MAX_NUM_USERS; i++) {
        if (mUserCnt[i] > 0) {
            if (strcmp(mUserName[i], szUserName) == 0) {
                found_idx = i;
                break;
            }
        }
    }
    if (found_idx == -1) {
        for (i = 0; i < IOPIPE_MAX_NUM_USERS; i++) {
            if (mUserCnt[i] == 0) {
                break;
            }
        }
        if (i < IOPIPE_MAX_NUM_USERS) {
            found_idx = i;
            strlcpy(mUserName[i], szUserName, strlen(szUserName) + 1);
        }
        else {
            PIPE_ERR("User count(%d) reaches maximum!", i);
            return MFALSE;
        }
    }

    mUserCnt[found_idx]++;
    mTotalUserCnt++;

    PIPE_DBG("%s ++, %d/%d", szUserName, mUserCnt[found_idx], mTotalUserCnt);

    return MTRUE;
}

MBOOL NormalSvPipe::delUser(char const* szUserName)
{
    /* search user names to find if exist
     * if not found, return err
     * else set idx to found loc
     * dec user cnt @idx
     * if user cnt is 0, set name[0]='\0'
     **/
    MINT32 i = 0, found_idx = -1, len = 0;

    len = strlen(szUserName);
    if ((len == 0) || (len >= IOPIPE_MAX_USER_NAME_LEN)) {
        PIPE_ERR("Invalid user name length: %d", len);
        return MFALSE;
    }

    for (i = 0; i < IOPIPE_MAX_NUM_USERS; i++) {
        if (mUserCnt[i] > 0) {
            if (strcmp(mUserName[i], szUserName) == 0) {
                found_idx = i;
            }
        }
    }

    if (found_idx == -1) {
        PIPE_ERR("Invalid user name(%s)", szUserName);
        return MFALSE;
    }

    mUserCnt[found_idx]--;
    mTotalUserCnt--;

    if (mUserCnt[found_idx] == 0) {
        mUserName[found_idx][0] = '\0';
    }

    PIPE_DBG("%s --, %d/%d", szUserName, mUserCnt[found_idx], mTotalUserCnt);

    return MTRUE;
}

MINT32 NormalSvPipe::getTotalUserNum(void)
{
    MINT32 i = 0, sum = 0;

    for (i = 0; i < IOPIPE_MAX_NUM_USERS; i++) {
        if (mUserCnt[i] > 0) {
            sum++;
        }
    }
    return sum;
}

/******************************************************************************
 *
 ******************************************************************************/
MBOOL NormalSvPipe::reset(void)
{
    return MTRUE;
}

/******************************************************************************
 *
 ******************************************************************************/
MBOOL NormalSvPipe::configPipe(QInitParam const& vInPorts, MINT32 burstQnum)
{
    IOPipeTrace      _traceObj("NormalSvPipe::configpipe", IOPIPE_TRACE_EN_NORMALSVPIPE);
    MBOOL            dump = 0, ret = MTRUE;
    MUINT32          temp, SIdx, sensorDevIdx, i = 0, j = 0;
    EImageFormat     tgi_eImgFmt[4]; //Temp Used
    CAMIO_Func       _InFunc, _OutFunc;
    //HWSyncDrv*       pHwSync = NULL;

    _InFunc.Raw = 0;
    _OutFunc.Raw = 0;
    mRawType = vInPorts.mRawType;

    PIPE_INF("+, SenIdx=%d", mpSensorIdx);

    Mutex::Autolock lock(mCfgLock);

    if (this->FSM_CHECK(op_cfg, __FUNCTION__) == MFALSE) {
        PIPE_ERR("configPipe multiple times, SenIdx=%d", mpSensorIdx);
        return MFALSE;
    }

    if ((burstQnum < 1) || (burstQnum == 2)) {
        PIPE_ERR("BurstQNum %d, should be 1 or > 2, SenIdx=%d", burstQnum, mpSensorIdx);
        return MFALSE;
    }

    ///1 Parameter Check
    if (1 != vInPorts.mSensorCfg.size()) {
        PIPE_ERR("SensorCfgCnt is not 1. (%d)", (MUINT32)vInPorts.mSensorCfg.size());
        return MFALSE;
    }

    if (mpSensorIdx != vInPorts.mSensorCfg.at(0).index) {
        PIPE_ERR("SensorIdx is not match. (%d,%d)", mpSensorIdx, vInPorts.mSensorCfg.at(0).index);
        return MFALSE;
    }


    ///2 Get Info
    memset(&m_sensorConfigParam, 0, sizeof(IHalSensor::ConfigParam));

    SIdx = mpSensorIdx;

    if (pAllSensorInfo->mSenInfo[SIdx].mOccupiedOwner) {
        PIPE_INF("WARNING: Overwrite SenIdx=%d-SensorCfg...", SIdx);
    }
    pAllSensorInfo->mSenInfo[SIdx].mConfig = vInPorts.mSensorCfg.at(0);
    pAllSensorInfo->mSenInfo[SIdx].mOccupiedOwner = (MUINTPTR)this;

    m_sensorConfigParam = vInPorts.mSensorCfg.at(0); //used for sensorCfg

    NSVPIPE_GET_TGIFMT(SIdx, tgi_eImgFmt[SIdx]);

    if (!mpHalSensor) {
         mpHalSensor = pAllSensorInfo->mSList->createSensor(THIS_NAME, 1, &mpSensorIdx);
    } else {
        PIPE_ERR("ERROR: ConfigPipe multiple times...");
        return MFALSE;
    }
    if (!mpHalSensor) {
        PIPE_ERR("mpHalSensor Fail");
        return MFALSE;
    }

    PIPE_TRACE_BEGIN("TAG_SENSOR_CONFIG");
    if(mpHalSensor->configure(1, &m_sensorConfigParam) == MFALSE){
        PIPE_ERR("fail when configure sensor driver\n");
        return MFALSE;
    }
    PIPE_TRACE_END("TAG_SENSOR_CONFIG");

    mpHalSensor->querySensorDynamicInfo(pAllSensorInfo->mSenInfo[SIdx].mDevId, &pAllSensorInfo->mSenInfo[SIdx].mDInfo);

    if (!mpCamIOPipe) {

        PortInfo                tgi, imgo;
        PortInfo                *pOut = NULL, *pOutTable = NULL;
        MBOOL                   enable2PixelTwin = MFALSE, stream_on = MTRUE;
        MUINT32                 _path = 0, imgHdrBufNum = 0, Sen_PixMode = 0;
        MUINTPTR                imgHdrAllocOfst[ePlane_max] = {0, 0, 0};
        E_INPUT                 TargetTG = TG_CAMSV_0;
        MUINT32                 SenTG = CAM_TG_NONE;
        ENPipeQueryCmd          _op = ENPipeQueryCmd_X_PIX;
        NormalPipe_InputInfo    qryInput;
        vector<PortInfo const*> vCamIOInPorts;
        vector<PortInfo const*> vCamIOOutPorts;
#if 0   //For legacy PMQoS BW
        MUINT32                 dmao_bw = 0;
#endif
        Mutex::Autolock lock(NormalSvPipe::NSvPipeCfgGLock); // for protect twin mode check

        // only pure raw supported in camsv channel
        mPureRaw = MTRUE;
        mDynSwtRawType = MFALSE;

        tgi.index        = (MUINT32)TargetTG;
        tgi.eImgFmt      = tgi_eImgFmt[mpSensorIdx];
        NSVPIPE_GET_TG_PIXID(mpSensorIdx, tgi.eRawPxlID);
        tgi.u4ImgWidth   = _NORMALSVPIPE_GET_SENSORCFG_CROP_W(mpSensorIdx);
        tgi.u4ImgHeight  = _NORMALSVPIPE_GET_SENSORCFG_CROP_H(mpSensorIdx);
        NSVPIPE_GET_TG_GRAB(mpSensorIdx, m_sensorConfigParam.scenarioId, tgi.crop1.x, tgi.crop1.y);
        tgi.crop1.floatX = 0;
        tgi.crop1.floatY = 0;
        tgi.crop1.w      = tgi.u4ImgWidth;
        tgi.crop1.h      = tgi.u4ImgHeight;
        tgi.type         = EPortType_Sensor;
        tgi.inout        = EPortDirection_In;

        mpHalSensor->sendCommand(pAllSensorInfo->mSenInfo[mpSensorIdx].mDevId, SENSOR_CMD_GET_DEFAULT_FRAME_RATE_BY_SCENARIO,
                                    (MUINTPTR)&m_sensorConfigParam.scenarioId, (MUINTPTR)&tgi.tgFps, 0);
        mpHalSensor->sendCommand(pAllSensorInfo->mSenInfo[mpSensorIdx].mDevId, SENSOR_CMD_GET_SENSOR_PIXELMODE,
                                    (MUINTPTR)&m_sensorConfigParam.scenarioId, (MUINTPTR)&tgi.tgFps , (MUINTPTR)&Sen_PixMode);
        mpHalSensor->sendCommand(pAllSensorInfo->mSenInfo[mpSensorIdx].mDevId, SENSOR_CMD_GET_MIPI_PIXEL_RATE,
                                    (MUINTPTR)&m_sensorConfigParam.scenarioId, (MUINTPTR)&tgi.MIPI_PixRate, 0);

        mpCamIOPipe = ICamIOPipe::createInstance((MINT8*)mpName, TargetTG, NSImageio::NSIspio::ICamIOPipe::NORMALSVIO, &tgi);

        SenTG = NSVPIPE_MAP_SEN_TG(TargetTG);
        mpHalSensor->sendCommand(pAllSensorInfo->mSenInfo[SIdx].mDevId, SENSOR_CMD_SET_SENINF_CAM_TG_MUX, (MUINTPTR)&_NORMALSVPIPE_GET_TG_IDX(SIdx), (MUINTPTR)&SenTG, 0);
        NormalSvPipe::pAllSensorInfo->mSenInfo[SIdx].mDInfo.TgInfo = SenTG;

        tgi.index                = NSVPIPE_MAP_TGI(NSVPIPE_MAP_SEN_TG(TargetTG));
        tgi.ePxlMode             = mSenPixMode = NSVPIPE_MAP_PXLMODE(mpSensorIdx, Sen_PixMode);
        tgi.tTimeClk             = _NORMALSVPIPE_GET_CLK_FREQ(SIdx)/100; //0.1MHz <- KHz
        mTgOut_size[TargetTG].w = tgi.u4ImgWidth;
        mTgOut_size[TargetTG].h = tgi.u4ImgHeight;
        mCamSvImgoOut_size.w = tgi.u4ImgWidth;
        mCamSvImgoOut_size.h = tgi.u4ImgHeight;
        vCamIOInPorts.push_back(&tgi);

        PIPE_INF("N:%d TG=%d Scen=%d tgFmt=x%x devID=%d eRawPxlID=%d PixelMode=%d W/H=[%d,%d] FPSx10=%d Clk=%d khz mipiPixRate=%d burst=%d",
             mpSensorIdx, (MUINT32)TargetTG, m_sensorConfigParam.scenarioId, tgi.eImgFmt, pAllSensorInfo->mSenInfo[mpSensorIdx].mDevId,
             tgi.eRawPxlID, tgi.ePxlMode, tgi.u4ImgWidth, tgi.u4ImgHeight, tgi.tgFps,  tgi.tTimeClk*100, tgi.MIPI_PixRate, burstQnum);

#if 0
        pHwSync = HWSyncDrv::createInstance();
        if (pHwSync != NULL) {
        //hwsync drv, sendCommand that the sensor is configured
            if(pHwSync->sendCommand(HW_SYNC_CMD_SET_MODE, pAllSensorInfo->mSenInfo[SIdx].mDevId, m_sensorConfigParam.scenarioId, m_sensorConfigParam.framerate))
            {
                PIPE_ERR("camera(%d) pHwSync sendCommand fail ", pAllSensorInfo->mSenInfo[SIdx].mDevId);
                return MFALSE;
            }
        }
#endif

        if(mRawType == 1){
            mpHalSensor->sendCommand(pAllSensorInfo->mSenInfo[SIdx].mDevId, SENSOR_CMD_SET_TEST_MODEL, (MUINTPTR)&stream_on, 0, 0);
        }
        else{
            mpHalSensor->sendCommand(pAllSensorInfo->mSenInfo[SIdx].mDevId, SENSOR_CMD_SET_STREAMING_RESUME, 0, 0, 0);
        }

        if (!mpCamIOPipe) {
            PIPE_ERR("CamIOPipe createInstance fail");
            return MFALSE;
        }

        if (MFALSE == mpCamIOPipe->init()) {
             PIPE_ERR("CamIOPipe init fail");
             return MFALSE;
        }

        //port's data paser
        for (i = 0; i < vInPorts.mPortInfo.size(); i++) {
            MBOOL bPushCamIO = MFALSE;

            switch (vInPorts.mPortInfo.at(i).mPortID.index) {
            case EPortIndex_IMGO:
                pOut          = &imgo;
                mPureRaw      = MTRUE; // only pure raw supported in Camsv Channel
                mOpenedPort  |= NormalSvPipe::_IMGO_ENABLE;
                bPushCamIO    = MTRUE;
                break;
            default:
                PIPE_ERR("unknown port id %d", vInPorts.mPortInfo.at(i).mPortID.index);
                return MFALSE;
                break;
            }

            if ((GetOpenedPortInfo(NormalSvPipe::_GetPort_OpendNum) > ePort_max)
                    || (GetOpenedPortInfo(NormalSvPipe::_GetPort_OpendNum) == 0)) {
                PIPE_ERR("dmao config error, opend port num:0x%x(max:0x%x)\n",
                        GetOpenedPortInfo(NormalSvPipe::_GetPort_OpendNum), ePort_max);
                return MFALSE;
            }

            pOut->index = vInPorts.mPortInfo.at(i).mPortID.index;
            mCfgFmt[NSVPIPE_MAP_PORTID(pOut->index)] = (MINT)vInPorts.mPortInfo.at(i).mFmt;
            NSVPIPE_PASS_PORTINFO(mpSensorIdx, tgi, vInPorts.mPortInfo.at(i), pOut);
            PortImgFormat[NSVPIPE_MAP_PORTID(pOut->index)] = pOut->eImgFmt; //for GetOpenedPortInfo()

            switch(pOut->index) {
                case EPortIndex_IMGO:
                    //IMGO Query x_pix
                    _op = (pOut->u4ImgWidth != ((MUINT32)mTgOut_size[TargetTG].w))? (ENPipeQueryCmd_CROP_X_PIX): (ENPipeQueryCmd_X_PIX);
                    qryInput.format    = pOut->eImgFmt;
                    qryInput.width     = pOut->u4ImgWidth;
                    qryInput.pixelMode = mSenPixMode;

                    if(NSVPIPE_GET_CAPIBILITY(_op, qryInput, *pOut) == MFALSE) {
                        return MFALSE;
                    }

                    //IMGO Query stride
                    _op            = ENPipeQueryCmd_STRIDE_BYTE;
                    qryInput.width = pOut->crop1.w;
                    qryInput.pixelMode = ePixMode_NONE;

                    if(NSVPIPE_GET_CAPIBILITY(_op, qryInput, *pOut) == MFALSE) {
                        return MFALSE;
                    }
#if 0               //For legacy PMQoS BW
                    dmao_bw += pOut->u4Stride[ePlane_1st] * pOut ->u4ImgHeight;
#endif
                    break;
                default:
                    break;
            }

            PIPE_DBG("OutPorts[%d] Crop=[%d,%d,%d,%d], sz=(%d,%d), Stride=(%d/%d/%d), p(%d)",\
                    pOut->index, pOut->crop1.x, pOut->crop1.y, pOut->crop1.w, pOut->crop1.h,\
                    pOut->u4ImgWidth, pOut->u4ImgHeight,pOut->u4Stride[ePlane_1st],
                    pOut->u4Stride[ePlane_2nd],pOut->u4Stride[ePlane_3rd],bPushCamIO);
            if (bPushCamIO) {
                vCamIOOutPorts.push_back(pOut);
            }
        }

        switch (_NORMALSVPIPE_GET_TG_IDX((mpSensorIdx))) {
        case CAM_SV_1:
            _path = NSVPIPE_CAMSV_1;
            break;
        case CAM_SV_2:
            _path = NSVPIPE_CAMSV_2;
            break;
        case CAM_SV_3:
            _path = NSVPIPE_CAMSV_3;
            break;
        case CAM_SV_4:
            _path = NSVPIPE_CAMSV_4;
            break;
        case CAM_SV_5:
            _path = NSVPIPE_CAMSV_5;
            break;
        case CAM_SV_6:
            _path = NSVPIPE_CAMSV_6;
            break;
        default:
            PIPE_ERR("unkown TG %d", _NORMALSVPIPE_GET_TG_IDX((mpSensorIdx)));
            return MFALSE;
        }

        _InFunc.Bits.SUBSAMPLE = burstQnum - 1;
        _InFunc.Bits.DATA_PATTERN = vInPorts.m_Func.Bits.DATA_PATTERN;

        if (MFALSE == mpCamIOPipe->configPipe(vCamIOInPorts, vCamIOOutPorts, &_InFunc, &_OutFunc)) {
            PIPE_ERR("CamIOPipe configPipe Fail");
            return MFALSE;
        }
#if 0
        if(vInPorts.m_bN3D == MTRUE){
            HWSyncDrv* pHwSync = NULL;
            pHwSync = HWSyncDrv::createInstance();
            if(pHwSync->sendCommand(HW_SYNC_CMD_ENABLE_SYNC, \
                0,pAllSensorInfo->mSenInfo[mpSensorIdx].mDevId,HW_SYNC_BLOCKING_EXE) != 0){
                pHwSync->destroyInstance();
                PIPE_ERR("N3D start fail\n");
                return MFALSE;
            }
            this->m_bN3DEn = MTRUE;
            pHwSync->destroyInstance();
        }
        else {
            this->m_bN3DEn = MFALSE;
        }
#endif

        mEnablePath     |= _path;
        mBurstQNum      = burstQnum;
        m_DropCB        = vInPorts.m_DropCB;
        m_returnCookie  = vInPorts.m_returnCookie;

        /*Image Header buffer allocation*/
        imgHdrBufNum = IMG_HEADER_BUF_NUM * mBurstQNum;

        for (MUINT32 PlaneID = 0; PlaneID <= (MUINT32)ePlane_3rd; PlaneID++) {
            mImgHeaderPool.ImemBufInfo[PlaneID].size = 0;
            for (MUINT32 headersize = 0, i = 0; i < vInPorts.mPortInfo.size(); i++) {
                this->mpCamIOPipe->sendCommand(EPIPECmd_GET_HEADER_SIZE,(MINTPTR)vInPorts.mPortInfo.at(i).mPortID.index,(MINTPTR)&headersize,0);
                mImgHeaderPool.ImemBufInfo[PlaneID].size += (headersize * imgHdrBufNum);
                mImgHeaderPool.ImemBufInfo[PlaneID].size += (15 * imgHdrBufNum); //allocate extra memory for 16 bytes alignment
            }
            mImgHeaderPool.ImemBufInfo[PlaneID].useNoncache = 1;
            if (mpIMem->allocVirtBuf(&mImgHeaderPool.ImemBufInfo[PlaneID]) < 0) {
                PIPE_ERR("imgHeader[%d] alloc %d fail", PlaneID, mImgHeaderPool.ImemBufInfo[PlaneID].size);
                return MFALSE;
            }
            if (mpIMem->mapPhyAddr(&mImgHeaderPool.ImemBufInfo[PlaneID], MFALSE) < 0) {
                PIPE_ERR("imgHeader[%d] map %d fail", PlaneID, mImgHeaderPool.ImemBufInfo[PlaneID].size);
                return MFALSE;
            }
        }

        /* image header buffer to each port*/
        for (MUINT32 headersize = 0,i = 0; i < vInPorts.mPortInfo.size(); i++) {
        //
            MUINT32 _port = NSVPIPE_MAP_PORTID(vInPorts.mPortInfo.at(i).mPortID.index);
            mpImgHeaderMgr[_port].init();
            this->mpCamIOPipe->sendCommand(EPIPECmd_GET_HEADER_SIZE, (MINTPTR)vInPorts.mPortInfo.at(i).mPortID.index, (MINTPTR)&headersize, 0);

            for (j = 0; j < imgHdrBufNum; j++) {
                NormalSvPipe::IMEM_MUTI_BUF_INFO iHeader;
                for (MUINT32 PlaneID = 0; PlaneID <= (MUINT32)ePlane_3rd; PlaneID++) {
                    MUINTPTR virtBA = ALIGN_CAL(mImgHeaderPool.ImemBufInfo[PlaneID].virtAddr, 16);
                    MUINTPTR phyBA = ALIGN_CAL(mImgHeaderPool.ImemBufInfo[PlaneID].phyAddr, 16);

                    imgHdrAllocOfst[PlaneID]                  = ALIGN_CAL(imgHdrAllocOfst[PlaneID], 16);
                    iHeader.ImemBufInfo[PlaneID].size         = headersize;
                    iHeader.ImemBufInfo[PlaneID].memID        = mImgHeaderPool.ImemBufInfo[PlaneID].memID;
                    iHeader.ImemBufInfo[PlaneID].virtAddr     = virtBA + imgHdrAllocOfst[PlaneID];
                    iHeader.ImemBufInfo[PlaneID].phyAddr      = phyBA + imgHdrAllocOfst[PlaneID];
                    iHeader.ImemBufInfo[PlaneID].bufSecu      = mImgHeaderPool.ImemBufInfo[PlaneID].bufSecu;
                    iHeader.ImemBufInfo[PlaneID].bufCohe      = mImgHeaderPool.ImemBufInfo[PlaneID].bufCohe;
                    iHeader.ImemBufInfo[PlaneID].useNoncache  = mImgHeaderPool.ImemBufInfo[PlaneID].useNoncache;
                    imgHdrAllocOfst[PlaneID]                 += headersize;
                }
                mpImgHeaderMgr[_port].sendCmd(QueueMgr<NormalSvPipe::IMEM_MUTI_BUF_INFO>::eCmd_push, (MUINTPTR)&iHeader, 0);
                mImgHdrLock.lock();
                mImgHdrCnt[_port]++;
                mImgHdrLock.unlock();
            }
        }

        if (mOpenedPort == 0) {
            PIPE_ERR("error:no dmao enabled\n");
            return MFALSE;
        }
        PIPE_DBG("DropCB(0x%p)/Cookie(0x%p) DynamicRawType(%d) PureRaw(%d)", m_DropCB, m_returnCookie,
                mDynSwtRawType, mPureRaw);

        mpCamIOPipe->sendCommand(EPIPECmd_SET_FRM_TIME, (MINTPTR)0, 0, 0);
        mpCamIOPipe->sendCommand(EPIPECmd_SET_FRM_TIME, (MINTPTR)0, 1, 0);
        mpNSvPipeThread->updateFrameTime(0, 0);
        mpNSvPipeThread->updateFrameTime(0, 1);

        // NPipeCfgGLock auto unlocked
    }
    else {
        PIPE_ERR("CamioPipe is not NULL for 1st cfg. Remained user(%d)", mTotalUserCnt);
        for (i = 0; i < IOPIPE_MAX_NUM_USERS; i++) {
            if (mUserCnt[i] > 0)
                PIPE_ERR("[%d]%s ",i, mUserName[i]);
        }
        return MFALSE;
    }

EXIT:
    this->FSM_UPDATE(op_cfg);

    PIPE_DBG("-");

    return ret;
}


/******************************************************************************
 *
 ******************************************************************************/
MINT32 NormalSvPipe::attach(const char* UserName)
{
    MINT32 user_key = 0;

    PIPE_DBG("+");

    if (this->FSM_CHECK(op_cmd, __FUNCTION__) == MFALSE) {
        return -1;
    }

    if (mpCamIOPipe) {
        user_key = mpCamIOPipe->registerIrq((MINT8*)UserName);
        if (user_key >= 0) {
            return user_key;
        }
        PIPE_ERR("registerIrq(%s, %d) fail", UserName, user_key);
    }
    else {
        PIPE_ERR("NormalSvIOPipe not create");
    }

    return -1;
}


/******************************************************************************
 *
 ******************************************************************************/
MBOOL NormalSvPipe::wait(EPipeSignal eSignal, EPipeSignalClearType eClear, const MINT32 mUserKey,
                        MUINT32 TimeoutMs, SignalTimeInfo *pTime)
{
    Irq_t irq;
    MBOOL ret = MTRUE;

    PIPE_DBG("+");

    if (this->FSM_CHECK(op_cmd, __FUNCTION__) == MFALSE) {
        return MFALSE;
    }

    irq.Type = (eClear == EPipeSignal_ClearNone)? Irq_t::_CLEAR_NONE: Irq_t::_CLEAR_WAIT;
    ret = NSVPIPE_MAP_SIGNAL(eSignal, irq.Status, irq.StatusType);
    if (!ret) {
        PIPE_ERR("Signal x%x not support", eSignal);
        return ret;
    }
    irq.UserKey = mUserKey;
    irq.Timeout = TimeoutMs; //ms

    if (mpCamIOPipe) {
        ret = mpCamIOPipe->irq(&irq);
        if (!ret) {
            PIPE_ERR("Wait Signal Fail(%d,%d,x%x,%d)", irq.UserKey, irq.Type, irq.Status, irq.Timeout);
            return ret;
        }
        else {
            if (pTime) {
                // fill time info in irq
                pTime->tLastSig_sec = irq.TimeInfo.tLastSig_sec;
                pTime->tLastSig_usec = irq.TimeInfo.tLastSig_usec;
                pTime->tMark2WaitSig_sec = irq.TimeInfo.tMark2WaitSig_sec;
                pTime->tMark2WaitSig_usec = irq.TimeInfo.tMark2WaitSig_usec;
                pTime->tLastSig2GetSig_sec = irq.TimeInfo.tLastSig2GetSig_sec;
                pTime->tLastSig2GetSig_usec = irq.TimeInfo.tLastSig2GetSig_usec;
                pTime->passedbySigcnt = irq.TimeInfo.passedbySigcnt;
            }
        }
    }
    else {
        PIPE_ERR("CamIOPipe not create");
    }

    PIPE_DBG("-");

    return ret;
}

MBOOL NormalSvPipe::signal(EPipeSignal eSignal, const MINT32 mUserKey)
{
    Irq_t irq;
    MBOOL ret = MTRUE;

    PIPE_DBG("+");

    if (this->FSM_CHECK(op_cmd, __FUNCTION__) == MFALSE) {
        return MFALSE;
    }

    ret = NSVPIPE_MAP_SIGNAL(eSignal, irq.Status, irq.StatusType);
    if (!ret) {
        PIPE_ERR("Signal x%x not support", eSignal);
        return ret;
    }

    if(mUserKey < 1){
        PIPE_ERR("invalid userkey(%d)", mUserKey);
        return ret;
    }

    irq.UserKey = mUserKey;

    if (mpCamIOPipe) {
        ret = mpCamIOPipe->signalIrq(irq);
        if (!ret) {
            PIPE_ERR("Signal x%x/x%x fail", eSignal, irq.Status);
            return ret;
        }
    }
    else {
        PIPE_ERR("CamIOPipe not create");
    }

    PIPE_DBG("-");

    return ret;
}

MBOOL NormalSvPipe::suspend(E_SUSPEND_TPYE etype)
{
    Mutex::Autolock lock(mCfgLock);
    MBOOL ret = MTRUE, stream_on = MFALSE;
    MUINT32 _OpenedPort = this->GetOpenedPortInfo(NormalSvPipe::_GetPort_OpendNum), i,p,size,port;
    QBufInfo _qBuf;

    if (this->FSM_CHECK(op_suspend, __FUNCTION__) == MFALSE) {
        PIPE_ERR("N:%d suspend can only be trigger under streaming", this->mpSensorIdx);
        return MFALSE;
    }

    PIPE_INF("N:%d Holding idle locks before suspend", this->mpSensorIdx);

    //lock only enqThres, deqThres will be locked via semophore which is posted by enqThres
    //in order to make sure all enque request being removed from driver
    this->mpNSvPipeThread->idleStateHold(1, NormalSvPipe_Thread::eThdGrp_Enq);
    /* Not holding stt locks for 3a threads to error return when suspend
     * NormalPipe enq/deq threads will blocking at mutex lock
     */
    this->mpNSvPipeThread->idleStateHold(1, NormalSvPipe_Thread::eThdGrp_Deq);

    ret = this->mpCamIOPipe->suspend(ICamIOPipe::SW_SUSPEND);

    this->mpNSvPipeThread->idleStateHold(0, NormalSvPipe_Thread::eThdGrp_Deq);

    if (this->mpNSvPipeThread->suspendNotifyCB.fpNotifyState) {
        if(this->mpNSvPipeThread->suspendNotifyCB.fpNotifyState(CAM_ENTER_SUSPEND, this->mpNSvPipeThread->suspendNotifyCB.Obj)
            == NOTIFY_FAIL){
            PIPE_ERR("N:%d suspend fail", this->mpSensorIdx);
            return MFALSE;
        }
    }

    PIPE_INF("N:%d type:%d DevId:0x%x p1 suspend done, release deqThd lock and flushing sof signal",
        this->mpSensorIdx,(MUINT32)etype, _NORMALSVPIPE_GET_SENSOR_DEV_ID(this->mpSensorIdx));

    this->signal(EPipeSignal_SOF, mpNSvPipeThread->m_irqUserKey); //flush SOF signal, since irqUserKey is here

    for (i = 0; i < _OpenedPort; i++) {
        ::sem_post(&this->mpNSvPipeThread->m_semDeqDonePort[NSVPIPE_MAP_PORTID(this->GetOpenedPortInfo(NormalSvPipe::_GetPort_Index, i))]);
    }

    switch(etype){
    case eSuspend_Type1:
        break;
    case eSuspend_Type2:
        if(mRawType == 1){
            mpHalSensor->sendCommand(_NORMALSVPIPE_GET_SENSOR_DEV_ID(this->mpSensorIdx), SENSOR_CMD_SET_TEST_MODEL, (MUINTPTR)&stream_on, 0, 0);
        }
        else{
            mpHalSensor->sendCommand(_NORMALSVPIPE_GET_SENSOR_DEV_ID(this->mpSensorIdx), SENSOR_CMD_SET_STREAMING_SUSPEND, 0, 0, 0);
        }
        break;
    default:
        PIPE_INF("unknow suspend type(%d)", (MUINT32)etype);
        break;
    }

    //drop request in enquecontainer
    this->mEnqContainerLock.lock();

    for (p = 0; p < this->GetOpenedPortInfo(NormalSvPipe::_GetPort_OpendNum); p++) {
        port = NSVPIPE_MAP_PORTID(this->GetOpenedPortInfo(NormalSvPipe::_GetPort_Index, p));
        this->mpEnqueRequest[port]->sendCmd(QueueMgr<QBufInfo>::eCmd_getsize, (MUINTPTR)&size, 0);

        for(i = 0; i < size; i++) {
            if (p == 0) {
                if(this->m_DropCB){
                    this->mpEnqueRequest[port]->sendCmd(QueueMgr<QBufInfo>::eCmd_front, (MUINTPTR)&_qBuf, 0);
                    for (MUINT32 m = 0; m < _qBuf.mvOut.size(); m++){
                        PIPE_INF("drop:%d_%d",m,_qBuf.mvOut.at(m).FrameBased.mMagicNum_tuning);
                        this->m_DropCB(_qBuf.mvOut.at(m).FrameBased.mMagicNum_tuning, this->m_returnCookie);
                    }
                }
                else
                    PIPE_ERR("dropCB is NULL\n");
            }
            this->mpEnqueRequest[port]->sendCmd(QueueMgr<QBufInfo>::eCmd_pop, 0, 0);
        }
    }

    this->mEnqContainerLock.unlock();
    this->FSM_UPDATE(op_suspend);

    return ret;
}


MBOOL NormalSvPipe::resume(QBufInfo const * pQBuf, E_SUSPEND_TPYE etype)
{
    Mutex::Autolock lock(mCfgLock);
    MBOOL       ret = MTRUE, stream_on = MTRUE;
    E_EnqueRst  retRst = eEnqueRst_OK;

    if (this->FSM_CHECK(op_start, __FUNCTION__) == MFALSE) {
        PIPE_ERR("N:%d suspend can only be trigger under streaming", this->mpSensorIdx);
        return MFALSE;
    }
    if (pQBuf == NULL) {
        PIPE_ERR("N:%d pQBuf null, MUST enque", this->mpSensorIdx);
        return MFALSE;
    }

    MUINT32 _shuttertime_us = (MUINT32)(pQBuf->mShutterTimeNs/1000);

    PIPE_INF("N:%d type:%d DevId:0x%x Release idle locks and resume shutter_us:%d",
        this->mpSensorIdx, etype, _NORMALSVPIPE_GET_SENSOR_DEV_ID(this->mpSensorIdx), _shuttertime_us);

    mpHalSensor->sendCommand(_NORMALSVPIPE_GET_SENSOR_DEV_ID(this->mpSensorIdx),
                                       SENSOR_CMD_SET_SENSOR_EXP_TIME, (MUINTPTR)&_shuttertime_us, 0, 0);

    switch(etype){
    case eSuspend_Type1:
        break;
    case eSuspend_Type2:
        if(mRawType == 1){
            mpHalSensor->sendCommand(_NORMALSVPIPE_GET_SENSOR_DEV_ID(this->mpSensorIdx),
                                        SENSOR_CMD_SET_TEST_MODEL, (MUINTPTR)&stream_on, 0, 0);
        }
        else{
            mpHalSensor->sendCommand(_NORMALSVPIPE_GET_SENSOR_DEV_ID(this->mpSensorIdx),
                                        SENSOR_CMD_SET_STREAMING_RESUME, (MUINTPTR)&_shuttertime_us, 0, 0);
        }
        break;
    default:
        PIPE_INF("unknow suspend type(%d)", (MUINT32)etype);
        break;
    }

    PIPE_INF("N:%d holding deque thread", this->mpSensorIdx);
    this->mpNSvPipeThread->idleStateHold(1, NormalSvPipe_Thread::eThdGrp_Deq);

    ret = this->mpCamIOPipe->preresume((MINT8*)this->mpName, _NORMALSVPIPE_GET_SENSORCFG_CROP_W(this->mpSensorIdx));
    if(ret == MFALSE){
        PIPE_ERR("N: preresume NG\n");
        return ret;
    }

    ret = this->acceptEnqRequest(*pQBuf);
    if (ret == MFALSE) {
        PIPE_ERR("N:%d enqueReq of resume NG\n", this->mpSensorIdx);
        return ret;
    }

    retRst = this->mpNSvPipeThread->frameToEnque();
    if (retRst != eEnqueRst_OK) {
        PIPE_ERR("N:%d frameToEnque of resume NG stat:0x%x\n", this->mpSensorIdx, retRst);
        return ret;
    }

    if (this->mpNSvPipeThread->suspendNotifyCB.fpNotifyState) {
        if(this->mpNSvPipeThread->suspendNotifyCB.fpNotifyState(CAM_EXIT_SUSPEND, this->mpNSvPipeThread->suspendNotifyCB.Obj)
            == NOTIFY_FAIL){
            PIPE_ERR("N:%d resume fail", this->mpSensorIdx);
            return MFALSE;
        }
    }

    ret = this->mpCamIOPipe->resume(ICamIOPipe::SW_SUSPEND);

    this->FSM_UPDATE(op_start);

    this->mpNSvPipeThread->idleStateHold(0, NormalSvPipe_Thread::eThdGrp_Deq);
    //only unlock enqThres, plz refer to suspend() for rationale
    this->mpNSvPipeThread->idleStateHold(0, NormalSvPipe_Thread::eThdGrp_Enq);

    return ret;
}

MBOOL NormalSvPipe::resume(MUINT64 shutterTimeUs, E_SUSPEND_TPYE etype)
{
    Mutex::Autolock lock(mCfgLock);
    MBOOL       ret = MTRUE, stream_on = MTRUE;
    E_EnqueRst  retRst = eEnqueRst_OK;
    MUINT32 _shuttertime_us = (MUINT32)shutterTimeUs;

    if (this->FSM_CHECK(op_start, __FUNCTION__) == MFALSE) {
        PIPE_ERR("N:%d suspend can only be trigger under streaming", this->mpSensorIdx);
        return MFALSE;
    }

    PIPE_INF("N:%d type:%d DevId:0x%x Release idle locks and resume shutter_us:%d v0",
        this->mpSensorIdx, etype, _NORMALSVPIPE_GET_SENSOR_DEV_ID(this->mpSensorIdx), _shuttertime_us);

    mpHalSensor->sendCommand(_NORMALSVPIPE_GET_SENSOR_DEV_ID(this->mpSensorIdx),
                                               SENSOR_CMD_SET_SENSOR_EXP_TIME, (MUINTPTR)&_shuttertime_us, 0, 0);

    switch(etype){
    case eSuspend_Type1:
        break;
    case eSuspend_Type2:
        if(mRawType == 1){
            mpHalSensor->sendCommand(_NORMALSVPIPE_GET_SENSOR_DEV_ID(this->mpSensorIdx),
                                        SENSOR_CMD_SET_TEST_MODEL, (MUINTPTR)&stream_on, 0, 0);
        }
        else{
            mpHalSensor->sendCommand(_NORMALSVPIPE_GET_SENSOR_DEV_ID(this->mpSensorIdx),
                                        SENSOR_CMD_SET_STREAMING_RESUME, (MUINTPTR)&_shuttertime_us, 0, 0);
        }
        break;
    default:
        PIPE_INF("unknow suspend type(%d)", (MUINT32)etype);
        break;
    }

    PIPE_INF("N:%d holding deque thread v0", this->mpSensorIdx);
    this->mpNSvPipeThread->idleStateHold(1, NormalSvPipe_Thread::eThdGrp_Deq);

    ret = this->mpCamIOPipe->preresume((MINT8*)this->mpName, _NORMALSVPIPE_GET_SENSORCFG_CROP_W(this->mpSensorIdx));
    if(ret == MFALSE){
        PIPE_ERR("N: preresume NG\n");
        return ret;
    }

    ret = this->mpCamIOPipe->resume(ICamIOPipe::SW_SUSPEND);

    this->FSM_UPDATE(op_start);

    this->mpNSvPipeThread->idleStateHold(0, NormalSvPipe_Thread::eThdGrp_Deq);
    //only unlock enqThres, plz refer to suspend() for rationale
    this->mpNSvPipeThread->idleStateHold(0, NormalSvPipe_Thread::eThdGrp_Enq);

    if (this->mpNSvPipeThread->suspendNotifyCB.fpNotifyState) {
        if(this->mpNSvPipeThread->suspendNotifyCB.fpNotifyState(CAM_EXIT_SUSPEND, this->mpNSvPipeThread->suspendNotifyCB.Obj)
            == NOTIFY_FAIL){
            PIPE_ERR("N:%d resume fail", this->mpSensorIdx);
            return MFALSE;
        }
    }

    return ret;
}


/******************************************************************************
 *
 ******************************************************************************/
MBOOL NormalSvPipe::abortDma(PortID port, char const* szCallerName)
{
    MBOOL ret = MTRUE;
    NSImageio::NSIspio::PortID portID;

    PIPE_INF("+, usr[%s]dma[0x%x]SenIdx[%d]", szCallerName, port.index, mpSensorIdx);

    if (this->FSM_CHECK(op_cmd, __FUNCTION__) == MFALSE) {
        return MFALSE;
    }

    if (mpCamIOPipe) {
        switch (port.index){
        case EPortIndex_IMGO:
            break;
        default:
            PIPE_ERR("not support dma(0x%x)user(%s)\n", port.index, szCallerName);
            return MFALSE;
        }
        portID.index = port.index;
        ret = mpCamIOPipe->abortDma(portID);
        if (!ret) {
            PIPE_ERR("abort dma(0x%x) fail", port.index);
            return ret;
        }
    }
    else {
        PIPE_ERR("CamIOPipe not create, user(%s)", szCallerName);
    }

    PIPE_DBG("-");

    return ret;
}

/******************************************************************************
 *
 ******************************************************************************/
MUINT32 NormalSvPipe::getIspReg(MUINT32 RegAddr, MUINT32 RegCount, MUINT32 RegData[], MBOOL bPhysical)
{
    PIPE_ERR("getIspReg() is not supported for Camsv Channel");
    return MTRUE;
}

/******************************************************************************
 *
 ******************************************************************************/
MUINT32 NormalSvPipe::getIspReg(IspP1RegIo *pRegs, MUINT32 RegCount, MBOOL bPhysical)
{
    PIPE_ERR("getIspReg() is not supported for Camsv Channel");
    return MTRUE;
}

/******************************************************************************
 *
 ******************************************************************************/
MUINT32 NormalSvPipe::getIspUniReg(IspP1RegIo *pRegs, MUINT32 RegCount, MBOOL bPhysical)
{
    PIPE_ERR("getIspUniReg() is not supported for Camsv Channel");
    return MTRUE;
}

/******************************************************************************
 *
 ******************************************************************************/
MBOOL NormalSvPipe::sendCommand(MINT32 cmd, MINTPTR arg1, MINTPTR arg2, MINTPTR arg3)
{
    MBOOL   ret = MTRUE;
	MSize *szIMGO = NULL;
    //MUINT32 tg;
    //MINT32  sensorDev = 1;

    if (this->FSM_CHECK(op_cmd, __FUNCTION__) == MFALSE) {
        ret = MFALSE;
        goto _EXIT_SEND_CMD;
    }

    if (!mpCamIOPipe) {
        PIPE_ERR("mpNormalSvIOPipe NULL");
        ret = MFALSE;
        goto _EXIT_SEND_CMD;
    }

    switch (cmd) {
    case ENPipeCmd_GET_TG_INDEX:
#if 1
        if (arg1) {
            *(MINT32*)arg1 = _NORMALSVPIPE_GET_TG_IDX(mpSensorIdx);
        }
        else {
            ret = MFALSE;
            goto _EXIT_SEND_CMD;
        }
#endif
        break;
    case ENPipeCmd_GET_BURSTQNUM:
        if (arg1) {
            *(MINT32*)arg1 = mBurstQNum;
        }
        else {
            ret = MFALSE;
            goto _EXIT_SEND_CMD;
        }
        break;
    case ENPipeCmd_GET_LAST_ENQ_SOF:
        //arg1: [Out] last enque sof_idx
        if (!arg1) {
            PIPE_ERR("NULL param");
            ret = MFALSE;
            goto _EXIT_SEND_CMD;
        }
        *((MUINT32*)arg1) = mpNSvPipeThread->m_LastEnqSof;
        break;
    case ENPipeCmd_SET_IDLE_HOLD_CB:
        if (0 == arg1) {
            ret = MFALSE;
            break;
        }

        mpNSvPipeThread->idleHoldNotifyCB = *(CAM_STATE_NOTIFY *)arg1;
        PIPE_INF("set idle hold cb: %p", mpNSvPipeThread->idleHoldNotifyCB.fpNotifyState);

        break;
    case ENPipeCmd_SET_SUSPEND_STAT_CB:
        if (0 == arg1) {
            ret = MFALSE;
            PIPE_ERR("arg1 ptr cannot be null");
            break;
        }

        mpNSvPipeThread->suspendNotifyCB = *(CAM_STATE_NOTIFY *)arg1;
        PIPE_INF("set suspend state cb: %p", mpNSvPipeThread->suspendNotifyCB.fpNotifyState);

        break;
    case ENPipeCmd_SET_ENQ_THRD_CB:
        if (0 == arg1) {
            ret = MFALSE;
            PIPE_ERR("arg1 ptr cannot be null");
            break;
        }

        mpNSvPipeThread->enqueThrdNotifyCB = *(CAM_STATE_NOTIFY *)arg1;
        PIPE_INF("set suspend state cb: %p", mpNSvPipeThread->enqueThrdNotifyCB.fpNotifyState);

        break;
    case ENPipeCmd_GET_TG_OUT_SIZE:
        //arg1 : width
        //arg2 : height
        if (!arg1 || !arg2) {
            PIPE_ERR("NULL param");
            ret = MFALSE;
            goto _EXIT_SEND_CMD;
        }
        *((MINT32*)arg1) = _NORMALSVPIPE_GET_SENSORCFG_CROP_W(mpSensorIdx);
        *((MINT32*)arg2) = _NORMALSVPIPE_GET_SENSORCFG_CROP_H(mpSensorIdx);
        break;

    case ENPipeCmd_GET_BIN_INFO:
		if(!arg1){
			PIPE_ERR("NULL param");
			ret = MFALSE;
			goto _EXIT_SEND_CMD;
		}
		PIPE_ERR("ENPipeCmd_GET_BIN_INFO w %d, h %d \n", mCamSvImgoOut_size.w, mCamSvImgoOut_size.h);
		*(MUINT32*)arg1 =mCamSvImgoOut_size.w;
		*(MUINT32*)arg2 = mCamSvImgoOut_size.h;

		break;

    case ENPipeCmd_GET_IMGO_INFO:
		if(!arg1){
			PIPE_ERR("NULL param");
			ret = MFALSE;
			goto _EXIT_SEND_CMD;
		}
		szIMGO = (MSize*)arg1;
		// element[0] : stand for processed raw size
		// element[1] : stand for pure raw size
		PIPE_ERR("ENPipeCmd_GET_IMGO_INFO w %d, h %d \n", mCamSvImgoOut_size.w, mCamSvImgoOut_size.h);
		for(MUINT32 i = 0 ; i < 2 ; i++){
			szIMGO[i].w = mCamSvImgoOut_size.w;
			szIMGO[i].h = mCamSvImgoOut_size.h;
		}
		break;

    case ENPipeCmd_GET_CUR_FRM_STATUS:
        ret = mpCamIOPipe->sendCommand(EPIPECmd_GET_CUR_FRM_STATUS, arg1, arg2, arg3);
        //if 1st enq loop does NOT apply patch, MW cannot enque at this timing
        if (MFALSE == m_b1stEnqLoopDone) {
            *(MUINT32*)arg1 = _1st_enqloop_status;
        }
        break;
    case ENPipeCmd_GET_CUR_SOF_IDX:
        if (!mpCamIOPipe) {
            PIPE_ERR("[Error]mpNormalSvIOPipe=NULL");
            ret = MFALSE;
            goto _EXIT_SEND_CMD;
        }
        ret = mpCamIOPipe->sendCommand(EPIPECmd_GET_CUR_SOF_IDX, arg1, arg2, arg3);
        break;
    default:
        ret = MFALSE;
        goto _EXIT_SEND_CMD;
        break;
    }


_EXIT_SEND_CMD:
    if (ret != MTRUE) {
        PIPE_ERR("error: sendCommand fail: (cmd,arg1,arg2,arg3)=(0x%08x,0x%" PRIXPTR ",0x%" PRIXPTR ",0x%" PRIXPTR ")", cmd, arg1, arg2, arg3);
    }
    return ret;
}

/******************************************************************************
 *
 ******************************************************************************/
//arg1 : only valid in cmd:_GetPort_Index
MUINT32 NormalSvPipe::GetOpenedPortInfo(NormalSvPipe::ePortInfoCmd cmd, MUINT32 arg1)
{
    MUINT32 _num = 0;

    switch (cmd) {
        case NormalSvPipe::_GetPort_Opened:
            return (MUINT32)mOpenedPort;
        case NormalSvPipe::_GetPort_OpendNum:
            if (mOpenedPort & NormalSvPipe::_IMGO_ENABLE)
                _num++;
            return _num;
        case NormalSvPipe::_GetPort_Index:
        {
            MUINT32 idx = 0;
            if (mOpenedPort & NormalSvPipe::_IMGO_ENABLE) {
                PortIndex[idx] = PORT_IMGO.index;
                idx++;
            }

            if ((arg1 >= idx) || (arg1 >= ePort_max)) {
                PIPE_ERR("Opened port num not enough %d/%d/%d", arg1, idx, ePort_max);
                return -1;
            }
            else {
                return PortIndex[arg1];
            }
        }
        case NormalSvPipe::_GetPort_ImgFmt:
            if (arg1 != ePort_imgo) {
                PIPE_ERR("Port num not enough %d/%d", arg1, ePort_max);
                return -1;
            }
            return PortImgFormat[arg1];
        default:
            PIPE_ERR("un-supported cmd:0x%x\n",cmd);
            return 0;
    }
}


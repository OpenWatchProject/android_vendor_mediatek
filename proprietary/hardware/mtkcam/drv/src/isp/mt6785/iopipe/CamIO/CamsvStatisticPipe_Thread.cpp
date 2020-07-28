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
#define LOG_TAG "CamsvStatisticPipe_Thread"

#include <sys/prctl.h>
#include <utils/Vector.h>
#include <utils/KeyedVector.h>
using namespace android;

#include <mtkcam/def/common.h>
#include <mtkcam/utils/std/common.h>
#include <mtkcam/utils/metadata/client/mtk_metadata_tag.h>
using namespace NSCam;
using namespace NSCam::Utils;

#include <mtkcam/def/PriorityDefs.h>

#include <utils/Thread.h>
#include <utils/Condition.h>
#include <cutils/properties.h>

#include "CamsvStatisticPipe.h"
#include "IOPipeProfile.h"
#include <Cam/buf_que_ctrl.h>

#undef DBG_LOG_TAG

#ifndef USING_MTK_LDVT
#define DBG_LOG_TAG ""
#else
#define DBG_LOG_TAG LOG_TAG
#endif

#define PTHREAD_CTRL_M (1)
#define DEFAULT_SHUTTERTIME_MS 16000
#define THIS_NAME "CamsvStatisticPipe_Thread"

using namespace NSCam;
using namespace NSIoPipe;
using namespace NSCamIOPipe;
using namespace NSImageio;
using namespace NSIspio;

DECLARE_DBG_LOG_VARIABLE(iocamsvspipet);
#undef PIPE_VRB
#undef PIPE_DBG
#undef PIPE_INF
#undef PIPE_WRN
#undef PIPE_ERR
#undef PIPE_AST
#define PIPE_VRB(fmt, arg...)        do { if (iocamsvspipet_DbgLogEnable_VERBOSE) { BASE_LOG_VRB(fmt, ##arg); } } while(0)
#define PIPE_DBG(fmt, arg...)        do { if (iocamsvspipet_DbgLogEnable_DEBUG  ) { BASE_LOG_DBG(fmt, ##arg); } } while(0)
#define PIPE_INF(fmt, arg...)        do { if (iocamsvspipet_DbgLogEnable_INFO   ) { BASE_LOG_INF(fmt, ##arg); } } while(0)
#define PIPE_WRN(fmt, arg...)        do { if (iocamsvspipet_DbgLogEnable_WARN   ) { BASE_LOG_WRN(fmt, ##arg); } } while(0)
#define PIPE_ERR(fmt, arg...)        do { if (iocamsvspipet_DbgLogEnable_ERROR  ) { BASE_LOG_ERR(fmt, ##arg); } } while(0)
#define PIPE_AST(cond, fmt, arg...)  do { if (iocamsvspipet_DbgLogEnable_ASSERT ) { BASE_LOG_AST(cond, fmt, ##arg); } } while(0)

CamsvStatisticPipe_Thread::CamsvStatisticPipe_Thread(CamsvStatisticPipe* obj)
{
    mpCamsvStatisticPipe = obj;

    m_EnqThrdState = _ThreadIdle;
    mInitCnt = 0;
    m_bStart = MFALSE;
    m_irqUserKey = -1;
    m_EnqueThread = 0;
    m_ShutterTimeMs[0] = 0;
    m_ShutterTimeMs[1] = 0;

    memset(&m_semEnqueThread, 0, sizeof(sem_t));

    DBG_LOG_CONFIG(imageio, iocamsvspipet);
}

CamsvStatisticPipe_Thread* CamsvStatisticPipe_Thread::createInstance(CamsvStatisticPipe* obj)
{
    if (obj == NULL) {
        PIPE_ERR("create instance fail");
        return NULL;
    }

    return new CamsvStatisticPipe_Thread(obj);
}

MBOOL CamsvStatisticPipe_Thread::destroyInstance(void)
{
    delete this;
    return MTRUE;
}

MBOOL CamsvStatisticPipe_Thread::init(void)
{
    MBOOL ret = MTRUE;
    MUINT32 i, r;
    MINT32 tmp = 0;
    #if (PTHREAD_CTRL_M == 1)
    pthread_attr_t attr_e;
    #endif

    PIPE_INF("mInitCount(%d)", mInitCnt);

    if (mInitCnt > 0) {
        tmp = android_atomic_inc(&mInitCnt);
        return ret;
    }

    ::sem_init(&m_semEnqueThread, 0, 0);

    m_EnqThrdState = _ThreadRunning;

    #if (PTHREAD_CTRL_M == 0)
    pthread_create(&m_EnqueThread, NULL, procCamsvEnque, this);
    #else
    if ((r = pthread_attr_init(&attr_e)) != 0) {
        PIPE_ERR("thread ctrl [%s] fail: %d", "set enqThd attr", r);
        return MFALSE;
    }
    if ((r = pthread_attr_setdetachstate(&attr_e, PTHREAD_CREATE_DETACHED)) != 0) {
        PIPE_ERR("thread ctrl [%s] fail: %d", "enqThd detach", r);
        return MFALSE;
    }
    if ((r = pthread_create(&m_EnqueThread, &attr_e, procCamsvEnque, this)) != 0) {
        PIPE_ERR("thread ctrl [%s] fail: %d", "enqThd create", r);
        return MFALSE;
    }
    if ((r = pthread_attr_destroy(&attr_e)) != 0) {
        PIPE_ERR("thread ctrl [%s] fail: %d", "destroy enqThd attr", r);
        return MFALSE;
    }
    #endif

    tmp = android_atomic_inc(&mInitCnt);

    return ret;
}

MBOOL CamsvStatisticPipe_Thread::uninit(void)
{
#define MAX_LOOP_CNT 10000
    MINT32 tmp = 0;
    MUINT32 enqLoopCnt = 0;

    PIPE_INF("mInitCount(%d)", mInitCnt);

    if (mInitCnt <= 0) {
        return MTRUE;
    }

    tmp = android_atomic_dec(&mInitCnt);

    if (mInitCnt > 0) {
        return MTRUE;
    }

    if (_ThreadRunning == m_EnqThrdState) {
        ::sem_post(&m_semEnqueThread);

        PIPE_INF("Post for enqThread exit");
        while(_ThreadRunning == m_EnqThrdState) {
            if (enqLoopCnt++ > MAX_LOOP_CNT) {
                PIPE_ERR("TIMEOUT: enque thread state(%d) not changed for a long while", m_EnqThrdState);
                break;
            }
            usleep(500);
        }

        ::sem_wait(&m_semEnqueThread); // wait thread exit
    }

    return MTRUE;
}

MBOOL CamsvStatisticPipe_Thread::start(void)
{
    PIPE_DBG("Thd: E");

    mLock.lock();
    m_bStart = MTRUE;
    mLock.unlock();

    ::sem_post(&m_semEnqueThread);

    PIPE_DBG("Thd: X");

    return MTRUE;
}

MBOOL CamsvStatisticPipe_Thread::stop(MBOOL bForce)
{
    IOPipeTrace _traceObj("CamsvStatisticPipe_Thread::stop", IOPIPE_TRACE_EN_CAMSVPIPE);

    PIPE_DBG("Thd: E");

    if (this->m_bStart == MTRUE) {
        this->prestop();
    }

    if(bForce == MTRUE) {
        PIPE_INF("N:%d signal enqThr", mpCamsvStatisticPipe->mpSensorIdx);
        mpCamsvStatisticPipe->signal(EPipeSignal_SOF, this->m_irqUserKey);
        mpCamsvStatisticPipe->abortDma(PORT_CAMSV_IMGO, THIS_NAME);
    }

    PIPE_INF("N:%d:Thd: wait enq", mpCamsvStatisticPipe->mpSensorIdx);
    ::sem_wait(&m_semEnqueThread); // wait until thread prepare to leave

    PIPE_DBG("Thd: X");

    return MTRUE;
}

MBOOL CamsvStatisticPipe_Thread::prestop(void)
{
    this->mLock.lock();
    this->m_bStart = MFALSE;
    this->mLock.unlock();

    return MTRUE;
}

MUINT32 CamsvStatisticPipe_Thread::estimateFrameTime(void)
{
#define SOF_TIMEOUT_TH (1500)
    MUINT32 _wait_time = 0, _t0 = m_ShutterTimeMs[0], _t1 = m_ShutterTimeMs[1];

    if (_t0 > _t1) {
        _wait_time = (_t0 > SOF_TIMEOUT_TH)? _t0: SOF_TIMEOUT_TH;
    }
    else {
        _wait_time = (_t1 > SOF_TIMEOUT_TH)? _t1: SOF_TIMEOUT_TH;
    }

    _wait_time *= 2;

    return _wait_time;
}

MBOOL CamsvStatisticPipe_Thread::enque()
{
    MBOOL ret = MTRUE;
    MUINT32 numReq;
    QBufInfo rQBuf;
    NSImageio::NSIspio::PortID portID;
    NSImageio::NSIspio::QBufInfo rQBufInfo;
    CamsvStatisticPipe* pPipe = mpCamsvStatisticPipe;

    pPipe->mEnqueContainerLock.lock();
    pPipe->mpEnqueContainer->sendCmd(QueueMgr<QBufInfo>::eCmd_getsize, (MUINTPTR)&numReq, 0);
    if(numReq == 0){
        PIPE_DBG("no request, bypass enque");
        pPipe->mEnqueContainerLock.unlock();
        goto _EXIT_ENQ;
    }
    pPipe->mpEnqueContainer->sendCmd(QueueMgr<QBufInfo>::eCmd_front, (MUINTPTR)&rQBuf, 0);
    pPipe->mEnqueContainerLock.unlock();

    if(rQBuf.mvOut.size()){
        pPipe->mpbuf_camsv_mnum[pPipe->ring_cnt] = rQBuf.mvOut.at(0).FrameBased.mMagicNum_tuning;
    }
    else{
        pPipe->mpbuf_camsv_mnum[pPipe->ring_cnt] = 0xbeefdead;
    }

    for(MUINT32 i = 0; i < pPipe->m_subSample + 1 ; i++){
        portID.index = EPortIndex_IMGO;

        rQBufInfo.vBufInfo.resize(pPipe->m_subSample + 1);

        rQBufInfo.vBufInfo[i].u4BufSize[ePlane_1st] = (MUINT32)pPipe->mpbuf_camsv_imgo[pPipe->ring_cnt * (pPipe->m_subSample + 1) + i].size;
        rQBufInfo.vBufInfo[i].u4BufVA[ePlane_1st] = (MUINTPTR)pPipe->mpbuf_camsv_imgo[pPipe->ring_cnt * (pPipe->m_subSample +1 ) + i].virtAddr;
        rQBufInfo.vBufInfo[i].u4BufPA[ePlane_1st] = (MUINTPTR)pPipe->mpbuf_camsv_imgo[pPipe->ring_cnt * (pPipe->m_subSample + 1) + i].phyAddr;
        rQBufInfo.vBufInfo[i].Frame_Header.u4BufSize[ePlane_1st] = (MUINT32)pPipe->mpbuf_camsv_imgo_fh[pPipe->ring_cnt * (pPipe->m_subSample + 1) + i].size;
        rQBufInfo.vBufInfo[i].Frame_Header.u4BufVA[ePlane_1st] = (MUINTPTR)pPipe->mpbuf_camsv_imgo_fh[pPipe->ring_cnt * (pPipe->m_subSample + 1) + i].virtAddr;
        rQBufInfo.vBufInfo[i].Frame_Header.u4BufPA[ePlane_1st] = (MUINTPTR)pPipe->mpbuf_camsv_imgo_fh[pPipe->ring_cnt * (pPipe->m_subSample + 1) + i].phyAddr;
        rQBufInfo.vBufInfo[i].img_w = pPipe->camsv_imgo_crop_w;
        rQBufInfo.vBufInfo[i].img_h = pPipe->camsv_imgo_crop_h;
        rQBufInfo.vBufInfo[i].crop_win.p.x = pPipe->camsv_imgo_crop_x;
        rQBufInfo.vBufInfo[i].crop_win.p.y = pPipe->camsv_imgo_crop_y;
        rQBufInfo.vBufInfo[i].crop_win.s.w = pPipe->camsv_imgo_crop_w;
        rQBufInfo.vBufInfo[i].crop_win.s.h = pPipe->camsv_imgo_crop_h;
        PIPE_DBG("enque_imgo_%d: size:0x%08x_0x%08x, pa:0x%p_0x%p, magic:0x%x, crop:%d_%d_%d_%d, ring_cnt:%d",\
            i,\
            rQBufInfo.vBufInfo.at(i).u4BufSize[ePlane_1st],\
            rQBufInfo.vBufInfo.at(i).Frame_Header.u4BufSize[ePlane_1st],\
            (void*)rQBufInfo.vBufInfo.at(i).u4BufPA[ePlane_1st],\
            (void*)rQBufInfo.vBufInfo.at(i).Frame_Header.u4BufPA[ePlane_1st],\
            rQBufInfo.vBufInfo.at(i).m_num,\
            pPipe->camsv_imgo_crop_x, pPipe->camsv_imgo_crop_y,\
            pPipe->camsv_imgo_crop_w, pPipe->camsv_imgo_crop_h,\
            pPipe->ring_cnt);
    }

    if(rQBuf.mShutterTimeNs){
        pPipe->mpCamsvIOPipe->sendCommand(EPIPECmd_SET_FRM_TIME, (MINTPTR)(rQBuf.mShutterTimeNs / 1000000), 1, 0);
    }
    else{
        PIPE_DBG("Warning:TG_Camsv_%d Shutter Time is 0!", pPipe->mTgInfoCamsv);
        pPipe->mpCamsvIOPipe->sendCommand(EPIPECmd_SET_FRM_TIME, (MINTPTR)(DEFAULT_SHUTTERTIME_MS), 1, 0);
    }
    if (pPipe->mpCamsvIOPipe->enqueOutBuf(portID, rQBufInfo) == MFALSE) {
        ret = MFALSE;
        PIPE_ERR("error:TG_Camsv_%d port_%d enque fail", pPipe->mTgInfoCamsv, portID.index);
        goto _EXIT_ENQ;
    }

    pPipe->ring_cnt = (pPipe->ring_cnt + 1) % CAMSV_BUF_DEPTH;

    pPipe->mEnqueContainerLock.lock();
    pPipe->mpEnqueContainer->sendCmd(QueueMgr<QBufInfo>::eCmd_pop, 0, 0);
    pPipe->mEnqueContainerLock.unlock();

_EXIT_ENQ:
    return ret;
}

MINT32 CamsvStatisticPipe_Thread::idleStateHold(MUINT32 hold, E_ThreadGrop thdGrpBm)
{
    if (hold) {
        PIPE_INF("Holding idle locks grp: 0x%x", thdGrpBm);

        if ((thdGrpBm & eThdGrp_Enq) == eThdGrp_Enq) {
            mThreadIdleLock_Enq.lock();
        }
    } else {
        if ((thdGrpBm & eThdGrp_Enq) == eThdGrp_Enq) {
            mThreadIdleLock_Enq.unlock();
        }

        PIPE_INF("Released idle locks grp: 0x%x", thdGrpBm);
    }

    return 0;
}

MBOOL CamsvStatisticPipe_Thread::updateFrameTime(MUINT32 timeMs, MUINT32 offset)
{
    if (offset >= 2) {
        PIPE_ERR("Wrong time index");
        return MFALSE;
    }

    m_ShutterTimeMs[offset] = timeMs;

    return MTRUE;
}

MVOID* CamsvStatisticPipe_Thread::procCamsvEnque(void *arg)
{
    CamsvStatisticPipe_Thread *_this = reinterpret_cast<CamsvStatisticPipe_Thread*>(arg);
    MUINT32 _sof_idx, _sof_idx2, ret;
    char userName[64];

    int const policy    = SCHED_OTHER;
    int const priority  = NICE_CAMERA_LOMO - 5;
    struct sched_param sched_p;

    ::sched_getparam(0, &sched_p);
    if (policy == SCHED_OTHER) {
        setThreadPriority(policy, priority);
    }
    else {
        sched_p.sched_priority = priority;
        ::sched_setscheduler(0, policy, &sched_p);
    }
    //  detach thread => cannot be join, it means that thread would release resource after exit
    #if (PTHREAD_CTRL_M == 0)
    ::pthread_detach(::pthread_self());
    #endif

    ::sem_wait(&_this->m_semEnqueThread);

    _this->mThreadIdleLock_Enq.lock();

    if (MFALSE == _this->m_bStart) {
        PIPE_INF("enqThread early exit");
        goto _exit;
    }

    PIPE_INF("WARNING: temp borrow NICE_CAMERA_LOMO priority, need to create self priority");
    switch (_this->mpCamsvStatisticPipe->mTgInfoCamsv) {
    case TG_CAMSV_0:  ::prctl(PR_SET_NAME,"IspEnqThd_STT_SV1",0,0,0);
        break;
    case TG_CAMSV_1:  ::prctl(PR_SET_NAME,"IspEnqThd_STT_SV2",0,0,0);
        break;
    case TG_CAMSV_2:  ::prctl(PR_SET_NAME,"IspEnqThd_STT_SV3",0,0,0);
        break;
    case TG_CAMSV_3:  ::prctl(PR_SET_NAME,"IspEnqThd_STT_SV4",0,0,0);
        break;
    case TG_CAMSV_4:  ::prctl(PR_SET_NAME,"IspEnqThd_STT_SV5",0,0,0);
        break;
    case TG_CAMSV_5:  ::prctl(PR_SET_NAME,"IspEnqThd_STT_SV6",0,0,0);
        break;
    case TG_CAMSV_6:  ::prctl(PR_SET_NAME,"IspEnqThd_STT_SV7",0,0,0);
        break;
    case TG_CAMSV_7:  ::prctl(PR_SET_NAME,"IspEnqThd_STT_SV8",0,0,0);
        break;
    default:
        PIPE_ERR("ERROR: TG:0x%x(0x%x)", _this->mpCamsvStatisticPipe->mTgInfoCamsv,\
                _this->mpCamsvStatisticPipe->mpSensorIdx);
        goto _exit;
    }

    sprintf(userName, "%s_%d", THIS_NAME, _this->mpCamsvStatisticPipe->mTgInfoCamsv);
    _this->m_irqUserKey = _this->mpCamsvStatisticPipe->attach(userName);
    if (_this->m_irqUserKey < 0) {
        PIPE_ERR("ERROR: attach IRQ failed %d", _this->m_irqUserKey);
        goto _exit;
    }
    PIPE_INF("Got irqUserKey(%d)", _this->m_irqUserKey);

    while (1) {
        if (MFALSE == _this->m_bStart) {
            PIPE_DBG("enqThread leave");
            break;
        }

        _this->mThreadIdleLock_Enq.unlock();
        ret = _this->mpCamsvStatisticPipe->wait(EPipeSignal_SOF, EPipeSignal_ClearWait, _this->m_irqUserKey, _this->estimateFrameTime(), NULL);
        _this->mThreadIdleLock_Enq.lock();
        if (MFALSE == ret) {
            PIPE_ERR("ERROR: wait SOF fail");
            continue;
        }

        if (_this->m_bStart == MFALSE) {
            PIPE_DBG("enqThread leave after wait SOF\n");
            break;
        }

        _this->mpCamsvStatisticPipe->sendCommand(ECAMSVSPipeCmd_GET_CAMSV_GET_CUR_SOF_IDX, (MINTPTR)&_sof_idx, 0, 0);
        PIPE_DBG("N:%d: cur SOF: %d", _this->mpCamsvStatisticPipe->mpSensorIdx, _sof_idx);

        _this->enque();

        _this->mpCamsvStatisticPipe->sendCommand(ECAMSVSPipeCmd_GET_CAMSV_GET_CUR_SOF_IDX, (MINTPTR)&_sof_idx2, 0, 0);
        if(_sof_idx != _sof_idx2){
            PIPE_INF("WARNING: N:%d: enque over vsync(%d_%d)", _this->mpCamsvStatisticPipe->mpSensorIdx,\
                    _sof_idx, _sof_idx2);
        }
    }
_exit:
    _this->m_EnqThrdState = _ThreadFinish;

    _this->mThreadIdleLock_Enq.unlock();

    ::sem_post(&_this->m_semEnqueThread);

    return NULL;
}

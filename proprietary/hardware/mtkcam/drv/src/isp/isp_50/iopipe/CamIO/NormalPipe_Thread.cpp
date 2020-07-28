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
#define LOG_TAG     "NormalPipe_Thread"

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
#include "NormalPipe.h"
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

#define THIS_NAME           "NormalPipe_Thread"

using namespace NSCam;
using namespace NSIoPipe;
using namespace NSCamIOPipe;
using namespace NSImageio;
using namespace NSIspio;

DECLARE_DBG_LOG_VARIABLE(iopipet);
#undef PIPE_VRB
#undef PIPE_DBG
#undef PIPE_INF
#undef PIPE_WRN
#undef PIPE_ERR
#undef PIPE_AST
#define PIPE_VRB(fmt, arg...)        do { if (iopipet_DbgLogEnable_VERBOSE) { BASE_LOG_VRB(fmt, ##arg); } } while(0)
#define PIPE_DBG(fmt, arg...)        do { if (iopipet_DbgLogEnable_DEBUG  ) { BASE_LOG_DBG(fmt, ##arg); } } while(0)
#define PIPE_INF(fmt, arg...)        do { if (iopipet_DbgLogEnable_INFO   ) { BASE_LOG_INF(fmt, ##arg); } } while(0)
#define PIPE_WRN(fmt, arg...)        do { if (iopipet_DbgLogEnable_WARN   ) { BASE_LOG_WRN(fmt, ##arg); } } while(0)
#define PIPE_ERR(fmt, arg...)        do { if (iopipet_DbgLogEnable_ERROR  ) { BASE_LOG_ERR(fmt, ##arg); } } while(0)
#define PIPE_AST(cond, fmt, arg...)  do { if (iopipet_DbgLogEnable_ASSERT ) { BASE_LOG_AST(cond, fmt, ##arg); } } while(0)

#include "NormalPipeDef.h"


NormalPipe_Thread::NormalPipe_Thread(NormalPipe* obj)
{
    m_bStart        = MFALSE;
    mCnt            = 0;
    m_pNormalpipe   = (NormalPipe*)obj;
    m_ConsumedReqCnt = 0;
    m_LastEnqSof    = 0;
    m_EnqThrdState  = _ThreadIdle;
    m_DeqThrdState  = _ThreadIdle;
    m_irqUserKey    = -1;
    m_ShutterTimeMs[0] = 0;
    m_ShutterTimeMs[1] = 0;

    idleHoldNotifyCB.fpNotifyState = NULL;
    idleHoldNotifyCB.Obj = NULL;
    suspendNotifyCB.fpNotifyState = NULL;
    suspendNotifyCB.Obj = NULL;
    enqueThrdNotifyCB.fpNotifyState = NULL;
    enqueThrdNotifyCB.Obj = NULL;
    m_EnqueThread = 0;
    m_DequeThread = 0;
    mpSecMgr = NULL;

    memset(&m_semEnqueThread, 0, sizeof(sem_t));
    memset(&m_semDequeThread, 0, sizeof(sem_t));
    memset(&m_semDeqLoopToken, 0, sizeof(sem_t));
    memset(&m_semDeqDonePort, 0, sizeof(sem_t));

    DBG_LOG_CONFIG(imageio, iopipet);
}

NormalPipe_Thread* NormalPipe_Thread::createInstance(NormalPipe* obj)
{
    if (obj == 0) {
        PIPE_ERR("create instance fail");
        return NULL;
    }

    return new NormalPipe_Thread(obj);
}

MBOOL NormalPipe_Thread::destroyInstance(void)
{
    delete this;
    return MTRUE;
}

MBOOL NormalPipe_Thread::init(MBOOL EnableSec)
{
    MBOOL ret = MTRUE;
    MUINT32 i, r;
    MINT32 tmp=0;
    #if (PTHREAD_CTRL_M == 1)
    pthread_attr_t  attr_e, attr_d;
    #endif

    PIPE_INF("mInitCount(%d)", this->mCnt);

    if (mCnt > 0) {
        tmp = android_atomic_inc((volatile MINT32*)&mCnt);
        return ret;
    }

    if(EnableSec){
        mpSecMgr = SecMgr::SecMgr_GetMgrObj();

        if(mpSecMgr == NULL){
            PIPE_ERR("Secure cam init fail");
            return MFALSE;
        }
    }

    ::sem_init(&m_semEnqueThread, 0, 0);
    ::sem_init(&m_semDequeThread, 0, 0);
    ::sem_init(&m_semDeqLoopToken, 0, 0);
    for (i = 0; i < ePort_max; i++) {
        ::sem_init(&m_semDeqDonePort[i], 0, 0);
    }

    m_EnqThrdState = _ThreadRunning;
    m_DeqThrdState = _ThreadRunning;

    #if (PTHREAD_CTRL_M == 0)
    pthread_create(&m_EnqueThread, NULL, ispEnqueThread, this);
    pthread_create(&m_DequeThread, NULL, ispDequeThread, this);
    #else
    /* Create main thread for preview and capture */
    if ((r = pthread_attr_init(&attr_e)) != 0) {
        PIPE_ERR("thread ctrl [%s] fail: %d", "set enqThd attr", r);
        return MFALSE;
    }
    if ((r = pthread_attr_init(&attr_d)) != 0) {
        PIPE_ERR("thread ctrl [%s] fail: %d", "set deqThd attr", r);
        return MFALSE;
    }
    if ((r = pthread_attr_setdetachstate(&attr_e, PTHREAD_CREATE_DETACHED)) != 0) {
        PIPE_ERR("thread ctrl [%s] fail: %d", "enqThd detach", r);
        return MFALSE;
    }
    if ((r = pthread_attr_setdetachstate(&attr_d, PTHREAD_CREATE_DETACHED)) != 0) {
        PIPE_ERR("thread ctrl [%s] fail: %d", "deqThd detach", r);
        return MFALSE;
    }
    if ((r = pthread_create(&m_EnqueThread, &attr_e, ispEnqueThread, this)) != 0) {
        PIPE_ERR("thread ctrl [%s] fail: %d", "enqThd create", r);
        return MFALSE;
    }
    if ((r = pthread_create(&m_DequeThread, &attr_d, ispDequeThread, this)) != 0) {
        PIPE_ERR("thread ctrl [%s] fail: %d", "deqThd create", r);
    }
    if ((r = pthread_attr_destroy(&attr_e)) != 0) {
        PIPE_ERR("thread ctrl [%s] fail: %d", "destroy enqThd attr", r);
        return MFALSE;
    }
    if ((r = pthread_attr_destroy(&attr_d)) != 0) {
        PIPE_ERR("thread ctrl [%s] fail: %d", "destroy deqThd attr", r);
        return MFALSE;
    }
    #endif

    tmp = android_atomic_inc((volatile MINT32*)&mCnt);

    return ret;
}

MBOOL NormalPipe_Thread::uninit(void)
{
    MINT32 tmp=0;

    PIPE_INF("mInitCount(%d)", mCnt);

    if (mCnt <= 0) {
        return MTRUE;
    }

    tmp = android_atomic_dec((volatile MINT32*)&mCnt);

    if (mCnt > 0) {
        return MTRUE;
    }

    if (_ThreadRunning == m_EnqThrdState) {
        ::sem_post(&m_semEnqueThread);

        PIPE_INF("Post for enqThread exit");
        while(_ThreadRunning == m_EnqThrdState) {
            usleep(500);
        }

        ::sem_wait(&m_semEnqueThread); //wait thread exit
    }
    if (_ThreadRunning == m_DeqThrdState) {
        ::sem_post(&m_semDequeThread);

        PIPE_INF("Post for deqThread exit");
        while(_ThreadRunning == m_DeqThrdState) {
            usleep(500);
        }

        ::sem_wait(&m_semDequeThread); //wait thread exit
    }

    return MTRUE;
}


MBOOL NormalPipe_Thread::start(void)
{
    PIPE_DBG("Thd: E");

    mLock.lock();
    m_bStart = MTRUE;
    mLock.unlock();

    ::sem_post(&m_semEnqueThread);
    ::sem_post(&m_semDequeThread);

    PIPE_DBG("Thd: X");

    return MTRUE;
}

MBOOL NormalPipe_Thread::stop(MBOOL bForce)
{
    IOPipeTrace _traceObj("NormalPipeThread::stop", IOPIPE_TRACE_EN_NOMALPIPE);

    MUINT32 i = 0, _OpenedPort = 0;

    PIPE_DBG("Thd: E");

    if (this->m_bStart == MTRUE) {
        this->prestop();
    }

    ::sem_post(&m_semDeqLoopToken);

    if(bForce == MTRUE) {
        /*abort enqThr*/
        PIPE_INF("N:%d signal enqThr", m_pNormalpipe->mpSensorIdx);

        this->m_pNormalpipe->signal(EPipeSignal_SOF, this->m_irqUserKey);

        /*abort deqThr*/
        _OpenedPort = m_pNormalpipe->GetOpenedPortInfo(NormalPipe::_GetPort_OpendNum);
        for (i = 0; i < _OpenedPort; i++) {
            m_pNormalpipe->abortDma(m_pNormalpipe->GetOpenedPortInfo(NormalPipe::_GetPort_Index, i), THIS_NAME);
        }
    }

    //PIPE_INF("Thd: join...");
    //pthread_join(m_EnqueThread, NULL);// <-- while still in SOF_CB, this may causes resource to be freed?? TBD fixme
    //pthread_join(m_DequeThread, NULL);

    PIPE_INF("N:%d:Thd: wait enq", m_pNormalpipe->mpSensorIdx);
    ::sem_wait(&m_semEnqueThread); // wait until thread prepare to leave
    PIPE_INF("N:%d:Thd: wait deq", m_pNormalpipe->mpSensorIdx);
    ::sem_wait(&m_semDequeThread); // wait until thread prepare to leave

    /* Prevent NormalPipe::deque blocking */
    PIPE_INF("N:%d:Thd: post for deque", m_pNormalpipe->mpSensorIdx);
    _OpenedPort = m_pNormalpipe->GetOpenedPortInfo(NormalPipe::_GetPort_OpendNum);
    for (i = 0; i < _OpenedPort; i++) {
        ::sem_post(&m_semDeqDonePort[NPIPE_MAP_PORTID(m_pNormalpipe->GetOpenedPortInfo(NormalPipe::_GetPort_Index, i))]);
    }

    PIPE_DBG("Thd: X");

    return MTRUE;
}

MBOOL NormalPipe_Thread::prestop(void)
{
    this->mLock.lock();
    this->m_bStart = MFALSE;
    this->mLock.unlock();

    return MTRUE;
}


/**
    this enque run in immediate mode,enque 1 port with 1 buffer at 1 time.

* return value:
* 1: isp is already stopped
* 0: sucessed
*-1: fail
*/
MINT32 NormalPipe_Thread::enque(QBufInfo* pQBuf)
{
    NSImageio::NSIspio::PortID      portID;
    NSImageio::NSIspio::QBufInfo    rQBufInfo;
    MINT32  ret = 0;
    MUINT32 i = 0, _port, PlaneMax = 0, _size = pQBuf->mvOut.size();
    MUINT32 imgHdrBufNum = 0, _idx = 0;
    MINT    ImgFormat = 0;
    MUINT32 SecPA = 0;

    PIPE_DBG("Thd: E");

    rQBufInfo.vBufInfo.resize(_size);

    ImgFormat = pQBuf->mvOut.at(0).mBuffer->getImgFormat();
    portID.index = pQBuf->mvOut.at(0).mPortID.index;
    _port = NPIPE_MAP_PORTID(portID.index);

    imgHdrBufNum = IMG_HEADER_BUF_NUM * m_pNormalpipe->mBurstQNum;
    _idx = m_pNormalpipe->mImgHdrIdx[_port] % imgHdrBufNum;

    if (CHECK_FMT_MISMATCH(portID.index, m_pNormalpipe->mCfgFmt[_port], ImgFormat) == MFALSE) {
        PIPE_ERR("port_%d enque UF/PAK fmt mismatch with configpipe phrase(0x%x/0x%x)",
            portID.index, m_pNormalpipe->mCfgFmt[_port], ImgFormat);
        return -1;
    }

    for (i = 0; i < _size; i++, _idx++) {
        NormalPipe::IMEM_MUTI_BUF_INFO imBuf;

        if (!m_pNormalpipe->mpImgHeaderMgr[_port].sendCmd(
                QueueMgr<NormalPipe::IMEM_MUTI_BUF_INFO>::eCmd_at, _idx, (MUINTPTR)&imBuf)) {
            PIPE_ERR("port_%d out of imgheader buf", portID.index);
            return -1;
        }

        rQBufInfo.vBufInfo.at(i).Frame_Header.bReplace = MFALSE;

        PlaneMax = (CHECK_FMT_UF(ImgFormat))  ? (MUINT32)ePlane_3rd : (MUINT32)ePlane_1st;

        for (MUINT32 PlaneID = 0; PlaneID <= PlaneMax; PlaneID++) {
            rQBufInfo.vBufInfo.at(i).Frame_Header.u4BufSize[PlaneID] = imBuf.ImemBufInfo[PlaneID].size;
            rQBufInfo.vBufInfo.at(i).Frame_Header.u4BufVA[PlaneID]   = imBuf.ImemBufInfo[PlaneID].virtAddr;
            rQBufInfo.vBufInfo.at(i).Frame_Header.u4BufPA[PlaneID]   = imBuf.ImemBufInfo[PlaneID].phyAddr;
            rQBufInfo.vBufInfo.at(i).Frame_Header.memID[PlaneID]     = imBuf.ImemBufInfo[PlaneID].memID;
            rQBufInfo.vBufInfo.at(i).Frame_Header.bufSecu[PlaneID]   = imBuf.ImemBufInfo[PlaneID].bufSecu;
            rQBufInfo.vBufInfo.at(i).Frame_Header.bufCohe[PlaneID]   = imBuf.ImemBufInfo[PlaneID].bufCohe;

            if(imBuf.ImemBufInfo[PlaneID].virtAddr == 0) {
                PIPE_ERR("[%d]null va, PA[%d]: 0x%" PRIXPTR " VA[%d]: 0x%" PRIXPTR "", i,
                    PlaneID, imBuf.ImemBufInfo[PlaneID].phyAddr, PlaneID, imBuf.ImemBufInfo[PlaneID].virtAddr);
            }
        }
        if(mpSecMgr && pQBuf->mvOut.at(i).mSecon) {
            mpSecMgr->SecMgr_QuerySecMVAFromHandle((MUINT64)pQBuf->mvOut.at(i).mPa,&SecPA);
            /*FIXME Secure CAM*/
            rQBufInfo.vBufInfo.at(i).u4BufPA[ePlane_1st] = SecPA;
            NPIPE_PASS_ENQ_BUFINFO(pQBuf->mvOut.at(i), rQBufInfo.vBufInfo.at(i),MTRUE);
        }
        else {
            NPIPE_PASS_ENQ_BUFINFO(pQBuf->mvOut.at(i), rQBufInfo.vBufInfo.at(i),MFALSE);
        }
    }

    if (m_pNormalpipe->mpCamIOPipe) {
        ret = m_pNormalpipe->mpCamIOPipe->enqueOutBuf(portID, rQBufInfo, 1);
        if (ret < 0) {
            PIPE_ERR("error:enqueOutBuf");
            return -1;
        }
        else if(ret > 0) {
            PIPE_WRN("enq:already stopped, ma:x%x",portID.index);
            return 1;
        }
        else {
            for (i = 0; i < _size; i++) {
                m_pNormalpipe->mImgHdrIdx[_port] = (m_pNormalpipe->mImgHdrIdx[_port] + 1) % imgHdrBufNum;

                m_pNormalpipe->mImgHdrLock.lock();
                m_pNormalpipe->mImgHdrCnt[_port]--;
                m_pNormalpipe->mImgHdrLock.unlock();

                #ifdef USE_IMAGEBUF_HEAP
                if(mpSecMgr && pQBuf->mvOut.at(i).mSecon)
                    m_pNormalpipe->mpFrameMgr->enque(pQBuf->mvOut.at(i).mBuffer,rQBufInfo.vBufInfo.at(i).u4BufPA[ePlane_1st]);
                else
                    m_pNormalpipe->mpFrameMgr->enque(pQBuf->mvOut.at(i).mBuffer,0);
                #endif
            }
        }
    }

    PIPE_DBG("Thd: X");

    return ret;
}


MBOOL NormalPipe_Thread::dmaCfg(QBufInfo* pQBuf)
{
    NSImageio::NSIspio::STImgCrop   imgCrop;
    NSImageio::NSIspio::STImgResize imgRsz;
    list<NSImageio::NSIspio::STImgCrop>     listImgCrop;
    list<NSImageio::NSIspio::STImgResize>   listImgRsz;
    list<MUINT32>                           listImgRawType;
    NSCam::NSIoPipe::PortID port;
    BufInfo *pBuf;
    MBOOL   ret = MTRUE;
    MUINT32 _cmd, i = 0;
    MINTPTR _arg1 = 0, _arg2 = 0;

    PIPE_DBG("+");

    if (0 == pQBuf->mvOut.size()) {
        PIPE_ERR("qBuf no buf");
        return MFALSE;
    }

    port = pQBuf->mvOut.at(0).mPortID;

    switch (port.index) {
    case EPortIndex_RRZO:
        _arg1 = (MINTPTR)&listImgCrop;
        _arg2 = (MINTPTR)&listImgRsz;
        _cmd = EPIPECmd_SET_RRZ;
        break;
    case EPortIndex_IMGO:
        _arg1 = (MINTPTR)&listImgCrop;
        _arg2 = (MINTPTR)&listImgRawType;
        _cmd = EPIPECmd_SET_IMGO;
        break;
    case EPortIndex_EISO:
    case EPortIndex_LCSO:
    case EPortIndex_RSSO:
        goto _BYPASS_DMA_CFG;
    default:
        PIPE_ERR("N:%d dma:x%x r not supported in dmaCfg()", m_pNormalpipe->mpSensorIdx, port.index);
        return MFALSE;
    }

    for (i = 0; i < pQBuf->mvOut.size(); i++) {
        pBuf = &(pQBuf->mvOut.at(i));

        if ((EPortIndex_IMGO == port.index) || (EPortIndex_RRZO == port.index)) {
            imgCrop.x       = pBuf->FrameBased.mCropRect.p.x;
            imgCrop.y       = pBuf->FrameBased.mCropRect.p.y;
            imgCrop.floatX  = 0;
            imgCrop.floatY  = 0;
            imgCrop.w       = pBuf->FrameBased.mCropRect.s.w;
            imgCrop.h       = pBuf->FrameBased.mCropRect.s.h;
            imgCrop.img_fmt = (EImageFormat)pBuf->mBuffer->getImgFormat();

            switch(port.index){
                case EPortIndex_IMGO://only for IMGO dynamic UFO/PAK/UNPAK
                    imgCrop.enqueue_img_stride = pBuf->mBuffer->getBufStridesInBytes(ePlane_1st);
                    break;
                default:
                    break;
            }

            listImgCrop.push_back(imgCrop);
        }
        if (EPortIndex_IMGO == port.index) {
            listImgRawType.push_back(pBuf->mRawOutFmt);
        }
        if (EPortIndex_RRZO == port.index) {
            imgRsz.src_x    = pBuf->FrameBased.mCropRect.p.x;
            imgRsz.src_y    = pBuf->FrameBased.mCropRect.p.y;
            imgRsz.src_w    = pBuf->FrameBased.mCropRect.s.w;
            imgRsz.src_h    = pBuf->FrameBased.mCropRect.s.h;
            imgRsz.tar_x    = 0;
            imgRsz.tar_y    = 0;
            imgRsz.tar_w    = pBuf->FrameBased.mDstSize.w;
            imgRsz.tar_h    = pBuf->FrameBased.mDstSize.h;
            listImgRsz.push_back(imgRsz);
        }

        PIPE_DBG("N:%d dma:x%x cmd:0x%x, crop:(%d,%d,%d,%d), outsize:(%d,%d)",\
                m_pNormalpipe->mpSensorIdx, port.index, _cmd, imgCrop.x, imgCrop.y,\
                imgCrop.w, imgCrop.h, pBuf->FrameBased.mDstSize.w, pBuf->FrameBased.mDstSize.h);
    }

    if (m_pNormalpipe->mpCamIOPipe) {
        //Arg1: list<STImgCrop>*    [RRZO][IMGO]
        //Arg2: list<STImgResize>*  [RRZO] //only tar_w & tar_h r useful here.

        //for de-tuningQueue input

        //lock rrz in order to update rrzo size for EIS to get rrz size
        NormalPipe::NPipeGLock.lock();

        ret = m_pNormalpipe->mpCamIOPipe->sendCommand(_cmd, _arg1, _arg2, 0);
        if (MFALSE == ret) {
            PIPE_ERR("Error: EPIPECmd_SET_RRZ/IMGO(0x%x) Fail\n",_cmd);
            NormalPipe::NPipeGLock.unlock();
            return MFALSE;
        }

        NormalPipe::NPipeGLock.unlock();
    }

_BYPASS_DMA_CFG:
    PIPE_DBG("-");

    return ret;
}


MBOOL NormalPipe_Thread::tuningUpdate(list<MUINT32>* pLMagicNum)
{
    PIPE_DBG("N:%d M:x%x~x%x", m_pNormalpipe->mpSensorIdx, pLMagicNum->front(), pLMagicNum->back());

    m_pNormalpipe->mLMagicNum = *pLMagicNum;
    //Arg2: magic number, list<MUINT32>
    //Arg3: senIdx, MUINT32, not a list
    return m_pNormalpipe->mpCamIOPipe->sendCommand(EPIPECmd_SET_P1_UPDATE, 0, (MINTPTR)pLMagicNum,
            (MINTPTR)(m_pNormalpipe->mpSensorIdx));
}

MBOOL NormalPipe_Thread::updateFrameTime(MUINT32 timeMs, MUINT32 offset)
{
    if (offset >= 2) {
        PIPE_ERR("Wrong time index");
        return MFALSE;
    }

    m_ShutterTimeMs[offset] = timeMs;

    return MTRUE;
}

MUINT32 NormalPipe_Thread::estimateFrameTime(void)
{
#define SOF_TIMEOUT_TH          (1500)
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

E_EnqueRst NormalPipe_Thread::frameToEnque(void)
{
    QBufInfo        _qBuf;
    list<MUINT32>   listMagic;
    MUINT32         i, _port, sofIdx = 0, _OpenedPort , AvailableBuf = 0;
    MBOOL           ret = MTRUE;
    MINT32          EqRet;
    E_EnqueRst      retRst = eEnqueRst_OK;
    NPIPE_PATH_E    cam_path = NPIPE_CAM_MAX;

    _OpenedPort = m_pNormalpipe->GetOpenedPortInfo(NormalPipe::_GetPort_OpendNum);

    // check frame header buffer first, prevent enque breaks for some port is out of buffer
    for(i = 0; i < _OpenedPort; i++) {
        _port = NPIPE_MAP_PORTID(m_pNormalpipe->GetOpenedPortInfo(NormalPipe::_GetPort_Index, i));

        m_pNormalpipe->mImgHdrLock.lock();
        AvailableBuf = m_pNormalpipe->mImgHdrCnt[_port];
        m_pNormalpipe->mImgHdrLock.unlock();

        if (AvailableBuf < m_pNormalpipe->mBurstQNum) {
            PIPE_ERR("N:%d dma:0x%x imgheader buf empty(available:%d, BurstQNum:%d)",
                m_pNormalpipe->mpSensorIdx,
                m_pNormalpipe->GetOpenedPortInfo(NormalPipe::_GetPort_Index, i),
                m_pNormalpipe->mImgHdrCnt[_port], m_pNormalpipe->mBurstQNum);
            retRst = eEnqueRst_Skip;
            goto _ENQ_DROP;
        }
    }

    cam_path = NPIPE_MAP_PATH(m_pNormalpipe->mpSensorIdx);

    if (NormalPipe::sIQData.IQInfo.FSM[cam_path] == NormalPipe::IQ_npipe_set) {
        if (NormalPipe::updateIQFSM(NormalPipe::IQ_camio_set, cam_path)) {
            configEnquePath(NSImageio::NSIspio::ICamIOPipe::CAMIO_IQ);
        }
    }
    else{
        configEnquePath(NSImageio::NSIspio::ICamIOPipe::CAMIO_D_Twin);
    }

    for(i = 0; i < _OpenedPort; i++) {
        _port = NPIPE_MAP_PORTID(m_pNormalpipe->GetOpenedPortInfo(NormalPipe::_GetPort_Index, i));

        m_pNormalpipe->mpEnqueRequest[_port]->sendCmd(QueueMgr<QBufInfo>::eCmd_front, (MUINTPTR)&_qBuf, 0);

        EqRet = enque(&_qBuf);
        if (EqRet != 0) {
            if(EqRet > 0) {
                PIPE_WRN("N:%d M:%d enque,already stopped", m_pNormalpipe->mpSensorIdx,
                            _qBuf.mvOut.at(0).FrameBased.mMagicNum_tuning);
            }
            else {
                retRst = eEnqueRst_NG;
                PIPE_ERR("N:%d M:%d enque failed", m_pNormalpipe->mpSensorIdx,
                            _qBuf.mvOut.at(0).FrameBased.mMagicNum_tuning);
            }
            goto _ENQ_DROP;
        }

        ret = dmaCfg(&_qBuf);
        if (MFALSE == ret) {
            PIPE_ERR("N:%d M:%d dmaCfg failed", m_pNormalpipe->mpSensorIdx,
                        _qBuf.mvOut.at(0).FrameBased.mMagicNum_tuning);
            retRst = eEnqueRst_NG;
            goto _ENQ_DROP;
        }

        //Need to pop buffer when tuningUpdate has no error.
        //m_pNormalpipe->mpEnqueRequest[_port]->sendCmd(QueueMgr<QBufInfo>::eCmd_pop, 0, 0);

        if (0 == listMagic.size()) {
            MUINT32 m;
            for (m = 0; m < _qBuf.mvOut.size(); m++) {
                listMagic.push_back(_qBuf.mvOut.at(m).FrameBased.mMagicNum_tuning);
            }
        }
    }

    m_pNormalpipe->mpCamIOPipe->sendCommand(EPIPECmd_SET_FRM_TIME, (MINTPTR)(_qBuf.mShutterTimeNs/1000000), 0, 0);
    updateFrameTime((MUINT32)(_qBuf.mShutterTimeNs/1000000), 0);

    if (MFALSE == tuningUpdate(&listMagic)) {
        PIPE_ERR("N:%d p1_update failed", m_pNormalpipe->mpSensorIdx);
        retRst = eEnqueRst_NG;
        goto _ENQ_DROP;
    }

    for(i = 0; i < _OpenedPort; i++) {
        _port = NPIPE_MAP_PORTID(m_pNormalpipe->GetOpenedPortInfo(NormalPipe::_GetPort_Index, i));

        m_pNormalpipe->mpEnqueRequest[_port]->sendCmd(QueueMgr<QBufInfo>::eCmd_pop, 0, 0);
    }

    m_pNormalpipe->sendCommand(ENPipeCmd_GET_CUR_SOF_IDX, (MINTPTR)&sofIdx, 0, 0);

    m_LastEnqSof = sofIdx;
    m_ConsumedReqCnt++;

    ::sem_post(&m_semDeqLoopToken); //let dequeThread get 1 loop, after all opened ports had enqued

_ENQ_DROP:

    return retRst;
}

E_EnqueRst NormalPipe_Thread::configEnquePath(NSImageio::NSIspio::ICamIOPipe::E_FRM_EVENT frameEvent)
{
    E_CamHwPathCfg  newCfg;
    CAMIO_Func      func;
    MUINT32         qsize = 0, _path = 0;
    E_CamHwPathCfg  TG_Num = eCamHwPathCfg_Num;
    MBOOL           Num_update = MFALSE;
    MUINT32         cam_path = 0;

    do { /* for critical section of class static member */
        Mutex::Autolock lock(NormalPipe::NPipeCfgGLock);

        if (CAM_TG_1 == _NORMALPIPE_GET_TG_IDX(this->m_pNormalpipe->mpSensorIdx)) {
            _path = NPIPE_CAM_A;
        }
        else {
            _path = NPIPE_CAM_B;
        }

        if (_path == NormalPipe::m_occupyingCtlPath) {
            NormalPipe::m_pathCfgQueue.sendCmd(QueueMgr<E_CamHwPathCfg>::eCmd_getsize, (MUINTPTR)&qsize, 0);

            /* Check previous issued status */
            if (NormalPipe::m_pathCfg_fsm == eCamHwPathFsm_Issuing) {
                if (qsize == 0) {
                    PIPE_WRN("N:%d no pathConfig input received", this->m_pNormalpipe->mpSensorIdx);
                }
                else {
                    NormalPipe::m_pathCfgQueue.sendCmd(QueueMgr<E_CamHwPathCfg>::eCmd_front, (MUINTPTR)&TG_Num, 0);
                    NormalPipe::m_pathCfgQueue.sendCmd(QueueMgr<E_CamHwPathCfg>::eCmd_pop, 0, 0);

                    qsize -= 1;

                    Num_update = MTRUE;//update TGnum

                    PIPE_INF("N:%d update to new path:%d depth:%d",
                        this->m_pNormalpipe->mpSensorIdx, TG_Num, qsize);
                }

                NormalPipe::m_pathCfg_fsm = eCamHwPathFsm_Applied;
            }

            /* Process new input , and can't update new cfg if frameevent = drop*/
            if ((qsize > 0) && (frameEvent != NSImageio::NSIspio::ICamIOPipe::CAMIO_DROP)){
                if (qsize > 2) {
                    PIPE_WRN("N:%d too many request:%d, drop to 2", this->m_pNormalpipe->mpSensorIdx, qsize);

                    for (; qsize > 2; qsize--) {
                        NormalPipe::m_pathCfgQueue.sendCmd(QueueMgr<E_CamHwPathCfg>::eCmd_pop, 0, 0);
                    }
                }

                NormalPipe::m_pathCfgQueue.sendCmd(QueueMgr<E_CamHwPathCfg>::eCmd_front, (MUINTPTR)&newCfg, 0);

                NormalPipe::m_pathCfg_fsm = eCamHwPathFsm_Issuing;

                PIPE_INF("N:%d issuing new path:%d depth:%d", this->m_pNormalpipe->mpSensorIdx, newCfg, qsize);
            }
            else if(qsize == 0){
                if(TG_Num != eCamHwPathCfg_Num)
                    newCfg = TG_Num;
                else
                    newCfg = NormalPipe::m_pathCfg_TGnum;

                NormalPipe::m_occupyingCtlPath = NPIPE_CAM_NONE;
            }
            else{//do nothing when drop frame
            }
        }
        else {//case for no new set_hw_path_cfg, use previous setting.
            newCfg = NormalPipe::m_pathCfg_TGnum;
        }

        func.Raw = 0;
        if (NSImageio::NSIspio::ICamIOPipe::CAMIO_D_Twin == frameEvent) {
            switch (newCfg) {
            case eCamHwPathCfg_One_TG:
                func.Bits.SensorNum = NSImageio::NSIspio::ECamio_1_SEN;
                break;
            case eCamHwPathCfg_Two_TG:
                func.Bits.SensorNum = NSImageio::NSIspio::ECamio_2_SEN;
                break;
            default:
                PIPE_ERR("N:%d unknown pathCfg:%d", this->m_pNormalpipe->mpSensorIdx, newCfg);
                return eEnqueRst_NG;
            }
        }
        else if (NSImageio::NSIspio::ICamIOPipe::CAMIO_IQ == frameEvent) {
            cam_path = NPIPE_MAP_PATH(this->m_pNormalpipe->mpSensorIdx);
            switch (NormalPipe::sIQData.IQInfo.lv[cam_path]) {
                case eCamIQ_L:
                case eCamIQ_H:
                    func.Bits.IQlv = NormalPipe::sIQData.IQInfo.lv[cam_path];

                    if(NormalPipe::sIQData.IQInfo.FSM[cam_path] != NormalPipe::IQ_unknown)
                        this->m_pNormalpipe->mpCamIOPipe->sendCommand(EPIPECmd_SET_IQ_CBFP,
                            (MINTPTR)NormalPipe::updateIQCB, 0, 0);
                    break;
                default:
                    PIPE_ERR("N:%d unkonw level(%d)", this->m_pNormalpipe->mpSensorIdx, (MUINT32)NormalPipe::sIQData.IQInfo.lv[cam_path]);
                    break;
            }
        }
    } while(0);

    PIPE_DBG("N:%d configFrame event:%d multiSenNum:%d occupy:%x status:%d(%d) fsm:%d depth:%d",
        this->m_pNormalpipe->mpSensorIdx, frameEvent, func.Bits.SensorNum, NormalPipe::m_occupyingCtlPath,
        NormalPipe::m_pathCfg_TGnum,TG_Num, NormalPipe::m_pathCfg_fsm, qsize);

    this->m_pNormalpipe->mpCamIOPipe->configFrame(frameEvent, func);

    //put here is because of not only HW is ready, but also need to make sure all status in imageio are ready.
    //and "m_pathCfg_TGnum" is also used to inform MW that pathcfg is ready or not.
    if(Num_update == MTRUE){
        NormalPipe::m_pathCfg_TGnum = TG_Num;
        Num_update = MFALSE;
    }
    return eEnqueRst_OK;
}


MVOID NormalPipe_Thread::enqueRequest(MUINT32 sof_idx)
{
    IOPipeTrace _traceObj("NormalPipeThread::enqueRequest", IOPIPE_TRACE_EN_NOMALPIPE);
    QBufInfo    enqQBuf;
    E_EnqueRst  enqRst = eEnqueRst_OK;
    MUINT32     _ReserveCurReq = 0, i = 0, j = 0;
    MUINT32     _OpenedPort = m_pNormalpipe->GetOpenedPortInfo(NormalPipe::_GetPort_OpendNum);
    MUINT32     _size = 0, reqQueNum[ePort_max], frm_status = 0, phyCqCnt = 0, virCqCnt = 0;

    m_pNormalpipe->mpCamIOPipe->sendCommand(EPIPECmd_GET_CUR_FRM_STATUS,(MINTPTR)&frm_status, 0, 0);
    /* check previous enque consumed or not */
    m_pNormalpipe->mpCamIOPipe->sendCommand(EPIPECmd_GET_CQUPDATECNT,(MINTPTR)MTRUE, (MINTPTR)&phyCqCnt, 0);
    m_pNormalpipe->mpCamIOPipe->sendCommand(EPIPECmd_GET_CQUPDATECNT,(MINTPTR)MFALSE, (MINTPTR)&virCqCnt, 0);

    PIPE_DBG("+N:%d CurSof:%d enqCnt:%d cqCnt(p/v):%d/%d FrameStatus(%d)",
        m_pNormalpipe->mpSensorIdx, sof_idx, m_ConsumedReqCnt, phyCqCnt, virCqCnt, frm_status);

    if (phyCqCnt != virCqCnt) {
        PIPE_WRN("N:%d cqCnt phy:%d vir:%d reserve request, previous not consumed.", m_pNormalpipe->mpSensorIdx, phyCqCnt, virCqCnt);

        MUINT32 _port = NPIPE_MAP_PORTID(m_pNormalpipe->GetOpenedPortInfo(NormalPipe::_GetPort_Index, 0));
        MUINT32 reqQueNum = 0;

        m_pNormalpipe->mpEnqueRequest[_port]->sendCmd(QueueMgr<QBufInfo>::eCmd_getsize, (MUINTPTR)&reqQueNum, 0);
        if (reqQueNum > 0) {
            m_DropEnQ.resize(0);

            for (i = 0; i < _OpenedPort; i++) {
                _port = NPIPE_MAP_PORTID(m_pNormalpipe->GetOpenedPortInfo(NormalPipe::_GetPort_Index, i));

                m_pNormalpipe->mpEnqueRequest[_port]->sendCmd(QueueMgr<QBufInfo>::eCmd_front, (MUINTPTR)&enqQBuf, 0);
                m_pNormalpipe->mpEnqueRequest[_port]->sendCmd(QueueMgr<QBufInfo>::eCmd_pop, 0, 0);

                for (j = 0; j < enqQBuf.mvOut.size(); j++) {
                    PIPE_DBG("N:%d drop dma:x%x PA:(0x%" PRIXPTR ") M:%d isUFO(%d)",
                        m_pNormalpipe->mpSensorIdx, enqQBuf.mvOut.at(j).mPortID.index, enqQBuf.mvOut.at(j).mBuffer->getBufPA(ePlane_1st),
                        enqQBuf.mvOut.at(j).FrameBased.mMagicNum_tuning, CHECK_FMT_UF(enqQBuf.mvOut.at(j).mBuffer->getImgFormat()));
                    if (i == 0) { // all ports should have the same magic_num
                        m_DropEnQ.push_back(enqQBuf.mvOut.at(j).FrameBased.mMagicNum_tuning);
                    }
                }
            }

            for (i = 0; i < m_DropEnQ.size(); i++) {
                if (m_pNormalpipe->m_DropCB) {
                    m_pNormalpipe->m_DropCB(m_DropEnQ.at(i), m_pNormalpipe->m_returnCookie);
                }
            }
            PIPE_INF("N:%d drop request size:%d mag_num[%d~%d] because last time enque is over p1 done.", m_pNormalpipe->mpSensorIdx,\
                (MUINT32)m_DropEnQ.size(), m_DropEnQ.at(0), m_DropEnQ.at(m_DropEnQ.size()-1));
        }

        return;
    }

    if (m_ConsumedReqCnt < m_pNormalpipe->mReqDepthPreStart) {
        /* '-1', because of the first tuning is consumed immediate_request at enque() */
        MUINT32 _availReq = 1, _expectConsumedReq;

        PIPE_DBG("N:%d static loop(%d_%d)", m_pNormalpipe->mpSensorIdx, m_pNormalpipe->mReqDepthPreStart,\
            m_ConsumedReqCnt);

        _expectConsumedReq = min((sof_idx+1), m_pNormalpipe->mReqDepthPreStart);
        if (_expectConsumedReq > (m_ConsumedReqCnt+1)) { /* +1 for this loop */
            MUINT32 _dropCnt = (_expectConsumedReq - m_ConsumedReqCnt - 1);
            _availReq += _dropCnt;
            m_ConsumedReqCnt += _dropCnt;

            PIPE_DBG("N:%d drop(%d) static loop", m_pNormalpipe->mpSensorIdx, _dropCnt);
        }

        /* drop expired request */
        for (; _availReq > 1; _availReq--) {
            m_DropEnQ.resize(0);
            for (i = 0; i < _OpenedPort; i++) {
                MUINT32 _port = NPIPE_MAP_PORTID(m_pNormalpipe->GetOpenedPortInfo(NormalPipe::_GetPort_Index, i));

                m_pNormalpipe->mpEnqueRequest[_port]->sendCmd(QueueMgr<QBufInfo>::eCmd_front, (MUINTPTR)&enqQBuf, 0);
                m_pNormalpipe->mpEnqueRequest[_port]->sendCmd(QueueMgr<QBufInfo>::eCmd_pop, 0, 0);

                for (j = 0; j < enqQBuf.mvOut.size(); j++) {
                    PIPE_DBG("N:%d drop dma:x%x PA:0x%" PRIXPTR " M:%d", m_pNormalpipe->mpSensorIdx,\
                        enqQBuf.mvOut.at(j).mPortID.index, enqQBuf.mvOut.at(j).mBuffer->getBufPA(0),\
                        enqQBuf.mvOut.at(j).FrameBased.mMagicNum_tuning);
                    if (i == 0) { // all ports should have the same magic_num
                        m_DropEnQ.push_back(enqQBuf.mvOut.at(j).FrameBased.mMagicNum_tuning);
                    }
                }
            }
            for (i = 0; i < m_DropEnQ.size(); i++) {
                if (m_pNormalpipe->m_DropCB) {
                    m_pNormalpipe->m_DropCB(m_DropEnQ.at(i), m_pNormalpipe->m_returnCookie);
                }
            }
            PIPE_INF("N:%d drop request# %d [%d~%d]", m_pNormalpipe->mpSensorIdx, (MUINT32)m_DropEnQ.size(),\
                m_DropEnQ.at(0), m_DropEnQ.at(m_DropEnQ.size()-1));
        }

        if (CAM_FST_DROP_FRAME == frm_status) {
            MUINT32 _cur_sof = sof_idx, _last_enq_sof = m_LastEnqSof;

            if ((_cur_sof+SOF_IDX_MSB) < _last_enq_sof) {
                _cur_sof += ((_cur_sof&SOF_IDX_MSB)? 0: (SOF_IDX_MSB<<1));
                _last_enq_sof += ((_last_enq_sof&SOF_IDX_MSB)? 0: (SOF_IDX_MSB<<1));
                PIPE_INF("N:%d wrapping sof: %d_%d", m_pNormalpipe->mpSensorIdx, _last_enq_sof, _cur_sof);
            }

            if (_cur_sof <= (_last_enq_sof + 1)) {
                _availReq = 0;
            }
        }

        if (_availReq) {
            enqRst = frameToEnque();
        }
        else {
            enqRst = eEnqueRst_Skip;
            PIPE_INF("N:%d reserve static request @%d", m_pNormalpipe->mpSensorIdx, m_ConsumedReqCnt);
        }

        //if 1st enq loop does NOT apply patch, MW cannot enque at this timing
        if (m_ConsumedReqCnt >= m_pNormalpipe->mReqDepthPreStart) {
            PIPE_DBG("N:%d static loop done(%d)", m_pNormalpipe->mpSensorIdx, m_ConsumedReqCnt);
            m_pNormalpipe->m_b1stEnqLoopDone = MTRUE;
        }
        PIPE_DBG("- N:%d static loop enqRst:%d", m_pNormalpipe->mpSensorIdx, enqRst);

        goto _EXIT_ENQREQ;
    }

    for (i = 0; i < _OpenedPort; i++) {
        MUINT32 _port = NPIPE_MAP_PORTID(m_pNormalpipe->GetOpenedPortInfo(NormalPipe::_GetPort_Index, i));

        m_pNormalpipe->mpEnqueRequest[_port]->sendCmd(QueueMgr<QBufInfo>::eCmd_getsize, (MUINTPTR)&(reqQueNum[_port]), 0);
        if (0 == reqQueNum[_port]) {
            PIPE_INF("N:%d dma:x%x no request sof_idx:%d, FrameStatus(%d)", m_pNormalpipe->mpSensorIdx,
                m_pNormalpipe->GetOpenedPortInfo(NormalPipe::_GetPort_Index, i), sof_idx, frm_status);

            enqRst = eEnqueRst_Skip;
            goto _EXIT_ENQREQ;
        }
    }

    //available request from MW
    //full mode means 1 request per frame, if requests r over 2, drop request
    m_DropEnQ.resize(0);
    for (i = 0; i < _OpenedPort; i++) {
        MUINT32 _dmaIdx = m_pNormalpipe->GetOpenedPortInfo(NormalPipe::_GetPort_Index,i);
        MUINT32 _port = NPIPE_MAP_PORTID(_dmaIdx);

        if (reqQueNum[_port] > 1) {
            MUINT32 _decentReqCnt = 2;

            PIPE_INF("WARNING: N:%d dma:x%x over 1 request: %d", m_pNormalpipe->mpSensorIdx, _dmaIdx, reqQueNum[_port]);

            //if reserved container r empty, drop requestes which r over 2, otherwise, leave only 1 request.
            while (reqQueNum[_port] > _decentReqCnt) {
                m_pNormalpipe->mpEnqueRequest[_port]->sendCmd(QueueMgr<QBufInfo>::eCmd_front, (MUINTPTR)&enqQBuf, 0);
                m_pNormalpipe->mpEnqueRequest[_port]->sendCmd(QueueMgr<QBufInfo>::eCmd_pop, 0, 0);
                reqQueNum[_port]--;

                for (j = 0; j < enqQBuf.mvOut.size(); j++) {
                    if (i == 0) { // all ports should have the same magic_num
                        PIPE_INF("WARNING: N:%d dma:x%x drop req M:x%x", m_pNormalpipe->mpSensorIdx, _dmaIdx,
                                    enqQBuf.mvOut.at(j).FrameBased.mMagicNum_tuning);
                        m_DropEnQ.push_back(enqQBuf.mvOut.at(j).FrameBased.mMagicNum_tuning);
                    }
                }
            }
        }
    }

    //CB for notify MW which enque be dropped.
    for (i = 0; i < m_DropEnQ.size(); i++) {
        PIPE_INF("N:%d drop req CB: M:x%x(%d)", m_pNormalpipe->mpSensorIdx, m_DropEnQ.at(i), (MUINT32)m_DropEnQ.size());
        if (m_pNormalpipe->m_DropCB != NULL) {
            m_pNormalpipe->m_DropCB(m_DropEnQ.at(i), m_pNormalpipe->m_returnCookie);
        }
    }
    /*
     * enqueRequest of each port should be 1
     */

    //for HDRViewFinder: if current SOF = enqueRequest SOF, reserve this request
    /* if (0 == m_DropEnQ.size()) */
    {
        MUINT32 _cur_sof_idx, _buf_sof_idx, _last_enq_sof_idx; /* for sof 255 wrapping */

        m_pNormalpipe->mpEnqueRequest[NPIPE_MAP_PORTID(m_pNormalpipe->GetOpenedPortInfo(NormalPipe::_GetPort_Index,0))]
                                      ->sendCmd(QueueMgr<QBufInfo>::eCmd_front, (MUINTPTR)&enqQBuf, 0);

        _cur_sof_idx        = sof_idx;
        _buf_sof_idx        = enqQBuf.mvOut.at(0).FrameBased.mSOFidx;
        _last_enq_sof_idx   = m_LastEnqSof;

        /* sof = 0~255, _cur_sof_idx-_buf_sof must == 1 */
        if (((_cur_sof_idx+SOF_IDX_MSB) < _buf_sof_idx) || ((_cur_sof_idx+SOF_IDX_MSB) < _last_enq_sof_idx) ||
            ((_buf_sof_idx+SOF_IDX_MSB) < _cur_sof_idx)) {
            /* need wraping, some var is at the other half */
            _cur_sof_idx += ((_cur_sof_idx&SOF_IDX_MSB)? 0: (SOF_IDX_MSB<<1));
            _buf_sof_idx += ((_buf_sof_idx&SOF_IDX_MSB)? 0: (SOF_IDX_MSB<<1));
            _last_enq_sof_idx += ((_last_enq_sof_idx&SOF_IDX_MSB)? 0: (SOF_IDX_MSB<<1));
            PIPE_INF("N:%d wrapping sof: %d_%d_%d", m_pNormalpipe->mpSensorIdx, _last_enq_sof_idx,
                        _buf_sof_idx, _cur_sof_idx);
        }

        if (0 == m_DropEnQ.size()) {
            if (_cur_sof_idx <= _buf_sof_idx) {
                PIPE_INF("WARNING: N:%d SOF cur_sof(%d)<=buf_sof(%d), postpone req", m_pNormalpipe->mpSensorIdx,
                            _cur_sof_idx, _buf_sof_idx);
                _ReserveCurReq = 1;
            }
        }

        #if 0
        if (_cur_sof_idx - _buf_sof_idx > 1) {
            PIPE_INF("WARNING: N:%d cur_sof(%d)-buf_sof(%d)>1", m_pNormalpipe->mpSensorIdx,
                        sof_idx, _buf_sof_idx);
        }
        #endif
    }

    if (0 == _ReserveCurReq) {
        enqRst = frameToEnque();
    }
    else {
        enqRst = eEnqueRst_Skip;
        PIPE_INF("WARNING: N:%d reserve request", m_pNormalpipe->mpSensorIdx);
    }

_EXIT_ENQREQ:
    if (enqRst == eEnqueRst_Skip) {
        configEnquePath(NSImageio::NSIspio::ICamIOPipe::CAMIO_DROP);
    }
    PIPE_DBG("-");
}


/******************************************************************************
 * NormalPipe::
 * 1: already stopped
 * 0: sucessed
 *-1: fail
 ******************************************************************************/
MINT32 NormalPipe_Thread::dequeResponse(void)
{
#define NPIPE_DEQUE_TIMEOUT     (5566)
#define NPIPE_MAX_DEQ_COUNT     (3)
    PIPE_DBG("+");

    MUINT32 _size = 0, _OpenedPort = 0, idx = 0, i, PlaneMax = 0, _port = 0, imgHdrBufNum = 0;
    E_BUF_STATUS deqRet = eBuf_Pass;
    vector<NSImageio::NSIspio::PortID>      QportID; //used for CamIOPipe
    NSImageio::NSIspio::QTimeStampBufInfo   rQTSBufInfo;
    NSImageio::NSIspio::PortID              portID;    //used for CamIOPipe
    IImageBuffer                            *pframe = NULL;
    NormalPipe::IMEM_MUTI_BUF_INFO          imBuf;

    CAM_STATE_NOTIFY                        notifyObj = {.fpNotifyState = camStateNotify, .Obj = this};
    vector<MUINT32>                         vMnum;

    vMnum.clear();
    _OpenedPort = m_pNormalpipe->GetOpenedPortInfo(NormalPipe::_GetPort_OpendNum);
    imgHdrBufNum = IMG_HEADER_BUF_NUM * m_pNormalpipe->mBurstQNum;

    for (i = 0; i < _OpenedPort; i++) {
        QBufInfo    deqQBuf;
        MUINT32     deqTryCount;

        portID.index = m_pNormalpipe->GetOpenedPortInfo(NormalPipe::_GetPort_Index, i);
        _port = NPIPE_MAP_PORTID(portID.index);

        m_pNormalpipe->mResetCount = 0;
        for (deqTryCount = NPIPE_MAX_DEQ_COUNT; deqTryCount > 0; deqTryCount--) {
            if((deqRet = m_pNormalpipe->mpCamIOPipe->dequeOutBuf(portID, rQTSBufInfo, NPIPE_DEQUE_TIMEOUT, &notifyObj)) \
                != eBuf_Fail){
                break;
            }

            if (m_pNormalpipe->mResetCount > 0) {
                MUINT32 retry = m_pNormalpipe->mResetCount; // only query mResetCount once

                for (; retry > 0; retry--) {
                    PIPE_INF("N:%d dma:x%x reset retry flow %d times",
                        m_pNormalpipe->mpSensorIdx, portID.index, retry);

                    m_pNormalpipe->reset();

                    if ((deqRet = m_pNormalpipe->mpCamIOPipe->dequeOutBuf(portID, rQTSBufInfo, NPIPE_DEQUE_TIMEOUT, &notifyObj)) != eBuf_Fail) {
                        m_pNormalpipe->mResetCount = 0;
                        break;
                    }
                }
                if (0 == retry) {
                    PIPE_ERR("N:%d dma:x%x deque(%d) fail after reset sensor",
                        m_pNormalpipe->mpSensorIdx, portID.index, NPIPE_MAX_DEQ_COUNT);
                    return -1;
                }

                break;
            }
        }

        switch(deqRet){
            case eBuf_suspending:
                if (vMnum.size() == 0) {//only 1 port need to drop request
                    //need to pop all requests in drv
                    for (idx = 0; idx < rQTSBufInfo.vBufInfo.size(); idx++) {
                        if (m_pNormalpipe->m_DropCB) {
                            vMnum.push_back(rQTSBufInfo.vBufInfo.at(idx).m_num);
                            PIPE_INF("port_%d: drop magic number:%d(%d)\n",portID.index,idx,rQTSBufInfo.vBufInfo.at(idx).m_num);
                        }
                        else{
                            PIPE_ERR("dropCB is NULL\n");
                        }
                    }
                }
                break;
            case eBuf_stopped:
                PIPE_DBG("N:%d already stopped, dma:x%x", m_pNormalpipe->mpSensorIdx, portID.index);
                return 1;
                break;
            default:
                break;
        }

        if(vMnum.size() == 0) {//if size != 0 => suspending
            if (0 == deqTryCount) {
                PIPE_ERR("N:%d dma:x%x deque(%d) fail", m_pNormalpipe->mpSensorIdx, portID.index, NPIPE_MAX_DEQ_COUNT);
                return -1;
            }

            if (rQTSBufInfo.vBufInfo.size() != m_pNormalpipe->mBurstQNum) {
                PIPE_ERR("N:%d dma:x%x deque buf num error, %d/%d", m_pNormalpipe->mpSensorIdx,\
                        portID.index, (MUINT32)rQTSBufInfo.vBufInfo.size(), m_pNormalpipe->mBurstQNum);
                return -1;
            }

            /* if there is at least one buf dequed
                     *  => return done buf with empty buf to caller
                     * if dequed buf num is not mBurstQNum,
                     *  => return error
                     */
            for (idx = 0; idx < rQTSBufInfo.vBufInfo.size(); idx++) {
                BufInfo         buff;
                MBOOL           isUF = CHECK_FMT_UF((MINT)rQTSBufInfo.vBufInfo.at(idx).img_fmt);
                UFDG_META_INFO *pUFDG = (UFDG_META_INFO*)rQTSBufInfo.vBufInfo.at(idx).m_pPrivate;

                if (pUFDG) {
                    pUFDG->bUF = (MUINT32)rQTSBufInfo.vBufInfo.at(idx).img_fmt;//put fmt for MW parsing
                    PIPE_INF("UFDG_META_INFO:dma(0x%x),isUF(%d),fmt(0x%x),OFST_ADDR(0x%x),AU_START(0x%x),AU2_SIZE(0x%x),BOND_MODE(0x%x)",
                         portID.index,
                         isUF,
                         pUFDG->bUF,
                         pUFDG->UFDG_BITSTREAM_OFST_ADDR,
                         pUFDG->UFDG_BS_AU_START,
                         pUFDG->UFDG_AU2_SIZE,
                         pUFDG->UFDG_BOND_MODE);
                }

                #ifdef USE_IMAGEBUF_HEAP
                //chage from va 2 pa is because of in camera3, drv may have no va infor.
                pframe = m_pNormalpipe->mpFrameMgr->deque((MINTPTR)rQTSBufInfo.vBufInfo.at(idx).u4BufPA[ePlane_1st]);
                if (NULL == pframe) {
                    PIPE_ERR("N:%d dma:x%x frameMgr null idx_%d,PA_0x%" PRIXPTR "", \
                        m_pNormalpipe->mpSensorIdx, portID.index, idx, rQTSBufInfo.vBufInfo.at(idx).u4BufPA[ePlane_1st]);
                    return -1;
                }
                else {
                    pframe->setTimestamp(rQTSBufInfo.vBufInfo.at(idx).getTimeStamp_ns());
                }
                buff.mBuffer = pframe;
                #endif

                if(NPIPE_PASS_DEQ_BUFINFO(portID, rQTSBufInfo.vBufInfo.at(idx), buff) == MFALSE){
                    return -1;
                }

                //
                deqQBuf.mvOut.push_back(buff);

                PIPE_DBG("DEQ:%d: dma:x%x pa(0x%" PRIXPTR ") io(%d_%d_%d_%d_%d_%d_%d_%d_%d_%d) mHighlightData(%d) mag(0x%x) IQlv(%d)",\
                    m_pNormalpipe->mpSensorIdx, portID.index, buff.mPa,\
                    buff.mMetaData.mCrop_s.p.x,buff.mMetaData.mCrop_s.p.y,buff.mMetaData.mCrop_s.s.w,buff.mMetaData.mCrop_s.s.h, \
                    buff.mMetaData.mCrop_d.p.x,buff.mMetaData.mCrop_d.p.y,buff.mMetaData.mCrop_d.s.w,buff.mMetaData.mCrop_d.s.h, \
                    buff.mMetaData.mDstSize.w,buff.mMetaData.mDstSize.h, buff.mMetaData.mHighlightData, \
                    buff.mMetaData.mMagicNum_tuning, (MUINT32)buff.mMetaData.eIQlv);

                /* Get back image header buffer */
                PlaneMax = (isUF == MTRUE) ? (MUINT32)ePlane_3rd : (MUINT32)ePlane_1st;

                for (MUINT32 PlaneID = 0; PlaneID <= PlaneMax; PlaneID++) {
                    if ((rQTSBufInfo.vBufInfo.at(idx).Frame_Header.u4BufVA[PlaneID] == 0) ||
                        (rQTSBufInfo.vBufInfo.at(idx).Frame_Header.u4BufPA[PlaneID] == 0)) {
                        PIPE_ERR("[%d]Frame_Header addr is null, PA[%d]:0x%" PRIXPTR " VA[%d]:0x%" PRIXPTR "",
                            idx, PlaneID, rQTSBufInfo.vBufInfo.at(idx).Frame_Header.u4BufPA[PlaneID],
                            PlaneID, rQTSBufInfo.vBufInfo.at(idx).Frame_Header.u4BufVA[PlaneID]);
                        return -1;
                    }
                }
                m_pNormalpipe->mImgHdrLock.lock();
                m_pNormalpipe->mImgHdrCnt[_port]++;
                m_pNormalpipe->mImgHdrLock.unlock();
            }

            m_pNormalpipe->mDeqContainerLock.lock();
            m_pNormalpipe->mpDeQueue[_port]->sendCmd(QueueMgr<QBufInfo>::eCmd_push, (MUINTPTR)&deqQBuf, 0);
            m_pNormalpipe->mDeqContainerLock.unlock();
        }
        else {
            for (idx = 0; idx < rQTSBufInfo.vBufInfo.size(); idx++) {
                m_pNormalpipe->mImgHdrLock.lock();
                m_pNormalpipe->mImgHdrCnt[_port]++;
                m_pNormalpipe->mImgHdrLock.unlock();
            }
        }
    } //for (i = 0; i < _OpenedPort; i++)


    //drop request when suspending, size of vMnum will not be 0 only when suspending.
    if(vMnum.size() != 0){
        //need to pop all requests in drv
        for(idx=0;idx<vMnum.size();idx++){
            m_pNormalpipe->m_DropCB(vMnum.at(idx), m_pNormalpipe->m_returnCookie);
        }
    }
    else{
        /* All ports dequed, and pushed into deque container */
        for (i = 0; i < _OpenedPort; i++) {
            portID.index = m_pNormalpipe->GetOpenedPortInfo(NormalPipe::_GetPort_Index, i);
            ::sem_post(&m_semDeqDonePort[NPIPE_MAP_PORTID(portID.index)]);
        }
    }

    PIPE_DBG("-");

    return 0;
}

MINT32 NormalPipe_Thread::idleStateHold(MUINT32 hold, E_ThreadGrop thdGrpBm)
{
    if (hold) {
        PIPE_INF("Holding idle locks grp: 0x%x", thdGrpBm);

        if ((thdGrpBm & eThdGrp_Enq) == eThdGrp_Enq) {
            mThreadIdleLock_Enq.lock();
        }
        if ((thdGrpBm & eThdGrp_Deq) == eThdGrp_Deq) {
            mThreadIdleLock_Deq.lock();
        }

        if ((thdGrpBm & eThdGrp_External) == eThdGrp_External) {
            if (idleHoldNotifyCB.fpNotifyState) {
                if(idleHoldNotifyCB.fpNotifyState(CAM_HOLD_IDLE, idleHoldNotifyCB.Obj)

                    == NOTIFY_FAIL){
                    PIPE_ERR("Notify fail, state:%d", (MUINT32)CAM_HOLD_IDLE);
                }
            }
        }
    } else {
        if ((thdGrpBm & eThdGrp_External) == eThdGrp_External) {
            if (idleHoldNotifyCB.fpNotifyState) {
                if(idleHoldNotifyCB.fpNotifyState(CAM_RELEASE_IDLE, idleHoldNotifyCB.Obj)

                    == NOTIFY_FAIL){
                    PIPE_ERR("Notify fail, state:%d", (MUINT32)CAM_RELEASE_IDLE);
                }
            }
        }

        if ((thdGrpBm & eThdGrp_Enq) == eThdGrp_Enq) {
            mThreadIdleLock_Enq.unlock();
        }
        if ((thdGrpBm & eThdGrp_Deq) == eThdGrp_Deq) {
            mThreadIdleLock_Deq.unlock();
        }

        PIPE_INF("Released idle locks grp: 0x%x", thdGrpBm);
    }

    return 0;
}

MUINT32 NormalPipe_Thread::camStateNotify(CAM_STATE_OP state, MVOID *Obj)
{
    NormalPipe_Thread *pNpipeThd = (NormalPipe_Thread *)Obj;

    switch (state) {
    case CAM_ENTER_WAIT:
        pNpipeThd->mThreadIdleLock_Deq.unlock();
        break;
    case CAM_EXIT_WAIT:
        pNpipeThd->mThreadIdleLock_Deq.lock();
        break;
    default:
        PIPE_ERR("Unknown state: %d", state);
        break;
    }

    return 0;
}

MVOID* NormalPipe_Thread::ispEnqueThread(void *arg)
{
    NormalPipe_Thread *_this = reinterpret_cast<NormalPipe_Thread*>(arg);
    MUINT32 _sof_idx, _sof_idx2, ret;

    int const policy    = SCHED_OTHER;
    int const priority  = NICE_CAMERA_LOMO - 5; //Need to adjust priority to -13;
    struct sched_param sched_p;

    ::sched_getparam(0, &sched_p);
    if (policy == SCHED_OTHER) {
        setThreadPriority(policy, priority);
        #if 0
        int _policy    = SCHED_OTHER;
        int _priority  = NICE_CAMERA_LOMO;
        getThreadPriority(_policy, _priority);

        PIPE_DBG(
            "policy:(expect, result)=(%d, %d), priority:(expect, result)=(%d, %d)"
            , policy, _policy, priority, _priority
            );
        #endif
    }
    else {   //  Note: "priority" is real-time priority.
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
    switch (_NORMALPIPE_GET_TG_IDX(_this->m_pNormalpipe->mpSensorIdx)) {
    case CAM_TG_1:  ::prctl(PR_SET_NAME,"IspEnqThd_TG1",0,0,0);
        break;
    case CAM_TG_2:  ::prctl(PR_SET_NAME,"IspEnqThd_TG2",0,0,0);
        break;
    case CAM_SV_1:  ::prctl(PR_SET_NAME,"IspEnqThd_SV1",0,0,0);
        break;
    case CAM_SV_2:  ::prctl(PR_SET_NAME,"IspEnqThd_SV2",0,0,0);
        break;
    default:
        PIPE_ERR("ERROR: TG:0x%x(0x%x)", _NORMALPIPE_GET_TG_IDX(_this->m_pNormalpipe->mpSensorIdx),\
                _this->m_pNormalpipe->mpSensorIdx);
        goto _exit;
    }

    _this->m_irqUserKey = _this->m_pNormalpipe->attach(THIS_NAME);
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
        ret = _this->m_pNormalpipe->wait(EPipeSignal_SOF, EPipeSignal_ClearWait, _this->m_irqUserKey, _this->estimateFrameTime(), NULL);
        _this->mThreadIdleLock_Enq.lock();
        if (MFALSE == ret) {
            PIPE_ERR("ERROR: wait SOF fail");
            continue;
        }

        if (_this->m_bStart == MFALSE) {
            PIPE_DBG("enqThread leave after wait SOF\n");
            break;
        }

        _this->m_pNormalpipe->sendCommand(ENPipeCmd_GET_CUR_SOF_IDX,(MINTPTR)&_sof_idx,0,0);
        PIPE_DBG("N:%d: cur SOF: %d", _this->m_pNormalpipe->mpSensorIdx, _sof_idx);

        if (_this->enqueThrdNotifyCB.fpNotifyState) {
            if (_this->enqueThrdNotifyCB.fpNotifyState(CAM_ENTER_ENQ, _this->enqueThrdNotifyCB.Obj) == NOTIFY_FAIL) {
                PIPE_ERR("Notify fail, state:%d", (MUINT32)CAM_ENTER_ENQ);
            }
        }

        _this->m_pNormalpipe->mEnqContainerLock.lock();
        _this->enqueRequest(_sof_idx);
        _this->m_pNormalpipe->mEnqContainerLock.unlock();

        if (_this->enqueThrdNotifyCB.fpNotifyState) {
            if (_this->enqueThrdNotifyCB.fpNotifyState(CAM_EXIT_ENQ, _this->enqueThrdNotifyCB.Obj) == NOTIFY_FAIL) {
                PIPE_ERR("Notify fail, state:%d", (MUINT32)CAM_EXIT_ENQ);
            }
        }

        _this->m_pNormalpipe->mEnqSofCBLock.lock();
        if (_this->m_pNormalpipe->m_SofCB) {
            _this->m_pNormalpipe->m_SofCB(&(_this->m_pNormalpipe->mLMagicNum), _this->m_pNormalpipe->m_SofCBCookie);
        }
        _this->m_pNormalpipe->mEnqSofCBLock.unlock();

        _this->m_pNormalpipe->sendCommand(ENPipeCmd_GET_CUR_SOF_IDX,(MINTPTR)&_sof_idx2,0,0);
        if(_sof_idx != _sof_idx2){
            PIPE_INF("WARNING: N:%d: enqueRequest over vsync(%d_%d)", _this->m_pNormalpipe->mpSensorIdx,\
                    _sof_idx, _sof_idx2);
        }
    }
_exit:
    _this->m_EnqThrdState = _ThreadFinish;

    _this->mThreadIdleLock_Enq.unlock();

    ::sem_post(&_this->m_semEnqueThread);

    return NULL;
}


MVOID* NormalPipe_Thread::ispDequeThread(void *arg)
{
    NormalPipe_Thread *_this = reinterpret_cast<NormalPipe_Thread*>(arg);
    MUINT32 _sof_idx;
    MINT32 ret = 0;//success

    int const policy    = SCHED_OTHER;
    int const priority  = NICE_CAMERA_LOMO;
    struct sched_param sched_p;

    ::sched_getparam(0, &sched_p);
    if (policy == SCHED_OTHER) {
        setThreadPriority(policy, priority);
    }
    else {  //Note: "priority" is real-time priority.
        sched_p.sched_priority = priority;
        ::sched_setscheduler(0, policy, &sched_p);
    }

    #if (PTHREAD_CTRL_M == 0)
    ::pthread_detach(::pthread_self());
    #endif

    ::sem_wait(&_this->m_semDequeThread);

    _this->mThreadIdleLock_Deq.lock();

    if (MFALSE == _this->m_bStart) {
        PIPE_INF("deqThread early exit");
        goto _exit_deq;
    }

    PIPE_INF("WARNING: temp borrow NICE_CAMERA_LOMO priority, need to create self priority");
    switch (_NORMALPIPE_GET_TG_IDX(_this->m_pNormalpipe->mpSensorIdx)) {
    case CAM_TG_1:  ::prctl(PR_SET_NAME,"IspDeqThd_TG1",0,0,0);
        break;
    case CAM_TG_2:  ::prctl(PR_SET_NAME,"IspDeqThd_TG2",0,0,0);
        break;
    case CAM_SV_1:  ::prctl(PR_SET_NAME,"IspDeqThd_SV1",0,0,0);
        break;
    case CAM_SV_2:  ::prctl(PR_SET_NAME,"IspDeqThd_TG2",0,0,0);
        break;
    default:
        PIPE_ERR("ERROR: TG:0x%x(0x%x)", _NORMALPIPE_GET_TG_IDX(_this->m_pNormalpipe->mpSensorIdx),\
                _this->m_pNormalpipe->mpSensorIdx);
        goto _exit_deq;
    }

    while (1) {
        if (_this->m_bStart == MFALSE) {
            PIPE_DBG("deqThread leave");
            break;
        }

        _this->mThreadIdleLock_Deq.unlock();
        ::sem_wait(&_this->m_semDeqLoopToken);
        _this->mThreadIdleLock_Deq.lock();

        if (_this->m_bStart == MFALSE) {
            PIPE_DBG("deqThread leave");
            break;
        }

        _this->m_pNormalpipe->sendCommand(ENPipeCmd_GET_CUR_SOF_IDX, (MINTPTR)&_sof_idx, 0, 0);
        PIPE_DBG("N:%d_deq@SOF_%d", _this->m_pNormalpipe->mpSensorIdx,  _sof_idx);

        ret = _this->dequeResponse();

        if (ret < 0) {
            PIPE_ERR("dequeResponse false break");
            break;
        } else if (ret > 0){
            PIPE_WRN("deqThread leave:dequeResponse already stopped");
            break;
        }
    }

_exit_deq:
    if (ret == 0) {
        _this->m_DeqThrdState = _ThreadFinish;
    }
    else {
        MUINT32 i = 0;
        MUINT32 _OpenedPort = _this->m_pNormalpipe->GetOpenedPortInfo(NormalPipe::_GetPort_OpendNum);

        if (ret > 0)
            _this->m_DeqThrdState = _ThreadStop;
        else
            _this->m_DeqThrdState = _ThreadErrExit;

        for (i = 0; i < _OpenedPort; i++) {
            ::sem_post(&_this->m_semDeqDonePort[NPIPE_MAP_PORTID(_this->m_pNormalpipe->GetOpenedPortInfo(NormalPipe::_GetPort_Index, i))]);
        }
    }

    _this->mThreadIdleLock_Deq.unlock();

    ::sem_post(&_this->m_semDequeThread);

    return NULL;
}



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

/*++++++++++++++++from _MyUtils.h++++++++++++++++ */
#ifdef LOG_TAG
    #undef LOG_TAG
#endif
#define LOG_TAG     "StatisticPipe"


#include <utils/Vector.h>
#include <utils/KeyedVector.h>
using namespace android;

#include <mtkcam/def/common.h>
#include <mtkcam/utils/std/common.h>
#include <mtkcam/utils/metadata/client/mtk_metadata_tag.h>
using namespace NSCam;
using namespace NSCam::Utils;

//#include <kd_imgsensor_define.h>

/*------------------from _MyUtils.h------------------ */

//#include "VDThread.h"
/*++++++++++++++++from VDThread.h++++++++++++++++ */
#include <utils/Thread.h>
#include <utils/Condition.h>
/*------------------from VDThread.h------------------ */
#include <cutils/properties.h>

#include <IPipe.h>
#include <ICamIOPipe.h>
#include <ispio_utility.h>
#include <ispio_pipe_ports.h>

#include <mtkcam/drv/IHalSensor.h>

#include <mtkcam/drv/iopipe/CamIO/INormalPipe.h>


#include "NormalPipe.h"
#include "StatisticPipe.h"
#include "camio_log.h"
#include "FakeSensor.h"

#undef   DBG_LOG_TAG                        // Decide a Log TAG for current file.
#ifndef USING_MTK_LDVT   // Not using LDVT.
    #define  DBG_LOG_TAG        ""
#else
    #define  DBG_LOG_TAG        LOG_TAG
#endif

#define THIS_NAME   "StatisticPipe"

using namespace NSCam;
using namespace NSIoPipe;
using namespace NSCamIOPipe;
using namespace NSImageio;
using namespace NSIspio;

DECLARE_DBG_LOG_VARIABLE(iospipe);
#undef PIPE_VRB
#undef PIPE_DBG
#undef PIPE_INF
#undef PIPE_WRN
#undef PIPE_ERR
#undef PIPE_AST
#define PIPE_VRB(fmt, arg...)        do { if (iospipe_DbgLogEnable_VERBOSE) { BASE_LOG_VRB(fmt, ##arg); } } while(0)
#define PIPE_DBG(fmt, arg...)        do { if (iospipe_DbgLogEnable_DEBUG  ) { BASE_LOG_DBG(fmt, ##arg); } } while(0)
#define PIPE_INF(fmt, arg...)        do { if (iospipe_DbgLogEnable_INFO   ) { BASE_LOG_INF(fmt, ##arg); } } while(0)
#define PIPE_WRN(fmt, arg...)        do { if (iospipe_DbgLogEnable_WARN   ) { BASE_LOG_WRN(fmt, ##arg); } } while(0)
#define PIPE_ERR(fmt, arg...)        do { if (iospipe_DbgLogEnable_ERROR  ) { BASE_LOG_ERR(fmt, ##arg); } } while(0)
#define PIPE_AST(cond, fmt, arg...)  do { if (iospipe_DbgLogEnable_ASSERT ) { BASE_LOG_AST(cond, fmt, ##arg); } } while(0)


/******************************************************************************
 *  Global Function Declaration.
 ******************************************************************************/
extern "C" INormalPipe* createInstance_INormalPipe(MUINT32 SensorIndex, char const* szCallerName);


/******************************************************************************
 *
 ******************************************************************************/
#include <mtkcam/def/PriorityDefs.h>
#define PTHREAD_CTRL_M_STT (1)

//map portId to deque container index
#define _PortMap(PortIdx)   ({\
    MUINT32 _idx = 0;\
    if (PortIdx == PORT_AAO.index)\
        _idx = 0;\
    else if (PortIdx == PORT_AFO.index)\
        _idx = 1;\
    else if(PortIdx == PORT_FLKO.index)\
        _idx = 2;\
    else if(PortIdx == PORT_PDO.index)\
        _idx = 3;\
    else if(PortIdx == PORT_PSO.index)\
        _idx = 4;\
    else PIPE_ERR("error:portidx:0x%x\n",PortIdx);\
    _idx;\
})

static const EPortIndex m_Slot2PortIndex[] = {
    EPortIndex_AAO, EPortIndex_AFO, EPortIndex_FLKO, EPortIndex_PDO, EPortIndex_PSO
};
#define _MapPort(index) ({\
    if (index >= MaxStatPortIdx) {\
        PIPE_ERR("Invalid _port %d", index);\
    }\
    m_Slot2PortIndex[index];\
})

// mapping element NormalPipe_FrmB::mTgOut_size
#define _TGMapping(x)({\
    MUINT32 _tg=0;\
    if(x==CAM_TG_1)\
        _tg = 0;\
    else if(x==CAM_TG_2)\
        _tg = 1;\
    else if(x == CAM_SV_1)\
        _tg = 2;\
    else if(x == CAM_SV_2)\
        _tg = 3;\
    _tg;\
})

#define _SensorTypeToPortFormat(sensorType) ({\
    NSImageio::NSIspio::EScenarioFmt _fmt = NSImageio::NSIspio::eScenarioFmt_RAW;\
    switch(sensorType){\
        case NSCam::SENSOR_TYPE_RAW:  _fmt = NSImageio::NSIspio::eScenarioFmt_RAW;\
            break;\
        case NSCam::SENSOR_TYPE_YUV:  _fmt = NSImageio::NSIspio::eScenarioFmt_YUV;\
            break;\
        case NSCam::SENSOR_TYPE_RGB:  _fmt = NSImageio::NSIspio::eScenarioFmt_RGB;\
            break;\
        case NSCam::SENSOR_TYPE_JPEG: _fmt = NSImageio::NSIspio::eScenarioFmt_JPG;\
            break;\
        default:\
            break;\
   }\
   _fmt;\
})

#define SPIPE_MEM_NEW(dstPtr,type,size)\
do {\
    SPipeAllocMemSum += size; \
    dstPtr = new type;\
} while(0)

#define SPIPE_MEM_DEL(dstPtr,size)\
do{\
    SPipeFreedMemSum += size; \
    delete dstPtr;\
    dstPtr = NULL;\
}while(0)

#define NPIPE_DUMP_MEM_INFO(string) \
do {\
    PIPE_DBG("%s::SPipeAllocMemSum=0x%x, SPipeFreedMemSum=0x%x", string, SPipeAllocMemSum, SPipeFreedMemSum);\
}while(0)


static          MUINT32 SPipeAllocMemSum = 0;
static          MUINT32 SPipeFreedMemSum = 0;

Mutex           StatisticPipe::SPipeGLock;
StatisticPipe*  StatisticPipe::pStatisticPipe[EPIPE_Sensor_RSVD] = {NULL, NULL};


/******************************************************************************
 *
 ******************************************************************************/
IStatisticPipe* IStatisticPipe::createInstance(MUINT32 SensorIndex, char const* szCallerName)
{
    PIPE_DBG("+ SenIdx=%d %s", SensorIndex, szCallerName);

    StatisticPipe* pPipe = 0;

    if (SensorIndex >= EPIPE_Sensor_RSVD) {
        PIPE_ERR("InvalidSensorIdx = %d", SensorIndex);
        return MFALSE;
    }

    StatisticPipe::SPipeGLock.lock();

    pPipe = StatisticPipe::pStatisticPipe[SensorIndex];
    if (NULL == pPipe) {
        SPIPE_MEM_NEW(StatisticPipe::pStatisticPipe[SensorIndex], StatisticPipe(SensorIndex, THIS_NAME), sizeof(StatisticPipe));
        pPipe = StatisticPipe::pStatisticPipe[SensorIndex];
    }

    pPipe->addUser(szCallerName);

    StatisticPipe::SPipeGLock.unlock();

    PIPE_DBG("-");

    return pPipe;
}


void IStatisticPipe::destroyInstance(char const* szCallerName)
{
    (void)szCallerName;
    return;
}


/******************************************************************************
 *
 ******************************************************************************/
MVOID StatisticPipe::destroyInstance(char const* szCallerName)
{
    MINT32 idx = 0;

    SPipeGLock.lock();

    for (idx = 0; idx < EPIPE_Sensor_RSVD; idx++) {
        if (this == pStatisticPipe[idx]) {
            break;
        }
    }

    if (EPIPE_Sensor_RSVD == idx) {
        PIPE_ERR("Can't find the entry");
        SPipeGLock.unlock();
        return;
    }

    if (mTotalUserCnt <= 0) {
        PIPE_DBG("No instance now");
        SPipeGLock.unlock();
        return;
    }

    if (!delUser(szCallerName)) {
        PIPE_ERR("No such user");
        SPipeGLock.unlock();
        return;
    }

    if (mTotalUserCnt == 0)  {

        if (mpNormalPipe) {
            CAM_STATE_NOTIFY    notifyObj = {.fpNotifyState = NULL, .Obj = NULL};

            mpNormalPipe->sendCommand(ENPipeCmd_SET_IDLE_HOLD_CB, (MINTPTR)&notifyObj, 0, 0);
            mpNormalPipe->sendCommand(ENPipeCmd_SET_SUSPEND_STAT_CB, (MINTPTR)&notifyObj, 0, 0);

            mpNormalPipe->destroyInstance(THIS_NAME);
            mpNormalPipe = NULL;
        }

        if (mpSttIOPipe) {
            mpSttIOPipe->destroyInstance();
            mpSttIOPipe = NULL;
        }

        if (mpIMem) {
            MUINT32 i, j;
            for (i = 0; i < MaxStatPortIdx; i++) {
                if (mpMemInfo[i].size()) {
                    for (j = 0; j < mpMemInfo[i].size(); j++) {
                        if (mpMemInfo[i].at(j).virtAddr) {
                            #if 0
                            if (mpIMem->unmapPhyAddr(&(mpMemInfo[i].at(j))) < 0) {
                                MUINT32 portIdx = _MapPort(i);
                                PIPE_ERR("port_%d imem unmap error %p", portIdx,\
                                        mpMemInfo[i].at(j).phyAddr);
                                return;
                            }

                            if (mpIMem->freeVirtBuf(&(mpMemInfo[i].at(j))) < 0) {
                                MUINT32 portIdx = _MapPort(i);
                                PIPE_ERR("port_%d imem free error %p", portIdx,\
                                        mpMemInfo[i].at(j).virtAddr);
                                return;
                            }
                            #endif
                            mpMemInfo[i].at(j).virtAddr = 0;
                            mpMemInfo[i].at(j).phyAddr = 0;
                        }
                    }
                }
            }
            if (mImgHeaderPool.virtAddr) {
                #if 0
                if (mpIMem->unmapPhyAddr(&mImgHeaderPool)) {
                    PIPE_ERR("imgheader unmap error %p", mImgHeaderPool.phyAddr);
                }
                if (mpIMem->freeVirtBuf(&mImgHeaderPool)) {
                    PIPE_ERR("imgheader free error %p", mImgHeaderPool.virtAddr);
                }
                #endif
                mImgHeaderPool.virtAddr = 0;
                mImgHeaderPool.phyAddr = 0;
            }
            mpIMem->uninit();
            mpIMem->destroyInstance();
            mpIMem = NULL;
        }

        PIPE_INF("Destroy SPipe[%d]=0x%p", idx, pStatisticPipe[idx]);
        SPIPE_MEM_DEL(pStatisticPipe[idx], sizeof(StatisticPipe));
    }

    SPipeGLock.unlock();

    PIPE_DBG("-");
}


/******************************************************************************
 *
 ******************************************************************************/
StatisticPipe::StatisticPipe(MUINT32 SensorIdx, char const* szCallerName)
    : mpNormalPipe(NULL)
    , mpSttIOPipe(NULL)
    , mpName(szCallerName)
    , m_FSM(op_unknown)
    , mBurstQNum(1)
    , mTgInfo(0)
    , mTotalUserCnt(0)
    , mpSensorIdx(SensorIdx)
    , mOpenedPort(_DMAO_NONE)
    , mpIMem(NULL)
    , m_thread(0)
    , mvInPorts(NULL)
{
    DBG_LOG_CONFIG(imageio, iospipe);

    #if (STT_ENQ_IMM == 1)
    memset((void*)mpDeqContainer, 0, sizeof(mpDeqContainer));
    #else
    memset((void*)mpEnqueRequest, 0, sizeof(mpEnqueRequest));
    #endif
    memset((void*)PortIndex, 0, sizeof(PortIndex));
    memset((void*)mUserCnt, 0, sizeof(mUserCnt));
    memset((void*)mUserName, 0, sizeof(mUserName));
    memset((void*)mMemNextLegalEnqIdx, 0, sizeof(mMemNextLegalEnqIdx));

    if (NULL == mpNormalPipe) {
        CAM_STATE_NOTIFY    notifyObj = {.fpNotifyState = sttIdleStateHoldCB, .Obj = this};
        CAM_STATE_NOTIFY    notifySuspendObj = {.fpNotifyState = sttSuspendStateCB, .Obj = this};

        mpNormalPipe = createInstance_INormalPipe(SensorIdx, THIS_NAME);

        mpNormalPipe->sendCommand(ENPipeCmd_SET_IDLE_HOLD_CB, (MINTPTR)&notifyObj, 0, 0);
        mpNormalPipe->sendCommand(ENPipeCmd_SET_SUSPEND_STAT_CB, (MINTPTR)&notifySuspendObj, 0, 0);
    }
}


MBOOL StatisticPipe::FSM_CHECK(E_FSM op, const char *callee)
{
    MUINT32 ret = 2;


    this->m_FSMLock.lock();

    switch(op){
    case op_unknown:
        if(this->m_FSM != op_uninit)
            ret = 0;
        break;
    case op_init:
        if(this->m_FSM != op_unknown)
            ret = 0;
        break;
    case op_cfg:
        if(this->m_FSM != op_init)
            ret = 0;
        break;
    case op_start:
        switch (this->m_FSM) {
        case op_cfg:
        case op_suspend:
            break;
        default:
            ret = 0;
            break;
        }
        break;
    case op_stop:
        switch (this->m_FSM) {
        case op_start:
        case op_suspend:
            break;
        default:
            ret = 0;
            break;
        }
        break;
    case op_suspend:
        if (this->m_FSM != op_start) {
            ret = 0;
        }
        break;
    case op_uninit:
        switch(this->m_FSM){
        case op_init:
        case op_cfg:
        case op_stop:
            break;
        default:
            ret = 0;
            break;
        }
        break;
    case op_cmd:
        switch(this->m_FSM){
        case op_cfg:
        case op_start:
        case op_stop:
        case op_suspend:
            break;
        default:
            ret= 0;
            break;
        }
        break;
    case op_streaming:
        switch(this->m_FSM){
        case op_cfg:
        case op_start:
            break;
        case op_suspend: //for 3a resume before drv's resume
            ret = 1;
            break;
        default:
            ret= 0;
            break;
        }
        break;
    default:
        ret = 0;
        break;
    }

    this->m_FSMLock.unlock();
    //
    if (ret == 0) {
        PIPE_ERR("[%s]op error:cur:0x%x,tar:0x%x\n", callee, this->m_FSM,op);
        return MFALSE;
    }
    else if(ret == 1){
        PIPE_WRN("[%s]op error:cur:0x%x,tar:0x%x\n", callee, this->m_FSM,op);
        return MFALSE;
    }
    else
        return MTRUE;

}

MBOOL StatisticPipe::FSM_UPDATE(E_FSM op)
{
    switch(op){
    case op_unknown:
    case op_init:
    case op_cfg:
    case op_start:
    case op_stop:
    case op_suspend:
    case op_uninit:
        break;
    default:
        PIPE_ERR("op update error: cur:0x%x,tar:0x%x\n",this->m_FSM,op);
        return MFALSE;
    }

    this->m_FSMLock.lock();
    this->m_FSM = op;
    this->m_FSMLock.unlock();

    return MTRUE;
}


/******************************************************************************
 *
 ******************************************************************************/
MBOOL StatisticPipe::start()
{
    MBOOL   ret = MTRUE;
    MUINT32 _size = 0, _tmp;
    int s;

    PIPE_INF("+");

    s = pthread_join(m_thread, &m_thread_rst);
    if(s!=0) {
        PIPE_ERR("error(%d):can't wait for configPipe thread.",s);
        return MFALSE;
    } else
        PIPE_INF("pass(%d) :joined configPipe thread. ret:%s",s, (char*)m_thread_rst);

    if (this->FSM_CHECK(op_start, __FUNCTION__) == MFALSE) {
        return MFALSE;
    }

    #if (STT_ENQ_IMM == 0)
    ret = mpNormalPipe->sendCommand(ENPipeCmd_SET_STT_SOF_CB, (MINTPTR)enqueRequestCB, (MINTPTR)this, 0);
    if (MFALSE == ret) {
        PIPE_ERR("Reg SOF CBFP fail");
        return MFALSE;
    }
    #endif

    if (mpSttIOPipe) {
        ret = mpSttIOPipe->start();
        if (MFALSE == ret) {
            PIPE_ERR("TG_%d start fail", mTgInfo);
            return MFALSE;
        }
    }

    PIPE_DBG("-");

    this->FSM_UPDATE(op_start);

    return ret;
}


/******************************************************************************
 *
 ******************************************************************************/
MBOOL StatisticPipe::stop(MBOOL bNonblocking)
{
    MBOOL ret = MTRUE;
    MUINT32 _port;
    bNonblocking;

    PIPE_INF("SenIdx[%d] locking thread mutex", mpSensorIdx);

    for (_port = 0; _port < MaxStatPortIdx; _port++) {
        mThreadIdleLock[_port].lock();
    }

    if (this->FSM_CHECK(op_stop, __FUNCTION__) == MFALSE) {
        PIPE_ERR("TG_%d not start yet", mTgInfo);
        ret = MFALSE;
        goto _EXIT_STOP;
    }

    #if (STT_ENQ_IMM == 0)
    ret = mpNormalPipe->sendCommand(ENPipeCmd_CLR_STT_SOF_CB, (MINTPTR)enqueRequestCB, 0, 0);
    if (MFALSE == ret) {
        PIPE_ERR("UnReg SOF CBFP fail");
        goto _EXIT_STOP;
    }
    #endif

    if (mpSttIOPipe) {
        PIPE_INF("SenIdx[%d] stoping", mpSensorIdx);
        ret = mpSttIOPipe->stop();
        if (MFALSE == ret) {
            PIPE_ERR("TG_%d stop fail", mTgInfo);
            goto _EXIT_STOP;
        }
    }

    this->FSM_UPDATE(op_stop);

    PIPE_DBG("-");

_EXIT_STOP:
    for (_port = 0; _port < MaxStatPortIdx; _port++) {
        mThreadIdleLock[_port].unlock();
    }

    return ret;
}


/******************************************************************************
 *
 ******************************************************************************/
MBOOL StatisticPipe::init()
{
    MBOOL       ret = MTRUE, cmdSent = 0;
    MUINT32     i = 0;

    PIPE_INF("+ SenIdx[%d]", mpSensorIdx);

    if (this->FSM_CHECK(op_init, __FUNCTION__) == MFALSE) {
        return MFALSE;
    }

    do {
        Mutex::Autolock lock(mCfgLock);

        //sendCommand to query TG index
        ret = mpNormalPipe->sendCommand(ENPipeCmd_GET_TG_INDEX, (MINTPTR)&mTgInfo, 0, 0);
        if (MFALSE == ret) {
            cmdSent = ENPipeCmd_GET_TG_INDEX;
            goto _SEND_CMD_FAIL;
        }

        if (mpIMem) {
            PIPE_ERR("re-init");
            return MFALSE;
        }
        mpIMem = IMemDrv::createInstance();
        if (mpIMem) {
            ret = mpIMem->init();
        }
        if ((NULL == mpIMem) || (MFALSE == ret)) {
            PIPE_ERR("TG_%d imem create/init fail %p, %d", mTgInfo, mpIMem, ret);
        }

        #if (STT_ENQ_IMM == 1)
        for (i = 0; i < MaxStatPortIdx; i++) {
            if (!mpDeqContainer[i]) {
                SPIPE_MEM_NEW(mpDeqContainer[i], QueueMgr<QBufInfo>(), sizeof(QueueMgr<QBufInfo>));
                mpDeqContainer[i]->init();
            }
        }
        #else
        for (i = 0; i < MaxStatPortIdx; i++) {
            if (!mpEnqueRequest[i]) {
                SPIPE_MEM_NEW(mpEnqueRequest[i], QueueMgr<QBufInfo>(), sizeof(QueueMgr<QBufInfo>));
                mpEnqueRequest[i]->init();
            }
        }
        #endif

        mvInPorts = new QInitStatParam(mvp);
        PIPE_DBG("####mvInputPorts(%p)",mvInPorts);

    } while(0);

    PIPE_DBG("-");

    this->FSM_UPDATE(op_init);

    return ret;

_SEND_CMD_FAIL:
    PIPE_ERR("TG_%d cmdSent x%08x fail", mTgInfo, cmdSent);

    return ret;
}


/******************************************************************************
 *
 ******************************************************************************/
MBOOL StatisticPipe::uninit()
{
    MUINT32 i = 0;
    MBOOL   ret = MTRUE;

    PIPE_INF("+ SenIdx[%d]", mpSensorIdx);

    if (this->FSM_CHECK(op_uninit, __FUNCTION__) == MFALSE) {
        return MFALSE;
    }

    if (mpSttIOPipe)  {
        ret = mpSttIOPipe->uninit();
    }

    for (i = 0; i < MaxStatPortIdx; i++) {
        #if (STT_ENQ_IMM == 1)
        if (mpDeqContainer[i])  {
            mpDeqContainer[i]->uninit();
            SPIPE_MEM_DEL(mpDeqContainer[i], sizeof(QueueMgr<QBufInfo>));
        }
        #else
        if (mpEnqueRequest[i])  {
            mpEnqueRequest[i]->uninit();
            SPIPE_MEM_DEL(mpEnqueRequest[i], sizeof(QueueMgr<QBufInfo>));
        }
        #endif
        mpImgHeaderMgr[i].uninit();
    }

    PIPE_DBG("####deleted mvInputPorts(%p)", mvInPorts);
    delete mvInPorts;

    this->FSM_UPDATE(op_uninit);

    PIPE_DBG("-");

    return ret;
}

/******************************************************************************
 *
 ******************************************************************************/
MBOOL StatisticPipe::enque(QBufInfo const& rQBuf)
{
#if (STT_ENQ_IMM == 1)
    NSImageio::NSIspio::PortID      portID;
    NSImageio::NSIspio::QBufInfo    rQBufInfo;
    IMEM_BUF_INFO imBuf;
    QBufInfo    _qBuf;
    MUINT32     i = 0, _port = 0, memIndex = 0, _bsize = 0;
    MINT32 ret = 0;

    Mutex::Autolock lock(mEnQLock);

    PIPE_DBG("+");

    if (this->FSM_CHECK(op_streaming, __FUNCTION__) == MFALSE) {
        return MFALSE;
    }

    _bsize = rQBuf.mvOut.size();

    if ((0 == _bsize) || (mBurstQNum != _bsize)) {
        PIPE_ERR("TG_%d enq buf size ng %d/%d", mTgInfo, _bsize, mBurstQNum);
        return MFALSE;
    }

    portID.index = rQBuf.mvOut.at(0).mPortID.index;
    _port = _PortMap(portID.index);

    if (this->m_FSM == op_start) {
        MUINT32 _curDepth = 0;

        mpDeqContainer[_port]->sendCmd(QueueMgr<QBufInfo>::eCmd_getsize, (MUINTPTR)&_curDepth, 0);

        if (0 == _curDepth) {
            PIPE_ERR("TG_%d port_%d ERROR: enque before deque", mTgInfo, portID.index);
            return MFALSE;
        }

        mpDeqContainer[_port]->sendCmd(QueueMgr<QBufInfo>::eCmd_front, (MUINTPTR)&_qBuf, 0);

        if (_qBuf.mvOut.size() != mBurstQNum) {
            PIPE_ERR("TG_%d port_%d ERROR: buf depth %d/%d", mTgInfo, portID.index, (MUINT32)_qBuf.mvOut.size(), mBurstQNum);
            return MFALSE;
        }

        for (i = 0; i < _bsize; i++) {
            if (_qBuf.mvOut.at(i).mPa != rQBuf.mvOut.at(i).mPa) {
                PIPE_ERR("TG_%d port_%d ERROR: addr x%" PRIXPTR "/x%" PRIXPTR " unsync", mTgInfo, portID.index,\
                        _qBuf.mvOut.at(i).mPa, rQBuf.mvOut.at(i).mPa);
                return MFALSE;
            }
        }
    }

    rQBufInfo.vBufInfo.resize(_bsize);
    for (i = 0, memIndex = mMemNextLegalEnqIdx[_port]; i < _bsize; i++) {
        rQBufInfo.vBufInfo[i].u4BufSize[0] = mpMemInfo[_port].at(memIndex).size;
        rQBufInfo.vBufInfo[i].u4BufVA[0] = mpMemInfo[_port].at(memIndex).virtAddr;
        rQBufInfo.vBufInfo[i].u4BufPA[0] = mpMemInfo[_port].at(memIndex).phyAddr;
        rQBufInfo.vBufInfo[i].memID[0] = mpMemInfo[_port].at(memIndex).memID;

        if (!mpImgHeaderMgr[_port].sendCmd(QueueMgr<IMEM_BUF_INFO>::eCmd_at, i, (MUINTPTR)&imBuf)) {
            PIPE_ERR("TG_%d port_%d out of imgheader buf", mTgInfo, portID.index);
            return MFALSE;
        }

        rQBufInfo.vBufInfo[i].Frame_Header.u4BufSize = imBuf.size;
        rQBufInfo.vBufInfo[i].Frame_Header.u4BufVA  = imBuf.virtAddr;
        rQBufInfo.vBufInfo[i].Frame_Header.u4BufPA  = imBuf.phyAddr;
        rQBufInfo.vBufInfo[i].Frame_Header.memID  = imBuf.memID;

        memIndex++;
        if (memIndex >= mpMemInfo[_port].size()) {
            memIndex = 0;
        }
    }

    if ((ret = mpSttIOPipe->enqueOutBuf(portID, rQBufInfo)) != 0) {
        if(ret > 0)
            PIPE_WRN("TG_%d port_%d enq:already stopped", mTgInfo, portID.index);
        else
            PIPE_ERR("TG_%d port_%d enq fail", mTgInfo, portID.index);
        return MFALSE;
    }

    mMemNextLegalEnqIdx[_port] = memIndex;
    for (i = 0; i < _bsize; i++) {
        mpImgHeaderMgr[_port].sendCmd(QueueMgr<IMEM_BUF_INFO>::eCmd_pop, 0, 0);
    }

    if (this->m_FSM == op_start) {
        mpDeqContainer[_port]->sendCmd(QueueMgr<QBufInfo>::eCmd_pop, 0, 0);
    }

    for (i = 0; i < _bsize; i++) {
        PIPE_DBG("SttE:%d: dma:x%x pa(x%" PRIXPTR "/x%" PRIXPTR ") fh_pa(x%" PRIXPTR ")", mpSensorIdx, portID.index,\
                rQBufInfo.vBufInfo[i].u4BufPA[0], rQBuf.mvOut.at(i).mPa, rQBufInfo.vBufInfo[i].Frame_Header.u4BufPA);
    }

    PIPE_DBG("-");

    return MTRUE;
#else
    NSImageio::NSIspio::PortID      portID;
    NSImageio::NSIspio::QBufInfo    rQBufInfo;
    IMEM_BUF_INFO imBuf;
    MBOOL       ret = MTRUE;
    MUINT32     i = 0, _port = 0, memIndex = 0, _bsize = rQBuf.mvOut.size();

    Mutex::Autolock lock(mEnQLock);

    PIPE_DBG("+");

    if ((0 == _bsize) || (mBurstQNum != _bsize)) {
        PIPE_ERR("TG_%d enq buf size ng %d/%d", mTgInfo, _bsize, mBurstQNum);
        return MFALSE;
    }

    _port = _PortMap(rQBuf.mvOut.at(0).mPortID.index);

    if (0 == mpMemInfo[_port].size()) {
        PIPE_ERR("TG_%d port_%d mem num %d", mTgInfo, _port, mpMemInfo[_port].size());
        return MFALSE;
    }

    portID.index = rQBuf.mvOut.at(0).mPortID.index;

    /*
     * Check enque PA duplicate or not
     */
    for (i = 0, memIndex = mMemNextLegalEnqIdx[_port]; i < _bsize; i++) {
        if (mpMemInfo[_port].at(memIndex).phyAddr != rQBuf.mvOut.at(i).mPa) {
            PIPE_ERR("TG_%d port_%d enq addr x%x/x%x unsync", mTgInfo, portID.index,\
                    mpMemInfo[_port].at(memIndex).phyAddr, rQBuf.mvOut.at(i).mPa);
            return MFALSE;
        }

        memIndex++;
        if (memIndex >= mpMemInfo[_port].size()) {
            memIndex = 0;
        }
    }
    mMemNextLegalEnqIdx[_port] = memIndex;

    mEnqContainerLock.lock();

    mpEnqueRequest[_port]->sendCmd(QueueMgr<QBufInfo>::eCmd_push, (MUINTPTR)&rQBuf, 0);

    mEnqContainerLock.unlock();

    for (i = 0; i < rQBuf.mvOut.size(); i++) {
        BufInfo* pBuf = (BufInfo*)&(rQBuf.mvOut.at(i));
        PIPE_INF("SttEnq: dma(0x%x) pa(0x%x) mag(0x%x)\n", pBuf->mPortID.index, pBuf->mPa,\
                pBuf->FrameBased.mMagicNum_tuning);
    }

    PIPE_DBG("-");

    return ret;
#endif
}

/******************************************************************************
 *
 ******************************************************************************/
void StatisticPipe::enqueRequestCB(void *para, void *cookie)
{
#if (STT_ENQ_IMM == 1)
    (void)para;
    (void)cookie;
    PIPE_ERR("Unsupported");
    return;
#else
    StatisticPipe*  _this = reinterpret_cast<StatisticPipe*>(cookie);
    QBufInfo        _qBuf;
    list<MUINT32>*  pLMagic = (list<MUINT32>*)para;
    MUINT32         i, g, p, _port, _bsize = 0;
    MUINT32         _magic = 0;
    MUINT32         reqQueNum[MaxStatPortIdx] = {0};
    MINT32          ret = 0;

    PIPE_DBG("+");

    if (pLMagic->size()) {
        _magic = pLMagic->back();
    }

    _this->mEnqContainerLock.lock();

    for (p = 0; p < _this->GetOpenedPortInfo(StatisticPipe::_GetPort_OpendNum); p++) {
        NSImageio::NSIspio::PortID      portID;
        NSImageio::NSIspio::QBufInfo    rQBufInfo;
        IMEM_BUF_INFO   imBuf;
        BufInfo*        pBuf;

        portID.index = _this->GetOpenedPortInfo(StatisticPipe::_GetPort_Index, p);
        _port = _PortMap(portID.index);

        _this->mpEnqueRequest[_port]->sendCmd(QueueMgr<QBufInfo>::eCmd_getsize, (MUINTPTR)&(reqQueNum[_port]), 0);
        if (0 == reqQueNum[_port]) {
            /* No enque at this port */
            PIPE_DBG("TG_%d port_%d no req at cur frame", _this->mTgInfo,\
                    _this->GetOpenedPortInfo(StatisticPipe::_GetPort_Index, p));
            continue;
        }

        _this->mpEnqueRequest[_port]->sendCmd(QueueMgr<QBufInfo>::eCmd_front, (MINTPTR)&_qBuf, 0);

        _bsize = _qBuf.mvOut.size();
        rQBufInfo.vBufInfo.resize(_bsize);
        for (i = 0; i < _bsize; i++) {

            rQBufInfo.vBufInfo[i].u4BufSize[0] = _this->mpMemInfo[_port].at(0).size;
            rQBufInfo.vBufInfo[i].u4BufVA[0] = _qBuf.mvOut.at(i).mVa;
            rQBufInfo.vBufInfo[i].u4BufPA[0] = _qBuf.mvOut.at(i).mPa;
            rQBufInfo.vBufInfo[i].m_num = _magic;

            //image header
            if (!_this->mpImgHeaderMgr[_port].sendCmd(QueueMgr<IMEM_BUF_INFO>::eCmd_at, i, (MUINTPTR)&imBuf)) {
                PIPE_ERR("TG_%d port_%d out of imgheader buf", _this->mTgInfo, portID.index);
                goto _CB_EXIT;
            }
            rQBufInfo.vBufInfo[i].Frame_Header.bReplace = MFALSE;
            rQBufInfo.vBufInfo[i].Frame_Header.u4BufSize = imBuf.size;
            rQBufInfo.vBufInfo[i].Frame_Header.u4BufVA  = imBuf.virtAddr;
            rQBufInfo.vBufInfo[i].Frame_Header.u4BufPA  = imBuf.phyAddr;
            rQBufInfo.vBufInfo[i].Frame_Header.memID    = imBuf.memID;
            rQBufInfo.vBufInfo[i].Frame_Header.bufSecu  = imBuf.bufSecu;
            rQBufInfo.vBufInfo[i].Frame_Header.bufCohe  = imBuf.bufCohe;
        }

        if (( ret = _this->mpSttIOPipe->enqueOutBuf(portID, rQBufInfo)) != 0) {
            if(ret > 1)
                PIPE_WRN("TG_%d port_%d enq:already stopped", _this->mTgInfo, portID.index);
            else
                PIPE_ERR("TG_%d port_%d enq fail", _this->mTgInfo, portID.index);
            goto _CB_EXIT;
        }
        else {
            for (i = 0; i < _bsize; i++) {
                _this->mpImgHeaderMgr[_port].sendCmd(QueueMgr<IMEM_BUF_INFO>::eCmd_pop, 0, 0);
            }

            _this->mpEnqueRequest[_port]->sendCmd(QueueMgr<QBufInfo>::eCmd_pop, 0, 0);
        }

        for (g = 0; g < _qBuf.mvOut.size(); g++) {
            PIPE_DBG("P(%p) M(%d)", _qBuf.mvOut.at(g).mPa, _magic);
        }
    }

_CB_EXIT:
    _this->mEnqContainerLock.unlock();

    PIPE_DBG("-");
#endif
}

MBOOL StatisticPipe::immediateEnque(QBufInfo const& rQBuf)
{
#if (STT_ENQ_IMM == 1)
    (void)rQBuf;
    PIPE_ERR("Unsupported");
    return MFALSE;
#else
    NSImageio::NSIspio::PortID      portID;
    NSImageio::NSIspio::QBufInfo    rQBufInfo;
    IMEM_BUF_INFO imBuf;
    MUINT32     i = 0, _port = 0, memIndex = 0, _bsize = rQBuf.mvOut.size();
    MINT32      ret = 0;

    Mutex::Autolock lock(mEnQLock);

    PIPE_DBG("SttImmEnq:+");

    if ((0 == _bsize) || (mBurstQNum != _bsize)) {
        PIPE_ERR("TG_%d enq buf size ng %d/%d", mTgInfo, _bsize, mBurstQNum);
        return MFALSE;
    }

    _port = _PortMap(rQBuf.mvOut.at(0).mPortID.index);

    if (0 == mpMemInfo[_port].size()) {
        PIPE_ERR("TG_%d port_%d mem num %d", mTgInfo, _port, mpMemInfo[_port].size());
        return MFALSE;
    }

    portID.index = rQBuf.mvOut.at(0).mPortID.index;

    rQBufInfo.vBufInfo.resize(_bsize);
    for (i = 0, memIndex = mMemNextLegalEnqIdx[_port]; i < _bsize; i++) {
        #if (STT_ENQ_IMM == 1)
        rQBufInfo.vBufInfo[i].u4BufSize[0] = mpMemInfo[_port].at(memIndex).size;
        rQBufInfo.vBufInfo[i].u4BufVA[0] = rQBuf.mvOut.at(i).mVa;
        rQBufInfo.vBufInfo[i].u4BufPA[0] = rQBuf.mvOut.at(i).mPa;
        //image header
        if (!mpImgHeaderMgr[_port].sendCmd(QueueMgr<IMEM_BUF_INFO>::eCmd_at, i, (MUINTPTR)&imBuf)) {
            PIPE_ERR("TG_%d port_%d out of imgheader buf", mTgInfo, portID.index);
            return MFALSE;
        }

        rQBufInfo.vBufInfo[i].Frame_Header.u4BufSize = imBuf.size;
        rQBufInfo.vBufInfo[i].Frame_Header.u4BufVA  = imBuf.virtAddr;
        rQBufInfo.vBufInfo[i].Frame_Header.u4BufPA  = imBuf.phyAddr;
        #else
        rQBufInfo.vBufInfo[i].u4BufSize[0] = mpMemInfo[_port].at(memIndex).size;
        rQBufInfo.vBufInfo[i].u4BufVA[0] = rQBuf.mvOut.at(i).mVa;
        rQBufInfo.vBufInfo[i].u4BufPA[0] = rQBuf.mvOut.at(i).mPa;
        rQBufInfo.vBufInfo[i].m_num = 0;//TBD temp solution

        //image header
        if (!mpImgHeaderMgr[_port].sendCmd(QueueMgr<IMEM_BUF_INFO>::eCmd_at, i, (MUINTPTR)&imBuf)) {
            PIPE_ERR("TG_%d port_%d out of imgheader buf", mTgInfo, portID.index);
            return MFALSE;
        }

        rQBufInfo.vBufInfo[i].Frame_Header.bReplace = MFALSE;
        rQBufInfo.vBufInfo[i].Frame_Header.u4BufSize = imBuf.size;
        rQBufInfo.vBufInfo[i].Frame_Header.u4BufVA  = imBuf.virtAddr;
        rQBufInfo.vBufInfo[i].Frame_Header.u4BufPA  = imBuf.phyAddr;
        rQBufInfo.vBufInfo[i].Frame_Header.memID    = imBuf.memID;
        rQBufInfo.vBufInfo[i].Frame_Header.bufSecu  = imBuf.bufSecu;
        rQBufInfo.vBufInfo[i].Frame_Header.bufCohe  = imBuf.bufCohe;
        #endif

        memIndex++;
        if (memIndex >= mpMemInfo[_port].size()) {
            memIndex = 0;
        }
    }

    if ( (ret = mpSttIOPipe->enqueOutBuf(portID, rQBufInfo)) != 0) {
        if(ret > 1)
            PIPE_WRN("TG_%d port_%d enq:already stopped", mTgInfo, portID.index);
        else
            PIPE_ERR("TG_%d port_%d enq fail", mTgInfo, portID.index);
        return MFALSE;
    }
    else {
        mMemNextLegalEnqIdx[_port] = memIndex;
        for (i = 0; i < _bsize; i++) {
            mpImgHeaderMgr[_port].sendCmd(QueueMgr<IMEM_BUF_INFO>::eCmd_pop, 0, 0);
        }
    }

    for (i = 0; i < _bsize; i++) {
        #if (STT_ENQ_IMM == 1)
        PIPE_DBG("SttEnq: dma(0x%x) pa(x%x/x%x) fh_pa(x%x)", portID.index, rQBufInfo.vBufInfo[i].u4BufPA[0],\
                rQBuf.mvOut.at(i).mPa, rQBufInfo.vBufInfo[i].Frame_Header.u4BufPA);
        #else
        PIPE_DBG("SttEnq: dma(0x%x) pa(x%x/x%x) mag(0x%x)", portID.index, rQBufInfo.vBufInfo[i].u4BufPA[0],\
                rQBufInfo.vBufInfo[i].Frame_Header.u4BufPA, rQBufInfo.vBufInfo[i].m_num);
        #endif
    }

    PIPE_DBG("SttImmEnq:-");

    return MTRUE;
#endif
}


/******************************************************************************
 *
 ******************************************************************************/
MBOOL StatisticPipe::deque(PortID port, QBufInfo& rQBuf, ESTT_CACHE_SYNC_POLICY cacheSyncPolicy, MUINT32 u4TimeoutMs)
{
    MUINT32         idx = 0, buf = 0, _port = 0;
    IMEM_BUF_INFO   imBuf;
    NSImageio::NSIspio::QTimeStampBufInfo   rQTSBufInfo;
    NSImageio::NSIspio::PortID              portID;
    CAM_STATE_NOTIFY                        notifyObj = {.fpNotifyState = sttStateNotify};

    PIPE_DBG("+");

    if (this->FSM_CHECK(op_streaming, __FUNCTION__) == MFALSE) {
        return MFALSE;
    }

    portID.index = port.index;
    _port = _PortMap(port.index);

    mThreadIdleLock[_port].lock();
    notifyObj.Obj = (MVOID*)&(mThreadIdleLock[_port]);

    switch(mpSttIOPipe->dequeOutBuf(portID, rQTSBufInfo, u4TimeoutMs, &notifyObj)){
        case eBuf_Fail:
            PIPE_ERR("error:TG%d port_%d deque fail", mTgInfo, portID.index);
            mThreadIdleLock[_port].unlock();
            return MFALSE;
            break;
        case eBuf_suspending:
            rQBuf.mvOut.resize(0);
            mThreadIdleLock[_port].unlock();
            return MFALSE;
            break;
        case eBuf_stopped:
            PIPE_WRN("TG%d deque port_%d:already stopped\n", mTgInfo, portID.index);
            rQBuf.mvOut.resize(0);
            mThreadIdleLock[_port].unlock();
            return MTRUE;
            break;
        default:
            break;
    }

    if (rQTSBufInfo.vBufInfo.size() != mBurstQNum) {
        PIPE_INF("WARNNING: TG_%d port_%d deque buf num unsync, %d/%d", mTgInfo,\
                portID.index, (MUINT32)rQTSBufInfo.vBufInfo.size(), mBurstQNum);
    }

    rQBuf.mvOut.resize(rQTSBufInfo.vBufInfo.size());
    for (idx = 0; idx < rQTSBufInfo.vBufInfo.size(); idx++) {
        BufInfo* pbuf = &(rQBuf.mvOut.at(idx));
        ResultMetadata result;

        result.mDstSize = rQTSBufInfo.vBufInfo.at(idx).DstSize;
        result.mMagicNum_hal = rQTSBufInfo.vBufInfo.at(idx).m_num;
        result.mMagicNum_tuning = rQTSBufInfo.vBufInfo.at(idx).m_num;
        result.mTimeStamp = getTimeStamp_ns(rQTSBufInfo.vBufInfo.at(idx).i4TimeStamp_sec,
                                rQTSBufInfo.vBufInfo.at(idx).i4TimeStamp_us);
        result.mRawType = rQTSBufInfo.vBufInfo.at(idx).raw_type;//for pso, 0:before LSC/1:before OB

        pbuf->mPortID   = portID.index;
        pbuf->mBuffer   = NULL;
        pbuf->mMetaData = result;
        pbuf->mSize     = rQTSBufInfo.vBufInfo.at(idx).u4BufSize[0]; //TBD or returns IMemSize: mpMemInfo[_port].at(0).size
        pbuf->mVa       = rQTSBufInfo.vBufInfo.at(idx).u4BufVA[0];
        pbuf->mPa       = rQTSBufInfo.vBufInfo.at(idx).u4BufPA[0];
        pbuf->mPa_offset = rQTSBufInfo.vBufInfo.at(idx).u4BufOffset[0];
        pbuf->mStride = rQTSBufInfo.vBufInfo.at(idx).img_stride;
        pbuf->mMetaData.mPrivateData = (MVOID*)rQTSBufInfo.vBufInfo.at(idx).m_pPrivate;

        PIPE_INF("SttD:%d: dma:x%x pa(0x%" PRIXPTR ") size(%d) mag(0x%x) stride(%d) type(%x)\n", mpSensorIdx,\
                portID.index, pbuf->mPa, pbuf->mSize, result.mMagicNum_hal,pbuf->mStride,result.mRawType);

        imBuf           = mImgHeaderPool;
        imBuf.virtAddr  = rQTSBufInfo.vBufInfo.at(idx).Frame_Header.u4BufVA;
        imBuf.phyAddr   = rQTSBufInfo.vBufInfo.at(idx).Frame_Header.u4BufPA;
        this->mpSttIOPipe->sendCommand(EPIPECmd_GET_HEADER_SIZE,(MINTPTR)portID.index,(MINTPTR)&imBuf.size,0);
        mpImgHeaderMgr[_port].sendCmd(QueueMgr<IMEM_BUF_INFO>::eCmd_push, (MUINTPTR)&imBuf, 0);

        /* cache invalidate */
        for (buf = 0; buf < mpMemInfo[_port].size(); buf++) {
            if (pbuf->mPa == mpMemInfo[_port].at(buf).phyAddr) {
                PIPE_DBG("cache sync PA(x%" PRIXPTR ") policy(%d)", pbuf->mPa, cacheSyncPolicy);
                switch (cacheSyncPolicy) {
                case ESTT_CacheBypass:
                    PIPE_WRN("SttD:%d: dma:x%x PA:0x%" PRIXPTR " bypass cache sync!", mpSensorIdx, portID.index, pbuf->mPa);
                    break;
                case ESTT_CacheInvalidAll:
                    //mpIMem->cacheSyncbyRange(IMEM_CACHECTRL_ENUM_INVALID_ALL, &(mpMemInfo[_port].at(buf)), MFALSE);
                    PIPE_WRN("SttD:%d: dma:x%x PA:0x%" PRIXPTR " no support CacheInvalidAll!", mpSensorIdx, portID.index, pbuf->mPa);
                    break;
                case ESTT_CacheInvalidByRange:
                default:
                    mpIMem->cacheSyncbyRange(IMEM_CACHECTRL_ENUM_INVALID, &(mpMemInfo[_port].at(buf)));
                    break;
                }
                break;
            }
        }
        if (buf == mpMemInfo[_port].size()) {
            PIPE_ERR("Cannot find pa %" PRIXPTR " to sync cache", pbuf->mPa);
        }

        mpDeqContainer[_port]->sendCmd(QueueMgr<QBufInfo>::eCmd_push, (MUINTPTR)&rQBuf, 0);
    }
    PIPE_DBG("-");

    mThreadIdleLock[_port].unlock();

    return MTRUE;
}

MBOOL StatisticPipe::addUser(char const* szUserName)
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
            strncpy(mUserName[i], szUserName, sizeof(mUserName[i])-1);
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

MBOOL StatisticPipe::delUser(char const* szUserName)
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

MINT32 StatisticPipe::getTotalUserNum(void)
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
MBOOL StatisticPipe::configPipe_bh()
{
    MBOOL  ret = MTRUE;
    MUINT32     uni_available = 1, MainImgBQNum = 0, totalBufNum = 0, _portNum = 0;
    MUINT32     i = 0, j = 0, _portIdx = 0, imgW = 0, imgH = 0, dma_size = 0;
    MUINT32     afo_stride = 0, afo_dummy_stride = 0,pdo_stride = 0;
    MUINTPTR    imgHdrAllocOfst = 0;
    list<MUINTPTR>                  ImgPA_L[MaxStatPortIdx];
    list<MUINTPTR>                  ImgHeaderPA_L[MaxStatPortIdx];
    PIPE_DBG("+ :start of configPipe_bh");

    do {

        totalBufNum = mBurstQNum * MAX_STAT_BUF_NUM;

        ret = mpNormalPipe->sendCommand(ENPipeCmd_GET_TG_OUT_SIZE, (MINTPTR)&imgW, (MINTPTR)&imgH, 0);
        if (MFALSE == ret) {
            PIPE_ERR("Get TG size fail");
            return MFALSE;
        }

        for (i = 0; i < mvInPorts->mStatPortInfo.size(); i++) {
            switch (mvInPorts->mStatPortInfo.at(i).mPortID.index) {
                case EPortIndex_AAO:
                case EPortIndex_AFO:
                case EPortIndex_FLKO:
                case EPortIndex_PSO:
                case EPortIndex_PDO:
                    break;
                default:
                    PIPE_ERR("TG_%d unknown idx", mTgInfo);
                    return MFALSE;
            }
        }


        /* allocate frame header buffer */
        /* afo needs additional size for addr offset after twin case */
        mImgHeaderPool.useNoncache = 1;
        mImgHeaderPool.size = 0;
        for (MUINT32 headersize = 0,i = 0; i < mvInPorts->mStatPortInfo.size(); i++) {
            this->mpSttIOPipe->sendCommand(EPIPECmd_GET_HEADER_SIZE,(MINTPTR)mvInPorts->mStatPortInfo.at(i).mPortID.index,(MINTPTR)&headersize,0);
            mImgHeaderPool.size += (headersize*totalBufNum);
        }
        mImgHeaderPool.size += 8;//+8 for 64bit alignment

        if (MFALSE == mpNormalPipe->sendCommand(ENPipeCmd_SET_MEM_CTRL, 0, (MINTPTR)&mImgHeaderPool, 0)) {
            return MFALSE;
        }
        imgHdrAllocOfst = ((8 - (mImgHeaderPool.virtAddr & 7)) & 7); //align 8

        for (MUINT32 headersize = 0,i = 0; i < mvInPorts->mStatPortInfo.size(); i++) {
            _portIdx = _PortMap(mvInPorts->mStatPortInfo.at(i).mPortID.index);
            this->mpSttIOPipe->sendCommand(EPIPECmd_GET_HEADER_SIZE,(MINTPTR)mvInPorts->mStatPortInfo.at(i).mPortID.index,(MINTPTR)&headersize,0);
            mpImgHeaderMgr[_portIdx].init();

            switch (mvInPorts->mStatPortInfo.at(i).mPortID.index) {
            case EPortIndex_AAO:
                mpSttIOPipe->sendCommand(EPIPECmd_GET_AAO_CONS,(MINTPTR)&dma_size,(MINTPTR)imgW,(MINTPTR)imgH);
                break;
            case EPortIndex_AFO:
                mpSttIOPipe->sendCommand(EPIPECmd_GET_AFO_CONS,(MINTPTR)&dma_size,(MINTPTR)imgW,(MINTPTR)imgH);
                break;
            case EPortIndex_FLKO:
                mpSttIOPipe->sendCommand(EPIPECmd_GET_FLKO_CONS,(MINTPTR)&dma_size,(MINTPTR)imgW,(MINTPTR)imgH);
                break;
            case EPortIndex_PDO:
                switch(mvInPorts->mStatPortInfo.at(i).mFunc.Bits.DATA_PATTERN){
                    case EPipe_Normal:
                        mpSttIOPipe->sendCommand(EPIPECmd_GET_PDO_CONS,(MINTPTR)&dma_size,(MINTPTR)mvInPorts->mStatPortInfo.at(i).mWidth,(MINTPTR)mvInPorts->mStatPortInfo.at(i).mHeight);
                        break;
                    case EPipe_Dual_pix:
                        mpSttIOPipe->sendCommand(EPIPECmd_GET_PDO_CONS,(MINTPTR)&dma_size,(MINTPTR)imgW,(MINTPTR)imgH);
                        break;
                    default:
                        PIPE_ERR("unknown pattern:%d\n", mvInPorts->mStatPortInfo.at(i).mFunc.Bits.DATA_PATTERN);
                        return MFALSE;
                        break;
                }
                break;
            case EPortIndex_PSO:
                mpSttIOPipe->sendCommand(EPIPECmd_GET_PSO_CONS,(MINTPTR)&dma_size,(MINTPTR)imgW,(MINTPTR)imgH);
                break;
            default:
                PIPE_ERR("TG_%d unknown idx", mTgInfo);
                return MFALSE;
            }

            mpMemInfo[_portIdx].resize(totalBufNum);
            for (j = 0; j < totalBufNum; j++, imgHdrAllocOfst += headersize) {
                IMEM_BUF_INFO   *pBuf = NULL;
                IMEM_BUF_INFO   iHeader(headersize, mImgHeaderPool.memID,
                                    (mImgHeaderPool.virtAddr + imgHdrAllocOfst),
                                    (mImgHeaderPool.phyAddr + imgHdrAllocOfst),
                                    mImgHeaderPool.bufSecu, mImgHeaderPool.bufCohe, mImgHeaderPool.useNoncache);
                mpImgHeaderMgr[_portIdx].sendCmd(QueueMgr<IMEM_BUF_INFO>::eCmd_push, (MUINTPTR)&iHeader, 0);

                pBuf = &(mpMemInfo[_portIdx].at(j));
                pBuf->size = dma_size;
                pBuf->useNoncache = 0; //cachable

                if (MFALSE == mpNormalPipe->sendCommand(ENPipeCmd_SET_MEM_CTRL, 0, (MINTPTR)pBuf, 0)) {
                    return MFALSE;
                }

                ImgPA_L[_portIdx].push_back(pBuf->phyAddr);
                ImgHeaderPA_L[_portIdx].push_back(iHeader.phyAddr);
            }
            mMemNextLegalEnqIdx[_portIdx] = 0; //enq start from 1st buf
            PIPE_DBG("TG_%d port_%d alloc %dx%d", mTgInfo, mvInPorts->mStatPortInfo.at(i).mPortID.index, dma_size, totalBufNum);

        }

        if (imgHdrAllocOfst >= mImgHeaderPool.size) {
            PIPE_ERR("TG_%d imgHeader execeed alloc v/p x%" PRIXPTR "/x%" PRIXPTR " + x%" PRIXPTR "/x%x", mTgInfo, mImgHeaderPool.virtAddr,\
                    mImgHeaderPool.phyAddr, imgHdrAllocOfst, mImgHeaderPool.size);
            return MFALSE;
        }
        else {
            PIPE_INF("TG_%d imgHeader alloc x%" PRIXPTR "/x%x @x%" PRIXPTR "/x%" PRIXPTR "", mTgInfo, imgHdrAllocOfst, mImgHeaderPool.size,\
                    mImgHeaderPool.virtAddr, mImgHeaderPool.phyAddr);
        }

        this->FSM_UPDATE(op_cfg);

    } while(0);

    _portNum = GetOpenedPortInfo(StatisticPipe::_GetPort_OpendNum);
    for (i = 0; i < _portNum; i++) {
        QBufInfo    rQBuf;
        MUINT32     _qi = 0, portId, staticEnqCount = 0;

        #if 0//(STT_ENQ_IMM == 1)
        staticEnqCount = 2 * mBurstQNum; /* two enque prevent 2nd frame drop */
        #else
        staticEnqCount = mpMemInfo[_portIdx].size();
        #endif

        portId = GetOpenedPortInfo(StatisticPipe::_GetPort_Index, i);
        _portIdx = _PortMap(portId);

        if (MFALSE == mpSttIOPipe->sendCommand(EPIPECmd_SET_STT_BA, (MINTPTR)portId, (MINTPTR)&ImgPA_L[_portIdx],
                        (MINTPTR)&ImgHeaderPA_L[_portIdx])) {
            PIPE_ERR("port_%d Config PA fail", portId);
            return MFALSE;
        }
        do {
            list<MUINTPTR>::iterator it, it2;
            for (it = ImgPA_L[_portIdx].begin(), it2 = ImgHeaderPA_L[_portIdx].begin();
                    (it !=  ImgPA_L[_portIdx].end()) && (it2 !=  ImgHeaderPA_L[_portIdx].end()); it++, it2++) {
                PIPE_DBG("Img PA(%" PRIXPTR "), FH PA(%" PRIXPTR ")", *it, *it2);
            }
        } while (0);

        j = 0;
        rQBuf.mvOut.resize(mBurstQNum);

        while (j < staticEnqCount) {
            for (_qi = 0; ((_qi < mBurstQNum) && (j < mpMemInfo[_portIdx].size())); _qi++, j++) {
                rQBuf.mvOut.at(_qi).mPortID.index = _MapPort(_portIdx);
                rQBuf.mvOut.at(_qi).mVa = mpMemInfo[_portIdx].at(j).virtAddr;
                rQBuf.mvOut.at(_qi).mPa = mpMemInfo[_portIdx].at(j).phyAddr;
            }
            if (_qi < mBurstQNum) {
                PIPE_ERR("TG_%d port_%d burstQNum %d, total num %d", mTgInfo, rQBuf.mvOut.at(_qi).mPortID.index,\
                        mBurstQNum, (MUINT32)mpMemInfo[_portIdx].size());
                return MFALSE;
            }
            #if (STT_ENQ_IMM == 1)
            if (MFALSE == enque(rQBuf)) {
                PIPE_ERR("TG_%d port_%d init enq buf fail", mTgInfo, rQBuf.mvOut.at(_qi).mPortID.index);
                return MFALSE;
            }
            #else
            if (MFALSE == immediateEnque(rQBuf)) {
                PIPE_ERR("TG_%d port_%d init enq buf fail", mTgInfo, rQBuf.mvOut.at(_qi).mPortID.index);
                return MFALSE;
            }
            #endif
        }
    }
    PIPE_DBG("- : end of configPipe_bh");

    return ret;
}

void * RunningThread(void *arg)
{
    int ret = 0;
    StatisticPipe *_this = (StatisticPipe*)(arg);
    {
        int const policy    = SCHED_OTHER;
        int const priority  = NICE_CAMERA_LOMO;
        struct sched_param sched_p;

        ::sched_getparam(0, &sched_p);
        if (policy == SCHED_OTHER) {
            setThreadPriority(policy, priority);
        }
        else {   //  Note: "priority" is real-time priority.
            sched_p.sched_priority = priority;
            ::sched_setscheduler(0, policy, &sched_p);
        }

        PIPE_DBG("###########[RunningThread]policy(%d), priority(%d)", policy, priority);
    }
    ret = _this->configPipe_bh();
    PIPE_DBG("############size of mvInPorts = %d############", (MUINT32)_this->mvInPorts->mStatPortInfo.size());

    if(ret)
       pthread_exit((void*)"RunningThread is finished without err");
    else
       pthread_exit((void*)"RunningThread is finished with err");
}

MBOOL StatisticPipe::configPipe(QInitStatParam const& vInPorts, MINT32 burstQnum)
{
    MBOOL       ret = MTRUE, cmdSent = 0;
    MUINT32     uni_available = 1, MainImgBQNum = 0, totalBufNum = 0, _portNum = 0;
    MUINT32     i = 0, j = 0, _portIdx = 0, imgW = 0, imgH = 0, dma_size = 0;
    MUINTPTR    imgHdrAllocOfst = 0;
    NSImageio::NSIspio::ICamIOPipe::E_INPUT eTgInput;
    vector<PortInfo const*>         vCamIOOutPorts, vCamIOInPorts;
    NSImageio::NSIspio::PortInfo    aao, afo, flko, pdo, pso, *_port = NULL, tgi;
    list<MUINTPTR>                  ImgPA_L[MaxStatPortIdx];
    list<MUINTPTR>                  ImgHeaderPA_L[MaxStatPortIdx];
    platSensorsInfo                 SenInfo;
    _sensorInfo_t                   *pSInfo = NULL;
    CAMIO_Func func;
#if (PTHREAD_CTRL_M_STT == 1)
    pthread_attr_t  attr_c;
    MUINT32 r;
#endif
    func.Raw = 0;
    (void)burstQnum;

    PIPE_INF("+ SenIdx[%d]", mpSensorIdx);

    if (this->FSM_CHECK(op_cfg, __FUNCTION__) == MFALSE) {
        return MFALSE;
    }

     /*1. Clone input data*/
     mvInPorts->mStatPortInfo = vInPorts.mStatPortInfo;
     PIPE_DBG("############size of mvInPorts@(%p) of size(%d)############", mvInPorts, (MUINT32)mvInPorts->mStatPortInfo.size());

    if (strcmp(this->mUserName[0], "iopipeUseTM") == 0) {
        SenInfo.mSList = TS_FakeSensorList::getTestModel();
    }
    else {
        SenInfo.mSList = IHalSensorList::get();
    }
    for (i = 0; i < SenInfo.mSList->queryNumberOfSensors(); i++) {
        pSInfo = &SenInfo.mSenInfo[i];

        pSInfo->mIdx       = i;
        pSInfo->mTypeforMw = SenInfo.mSList->queryType(i);
        pSInfo->mDevId     = SenInfo.mSList->querySensorDevIdx(i);
        SenInfo.mSList->querySensorStaticInfo(pSInfo->mDevId, &pSInfo->mSInfo);
    }

     do {
         //mCfgLock.lock();
        if (MFALSE == mpNormalPipe->sendCommand(ENPipeCmd_GET_BURSTQNUM, (MINTPTR)&MainImgBQNum, 0, 0)) {
            PIPE_ERR("TG_%d NPipe sendCmd fail x%x", mTgInfo, ENPipeCmd_GET_BURSTQNUM);
            return MFALSE;
        }
        if (MainImgBQNum < 1) {
            PIPE_ERR("NPipe query BurstQNum %d < 1, SenIdx=%d", MainImgBQNum, mpSensorIdx);
            return MFALSE;
        }
        #if 0 /* reserved for latter chagne */
        mBurstQNum = MainImgBQNum;
        #endif
        totalBufNum = mBurstQNum * MAX_STAT_BUF_NUM;

        switch (mTgInfo) {
        case CAM_TG_1:
            eTgInput = NSImageio::NSIspio::ICamIOPipe::TG_A;
            break;
        case CAM_TG_2:
            eTgInput = NSImageio::NSIspio::ICamIOPipe::TG_B;
            break;
        default:
            PIPE_ERR("TG_%d %s fail", mTgInfo, "unknown idx");
            return MFALSE;
        }

        if (!mpSttIOPipe) {
            mpSttIOPipe = ICamIOPipe::createInstance((MINT8*)mpName, eTgInput, NSImageio::NSIspio::ICamIOPipe::STTIO);
            if (NULL == mpSttIOPipe) {
                PIPE_ERR("TG_%d %s fail", mTgInfo, "create");
                return MFALSE;
            }
        }

        if (MFALSE == mpSttIOPipe->init()) {
            PIPE_ERR("TG_%d %s fail", mTgInfo, "init");
            return MFALSE;
        }

        //configpipe
        ret = mpNormalPipe->sendCommand(ENPipeCmd_GET_UNI_INFO, (MINTPTR)&uni_available, 0, 0);

        for (i = 0; i < vInPorts.mStatPortInfo.size(); i++) {
            _portIdx = _PortMap(vInPorts.mStatPortInfo.at(i).mPortID.index);

            switch (vInPorts.mStatPortInfo.at(i).mPortID.index) {
                case EPortIndex_AAO:
                    _port = &aao;

                    mOpenedPort |= StatisticPipe::_AAO_ENABLE;
                    break;
                case EPortIndex_AFO:
                    _port = &afo;

                    mOpenedPort |= StatisticPipe::_AFO_ENABLE;
                    break;
                case EPortIndex_FLKO:
                    _port = &flko;

                    mOpenedPort |= StatisticPipe::_FLKO_ENABLE;
                    break;
                case EPortIndex_PDO:
                    if(vInPorts.mStatPortInfo.at(i).mWidth * vInPorts.mStatPortInfo.at(i).mHeight == 0){
                        PIPE_ERR("PDO H/W can't be 0,PDO open fail\n");
                        _port = NULL;
                    }
                    else{
                        _port = &pdo;
                        pdo.u4ImgWidth = vInPorts.mStatPortInfo.at(i).mWidth;
                        pdo.u4ImgHeight = vInPorts.mStatPortInfo.at(i).mHeight;
                        mOpenedPort |= StatisticPipe::_PDO_ENABLE;

                        func.Bits.Density = vInPorts.mStatPortInfo.at(i).mFunc.Bits.Density;
                    }
                    break;
                case EPortIndex_PSO:
                    _port = &pso;
                    mOpenedPort |= StatisticPipe::_PSO_ENABLE;
                    break;
                default:
                    _port = NULL;
                    PIPE_ERR("TG_%d unknown idx", mTgInfo);
                    return MFALSE;
            }

            if(_port != NULL){
                _port->type  = EPortType_Memory;
                _port->inout = EPortDirection_Out;
                _port->index = vInPorts.mStatPortInfo.at(i).mPortID.index;


                if (uni_available) {
                    vCamIOOutPorts.push_back(_port);
                }
                else {
                    switch (vInPorts.mStatPortInfo.at(i).mPortID.index) {
                    case EPortIndex_FLKO:
                    //case EPortIndex_EISO:
                        /* only allocate buffer for possible dynamic switch */
                        break;;
                    default:
                        vCamIOOutPorts.push_back(_port);
                        break;
                    }
                }
            }

        }


        func.Bits.SUBSAMPLE = MainImgBQNum-1;

        switch (SenInfo.mSenInfo[mpSensorIdx].mSInfo.sensorType) {
        case SENSOR_TYPE_RAW:
            switch(SenInfo.mSenInfo[mpSensorIdx].mSInfo.rawSensorBit) {
                case RAW_SENSOR_8BIT:   tgi.eImgFmt = NSCam::eImgFmt_BAYER8; break;
                case RAW_SENSOR_10BIT:  tgi.eImgFmt = NSCam::eImgFmt_BAYER10; break;
                case RAW_SENSOR_12BIT:  tgi.eImgFmt = NSCam::eImgFmt_BAYER12; break;
                case RAW_SENSOR_14BIT:  tgi.eImgFmt = NSCam::eImgFmt_BAYER14; break;
                default: PIPE_ERR("Err sen raw fmt(%d) err\n", SenInfo.mSenInfo[mpSensorIdx].mSInfo.rawSensorBit); break;
            }
            break;
        case SENSOR_TYPE_YUV:
            switch(SenInfo.mSenInfo[mpSensorIdx].mSInfo.sensorFormatOrder) {
                case SENSOR_FORMAT_ORDER_UYVY: tgi.eImgFmt = NSCam::eImgFmt_UYVY; break;
                case SENSOR_FORMAT_ORDER_VYUY: tgi.eImgFmt = NSCam::eImgFmt_VYUY; break;
                case SENSOR_FORMAT_ORDER_YVYU: tgi.eImgFmt = NSCam::eImgFmt_YVYU; break;
                case SENSOR_FORMAT_ORDER_YUYV: tgi.eImgFmt = NSCam::eImgFmt_YUY2; break;
                default:    PIPE_ERR("Err sen yuv fmt err\n"); break;
            }
            break;
        case SENSOR_TYPE_JPEG:
            tgi.eImgFmt = NSCam::eImgFmt_JPEG; break;
        default:
            PIPE_ERR("Err sen type(%d) err\n", SenInfo.mSenInfo[mpSensorIdx].mSInfo.sensorType);
            break;
        }

        vCamIOInPorts.push_back(&tgi);

        if (MFALSE == mpSttIOPipe->configPipe(vCamIOInPorts, vCamIOOutPorts, func)) {
            PIPE_ERR("TG_%d %s fail", mTgInfo, "config");
            return MFALSE;
        }

        /*2. Create a thread to do the actual works and return control back to the caller.*/
#if (PTHREAD_CTRL_M_STT == 0)
        pthread_create(&m_thread, NULL, RunningThread, this);
#else
    if ((r = pthread_attr_init(&attr_c)) != 0) {
        PIPE_ERR("thread ctrl [%s] fail: %d", "set sttThd attr", r);
        return MFALSE;
    }
    if ((r = pthread_create(&m_thread, &attr_c, RunningThread, this)) != 0) {
        PIPE_ERR("thread ctrl [%s] fail: %d", "sttThd create", r);
        return MFALSE;
    }
    if ((r = pthread_attr_destroy(&attr_c)) != 0) {
        PIPE_ERR("thread ctrl [%s] fail: %d", "destroy sttThd attr", r);
        return MFALSE;
    }
#endif
    } while(0);

    PIPE_DBG("-");

    return ret;
}


/******************************************************************************
 *
 ******************************************************************************/
MINT32 StatisticPipe::attach(const char* UserName)
{
    PIPE_DBG("+");

    if (this->FSM_CHECK(op_cmd, __FUNCTION__) == MFALSE) {
        return -1;
    }

    if (mpNormalPipe) {
        return mpNormalPipe->attach(UserName);
    }
    else {
        PIPE_ERR("instance not create");
        return -1;
    }
}


/******************************************************************************
 *
 ******************************************************************************/
MBOOL StatisticPipe::wait(EPipeSignal eSignal, EPipeSignalClearType eClear, const MINT32 mUserKey,
                            MUINT32 TimeoutMs, SignalTimeInfo *pTime)
{
    PIPE_DBG("+");

    if (this->FSM_CHECK(op_cmd, __FUNCTION__) == MFALSE) {
        return MFALSE;
    }

    if (mpNormalPipe) {
        return mpNormalPipe->wait(eSignal, eClear, mUserKey, TimeoutMs, pTime);
    }
    else {
        PIPE_ERR("instance not create");
        return MFALSE;
    }
}

MBOOL StatisticPipe::signal(EPipeSignal eSignal, const MINT32 mUserKey)
{
    PIPE_DBG("+");

    if (this->FSM_CHECK(op_cmd, __FUNCTION__) == MFALSE) {
        return MFALSE;
    }

    if (mpNormalPipe) {
        return mpNormalPipe->signal(eSignal, mUserKey);
    }
    else {
        PIPE_ERR("instance not create");
        return MFALSE;
    }
}

/******************************************************************************
 *
 ******************************************************************************/
MBOOL StatisticPipe::abortDma(PortID port, char const* szCallerName)
{
    NSImageio::NSIspio::PortID portID;

    PIPE_DBG("+, usr[%s]dma[0x%x]SenIdx[%d]", szCallerName, port.index, mpSensorIdx);

    if (this->FSM_CHECK(op_cmd, __FUNCTION__) == MFALSE) {
        return MFALSE;
    }

    if (mpSttIOPipe) {
        switch (port.index){
        case EPortIndex_AAO:
        case EPortIndex_AFO:
        case EPortIndex_FLKO:
        case EPortIndex_PDO:
        case EPortIndex_PSO:
            break;
        default:
            PIPE_ERR("not support dma(0x%x)user(%s)\n", port.index, szCallerName);
            return MFALSE;
        }

        portID.index = port.index;
        return mpSttIOPipe->abortDma(portID);
    }
    else {
        PIPE_ERR("instance not create, user(%s)", szCallerName);
        return MFALSE;
    }
}

/******************************************************************************
 *
 ******************************************************************************/
MUINT32 StatisticPipe::getIspReg(MUINT32 RegAddr, MUINT32 RegCount, MUINT32 RegData[], MBOOL bPhysical)
{
    PIPE_DBG("+");

    if (this->FSM_CHECK(op_cmd, __FUNCTION__) == MFALSE) {
        return MFALSE;
    }

    if (mpNormalPipe) {
        return mpNormalPipe->getIspReg(RegAddr, RegCount, RegData, bPhysical);
    }
    else {
        PIPE_ERR("instance not create");
        return MFALSE;
    }
}

/******************************************************************************
 *
 ******************************************************************************/
MUINT32 StatisticPipe::getIspReg(IspP1RegIo *pRegs, MUINT32 RegCount, MBOOL bPhysical)
{
    PIPE_DBG("+");

    if (this->FSM_CHECK(op_cmd, __FUNCTION__) == MFALSE) {
        return MFALSE;
    }

    if (mpNormalPipe) {
        return mpNormalPipe->getIspReg(pRegs, RegCount, bPhysical);
    }
    else {
        PIPE_ERR("instance not create");
        return MFALSE;
    }
}

/******************************************************************************
 *
 ******************************************************************************/
MUINT32 StatisticPipe::getIspUniReg(IspP1RegIo *pRegs, MUINT32 RegCount, MBOOL bPhysical)
{
    PIPE_DBG("+");

    if (this->FSM_CHECK(op_cmd, __FUNCTION__) == MFALSE) {
        return MFALSE;
    }

    if (mpNormalPipe) {
        return mpNormalPipe->getIspUniReg(pRegs, RegCount, bPhysical);
    }
    else {
        PIPE_ERR("instance not create");
        return MFALSE;
    }
}

/******************************************************************************
 *
 ******************************************************************************/
MBOOL StatisticPipe::sendCommand(MINT32 cmd, MINTPTR arg1, MINTPTR arg2, MINTPTR arg3)
{
    MBOOL   ret = MTRUE;
    MUINT32 tg, i, _portidx;
    vector<MUINTPTR> * vBuf;
    PortID port;

    if (this->FSM_CHECK(op_cmd, __FUNCTION__) == MFALSE) {
        PIPE_ERR("Command(0x%x) fail, before configPipe(0x%p)", cmd, this);
        return MFALSE;
    }

    switch (cmd) {
    case ESPipeCmd_GET_TG_INDEX:
        //arg1: [Out] (MUINT32*)  CAM_TG_1/CAM_TG_2/...
        ret = mpNormalPipe->sendCommand(ENPipeCmd_GET_TG_INDEX, arg1, arg2, arg3);
        if (MFALSE == ret) {
            goto _EXIT_SEND_CMD;
        }
        break;
    case ESPipeCmd_GET_BURSTQNUM:
        if (arg1) {
            *(MINT32*)arg1 = mBurstQNum;
        }
        else {
            ret = MFALSE;
            goto _EXIT_SEND_CMD;
        }
        break;
    case ESPipeCmd_GET_TG_OUT_SIZE:
        //arg1 : sensor-idx
        //arg2 : width
        //arg3 : height
        ret = mpNormalPipe->sendCommand(ENPipeCmd_GET_TG_OUT_SIZE, arg1, arg2, arg3);
        if (MFALSE == ret) {
            goto _EXIT_SEND_CMD;
        }
        break;
    case ESPipeCmd_SET_TG_INT_LINE:
        //arg1: TG_INT1_LINENO
        ret = mpNormalPipe->sendCommand(ENPipeCmd_SET_TG_INT_LINE, arg1, arg2, arg3);
        if (MFALSE == ret) {
            goto _EXIT_SEND_CMD;
        }
        break;
    case ESPipeCmd_GET_STT_CUR_BUF:
        //arg1: [in] dma port.index
        //arg2: [out] MUINTPTR *: dma buf
        port = (PortID)arg1;
        ret = mpSttIOPipe->sendCommand(EPIPECmd_GET_STT_CUR_BUF, (MUINT32)port.index, arg2, arg3);
        if (MFALSE == ret) {
            goto _EXIT_SEND_CMD;
        }
        _portidx = _PortMap(port.index);
        for (i = 0; i < mpMemInfo[_portidx].size(); i++){
            if(*(MUINTPTR*)arg2 == mpMemInfo[_portidx].at(i).phyAddr){
                *(MUINTPTR*)arg2 = mpMemInfo[_portidx].at(i).virtAddr;
                break;
            }
        }
        if(i == mpMemInfo[_portidx].size()) {
            ret = MFALSE;
            PIPE_ERR("dma(0x%x)can't find PA(0x%" PRIXPTR ") in list\n", port.index, *(MUINTPTR*)arg2);
            goto _EXIT_SEND_CMD;
        }
        break;
    default:
        ret = MFALSE;
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
MUINT32 StatisticPipe::GetOpenedPortInfo(StatisticPipe::ePortInfoCmd cmd, MUINT32 arg1)
{
    MUINT32 _num=0;
    switch (cmd) {
    case StatisticPipe::_GetPort_Opened:
        return (MUINT32)mOpenedPort;
    case StatisticPipe::_GetPort_OpendNum:
        if (mOpenedPort& StatisticPipe::_AAO_ENABLE)
            _num++;
        if (mOpenedPort& StatisticPipe::_AFO_ENABLE)
            _num++;
        if (mOpenedPort& StatisticPipe::_FLKO_ENABLE)
            _num++;
        if (mOpenedPort& StatisticPipe::_PDO_ENABLE)
            _num++;
        if (mOpenedPort& StatisticPipe::_PSO_ENABLE)
            _num++;
        return _num;
    case StatisticPipe::_GetPort_Index:
    {
        MUINT32 idx = 0;
        if (mOpenedPort & StatisticPipe::_AAO_ENABLE) {
            PortIndex[idx] = PORT_AAO.index;
            idx++;
        }
        if (mOpenedPort & StatisticPipe::_AFO_ENABLE) {
            PortIndex[idx] = PORT_AFO.index;
            idx++;
        }
        if (mOpenedPort & StatisticPipe::_FLKO_ENABLE) {
            PortIndex[idx] = PORT_FLKO.index;
            idx++;
        }
        if (mOpenedPort & StatisticPipe::_PDO_ENABLE) {
            PortIndex[idx] = PORT_PDO.index;
            idx++;
        }
        if (mOpenedPort & StatisticPipe::_PSO_ENABLE) {
            PortIndex[idx] = PORT_PSO.index;
            idx++;
        }

        if ((arg1 >= idx) || (arg1 >= MaxStatPortIdx)) {
            PIPE_ERR("Opened port num not enough %d/%d/%d", arg1, idx, MaxStatPortIdx);
            return -1;
        }
        else {
            return PortIndex[arg1];
        }
    }
    default:
        PIPE_ERR("un-supported cmd:0x%x\n",cmd);
        return 0;
    }
}

MUINT32 StatisticPipe::sttStateNotify(CAM_STATE_OP state, MVOID *Obj)
{
    Mutex *pLock = (Mutex *)Obj;

    switch (state) {
    case CAM_ENTER_WAIT:
        pLock->unlock();
        break;
    case CAM_EXIT_WAIT:
        pLock->lock();
        break;
    default:
        PIPE_ERR("Unknown state: %d", state);
        break;
    }

    return 0;
}

MUINT32 StatisticPipe::sttIdleStateHoldCB(CAM_STATE_OP state, MVOID *Obj)
{
    StatisticPipe *_this = (StatisticPipe*)Obj;
    MUINT32 _port = 0;

    switch (state) {
    case CAM_HOLD_IDLE:
        _this->mEnQLock.lock();
        for (_port = 0; _port < MaxStatPortIdx; _port++) {
            _this->mThreadIdleLock[_port].lock();
        }
        break;
    case CAM_RELEASE_IDLE:
        for (_port = 0; _port < MaxStatPortIdx; _port++) {
            _this->mThreadIdleLock[_port].unlock();
        }
        _this->mEnQLock.unlock();
        break;
    default:
        PIPE_ERR("Unknown state: %d", state);
        break;
    }

    PIPE_INF("state %d", state);

    return 0;
}

MUINT32 StatisticPipe::sttSuspendStateCB(CAM_STATE_OP state, MVOID *Obj)
{
    StatisticPipe *_this = (StatisticPipe*)Obj;

    switch (state) {
    case CAM_ENTER_SUSPEND:
        // FSM START -> SUSPEND
        _this->FSM_UPDATE(op_suspend);
        break;
    case CAM_EXIT_SUSPEND:
        // FSM SUSPEND -> START
        _this->FSM_UPDATE(op_start);
        break;
    default:
        break;
    }

    return 0;
}




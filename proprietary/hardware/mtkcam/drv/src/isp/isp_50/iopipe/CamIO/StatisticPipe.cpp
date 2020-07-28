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

#define SPIPE_MEM_NEW(dstPtr,type,size,mMemInfo)\
do {\
    mMemInfo.SPipeAllocMemSum += size; \
    dstPtr = new type;\
} while(0)

#define SPIPE_MEM_DEL(dstPtr,size,mMemInfo)\
do{\
    mMemInfo.SPipeFreedMemSum += size; \
    delete dstPtr;\
    dstPtr = NULL;\
}while(0)


Mutex                           StatisticPipe::SPipeGLock;
StatisticPipe*                  StatisticPipe::pStatisticPipe[EPIPE_Sensor_RSVD] = {NULL, NULL};
StatisticPipe::_SPipeMemInfo_t  StatisticPipe::mMemInfo;

const EPortIndex m_Slot2PortIndex[StatisticPipe::SttPortID_MAX] = {
    EPortIndex_AAO, EPortIndex_AFO, EPortIndex_FLKO, EPortIndex_PDO, EPortIndex_PSO
};

/******************************************************************************
 *
 ******************************************************************************/

//map portId to deque container index
inline MUINT32 _PortMap(MUINT32 const PortIdx)
{
    MUINT32 _idx = 0;

    if (PortIdx == PORT_AAO.index)
        _idx = StatisticPipe::eAAO;
    else if (PortIdx == PORT_AFO.index)
        _idx = StatisticPipe::eAFO;
    else if(PortIdx == PORT_FLKO.index)
        _idx = StatisticPipe::eFLKO;
    else if(PortIdx == PORT_PDO.index)
        _idx = StatisticPipe::ePDO;
    else if(PortIdx == PORT_PSO.index)
        _idx = StatisticPipe::ePSO;
    else PIPE_ERR("error:portidx:0x%x\n",PortIdx);

    return _idx;
}

inline MUINT32 _MapPort(MUINT32 const index)
{
    MUINT32 ret;

    ret = m_Slot2PortIndex[index];

    return ret;
}

inline MUINTPTR _AlignCal(MUINTPTR data, MUINT32 align_num)
{
    MUINT32 ShiftBit = 0;

    switch(align_num) {
        case 16:
            ShiftBit = 4;
            break;
        default:
            PIPE_ERR("unknow align num(%d)", align_num);
            break;
    }

    data = ((data + align_num - 1) >> ShiftBit) << ShiftBit;

    return data;
}

inline MVOID _SPIPE_PASS_FHINFO(
    IMEM_BUF_INFO const src,
    NSImageio::NSIspio::BufInfo &dst)
{
    dst.Frame_Header.bReplace  = MFALSE;
    dst.Frame_Header.u4BufSize[ePlane_1st] = src.size;
    dst.Frame_Header.u4BufVA[ePlane_1st]   = src.virtAddr;
    dst.Frame_Header.u4BufPA[ePlane_1st]   = src.phyAddr;
    dst.Frame_Header.memID[ePlane_1st]     = src.memID;
    dst.Frame_Header.bufSecu[ePlane_1st]   = src.bufSecu;
    dst.Frame_Header.bufCohe[ePlane_1st]   = src.bufCohe;
}

inline MVOID _SPIPE_GET_TGIFMT(
    platSensorsInfo const SenInfo,
    MUINT32 const mpSensorIdx,
    NSImageio::NSIspio::PortInfo &tgi)
{
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
}
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
        SPIPE_MEM_NEW(StatisticPipe::pStatisticPipe[SensorIndex], StatisticPipe(SensorIndex, THIS_NAME), sizeof(StatisticPipe)
            , StatisticPipe::mMemInfo);
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
            mpNormalPipe->sendCommand(ENPipeCmd_SET_ENQ_THRD_CB, (MINTPTR)&notifyObj, 0, 0);

            mpNormalPipe->destroyInstance(THIS_NAME);
            mpNormalPipe = NULL;
        }

        if (mpSttIOPipe) {
            mpSttIOPipe->destroyInstance();
            mpSttIOPipe = NULL;
        }

        if (mpIMem) {
            MUINT32 i, j;
            for (i = 0; i < StatisticPipe::SttPortID_MAX; i++) {
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
        SPIPE_MEM_DEL(pStatisticPipe[idx], sizeof(StatisticPipe), mMemInfo);
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
    , mBufMax(4)
    , mBurstQNum(1)
    , m_FSM(op_unknown)
    , mTgInfo(0)
    , mTotalUserCnt(0)
    , mpSensorIdx(SensorIdx)
    , mOpenedPort(_DMAO_NONE)
    , mpIMem(NULL)
    , m_thread(0)
    , mvInPorts(NULL)
{
    DBG_LOG_CONFIG(imageio, iospipe);

    memset((void*)mpDeqContainer, 0, sizeof(mpDeqContainer));
    memset((void*)PortIndex, 0, sizeof(PortIndex));
    memset((void*)mUserCnt, 0, sizeof(mUserCnt));
    memset((void*)mUserName, 0, sizeof(mUserName));
    memset(&mMemInfo, 0, sizeof(mMemInfo));

    if (NULL == mpNormalPipe) {
        CAM_STATE_NOTIFY    notifyObj = {.fpNotifyState = sttIdleStateHoldCB, .Obj = this};
        CAM_STATE_NOTIFY    notifySuspendObj = {.fpNotifyState = sttSuspendStateCB, .Obj = this};
        CAM_STATE_NOTIFY    notifyEnqThrdObj = {.fpNotifyState = sttEnqueStateCB, .Obj = this};

        mpNormalPipe = createInstance_INormalPipe(SensorIdx, THIS_NAME);

        mpNormalPipe->sendCommand(ENPipeCmd_SET_IDLE_HOLD_CB, (MINTPTR)&notifyObj, 0, 0);
        mpNormalPipe->sendCommand(ENPipeCmd_SET_SUSPEND_STAT_CB, (MINTPTR)&notifySuspendObj, 0, 0);
        mpNormalPipe->sendCommand(ENPipeCmd_SET_ENQ_THRD_CB, (MINTPTR)&notifyEnqThrdObj, 0, 0);
    }
}


inline MBOOL StatisticPipe::FSM_CHECK(E_FSM op, const char *callee)
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
    case op_enque:
        switch(this->m_FSM){
        case op_cfg:
        case op_start:
            break;
        case op_suspend: //for 3a enque while suspend is acceptable
            ret = 2;
            break;
        default:
            ret= 0;
            break;
        }
        break;
    case op_deque:
        switch(this->m_FSM){
        case op_cfg:
        case op_start:
            break;
        case op_suspend: //for 3a can't deque while suspend
            ret = MFALSE;
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

inline MBOOL StatisticPipe::FSM_UPDATE(E_FSM op)
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
    Mutex::Autolock lock(mCfgLock);
    bNonblocking;

    PIPE_INF("SenIdx[%d] locking thread mutex", mpSensorIdx);

    for (_port = 0; _port < StatisticPipe::SttPortID_MAX; _port++) {
        mThreadIdleLock[_port].lock();
    }

    if (this->FSM_CHECK(op_stop, __FUNCTION__) == MFALSE) {
        PIPE_ERR("TG_%d not start yet", mTgInfo);
        ret = MFALSE;
        goto _EXIT_STOP;
    }

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
    for (_port = 0; _port < StatisticPipe::SttPortID_MAX; _port++) {
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

        for (i = 0; i < StatisticPipe::SttPortID_MAX; i++) {
            if (!mpDeqContainer[i]) {
                SPIPE_MEM_NEW(mpDeqContainer[i], QueueMgr<QBufInfo>(), sizeof(QueueMgr<QBufInfo>), mMemInfo);
                mpDeqContainer[i]->init();
            }
        }

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

    for (i = 0; i < StatisticPipe::SttPortID_MAX; i++) {
        if (mpDeqContainer[i])  {
            mpDeqContainer[i]->uninit();
            SPIPE_MEM_DEL(mpDeqContainer[i], sizeof(QueueMgr<QBufInfo>), mMemInfo);
        }

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
    NSImageio::NSIspio::PortID      portID;
    NSImageio::NSIspio::QBufInfo    rQBufInfo;
    IMEM_BUF_INFO imBuf;
    QBufInfo    _qBuf;
    MUINT32     i = 0, _port = 0, memIndex = 0, _bsize = 0, BufDepth = 0;
    MINT32 ret = 0;

    Mutex::Autolock lock(mEnQLock);

    PIPE_DBG("+");

    if (this->FSM_CHECK(op_enque, __FUNCTION__) == MFALSE) {
        return MFALSE;
    }

    portID.index = rQBuf.mvOut.at(0).mPortID.index;
    _port = _PortMap(portID.index);
    _bsize = rQBuf.mvOut.size();

    if ((0 == _bsize) || (mBurstQNum != _bsize)) {
        PIPE_ERR("TG_%d enq buf size ng %d/%d", mTgInfo, _bsize, mBurstQNum);
        return MFALSE;
    }

    if ((this->m_FSM == op_start) || (this->m_FSM == op_suspend)) {
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
                PIPE_ERR("wait for 3a's patch\n");
                //return MFALSE;
                return MTRUE;
            }
        }
    }

    rQBufInfo.vBufInfo.resize(_bsize);
    for (i = 0; i < _bsize; i++) {
        rQBufInfo.vBufInfo[i].u4BufSize[ePlane_1st] = rQBuf.mvOut.at(i).mSize;
        rQBufInfo.vBufInfo[i].u4BufVA[ePlane_1st] = rQBuf.mvOut.at(i).mVa;
        rQBufInfo.vBufInfo[i].u4BufPA[ePlane_1st] = rQBuf.mvOut.at(i).mPa;
        rQBufInfo.vBufInfo[i].memID[ePlane_1st] = rQBuf.mvOut.at(i).mMemID;

        if (!mpImgHeaderMgr[_port].sendCmd(QueueMgr<IMEM_BUF_INFO>::eCmd_at, i, (MUINTPTR)&imBuf)) {
            PIPE_ERR("TG_%d port_%d out of imgheader buf", mTgInfo, portID.index);
            return MFALSE;
        }

        rQBufInfo.vBufInfo[i].Frame_Header.u4BufSize[ePlane_1st] = imBuf.size;
        rQBufInfo.vBufInfo[i].Frame_Header.u4BufVA[ePlane_1st]   = imBuf.virtAddr;
        rQBufInfo.vBufInfo[i].Frame_Header.u4BufPA[ePlane_1st]   = imBuf.phyAddr;
        rQBufInfo.vBufInfo[i].Frame_Header.memID[ePlane_1st]     = imBuf.memID;
    }

    if ((ret = mpSttIOPipe->enqueOutBuf(portID, rQBufInfo)) != 0) {
        if(ret > 0)
            PIPE_WRN("TG_%d port_%d enq:already stopped", mTgInfo, portID.index);
        else
            PIPE_ERR("TG_%d port_%d enq fail", mTgInfo, portID.index);
        return MFALSE;
    }

    for (i = 0; i < _bsize; i++) {
        mpImgHeaderMgr[_port].sendCmd(QueueMgr<IMEM_BUF_INFO>::eCmd_pop, 0, 0);
    }

    if ((this->m_FSM == op_start) || (this->m_FSM == op_suspend)) {
        mpDeqContainer[_port]->sendCmd(QueueMgr<QBufInfo>::eCmd_pop, 0, 0);
    }

    for (i = 0; i < _bsize; i++) {
        PIPE_DBG("SttE:%d: dma:x%x pa(x%" PRIXPTR "/x%" PRIXPTR ") fh_pa(x%" PRIXPTR ")", mpSensorIdx, portID.index,\
                rQBufInfo.vBufInfo[i].u4BufPA[ePlane_1st], rQBuf.mvOut.at(i).mPa, rQBufInfo.vBufInfo[i].Frame_Header.u4BufPA[ePlane_1st]);
    }

    PIPE_DBG("-");

    return MTRUE;
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

    if (this->FSM_CHECK(op_deque, __FUNCTION__) == MFALSE) {
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

        result.mDstSize              = rQTSBufInfo.vBufInfo.at(idx).DstSize;
        result.mMagicNum_hal         = rQTSBufInfo.vBufInfo.at(idx).m_num;
        result.mMagicNum_tuning      = rQTSBufInfo.vBufInfo.at(idx).m_num;
        result.mTimeStamp            = rQTSBufInfo.vBufInfo.at(idx).getTimeStamp_ns();
#ifdef TS_PERF
        result.mTimeStamp_B          = rQTSBufInfo.vBufInfo.at(idx).getTimeStamp_B_ns();
#endif
        result.mRawType              = rQTSBufInfo.vBufInfo.at(idx).raw_type;//for pso, 0:before LSC/1:before OB
        result.eIQlv                 = (E_CamIQLevel)rQTSBufInfo.vBufInfo.at(idx).eIQlv;

        pbuf->mPortID                = portID.index;
        pbuf->mBuffer                = NULL;
        pbuf->mMetaData              = result;
        pbuf->mMemID                 = rQTSBufInfo.vBufInfo.at(idx).memID[ePlane_1st];
        pbuf->mSize                  = rQTSBufInfo.vBufInfo.at(idx).u4BufSize[ePlane_1st]; //TBD or returns IMemSize: mpMemInfo[_port].at(0).size
        pbuf->mVa                    = rQTSBufInfo.vBufInfo.at(idx).u4BufVA[ePlane_1st];
        pbuf->mPa                    = rQTSBufInfo.vBufInfo.at(idx).u4BufPA[ePlane_1st];
        pbuf->mPa_offset             = rQTSBufInfo.vBufInfo.at(idx).u4BufOffset[ePlane_1st];
        pbuf->mStride                = rQTSBufInfo.vBufInfo.at(idx).img_stride;
        pbuf->mMetaData.mPrivateData = (MVOID*)rQTSBufInfo.vBufInfo.at(idx).m_pPrivate;

        PIPE_DBG("SttD:%d: dma:x%x pa(0x%" PRIXPTR ") size(%d) mag(0x%x) stride(%d) type(%x) IQlv(%d)\n", mpSensorIdx,\
                portID.index, pbuf->mPa, pbuf->mSize, result.mMagicNum_hal,pbuf->mStride,result.mRawType, (MUINT32)result.eIQlv);

        imBuf           = mImgHeaderPool;
        imBuf.virtAddr  = rQTSBufInfo.vBufInfo.at(idx).Frame_Header.u4BufVA[ePlane_1st];
        imBuf.phyAddr   = rQTSBufInfo.vBufInfo.at(idx).Frame_Header.u4BufPA[ePlane_1st];
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
                    mpIMem->cacheSyncbyRange(IMEM_CACHECTRL_ENUM_INVALID_ALL, &(mpMemInfo[_port].at(buf)));
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
    }
    mpDeqContainer[_port]->sendCmd(QueueMgr<QBufInfo>::eCmd_push, (MUINTPTR)&rQBuf, 0);

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
            strlcpy(mUserName[i], szUserName, strlen(szUserName)+1);
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
    list<MUINTPTR>                  ImgPA_L[StatisticPipe::SttPortID_MAX];
    list<MUINTPTR>                  ImgHeaderPA_L[StatisticPipe::SttPortID_MAX];
    PIPE_DBG("+ :start of configPipe_bh");

    do {

        if ((this->mpSttIOPipe == NULL) || (mvInPorts == NULL)) {
            PIPE_ERR("fatal error wrong obj ptr, potential corruption: %p_%p", this->mpSttIOPipe, mvInPorts);
            return MFALSE;
        }

        totalBufNum = mBurstQNum * mBufMax;

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
            mImgHeaderPool.size += (headersize * totalBufNum);
            mImgHeaderPool.size += (totalBufNum * 15); //allocate extra memory for 16 bytes alignment
        }

        if (MFALSE == mpNormalPipe->sendCommand(ENPipeCmd_SET_MEM_CTRL, 0, (MINTPTR)&mImgHeaderPool, 0)) {
            return MFALSE;
        }

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
                    case eCAM_NORMAL:
                        mpSttIOPipe->sendCommand(EPIPECmd_GET_PDO_CONS,(MINTPTR)&dma_size,(MINTPTR)mvInPorts->mStatPortInfo.at(i).mWidth,(MINTPTR)mvInPorts->mStatPortInfo.at(i).mHeight);
                        break;
                    case eCAM_DUAL_PIX:
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

            MUINTPTR virtBA = _AlignCal(mImgHeaderPool.virtAddr, 16);
            MUINTPTR phytBA = _AlignCal(mImgHeaderPool.phyAddr, 16);

            for (j = 0; j < totalBufNum; j++) {
                IMEM_BUF_INFO   *pBuf = NULL;
                IMEM_BUF_INFO   iHeader;

                imgHdrAllocOfst     = _AlignCal(imgHdrAllocOfst, 16); //16 bytes alignment
                iHeader.size        = headersize;
                iHeader.memID       = mImgHeaderPool.memID;
                iHeader.virtAddr    = virtBA + imgHdrAllocOfst;
                iHeader.phyAddr     = phytBA + imgHdrAllocOfst;
                iHeader.bufSecu     = mImgHeaderPool.bufSecu;
                iHeader.bufCohe     = mImgHeaderPool.bufCohe;
                iHeader.useNoncache = mImgHeaderPool.useNoncache;
                imgHdrAllocOfst    += headersize;

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

        staticEnqCount = mpMemInfo[_portIdx].size();

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
                rQBuf.mvOut.at(_qi).mSize = mpMemInfo[_portIdx].at(j).size;
                rQBuf.mvOut.at(_qi).mMemID = mpMemInfo[_portIdx].at(j).memID;
            }
            if (_qi < mBurstQNum) {
                PIPE_ERR("TG_%d port_%d burstQNum %d, total num %d", mTgInfo, rQBuf.mvOut.at(_qi).mPortID.index,\
                        mBurstQNum, (MUINT32)mpMemInfo[_portIdx].size());
                return MFALSE;
            }
            if (MFALSE == enque(rQBuf)) {
                PIPE_ERR("TG_%d port_%d init enq buf fail", mTgInfo, rQBuf.mvOut.at(_qi).mPortID.index);
                return MFALSE;
            }
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
    vector<PortInfo const*>         vCamIOOutPorts, vCamIOInPorts;
    NSImageio::NSIspio::PortInfo    aao, afo, flko, pdo, pso, *_port = NULL, tgi;
    list<MUINTPTR>                  ImgPA_L[StatisticPipe::SttPortID_MAX];
    list<MUINTPTR>                  ImgHeaderPA_L[StatisticPipe::SttPortID_MAX];
    platSensorsInfo                 SenInfo;
    _sensorInfo_t                   *pSInfo = NULL;
    E_INPUT                         TG_input = TG_CAM_MAX;
    CAMIO_Func func;
#if (PTHREAD_CTRL_M_STT == 1)
    pthread_attr_t  attr_c;
    MUINT32 r;
#endif
    func.Raw = 0;
    (void)burstQnum;

    if(mpNormalPipe){
        if (MFALSE == mpNormalPipe->sendCommand(ENPipeCmd_GET_TG_INDEX, (MINTPTR)&mTgInfo, 0, 0)) {
            PIPE_ERR("sendCmd fail x%x", ENPipeCmd_GET_TG_INDEX);
            return MFALSE;
        }

        switch(mTgInfo){
            case CAM_TG_1:
                TG_input = TG_A;
                break;
            case CAM_TG_2:
                TG_input = TG_B;
                break;
            default:
                PIPE_ERR("TG index is not supported.");
                break;
        }
    }

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
        totalBufNum = mBurstQNum * mBufMax;

        if (!mpSttIOPipe) {
            mpSttIOPipe = ICamIOPipe::createInstance((MINT8*)mpName, TG_input, NSImageio::NSIspio::ICamIOPipe::STTIO);
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
        func.Bits.RMB_SEL = vInPorts.m_StatFunc.Bits.RMB_SEL;

        _SPIPE_GET_TGIFMT(SenInfo, mpSensorIdx, tgi);

        vCamIOInPorts.push_back(&tgi);

        if (MFALSE == mpSttIOPipe->configPipe(vCamIOInPorts, vCamIOOutPorts, &func)) {
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
MBOOL StatisticPipe::suspend()
{
    MUINT32 BufDepth = 0, u4TimeoutMs = 0, PortNum = 0, idx = 0;
    MUINT32 BufNum = 0,BufIdx = 0, BurstQIdx = 0, PortIdx = 0;
    NSImageio::NSIspio::PortID portID;
    CAM_STATE_NOTIFY notifyObj = {.fpNotifyState = sttStateNotify};
    NSImageio::NSIspio::QTimeStampBufInfo   rQTSBufInfo;
    E_BUF_STATUS buf_status;
    IMEM_BUF_INFO   imBuf;
    QBufInfo rQBuf;
    MBOOL rst = MTRUE;

    PortNum = GetOpenedPortInfo(StatisticPipe::_GetPort_OpendNum);
    for(PortIdx = 0; PortIdx < PortNum; PortIdx++) {
        MUINT32 _portIdx = 0;

        portID.index = GetOpenedPortInfo(StatisticPipe::_GetPort_Index, PortIdx);
        _portIdx = _PortMap(portID.index);

        mThreadIdleLock[_portIdx].lock();
    }

    for(PortIdx = 0; PortIdx < PortNum; PortIdx++) {
        MUINT32 _portIdx = 0;

        portID.index = GetOpenedPortInfo(StatisticPipe::_GetPort_Index, PortIdx);
        _portIdx = _PortMap(portID.index);

        mpDeqContainer[_portIdx]->sendCmd(QueueMgr<QBufInfo>::eCmd_getsize, (MUINTPTR)&BufDepth, 0);

        BufNum = mBufMax - BufDepth;

        for(BufIdx = 0; BufIdx < BufNum; BufIdx++) {
            notifyObj.Obj = (MVOID*)&(mThreadIdleLock[_portIdx]);

            buf_status = mpSttIOPipe->dequeOutBuf(portID, rQTSBufInfo, u4TimeoutMs, &notifyObj);
            rQBuf.mvOut.resize(0);

            if (rQTSBufInfo.vBufInfo.size() != mBurstQNum) {
                PIPE_ERR("TG_%d port_%d deque buf num unsync, %d/%d", mTgInfo,\
                        portID.index, (MUINT32)rQTSBufInfo.vBufInfo.size(), mBurstQNum);
                rst = MFALSE;
                goto EXIT;
            }

            switch(buf_status){
                case eBuf_suspending:
                    rQBuf.mvOut.resize(mBurstQNum);

                    for (BurstQIdx = 0; BurstQIdx < mBurstQNum; BurstQIdx++) {

                        BufInfo* pbuf = &(rQBuf.mvOut.at(idx));
                        ResultMetadata result;

                        result.mDstSize              = rQTSBufInfo.vBufInfo.at(idx).DstSize;
                        result.mMagicNum_hal         = rQTSBufInfo.vBufInfo.at(idx).m_num;
                        result.mMagicNum_tuning      = rQTSBufInfo.vBufInfo.at(idx).m_num;
                        result.mTimeStamp            = rQTSBufInfo.vBufInfo.at(idx).getTimeStamp_ns();
                        result.mTimeStamp_B          = rQTSBufInfo.vBufInfo.at(idx).getTimeStamp_B_ns();
                        //result.mRawType              = rQTSBufInfo.vBufInfo.at(idx).raw_type;//for pso, 0:before LSC/1:before OB //LafiteEP_TODO: PS/PSO removed
                        result.eIQlv                 = (E_CamIQLevel)rQTSBufInfo.vBufInfo.at(idx).eIQlv;

                        pbuf->mPortID                = portID.index;
                        pbuf->mBuffer                = NULL;
                        pbuf->mMetaData              = result;
                        pbuf->mSize                  = rQTSBufInfo.vBufInfo.at(idx).u4BufSize[ePlane_1st]; //TBD or returns IMemSize: mpMemInfo[_port].at(0).size
                        pbuf->mVa                    = rQTSBufInfo.vBufInfo.at(idx).u4BufVA[ePlane_1st];
                        pbuf->mPa                    = rQTSBufInfo.vBufInfo.at(idx).u4BufPA[ePlane_1st];
                        pbuf->mPa_offset             = rQTSBufInfo.vBufInfo.at(idx).u4BufOffset[ePlane_1st];
                        pbuf->mStride                = rQTSBufInfo.vBufInfo.at(idx).img_stride;
                        pbuf->mMetaData.mPrivateData = (MVOID*)rQTSBufInfo.vBufInfo.at(idx).m_pPrivate;

                        PIPE_DBG("SttD:%d: dma:x%x pa(0x%" PRIXPTR ") size(%d) mag(0x%x) stride(%d) type(%x) IQlv(%d)\n", mpSensorIdx,\
                                portID.index, pbuf->mPa, pbuf->mSize, result.mMagicNum_hal,pbuf->mStride,result.mRawType, (MUINT32)result.eIQlv);

                        imBuf           = mImgHeaderPool;
                        imBuf.virtAddr  = rQTSBufInfo.vBufInfo.at(BurstQIdx).Frame_Header.u4BufVA[ePlane_1st];
                        imBuf.phyAddr   = rQTSBufInfo.vBufInfo.at(BurstQIdx).Frame_Header.u4BufPA[ePlane_1st];
                        this->mpSttIOPipe->sendCommand(EPIPECmd_GET_HEADER_SIZE,(MINTPTR)portID.index,(MINTPTR)&imBuf.size,0);
                        mpImgHeaderMgr[_portIdx].sendCmd(QueueMgr<IMEM_BUF_INFO>::eCmd_push, (MUINTPTR)&imBuf, 0);
                    }
                    mpDeqContainer[_portIdx]->sendCmd(QueueMgr<QBufInfo>::eCmd_push, (MUINTPTR)&rQBuf, 0);
                    break;
                default:
                    PIPE_ERR("dequeOutBuf status error(%d)", buf_status);
                    rst = MFALSE;
                    goto EXIT;
                    break;
            }
        }

    }
    //
    mpSttIOPipe->suspend(ICamIOPipe::SW_SUSPEND);
EXIT:
    //
    for(PortIdx = 0; PortIdx < PortNum; PortIdx++) {
        MUINT32 _portIdx = 0;
        portID.index = GetOpenedPortInfo(StatisticPipe::_GetPort_Index, PortIdx);
        _portIdx = _PortMap(portID.index);
        mThreadIdleLock[_portIdx].unlock();
    }
    return rst;
}
MBOOL StatisticPipe::resume()
{
    MUINT32 PortIdx = 0,  BufIdx = 0, BurstQIdx = 0;
    MUINT32 PortNum = 0,  DeqDepth = 0;
    MUINT32 _PortIdx = 0;
    NSImageio::NSIspio::PortID portID;
    QBufInfo    _qBuf;
    QBufInfo    rQBuf[mBufMax];
    IMEM_BUF_INFO rQHdrRem;

    PortNum = GetOpenedPortInfo(StatisticPipe::_GetPort_OpendNum);


    mpSttIOPipe->resume(ICamIOPipe::SW_SUSPEND);//reset CQ, start from 1st BA


    for (PortIdx = 0; PortIdx < PortNum; PortIdx++) {
        portID.index = GetOpenedPortInfo(StatisticPipe::_GetPort_Index, PortIdx);
        _PortIdx = _PortMap(portID.index);

        //reorder que containter to init order. must be!due to cmdq is also back to init address
        mpDeqContainer[_PortIdx]->sendCmd(QueueMgr<QBufInfo>::eCmd_getsize, (MUINTPTR)&DeqDepth, 0);
        for(BufIdx = 0; BufIdx < DeqDepth; BufIdx++){
            mpDeqContainer[_PortIdx]->sendCmd(QueueMgr<QBufInfo>::eCmd_at, 0, (MUINTPTR)&_qBuf);

            if(_qBuf.mvOut.at(0).mPa != mpMemInfo[_PortIdx].at(0).phyAddr){
                mpDeqContainer[_PortIdx]->sendCmd(QueueMgr<QBufInfo>::eCmd_pop, 0, 0);
                mpDeqContainer[_PortIdx]->sendCmd(QueueMgr<QBufInfo>::eCmd_push, (MUINTPTR)&_qBuf, 0);


                mpImgHeaderMgr[_PortIdx].sendCmd(QueueMgr<IMEM_BUF_INFO>::eCmd_front, (MUINTPTR)&rQHdrRem, 0);
                mpImgHeaderMgr[_PortIdx].sendCmd(QueueMgr<IMEM_BUF_INFO>::eCmd_pop, 0, 0);
                mpImgHeaderMgr[_PortIdx].sendCmd(QueueMgr<IMEM_BUF_INFO>::eCmd_push, (MUINTPTR)&rQHdrRem, 0);
            }
        }

        for(BufIdx = 0; BufIdx < DeqDepth; BufIdx++) {
            rQBuf[BufIdx].mvOut.resize(mBurstQNum);
            // always enque qBuf from mpDeqContainer:0, since enque function will pop each time
            mpDeqContainer[_PortIdx]->sendCmd(QueueMgr<QBufInfo>::eCmd_at, 0, (MUINTPTR)&_qBuf);
            for(BurstQIdx = 0; BurstQIdx < mBurstQNum; BurstQIdx++){
                rQBuf[BufIdx].mvOut.at(BurstQIdx).mPortID.index = portID.index;
                rQBuf[BufIdx].mvOut.at(BurstQIdx).mVa = _qBuf.mvOut.at(BurstQIdx).mVa;
                rQBuf[BufIdx].mvOut.at(BurstQIdx).mPa = _qBuf.mvOut.at(BurstQIdx).mPa;
            }
            if (MFALSE == enque(rQBuf[BufIdx])) {
                PIPE_ERR("TG_%d port_%d init enq buf fail", mTgInfo, rQBuf[BufIdx].mvOut.at(BurstQIdx).mPortID.index);
                return MFALSE;
            }
        }
    }

    return MTRUE;
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

        if ((arg1 >= idx) || (arg1 >= StatisticPipe::SttPortID_MAX)) {
            PIPE_ERR("Opened port num not enough %d/%d/%d", arg1, idx, StatisticPipe::SttPortID_MAX);
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
    MUINT32 ret = (MUINT32) NOTIFY_PASS;

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

    return ret;
}

MUINT32 StatisticPipe::sttIdleStateHoldCB(CAM_STATE_OP state, MVOID *Obj)
{
    StatisticPipe *_this = (StatisticPipe*)Obj;
    MUINT32 _port = 0;
    MUINT32 ret = (MUINT32)NOTIFY_PASS;

    switch (state) {
    case CAM_HOLD_IDLE:
        _this->mEnQLock.lock();
        for (_port = 0; _port < StatisticPipe::SttPortID_MAX; _port++) {
            _this->mThreadIdleLock[_port].lock();
        }
        break;
    case CAM_RELEASE_IDLE:
        for (_port = 0; _port < StatisticPipe::SttPortID_MAX; _port++) {
            _this->mThreadIdleLock[_port].unlock();
        }
        _this->mEnQLock.unlock();
        break;
    default:
        PIPE_ERR("Unknown state: %d", state);
        break;
    }

    PIPE_INF("state %d", state);

    return ret;
}

MUINT32 StatisticPipe::sttSuspendStateCB(CAM_STATE_OP state, MVOID *Obj)
{
    StatisticPipe *_this = (StatisticPipe*)Obj;
    MUINT32 ret = (MUINT32)NOTIFY_PASS;

    switch (state) {
    case CAM_ENTER_SUSPEND:
        // FSM START -> SUSPEND
        _this->FSM_UPDATE(op_suspend);
        if(_this->suspend() == MFALSE){
            PIPE_INF("stt suspend  fail");
            ret = (MUINT32)NOTIFY_FAIL;
        }
        break;
    case CAM_EXIT_SUSPEND:
        if(_this->resume() == MFALSE){
            PIPE_INF("stt resume  fail");
            ret = (MUINT32)NOTIFY_FAIL;
        }
        // FSM SUSPEND -> START
        _this->FSM_UPDATE(op_start);
        break;
    default:
        break;
    }

    return ret;
}

MUINT32 StatisticPipe::sttEnqueStateCB(CAM_STATE_OP state, MVOID *Obj)
{
    StatisticPipe *_this = (StatisticPipe*)Obj;
    MUINT32 ret = (MUINT32)NOTIFY_PASS;

    switch (state) {
    case CAM_ENTER_ENQ:
        _this->mCfgLock.lock();
        break;
    case CAM_EXIT_ENQ:
        _this->mCfgLock.unlock();
        break;
    default:
        break;
    }

    return ret;
}



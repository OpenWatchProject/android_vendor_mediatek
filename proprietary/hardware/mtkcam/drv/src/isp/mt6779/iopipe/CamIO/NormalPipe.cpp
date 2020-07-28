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
#define LOG_TAG     "NormalPipe"

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

#define THIS_NAME           "NormalPipe"

using namespace NSCam;
using namespace NSIoPipe;
using namespace NSCamIOPipe;
using namespace NSImageio;
using namespace NSIspio;

DECLARE_DBG_LOG_VARIABLE(iopipe);
#undef PIPE_VRB
#undef PIPE_DBG
#undef PIPE_INF
#undef PIPE_WRN
#undef PIPE_ERR
#undef PIPE_AST
#define PIPE_VRB(fmt, arg...)        do { if (iopipe_DbgLogEnable_VERBOSE) { BASE_LOG_VRB(fmt, ##arg); } } while(0)
#define PIPE_DBG(fmt, arg...)        do { if (iopipe_DbgLogEnable_DEBUG  ) { BASE_LOG_DBG(fmt, ##arg); } } while(0)
#define PIPE_INF(fmt, arg...)        do { if (iopipe_DbgLogEnable_INFO   ) { BASE_LOG_INF(fmt, ##arg); } } while(0)
#define PIPE_WRN(fmt, arg...)        do { if (iopipe_DbgLogEnable_WARN   ) { BASE_LOG_WRN(fmt, ##arg); } } while(0)
#define PIPE_ERR(fmt, arg...)        do { if (iopipe_DbgLogEnable_ERROR  ) { BASE_LOG_ERR(fmt, ##arg); } } while(0)
#define PIPE_AST(cond, fmt, arg...)  do { if (iopipe_DbgLogEnable_ASSERT ) { BASE_LOG_AST(cond, fmt, ##arg); } } while(0)

#include "NormalPipeDef.h"
#include "BufCtrlDef.h"

/******************************************************************************
 *
 ******************************************************************************/

Mutex                       NormalPipe::NPipeGLock;
Mutex                       NormalPipe::NIOGLock;
Mutex                       NormalPipe::NPipeCfgGLock;
Mutex                       NormalPipe::NPipeDbgLock;
Mutex                       NormalPipe::NPipeSwitchDoneGLock;
NormalPipe*                 NormalPipe::pNormalPipe[EPIPE_Sensor_RSVD] = {NULL, NULL, NULL, NULL, NULL, NULL};
platSensorsInfo*            NormalPipe::pAllSensorInfo = NULL;
MUINT32                     NormalPipe::mEnablePath = 0;
MUINT32                     NormalPipe::mUniLinkTG = CAM_TG_NONE; /* Set at 1st config, clear when all cam stopped */
MUINT32                     NormalPipe::mFirstConfgCam = CAM_TG_NONE;
NPIPE_PATH_E                NormalPipe::m_occupyingCtlPath = NPIPE_CAM_NONE;
E_CamHwPathFSM              NormalPipe::m_pathCfg_fsm = eCamHwPathFsm_Applied;
E_CamHwPathCfg              NormalPipe::m_pathCfg_TGnum = eCamHwPathCfg_One_TG;
QueueMgr<E_CamHwPathCfg>    NormalPipe::m_pathCfgQueue;
NormalPipe::_MemInfo_t      NormalPipe::mMemInfo;

Mutex                       NpipeIQContol::NPipeIQGlock;
Mutex                       NpipeIQContol::NPipeIQCmdGlock;
NpipeIQContol::IQData_t     NpipeIQContol::sIQData;

Mutex                       BufCtrlCenter::BfcNGlock;
BufCtrlCenter::NLinkInfo    BufCtrlCenter::mNLinkInfo[IQSenSum];
BufCtrlCenter::BufCtrlFSM   BufCtrlCenter::BfcFSM[IQSenSum];
Mutex                       BufCtrlCenter::BfcDeqNPipelock[IQSenSum];
Mutex                       BufCtrlCenter::BfcFSMlock;
MUINT32                     BufCtrlCenter::mBfcNSwitchCnt;
MUINT32                     BufCtrlCenter::mBfcUsrCnt = 0;
MBOOL                       BufCtrlCenter::bSwitch;
P1_TUNING_NOTIFY*           BufCtrlCenter::IQNotify;

#if (CONFIG_IOPIPE_PROFILING == 1)
MUINT32                     IOPipeTrace::m_traceEnable = MFALSE;
MUINT32                     IOPipeTrace::m_metPrfEnable = MFALSE;
MUINT32                     IOPipeTrace::m_userCount = 0;
Mutex                       IOPipeTrace::m_traceLock;
#endif

/******************************************************************************
 *
 ******************************************************************************/
NormalPipe::NormalPipe(
    MUINT32                         _SensorIdx,
    char const*                     _szCallerName
    )
    : mpHalSensor(NULL)
    , mBurstQNum(0)
    , mSVRawBurstNum(0)
    , mRawType(0)
    , mpCamIOPipe(NULL)
    , mpCamSVIOPipe(NULL)
    #ifdef USE_IMAGEBUF_HEAP
    , mpFrameMgr(NULL)
    #endif
    , mpName(_szCallerName)
    , mTotalUserCnt(0)
    , mpSensorIdx(_SensorIdx)
    , mSenPixMode(ePixMode_NONE)
    , m_dataPath(eCamDataPath_TGDL)
    , m_FSM(op_unknown)
    , mInitSettingApplied(MFALSE)
    , mDynSwtRawType(MTRUE)
    , mPureRaw(MFALSE)
    , m_bN3DEn(MFALSE)
    , mResetCount(0)
    , mOpenedPort(0)
    , mOpenedPortNum(0)
    , mPrvEnqSOFIdx(0xFFFFFFFF)
    , mReqDepthPreStart(0)
    , m_b1stEnqLoopDone(MFALSE)
    , mpNPipeThread(NULL)
    , m_DropCB(NULL)
    , m_returnCookie(NULL)
    , m_SofCB(NULL)
    , m_SofCBCookie(NULL)
    , mpIMem(NULL)
    , mpSecMgr(NULL)
{
    DBG_LOG_CONFIG(imageio, iopipe);

    for (MUINT32 i = 0; i < ePort_max; i++) {
        PortImgFormat[i] = eImgFmt_UNKNOWN;
    }

    memset((void*)mpEnqueRequest, 0, sizeof(mpEnqueRequest));
    memset((void*)mpEnqueReserved, 0, sizeof(mpEnqueReserved));
    memset((void*)mpDeQueue, 0, sizeof(mpDeQueue));
    memset((void*)mpVirPortPA, 0, sizeof(mpVirPortPA));
    memset((void*)PortIndex, 0, sizeof(PortIndex));
    memset((void*)mUserCnt, 0, sizeof(mUserCnt));
    memset((void*)mUserName, 0, sizeof(mUserName));
    memset((void*)mImgHdrIdx, 0, sizeof(mImgHdrIdx));
    memset((void*)mImgHdrCnt, 0, sizeof(mImgHdrCnt));
    memset(&mMemInfo, 0, sizeof(mMemInfo));
}

static const NSImageio::NSIspio::EPortIndex m_Slot2PortIndex[] = {
    EPortIndex_RRZO, EPortIndex_IMGO, EPortIndex_YUVO, EPortIndex_CRZO, EPortIndex_CRZO_R2, EPortIndex_EISO, EPortIndex_LCSO, EPortIndex_RSSO, EPortIndex_CAMSV_IMGO, EPortIndex_RAWI
};

/******************************************************************************
 *
 ******************************************************************************/
extern "C"
INormalPipe* createInstance_INormalPipe(MUINT32 SensorIndex, char const* szCallerName)
{
    NormalPipe* pNPipe = NULL;

    PIPE_DBG("+");

    NormalPipe::NPipeGLock.lock();

    if (NormalPipe::pAllSensorInfo == NULL) {
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

        NPIPE_MEM_NEW(NormalPipe::pAllSensorInfo, platSensorsInfo, sizeof(platSensorsInfo), NormalPipe::mMemInfo);
        PIPE_INF("N:%d,%s,pAllSensorInfo=0x%p", SensorIndex, szCallerName, NormalPipe::pAllSensorInfo);

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

        NormalPipe::pAllSensorInfo->mSList          = mSList;
        NormalPipe::pAllSensorInfo->mExistedSCnt    = SCnt;
        NormalPipe::pAllSensorInfo->mUserCnt        = 0;
        if ((SCnt > IOPIPE_MAX_SENSOR_CNT) || (SCnt == 0)) {
            PIPE_ERR("Not support %d sensors", SCnt);
            NPIPE_MEM_DEL(NormalPipe::pAllSensorInfo, sizeof(platSensorsInfo), NormalPipe::mMemInfo);
            NormalPipe::pAllSensorInfo = NULL;
            NormalPipe::NPipeGLock.unlock();
            return MFALSE;
        }

        for (i = 0; i < SCnt; i++) {
            pSInfo = &NormalPipe::pAllSensorInfo->mSenInfo[i];

            pSInfo->mIdx       = i;
            pSInfo->mTypeforMw = mSList->queryType(i);
            pSInfo->mDevId     = mSList->querySensorDevIdx(i);
            mSList->querySensorStaticInfo(pSInfo->mDevId, &pSInfo->mSInfo);
            PIPE_INF("N:%d,SensorName=%s,Type=%d,DevId=%d", i, mSList->queryDriverName(i),\
                    pSInfo->mTypeforMw, pSInfo->mDevId);
        }

        PIPE_DBG("Attach seninf dump callback");
        SENINF_DBG::m_fp_Sen = NormalPipe::dumpSeninfDebugCB;
    }
    NormalPipe::pAllSensorInfo->mUserCnt++;

    NormalPipe::NPipeGLock.unlock();

    if (SensorIndex >= EPIPE_Sensor_RSVD) {
        PIPE_ERR("InvalidSensorIdx = %d", SensorIndex);
        return MFALSE;
    }

    NormalPipe::NPipeGLock.lock();
    pNPipe = NormalPipe::pNormalPipe[SensorIndex];
    if (NULL == pNPipe) {
        NPIPE_MEM_NEW(NormalPipe::pNormalPipe[SensorIndex], NormalPipe(SensorIndex, THIS_NAME), sizeof(NormalPipe), NormalPipe::mMemInfo);
        pNPipe = NormalPipe::pNormalPipe[SensorIndex];

        PIPE_INF("%s NPipe[%d]=0x%p create", szCallerName, SensorIndex, pNPipe);

        if (pNPipe->mpNPipeThread == NULL){
            pNPipe->mpNPipeThread = NormalPipe_Thread::createInstance(pNPipe);
            if (NULL == pNPipe->mpNPipeThread){
                PIPE_ERR("error: Thread:createinstance fail");
                return NULL;
            }
        }
    }

    pNPipe->addUser(szCallerName); //pNPipe->mUserCnt++;

    NormalPipe::NPipeGLock.unlock();

    PIPE_INF("- NPipe[%d](0x%p) mUserCnt(%d) userName(%s)", SensorIndex, pNPipe, pNPipe->mTotalUserCnt, szCallerName);

    return pNPipe;
}


/******************************************************************************
 *
 ******************************************************************************/

MVOID NormalPipe::dumpSeninfDebugCB(MUINT32 ispModule, MUINT32 tgStat)
{
#define NPIPE_DBG_USER      "NPipeDbg"
    MUINT32 idx = 0;
    MUINT32 camTg = CAM_TG_NONE;
    INormalPipe *pNPipeObj = NULL;

    NormalPipe::NPipeDbgLock.lock();

    switch (ispModule) {
    case CAM_A:
        camTg = CAM_TG_1;
        break;
    case CAM_B:
        camTg = CAM_TG_2;
        break;
    case CAM_C:
        camTg = CAM_TG_3;
        break;
    default:
        PIPE_ERR("ERROR: unknow isp hw module : %d, dump all", ispModule);
        NormalPipe::NPipeDbgLock.unlock();

        dumpSeninfDebugCB(CAM_A, tgStat);
        dumpSeninfDebugCB(CAM_B, tgStat);
        return;
    }

    for (idx = 0; idx < EPIPE_Sensor_RSVD; idx++) {
        if (_NORMALPIPE_GET_TG_IDX(idx) == camTg) {
            break;
        }
    }
    if (EPIPE_Sensor_RSVD == idx) {
        PIPE_ERR("ERROR: unknow tg index : %d", camTg);
        goto _EXIT_DUMP;
    }

    if (NormalPipe::pNormalPipe[idx]) {
        MINT32 seninfStatus = 0;

        PIPE_WRN("Start dump seninf debug info: sensor_idx:%d sensor_devid:x%x",
                    idx, NormalPipe::pAllSensorInfo->mSenInfo[idx].mDevId);
        seninfStatus = NormalPipe::pNormalPipe[idx]->mpHalSensor->sendCommand(NormalPipe::pAllSensorInfo->mSenInfo[idx].mDevId,
                                                                SENSOR_CMD_DEBUG_P1_DQ_SENINF_STATUS, 0, 0, 0);
        if ((1 == tgStat) || (0 != seninfStatus)) {
            NormalPipe::pNormalPipe[idx]->mResetCount = MAX_P1_RST_COUNT;
        }
    }
    else {
        PIPE_ERR("ERROR: NormalPipe[%d] object not create yet", idx);
        goto _EXIT_DUMP;
    }

_EXIT_DUMP:
    NormalPipe::NPipeDbgLock.unlock();
}

/******************************************************************************
 *
 ******************************************************************************/

MVOID NormalPipe::switchDoneCB(MUINT32 HwModule)
{
    NPIPE_PATH_E CamNum = NPIPE_CAM_NONE;
    MBOOL ret = MTRUE;
    NPIPE_PATH_E e_npipe_sub;

    Mutex::Autolock lock(NPipeSwitchDoneGLock);

    switch((ISP_HW_MODULE)HwModule){
        case CAM_A:
            CamNum = NPIPE_CAM_A;
            break;
        case CAM_B:
            CamNum = NPIPE_CAM_B;
            break;
        case CAM_C:
            CamNum = NPIPE_CAM_C;
            break;
        default:
            PIPE_ERR("index error %d", HwModule);
            break;
    }

    PIPE_INF("switchDoneCB hwModule:%d", HwModule);

    if((BfcFSM[0].mFsm == BufCtrl_switch_done) && (BfcFSM[1].mFsm == BufCtrl_switch_done)) {
        e_npipe_sub = (eCamIQ_MAX == sIQData.IQInfo.lv[NPIPE_CAM_C]) ? NPIPE_CAM_B : NPIPE_CAM_C;
        // reset bfc fsm
        NormalPipe::updateBufCtrlFSM(NormalPipe::BufCtrl_start, NPIPE_CAM_A);
        NormalPipe::updateBufCtrlFSM(NormalPipe::BufCtrl_start, e_npipe_sub);

        // update switch flag
        BfcNGlock.lock();
        mBfcNSwitchCnt = 0;
        BfcNGlock.unlock();

        // switch done notification
        if(IQNotify != NULL) {
            PIPE_INF("IQ Setting done.");
            IQNotify->p1TuningNotify(NULL, NULL);
            IQNotify = NULL;
        }
        else {
            PIPE_ERR("IQNotify is NULL. IQNotify:0x%p", IQNotify);
        }
    }
}

/******************************************************************************
 *
 ******************************************************************************/

MVOID NormalPipe::destroyInstance(char const* szCallerName)
{
    MUINT32 idx = 0;
    BufCtrlCenter::NLinkInfo* plinkInfo;

    PIPE_INF("+ : %s", szCallerName);

    NPipeGLock.lock();
    for (idx = 0; idx < EPIPE_Sensor_RSVD; idx++){
        if (this == pNormalPipe[idx]) {
            break;
        }
    }

    if (EPIPE_Sensor_RSVD == idx) {
        PIPE_ERR("Can't find the entry");
        NPipeGLock.unlock();
        return;
    }

    if ((mTotalUserCnt <= 0)){
        PIPE_ERR("No instance now");
        NPipeGLock.unlock();
        return;
    }

    if (!delUser(szCallerName)) {
        PIPE_ERR("No such user : %s", szCallerName);
        NPipeGLock.unlock();
        return;
    }

    sendcmdBufCtrl(BufCtrlCenter::BUF_CTRL_CMD_GET_CUR_NLINK, (MINTPTR)&mpCamIOPipe, (MINTPTR)&plinkInfo, 0);

    if (mTotalUserCnt == 0)  {
        if (mpHalSensor) {
            mpHalSensor->destroyInstance(THIS_NAME);
            mpHalSensor = NULL;
        }

        if (mpNPipeThread) {
            mpNPipeThread->destroyInstance();
            mpNPipeThread = NULL;
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
            for (MUINT32 PlaneID = 0; PlaneID <= (MUINT32)ePlane_3rd; PlaneID++) {
                if (mImgSVHeaderPool.ImemBufInfo[PlaneID].virtAddr) {
                    if (mpIMem->unmapPhyAddr(&mImgSVHeaderPool.ImemBufInfo[PlaneID])) {
                        PIPE_ERR("imgheader unmap error %" PRIXPTR "", mImgSVHeaderPool.ImemBufInfo[PlaneID].phyAddr);
                    }
                    if (mpIMem->freeVirtBuf(&mImgSVHeaderPool.ImemBufInfo[PlaneID], MFALSE)) {
                        PIPE_ERR("imgheader free error %" PRIXPTR "", mImgSVHeaderPool.ImemBufInfo[PlaneID].virtAddr);
                    }
                    mImgSVHeaderPool.ImemBufInfo[PlaneID].virtAddr = 0;
                    mImgSVHeaderPool.ImemBufInfo[PlaneID].phyAddr = 0;
                }
            }

            if (plinkInfo->mvSttMem.size()) {
                MUINT32 i;

                PIPE_INF("Post free mem");
                for (i = 0; i < plinkInfo->mvSttMem.size(); i++) {
                    if (mpIMem->unmapPhyAddr(&(plinkInfo->mvSttMem.at(i)))) {
                        PIPE_ERR("sttMem unmap error");
                    }
                    if (mpIMem->freeVirtBuf(&(plinkInfo->mvSttMem.at(i)), MFALSE)) {
                        PIPE_ERR("sttMem free error");
                    }
                }
                plinkInfo->mvSttMem.clear();
            }
            mpIMem->uninit();
            mpIMem->destroyInstance();
            mpIMem = NULL;
        }

        //
        this->uninitBufCtrl(mpSensorIdx);

        PIPE_INF("Destroy NPipe[%d]=0x%p", idx, pNormalPipe[idx]);
        NPIPE_MEM_DEL(pNormalPipe[idx], sizeof(NormalPipe), mMemInfo);
    }

    if (NormalPipe::pAllSensorInfo) {
       if (NormalPipe::pAllSensorInfo->mUserCnt > 0) {
           NormalPipe::pAllSensorInfo->mUserCnt--;
       }

       if (NormalPipe::pAllSensorInfo->mUserCnt == 0) {
           PIPE_INF("Destroy pAllSensorInfo. idx=%d", idx);
           NPIPE_MEM_DEL(NormalPipe::pAllSensorInfo, sizeof(platSensorsInfo), mMemInfo);
           PIPE_DBG("Detach seninf dump callback (Alloc:0x%x, Free:0x%x)", mMemInfo.NPipeAllocMemSum, mMemInfo.NPipeFreedMemSum);
           SENINF_DBG::m_fp_Sen = NULL;

           if (IOPipeTrace::destroyTrace() < 0) {
               PIPE_ERR("destroyTrace failed...");
           }
       }

    }

    NPipeGLock.unlock();

    PIPE_DBG("-");
}

/******************************************************************************
 *
 ******************************************************************************/
MBOOL NormalPipe::start()
{
    IOPipeTrace _traceObj("NormalPipe::start", IOPIPE_TRACE_EN_NOMALPIPE);
    Mutex::Autolock lock(mCfgLock);
    NPIPE_PATH_E    cam_path = NPIPE_CAM_MAX;

    MUINT32 _size = 0;

    PIPE_DBG("E:");

    if (this->FSM_CHECK(op_start, __FUNCTION__) == MFALSE) {
        return MFALSE;
    }

    /*
     * Consume 1st element, which alread applied in immediate request
     */
    if (MTRUE == mInitSettingApplied) {
        if (mpNPipeThread->m_ConsumedReqCnt >= mReqDepthPreStart) {
            PIPE_DBG("N:%d static loop done 0(%d)", mpSensorIdx, mpNPipeThread->m_ConsumedReqCnt);
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
            PIPE_ERR("CamIOPipe:%d start fail", mpSensorIdx);
            return MFALSE;
        }
    }

    /* start: CamSvIO after camIOPipe if DCIF mode*/
    if (eCamDataPath_DCIF == this->m_dataPath) {
        if (mpCamSVIOPipe) {
            if (MFALSE == mpCamSVIOPipe->start()) {
                PIPE_ERR("CamSVPipe:%d start fail", mpSensorIdx);
                return MFALSE;
            }
        }
    }

    if (mpNPipeThread->start() == MFALSE) {
        PIPE_ERR("error: NPipeThread start fail\n");
        return MFALSE;
    }

    cam_path = NPIPE_MAP_PATH(mpSensorIdx);
    updateBufCtrlFSM(NormalPipe::BufCtrl_start, cam_path);

    PIPE_DBG("X:");

    this->FSM_UPDATE(op_start);

    return MTRUE;
}


/******************************************************************************
 *
 ******************************************************************************/
MBOOL NormalPipe::stop(MBOOL bNonblocking)
{
    IOPipeTrace _traceObj("NormalPipe::Stop", IOPIPE_TRACE_EN_NOMALPIPE);
    Mutex::Autolock lock(mCfgLock);

    bool ret = MTRUE;

    if (this->FSM_CHECK(op_stop, __FUNCTION__) == MFALSE) {
        return MFALSE;
    }

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

    PIPE_INF("N:%d stop+ %s", mpSensorIdx, (bNonblocking? "Non-blocking": "Blocking"));

    if (bNonblocking) {
        NormalPipe_Thread::E_ThreadGrop thdGrp;

        switch(this->m_FSM) {
        case op_start:
            thdGrp = (NormalPipe_Thread::E_ThreadGrop)(NormalPipe_Thread::eThdGrp_Internal|NormalPipe_Thread::eThdGrp_External);
            break;
        case op_suspend:
            thdGrp = NormalPipe_Thread::eThdGrp_External;
            break;
        default:
            PIPE_ERR("Unknown state: %d\n", this->m_FSM);
            return MFALSE;;
        }

        /* stop isp hw first */
        if (mpCamIOPipe) {
            if (mpNPipeThread == 0) {
                PIPE_ERR("mpNPipeThread is null");
            } else {
                /* hold idle lock */
                mpNPipeThread->idleStateHold(1, thdGrp);
            }

            /* stop camsv if DCIF mode */
            if (eCamDataPath_DCIF == this->m_dataPath) {
                if (mpCamSVIOPipe) {
                    PIPE_TRACE_BEGIN("TAG_CAMSV_STOP");
                    ret = mpCamSVIOPipe->stop(MFALSE);
                    PIPE_TRACE_END("TAG_CAMSV_STOP");
                }
            }

            PIPE_TRACE_BEGIN("TAG_CAM_STOP");
            ret = mpCamIOPipe->stop(MFALSE);
            PIPE_TRACE_END("TAG_CAM_STOP");

            if (mpNPipeThread) {
                this->mpNPipeThread->prestop();

                /* release idle lock */
                mpNPipeThread->idleStateHold(0,
                    (NormalPipe_Thread::E_ThreadGrop)(NormalPipe_Thread::eThdGrp_Internal|NormalPipe_Thread::eThdGrp_External));
            }
        }


        if (mpNPipeThread) {
            if (mpNPipeThread->stop(MTRUE) == MFALSE) {
                PIPE_ERR("error: NPipeThread stop fail");
                return MFALSE;
            }
        }

        //For secure cam disable
        if (mpSecMgr) {
            PIPE_INF("disable secure cam (SenIdx:%d)", (MUINT32)pAllSensorInfo->mSenInfo[mpSensorIdx].mDevId);
            if (MFALSE == this->mpSecMgr->SecMgr_detach((MUINT32)pAllSensorInfo->mSenInfo[mpSensorIdx].mDevId)) {
                PIPE_ERR("Secure CAM distach failure");
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

        if (mpNPipeThread) {
            if (mpNPipeThread->stop() == MFALSE) {
                PIPE_ERR("error: NPipeThread stop fail");
                return MFALSE;
            }
        }

        if (mpCamIOPipe) {
            PIPE_TRACE_BEGIN("TAG_CAM_STOP");
            ret = mpCamIOPipe->stop(MFALSE);
            PIPE_TRACE_END("TAG_CAM_STOP");

            mpCamIOPipe->sendCommand(EPIPECmd_DEALLOC_UNI, 0, 0, 0);

            if (ret) {
                Mutex::Autolock lock(NPipeCfgGLock);

                switch (_NORMALPIPE_GET_TG_IDX(mpSensorIdx)) {
                case CAM_TG_1:
                    mEnablePath &= ~NPIPE_CAM_A;
                    break;
                case CAM_TG_2:
                    mEnablePath &= ~NPIPE_CAM_B;
                    break;
                case CAM_TG_3:
                    mEnablePath &= ~NPIPE_CAM_C;
                    break;
                default:
                    PIPE_ERR("unknown TG");
                    break;
                }
                if (0 == (mEnablePath & (NPIPE_CAM_A | NPIPE_CAM_B | NPIPE_CAM_C))) {
                    /* all cam stopped, uni can then change link */
                    mUniLinkTG = CAM_TG_NONE;
                    mFirstConfgCam = CAM_TG_NONE;
                    NormalPipe::sIQData.IQNotify = NULL;

                    PIPE_DBG("HW path config queue uninit");
                    NormalPipe::m_pathCfgQueue.uninit();
                }
            }
        }

        if (eCamDataPath_DCIF == this->m_dataPath) {
            if(mpCamSVIOPipe) {
                PIPE_TRACE_BEGIN("TAG_CAMSV_STOP");
                ret = mpCamSVIOPipe->stop(MFALSE);
                PIPE_TRACE_END("TAG_CAMSV_STOP");
            }
        }

    }

    PIPE_DBG("-");

    this->FSM_UPDATE(op_stop);

    return ret;
}

/******************************************************************************
 *
 ******************************************************************************/
MBOOL NormalPipe::abort()
{
    IOPipeTrace _traceObj("Normalpipe::abort", IOPIPE_TRACE_EN_NOMALPIPE);
    Mutex::Autolock lock(mCfgLock);

    bool ret = MTRUE;

    if (this->FSM_CHECK(op_stop, __FUNCTION__) == MFALSE) {
        return MFALSE;
    }

    PIPE_INF("+ N:%d", mpSensorIdx);

    NormalPipe_Thread::E_ThreadGrop thdGrp;

    switch(this->m_FSM) {
    case op_start:
        thdGrp = (NormalPipe_Thread::E_ThreadGrop)(NormalPipe_Thread::eThdGrp_Internal|NormalPipe_Thread::eThdGrp_External);
        break;
    case op_suspend:
        thdGrp = NormalPipe_Thread::eThdGrp_External;
        break;
    default:
        PIPE_ERR("Unknown state: %d\n", this->m_FSM);
        return MFALSE;;
    }

    /* stop isp hw first */
    if (mpCamIOPipe) {
        if (mpNPipeThread == 0) {
            PIPE_ERR("mpNPipeThread is null");
        } else {
            /* hold idle lock */
            mpNPipeThread->idleStateHold(1, thdGrp);
        }

        if (eCamDataPath_DCIF == this->m_dataPath) {
            PIPE_TRACE_BEGIN("TAG_CAMSV_STOP");
            ret = mpCamSVIOPipe->stop(MTRUE);/*force stop*/
            PIPE_TRACE_END("TAG_CAMSV_STOP");
        }

        PIPE_TRACE_BEGIN("TAG_CAM_STOP");
        ret = mpCamIOPipe->stop(MTRUE);/*force stop*/
        PIPE_TRACE_END("TAG_CAM_STOP");

        if (mpNPipeThread) {
            this->mpNPipeThread->prestop();

            /* release idle lock */
            mpNPipeThread->idleStateHold(0,
                (NormalPipe_Thread::E_ThreadGrop)(NormalPipe_Thread::eThdGrp_Internal|NormalPipe_Thread::eThdGrp_External));
        }
    }

    if (mpNPipeThread) {
        if (mpNPipeThread->stop(MTRUE) == MFALSE) {
            PIPE_ERR("error: NPipeThread stop fail");
            return MFALSE;
        }
    }

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

    PIPE_INF("- N:%d", mpSensorIdx);

    this->FSM_UPDATE(op_stop);

    return ret;
}


/******************************************************************************
 *
 ******************************************************************************/
MBOOL NormalPipe::init(MBOOL EnableSec)
{
    IOPipeTrace _traceObj("NormalPipe::init", IOPIPE_TRACE_EN_NOMALPIPE);
    MUINT32     i, j;
    MBOOL       ret = MTRUE;

    if (this->FSM_CHECK(op_init, __FUNCTION__) == MFALSE) {
        for (i = 0 ; i < IOPIPE_MAX_NUM_USERS ; i++) {
            PIPE_ERR("user[%d]:%s", i, mUserName[i]);
        }
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
     * Secure camera init
     */
    if (EnableSec) {
        mpSecMgr = SecMgr::SecMgr_Init();
        if(mpSecMgr == NULL){
            PIPE_ERR("Secure Cam init fail");
            return MFALSE;
        }else{
            PIPE_DBG("Secure Cam (%p) init success",mpSecMgr);
        }
    }

    /*
     * NormalPipe thread init
     */
    if (mpNPipeThread) {
        if (mpNPipeThread->init(EnableSec) == MFALSE) {
            PIPE_ERR("error: NPipeThread init fail (sec:%d)\n",EnableSec);
            return MFALSE;
        }
    }

    #ifdef USE_IMAGEBUF_HEAP
    if (!mpFrameMgr) {
        NPIPE_MEM_NEW(mpFrameMgr,FrameMgr(),sizeof(FrameMgr), mMemInfo);
    }
    mpFrameMgr->init();
    #endif

    for (i = 0; i < ePort_max; i++) {
        if (!mpEnqueRequest[i]) {
            NPIPE_MEM_NEW(mpEnqueRequest[i], QueueMgr<QBufInfo>(), sizeof(QueueMgr<QBufInfo>), mMemInfo);
            mpEnqueRequest[i]->init();
        }
        if (!mpEnqueReserved[i]) {
            NPIPE_MEM_NEW(mpEnqueReserved[i], QueueMgr<QBufInfo>(), sizeof(QueueMgr<QBufInfo>), mMemInfo);
            mpEnqueReserved[i]->init();
        }
        if (!mpDeQueue[i]) {
            NPIPE_MEM_NEW(mpDeQueue[i], QueueMgr<QBufInfo>(), sizeof(QueueMgr<QBufInfo>), mMemInfo);
            mpDeQueue[i]->init();
        }
        if (!mpVirPortPA[i]) {
            NPIPE_MEM_NEW(mpVirPortPA[i], QueueMgr<MUINT32>(), sizeof(QueueMgr<MUINT32>), mMemInfo);
            mpVirPortPA[i]->init();
        }
    }

    this->initBufCtrl();
    PIPE_DBG("-");

    this->FSM_UPDATE(op_init);

    return ret;
}


/******************************************************************************
 *
 ******************************************************************************/
MBOOL NormalPipe::uninit()
{
    IOPipeTrace _traceObj("NormalPipe::uninit", IOPIPE_TRACE_EN_NOMALPIPE);
    MUINT32 i;
    MBOOL   ret = MTRUE;

    if (this->FSM_CHECK(op_uninit, __FUNCTION__) == MFALSE) {
        return MFALSE;
    }

    PIPE_INF("+ SenIdx=%d", mpSensorIdx);

    //Mutex::Autolock lock(mCfgLock);
    if (mpNPipeThread) {
        if(mpNPipeThread->uninit() == MFALSE){
            PIPE_ERR("error NPipeThread uninit fail\n");
            return MFALSE;
        }
    }

    if (mpCamIOPipe)  {
        ret = mpCamIOPipe->uninit();
    }

    if (mpCamIOPipe)  {
        Mutex::Autolock lock(NPipeCfgGLock);

        switch (_NORMALPIPE_GET_TG_IDX(mpSensorIdx)) {
        case CAM_TG_1:
            mEnablePath &= ~NPIPE_CAM_A;
            NormalPipe::sIQData.IQInfo.lv[NPIPE_MAP_PATH(mpSensorIdx)] = eCamIQ_MAX;
            break;
        case CAM_TG_2:
            mEnablePath &= ~NPIPE_CAM_B;
            NormalPipe::sIQData.IQInfo.lv[NPIPE_MAP_PATH(mpSensorIdx)] = eCamIQ_MAX;
            break;
        case CAM_TG_3:
            mEnablePath &= ~NPIPE_CAM_C;
            NormalPipe::sIQData.IQInfo.lv[NPIPE_MAP_PATH(mpSensorIdx)] = eCamIQ_MAX;
            break;
        default:
            PIPE_ERR("unknown TG");
            break;
        }

        if (0 == (mEnablePath & (NPIPE_CAM_A | NPIPE_CAM_B))) {
            /* all cam stopped, uni can then change link */
            mUniLinkTG = CAM_TG_NONE;
            mFirstConfgCam = CAM_TG_NONE;
            NormalPipe::sIQData.IQNotify = NULL;

            PIPE_DBG("HW path config queue uninit");
            NormalPipe::m_pathCfgQueue.uninit();
        }
    }

    NPipeGLock.lock();
    if (NormalPipe::pAllSensorInfo) {
        NormalPipe::pAllSensorInfo->mSenInfo[mpSensorIdx].mOccupiedOwner = (MUINT32)NULL;
    }
    else {
        PIPE_ERR("error:NormalPipe::uninit: pAllSensorInfo NULL");
    }
    NPipeGLock.unlock();

    #ifdef USE_IMAGEBUF_HEAP
    if (mpFrameMgr)  {
        mpFrameMgr->uninit();
        NPIPE_MEM_DEL(mpFrameMgr,sizeof(FrameMgr), mMemInfo);
    }
    #endif

    for (i = 0; i < ePort_max; i++) {
        if (mpEnqueRequest[i])  {
            mpEnqueRequest[i]->uninit();
            NPIPE_MEM_DEL(mpEnqueRequest[i],sizeof(QueueMgr<QBufInfo>), mMemInfo);
        }
        if (mpEnqueReserved[i])  {
            mpEnqueReserved[i]->uninit();
            NPIPE_MEM_DEL(mpEnqueReserved[i],sizeof(QueueMgr<QBufInfo>), mMemInfo);
        }
        if (mpDeQueue[i])  {
            mpDeQueue[i]->uninit();
            NPIPE_MEM_DEL(mpDeQueue[i],sizeof(QueueMgr<QBufInfo>), mMemInfo);
        }
        if (mpVirPortPA[i])  {
            mpVirPortPA[i]->uninit();
            NPIPE_MEM_DEL(mpVirPortPA[i],sizeof(QueueMgr<MUINT32>), mMemInfo);
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
MBOOL NormalPipe::enque(QBufInfo const& rQBuf)
{
    IOPipeTrace _traceObj("NormalPipe::enque", IOPIPE_TRACE_EN_NOMALPIPE);
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

        _OpenedPort = GetOpenedPortInfo(NormalPipe::_GetPort_OpendNum);

        for (i = 0; i < _OpenedPort; i++) {
            _port = NPIPE_MAP_PORTID(GetOpenedPortInfo(NormalPipe::_GetPort_Index, i));

            mpEnqueRequest[_port]->sendCmd(QueueMgr<QBufInfo>::eCmd_getsize, (MUINTPTR)&_size, 0);

            if (minDepth > _size) {
                minDepth = _size;
            }
        }
        mReqDepthPreStart = minDepth;

        if (this->mInitSettingApplied == MFALSE) {
            PIPE_INF("N:%d immeidateEnque", this->mpSensorIdx);
            mpNPipeThread->frameToEnque();

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
MBOOL NormalPipe::acceptEnqRequest(QBufInfo const& rQBuf)
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

        _port = NPIPE_MAP_PORTID(rQBuf.mvOut.at(i).mPortID.index);

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
        /*
        else if (vPortQBufSlot[_port].mvOut.size() != mBurstQNum) {
            PIPE_ERR("enque buf num not sync: %d/%d", (MUINT32)vPortQBufSlot[_port].mvOut.size(), mBurstQNum);
            return MFALSE;
        }
        */
        vPortQBufSlot[_port].mShutterTimeNs = rQBuf.mShutterTimeNs;
    }

    if (checkDropEnque(vPortQBufSlot) == MFALSE) {
        return MFALSE;
    }

    mpCamIOPipe->sendCommand(EPIPECmd_SET_FRM_TIME, (MINTPTR)(rQBuf.mShutterTimeNs/1000000), 1, 0);
    mpNPipeThread->updateFrameTime((MUINT32)(rQBuf.mShutterTimeNs/1000000), 1);

    for (_port = 0; _port < ePort_max; _port++) {
        if (vPortQBufSlot[_port].mvOut.size() == 0) {
            continue;
        }

        mpEnqueRequest[_port]->sendCmd(QueueMgr<QBufInfo>::eCmd_push, (MUINTPTR)&vPortQBufSlot[_port], 0);
        for (i = 0; i < vPortQBufSlot[_port].mvOut.size(); i++) {
            pBuf = &(vPortQBufSlot[_port].mvOut.at(i));

            char _tmp[128];
            MUINT32 curLeng = snprintf(_tmp,sizeof(_tmp),"dma:x%x pa(0x%" PRIXPTR ")io(%d_%d_%d_%d_%d_%d)M:x%x,isUF(%d),RawType(%d),Sec(%d),shutter(%" PRIu64 ")",
                    pBuf->mPortID.index, pBuf->mBuffer->getBufPA(ePlane_1st),
                    pBuf->FrameBased.mCropRect.p.x,pBuf->FrameBased.mCropRect.p.y,
                    pBuf->FrameBased.mCropRect.s.w,pBuf->FrameBased.mCropRect.s.h,
                    pBuf->FrameBased.mDstSize.w,pBuf->FrameBased.mDstSize.h,
                    pBuf->FrameBased.mMagicNum_tuning, CHECK_FMT_UF(pBuf->mBuffer->getImgFormat()),
                    pBuf->mRawOutFmt,pBuf->mSecon,
                    rQBuf.mShutterTimeNs);
            if(strLeng - totalStrLeng < curLeng){
                PIPE_INF("MyEnq:%d: %s", mpSensorIdx, str);
                str[0] = '\0';
                totalStrLeng = 0;
            }
            strncat(str, _tmp, (sizeof(str) - totalStrLeng));
            totalStrLeng += curLeng;
        }
    }
    /* for log reduction */
    PIPE_INF("MyEnq:%d: %s", mpSensorIdx, str);

    return MTRUE;
}

MBOOL NormalPipe::checkEnque(QBufInfo const& rQBuf)
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

    _tgsize.w = _NORMALPIPE_GET_SENSORCFG_CROP_W(mpSensorIdx);
    _tgsize.h= _NORMALPIPE_GET_SENSORCFG_CROP_H(mpSensorIdx);

    if (mBurstQNum && (_size % mBurstQNum)) {
        PIPE_ERR("enque buf cnt is not multiple of mBurstQNum %d/%d", _size, mBurstQNum);
        return MFALSE;
    }

    //IO checking
    for (i = 0; i < _size; i++) {
        MINT ImgFormat      = rQBuf.mvOut.at(i).mBuffer->getImgFormat();
        PortInfo.index      = rQBuf.mvOut.at(i).mPortID.index;
        qryInput.format     = (EImageFormat)NPIPE_MAP_FMT(PortInfo.index, (EImageFormat)GetOpenedPortInfo(NormalPipe::_GetPort_ImgFmt, NPIPE_MAP_PORTID(PortInfo.index)));
        qryInput.width      = rQBuf.mvOut.at(i).FrameBased.mDstSize.w;
        qryInput.pixelMode  = mSenPixMode;

        switch (PortInfo.index) {
            case EPortIndex_IMGO:
                _op = (_tgsize.w != qryInput.width)? (ENPipeQueryCmd_CROP_X_PIX): (ENPipeQueryCmd_X_PIX);
                break;
            case EPortIndex_RRZO:
                _op = ENPipeQueryCmd_X_PIX;
                break;
            case EPortIndex_CAMSV_IMGO:
                _op = (_tgsize.w != qryInput.width)? (ENPipeQueryCmd_CROP_X_PIX): (ENPipeQueryCmd_X_PIX);
                break;
            case EPortIndex_YUVO:
                _op = (_tgsize.w != qryInput.width)? (ENPipeQueryCmd_CROP_X_PIX): (ENPipeQueryCmd_X_PIX);
                break;
            case EPortIndex_CRZO:
                _op = ENPipeQueryCmd_X_PIX;
                break;
            case EPortIndex_CRZO_R2:
                _op = ENPipeQueryCmd_X_PIX;
                break;
            case EPortIndex_EISO:
            case EPortIndex_LCSO:
            case EPortIndex_RSSO:
                if(CHECK_FMT_UF(ImgFormat)){
                    PIPE_ERR("UFO fmt only support imgo/rrzo (port:%d, fmt0x%x)",
                        PortInfo.index, (MUINT32)CHECK_FMT_UF(ImgFormat));
                    return MFALSE;
                }
                return MTRUE;
            case EPortIndex_RAWI:
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

        if (NPIPE_GET_CAPIBILITY(_op, qryInput, PortInfo) == MFALSE) {
            return MFALSE;
        }
    }

    PIPE_DBG("-");

    return ret;
}

/******************************************************************************
 *
 ******************************************************************************/
MBOOL NormalPipe::checkDropEnque(QBufInfo *pQBuf)
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
                    if (_NOMRALPIPE_BUF_GETPA(pQBuf[_port].mvOut.at(k),PLANE_ID) ==
                            _NOMRALPIPE_BUF_GETPA(_popQBuf.mvOut.at(j),PLANE_ID)) {
                        PIPE_ERR("error: buf already enqued before: x%" PRIXPTR "@%d/%d",\
                                _NOMRALPIPE_BUF_GETPA(_popQBuf.mvOut.at(j),PLANE_ID), _port, k);
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
MBOOL NormalPipe::deque(QPortID& rQPort, QBufInfo& rQBuf, MUINT32 u4TimeoutMs)
{
    IOPipeTrace _traceObj("NormalPipe::deque", IOPIPE_TRACE_EN_NOMALPIPE);
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
                waitStat = ::sem_wait(&mpNPipeThread->m_semDeqDonePort[NPIPE_MAP_PORTID(portID.index)]);
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
                    waitStat = ::sem_wait(&mpNPipeThread->m_semDeqDonePort[NPIPE_MAP_PORTID(portID.index)]);
                    if ((waitStat == -1) && (errno == EINTR)) {
                        PIPE_INF("WARNING: wait again 2!");
                    }
                } while ((waitStat == -1) && (errno == EINTR));
            }
            else {
                TIME_SPEC_CAL(ts, u4TimeoutMs);

                do {
                    waitStat = sem_timedwait(&mpNPipeThread->m_semDeqDonePort[NPIPE_MAP_PORTID(portID.index)], &ts);
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

        if (NormalPipe_Thread::_ThreadErrExit == mpNPipeThread->m_DeqThrdState) {
            PIPE_ERR("N:%d dma:x%x deque fail", mpSensorIdx, portID.index);
            return MFALSE;
        }
        else if (NormalPipe_Thread::_ThreadStop == mpNPipeThread->m_DeqThrdState) {
            rQBuf.mvOut.resize(0);
            PIPE_WRN("N:%d dma:x%x deque, already stopped", mpSensorIdx, portID.index);
            return MTRUE;
        }

        mDeqContainerLock.lock();
        mpDeQueue[NPIPE_MAP_PORTID(portID.index)]->sendCmd(QueueMgr<QBufInfo>::eCmd_getsize, (MUINTPTR)&queueDepth, 0);
        if (queueDepth) {
            mpDeQueue[NPIPE_MAP_PORTID(portID.index)]->sendCmd(QueueMgr<QBufInfo>::eCmd_front, (MUINTPTR)&_qBuf, 0);
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
            mpDeQueue[NPIPE_MAP_PORTID(portID.index)]->sendCmd(QueueMgr<QBufInfo>::eCmd_getsize, (MUINTPTR)&queueDepth, 0);
            if (queueDepth) {
                mpDeQueue[NPIPE_MAP_PORTID(portID.index)]->sendCmd(QueueMgr<QBufInfo>::eCmd_pop, 0, 0);
            }
            mDeqContainerLock.unlock();
        }
    }

    PIPE_DBG("-");

    return ret;
}

MBOOL NormalPipe::addUser(char const* szUserName)
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

    PIPE_INF("%s ++, %d/%d", szUserName, mUserCnt[found_idx], mTotalUserCnt);

    return MTRUE;
}

MBOOL NormalPipe::delUser(char const* szUserName)
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

    PIPE_INF("%s --, %d/%d", szUserName, mUserCnt[found_idx], mTotalUserCnt);

    return MTRUE;
}

MINT32 NormalPipe::getTotalUserNum(void)
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
MBOOL NormalPipe::reset(void)
{
    char const* szCallerName = THIS_NAME;
    MINT32 irqUserKey = -1;

    PIPE_WRN("ESD flow start +");

    if (NULL == mpHalSensor) {
         mpHalSensor = pAllSensorInfo->mSList->createSensor(szCallerName, 1, &mpSensorIdx);
         PIPE_ERR("error: ESD flow error, HalSensor NULL obj");
    }

    PIPE_WRN("Reset+, SenIdx=%d", mpSensorIdx);

    //we need to power off sensor and power on the sensor.
    //Power off
    //avoid reset when HWSYNC is enabled
    if(this->m_bN3DEn){
        HWSyncDrv* pHwSync = NULL;
        pHwSync = HWSyncDrv::createInstance();
        if(pHwSync->sendCommand(HW_SYNC_CMD_DISABLE_SYNC, \
                HW_SYNC_BLOCKING_EXE, pAllSensorInfo->mSenInfo[mpSensorIdx].mDevId,0) != 0){
                PIPE_ERR("N3D stop fail\n");
        }
        pHwSync->destroyInstance();
    }

    if (1 == this->mBurstQNum) {
        this->mpCamIOPipe->suspend(ICamIOPipe::HW_RST_SUSPEND);
    }

    //reset
    this->mpHalSensor->reset();

    if (1 == this->mBurstQNum) {
        if(MFALSE == this->mpCamIOPipe->preresume((MINT8*)this->mpName, _NORMALPIPE_GET_SENSORCFG_CROP_W(this->mpSensorIdx))){
            PIPE_ERR("N: preresume NG\n");
            return MFALSE;
        }
        this->mpCamIOPipe->resume(ICamIOPipe::HW_RST_SUSPEND);
    }

    irqUserKey = attach(THIS_NAME);
    if (irqUserKey < 0) {
        PIPE_ERR("attach IRQ failed %d !", irqUserKey);
    }
    else {
        PIPE_WRN("Wait one VSYNC for sensor alive");
        if (wait(EPipeSignal_VSYNC, EPipeSignal_ClearWait, irqUserKey, 5000) == MFALSE) {
            PIPE_ERR("wait VSYNC fail, reset failed!");
        }
        else{
            if(this->m_bN3DEn){
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
        }
    }

    PIPE_WRN("-");

    return MTRUE;
}

/******************************************************************************
 *
 ******************************************************************************/
MBOOL NormalPipe::configPipe(QInitParam const& vInPorts, MINT32 burstQnum)
{
    IOPipeTrace      _traceObj("NormalPipe::configpipe", IOPIPE_TRACE_EN_NOMALPIPE);
    MBOOL            dump = 0, ret = MTRUE;
    MUINT32          temp, SIdx, sensorDevIdx, i = 0, j = 0;
    EImageFormat     tgi_eImgFmt[4]; //Temp Used
    CAMIO_Func       _InFunc, _OutFunc;
    HWSyncDrv*       pHwSync = NULL;
    _sensorInfo_t  *pSInfo = NULL;
    CAM_RESCONFIG    camResConfig;
    PortInfo  qryOut;
    NormalPipe_InputInfo  emptyInput;
    BufCtrlCenter::NLinkInfo linkInfo;
    MUINT32          cam_path;

    _InFunc.Raw = 0;
    _OutFunc.Raw = 0;
    mRawType = vInPorts.mRawType;
    camResConfig = vInPorts.m_CamResConfig;
    mCamResConfig = camResConfig;

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

    NPIPE_GET_TGIFMT(SIdx, tgi_eImgFmt[SIdx]);

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

    if (!mpCamIOPipe) {
        //para of SV here is due to camio's cmdq will be borrowed by sv-camio......
        CAMIO_Func              _SVInFunc, _SVOutFunc;
        vector<PortInfo const*> vCamSVIOInPorts;
        vector<PortInfo const*> vCamSVIOOutPorts;
        PortInfo                tgi,rawi;
        PortInfo                inPort,imgo, rrzo, yuvo, crzo_r1, crzo_r2, eiso, lcso, camsv_imgo, camsv2_imgo, rsso;
        MBOOL                   enable2PixelTwin = MFALSE, _LinkToUni = MFALSE, stream_on = MTRUE;
        MUINT32                 _path = 0, imgHdrBufNum = 0, Sen_PixMode = 0, rrz_w = 0;
        MUINT32                 svStride[3];
        MUINTPTR                imgHdrAllocOfst[ePlane_max] = {0, 0, 0};
        MUINTPTR                imgsvHdrAllocOfst[ePlane_max] = {0, 0, 0};
        E_INPUT                 TargetTG = TG_CAM_MAX;
        MUINT32                 SenTG = CAM_TG_NONE;
        ENPipeQueryCmd          _op = ENPipeQueryCmd_X_PIX;
        NormalPipe_InputInfo    qryInput;
        vector<PortInfo const*> vCamIOInPorts;
        vector<PortInfo const*> vCamIOOutPorts;
        vCamIOInPorts.clear();
        vCamIOOutPorts.clear();

        Mutex::Autolock lock(NPipeCfgGLock); // for protect twin mode check
        for (j = 0; j < vInPorts.mPortInfo.size(); j++) {
            if (PORT_IMGO == vInPorts.mPortInfo.at(j).mPortID) {
                mPureRaw = vInPorts.mPortInfo.at(j).mPureRaw;
            }
            if (PORT_CAMSV_IMGO == vInPorts.mPortInfo.at(j).mPortID) {
                if(this->m_dataPath != eCamDataPath_TGDL){
                    PIPE_ERR("can't enable SV_IMGO & RAWI at the same time\n");
                    return MFALSE;
                }
                this->m_dataPath = eCamDataPath_DCIF;
            }
            else if(PORT_RAWI == vInPorts.mPortInfo.at(j).mPortID) {
                if(this->m_dataPath != eCamDataPath_TGDL){
                    PIPE_ERR("can't enable SV_IMGO & RAWI at the same time\n");
                    return MFALSE;
                }
                this->m_dataPath = eCamDataPath_M2M;
            }
        }
        mDynSwtRawType = vInPorts.m_DynamicRawType;
        PIPE_WRN("start configure with hw-path:%d\n",this->m_dataPath);
        switch(this->m_dataPath)
        {
            case eCamDataPath_DCIF:
                {
                    //set full frame svtg info 1st
                    tgi.eImgFmt      = tgi_eImgFmt[mpSensorIdx];
                    NPIPE_GET_TG_PIXID(mpSensorIdx, tgi.eRawPxlID);
                    tgi.u4ImgWidth   = _NORMALPIPE_GET_SENSORCFG_CROP_W(mpSensorIdx);
                    tgi.u4ImgHeight  = _NORMALPIPE_GET_SENSORCFG_CROP_H(mpSensorIdx);
                    NPIPE_GET_TG_GRAB(mpSensorIdx, m_sensorConfigParam.scenarioId, tgi.crop1.x, tgi.crop1.y);
                    tgi.crop1.floatX = 0;
                    tgi.crop1.floatY = 0;
                    tgi.crop1.w      = tgi.u4ImgWidth;
                    tgi.crop1.h      = tgi.u4ImgHeight;
                    tgi.type         = EPortType_Sensor;
                    tgi.inout        = EPortDirection_In;


                    for (i = 0; i < vInPorts.mPortInfo.size(); i++) {
                            if(vInPorts.mPortInfo.at(i).mPortID.index == EPortIndex_CAMSV_IMGO){
                                rawi.index = EPortIndex_RAWI;
                                NPIPE_PASS_PORTINFO(mpSensorIdx, tgi, vInPorts.mPortInfo.at(i), &rawi);
                                PortImgFormat[NPIPE_MAP_PORTID(rawi.index)] = rawi.eImgFmt;

                                NPIPE_GET_TG_PIXID(mpSensorIdx, rawi.eRawPxlID);
                                rawi.frameblkT    = 33000-330;                        // assume DCIF mode, cam always get lower than 30fps, vblanking = 10%, unit: us
                                rawi.lineT        = rawi.frameblkT * 100 / rawi.u4ImgHeight;
                                rawi.lineblkT     = 0;
                                rawi.ePxlMode     = ePixMode_2; // this value will be decided by imageio
                            }
                    }

                    mpHalSensor->sendCommand(pAllSensorInfo->mSenInfo[mpSensorIdx].mDevId, SENSOR_CMD_GET_DEFAULT_FRAME_RATE_BY_SCENARIO,
                                                (MUINTPTR)&m_sensorConfigParam.scenarioId, (MUINTPTR)&tgi.tgFps, 0);

                    rawi.tgFps = tgi.tgFps/vInPorts.m_Func.Bits.SVRawBurstNum;
#if 0   //this is useless under DCIF by SenDrv !!
                    mpHalSensor->sendCommand(pAllSensorInfo->mSenInfo[mpSensorIdx].mDevId, SENSOR_CMD_GET_SENSOR_PIXELMODE,
                                                (MUINTPTR)&m_sensorConfigParam.scenarioId, (MUINTPTR)&tgi.tgFps , (MUINTPTR)&Sen_PixMode);
#endif
                    mpHalSensor->sendCommand(pAllSensorInfo->mSenInfo[mpSensorIdx].mDevId, SENSOR_CMD_GET_SENINF_PIXEL_RATE,
                                                (MUINTPTR)&m_sensorConfigParam.scenarioId, (MUINTPTR)&tgi.MIPI_PixRate, (MUINTPTR)&m_sensorConfigParam.scenarioId);
                    rawi.MIPI_PixRate = tgi.MIPI_PixRate;

                    mpCamIOPipe = ICamIOPipe::createInstance((MINT8*)mpName, TargetTG, NSImageio::NSIspio::ICamIOPipe::CAMIO, &rawi);
                    if (!mpCamIOPipe) {
                        PIPE_ERR("mpCamIOPipe createInstance fail");
                        return MFALSE;
                    }
                    {
                        enum EPortIndex _out;
                        if(TG_MAP_Port(TargetTG,NULL,&_out) == MFALSE)
                            return MFALSE;
                        else{
                            if(rawi.index != (MUINT32)_out){
                                PIPE_ERR("only TG_A support rawi(%d_%d)\n",rawi.index,_out);
                                return MFALSE;
                            }
                        }
                    }
                    rawi.tTimeClk             = _NORMALPIPE_GET_CLK_FREQ(SIdx)/100; //0.1MHz <- KHz

                    // Query RawI related sizePortInfo
                    //RAWI Query x_pix
                    _op                = ENPipeQueryCmd_X_PIX;
                    qryInput.format    = rawi.eImgFmt;
                    qryInput.width     = rawi.u4ImgWidth;
                    qryInput.pixelMode = ePixMode_NONE;
                    if(NPIPE_GET_CAPIBILITY(_op, qryInput, rawi) == MFALSE) {
                        return MFALSE;
                    }
                    /*query rawi stride */
                    _op                 = ENPipeQueryCmd_STRIDE_BYTE;
                    qryInput.format     = rawi.eImgFmt;
                    qryInput.width      = rawi.crop1.w;
                    qryInput.pixelMode  = ePixMode_NONE;
                    if(NPIPE_GET_CAPIBILITY(_op, qryInput, rawi) == MFALSE) {
                        return MFALSE;
                    }

                    vCamIOInPorts.push_back(&rawi);
                    //cam's input is from rawi
                    inPort = rawi;

                    PIPE_INF("N:%d TG=%d Scen=%d tgFmt=x%x devID=%d eRawPxlID=%d PixelMode=%d W/H=[%d,%d] stride[%d,%d,%d], FPSx10=%d Clk=%d khz mipiPixRate=%d burst=%d",
                         mpSensorIdx, (MUINT32)TargetTG, m_sensorConfigParam.scenarioId, rawi.eImgFmt, pAllSensorInfo->mSenInfo[mpSensorIdx].mDevId,
                         rawi.eRawPxlID, rawi.ePxlMode, rawi.u4ImgWidth, rawi.u4ImgHeight,
                         rawi.u4Stride[ePlane_1st], rawi.u4Stride[ePlane_2nd], rawi.u4Stride[ePlane_3rd],
                         rawi.tgFps,  rawi.tTimeClk*100, rawi.MIPI_PixRate, burstQnum);

                    if (!mpCamSVIOPipe) {//muist after cam , need to know link to which CAM-TG
                        vector<E_INPUT>         vTargetSVTG;
                        MUINT32                 pixel_mode = 0;
                        MUINT32                 isp_grab_alignment = 0;
                        MUINT32                 TG_M, TG_S;
                        MBOOL                   enable_camsv_twin;


                        vCamSVIOInPorts.clear();
                        vCamSVIOOutPorts.clear();

                        // Mutex::Autolock lock(NormalSvPipe::NSvPipeCfgGLock); // for protect twin mode check
                        mPureRaw = MTRUE;
                        mDynSwtRawType = MFALSE;

                        vTargetSVTG.push_back(TG_CAMSV_0);
                        tgi.index        = (MUINT32)vTargetSVTG.at(0);
                        //output sv-twin is needed or not
                        mpCamSVIOPipe = ICamIOPipe::createInstance((MINT8*)mpName, vTargetSVTG, NSImageio::NSIspio::ICamIOPipe::NORMALSVIO, &tgi);
                        if (!mpCamSVIOPipe) {
                            PIPE_ERR("mpCamSVIOPipe createInstance fail");
                            return MFALSE;
                        }
                        enable_camsv_twin = (vTargetSVTG.size()>1);

                        m_sensorConfigParam.isp_pixel_mode = FOUR_PIXEL_MODE;
                        m_sensorConfigParam.isp_tg_num = (enable_camsv_twin) ? 2 : 1;
                        m_sensorConfigParam.isp_grab_alignment = 64;

                        // configure after camsviopipe create
                        NPipeCfgGLock.unlock();
                        // Ignore sensor config if have done by P1Node
                        PIPE_TRACE_BEGIN("TAG_SENSOR_CONFIG");
                        PIPE_WRN("sensor configure ++\n");
                        if (!NPIPE_GET_CAPIBILITY(ENPipeQueryCmd_NEED_CFG_SENSOR_BY_MW, emptyInput, qryOut)) {
                            if (mpHalSensor->configure(1, &m_sensorConfigParam) == MFALSE) {
                                PIPE_ERR("fail when configure sensor driver\n");
                                return MFALSE;
                            }
                        } else {
                            PIPE_INF("Ignore mpHalSensor(%d)->configure, should have done by P1Node.", mpSensorIdx);
                        }
                        PIPE_TRACE_END("TAG_SENSOR_CONFIG");
                        PIPE_WRN("sensor configure --\n");

                        NPipeCfgGLock.lock();

                        mpHalSensor->querySensorDynamicInfo(pAllSensorInfo->mSenInfo[SIdx].mDevId, &pAllSensorInfo->mSenInfo[SIdx].mDInfo);
                        PIPE_DBG("querySensorDynamicInfo DevId:%d, DInfo:%d",
                            (MUINT32)pAllSensorInfo->mSenInfo[SIdx].mDevId,
                            (MUINT32)pAllSensorInfo->mSenInfo[SIdx].mDInfo.TgInfo);

                        // Config master sen_mux
                        TG_M = pAllSensorInfo->mSenInfo[SIdx].mDInfo.config_infos.at(0).tg_info;
                        SenTG = NSVPIPE_MAP_SEN_TG(vTargetSVTG.at(0));
                        pAllSensorInfo->mSenInfo[SIdx].mDInfo.config_infos.at(0).tg_info = SenTG;
                        mpHalSensor->sendCommand(pAllSensorInfo->mSenInfo[SIdx].mDevId, SENSOR_CMD_SET_SENINF_CAM_TG_MUX, (MUINTPTR)&TG_M, (MUINTPTR)&SenTG, 0);

                        mpHalSensor->sendCommand(pAllSensorInfo->mSenInfo[mpSensorIdx].mDevId, SENSOR_CMD_GET_HORIZONTAL_BLANKING,
                                                (MUINTPTR)&SenTG, (MUINTPTR)&tgi.lineblkT, (MUINTPTR)&tgi.lineT);
                        mpHalSensor->sendCommand(pAllSensorInfo->mSenInfo[mpSensorIdx].mDevId, SENSOR_CMD_GET_VERTICAL_BLANKING, \
                                                    (MUINTPTR)&tgi.frameblkT, 0, 0);
                        // Config tgi
                        // Get pixel mode, width according to config_infos in dynamic info
                        tgi.index                = NSVPIPE_MAP_TGI(NSVPIPE_MAP_SEN_TG(vTargetSVTG.at(0)));
                        tgi.ePxlMode             = NPIPE_MAP_PXLMODE(mpSensorIdx, pAllSensorInfo->mSenInfo[SIdx].mDInfo.config_infos.at(0).mux_pixel_mode);
                        tgi.tTimeClk             = _NORMALPIPE_GET_CLK_FREQ(SIdx)/100; //0.1MHz <- KHz
                        tgi.u4ImgWidth           = (enable_camsv_twin) ? _NORMALPIPE_GET_SENSORCFG_CROP_W(mpSensorIdx) : pAllSensorInfo->mSenInfo[SIdx].mDInfo.config_infos.at(0).grab_size_x;
                        tgi.u4ImgHeight          = _NORMALPIPE_GET_SENSORCFG_CROP_H(mpSensorIdx);
                        tgi.crop1.floatX         = 0;
                        tgi.crop1.floatY         = 0;
                        tgi.crop1.w              = tgi.u4ImgWidth;
                        tgi.crop1.h              = tgi.u4ImgHeight;

                        mCamSvImgoOut_size.w = tgi.crop1.w;
                        mCamSvImgoOut_size.h = tgi.u4ImgHeight;
                        vCamSVIOInPorts.push_back(&tgi);

                        PIPE_INF("N:%d SVTG=%d Scen=%d tgFmt=x%x devID=%d eRawPxlID=%d PixelMode=%d W/H=[%d,%d] FPSx10=%d Clk=%d khz mipiPixRate=%d burst=%d SVBurst=%d",
                             mpSensorIdx, (MUINT32)vTargetSVTG.at(0), m_sensorConfigParam.scenarioId, tgi.eImgFmt, pAllSensorInfo->mSenInfo[mpSensorIdx].mDevId,
                             tgi.eRawPxlID, tgi.ePxlMode, tgi.crop1.w, tgi.u4ImgHeight, tgi.tgFps,  tgi.tTimeClk*100, tgi.MIPI_PixRate, burstQnum, vInPorts.m_Func.Bits.SVRawBurstNum);

                        if(enable_camsv_twin) {
                            // Config slave sen_mux
                            TG_S = pAllSensorInfo->mSenInfo[SIdx].mDInfo.config_infos.at(1).tg_info;
                            SenTG = NPIPE_MAP_SEN_TG(vTargetSVTG.at(1));
                            pAllSensorInfo->mSenInfo[SIdx].mDInfo.config_infos.at(0).tg_info = SenTG;
                            mpHalSensor->sendCommand(pAllSensorInfo->mSenInfo[SIdx].mDevId, SENSOR_CMD_SET_SENINF_CAM_TG_MUX, (MUINTPTR)&TG_S, (MUINTPTR)&SenTG, 0);

                            mpHalSensor->sendCommand(pAllSensorInfo->mSenInfo[mpSensorIdx].mDevId, SENSOR_CMD_GET_HORIZONTAL_BLANKING,
                                                (MUINTPTR)&SenTG, (MUINTPTR)&tgi.lineblkT, (MUINTPTR)&tgi.lineT);
                            mpHalSensor->sendCommand(pAllSensorInfo->mSenInfo[mpSensorIdx].mDevId, SENSOR_CMD_GET_VERTICAL_BLANKING, \
                                                    (MUINTPTR)&tgi.frameblkT, 0, 0);
                            // Config tgi
                            // Get pixel mode, width according to config_infos in dynamic info
                            tgi.index                = NSVPIPE_MAP_TGI(NSVPIPE_MAP_SEN_TG(vTargetSVTG.at(1)));
                            tgi.ePxlMode             = NPIPE_MAP_PXLMODE(mpSensorIdx, pAllSensorInfo->mSenInfo[SIdx].mDInfo.config_infos.at(0).mux_pixel_mode);
                            tgi.tTimeClk             = _NORMALPIPE_GET_CLK_FREQ(SIdx)/100; //0.1MHz <- KHz
                            tgi.u4ImgWidth           = pAllSensorInfo->mSenInfo[SIdx].mDInfo.config_infos.at(1).grab_size_x;
                            tgi.u4ImgHeight          = _NORMALPIPE_GET_SENSORCFG_CROP_H(mpSensorIdx);
                            tgi.crop1.floatX         = 0;
                            tgi.crop1.floatY         = 0;
                            tgi.crop1.w              = pAllSensorInfo->mSenInfo[SIdx].mDInfo.config_infos.at(1).grab_size_x;
                            tgi.crop1.h              = tgi.u4ImgHeight;
                            mCamSv2ImgoOut_size.w = pAllSensorInfo->mSenInfo[SIdx].mDInfo.config_infos.at(1).grab_size_x;
                            mCamSv2ImgoOut_size.h = tgi.u4ImgHeight;
                            vCamSVIOInPorts.push_back(&tgi);

                            PIPE_INF("sv-twin slave W[%d]", tgi.crop1.w);
                        }

                        _SVInFunc.Bits.DCIF_SvTwin_En = enable_camsv_twin;
                        _SVInFunc.Bits.DATA_PATTERN = vInPorts.m_Func.Bits.DATA_PATTERN;
                        _SVInFunc.Bits.SUBSAMPLE = vInPorts.m_Func.Bits.SVRawBurstNum - 1;
                        _SVInFunc.Bits.DCIF_En = 1;
                        _SVInFunc.Bits.DCIF_MasterRaw = NPIPE_MAP_PATH(TargetTG);

                        //output
                        for (i = 0; i < vInPorts.mPortInfo.size(); i++) {
                            if(vInPorts.mPortInfo.at(i).mPortID.index == EPortIndex_CAMSV_IMGO){
                                if(GetOpenedPortInfo(NormalPipe::_GetPort_SetPort,vInPorts.mPortInfo.at(i).mPortID.index) == -1){
                                    PIPE_ERR("dmao config error\n");
                                    return MFALSE;
                                }

                                camsv_imgo.index = vInPorts.mPortInfo.at(i).mPortID.index;
                                NPIPE_PASS_PORTINFO(mpSensorIdx, tgi, vInPorts.mPortInfo.at(i), &camsv_imgo);
                                PortImgFormat[NPIPE_MAP_PORTID(camsv_imgo.index)] = camsv_imgo.eImgFmt; //for GetOpenedPortInfo()

                                //camsv IMGO Query x_pix
                                _op = (camsv_imgo.u4ImgWidth != _NORMALPIPE_GET_SENSORCFG_CROP_W(mpSensorIdx))? (ENPipeQueryCmd_CROP_X_PIX): (ENPipeQueryCmd_X_PIX);
                                qryInput.format    = camsv_imgo.eImgFmt;
                                qryInput.width     = camsv_imgo.u4ImgWidth;
                                qryInput.pixelMode = mSenPixMode;

                                if(NPIPE_GET_CAPIBILITY(_op, qryInput, camsv_imgo) == MFALSE) {
                                    return MFALSE;
                                }
                                //SVIMGO Query stride
                                _op            = ENPipeQueryCmd_STRIDE_BYTE;
                                qryInput.width = camsv_imgo.crop1.w;
                                qryInput.pixelMode = ePixMode_NONE;

                                if(NPIPE_GET_CAPIBILITY(_op, qryInput, camsv_imgo) == MFALSE) {
                                    return MFALSE;
                                }

                                PIPE_DBG("OutPorts[%d] Crop=[%d,%d,%d,%d], sz=(%d,%d), Stride=(%d/%d/%d)",\
                                        camsv_imgo.index, camsv_imgo.crop1.x, camsv_imgo.crop1.y, camsv_imgo.crop1.w, camsv_imgo.crop1.h,\
                                        camsv_imgo.u4ImgWidth, camsv_imgo.u4ImgHeight,camsv_imgo.u4Stride[ePlane_1st],
                                        camsv_imgo.u4Stride[ePlane_2nd],camsv_imgo.u4Stride[ePlane_3rd]);

                                vCamSVIOOutPorts.push_back(&camsv_imgo);
                            }
                        }

                        PIPE_TRACE_BEGIN("TAG_CAMSV_CONFIG");

                        if (MFALSE == mpCamSVIOPipe->init()) {
                             PIPE_ERR("CamSVIOPipe init fail");
                             return MFALSE;
                        }
                        #if 0     //para of SV here is due to camio's cmdq will be borrowed by sv-camio......
                        if (MFALSE == mpCamSVIOPipe->configPipe(vCamSVIOInPorts, vCamSVIOOutPorts, &_SVInFunc, &_SVOutFunc)) {
                            PIPE_ERR("CamIOPipe configPipe Fail");
                            return MFALSE;
                        }
                        #endif
                        PIPE_TRACE_END("TAG_CAMSV_CONFIG");
                    }
                }
                break;
            case eCamDataPath_M2M:
            case eCamDataPath_TGDL:
                tgi.eImgFmt      = tgi_eImgFmt[mpSensorIdx];
                NPIPE_GET_TG_PIXID(mpSensorIdx, tgi.eRawPxlID);
                tgi.u4ImgWidth   = _NORMALPIPE_GET_SENSORCFG_CROP_W(mpSensorIdx);
                tgi.u4ImgHeight  = _NORMALPIPE_GET_SENSORCFG_CROP_H(mpSensorIdx);
                NPIPE_GET_TG_GRAB(mpSensorIdx, m_sensorConfigParam.scenarioId, tgi.crop1.x, tgi.crop1.y);
                tgi.crop1.floatX = 0;
                tgi.crop1.floatY = 0;
                tgi.crop1.w      = tgi.u4ImgWidth;
                tgi.crop1.h      = tgi.u4ImgHeight;
                tgi.type         = EPortType_Sensor;
                tgi.inout        = EPortDirection_In;

                mpHalSensor->sendCommand(pAllSensorInfo->mSenInfo[mpSensorIdx].mDevId, SENSOR_CMD_GET_DEFAULT_FRAME_RATE_BY_SCENARIO,
                                            (MUINTPTR)&m_sensorConfigParam.scenarioId, (MUINTPTR)&tgi.tgFps, 0);
                mpHalSensor->sendCommand(pAllSensorInfo->mSenInfo[mpSensorIdx].mDevId, SENSOR_CMD_GET_SENINF_PIXEL_RATE,
                                            (MUINTPTR)&m_sensorConfigParam.scenarioId, (MUINTPTR)&tgi.MIPI_PixRate, (MUINTPTR)&m_sensorConfigParam.scenarioId);

                // Use User specified TG to create CamIO
                if (TG_CAM_MAX != (E_INPUT)camResConfig.Bits.targetTG) {
                    TargetTG = (E_INPUT)camResConfig.Bits.targetTG;
                    PIPE_WRN("user assign tg idx:%d(path:%d)\n", TargetTG, this->m_dataPath);
                }
                mpCamIOPipe = ICamIOPipe::createInstance((MINT8*)mpName, TargetTG, NSImageio::NSIspio::ICamIOPipe::CAMIO, &tgi);
                if (!mpCamIOPipe) {
                    PIPE_ERR("CamIOPipe createInstance fail");
                    return MFALSE;
                }
                m_sensorConfigParam.isp_pixel_mode = FOUR_PIXEL_MODE;
                m_sensorConfigParam.isp_tg_num = 1;
                m_sensorConfigParam.isp_grab_alignment = 64;
                // configure after camiopipe create
                NPipeCfgGLock.unlock();
                // Ignore sensor config if have done by P1Node
                PIPE_TRACE_BEGIN("TAG_SENSOR_CONFIG");
                PIPE_WRN("sensor configure ++\n");
                if (!NPIPE_GET_CAPIBILITY(ENPipeQueryCmd_NEED_CFG_SENSOR_BY_MW, emptyInput, qryOut)) {
                    if (mpHalSensor->configure(1, &m_sensorConfigParam) == MFALSE) {
                        PIPE_ERR("fail when configure sensor driver\n");
                        return MFALSE;
                    }
                } else {
                    PIPE_INF("Ignore mpHalSensor(%d)->configure, should have done by P1Node.", mpSensorIdx);
                }
                PIPE_TRACE_END("TAG_SENSOR_CONFIG");
                PIPE_WRN("sensor configure --\n");
                NPipeCfgGLock.lock();

                mpHalSensor->querySensorDynamicInfo(pAllSensorInfo->mSenInfo[SIdx].mDevId, &pAllSensorInfo->mSenInfo[SIdx].mDInfo);

                //Secure cam support
                if(vInPorts.m_SecureCam.buf_type == E_SEC_LEVEL_SECURE || vInPorts.m_SecureCam.buf_type == E_SEC_LEVEL_PROTECTED) {
                    vector<MUINT32> sec_port;

                    for (j = 0; j < vInPorts.mPortInfo.size(); j++) {
                        PIPE_DBG("DMA port:(%d)",vInPorts.mPortInfo.at(j).mPortID.index);
                        if(vInPorts.mPortInfo.at(j).mSecon) {
                            PIPE_DBG("secure port:(%d)",vInPorts.mPortInfo.at(j).mPortID.index);
                            sec_port.push_back(vInPorts.mPortInfo.at(j).mPortID.index);
                        }
                    }

                    PIPE_INF("enable secure cam (%d) TEE type:0x%x chk:0x%llx", (MUINT32)TargetTG,
                             (MUINT32)vInPorts.m_SecureCam.buf_type,(MUINT64)vInPorts.m_SecureCam.chk_handle);

                    if(MFALSE == this->mpSecMgr->SecMgr_attach((MUINT32)TargetTG, pAllSensorInfo->mSenInfo[mpSensorIdx].mDevId,sec_port,burstQnum,
                        (MUINT32)vInPorts.m_SecureCam.buf_type,(MUINT64)vInPorts.m_SecureCam.chk_handle)) {
                        PIPE_ERR("Secure CAM attach failure");
                        return MFALSE;
                    }
                }

                SenTG = NPIPE_MAP_SEN_TG(TargetTG);
                mpHalSensor->sendCommand(pAllSensorInfo->mSenInfo[SIdx].mDevId, SENSOR_CMD_SET_SENINF_CAM_TG_MUX, (MUINTPTR)&_NORMALPIPE_GET_TG_IDX(SIdx), (MUINTPTR)&SenTG, 0);
                mpHalSensor->sendCommand(pAllSensorInfo->mSenInfo[mpSensorIdx].mDevId, SENSOR_CMD_GET_HORIZONTAL_BLANKING,
                                                (MUINTPTR)&SenTG, (MUINTPTR)&tgi.lineblkT, (MUINTPTR)&tgi.lineT);
                mpHalSensor->sendCommand(pAllSensorInfo->mSenInfo[mpSensorIdx].mDevId, SENSOR_CMD_GET_VERTICAL_BLANKING, \
                                            (MUINTPTR)&tgi.frameblkT, 0, 0);
                mpHalSensor->sendCommand(pAllSensorInfo->mSenInfo[mpSensorIdx].mDevId, SENSOR_CMD_GET_SENSOR_PIXELMODE,
                                            (MUINTPTR)&m_sensorConfigParam.scenarioId, (MUINTPTR)&tgi.tgFps , (MUINTPTR)&Sen_PixMode);
                NormalPipe::pAllSensorInfo->mSenInfo[SIdx].mDInfo.TgInfo = SenTG;


                {
                    enum EPortIndex _out;
                    if(TG_MAP_Port(TargetTG,&_out,NULL) == MFALSE)
                        return MFALSE;
                    else
                        tgi.index = (MUINT32)_out;
                }
                tgi.ePxlMode             = mSenPixMode = NPIPE_MAP_PXLMODE(mpSensorIdx, Sen_PixMode);
                tgi.tTimeClk             = _NORMALPIPE_GET_CLK_FREQ(SIdx)/100; //0.1MHz <- KHz
                vCamIOInPorts.push_back(&tgi);

                PIPE_INF("N:%d TG=%d Scen=%d tgFmt=x%x devID=%d eRawPxlID=%d PixelMode=%d W/H=[%d,%d] FPSx10=%d Clk=%d khz mipiPixRate=%d burst=%d lineT/blk=%d_%d",
                     mpSensorIdx, (MUINT32)TargetTG, m_sensorConfigParam.scenarioId, tgi.eImgFmt, pAllSensorInfo->mSenInfo[mpSensorIdx].mDevId,
                     tgi.eRawPxlID, tgi.ePxlMode, tgi.u4ImgWidth, tgi.u4ImgHeight, tgi.tgFps,  tgi.tTimeClk*100, tgi.MIPI_PixRate, burstQnum,tgi.lineT,tgi.lineblkT);

                //cam's input is from tg
                inPort = tgi;

                // Set LinkInfo for central buffer control units
                linkInfo.mpSenIdx       = mpSensorIdx;
                linkInfo.mTgIdx         = (E_INPUT)TargetTG;
                linkInfo.mTgSize.w      = tgi.u4ImgWidth;
                linkInfo.mTgSize.h      = tgi.u4ImgHeight;
                linkInfo.mpCamIO        = mpCamIOPipe;
                linkInfo.mBufCnt        = 0;
                linkInfo.pix_mode_tg    = tgi.ePxlMode;
                linkInfo.raw_fps        = tgi.tgFps;
                linkInfo.SrcImgFmt      = (NSCam::EImageFormat)tgi.eImgFmt;
                linkInfo.bypass_rrzo    = MTRUE;
                linkInfo.offBin         = MFALSE;
                linkInfo.offTwin        = MFALSE;
                linkInfo.pattern        = (E_CamPattern)vInPorts.m_Func.Bits.DATA_PATTERN;
                linkInfo.MIPI_pixrate   = tgi.MIPI_PixRate;
                cam_path                = NPIPE_MAP_PATH_BY_TG(SenTG);

                if(this->m_dataPath == eCamDataPath_M2M){
                    for (i = 0; i < vInPorts.mPortInfo.size(); i++) {
                        if(vInPorts.mPortInfo.at(i).mPortID.index == EPortIndex_RAWI){
                            {
                                enum EPortIndex _out;
                                if(TG_MAP_Port(TargetTG,NULL,&_out) == MFALSE)
                                    return MFALSE;
                                else
                                    rawi.index = (MUINT32)_out;
                            }
                            if(GetOpenedPortInfo(NormalPipe::_GetPort_SetPort,vInPorts.mPortInfo.at(i).mPortID.index) == -1){
                                PIPE_ERR("dmao config error\n");
                                return MFALSE;
                            }
                            PortImgFormat[NPIPE_MAP_PORTID(rawi.index)] = rawi.eImgFmt      = vInPorts.mPortInfo.at(i).mFmt;

                            NPIPE_PASS_PORTINFO(mpSensorIdx, tgi, vInPorts.mPortInfo.at(i), &rawi);

                            NPIPE_GET_TG_PIXID(mpSensorIdx, rawi.eRawPxlID);

                            if((rawi.u4ImgWidth != vInPorts.mPortInfo.at(i).mDstSize.w) || (rawi.u4ImgHeight != vInPorts.mPortInfo.at(i).mDstSize.h)){
                                PIPE_ERR("rawi size mismatch.(tg:%d_%d, rawi:%d_%d)\n");
                                return MFALSE;
                            }

                            rawi.frameblkT    = 33000-330;                        // assume DCIF mode, cam always get lower than 30fps, vblanking = 10%, unit: us
                            rawi.lineT        = rawi.frameblkT * 100 / rawi.u4ImgHeight;
                            rawi.lineblkT     = 0;
                            rawi.tgFps = tgi.tgFps;
                            rawi.ePxlMode = ePixMode_2; // this value will be decided by imageio
                            rawi.MIPI_PixRate = tgi.MIPI_PixRate;
                            rawi.tTimeClk             = _NORMALPIPE_GET_CLK_FREQ(SIdx)/100; //0.1MHz <- KHz

                            // Query RawI related sizePortInfo
                            //RAWI Query x_pix
                            _op                = ENPipeQueryCmd_X_PIX;
                            qryInput.format    = rawi.eImgFmt;
                            qryInput.width     = rawi.u4ImgWidth;
                            qryInput.pixelMode = ePixMode_NONE;
                            if(NPIPE_GET_CAPIBILITY(_op, qryInput, rawi) == MFALSE) {
                                return MFALSE;
                            }
                            /*query rawi stride */
                            _op                 = ENPipeQueryCmd_STRIDE_BYTE;
                            qryInput.format     = rawi.eImgFmt;
                            qryInput.width      = rawi.crop1.w;
                            qryInput.pixelMode  = ePixMode_NONE;
                            if(NPIPE_GET_CAPIBILITY(_op, qryInput, rawi) == MFALSE) {
                                return MFALSE;
                            }

                            vCamIOInPorts.push_back(&rawi);

                            //cam's input is from rawi
                            inPort = rawi;

                            PIPE_INF("N:%d RAWI-TG=%d Scen=%d tgFmt=x%x devID=%d eRawPxlID=%d PixelMode=%d W/H=[%d,%d] stride[%d,%d,%d], FPSx10=%d Clk=%d khz mipiPixRate=%d burst=%d",
                                 mpSensorIdx, (MUINT32)TargetTG, m_sensorConfigParam.scenarioId, rawi.eImgFmt, pAllSensorInfo->mSenInfo[mpSensorIdx].mDevId,
                                 rawi.eRawPxlID, rawi.ePxlMode, rawi.u4ImgWidth, rawi.u4ImgHeight,
                                 rawi.u4Stride[ePlane_1st], rawi.u4Stride[ePlane_2nd], rawi.u4Stride[ePlane_3rd],
                                 rawi.tgFps,  rawi.tTimeClk*100, rawi.MIPI_PixRate, burstQnum);
                        }
                    }
                }
                break;
            default:
                PIPE_ERR("unsupported path:%d\n");
                return MFALSE;
                break;
        }


        pHwSync = HWSyncDrv::createInstance();
        if (pHwSync != NULL) {
        //hwsync drv, sendCommand that the sensor is configured
            if(pHwSync->sendCommand(HW_SYNC_CMD_SET_MODE, pAllSensorInfo->mSenInfo[SIdx].mDevId, m_sensorConfigParam.scenarioId, m_sensorConfigParam.framerate))
            {
                PIPE_ERR("camera(%d) pHwSync sendCommand fail ", pAllSensorInfo->mSenInfo[SIdx].mDevId);
                return MFALSE;
            }
        }

        if(mRawType == 1){
            mpHalSensor->sendCommand(pAllSensorInfo->mSenInfo[SIdx].mDevId, SENSOR_CMD_SET_TEST_MODEL, (MUINTPTR)&stream_on, 0, 0);
        }
        else{
            mpHalSensor->sendCommand(pAllSensorInfo->mSenInfo[SIdx].mDevId, SENSOR_CMD_SET_STREAMING_RESUME, 0, 0, 0);
        }

        if (MFALSE == mpCamIOPipe->init()) {
             PIPE_ERR("CamIOPipe init fail");
             return MFALSE;
        }

        //port's data paser
        for (i = 0; i < vInPorts.mPortInfo.size(); i++) {
            PortInfo *pOut = NULL;
            switch (vInPorts.mPortInfo.at(i).mPortID.index) {
                case EPortIndex_IMGO:
                    pOut          = &imgo;
                    mPureRaw      = vInPorts.mPortInfo.at(i).mPureRaw;
                    break;
                case EPortIndex_RRZO:
                    pOut          = &rrzo;
                    break;
                case EPortIndex_EISO:
                    pOut         = &eiso;
                    break;
                case EPortIndex_LCSO:
                    pOut         = &lcso;
                    break;
                case EPortIndex_RSSO:
                    pOut         = &rsso;
                    break;
                case EPortIndex_YUVO:
                    pOut         = &yuvo;
                    break;
                case EPortIndex_CRZO:
                    pOut         = &crzo_r1;
                    break;
                case EPortIndex_CRZO_R2:
                    pOut         = &crzo_r2;
                    break;
                case EPortIndex_RAWI:       //do nothing , this is input port
                case EPortIndex_CAMSV_IMGO: //do nothing, already exe. at case of camsv
                    pOut         = NULL;
                    break;
                default:
                    PIPE_ERR("unknown port id %d", vInPorts.mPortInfo.at(i).mPortID.index);
                    return MFALSE;
                    break;
            }

            if(pOut != NULL){
                if(GetOpenedPortInfo(NormalPipe::_GetPort_SetPort,vInPorts.mPortInfo.at(i).mPortID.index) == -1){
                    PIPE_ERR("dmao config error\n");
                    return MFALSE;
                }

                pOut->index = vInPorts.mPortInfo.at(i).mPortID.index;
                NPIPE_PASS_PORTINFO(mpSensorIdx, inPort, vInPorts.mPortInfo.at(i), pOut);

                PortImgFormat[NPIPE_MAP_PORTID(pOut->index)] = pOut->eImgFmt; //for GetOpenedPortInfo()

                switch(pOut->index) {
                    case EPortIndex_IMGO:
                        //IMGO Query x_pix
                        _op = (pOut->u4ImgWidth != _NORMALPIPE_GET_SENSORCFG_CROP_W(mpSensorIdx))? (ENPipeQueryCmd_CROP_X_PIX): (ENPipeQueryCmd_X_PIX);
                        qryInput.format    = pOut->eImgFmt;
                        qryInput.width     = pOut->u4ImgWidth;
                        qryInput.pixelMode = mSenPixMode;

                        if(NPIPE_GET_CAPIBILITY(_op, qryInput, *pOut) == MFALSE) {
                            return MFALSE;
                        }
                        //IMGO Query stride
                        _op            = ENPipeQueryCmd_STRIDE_BYTE;
                        qryInput.width = pOut->crop1.w;
                        qryInput.pixelMode = ePixMode_NONE;

                        if(NPIPE_GET_CAPIBILITY(_op, qryInput, *pOut) == MFALSE) {
                            return MFALSE;
                        }
                        break;
                    case EPortIndex_RRZO:
                        //RRZO Query x_pix
                        _op = ENPipeQueryCmd_X_PIX;
                        qryInput.format    = pOut->eImgFmt;
                        qryInput.width     = pOut->u4ImgWidth;
                        qryInput.pixelMode = mSenPixMode;
                        rrz_w              = pOut->u4ImgWidth;

                        if(NPIPE_GET_CAPIBILITY(_op, qryInput, *pOut) == MFALSE) {
                            return MFALSE;
                        }
                        //RRZO Query stride
                        _op                = ENPipeQueryCmd_STRIDE_BYTE;
                        qryInput.width     = pOut->crop1.w;
                        qryInput.pixelMode = ePixMode_NONE;

                        if(NPIPE_GET_CAPIBILITY(_op, qryInput, *pOut) == MFALSE) {
                            return MFALSE;
                        }
                        linkInfo.bypass_rrzo    = MFALSE;
                        break;
                    case EPortIndex_YUVO:
                        //YUVO Query x_pix
                        _op = (pOut->u4ImgWidth != _NORMALPIPE_GET_SENSORCFG_CROP_W(mpSensorIdx))? (ENPipeQueryCmd_CROP_X_PIX): (ENPipeQueryCmd_X_PIX);
                        qryInput.format    = pOut->eImgFmt;
                        qryInput.width     = pOut->u4ImgWidth;
                        qryInput.pixelMode = mSenPixMode;

                        if(NPIPE_GET_CAPIBILITY(_op, qryInput, *pOut) == MFALSE) {
                            return MFALSE;
                        }

                        //YUVO Query stride
                        _op            = ENPipeQueryCmd_STRIDE_BYTE;
                        qryInput.width = pOut->crop1.w;
                        qryInput.pixelMode = ePixMode_NONE;

                        if(NPIPE_GET_CAPIBILITY(_op, qryInput, *pOut) == MFALSE) {
                            return MFALSE;
                        }
                        break;
                    case EPortIndex_CRZO:
                        //CRZO Query x_pix
                        _op = ENPipeQueryCmd_X_PIX;
                        qryInput.format    = pOut->eImgFmt;
                        qryInput.width     = pOut->u4ImgWidth;
                        qryInput.pixelMode = mSenPixMode;

                        if(NPIPE_GET_CAPIBILITY(_op, qryInput, *pOut) == MFALSE) {
                            return MFALSE;
                        }
                        //CRZO Query stride
                        _op                = ENPipeQueryCmd_STRIDE_BYTE;
                        qryInput.width     = pOut->crop1.w;
                        qryInput.pixelMode = ePixMode_NONE;

                        if(NPIPE_GET_CAPIBILITY(_op, qryInput, *pOut) == MFALSE) {
                            return MFALSE;
                        }
                        break;
                    case EPortIndex_CRZO_R2:
                        //CRZO_R2 Query x_pix
                        _op = ENPipeQueryCmd_X_PIX;
                        qryInput.format    = pOut->eImgFmt;
                        qryInput.width     = pOut->u4ImgWidth;
                        qryInput.pixelMode = mSenPixMode;

                        if(NPIPE_GET_CAPIBILITY(_op, qryInput, *pOut) == MFALSE) {
                            return MFALSE;
                        }
                        //CRZO_R2 Query stride
                        _op                = ENPipeQueryCmd_STRIDE_BYTE;
                        qryInput.width     = pOut->crop1.w;
                        qryInput.pixelMode = ePixMode_NONE;

                        if(NPIPE_GET_CAPIBILITY(_op, qryInput, *pOut) == MFALSE) {
                            return MFALSE;
                        }
                        break;
                    default:
                        break;
                }

                //
                vCamIOOutPorts.push_back(pOut);

                PIPE_INF("OutPorts[%d] Crop=[%d,%d,%d,%d], sz=(%d,%d), Stride=(%d/%d/%d)",\
                        pOut->index, pOut->crop1.x, pOut->crop1.y, pOut->crop1.w, pOut->crop1.h,\
                        pOut->u4ImgWidth, pOut->u4ImgHeight,pOut->u4Stride[ePlane_1st],
                        pOut->u4Stride[ePlane_2nd],pOut->u4Stride[ePlane_3rd]);
            }
        }

        switch (_NORMALPIPE_GET_TG_IDX((mpSensorIdx))) {
        case CAM_TG_1:
        case CAM_TG_2:
        case CAM_TG_3:
            if (vInPorts.m_UniLinkSel == E_UNI_LINK_AUTO) {
                if (!(mEnablePath & (NPIPE_CAM_A|NPIPE_CAM_B))) {
                    _LinkToUni = MTRUE;
                }
                if (_NORMALPIPE_GET_TG_IDX(mpSensorIdx) == mUniLinkTG) {
                    _LinkToUni = MTRUE;
                }
            }
            else {
                PIPE_WRN("uni only support auto mode. %d", vInPorts.m_UniLinkSel);
            }

            if (CAM_TG_1 == _NORMALPIPE_GET_TG_IDX(mpSensorIdx)) {
                _path = NPIPE_CAM_A;
            }
            else if (CAM_TG_2 == _NORMALPIPE_GET_TG_IDX(mpSensorIdx)) {
                _path = NPIPE_CAM_B;
            }
            else {
                _path = NPIPE_CAM_C;
            }
            break;
        default:
            PIPE_ERR("unkown TG %d", _NORMALPIPE_GET_TG_IDX((mpSensorIdx)));
            return MFALSE;
        }

        //tempory code, wait for me15 remove m_bOffBin

        switch(vInPorts.m_IQlv){
            case eCamIQ_L:
                linkInfo.offBin = _InFunc.Bits.bin_off = MFALSE;
                break;
            case eCamIQ_H:
                linkInfo.offBin = _InFunc.Bits.bin_off = MTRUE;
                break;
            case eCamIQ_MAX:
                /*if m_IQlv is init value, means it's ISP 4.x*/
                linkInfo.offBin = _InFunc.Bits.bin_off = vInPorts.m_bOffBin;
                if(vInPorts.m_Func.Bits.OFF_BIN) {
                    linkInfo.offBin = _InFunc.Bits.bin_off = vInPorts.m_Func.Bits.OFF_BIN;
                }
                break;
            default:
                PIPE_ERR("unsupport IQlv(%d)", (MUINT32)vInPorts.m_IQlv);
                return MFALSE;
                break;
        }

        _InFunc.Bits.TWINOFF = camResConfig.Bits.isOffTwin;
        _InFunc.Bits.SUBSAMPLE = burstQnum - 1;
        _InFunc.Bits.DATA_PATTERN = vInPorts.m_Func.Bits.DATA_PATTERN;
        if((vInPorts.m_Func.Bits.DATA_PATTERN == eCAM_NORMAL) &&
           (NormalPipe::pAllSensorInfo->mSenInfo[SIdx].mSInfo.PDAF_Support == PDAF_SUPPORT_RAW)){
            _InFunc.Bits.DATA_PATTERN = eCAM_NORMAL_PD;
            PIPE_DBG("data pattern = Normal_PD\n");
        }
        _InFunc.Bits.DCIF_En = (eCamDataPath_DCIF == this->m_dataPath) ? 1 : 0;

        if (!(mEnablePath & (NPIPE_CAM_A|NPIPE_CAM_B))) {
            PIPE_DBG("HW path config queue init");
            NormalPipe::m_pathCfgQueue.init();

            NormalPipe::mFirstConfgCam = _NORMALPIPE_GET_TG_IDX(mpSensorIdx);

            NormalPipe::m_occupyingCtlPath = NPIPE_CAM_NONE;

            NormalPipe::m_pathCfg_fsm = eCamHwPathFsm_Applied;

            NormalPipe::sIQData.IQNotify = NULL;

            switch(vInPorts.m_Func.Bits.SensorNum){
            case E_1_SEN:
                 NormalPipe::m_pathCfg_TGnum = eCamHwPathCfg_One_TG;
                break;
            case E_2_SEN:
                 NormalPipe::m_pathCfg_TGnum = eCamHwPathCfg_Two_TG;
                break;
            default:
                PIPE_ERR("N:%d Unsupported 1st cfg sensorNum:%d", mpSensorIdx, vInPorts.m_Func.Bits.SensorNum);
                return MFALSE;
            }
        }

        switch(NormalPipe::m_pathCfg_TGnum){
        case eCamHwPathCfg_One_TG:
            _InFunc.Bits.SensorNum = ECamio_1_SEN;
            break;
        case eCamHwPathCfg_Two_TG:
            _InFunc.Bits.SensorNum = ECamio_2_SEN;
            break;
        default:
            PIPE_ERR("N:%d max input sensor is E_2_SEN. (%d) pathStatus:%d EnablePath:0x%x",
                mpSensorIdx, vInPorts.m_Func.Bits.SensorNum, NormalPipe::m_pathCfg_TGnum, mEnablePath);
            return MFALSE;
            break;
        }

        //cfg Dynamic twin mode (only mt6763/mt6758/mt6775 support this Scenario-based ON/OFF)
        {
            PortInfo dummy;
            if(vInPorts.m_DynamicTwin == MTRUE){
                if(NPIPE_GET_CAPIBILITY(ENPipeQueryCmd_D_Twin, qryInput, dummy)) {
                    _InFunc.Bits.DynamicTwin = MTRUE;
                    PIPE_INF("enable Dynamic twin mode\n");
                }
                else{
                    PIPE_ERR("enable dynamic twin fail, running in non-D-Twin mode\n");
                }
            }
            else{
                _InFunc.Bits.DynamicTwin = MFALSE;
                PIPE_INF("running in non-D-Twin mode\n");
            }
        }

        PIPE_TRACE_BEGIN("TAG_CAM_CONFIG");

        linkInfo.rrz_out_w = rrz_w;
        PIPE_INF("Bfc NLink, mpSenIdx(%d), mTgIdx(%d), mTgSize(%d/%d), pix_mode_tg(%d), raw_fps(%d), SrcImgFmt(%d), bypass_rrzo(%d), offBin(%d), offTwin(%d), pattern(%d), MIPI_pixrate(%d), rrz_w(%d)\n",
            linkInfo.mpSenIdx,
            linkInfo.mTgIdx,
            linkInfo.mTgSize.w,
            linkInfo.mTgSize.h,
            linkInfo.pix_mode_tg,
            linkInfo.raw_fps,
            linkInfo.SrcImgFmt,
            linkInfo.bypass_rrzo,
            linkInfo.offBin,
            linkInfo.offTwin,
            linkInfo.pattern,
            linkInfo.MIPI_pixrate,
            linkInfo.rrz_out_w);

        sendcmdBufCtrl(BufCtrlCenter::BUF_CTRL_CMD_SET_NLINK, (MUINTPTR)&linkInfo, (MUINTPTR)&cam_path, 0);
        if (MFALSE == mpCamIOPipe->configPipe(vCamIOInPorts, vCamIOOutPorts, &_InFunc, &_OutFunc)) {
            PIPE_ERR("CamIOPipe configPipe Fail");
            return MFALSE;
        }
        PIPE_TRACE_END("TAG_CAM_CONFIG");

        if (eCamDataPath_DCIF == this->m_dataPath){//para of SV here is due to camio's cmdq will be borrowed by sv-camio......
            PIPE_TRACE_BEGIN("TAG_CAMSV_CONFIG");

            if (MFALSE == mpCamSVIOPipe->configPipe(vCamSVIOInPorts, vCamSVIOOutPorts, &_SVInFunc, &_SVOutFunc)) {
                PIPE_ERR("CamIOPipe configPipe Fail");
                return MFALSE;
            }

            PIPE_TRACE_END("TAG_CAMSV_CONFIG");
        }

        /*Get IQ default value*/
        NormalPipe::sIQData.IQInfo.lv[_path] = vInPorts.m_IQlv;
        if (TargetTG == TG_A)
            NormalPipe::sIQData.IQInfo.sensoridx[0] = mpSensorIdx;
        else
            NormalPipe::sIQData.IQInfo.sensoridx[1] = mpSensorIdx;

        //check TG cfg is valid when mEnablePath is not 0
        if(mEnablePath & (NPIPE_CAM_A|NPIPE_CAM_B))
        {
            MBOOL arg2 = MFALSE;
            this->mpCamIOPipe->sendCommand(EPipeCmd_DTwin_INFO,0,(MINTPTR)&arg2,0);
            if(arg2){
                if ((NormalPipe::m_pathCfg_TGnum != eCamHwPathCfg_Two_TG) ||
                    (vInPorts.m_Func.Bits.SensorNum != E_2_SEN)) {
                    PIPE_ERR("N:%d Cam HW path config un-sync org:%d cur:%d",
                        mpSensorIdx, NormalPipe::m_pathCfg_TGnum, vInPorts.m_Func.Bits.SensorNum);
                    return MFALSE;
                }
            }
        }

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
        mEnablePath     |= _path;
        mBurstQNum      = burstQnum;
        mSVRawBurstNum  = vInPorts.m_Func.Bits.SVRawBurstNum;
        m_DropCB        = vInPorts.m_DropCB;
        m_returnCookie  = vInPorts.m_returnCookie;

        if (_LinkToUni) {
            PIPE_INF("EnablePath: x%x %s-link UNI [%s]", mEnablePath,
                ((CAM_TG_NONE == mUniLinkTG)? "1st": "re"),
                ((vInPorts.m_UniLinkSel == E_UNI_LINK_AUTO)? "auto": "manual"));
        }
        else {
            PIPE_INF("EnablePath: x%x no UNI [%s]", mEnablePath,
                ((vInPorts.m_UniLinkSel == E_UNI_LINK_AUTO)? "auto": "manual"));
        }

        if (_LinkToUni) {
            mUniLinkTG = _NORMALPIPE_GET_TG_IDX((mpSensorIdx));

            mpCamIOPipe->sendCommand(EPIPECmd_ALLOC_UNI, 0, 0, 0);
        }

        for (MUINT32 PlaneID = 0; PlaneID <= (MUINT32)ePlane_3rd; PlaneID++) {
            mImgHeaderPool.ImemBufInfo[PlaneID].size = 0;
            mImgSVHeaderPool.ImemBufInfo[PlaneID].size = 0;
            for (MUINT32 headersize = 0, i = 0; i < vInPorts.mPortInfo.size(); i++) {
                /*Image Header buffer allocation*/
                if(vInPorts.mPortInfo.at(i).mPortID.index == EPortIndex_CAMSV_IMGO) {
                    this->mpCamSVIOPipe->sendCommand(EPIPECmd_GET_HEADER_SIZE,(MINTPTR)vInPorts.mPortInfo.at(i).mPortID.index,(MINTPTR)&headersize,0);
                    imgHdrBufNum = IMG_HEADER_BUF_NUM * mBurstQNum * mSVRawBurstNum;

                    mImgSVHeaderPool.ImemBufInfo[PlaneID].size += (headersize * imgHdrBufNum);
                    mImgSVHeaderPool.ImemBufInfo[PlaneID].size += (15 * imgHdrBufNum); //allocate extra memory for 16 bytes alignment
                }
                else {
                    this->mpCamIOPipe->sendCommand(EPIPECmd_GET_HEADER_SIZE,(MINTPTR)vInPorts.mPortInfo.at(i).mPortID.index,(MINTPTR)&headersize,0);
                    imgHdrBufNum = IMG_HEADER_BUF_NUM * mBurstQNum;

                    mImgHeaderPool.ImemBufInfo[PlaneID].size += (headersize * imgHdrBufNum);
                    mImgHeaderPool.ImemBufInfo[PlaneID].size += (15 * imgHdrBufNum); //allocate extra memory for 16 bytes alignment
                }
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

            if (eCamDataPath_DCIF == this->m_dataPath) {
                mImgSVHeaderPool.ImemBufInfo[PlaneID].useNoncache = 1;
                if (mpIMem->allocVirtBuf(&mImgSVHeaderPool.ImemBufInfo[PlaneID]) < 0) {
                    PIPE_ERR("imgSVHeader[%d] alloc %d fail", PlaneID, mImgSVHeaderPool.ImemBufInfo[PlaneID].size);
                    return MFALSE;
                }
                if (mpIMem->mapPhyAddr(&mImgSVHeaderPool.ImemBufInfo[PlaneID], MFALSE) < 0) {
                    PIPE_ERR("imgSVHeader[%d] map %d fail", PlaneID, mImgSVHeaderPool.ImemBufInfo[PlaneID].size);
                    return MFALSE;
                }
            }
        }

        /* image header buffer to each port*/

        for (MUINT32 headersize = 0,i = 0; i < vInPorts.mPortInfo.size(); i++) {
        //
            MUINT32 _port = NPIPE_MAP_PORTID(vInPorts.mPortInfo.at(i).mPortID.index);
            mpImgHeaderMgr[_port].init();
            if(vInPorts.mPortInfo.at(i).mPortID.index == EPortIndex_CAMSV_IMGO) {
                this->mpCamSVIOPipe->sendCommand(EPIPECmd_GET_HEADER_SIZE,(MINTPTR)vInPorts.mPortInfo.at(i).mPortID.index,(MINTPTR)&headersize,0);
                imgHdrBufNum = IMG_HEADER_BUF_NUM * mBurstQNum * mSVRawBurstNum;

                for (j = 0; j < imgHdrBufNum; j++) {
                    NormalPipe::IMEM_MUTI_BUF_INFO iHeader;
                    for (MUINT32 PlaneID = 0; PlaneID <= (MUINT32)ePlane_3rd; PlaneID++) {
                        MUINTPTR virtBA = ALIGN_CAL(mImgSVHeaderPool.ImemBufInfo[PlaneID].virtAddr, 16);
                        MUINTPTR phyBA = ALIGN_CAL(mImgSVHeaderPool.ImemBufInfo[PlaneID].phyAddr, 16);

                        imgsvHdrAllocOfst[PlaneID]                  = ALIGN_CAL(imgsvHdrAllocOfst[PlaneID], 16);
                        iHeader.ImemBufInfo[PlaneID].size         = headersize;
                        iHeader.ImemBufInfo[PlaneID].memID        = mImgSVHeaderPool.ImemBufInfo[PlaneID].memID;
                        iHeader.ImemBufInfo[PlaneID].virtAddr     = virtBA + imgsvHdrAllocOfst[PlaneID];
                        iHeader.ImemBufInfo[PlaneID].phyAddr      = phyBA + imgsvHdrAllocOfst[PlaneID];
                        iHeader.ImemBufInfo[PlaneID].bufSecu      = mImgSVHeaderPool.ImemBufInfo[PlaneID].bufSecu;
                        iHeader.ImemBufInfo[PlaneID].bufCohe      = mImgSVHeaderPool.ImemBufInfo[PlaneID].bufCohe;
                        iHeader.ImemBufInfo[PlaneID].useNoncache  = mImgSVHeaderPool.ImemBufInfo[PlaneID].useNoncache;
                        imgsvHdrAllocOfst[PlaneID]                 += headersize;
                    }
                    mpImgHeaderMgr[_port].sendCmd(QueueMgr<NormalPipe::IMEM_MUTI_BUF_INFO>::eCmd_push, (MUINTPTR)&iHeader, 0);
                    mImgHdrLock.lock();
                    mImgHdrCnt[_port]++;
                    mImgHdrLock.unlock();
                }
            }
            else {
                this->mpCamIOPipe->sendCommand(EPIPECmd_GET_HEADER_SIZE,(MINTPTR)vInPorts.mPortInfo.at(i).mPortID.index,(MINTPTR)&headersize,0);
                imgHdrBufNum = IMG_HEADER_BUF_NUM * mBurstQNum;
                NormalPipe::IMEM_MUTI_BUF_INFO iHeader;
                for (j = 0; j < imgHdrBufNum; j++) {
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
                        if (_port != ePort_svimgo)
                            imgHdrAllocOfst[PlaneID]                 += headersize;
                    }
                    mpImgHeaderMgr[_port].sendCmd(QueueMgr<NormalPipe::IMEM_MUTI_BUF_INFO>::eCmd_push, (MUINTPTR)&iHeader, 0);
                    mImgHdrLock.lock();
                    mImgHdrCnt[_port]++;
                    mImgHdrLock.unlock();
                    //For secure camera
                    if(vInPorts.mPortInfo.at(i).mSecon && (vInPorts.m_SecureCam.buf_type == E_SEC_LEVEL_SECURE || vInPorts.m_SecureCam.buf_type == E_SEC_LEVEL_PROTECTED)){
                        SecMgr_SecInfo secinfo;
                        secinfo.type = SECMEM_FRAME_HEADER;
                        secinfo.module = (MUINT32)TargetTG;
                        secinfo.buff_size = iHeader.ImemBufInfo[0].size;
                        secinfo.buff_va = (uint64_t)iHeader.ImemBufInfo[0].virtAddr;
                        secinfo.port = vInPorts.mPortInfo.at(i).mPortID.index;
                        secinfo.memID = iHeader.ImemBufInfo[0].memID;
                        PIPE_INF("Register FH - size:0x%x VA:0x%llx PA:0x%x memID:0x%x",
                        iHeader.ImemBufInfo[0].size,iHeader.ImemBufInfo[0].virtAddr,iHeader.ImemBufInfo[0].phyAddr,iHeader.ImemBufInfo[0].memID);
                        if(this->mpSecMgr->SecMgr_P1RegisterShareMem(&secinfo) == MFALSE){
                            PIPE_ERR("Register FH ShareMem failed(0x%x_0x%llx)",vInPorts.mPortInfo.at(i).mPortID.index,iHeader.ImemBufInfo[0].virtAddr);
                            return MFALSE;
                        }
                    }
                }


            }


        }

        if (this->GetOpenedPortInfo(NormalPipe::_GetPort_OpendNum) == 0) {
            PIPE_ERR("error:no dmao enabled\n");
            return MFALSE;
        }
        PIPE_DBG("DropCB(0x%p)/Cookie(0x%p) DynamicRawType(%d) PureRaw(%d)", m_DropCB, m_returnCookie,
                mDynSwtRawType, mPureRaw);

        mpCamIOPipe->sendCommand(EPIPECmd_SET_FRM_TIME, (MINTPTR)0, 0, 0);
        mpCamIOPipe->sendCommand(EPIPECmd_SET_FRM_TIME, (MINTPTR)0, 1, 0);
        mpNPipeThread->updateFrameTime(0, 0);
        mpNPipeThread->updateFrameTime(0, 1);

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
MINT32 NormalPipe::attach(const char* UserName)
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
        PIPE_ERR("CamIOPipe not create");
    }

    return -1;
}


/******************************************************************************
 *
 ******************************************************************************/
MBOOL NormalPipe::wait(EPipeSignal eSignal, EPipeSignalClearType eClear, const MINT32 mUserKey,
                        MUINT32 TimeoutMs, SignalTimeInfo *pTime)
{
    Irq_t irq;
    MBOOL ret = MTRUE;

    PIPE_DBG("+");

    if (this->FSM_CHECK(op_cmd, __FUNCTION__) == MFALSE) {
        return MFALSE;
    }

    irq.Type = (eClear == EPipeSignal_ClearNone)? Irq_t::_CLEAR_NONE: Irq_t::_CLEAR_WAIT;
    ret = NPIPE_MAP_SIGNAL(eSignal, irq.Status, irq.StatusType);
    if (!ret) {
        PIPE_ERR("Signal x%x not support", eSignal);
        return ret;
    }

    irq.UserKey = mUserKey;
    irq.Timeout = TimeoutMs; //ms
    switch(NPIPE_MAP_CAM_TG(mpSensorIdx)){
        case TG_A:
            irq.IrqType = ISP_IRQ_TYPE_INT_CAM_A_ST;
            break;
        case TG_B:
            irq.IrqType = ISP_IRQ_TYPE_INT_CAM_B_ST;
            break;
        case TG_C:
            irq.IrqType = ISP_IRQ_TYPE_INT_CAM_C_ST;
            break;
        default:
            PIPE_ERR("un-support tg index:%d_%d", mpSensorIdx, NPIPE_MAP_CAM_TG(mpSensorIdx));
            break;
    }

    if ((eCamDataPath_DCIF == this->m_dataPath) && (EPipeSignal_AFDONE != eSignal)) {
        if (mpCamSVIOPipe) {
            ret = mpCamSVIOPipe->irq(&irq);
            if (!ret) {
                PIPE_ERR("Wait SV Signal Fail(%d,%d,x%x,%d)", irq.UserKey, irq.Type, irq.Status, irq.Timeout);
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
            PIPE_ERR("CamSVIOPipe not create");
        }
    }
    else {
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
    }

    PIPE_DBG("-");

    return ret;
}

MBOOL NormalPipe::signal(EPipeSignal eSignal, const MINT32 mUserKey)
{
    Irq_t irq;
    MBOOL ret = MTRUE;

    PIPE_DBG("+");

    if (this->FSM_CHECK(op_cmd, __FUNCTION__) == MFALSE) {
        return MFALSE;
    }

    ret = NPIPE_MAP_SIGNAL(eSignal, irq.Status, irq.StatusType);
    if (!ret) {
        PIPE_ERR("Signal x%x not support", eSignal);
        return ret;
    }

    if(mUserKey < 1){
        PIPE_ERR("invalid userkey(%d)", mUserKey);
        return ret;
    }

    irq.UserKey = mUserKey;
    switch(NPIPE_MAP_CAM_TG(mpSensorIdx)){
        case TG_A:
            irq.IrqType = ISP_IRQ_TYPE_INT_CAM_A_ST;
            break;
        case TG_B:
            irq.IrqType = ISP_IRQ_TYPE_INT_CAM_B_ST;
            break;
        case TG_C:
            irq.IrqType = ISP_IRQ_TYPE_INT_CAM_C_ST;
            break;
        default:
            PIPE_ERR("un-support tg index:%d_%d", mpSensorIdx, NPIPE_MAP_CAM_TG(mpSensorIdx));
            break;
    }

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

MBOOL NormalPipe::suspend(E_SUSPEND_TPYE etype)
{
    Mutex::Autolock lock(mCfgLock);
    MBOOL ret = MTRUE, stream_on = MFALSE;
    MUINT32 _OpenedPort = this->GetOpenedPortInfo(NormalPipe::_GetPort_OpendNum), i,p,size,port;
    QBufInfo _qBuf;
    NPIPE_PATH_E    cam_path = NPIPE_CAM_MAX;

    cam_path = NPIPE_MAP_PATH(mpSensorIdx);
    if (MFALSE == this->updateBufCtrlFSM(NormalPipe::BufCtrl_suspend, cam_path)) {
        PIPE_ERR("Cannot suspend during IQ set!\n");
        return MFALSE;
    }

    if (this->FSM_CHECK(op_suspend, __FUNCTION__) == MFALSE) {
        PIPE_ERR("N:%d suspend can only be trigger under streaming", this->mpSensorIdx);
        return MFALSE;
    }

    PIPE_INF("N:%d Holding idle locks before suspend", this->mpSensorIdx);

    //lock only enqThres, deqThres will be locked via semophore which is posted by enqThres
    //in order to make sure all enque request being removed from driver
    this->mpNPipeThread->idleStateHold(1, NormalPipe_Thread::eThdGrp_Enq);
    /* Not holding stt locks for 3a threads to error return when suspend
     * NormalPipe enq/deq threads will blocking at mutex lock
     */
    this->mpNPipeThread->idleStateHold(1, NormalPipe_Thread::eThdGrp_Deq);

    if (eCamDataPath_DCIF == this->m_dataPath) {
        ret = this->mpCamSVIOPipe->suspend(ICamIOPipe::SW_SUSPEND);
    }
    ret = this->mpCamIOPipe->suspend(ICamIOPipe::SW_SUSPEND);

    this->mpNPipeThread->idleStateHold(0, NormalPipe_Thread::eThdGrp_Deq);

    if (this->mpNPipeThread->suspendNotifyCB.fpNotifyState) {
        if(this->mpNPipeThread->suspendNotifyCB.fpNotifyState(CAM_ENTER_SUSPEND, this->mpNPipeThread->suspendNotifyCB.Obj)
            == NOTIFY_FAIL){
            PIPE_ERR("N:%d suspend fail", this->mpSensorIdx);
            return MFALSE;
        }
    }

    PIPE_INF("N:%d type:%d DevId:0x%x p1 suspend done, release deqThd lock and flushing sof signal",
        this->mpSensorIdx,(MUINT32)etype, _NORMALPIPE_GET_SENSOR_DEV_ID(this->mpSensorIdx));

    this->signal(EPipeSignal_SOF, mpNPipeThread->m_irqUserKey); //flush SOF signal, since irqUserKey is here

    for (i = 0; i < _OpenedPort; i++) {
        ::sem_post(&this->mpNPipeThread->m_semDeqDonePort[NPIPE_MAP_PORTID(this->GetOpenedPortInfo(NormalPipe::_GetPort_Index, i))]);
    }

    switch(etype){
    case eSuspend_Type1:
        break;
    case eSuspend_Type2:
        if(mRawType == 1){
            mpHalSensor->sendCommand(_NORMALPIPE_GET_SENSOR_DEV_ID(this->mpSensorIdx), SENSOR_CMD_SET_TEST_MODEL, (MUINTPTR)&stream_on, 0, 0);
        }
        else{
            mpHalSensor->sendCommand(_NORMALPIPE_GET_SENSOR_DEV_ID(this->mpSensorIdx), SENSOR_CMD_SET_STREAMING_SUSPEND, 0, 0, 0);
        }
        break;
    default:
        PIPE_INF("unknow suspend type(%d)", (MUINT32)etype);
        break;
    }

    //drop request in enquecontainer
    this->mEnqContainerLock.lock();

    for (p = 0; p < this->GetOpenedPortInfo(NormalPipe::_GetPort_OpendNum); p++) {
        port = NPIPE_MAP_PORTID(this->GetOpenedPortInfo(NormalPipe::_GetPort_Index, p));
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


MBOOL NormalPipe::resume(QBufInfo const * pQBuf, E_SUSPEND_TPYE etype)
{
    Mutex::Autolock lock(mCfgLock);
    MBOOL ret = MTRUE, stream_on = MTRUE;
    E_EnqueRst retRst = eEnqueRst_OK;
    NPIPE_PATH_E cam_path = NPIPE_CAM_MAX;
    E_INPUT TargetTG;
    BufCtrlCenter::NLinkInfo* plinkInfo;

    sendcmdBufCtrl(BufCtrlCenter::BUF_CTRL_CMD_GET_CUR_NLINK, (MINTPTR)&mpCamIOPipe, (MINTPTR)&plinkInfo, 0);
    TargetTG = plinkInfo->mCurTgIdx;

    cam_path = NPIPE_MAP_PATH(mpSensorIdx);
    if (MFALSE == this->updateBufCtrlFSM(NormalPipe::BufCtrl_start, cam_path)) {
        PIPE_ERR("Cannot resume before suspend!\n");
        return MFALSE;
    }

    if (this->FSM_CHECK(op_start, __FUNCTION__) == MFALSE) {
        PIPE_ERR("N:%d suspend can only be trigger under streaming", this->mpSensorIdx);
        return MFALSE;
    }
    if (pQBuf == NULL) {
        PIPE_ERR("N:%d pQBuf null, MUST enque", this->mpSensorIdx);
        return MFALSE;
    }

    MUINT32     _shuttertime_us = (MUINT32)(pQBuf->mShutterTimeNs/1000);

    PIPE_INF("N:%d type:%d DevId:0x%x Release idle locks and resume shutter_us:%d",
        this->mpSensorIdx, etype, _NORMALPIPE_GET_SENSOR_DEV_ID(this->mpSensorIdx), _shuttertime_us);

    mpHalSensor->sendCommand(_NORMALPIPE_GET_SENSOR_DEV_ID(this->mpSensorIdx),
                                       SENSOR_CMD_SET_SENSOR_EXP_TIME, (MUINTPTR)&_shuttertime_us, 0, 0);

    switch(etype){
    case eSuspend_Type1:
        break;
    case eSuspend_Type2:
        if(mRawType == 1){
            mpHalSensor->sendCommand(_NORMALPIPE_GET_SENSOR_DEV_ID(this->mpSensorIdx),
                                        SENSOR_CMD_SET_TEST_MODEL, (MUINTPTR)&stream_on, 0, 0);
        }
        else{
            mpHalSensor->sendCommand(_NORMALPIPE_GET_SENSOR_DEV_ID(this->mpSensorIdx),
                                        SENSOR_CMD_SET_STREAMING_RESUME, (MUINTPTR)&_shuttertime_us, 0, 0);
        }
        break;
    default:
        PIPE_INF("unknow suspend type(%d)", (MUINT32)etype);
        break;
    }

    for(MUINT32 i = 0 ; i < ePort_max ; i++){
        MUINT32 queSize;
        mpVirPortPA[i]->sendCmd(QueueMgr<MUINT32>::eCmd_getsize, (MUINTPTR)&queSize, 0);
        for(MUINT32 j = 0 ; j < queSize; j++){
            mpVirPortPA[i]->sendCmd(QueueMgr<MUINT32>::eCmd_pop, 0, 0);
        }
    }

    PIPE_INF("N:%d holding deque thread", this->mpSensorIdx);
    this->mpNPipeThread->idleStateHold(1, NormalPipe_Thread::eThdGrp_Deq);

    ret = this->mpCamIOPipe->preresume((MINT8*)this->mpName, _NORMALPIPE_GET_SENSORCFG_CROP_W(this->mpSensorIdx), TargetTG);
    if(ret == MFALSE){
        PIPE_ERR("N: preresume NG\n");
        return ret;
    }

    if (eCamDataPath_DCIF == this->m_dataPath) {
        ret = this->mpCamSVIOPipe->preresume((MINT8*)this->mpName, _NORMALPIPE_GET_SENSORCFG_CROP_W(this->mpSensorIdx));
        if(ret == MFALSE){
            PIPE_ERR("NSV: preresume NG\n");
            return ret;
        }
    }

    ret = this->acceptEnqRequest(*pQBuf);
    if (ret == MFALSE) {
        PIPE_ERR("N:%d enqueReq of resume NG\n", this->mpSensorIdx);
        return ret;
    }

    if (this->mpNPipeThread->suspendNotifyCB.fpNotifyState) {
        if(this->mpNPipeThread->suspendNotifyCB.fpNotifyState(CAM_EXIT_SUSPEND, this->mpNPipeThread->suspendNotifyCB.Obj)
            == NOTIFY_FAIL){
            PIPE_ERR("N:%d resume fail", this->mpSensorIdx);
            return MFALSE;
        }
    }

    retRst = this->mpNPipeThread->frameToEnque();
    if (retRst != eEnqueRst_OK) {
        PIPE_ERR("N:%d frameToEnque of resume NG stat:0x%x\n", this->mpSensorIdx, retRst);
        return ret;
    }

    ret = this->mpCamIOPipe->resume(ICamIOPipe::SW_SUSPEND);
    if (eCamDataPath_DCIF == this->m_dataPath) {
        ret = this->mpCamSVIOPipe->resume(ICamIOPipe::SW_SUSPEND);
    }

    this->FSM_UPDATE(op_start);

    this->mpNPipeThread->idleStateHold(0, NormalPipe_Thread::eThdGrp_Deq);
    //only unlock enqThres, plz refer to suspend() for rationale
    this->mpNPipeThread->idleStateHold(0, NormalPipe_Thread::eThdGrp_Enq);

    return ret;
}

MBOOL NormalPipe::resume(MUINT64 shutterTimeUs, E_SUSPEND_TPYE etype)
{
    Mutex::Autolock lock(mCfgLock);
    MBOOL ret = MTRUE, stream_on = MTRUE;
    E_EnqueRst retRst = eEnqueRst_OK;
    MUINT32 _shuttertime_us = (MUINT32)shutterTimeUs;
    E_INPUT TargetTG;
    BufCtrlCenter::NLinkInfo* plinkInfo;

    sendcmdBufCtrl(BufCtrlCenter::BUF_CTRL_CMD_GET_CUR_NLINK, (MINTPTR)&mpCamIOPipe, (MINTPTR)&plinkInfo, 0);
    TargetTG = plinkInfo->mCurTgIdx;

    if (this->FSM_CHECK(op_start, __FUNCTION__) == MFALSE) {
        PIPE_ERR("N:%d suspend can only be trigger under streaming", this->mpSensorIdx);
        return MFALSE;
    }

    PIPE_INF("N:%d type:%d DevId:0x%x Release idle locks and resume shutter_us:%d v0",
        this->mpSensorIdx, etype, _NORMALPIPE_GET_SENSOR_DEV_ID(this->mpSensorIdx), _shuttertime_us);

    mpHalSensor->sendCommand(_NORMALPIPE_GET_SENSOR_DEV_ID(this->mpSensorIdx),
                                               SENSOR_CMD_SET_SENSOR_EXP_TIME, (MUINTPTR)&_shuttertime_us, 0, 0);

    switch(etype){
    case eSuspend_Type1:
        break;
    case eSuspend_Type2:
        if(mRawType == 1){
            mpHalSensor->sendCommand(_NORMALPIPE_GET_SENSOR_DEV_ID(this->mpSensorIdx),
                                        SENSOR_CMD_SET_TEST_MODEL, (MUINTPTR)&stream_on, 0, 0);
        }
        else{
            mpHalSensor->sendCommand(_NORMALPIPE_GET_SENSOR_DEV_ID(this->mpSensorIdx),
                                        SENSOR_CMD_SET_STREAMING_RESUME, (MUINTPTR)&_shuttertime_us, 0, 0);
        }
        break;
    default:
        PIPE_INF("unknow suspend type(%d)", (MUINT32)etype);
        break;
    }

    PIPE_INF("N:%d holding deque thread v0", this->mpSensorIdx);
    this->mpNPipeThread->idleStateHold(1, NormalPipe_Thread::eThdGrp_Deq);

    ret = this->mpCamIOPipe->preresume((MINT8*)this->mpName, _NORMALPIPE_GET_SENSORCFG_CROP_W(this->mpSensorIdx), TargetTG);
    if(ret == MFALSE){
        PIPE_ERR("N: preresume NG\n");
        return ret;
    }

    ret = this->mpCamIOPipe->resume(ICamIOPipe::SW_SUSPEND);

    this->FSM_UPDATE(op_start);

    this->mpNPipeThread->idleStateHold(0, NormalPipe_Thread::eThdGrp_Deq);
    //only unlock enqThres, plz refer to suspend() for rationale
    this->mpNPipeThread->idleStateHold(0, NormalPipe_Thread::eThdGrp_Enq);

    if (this->mpNPipeThread->suspendNotifyCB.fpNotifyState) {
        if(this->mpNPipeThread->suspendNotifyCB.fpNotifyState(CAM_EXIT_SUSPEND, this->mpNPipeThread->suspendNotifyCB.Obj)
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
MBOOL NormalPipe::abortDma(PortID port, char const* szCallerName)
{
    MBOOL ret = MTRUE;
    NSImageio::NSIspio::PortID portID;

    PIPE_INF("+, usr[%s]dma[0x%x]SenIdx[%d]", szCallerName, port.index, mpSensorIdx);

    if (this->FSM_CHECK(op_cmd, __FUNCTION__) == MFALSE) {
        return MFALSE;
    }

    if ((eCamDataPath_DCIF == this->m_dataPath) && (port.index == EPortIndex_CAMSV_IMGO)) {
        if (mpCamSVIOPipe) {
            portID.index = port.index;
            ret = mpCamSVIOPipe->abortDma(portID);
            if (!ret) {
                PIPE_ERR("abort dma(0x%x) fail", port.index);
                return ret;
            }
        }
        else {
            PIPE_ERR("CamSVIOPipe not create, user(%s)", szCallerName);
            return MFALSE;
        }
    }

    if (mpCamIOPipe) {
        switch (port.index){
        case EPortIndex_RRZO:
        case EPortIndex_IMGO:
        case EPortIndex_YUVO:
        case EPortIndex_CRZO:
        case EPortIndex_CRZO_R2:
        case EPortIndex_LCSO:
        case EPortIndex_EISO:
        case EPortIndex_RSSO:
        case EPortIndex_RAWI:
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
MUINT32 NormalPipe::getIspReg(MUINT32 RegAddr, MUINT32 RegCount, MUINT32 RegData[], MBOOL bPhysical)
{
    PIPE_DBG("+");

    if (this->FSM_CHECK(op_cmd, __FUNCTION__) == MFALSE) {
        return MFALSE;
    }

    if (NULL == RegData) {
        PIPE_ERR("Wrong out buf");
        return MFALSE;
    }

    if (mpCamIOPipe) {
        IspP1RegIo pRegs[RegCount];
        MUINT32 getRaw = 0x0; /* default: get single cam */
        MUINT32 i = 0;

        for (i = 0; i < RegCount; i++) {
            pRegs[i].Addr = RegAddr + (i << 2);
        }
        mpCamIOPipe->ReadReg((MINTPTR)pRegs, RegCount, getRaw, bPhysical);

        for (i = 0; i < RegCount; i++) {
            RegData[i] = pRegs[i].Data;
        }
    }
    else {
        return MFALSE;
    }

    return MTRUE;
}

/******************************************************************************
 *
 ******************************************************************************/
MUINT32 NormalPipe::getIspReg(IspP1RegIo *pRegs, MUINT32 RegCount, MBOOL bPhysical)
{
    PIPE_DBG("+");

    if (this->FSM_CHECK(op_cmd, __FUNCTION__) == MFALSE) {
        return MFALSE;
    }

    if (NULL == pRegs) {
        PIPE_ERR("NULL param");
        return MFALSE;
    }

    if (mpCamIOPipe) {
        MUINT32 getRaw = 0x0; /* default: get single cam */
        mpCamIOPipe->ReadReg((MINTPTR)pRegs, RegCount, getRaw, bPhysical);
    } else {
        return MFALSE;
    }

    return MTRUE;
}

/******************************************************************************
 *
 ******************************************************************************/
MUINT32 NormalPipe::getIspReg(IspP1RegIo *pRegs, MUINT32 RegCount, MUINT32 getRaw, MBOOL bPhysical)
{
    PIPE_DBG("+");

    if (this->FSM_CHECK(op_cmd, __FUNCTION__) == MFALSE) {
        return MFALSE;
    }

    if (NULL == pRegs) {
        PIPE_ERR("NULL param");
        return MFALSE;
    }

    if (mpCamIOPipe) {
        mpCamIOPipe->ReadReg((MINTPTR)pRegs, RegCount, getRaw, bPhysical);
    } else {
        return MFALSE;
    }

    return MTRUE;
}

/******************************************************************************
 *
 ******************************************************************************/
MUINT32 NormalPipe::getIspUniReg(IspP1RegIo *pRegs, MUINT32 RegCount, MBOOL bPhysical)
{
    PIPE_DBG("+");
    PIPE_ERR("IspUniReg is not supported\n");
    return MFALSE;
}

/******************************************************************************
 *
 ******************************************************************************/
MBOOL NormalPipe::sendCommand(MINT32 cmd, MINTPTR arg1, MINTPTR arg2, MINTPTR arg3)
{
    MBOOL   ret = MTRUE, ret1 = MTRUE, ret2 = MTRUE;
    MUINT32 tg;
    MUINT32 cur_cam;
    MINT32  sensorDev = 1;
    NormalPipe_HBIN_Info    *hbin = NULL;
    NormalPipe_EIS_Info     *pEis = NULL;
    NormalPipe_RSS_Info     *pRss = NULL;

    MUINT32 CamPipeStatus = 0, SVPipeStatus = 0;
    NSImageio::NSIspio::ICamIOPipe* pCamIOPipe;

    NIOGLock.lock();
    pCamIOPipe = GetCamIOPipeInstance(mpSensorIdx);
    NIOGLock.unlock();

    if (this->FSM_CHECK(op_cmd, __FUNCTION__) == MFALSE) {
        ret = MFALSE;
        goto _EXIT_SEND_CMD;
    }

    if (!pCamIOPipe) {
        PIPE_ERR("CamIOPipe NULL");
        ret = MFALSE;
        goto _EXIT_SEND_CMD;
    }
    switch (cmd) {
    case ENPipeCmd_GET_TG_INDEX:
        //arg1: [Out] (MUINT32*)  CAM_TG_1/CAM_TG_2/...
        if (arg1) {
            *(MINT32*)arg1 = _NORMALPIPE_GET_TG_IDX(mpSensorIdx);
        }
        else {
            ret = MFALSE;
            goto _EXIT_SEND_CMD;
        }
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
    case ENPipeCmd_SET_STT_SOF_CB:
        mEnqSofCBLock.lock();
        if (m_SofCB) {
            PIPE_ERR("SOF callback already registered!");
            ret = MFALSE;
            mEnqSofCBLock.unlock();
            goto _EXIT_SEND_CMD;
        }
        m_SofCB = (fp_EventCB)arg1;
        m_SofCBCookie = (void*)arg2;
        mEnqSofCBLock.unlock();
        break;
    case ENPipeCmd_CLR_STT_SOF_CB:
        mEnqSofCBLock.lock();
        if (m_SofCB != (fp_EventCB)arg1) {
            PIPE_ERR("SOF CBFP un-reg wrong!");
            ret = MFALSE;
            mEnqSofCBLock.unlock();
            goto _EXIT_SEND_CMD;
        }
        m_SofCB = NULL;
        mEnqSofCBLock.unlock();
        break;
    case ENPipeCmd_GET_LAST_ENQ_SOF:
        //arg1: [Out] last enque sof_idx
        if (!arg1) {
            PIPE_ERR("NULL param");
            ret = MFALSE;
            goto _EXIT_SEND_CMD;
        }
        *((MUINT32*)arg1) = mpNPipeThread->m_LastEnqSof;
        break;
    case ENPipeCmd_SET_MEM_CTRL:
        //arg1: [In]  0: allocate
        //arg2: [In] IMEM_BUF_INFO*
        //arg3: -
        BufCtrlCenter::NLinkInfo* plinkInfo;
        if (arg1 == 0) {
            if (arg2) {
                IMEM_BUF_INFO *pBuf = (IMEM_BUF_INFO*)arg2;

                if (mpIMem->allocVirtBuf(pBuf) < 0) {
                    PIPE_ERR("TG_%d alloc %d", mpSensorIdx, pBuf->size);
                    ret = MFALSE;
                    break;
                }
                if (mpIMem->mapPhyAddr(pBuf, MFALSE) < 0) {
                    PIPE_ERR("TG_%d map %d", mpSensorIdx, pBuf->size);
                    ret = MFALSE;
                    break;
                }
                sendcmdBufCtrl(BufCtrlCenter::BUF_CTRL_CMD_GET_CUR_NLINK, (MINTPTR)&pCamIOPipe, (MINTPTR)&plinkInfo, 0);
                plinkInfo->mvSttMem.push_back(*pBuf);
                PIPE_DBG("SttBuf depth: %d, cur size: %d", (MUINT32)plinkInfo->mvSttMem.size(), pBuf->size);
            }
        }
        else {
            ret = MFALSE;
        }
        break;
    case ENPipeCmd_SET_IDLE_HOLD_CB:
        if (0 == arg1) {
            ret = MFALSE;
            break;
        }

        mpNPipeThread->idleHoldNotifyCB = *(CAM_STATE_NOTIFY *)arg1;
        PIPE_INF("set idle hold cb: %p", mpNPipeThread->idleHoldNotifyCB.fpNotifyState);

        break;
    case ENPipeCmd_SET_SUSPEND_STAT_CB:
        if (0 == arg1) {
            ret = MFALSE;
            PIPE_ERR("arg1 ptr cannot be null");
            break;
        }

        mpNPipeThread->suspendNotifyCB = *(CAM_STATE_NOTIFY *)arg1;
        PIPE_INF("set suspend state cb: %p", mpNPipeThread->suspendNotifyCB.fpNotifyState);

        break;
    case ENPipeCmd_SET_STT_DSWITCH_CB:
        if (0 == arg1) {
            ret = MFALSE;
            PIPE_ERR("arg1 ptr cannot be null");
            break;
        }

        mpNPipeThread->switchNotifyCB = *(CAM_STATE_NOTIFY *)arg1;
        PIPE_INF("set suspend state cb: %p", mpNPipeThread->switchNotifyCB.fpNotifyState);

        break;
    case ENPipeCmd_SET_ENQ_THRD_CB:
        if (0 == arg1) {
            ret = MFALSE;
            PIPE_ERR("arg1 ptr cannot be null");
            break;
        }

        mpNPipeThread->enqueThrdNotifyCB = *(CAM_STATE_NOTIFY *)arg1;
        PIPE_INF("set suspend state cb: %p", mpNPipeThread->enqueThrdNotifyCB.fpNotifyState);

        break;
    case ENPipeCmd_GET_STT_CUR_BUF:
        if (!pCamIOPipe) {
            PIPE_ERR("[Error]pCamIOPipe=NULL");
            ret = MFALSE;
            goto _EXIT_SEND_CMD;
        }
        ret = pCamIOPipe->sendCommand(EPIPECmd_GET_STT_CUR_BUF, arg1, arg2, arg3);
        break;
    case ENPipeCmd_SET_STT_SUSPEND_CB:
        {
            /*Not support cmd*/
            ret = MFALSE;
            goto _EXIT_SEND_CMD;
        }
        break;
    case ENPipeCmd_SET_HW_PATH_CFG:
        {
            MUINT32         _size;
            NPIPE_PATH_E    _cur_path;
            E_CamHwPathCfg  _new_cfg = (E_CamHwPathCfg)arg1;
            Mutex::Autolock lock(NPipeCfgGLock);

            switch(_new_cfg) {
            case eCamHwPathCfg_One_TG:
            case eCamHwPathCfg_Two_TG:
                break;
            default:
                ret = MFALSE;
                break;
            }
            if (MFALSE == ret) {
                PIPE_ERR("N:%d unknow h/w path config:%d", this->mpSensorIdx, _new_cfg);
                break;
            }

            if (CAM_TG_1 == _NORMALPIPE_GET_TG_IDX(this->mpSensorIdx)) {
                _cur_path = NPIPE_CAM_A;
            }
            else if(CAM_TG_2 == _NORMALPIPE_GET_TG_IDX(this->mpSensorIdx)) {
                _cur_path = NPIPE_CAM_B;
            }
            else {
                _cur_path = NPIPE_CAM_C;
            }

            if ((NPIPE_CAM_NONE != NormalPipe::m_occupyingCtlPath) &&
                (_cur_path != NormalPipe::m_occupyingCtlPath)) {
                PIPE_ERR("N:%d cur path:0x%x not control path:0x%x", this->mpSensorIdx, _cur_path, NormalPipe::m_occupyingCtlPath);
                ret = MFALSE;
                break;
            }

            NormalPipe::m_pathCfgQueue.sendCmd(QueueMgr<E_CamHwPathCfg>::eCmd_push, (MUINTPTR)&_new_cfg, 0);
            NormalPipe::m_occupyingCtlPath = _cur_path;

            NormalPipe::m_pathCfgQueue.sendCmd(QueueMgr<E_CamHwPathCfg>::eCmd_getsize, (MUINTPTR)&_size, 0);
            PIPE_INF("N:%d HW pathConfig new_cfg:%d queueDepth:%d occupy:0x%x enablePath:0x%x",
                this->mpSensorIdx, _new_cfg, _size, NormalPipe::m_occupyingCtlPath, this->mEnablePath);
        }

        break;
    case ENPipeCmd_GET_HW_PATH_CFG:
        if (arg1) {
            Mutex::Autolock lock(NPipeCfgGLock);

            *(E_CamHwPathCfg *)arg1 = NormalPipe::m_pathCfg_TGnum;
        }
        else {
            PIPE_ERR("N:%d null arg1\n", this->mpSensorIdx);
        }
        break;
    case ENPIPECmd_AE_SMOOTH:
        if (!pCamIOPipe) {
            PIPE_ERR("[Error]pCamIOPipe=NULL");
            ret = MFALSE;
            goto _EXIT_SEND_CMD;
        }
        ret = pCamIOPipe->sendCommand(EPIPECmd_AE_SMOOTH, arg1, arg2, arg3);
        break;
    case ENPipeCmd_HIGHSPEED_AE:
        if (!pCamIOPipe) {
            PIPE_ERR("[Error]pCamIOPipe=NULL");
            ret = MFALSE;
            goto _EXIT_SEND_CMD;
        }
        ret = pCamIOPipe->sendCommand(EPIPECmd_HIGHSPEED_AE, arg1, arg2, arg3);
        break;
    case ENPipeCmd_SET_TG_INT_LINE:
        if (eCamDataPath_DCIF == this->m_dataPath) {
            if (!mpCamSVIOPipe) {
                PIPE_ERR("[Error]mpCamSVIOPipe=NULL");
                ret = MFALSE;
                goto _EXIT_SEND_CMD;
            }
            ret = mpCamSVIOPipe->sendCommand(EPIPECmd_SET_TG_INT_LINE, arg1, (MINTPTR)mSenPixMode, arg3);
        }
        else {
            if (!mpCamIOPipe) {
                PIPE_ERR("[Error]mpCamIOPipe=NULL");
                ret = MFALSE;
                goto _EXIT_SEND_CMD;
            }
            ret = mpCamIOPipe->sendCommand(EPIPECmd_SET_TG_INT_LINE, arg1, (MINTPTR)mSenPixMode, arg3);
        }
        break;
    case ENPipeCmd_GET_TG_OUT_SIZE:
        //arg1 : width
        //arg2 : height
        if (!arg1 || !arg2) {
            PIPE_ERR("NULL param");
            ret = MFALSE;
            goto _EXIT_SEND_CMD;
        }
        *((MINT32*)arg1) = _NORMALPIPE_GET_SENSORCFG_CROP_W(mpSensorIdx);
        *((MINT32*)arg2) = _NORMALPIPE_GET_SENSORCFG_CROP_H(mpSensorIdx);
        break;
    case ENPipeCmd_GET_RMX_OUT_SIZE:
        {
            /*Not support cmd*/
            ret = MFALSE;
            goto _EXIT_SEND_CMD;
        }
        break;
    case ENPipeCmd_GET_HBIN_INFO:
        ret = pCamIOPipe->sendCommand(EPIPECmd_GET_HBIN_INFO, arg1, arg2, arg3);
        break;
    case ENPipeCmd_GET_RMB_INFO:
        ret = pCamIOPipe->sendCommand(EPIPECmd_GET_RMB_INFO, arg1, arg2, arg3);
        break;
    case ENPipeCmd_GET_PIPE_BIT_MODE:
        //arg1 : MUINT32*
        {
            MUINT32 _grp = 0, group = 0;

            ret = pCamIOPipe->sendCommand(EPIPECmd_GET_PIPE_BIT_MODE, (MINTPTR)&_grp, 0, 0);
            if (_grp & eLbitGroup_AfterTG) {
                group |= eBitdepthMode_Lowbit_AfterTG;
            }
            if (_grp & eLbitGroup_AfterFUS) {
                group |= eBitdepthMode_Lowbit_AfterFUS;
            }
            *(MUINT32*)arg1 = group;

            break;
        }
    case ENPipeCmd_GET_EIS_INFO:
        //[X]arg1 : sensor-idx
        //arg2 : NormalPipe_EIS_Info*
        if (0 == arg2) {
            PIPE_ERR("ENPipeCmd_GET_EIS_INFO Err: Invalid Para arg1(%d)",arg1);
            ret = MFALSE;
            goto _EXIT_SEND_CMD;
        }
        else {
            MUINT32 _uni = 0;

            pEis = (NormalPipe_EIS_Info*) arg2;
            /*UNI Switch is not supported, always return true.*/
            pEis->mSupported = MTRUE;
        }
        break;
    case ENPipeCmd_GET_UNI_INFO:
        // only CAM_A, CAM_B support UNI
        ret = pCamIOPipe->sendCommand(EPIPECmd_GET_CUR_RAW, (MUINTPTR)&cur_cam, 0, 0);
        if (CAM_C == cur_cam)
            *(MUINT32*)arg1 = MFALSE;
        else
            *(MUINT32*)arg1 = MTRUE;
        break;
    case ENPipeCmd_GET_CUR_RAW:
        {
            ret = pCamIOPipe->sendCommand(EPIPECmd_GET_CUR_RAW, (MUINTPTR)&cur_cam, arg2, arg3);
            *(MUINT32*)arg1 = NPIPE_MAP_CAM_PIPE(cur_cam);
        }
        break;
    case ENPipeCmd_GET_BIN_INFO:
        ret = pCamIOPipe->sendCommand(EPIPECmd_GET_BIN_INFO, arg1, arg2, arg3);
        break;
    case ENPipeCmd_GET_RSS_INFO:
        {
            /*Not support cmd*/
            ret = MFALSE;
            goto _EXIT_SEND_CMD;
        }
        break;
    case ENPipeCmd_SET_EIS_CBFP:
        //arg1: [In]   enum EModule
        //arg2: [Out] (MUINT32)Module_Handle
        //arg3: user name string
        ret = pCamIOPipe->sendCommand(EPIPECmd_SET_EIS_CBFP, arg1, arg2, arg3);
        break;
    case ENPipeCmd_SET_LCS_CBFP:
        //arg1: [In]   enum EModule
        //arg2: [Out] (MUINT32)Module_Handle
        //arg3: user name string
        ret = pCamIOPipe->sendCommand(EPIPECmd_SET_LCS_CBFP, arg1, arg2, arg3);
        break;
    case ENPipeCmd_SET_SGG2_CBFP:
        if (!pCamIOPipe) {
            PIPE_ERR("[Error]pNPipe=NULL");
            ret = MFALSE;
            goto _EXIT_SEND_CMD;
        }
        ret = pCamIOPipe->sendCommand(EPIPECmd_SET_SGG2_CBFP, arg1, arg2, arg3);
        break;
    case ENPipeCmd_SET_RSS_CBFP:
        //arg1: [In]   enum EModule
        //arg2: [Out] (MUINT32)Module_Handle
        //arg3: user name string
        ret = pCamIOPipe->sendCommand(EPIPECmd_SET_RSS_CBFP, arg1, arg2, arg3);
        break;
    case ENPipeCmd_GET_PMX_INFO:
        {
            /*Not support cmd*/
            ret = MFALSE;
            goto _EXIT_SEND_CMD;
        }
        break;
    case ENPipeCmd_GET_IMGO_INFO:
        if (!pCamIOPipe) {
            PIPE_ERR("[Error]pCamIOPipe=NULL");
            ret = MFALSE;
            goto _EXIT_SEND_CMD;
        }
        ret = pCamIOPipe->sendCommand(EPIPECmd_GET_IMGO_INFO, arg1, arg2, arg3);
        break;
    case ENPipeCmd_GET_CUR_FRM_STATUS:
        if (eCamDataPath_DCIF == this->m_dataPath) {
            ret1 = pCamIOPipe->sendCommand(EPIPECmd_GET_CUR_FRM_STATUS, (MINTPTR)&CamPipeStatus, arg2, arg3);
            ret2 = mpCamSVIOPipe->sendCommand(EPIPECmd_GET_CUR_FRM_STATUS, (MINTPTR)&SVPipeStatus, arg2, arg3);

            if (CamPipeStatus != SVPipeStatus) {
                PIPE_ERR("[Error]CamIOPipe(%d) & CamSVIOPipe(%d) status different !", CamPipeStatus, SVPipeStatus);
                ret = MFALSE;
                goto _EXIT_SEND_CMD;
            }
            ret = ret1 && ret2;
            *(MUINT32*)arg1 = SVPipeStatus;
        }
        else {
            ret = pCamIOPipe->sendCommand(EPIPECmd_GET_CUR_FRM_STATUS, (MINTPTR)&CamPipeStatus, arg2, arg3);
            *(MUINT32*)arg1 = CamPipeStatus;
        }
        //if 1st enq loop does NOT apply patch, MW cannot enque at this timing
        if (MFALSE == m_b1stEnqLoopDone) {
            *(MUINT32*)arg1 = _1st_enqloop_status;
        }
        break;
    case ENPipeCmd_GET_CUR_SOF_IDX:
        if (eCamDataPath_DCIF == this->m_dataPath) {
            if (!mpCamSVIOPipe) {
                PIPE_ERR("[Error]mpCamSVIOPipe=NULL");
                ret = MFALSE;
                goto _EXIT_SEND_CMD;
            }
            ret = mpCamSVIOPipe->sendCommand(EPIPECmd_GET_CUR_SOF_IDX, arg1, arg2, arg3);
        }
        else {
            if (!pCamIOPipe) {
                PIPE_ERR("[Error]pCamIOPipe=NULL");
                ret = MFALSE;
                goto _EXIT_SEND_CMD;
            }
            // use srcTg index rahter than hwModule to get sof index especially for w+t switch
            *(MUINT32*)arg1 = NPIPE_MAP_CAM_TG(mpSensorIdx);
            ret = pCamIOPipe->sendCommand(EPIPECmd_GET_CUR_SOF_IDX, arg1, arg2, arg3);
        }
        break;
    case ENPipeCmd_GET_RCP_SIZE:
        {
            V_CAMIO_CROP_INFO var1;
            if( (ret = pCamIOPipe->sendCommand(EPIPECmd_GET_RCP_INFO, (MINTPTR)&var1, 0, 0)) == MFALSE){
                goto _EXIT_SEND_CMD;
            }
            else{
                V_NormalPipe_CROP_INFO* ptr = (V_NormalPipe_CROP_INFO*)arg1;
                if(ptr == NULL){
                    ret = MFALSE;
                    goto _EXIT_SEND_CMD;
                }
                else{
                    NormalPipe_CROP_INFO info;
                    MRect rect;
                    vector<ST_RECT>::iterator it;

                    ptr->clear();
                    for(MUINT32 i=0;i<var1.size();i++){
                        CAM_MAP(var1.at(i).Pipe_path,info.Pipe_path);
                        //
                        info.v_rect.clear();
                        it = var1.at(i).v_rect.begin();
                        for(MUINT32 j=0;j<var1.at(i).v_rect.size();j++,it++){
                            rect.p.x = it->x;
                            rect.p.y = it->y;
                            rect.s.w = it->w;
                            rect.s.h = it->h;
                            info.v_rect.push_back(rect);
                        }
                        ptr->push_back(info);
                    }
                }
            }
        }
        break;
    case ENPipeCmd_UNI_SWITCH:
        PIPE_ERR("uni switch is not supported.");
        ret = MTRUE;
        goto _EXIT_SEND_CMD;
        break;
    case ENPipeCmd_GET_UNI_SWITCH_STATE:
        PIPE_ERR("uni switch is not supported.");
        ret = MTRUE;
        goto _EXIT_SEND_CMD;
        break;
    case ENPipeCmd_GET_MAGIC_REG_ADDR:
        {
            V_NormalPipe_MagReg* ptr = (V_NormalPipe_MagReg*)arg1;
            vector<E_CAMIO_CAM> var1;
            vector<MUINT32> var2;
            if((ret = pCamIOPipe->sendCommand(EPipeCmd_GET_MAGIC_REG_ADDR, (MINTPTR)&var1, (MINTPTR)&var2, 0)) == MFALSE){
                goto _EXIT_SEND_CMD;
            }
            else{
                if(ptr == NULL){
                    ret = MFALSE;
                    goto _EXIT_SEND_CMD;
                }
                else{
                    NormalPipe_XXXReg var;
                    for(MUINT32 i=0;i<var1.size();i++){//size of var1 shoud be equal to var2
                        CAM_MAP(var1.at(i),var.Pipe_path);
                        var.regAddr = var2.at(i);
                        ptr->push_back(var);
                    }
                }
            }
        }
        break;
    case ENPipeCmd_GET_DTwin_INFO:
        ret = pCamIOPipe->sendCommand(EPipeCmd_DTwin_INFO, 0, arg1, 0);
        break;
    case ENPipeCmd_GET_TWIN_REG_ADDR:
        {
            NormalPipe_XXXReg var;
            V_NormalPipe_TwinReg* ptr = (V_NormalPipe_TwinReg*)arg1;
            vector<E_CAMIO_CAM> var1;
            vector<MUINT32> var2;
            if(arg1 == 0){
                ret = MFALSE;
                goto _EXIT_SEND_CMD;
            }

            if((ret = pCamIOPipe->sendCommand(EPipeCmd_GET_TWIN_REG_ADDR, (MINTPTR)&var1, (MINTPTR)&var2, 0)) == MFALSE){
                goto _EXIT_SEND_CMD;
            }
            else{
                ptr->clear();
                for(MUINT32 i=0;i<var1.size();i++){
                    CAM_MAP(var1.at(i),var.Pipe_path);
                    var.regAddr = var2.at(i);
                    ptr->push_back(var);
                }
            }
        }
        break;
    case ENPipeCmd_SET_FAST_AF:
        {
            /*Not support cmd*/
            ret = MFALSE;
            goto _EXIT_SEND_CMD;
        }
        break;
    case ENPipeCmd_SET_RRZ_CBFP:
        if (!pCamIOPipe) {
            PIPE_ERR("[Error]pCamIOPipe=NULL");
            ret = MFALSE;
            goto _EXIT_SEND_CMD;
        }
        ret = pCamIOPipe->sendCommand(EPIPECmd_SET_RRZ_CBFP, arg1, arg2, arg3);
        break;
    case ENPipeCmd_SET_3A_HAL_CBFP:
        if (!mpCamIOPipe) {
            PIPE_ERR("[Error]mpCamIOPipe=NULL");
            ret = MFALSE;
            goto _EXIT_SEND_CMD;
        }
        ret = mpCamIOPipe->sendCommand(EPIPECmd_SET_3A_HAL_CBFP, arg1, arg2, arg3);
        break;

    case ENPipeCmd_SET_CRZO_R1_CBFP:
        if (!pCamIOPipe) {
            PIPE_ERR("[Error]pCamIOPipe=NULL");
            ret = MFALSE;
            goto _EXIT_SEND_CMD;
        }
        ret = pCamIOPipe->sendCommand(EPIPECmd_SET_CRZR1_CBFP, arg1, arg2, arg3);
        break;
    case ENPipeCmd_SET_CRZO_R2_CBFP:
        if (!pCamIOPipe) {
            PIPE_ERR("[Error]pCamIOPipe=NULL");
            ret = MFALSE;
            goto _EXIT_SEND_CMD;
        }
        ret = pCamIOPipe->sendCommand(EPIPECmd_SET_CRZR2_CBFP, arg1, arg2, arg3);
        break;
    case ENPipeCmd_SET_WT_TUNING_CBFP:
        /*  ENPipeCmd_SET_WT_TUNING_CBFP only for W+T case in mt6779
         *  ENPipeCmd_SET_TUNING_CBFP no support in mt6779
         *  discuss with isp_tuning_mgr owner.
         */
        if (!pCamIOPipe) {
            PIPE_ERR("[Error]pCamIOPipe=NULL");
            ret = MFALSE;
            goto _EXIT_SEND_CMD;
        }
        ret = pCamIOPipe->sendCommand(EPIPECmd_SET_TUNING_CBFP, arg1, arg2, arg3);
        break;
    case ENPipeCmd_SET_REGDUMP_CBFP:
        if (!pCamIOPipe) {
            PIPE_ERR("[Error]pCamIOPipe=NULL");
            ret = MFALSE;
            goto _EXIT_SEND_CMD;
        }
        ret = pCamIOPipe->sendCommand(EPIPECmd_SET_REGDUMP_CBFP, arg1, arg2, arg3);
        break;
    case ENPipeCmd_SET_AWB_CBFP:
        if (!pCamIOPipe) {
            PIPE_ERR("[Error]pCamIOPipe=NULL");
            ret = MFALSE;
            goto _EXIT_SEND_CMD;
        }
        ret = pCamIOPipe->sendCommand(EPIPECmd_SET_AWB_CBFP, arg1, arg2, arg3);
        break;
    case ENPipeCmd_SET_CQSTART_CBFP:
        if (!pCamIOPipe) {
            PIPE_ERR("[Error]pCamIOPipe=NULL");
            ret = MFALSE;
            goto _EXIT_SEND_CMD;
        }
        ret = pCamIOPipe->sendCommand(EPIPECmd_SET_CQSTART_CBFP, arg1, arg2, arg3);
        break;
    case ENPipeCmd_GET_FLK_INFO:
        // only CAM_A, CAM_B support FLK
        ret = pCamIOPipe->sendCommand(EPIPECmd_GET_CUR_RAW, (MUINTPTR)&cur_cam, 0, 0);
        if (CAM_C == cur_cam)
            *(MUINT32*)arg1 = MFALSE;
        else
            *(MUINT32*)arg1 = MTRUE;
        break;
    case ENPipeCmd_SET_QUALITY:
        {
            NPipeIQCmdGlock.lock();

            if (!pCamIOPipe) {
                PIPE_ERR("[Error]pCamIOPipe=NULL");
                ret = MFALSE;
                NPipeIQCmdGlock.unlock();
                goto _EXIT_SEND_CMD;
            }

            NPIPE_PATH_E e_npipe_sub;
            CAM_QUALITY cam_qulity;
            cam_qulity.Raw = (MUINT32) arg2;

            // check which is PIPE_SUB, may be NPIPE_CAM_B or C
            e_npipe_sub = (eCamIQ_MAX == sIQData.IQInfo.lv[NPIPE_CAM_C]) ? NPIPE_CAM_B : NPIPE_CAM_C;

            PIPE_INF("set_quality: Qlvl[%d,%d], SensorIdx[%d,%d],npipe[%d,%d]\n",
                cam_qulity.Bits.Qlvl_0, cam_qulity.Bits.Qlvl_1,
                cam_qulity.Bits.SensorIdx_0, cam_qulity.Bits.SensorIdx_1,
                NPIPE_CAM_A, e_npipe_sub);

            if (((cam_qulity.Bits.Qlvl_0 == eCamIQ_H) && (cam_qulity.Bits.Qlvl_1 == eCamIQ_H)) ||
                ((cam_qulity.Bits.Qlvl_0 == eCamIQ_L) && (cam_qulity.Bits.Qlvl_1 == eCamIQ_L)) ) {
                PIPE_INF("Skip set_quality in H/H or L/L scenario\n");
                NormalPipe::IQNotify = (P1_TUNING_NOTIFY*)arg1;
                NormalPipe::IQNotify->p1TuningNotify(NULL, NULL);

                NPipeIQCmdGlock.unlock();
                goto _EXIT_SEND_CMD;
            }

            // check whether IQ need to be set
            if ((cam_qulity.Bits.Qlvl_0 == sIQData.IQInfo.lv[NPIPE_CAM_A]) &&
                (cam_qulity.Bits.Qlvl_1 == sIQData.IQInfo.lv[e_npipe_sub]) &&
                (cam_qulity.Bits.SensorIdx_0 == sIQData.IQInfo.sensoridx[0]) &&
                (cam_qulity.Bits.SensorIdx_1 == sIQData.IQInfo.sensoridx[1])) {
                PIPE_INF("CAM_QUALITY remains the same, skip set_quality\n");
                NormalPipe::IQNotify = (P1_TUNING_NOTIFY*)arg1;
                NormalPipe::IQNotify->p1TuningNotify(NULL, NULL);

                NPipeIQCmdGlock.unlock();
                goto _EXIT_SEND_CMD;
            }
            // check if SET_IQ cmd timing valid
            if (BufCtrl_start != BfcFSM[0].mFsm || BufCtrl_start != BfcFSM[1].mFsm) {
                PIPE_ERR("Not both NPipe started, cannot SET_QUALITY!!!\n");
                ret = MFALSE;
                NPipeIQCmdGlock.unlock();
                goto _EXIT_SEND_CMD;
            }
            // set IQ quality
            if(updateBufCtrlFSM(BufCtrl_switch_set, NPIPE_CAM_A) && updateBufCtrlFSM(BufCtrl_switch_set, e_npipe_sub)){
                NormalPipe::IQNotify = (P1_TUNING_NOTIFY*)arg1;
                sIQData.IQInfo.lv[NPIPE_CAM_A] = (E_CamIQLevel)cam_qulity.Bits.Qlvl_0;
                sIQData.IQInfo.lv[e_npipe_sub] = (E_CamIQLevel)cam_qulity.Bits.Qlvl_1;
                sIQData.IQInfo.sensoridx[0] = cam_qulity.Bits.SensorIdx_0;
                sIQData.IQInfo.sensoridx[1] = cam_qulity.Bits.SensorIdx_1;
            }
            else{
                PIPE_ERR("set_quality: update iq fsm fail");
                ret = MFALSE;
                NPipeIQCmdGlock.unlock();
                goto _EXIT_SEND_CMD;
            }

            NPipeIQCmdGlock.unlock();
        }
        break;
    case ENPipeCmd_GET_QUALITY:
        {
            CAM_QUALITY res_cam_qulity;
            NPIPE_PATH_E e_npipe_sub;
            e_npipe_sub = (eCamIQ_MAX == sIQData.IQInfo.lv[NPIPE_CAM_C]) ? NPIPE_CAM_B : NPIPE_CAM_C;
            res_cam_qulity.Bits.Qlvl_0 = sIQData.IQInfo.lv[NPIPE_CAM_A];
            res_cam_qulity.Bits.Qlvl_1 = sIQData.IQInfo.lv[e_npipe_sub];
            res_cam_qulity.Bits.SensorIdx_0 = sIQData.IQInfo.sensoridx[0];
            res_cam_qulity.Bits.SensorIdx_1 = sIQData.IQInfo.sensoridx[1];
            *((CAM_QUALITY*)arg2) = res_cam_qulity;
#if 0
            PIPE_INF("get_quality: Qlvl[%d,%d], SensorIdx[%d,%d],npipe[%d,%d]\n",
                res_cam_qulity.Bits.Qlvl_0, res_cam_qulity.Bits.Qlvl_1,
                res_cam_qulity.Bits.SensorIdx_0, res_cam_qulity.Bits.SensorIdx_1,
                NPIPE_CAM_A, e_npipe_sub);
#endif
        }
        break;
    case ENPipeCmd_SET_MODULE_EN:
    case ENPipeCmd_SET_MODULE_SEL:
    case ENPipeCmd_SET_MODULE_CFG:
    case ENPipeCmd_GET_MODULE_HANDLE:
    case ENPipeCmd_SET_MODULE_CFG_DONE:
    case ENPipeCmd_RELEASE_MODULE_HANDLE:
    case ENPipeCmd_SET_MODULE_DBG_DUMP:
        {
            /*cmd for p1hwcfg, only isp3.0 support*/
            ret = MFALSE;
            goto _EXIT_SEND_CMD;
        }
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
MUINT32 NormalPipe::GetOpenedPortInfo(NormalPipe::ePortInfoCmd cmd, MUINT32 arg1)
{
    switch (cmd) {
        case NormalPipe::_GetPort_SetPort:
            {
                MUINT32 portIdx;
                NormalPipe::eDMAO Port;
                switch(arg1){
                    case EPortIndex_RRZO:   Port = NormalPipe::_RRZO_ENABLE;  portIdx = PORT_RRZO.index;
                        break;
                    case EPortIndex_IMGO:   Port = NormalPipe::_IMGO_ENABLE;  portIdx = PORT_IMGO.index;
                        break;
                    case EPortIndex_EISO:   Port = NormalPipe::_EISO_ENABLE;  portIdx = PORT_EISO.index;
                        break;
                    case EPortIndex_LCSO:   Port = NormalPipe::_LCSO_ENABLE;  portIdx = PORT_LCSO.index;
                        break;
                    case EPortIndex_RSSO:   Port = NormalPipe::_RSSO_ENABLE;  portIdx = PORT_RSSO.index;
                        break;
                    case EPortIndex_YUVO:   Port = NormalPipe::_YUVO_ENABLE;  portIdx = PORT_YUVO.index;
                        break;
                    case EPortIndex_CRZO:   Port = NormalPipe::_CRZO_R1_ENABLE;  portIdx = PORT_CRZO_R1.index;
                        break;
                    case EPortIndex_CRZO_R2:    Port = NormalPipe::_CRZO_R2_ENABLE;  portIdx = PORT_CRZO_R2.index;
                        break;
                    case EPortIndex_CAMSV_IMGO:    Port = NormalPipe::_SVIMGO_ENABLE;  portIdx = PORT_CAMSV_IMGO.index;
                        break;
                    case EPortIndex_RAWI:   Port = NormalPipe::_RAWI_ENABLE;  portIdx = PORT_RAWI.index;
                        break;
                    default:
                        PIPE_ERR("unsupported port %d", arg1);
                        return -1;
                        break;
                }

                if (mOpenedPortNum < ePort_max){
                    PortIndex[mOpenedPortNum] = portIdx;
                    mOpenedPortNum++;
                    mOpenedPort |= Port;

                }
                else{
                    PIPE_ERR("Opened port num not enough %d/%d/%d", arg1, mOpenedPortNum, ePort_max);
                    return -1;
                }
            }
            break;
        case NormalPipe::_GetPort_Opened:
            return (MUINT32)mOpenedPort;
        case NormalPipe::_GetPort_OpendNum:
            return mOpenedPortNum;
        case NormalPipe::_GetPort_Index:
            return PortIndex[arg1];
        case NormalPipe::_GetPort_ImgFmt:
            if (arg1 >= ePort_max) {
                PIPE_ERR("Port num not enough %d/%d", arg1,  ePort_max);
                return -1;
            }
            return PortImgFormat[arg1];
        default:
            PIPE_ERR("un-supported cmd:0x%x\n",cmd);
            return 0;
    }

    return 0;
}

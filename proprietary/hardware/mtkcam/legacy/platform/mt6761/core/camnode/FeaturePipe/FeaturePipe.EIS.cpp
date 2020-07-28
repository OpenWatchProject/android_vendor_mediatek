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
//#define LOG_TAG "MtkCam/PostProc.PipeWrapper"
/*************************************************************************************
* Log Utility
*************************************************************************************/
#undef   LOG_TAG    // Decide a Log TAG for current file.
#define  LOG_TAG    "FPipeEis"
#define  ATRACE_TAG ATRACE_TAG_ALWAYS

#include <mtkcam/Log.h>

// Clear previous define, use our own define.
#undef LOG_VRB
#undef LOG_DBG
#undef LOG_INF
#undef LOG_WRN
#undef LOG_ERR
#undef LOG_AST
//
#if defined(__func__)
#undef __func__
#endif
#define __func__ __FUNCTION__

#define LOG_VRB(fmt, arg...)        CAM_LOGV("[%s] " fmt, __func__, ##arg)
#define LOG_DBG(fmt, arg...)        CAM_LOGD("[%s] " fmt, __func__, ##arg)
#define LOG_INF(fmt, arg...)        CAM_LOGI("[%s] " fmt, __func__, ##arg)
#define LOG_WRN(fmt, arg...)        CAM_LOGW("[%s] " fmt, __func__, ##arg)
#define LOG_ERR(fmt, arg...)        CAM_LOGE("[ERROR][%s] " fmt, __func__, ##arg)
#define LOG_AST(cond, fmt, arg...)  do{ if(!(cond)) CAM_LOGA("[%s] " fmt, __func__, ##arg); } while(0)

#define FUNCTION_LOG_START      LOG_INF("+");
#define FUNCTION_LOG_END        LOG_INF("-");
#define ERROR_LOG               LOG_ERR("Error");
//
#define FEATUREPIPE_EIS_NAME "FeaturePipeEis"
//
#include <mtkcam/imageio/IPipe.h>
#include <mtkcam/imageio/IPostProcPipe.h>
#include <mtkcam/ispio_sw_scenario.h>
#include <mtkcam/iopipe/PostProc/INormalStream.h>
#include <mtkcam/iopipe/PostProc/IFeatureStream.h>
//#include <IParamsManager.h>
#include <vector>
#include <queue>
#include <cutils/atomic.h>
#include <cutils/properties.h>
#include "FeaturePipe.BufHandler.h"
#include "FeaturePipe.h"
#include <libwarp/MTKWarp.h>
#include "MTKFeatureSet.h"
#include "DpBlitStream.h"
#include <mtkcam/utils/Format.h>
#include <mtkcam/utils/sys/CpuCtrl.h>
#include <mtkcam/common.h>
#include <mtkcam/featureio/IHal3A.h>
#include "eis_hal.h"
#include "vhdr_hal.h"
#include "FeaturePipeNode.h"
#include "FeaturePipe.EIS.h"
#include <camera_custom_eis.h>

using namespace NSCam;
using namespace NSCamNode;
using namespace NSCam::Utils::Format;

/******************************************************************************
*
******************************************************************************/

static MINT32 g_debugDump = 0;

#define EIS_GPU_WARP_MAP_NUM 3

#define EIS_START_FRAME (18)

enum EISAlgMode
{
    EISALG_QUEUE_NONE = 0,
    EISALG_QUEUE_INIT = 1,
    EISALG_QUEUE_WAIT = 2,
    EISALG_QUEUE      = 3,
    EISALG_QUEUE_STOP = 4,
};

/******************************************************************************
*
******************************************************************************/
#ifdef _DEBUG_DUMP
#include <fcntl.h>
#include <sys/stat.h>

static bool saveBufToFile(char const*const fname, MUINT8 *const buf, MUINT32 const size)
{
    int nw, cnt = 0;
    uint32_t written = 0;

    //LOG_INF("(name, buf, size) = (%s, %x, %d)", fname, buf, size);
    //LOG_INF("opening file [%s]\n", fname);
    int fd = ::open(fname, O_RDWR | O_CREAT | O_TRUNC, S_IRWXU);
    if (fd < 0) {
        LOG_ERR("failed to create file [%s]: %s", fname, ::strerror(errno));
        return false;
    }

    //LOG_INF("writing %d bytes to file [%s]\n", size, fname);
    while (written < size) {
        nw = ::write(fd,
                     buf + written,
                     size - written);
        if (nw < 0) {
            LOG_ERR("failed to write to file [%s]: %s", fname, ::strerror(errno));
            break;
        }
        written += nw;
        cnt++;
    }
    LOG_INF("done writing %d bytes to file [%s]\n", size, fname);
    ::close(fd);
    return true;
}

#endif

/******************************************************************************
*
******************************************************************************/
FeaturePipeEisNode *FeaturePipeEisNode::createInstance(MUINT32 featureOnMask,MUINT32 mOpenedSensorIndex)
{
    //LOG_AST(FEATURE_MASK_IS_EIS_ENABLED(featureOnMask), "Create EIS node while EIS is off", 0);
    return new FeaturePipeEisNode("FeaturePipeEisNode", SingleTrigger, SCHED_POLICY, (SCHED_PRIORITY-4),featureOnMask,mOpenedSensorIndex);
}

/******************************************************************************
*
******************************************************************************/
MVOID FeaturePipeEisNode::destroyInstance()
{
    delete this;
}

/******************************************************************************
*
******************************************************************************/
MVOID FeaturePipeEisNode::startVideoRecord()
{
    FUNCTION_LOG_START;
    Mutex::Autolock lock(mUninitMtx);
    mFrameCount = 0;
    mIsRecStarted = MTRUE;
    mIsRecStopNotified = MFALSE;
    FUNCTION_LOG_END;
}

/******************************************************************************
*
******************************************************************************/
MVOID FeaturePipeEisNode::notifyRecordStopping()
{
    FUNCTION_LOG_START;
    Mutex::Autolock lock(mUninitMtx);
    mIsRecStopNotified = MTRUE;
    FUNCTION_LOG_END;
}

/******************************************************************************
*
******************************************************************************/
MVOID FeaturePipeEisNode::stopVideoRecord()
{
    FUNCTION_LOG_START;
    Mutex::Autolock lock(mUninitMtx);
    mIsRecStarted = MFALSE;
    mIsRecStopNotified = MFALSE;
    FUNCTION_LOG_END;
}

MVOID FeaturePipeEisNode::applyEISConfig(EIS_HAL_CONFIG_DATA &config, MINT32 algMode, MINT32 algCounter)
{
    //FUNCTION_LOG_START;
    Mutex::Autolock lock(mLock);
    EIS_HAL_CONFIG_DATA curConfig;

    // Use latest data
    if( algMode == EISALG_QUEUE_NONE ||
        algMode == EISALG_QUEUE_INIT ||
        algMode == EISALG_QUEUE_WAIT )
    {
        curConfig = mEisHalCfgData.back();
    }
    // Use queue data
    else if( algMode == EISALG_QUEUE ||
             algMode == EISALG_QUEUE_STOP )
    {
        curConfig = mEisHalCfgData.front();
    }

    // Apply current configs
    if( !mEisHalCfgData.empty() )
    {
        memcpy(&config, &curConfig, sizeof(EIS_HAL_CONFIG_DATA));
    }
    else
    {
        LOG_WRN("Unknown EIS config!");
    }
    config.process_mode = algMode;
    config.process_idx  = algCounter;

    // Need queue during EISALG_QUEUE_WAIT
    if( algMode != EISALG_QUEUE_WAIT )
    {
        //LOG_DBG("Current EISConfig size(%d), now pop one", mEisHalCfgData.size());
        // Pop out config after using
        mEisHalCfgData.pop();
    }
    //FUNCTION_LOG_END;
}

/******************************************************************************
*
******************************************************************************/
FeaturePipeEisNode::FeaturePipeEisNode( const char* name,
                                              eThreadNodeType type,
                                              int policy,
                                              int priority,
                                              MUINT32 featureOnMask,
                                              MUINT32 mOpenedSensorIndex)
                                              : FeaturePipeNode(name, type, policy, priority, featureOnMask, mOpenedSensorIndex)
{
    if(mFeatureOnMask & FEATURE_ON_MASK_BIT_VFB)
    {
        LOG_DBG("EisPlus+VFB");
        addDataSupport(ENDPOINT_SRC, P2A_TO_EISSW);
        addDataSupport(ENDPOINT_DST, EISSW_TO_VFBSW);
    }
    else
    {
        LOG_DBG("EisPlus");
        addDataSupport(ENDPOINT_SRC, P2A_TO_EISSW);
        addDataSupport(ENDPOINT_DST, EISSW_TO_GPU);
    }

    mpEisHal = NULL;

    mEisPlusGpuInfo.gridW = 0;
    mEisPlusGpuInfo.gridH = 0;

    mpClearWarpMapX = NULL;
    mpClearWarpMapY = NULL;

    while(!mFrameNum.empty())
    {
        mFrameNum.pop();
    }

    while(!mFrameTS.empty())
    {
        mFrameTS.pop();
    }

    while(!mFrameByPass.empty())
    {
        mFrameByPass.pop();
    }

    while(!mEisHalCfgData.empty())
    {
        mEisHalCfgData.pop();
    }

    while(!mGpuTargetW.empty())
    {
        mGpuTargetW.pop();
    }

    while(!mGpuTargetH.empty())
    {
        mGpuTargetH.pop();
    }
    mRecordingHint = MFALSE;
    mIsRecStarted = MFALSE;
    mIsRecStopNotified = MFALSE;
    mNeedQCount = 0;
    mAlgoCounter = 0;
    mFrameCount = 0;
    mpCpuCtrl = NULL;


}

/******************************************************************************
*
******************************************************************************/
MBOOL FeaturePipeEisNode::onInit()
{
    FUNCTION_LOG_START;

    MRESULT mret;
    MUINT8 imemBufInfoIdx = 0;
    if(FEATURE_MASK_IS_EIS_ENABLED(mFeatureOnMask) && FEATURE_MASK_IS_EIS_Q_ENABLED(mFeatureOnMask))
        mNeedQCount = getEisSupFrames_Hal1(); // From camera_custom_eis
    else
        mNeedQCount = 0;
    NodeDataTypes allocDataType = (mFeatureOnMask & FEATURE_ON_MASK_BIT_VFB)? EISSW_TO_VFBSW: EISSW_TO_GPU;

    //====== Dynamic Debug ======

    char value[PROPERTY_VALUE_MAX] = {'\0'};
    property_get("vendor.debug.eis.dump", value, "0");
    g_debugDump = atoi(value);

    //====== Create EIS HAL instance ======

    LOG_DBG("mOpenedSensorIndex(%u)",mOpenedSensorIndex);
    mpEisHal = EisHal::CreateInstance(FEATUREPIPE_EIS_NAME,mOpenedSensorIndex);
    if (mpEisHal == NULL)
    {
        LOG_ERR("mpEisHal CreateInstance failed");
        goto _Exit;
    }

    if(EIS_RETURN_NO_ERROR != mpEisHal->Init())
    {
        LOG_ERR("mpEisHal init failed");
        goto _Exit;
    }

    //====== Allocate buffer for next node ======
    if (mRecordingHint && (mFeatureOnMask&FEATURE_ON_MASK_BIT_EIS) == FEATURE_ON_MASK_BIT_EIS )
    {
        pBufHandler = getBufferHandler(allocDataType);
        if (pBufHandler == NULL)
        {
            LOG_ERR("ICamBufHandler failed!");
            goto _Exit;
        }
    }
    //> get EIS Plus GPU grid size

    mpEisHal->GetEisPlusGpuInfo(&mEisPlusGpuInfo);

    LOG_DBG("GPU grid (W/H)=(%u/%u)",mEisPlusGpuInfo.gridW,mEisPlusGpuInfo.gridH);

    // GPU warp map
    if (mRecordingHint && (mFeatureOnMask&FEATURE_ON_MASK_BIT_EIS) == FEATURE_ON_MASK_BIT_EIS )
    {
        AllocInfo allocinfo(mEisPlusGpuInfo.gridW * 4 , //Warp Map X + Y (both X and Y need gridW * gridH * 4 bytes)
                            mEisPlusGpuInfo.gridH * 2,
                            eImgFmt_BAYER8,
                            eBUFFER_USAGE_SW_READ_OFTEN | eBUFFER_USAGE_SW_WRITE_OFTEN);

        size_t warpMapNum = EIS_GPU_WARP_MAP_NUM + mNeedQCount;
        for(MUINT32 i = 0; i < warpMapNum; i++)
        {
            if( !pBufHandler->requestBuffer(allocDataType, allocinfo) )
            {
                LOG_ERR("requestBuffer failed!");
                goto _Exit;
            }
        }

        LOG_DBG("Enable FHD VR GIS thermal_manager");
        //system("/vendor/bin/thermal_manager thermal_policy_02 1");

        mpCpuCtrl = CpuCtrl::createInstance();
        mpCpuCtrl->init();

        //
        CpuCtrl::CPU_CTRL_RANGE_INFO cpuRangeInfo;

        cpuRangeInfo.bigCoreMin = 0;
        cpuRangeInfo.littleCoreMin = 3;
        //
        cpuRangeInfo.bigCoreMax = 0;
        cpuRangeInfo.littleCoreMax = 3;
        //
        cpuRangeInfo.bigFreqMin = -1;
        cpuRangeInfo.littleFreqMin = -1;
        //
        cpuRangeInfo.bigFreqMax = -1;
        cpuRangeInfo.littleFreqMax = -1;
        //


        mpCpuCtrl->enable(cpuRangeInfo);
        LOG_DBG("Enable FHD VR GIS CPU ctrl");

    }

    //====== Register EIS Pass2 User ======

    //RegisterEisP2User();


    FUNCTION_LOG_END;
    return MTRUE;

_Exit:

    if (mpEisHal != NULL)
    {
        mpEisHal->DestroyInstance(FEATUREPIPE_EIS_NAME);
    }
    return MFALSE;
}

/******************************************************************************
*
******************************************************************************/
MBOOL FeaturePipeEisNode::onUninit()
{
    Mutex::Autolock autoLock(mUninitMtx);

    FUNCTION_LOG_START;

    if (mpEisHal != NULL)
    {
        mpEisHal->Uninit();
        mpEisHal->DestroyInstance(FEATUREPIPE_EIS_NAME);
        mpEisHal = NULL;
    }

    {
        Mutex::Autolock lock(mLock);

        while(!mFrameNum.empty())
        {
            mFrameNum.pop();
        }

        while(!mFrameTS.empty())
        {
            mFrameTS.pop();
        }

        while(!mFrameByPass.empty())
        {
            mFrameByPass.pop();
        }

        while(!mEisHalCfgData.empty())
        {
            mEisHalCfgData.pop();
        }

        while(!mGpuTargetW.empty())
        {
            mGpuTargetW.pop();
        }

        while(!mGpuTargetH.empty())
        {
            mGpuTargetH.pop();
        }

        while(!mqJobWarpMap.empty())
        {
            mqJobWarpMap.pop();
        }
    }

    if (mRecordingHint && (mFeatureOnMask&FEATURE_ON_MASK_BIT_EIS) == FEATURE_ON_MASK_BIT_EIS )
    {
        LOG_DBG("Disable FHD VR GIS thermal_manager");
        //system("/vendor/bin/thermal_manager thermal_policy_02 0");
    }


    mRecordingHint = MFALSE;
    mIsRecStarted = MFALSE;
    mFrameCount = 0;

    if(mpCpuCtrl)
    {

        LOG_DBG("Disable FHD VR GIS CPU ctrl");
        mpCpuCtrl->disable();
        mpCpuCtrl->uninit();
        mpCpuCtrl->destroyInstance();
        mpCpuCtrl = NULL;
    }


    FUNCTION_LOG_END;
    return MTRUE;
}

/******************************************************************************
*
******************************************************************************/
MBOOL FeaturePipeEisNode::onStart()
{
    FUNCTION_LOG_START;
    FUNCTION_LOG_END;
    return MTRUE;
}

/******************************************************************************
*
******************************************************************************/
MBOOL FeaturePipeEisNode::onStop()
{
    FUNCTION_LOG_START;
    if (mpEisHal)
    {
        mpEisHal->AbortP2Calibration();
        LOG_DBG("AbortP2Calibration");
    }
    MBOOL ret = syncWithThread(); // wait for jobs done
    // return remain buffers
    removeQueuedInfos();
    FUNCTION_LOG_END;
    return MTRUE;
}

/******************************************************************************
*
******************************************************************************/
MVOID FeaturePipeEisNode::removeQueuedInfos()
{
    FUNCTION_LOG_START;
    // return remain buffers
    Mutex::Autolock lock(mUninitMtx);
    LOG_DBG("need wait EISSW warpJobs(%u) done", mqJobWarpMap.size());
    while(mqJobWarpMap.size() > 0)
    {
        FeaturePipeNodeImgReqJob_s jobWarpImg;
        jobWarpImg = mqJobWarpMap.front();
        mqJobWarpMap.pop();
        pBufHandler->enqueBuffer(EISSW_TO_GPU, jobWarpImg.imgReq.mBuffer);
    }

    while(!mEisHalCfgData.empty())
    {
        mEisHalCfgData.pop();
    }

    while(!mGpuTargetW.empty())
    {
        mGpuTargetW.pop();
    }

    while(!mGpuTargetH.empty())
    {
        mGpuTargetH.pop();
    }
    mIsRecStopNotified = MFALSE;
    mAlgoCounter = 0;
    FUNCTION_LOG_END;
}
/******************************************************************************
*
******************************************************************************/
MBOOL FeaturePipeEisNode::onNotify(MUINT32 const msg, MUINT32 const ext1, MUINT32 const ext2)
{
    LOG_DBG("msg(0x%x), ext1(0x%x), ext2(0x%x)", msg, ext1, ext2);
    return MTRUE;
}

/******************************************************************************
*
******************************************************************************/
MBOOL FeaturePipeEisNode::onPostBuffer(MUINT32 const data, MUINTPTR const buf, MUINT32 const ext)
{
    LOG_DBG("data(%d), buf(0x%x), ext(0x%x)", data, buf, ext);

    Mutex::Autolock lock(mLock);
#if 0   // EISPlus does not need input image

    FeaturePipeNodeJob_s job;

    job.pBufList = &((reinterpret_cast<FeaturePipeNodeBuf_s*>(buf))->bufList);
    job.mFrameNo = (reinterpret_cast<FeaturePipeNodeBuf_s*>(buf))->mFrameNo;
    job.buf = buf;
    job.ext = ext;
    mqJobInput.push(job);
#endif
    TagImgRequest *pReq = reinterpret_cast<TagImgRequest*>(buf);
    MUINT64 ts = pReq->imgReq.mBuffer->getTimestamp();
    //LOG_DBG("IMAGE ts :(%lld)", ts);

    mFrameNum.push(ext);
    mFrameTS.push(ts);
    mFrameByPass.push(pReq->needByPass);

    if (mpEisHal)
    {
        mpEisHal->ForcedDoEisPass2();
    }

    triggerLoop();

    return MTRUE;
}

/******************************************************************************
*
******************************************************************************/
MBOOL FeaturePipeEisNode::threadLoopUpdate()
{
    Mutex::Autolock autoLock(mUninitMtx);

    CAM_TRACE_FMT_BEGIN("FeaturePipeEisNode");


    MRESULT mret;

    MUINT32 algCounter = 0;
    MBOOL   recStopNotified = MFALSE;
    MUINT32 frameNo = 0;
    MBOOL   needByPass = MFALSE;

    ImgRequest warpImgReq;
    ImgRequest *pWarpReq = NULL;
    ImgRequest dummyImgReq;
    NodeDataTypes allocDataType = (mFeatureOnMask & FEATURE_ON_MASK_BIT_VFB)? EISSW_TO_VFBSW: EISSW_TO_GPU;
    EIS_HAL_CONFIG_DATA eisHalCfgData;

    MUINT64 ts = mFrameTS.front();


    //====== EISPlus Process ======
    //Check if in Recording
    if (mRecordingHint && (mFeatureOnMask&FEATURE_ON_MASK_BIT_EIS) == FEATURE_ON_MASK_BIT_EIS )
    {
        if(!pBufHandler->dequeBuffer(allocDataType, &warpImgReq) || warpImgReq.mBuffer == NULL)
        {
            LOG_AST(false, "Out of buffer", 0);
            return MFALSE;
        }

        pWarpReq = &warpImgReq;

        {
            Mutex::Autolock lock(mLock);
            frameNo = mFrameNum.front();
            mFrameNum.pop();
            needByPass = mFrameByPass.front();
            mFrameByPass.pop();
            mFrameTS.pop();
        }
        recStopNotified = mIsRecStopNotified;

        //====== Flush queue if EIS 3.0 record stopped ======
        if( !mIsRecStarted && mqJobWarpMap.size() > 0)
        // Need to output remain warp configs after record stopped
        // When onStop called, the queue will be flushed to return buffers
        {
            //
            FeaturePipeNodeImgReqJob_s job;
            job.imgReq = warpImgReq;
            job.mFrameNo = frameNo;
            mqJobWarpMap.push(job);
            //
            FeaturePipeNodeImgReqJob_s firstJob = mqJobWarpMap.front();
            mqJobWarpMap.pop();
            warpImgReq = firstJob.imgReq;
            frameNo = firstJob.mFrameNo;
            pWarpReq = &warpImgReq;

            LOG_DBG("(EIS3.0) [Output remain frames](out 1 frame 1 time) EIS->GPU newFrameNo(%u)\
                ,recFrameCount(%u), warpQueSize(%d)", frameNo, mFrameCount, mqJobWarpMap.size());

        }

        LOG_DBG("recHint(%d), recStart(%d), eison(%d), recStopNotify(%d), needByPass(%d)", mRecordingHint, mIsRecStarted, ((mFeatureOnMask&FEATURE_ON_MASK_BIT_EIS) == FEATURE_ON_MASK_BIT_EIS), recStopNotified, needByPass);
        //====== Check queue buffer or not for EIS Plus ======
        if ( mRecordingHint && !needByPass &&
             (mFeatureOnMask&FEATURE_ON_MASK_BIT_EIS) == FEATURE_ON_MASK_BIT_EIS &&
            !(mFeatureOnMask & FEATURE_ON_MASK_BIT_VFB) )
        {
            MUINT32 newFrameNo = frameNo;
            if(mFrameCount < EIS_START_FRAME || mNeedQCount == 0 || !mIsRecStarted) // output normal eis frame
            {
                mpEisHal->SetEisPlusGpuInfo((MINT32 *) (const_cast<IImageBuffer*>(warpImgReq.mBuffer)->getBufVA(0)),
                                            (MINT32 *) (const_cast<IImageBuffer*>(warpImgReq.mBuffer)->getBufVA(0) + mEisPlusGpuInfo.gridW*mEisPlusGpuInfo.gridH*4));
                algCounter = mqJobWarpMap.size();
                applyEISConfig(eisHalCfgData, EISALG_QUEUE_NONE, algCounter);
                mAlgoCounter = algCounter;
                //> do EIS pass2
                mpEisHal->DoRSCMEEis(&eisHalCfgData, ts);
            }
            else if(mFrameCount == EIS_START_FRAME)
            {
                mpEisHal->SetEisPlusGpuInfo((MINT32 *) (const_cast<IImageBuffer*>(warpImgReq.mBuffer)->getBufVA(0)),
                                            (MINT32 *) (const_cast<IImageBuffer*>(warpImgReq.mBuffer)->getBufVA(0) + mEisPlusGpuInfo.gridW*mEisPlusGpuInfo.gridH*4));
                algCounter = 0;
                applyEISConfig(eisHalCfgData, EISALG_QUEUE_INIT, algCounter);
                mAlgoCounter = algCounter;
                //> do EIS pass2
                mpEisHal->DoRSCMEEis(&eisHalCfgData, ts);

            }
            else // do EIS 3.0 flow
            {
                FeaturePipeNodeImgReqJob_s job;
                job.imgReq = warpImgReq;
                job.mFrameNo = frameNo;
                mqJobWarpMap.push(job);

                // we need to queue frames and don't output it currently
                if(mqJobWarpMap.size() <= mNeedQCount )
                {
                    pWarpReq = &dummyImgReq; // ask gpu reply
                    mpEisHal->SetEisPlusGpuInfo((MINT32 *) (const_cast<IImageBuffer*>(warpImgReq.mBuffer)->getBufVA(0)),
                                                (MINT32 *) (const_cast<IImageBuffer*>(warpImgReq.mBuffer)->getBufVA(0) + mEisPlusGpuInfo.gridW*mEisPlusGpuInfo.gridH*4));

                    algCounter = mqJobWarpMap.size();
                    applyEISConfig(eisHalCfgData, EISALG_QUEUE_WAIT, algCounter);
                    mAlgoCounter = algCounter;
                    //> do EIS pass2
                    mpEisHal->DoRSCMEEis(&eisHalCfgData, ts);
                }
                else if(recStopNotified)
                {
                    //Stop Algo queue
                    FeaturePipeNodeImgReqJob_s firstJob = mqJobWarpMap.front();
                    mqJobWarpMap.pop();
                    warpImgReq = firstJob.imgReq;
                    newFrameNo = firstJob.mFrameNo;
                    pWarpReq = &warpImgReq;

                    mpEisHal->SetEisPlusGpuInfo((MINT32 *) (const_cast<IImageBuffer*>(warpImgReq.mBuffer)->getBufVA(0)),
                                                (MINT32 *) (const_cast<IImageBuffer*>(warpImgReq.mBuffer)->getBufVA(0) + mEisPlusGpuInfo.gridW*mEisPlusGpuInfo.gridH*4));

                    algCounter = (mAlgoCounter > 0) ? (--mAlgoCounter) : 0;
                    applyEISConfig(eisHalCfgData, EISALG_QUEUE_STOP, algCounter);
                    mAlgoCounter = algCounter;
                    //> do EIS pass2
                    mpEisHal->DoRSCMEEis(&eisHalCfgData, ts);
                }
                else // start to output frame with EIS 2.5 effect
                {
                    FeaturePipeNodeImgReqJob_s firstJob = mqJobWarpMap.front();
                    mqJobWarpMap.pop();
                    warpImgReq = firstJob.imgReq;
                    newFrameNo = firstJob.mFrameNo;
                    pWarpReq = &warpImgReq;

                    mpEisHal->SetEisPlusGpuInfo((MINT32 *) (const_cast<IImageBuffer*>(warpImgReq.mBuffer)->getBufVA(0)),
                                                (MINT32 *) (const_cast<IImageBuffer*>(warpImgReq.mBuffer)->getBufVA(0) + mEisPlusGpuInfo.gridW*mEisPlusGpuInfo.gridH*4));

                    algCounter = mqJobWarpMap.size();
                    applyEISConfig(eisHalCfgData, EISALG_QUEUE, algCounter);
                    mAlgoCounter = algCounter;
                    //> do EIS pass2
                    mpEisHal->DoRSCMEEis(&eisHalCfgData, ts);
                }
            }
            LOG_DBG("(EIS3.0) EIS->GPU origFrameNo(%u), newFrameNo(%u), recFrameCount(%u), warpQueSize(%d), algoCount(%d)", frameNo, newFrameNo, mFrameCount, mqJobWarpMap.size(), algCounter);
            frameNo = newFrameNo; // use new Frame No
            mFrameCount++;
        } // end of do EIS algo ( if recording)
        else
        {
            // No need queue, pop configs
            applyEISConfig(eisHalCfgData, EISALG_QUEUE_NONE, algCounter);
        }

        //LOG_DBG("EIS->GPU warp map(%u)", frameNo);
        if (mRecordingHint && (mFeatureOnMask&FEATURE_ON_MASK_BIT_EIS) == FEATURE_ON_MASK_BIT_EIS )
        {
            TagImgRequest tagReq;
            tagReq.imgReq = *pWarpReq;
            tagReq.needByPass = needByPass;
            handlePostBuffer(EISSW_TO_GPU, (MUINTPTR) &tagReq, frameNo);
        }


    }
    else
    {
        // No need queue, pop configs
        applyEISConfig(eisHalCfgData, EISALG_QUEUE_NONE, algCounter);

        if ( (MFALSE == mRecordingHint)  && !FEATURE_MASK_IS_VHDR_ENABLED(mFeatureOnMask))
        {
            mpEisHal->DoCalibration(EIS_PASS_2, &eisHalCfgData, ts, mRecordingHint);
        }


        {
            Mutex::Autolock lock(mLock);
            mGpuTargetW.pop();
            mGpuTargetH.pop();
        }


        {
            Mutex::Autolock lock(mLock);
            mFrameNum.pop();
            mFrameTS.pop();
            mFrameByPass.pop();
        }

        //LOG_DBG("EIS->GPU warp map(%u)", frameNo);
        if (mRecordingHint && (mFeatureOnMask&FEATURE_ON_MASK_BIT_EIS) == FEATURE_ON_MASK_BIT_EIS )
        {
            TagImgRequest tagReq;
            tagReq.imgReq = *pWarpReq;
            tagReq.needByPass = needByPass;
            handlePostBuffer(EISSW_TO_GPU, (MUINTPTR) &tagReq, frameNo);
        }

    }



    //====== Post Buffer to Next Node ======

    if(mFeatureOnMask & FEATURE_ON_MASK_BIT_VFB)    //vFB on
    {
        LOG_DBG("EIS->vFB warp map(%u)", frameNo);
        handlePostBuffer(EISSW_TO_VFBSW, (MUINTPTR) &warpImgReq, frameNo);
    }


    CAM_TRACE_FMT_END();

    return MTRUE;
}

/******************************************************************************
*
******************************************************************************/
MBOOL FeaturePipeEisNode::onReturnBuffer(MUINT32 const data, MUINTPTR const buf, MUINT32 const ext)
{
    LOG_DBG("data(0x%x),buf(0x%x),ext(0x%x)", data, buf, ext);
    if (mRecordingHint && (mFeatureOnMask&FEATURE_ON_MASK_BIT_EIS) == FEATURE_ON_MASK_BIT_EIS )
    {
        pBufHandler->enqueBuffer(data, ((ImgRequest*)buf)->mBuffer);
    }
    return MTRUE;
}

/******************************************************************************
*
******************************************************************************/
MVOID FeaturePipeEisNode::onDumpBuffer(const char* usr, MUINT32 const data, MUINTPTR const buf, MUINT32 const ext)
{
}

/******************************************************************************
*
******************************************************************************/
MBOOL FeaturePipeEisNode::ConfigEis(const EIS_PASS_ENUM &aEisPass,const EIS_HAL_CONFIG_DATA &aEisConfig)
{
    Mutex::Autolock lock(mLock);

    mEisHalCfgData.push(aEisConfig);
    mGpuTargetW.push(aEisConfig.gpuTargetW);
    mGpuTargetH.push(aEisConfig.gpuTargetH);

    //LOG_DBG("Push current EISConfig. Size=(e:%d/gw:%d/gh:%d)", mEisHalCfgData.size(), mGpuTargetW.size(), mGpuTargetH.size());
    if (mRecordingHint && (mFeatureOnMask&FEATURE_ON_MASK_BIT_EIS) == FEATURE_ON_MASK_BIT_EIS )
    {
        if(EIS_RETURN_NO_ERROR != mpEisHal->ConfigRSCMEEis(aEisConfig))
        {
            LOG_ERR("mpEisHal->ConfigEis fail");
            return MFALSE;
        }

    }else
    {
        if ( (MFALSE == mRecordingHint) && !FEATURE_MASK_IS_VHDR_ENABLED(mFeatureOnMask))
        {
            if(EIS_RETURN_NO_ERROR != mpEisHal->ConfigCalibration(aEisPass,aEisConfig))
            {
                LOG_ERR("mpEisHal->ConfigEis fail");
                return MFALSE;
            }
        }
    }



    return MTRUE;
}

/******************************************************************************
*
******************************************************************************/
MVOID FeaturePipeEisNode::GetEisResult( MUINT32 &a_CMV_X_Int,
                                           MUINT32 &a_CMV_X_Flt,
                                           MUINT32 &a_CMV_Y_Int,
                                           MUINT32 &a_CMV_Y_Flt,
                                           MUINT32 &a_TarWidth,
                                           MUINT32 &a_TarHeight)
{
    mpEisHal->GetEisResult(a_CMV_X_Int,a_CMV_X_Flt,a_CMV_Y_Int,a_CMV_Y_Flt,a_TarWidth,a_TarHeight);
}

/******************************************************************************
*
******************************************************************************/
MVOID FeaturePipeEisNode::GetEisGmv(MINT32 &aGMV_X,MINT32 &aGMV_Y,MUINT32 *aEisInW,MUINT32 *aEisInH)
{
    mpEisHal->GetEisGmv(aGMV_X,aGMV_Y,aEisInW,aEisInH);
}

/******************************************************************************
*
******************************************************************************/
MVOID FeaturePipeEisNode::GetFeoRegInfo(FEO_REG_INFO *aFeoRegInfo)
{
    mpEisHal->GetFeoRegInfo(aFeoRegInfo);
}

/******************************************************************************
*
******************************************************************************/
MUINT32 FeaturePipeEisNode::GetEisPlusCropRatio()
{
    return mpEisHal->GetEisPlusCropRatio();
}

/******************************************************************************
*
******************************************************************************/
MVOID FeaturePipeEisNode::GetEisPlusTargetSize(MUINT32 &aWidth,MUINT32 &aHeight)
{
    Mutex::Autolock lock(mLock);

    if(mGpuTargetW.empty() || mGpuTargetH.empty())
    {
        LOG_ERR("empty queue(%d,%d)",mGpuTargetW.size(),mGpuTargetH.size());
        return;
    }

    aWidth  = mGpuTargetW.front();
    aHeight = mGpuTargetH.front();

    mGpuTargetW.pop();
    mGpuTargetH.pop();
}

/******************************************************************************
*
******************************************************************************/
MVOID FeaturePipeEisNode::GetEisInfoAtP2(char const *userName,EIS_P1_RESULT_INFO *apEisResult,const MINT64 &aTimeStamp,MBOOL lastUser)
{
    FUNCTION_LOG_START
    mpEisHal->GetEisInfoAtP2(userName,apEisResult,aTimeStamp,lastUser);
    FUNCTION_LOG_END
}

/******************************************************************************
*
******************************************************************************/
MVOID FeaturePipeEisNode::FlushMemory(const EIS_DMA_ENUM &aDma,const EIS_FLUSH_ENUM &aFlush)
{
    mpEisHal->FlushMemory(aDma,aFlush);
}

/******************************************************************************
*
******************************************************************************/
MVOID FeaturePipeEisNode::RegisterEisP2User()
{
    //mpEisHal->SetEisP2User("3DNR");
}
/******************************************************************************
*
******************************************************************************/

MBOOL FeaturePipeEisNode::set(FeaturePipeSetType setType, const MINTPTR setData)
{
    switch (setType)
    {
        case RECORDING_HINT:
            if (mRecordingHint != (MBOOL) setData)
            {
                mRecordingHint = (MBOOL) setData;
                LOG_DBG("Set recording hint to %d", mRecordingHint);
            }
            break;
    }
    return MTRUE;
}



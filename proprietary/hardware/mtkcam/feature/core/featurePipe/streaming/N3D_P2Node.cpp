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
 * MediaTek Inc. (C) 2016. All rights reserved.
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

#include "P2ANode.h"
#include "N3D_P2Node.h"

#define PIPE_CLASS_TAG "N3D_P2Node"
#define PIPE_TRACE TRACE_N3DP2_NODE
#include <featurePipe/core/include/PipeLog.h>
#include <stereo_crop_util.h>

using NSIoPipe::Input;
using NSIoPipe::Output;
using NSIoPipe::ModuleInfo;
using NSIoPipe::FrameParams;
using NSIoPipe::ExtraParam;
using NSIoPipe::EPIPE_FE_INFO_CMD;
using NSIoPipe::EPIPE_FM_INFO_CMD;
using NSCam::NSIoPipe::NSPostProc::ENormalStreamTag_Normal;
using NSCam::NSIoPipe::NSPostProc::ENormalStreamTag_FM;
using NSImageio::NSIspio::EPortIndex_WDMAO;
using NSImageio::NSIspio::EPortIndex_WROTO;
using NSImageio::NSIspio::EPortIndex_IMG2O;
using NSImageio::NSIspio::EPortIndex_IMGI;
using NSImageio::NSIspio::EPortIndex_DEPI;
using NSImageio::NSIspio::EPortIndex_DMGI;
using NSImageio::NSIspio::EPortIndex_FEO;
using NSImageio::NSIspio::EPortIndex_MFBO;
using NSCam::NSIoPipe::FEInfo;
using NSCam::NSIoPipe::FMInfo;

#define N3DP2_USER_NAME "N3D_P2Node"

namespace NSCam {
namespace NSCamFeature {
namespace NSFeaturePipe {

N3D_P2Node::N3D_P2Node(const char *name)
    : StreamingFeatureNode(name)
    , mNormalStream(NULL)
{
    TRACE_FUNC_ENTER();
    this->addWaitQueue(&mRequests);
    TRACE_FUNC_EXIT();
}

N3D_P2Node::~N3D_P2Node()
{
    TRACE_FUNC_ENTER();
    TRACE_FUNC_EXIT();
}

MBOOL N3D_P2Node::onInit()
{
    TRACE_FUNC_ENTER();
    MY_LOGD("Support N3D, create N3D buffer pools.");
    // create image buffer
    createImgBufPool();
    // prepare templates
    prepareTemParams();
    // setup FE size info
    setFESizInfo();
    // create normal stream
    mNormalStream = NSCam::NSIoPipe::NSPostProc::INormalStream::createInstance(mSensorIndex);
    if (mNormalStream == NULL)
    {
        MY_LOGE("mNormalStream create instance for N3D_P2Node failed!");
        return MFALSE;
    }
    else
    {
        mNormalStream->init(N3DP2_USER_NAME);
    }
    TRACE_FUNC_EXIT();
    return MTRUE;
}

MBOOL N3D_P2Node::onUninit()
{
    TRACE_FUNC_ENTER();
    ImageBufferPool::destroy(mN3dFEBMasterImgPool);
    ImageBufferPool::destroy(mN3dFEBSlaveImgPool);
    ImageBufferPool::destroy(mN3dFECMasterImgPool);
    ImageBufferPool::destroy(mN3dFECSlaveImgPool);
    ImageBufferPool::destroy(mN3dFEBOImgPool);
    ImageBufferPool::destroy(mN3dFECOImgPool);
    ImageBufferPool::destroy(mN3dFMBOImgPool);
    ImageBufferPool::destroy(mN3dFMCOImgPool);
    ImageBufferPool::destroy(mN3dCCinImgPool);
    ImageBufferPool::destroy(mN3dRectinMasterImgPool);
    ImageBufferPool::destroy(mN3dRectinSlaveImgPool);
    TuningBufferPool::destroy(mN3dTuningBufferPool);
    //
    if(mNormalStream != NULL)
    {
        mNormalStream->uninit(N3DP2_USER_NAME);
        mNormalStream->destroyInstance();
        mNormalStream = NULL;
    }
    TRACE_FUNC_EXIT();
    return MTRUE;
}

MBOOL N3D_P2Node::onThreadStart()
{
    TRACE_FUNC_ENTER();
    Timer timer;
    timer.start();
    // master + slave = 2 sets
    mN3dFEBMasterImgPool->allocate(DENOISE_WORKING_BUF_SET);
    mN3dFEBSlaveImgPool->allocate(DENOISE_WORKING_BUF_SET);
    mN3dFECMasterImgPool->allocate(DENOISE_WORKING_BUF_SET);
    mN3dFECSlaveImgPool->allocate(DENOISE_WORKING_BUF_SET);
    mN3dFEBOImgPool->allocate(DENOISE_WORKING_BUF_SET * 2);
    mN3dFECOImgPool->allocate(DENOISE_WORKING_BUF_SET * 2);
    mN3dFMBOImgPool->allocate(DENOISE_WORKING_BUF_SET * 2);
    mN3dFMCOImgPool->allocate(DENOISE_WORKING_BUF_SET * 2);
    mN3dCCinImgPool->allocate(DENOISE_WORKING_BUF_SET * 2);
    //Rectin
    mN3dRectinMasterImgPool->allocate(DENOISE_WORKING_BUF_SET);
    mN3dRectinSlaveImgPool->allocate(DENOISE_WORKING_BUF_SET);
    // tuning buffers
    mN3dTuningBufferPool->allocate(DENOISE_TUNING_BUF_SIZE);

    MY_LOGD("N3D buffers %s in %d ms", STR_ALLOCATE, timer.getElapsed());
    initN3DP2();
    return MTRUE;
    TRACE_FUNC_EXIT();
}

MBOOL N3D_P2Node::onThreadStop()
{
    TRACE_FUNC_ENTER();
    this->waitNormalStreamBaseDone();
    uninitN3DP2();
    TRACE_FUNC_EXIT();
    return MTRUE;
}

MBOOL N3D_P2Node::onData(DataID id, const RequestPtr &data)
{
    TRACE_FUNC_ENTER();
    TRACE_FUNC("Frame %d: %s arrived", data->mRequestNo, ID2Name(id));
    MBOOL ret = MFALSE;
    if( id == ID_P2A_TO_N3DP2 )
    {
        mRequests.enque(data);
        ret = MTRUE;
    }
    TRACE_FUNC_EXIT();
    return ret;
}

MBOOL N3D_P2Node::onThreadLoop()
{
    TRACE_FUNC("Waitloop");
    RequestPtr request;
    if( !waitAllQueue() )
    {
        return MFALSE;
    }
    if( !mRequests.deque(request) )
    {
        MY_LOGE("Request deque out of sync");
        return MFALSE;
    }
    else if( request == NULL )
    {
        MY_LOGE("Request out of sync");
        return MFALSE;
    }

    TRACE_FUNC_ENTER();
    if(!request->needN3D())
    {
        TRACE_FUNC("Frame %d no need n3d, bypass!", request->mRequestNo);
        // wait the previous callback done -> keep order
        this->waitNormalStreamBaseDone();
        TRACE_FUNC("Frame %d no need n3d, bypass! wait done!", request->mRequestNo);
        N3DResult emptyData;
        handleData(ID_N3DP2_TO_N3D, N3DData(emptyData, request));
    }else
    {
        TRACE_FUNC("Frame %d in N3D_P2Node", request->mRequestNo);
        processN3DP2(request);
    }

    TRACE_FUNC_EXIT();
    return MTRUE;
}

MBOOL N3D_P2Node::processN3DP2(const RequestPtr &request)
{
    TRACE_FUNC_ENTER();
    N3DP2EnqueData data;
    QParams param;
    data.mRequest = request;
    prepareN3DImgs(param, request, data);
    prepareCropInfo_N3D(param, request, data);
    request->mTimer.startN3DP2();
    this->incExtThreadDependency();
    this->enqueNormalStreamBase(mNormalStream, param, data);
    TRACE_FUNC_EXIT();
    return MTRUE;
}

MVOID N3D_P2Node::onNormalStreamBaseCB(const QParams &params, const N3DP2EnqueData &data)
{
    TRACE_FUNC_ENTER();
    RequestPtr request = data.mRequest;
    if( request == NULL )
    {
        MY_LOGE("Missing request");
    }
    else
    {
        request->mTimer.stopN3DP2();
        MY_LOGD("sensor(%d) Frame %d enque done in %d ms, result = %d", mSensorIndex, request->mRequestNo, request->mTimer.getElapsedN3DP2(), params.mDequeSuccess);

        if( !params.mDequeSuccess )
        {
            MY_LOGE("Frame %d enque result failed", request->mRequestNo);
            return;
        }
        if(request->needDump())
        {
            dumpData(data.mRequest, data.mN3DResult.mFEBInputImg_Master->getImageBufferPtr(), "mFEBInputImg_Master");
            dumpData(data.mRequest, data.mN3DResult.mFEBInputImg_Slave->getImageBufferPtr(), "mFEBInputImg_Slave");
            dumpData(data.mRequest, data.mN3DResult.mFECInputImg_Master->getImageBufferPtr(), "mFECInputImg_Master");
            dumpData(data.mRequest, data.mN3DResult.mFECInputImg_Slave->getImageBufferPtr(), "mFECInputImg_Slave");
            dumpData(data.mRequest, data.mN3DResult.mCCin_Master->getImageBufferPtr(), "mCCin_Master");
            dumpData(data.mRequest, data.mN3DResult.mCCin_Slave->getImageBufferPtr(), "mCCin_Slave");
            dumpData(data.mRequest, data.mN3DResult.mRectin_Master->getImageBufferPtr(), "mRectin_Master");
            dumpData(data.mRequest, data.mN3DResult.mRectin_Slave->getImageBufferPtr(), "mRectin_Slave");
            dumpData(data.mRequest, data.mN3DResult.mFEBO_Master->getImageBufferPtr(), "mFEBO_Master");
            dumpData(data.mRequest, data.mN3DResult.mFEBO_Slave->getImageBufferPtr(), "mFEBO_Slave");
            dumpData(data.mRequest, data.mN3DResult.mFECO_Master->getImageBufferPtr(), "mFECO_Master");
            dumpData(data.mRequest, data.mN3DResult.mFECO_Slave->getImageBufferPtr(), "mFECO_Slave");
            dumpData(data.mRequest, data.mN3DResult.mFMBO_MtoS->getImageBufferPtr(), "mFMBO_MtoS");
            dumpData(data.mRequest, data.mN3DResult.mFMBO_StoM->getImageBufferPtr(), "mFMBO_StoM");
            dumpData(data.mRequest, data.mN3DResult.mFMCO_MtoS->getImageBufferPtr(), "mFMCO_MtoS");
            dumpData(data.mRequest, data.mN3DResult.mFMCO_StoM->getImageBufferPtr(), "mFMCO_StoM");
        }

        request->mN3D_P2Node_QParams = params;
        request->updateResult(params.mDequeSuccess);
        handleResultData(request, data);
    }
    this->decExtThreadDependency();
    TRACE_FUNC_EXIT();
}

MVOID N3D_P2Node::handleResultData(const RequestPtr &request, const N3DP2EnqueData &data)
{
    TRACE_FUNC_ENTER();
    handleData(ID_N3DP2_TO_N3D, N3DData(data.mN3DResult, request));
    TRACE_FUNC_EXIT();
}

MBOOL N3D_P2Node::initN3DP2()
{
    TRACE_FUNC_ENTER();
    MBOOL ret = MFALSE;
    if( mNormalStream != NULL )
    {
        ret = MTRUE;
    }
    TRACE_FUNC_EXIT();
    return ret;
}

MVOID N3D_P2Node::uninitN3DP2()
{
    TRACE_FUNC_ENTER();
    mNormalStream = NULL;
    TRACE_FUNC_EXIT();
}

MVOID N3D_P2Node::queryFEOBufferSize(MSize iBufSize, MUINT iBlockSize, MUINT32 &riFEOWidth, MUINT32 &riFEOHeight)
{
    riFEOWidth = iBufSize.w/iBlockSize*40;
    riFEOHeight = iBufSize.h/iBlockSize;
    MY_LOGD("queryFEOBufferSize: iBufSize=%dx%d  ouput=%dx%d", iBufSize.w, iBufSize.h, riFEOWidth, riFEOHeight);
}

MVOID N3D_P2Node::queryFMOBufferSize(MUINT32 iFEOWidth, MUINT32 iFEOHeight, MUINT32 &riFMOWidth, MUINT32 &riFMOHeight)
{
    riFMOWidth = (iFEOWidth/40) * 2;
    riFMOHeight = iFEOHeight;
    MY_LOGD("queryFMOBufferSize: iFEOWidth=%d iFEOHeight=%d  ouput=%dx%d", iFEOWidth, iFEOHeight, riFMOWidth, riFMOHeight);
}

MVOID N3D_P2Node::createImgBufPool()
{
    MY_LOGD("+");
    // query size from hal
    StereoSizeProvider* pSizePrvder = StereoSizeProvider::getInstance();
    Pass2SizeInfo pass2SizeInfo;
    // PV frame 0
    pSizePrvder->getPass2SizeInfo(PASS2A, eSTEREO_SCENARIO_RECORD, pass2SizeInfo);
    config.mFEBInputImg_Master = pass2SizeInfo.areaWROT;
    // frame 1
    pSizePrvder->getPass2SizeInfo(PASS2A_P, eSTEREO_SCENARIO_RECORD, pass2SizeInfo);
    config.mFEBInputImg_Slave = pass2SizeInfo.areaWROT;
    // frame 2
    pSizePrvder->getPass2SizeInfo(PASS2A_2, eSTEREO_SCENARIO_PREVIEW, pass2SizeInfo);
    config.mFECInputImg_Master = pass2SizeInfo.areaIMG2O;
    config.mRectin_Master = pass2SizeInfo.areaWDMA.size;
    config.P2A_MAIN2_FEBO_AREA = pass2SizeInfo.areaFEO;
    // frame 3
    pSizePrvder->getPass2SizeInfo(PASS2A_P_2, eSTEREO_SCENARIO_PREVIEW, pass2SizeInfo);
    config.mFECInputImg_Slave = pass2SizeInfo.areaIMG2O;
    config.mRectin_Slave = pass2SizeInfo.areaWDMA.size;
    // frame 4,5
    pSizePrvder->getPass2SizeInfo(PASS2A_P_3, eSTEREO_SCENARIO_RECORD, pass2SizeInfo);
    config.P2A_MAIN2_FECO_AREA = pass2SizeInfo.areaFEO;
    config.mCCinImgSize= pass2SizeInfo.areaIMG2O;
    // create image buffer pool
    mN3dFEBMasterImgPool = ImageBufferPool::create("fpipe.n3dfebmImg", config.mFEBInputImg_Master, eImgFmt_YV12, ImageBufferPool::USAGE_HW);
    mN3dFEBSlaveImgPool = ImageBufferPool::create("fpipe.n3dfebsImg", config.mFEBInputImg_Slave, eImgFmt_YV12, ImageBufferPool::USAGE_HW);
    mN3dFECMasterImgPool = ImageBufferPool::create("fpipe.n3dfecmImg", config.mFECInputImg_Master, eImgFmt_YUY2, ImageBufferPool::USAGE_HW);
    mN3dFECSlaveImgPool = ImageBufferPool::create("fpipe.n3dfecsImg", config.mFECInputImg_Slave, eImgFmt_YUY2, ImageBufferPool::USAGE_HW);
    mN3dRectinMasterImgPool = ImageBufferPool::create("fpipe.n3drectinImg", config.mRectin_Master, eImgFmt_YV12, ImageBufferPool::USAGE_HW);
    mN3dRectinSlaveImgPool = ImageBufferPool::create("fpipe.n3drectinImg", config.mRectin_Slave, eImgFmt_YV12, ImageBufferPool::USAGE_HW);
    mN3dCCinImgPool = ImageBufferPool::create("fpipe.n3dccinImg", config.mCCinImgSize, eImgFmt_Y8, ImageBufferPool::USAGE_HW);
    // create tuning buffer pool
    mN3dTuningBufferPool = TuningBufferPool::create("fpipe.n3dtuningBuf", NSIoPipe::NSPostProc::INormalStream::getRegTableSize());
    // create FE FM Buffer pool
    createFEFMBufferPool();
    MY_LOGD("-");
}

MVOID N3D_P2Node::createFEFMBufferPool()
{
    MY_LOGD("+");
    MUINT32 fe_width=0, fe_height=0;
    MUINT32 fm_width=0, fm_height=0;
    MUINT32 iBlockSize;
    if(DENOISE_CONST_FE_EXEC_TIMES == 2)
    {
        // stage 2 - FEFM
        MUINT32 iBlockSize = StereoSettingProvider::fefmBlockSize(1);
        // calculate the buffers which is the input for FE HW, use Main1 version/because Main2 has SRC CROP
        MSize szFEBufSize = config.mFEBInputImg_Master;
        queryFEOBufferSize(szFEBufSize, iBlockSize, fe_width, fe_height);
        queryFMOBufferSize(fe_width, fe_height, fm_width, fm_height);
        // create image buffer pool
        config.mFMBOImgSize = MSize(fm_width, fm_height);
        mN3dFMBOImgPool = ImageBufferPool::create("fpipe.n3dfmboImg", config.mFMBOImgSize, eImgFmt_STA_BYTE, ImageBufferPool::USAGE_HW);
        config.mFEBOImgSize = MSize(fe_width, fe_height);
        mN3dFEBOImgPool = ImageBufferPool::create("fpipe.n3dfeboImg", config.mFEBOImgSize, eImgFmt_STA_BYTE, ImageBufferPool::USAGE_HW);
        // stage 3 - FEFM
        iBlockSize = StereoSettingProvider::fefmBlockSize(2);
        // calculate the buffers which is the input for FE HW, use Main1 version/because Main2 has SRC CROP
        szFEBufSize = config.mFECInputImg_Master;
        queryFEOBufferSize(szFEBufSize, iBlockSize, fe_width, fe_height);
        queryFMOBufferSize(fe_width, fe_height, fm_width, fm_height);
        config.mFMCOImgSize = MSize(fm_width, fm_height);
        mN3dFMCOImgPool = ImageBufferPool::create("fpipe.n3dfmcoImg", config.mFMCOImgSize, eImgFmt_STA_BYTE, ImageBufferPool::USAGE_HW);
        config.mFECOImgSize = MSize(fe_width, fe_height);
        mN3dFECOImgPool = ImageBufferPool::create("fpipe.n3dfecoImg", config.mFECOImgSize, eImgFmt_STA_BYTE, ImageBufferPool::USAGE_HW);
    }
    else
    {
        MY_LOGE("DENOISE_CONST_FE_EXEC_TIMES const error! val = %d", DENOISE_CONST_FE_EXEC_TIMES);
    }
    MY_LOGD("-");
}

MVOID N3D_P2Node::prepareTemParams()
{
    MY_LOGD("+");
    // FM tuning template
    // frame 7/9 - forward +  frame 6/8 - backward
    for(int iFrameIdx=6; iFrameIdx<10; ++iFrameIdx)
    {
        FMInfo fmInfo;
        setupEmptyTuningWithFM(fmInfo, iFrameIdx);
        mFMTuningBufferMap.add(iFrameIdx, fmInfo);
    }
    // prepare FE tuning buffer
    for (int iStage=1; iStage<=2; ++iStage)
    {
        MUINT32 iBlockSize = StereoSettingProvider::fefmBlockSize(iStage);
        FEInfo feInfo;
        StereoTuningProvider::getFETuningInfo(feInfo, iBlockSize);
        mFETuningBufferMap.add(iStage, feInfo);
    }
    MY_LOGD("-");
}

MVOID N3D_P2Node::setFESizInfo()
{
    MY_LOGD("+");
    // setup FE size info: Stage1
    mN3dStage1SrzInfo.in_w = config.P2A_MAIN2_FEBO_AREA.size.w;
    mN3dStage1SrzInfo.in_h = config.P2A_MAIN2_FEBO_AREA.size.h;
    mN3dStage1SrzInfo.crop_w = config.P2A_MAIN2_FEBO_AREA.size.w - config.P2A_MAIN2_FEBO_AREA.padding.w;
    mN3dStage1SrzInfo.crop_h = config.P2A_MAIN2_FEBO_AREA.size.h - config.P2A_MAIN2_FEBO_AREA.padding.h;
    mN3dStage1SrzInfo.crop_x = config.P2A_MAIN2_FEBO_AREA.startPt.x;
    mN3dStage1SrzInfo.crop_y = config.P2A_MAIN2_FEBO_AREA.startPt.y;
    mN3dStage1SrzInfo.crop_floatX = 0;
    mN3dStage1SrzInfo.crop_floatY = 0;
    mN3dStage1SrzInfo.out_w = config.P2A_MAIN2_FEBO_AREA.size.w - config.P2A_MAIN2_FEBO_AREA.padding.w;
    mN3dStage1SrzInfo.out_h = config.P2A_MAIN2_FEBO_AREA.size.h - config.P2A_MAIN2_FEBO_AREA.padding.h;

    //setup FE size info: Stage2
    mN3dStage2SrzInfo.in_w =  config.P2A_MAIN2_FECO_AREA.size.w;
    mN3dStage2SrzInfo.in_h =  config.P2A_MAIN2_FECO_AREA.size.h;
    mN3dStage2SrzInfo.crop_w = config.P2A_MAIN2_FECO_AREA.size.w - config.P2A_MAIN2_FECO_AREA.padding.w;
    mN3dStage2SrzInfo.crop_h = config.P2A_MAIN2_FECO_AREA.size.h - config.P2A_MAIN2_FECO_AREA.padding.h;
    mN3dStage2SrzInfo.crop_x = config.P2A_MAIN2_FECO_AREA.startPt.x;
    mN3dStage2SrzInfo.crop_y = config.P2A_MAIN2_FECO_AREA.startPt.y;
    mN3dStage2SrzInfo.crop_floatX = 0;
    mN3dStage2SrzInfo.crop_floatY = 0;
    mN3dStage2SrzInfo.out_w = config.P2A_MAIN2_FECO_AREA.size.w - config.P2A_MAIN2_FECO_AREA.padding.w;
    mN3dStage2SrzInfo.out_h = config.P2A_MAIN2_FECO_AREA.size.h - config.P2A_MAIN2_FECO_AREA.padding.h;
    MY_LOGD("-");
}

MVOID N3D_P2Node::setupEmptyTuningWithFM(NSCam::NSIoPipe::FMInfo& fmInfo, MUINT iFrameIdx)
{
    MY_LOGD("+");
    MSize szFEBufSize = (iFrameIdx<=7) ? config.mFEBInputImg_Master : config.mFECInputImg_Master;
    MUINT32 iStage = (iFrameIdx<=7) ? 1 : 2;
    ENUM_FM_DIRECTION eDir = (iFrameIdx % 2 == 0) ? E_FM_L_TO_R : E_FM_R_TO_L;
    // query tuning parameter
    StereoTuningProvider::getFMTuningInfo(eDir, fmInfo);
    MUINT32 iBlockSize =  StereoSettingProvider::fefmBlockSize(iStage);
    // set width/height
    fmInfo.mFMWIDTH = szFEBufSize.w/iBlockSize;
    fmInfo.mFMHEIGHT = szFEBufSize.h/iBlockSize;
    MY_LOGD("-");
}

MBOOL N3D_P2Node::configN3DFrame_0(const RequestPtr &request, N3DResult& rP2AResult, NSCam::NSIoPipe::FrameParams& rFrame, MINT32 iModuleTrans)
{
    // frame 0 : input already exist in the QParams
    MINT32 senID_master = request->getVar<MINT32>(VAR_DUALCAM_FOV_MASTER_ID, -1);
    rFrame.mSensorIdx = senID_master;
    rFrame.mStreamTag = ENormalStreamTag_Normal;
    // input is same as master QParam
    for(ssize_t index=0;index<request->mQParams.mvFrameParams.itemAt(0).mvIn.size();index++)
    {
        rFrame.mvIn.push_back(request->mQParams.mvFrameParams.itemAt(0).mvIn.itemAt(index));
    }
    // output
    Output output = PortID(EPortType_Memory, EPortIndex_WROTO, PORTID_OUT);
    output.mBuffer = rP2AResult.mFEBInputImg_Master->getImageBufferPtr();
    output.mTransform = iModuleTrans;
    rFrame.mvOut.push_back(output);
    //
    rFrame.mTuningData = request->mQParams.mvFrameParams.itemAt(0).mTuningData;
    return MTRUE;
}

MBOOL N3D_P2Node::configN3DFrame_1(const RequestPtr &request, N3DResult& rP2AResult, NSCam::NSIoPipe::FrameParams& rFrame, MINT32 iModuleTrans)
{
    // Get the slave feature param
    FeaturePipeParam fparam_slave;
    if(!request->tryGetVar<FeaturePipeParam>(VAR_DUALCAM_FOV_SLAVE_PARAM, fparam_slave))
    {
        MY_LOGE("Missing slave param");
        return MFALSE;
    }
    fparam_slave = request->getVar<FeaturePipeParam>(VAR_DUALCAM_FOV_SLAVE_PARAM, fparam_slave);
    QParams& qParam_Slave = fparam_slave.mQParams;
    //
    MINT32 senID_slave = request->getVar<MINT32>(VAR_DUALCAM_FOV_SLAVE_ID, -1);
    rFrame.mSensorIdx = senID_slave;
    rFrame.mStreamTag = ENormalStreamTag_Normal;
    // input needs to retrieve from slave feature pipe param
    for(ssize_t index=0;index<qParam_Slave.mvFrameParams.itemAt(0).mvIn.size();index++)
    {
        rFrame.mvIn.push_back(qParam_Slave.mvFrameParams.itemAt(0).mvIn.itemAt(index));
    }
    // output
    Output output = PortID(EPortType_Memory, EPortIndex_WROTO, PORTID_OUT);
    output.mBuffer = rP2AResult.mFEBInputImg_Slave->getImageBufferPtr();
    output.mTransform = iModuleTrans;
    rFrame.mvOut.push_back(output);
    // tuning buffer
    rFrame.mTuningData = qParam_Slave.mvFrameParams.itemAt(0).mTuningData;
    return MTRUE;
}

MBOOL N3D_P2Node::configN3DFrame_2(const RequestPtr &request, N3DResult& rP2AResult, const NSCam::NSIoPipe::FEInfo& pFEInfo, NSCam::NSIoPipe::FrameParams& rFrame)
{
    MINT32 senID_master = request->getVar<MINT32>(VAR_DUALCAM_FOV_MASTER_ID, -1);
    rFrame.mSensorIdx = senID_master;
    rFrame.mStreamTag = ENormalStreamTag_Normal;
    // input
    Input input = PortID(EPortType_Memory, EPortIndex_IMGI, PORTID_IN);
    input.mBuffer = rP2AResult.mFEBInputImg_Master->getImageBufferPtr();
    rFrame.mvIn.push_back(input);
    // output
    // WDMA
    Output output = PortID(EPortType_Memory, EPortIndex_WDMAO, PORTID_OUT);
    output.mBuffer = rP2AResult.mRectin_Master->getImageBufferPtr();
    rFrame.mvOut.push_back(output);
    // IMG2
    output = PortID(EPortType_Memory, EPortIndex_IMG2O, PORTID_OUT);
    output.mBuffer = rP2AResult.mFECInputImg_Master->getImageBufferPtr();
    rFrame.mvOut.push_back(output);
    // FEBO
    output = PortID(EPortType_Memory, EPortIndex_FEO, PORTID_OUT);
    output.mBuffer = rP2AResult.mFEBO_Master->getImageBufferPtr();
    rFrame.mvOut.push_back(output);
    // tuning
    rFrame.mTuningData = rP2AResult.tuningBuf1->mpVA;
    // FE config
    ExtraParam extra;
    extra.CmdIdx = EPIPE_FE_INFO_CMD;
    extra.moduleStruct = (MVOID*)&pFEInfo; // stage 1 FE
    rFrame.mvExtraParam.push_back(extra);
    return MTRUE;
}

MBOOL N3D_P2Node::configN3DFrame_3(const RequestPtr &request, N3DResult& rP2AResult, const NSCam::NSIoPipe::FEInfo& pFEInfo, NSCam::NSIoPipe::FrameParams& rFrame)
{
    MINT32 senID_slave = request->getVar<MINT32>(VAR_DUALCAM_FOV_SLAVE_ID, -1);
    rFrame.mSensorIdx = senID_slave;
    rFrame.mStreamTag = ENormalStreamTag_Normal;
    // input
    Input input = PortID(EPortType_Memory, EPortIndex_IMGI, PORTID_IN);
    input.mBuffer = rP2AResult.mFEBInputImg_Slave->getImageBufferPtr();
    rFrame.mvIn.push_back(input);
    // output
    // WDMA
    Output output = PortID(EPortType_Memory, EPortIndex_WDMAO, PORTID_OUT);
    output.mBuffer = rP2AResult.mRectin_Slave->getImageBufferPtr();
    rFrame.mvOut.push_back(output);
    // IMG2
    output = PortID(EPortType_Memory, EPortIndex_IMG2O, PORTID_OUT);
    output.mBuffer = rP2AResult.mFECInputImg_Slave->getImageBufferPtr();
    rFrame.mvOut.push_back(output);
    // FEBO
    output = PortID(EPortType_Memory, EPortIndex_FEO, PORTID_OUT);
    output.mBuffer = rP2AResult.mFEBO_Slave->getImageBufferPtr();
    rFrame.mvOut.push_back(output);
    // tuning
    rFrame.mTuningData = rP2AResult.tuningBuf2->mpVA;
    // FE config
    ExtraParam extra;
    extra.CmdIdx = EPIPE_FE_INFO_CMD;
    extra.moduleStruct = (MVOID*)&pFEInfo; // stage 1 FE
    rFrame.mvExtraParam.push_back(extra);
    return MTRUE;
}

MBOOL N3D_P2Node::configN3DFrame_4(const RequestPtr &request, N3DResult& rP2AResult, const NSCam::NSIoPipe::FEInfo& pFEInfo, NSCam::NSIoPipe::FrameParams& rFrame)
{
    MINT32 senID_master = request->getVar<MINT32>(VAR_DUALCAM_FOV_MASTER_ID, -1);
    rFrame.mSensorIdx = senID_master;
    rFrame.mStreamTag = ENormalStreamTag_Normal;
    // input
    Input input = PortID(EPortType_Memory, EPortIndex_IMGI, PORTID_IN);
    input.mBuffer = rP2AResult.mFECInputImg_Master->getImageBufferPtr();
    rFrame.mvIn.push_back(input);
    // output
    // WDMA
    Output output = PortID(EPortType_Memory, EPortIndex_WDMAO, PORTID_OUT);
    output.mBuffer = rP2AResult.mCCin_Master->getImageBufferPtr();
    rFrame.mvOut.push_back(output);
    // FECO
    output = PortID(EPortType_Memory, EPortIndex_FEO, PORTID_OUT);
    output.mBuffer = rP2AResult.mFECO_Master->getImageBufferPtr();
    rFrame.mvOut.push_back(output);
    // tuning
    rFrame.mTuningData = rP2AResult.tuningBuf3->mpVA;
    // FE config
    ExtraParam extra;
    extra.CmdIdx = EPIPE_FE_INFO_CMD;
    extra.moduleStruct = (MVOID*)&pFEInfo; // stage 2 FE
    rFrame.mvExtraParam.push_back(extra);
    return MTRUE;
}

MBOOL N3D_P2Node::configN3DFrame_5(const RequestPtr &request, N3DResult& rP2AResult, const NSCam::NSIoPipe::FEInfo& pFEInfo, NSCam::NSIoPipe::FrameParams& rFrame)
{
    MINT32 senID_slave = request->getVar<MINT32>(VAR_DUALCAM_FOV_SLAVE_ID, -1);
    rFrame.mSensorIdx = senID_slave;
    rFrame.mStreamTag = ENormalStreamTag_Normal;
    // input
    Input input = PortID(EPortType_Memory, EPortIndex_IMGI, PORTID_IN);
    input.mBuffer = rP2AResult.mFECInputImg_Slave->getImageBufferPtr();
    rFrame.mvIn.push_back(input);
    // output
    // WDMA
    Output output = PortID(EPortType_Memory, EPortIndex_WDMAO, PORTID_OUT);
    output.mBuffer =rP2AResult.mCCin_Slave->getImageBufferPtr();
    rFrame.mvOut.push_back(output);
    // FECO
    output = PortID(EPortType_Memory, EPortIndex_FEO, PORTID_OUT);
    output.mBuffer = rP2AResult.mFECO_Slave->getImageBufferPtr();
    rFrame.mvOut.push_back(output);
    // tuning
    rFrame.mTuningData = rP2AResult.tuningBuf4->mpVA;
    // FE config
    ExtraParam extra;
    extra.CmdIdx = EPIPE_FE_INFO_CMD;
    extra.moduleStruct = (MVOID*)&pFEInfo; // stage 2 FE
    rFrame.mvExtraParam.push_back(extra);
    return MTRUE;
}

MBOOL N3D_P2Node::configN3DFrame_6(const RequestPtr &request, N3DResult& rP2AResult, const NSCam::NSIoPipe::FMInfo& pFMInfo, NSCam::NSIoPipe::FrameParams& rFrame)
{
    MINT32 senID_master = request->getVar<MINT32>(VAR_DUALCAM_FOV_MASTER_ID, -1);
    rFrame.mSensorIdx = senID_master;
    rFrame.mStreamTag = ENormalStreamTag_FM;
    // input
    Input input = PortID(EPortType_Memory, EPortIndex_DMGI, PORTID_IN);
    input.mBuffer = (STEREO_SENSOR_REAR_MAIN_TOP == StereoSettingProvider::getSensorRelativePosition())?
                    rP2AResult.mFEBO_Master->getImageBufferPtr() :
                    rP2AResult.mFEBO_Slave->getImageBufferPtr();
    rFrame.mvIn.push_back(input);
    input = PortID(EPortType_Memory, EPortIndex_DEPI, PORTID_IN);
    input.mBuffer = (STEREO_SENSOR_REAR_MAIN_TOP == StereoSettingProvider::getSensorRelativePosition())?
                    rP2AResult.mFEBO_Slave->getImageBufferPtr() :
                    rP2AResult.mFEBO_Master->getImageBufferPtr();
     rFrame.mvIn.push_back(input);
    // output
    Output output = PortID(EPortType_Memory, EPortIndex_MFBO, PORTID_OUT);
    output.mBuffer = rP2AResult.mFMBO_StoM->getImageBufferPtr();
    rFrame.mvOut.push_back(output);
    // tuning
    rFrame.mTuningData = rP2AResult.tuningBuf5->mpVA;
    // FM config
    ExtraParam extra;
    extra.CmdIdx = EPIPE_FM_INFO_CMD;
    extra.moduleStruct = (MVOID*)&pFMInfo;
    rFrame.mvExtraParam.push_back(extra);
    return MTRUE;
}

MBOOL N3D_P2Node::configN3DFrame_7(const RequestPtr &request, N3DResult& rP2AResult, const NSCam::NSIoPipe::FMInfo& pFMInfo, NSCam::NSIoPipe::FrameParams& rFrame)
{
    MINT32 senID_slave = request->getVar<MINT32>(VAR_DUALCAM_FOV_SLAVE_ID, -1);
    rFrame.mSensorIdx = senID_slave;
    rFrame.mStreamTag = ENormalStreamTag_FM;
    // input
    Input input = PortID(EPortType_Memory, EPortIndex_DEPI, PORTID_IN);
    input.mBuffer = (STEREO_SENSOR_REAR_MAIN_TOP == StereoSettingProvider::getSensorRelativePosition())?
                    rP2AResult.mFEBO_Master->getImageBufferPtr() :
                    rP2AResult.mFEBO_Slave->getImageBufferPtr();
    rFrame.mvIn.push_back(input);
    input = PortID(EPortType_Memory, EPortIndex_DMGI, PORTID_IN);
    input.mBuffer = (STEREO_SENSOR_REAR_MAIN_TOP == StereoSettingProvider::getSensorRelativePosition())?
                    rP2AResult.mFEBO_Slave->getImageBufferPtr() :
                    rP2AResult.mFEBO_Master->getImageBufferPtr();
    rFrame.mvIn.push_back(input);
    // output
    Output output = PortID(EPortType_Memory, EPortIndex_MFBO, PORTID_OUT);
    output.mBuffer = rP2AResult.mFMBO_MtoS->getImageBufferPtr();
    rFrame.mvOut.push_back(output);
    // tuning
    rFrame.mTuningData = rP2AResult.tuningBuf6->mpVA;
    // FM config
    ExtraParam extra;
    extra.CmdIdx = EPIPE_FM_INFO_CMD;
    extra.moduleStruct = (MVOID*)&pFMInfo;
    rFrame.mvExtraParam.push_back(extra);
    return MTRUE;
}

MBOOL N3D_P2Node::configN3DFrame_8(const RequestPtr &request, N3DResult& rP2AResult, const NSCam::NSIoPipe::FMInfo& pFMInfo, NSCam::NSIoPipe::FrameParams& rFrame)
{
    MINT32 senID_master = request->getVar<MINT32>(VAR_DUALCAM_FOV_MASTER_ID, -1);
    rFrame.mSensorIdx = senID_master;
    rFrame.mStreamTag = ENormalStreamTag_FM;
    // input
    Input input = PortID(EPortType_Memory, EPortIndex_DMGI, PORTID_IN);
    input.mBuffer = (STEREO_SENSOR_REAR_MAIN_TOP == StereoSettingProvider::getSensorRelativePosition())?
                    rP2AResult.mFECO_Master->getImageBufferPtr() :
                    rP2AResult.mFECO_Slave->getImageBufferPtr();
    rFrame.mvIn.push_back(input);
    input = PortID(EPortType_Memory, EPortIndex_DEPI, PORTID_IN);
    input.mBuffer = (STEREO_SENSOR_REAR_MAIN_TOP == StereoSettingProvider::getSensorRelativePosition())?
                    rP2AResult.mFECO_Slave->getImageBufferPtr() :
                    rP2AResult.mFECO_Master->getImageBufferPtr();
    rFrame.mvIn.push_back(input);
    // output
    Output output = PortID(EPortType_Memory, EPortIndex_MFBO, PORTID_OUT);
    output.mBuffer =  rP2AResult.mFMCO_StoM->getImageBufferPtr();
    rFrame.mvOut.push_back(output);
    // tuning
    rFrame.mTuningData = rP2AResult.tuningBuf7->mpVA;
    // FM config
    ExtraParam extra;
    extra.CmdIdx = EPIPE_FM_INFO_CMD;
    extra.moduleStruct = (MVOID*)&pFMInfo;
    rFrame.mvExtraParam.push_back(extra);
    return MTRUE;
}

MBOOL N3D_P2Node::configN3DFrame_9(const RequestPtr &request, N3DResult& rP2AResult, const NSCam::NSIoPipe::FMInfo& pFMInfo, NSCam::NSIoPipe::FrameParams& rFrame)
{
    MINT32 senID_slave = request->getVar<MINT32>(VAR_DUALCAM_FOV_SLAVE_ID, -1);
    rFrame.mSensorIdx = senID_slave;
    rFrame.mStreamTag = ENormalStreamTag_FM;
    // input
    Input input = PortID(EPortType_Memory, EPortIndex_DEPI, PORTID_IN);
    input.mBuffer = (STEREO_SENSOR_REAR_MAIN_TOP == StereoSettingProvider::getSensorRelativePosition())?
                    rP2AResult.mFECO_Master->getImageBufferPtr() :
                    rP2AResult.mFECO_Slave->getImageBufferPtr();
    rFrame.mvIn.push_back(input);
    input = PortID(EPortType_Memory, EPortIndex_DMGI, PORTID_IN);
    input.mBuffer = (STEREO_SENSOR_REAR_MAIN_TOP == StereoSettingProvider::getSensorRelativePosition())?
                    rP2AResult.mFECO_Slave->getImageBufferPtr() :
                    rP2AResult.mFECO_Master->getImageBufferPtr();
    rFrame.mvIn.push_back(input);
    // output
    Output output = PortID(EPortType_Memory, EPortIndex_MFBO, PORTID_OUT);
    output.mBuffer = rP2AResult.mFMCO_MtoS->getImageBufferPtr();
    rFrame.mvOut.push_back(output);
    // tuning
    rFrame.mTuningData = rP2AResult.tuningBuf8->mpVA;
    // FM config
    ExtraParam extra;
    extra.CmdIdx = EPIPE_FM_INFO_CMD;
    extra.moduleStruct = (MVOID*)&pFMInfo;
    rFrame.mvExtraParam.push_back(extra);
    return MTRUE;
}

MVOID N3D_P2Node::prepareN3DImgs(QParams &params, const RequestPtr &request, N3DP2EnqueData &data)
{
    TRACE_FUNC_ENTER();
    N3DResult& rP2AResult = data.mN3DResult;
    rP2AResult.mFEBInputImg_Master = mN3dFEBMasterImgPool->requestIIBuffer();
    rP2AResult.mFEBInputImg_Slave = mN3dFEBSlaveImgPool->requestIIBuffer();
    rP2AResult.mFECInputImg_Master = mN3dFECMasterImgPool->requestIIBuffer();
    rP2AResult.mFECInputImg_Slave = mN3dFECSlaveImgPool->requestIIBuffer();
    rP2AResult.mFEBO_Master = mN3dFEBOImgPool->requestIIBuffer();
    rP2AResult.mFEBO_Slave = mN3dFEBOImgPool->requestIIBuffer();
    rP2AResult.mFECO_Master = mN3dFECOImgPool->requestIIBuffer();
    rP2AResult.mFECO_Slave = mN3dFECOImgPool->requestIIBuffer();
    rP2AResult.mFMBO_MtoS = mN3dFMBOImgPool->requestIIBuffer();
    rP2AResult.mFMBO_StoM = mN3dFMBOImgPool->requestIIBuffer();
    rP2AResult.mFMCO_MtoS = mN3dFMCOImgPool->requestIIBuffer();
    rP2AResult.mFMCO_StoM = mN3dFMCOImgPool->requestIIBuffer();
    rP2AResult.mCCin_Master = mN3dCCinImgPool->requestIIBuffer();
    rP2AResult.mCCin_Slave = mN3dCCinImgPool->requestIIBuffer();
    rP2AResult.mRectin_Master = mN3dRectinMasterImgPool->requestIIBuffer();
    rP2AResult.mRectin_Slave = mN3dRectinSlaveImgPool->requestIIBuffer();
    rP2AResult.tuningBuf1 = mN3dTuningBufferPool->request();
    rP2AResult.tuningBuf2 = mN3dTuningBufferPool->request();
    rP2AResult.tuningBuf3 = mN3dTuningBufferPool->request();
    rP2AResult.tuningBuf4 = mN3dTuningBufferPool->request();
    rP2AResult.tuningBuf5 = mN3dTuningBufferPool->request();
    rP2AResult.tuningBuf6 = mN3dTuningBufferPool->request();
    rP2AResult.tuningBuf7 = mN3dTuningBufferPool->request();
    rP2AResult.tuningBuf8 = mN3dTuningBufferPool->request();

    // module rotation
    ENUM_ROTATION eRot = StereoSettingProvider::getModuleRotation();
    MINT32 iModuleTrans = -1;
    switch(eRot)
    {
        case eRotate_0:
            iModuleTrans = 0;
            break;
        case eRotate_90:
            iModuleTrans = eTransform_ROT_90;
            break;
        case eRotate_180:
            iModuleTrans = eTransform_ROT_180;
            break;
        case eRotate_270:
            iModuleTrans = eTransform_ROT_270;
            break;
        default:
            MY_LOGE("Not support module rotation =%d", eRot);
    }

    // Get current frame num
    int iFrameIdx = 0;
    MBOOL bRet = MFALSE;
    // frame 0
    params.mvFrameParams.push_back(FrameParams());
    bRet = configN3DFrame_0(request, rP2AResult, params.mvFrameParams.editItemAt(iFrameIdx++), iModuleTrans);
    // frame 1
    params.mvFrameParams.push_back(FrameParams());
    bRet &= configN3DFrame_1(request, rP2AResult, params.mvFrameParams.editItemAt(iFrameIdx++), iModuleTrans);
    // frame 2
    params.mvFrameParams.push_back(FrameParams());
    bRet &= configN3DFrame_2(request, rP2AResult, mFETuningBufferMap.valueFor(1),
                             params.mvFrameParams.editItemAt(iFrameIdx++));
    // frame 3
    params.mvFrameParams.push_back(FrameParams());
    bRet &= configN3DFrame_3(request, rP2AResult, mFETuningBufferMap.valueFor(1),
                             params.mvFrameParams.editItemAt(iFrameIdx++));
    // frame 4
    params.mvFrameParams.push_back(FrameParams());
    bRet &= configN3DFrame_4(request, rP2AResult, mFETuningBufferMap.valueFor(2),
                             params.mvFrameParams.editItemAt(iFrameIdx++));
    // frame 5
    params.mvFrameParams.push_back(FrameParams());
    bRet &= configN3DFrame_5(request, rP2AResult, mFETuningBufferMap.valueFor(2),
                             params.mvFrameParams.editItemAt(iFrameIdx++));
    // frame 6
    params.mvFrameParams.push_back(FrameParams());
    bRet &= configN3DFrame_6(request, rP2AResult, mFMTuningBufferMap.valueFor(6),
                             params.mvFrameParams.editItemAt(iFrameIdx++));
    // frame 7
    params.mvFrameParams.push_back(FrameParams());
    bRet &= configN3DFrame_7(request, rP2AResult, mFMTuningBufferMap.valueFor(7),
                             params.mvFrameParams.editItemAt(iFrameIdx++));
    // frame 8
    params.mvFrameParams.push_back(FrameParams());
    bRet &= configN3DFrame_8(request, rP2AResult, mFMTuningBufferMap.valueFor(8),
                             params.mvFrameParams.editItemAt(iFrameIdx++));
    // frame 9
    params.mvFrameParams.push_back(FrameParams());
    bRet &= configN3DFrame_9(request, rP2AResult, mFMTuningBufferMap.valueFor(9),
                             params.mvFrameParams.editItemAt(iFrameIdx++));
    if(!bRet)
    {
        MY_LOGE("Failed to config N3D Input/Output!");
    }
    TRACE_FUNC_EXIT();
}

MBOOL N3D_P2Node::calCropForScreen(MPoint &rCropStartPt, MSize& rCropSize)
{
    MSize srcSize = rCropSize;

    MRect rect;
    rect.p.x = 0;
    rect.p.y = 0;
    rect.s.w = srcSize.w;
    rect.s.h = srcSize.h;

    CropUtil::cropRectByImageRatio(rect, StereoSettingProvider::imageRatio());

    rCropStartPt.x = rect.p.x;
    rCropStartPt.y = rect.p.y;
    rCropSize.w = rect.s.w;
    rCropSize.h = rect.s.h;


    return MTRUE;

}

MBOOL N3D_P2Node::cropN3DFrame_0(const RequestPtr &request, N3DResult& rP2AResult, NSCam::NSIoPipe::FrameParams& rFrame)
{
    IImageBuffer *pImgBuf = rP2AResult.mFEBInputImg_Master->getImageBufferPtr();
    MCrpRsInfo crop;
    crop.mGroupID = 3; // WROT
    MPoint startPointMain1;
    MSize cropSizeMain1 = request->mFullImgSize;
    calCropForScreen(startPointMain1, cropSizeMain1);
    crop.mCropRect = MCropRect(startPointMain1, cropSizeMain1);
    crop.mResizeDst = pImgBuf->getImgSize();
    rFrame.mvCropRsInfo.push_back(crop);
    return MTRUE;
}

MBOOL N3D_P2Node::cropN3DFrame_1(const RequestPtr &request, N3DResult& rP2AResult, NSCam::NSIoPipe::FrameParams& rFrame)
{
    // Get the slave feature param
    FeaturePipeParam fparam_slave;
    fparam_slave = request->getVar<FeaturePipeParam>(VAR_DUALCAM_FOV_SLAVE_PARAM, fparam_slave);
    QParams& qParam_Slave = fparam_slave.mQParams;
    IImageBuffer *pImgBuf = qParam_Slave.mvFrameParams.itemAt(0).mvIn[0].mBuffer;
    MCrpRsInfo crop;
    crop.mGroupID = 3; // WROT
    MPoint startPointMain2;
    MSize cropSizeMain2 = pImgBuf->getImgSize();
    calCropForScreen(startPointMain2, cropSizeMain2);
    crop.mCropRect = MCropRect(startPointMain2, cropSizeMain2);
    crop.mResizeDst = rP2AResult.mFEBInputImg_Slave->getImageBufferPtr()->getImgSize();
    rFrame.mvCropRsInfo.push_back(crop);
    return MTRUE;
}

MBOOL N3D_P2Node::cropN3DFrame_2(const RequestPtr &request, N3DResult& rP2AResult, NSCam::NSIoPipe::FrameParams& rFrame)
{
    IImageBuffer *pImgBuf = rP2AResult.mFEBInputImg_Master->getImageBufferPtr();
    MCrpRsInfo crop;
    crop.mGroupID = 1; // CRZ
    crop.mCropRect = MCropRect(MPoint(0, 0), pImgBuf->getImgSize());
    pImgBuf = rP2AResult.mFECInputImg_Master->getImageBufferPtr();
    crop.mResizeDst = pImgBuf->getImgSize();
    rFrame.mvCropRsInfo.push_back(crop);
    pImgBuf = rP2AResult.mFEBInputImg_Master->getImageBufferPtr();
    crop.mGroupID = 2; // WDMA
    crop.mCropRect = MCropRect(MPoint(0, 0), pImgBuf->getImgSize());
    pImgBuf = rP2AResult.mRectin_Master->getImageBufferPtr();
    crop.mResizeDst = pImgBuf->getImgSize();
    rFrame.mvCropRsInfo.push_back(crop);
    return MTRUE;
}

MBOOL N3D_P2Node::cropN3DFrame_3(const RequestPtr &request, N3DResult& rP2AResult, NSCam::NSIoPipe::FrameParams& rFrame)
{
    IImageBuffer *pImgBuf = rP2AResult.mFEBInputImg_Slave->getImageBufferPtr();
    MCrpRsInfo crop;
    crop.mGroupID = 1; // CRZ
    crop.mCropRect = MCropRect(MPoint(0, 0), pImgBuf->getImgSize());
    pImgBuf = rP2AResult.mFECInputImg_Slave->getImageBufferPtr();
    crop.mResizeDst = pImgBuf->getImgSize();
    rFrame.mvCropRsInfo.push_back(crop);
    pImgBuf = rP2AResult.mFEBInputImg_Slave->getImageBufferPtr();
    crop.mGroupID = 2; // WDMA
    crop.mCropRect = MCropRect(MPoint(0, 0), pImgBuf->getImgSize());
    pImgBuf = rP2AResult.mRectin_Slave->getImageBufferPtr();
    crop.mResizeDst = pImgBuf->getImgSize();
    rFrame.mvCropRsInfo.push_back(crop);
    setFESizInfo();//
    ModuleInfo moduleInfo;
    moduleInfo.moduleTag = EDipModule_SRZ1;
    moduleInfo.moduleStruct = reinterpret_cast<MVOID*> (&mN3dStage1SrzInfo);
    rFrame.mvModuleData.push_back(moduleInfo);
    return MTRUE;
}

MBOOL N3D_P2Node::cropN3DFrame_4(const RequestPtr &request, N3DResult& rP2AResult, NSCam::NSIoPipe::FrameParams& rFrame)
{
    IImageBuffer *pImgBuf =  rP2AResult.mFECInputImg_Master->getImageBufferPtr();
    MCrpRsInfo crop;
    crop.mGroupID = 2; // WDMA
    crop.mCropRect = MCropRect(MPoint(0, 0), pImgBuf->getImgSize());
    pImgBuf =  rP2AResult.mCCin_Master->getImageBufferPtr();
    crop.mResizeDst = pImgBuf->getImgSize();
    rFrame.mvCropRsInfo.push_back(crop);
    return MTRUE;
}

MBOOL N3D_P2Node::cropN3DFrame_5(const RequestPtr &request,N3DResult& rP2AResult, NSCam::NSIoPipe::FrameParams& rFrame)
{
    IImageBuffer *pImgBuf = rP2AResult.mFECInputImg_Slave->getImageBufferPtr();
    MCrpRsInfo crop;
    crop.mGroupID = 2; // WDMA
    crop.mCropRect = MCropRect(MPoint(0, 0), pImgBuf->getImgSize());
    pImgBuf = rP2AResult.mCCin_Slave->getImageBufferPtr();
    crop.mResizeDst = pImgBuf->getImgSize();
    rFrame.mvCropRsInfo.push_back(crop);
    ModuleInfo moduleInfo;
    moduleInfo.moduleTag = EDipModule_SRZ1;
    moduleInfo.moduleStruct = reinterpret_cast<MVOID*> (&mN3dStage2SrzInfo);
    rFrame.mvModuleData.push_back(moduleInfo);
    return MTRUE;
}

MVOID N3D_P2Node::prepareCropInfo_N3D(QParams &params, const RequestPtr &request, N3DP2EnqueData &data)
{
    TRACE_FUNC_ENTER();
    N3DResult& rP2AResult = data.mN3DResult;
    int iFrameIdx = 0;
    MBOOL bRet = MFALSE;
    // frame 0
    bRet = cropN3DFrame_0(request, rP2AResult, params.mvFrameParams.editItemAt(iFrameIdx++));
    // frame 1
    bRet &= cropN3DFrame_1(request, rP2AResult, params.mvFrameParams.editItemAt(iFrameIdx++));
    // frame 2
    bRet &= cropN3DFrame_2(request, rP2AResult, params.mvFrameParams.editItemAt(iFrameIdx++));
    // frame 3
    bRet &= cropN3DFrame_3(request, rP2AResult, params.mvFrameParams.editItemAt(iFrameIdx++));
    // frame 4
    bRet &= cropN3DFrame_4(request, rP2AResult, params.mvFrameParams.editItemAt(iFrameIdx++));
    // frame 5
    bRet &= cropN3DFrame_5(request, rP2AResult, params.mvFrameParams.editItemAt(iFrameIdx++));
    // frame 6~9 - FM only, no crop and module info
    if(!bRet)
    {
        MY_LOGE("Failed to config N3D Input/Output!");
    }
    TRACE_FUNC_EXIT();
}

} // namespace NSFeaturePipe
} // namespace NSCamFeature
} // namespace NSCam
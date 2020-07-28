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
//
#define MY_LOGV_IF(cond, ...)       do { if ( (cond) ) { MY_LOGV(__VA_ARGS__); } }while(0)
#define MY_LOGD_IF(cond, ...)       do { if ( (cond) ) { MY_LOGD(__VA_ARGS__); } }while(0)
#define MY_LOGI_IF(cond, ...)       do { if ( (cond) ) { MY_LOGI(__VA_ARGS__); } }while(0)
#define MY_LOGW_IF(cond, ...)       do { if ( (cond) ) { MY_LOGW(__VA_ARGS__); } }while(0)
#define MY_LOGE_IF(cond, ...)       do { if ( (cond) ) { MY_LOGE(__VA_ARGS__); } }while(0)
#define MY_LOGA_IF(cond, ...)       do { if ( (cond) ) { MY_LOGA(__VA_ARGS__); } }while(0)
#define MY_LOGF_IF(cond, ...)       do { if ( (cond) ) { MY_LOGF(__VA_ARGS__); } }while(0)
//
#include <mtkcam/feature/stereo/pipe/vsdof_common.h>
#include <mtkcam/feature/stereo/pipe/vsdof_data_define.h>

#include "bokeh_common.h"
//
#define NODE_NAME "VSDOF_P2BNode"
//
#define PIPE_MODULE_TAG "BokehPipe"
#define PIPE_CLASS_TAG "VSDOF_P2BNode"
#include <PipeLog.h>
#include <drv/isp_reg.h>
#include <mtkcam/feature/stereo/hal/stereo_size_provider.h>
#include <stereo_tuning_provider.h>
#include <mtkcam/drv/def/Dip_Notify_datatype.h>
//
#include "VSDOF_P2BNode.h"
#include <vsdof/util/vsdof_util.h>
// for NormalStream
#define FEATURE_PIPE_CALLER_NAME "FeaturePipe_Bokeh_P2B"
// tuning buffer alloc size
#define TUNING_BUFFER_ALLOC_SIZE 3

#include <string>
//
#include <DpBlitStream.h>
//
#include <mtkcam/aaa/IHal3A.h>
//
using namespace android;
using namespace std;

using namespace NSCam::NSCamFeature::NSFeaturePipe;
using namespace NSCam::NSIoPipe::NSPostProc;
using namespace NSCam::NSIoPipe;
//************************************************************************
// utility function
//************************************************************************
#define NS_PER_SEC  1000000000
#define NS_PER_MS   1000000
#define NS_PER_US   1000
static void Wait(int ms)
{
  long waitSec;
  long waitNSec;
  waitSec = (ms * NS_PER_MS) / NS_PER_SEC;
  waitNSec = (ms * NS_PER_MS) % NS_PER_SEC;
  struct timespec t;
  t.tv_sec = waitSec;
  t.tv_nsec = waitNSec;
  if( nanosleep(&t, NULL) != 0 )
  {
  }
}
/*******************************************************************************
 *
 ********************************************************************************/
VSDOF_P2BNode::
VSDOF_P2BNode(
    const char *name,
    Graph_T *graph,
    MINT32 openId,
    MINT8 mode)
    : BokehPipeNode(name, graph)
    , miOpenId(openId)
    , miMode(mode)
    , mpDebugDrawID(nullptr)
{
    MY_LOGD("ctor(0x%x)", this);
    this->addWaitQueue(&mRequests);
    char cLogLevel[PROPERTY_VALUE_MAX];
    ::property_get("vendor.debug.vsdof.showdepthmap", cLogLevel, "0");
    mShowDepthMap = ::atoi(cLogLevel);

    extract_by_SensorOpenId(&mDumpHint, miOpenId);

    mpDebugDrawID = DebugDrawID::createInstance();

    MY_LOGD("miPipeLogEnable(%d) mode(%d) openid(%x)", miPipeLogEnable, miMode, miOpenId);
}
/*******************************************************************************
 *
 ********************************************************************************/
VSDOF_P2BNode::
~VSDOF_P2BNode()
{
    if( mpDebugDrawID != nullptr ){
        mpDebugDrawID->destroyInstance();
        mpDebugDrawID = nullptr;
    }
    MY_LOGD("dctor(0x%x)", this);
}
/*******************************************************************************
 *
 ********************************************************************************/
MBOOL
VSDOF_P2BNode::
onInit()
{
    CAM_TRACE_NAME("VSDOF_P2BNode::onInit");
    BokehPipeNode::onInit();
    //
    return MTRUE;
}
/*******************************************************************************
 *
 ********************************************************************************/
MBOOL
VSDOF_P2BNode::
createBufferPool(
    android::sp<ImageBufferPool> &pPool,
    MUINT32 width,
    MUINT32 height,
    NSCam::EImageFormat format,
    MUINT32 bufCount,
    const char* caller,
    MUINT32 bufUsage,
    MBOOL continuesBuffer
)
{
    FUNC_START;
    MBOOL ret = MFALSE;
    pPool = ImageBufferPool::create(caller, width, height, format, bufUsage, continuesBuffer);
    if(pPool == nullptr)
    {
        ret = MFALSE;
        MY_LOGE("Create [%s] failed.", caller);
        goto lbExit;
    }
    for(MUINT32 i=0;i<bufCount;++i)
    {
        if(!pPool->allocate())
        {
            ret = MFALSE;
            MY_LOGE("Allocate [%s] working buffer failed.", caller);
            goto lbExit;
        }
    }
    ret = MTRUE;
    FUNC_END;
lbExit:
    return ret;
}
/*******************************************************************************
 *
 ********************************************************************************/
MBOOL
VSDOF_P2BNode::
onUninit()
{
    CAM_TRACE_NAME("VSDOF_P2BNode::onUninit");
    cleanUp();
    return MTRUE;
}
/*******************************************************************************
 *
 ********************************************************************************/
MVOID
VSDOF_P2BNode::
cleanUp()
{
    FUNC_START;
    if(nullptr != mpINormalStream)
    {
        MY_LOGD("release mpINormalStream +");
        mpINormalStream->uninit(getName());
        mpINormalStream->destroyInstance();
        MY_LOGD("release mpINormalStream -");
    }
    mpINormalStream = nullptr;
    // dump all queue size
    MY_LOGD("mRequests.size(%d)", mRequests.size());
    if(mpDpStream!= nullptr)
        delete mpDpStream;
    // Tuning buffer pool
    TuningBufferPool::destroy(mpTuningBufferPool);
    // 3A
    if(mp3AHal_Main1)
    {
        mp3AHal_Main1->destroyInstance("BOKEH_3A_MAIN1");
        mp3AHal_Main1 = NULL;
    }
    FUNC_END;
}
/*******************************************************************************
 *
 ********************************************************************************/
MBOOL
VSDOF_P2BNode::
onThreadStart()
{
    TRACE_FUNC_ENTER();
    FUNC_START;
    MBOOL ret = MTRUE;
    const NSCam::EImageFormat imgFmt = eImgFmt_YV12;
    const MINT32 usage = ImageBufferPool::USAGE_HW;
    CAM_TRACE_BEGIN("VSDOF_P2BNode::NormalStream::createInstance+init");
    // Create NormalStream
    mpINormalStream = INormalStream::createInstance(miOpenId);
    if(nullptr == mpINormalStream)
    {
        MY_LOGE("INormalStream: create instance for FeaturePipe_P2 failed!");
        return MFALSE;
    }
    mpINormalStream->init(getName());
    CAM_TRACE_END();
    // set dmgi & depi size
    StereoArea sDmgArea =
            StereoSizeProvider::getInstance()->getBufferSize(E_DMG, eSTEREO_SCENARIO_PREVIEW);
    miDmgi = sDmgArea.size;
    StereoArea sDmbgArea =
            StereoSizeProvider::getInstance()->getBufferSize(E_DMBG, eSTEREO_SCENARIO_PREVIEW);
    miDepi = sDmbgArea.size;
    MY_LOGD("DMGI size(%dx%d) DEPI size(%dx%d)",
                    miDmgi.w,
                    miDmgi.h,
                    miDepi.w,
                    miDepi.h);
    //
    if(mShowDepthMap)
    {
        createBufferPool(
                mpDepthMapBufPool,
                720,
                408,
                eImgFmt_Y8,
                2,
                "BOKEH_P2B_DEPTHMAP_BUF",
                usage,
                MTRUE);
        mpDepthMapBufPool->allocate(2);
        mpDpStream = new DpBlitStream();
    }
    // set tuning buffer pool
    MY_LOGD("[%s] tuningBufferCount:%d", "Bokeh", TUNING_BUFFER_ALLOC_SIZE);
    mpTuningBufferPool = TuningBufferPool::create("Bokeh", NSIoPipe::NSPostProc::INormalStream::getRegTableSize());
    mpTuningBufferPool->allocate(TUNING_BUFFER_ALLOC_SIZE);
    // 3A: create instance
    // UT does not test 3A
    CAM_TRACE_BEGIN("Bokeh::P2BNode::create_3A_instance");
    #ifndef GTEST
    mp3AHal_Main1 = MAKE_Hal3A(miOpenId, "BOKEH_3A_MAIN1");
    MY_LOGD("3A create instance, Main1: %x", mp3AHal_Main1 );
    #endif
    CAM_TRACE_END();


    FUNC_END;
    TRACE_FUNC_EXIT();
    return MTRUE;
}
/*******************************************************************************
 *
 ********************************************************************************/
MBOOL
VSDOF_P2BNode::
onThreadStop()
{
    FUNC_START;
    FUNC_END;
    return MTRUE;
}
/*******************************************************************************
 *
 ********************************************************************************/
MBOOL
VSDOF_P2BNode::
onData(
    DataID id,
    EffectRequestPtr &request
)
{
    TRACE_FUNC_ENTER();

    MBOOL ret;
    switch(id)
    {
        case ID_ROOT_ENQUE:
            mRequests.enque(request);
            ret = MTRUE;
            break;
        default:
            ret = MFALSE;
            break;
    }

    TRACE_FUNC_EXIT();
    return ret;
}
/*******************************************************************************
 *
 ********************************************************************************/
MBOOL
VSDOF_P2BNode::
onThreadLoop()
{
    EffectRequestPtr request = nullptr;

    if( !waitAllQueue() )
    {
        // mJobs.abort() called
        return MFALSE;
    }
    if( !mRequests.deque(request) )
    {
        MY_LOGD("mRequests.deque() failed");
        return MFALSE;
    }
    CAM_TRACE_NAME("VSDOF_P2BNode::onThreadLoop");

    if(!enqueP2B(request/*, dmgBuffer, dmbgBuffer*/))
    {
        MY_LOGE("enqueP2B fail.");
        return MFALSE;
    }
    return MTRUE;
}
/*******************************************************************************
 *
 ********************************************************************************/
MBOOL
VSDOF_P2BNode::
enqueP2B(EffectRequestPtr request)
{
    CAM_TRACE_NAME("VSDOF_P2BNode::enqueP2B");
    MY_LOGD_IF(miPipeLogEnable, "enqueP2B + %d", request->getRequestNo());
    // prepare enque parameter.
    QParams enqueParams;
    FrameParams frameParams;
    enqueData *data = nullptr;

    SmartTuningBuffer pTuningBuffer = nullptr;
    SmartImageBuffer pWDMABuffer = nullptr;
    StereoArea sImgiSize;
    MSize outputImageSize;
    //
    MBOOL ret = MFALSE;
    //
    MSize imgiSize;
    // get scenario id.
    const sp<EffectParameter> params = request->getRequestParameter();
    MINT32 scenarioId = params->getInt(VSDOF_FRAME_SCENARIO);
    auto convertAngleToEnum = [](MINT32 angle)->MINT32
    {
        switch(angle)
        {
            case 90:
                return eTransform_ROT_90;
            case 180:
                return eTransform_ROT_180;
            case 270:
                return eTransform_ROT_270;
            default:
                return 0;
        }
        return 0;
    };
    MINT32 gsensorOri = convertAngleToEnum(params->getInt(VSDOF_FRAME_G_SENSOR_ORIENTATION));
    //
    if(!unpackInputBufferFromRequest(request, frameParams))
    {
        goto lbExit;
    }
    unpackOutputBufferFromRequest(request, frameParams, gsensorOri);
    // get imgi size
    for(auto data : frameParams.mvIn)
    {
        if(data.mPortID==PORT_IMGI)
        {
            imgiSize = data.mBuffer->getImgSize();
            break;
        }
    }
    // set mdp crop info
    for(auto data : frameParams.mvOut)
    {
        // Both MDMA and WROT need to set CRZ crop info.
        MCrpRsInfo mdp_crop;
        MCrpRsInfo crz_crop;
        if(data.mPortID==PORT_WDMAO)
        {
            // MDP
            mdp_crop.mGroupID = 2;
            MSize const& imgSize = data.mBuffer->getImgSize();
            //mdp_crop.mCropRect = MCropRect(imgSize.w, imgSize.h);
            mdp_crop.mCropRect = MCropRect(imgiSize.w, imgiSize.h);
            mdp_crop.mResizeDst = imgSize;
            // CRZ
            crz_crop.mGroupID = 1;
            crz_crop.mCropRect = MCropRect(imgiSize.w, imgiSize.h);
            crz_crop.mResizeDst = imgiSize;
        }
        else if(data.mPortID==PORT_WROTO)
        {
            // MDP
            mdp_crop.mGroupID = 3;
            MSize const& imgSize = data.mBuffer->getImgSize();
            MSize dstSize = ( data.mTransform == eTransform_ROT_90 || data.mTransform == eTransform_ROT_270 )
                                ? MSize(imgSize.h, imgSize.w)
                                : imgSize;

            mdp_crop.mCropRect = MCropRect(imgiSize.w, imgiSize.h);
            mdp_crop.mResizeDst = dstSize;
            // CRZ
            crz_crop.mGroupID = 1;
            crz_crop.mCropRect = MCropRect(imgiSize.w, imgiSize.h);
            crz_crop.mResizeDst = imgiSize;
        }
        if(miPipeLogEnable)
        {
            MY_LOGD("IMGI size(%dx%d)",
                    imgiSize.w,
                    imgiSize.h);
            MY_LOGD("P2B MDP setting: GroupId(%d) CropRect(%dx%d)",
                    mdp_crop.mGroupID,
                    mdp_crop.mCropRect.s.w,
                    mdp_crop.mCropRect.s.h);
            MY_LOGD("P2B CRZ setting: GroupId(%d) CropRect(%dx%d)",
                    crz_crop.mGroupID,
                    crz_crop.mCropRect.s.w,
                    crz_crop.mCropRect.s.h);
        }
        frameParams.mvCropRsInfo.push_back(mdp_crop);
        frameParams.mvCropRsInfo.push_back(crz_crop);
    }
    {
        // set SRZ 3
        setSRZInfo(frameParams, EDipModule_SRZ3, miDmgi, imgiSize);
        // set SRZ 4
        if(miMode == ADVANCE)
        {
            setSRZInfo(frameParams, EDipModule_SRZ4, miDepi, imgiSize);
        }
    }
    //
    {
        data = new enqueData();
        //
        data->mpTuningBuffer = pTuningBuffer;

        if(pWDMABuffer!=nullptr)
        {
            data->mEnquedSmartImgBufMap.add(BOKEH_ER_BUF_WDMAIMG, pWDMABuffer);
        }

        data->request = request;
        data->scenarioId = scenarioId;
    }
    data->cookies = this;
    enqueParams.mvFrameParams.push_back(frameParams);
    enqueParams.mpfnCallback = onP2Callback;
    enqueParams.mpfnEnQFailCallback = onP2FailCallback;
    enqueParams.mpCookie = data;
    CAM_TRACE_BEGIN("VSDOF_P2BNode::NormalStream::enque");
    data->start = std::chrono::system_clock::now();
    ret = mpINormalStream->enque(enqueParams);
    CAM_TRACE_END();
    if(!ret)
    {
        MY_LOGE("mpINormalStream enque failed");
        goto lbExit;
    }
    {
        android::Mutex::Autolock lock(mP2DoneLock);
        this->incExtThreadDependency();
    }
    // dump debug information
    if(miPipeLogEnable)
    {
        MY_LOGD("scenarioId(%d) gsensorOri(%d)",
                scenarioId,
                gsensorOri);
    }
lbExit:
    if(!ret)
    {
        if(pTuningBuffer!= nullptr)
            pTuningBuffer = nullptr;
        if(data != nullptr)
            delete data;
        this->decExtThreadDependency();
    }
    return ret;
}
/*******************************************************************************
 *
 ********************************************************************************/
MBOOL
VSDOF_P2BNode::
getP2AOutputSize(
    MINT32 scenarioId,
    Pass2SizeInfo& outSize
)
{
    if(scenarioId == ENUM_STEREO_SCENARIO::eSTEREO_SCENARIO_PREVIEW)
    {
        StereoSizeProvider::getInstance()->getPass2SizeInfo(
                                            PASS2A,
                                            eSTEREO_SCENARIO_PREVIEW,
                                            outSize);
    }
    else if(scenarioId == ENUM_STEREO_SCENARIO::eSTEREO_SCENARIO_CAPTURE)
    {
        StereoSizeProvider::getInstance()->getPass2SizeInfo(
                                            PASS2A,
                                            eSTEREO_SCENARIO_CAPTURE,
                                            outSize);
    }
    else if(scenarioId == ENUM_STEREO_SCENARIO::eSTEREO_SCENARIO_RECORD)
    {
        StereoSizeProvider::getInstance()->getPass2SizeInfo(
                                            PASS2A,
                                            eSTEREO_SCENARIO_RECORD,
                                            outSize);
    }
    else
    {
        MY_LOGE("Unsupported scenario(%d)", scenarioId);
        return false;
    }
    return true;
}
/*******************************************************************************
 *
 ********************************************************************************/
MBOOL
VSDOF_P2BNode::
setSRZInfo(
    FrameParams& frameParam,
    MINT32 modulTag,
    MSize inputSize,
    MSize outputSize
)
{
    CAM_TRACE_NAME("VSDOF_P2BNode::setSRZInfo");
    // set SRZ 3
    _SRZ_SIZE_INFO_ *srzInfo = (_SRZ_SIZE_INFO_ *)malloc(sizeof(_SRZ_SIZE_INFO_));
    srzInfo->in_w = inputSize.w;
    srzInfo->in_h = inputSize.h;
    srzInfo->crop_w = inputSize.w;
    srzInfo->crop_h = inputSize.h;
    srzInfo->crop_x = 0;
    srzInfo->crop_y = 0;
    srzInfo->crop_floatX = 0;
    srzInfo->crop_floatY = 0;
    srzInfo->out_w = outputSize.w;
    srzInfo->out_h = outputSize.h;
    //
    ModuleInfo moduleInfo;
    moduleInfo.moduleTag = modulTag;
    moduleInfo.frameGroup = 0;
    moduleInfo.moduleStruct = srzInfo;
    //
    if(miPipeLogEnable)
    {
        MY_LOGD("srz moduleTag (%d) in(%dx%d) out(%dx%d)",
            modulTag,
            srzInfo->in_w,
            srzInfo->in_h,
            srzInfo->out_w,
            srzInfo->out_h);
    }
    frameParam.mvModuleData.push_back(moduleInfo);
    return MTRUE;
}
/*******************************************************************************
 *
 ********************************************************************************/
MBOOL
VSDOF_P2BNode::
unpackInputBufferFromRequest(
    EffectRequestPtr request,
    FrameParams& frameParam
)
{
    // unpack input buffer
    MY_LOGD_IF(miPipeLogEnable,
            "Input buffer size(%d)",
            request->vInputFrameInfo.size());
    // set input FrameParams and use frameNo to decide which port to set.
    sp<EffectFrameInfo> pFrameInfo = nullptr;
    for(MINT32 i = 0;i<request->vInputFrameInfo.size();++i)
    {
        pFrameInfo = request->vInputFrameInfo.valueAt(i);
        if(isMetadataBuffer(pFrameInfo->getFrameNo()))
        {
            MY_LOGD_IF(miPipeLogEnable,"Check input is metadata:%d",pFrameInfo->getFrameNo());
            continue;
        }
        setP2BInputPort(frameParam, pFrameInfo, pFrameInfo->getFrameNo());
    }
    // apply 3A basic ISPtuning buffer
    SmartTuningBuffer tuningBuf = mpTuningBufferPool->request();
    memset(tuningBuf->mpVA, 0, mpTuningBufferPool->getBufSize());
    IMetadata* pMeta_InApp  = nullptr;
    IMetadata* pMeta_InHal  = nullptr;
    if(!getInputMetadataFromRequest(request, BOKEH_ER_IN_APP_META, pMeta_InApp))
    {
        MY_LOGE("Get App metadata fail.");
        return MFALSE;
    }
    if(!getInputMetadataFromRequest(request, BOKEH_ER_IN_HAL_META_MAIN1, pMeta_InHal))
    {
        MY_LOGE("Get HAL metadata fail.");
        return MFALSE;
    }

    //StereoTuningProvider overwrite some 3A tuning buffer for tuning
    #ifndef GTEST
    if(!StereoTuningProvider::getBokehTuningInfo(tuningBuf->mpVA))
    {
        MY_LOGE("set tuning parameter fail.");
        return MFALSE;
    }
    #else
        MY_LOGW("GTEST do not getBokehTuningInfo");
    #endif

    TuningParam rTuningParam;
    MINT32 reqNo = request->getRequestNo();
    trySetMetadata<MUINT8>(pMeta_InHal, MTK_3A_ISP_PROFILE, NSIspTuning::EIspProfile_Bokeh);
    ISPTuningConfig ispConfig = {pMeta_InApp, pMeta_InHal, mp3AHal_Main1, MFALSE, reqNo};
    rTuningParam = applyISPTuning(PIPE_LOG_TAG, tuningBuf, ispConfig, MTRUE);

    frameParam.mTuningData = tuningBuf->mpVA;
    frameParam.mStreamTag = ENormalStreamTag_Bokeh;
    return MTRUE;
}
/*******************************************************************************
 *
 ********************************************************************************/
MBOOL
VSDOF_P2BNode::
unpackOutputBufferFromRequest(
    EffectRequestPtr request,
    FrameParams& frameParam,
    MINT32 rot
)
{
    MY_LOGD_IF(miPipeLogEnable,
            "Output buffer size(%d)",
            request->vOutputFrameInfo.size());
    //
    sp<EffectFrameInfo> pFrameInfo = nullptr;
    for(MINT32 i = 0;i<request->vOutputFrameInfo.size();++i)
    {
        pFrameInfo = request->vOutputFrameInfo.valueAt(i);
        if(isMetadataBuffer(pFrameInfo->getFrameNo()))
        {
            MY_LOGD_IF(miPipeLogEnable,"Check output is metadata:%d",pFrameInfo->getFrameNo());
            continue;
        }
        setP2BOutputPort(frameParam, pFrameInfo, pFrameInfo->getFrameNo(), rot);
    }
    return MTRUE;
}
/*******************************************************************************
 *
 ********************************************************************************/
MBOOL
VSDOF_P2BNode::
setP2BInputPort(
    FrameParams &frameParam,
    FrameInfoPtr frameInfo,
    MUINT32 bufferType
)
{
    if(frameInfo.get() == nullptr)
    {
        MY_LOGD("setP2BInputPort failed: null buffer bufferType(%d)", bufferType);
        return MFALSE;
    }
    //
    sp<IImageBuffer> frame = nullptr;
    frameInfo->getFrameBuffer(frame);
    return setP2BInputPort(frameParam, frame, bufferType);
}
/*******************************************************************************
 *
 ********************************************************************************/
MBOOL
VSDOF_P2BNode::
setP2BInputPort(
    FrameParams &frameParam,
    SmartImageBuffer imgBuffer,
    MUINT32 bufferType
)
{
    if(imgBuffer == nullptr)
    {
        MY_LOGD("setP2BInputPort failed: null buffer bufferType(%d)", bufferType);
        return MFALSE;
    }
    return setP2BInputPort(frameParam, imgBuffer->mImageBuffer, bufferType);
}
/*******************************************************************************
 *
 ********************************************************************************/
MBOOL
VSDOF_P2BNode::
setP2BInputPort(
    FrameParams &frameParam,
    sp<IImageBuffer> imgBuffer,
    MUINT32 bufferType
)
{
    if(imgBuffer == nullptr)
    {
        MY_LOGD("setP2BInputPort failed: null buffer bufferType(%d)", bufferType);
        return MFALSE;
    }
    // parse portID first.
    NSCam::NSIoPipe::PortID portId = mapToPortID(bufferType);
    if(portId.index == EPortIndex_UNKNOW)
    {
        // ignore unknow port.
        MY_LOGD("unmap buffer type(%d)", bufferType);
        return MTRUE;
    }

    Input src;
    //
    // set IMGI
    src.mPortID = portId;
    src.mBuffer = imgBuffer.get();
    //
    MY_LOGD_IF(miPipeLogEnable,
                "BufType(%d) PortID: %d Buf: 0x%x fmt(%x)",
                bufferType,
                src.mPortID.index,
                src.mBuffer,
                imgBuffer->getImgFormat());
    frameParam.mvIn.push_back(src);
    return MTRUE;
}
/*******************************************************************************
 *
 ********************************************************************************/
MBOOL
VSDOF_P2BNode::
setP2BOutputPort(
    FrameParams &frameParam,
    FrameInfoPtr frameInfo,
    MUINT32 bufferType,
    MINT32 transform
)
{
    if(frameInfo.get() == nullptr)
    {
        MY_LOGD("setP2BOutputPort failed: null buffer bufferType(%d)", bufferType);
        return MFALSE;
    }
    //
    sp<IImageBuffer> frame = nullptr;
    frameInfo->getFrameBuffer(frame);
    return setP2BOutputPort(frameParam, frame, bufferType, transform);
}
/*******************************************************************************
 *
 ********************************************************************************/
MBOOL
VSDOF_P2BNode::
setP2BOutputPort(
    FrameParams &frameParam,
    SmartImageBuffer imgBuffer,
    MUINT32 bufferType,
    MINT32 transform
)
{
    if(imgBuffer == nullptr)
    {
        MY_LOGD("setP2BOutputePort failed: null buffer bufferType(%d)", bufferType);
        return MFALSE;
    }
    return setP2BOutputPort(frameParam, imgBuffer->mImageBuffer, bufferType, transform);
}
/*******************************************************************************
 *
 ********************************************************************************/
MBOOL
VSDOF_P2BNode::
setP2BOutputPort(
    FrameParams &frameParam,
    sp<IImageBuffer> imgBuffer,
    MUINT32 bufferType,
    MINT32 transform
)
{
    if(imgBuffer == nullptr)
    {
        MY_LOGD("setP2BOutputPort failed: null buffer bufferType(%d)", bufferType);
        return MFALSE;
    }
    // parse portID first.
    NSCam::NSIoPipe::PortID portId = mapToPortID(bufferType);
    if(portId.index == EPortIndex_UNKNOW)
    {
        // ignore unknow port.
        MY_LOGD("unmap buffer type(%d)", bufferType);
        return MTRUE;
    }
    Output src;
    //
    // set IMGI
    src.mPortID = portId;
    src.mBuffer = imgBuffer.get();
    if(portId == PORT_WROTO)
    {
        src.mTransform = transform;
    }
    else
        src.mTransform = 0;
    //
    MY_LOGD(
                "Output BufType(%d) PortID: %d Buf: 0x%x fmt(%x)",
                bufferType,
                src.mPortID.index,
                src.mBuffer,
                imgBuffer->getImgFormat());
    frameParam.mvOut.push_back(src);
    return MTRUE;
}
/*******************************************************************************
 *
 ********************************************************************************/
MVOID
VSDOF_P2BNode::
onP2Callback(QParams& rParams)
{
    enqueData* pEnqueData = (enqueData*) (rParams.mpCookie);
	if(nullptr!=pEnqueData->cookies)
	{
		VSDOF_P2BNode* pP2B = (VSDOF_P2BNode*)pEnqueData->cookies;
		pP2B->handleP2Done(rParams);
	}
}
/*******************************************************************************
 *
 ********************************************************************************/
MVOID
VSDOF_P2BNode::
handleP2Done(QParams& rParams)
{
    CAM_TRACE_NAME("VSDOF_P2BNode::handleP2Done");
    MY_LOGD_IF(miPipeLogEnable, "+");
    //
    MUINT32 scenarioId = eSTEREO_SCENARIO_UNKNOWN;
    enqueData *data = (enqueData*)rParams.mpCookie;
    EffectRequestPtr request = nullptr;
    if(nullptr == data)
    {
        MY_LOGE("enqueData is null");
        return;
    }
    {
        data->end = std::chrono::system_clock::now();
        std::chrono::duration<double> elapsed_seconds = data->end-data->start;
        VSDOF_PRFLOG("VSDOF_Profile: reqID=%d p2b processing time(%lf ms)", data->request->getRequestNo(), elapsed_seconds.count()*1000);
    }
    {
        scenarioId = data->scenarioId;
        request = data->request;
    }
    MY_LOGE_IF(rParams.mvFrameParams.size()==0, "rParams.mvFrameParams is zero. should not happened");
    for(MUINT32 i=0;i<rParams.mvFrameParams.size();++i)
    {
    MY_LOGD_IF(miPipeLogEnable,
            "RequestId(%d) in(%d) out(%d)",
            request->getRequestNo(),
                rParams.mvFrameParams[i].mvIn.size(),
                rParams.mvFrameParams[i].mvOut.size());
    }
    //
    if(mShowDepthMap > 0)
    {
        IImageBuffer* input = nullptr;
        IImageBuffer* output = nullptr;
        for(const auto& frameParam : rParams.mvFrameParams)
        {
            for(const auto& inParam : frameParam.mvIn)
            {
                NSCam::NSIoPipe::PortID portId = inParam.mPortID;
                MUINT32 bufDataType = mapToBufferID( portId,  scenarioId);
                if(bufDataType == BOKEH_ER_BUF_DMBG)
                {
                    input = inParam.mBuffer;
                    break;
                }
            }
        }
        for(const auto& frameParam : rParams.mvFrameParams)
        {
            for(const auto& outParam : frameParam.mvOut)
            {
                NSCam::NSIoPipe::PortID portId = outParam.mPortID;
                MUINT32 bufDataType = mapToBufferID( portId,  scenarioId);
                if(bufDataType == BOKEH_ER_BUF_DISPLAY)
                {
                    output = outParam.mBuffer;
                    break;
                }
            }

            // mark cache to invalid, ensure get buffer from memory.
            if(input != nullptr)
            {
                input->syncCache(eCACHECTRL_INVALID);
                shiftDepthMapValue(input, 2);
                input->syncCache(eCACHECTRL_FLUSH);
                outputDepthMap(request->getRequestNo(), input, output);
            }
            else
            {
                MY_LOGE("Passing null pointer input to syncCache");
            }
        }
    }
    //draw magic number
    if(mpDebugDrawID->needDraw())
    {
        IMetadata* pMeta_inHal = nullptr;
        if(!getInputMetadataFromRequest(request, BOKEH_ER_IN_HAL_META_MAIN1, pMeta_inHal))
        {
            MY_LOGE("Get HAL metadata fail.");
        } else {
            for(const auto& frameParam : rParams.mvFrameParams)
            {
                for(const auto& outParam : frameParam.mvOut)
                {
                    NSCam::NSIoPipe::PortID portId = outParam.mPortID;
                    IImageBuffer* buf = outParam.mBuffer;
                    if(portId == PORT_WDMAO)
                    {
                        drawMagicID(pMeta_inHal, buf);
                    }
                }
            }
        }
    }
    // dump out port
    if(mbDumpImageBuffer && checkReqIndexToDump(request))
    {
        MY_LOGD("Dump image(%d)+", request->getRequestNo());

        IMetadata* pMeta_InHal = nullptr;
        if(!getInputMetadataFromRequest(request, BOKEH_ER_IN_HAL_META_MAIN1, pMeta_InHal))
        {
            MY_LOGE("Get HAL metadata fail.");
        } else {
            const size_t PATH_SIZE = 1024;
            char writepath[PATH_SIZE];
            extract(&mDumpHint, pMeta_InHal);

            for(const auto& frameParam : rParams.mvFrameParams)
            {
                for(const auto& outParam : frameParam.mvOut)
                {
                    NSCam::NSIoPipe::PortID portId = outParam.mPortID;
                    IImageBuffer* buf = outParam.mBuffer;
                    extract(&mDumpHint, buf);

                    if(portId == PORT_WDMAO)
                    {
                        genFileName_YUV(writepath, PATH_SIZE, &mDumpHint, TuningUtils::YUV_PORT_WDMAO, "P2B_DISPLAY");
                    }
                    else if(portId == PORT_WROTO)
                    {
                        genFileName_YUV(writepath, PATH_SIZE, &mDumpHint, TuningUtils::YUV_PORT_WROTO, "P2B_VSDOF");
                    }
                    buf->saveToFile(writepath);
                }

                for(const auto& inParam : frameParam.mvIn)
                {
                    NSCam::NSIoPipe::PortID portId = inParam.mPortID;
                    MUINT32 bufferid = mapToBufferID(portId, eSTEREO_SCENARIO_PREVIEW);
                    if(portId == PORT_DMGI)
                    {
                        IImageBuffer* buf = inParam.mBuffer;
                        extract(&mDumpHint, buf);
                        genFileName_YUV(writepath, PATH_SIZE, &mDumpHint, TuningUtils::YUV_PORT_UNDEFINED, "P2B_DMBG");
                        buf->saveToFile(writepath);
                    }
                }
            }
        }
        MY_LOGD("Dump image-");
    }
    //
    if(scenarioId == ENUM_STEREO_SCENARIO::eSTEREO_SCENARIO_CAPTURE)
    {
        handleData(P2B_MDP_REQUEST, request);
        MY_LOGD_IF(miPipeLogEnable, "Capture output");
    }
    else
    {
        handleData(P2B_OUT, request);
        MY_LOGD_IF(miPipeLogEnable, "Preview/Record");
    }

    // release smartBuffer
    // sb contain tuning buffer, so clear mEnquedSmartImgBufMap can return tuning
    // buffer to sbPool.
    data->mEnquedSmartImgBufMap.clear();
    data->mpTuningBuffer = nullptr;
    delete data;
    // release malloc buffer
    for(const auto& frameParam : rParams.mvFrameParams)
    {
        for(const auto& moduleInfo : frameParam.mvModuleData)
        {
            if(moduleInfo.moduleTag == EDipModule_SRZ3 ||
               moduleInfo.moduleTag == EDipModule_SRZ4)
            {
                if(moduleInfo.moduleStruct != nullptr)
                    free(moduleInfo.moduleStruct);
            }
        }
    }
    this->decExtThreadDependency();
    MY_LOGD_IF(miPipeLogEnable, "-");
}
/*******************************************************************************
 *
 ********************************************************************************/
MVOID
VSDOF_P2BNode::
onP2FailCallback(QParams& rParams)
{
    CAM_TRACE_NAME("VSDOF_P2BNode::onP2FailCallback");
    MY_LOGE("Process p2 fail.");
    enqueData* pEnqueData = (enqueData*) (rParams.mpCookie);
    if(nullptr!=pEnqueData->cookies)
    {
        VSDOF_P2BNode* pP2B = (VSDOF_P2BNode*)pEnqueData->cookies;
        pP2B->handleP2Fail(rParams);
    }
}
/*******************************************************************************
 *
 ********************************************************************************/
MVOID
VSDOF_P2BNode::
handleP2Fail(QParams& rParams)
{
    //
    MUINT32 scenarioId = eSTEREO_SCENARIO_UNKNOWN;
    enqueData *data = (enqueData*)rParams.mpCookie;
    EffectRequestPtr request = nullptr;
    if(nullptr == data)
    {
        MY_LOGE("enqueData is null");
        return;
    }
    {
        scenarioId = data->scenarioId;
        request = data->request;
    }
    for(const auto& frameParam : rParams.mvFrameParams)
    {
        for(const auto& moduleInfo : frameParam.mvModuleData)
        {
            if(moduleInfo.moduleTag == EDipModule_SRZ3 ||
               moduleInfo.moduleTag == EDipModule_SRZ4)
            {
                if(moduleInfo.moduleStruct != nullptr)
                    free(moduleInfo.moduleStruct);
            }
        }
    }
    // release smartBuffer
    // sb contain tuning buffer, so clear mEnquedSmartImgBufMap can return tuning
    // buffer to sbPool.
    data->mEnquedSmartImgBufMap.clear();
    data->mpTuningBuffer = nullptr;
    this->decExtThreadDependency();
    delete data;
}
/*******************************************************************************
 *
 ********************************************************************************/
MBOOL
VSDOF_P2BNode::
isMetadataBuffer(const MUINT32 bufDataType)
{
    switch(bufDataType)
    {
        case BOKEH_ER_IN_APP_META:
        case BOKEH_ER_IN_HAL_META_MAIN1:
        case BOKEH_ER_IN_HAL_META_MAIN2:
        case BOKEH_ER_OUT_APP_META:
        case BOKEH_ER_OUT_HAL_META:
        case BOKEH_ER_IN_P1_RETURN_META:
            return MTRUE;
            break;
        default:
            break;
    }
    return MFALSE;
}
/*******************************************************************************
 *
 ********************************************************************************/
NSCam::NSIoPipe::PortID
VSDOF_P2BNode::
mapToPortID(const MUINT32 bufDataType)
{
    switch(bufDataType)
    {
        case BOKEH_ER_BUF_MAIN1:
            return PORT_IMGI;
            break;
        case BOKEH_ER_BUF_DMG:
            return PORT_DEPI;
            break;
        case BOKEH_ER_BUF_DMBG:
            return PORT_DMGI;
            break;
        case BOKEH_ER_BUF_VSDOF_IMG:
        case BOKEH_ER_BUF_RECORD:
        case BOKEH_ER_BUF_PRVCB:
            return PORT_WROTO;
            break;
        case BOKEH_ER_BUF_DISPLAY:
        case BOKEH_ER_BUF_WDMAIMG:
            return PORT_WDMAO;
            break;
        default:
            //MY_LOGE("mapToPortID: not exist bufDataType=%d", bufDataType);
            break;
    }
    NSCam::NSIoPipe::PortID unsupportPort;
    unsupportPort.index = EPortIndex_UNKNOW;
    return unsupportPort;
}
/*******************************************************************************
 *
 ********************************************************************************/
MUINT32
VSDOF_P2BNode::
mapToBufferID(
    NSCam::NSIoPipe::PortID const portId,
    const MUINT32 scenarioID
)
{
    if(portId == PORT_IMGI)
    {
        return BOKEH_ER_BUF_MAIN1;
    }
    if(portId == PORT_DMGI)
    {
        return BOKEH_ER_BUF_DMBG;
    }
    if(portId == PORT_DEPI)
    {
        return BOKEH_ER_BUF_DMG;
    }
    if(portId == PORT_WROTO)
    {
        if(scenarioID == eSTEREO_SCENARIO_CAPTURE)
            return BOKEH_ER_BUF_VSDOF_IMG;
        else if(scenarioID == eSTEREO_SCENARIO_RECORD)
            return BOKEH_ER_BUF_RECORD;
        else if(scenarioID == eSTEREO_SCENARIO_PREVIEW)
            return BOKEH_ER_BUF_PRVCB;
    }
    if(portId == PORT_WDMAO)
    {
        if(scenarioID == eSTEREO_SCENARIO_CAPTURE)
            return BOKEH_ER_BUF_WDMAIMG;
        if(scenarioID == eSTEREO_SCENARIO_RECORD || scenarioID == eSTEREO_SCENARIO_PREVIEW)
            return BOKEH_ER_BUF_DISPLAY;
    }
    return BOKEH_ER_BUF_START;
}
/*******************************************************************************
 *
 ********************************************************************************/
MBOOL
VSDOF_P2BNode::
getFrameInfoFromRequest(
    EffectRequestPtr request,
    FrameInfoPtr& frame,
    MINT32 bufType,
    MBOOL isInputPort
)
{
    ssize_t keyIndex = -1;
    if(isInputPort)
    {
        keyIndex = request->vInputFrameInfo.indexOfKey(bufType);
        if(keyIndex>=0)
        {
            frame = request->vInputFrameInfo.valueAt(keyIndex);
            return MTRUE;
        }
        else
        {
            return MFALSE;
        }
    }
    else
    {
        keyIndex = request->vOutputFrameInfo.indexOfKey(bufType);
        if(keyIndex>=0)
        {
            frame = request->vOutputFrameInfo.valueAt(keyIndex);
            return MTRUE;
        }
        else
        {
            return MFALSE;
        }
    }
    return MFALSE;
}
/*******************************************************************************
 *
 ********************************************************************************/
MBOOL
VSDOF_P2BNode::
removeInPortId(
    FrameParams &frameParam,
    NSCam::NSIoPipe::PortID portId
)
{
    int index = -1;
    MBOOL ret = MFALSE;
    for(auto lPortId: frameParam.mvIn)
    {
        index++;
        if(lPortId.mPortID.index == portId.index)
        {
            ret = MTRUE;
            break;
        }
    }
    if(index != -1)
        frameParam.mvIn.erase(frameParam.mvIn.begin() + index);
    return ret;
}
/*******************************************************************************
 *
 ********************************************************************************/
MBOOL
VSDOF_P2BNode::
removeOutPortId(
    FrameParams &frameParam,
    NSCam::NSIoPipe::PortID portId
)
{
    int index = -1;
    MBOOL ret = MFALSE;
    for(auto lPortId: frameParam.mvOut)
    {
        index++;
        if(lPortId.mPortID.index == portId.index)
        {
            ret = MTRUE;
            break;
        }
    }
    if(index != -1)
        frameParam.mvOut.erase(frameParam.mvOut.begin() + index);
    return ret;
}
//************************************************************************
//
//************************************************************************
sp<IImageBuffer>
VSDOF_P2BNode::
createEmptyImageBuffer(
    const IImageBufferAllocator::ImgParam imgParam,
    const char* name,
    MINT usage
)
{
    IImageBufferAllocator *allocator = IImageBufferAllocator::getInstance();
    sp<IImageBuffer> pImgBuf = allocator->alloc(name, imgParam);
    pImgBuf->lockBuf(name, usage);
    return pImgBuf;
}
//************************************************************************
//
//************************************************************************
MVOID
VSDOF_P2BNode::
shiftDepthMapValue(
    IImageBuffer* depthMap,
    MUINT8 shiftValue
)
{
    // offset value
    MUINT8* data = (MUINT8*)depthMap->getBufVA(0);
    MSize const size = depthMap->getImgSize();
    for(int i=0;i<size.w*size.h;++i)
    {
        *data = *data << shiftValue;
        *data = (MUINT8)std::max(0, std::min((int)*data, 255));
        data++;
    }
}
//************************************************************************
//
//************************************************************************
MVOID
VSDOF_P2BNode::
outputDepthMap(MINT32 id,
    IImageBuffer* depthMap,
    IImageBuffer* displayResult
)
{
    if(displayResult == nullptr || displayResult->getPlaneCount()!=3)
        return;
    if(mpDpStream == nullptr)
        return;
    //
    SmartImageBuffer mdpBuffer = mpDepthMapBufPool->request();
    //
    VSDOF::util::sMDP_Config config;
    config.pDpStream = mpDpStream;
    config.pSrcBuffer = depthMap;
    config.pDstBuffer = mdpBuffer->mImageBuffer.get();
    config.rotAngle = 0;
    if(!excuteMDP(config))
    {
        MY_LOGE("excuteMDP fail.");
        return;
    }
    //
    MSize outImgSize = displayResult->getImgSize();
    MSize inImgSize = mdpBuffer->mImageBuffer->getImgSize();
    char* outAddr0 = (char*)displayResult->getBufVA(0);
    char* outAddr1 = (char*)displayResult->getBufVA(1);
    char* outAddr2 = (char*)displayResult->getBufVA(2);
    char* inAddr = (char*)mdpBuffer->mImageBuffer->getBufVA(0);
    MINT32 halfInWidth = inImgSize.w >> 1;
    MINT32 halfInHeight = inImgSize.h >> 1;
    MINT32 halfOutWidth = outImgSize.w  >> 1;
    //
    for(int i=0;i<inImgSize.h;++i)
    {
        memcpy(outAddr0, inAddr, inImgSize.w);
        outAddr0 += outImgSize.w;
        inAddr += inImgSize.w;
    }
    //
    for(int i=0;i<halfInHeight;++i)
    {
        memset(outAddr1, 128, halfInWidth);
        memset(outAddr2, 128, halfInWidth);
        outAddr1 += halfOutWidth;
        outAddr2 += halfOutWidth;
    }
    mdpBuffer = nullptr;
    return;
}
//************************************************************************
//
//************************************************************************
MVOID
VSDOF_P2BNode::
outputDepthMapAndAddDebugInfo(MINT32 id,
    IImageBuffer* depthMap,
    IImageBuffer* displayResult
)
{
    if(displayResult == nullptr || displayResult->getPlaneCount()!=3)
        return;
    if(mpDpStream == nullptr)
        return;
    // get center four point
    {
        MUINT8 values[4];
        MINT32 width = depthMap->getImgSize().w;
        MINT32 height = depthMap->getImgSize().h;
        MINT32 halfWidth = width>>1;
        MINT32 halfHight = height>>1;
        values[0] = ((MUINT8*)depthMap->getBufVA(0))[width*(halfHight-2) + (halfWidth-1)];
        values[1] = ((MUINT8*)depthMap->getBufVA(0))[width*(halfHight-2) + (halfWidth)];
        values[2] = ((MUINT8*)depthMap->getBufVA(0))[width*(halfHight-1) + (halfWidth-1)];
        values[3] = ((MUINT8*)depthMap->getBufVA(0))[width*(halfHight-1) + (halfWidth)];
        MY_LOGD("lu(%d) ru(%d) ld(%d) rd(%d)", values[0], values[1], values[2], values[3]);
        ((MUINT8*)depthMap->getBufVA(0))[width*(halfHight-2) + (halfWidth-1)] = 0;
        ((MUINT8*)depthMap->getBufVA(0))[width*(halfHight-2) + (halfWidth)] = 0;
        ((MUINT8*)depthMap->getBufVA(0))[width*(halfHight-1) + (halfWidth-1)] = 0;
        ((MUINT8*)depthMap->getBufVA(0))[width*(halfHight-1) + (halfWidth)] = 0;
    }
    //
    SmartImageBuffer mdpBuffer = mpDepthMapBufPool->request();
    //
    VSDOF::util::sMDP_Config config;
    config.pDpStream = mpDpStream;
    config.pSrcBuffer = depthMap;
    config.pDstBuffer = mdpBuffer->mImageBuffer.get();
    config.rotAngle = 0;
    if(!excuteMDP(config))
    {
        MY_LOGE("excuteMDP fail.");
        return;
    }
    //
    MSize outImgSize = displayResult->getImgSize();
    MSize inImgSize = mdpBuffer->mImageBuffer->getImgSize();
    char* outAddr0 = (char*)displayResult->getBufVA(0);
    char* outAddr1 = (char*)displayResult->getBufVA(1);
    char* outAddr2 = (char*)displayResult->getBufVA(2);
    char* inAddr = (char*)mdpBuffer->mImageBuffer->getBufVA(0);
    MINT32 halfInWidth = inImgSize.w >> 1;
    MINT32 halfInHeight = inImgSize.h >> 1;
    MINT32 halfOutWidth = outImgSize.w  >> 1;
    //
    for(int i=0;i<inImgSize.h;++i)
    {
        memcpy(outAddr0, inAddr, inImgSize.w);
        outAddr0 += outImgSize.w;
        inAddr += inImgSize.w;
    }
    //
    for(int i=0;i<halfInHeight;++i)
    {
        memset(outAddr1, 128, halfInWidth);
        memset(outAddr2, 128, halfInWidth);
        outAddr1 += halfOutWidth;
        outAddr2 += halfOutWidth;
    }
    mdpBuffer = nullptr;
    return;
}
//************************************************************************
//
//************************************************************************
MVOID
VSDOF_P2BNode::
drawMagicID(IMetadata* pHalMeta, IImageBuffer* pBuf)
{

    MINT32 magicNumber = 0;
    IMetadata::IEntry entry;
    entry = pHalMeta->entryFor(MTK_P1NODE_PROCESSOR_MAGICNUM);
    if (!entry.isEmpty()){
        magicNumber = entry.itemAt(0, Type2Type<MINT32>());
    }else{
        MY_LOGW("no magic found!");
    }

    mpDebugDrawID->draw(magicNumber,
                        (char*)pBuf->getBufVA(0),
                        pBuf->getImgSize().w,
                        pBuf->getImgSize().h,
                        pBuf->getBufStridesInBytes(0),
                        pBuf->getBufSizeInBytes(0));
}

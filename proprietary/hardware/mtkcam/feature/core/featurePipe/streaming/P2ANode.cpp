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
#include <mtkcam/drv/iopipe/CamIO/IHalCamIO.h>
#include <mtkcam/feature/eis/eis_hal.h>
#include <mtkcam/feature/DualCam/FOVHal.h>
#include <mtkcam/feature/fsc/fsc_util.h>
//#include "FMHal.h"

#define PIPE_CLASS_TAG "P2ANode"
#define PIPE_TRACE TRACE_P2A_NODE
#include <featurePipe/core/include/PipeLog.h>
#include "P2CamContext.h"

using NSCam::NSIoPipe::NSPostProc::ENormalStreamTag_Normal;
using NSCam::NSIoPipe::NSPostProc::ENormalStreamTag_Vss;
using NSCam::NSIoPipe::NSPostProc::ENormalStreamTag_FM;
using NSCam::NSIoPipe::NSPostProc::ENormalStreamTag_FE;
using NSCam::NSIoPipe::Input;
using NSCam::NSIoPipe::Output;
using NSImageio::NSIspio::EPortIndex_LCEI;
using NSImageio::NSIspio::EPortIndex_IMG3O;
using NSImageio::NSIspio::EPortIndex_IMG2O;
using NSImageio::NSIspio::EPortIndex_WDMAO;
using NSImageio::NSIspio::EPortIndex_WROTO;
using NSImageio::NSIspio::EPortIndex_VIPI;
using NSImageio::NSIspio::EPortIndex_IMGI;
using NSImageio::NSIspio::EPortIndex_DEPI;//left
using NSImageio::NSIspio::EPortIndex_DMGI;//right
using NSImageio::NSIspio::EPortIndex_FEO;
using NSImageio::NSIspio::EPortIndex_MFBO;
using NSCam::NSIoPipe::ModuleInfo;

using namespace NSCam::NSIoPipe;

namespace NSCam {
namespace NSCamFeature {
namespace NSFeaturePipe {

// turn on flag to keep crop mode size in mFullImgSize
// must review usage of mFullImgSize before turn on this flag
//#define USE_REQ_REC_FULLIMGSIZE
static MVOID getFullImgSize(MBOOL isCropMode, const RequestPtr &request, MSize &imgSize)
{
    if (isCropMode)
    {
        imgSize = request->mMDPCrop.s;
        MY_LOGD("crop size is %dx%d", imgSize.w, imgSize.h);
    }
    else
    {
        imgSize = request->mFullImgSize;
    }
}

/**
 * @brief get image crop information of img3o
 *
 * @param[in]isCRZMode  : need CRZ mode to determine output
 * @param[in]request    : need imgo.2imgi.enabled to determine output
 * @param[out]imgRect   : crop rectangle of img3o
 * @param[out]isSrcCrop : crop rectangle is in imgi domain
 */
static MVOID getFullImgRect(MBOOL isCRZMode, const RequestPtr &request, MRect &imgRect, MBOOL *isSrcCrop = NULL)
{
    if (isSrcCrop != NULL)
        *isSrcCrop = MFALSE;

    if (isCRZMode)
    {
    #ifdef CRZ_NR3D_REPLACED_BY_TILE_SRC_CROP
        imgRect = MRect(request->mMDPCrop.p_integral, request->mMDPCrop.s);
        if (isSrcCrop != NULL)
            *isSrcCrop = MTRUE;
    #else // ori: CRZ_NR3D
        imgRect = MRect(request->mMDPCrop.s.w, request->mMDPCrop.s.h);
    #endif

    }
    else
    {
        MBOOL imgo2ImgiEnabled = MFALSE;
        imgo2ImgiEnabled = request->getVar<MBOOL>(VAR_IMGO_2IMGI_ENABLE, MFALSE);
        if (imgo2ImgiEnabled)
        {
            imgRect = MRect(request->mFullImgSize.w, request->mFullImgSize.h);
            imgRect = request->getVar<MRect>(VAR_IMGO_2IMGI_P1CROP, imgRect);

            if (isSrcCrop != NULL)
                *isSrcCrop = MTRUE;
        }
        else
        {
            imgRect = MRect(request->mFullImgSize.w, request->mFullImgSize.h);
        }
    }

    if (isSrcCrop != NULL && (*isSrcCrop))
    {
        // HW limitation
        imgRect.p.x &= (~1);
    }

    if (isSrcCrop != NULL && (*isSrcCrop))
    {
        // HW limitation
        imgRect.p.x &= (~1);
    }
}

static MUINT32 calImgOffset(sp<IImageBuffer> pIMGBuffer, const MRect &tmpRect)
{
    MUINT32 u4PixelToBytes = 0;

    MINT imgFormat = pIMGBuffer->getImgFormat();

    if (imgFormat == eImgFmt_YV12)
    {
        u4PixelToBytes = 1;
    }
    else if (imgFormat == eImgFmt_YUY2)
    {
        u4PixelToBytes = 2;
    }
    else
    {
        MY_LOGW("unsupported image format %d", imgFormat);
    }

    return tmpRect.p.y * pIMGBuffer->getBufStridesInBytes(0) + tmpRect.p.x * u4PixelToBytes; //in byte
}

P2ANode::P2ANode(const char *name)
    : StreamingFeatureNode(name)
    , mp3A(NULL)
    , mNormalStream(NULL)
    , mFullImgPoolAllocateNeed(0)
    , mFEFMTuning{NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL}
    , mCropMode(CROP_MODE_NONE)
    , mEisMode(0)
{
    TRACE_FUNC_ENTER();
    this->addWaitQueue(&mRequests);

    m3dnrLogLevel = getPropertyValue("vendor.camera.3dnr.log.level", 0);

    TRACE_FUNC_EXIT();
}

P2ANode::~P2ANode()
{
    TRACE_FUNC_ENTER();
    TRACE_FUNC_EXIT();
}

MVOID P2ANode::setNormalStream(NSCam::NSIoPipe::NSPostProc::INormalStream *stream)
{
    TRACE_FUNC_ENTER();
    mNormalStream = stream;
    TRACE_FUNC_EXIT();
}
MVOID P2ANode::setDsImgPool(const android::sp<ImageBufferPool> &pool)
{
    TRACE_FUNC_ENTER();
    mDsImgPool = pool;
    TRACE_FUNC_EXIT();
}

MVOID P2ANode::setFullImgPool(const android::sp<IBufferPool> &pool, MUINT32 allocate)
{
    TRACE_FUNC_ENTER();
    mFullImgPool = pool;
    mFullImgPoolAllocateNeed = allocate;
    TRACE_FUNC_EXIT();
}

MVOID P2ANode::setEisFullImgPool(const android::sp<IBufferPool> &pool)
{
    TRACE_FUNC_ENTER();
    mEisFullImgPool = pool;
    TRACE_FUNC_EXIT();
}

MVOID P2ANode::setVendorFullImgPool(const android::sp<IBufferPool> &pool)
{
    TRACE_FUNC_ENTER();
    mVendorFullImgPool = pool;
    TRACE_FUNC_EXIT();
}

MVOID P2ANode::setVFOVFullImgPool(const android::sp<IBufferPool> &pool)
{
    TRACE_FUNC_ENTER();
    mVendorFovFullImgPool = pool;
    TRACE_FUNC_EXIT();
}

MBOOL P2ANode::onInit()
{
    TRACE_FUNC_ENTER();
    StreamingFeatureNode::onInit();
    if( mPipeUsage.supportFEFM() )
    {
        mEisMode = EIS_MODE_OFF;
        FMHal::getConfig(mPipeUsage.getStreamingSize(),mFM_FE_cfg);
        MY_LOGD("EIS2.5: FE0(%dx%d) FE1(%dx%d) FE2(%dx%d) feo_h(%dx%d) feo_m(%dx%d) feo_l(%dx%d) fmo_h(%dx%d) fmo_m(%dx%d) fmo_l(%dx%d)",
               mFM_FE_cfg[0].mImageSize.w,mFM_FE_cfg[0].mImageSize.h,
               mFM_FE_cfg[1].mImageSize.w,mFM_FE_cfg[1].mImageSize.h,
               mFM_FE_cfg[2].mImageSize.w,mFM_FE_cfg[2].mImageSize.h,
               mFM_FE_cfg[0].mFESize.w,mFM_FE_cfg[0].mFESize.h,
               mFM_FE_cfg[1].mFESize.w,mFM_FE_cfg[1].mFESize.h,
               mFM_FE_cfg[2].mFESize.w,mFM_FE_cfg[2].mFESize.h,
               mFM_FE_cfg[0].mFMSize.w,mFM_FE_cfg[0].mFMSize.h,
               mFM_FE_cfg[1].mFMSize.w,mFM_FE_cfg[1].mFMSize.h,
               mFM_FE_cfg[2].mFMSize.w,mFM_FE_cfg[2].mFMSize.h);
        mFE1ImgPool = ImageBufferPool::create("fpipe.fe1Img", mFM_FE_cfg[1].mImageSize, eImgFmt_YV12, ImageBufferPool::USAGE_HW);
        mFE2ImgPool = ImageBufferPool::create("fpipe.fe2Img", mFM_FE_cfg[2].mImageSize, eImgFmt_YV12, ImageBufferPool::USAGE_HW);
        mFE3ImgPool = ImageBufferPool::create("fpipe.fe3Img", mFM_FE_cfg[2].mImageSize, eImgFmt_YV12, ImageBufferPool::USAGE_HW);
        mFEOutputPool = FatImageBufferPool::create("fpipe.feo", mFM_FE_cfg[0].mFESize, eImgFmt_STA_BYTE, FatImageBufferPool::USAGE_HW);
        mFEOutputPool_m = FatImageBufferPool::create("fpipe.feo_m", mFM_FE_cfg[1].mFESize, eImgFmt_STA_BYTE, FatImageBufferPool::USAGE_HW);
        mFMOutputPool = FatImageBufferPool::create("fpipe.fmo", mFM_FE_cfg[0].mFMSize, eImgFmt_STA_BYTE, FatImageBufferPool::USAGE_HW);
        mFMOutputPool_m = FatImageBufferPool::create("fpipe.fmo_m", mFM_FE_cfg[1].mFMSize, eImgFmt_STA_BYTE, FatImageBufferPool::USAGE_HW);
        mFMRegisterPool = FatImageBufferPool::create("fpipe.fm_readregister", MSize(1,4), eImgFmt_STA_BYTE, FatImageBufferPool::USAGE_HW);

        for( unsigned i = 0; i < sizeof(mFEInfo)/sizeof(mFEInfo[0]); ++i )
        {
            memset(&mFEInfo[i], 0, sizeof(mFEInfo[0]));
        }
        for( unsigned i = 0; i < sizeof(mFMInfo)/sizeof(mFMInfo[0]); ++i )
        {
            memset(&mFMInfo[i], 0, sizeof(mFMInfo[0]));
        }
        for( unsigned i = 0; i < 9; ++i )
        {
            unsigned int tuningsize = NSIoPipe::NSPostProc::INormalStream::getRegTableSize();
            mFEFMTuning[i] = (char*)malloc(tuningsize);
            if( mFEFMTuning[i] )
            {
                memset(mFEFMTuning[i], 0, tuningsize);
            }
            else
            {
                MY_LOGE("Allocate FEFM tuning buffer failed. Id = %d, Size = %d", i, tuningsize);
            }
        }
    }
    if(mPipeUsage.supportFOV())
    {
#if SUPPORT_FOV
        MY_LOGD("Support Dual, create FOV buffer pools.");
        // query size from hal
        FOVHal::SizeConfig config = FOVHal::getSizeConfig(FOVHal::RATIO_4_3);
        // create pool with larger 4:3 buffer pool
        mFovFEOImgPool = ImageBufferPool::create("fpipe.fovfeoImg", config.mFEOSize, eImgFmt_STA_BYTE, ImageBufferPool::USAGE_HW);
        mFovFMOImgPool = ImageBufferPool::create("fpipe.fovfmoImg", config.mFMOSize, eImgFmt_STA_BYTE, ImageBufferPool::USAGE_HW);
        #if (MTKCAM_HAVE_DUAL_ZOOM_SUPPORT == 1)
        mFovTuningBufferPool = TuningBufferPool::create("fpipe.fovtuningBuf", NSIoPipe::NSPostProc::INormalStream::getRegTableSize());
        #endif
#endif
    }

    if( mPipeUsage.support3DNRRSC() )
    {
        MY_LOGD("P2A add RSC waitQueue for 3DNR");
        this->addWaitQueue(&mRSCDatas);
    }

    TRACE_FUNC_EXIT();
    return MTRUE;
}

MBOOL P2ANode::onUninit()
{
    TRACE_FUNC_ENTER();
    ImageBufferPool::destroy(mFE1ImgPool);
    ImageBufferPool::destroy(mFE2ImgPool);
    ImageBufferPool::destroy(mFE3ImgPool);
    FatImageBufferPool::destroy(mFEOutputPool);
    FatImageBufferPool::destroy(mFEOutputPool_m);
    FatImageBufferPool::destroy(mFMOutputPool);
    FatImageBufferPool::destroy(mFMOutputPool_m);
    FatImageBufferPool::destroy(mFMRegisterPool);
    ImageBufferPool::destroy(mFovFEOImgPool);
    ImageBufferPool::destroy(mFovFMOImgPool);
    for( unsigned i = 0; i < 9; ++i )
    {
        if( mFEFMTuning[i] )
        {
            ::free(mFEFMTuning[i]);
            mFEFMTuning[i] = NULL;
        }
    }
    #if (MTKCAM_HAVE_DUAL_ZOOM_SUPPORT == 1)
    TuningBufferPool::destroy(mFovTuningBufferPool);
    #endif
    TRACE_FUNC_EXIT();
    return MTRUE;
}

MBOOL P2ANode::onThreadStart()
{
    TRACE_FUNC_ENTER();
    MUINT32 numBasicBuffer = mPipeUsage.getNumP2ABuffer();

    if( mPipeUsage.supportVFB() && mDsImgPool != NULL )
    {
        mDsImgPool->allocate(numBasicBuffer);
    }
    if( mFullImgPoolAllocateNeed && mFullImgPool != NULL )
    {
        Timer timer;
        timer.start();
        mFullImgPool->allocate(mFullImgPoolAllocateNeed);
        timer.stop();
        MY_LOGD("mFullImg %s %d buf in %d ms", STR_ALLOCATE, mFullImgPoolAllocateNeed, timer.getElapsed());
    }

    if( mPipeUsage.supportFEFM() &&
        mFE1ImgPool != NULL && mFE2ImgPool != NULL && mFE3ImgPool != NULL &&
        mFEOutputPool != NULL && mFEOutputPool_m != NULL &&
        mFMOutputPool != NULL && mFMOutputPool_m != NULL && mFMRegisterPool != NULL)
    {
        Timer timer;
        timer.start();
        mFE1ImgPool->allocate(1*3);
        mFE2ImgPool->allocate(1*3);
        mFE3ImgPool->allocate(1*3);
        mFEOutputPool->allocate(3*4);
        mFEOutputPool_m->allocate(3*4);
        mFMOutputPool->allocate(6*3);
        mFMOutputPool_m->allocate(6*3);
        mFMRegisterPool->allocate(6*4);
        timer.stop();
        MY_LOGD("FE FM %s in %d ms", STR_ALLOCATE, timer.getElapsed());
    }

    if( mPipeUsage.supportFOV())
    {
        const int FOV_IMG_BUFSIZE = 3;
        Timer timer;
        timer.start();
        // master + slave = 2 sets
        mFovFEOImgPool->allocate(FOV_IMG_BUFSIZE * 2);
        mFovFMOImgPool->allocate(FOV_IMG_BUFSIZE * 2);
        #if (MTKCAM_HAVE_DUAL_ZOOM_SUPPORT == 1)
        mFovTuningBufferPool->allocate(FOV_IMG_BUFSIZE * 2);
        #endif
        MY_LOGD("FOV buffers %s in %d ms", STR_ALLOCATE, timer.getElapsed());
    }
    init3A();
    initVHDR();
    initFEFM();
    initP2();

    TRACE_FUNC_EXIT();
    return MTRUE;
}

MBOOL P2ANode::onThreadStop()
{
    TRACE_FUNC_ENTER();
    this->waitNormalStreamBaseDone();
    uninitVHDR();
    uninitFEFM();
    uninitP2();
    uninit3A();

    TRACE_FUNC_EXIT();
    return MTRUE;
}

MBOOL P2ANode::onData(DataID id, const RequestPtr &data)
{
    TRACE_FUNC_ENTER();
    TRACE_FUNC("Frame %d: %s arrived", data->mRequestNo, ID2Name(id));
    MBOOL ret = MFALSE;

    switch( id )
    {
    case ID_ROOT_TO_P2A:
        mRequests.enque(data);
        ret = MTRUE;
        break;
    default:
        ret = MFALSE;
        break;
    }

    TRACE_FUNC_EXIT();
    return ret;
}

MBOOL P2ANode::onData(DataID id, const RSCData &data)
{
    TRACE_FUNC_ENTER();
    TRACE_FUNC("Frame %d: %s  rsc_data arrived", data.mRequest->mRequestNo, ID2Name(id));
    MBOOL ret = MFALSE;

    if( mPipeUsage.support3DNRRSC() )
    {
        if( id == ID_RSC_TO_P2A )
        {
            mRSCDatas.enque(data);
            ret = MTRUE;
        }
    }

    TRACE_FUNC_EXIT();
    return ret;
}

MBOOL P2ANode::onThreadLoop()
{
    TRACE_FUNC("Waitloop");
    RequestPtr request;
    RSCData rscData;

    CAM_TRACE_CALL();

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

    if( mPipeUsage.support3DNRRSC() )
    {
        if( !mRSCDatas.deque(rscData) )
        {
            MY_LOGE("RSCData deque out of sync");
            return MFALSE;
        }
        if( request != rscData.mRequest )
        {
            MY_LOGE("P2A_RSCData out of sync request(%d) rsc(%d)", request->mRequestNo, rscData.mRequest->mRequestNo);
            return MFALSE;
        }
    }

    TRACE_FUNC_ENTER();

    request->mTimer.startP2A();
    {
        MUINT32 in = 0, out = 0;
        if( request->mvFrameParams.size() )
        {
            in = request->mvFrameParams[0].mvIn.size();
            out = request->mvFrameParams[0].mvOut.size();
        }
        MY_LOGD("sensor(%d/%d) Frame %d in P2A, feature=0x%04x(%s), in/out=(%d/%d) fovFEFM=%d", request->getMasterID(), mSensorIndex, request->mRequestNo, request->mFeatureMask, request->getFeatureMaskName(), in, out, request->needFOVFEFM());

        if ( request->needPrintIO() )
        {
            printIO(request, request->mQParams);
        }
    }
    processP2A(request, rscData);
    request->mTimer.stopP2A();

    TRACE_FUNC_EXIT();
    return MTRUE;
}

MBOOL P2ANode::processP2A(const RequestPtr &request, const RSCData &rscData)
{
    CAM_TRACE_CALL();

    TRACE_FUNC_ENTER();
    P2AEnqueData data;
    QParams param;
    MRect postCropSize;
    MCrpRsInfo P2AOutCropInfo;

    data.mRequest = request;

    calcSizeInfo(request);

    prepareQParams(param, request);

    prepare3A(param, request);

    if( request->need3DNR() )
    {
        if (!prepare3DNR(param, request, rscData))
        {
            // set mPrevFullImg to NULL to disable VIPI port
            getP2CamContext(request->getMasterID())->setPrevFullImg(NULL);
        }
    }

    if( mPipeUsage.supportBypassP2A() )
    {
        prepareFullImgFromInput(param, request, data);
        handleResultData(request, data);
    }
    else
    {
        prepareIO(param, request, data);
        prepareCropInfo(param, request, data);

        if( request->needPrintIO() )
        {
            printIO(request, param);
        }

        if( mPipeUsage.supportEISNode() && !mPipeUsage.supportFEFM() )
        {
            handleData(ID_P2A_TO_EIS_P2DONE, FMData(data.mFMResult, request));
        }

        enqueFeatureStream(param, data);
    }

    TRACE_FUNC_EXIT();
    return MTRUE;
}

MBOOL P2ANode::calcSizeInfo(const RequestPtr &request)
{
    TRACE_FUNC_ENTER();

#ifdef USE_REQ_REC_FULLIMGSIZE
    request->mMDPCrop = request->getP2Crop();
    if (request->isP2ACRZMode())
    {
        // 3DNR + EIS1.2 in 4K2K record mode use CRZ to reduce throughput
        request->mFullImgSize = request->mMDPCrop.s;
        request->mFullImgCrop = request->mMDPCrop;
        MY_LOGD("crop size is %dx%d", request->mFullImgSize.w, request->mFullImgSize.h);
    }
    else
    {
        request->mFullImgSize = request->getInputSize();
        request->mFullImgCrop = MCropRect(MPoint(0, 0), request->mFullImgSize);
    }
#else
    request->mFullImgSize = request->getInputSize();
    request->mFullImgCrop = MCropRect(MPoint(0, 0), request->mFullImgSize);
    request->mMDPCrop = request->getP2Crop();
#endif
    TRACE_FUNC_EXIT();
    return MTRUE;
}

MVOID P2ANode::onNormalStreamBaseCB(const QParams &params, const P2AEnqueData &data)
{
    // This function is not thread safe,
    // avoid accessing P2ANode class members
    TRACE_FUNC_ENTER();

    for( size_t i = 0; i < params.mvFrameParams[0].mvExtraParam.size(); i++ )
    {
        MUINT cmdIdx = params.mvFrameParams[0].mvExtraParam[i].CmdIdx;

        if( cmdIdx == EPIPE_IMG3O_CRSPINFO_CMD )
        {
            CrspInfo* extraParam = static_cast<CrspInfo*>(params.mvFrameParams[0].mvExtraParam[i].moduleStruct);
            if( extraParam )
            {
                delete extraParam;
            }
        }
    }

    RequestPtr request = data.mRequest;
    if( request == NULL )
    {
        MY_LOGE("Missing request");
    }
    else
    {
        request->mTimer.stopEnqueP2A();
        MY_LOGD("sensor(%d) Frame %d enque done in %d ms, result = %d", mSensorIndex, request->mRequestNo, request->mTimer.getElapsedEnqueP2A(), params.mDequeSuccess);

        if( !params.mDequeSuccess )
        {
            MY_LOGW("Frame %d enque result failed", request->mRequestNo);
        }

        request->mP2A_QParams = params;
        request->updateResult(params.mDequeSuccess);
        handleResultData(request, data);
        request->mTimer.stopP2A();
    }

    this->decExtThreadDependency();
    TRACE_FUNC_EXIT();
}

MVOID P2ANode::handleResultData(const RequestPtr &request, const P2AEnqueData &data)
{
    // This function is not thread safe,
    // because it is called by onQParamsCB,
    // avoid accessing P2ANode class members
    TRACE_FUNC_ENTER();
    if( mPipeUsage.supportDual())
    {
        if (mPipeUsage.supportVendorFOV())
        {
            TRACE_FUNC("to vendor fov");
            handleData(ID_P2A_TO_VFOV_FULLIMG,
                DualBasicIOImgData(DualBasicIOImg(BasicIOImg(data.mMasterFullImg, data.mEISFullImg), BasicIOImg(data.mSlaveFullImg)), request));

        }
        else
        {
            TRACE_FUNC("to fov");
            handleData(ID_P2A_TO_FOV_FEFM, FOVP2AData(data.mFovP2AResult, request));
            handleData(ID_P2A_TO_FOV_FULLIMG, ImgBufferData(data.mFullImg.mBuffer, request));
            handleData(ID_P2A_TO_FOV_WARP, BasicIOImgData(BasicIOImg(data.mFullImg, data.mEISFullImg), request));
        }
    }
    else if( mPipeUsage.supportVendor() )
    {
        BasicImg vendorFull = data.mVendorFullImg.mBuffer == NULL ? data.mFullImg : data.mVendorFullImg;
        handleData(ID_P2A_TO_VENDOR_FULLIMG, BasicIOImgData(BasicIOImg(vendorFull, data.mEISFullImg), request));

        if( mPipeUsage.supportN3D() )
        {
            handleData(ID_P2A_TO_N3DP2, request);
        }
    }
    else if( mPipeUsage.supportWarpNode() )
    {
        handleData(ID_P2A_TO_WARP_FULLIMG, BasicImgData(data.mFullImg, request));
    }
    else if( mPipeUsage.supportN3D() )
    {
        handleData(ID_P2A_TO_N3DP2, request);
    }
    else
    {
        handleData(ID_P2A_TO_HELPER, CBMsgData(FeaturePipeParam::MSG_FRAME_DONE, request));
    }

    if( request->needP2AEarlyDisplay() && !mPipeUsage.supportDual() )
    {
        handleData(ID_P2A_TO_HELPER, CBMsgData(FeaturePipeParam::MSG_DISPLAY_DONE, request));
    }

    if( request->needVFB() )
    {
        handleData(ID_P2A_TO_P2B_FULLIMG, ImgBufferData(data.mFullImg.mBuffer, request));
        handleData(ID_P2A_TO_FD_DSIMG, ImgBufferData(data.mDsImg, request));
        handleData(ID_P2A_TO_VFB_DSIMG, ImgBufferData(data.mDsImg, request));
    }

    if( mPipeUsage.supportEISNode() && mPipeUsage.supportFEFM() )
    {
        handleData(ID_P2A_TO_EIS_P2DONE, FMData(data.mFMResult, request));
    }

    MBOOL debugDump = request->getVar<MBOOL>(VAR_DEBUG_DUMP, MFALSE);
    if( debugDump )
    {
        TuningUtils::FILE_DUMP_NAMING_HINT hint;
        if( request->tryGetVar<TuningUtils::FILE_DUMP_NAMING_HINT>(VAR_DEBUG_DUMP_HINT, hint) )
        {
            if( data.mFullImg.mBuffer != NULL )
            {
                data.mFullImg.mBuffer->getImageBuffer()->syncCache(eCACHECTRL_INVALID);
                dumpNddData(&hint, data.mFullImg.mBuffer->getImageBufferPtr(), EPortIndex_IMG3O);
            }
        }
    }

    if( request->needDump() )
    {
        if( data.mFullImg.mBuffer != NULL )
        {
            data.mFullImg.mBuffer->getImageBuffer()->syncCache(eCACHECTRL_INVALID);
            dumpData(data.mRequest, data.mFullImg.mBuffer->getImageBufferPtr(), "full");
        }
        if( data.mDsImg != NULL )
        {
            data.mDsImg->getImageBuffer()->syncCache(eCACHECTRL_INVALID);
            dumpData(data.mRequest, data.mDsImg->getImageBufferPtr(), "ds");
        }
        if( data.mFMResult.FM_B.Register_Medium != NULL )
        {
            dumpData(data.mRequest, data.mFMResult.FM_B.Register_Medium->getImageBufferPtr(), "fm_reg_m");
        }
        if( data.mFovP2AResult.mFEO_Master != NULL )
        {
            dumpData(data.mRequest, data.mFovP2AResult.mFEO_Master->getImageBufferPtr(), "mFEO_Master");
        }
        if( data.mFovP2AResult.mFEO_Slave != NULL )
        {
            dumpData(data.mRequest, data.mFovP2AResult.mFEO_Slave->getImageBufferPtr(), "mFEO_Slave");
        }
        if( data.mFovP2AResult.mFMO_MtoS != NULL )
        {
            dumpData(data.mRequest, data.mFovP2AResult.mFMO_MtoS->getImageBufferPtr(), "mFMO_MtoS");
        }
        if( data.mFovP2AResult.mFMO_StoM != NULL )
        {
            dumpData(data.mRequest, data.mFovP2AResult.mFMO_StoM->getImageBufferPtr(), "mFMO_StoM");
        }
    }
    TRACE_FUNC_EXIT();
}

MBOOL P2ANode::initP2()
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

MVOID P2ANode::uninitP2()
{
    TRACE_FUNC_ENTER();

    mNormalStream = NULL;
    mPrevFE.clear();

    TRACE_FUNC_EXIT();
}

MBOOL P2ANode::prepareQParams(QParams &params, const RequestPtr &request)
{
    TRACE_FUNC_ENTER();
    params = request->mQParams;
    prepareStreamTag(params, request);
    prepareSenorIdx(params, request);
    TRACE_FUNC_EXIT();
    return MFALSE;
}

MBOOL P2ANode::prepareSenorIdx(QParams &params, const RequestPtr &request)
{
    TRACE_FUNC_ENTER();
    (void)(params);
    (void)(request);
#if SUPPORT_FOV
    if( params.mvFrameParams.size() )
    {
        params.mvFrameParams.editItemAt(0).mSensorIdx = request->getMasterID();
    }
#endif
    TRACE_FUNC_EXIT();
    return MTRUE;
}

MCrpRsInfo P2ANode::calViewAngleCrop(QParams &params, const RequestPtr &request, MRect postCropSize)
{
    TRACE_FUNC_ENTER();
    (void)(params);

    MUINT32 PostProcOutWidth = 0, PostProcOutHeight = 0;
    MCrpRsInfo cropInfo;
    MUINT32 eisCrop2Idx = 0,i;

    if( request->mvFrameParams.size() )
    {
        for( i = 0; i < request->mvFrameParams[0].mvCropRsInfo.size(); ++i )
        {
            if( request->mvFrameParams[0].mvCropRsInfo[i].mGroupID == 2 )
            {
                eisCrop2Idx = i;
            }
        }
        cropInfo = request->mvFrameParams[0].mvCropRsInfo[eisCrop2Idx];
    }

    PostProcOutWidth  = postCropSize.s.w;
    PostProcOutHeight = postCropSize.s.h;

    cropInfo.mGroupID = 1;
    cropInfo.mCropRect.p_integral.x = postCropSize.p.x;
    cropInfo.mCropRect.p_integral.y = postCropSize.p.y;
    cropInfo.mCropRect.s.w = PostProcOutWidth;
    cropInfo.mCropRect.s.h = PostProcOutHeight;
    cropInfo.mResizeDst.w  = PostProcOutWidth;
    cropInfo.mResizeDst.h  = PostProcOutHeight;

    TRACE_FUNC_EXIT();
    return cropInfo;
}

MBOOL P2ANode::needDigitalZoomCrop(const RequestPtr &request)
{
    return !request->needEIS();
}

MBOOL P2ANode::prepareCropInfo(QParams &params, const RequestPtr &request, P2AEnqueData &data)
{
    TRACE_FUNC_ENTER();

    if( !request->isLastNodeP2A() )
    {
        if( params.mvFrameParams.size() )
        {
            params.mvFrameParams.editItemAt(0).mvCropRsInfo.clear();
            prepareFDCrop(params, request, data);
        }
    }
    if( request->needFullImg() )
    {
        if (mPipeUsage.supportVendorFOV())
        {
            MCrpRsInfo crop;
            if (request->mvFrameParams[0].mvCropRsInfo.size()) {
                crop = request->mvFrameParams[0].mvCropRsInfo[0];
                TRACE_FUNC("cropRect= %dx%d resizeDst=%dx%d", crop.mCropRect.s.w, crop.mCropRect.s.h, crop.mResizeDst.w, crop.mResizeDst.h);
            }
            crop.mGroupID = WDMAO_CROP_GROUP;
            params.mvFrameParams.editItemAt(0).mvCropRsInfo.push_back(crop);
        }
        if (request->isP2ACRZMode())
        {
        #ifdef CRZ_NR3D_REPLACED_BY_TILE_SRC_CROP
            // do nothing
        #else // ori: CRZ_NR3D
            prepareCRZCrop(params, request, data);
        #endif
        }
    }

    if( request->needFEFM() )
    {
        prepareCropInfo_FE(params,request,data);
    }
    else if( request->needP2AEarlyDisplay() )
    {
        prepareEarlyDisplayCrop(params, request, data);
    }

    if( request->needP2AEarlyEISFullImg() ) // should same as fullimg
    {
        prepareEarlyEISFullImgCrop(params, request, data);
    }

    if( request->needVendorFullImg() )
    {
        prepareVendorFullImgCrop(params, request, data);
    }

    if( request->needDsImg() )
    {
        MCrpRsInfo crop;
        crop.mGroupID = WDMAO_CROP_GROUP;
        crop.mCropRect = MCropRect(MPoint(0, 0), request->mFullImgSize);
        crop.mResizeDst = data.mDsImg->getImageBuffer()->getImgSize();
        if( params.mvFrameParams.size() )
        {
            params.mvFrameParams.editItemAt(0).mvCropRsInfo.push_back(crop);
        }
    }

    if(request->needFOVFEFM())
    {
#if SUPPORT_FOV
        prepareCropInfo_FOV(params, request, data);
#endif
    }
    TRACE_FUNC_EXIT();
    return MTRUE;
}

MVOID P2ANode::prepareCRZCrop(QParams &params, const RequestPtr &request, P2AEnqueData &data)
{
    TRACE_FUNC_ENTER();
    (void)(data);

    if( request->mvFrameParams.size() )
    {
        params.mvFrameParams.editItemAt(0).mvCropRsInfo.clear();

        MCrpRsInfo crop;
        MCrpRsInfo maxCrop;

        // get max crop size
        maxCrop.mCropRect = request->getP2Crop();

        // adjust MDPCrop vOutPort origin due to CRZ crop
        for (int i = 0; i < request->mvFrameParams[0].mvCropRsInfo.size(); ++i)
        {
            crop = request->mvFrameParams[0].mvCropRsInfo[i];

            MY_LOGD_IF(m3dnrLogLevel >= 2, "mkdbg: asis_crop[%d]: origin(%d, %d), frac(%d,%d), w/h=(%d, %d)",
                i,
                crop.mCropRect.p_integral.x,
                crop.mCropRect.p_integral.y,
                crop.mCropRect.p_fractional.x,
                crop.mCropRect.p_fractional.y,
                crop.mCropRect.s.w,
                crop.mCropRect.s.h);

            crop.mCropRect.p_integral.x -= maxCrop.mCropRect.p_integral.x;
            crop.mCropRect.p_integral.y -= maxCrop.mCropRect.p_integral.y;
            crop.mCropRect.p_fractional.x = 0;
            crop.mCropRect.p_fractional.y = 0;
            crop.mCropRect.s.w &= ~1;
            crop.mCropRect.s.h &= ~1;

            params.mvFrameParams.editItemAt(0).mvCropRsInfo.push_back(crop);

            MY_LOGD_IF(m3dnrLogLevel >= 2, "mkdbg: tobe_crop[%d]: origin(%d, %d), frac(%d,%d), w/h=(%d, %d)",
                i,
                crop.mCropRect.p_integral.x,
                crop.mCropRect.p_integral.y,
                crop.mCropRect.p_fractional.x,
                crop.mCropRect.p_fractional.y,
                crop.mCropRect.s.w,
                crop.mCropRect.s.h);
        }

        maxCrop.mResizeDst = maxCrop.mCropRect.s;
        maxCrop.mGroupID = 1; //Use for CRZ
        params.mvFrameParams.editItemAt(0).mvCropRsInfo.push_back(maxCrop);
    }

    TRACE_FUNC_EXIT();
}

MVOID P2ANode::prepareEarlyDisplayCrop(QParams &params, const RequestPtr &request, P2AEnqueData &data)
{
    TRACE_FUNC_ENTER();
    (void)(data);
    Output output;
    if( request->getDisplayOutput(output) )
    {
        MCrpRsInfo crop;
        MCrpRsInfo eis_displaycrop;
        if( request->getDisplayCrop(eis_displaycrop, RRZO_DOMAIN) )
        {
             crop.mCropRect = eis_displaycrop.mCropRect;
             crop.mResizeDst = eis_displaycrop.mResizeDst;
        } else {
             MY_LOGD("default display  crop");
             crop.mCropRect = MCropRect(MPoint(0, 0), request->mFullImgSize);
             crop.mResizeDst = request->mFullImgSize;
        }
        crop.mGroupID = WROTO_CROP_GROUP;//wrot
        crop.mFrameGroup = 0;
        if( params.mvFrameParams.size() )
        {
            params.mvFrameParams.editItemAt(0).mvCropRsInfo.push_back(crop);
        }
    }
    TRACE_FUNC_EXIT();
}

MVOID P2ANode::prepareEarlyEISFullImgCrop(QParams &params, const RequestPtr &request, P2AEnqueData &data)
{
    TRACE_FUNC_ENTER();
    (void)(data);
    MCrpRsInfo crop;
    crop.mCropRect = MCropRect(MPoint(0, 0), request->mFullImgSize);
    crop.mResizeDst = request->mFullImgSize;
    crop.mGroupID = WDMAO_CROP_GROUP;
    crop.mFrameGroup = 0;
    if( params.mvFrameParams.size() )
    {
        params.mvFrameParams.editItemAt(0).mvCropRsInfo.push_back(crop);
    }
    TRACE_FUNC_EXIT();
}

MVOID P2ANode::prepareVendorFullImgCrop(QParams &params, const RequestPtr &request, P2AEnqueData &data)
{
    TRACE_FUNC_ENTER();
    (void)(data);

    MRect srcCropRect;
    MBOOL isSrcCrop = MFALSE;

    getFullImgRect(request->isP2ACRZMode(), request, srcCropRect, &isSrcCrop);

    MCrpRsInfo crop;

    crop.mResizeDst = mPipeUsage.supportVendorCusSize() ? mPipeUsage.getVendorCusSize() : srcCropRect.s;
    if( request->needEarlyFSCVendorFullImg() )
    {
        MRectF FSCRRZOCrop;
        FSCRRZOCrop = request->getFSCCropRegion();
        crop.mCropRect = getCropRect(FSCRRZOCrop);
        if( FSCUtil::isFSCSubpixelEnabled(mPipeUsage.getFSCMode()) != MTRUE)
        {
            crop.mCropRect.p_fractional.x = 0;
            crop.mCropRect.p_fractional.y = 0;
            crop.mCropRect.w_fractional = 0;
            crop.mCropRect.h_fractional = 0;
        }
        MY_LOGD_IF(FSCUtil::getFSCDebugLevel(mPipeUsage.getFSCMode()), "f(%d) early fsc (%f,%f,%f,%f)->crop(%d,%d,0x%x,0x%x,%d,%d) dst(%d,%d)",
            request->mRequestNo, FSCRRZOCrop.p.x, FSCRRZOCrop.p.y, FSCRRZOCrop.s.w, FSCRRZOCrop.s.h,
            crop.mCropRect.p_integral.x, crop.mCropRect.p_integral.y,
            crop.mCropRect.p_fractional.x, crop.mCropRect.p_fractional.y, crop.mCropRect.s.w, crop.mCropRect.s.h,
            crop.mResizeDst.w, crop.mResizeDst.h);
    }
    else
    {
        crop.mCropRect = MCropRect(srcCropRect.p, srcCropRect.s);
    }
    crop.mGroupID = WDMAO_CROP_GROUP;
    crop.mFrameGroup = 0;
    if( params.mvFrameParams.size() )
    {
        params.mvFrameParams.editItemAt(0).mvCropRsInfo.push_back(crop);
    }
    TRACE_FUNC_EXIT();
}

MBOOL P2ANode::prepareStreamTag(QParams &params, const RequestPtr &request)
{
    TRACE_FUNC_ENTER();
    (void)(request);
    if( params.mvFrameParams.size() )
    {
    #ifdef CRZ_NR3D_REPLACED_BY_TILE_SRC_CROP
        // 1. if non-TimeSharing --> use the original assgined stream tag, ex: ENormalStreamTag_Normal
        // 2. if TimeSharing -> use ENormalStreamTag_Vss

        if( mPipeUsage.supportTimeSharing() )
        {
            params.mvFrameParams.editItemAt(0).mStreamTag = ENormalStreamTag_Vss;
        }
    #else // ori
        if( request->need3DNR() && request->isP2ACRZMode() )
        {
            params.mvFrameParams.editItemAt(0).mStreamTag = ENormalStreamTag_FE;
        }
        else if( request->need3DNR() || !request->isLastNodeP2A() )
        {
            params.mvFrameParams.editItemAt(0).mStreamTag = ENormalStreamTag_Normal;
        }
        else if( mPipeUsage.supportTimeSharing() )
        {
            params.mvFrameParams.editItemAt(0).mStreamTag = ENormalStreamTag_Vss;
        }
    #endif // CRZ_NR3D_REPLACED_BY_TILE_SRC_CROP
    }
    TRACE_FUNC_EXIT();
    return MTRUE;
}

MBOOL P2ANode::prepareFullImgFromInput(QParams &params, const RequestPtr &request, P2AEnqueData &data)
{
    TRACE_FUNC_ENTER();
    (void)(params);
    MBOOL ret = MTRUE;
    IImageBuffer *input = NULL;
    if( (input = request->getInputBuffer()) == NULL )
    {
        MY_LOGE("Cannot get input image buffer");
        ret = MFALSE;
    }
    else if( (data.mFullImg.mBuffer= new IIBuffer_IImageBuffer(input)) == NULL )
    {
        MY_LOGE("OOM: failed to allocate IIBuffer");
        ret = MFALSE;
    }
    TRACE_FUNC_EXIT();
    return ret;
}

MBOOL P2ANode::prepareIO(QParams &params, const RequestPtr &request, P2AEnqueData &data)
{
    TRACE_FUNC_ENTER();
    MBOOL ret = MTRUE;

    if( request->mvFrameParams.size() )
    {
        params.mvFrameParams.editItemAt(0).mvIn = request->mvFrameParams[0].mvIn;
        params.mvFrameParams.editItemAt(0).mvOut.clear();

        if( request->isLastNodeP2A() )
        {
            params.mvFrameParams.editItemAt(0).mvOut = request->mvFrameParams[0].mvOut;
        }
        else
        {
            prepareFDImg(params, request, data);
        }
    }

    if( request->needFullImg() )
    {
        prepareFullImg(params, request, data);
    }

    if( request->needEISFullImg() )
    {
        prepareEISFullImg(params, request, data);
    }

    if( request->needVendorFullImg() )
    {
        prepareVendorFullImg(params, request, data);
    }

    if( request->needDsImg() )
    {
        prepareDsImg(params, request, data);
    }
    if( request->need3DNR() &&
        getP2CamContext(request->getMasterID())->getPrevFullImg() != NULL)
    {
        prepareVIPI(params, request, data);
        //handleVipiNr3dOffset(params, request, data);
    }
    if( request->needVHDR() )
    {
        prepareLCEI(params, request, data);
    }
    if( request->needFEFM() )
    {
        prepareFEFM(params, request, data);
    }
    else if( request->needP2AEarlyDisplay() )
    {
        prepareEarlyDisplayImg(params, request, data);
    }
    if(request->needFOVFEFM())
    {
#if SUPPORT_FOV
        prepareFOVFEFM(params, request, data);
#endif
    }
    mPrevFE = data.mFMResult.FE;

    if(request->needVendorFOVFullImg())
    {
        prepareMasterFullImg(params, request, data);
        prepareSlaveFullImg(params, request, data);
    }
    getP2CamContext(request->getMasterID())->setPrevFullImg(
        (request->need3DNR() ? data.mFullImg.mBuffer: NULL));

    TRACE_FUNC_EXIT();
    return ret;
}

MVOID P2ANode::prepareFullImg(QParams &params, const RequestPtr &request, P2AEnqueData &data)
{
    TRACE_FUNC_ENTER();
    TRACE_FUNC("Frame %d FullImgPool=(%d/%d)", request->mRequestNo, mFullImgPool->peakAvailableSize(), mFullImgPool->peakPoolSize());
    data.mFullImg = mFullImgPool->requestIIBuffer();
    sp<IImageBuffer> pIMGBuffer = data.mFullImg.mBuffer->getImageBuffer();

    MRect tmpRect;
    MBOOL isSrcCrop = MFALSE;
    MUINT32 tmpOffst = 0; //in byte

    getFullImgRect(request->isP2ACRZMode(), request, tmpRect, &isSrcCrop);

    if (isSrcCrop && params.mvFrameParams.size())
    {
        tmpOffst = calImgOffset(pIMGBuffer, tmpRect);
        data.mFullImg.mDomainOffset = MSizeF(tmpRect.p.x, tmpRect.p.y);
        // new driver interface to overwrite mOffsetInBytes
        CrspInfo* crspParam = new CrspInfo();
        crspParam->m_CrspInfo.p_integral.x = tmpRect.p.x;
        crspParam->m_CrspInfo.p_integral.y = tmpRect.p.y;
        crspParam->m_CrspInfo.s.w = tmpRect.s.w;
        crspParam->m_CrspInfo.s.h = tmpRect.s.h;
        ExtraParam extraParam;
        extraParam.CmdIdx = EPIPE_IMG3O_CRSPINFO_CMD;
        extraParam.moduleStruct = static_cast<void*>(crspParam);
        params.mvFrameParams.editItemAt(0).mvExtraParam.push_back(extraParam);
    }

    pIMGBuffer->setExtParam(tmpRect.s);

    Output output;
    output.mPortID = PortID(EPortType_Memory, EPortIndex_IMG3O, PORTID_OUT);
    output.mBuffer = data.mFullImg.mBuffer->getImageBufferPtr();
    output.mBuffer->setTimestamp(request->mQParams.mvFrameParams.editItemAt(0).mvIn[0].mBuffer->getTimestamp());
    output.mOffsetInBytes = tmpOffst;
    if( params.mvFrameParams.size() )
    {
        params.mvFrameParams.editItemAt(0).mvOut.push_back(output);
    }
    TRACE_FUNC_EXIT();
}

MVOID P2ANode::prepareDsImg(QParams &params, const RequestPtr &request, P2AEnqueData &data)
{
    TRACE_FUNC_ENTER();
    MSize dsSize;
    dsSize = calcDsImgSize(request->mFullImgSize);
    MY_LOGD("DsImg size from %dx%d to %dx%d", MSize_ARG(request->mFullImgSize), MSize_ARG(dsSize));
    data.mDsImg = mDsImgPool->requestIIBuffer();
    data.mDsImg->getImageBuffer()->setExtParam(dsSize);
    Output output;
    output.mPortID = PortID(EPortType_Memory, EPortIndex_WDMAO, PORTID_OUT);
    output.mBuffer = data.mDsImg->getImageBufferPtr();
    if( params.mvFrameParams.size() )
    {
        params.mvFrameParams.editItemAt(0).mvOut.push_back(output);
    }
    TRACE_FUNC_EXIT();
}

MVOID P2ANode::prepareVIPI(QParams &params, const RequestPtr &request, P2AEnqueData &data)
{
    TRACE_FUNC_ENTER();
    data.mPrevFullImg = getP2CamContext(request->getMasterID())->getPrevFullImg();
    Input input;
    input.mPortID = PortID(EPortType_Memory, EPortIndex_VIPI, PORTID_IN);
    input.mBuffer = data.mPrevFullImg->getImageBufferPtr();

    if( params.mvFrameParams.size() )
    {
        params.mvFrameParams.editItemAt(0).mvIn.push_back(input);
    }

    TRACE_FUNC_EXIT();
}

MVOID P2ANode::prepareLCEI(QParams &params, const RequestPtr &request, P2AEnqueData &data)
{
    TRACE_FUNC_ENTER();
    (void)(data);
    prepareVHDR(params, request);
    TRACE_FUNC_EXIT();
}

MVOID P2ANode::prepareFEFM(QParams &params, const RequestPtr &request, P2AEnqueData &data)
{
    TRACE_FUNC_ENTER();

    data.mFE1Img = mFE1ImgPool->requestIIBuffer();
    data.mFE2Img = mFE2ImgPool->requestIIBuffer();
    data.mFE3Img = mFE3ImgPool->requestIIBuffer();
    data.mFMResult.FE.High = mFEOutputPool->requestIIBuffer();
    data.mFMResult.FE.Medium = mFEOutputPool_m->requestIIBuffer();
    data.mFMResult.FE.Low = mFEOutputPool_m->requestIIBuffer();//mFEOutputPool_l->requestIIBuffer();

    prepareFE(params, request, data);

    if( mPrevFE.isValid() )
    {
        data.mFMResult.PrevFE = mPrevFE;

        //Previous to Current
        data.mFMResult.FM_A.High = mFMOutputPool->requestIIBuffer();
        data.mFMResult.FM_A.Medium = mFMOutputPool_m->requestIIBuffer();
        data.mFMResult.FM_A.Low = mFMOutputPool_m->requestIIBuffer();//mFMOutputPool_l->requestIIBuffer();

        data.mFMResult.FM_A.Register_High = mFMRegisterPool->requestIIBuffer();
        data.mFMResult.FM_A.Register_Medium = mFMRegisterPool->requestIIBuffer();
        data.mFMResult.FM_A.Register_Low = mFMRegisterPool->requestIIBuffer();

        //Current to Previous
        data.mFMResult.FM_B.High = mFMOutputPool->requestIIBuffer();
        data.mFMResult.FM_B.Medium = mFMOutputPool_m->requestIIBuffer();
        data.mFMResult.FM_B.Low = mFMOutputPool_m->requestIIBuffer();//mFMOutputPool_l->requestIIBuffer();

        data.mFMResult.FM_B.Register_High = mFMRegisterPool->requestIIBuffer();
        data.mFMResult.FM_B.Register_Medium = mFMRegisterPool->requestIIBuffer();
        data.mFMResult.FM_B.Register_Low = mFMRegisterPool->requestIIBuffer();

        prepareFM(params, request, data);
    }
    TRACE_FUNC_EXIT();
}

MVOID P2ANode::prepareEarlyDisplayImg(QParams &params, const RequestPtr &request, P2AEnqueData &data)
{
    TRACE_FUNC_ENTER();
    (void)(data);
    Output output;
    if( request->getDisplayOutput(output) )
    {
        output.mPortID = PortID(EPortType_Memory, EPortIndex_WROTO, PORTID_OUT);
        if( params.mvFrameParams.size() )
        {
            params.mvFrameParams.editItemAt(0).mvOut.push_back(output);
        }
    }
    TRACE_FUNC_EXIT();
}

MVOID P2ANode::prepareEISFullImg(QParams &params, const RequestPtr &request, P2AEnqueData &data)
{
    TRACE_FUNC_ENTER();
    if( mEisFullImgPool == NULL)
    {
        data.mEISFullImg = data.mFullImg;
    }
    else
    {
        MSize eisInputSize;
        eisInputSize = request->getEISInputSize();
        data.mEISFullImg.mBuffer = mEisFullImgPool->requestIIBuffer();
        data.mEISFullImg.mBuffer->getImageBuffer()->setExtParam(eisInputSize);

        if( request->needP2AEarlyEISFullImg() ) // should same as fullimg
        {
            Output output;
            output.mPortID = PortID(EPortType_Memory, EPortIndex_WDMAO, PORTID_OUT);
            output.mBuffer = data.mEISFullImg.mBuffer->getImageBufferPtr();
            if( params.mvFrameParams.size() )
            {
                params.mvFrameParams.editItemAt(0).mvOut.push_back(output);
            }
        }
        else
        {
            data.mEISFullImg.mIsReady = MFALSE;
        }
    }
    TRACE_FUNC_EXIT();
}

MVOID P2ANode::prepareVendorFullImg(QParams &params, const RequestPtr &request, P2AEnqueData &data)
{
    TRACE_FUNC_ENTER();
    if ( mVendorFullImgPool == NULL )
    {
        MY_LOGE("vendor full pool is null");
        return;
    }

    MRect srcCropRect;
    MBOOL isSrcCrop = MFALSE;

    getFullImgRect(request->isP2ACRZMode(), request, srcCropRect, &isSrcCrop);

    TRACE_FUNC("FullImg %dx%d Streaming %dx%d srcCropRect (%d,%d)(%dx%d), isSrcCrop %d", request->mFullImgSize.w, request->mFullImgSize.h,
        mPipeUsage.getStreamingSize().w, mPipeUsage.getStreamingSize().h, srcCropRect.p.x, srcCropRect.p.y, srcCropRect.s.w, srcCropRect.s.h, isSrcCrop);

    request->mFullImgSize = srcCropRect.s;
    MSize vendorFullImgSize = mPipeUsage.supportVendorCusSize() ? mPipeUsage.getVendorCusSize() : srcCropRect.s;

    data.mVendorFullImg.mBuffer = mVendorFullImgPool->requestIIBuffer();
    data.mVendorFullImg.mBuffer->getImageBuffer()->setExtParam(vendorFullImgSize);
    if( request->needEarlyFSCVendorFullImg() )
    {
        data.mVendorFullImg.mDomainOffset = request->getFSCMarginPixel();
        MY_LOGD_IF(FSCUtil::getFSCDebugLevel(mPipeUsage.getFSCMode()), "f(%d) set domainoffset(%f, %f), supportVendorCusSize(%d)", request->mRequestNo,
            data.mVendorFullImg.mDomainOffset.w, data.mVendorFullImg.mDomainOffset.h, mPipeUsage.supportVendorCusSize());
    }
    else
    {
        data.mVendorFullImg.mDomainOffset = MSizeF(srcCropRect.p.x, srcCropRect.p.y);
    }

    Output output;
    output.mPortID = PortID(EPortType_Memory, EPortIndex_WDMAO, PORTID_OUT);
    output.mBuffer = data.mVendorFullImg.mBuffer->getImageBufferPtr();
    if( params.mvFrameParams.size() )
    {
        params.mvFrameParams.editItemAt(0).mvOut.push_back(output);
    }

    TRACE_FUNC_EXIT();
}

MVOID P2ANode::prepareFDImg(QParams &params, const RequestPtr &request, P2AEnqueData &data)
{
    TRACE_FUNC_ENTER();
    (void)(data);
    Output output;
    if ( request->getFDOutput(output) )
    {
        params.mvFrameParams.editItemAt(0).mvOut.push_back(output);
    }
    TRACE_FUNC_EXIT();
}

MVOID P2ANode::prepareFDCrop(QParams &params, const RequestPtr &request, P2AEnqueData &data)
{
    TRACE_FUNC_ENTER();
    (void)(data);
    Output output;
    if ( request->getFDOutput(output) )
    {
        MCrpRsInfo crop;
        if( !request->getFDCrop(crop, RRZO_DOMAIN) )
        {
            MY_LOGD("default fd crop");
            crop.mCropRect = MCropRect(MPoint(0, 0), request->mFullImgSize);
            crop.mResizeDst = output.mBuffer->getImgSize();
        }
        params.mvFrameParams.editItemAt(0).mvCropRsInfo.push_back(crop);
    }
    TRACE_FUNC_EXIT();
}

MVOID P2ANode::prepareMasterFullImg(QParams &params, const RequestPtr &request, P2AEnqueData &data)
{
    TRACE_FUNC_ENTER();

    MSize fullImgSize = data.mFullImg.mBuffer->getImageBufferPtr()->getImgSize();

    if (mPipeUsage.supportVendorFOV())
    {
        if (!mVendorFovFullImgPool.get())
        {
            MY_LOGE("vendor fov pool is null");
            return;
        }
        TRACE_FUNC("Frame %d VendorFovFullImgPool=(%d/%d)", request->mRequestNo,
            mVendorFovFullImgPool->peakAvailableSize(), mVendorFovFullImgPool->peakPoolSize());
        data.mMasterFullImg = mVendorFovFullImgPool->requestIIBuffer();
        data.mMasterFullImg->getImageBuffer()->setExtParam(fullImgSize);

        Output output_master;
        output_master.mPortID = PortID(EPortType_Memory, EPortIndex_WDMAO, PORTID_OUT);
        output_master.mBuffer = data.mMasterFullImg->getImageBufferPtr();
        output_master.mBuffer->setTimestamp(request->mQParams.mvFrameParams.editItemAt(0).mvIn[0].mBuffer->getTimestamp());

        if( params.mvFrameParams.size() )
        {
            params.mvFrameParams.editItemAt(0).mvOut.push_back(output_master);
        }
    }
        TRACE_FUNC_EXIT();
}

MVOID P2ANode::prepareSlaveFullImg(QParams &params, const RequestPtr &request, P2AEnqueData &data)
{
        TRACE_FUNC_ENTER();
        TRACE_FUNC("Frame %d mVendorFovFullImgPool=(%d/%d)", request->mRequestNo,
            mVendorFovFullImgPool->peakAvailableSize(), mVendorFovFullImgPool->peakPoolSize());
        data.mSlaveFullImg = NULL;
        //return;
        FeaturePipeParam fparam_slave;
        if(!request->tryGetVar<FeaturePipeParam>(VAR_DUALCAM_FOV_SLAVE_PARAM, fparam_slave))
        {
            MY_LOGD("Missing slave param");
            return;
        }
        fparam_slave = request->getVar<FeaturePipeParam>(VAR_DUALCAM_FOV_SLAVE_PARAM, fparam_slave);
        QParams& qParam_Slave = fparam_slave.mQParams;
        IImageBuffer *buffer = findInBuffer(qParam_Slave);
        if (!buffer)
        {
            MY_LOGE("Slave no IMGI buffer");
            return;
        }

        TRACE_FUNC("slave qparams");
        printIO(request, qParam_Slave);
        NSIoPipe::FrameParams frame;
        frame.mSensorIdx = request->getVar<MINT32>(VAR_DUALCAM_FOV_SLAVE_ID, -1);
        frame.mStreamTag = ENormalStreamTag_Normal;

        //input
        TRACE_FUNC("slave in cnt:%d", qParam_Slave.mvFrameParams.size());
        for(size_t index=0;index<qParam_Slave.mvFrameParams.itemAt(0).mvIn.size();index++)
        {
            frame.mvIn.push_back(qParam_Slave.mvFrameParams.itemAt(0).mvIn.itemAt(index));
        }
        frame.mTuningData = qParam_Slave.mvFrameParams.itemAt(0).mTuningData;

        data.mSlaveFullImg = mVendorFovFullImgPool->requestIIBuffer();
        MSize size = MSize(0, 0);
        size = buffer->getImgSize();
        TRACE_FUNC("slave buffer size: %dx%d", size.w, size.h);
        data.mSlaveFullImg->getImageBuffer()->setExtParam(size);

        // output
        Output output;
        output.mPortID = PortID(EPortType_Memory, EPortIndex_WDMAO , PORTID_OUT);
        MCrpRsInfo crop;

        crop.mCropRect = MCropRect(MPoint(0, 0), size); // use slave full size
        crop.mResizeDst = size;
        crop.mGroupID = WDMAO_CROP_GROUP;
        frame.mvCropRsInfo.push_back(crop);

        output.mBuffer = data.mSlaveFullImg->getImageBufferPtr();
        output.mBuffer->setTimestamp(qParam_Slave.mvFrameParams.editItemAt(0).mvIn[0].mBuffer->getTimestamp());
        frame.mvOut.push_back(output);

        params.mvFrameParams.push_back(frame);
        TRACE_FUNC_EXIT();
}

MVOID P2ANode::enqueFeatureStream(NSCam::NSIoPipe::QParams &params, P2AEnqueData &data)
{
    TRACE_FUNC_ENTER();
    MBOOL ret;
    MY_LOGD("sensor(%d) Frame %d enque start", mSensorIndex, data.mRequest->mRequestNo);
    data.mRequest->mTimer.startEnqueP2A();
    this->incExtThreadDependency();
    this->enqueNormalStreamBase(mNormalStream, params, data);
    TRACE_FUNC_EXIT();
}

MVOID P2ANode::printIO(const RequestPtr &request, const QParams &params)
{
    for( unsigned f = 0, fCount = params.mvFrameParams.size(); f < fCount; ++f )
    {
        for( unsigned i = 0, n = params.mvFrameParams[f].mvIn.size(); i < n; ++i )
        {
            unsigned index = params.mvFrameParams[f].mvIn[i].mPortID.index;
            MSize size = params.mvFrameParams[f].mvIn[i].mBuffer->getImgSize();
            MY_LOGD("sensor(%d) Frame %d(%d/%d) mvIn[%d] idx=%d size=(%d,%d)", mSensorIndex, request->mRequestNo, f, fCount, i, index, size.w, size.h);
        }
        for( unsigned i = 0, n = params.mvFrameParams[f].mvOut.size(); i < n; ++i )
        {
            unsigned index = params.mvFrameParams[f].mvOut[i].mPortID.index;
            MSize size = params.mvFrameParams[f].mvOut[i].mBuffer->getImgSize();
            MBOOL isGraphic = (getGraphicBufferAddr(params.mvFrameParams[f].mvOut[i].mBuffer) != NULL);
            MINT fmt = params.mvFrameParams[f].mvOut[i].mBuffer->getImgFormat();
            MUINT32 cap = params.mvFrameParams[f].mvOut[i].mPortID.capbility;
            MINT32 transform = params.mvFrameParams[f].mvOut[i].mTransform;
            MY_LOGD("sensor(%d) Frame %d(%d/%d) mvOut[%d] idx=%d size=(%d,%d) fmt=%d, cap=%02x, isGraphic=%d transform=%d", mSensorIndex, request->mRequestNo, f, fCount, i, index, size.w, size.h, fmt, cap, isGraphic, transform);
        }
        for( unsigned i = 0, n = params.mvFrameParams[f].mvCropRsInfo.size(); i < n; ++i )
        {
            MCrpRsInfo crop = params.mvFrameParams[f].mvCropRsInfo[i];
            MY_LOGD("sensor(%d) Frame %d(%d/%d) crop[%d] " MCrpRsInfo_STR, mSensorIndex, request->mRequestNo, f, fCount, i, MCrpRsInfo_ARG(crop));
        }

        for( unsigned i = 0, n = params.mvFrameParams[f].mvModuleData.size(); i < n; ++i )
        {
            ModuleInfo info = params.mvFrameParams[f].mvModuleData[i];
            switch(info.moduleTag)
            {
                case EDipModule_SRZ1:
                    MY_LOGD("sensor(%d) Frame %d(%d/%d) moduleinfo[%d] SRZ1 " ModuleInfo_STR, mSensorIndex, request->mRequestNo, f, fCount, i, ModuleInfo_ARG( ((_SRZ_SIZE_INFO_*) info.moduleStruct) ));
                    break;
                case EDipModule_SRZ4:
                    MY_LOGD("sensor(%d) Frame %d(%d/%d) moduleinfo[%d] SRZ4 " ModuleInfo_STR, mSensorIndex, request->mRequestNo, f, fCount, i, ModuleInfo_ARG( ((_SRZ_SIZE_INFO_*) info.moduleStruct) ));
                    break;
                default:
                    break;
            };
        }

        for( unsigned i = 0, n = params.mvFrameParams[f].mvExtraParam.size(); i < n; ++i )
        {
            PQParam* pqParam = NULL;
            ExtraParam ext = params.mvFrameParams[f].mvExtraParam[i];
            switch(ext.CmdIdx)
            {
                case NSIoPipe::EPIPE_FE_INFO_CMD:
                    MY_LOGD("sensor(%d) Frame %d(%d/%d) extra[%d] FE_CMD " ExtraParam_FE_STR, mSensorIndex, request->mRequestNo, f, fCount, i, ExtraParam_FE_ARG(((FEInfo*) ext.moduleStruct)));
                    break;
                case NSIoPipe::EPIPE_FM_INFO_CMD:
                    MY_LOGD("sensor(%d) Frame %d(%d/%d) extra[%d] FM_CMD " ExtraParam_FM_STR, mSensorIndex, request->mRequestNo, f, fCount, i, ExtraParam_FM_ARG(((FMInfo*) ext.moduleStruct)));
                    break;
                case NSIoPipe::EPIPE_MDP_PQPARAM_CMD:
                    pqParam = (PQParam*)ext.moduleStruct;
                    MY_LOGD("sensor(%d) Frame %d(%d/%d) extra[%d] PQ_CMD " ExtraParam_PQ_STR , mSensorIndex, request->mRequestNo, f, fCount, i, ExtraParam_PQ_ARG(pqParam));
                    if( pqParam->WDMAPQParam )
                    {
                        MY_LOGD("sensor(%d) Frame %d(%d/%d) extra[%d] PQ_WDMA_CMD " DpPqParam_STR , mSensorIndex, request->mRequestNo, f, fCount, i, DpPqParam_ARG(((DpPqParam*) pqParam->WDMAPQParam)));
                    }
                    if( pqParam->WROTPQParam )
                    {
                        MY_LOGD("sensor(%d) Frame %d(%d/%d) extra[%d] PQ_WROT_CMD " DpPqParam_STR , mSensorIndex, request->mRequestNo, f, fCount, i, DpPqParam_ARG(((DpPqParam*) pqParam->WROTPQParam)));
                    }
                    break;
                case NSIoPipe::EPIPE_IMG3O_CRSPINFO_CMD:
                    MY_LOGD("sensor(%d) Frame %d(%d/%d) extra[%d] CRSPINFO_CMD " ExtraParam_CRSPINFO_STR, mSensorIndex, request->mRequestNo, f, fCount, i, ExtraParam_CRSPINFO_ARG(((CrspInfo*) ext.moduleStruct)));
                    break;
                default:
                    break;
            };
        }
    }
}

MBOOL P2ANode::init3A()
{
    TRACE_FUNC_ENTER();

    if (mp3A == NULL)
    {
        mp3A = MAKE_Hal3A(mSensorIndex, PIPE_CLASS_TAG);
    }

    MUINT eisMode = mPipeUsage.getEISMode();
    if( EIS_MODE_IS_EIS_22_ENABLED(eisMode) ||
        EIS_MODE_IS_EIS_25_ENABLED(eisMode) ||
        EIS_MODE_IS_EIS_30_ENABLED(eisMode) )
    {
        //Disable OIS
        MY_LOGD("mEisMode: 0x%x => Disable OIS \n", eisMode);
        if (mp3A)
        {
            mp3A ->send3ACtrl(E3ACtrl_SetEnableOIS, 0, 0);
        }
        else
        {
            MY_LOGE("mp3A is NULL\n");
        }
        mEisMode = mPipeUsage.getEISMode();
    }

    TRACE_FUNC_EXIT();
    return MTRUE;
}

MVOID P2ANode::uninit3A()
{
    TRACE_FUNC_ENTER();

    if (mp3A)
    {
        //Enable OIS
        if( EIS_MODE_IS_EIS_22_ENABLED(mEisMode) ||
            EIS_MODE_IS_EIS_25_ENABLED(mEisMode) ||
            EIS_MODE_IS_EIS_30_ENABLED(mEisMode) )
        {
            MY_LOGD("mEisMode: 0x%x => Enable OIS \n", mEisMode);
            mp3A ->send3ACtrl(E3ACtrl_SetEnableOIS, 1, 0);
            mEisMode = EIS_MODE_OFF;
        }

        // turn OFF 'pull up ISO value to gain FPS'
        AE_Pline_Limitation_T params;
        params. bEnable = MFALSE; // disable
        params. bEquivalent= MTRUE;
        params. u4IncreaseISO_x100= 100;
        params. u4IncreaseShutter_x100= 100;
        mp3A ->send3ACtrl(E3ACtrl_SetAEPlineLimitation, (MINTPTR)&params, 0);

        // destroy the instance
        mp3A->destroyInstance(PIPE_CLASS_TAG);
        mp3A = NULL;
    }

    TRACE_FUNC_EXIT();
}

MBOOL P2ANode::prepare3A(NSCam::NSIoPipe::QParams &params, const RequestPtr &request)
{
    TRACE_FUNC_ENTER();
    (void)params;
    (void)request;
    TRACE_FUNC_EXIT();
    return MTRUE;
}

MBOOL P2ANode::prepare3DNR(NSCam::NSIoPipe::QParams &params, const RequestPtr &request, const RSCData &rscData)
{
    TRACE_FUNC_ENTER();

    // === prepare cropInfo ===

    MBOOL result = MTRUE;
    // 1. prepare all info
    // MBOOL prepare3DNR(
    //     NSCam::NSIoPipe::QParams &params,
    //      crop_infos const& cropInfo, // TODO: try
    //      MINT32 iso,  // TODO: try 200, 800,
    //      IImageBuffer const *pIMGIBuffer,
    //      IImageBuffer *pIMG3OBuffer // TODO
    // );
    // [markInfo]: TODO: Crop, ISO are needed from upper layer

    // === cropInfo ===
    MRect postCropSize;
//    postCropSize.s.w = postCropSize.s.h = 0;
//    postCropSize = calcPostCropSize(request);
#ifdef USE_REQ_REC_FULLIMGSIZE
    postCropSize.s = request->mFullImgSize;
#else
    getFullImgRect(request->isP2ACRZMode(), request, postCropSize);
#endif
    MY_LOGD_IF(m3dnrLogLevel >= 2, "aaaa_cropInfo_test: w=%d, h=%d", postCropSize.s.w, postCropSize.s.h);

    // === prepare eis info ===
    eis_region tmpEisRegion;
    tmpEisRegion.gmvX = request->getVar<MUINT32>(VAR_3DNR_EIS_GMVX, 0);
    tmpEisRegion.gmvY = request->getVar<MUINT32>(VAR_3DNR_EIS_GMVY, 0);
    tmpEisRegion.x_int = request->getVar<MUINT32>(VAR_3DNR_EIS_X_INT, 0);
    tmpEisRegion.y_int = request->getVar<MUINT32>(VAR_3DNR_EIS_Y_INT, 0);
    tmpEisRegion.confX = request->getVar<MUINT32>(VAR_3DNR_EIS_CONFX, 0);
    tmpEisRegion.confY = request->getVar<MUINT32>(VAR_3DNR_EIS_CONFY, 0);

    // prepare iso
    MINT32 iso = 200;
    iso = request->getVar<MUINT32>(VAR_3DNR_ISO, iso);

    MINT32 isoThreshold = 0;
    isoThreshold = request->getVar<MUINT32>(VAR_3DNR_ISO_THRESHOLD, 0);
    // TODO: move postCropSize, tmpEisRegion, iso and No. to p2a_3dnr to reduce parameters
    MBOOL res = do3dnrFlow(params, request, postCropSize, tmpEisRegion, iso, isoThreshold, request->mRequestNo, rscData);

    TRACE_FUNC_EXIT();
    return res;
}

} // namespace NSFeaturePipe
} // namespace NSCamFeature
} // namespace NSCam

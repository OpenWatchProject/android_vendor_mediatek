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

#include "RSCNode.h"

#define PIPE_CLASS_TAG "RSCNode"
#define PIPE_TRACE TRACE_RSC_NODE
#include <featurePipe/core/include/PipeLog.h>
#include <mtkcam/feature/fsc/fsc_util.h>

namespace NSCam {
namespace NSCamFeature {
namespace NSFeaturePipe {

#define RSC_STREAM_NAME "FeaturePipe_RSC"
#define MVO_OUTPUT_WIDTH(input_width)   ((((input_width+1)/2+6)/7)*16)
#define MVO_OUTPUT_HEIGHT(input_height) (input_height>>1)
#define BVO_OUTPUT_WIDTH(input_width)   (input_width>>1)
#define BVO_OUTPUT_HEIGHT(input_height) (input_height>>1)

RSCNode::RSCNode(const char *name)
    : StreamingFeatureNode(name)
    , mRSCStream(NULL)
{
    TRACE_FUNC_ENTER();
    this->addWaitQueue(&mRequests);
    TRACE_FUNC_EXIT();
}

RSCNode::~RSCNode()
{
    TRACE_FUNC_ENTER();
    TRACE_FUNC_EXIT();
}

MBOOL RSCNode::onData(DataID id, const RequestPtr &data)
{
    TRACE_FUNC_ENTER();
    TRACE_FUNC("Frame %d: %s arrived", data->mRequestNo, ID2Name(id));
    MBOOL ret = MFALSE;
    if( id == ID_ROOT_TO_RSC )
    {
        mRequests.enque(data);
        ret = MTRUE;
    }
    TRACE_FUNC_EXIT();
    return ret;
}

MBOOL RSCNode::onInit()
{
    TRACE_FUNC_ENTER();
    StreamingFeatureNode::onInit();

    MBOOL ret = MFALSE;

    // enable after IT
    mRSCStream = NSCam::NSIoPipe::NSRsc::IRscStream::createInstance(RSC_STREAM_NAME);

    if( !mRSCStream )
    {
        MY_LOGE("Failed to create rsc module");
        return ret;
    }

    bool enableRSCTuning = getPropertyValue(KEY_FORCE_RSC_TUNING, VAL_FORCE_RSC_TUNING);
    if( enableRSCTuning )
    {
        mRSCStream = RSCTuningStream::createInstance(mRSCStream);
    }

    if( mRSCStream )
    {
        ret = mRSCStream->init();
    }

    // Use 128 bits(16 bytes) to store 7 blocks motions
    // MIN_MVO_STRIDE = (((width+1)/2+6)/7)*16
    //            336 = (((288  +1)/2+6)/7)*16
    mMVBufferPool = ImageBufferPool::create("rsc_mv", MSize(336,256), eImgFmt_STA_BYTE, ImageBufferPool::USAGE_HW);
    mBVBufferPool = ImageBufferPool::create("rsc_bv", MSize(144,256), eImgFmt_STA_BYTE, ImageBufferPool::USAGE_HW);
    mDummyBufferPool = ImageBufferPool::create("rsc_dummy", MSize(288,512), eImgFmt_STA_BYTE, ImageBufferPool::USAGE_HW);
    if( mPipeUsage.supportFSC() )
    {
        mFSCRSSOBufferPool = ImageBufferPool::create("fsc_rsso", MSize(288,512), eImgFmt_STA_BYTE, ImageBufferPool::USAGE_HW);
    }
    TRACE_FUNC_EXIT();
    return ret;
}

MBOOL RSCNode::onUninit()
{
    TRACE_FUNC_ENTER();
    if( mRSCStream )
    {
        mRSCStream->uninit();
        mRSCStream->destroyInstance(RSC_STREAM_NAME);
        mRSCStream = NULL;
    }
    mLastMV = NULL;
    mDummy = NULL;
    ImageBufferPool::destroy(mMVBufferPool);
    ImageBufferPool::destroy(mBVBufferPool);
    ImageBufferPool::destroy(mDummyBufferPool);
    mPrevFSCinRSCImgPair.mFSCRsso = NULL;
    mCurrFSCinRSCImgPair.mFSCRsso = NULL;
    ImageBufferPool::destroy(mFSCRSSOBufferPool);
    TRACE_FUNC_EXIT();
    return MTRUE;
}

MBOOL RSCNode::onThreadStart()
{
    TRACE_FUNC_ENTER();
    MBOOL ret = MTRUE;
    Timer timer(MTRUE);
    if( mMVBufferPool != NULL && mBVBufferPool != NULL && mDummyBufferPool != NULL)
    {
        mMVBufferPool->allocate(4);
        mBVBufferPool->allocate(3);
        mDummyBufferPool->allocate(1);
    }
    if( mPipeUsage.supportFSC() )
    {
        if( mFSCRSSOBufferPool != NULL)
        {
            mFSCRSSOBufferPool->allocate(10);
        }
        else
        {
            MY_LOGE("FSC no buffer pool!");
            ret = MFALSE;
        }
    }

    timer.stop();
    MY_LOGD("RSC %s buf in %d ms", STR_ALLOCATE, timer.getElapsed());
    TRACE_FUNC_EXIT();
    return ret;
}

MBOOL RSCNode::onThreadStop()
{
    TRACE_FUNC_ENTER();
    this->waitRSCStreamBaseDone();
    TRACE_FUNC_EXIT();
    return MTRUE;
}

MBOOL RSCNode::onThreadLoop()
{
    CAM_TRACE_CALL();
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
    if( request == NULL )
    {
        MY_LOGE("Request out of sync");
        return MFALSE;
    }

    TRACE_FUNC_ENTER();
    request->mTimer.startRSC();
    TRACE_FUNC("Frame %d in RSC needRSC %d", request->mRequestNo, request->needRSC());
    if( mPipeUsage.supportFSC() )
    {
        mCurrFSCinRSCImgPair.mCropRectF = request->getVar<MRectF>(VAR_FSC_RSSO_CROP_REGION, MRectF(0, 0));
    }

    if( request->needRSC() )
    {
        processRSC(request);
    }
    else
    {
        handleResultData(request, RSCResult());
    }

    if( mPipeUsage.supportFSC() )
    {
        mPrevFSCinRSCImgPair = mCurrFSCinRSCImgPair;
    }
    request->mTimer.stopRSC();
    TRACE_FUNC_EXIT();
    return MTRUE;
}

MBOOL RSCNode::processRSC(const RequestPtr &request)
{
    TRACE_FUNC_ENTER();

    RSCEnqueData data;
    prepareRSCEnqueData(request, data);

    if (data.useDummy)
    {
        //return result immediately for first frame to speed up pipeline
        MY_LOGD("return dummy RSC result");
        handleResultData(request, RSCResult());
    }
    else
    {
        NSIoPipe::RSCConfig rscconfig;
        prepareRSCConfig(data, rscconfig);

        RSCParam param;
        param.mRSCConfigVec.push_back(rscconfig);

        enqueRSCStream(param, data);
    }

    TRACE_FUNC_EXIT();
    return MTRUE;
}

MVOID RSCNode::prepareRSCEnqueData(const RequestPtr &request, RSCEnqueData &data)
{
    CAM_TRACE_CALL();
    TRACE_FUNC_ENTER();

    IImageBuffer *prevRsso = NULL, *currRsso = NULL;

    prevRsso = request->getVar<IImageBuffer*>(VAR_PREV_RSSO, NULL);
    currRsso = request->getVar<IImageBuffer*>(VAR_CURR_RSSO, NULL);

    if ( mPipeUsage.supportFSC() )
    {
        data.mPrevFSCOrgRsso = prevRsso;
        data.mCurrFSCOrgRsso = currRsso;
        prepareFSCRsso(prevRsso, mPrevFSCinRSCImgPair);
        data.mPrevFSCRsso = mPrevFSCinRSCImgPair.mFSCRsso;//keep reference
        prepareFSCRsso(currRsso, mCurrFSCinRSCImgPair);
        data.mCurrFSCRsso = mCurrFSCinRSCImgPair.mFSCRsso;//keep reference

        MY_LOGD_IF(FSCUtil::getFSCDebugLevel(mPipeUsage.getFSCMode()), "sensor(%d) Frame %d rsso(prev/curr)=(%p,%p) fsc_rsso(prev/curr)=(%p,%p)", mSensorIndex, request->mRequestNo, prevRsso, currRsso,
            data.mPrevFSCRsso.get(), data.mCurrFSCRsso.get());

        if (mPrevFSCinRSCImgPair.mFSCRsso != NULL)
            prevRsso = mPrevFSCinRSCImgPair.mFSCRsso->getImageBufferPtr();
        if (mCurrFSCinRSCImgPair.mFSCRsso != NULL)
            currRsso = mCurrFSCinRSCImgPair.mFSCRsso->getImageBufferPtr();
    }

    if ( prevRsso != NULL && currRsso != NULL && prevRsso->getImgSize() == currRsso->getImgSize() )
    {
        data.mPrevRsso = prevRsso;
        data.mCurrRsso = currRsso;
        data.useDummy  = MFALSE;
    }
    else
    {
        MSize prevRssoSize = prevRsso ? prevRsso->getImgSize() : MSize();
        MSize currRssoSize = currRsso ? currRsso->getImgSize() : MSize();

        MY_LOGW("sensor(%d) Frame %d rsso(prev/curr)=(%p_%dx%d,%p_%dx%d)", mSensorIndex, request->mRequestNo, prevRsso, prevRssoSize.w, prevRssoSize.h,
            currRsso, currRssoSize.w, currRssoSize.h);

        if( mDummy == NULL )
        {
            mDummy = mDummyBufferPool->requestIIBuffer();
            mDummy->getImageBufferPtr()->setExtParam(MSize(288, 163));
        }

        data.mPrevRsso = mDummy->getImageBufferPtr();
        data.mCurrRsso = mDummy->getImageBufferPtr();
        data.useDummy = MTRUE;
    }

    MSize rssoSize = data.mCurrRsso->getImgSize();

    data.mRequest = request;
    data.mMV = mMVBufferPool->requestIIBuffer();
    data.mMV->getImageBufferPtr()->setExtParam(MSize(MVO_OUTPUT_WIDTH(rssoSize.w), MVO_OUTPUT_HEIGHT(rssoSize.h)));
    data.mBV = mBVBufferPool->requestIIBuffer();
    data.mBV->getImageBufferPtr()->setExtParam(MSize(BVO_OUTPUT_WIDTH(rssoSize.w), BVO_OUTPUT_HEIGHT(rssoSize.h)));

    if( data.useDummy )
    {
        mLastMV = NULL;
    }
    else
    {
        data.mPrevMV = mLastMV;
        mLastMV = data.mMV;
    }

    if( data.mPrevMV == NULL )
    {
        data.mPrevMV = mMVBufferPool->requestIIBuffer();
        data.mPrevMV ->getImageBufferPtr()->setExtParam(MSize(MVO_OUTPUT_WIDTH(rssoSize.w), MVO_OUTPUT_HEIGHT(rssoSize.h)));
    }

    TRACE_FUNC_EXIT();
}

MBOOL RSCNode::prepareFSCRsso(IImageBuffer *iRsso, FSCinRSCImgPair &fscinRSCImgPair)
{
    if (iRsso == NULL)
    {
        fscinRSCImgPair.mFSCRsso = NULL;
    }
    else
    {
        if (fscinRSCImgPair.mRssoTimestamp != iRsso->getTimestamp() || fscinRSCImgPair.mFSCRsso == NULL)
        {
            fscinRSCImgPair.mRssoTimestamp = iRsso->getTimestamp();// store image pair
            //MDP+copy last line
            MSize sourceSize = iRsso->getImgSize();
            if (sourceSize.h > 0)
                sourceSize.h -= 1;/*last line is pattern*/
            iRsso->syncCache(eCACHECTRL_INVALID);

            // prepare source crop
            MCropRect cropRect;
            cropRect = getCropRect(fscinRSCImgPair.mCropRectF);
            if( FSCUtil::isFSCSubpixelEnabled(mPipeUsage.getFSCMode()) != MTRUE)
            {
                cropRect.p_fractional.x = 0;
                cropRect.p_fractional.y = 0;
                cropRect.w_fractional = 0;
                cropRect.h_fractional = 0;
            }

            //get new buffer
            fscinRSCImgPair.mFSCRsso = mFSCRSSOBufferPool->requestIIBuffer();
            IImageBuffer *fscRsso = fscinRSCImgPair.mFSCRsso->getImageBufferPtr();
            //keep aspect ratio
            MSize targetSize = fscRsso->getImgSize();
            targetSize.h = targetSize.w * sourceSize.h / sourceSize.w;
            fscRsso->setExtParam(MSize(targetSize.w, targetSize.h));

            //prepare output
            MDPWrapper::OUTPUT_ARRAY outputs;
            NSIoPipe::Output output;
            output.mPortID = PortID(EPortType_Memory, NSImageio::NSIspio::EPortIndex_RSSO, PORTID_OUT, NSIoPipe::EPortCapbility_Disp);//EPortCapbility_Disp for full range
            output.mBuffer = fscRsso;
            outputs.push_back(MDPWrapper::MDPOutput(output, cropRect));

            if (!mMDP.process(iRsso, outputs))
            {
                MY_LOGE("FSC RSSO MDP failed!");
            }

            // copy last line
            MUINT8 *pSrc = (MUINT8*)iRsso->getBufVA(0);
            pSrc += (iRsso->getBufStridesInBytes(0)*sourceSize.h);
            MUINT8 *pDst = (MUINT8*)fscRsso->getBufVA(0);
            pDst += (fscRsso->getBufStridesInBytes(0)*targetSize.h);
            if( sourceSize.h > 1 &&  targetSize.h > 1 )
            {
                const int RSS_APLI_DATA_BYTES = 4;
                memcpy(pDst, pSrc, RSS_APLI_DATA_BYTES);
                fscRsso->setExtParam(MSize(targetSize.w, targetSize.h+1/*last line is pattern*/));
            }
            else
            {
                MY_LOGW("Invalid FSC RSSO sourceSize(%dx%d),targetSize(%dx%d)",
                        sourceSize.w, sourceSize.h, targetSize.w, targetSize.h);
            }

            MY_LOGD_IF(FSCUtil::getFSCDebugLevel(mPipeUsage.getFSCMode()), "rsso(%p) t(%" PRIi64 ") %dx%d_%d->%dx%d_%d"
                " (%f,%f,%f,%f)->crop(%d,%d,0x%x,0x%x,%d,%d) fsc_rsso(%p) copy(0x%x)",
                iRsso, iRsso->getTimestamp(), iRsso->getImgSize().w, iRsso->getImgSize().h, iRsso->getBufStridesInBytes(0),
                fscRsso->getImgSize().w, fscRsso->getImgSize().h, fscRsso->getBufStridesInBytes(0),
                fscinRSCImgPair.mCropRectF.p.x, fscinRSCImgPair.mCropRectF.p.y, fscinRSCImgPair.mCropRectF.s.w, fscinRSCImgPair.mCropRectF.s.h,
                cropRect.p_integral.x, cropRect.p_integral.y, cropRect.p_fractional.x, cropRect.p_fractional.y, cropRect.s.w, cropRect.s.h,
                fscRsso, *(MUINT32*)pSrc);
            fscRsso->syncCache(eCACHECTRL_FLUSH);

        }
    }
    return MTRUE;
}

MVOID RSCNode::prepareRSCConfig(RSCEnqueData &data, NSIoPipe::RSCConfig &config)
{
    TRACE_FUNC_ENTER();

    IImageBuffer2RSCBufInfo(NSIoPipe::DMA_RSC_IMGI_C, data.mCurrRsso, config.Rsc_Imgi_c);
    IImageBuffer2RSCBufInfo(NSIoPipe::DMA_RSC_IMGI_P, data.mPrevRsso, config.Rsc_Imgi_p);
    IImageBuffer2RSCBufInfo(NSIoPipe::DMA_RSC_MVO, data.mMV->getImageBufferPtr(), config.Rsc_mvo);
    IImageBuffer2RSCBufInfo(NSIoPipe::DMA_RSC_BVO, data.mBV->getImageBufferPtr(), config.Rsc_bvo);
    if( data.mPrevMV != NULL )
    {
        IImageBuffer2RSCBufInfo(NSIoPipe::DMA_RSC_MVI, data.mPrevMV->getImageBufferPtr(), config.Rsc_mvi);
        config.Rsc_Ctrl_Skip_Pre_Mv = false;
    }
    else
    {
        config.Rsc_Ctrl_Skip_Pre_Mv = true;
    }

    if( data.mCurrRsso )
    {
        config.Rsc_Size_Height = data.mCurrRsso->getImgSize().h;
        config.Rsc_Size_Width = data.mCurrRsso->getImgSize().w;
        config.Rsc_Ctrl_Init_MV_Waddr     = ((config.Rsc_Size_Width+1)/2-1)/7;
        config.Rsc_Ctrl_Init_MV_Flush_cnt = ((config.Rsc_Size_Width+1)/2-1)%7;
    }

    if( data.mPrevRsso )
    {
        config.Rsc_Size_Height_p = data.mPrevRsso->getImgSize().h;
        config.Rsc_Size_Width_p = data.mPrevRsso->getImgSize().w;
    }

    TRACE_FUNC("Rsc_Ctrl_Skip_Pre_Mv(%d) Rsc_Size_Width(%d) Rsc_Size_Height(%d) Rsc_Ctrl_Init_MV_Waddr(%d) Rsc_Ctrl_Init_MV_Flush_cnt(%d)",
            config.Rsc_Ctrl_Skip_Pre_Mv, config.Rsc_Size_Width, config.Rsc_Size_Height, config.Rsc_Ctrl_Init_MV_Waddr, config.Rsc_Ctrl_Init_MV_Flush_cnt);
    TRACE_FUNC("Rsc_Size_Width_p(%d) Rsc_Size_Height_p(%d)",
            config.Rsc_Size_Width_p, config.Rsc_Size_Height_p);

    TRACE_FUNC_EXIT();
}

MVOID RSCNode::enqueRSCStream(const RSCParam &param, const RSCEnqueData &data)
{
    CAM_TRACE_ASYNC_BEGIN("RSC:enqueRSCStream", data.mRequest->mRequestNo);
    TRACE_FUNC_ENTER();

    MY_LOGD("sensor(%d) Frame %d rsc enque start", mSensorIndex, data.mRequest->mRequestNo);
    data.mRequest->mTimer.startEnqueRSC();
    this->incExtThreadDependency();
    this->enqueRSCStreamBase(mRSCStream, param, data);

    TRACE_FUNC_EXIT();
}

MVOID RSCNode::onRSCStreamBaseCB(const RSCParam &param, const RSCEnqueData &data)
{
    // This function is not thread safe,
    // avoid accessing RSCNode class memebers
    TRACE_FUNC_ENTER();
    (void)(param);

    RequestPtr request = data.mRequest;
    if( request == NULL )
    {
        MY_LOGE("Missing request");
    }
    else
    {
        request->mTimer.stopEnqueRSC();

        MY_LOGD("sensor(%d) Frame %d rsc enque done in %d ms", mSensorIndex, request->mRequestNo, request->mTimer.getElapsedEnqueRSC());

        if( request->needDump() )
        {
            data.mCurrRsso->syncCache(eCACHECTRL_INVALID);
            dumpData(data.mRequest, data.mCurrRsso, "rsso");
            data.mPrevRsso->syncCache(eCACHECTRL_INVALID);
            dumpData(data.mRequest, data.mPrevRsso, "prev_rsso");
            data.mMV->getImageBuffer()->syncCache(eCACHECTRL_INVALID);
            dumpData(data.mRequest, data.mMV->getImageBufferPtr(), "rsc_mv");
            data.mBV->getImageBuffer()->syncCache(eCACHECTRL_INVALID);
            dumpData(data.mRequest, data.mBV->getImageBufferPtr(), "rsc_bv");

            if( data.mPrevMV != NULL )
            {
                data.mPrevMV->getImageBuffer()->syncCache(eCACHECTRL_INVALID);
                dumpData(data.mRequest, data.mPrevMV->getImageBufferPtr(), "rsc_prev_mv");
            }
            if( data.mPrevFSCOrgRsso != NULL)
            {
                data.mPrevFSCOrgRsso->syncCache(eCACHECTRL_INVALID);
                dumpData(data.mRequest, data.mPrevFSCOrgRsso, "fsc_prev_rsso");
            }
            if( data.mCurrFSCOrgRsso != NULL)
            {
                data.mCurrFSCOrgRsso->syncCache(eCACHECTRL_INVALID);
                dumpData(data.mRequest, data.mCurrFSCOrgRsso, "fsc_rsso");
            }
        }

        if( data.useDummy )
        {
            handleResultData(request, RSCResult());
        }
        else
        {
            const NSIoPipe::RSCConfig *rscconfig = param.mRSCConfigVec.data();
            RSCResult::RSC_STA_0 rscSta;
            rscSta.value = (rscconfig != NULL) ? rscconfig->feedback.RSC_STA_0 : 0;

            handleResultData(request, RSCResult(data.mMV, data.mBV, data.mCurrRsso->getImgSize(), rscSta, true));
        }

        CAM_TRACE_ASYNC_END("RSC:enqueRSCStream", request->mRequestNo);
        request->mTimer.stopRSC();
    }

    this->decExtThreadDependency();
    TRACE_FUNC_EXIT();
}

MVOID RSCNode::handleResultData(const RequestPtr &request, const RSCResult &result)
{
    CAM_TRACE_CALL();

    if( mPipeUsage.supportEIS_30() )
    {
        handleData(ID_RSC_TO_EIS, RSCData(result, request));
    }
    if( mPipeUsage.support3DNRRSC() )
    {
        handleData(ID_RSC_TO_P2A, RSCData(result, request));
    }
    handleData(ID_RSC_TO_HELPER, CBMsgData(FeaturePipeParam::MSG_RSSO_DONE, request));
}

MVOID RSCNode::IImageBuffer2RSCBufInfo(NSIoPipe::RSCDMAPort dmaPort, IImageBuffer *pImageBuffer, NSIoPipe::RSCBufInfo& rscBufInfo)
{
    TRACE_FUNC_ENTER();

    if( pImageBuffer )
    {
        rscBufInfo.dmaport = dmaPort;
        rscBufInfo.memID = pImageBuffer->getFD(0);                      //  memory ID
        rscBufInfo.u4BufVA = pImageBuffer->getBufVA(0);                 //  Vir Address of pool
        rscBufInfo.u4BufPA = pImageBuffer->getBufPA(0);                 //  Phy Address of pool
        rscBufInfo.u4BufSize = pImageBuffer->getBufSizeInBytes(0);      //  Per buffer size
        rscBufInfo.u4Stride = pImageBuffer->getBufStridesInBytes(0);    //  Buffer Stride


        TRACE_FUNC("dmaPort(%d) u4BufVA(%p) u4BufSize(%d)", rscBufInfo.dmaport, rscBufInfo.u4BufVA, rscBufInfo.u4BufSize);
        TRACE_FUNC("memID(%d) u4BufPA(%p) u4Stride(%d)", rscBufInfo.memID, rscBufInfo.u4BufPA, rscBufInfo.u4Stride);
    }

    TRACE_FUNC_EXIT();
}

} // namespace NSFeaturePipe
} // namespace NSCamFeature
} // namespace NSCam

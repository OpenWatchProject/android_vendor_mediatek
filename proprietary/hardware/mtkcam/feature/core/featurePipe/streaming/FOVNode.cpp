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

#include "FOVNode.h"

#define PIPE_CLASS_TAG "FOVNode"
#define PIPE_TRACE TRACE_FOV_NODE
#include <featurePipe/core/include/PipeLog.h>

namespace NSCam {
namespace NSCamFeature {
namespace NSFeaturePipe {

using namespace NSCam::NSIoPipe;

FOVNode::FOVNode(const char *name)
    : StreamingFeatureNode(name)
{
    TRACE_FUNC_ENTER();
    this->addWaitQueue(&mFOVDatas);
    this->addWaitQueue(&mFullImgDatas);
    TRACE_FUNC_EXIT();
}

FOVNode::~FOVNode()
{
    TRACE_FUNC_ENTER();
    TRACE_FUNC_EXIT();
}

MBOOL FOVNode::onData(DataID id, const FOVP2AData &data)
{
    TRACE_FUNC_ENTER();
    TRACE_FUNC("Frame %d: %s arrived", data.mRequest->mRequestNo, ID2Name(id));
    MBOOL ret = MFALSE;
    if( id == ID_P2A_TO_FOV_FEFM )
    {
        mFOVDatas.enque(data);
        ret = MTRUE;
    }
    TRACE_FUNC_EXIT();
    return ret;
}


MBOOL FOVNode::onData(DataID id, const ImgBufferData &data)
{
    TRACE_FUNC_ENTER();
    TRACE_FUNC("Frame %d: %s arrived", data.mRequest->mRequestNo, ID2Name(id));
    MBOOL ret = MFALSE;
    if( id == ID_P2A_TO_FOV_FULLIMG )
    {
        mFullImgDatas.enque(data);
        ret = MTRUE;

    }
    TRACE_FUNC_EXIT();
    return ret;
}


MBOOL FOVNode::onInit()
{
    TRACE_FUNC_ENTER();
    FOVHal::InitConfig config;
    StreamingFeatureNode::onInit();
    mWarpBufPool = FatImageBufferPool::create("fpipe.fov.warp", 2, 2, eImgFmt_WARP_2PLANE, ImageBufferPool::USAGE_HW);
    mWarpBufPool->allocate(10);
    mpFOVHal = FOVHal::getInstance();
    mpFOVHal->init(config);
    TRACE_FUNC_EXIT();
    return MTRUE;
}

MBOOL FOVNode::onUninit()
{
    TRACE_FUNC_ENTER();
    FatImageBufferPool::destroy(mWarpBufPool);
    mpFOVHal->uninit();
    mpFOVHal = NULL;
    TRACE_FUNC_EXIT();
    return MTRUE;
}

MBOOL FOVNode::onThreadStart()
{
    TRACE_FUNC_ENTER();
    TRACE_FUNC_EXIT();
    return MTRUE;
}

MBOOL FOVNode::onThreadStop()
{
    TRACE_FUNC_ENTER();
    TRACE_FUNC_EXIT();
    return MTRUE;
}

MBOOL FOVNode::onThreadLoop()
{
    TRACE_FUNC("Waitloop");
    FOVP2AData fovData;
    ImgBufferData fullImg;
    RequestPtr request;
    if( !waitAllQueue() )
    {
        return MFALSE;
    }
    if( !mFOVDatas.deque(fovData) )
    {
        MY_LOGE("Request deque out of sync");
        return MFALSE;
    }
    if( !mFullImgDatas.deque(fullImg) )
    {
        MY_LOGE("Request deque out of sync");
        return MFALSE;
    }

    if( fovData.mRequest == NULL || fullImg.mRequest == NULL)
    {
        MY_LOGE("Request out of sync");
        return MFALSE;
    }
    TRACE_FUNC_ENTER();
    request = fovData.mRequest;
    request->mTimer.startFOV();
    TRACE_FUNC("Frame %d in FOVNode", request->mRequestNo);
    processFOV(request, fovData, fullImg);
    request->mTimer.stopFOV();
    TRACE_FUNC_EXIT();
    return MTRUE;
}

MBOOL FOVNode::processFOV(
    const RequestPtr &request,
    const FOVP2AData &fovData,
    const ImgBufferData &fullImg
)
{
    TRACE_FUNC_ENTER();
    (void)(fovData);
    // TODO: add fov processing here


    // TODO: pass warp map for FOVWarpNode input
    //ImgBuffer pWarpMap = nullptr;
    FOVResult Warp;
    FOVHal::Params param;
    Warp.mWarpMap = mWarpBufPool->requestIIBuffer();

    mpFOVHal->setCamId(request->getMasterID());

    if (!request->needFullImg() && !request->needFOVFEFM()) {
        TRACE_FUNC("Frame:%d, do not need fov", request->mRequestNo);
        MY_LOGD("needFOVFEFM : %d", request->needFOVFEFM());
        handleData(ID_FOV_TO_FOV_WARP,  FOVData(Warp, request));

        if( mPipeUsage.supportEISNode() )
        {
            handleData(ID_FOV_TO_EIS_WARP, FOVData(Warp, request));
        }

        return MTRUE;
    }
    param.FOVType = FOVHal::MTK_FOV_TYPE_OFFLINE;
    param.ZoomRatio = request->getVar<MUINT32>(VAR_DUALCAM_ZOOM_RATIO, 0); // get by param
    if (param.ZoomRatio == 0) {
        MY_LOGW("zoom ratio should not be zero...set 100(1x)");
        param.ZoomRatio = 100;
    }
    MY_LOGD("zoom ratio : %d", param.ZoomRatio);
    param.FOVInfo = request->getVar<DUAL_ZOOM_FOV_INPUT_INFO>(VAR_DUALCAM_FOV_RECT, param.FOVInfo);
    if (request->needFullImg())
    {
        param.SrcImgSize = fullImg->getImageBufferPtr()->getImgSize();
    }
    else
    {
        param.SrcImgSize = MSize(1920, 1440);
    }
    param.DoFOV = false;
    param.DoCalibration = false;
    param.FOVUpdated = 0;
    if (request->needFOV())
    {
        param.DoFOV = true;
    }

    if (request->needFOVFEFM())
    {
        param.DoCalibration = true;
        param.FE_master = (MINT16 *)(fovData.mData.mFEO_Master->getImageBufferPtr()->getBufVA(0));
        param.FE_slave  = (MINT16 *)(fovData.mData.mFEO_Slave->getImageBufferPtr()->getBufVA(0));
        param.FM_master = (MINT16 *)(fovData.mData.mFMO_MtoS->getImageBufferPtr()->getBufVA(0));
        param.FM_slave  = (MINT16 *)(fovData.mData.mFMO_StoM->getImageBufferPtr()->getBufVA(0));
        param.FECrop_master = fovData.mData.mFEInSize_Master;
        param.FECrop_slave  = fovData.mData.mFEInSize_Slave;
        param.MasterID  = request->getMasterID();
    }
    else
    {
        param.DoCalibration = false;
    }

    param.pWarpMapX = (MUINT32 *)(Warp.mWarpMap->getImageBufferPtr()->getBufVA(0));
    param.pWarpMapY = (MUINT32 *)(Warp.mWarpMap->getImageBufferPtr()->getBufVA(1));
    mpFOVHal->doFOV(param);

    if( request->needEIS() )
    {
        param.WPEOutput.w = request->getEISInputSize().w;
        param.WPEOutput.h = request->getEISInputSize().h;
    }

    Warp.mWarpMapSize = param.WarpMapSize;
    Warp.mWPESize = param.WPEOutput;
    Warp.mSensorBaseMargin = param.SensorMargin;
    Warp.mRRZOBaseMargin = param.ResizeMargin;
    Warp.mFOVShift = param.FOVShift;
    Warp.mFOVScale = param.FOVScale;

    request->setVar<MUINT32>(VAR_DUALCAM_FOV_ONLINE, param.FOVUpdated);

    if (request->needFOV())
    {
        DecideStreamsCrop(Warp, request);
    }

    handleData(ID_FOV_TO_FOV_WARP,  FOVData(Warp, request));

    if( mPipeUsage.supportEISNode() )
    {
        handleData(ID_FOV_TO_EIS_WARP, FOVData(Warp, request));
    }

    TRACE_FUNC_EXIT();
    return MTRUE;
}

MBOOL FOVNode::DecideStreamsCrop(
        FOVResult &Warp,
        const RequestPtr &request
)
{
    if( request->needEIS() )
    {
        NSIoPipe::MCrpRsInfo DisCrop;
        Output yuvOut;

        if( request->getDisplayOutput(yuvOut) )
        {
            request->getDisplayCrop(DisCrop);
            if (DisCrop.mCropRect.p_integral.x > (Warp.mRRZOBaseMargin.w/2))
            {
                DisCrop.mCropRect.p_integral.x -= (Warp.mRRZOBaseMargin.w/2);
            }
            else
            {
                DisCrop.mCropRect.p_integral.x = 0;
            }
            if (DisCrop.mCropRect.p_integral.y > (Warp.mRRZOBaseMargin.h/2))
            {
                DisCrop.mCropRect.p_integral.y -= (Warp.mRRZOBaseMargin.h/2);
            }
            else
            {
                DisCrop.mCropRect.p_integral.y = 0;
            }
            Warp.mDisplayCrop.p.x = (DisCrop.mCropRect.p_integral.x) & ~(0x01);
            Warp.mDisplayCrop.p.y = (DisCrop.mCropRect.p_integral.y) & ~(0x01);
            Warp.mDisplayCrop.s.w = DisCrop.mCropRect.s.w;
            Warp.mDisplayCrop.s.h = DisCrop.mCropRect.s.h;
            if ((Warp.mDisplayCrop.p.x + Warp.mDisplayCrop.s.w) > Warp.mWPESize.w)
            {
                Warp.mDisplayCrop.s.w = Warp.mWPESize.w - Warp.mDisplayCrop.p.x;
            }
            if ((Warp.mDisplayCrop.p.y + Warp.mDisplayCrop.s.h) > Warp.mWPESize.h)
            {
                Warp.mDisplayCrop.s.h = Warp.mWPESize.h - Warp.mDisplayCrop.p.y;
            }
            MY_LOGD("WPE output size : %dx%d", Warp.mWPESize.w, Warp.mWPESize.h);
            MY_LOGD("display crop : (%d, %d), %dx%d", Warp.mDisplayCrop.p.x, Warp.mDisplayCrop.p.y,
                                                      Warp.mDisplayCrop.s.w, Warp.mDisplayCrop.s.h);
        }

    }
    else
    {
        Output yuvOut;
        if( request->getDisplayOutput(yuvOut) )
        {
            NSIoPipe::MCrpRsInfo DisCrop;
            request->getDisplayCrop(DisCrop);
            if (DisCrop.mCropRect.p_integral.x > (Warp.mRRZOBaseMargin.w/2))
            {
                DisCrop.mCropRect.p_integral.x -= (Warp.mRRZOBaseMargin.w/2);
            }
            else
            {
                DisCrop.mCropRect.p_integral.x = 0;
            }
            if (DisCrop.mCropRect.p_integral.y > (Warp.mRRZOBaseMargin.h/2))
            {
                DisCrop.mCropRect.p_integral.y -= (Warp.mRRZOBaseMargin.h/2);
            }
            else
            {
                DisCrop.mCropRect.p_integral.y = 0;
            }
            Warp.mDisplayCrop.p.x = (DisCrop.mCropRect.p_integral.x) & ~(0x01);
            Warp.mDisplayCrop.p.y = (DisCrop.mCropRect.p_integral.y) & ~(0x01);
            Warp.mDisplayCrop.s.w = DisCrop.mCropRect.s.w;
            Warp.mDisplayCrop.s.h = DisCrop.mCropRect.s.h;
            if ((Warp.mDisplayCrop.p.x + Warp.mDisplayCrop.s.w) > Warp.mWPESize.w)
            {
                Warp.mDisplayCrop.s.w = Warp.mWPESize.w - Warp.mDisplayCrop.p.x;
            }
            if ((Warp.mDisplayCrop.p.y + Warp.mDisplayCrop.s.h) > Warp.mWPESize.h)
            {
                Warp.mDisplayCrop.s.h = Warp.mWPESize.h - Warp.mDisplayCrop.p.y;
            }
            MY_LOGD("WPE output size : %dx%d", Warp.mWPESize.w, Warp.mWPESize.h);
            MY_LOGD("display crop : (%d, %d), %dx%d", Warp.mDisplayCrop.p.x, Warp.mDisplayCrop.p.y,
                                                      Warp.mDisplayCrop.s.w, Warp.mDisplayCrop.s.h);
        }
        // get output : record
        if( request->getRecordOutput(yuvOut) )
        {

            MCrpRsInfo RecCrop;
            request->getRecordCrop(RecCrop);
            if (RecCrop.mCropRect.p_integral.x > (Warp.mRRZOBaseMargin.w/2))
            {
                RecCrop.mCropRect.p_integral.x -= (Warp.mRRZOBaseMargin.w/2);
            }
            else
            {
                RecCrop.mCropRect.p_integral.x = 0;
            }
            if (RecCrop.mCropRect.p_integral.y > (Warp.mRRZOBaseMargin.h/2))
            {
                RecCrop.mCropRect.p_integral.y -= (Warp.mRRZOBaseMargin.h/2);
            }
            else
            {
                RecCrop.mCropRect.p_integral.y = 0;
            }
            Warp.mRecordCrop.p.x = (RecCrop.mCropRect.p_integral.x) & ~(0x01);
            Warp.mRecordCrop.p.y = (RecCrop.mCropRect.p_integral.y) & ~(0x01);
            Warp.mRecordCrop.s.w = RecCrop.mCropRect.s.w;
            Warp.mRecordCrop.s.h = RecCrop.mCropRect.s.h;
            if ((Warp.mRecordCrop.p.x + Warp.mRecordCrop.s.w) > Warp.mWPESize.w)
            {
                Warp.mRecordCrop.s.w = Warp.mWPESize.w - Warp.mRecordCrop.p.x;
            }
            if ((Warp.mRecordCrop.p.y + Warp.mRecordCrop.s.h) > Warp.mWPESize.h)
            {
                Warp.mRecordCrop.s.h = Warp.mWPESize.h - Warp.mRecordCrop.p.y;
            }
            MY_LOGD("WPE output size : %dx%d", Warp.mWPESize.w, Warp.mWPESize.h);
            MY_LOGD("Record crop : (%d, %d), %dx%d", Warp.mRecordCrop.p.x, Warp.mRecordCrop.p.y,
                                                      Warp.mRecordCrop.s.w, Warp.mRecordCrop.s.h);
        }

        // output : extra yuv
        Output extra;
        if( request->getExtraOutput(extra) )
        {
            MSize YuvSize = extra.mBuffer->getImgSize();
            int transform = extra.mTransform;
            MY_LOGD("Extra size : %dx%d, rot : %d", YuvSize.w, YuvSize.h, transform);
            if (transform & eTransform_ROT_90)
            {
                int swap = YuvSize.w;
                YuvSize.w = YuvSize.h;
                YuvSize.h = swap;
            }
            GetCrop(Warp.mWPESize, YuvSize, Warp.mExtraCrop);
            MY_LOGD("Extra crop : (%d, %d), %dx%d", Warp.mExtraCrop.p.x, Warp.mExtraCrop.p.y,
                                                      Warp.mExtraCrop.s.w, Warp.mExtraCrop.s.h);
        }
    }
    return MTRUE;
}


MVOID FOVNode::GetCrop(
        MSize &ImgSize,
        MSize &StreamSize,
        MRect &Crop
)
{
    if( ImgSize.w * StreamSize.h > ImgSize.h * StreamSize.w ) { // pillarbox
        Crop.s.w = ImgSize.h * StreamSize.w / StreamSize.h;
        Crop.s.h = ImgSize.h;
        Crop.p.x = (ImgSize.w - Crop.s.w) >> 1;
        Crop.p.y = 0;
    }
    else { // letterbox
        Crop.s.w = ImgSize.w;
        Crop.s.h = ImgSize.w * StreamSize.h / StreamSize.w;
        Crop.p.x = 0;
        Crop.p.y = (ImgSize.h - Crop.s.h) >> 1;
    }
}

} // namespace NSFeaturePipe
} // namespace NSCamFeature
} // namespace NSCam

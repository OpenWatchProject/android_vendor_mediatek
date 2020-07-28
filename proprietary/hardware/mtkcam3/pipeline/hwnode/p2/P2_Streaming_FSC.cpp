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
 * MediaTek Inc. (C) 2017. All rights reserved.
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

#include <cutils/properties.h>
#include "P2_StreamingProcessor.h"

#include "P2_DebugControl.h"
#define P2_CLASS_TAG    Streaming_FSC
#define P2_TRACE        TRACE_STREAMING_FSC
#include "P2_LogHeader.h"
#include <mtkcam3/feature/fsc/fsc_defs.h>
#include <mtkcam3/feature/fsc/fsc_ext.h>

CAM_ULOG_DECLARE_MODULE_ID(MOD_P2_STR_PROC);

namespace P2
{

using namespace NSCam::FSC;
using namespace NSCam::NSCamFeature::NSFeaturePipe;

static MVOID drawRect(IImageBuffer *buffer, const MRect &cropRect, MINT32 lineWidth, MINT32 value)
{
    MVOID* virtAddr = (MVOID*)(buffer->getBufVA(0));
    MINT32 imgStride = buffer->getBufStridesInBytes(0);
    MUINT32 pbpp = buffer->getPlaneBitsPerPixel(0);//bayer8 = 15 bits
    MINT32 i = 0;
    buffer->syncCache(NSCam::eCACHECTRL_INVALID);

    for( i=cropRect.p.y; i<cropRect.p.y+lineWidth; i++ )
    {
        memset((void*)((char*)virtAddr+imgStride*i),
                       value,
                       imgStride);
    }

    for( i=cropRect.p.y+cropRect.s.h-1; i>=cropRect.p.y+cropRect.s.h-lineWidth; i-- )
    {
        memset((void*)((char*)virtAddr+imgStride*i),
                       value,
                       imgStride);
    }

    for( i=0; i<buffer->getImgSize().h; i++ )
    {
        memset((void*)((char*)virtAddr+cropRect.p.x*pbpp/8+imgStride*i),
                       value,
                       lineWidth*pbpp/8);
    }

    for( i=0; i<buffer->getImgSize().h; i++ )
    {
        memset((void*)((char*)virtAddr+(cropRect.p.x+cropRect.s.w-1-lineWidth)*pbpp/8+imgStride*i),
                       value,
                       lineWidth*pbpp/8);
    }
    buffer->syncCache(NSCam::eCACHECTRL_FLUSH);
}
MBOOL StreamingProcessor::prepareFSC(P2Util::SimpleIn& input, const ILog &log) const
{
    TRACE_S_FUNC_ENTER(log);

    if (EFSC_FSC_ENABLED(mPipeUsageHint.mFSCMode))
    {
        const sp<P2Request> &request = input.mRequest;
        const sp<Cropper> cropper = request->getCropper();
        if (isEIS12() && cropper->isEISAppOn())
        {
            TRACE_S_FUNC(log, "disable FSC due to EIS 1.2!");
        }
        else
        {
            FeaturePipeParam &featureParam = input.mFeatureParam;
            IMetadata *inHal = input.mRequest->getMetaPtr(IN_P1_HAL);

            // set FSC crop data
            IMetadata::IEntry cropEntry;
            if( inHal )
            {
                cropEntry = inHal->entryFor(MTK_FSC_CROP_DATA);
            }
            if (cropEntry.count())
            {
                IMetadata::Memory metaMemory = cropEntry.itemAt(0, Type2Type< IMetadata::Memory >());
                FSC_CROPPING_DATA_STRUCT *cropData = (FSC_CROPPING_DATA_STRUCT*)metaMemory.array();
                auto& rrzR = cropData->RRZOFSCRegion;
                auto& rssR = cropData->RSSOFSCRegion;
                featureParam.setVar<MRectF>(SFP_VAR::FSC_RRZO_CROP_REGION, rrzR);
                featureParam.setVar<MRectF>(SFP_VAR::FSC_RSSO_CROP_REGION, rssR);

                MFLOAT fscScaleRatio = cropData->image_scale;
                if (EFSC_DEBUG_ENABLED(mPipeUsageHint.mFSCMode) && ::property_get_int32(FSC_FORCE_MAX_CROP_PROPERTY, 0))
                {
                    MFLOAT maxCropValue = ::property_get_int32(FSC_MAX_CROP_VALUE_PROPERTY, 80);
                    fscScaleRatio = maxCropValue/100;
                }
                input.addCropRatio("fsc", fscScaleRatio);
                TRACE_S_FUNC(log, "set FSC crop data fscScaleRatio(%f) RRZOFSCRegion(%f,%f,%f,%f)", fscScaleRatio,
                    rrzR.p.x, rrzR.p.y,
                    rrzR.s.w, rrzR.s.h);
            }
            else
            {
                MY_LOGW("No FSC crop data!");
            }
            // set FSC warp data
            IMetadata::IEntry warpEntry;
            if( inHal )
            {
                warpEntry = inHal->entryFor(MTK_FSC_WARP_DATA);
            }
            if (warpEntry.count())
            {
                IMetadata::Memory metaMemory = warpEntry.itemAt(0, Type2Type< IMetadata::Memory >());
                FSC_WARPING_DATA_STRUCT *warpData = (FSC_WARPING_DATA_STRUCT*)metaMemory.array();
                featureParam.setVar<FSC_WARPING_DATA_STRUCT>(SFP_VAR::FSC_RRZO_WARP_DATA, *warpData);
                TRACE_S_FUNC(log, "set FSC warp data magic(%d) maxRRZOCropRegion(%d,%d,%d,%d)",
                    warpData->magicNum, warpData->maxRRZOCropRegion.p.x, warpData->maxRRZOCropRegion.p.y,
                    warpData->maxRRZOCropRegion.s.w, warpData->maxRRZOCropRegion.s.h);
            }
            featureParam.setFeatureMask(MASK_FSC, MTRUE);
        }
    }

    TRACE_S_FUNC_EXIT(log);
    return MTRUE;
}

MBOOL StreamingProcessor::isSubPixelEnabled() const
{
    return EFSC_SUBPIXEL_ENABLED(mPipeUsageHint.mFSCMode) ? MTRUE : MFALSE;
}

MVOID StreamingProcessor::drawCropRegion(const sp<Payload>& payload) const
{
#define DRAW_CROP_REGION_DISPLAY (0x1)
#define DRAW_CROP_REGION_RECORD  (0x2)
#define DRAW_CROP_REGION_ALL     (0xF)

    if (mDebugDrawCropMask != 0)
    {
        for(auto& partialPayload : payload->mPartialPayloads)
        {
            auto& reqPack = partialPayload->mRequestPack;
            auto& inputs = reqPack->mInputs;
            for(auto& it : reqPack->mOutputs)
            {
                for(auto& out : it.second)
                {
                    if ((mDebugDrawCropMask & DRAW_CROP_REGION_DISPLAY && out.isDisplay()) ||
                        (mDebugDrawCropMask & DRAW_CROP_REGION_RECORD && out.isRecord()) ||
                        (mDebugDrawCropMask == DRAW_CROP_REGION_ALL))
                    {
                        const auto& in = inputs[reqPack->mSensorInputMap[out.getSensorId()]];
                        if( isValid(in.mIMGI) )
                        {
                            MRectF dupCropRect = out.mCrop;
                            MINT32 fsc_offset = ::property_get_int32("vendor.debug.camera.drawcrop.fsc_offset", 0);
                            MFLOAT fscScaleRatio = 1.0f;
                            MUINT32 featureMask = in.mFeatureParam.mFeatureMask;

                            if (fsc_offset && HAS_FSC(featureMask))
                            {
                                IMetadata *inHal = in.mRequest->getMetaPtr(IN_P1_HAL);
                                IMetadata::IEntry cropEntry;
                                if( inHal )
                                {
                                    cropEntry = inHal->entryFor(MTK_FSC_CROP_DATA);
                                }
                                if (cropEntry.count())
                                {
                                    IMetadata::Memory metaMemory = cropEntry.itemAt(0, Type2Type< IMetadata::Memory >());
                                    FSC_CROPPING_DATA_STRUCT *cropData = (FSC_CROPPING_DATA_STRUCT*)metaMemory.array();
                                    fscScaleRatio = cropData->image_scale;
                                    dupCropRect.p.x -= dupCropRect.s.w*(1.0f-fscScaleRatio)/2/fscScaleRatio;
                                    dupCropRect.p.y -= dupCropRect.s.h*(1.0f-fscScaleRatio)/2/fscScaleRatio;
                                    dupCropRect.p.x = max(dupCropRect.p.x, 0.0f);
                                    dupCropRect.p.y = max(dupCropRect.p.y, 0.0f);
                                }
                            }

                            MY_LOGD("n(%d) out type(%d) xy offset (%f,%f,%f,%f)->(%.0f,%.0f,%.0f,%.0f) FSC(%d) EIS(%d) f(%f)",
                                payload->mLog.getLogFrameID(), mDebugDrawCropMask,
                                out.mCrop.p.x, out.mCrop.p.y, out.mCrop.s.w, out.mCrop.s.h,
                                dupCropRect.p.x, dupCropRect.p.y, dupCropRect.s.w, dupCropRect.s.h,
                                HAS_FSC(featureMask), HAS_EIS(featureMask), fscScaleRatio);

                            MINT32 lineWidth = ::property_get_int32("vendor.debug.camera.drawcrop.line_width", 20);
                            MINT32 value = ::property_get_int32("vendor.debug.camera.drawcrop.value", 255);
                            IImageBuffer *inputImg = in.mIMGI->getIImageBufferPtr();
                            drawRect(inputImg, dupCropRect.toMRect(), lineWidth, value);
                        }
                        else
                        {
                            MY_LOGW("not valid input!");
                        }
                    }
                }
            }
        }
    }
}

}; // namespace P2

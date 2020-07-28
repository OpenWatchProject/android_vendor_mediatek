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

#define LOG_TAG "mtkcam-P2NodeDecisionPolicy"

#include <mtkcam3/pipeline/policy/IP2NodeDecisionPolicy.h>
#include <mtkcam/utils/std/ULog.h>

//
#include "MyUtils.h"

CAM_ULOG_DECLARE_MODULE_ID(MOD_PIPELINE_POLICY);


/******************************************************************************
 *
 ******************************************************************************/
using namespace android;
using namespace NSCam;
using namespace NSCam::v3;
using namespace NSCam::v3::pipeline::policy;
using namespace NSCam::v3::pipeline::policy::p2nodedecision;

/******************************************************************************
 *
 ******************************************************************************/
#define MY_LOGV(fmt, arg...)        CAM_ULOGMV("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGD(fmt, arg...)        CAM_ULOGMD("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGI(fmt, arg...)        CAM_ULOGMI("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGW(fmt, arg...)        CAM_ULOGMW("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGE(fmt, arg...)        CAM_ULOGME("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGA(fmt, arg...)        CAM_LOGA("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGF(fmt, arg...)        CAM_LOGF("[%s] " fmt, __FUNCTION__, ##arg)
//
#define MY_LOGV_IF(cond, ...)       do { if (            (cond) ) { MY_LOGV(__VA_ARGS__); } }while(0)
#define MY_LOGD_IF(cond, ...)       do { if (            (cond) ) { MY_LOGD(__VA_ARGS__); } }while(0)
#define MY_LOGI_IF(cond, ...)       do { if (            (cond) ) { MY_LOGI(__VA_ARGS__); } }while(0)
#define MY_LOGW_IF(cond, ...)       do { if ( CC_UNLIKELY(cond) ) { MY_LOGW(__VA_ARGS__); } }while(0)
#define MY_LOGE_IF(cond, ...)       do { if ( CC_UNLIKELY(cond) ) { MY_LOGE(__VA_ARGS__); } }while(0)
#define MY_LOGA_IF(cond, ...)       do { if ( CC_UNLIKELY(cond) ) { MY_LOGA(__VA_ARGS__); } }while(0)
#define MY_LOGF_IF(cond, ...)       do { if ( CC_UNLIKELY(cond) ) { MY_LOGF(__VA_ARGS__); } }while(0)


#define MAX_IMAGE_SIZE(_target, _compete)      \
    do {                                       \
        if (_compete.size() > _target.size() ) \
            _target = _compete;                \
    } while (0)


/******************************************************************************
 *
 ******************************************************************************/
static
auto
getControlCaptureIntent(
    IMetadata const* pRequest_AppControl,
    ParsedMetaControl const* pRequest_ParsedAppMetaControl
) -> uint8_t
{
    static constexpr auto BAD_CAPTURE_INTENT = static_cast<uint8_t>(-1L);

    uint8_t captureIntent = BAD_CAPTURE_INTENT;

    if (CC_LIKELY( pRequest_ParsedAppMetaControl != nullptr )) {
        captureIntent = pRequest_ParsedAppMetaControl->control_captureIntent;

        //invalid parsed cache; use metadata
        if ( captureIntent == BAD_CAPTURE_INTENT ) {
            bool ret = IMetadata::getEntry<uint8_t>(
                            pRequest_AppControl,
                            MTK_CONTROL_CAPTURE_INTENT,
                            captureIntent);
            MY_LOGE_IF(!ret, "MTK_CONTROL_CAPTURE_INTENT doesn't exist");
        }
    }

    return captureIntent;
}


/******************************************************************************
 *
 ******************************************************************************/
static
auto
getEdgeMode(
    IMetadata const* pRequest_AppControl,
    ParsedMetaControl const* pRequest_ParsedAppMetaControl __unused
) -> uint8_t
{
    static constexpr auto BAD_VALUE = static_cast<uint8_t>(-1L);

    uint8_t edgeMode = BAD_VALUE;

    if (CC_LIKELY( pRequest_ParsedAppMetaControl != nullptr )) {
        edgeMode = pRequest_ParsedAppMetaControl->edge_mode;

        //invalid parsed cache; use metadata
        if ( edgeMode == BAD_VALUE ) {
            bool ret = IMetadata::getEntry<uint8_t>(
                            pRequest_AppControl,
                            MTK_EDGE_MODE,
                            edgeMode);
            MY_LOGE_IF(!ret, "MTK_EDGE_MODE doesn't exist");
        }
    }

    return edgeMode;
}


/******************************************************************************
 *
 ******************************************************************************/
static
auto
checkIsCaptureControl(
    IMetadata const* pRequest_AppControl,
    ParsedMetaControl const* pRequest_ParsedAppMetaControl
) -> bool
{
    bool ret =  MTK_CONTROL_CAPTURE_INTENT_STILL_CAPTURE
            == (getControlCaptureIntent(pRequest_AppControl, pRequest_ParsedAppMetaControl))
                ;
    return ret;
}


static
auto
checkIsVSSControl(
    IMetadata const* pRequest_AppControl,
    ParsedMetaControl const* pRequest_ParsedAppMetaControl
) -> bool
{
    bool ret =  MTK_CONTROL_CAPTURE_INTENT_VIDEO_SNAPSHOT
            == (getControlCaptureIntent(pRequest_AppControl, pRequest_ParsedAppMetaControl))
                ;
    return ret;
}


static
auto
isCaptureIntentManual(
    IMetadata const* pRequest_AppControl,
    ParsedMetaControl const* pRequest_ParsedAppMetaControl
) -> bool
{
    bool ret =  MTK_CONTROL_CAPTURE_INTENT_MANUAL
            == (getControlCaptureIntent(pRequest_AppControl, pRequest_ParsedAppMetaControl))
                ;
    return ret;
}


/******************************************************************************
 *
 ******************************************************************************/
static
auto
isCustomCapStream(
    IMetadata const* pRequest_AppControl __unused,
    android::sp<IImageStreamInfo> pImgInfo __unused
) -> bool
{
    return false;
}


/******************************************************************************
 *
 ******************************************************************************/
static
auto
isCustomPrvStream(
    IMetadata const* pRequest_AppControl,
    android::sp<IImageStreamInfo> pImgInfo __unused
) -> bool
{
    return false;
}


/******************************************************************************
 *
 ******************************************************************************/
static
auto
hasCustomStreamingOut(
    RequestInputParams const* in
) -> bool
{
    MINT32 cshot = 0;
    bool ret = IMetadata::getEntry<MINT32>(in->pRequest_AppControl, MTK_CSHOT_FEATURE_CAPTURE, cshot);
    if(ret && cshot)
    {
        return true;
    }
    return false;
}


/******************************************************************************
 *
 ******************************************************************************/
static
auto
checkOnlyCapture(
    RequestInputParams const* in
) -> bool
{
    auto const pRequest_AppImageStreamInfo = in->pRequest_AppImageStreamInfo;

    bool ret = false;
    if (in->hasP2StreamNode == false)
    {
        return true;
    }
    if (!(pRequest_AppImageStreamInfo->hasVideoConsumer) || !checkIsVSSControl(in->pRequest_AppControl, in->pRequest_ParsedAppMetaControl))
    {
        ret =
                pRequest_AppImageStreamInfo->pAppImage_Jpeg != nullptr
            ||  pRequest_AppImageStreamInfo->pAppImage_Input_Yuv != nullptr
            ||  pRequest_AppImageStreamInfo->pAppImage_Input_Priv != nullptr
            ||  checkIsCaptureControl(in->pRequest_AppControl, in->pRequest_ParsedAppMetaControl)
                ;

        if ( ! ret )
        {
            //[ITS test] captureIntent=manual + edgeMode=HIGH_QUALITY => P2CaptureNode
            ret =   isCaptureIntentManual(in->pRequest_AppControl, in->pRequest_ParsedAppMetaControl)
                && (MTK_EDGE_MODE_HIGH_QUALITY ==
                    getEdgeMode(in->pRequest_AppControl, in->pRequest_ParsedAppMetaControl))
                    ;
            MY_LOGD_IF(ret, "use P2C (captureIntent=MANUAL edge=HIGH_QUALITY)");
        }
    }
    return ret && !(hasCustomStreamingOut(in));
}


/******************************************************************************
 *
 ******************************************************************************/
static
auto
addStreamToPhysicalList(
    std::unordered_map<uint32_t, std::vector<StreamId_T>>& target,
    uint32_t sensorId,
    StreamId_T streamId
) -> void
{
    auto iter = target.find(sensorId);
    if(iter != target.end())
    {
        iter->second.push_back(streamId);
    }
    else
    {
        std::vector<StreamId_T> streamList;
        streamList.push_back(streamId);
        target.emplace(sensorId, streamList);
    }
}


/******************************************************************************
 *
 ******************************************************************************/
static
auto
handleFD(
    RequestOutputParams* out,
    RequestInputParams const* in
) -> void
{
    //  FD is enable only when:
    //  - A FD request is sent from users, and
    //  - P2StreamNode is enabled due to other output image streams.
    if ( in->isFdEnabled ) {
        out->vImageStreamId_from_StreamNode.push_back(in->pConfiguration_StreamInfo_NonP1->pHalImage_FD_YUV->getStreamId());
        out->needP2StreamNode = true;
    }
}


/******************************************************************************
 *
 ******************************************************************************/
static
auto
handleJpegPack(
    RequestOutputParams* out,
    RequestInputParams const* in
) -> void
{
    // [Jpeg packed]
    if(in->hasP2CaptureNode)
    {
        auto const pConfiguration_StreamInfo_NonP1 = in->pConfiguration_StreamInfo_NonP1;
        auto& imageIds_Capture = out->vImageStreamId_from_CaptureNode;
        //
        auto const& stream_Jpeg_Sub_YUV = pConfiguration_StreamInfo_NonP1->pHalImage_Jpeg_Sub_YUV;
        if(stream_Jpeg_Sub_YUV.get())
        {
            imageIds_Capture.push_back(stream_Jpeg_Sub_YUV->getStreamId());
        }
        auto const& stream_Depth_YUV = pConfiguration_StreamInfo_NonP1->pHalImage_Depth_YUV;
        if(stream_Depth_YUV.get())
        {
            imageIds_Capture.push_back(stream_Depth_YUV->getStreamId());
        }
    }
}


/******************************************************************************
 *
 ******************************************************************************/
static
auto
handleAllImageStream_ExceptFD(
    std::vector<StreamId_T>& vImageId,
    std::unordered_map<uint32_t, std::vector<StreamId_T>>& vImageId_Physical,
    MSize&                   maxSize,
    RequestInputParams const* in
) -> void
{
    auto const pConfiguration_StreamInfo_NonP1 = in->pConfiguration_StreamInfo_NonP1;
    auto const pRequest_AppImageStreamInfo = in->pRequest_AppImageStreamInfo;

    for (auto const& it : pRequest_AppImageStreamInfo->vAppImage_Output_Proc) {
        vImageId.push_back(it.first);
        MAX_IMAGE_SIZE(maxSize, it.second->getImgSize());
    }
    // physical
    for (auto const& it : pRequest_AppImageStreamInfo->vAppImage_Output_Proc_Physical) {
        for(auto const& info:it.second) {
            addStreamToPhysicalList(
                                vImageId_Physical,
                                it.first,
                                info->getStreamId());
            MAX_IMAGE_SIZE(maxSize, info->getImgSize());
        }
    }
    if (pRequest_AppImageStreamInfo->pAppImage_Jpeg != nullptr) {
        vImageId.push_back(pConfiguration_StreamInfo_NonP1->pHalImage_Jpeg_YUV->getStreamId());
        MAX_IMAGE_SIZE(maxSize, pConfiguration_StreamInfo_NonP1->pHalImage_Jpeg_YUV->getImgSize());
        if (in->needThumbnail)
        {
            vImageId.push_back(pConfiguration_StreamInfo_NonP1->pHalImage_Thumbnail_YUV->getStreamId());
        }
    }
}

/******************************************************************************
 *
 ******************************************************************************/
static
auto
checkIsOutputYuvFromP2c(
    RequestOutputParams* out,
    RequestInputParams const* in
) -> void
{
    auto checkIsYuvStream = [](
        auto const streamId,
        auto const* pConfiguration_StreamInfo_NonP1,
        auto const* pRequest_AppImageStreamInfo
    ) -> bool {
        if( (pConfiguration_StreamInfo_NonP1->pHalImage_Jpeg_YUV.get() && streamId == pConfiguration_StreamInfo_NonP1->pHalImage_Jpeg_YUV->getStreamId()) ||
            (pConfiguration_StreamInfo_NonP1->pHalImage_Thumbnail_YUV.get() && streamId == pConfiguration_StreamInfo_NonP1->pHalImage_Thumbnail_YUV->getStreamId())
            )
        {
            return true;
        }
        for(auto const& it : pRequest_AppImageStreamInfo->vAppImage_Output_Proc)
        {
            if(streamId==it.first)
                return true;
        }
        for(auto const& it : pRequest_AppImageStreamInfo->vAppImage_Output_Proc_Physical)
        {
            for(auto const& s : it.second)
            {
                if(streamId==s->getStreamId())
                    return true;
            }
        }
        return false;
    };
    //
    auto const* pConfiguration_StreamInfo_NonP1 = in->pConfiguration_StreamInfo_NonP1;
    auto const* pRequest_AppImageStreamInfo = in->pRequest_AppImageStreamInfo;
    auto const imageIds_Capture = out->vImageStreamId_from_CaptureNode;
    auto const imageIds_Capture_Physical = out->vImageStreamId_from_CaptureNode_Physical;
    //
    for (auto const& it : imageIds_Capture)
    {
        if(checkIsYuvStream(it, pConfiguration_StreamInfo_NonP1, pRequest_AppImageStreamInfo))
        {
            MY_LOGD("find output YUV stream (%#" PRIxPTR ") from P2C", it);
            out->needYuvOutputFromP2C = true;
            return;
        }
    }
    for (auto const& it : imageIds_Capture_Physical)
    {
        if(checkIsYuvStream(it.first, pConfiguration_StreamInfo_NonP1, pRequest_AppImageStreamInfo))
        {
            MY_LOGD("find output YUV stream (%#" PRIxPTR ") from P2C", it.first);
            out->needYuvOutputFromP2C = true;
            return;
        }
    }
    out->needYuvOutputFromP2C = false;
}

/******************************************************************************
 *
 ******************************************************************************/
static
auto
decideStreamOut(
    RequestOutputParams* out,
    RequestInputParams const* in
) -> void
{
    auto const pConfiguration_StreamInfo_NonP1 = in->pConfiguration_StreamInfo_NonP1;
    auto const pConfiguration_StreamInfo_P1    = in->pConfiguration_StreamInfo_P1;
    auto const pRequest_AppControl = in->pRequest_AppControl;
    auto const pRequest_AppImageStreamInfo = in->pRequest_AppImageStreamInfo;

    auto& needP2CaptureNode = out->needP2CaptureNode;
    auto& needP2StreamNode = out->needP2StreamNode;


    needP2CaptureNode = false;
    needP2StreamNode = false;
    {
        auto& imageIds_Capture = out->vImageStreamId_from_CaptureNode;
        auto& imageIds_Stream = out->vImageStreamId_from_StreamNode;
        auto& imageIds_Capture_Physical = out->vImageStreamId_from_CaptureNode_Physical;
        auto& imageIds_Stream_Physical = out->vImageStreamId_from_StreamNode_Physical;

        // logical
        for (auto const& it : pRequest_AppImageStreamInfo->vAppImage_Output_Proc)
        {
            if (isCustomCapStream(pRequest_AppControl, it.second))
            {
                needP2CaptureNode = true;
                imageIds_Capture.push_back(it.first);
                MAX_IMAGE_SIZE(out->maxP2CaptureSize, it.second->getImgSize());
            }
            else if (isCustomPrvStream(pRequest_AppControl, it.second))
            {
                needP2StreamNode = true;
                imageIds_Stream.push_back(it.first);
                MAX_IMAGE_SIZE(out->maxP2StreamSize, it.second->getImgSize());
            }
            else if ( ((pRequest_AppImageStreamInfo->pAppImage_Jpeg != nullptr)
                    || checkIsCaptureControl(pRequest_AppControl, in->pRequest_ParsedAppMetaControl)
                    || (it.second->getImgSize().size() > pConfiguration_StreamInfo_P1->pHalImage_P1_Rrzo->getImgSize().size()))
                    && !pRequest_AppImageStreamInfo->hasVideoConsumer )
            {
                needP2CaptureNode = true;
                imageIds_Capture.push_back(it.first);
                MAX_IMAGE_SIZE(out->maxP2CaptureSize, it.second->getImgSize());
            }
            else
            {
                needP2StreamNode = true;
                imageIds_Stream.push_back(it.first);
                MAX_IMAGE_SIZE(out->maxP2StreamSize, it.second->getImgSize());
            }
        }
        // physical
        for (auto const& it : pRequest_AppImageStreamInfo->vAppImage_Output_Proc_Physical)
        {
            for(auto const& info:it.second)
            {
                if (isCustomCapStream(pRequest_AppControl, info))
                {
                    needP2CaptureNode = true;
                    addStreamToPhysicalList(
                                        imageIds_Capture_Physical,
                                        it.first,
                                        info->getStreamId());
                    MAX_IMAGE_SIZE(out->maxP2CaptureSize, info->getImgSize());
                }
                else if (isCustomPrvStream(pRequest_AppControl, info))
                {
                    needP2StreamNode = true;
                    addStreamToPhysicalList(
                                        imageIds_Stream_Physical,
                                        it.first,
                                        info->getStreamId());
                    MAX_IMAGE_SIZE(out->maxP2StreamSize, info->getImgSize());
                }
                else if ( ((pRequest_AppImageStreamInfo->pAppImage_Jpeg != nullptr)
                        || checkIsCaptureControl(pRequest_AppControl, in->pRequest_ParsedAppMetaControl))
                        && !pRequest_AppImageStreamInfo->hasVideoConsumer )
                {
                    needP2CaptureNode = true;
                    addStreamToPhysicalList(
                                        imageIds_Capture_Physical,
                                        it.first,
                                        info->getStreamId());
                    MAX_IMAGE_SIZE(out->maxP2CaptureSize, info->getImgSize());
                }
                else
                {
                    needP2StreamNode = true;
                    addStreamToPhysicalList(
                                        imageIds_Stream_Physical,
                                        it.first,
                                        info->getStreamId());
                    MAX_IMAGE_SIZE(out->maxP2StreamSize, info->getImgSize());
                }
            }
        }
        if (pRequest_AppImageStreamInfo->pAppImage_Jpeg != nullptr)
        {
            StreamId_T streamId_Thumbnail_YUV = (in->needThumbnail) ? pConfiguration_StreamInfo_NonP1->pHalImage_Thumbnail_YUV->getStreamId() : (-1);

            auto const& stream_Jpeg_YUV = pConfiguration_StreamInfo_NonP1->pHalImage_Jpeg_YUV;
            if ((pRequest_AppImageStreamInfo->hasVideoConsumer || checkIsVSSControl(in->pRequest_AppControl, in->pRequest_ParsedAppMetaControl)) &&
                stream_Jpeg_YUV->getImgSize().size() <= pConfiguration_StreamInfo_P1->pHalImage_P1_Rrzo->getImgSize().size())
            {
                needP2StreamNode = true;
                imageIds_Stream.push_back(stream_Jpeg_YUV->getStreamId());
                MAX_IMAGE_SIZE(out->maxP2StreamSize, stream_Jpeg_YUV->getImgSize());
                if  ( streamId_Thumbnail_YUV >= 0 ) {
                    imageIds_Stream.push_back(streamId_Thumbnail_YUV);
                }
            }
            else
            {
                needP2CaptureNode = true;
                imageIds_Capture.push_back(stream_Jpeg_YUV->getStreamId());
                MAX_IMAGE_SIZE(out->maxP2CaptureSize, stream_Jpeg_YUV->getImgSize());
                if  ( streamId_Thumbnail_YUV >= 0 ) {
                    imageIds_Capture.push_back(streamId_Thumbnail_YUV);
                }
            }
        }
        handleJpegPack(out, in);
        handleFD(out, in);
    }

    // metadata
    {
        auto& metaIds_Capture = out->vMetaStreamId_from_CaptureNode;
        auto& metaIds_Stream = out->vMetaStreamId_from_StreamNode;
        auto& metaIds_Stream_Physical = out->vMetaStreamId_from_StreamNode_Physical;
        auto& metaIds_Capture_Physical = out->vMetaStreamId_from_CaptureNode_Physical;
        auto& imageIds_Capture_Physical = out->vImageStreamId_from_CaptureNode_Physical;
        auto& imageIds_Stream_Physical = out->vImageStreamId_from_StreamNode_Physical;

        if (needP2CaptureNode)
        {
            metaIds_Capture.push_back(pConfiguration_StreamInfo_NonP1->pHalMeta_DynamicP2CaptureNode->getStreamId());
            metaIds_Capture.push_back(pConfiguration_StreamInfo_NonP1->pAppMeta_DynamicP2CaptureNode->getStreamId());
            // according to physical image stream id list to decide needing this physical metadata or not.
            for(auto&& item:imageIds_Capture_Physical)
            {
                auto iter = pConfiguration_StreamInfo_NonP1->vAppMeta_DynamicP2CaptureNode_Physical.find(
                            item.first);
                if(iter != pConfiguration_StreamInfo_NonP1->vAppMeta_DynamicP2CaptureNode_Physical.end())
                {
                    std::vector<StreamId_T> temp;
                    temp.push_back(iter->second->getStreamId());
                    metaIds_Capture_Physical.emplace(iter->first, temp);
                }
            }
        }
        if (needP2StreamNode)
        {
            metaIds_Stream.push_back(pConfiguration_StreamInfo_NonP1->pHalMeta_DynamicP2StreamNode->getStreamId());
            if (!needP2CaptureNode)
            {
                metaIds_Stream.push_back(pConfiguration_StreamInfo_NonP1->pAppMeta_DynamicP2StreamNode->getStreamId());
                // according to physical image stream id list to decide needing this physical metadata or not.
                for(auto&& item:imageIds_Stream_Physical)
                {
                    auto iter = pConfiguration_StreamInfo_NonP1->vAppMeta_DynamicP2StreamNode_Physical.find(
                                item.first);
                    if(iter != pConfiguration_StreamInfo_NonP1->vAppMeta_DynamicP2StreamNode_Physical.end())
                    {
                        std::vector<StreamId_T> temp;
                        temp.push_back(iter->second->getStreamId());
                        metaIds_Stream_Physical.emplace(iter->first, temp);
                    }
                }
            }
        }
    }
}


/******************************************************************************
 *
 ******************************************************************************/
static
auto
evaluateRequest(
    RequestOutputParams& out,
    RequestInputParams const& in
) -> int
{
    auto const pConfiguration_StreamInfo_NonP1 = in.pConfiguration_StreamInfo_NonP1;
    auto const pRequest_AppImageStreamInfo = in.pRequest_AppImageStreamInfo;

    if (!(in.hasP2CaptureNode) && !(in.hasP2StreamNode))
    {
        out.needP2CaptureNode = false;
        out.needP2StreamNode = false;
        MY_LOGD("didn't have p2 node.....");
        return OK;
    }
    if (!(in.hasP2CaptureNode))
    {
        MY_LOGD("Only use P2S node");

        handleAllImageStream_ExceptFD(
                    out.vImageStreamId_from_StreamNode,
                    out.vImageStreamId_from_StreamNode_Physical,
                    out.maxP2StreamSize,
                    &in);

        out.needP2CaptureNode = false;
        out.needP2StreamNode = !out.vImageStreamId_from_StreamNode.empty();
        handleFD(&out, &in);

        if  ( out.needP2StreamNode )
        {
            auto& metaIds_Stream_Physical = out.vMetaStreamId_from_StreamNode_Physical;
            out.vMetaStreamId_from_StreamNode.push_back(pConfiguration_StreamInfo_NonP1->pHalMeta_DynamicP2StreamNode->getStreamId());
            out.vMetaStreamId_from_StreamNode.push_back(pConfiguration_StreamInfo_NonP1->pAppMeta_DynamicP2StreamNode->getStreamId());
            // according to physical image stream id list to decide needing this physical metadata or not.
            for(auto&& item:out.vImageStreamId_from_StreamNode_Physical)
            {
                auto iter = pConfiguration_StreamInfo_NonP1->vAppMeta_DynamicP2StreamNode_Physical.find(
                            item.first);
                if(iter != pConfiguration_StreamInfo_NonP1->vAppMeta_DynamicP2StreamNode_Physical.end())
                {
                    std::vector<StreamId_T> temp;
                    temp.push_back(iter->second->getStreamId());
                    metaIds_Stream_Physical.emplace(iter->first, temp);
                }
            }
        }
        checkIsOutputYuvFromP2c(&out, &in);
        return OK;
    }
    if ( checkOnlyCapture(&in) )
    {
        MY_LOGD("Only use P2C node");
        handleAllImageStream_ExceptFD(
                    out.vImageStreamId_from_CaptureNode,
                    out.vImageStreamId_from_CaptureNode_Physical,
                    out.maxP2StreamSize,
                    &in);

        out.needP2CaptureNode =
                    !(out.vImageStreamId_from_CaptureNode.empty()&&
                      out.vImageStreamId_from_CaptureNode_Physical.empty());
        out.needP2StreamNode = false;
        handleJpegPack(&out, &in);
        handleFD(&out, &in);

        if  ( out.needP2CaptureNode )
        {
            auto& metaIds_Capture_Physical = out.vMetaStreamId_from_CaptureNode_Physical;
            out.vMetaStreamId_from_CaptureNode.push_back(pConfiguration_StreamInfo_NonP1->pHalMeta_DynamicP2CaptureNode->getStreamId());
            out.vMetaStreamId_from_CaptureNode.push_back(pConfiguration_StreamInfo_NonP1->pAppMeta_DynamicP2CaptureNode->getStreamId());
            // according to physical image stream id list to decide needing this physical metadata or not.
            for(auto&& item:out.vImageStreamId_from_CaptureNode_Physical)
            {
                auto iter = pConfiguration_StreamInfo_NonP1->vAppMeta_DynamicP2CaptureNode_Physical.find(
                            item.first);
                if(iter != pConfiguration_StreamInfo_NonP1->vAppMeta_DynamicP2CaptureNode_Physical.end())
                {
                    std::vector<StreamId_T> temp;
                    temp.push_back(iter->second->getStreamId());
                    metaIds_Capture_Physical.emplace(iter->first, temp);
                }
            }
        }
        if  ( out.needP2StreamNode )
        {
            auto& metaIds_Stream_Physical = out.vMetaStreamId_from_StreamNode_Physical;
            out.vMetaStreamId_from_StreamNode.push_back(pConfiguration_StreamInfo_NonP1->pHalMeta_DynamicP2StreamNode->getStreamId());
            // according to physical image stream id list to decide needing this physical metadata or not.
            for(auto&& item:out.vImageStreamId_from_StreamNode_Physical)
            {
                auto iter = pConfiguration_StreamInfo_NonP1->vAppMeta_DynamicP2StreamNode_Physical.find(
                            item.first);
                if(iter != pConfiguration_StreamInfo_NonP1->vAppMeta_DynamicP2StreamNode_Physical.end())
                {
                    std::vector<StreamId_T> temp;
                    temp.push_back(iter->second->getStreamId());
                    metaIds_Stream_Physical.emplace(iter->first, temp);
                }
            }
        }
        checkIsOutputYuvFromP2c(&out, &in);
        return OK;
    }
    decideStreamOut(&out, &in);
    checkIsOutputYuvFromP2c(&out, &in);
    return OK;
}


/******************************************************************************
 *
 ******************************************************************************/
namespace NSCam {
namespace v3 {
namespace pipeline {
namespace policy {


/**
 * Make a function target as a policy - default version
 */
FunctionType_P2NodeDecisionPolicy makePolicy_P2NodeDecision_Default()
{
    return evaluateRequest;
}


};  //namespace policy
};  //namespace pipeline
};  //namespace v3
};  //namespace NSCam


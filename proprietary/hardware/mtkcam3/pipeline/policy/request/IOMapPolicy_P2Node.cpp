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

#define LOG_TAG "mtkcam-P2NodeIOMapPolicy"

#include <mtkcam3/pipeline/policy/IIOMapPolicy.h>
//
#include <mtkcam3/pipeline/hwnode/NodeId.h>
#include <mtkcam3/pipeline/hwnode/StreamId.h>
#include <mtkcam/utils/std/ULog.h>
//
#include "MyUtils.h"

CAM_ULOG_DECLARE_MODULE_ID(MOD_PIPELINE_POLICY);


/******************************************************************************
 *
 ******************************************************************************/
using namespace android;
using namespace NSCam::v3::pipeline;
using namespace NSCam::v3::pipeline::NSPipelineContext;
using namespace NSCam::v3::pipeline::policy::iomap;


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


/******************************************************************************
 *
 ******************************************************************************/
namespace NSCam {
namespace v3 {
namespace pipeline {
namespace policy {


/******************************************************************************
 *
 ******************************************************************************/
static
bool
getIsNeedImgo(StreamId_T streamid, RequestInputParams const& in, MSize rrzoSize, MSize imgoSize)
{
    if (rrzoSize.w == imgoSize.w && rrzoSize.h == imgoSize.h)
    {
        return false;
    }
    #define IS_IMGO(size, threshold) (size.w > threshold.w || size.h > threshold.h)
    for( const auto& n : in.pRequest_AppImageStreamInfo->vAppImage_Output_Proc ) {
        if (streamid == n.first)
        {
            return IS_IMGO(n.second->getImgSize(), rrzoSize);
        }
    }
    #undef IS_IMGO

    /*if ( streamid == in.pConfiguration_StreamInfo_NonP1->pHalImage_Jpeg_YUV->getStreamId() )
        return true;

    if ( streamid == in.pConfiguration_StreamInfo_NonP1->pHalImage_Thumbnail_YUV->getStreamId() )
        return true;

    if ( streamid == in.pConfiguration_StreamInfo_NonP1->pHalImage_FD_YUV->getStreamId() )
        return false;

    MY_LOGW("Cannot find streamId : %lld and should not be here, check input parameter please", streamid);*/
    return false;
}


/******************************************************************************
 *
 ******************************************************************************/
static
MERROR
evaluateRequest_P2StreamNode(
    RequestOutputParams& out,
    RequestInputParams const& in
)
{
    if ( !in.pRequest_PipelineNodesNeed->needP2StreamNode || !in.isMainFrame )
    {
        //MY_LOGD("No need P2StreamNode");
        return OK;
    }

    auto const& needP1Node = in.pRequest_PipelineNodesNeed->needP1Node;
    auto const& needP1Dma = *in.pRequest_NeedP1Dma;

    IOMap imgoMap, rrzoMap, metaMap, submetaMap;
    sp<IImageStreamInfo> main1RrzoInfo;
    sp<IImageStreamInfo> main1ImgoInfo;
    bool bMainHasImgo = false, bMainHasRrzo = false, bMain1Skip = false;
    for (size_t i = 0; i < needP1Node.size(); i++)
    {
        const MBOOL isMain = (i == 0);
        if ( ! needP1Node[i] ) {
            bMain1Skip = true;
            continue;
        }

        auto& p1Info = (*in.pConfiguration_StreamInfo_P1)[i];
        const auto need_P1Dma = needP1Dma[i];

        const MBOOL isMaster = (!isMain && bMain1Skip);

        if (isMain || isMaster)
        {
            main1ImgoInfo = in.pRequest_AppImageStreamInfo->pAppImage_Output_Priv.get()
                                                ? in.pRequest_AppImageStreamInfo->pAppImage_Output_Priv
                                                : p1Info.pHalImage_P1_Imgo;

            main1RrzoInfo = p1Info.pHalImage_P1_Rrzo;

            if (need_P1Dma & P1_IMGO) {
                imgoMap.addIn(main1ImgoInfo->getStreamId());
                bMainHasImgo = true;
            }

            if (need_P1Dma & P1_RRZO) {
                rrzoMap.addIn(main1RrzoInfo->getStreamId());
                bMainHasRrzo = true;
            }
        }
        else
        {
            sp<IImageStreamInfo> subRrzoInfo = p1Info.pHalImage_P1_Rrzo;
            sp<IImageStreamInfo> subImgoInfo = p1Info.pHalImage_P1_Imgo;
            if (subRrzoInfo != nullptr)
            {
                if ((need_P1Dma & P1_RRZO) && bMainHasImgo) {
                    imgoMap.addIn(subRrzoInfo->getStreamId());
                }
                if ((need_P1Dma & P1_RRZO) && bMainHasRrzo) {
                    rrzoMap.addIn(subRrzoInfo->getStreamId());
                }
            }
            else if (subImgoInfo != nullptr)
            {
                if ((need_P1Dma & P1_IMGO) && bMainHasImgo) {
                    imgoMap.addIn(subImgoInfo->getStreamId());
                }
                if ((need_P1Dma & P1_IMGO) && bMainHasRrzo) {
                    rrzoMap.addIn(subImgoInfo->getStreamId());
                }
            }
        }

        if (need_P1Dma & P1_LCSO) {
            imgoMap.addIn(p1Info.pHalImage_P1_Lcso->getStreamId());
            rrzoMap.addIn(p1Info.pHalImage_P1_Lcso->getStreamId());
        }

        if (need_P1Dma & P1_RSSO) {
            imgoMap.addIn(p1Info.pHalImage_P1_Rsso->getStreamId());
            rrzoMap.addIn(p1Info.pHalImage_P1_Rsso->getStreamId());
        }

        if (need_P1Dma & P1_SCALEDYUV) {
            if( p1Info.pHalImage_P1_ScaledYuv != nullptr )
            {
                if(imgoMap.sizeIn() > 0)
                    imgoMap.addIn(p1Info.pHalImage_P1_ScaledYuv->getStreamId());
                if(rrzoMap.sizeIn() > 0)
                    rrzoMap.addIn(p1Info.pHalImage_P1_ScaledYuv->getStreamId());
            }
        }

        metaMap.addIn(p1Info.pAppMeta_DynamicP1->getStreamId());
        metaMap.addIn(p1Info.pHalMeta_DynamicP1->getStreamId());
        submetaMap.addIn(p1Info.pAppMeta_DynamicP1->getStreamId());
        submetaMap.addIn(p1Info.pHalMeta_DynamicP1->getStreamId());
    }

    if (!(bMainHasImgo || bMainHasRrzo))
    {
        MY_LOGE("No Imgo or Rrzo");
        return OK;
    }

    auto const vImageStreamId = *in.pvImageStreamId_from_StreamNode;
    if (!bMainHasImgo)
    {
        for (size_t i = 0; i < vImageStreamId.size(); i++)
        {
            rrzoMap.addOut(vImageStreamId[i]);
        }
    }
    else if (!bMainHasRrzo)
    {
        for (size_t i = 0; i < vImageStreamId.size(); i++)
        {
            imgoMap.addOut(vImageStreamId[i]);
        }
    }
    else
    {
        for (size_t i = 0; i < vImageStreamId.size(); i++)
        {
            if ( getIsNeedImgo(vImageStreamId[i], in, main1RrzoInfo->getImgSize(), main1ImgoInfo->getImgSize()) )
            {
                imgoMap.addOut(vImageStreamId[i]);
            }
            else
            {
                rrzoMap.addOut(vImageStreamId[i]);
            }
        }
    }

    metaMap.addIn(in.pConfiguration_StreamInfo_NonP1->pAppMeta_Control->getStreamId());
    submetaMap.addIn(in.pConfiguration_StreamInfo_NonP1->pAppMeta_Control->getStreamId());
    for( const auto n : (*in.pvMetaStreamId_from_StreamNode) )
        metaMap.addOut(n);

    IOMapSet P2ImgIO, P2MetaIO;
    if (imgoMap.sizeOut() > 0)
    {
        P2ImgIO.add(imgoMap);
        out.bP2UseImgo = true;
    }
    if (rrzoMap.sizeOut() > 0) P2ImgIO.add(rrzoMap);

    P2MetaIO.add(metaMap);
    if (P2ImgIO.size() > 1)
    {
        P2MetaIO.add(submetaMap);
    }

    (*out.pNodeIOMapImage)[eNODEID_P2StreamNode] = P2ImgIO;
    (*out.pNodeIOMapMeta) [eNODEID_P2StreamNode] = P2MetaIO;

    return OK;
}


/******************************************************************************
 *
 ******************************************************************************/
static
MERROR
evaluateRequest_P2CaptureNode(
    RequestOutputParams& out,
    RequestInputParams const& in
)
{
    auto addIn = [](IOMap& iomap, auto const& pStream) {
        if ( pStream != nullptr ) {
            iomap.addIn(pStream->getStreamId());
            return true;
        }
        return false;
    };

    auto pRequest_AppImageStreamInfo = in.pRequest_AppImageStreamInfo;
    auto pRequest_PipelineNodesNeed = in.pRequest_PipelineNodesNeed;
    auto const& needP1Node = in.pRequest_PipelineNodesNeed->needP1Node;
    auto const& needP1Dma = *in.pRequest_NeedP1Dma;

    if ( !pRequest_PipelineNodesNeed->needP2CaptureNode )
    {
        //MY_LOGD("No need P2CaptureNode");
        return OK;
    }

    IOMap imageMap, metaMap;
    bool bImgo = false;
    for (size_t i = 0; i < needP1Node.size(); i++)
    {
        bool isSkip =  ( ! needP1Node[i] )
                    || CC_UNLIKELY( i >= needP1Dma.size() )
                    || CC_UNLIKELY( i >= (*in.pConfiguration_StreamInfo_P1).size() )
                        ;
        if ( isSkip ) {
            continue;
        }

        auto const need_P1Dma = needP1Dma[i];
        auto const& configP1Streams = (*in.pConfiguration_StreamInfo_P1)[i];

        switch (i)
        {
        case 0:{
            // IMGO
            {
                IImageStreamInfo* pImgo = nullptr;
                IImageStreamInfo* pImgoOutput = nullptr;
                bool hasAdded = false;
                if ( in.isMainFrame )
                {
                    ////////////////////////////////////////////////////////////
                    // Input App stream
                    if ( auto p = pRequest_AppImageStreamInfo->pAppImage_Input_Yuv.get() ) {
                        pImgo = p;
                    }
                    else
                    if ( auto p = pRequest_AppImageStreamInfo->pAppImage_Input_Priv.get() ) {
                        pImgo = p;
                    }
                    else
                    if ( auto p = pRequest_AppImageStreamInfo->pAppImage_Input_RAW16.get() ) {
                        pImgo = p;
                    }
                    ////////////////////////////////////////////////////////////
                    // Output App stream
                    else
                    {
                        //set APP raw as p2capture node output raw
                        if(true == in.isAppRawOutFromP2C)
                        {
                            // Priv Raw
                            if(pRequest_AppImageStreamInfo->pAppImage_Output_Priv.get())
                            {
                                // P2Node directly outputs App private stream.
                                pImgoOutput = pRequest_AppImageStreamInfo->pAppImage_Output_Priv.get();
                            }
                            // RAW16
                            else if(false == pRequest_PipelineNodesNeed->needRaw16Node
                                    && pRequest_AppImageStreamInfo->vAppImage_Output_RAW16.size() != 0)
                            {
                                // P2Node directly outputs App RAW16 stream: (if no Raw16Node).
                                auto it = pRequest_AppImageStreamInfo->vAppImage_Output_RAW16.find(in.pPipelineStaticInfo->sensorId[i]);
                                if (it != pRequest_AppImageStreamInfo->vAppImage_Output_RAW16.end())
                                {
                                    pImgoOutput = it->second.get();
                                }
                            }
                        }
                        else
                        {
                            // Priv Raw
                            if ( auto p = pRequest_AppImageStreamInfo->pAppImage_Output_Priv.get() )
                            {   // P1Node directly outputs App private stream.
                                pImgo = p;
                            }
                            // RAW16
                            else if(false == pRequest_PipelineNodesNeed->needRaw16Node
                                    && pRequest_AppImageStreamInfo->vAppImage_Output_RAW16.size() != 0)
                            {
                                // P2Node directly outputs App RAW16 stream: (if no Raw16Node).
                                auto it = pRequest_AppImageStreamInfo->vAppImage_Output_RAW16.find(in.pPipelineStaticInfo->sensorId[i]);
                                if (it != pRequest_AppImageStreamInfo->vAppImage_Output_RAW16.end())
                                {
                                    pImgo = it->second.get();
                                }
                            }
                        }
                    }
                }

                ////////////////////////////////////////////////////////////////
                // Output Hal stream (IMGO)
                if ( pImgo == nullptr ) {
                    if ( (need_P1Dma & P1_IMGO) ) {
                        pImgo = configP1Streams.pHalImage_P1_Imgo.get();
                    }
                }

                ////////////////////////////////////////////////////////////////
                bImgo = addIn(imageMap, pImgo);

                ////////////////////////////////////////////////////////////////
                if ( !hasAdded && pImgoOutput != nullptr ) {
                    imageMap.addOut(pImgoOutput->getStreamId());
                }
            }
            }break;

        default:
        case 1:/* P1 Main2 */{
            if (need_P1Dma & P1_IMGO) {
                // Why do we have Hal P1 imgo stream in the cases of App input streams?
                bImgo = addIn(imageMap, configP1Streams.pHalImage_P1_Imgo);
            }
            }break;
        }

        if (need_P1Dma & P1_RRZO) {
            addIn(imageMap, configP1Streams.pHalImage_P1_Rrzo);
        }
        if (need_P1Dma & P1_LCSO) {
            addIn(imageMap, configP1Streams.pHalImage_P1_Lcso);
        }

        addIn(metaMap, configP1Streams.pAppMeta_DynamicP1);
        addIn(metaMap, configP1Streams.pHalMeta_DynamicP1);
    }

    if (!bImgo)
    {
        MY_LOGE("No Imgo");
        return OK;
    }
    out.bP2UseImgo = true;

    if (in.isMainFrame)
    {
        for( const auto n : (*in.pvImageStreamId_from_CaptureNode) ) {
            imageMap.addOut(n);
        }
    }

    addIn(metaMap, in.pConfiguration_StreamInfo_NonP1->pAppMeta_Control);
    for( const auto n : (*in.pvMetaStreamId_from_CaptureNode) ) {
        metaMap.addOut(n);
    }

    (*out.pNodeIOMapImage)[eNODEID_P2CaptureNode] = IOMapSet().add(imageMap);
    (*out.pNodeIOMapMeta) [eNODEID_P2CaptureNode] = IOMapSet().add(metaMap);

    return OK;
}


/******************************************************************************
 * Make a function target as a policy - default version
 ******************************************************************************/
FunctionType_IOMapPolicy_P2Node makePolicy_IOMap_P2Node_Default()
{
    return [](
        RequestOutputParams& out,
        RequestInputParams const& in
    ) -> int
    {
        evaluateRequest_P2StreamNode(out, in);
        evaluateRequest_P2CaptureNode(out, in);
        return OK;
    };
}


};  //namespace policy
};  //namespace pipeline
};  //namespace v3
};  //namespace NSCam

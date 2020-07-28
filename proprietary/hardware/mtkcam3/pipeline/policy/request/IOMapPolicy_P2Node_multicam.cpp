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

#define LOG_TAG "mtkcam-P2NodeIOMapPolicyMC"

#include <mtkcam3/pipeline/policy/IIOMapPolicy.h>
//
#include <mtkcam3/pipeline/hwnode/NodeId.h>
#include <mtkcam3/pipeline/hwnode/StreamId.h>

#include <bitset>
#include <map>
#include <unordered_map>
#include <utility>
#include <mtkcam/utils/std/ULog.h>
//
#include "MyUtils.h"

CAM_ULOG_DECLARE_MODULE_ID(MOD_PIPELINE_POLICY);

#define CAP_IOMAP_COMBINE false
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

/*static
bool
dumpIOMap(IOMap &iomap)
{
    android::String8 val("");
    for(size_t i=0;i<iomap.sizeIn();i++)
    {
        val.appendFormat("in(%" PRIx64 ")", iomap.vIn[i]);
    }
    for(size_t i=0;i<iomap.sizeOut();i++)
    {
        val.appendFormat("out(%" PRIx64 ")", iomap.vOut[i]);
    }
    MY_LOGD("%s", val.string());
    return true;
}*/
/******************************************************************************
 *
 ******************************************************************************/
static
bool
getIsNeedImgo(StreamId_T streamid, RequestInputParams const& in, MSize rrzoSize)
{
    #define IS_IMGO(size, threshold) (size.w > threshold.w || size.h > threshold.h)
    // check logical
    for( const auto& n : in.pRequest_AppImageStreamInfo->vAppImage_Output_Proc ) {
        if (streamid == n.first)
        {
            return IS_IMGO(n.second->getImgSize(), rrzoSize);
        }
    }
    // check physical
    for( const auto& n : in.pRequest_AppImageStreamInfo->vAppImage_Output_Proc_Physical ) {
        for(const auto& m :n.second) {
            if (streamid == m->getStreamId()) {
                return IS_IMGO(m->getImgSize(), rrzoSize);
            }
        }
    }
    #undef IS_IMGO
    // for reprocessing flow, it has to use imgo.
    if ((in.pRequest_AppImageStreamInfo->pAppImage_Output_Priv.get()) ||
        (in.pRequest_AppImageStreamInfo->pAppImage_Input_Priv.get()) ||
        (in.pRequest_AppImageStreamInfo->pAppImage_Input_Yuv.get()))
    {
        return true;
    }
    return false;
}

/******************************************************************************
 *
 ******************************************************************************/
/*static
bool
isPhysicalStream(StreamId_T streamid, RequestInputParams const& in, bool isMain)
{
    //
    if (in.pPipelineStaticInfo->sensorId.size() <= 1) {
        MY_LOGE("Sensor size <=1, no need to check  physical stream.");
        return false;
    }
    //
    for( const auto& n : in.pRequest_AppImageStreamInfo->vAppImage_Output_Proc ) {
        if (streamid == n.first)
        {
            MINT sensorId = n.second->getPhysicalCameraId();
            // 0-length string is not a physical output stream
            if (sensorId < 0) {
                return false;
            }
            //
            if (sensorId == in.pPipelineStaticInfo->sensorId[isMain? 0 : 1]) {
                return true;
            }
        }
    }
    return false;
}*/


/******************************************************************************
 *
 ******************************************************************************/
static
bool
DataCheck(
    RequestInputParams const& in
)
{
    bool ret = true;
    if(in.pPipelineStaticInfo == nullptr) {
        MY_LOGE("pPipelineStaticInfo is nullptr");
        ret &= false;
        goto lbExit;
    }
    if(in.pPipelineStaticInfo->sensorId.size() <
       in.pRequest_PipelineNodesNeed->needP1Node.size()) {
        MY_LOGE("mismatch: sensorId(%zu) p1node(%zu)",
                in.pPipelineStaticInfo->sensorId.size(),
                in.pRequest_PipelineNodesNeed->needP1Node.size());
        ret &= false;
    }
lbExit:
    return ret;
}

/******************************************************************************
 *
 ******************************************************************************/
static
MERROR
evaluateRequest_P2StreamNode_multicam(
    RequestOutputParams& out,
    RequestInputParams const& in
)
{
    if(!DataCheck(in)) {
        return OK;
    }

    if((in.pvImageStreamId_from_StreamNode_Physical == nullptr)||
       (in.pvMetaStreamId_from_StreamNode_Physical == nullptr)||
       (in.pvImageStreamId_from_StreamNode == nullptr)||
       (in.pvMetaStreamId_from_StreamNode == nullptr)) {
        MY_LOGE("pvImageStreamId_from_StreamNode_Physical(%p)"
                "pvMetaStreamId_from_StreamNode_Physical(%p)"
                "pvImageStreamId_from_StreamNode(%p)"
                "pvMetaStreamId_from_StreamNode(%p)",
                in.pvImageStreamId_from_StreamNode_Physical,
                in.pvMetaStreamId_from_StreamNode_Physical,
                in.pvImageStreamId_from_StreamNode,
                in.pvMetaStreamId_from_StreamNode);
        return OK;
    }

    if ( !in.pRequest_PipelineNodesNeed->needP2StreamNode || !in.isMainFrame ) {
        MY_LOGD("No need P2StreamNode");
        return OK;
    }

    // define local struct.
    struct IOMapGroup {
        IOMap imgoMap;
        IOMap rrzoMap;
        IOMap metaMap;
        IOMap subMetaMap;
        MSize rrzoSize;
    };
    //
    auto sensorIdToIndex = [&in](MINT32 sensorId) {
        for(size_t i=0;i<in.pPipelineStaticInfo->sensorId.size();i++) {
            if(in.pPipelineStaticInfo->sensorId[i] == sensorId) return (MINT32)i;
        }
        return -1;
    };
    //
    auto addAppStreamIdToMap = [&out](
                                    uint32_t id,
                                    StreamId_T streamid) {
        if(out.pvMetaStreamId_All_Physical != nullptr) {
            auto iter_phy = out.pvMetaStreamId_All_Physical->find(id);
            if(iter_phy != out.pvMetaStreamId_All_Physical->end()) {
                iter_phy->second.push_back(streamid);
            }
            else {
                std::vector<StreamId_T> temp;
                temp.push_back(streamid);
                out.pvMetaStreamId_All_Physical->insert({
                                                id,
                                                std::move(temp)});
            }
        }
        return;
    };
    //
    IOMapGroup logicalIOMap;
    // <sensor id, IOMapGroup>
    std::unordered_map<uint32_t, IOMapGroup> physicalIOMap;

    auto const& needP1Node = in.pRequest_PipelineNodesNeed->needP1Node;
    auto const& needP1Dma = *in.pRequest_NeedP1Dma;
    auto const& physicalOut_img = *in.pvImageStreamId_from_StreamNode_Physical;
    auto const& physicalOut_meta = *in.pvMetaStreamId_from_StreamNode_Physical;
    auto const& logicalOut_img = *in.pvImageStreamId_from_StreamNode;
    auto const& logicalOut_meta = *in.pvMetaStreamId_from_StreamNode;

    bool hasImgo = false;
    bool hasRrzo = false;

    // using needP1Node data to decide physical p1node input iomap.
    for(size_t i=0;i<needP1Node.size();i++){
        if ( ! needP1Node[i] ) {
            continue;
        }

        auto& p1Info = (*in.pConfiguration_StreamInfo_P1)[i];
        const auto need_P1Dma = needP1Dma[i];

        IOMapGroup iomap;
        sp<IImageStreamInfo> rrzoInfo = p1Info.pHalImage_P1_Rrzo;
        sp<IImageStreamInfo> imgoInfo = p1Info.pHalImage_P1_Imgo;

        if(need_P1Dma & P1_IMGO) {
            iomap.imgoMap.addIn(imgoInfo->getStreamId());
            hasImgo = true;

            if(need_P1Dma & P1_LCSO) {
                iomap.imgoMap.addIn(p1Info.pHalImage_P1_Lcso->getStreamId());
            }

            if(need_P1Dma & P1_RSSO) {
                iomap.imgoMap.addIn(p1Info.pHalImage_P1_Rsso->getStreamId());
            }
        }

        if(need_P1Dma & P1_RRZO) {
            iomap.rrzoMap.addIn(rrzoInfo->getStreamId());
            iomap.rrzoSize = rrzoInfo->getImgSize();
            hasRrzo = true;

            if(need_P1Dma & P1_LCSO) {
                iomap.rrzoMap.addIn(p1Info.pHalImage_P1_Lcso->getStreamId());
            }

            if(need_P1Dma & P1_RSSO) {
                iomap.rrzoMap.addIn(p1Info.pHalImage_P1_Rsso->getStreamId());
            }
        }

        if (need_P1Dma & P1_SCALEDYUV) {
            if( p1Info.pHalImage_P1_ScaledYuv != nullptr )
            {
                if(iomap.imgoMap.sizeIn() > 0)
                    iomap.imgoMap.addIn(p1Info.pHalImage_P1_ScaledYuv->getStreamId());
                if(iomap.rrzoMap.sizeIn() > 0)
                    iomap.rrzoMap.addIn(p1Info.pHalImage_P1_ScaledYuv->getStreamId());
            }
        }

        iomap.metaMap.addIn(p1Info.pAppMeta_DynamicP1->getStreamId());
        iomap.metaMap.addIn(p1Info.pHalMeta_DynamicP1->getStreamId());
        iomap.metaMap.addIn(in.pConfiguration_StreamInfo_NonP1->pAppMeta_Control->getStreamId());
        iomap.subMetaMap.addIn(p1Info.pAppMeta_DynamicP1->getStreamId());
        iomap.subMetaMap.addIn(p1Info.pHalMeta_DynamicP1->getStreamId());
        iomap.subMetaMap.addIn(in.pConfiguration_StreamInfo_NonP1->pAppMeta_Control->getStreamId());
        // set to map
        physicalIOMap.emplace(
                            in.pPipelineStaticInfo->sensorId[i],
                            iomap);
        // check needs output physical stream or not.
        // If needs output physical stream, add P1 app metadata to pAppMeta_DynamicP1.
        for(auto&& item:physicalOut_img) {
            // item.first: unique sensor id.(no duplicate sensor id)
            if(in.pPipelineStaticInfo->sensorId[i] == item.first) {
                addAppStreamIdToMap(item.first, p1Info.pAppMeta_DynamicP1->getStreamId());
            }
        }
    }

    // check any imgo/rrzo are set or not.
    if (!(hasImgo || hasRrzo)) {
        MY_LOGE("No Imgo or Rrzo");
        return OK;
    }

    // build physical image map
    for(auto&& outputList:physicalOut_img) {
        // image stream
        auto phy_iomap = physicalIOMap.find(outputList.first);
        if(phy_iomap == physicalIOMap.end()) {
            MY_LOGE("need output physical stream, but cannot find id in physicalIOMap");
            return OK;
        }
        size_t imgo_size = phy_iomap->second.imgoMap.sizeIn();
        size_t rrzo_size = phy_iomap->second.rrzoMap.sizeIn();
        for(auto&& streamId:outputList.second) {
            if((imgo_size > 0) && (rrzo_size > 0)) {
                // if target output size > rrzo size, use imgo size.
                if(getIsNeedImgo(streamId, in, phy_iomap->second.rrzoSize)) {
                    phy_iomap->second.imgoMap.addOut(streamId);
                }
                else {
                    phy_iomap->second.rrzoMap.addOut(streamId);
                }
            }
            else if(imgo_size > 0) {
                phy_iomap->second.imgoMap.addOut(streamId);
            }
            else {
                phy_iomap->second.rrzoMap.addOut(streamId);
            }
        }
    }
    // build physical metadata
    for(auto&& n : physicalOut_meta ) {
        auto phy_iomap = physicalIOMap.find(n.first);
        if(phy_iomap != physicalIOMap.end()) {
            for(auto&& m:n.second) {
                phy_iomap->second.metaMap.addOut(m);
                addAppStreamIdToMap(n.first, m);
            }
        }
    }
    // build logical iomap
    for(auto&& outputStreamId:logicalOut_img) {
        bool bAddToImgo = false;
        for(auto&& phy_iomap_group:physicalIOMap) {
            bool needCheckImgo = false;
            if(sensorIdToIndex(phy_iomap_group.first) == 0) {
                // cam_0, check need imgo or not.
                needCheckImgo = true;
            }
            size_t imgo_size = phy_iomap_group.second.imgoMap.sizeIn();
            size_t rrzo_size = phy_iomap_group.second.rrzoMap.sizeIn();
            if((imgo_size > 0) && (rrzo_size > 0) && needCheckImgo) {
                // if target output size > rrzo size, use imgo size.
                if(getIsNeedImgo(outputStreamId, in, phy_iomap_group.second.rrzoSize)) {
                    logicalIOMap.imgoMap.addIn(phy_iomap_group.second.imgoMap.vIn);
                    bAddToImgo = true;
                }
                else {
                    logicalIOMap.rrzoMap.addIn(phy_iomap_group.second.rrzoMap.vIn);
                    bAddToImgo = false;
                }
            }
            else if((imgo_size > 0) && needCheckImgo) {
                logicalIOMap.imgoMap.addIn(phy_iomap_group.second.imgoMap.vIn);
                bAddToImgo = true;
            }
            else {
                logicalIOMap.rrzoMap.addIn(phy_iomap_group.second.rrzoMap.vIn);
                bAddToImgo = false;
            }
            // set metadata
            logicalIOMap.metaMap.addIn(phy_iomap_group.second.metaMap.vIn);
            logicalIOMap.subMetaMap.addIn(phy_iomap_group.second.subMetaMap.vIn);
        }
        // add output stream to logical output iomap
        if(bAddToImgo) {
            logicalIOMap.imgoMap.addOut(outputStreamId);
        }
        else {
            logicalIOMap.rrzoMap.addOut(outputStreamId);
        }
    }
    // build output logical metadata
    for(const auto n : logicalOut_meta) {
        logicalIOMap.metaMap.addOut(n);
    }
    logicalIOMap.metaMap.addIn(in.pConfiguration_StreamInfo_NonP1->pAppMeta_Control->getStreamId());
    logicalIOMap.subMetaMap.addIn(in.pConfiguration_StreamInfo_NonP1->pAppMeta_Control->getStreamId());

    IOMapSet P2ImgIO, P2MetaIO;
    auto buildP2IOMapSet = [&P2ImgIO, &P2MetaIO](IOMapGroup &group) {
        if (group.imgoMap.sizeOut() > 0) {
            //dumpIOMap(group.imgoMap);
            P2ImgIO.add(group.imgoMap);
        }
        if (group.rrzoMap.sizeOut() > 0) {
            //dumpIOMap(group.rrzoMap);
            P2ImgIO.add(group.rrzoMap);
        }
        auto logicalIOMapSize = (group.imgoMap.sizeOut() + group.rrzoMap.sizeOut());
        if(logicalIOMapSize > 0) {
            //dumpIOMap(group.metaMap);
            P2MetaIO.add(group.metaMap);
            if(logicalIOMapSize > 1) {
                //dumpIOMap(group.subMetaMap);
                P2MetaIO.add(group.subMetaMap);
            }
        }
    };
    // logical
    {
        if (logicalIOMap.imgoMap.sizeOut() > 0) {
            out.bP2UseImgo = true;
        }
        buildP2IOMapSet(logicalIOMap);
    }
    // physical
    {
        for(auto&& n:physicalIOMap) {
            buildP2IOMapSet(n.second);
        }
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
evaluateRequest_P2CaptureNode_multicam(
    RequestOutputParams& out,
    RequestInputParams const& in
)
{
    if(!DataCheck(in)) {
        return OK;
    }

    if((in.pvImageStreamId_from_CaptureNode_Physical == nullptr)||
       (in.pvMetaStreamId_from_CaptureNode_Physical == nullptr)||
       (in.pvImageStreamId_from_CaptureNode == nullptr)||
       (in.pvMetaStreamId_from_CaptureNode == nullptr)) {
        MY_LOGE("pvImageStreamId_from_CaptureNode_Physical(%p)"
                "pvMetaStreamId_from_CaptureNode_Physical(%p)"
                "pvImageStreamId_from_CaptureNode(%p)"
                "pvMetaStreamId_from_CaptureNode(%p)",
                in.pvImageStreamId_from_CaptureNode_Physical,
                in.pvMetaStreamId_from_CaptureNode_Physical,
                in.pvImageStreamId_from_CaptureNode,
                in.pvMetaStreamId_from_CaptureNode);
        return OK;
    }

    if ( !in.pRequest_PipelineNodesNeed->needP2CaptureNode )
    {
        return OK;
    }

    // define local struct.
    struct IOMapGroup {
        IOMap imageMap;
        IOMap metaMap;
        MSize rrzoSize;
    };
    //
    /*auto sensorIdToIndex = [&in](MINT32 sensorId) {
        for(size_t i=0;i<in.pPipelineStaticInfo->sensorId.size();i++) {
            if(in.pPipelineStaticInfo->sensorId[i] == sensorId) return (MINT32)i;
        }
        return -1;
    };*/
    //
    auto addAppStreamIdToMap = [&out](
                                    uint32_t id,
                                    StreamId_T streamid) {
        if(out.pvMetaStreamId_All_Physical != nullptr) {
            auto iter_phy = out.pvMetaStreamId_All_Physical->find(id);
            if(iter_phy != out.pvMetaStreamId_All_Physical->end()) {
                iter_phy->second.push_back(streamid);
            }
            else {
                std::vector<StreamId_T> temp;
                temp.push_back(streamid);
                out.pvMetaStreamId_All_Physical->insert({
                                                id,
                                                std::move(temp)});
            }
        }
        return;
    };
    //
    IOMapGroup logicalIOMap;
    // <sensor id, IOMapGroup>
    std::unordered_map<uint32_t, IOMapGroup> physicalIOMap;

    auto const& needP1Node = in.pRequest_PipelineNodesNeed->needP1Node;
    auto const& needP1Dma = *in.pRequest_NeedP1Dma;
    auto const& physicalOut_img = *in.pvImageStreamId_from_CaptureNode_Physical;
    auto const& physicalOut_meta = *in.pvMetaStreamId_from_CaptureNode_Physical;
    auto const& logicalOut_img = *in.pvImageStreamId_from_CaptureNode;
    auto const& logicalOut_meta = *in.pvMetaStreamId_from_CaptureNode;

    bool hasImgo = false;

    // using needP1Node data to decide physical p1node input iomap.
    for(size_t i=0;i<needP1Node.size();i++){
        if ( ! needP1Node[i] ) {
            continue;
        }

        auto& p1Info = (*in.pConfiguration_StreamInfo_P1)[i];
        const auto need_P1Dma = needP1Dma[i];

        IOMapGroup iomap;
        sp<IImageStreamInfo> rrzoInfo = p1Info.pHalImage_P1_Rrzo;
        sp<IImageStreamInfo> imgoInfo = p1Info.pHalImage_P1_Imgo;
        if (i == 0)
        {
            if(false == in.isAppRawOutFromP2C)
            {
                if (in.pRequest_AppImageStreamInfo->pAppImage_Output_Priv.get())
                {
                    imgoInfo = in.pRequest_AppImageStreamInfo->pAppImage_Output_Priv;
                }
            }
            if (in.pRequest_AppImageStreamInfo->pAppImage_Input_Priv.get())
            {
                imgoInfo = in.pRequest_AppImageStreamInfo->pAppImage_Input_Priv;
            }
            else if (in.pRequest_AppImageStreamInfo->pAppImage_Input_Yuv.get())
            {
                imgoInfo = in.pRequest_AppImageStreamInfo->pAppImage_Input_Yuv;
            }
            else
            {
                imgoInfo = p1Info.pHalImage_P1_Imgo;
            }
        }
        else
        {
            imgoInfo = p1Info.pHalImage_P1_Imgo;
        }

        if(need_P1Dma & P1_IMGO) {
            iomap.imageMap.addIn(imgoInfo->getStreamId());
            hasImgo = true;
        }

        if(need_P1Dma & P1_RRZO) {
            iomap.imageMap.addIn(rrzoInfo->getStreamId());
            iomap.rrzoSize = rrzoInfo->getImgSize();
        }

        if(need_P1Dma & P1_LCSO) {
            iomap.imageMap.addIn(p1Info.pHalImage_P1_Lcso->getStreamId());
        }
        iomap.metaMap.addIn(p1Info.pAppMeta_DynamicP1->getStreamId());
        iomap.metaMap.addIn(p1Info.pHalMeta_DynamicP1->getStreamId());
        iomap.metaMap.addIn(in.pConfiguration_StreamInfo_NonP1->pAppMeta_Control->getStreamId());
        // set to map
        physicalIOMap.emplace(
                            in.pPipelineStaticInfo->sensorId[i],
                            iomap);
        // check needs output physical stream or not.
        // If needs output physical stream, add P1 app metadata to pAppMeta_DynamicP1.
        for(auto&& item:physicalOut_img) {
            // item.first: unique sensor id.(no duplicate sensor id)
            if(in.pPipelineStaticInfo->sensorId[i] == item.first) {
                addAppStreamIdToMap(item.first, p1Info.pAppMeta_DynamicP1->getStreamId());
            }
        }
    }

    if (!hasImgo)
    {
        MY_LOGE("No Imgo");
        return OK;
    }
    {
        // build physical image map
        for(auto&& outputList:physicalOut_img) {
            // image stream
            auto phy_iomap = physicalIOMap.find(outputList.first);
            if(phy_iomap == physicalIOMap.end()) {
                MY_LOGE("need output physical stream, but cannot find id(%" PRIu32 ") in physicalIOMap",
                            outputList.first);
                return OK;
            }
            for(auto&& streamId:outputList.second) {
                if (in.isMainFrame) {
                    phy_iomap->second.imageMap.addOut(streamId);
                }
            }
        }
        // build logical iomap
        for(auto&& outputStreamId:logicalOut_img) {
            // add logica input stream id by query physicalIOMap.
            for(auto&& phy_iomap_group:physicalIOMap) {
                logicalIOMap.imageMap.addIn(phy_iomap_group.second.imageMap.vIn);
                // set metadata
                logicalIOMap.metaMap.addIn(phy_iomap_group.second.metaMap.vIn);
            }
            // add output stream to logical output iomap
            if (in.isMainFrame) {
                logicalIOMap.imageMap.addOut(outputStreamId);
            }
        }
    }
    // build physical metadata
    if(out.pMultiCamReqOutputParams != nullptr)
    {
        auto &activeSensors = out.pMultiCamReqOutputParams->prvStreamingSensorList;
        for(auto&& n : physicalOut_meta ) {
            auto sensorId = n.first;
            if(std::find(activeSensors.begin(), activeSensors.end(), sensorId) == activeSensors.end())
            {
                MY_LOGD("Ignore to add output physical metadata for inactive sensor %d", sensorId);
                continue;
            }

            auto phy_iomap = physicalIOMap.find(sensorId);
            if(phy_iomap != physicalIOMap.end()) {
                for(auto&& m:n.second) {
                    phy_iomap->second.metaMap.addOut(m);
                    addAppStreamIdToMap(sensorId, m);
                }
            }
        }
    }

    // build output logical metadata
    for(const auto n : logicalOut_meta) {
        logicalIOMap.metaMap.addOut(n);
    }
    logicalIOMap.metaMap.addIn(in.pConfiguration_StreamInfo_NonP1->pAppMeta_Control->getStreamId());

    IOMapSet P2ImgIO, P2MetaIO;
    auto buildP2IOMapSet = [&P2ImgIO, &P2MetaIO, &in](IOMapGroup &group, bool isPhysical) {
        auto combineIOMapSet = [](IOMapSet &source, IOMap &target, bool needCombine) {
            if(needCombine) {
                if(source.size() == 0) {
                    // if target size is zero, add directly.
                    source.add(target);
                }
                else {
                    // in combine flow, source size always is 1.
                    source.editItemAt(0).addIn(target.vIn);
                    source.editItemAt(0).addOut(target.vOut);
                }
            }
            else {
                source.add(target);
            }
            return;
        };
        // if isPhysical is true, isMainFrame is true and not output anything, skip.
        if(in.isMainFrame && isPhysical && group.imageMap.sizeOut() == 0)
        {
            MY_LOGD("isMainFrame %d isPhysical %d imageMap.sizeOut %zu, imageMap.sizeIn %zu, skip buildP2IOMapSet",
                    in.isMainFrame, isPhysical, group.imageMap.sizeOut(), group.imageMap.sizeIn());
            return;
        }
        // skip when no image out since image and metadata should be 1-1 mapped
        if((!in.isMainFrame && isPhysical) ||
           (group.imageMap.sizeOut() == 0 && group.imageMap.sizeIn() == 0))
        {
            MY_LOGD("isMainFrame %d isPhysical %d imageMap.sizeOut %zu, imageMap.sizeIn %zu, skip buildP2IOMapSet",
                    in.isMainFrame, isPhysical, group.imageMap.sizeOut(), group.imageMap.sizeIn());
            return;
        }
        if( !in.isMainFrame || group.imageMap.sizeOut() > 0 ) {
            //dumpIOMap(group.imageMap);
            combineIOMapSet(P2ImgIO, group.imageMap, CAP_IOMAP_COMBINE);
        }
        if( !in.isMainFrame || group.metaMap.sizeOut() > 0 || isPhysical) {
            //dumpIOMap(group.metaMap);
            combineIOMapSet(P2MetaIO, group.metaMap, CAP_IOMAP_COMBINE);
        }
    };
    // logical
    {
        if (logicalIOMap.imageMap.sizeOut() > 0) {
            out.bP2UseImgo = true;
        }
        buildP2IOMapSet(logicalIOMap, false);
    }
    // physical
    {
        for(auto&& n:physicalIOMap) {
            buildP2IOMapSet(n.second, true);
        }
    }
    (*out.pNodeIOMapImage)[eNODEID_P2CaptureNode] = P2ImgIO;
    (*out.pNodeIOMapMeta) [eNODEID_P2CaptureNode] = P2MetaIO;

    return OK;
}


/******************************************************************************
 * Make a function target as a policy - multicam version
 ******************************************************************************/
FunctionType_IOMapPolicy_P2Node makePolicy_IOMap_P2Node_multicam()
{
    return [](
        RequestOutputParams& out,
        RequestInputParams const& in
    ) -> int
    {
        evaluateRequest_P2StreamNode_multicam(out, in);
        evaluateRequest_P2CaptureNode_multicam(out, in);
        return OK;
    };
}

};  //namespace policy
};  //namespace pipeline
};  //namespace v3
};  //namespace NSCam


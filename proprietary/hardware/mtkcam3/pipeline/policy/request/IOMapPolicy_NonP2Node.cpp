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

#define LOG_TAG "mtkcam-NonP2NodeIOMapPolicy"

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

#define CHECK_INDEX_POSITIVE(index) if(index<0) {MY_LOGE("index < 0"); return UNKNOWN_ERROR;}

/******************************************************************************
 *
 ******************************************************************************/
static
MERROR
evaluateRequest_Jpeg(
    RequestOutputParams const& out,
    RequestInputParams const& in
)
{
    if (  !in.pRequest_PipelineNodesNeed->needJpegNode
       || !in.pConfiguration_StreamInfo_NonP1->pHalImage_Jpeg_YUV.get()
       || !in.isMainFrame )
    {
        //MY_LOGD("No need Jpeg node");
        return OK;
    }
    //
    StreamId_T streamid = in.pConfiguration_StreamInfo_NonP1->pHalImage_Jpeg_YUV->getStreamId();
    bool isStreamNode = false;
    for( const auto n : (*in.pvImageStreamId_from_StreamNode) )
    {
        if (n == streamid)
        {
            isStreamNode = true;
            break;
        }
    }
    //
    IOMap JpegMap;
    JpegMap.addIn(streamid);
    // [Jpeg pack]
    // check feature warning first, and check each streaminfo instance.
    if(in.pRequest_ParsedAppMetaControl != nullptr &&
       in.pRequest_ParsedAppMetaControl->control_vsdofFeatureWarning == 0)
    {
        if (in.pConfiguration_StreamInfo_NonP1->pHalImage_Jpeg_Sub_YUV.get())
        {
            StreamId_T streamid = in.pConfiguration_StreamInfo_NonP1->pHalImage_Jpeg_Sub_YUV->getStreamId();
            JpegMap.addIn(streamid);
        }
        if (in.pConfiguration_StreamInfo_NonP1->pHalImage_Depth_YUV.get())
        {
            StreamId_T streamid = in.pConfiguration_StreamInfo_NonP1->pHalImage_Depth_YUV->getStreamId();
            JpegMap.addIn(streamid);
        }
    }
    if (in.pRequest_HalImage_Thumbnail_YUV != NULL)
    {
        JpegMap.addIn(in.pRequest_HalImage_Thumbnail_YUV->getStreamId());
    }
    JpegMap.addOut(in.pRequest_AppImageStreamInfo->pAppImage_Jpeg->getStreamId());
    //
    (*out.pNodeIOMapImage)[eNODEID_JpegNode] = IOMapSet().add(JpegMap);
    (*out.pNodeIOMapMeta) [eNODEID_JpegNode] = IOMapSet().add(IOMap()
                                              .addIn(in.pConfiguration_StreamInfo_NonP1->pAppMeta_Control->getStreamId())
                                              .addIn(isStreamNode ? in.pConfiguration_StreamInfo_NonP1->pHalMeta_DynamicP2StreamNode->getStreamId()
                                                                  : in.pConfiguration_StreamInfo_NonP1->pHalMeta_DynamicP2CaptureNode->getStreamId())
                                              .addOut(in.pConfiguration_StreamInfo_NonP1->pAppMeta_DynamicJpeg->getStreamId()));
    //
    return OK;
}


/******************************************************************************
 *
 ******************************************************************************/
static
MERROR
evaluateRequest_P1RawIspTuningDataPack(
    RequestOutputParams const& out,
    RequestInputParams const& in
)
{
    if ( !in.pRequest_PipelineNodesNeed->needP1RawIspTuningDataPackNode )
    {
        //MY_LOGD("No need P1RawIspTuningDataPack node");
        return OK;
    }
    //
    IOMap imageIOMap;
    IOMap metaIOMap;
    //
    int32_t p1Index = 0;
    bool hasMulitcamSensorList = false;
    if(out.pMultiCamReqOutputParams != nullptr &&
       out.pMultiCamReqOutputParams->prvStreamingSensorList.size() > 0)
    {
        auto sensorId = out.pMultiCamReqOutputParams->prvStreamingSensorList[0];
        p1Index = in.pPipelineStaticInfo->getIndexFromSensorId(sensorId);
        CHECK_INDEX_POSITIVE(p1Index);
        hasMulitcamSensorList = true;
    }

    if (in.pRequest_AppImageStreamInfo->pAppImage_Output_IspTuningData_Raw != nullptr)
    {
        imageIOMap.addIn((*(in.pConfiguration_StreamInfo_P1))[p1Index].pHalImage_P1_Lcso->getStreamId())
                  .addIn((*(in.pConfiguration_StreamInfo_P1))[p1Index].pHalImage_P1_Rrzo->getStreamId())
                  .addOut(in.pRequest_AppImageStreamInfo->pAppImage_Output_IspTuningData_Raw->getStreamId());
    }
    else
    {
        imageIOMap.addIn((*(in.pConfiguration_StreamInfo_P1))[p1Index].pHalImage_P1_Lcso->getStreamId())
                  .addIn((*(in.pConfiguration_StreamInfo_P1))[p1Index].pHalImage_P1_Rrzo->getStreamId());
    }

    // Only add logical metadata to IOMap if logical RAW is requested
    if( (in.pRequest_AppImageStreamInfo->vAppImage_Output_RAW16.size() > 0 || in.pRequest_AppImageStreamInfo->pAppImage_Output_Priv.get()) &&
        in.pConfiguration_StreamInfo_NonP1->pAppMeta_DynamicP1ISPPack.get() )
    {
        metaIOMap.addIn(in.pConfiguration_StreamInfo_NonP1->pAppMeta_Control->getStreamId())
                 .addIn((*(in.pConfiguration_StreamInfo_P1))[p1Index].pAppMeta_DynamicP1->getStreamId())
                 .addIn((*(in.pConfiguration_StreamInfo_P1))[p1Index].pHalMeta_DynamicP1->getStreamId())
                 .addOut(in.pConfiguration_StreamInfo_NonP1->pAppMeta_DynamicP1ISPPack->getStreamId());
    }

    // Physical isp meta stream(to meta stream)
    if(hasMulitcamSensorList)
    {
        // Rules to add metadata to IOMap:
        // 1. Only add active sensors
        // 2. Physical RAW streams of the active sensors are requested
        // 3. Physical RAW streams of the active sensors are configured
        auto &activeSensors = out.pMultiCamReqOutputParams->prvStreamingSensorList;

        // Physical isp meta stream(to image stream)
        for(auto &stream : in.pRequest_AppImageStreamInfo->vAppImage_Output_IspTuningData_Raw_Physical)
        {
            if(std::find(activeSensors.begin(), activeSensors.end(), stream.first) == activeSensors.end())
            {
                MY_LOGD("Ignore to pack ISP tuning data for inactive sensor %d", stream.first);
                continue;
            }

            int index = in.pPipelineStaticInfo->getIndexFromSensorId(stream.first);
            CHECK_INDEX_POSITIVE(index);
            imageIOMap.addIn((*(in.pConfiguration_StreamInfo_P1))[index].pHalImage_P1_Lcso->getStreamId())
                      .addIn((*(in.pConfiguration_StreamInfo_P1))[index].pHalImage_P1_Rrzo->getStreamId())
                      .addOut(stream.second->getStreamId());
        }

        for(auto &stream : in.pRequest_AppImageStreamInfo->vAppImage_Output_RAW16_Physical)
        {
            auto sensorId = stream.first;
            if(std::find(activeSensors.begin(), activeSensors.end(), sensorId) == activeSensors.end())
            {
                MY_LOGD("Ignore to pack ISP tuning data for inactive sensor %d", sensorId);
                continue;
            }

            auto metaIter = in.pConfiguration_StreamInfo_NonP1->vAppMeta_DynamicP1ISPPack_Physical.find(sensorId);
            if(metaIter == in.pConfiguration_StreamInfo_NonP1->vAppMeta_DynamicP1ISPPack_Physical.end())
            {
                MY_LOGW("Sensor %d not found in P1 physical meta config", sensorId);
                continue;
            }

            int index = in.pPipelineStaticInfo->getIndexFromSensorId(sensorId);
            CHECK_INDEX_POSITIVE(index);
            imageIOMap.addIn((*(in.pConfiguration_StreamInfo_P1))[index].pHalImage_P1_Lcso->getStreamId())
                              .addIn((*(in.pConfiguration_StreamInfo_P1))[index].pHalImage_P1_Rrzo->getStreamId());

            metaIOMap.addIn(in.pConfiguration_StreamInfo_NonP1->pAppMeta_Control->getStreamId())
                             .addIn((*(in.pConfiguration_StreamInfo_P1))[index].pAppMeta_DynamicP1->getStreamId())
                             .addIn((*(in.pConfiguration_StreamInfo_P1))[index].pHalMeta_DynamicP1->getStreamId())
                             .addOut(metaIter->second->getStreamId());

            // add out physical metadata stream id.
            if(out.pvMetaStreamId_All_Physical != nullptr) {
                auto iter_phy = (*out.pvMetaStreamId_All_Physical).find(sensorId);
                if(iter_phy != (*out.pvMetaStreamId_All_Physical).end())
                {
                    iter_phy->second.push_back(metaIter->second->getStreamId());
                }
                else
                {
                    std::vector<StreamId_T> temp;
                    temp.push_back(metaIter->second->getStreamId());
                    out.pvMetaStreamId_All_Physical->emplace(sensorId, std::move(temp));
                }
            }
        }
    }

    if(!imageIOMap.isEmpty())
    {
        (*out.pNodeIOMapImage)[eNODEID_P1RawIspTuningDataPackNode] = IOMapSet().add(imageIOMap);
    }

    if(!metaIOMap.isEmpty())
    {
        (*out.pNodeIOMapMeta)[eNODEID_P1RawIspTuningDataPackNode] = IOMapSet().add(metaIOMap);
    }
    //
    return OK;
}


/******************************************************************************
 *
 ******************************************************************************/
static
MERROR
evaluateRequest_P2YuvIspTuningDataPack(
    RequestOutputParams const& out,
    RequestInputParams const& in
)
{
    if ( !in.pRequest_PipelineNodesNeed->needP2YuvIspTuningDataPackNode )
    {
        //MY_LOGD("No need P2YuvIspTuningDataPack node");
        return OK;
    }
    //
    IOMap imageIOMap;
    IOMap metaIOMap;
    //
    if (in.pRequest_AppImageStreamInfo->pAppImage_Output_IspTuningData_Yuv != nullptr)
    {
        imageIOMap.addIn((*(in.pConfiguration_StreamInfo_P1))[0].pHalImage_P1_Lcso->getStreamId())
                  .addOut(in.pRequest_AppImageStreamInfo->pAppImage_Output_IspTuningData_Yuv->getStreamId());
    }
    else
    {
        imageIOMap.addIn((*(in.pConfiguration_StreamInfo_P1))[0].pHalImage_P1_Lcso->getStreamId());
    }

    if( in.pRequest_AppImageStreamInfo->vAppImage_Output_Proc.size() > 0 &&
        in.pConfiguration_StreamInfo_NonP1->pAppMeta_DynamicP2ISPPack.get() )
    {
        metaIOMap.addIn(in.pConfiguration_StreamInfo_NonP1->pAppMeta_Control->getStreamId())
                 .addIn((*(in.pConfiguration_StreamInfo_P1))[0].pAppMeta_DynamicP1->getStreamId())
                 .addIn(in.pConfiguration_StreamInfo_NonP1->pAppMeta_DynamicP2CaptureNode->getStreamId())
                 .addIn(in.pConfiguration_StreamInfo_NonP1->pHalMeta_DynamicP2CaptureNode->getStreamId())
                 .addOut(in.pConfiguration_StreamInfo_NonP1->pAppMeta_DynamicP2ISPPack->getStreamId());
    }

    // Physical isp meta stream(to meta stream)
    if(out.pMultiCamReqOutputParams != nullptr &&
       out.pMultiCamReqOutputParams->prvStreamingSensorList.size() > 0)
    {
        auto &activeSensors = out.pMultiCamReqOutputParams->prvStreamingSensorList;
        // Physical isp meta stream(to image stream)
        for(auto &stream : in.pRequest_AppImageStreamInfo->vAppImage_Output_IspTuningData_Yuv_Physical)
        {
            if(std::find(activeSensors.begin(), activeSensors.end(), stream.first) == activeSensors.end())
            {
                MY_LOGD("Ignore to pack ISP tuning data for inactive sensor %d", stream.first);
                continue;
            }

            int index = in.pPipelineStaticInfo->getIndexFromSensorId(stream.first);
            CHECK_INDEX_POSITIVE(index);
            imageIOMap.addIn((*(in.pConfiguration_StreamInfo_P1))[index].pHalImage_P1_Lcso->getStreamId())
                      .addIn((*(in.pConfiguration_StreamInfo_P1))[index].pHalImage_P1_Rrzo->getStreamId())
                      .addOut(stream.second->getStreamId());
        }

        for(auto &stream : in.pRequest_AppImageStreamInfo->vAppImage_Output_Proc_Physical)
        {
            auto sensorId = stream.first;
            if(std::find(activeSensors.begin(), activeSensors.end(), sensorId) == activeSensors.end())
            {
                MY_LOGD("Ignore to pack ISP tuning data for inactive sensor %d", sensorId);
                continue;
            }

            auto metaIter = in.pConfiguration_StreamInfo_NonP1->vAppMeta_DynamicP2ISPPack_Physical.find(sensorId);
            if(metaIter == in.pConfiguration_StreamInfo_NonP1->vAppMeta_DynamicP2ISPPack_Physical.end())
            {
                MY_LOGW("Sensor %d not found in P1 physical meta config", sensorId);
                continue;
            }

            int index = in.pPipelineStaticInfo->getIndexFromSensorId(stream.first);
            auto iter = in.pConfiguration_StreamInfo_NonP1->vAppMeta_DynamicP2CaptureNode_Physical.find(sensorId);
            if(iter != in.pConfiguration_StreamInfo_NonP1->vAppMeta_DynamicP2CaptureNode_Physical.end())
            {
                CHECK_INDEX_POSITIVE(index);
                imageIOMap.addIn((*(in.pConfiguration_StreamInfo_P1))[index].pHalImage_P1_Lcso->getStreamId());

                metaIOMap.addIn(in.pConfiguration_StreamInfo_NonP1->pAppMeta_Control->getStreamId())
                         .addIn((*(in.pConfiguration_StreamInfo_P1))[index].pAppMeta_DynamicP1->getStreamId())
                         .addIn(iter->second->getStreamId())
                         // .addIn(in.pConfiguration_StreamInfo_NonP1->pHalMeta_DynamicP2CaptureNode->getStreamId())
                         .addIn((*(in.pConfiguration_StreamInfo_P1))[index].pHalMeta_DynamicP1->getStreamId())
                         .addOut(metaIter->second->getStreamId());

                // add out physical metadata stream id.
                if(out.pvMetaStreamId_All_Physical != nullptr) {
                    auto iter_phy = (*out.pvMetaStreamId_All_Physical).find(sensorId);
                    if(iter_phy != (*out.pvMetaStreamId_All_Physical).end())
                    {
                        iter_phy->second.push_back(metaIter->second->getStreamId());
                    }
                    else
                    {
                        std::vector<StreamId_T> temp;
                        temp.push_back(metaIter->second->getStreamId());
                        out.pvMetaStreamId_All_Physical->emplace(sensorId, std::move(temp));
                    }
                }
            }
            else
            {
                MY_LOGE("Cannot find sensor %d in vAppMeta_DynamicP2CaptureNode_Physical",sensorId);
            }
        }
    }

    if(!imageIOMap.isEmpty())
    {
        (*out.pNodeIOMapImage)[eNODEID_P2YuvIspTuningDataPackNode] = IOMapSet().add(imageIOMap);
    }

    if(!metaIOMap.isEmpty())
    {
        (*out.pNodeIOMapMeta)[eNODEID_P2YuvIspTuningDataPackNode] = IOMapSet().add(metaIOMap);
    }
    //
    return OK;
}

/******************************************************************************
 *
 ******************************************************************************/
static
MERROR
evaluateRequest_P2RawIspTuningDataPack(
    RequestOutputParams const& out,
    RequestInputParams const& in
)
{
    if ( !in.pRequest_PipelineNodesNeed->needP2RawIspTuningDataPackNode )
    {
        //MY_LOGD("No need P2RawIspTuningDataPack node");
        return OK;
    }
    //
    IOMap imageIOMap;
    IOMap metaIOMap;
    //
    int32_t p1Index = 0;
    if(out.pMultiCamReqOutputParams != nullptr &&
       out.pMultiCamReqOutputParams->prvStreamingSensorList.size() > 0)
    {
        auto sensorId = out.pMultiCamReqOutputParams->prvStreamingSensorList[0];
        p1Index = in.pPipelineStaticInfo->getIndexFromSensorId(sensorId);
        CHECK_INDEX_POSITIVE(p1Index);
    }

    if (in.pRequest_AppImageStreamInfo->pAppImage_Output_IspTuningData_Raw != nullptr)
    {
        imageIOMap.addIn((*(in.pConfiguration_StreamInfo_P1))[p1Index].pHalImage_P1_Rrzo->getStreamId())
                  .addIn((*(in.pConfiguration_StreamInfo_P1))[p1Index].pHalImage_P1_Lcso->getStreamId())
                  .addOut(in.pRequest_AppImageStreamInfo->pAppImage_Output_IspTuningData_Raw->getStreamId());
    }
    else
    {
        imageIOMap.addIn((*(in.pConfiguration_StreamInfo_P1))[p1Index].pHalImage_P1_Rrzo->getStreamId())
                  .addIn((*(in.pConfiguration_StreamInfo_P1))[p1Index].pHalImage_P1_Lcso->getStreamId());
    }

    if( in.pRequest_AppImageStreamInfo->vAppImage_Output_RAW16.size() > 0 &&
        in.pConfiguration_StreamInfo_NonP1->pAppMeta_DynamicP2ISPPack.get() )
    {
        metaIOMap.addIn(in.pConfiguration_StreamInfo_NonP1->pAppMeta_Control->getStreamId())
                 .addIn((*(in.pConfiguration_StreamInfo_P1))[p1Index].pAppMeta_DynamicP1->getStreamId())
                 .addIn(in.pConfiguration_StreamInfo_NonP1->pAppMeta_DynamicP2CaptureNode->getStreamId())
                 .addIn(in.pConfiguration_StreamInfo_NonP1->pHalMeta_DynamicP2CaptureNode->getStreamId())
                 .addOut(in.pConfiguration_StreamInfo_NonP1->pAppMeta_DynamicP2ISPPack->getStreamId());
    }

    // Physical isp meta stream(to image stream)
    if(out.pMultiCamReqOutputParams != nullptr &&
       out.pMultiCamReqOutputParams->prvStreamingSensorList.size() > 0)
    {
        auto &activeSensors = out.pMultiCamReqOutputParams->prvStreamingSensorList;
        // Physical isp meta stream(to image stream)
        for(auto &stream : in.pRequest_AppImageStreamInfo->vAppImage_Output_IspTuningData_Raw_Physical)
        {
            if(std::find(activeSensors.begin(), activeSensors.end(), stream.first) == activeSensors.end())
            {
                MY_LOGD("Ignore to pack ISP tuning data for inactive sensor %d", stream.first);
                continue;
            }

            int index = in.pPipelineStaticInfo->getIndexFromSensorId(stream.first);
            CHECK_INDEX_POSITIVE(index);
            imageIOMap.addIn((*(in.pConfiguration_StreamInfo_P1))[index].pHalImage_P1_Rrzo->getStreamId())
                      .addIn((*(in.pConfiguration_StreamInfo_P1))[index].pHalImage_P1_Lcso->getStreamId())
                      .addOut(stream.second->getStreamId());
        }

        // Physical isp meta stream(to meta stream)
        for(auto &stream : in.pRequest_AppImageStreamInfo->vAppImage_Output_RAW16_Physical)
        {
            auto sensorId = stream.first;
            if(std::find(activeSensors.begin(), activeSensors.end(), sensorId) == activeSensors.end())
            {
                MY_LOGD("Ignore to pack ISP tuning data for inactive sensor %d", sensorId);
                continue;
            }

            auto metaIter = in.pConfiguration_StreamInfo_NonP1->vAppMeta_DynamicP1ISPPack_Physical.find(sensorId);
            if(metaIter == in.pConfiguration_StreamInfo_NonP1->vAppMeta_DynamicP1ISPPack_Physical.end())
            {
                MY_LOGW("Sensor %d not found in P1 physical meta config", sensorId);
                continue;
            }

            int index = in.pPipelineStaticInfo->getIndexFromSensorId(stream.first);
            auto iter = in.pConfiguration_StreamInfo_NonP1->vAppMeta_DynamicP2CaptureNode_Physical.find(sensorId);
            if(iter != in.pConfiguration_StreamInfo_NonP1->vAppMeta_DynamicP2CaptureNode_Physical.end())
            {
                CHECK_INDEX_POSITIVE(index);
                imageIOMap.addIn((*(in.pConfiguration_StreamInfo_P1))[index].pHalImage_P1_Rrzo->getStreamId())
                          .addIn((*(in.pConfiguration_StreamInfo_P1))[index].pHalImage_P1_Lcso->getStreamId());

                metaIOMap.addIn(in.pConfiguration_StreamInfo_NonP1->pAppMeta_Control->getStreamId())
                         .addIn((*(in.pConfiguration_StreamInfo_P1))[index].pAppMeta_DynamicP1->getStreamId())
                         .addIn(iter->second->getStreamId())
                         .addIn(in.pConfiguration_StreamInfo_NonP1->pHalMeta_DynamicP2CaptureNode->getStreamId())
                         .addOut(metaIter->second->getStreamId());

                // add out physical metadata stream id.
                if(out.pvMetaStreamId_All_Physical != nullptr) {
                    auto iter_phy = (*out.pvMetaStreamId_All_Physical).find(sensorId);
                    if(iter_phy != (*out.pvMetaStreamId_All_Physical).end())
                    {
                        iter_phy->second.push_back(metaIter->second->getStreamId());
                    }
                    else
                    {
                        std::vector<StreamId_T> temp;
                        temp.push_back(metaIter->second->getStreamId());
                        out.pvMetaStreamId_All_Physical->emplace(sensorId, std::move(temp));
                    }
                }
            }
            else
            {
                MY_LOGE("Cannot find sensor %d in vAppMeta_DynamicP2CaptureNode_Physical");
            }
        }
    }

    if(!imageIOMap.isEmpty())
    {
        (*out.pNodeIOMapImage)[eNODEID_P2RawIspTuningDataPackNode] = IOMapSet().add(imageIOMap);
    }

    if(!metaIOMap.isEmpty())
    {
        (*out.pNodeIOMapMeta)[eNODEID_P2RawIspTuningDataPackNode] = IOMapSet().add(metaIOMap);
    }
    //
    return OK;
}


/******************************************************************************
 *
 ******************************************************************************/
static
MERROR
evaluateRequest_FD(
    RequestOutputParams const& out,
    RequestInputParams const& in
)
{
    if ( !in.pRequest_PipelineNodesNeed->needFDNode || !in.isMainFrame )
    {
        //MY_LOGD("No need FD node");
        return OK;
    }
    //
    if ( in.useP1FDYUV )
    {
        (*out.pNodeIOMapImage)[eNODEID_FDNode] = IOMapSet().add(IOMap().addIn((*(in.pConfiguration_StreamInfo_P1))[0].pHalImage_P1_FDYuv->getStreamId()));
        (*out.pNodeIOMapMeta) [eNODEID_FDNode] = IOMapSet().add(IOMap()
                                            .addIn(in.pConfiguration_StreamInfo_NonP1->pAppMeta_Control->getStreamId())
                                            .addIn((*(in.pConfiguration_StreamInfo_P1))[0].pHalMeta_DynamicP1->getStreamId())
                                            .addOut(in.pConfiguration_StreamInfo_NonP1->pAppMeta_DynamicFD->getStreamId()));
    }
    else
    {
        if (in.pRequest_PipelineNodesNeed->needP2CaptureNode && !in.pRequest_PipelineNodesNeed->needP2StreamNode)
        {
            (*out.pNodeIOMapMeta) [eNODEID_FDNode] = IOMapSet().add(IOMap()
                                                .addIn(in.pConfiguration_StreamInfo_NonP1->pAppMeta_Control->getStreamId())
                                                .addOut(in.pConfiguration_StreamInfo_NonP1->pAppMeta_DynamicFD->getStreamId()));
        }
        else if (in.pRequest_PipelineNodesNeed->needP2StreamNode)
        {
            (*out.pNodeIOMapImage)[eNODEID_FDNode] = IOMapSet().add(IOMap().addIn(in.pConfiguration_StreamInfo_NonP1->pHalImage_FD_YUV->getStreamId()));
            (*out.pNodeIOMapMeta) [eNODEID_FDNode] = IOMapSet().add(IOMap()
                                                .addIn(in.pConfiguration_StreamInfo_NonP1->pAppMeta_Control->getStreamId())
                                                .addIn(in.pConfiguration_StreamInfo_NonP1->pHalMeta_DynamicP2StreamNode->getStreamId())
                                                .addOut(in.pConfiguration_StreamInfo_NonP1->pAppMeta_DynamicFD->getStreamId()));
        }
    }

    return OK;
}


/******************************************************************************
 *
 ******************************************************************************/
static
MERROR
evaluateRequest_PDE(
    RequestOutputParams const& out,
    RequestInputParams const& in
)
{
    if ( !in.pRequest_PipelineNodesNeed->needPDENode || !in.isMainFrame )
    {
        //MY_LOGD("No need PDE node");
        return OK;
    }
    //
    (*out.pNodeIOMapImage)[eNODEID_PDENode] = IOMapSet().add(IOMap().addIn((*in.pConfiguration_StreamInfo_P1)[0].pHalImage_P1_Imgo->getStreamId()));
    (*out.pNodeIOMapMeta) [eNODEID_PDENode] = IOMapSet().add(IOMap()
                                             .addIn((*in.pConfiguration_StreamInfo_P1)[0].pHalMeta_DynamicP1->getStreamId())
                                             .addOut(in.pConfiguration_StreamInfo_NonP1->pHalMeta_DynamicPDE->getStreamId()));
    return OK;
}


/******************************************************************************
 *
 ******************************************************************************/
static
MERROR
evaluateRequest_Raw16(
    RequestOutputParams const& out,
    RequestInputParams const& in
)
{
    if ( !in.pRequest_PipelineNodesNeed->needRaw16Node || !in.isMainFrame || in.isAppRawOutFromP2C )
    {
        //MY_LOGD("No need Raw16 node");
        return OK;
    }
    for( const auto& raw16 : in.pRequest_AppImageStreamInfo->vAppImage_Output_RAW16 )
    {
        int index = in.pPipelineStaticInfo->getIndexFromSensorId(raw16.first);
        if(index == -1 && in.pMultiCamReqOutputParams != nullptr)
        {
            if(in.pMultiCamReqOutputParams->prvStreamingSensorList.size() == 0)
            {
                MY_LOGA("should check flow");
            }
            index = in.pPipelineStaticInfo->getIndexFromSensorId(
                                in.pMultiCamReqOutputParams->prvStreamingSensorList[0]);
        }
        if (index == -1)
        {
            MY_LOGD("index == -1, cannot map raw16 stream to a real sensor id, force index = 0");
            index = 0;
        }
        auto const Raw16Node = NodeIdUtils::getRaw16NodeId(index);
        (*out.pNodeIOMapImage)[Raw16Node] = IOMapSet().add(IOMap()
                                                  .addIn((*in.pConfiguration_StreamInfo_P1)[index].pHalImage_P1_Imgo->getStreamId())
                                                  .addOut(raw16.second->getStreamId()));
        if (index == 0)
        {
            (*out.pNodeIOMapMeta) [Raw16Node] = IOMapSet().add(IOMap()
                                             .addIn((*in.pConfiguration_StreamInfo_P1)[index].pHalMeta_DynamicP1->getStreamId())
                                             .addIn((*in.pConfiguration_StreamInfo_P1)[index].pAppMeta_DynamicP1->getStreamId())
                                             .addOut(in.pConfiguration_StreamInfo_NonP1->pAppMeta_DynamicRAW16->getStreamId()));
        }
        else
        {
            (*out.pNodeIOMapMeta) [Raw16Node] = IOMapSet().add(IOMap()
                                             .addIn((*in.pConfiguration_StreamInfo_P1)[index].pHalMeta_DynamicP1->getStreamId())
                                             .addIn((*in.pConfiguration_StreamInfo_P1)[index].pAppMeta_DynamicP1->getStreamId()));
        }
    }

    for( const auto & elm : in.pRequest_AppImageStreamInfo->vAppImage_Output_RAW16_Physical) {
        auto sensorId = elm.first;
        for( const auto& raw16 : elm.second )
        {
            int index = in.pPipelineStaticInfo->getIndexFromSensorId(sensorId);
            if (index == -1)
            {
                MY_LOGD("index == -1, cannot map raw16 stream to a real sensor id, force index = 0");
                index = 0;
            }
            auto const Raw16Node = NodeIdUtils::getRaw16NodeId(index);
            (*out.pNodeIOMapImage)[Raw16Node] = IOMapSet().add(IOMap()
                                                      .addIn((*in.pConfiguration_StreamInfo_P1)[index].pHalImage_P1_Imgo->getStreamId())
                                                      .addOut(raw16->getStreamId()));

            const auto &vRaw16AppMeta = in.pConfiguration_StreamInfo_NonP1->vAppMeta_DynamicRAW16_Physical;
            const auto &iter = vRaw16AppMeta.find(sensorId);
            if(iter != vRaw16AppMeta.end()) {
                (*out.pNodeIOMapMeta) [Raw16Node] = IOMapSet().add(IOMap()
                                                 .addIn((*in.pConfiguration_StreamInfo_P1)[index].pHalMeta_DynamicP1->getStreamId())
                                                 .addIn((*in.pConfiguration_StreamInfo_P1)[index].pAppMeta_DynamicP1->getStreamId())
                                                 .addOut(iter->second->getStreamId()));
                // add out physical metadata stream id.
                if(out.pvMetaStreamId_All_Physical != nullptr) {
                    auto iter_phy = (*out.pvMetaStreamId_All_Physical).find(sensorId);
                    if(iter_phy != (*out.pvMetaStreamId_All_Physical).end()) {
                        iter_phy->second.push_back(iter->second->getStreamId());
                    }
                    else {
                        std::vector<StreamId_T> temp;
                        temp.push_back(iter->second->getStreamId());
                        out.pvMetaStreamId_All_Physical->insert({
                                                        sensorId,
                                                        std::move(temp)});
                    }
                }
                //add p1 metadata stream id to physical metadata vector
                if(out.pvMetaStreamId_All_Physical != nullptr) {
                    StreamId_T appMeta_DynamicP1 = (*in.pConfiguration_StreamInfo_P1)[index].pAppMeta_DynamicP1->getStreamId();
                    auto iter_phy = (*out.pvMetaStreamId_All_Physical).find(sensorId);
                    if(iter_phy != (*out.pvMetaStreamId_All_Physical).end()) {
                        iter_phy->second.push_back(appMeta_DynamicP1);
                    }
                    else {
                        std::vector<StreamId_T> temp;
                        temp.push_back(appMeta_DynamicP1);
                        out.pvMetaStreamId_All_Physical->insert({
                                                        sensorId,
                                                        std::move(temp)});
                    }
                }

            } else {
                (*out.pNodeIOMapMeta) [Raw16Node] = IOMapSet().add(IOMap()
                                                 .addIn((*in.pConfiguration_StreamInfo_P1)[index].pHalMeta_DynamicP1->getStreamId())
                                                 .addIn((*in.pConfiguration_StreamInfo_P1)[index].pAppMeta_DynamicP1->getStreamId()));
            }
        }
    }
    return OK;
}

/******************************************************************************
 *
 ******************************************************************************/
static
MERROR
evaluateRequest_Pass1(
    RequestOutputParams const& out,
    RequestInputParams const& in
)
{
    auto addOut = [](IOMap& iomap, auto const& pStream) {
        if ( pStream != nullptr ) {
            iomap.addOut(pStream->getStreamId());
            return true;
        }
        return false;
    };

    auto pRequest_AppImageStreamInfo = in.pRequest_AppImageStreamInfo;
    auto pRequest_PipelineNodesNeed = in.pRequest_PipelineNodesNeed;
    auto const& needP1Node = in.pRequest_PipelineNodesNeed->needP1Node;
    auto const& needP1Dma = *in.pRequest_NeedP1Dma;
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

        IOMap imageMap;
        auto const nodeId = NodeIdUtils::getP1NodeId(i);

        IImageStreamInfo* pImgoInput = nullptr;
        IImageStreamInfo* pImgoOutput = nullptr;
        bool hasAdded = false;
        if (   false == pRequest_PipelineNodesNeed->needRaw16Node
            && false == in.isAppRawOutFromP2C
            && (pRequest_AppImageStreamInfo->vAppImage_Output_RAW16_Physical.size() != 0)
           )
        {// P1Node directly outputs App RAW16 stream(physical): (if no Raw16Node).
             auto it = pRequest_AppImageStreamInfo->vAppImage_Output_RAW16_Physical.find(in.pPipelineStaticInfo->sensorId[i]);
             if (it != pRequest_AppImageStreamInfo->vAppImage_Output_RAW16_Physical.end())
             {
                for(auto &elm : it->second) {
                    pImgoOutput = elm.get();
                    if ( pImgoOutput != nullptr ) {
                        imageMap.addOut(pImgoOutput->getStreamId());
                        hasAdded = true;
                        // If physical Raw16 add to out, add metadata stream id.
                        if(out.pvMetaStreamId_All_Physical != nullptr) {
                            auto iter_phy = out.pvMetaStreamId_All_Physical->find(
                                                                in.pPipelineStaticInfo->sensorId[i]);
                            if(iter_phy != out.pvMetaStreamId_All_Physical->end()) {
                                iter_phy->second.push_back(configP1Streams.pAppMeta_DynamicP1->getStreamId());
                            } else {
                                std::vector<StreamId_T> temp;
                                temp.push_back(configP1Streams.pAppMeta_DynamicP1->getStreamId());
                                out.pvMetaStreamId_All_Physical->insert({
                                                                in.pPipelineStaticInfo->sensorId[i],
                                                                std::move(temp)});
                            }
                        }
                    }
                }
            }
        }

        if ( i == in.masterIndex )
        {
            bool canRRZO = true;
            bool canLCSO = true;
            bool canRSSO = true;
            bool canFDYUV= true;
            bool canScaledYUV = true;

            // IMGO
            {
                ////////////////////////////////////////////////////////////////
                // Input/Output App stream
                if ( in.isMainFrame )
                {
                    ////////////////////////////////////////////////////////////
                    // Input App stream
                    if ( auto p = pRequest_AppImageStreamInfo->pAppImage_Input_Yuv.get() ) {
                        pImgoInput = p;
                        canRRZO = canLCSO = canRSSO = canFDYUV = canScaledYUV = false;
                    }
                    else
                    if ( auto p = pRequest_AppImageStreamInfo->pAppImage_Input_Priv.get() ) {
                        pImgoInput = p;
                        canRRZO = canLCSO = canRSSO = canFDYUV = canScaledYUV = false;
                    }
                    else
                    if ( auto p = pRequest_AppImageStreamInfo->pAppImage_Input_RAW16.get() ) {
                        pImgoInput = p;
                        canRRZO = canLCSO = canRSSO = canFDYUV = canScaledYUV = false;
                    }
                    ////////////////////////////////////////////////////////////
                    // Output App stream
                    else
                    if (    false == in.isAppRawOutFromP2C
                        && pRequest_AppImageStreamInfo->pAppImage_Output_Priv.get() )
                    {// P1Node directly outputs App private stream.
                        pImgoOutput = pRequest_AppImageStreamInfo->pAppImage_Output_Priv.get();
                    }
                    else
                    if (    false == pRequest_PipelineNodesNeed->needRaw16Node
                        && false == in.isAppRawOutFromP2C
                        && (pRequest_AppImageStreamInfo->vAppImage_Output_RAW16.size() != 0)
                       )
                    {// P1Node directly outputs App RAW16 stream: (if no Raw16Node).
                         auto it = pRequest_AppImageStreamInfo->vAppImage_Output_RAW16.find(in.pPipelineStaticInfo->openId);
                         if (it != pRequest_AppImageStreamInfo->vAppImage_Output_RAW16.end())
                         {
                             pImgoOutput = it->second.get();
                         }
                    }
                }

                ////////////////////////////////////////////////////////////////
                // Output Hal stream (IMGO)
                if ( pImgoOutput == nullptr && pImgoInput == nullptr ) {
                    if ( (need_P1Dma & P1_IMGO) ) {
                        pImgoOutput = (*in.pConfiguration_StreamInfo_P1)[i].pHalImage_P1_Imgo.get();
                    }
                }

                ////////////////////////////////////////////////////////////////
                if ( !hasAdded && pImgoOutput != nullptr ) {
                    imageMap.addOut(pImgoOutput->getStreamId());
                }
                else
                if ( pImgoInput != nullptr ) {
                    imageMap.addIn(pImgoInput->getStreamId());
                }
            }

            if ( canRRZO && (need_P1Dma & P1_RRZO) ) {
                addOut(imageMap, configP1Streams.pHalImage_P1_Rrzo );
            }
            if ( canLCSO && (need_P1Dma & P1_LCSO) ) {
                addOut(imageMap, configP1Streams.pHalImage_P1_Lcso );
            }
            if ( canRSSO && (need_P1Dma & P1_RSSO) ) {
                addOut(imageMap, configP1Streams.pHalImage_P1_Rsso );
            }
            if ( canFDYUV && (need_P1Dma & P1_FDYUV) ) {
                addOut(imageMap, configP1Streams.pHalImage_P1_FDYuv );
            }
            if ( canScaledYUV && (need_P1Dma & P1_SCALEDYUV) ) {
                addOut(imageMap, configP1Streams.pHalImage_P1_ScaledYuv );
            }
        }
        else
        {

            if (need_P1Dma & P1_IMGO) {
                addOut(imageMap, configP1Streams.pHalImage_P1_Imgo );
            }
            if (need_P1Dma & P1_RRZO) {
                addOut(imageMap, configP1Streams.pHalImage_P1_Rrzo );
            }
            if (need_P1Dma & P1_LCSO) {
                addOut(imageMap, configP1Streams.pHalImage_P1_Lcso );
            }
            if (need_P1Dma & P1_RSSO) {
                addOut(imageMap, configP1Streams.pHalImage_P1_Rsso );
            }
            // [After ISP 6.0]
            if (need_P1Dma & P1_SCALEDYUV) {
                addOut(imageMap, configP1Streams.pHalImage_P1_ScaledYuv );
            }
        }

        // dummy frame does not need image IOMap
        if ( ! in.isDummyFrame )
            (*out.pNodeIOMapImage)[nodeId] = IOMapSet().add(imageMap);

        (*out.pNodeIOMapMeta) [nodeId] = IOMapSet().add(IOMap()
                                        .addIn( in.pConfiguration_StreamInfo_NonP1->pAppMeta_Control->getStreamId())
                                        .addIn( configP1Streams.pHalMeta_Control->getStreamId())
                                        .addOut(configP1Streams.pAppMeta_DynamicP1->getStreamId())
                                        .addOut(configP1Streams.pHalMeta_DynamicP1->getStreamId()));
    }
    return OK;
}


/******************************************************************************
 * Make a function target as a policy - default version
 ******************************************************************************/
FunctionType_IOMapPolicy_NonP2Node makePolicy_IOMap_NonP2Node_Default()
{
    return [](
        RequestOutputParams const& out,
        RequestInputParams const& in
    ) -> int
    {
        evaluateRequest_Jpeg (out, in);
        evaluateRequest_P1RawIspTuningDataPack (out, in);
        evaluateRequest_P2YuvIspTuningDataPack (out, in);
        evaluateRequest_P2RawIspTuningDataPack (out, in);
        evaluateRequest_FD   (out, in);
        evaluateRequest_PDE  (out, in);
        evaluateRequest_Raw16(out, in);
        evaluateRequest_Pass1(out, in);
        return OK;
    };
}


};  //namespace policy
};  //namespace pipeline
};  //namespace v3
};  //namespace NSCam


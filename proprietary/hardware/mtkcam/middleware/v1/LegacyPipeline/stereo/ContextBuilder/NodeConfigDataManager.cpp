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
#define LOG_TAG "ContextBuilder/NodeConfigDataManager"
//
#include <mtkcam/utils/std/Log.h>
#include <mtkcam/middleware/v1/LegacyPipeline/stereo/ContextBuilder/NodeConfigDataManager.h>
//
#include <mtkcam/middleware/v1/LegacyPipeline/NodeId.h>
//
#include <mtkcam/pipeline/pipeline/PipelineContext.h>
//
#include <vector>
//
#include "NodeData/StereoP1NodeConfigData.h"
#include "NodeData/StereoP1NodeMain2ConfigData.h"
#include "NodeData/StereoP2NodeConfigData.h"
#include "NodeData/StereoP2NodeMain2ConfigData.h"
#include "NodeData/Raw16NodeConfigData.h"
#include "NodeData/StereoRootNodeConfigData.h"
#include "NodeData/DualYUVNodeConfigData.h"
#include "NodeData/Dual3rdPartyNodeConfigData.h"

#if MTKCAM_HAVE_VSDOF_SUPPORT
#include "NodeData/DepthMapNodeConfigData.h"
#include "NodeData/BokehNodeConfigData.h"
#include "NodeData/DualImageTransformNodeConfigData.h"
#include "NodeData/JpegEnc_BokehNodeConfigData.h"
#include "NodeData/JpegEnc_JPSNodeConfigData.h"
#include "NodeData/JpegEnc_JpegNodeConfigData.h"
#endif // MTKCAM_HAVE_VSDOF_SUPPORT
//#include "NodeData/PostViewNodeConfigData.h"
// for 3rd party use
#include "NodeData/TPNodeConfigData.h"
#include "NodeData/JpegEnc_CleanImageNodeConfigData.h"
#include "NodeData/JpegEnc_TP_JpegNodeConfigData.h"
#include "NodeData/JpegEnc_TP0_JpegNodeConfigData.h"
/******************************************************************************
 *
 ******************************************************************************/
#define MY_LOGV(fmt, arg...)        CAM_LOGV("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGD(fmt, arg...)        CAM_LOGD("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGI(fmt, arg...)        CAM_LOGI("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGW(fmt, arg...)        CAM_LOGW("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGE(fmt, arg...)        CAM_LOGE("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGA(fmt, arg...)        CAM_LOGA("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGF(fmt, arg...)        CAM_LOGF("[%s] " fmt, __FUNCTION__, ##arg)
//
using namespace NSCam;
using namespace v3;
using namespace NSCam::v3::NSPipelineContext;
using namespace std;
//
sp<NodeConfigDataManager>
NodeConfigDataManager::
create(char const* callerName, const NodeConfigMgrSetting* setting)
{
    return new NodeConfigDataManager(callerName, setting);
}
//
void
NodeConfigDataManager::
destroy()
{
    for(MUINT32 i=0;i<mvNodeDataConfigContainer.size();++i)
    {
        mvNodeDataConfigContainer[i]->destroy();
    }
    mvNodeDataConfigContainer.clear();
}
//
NodeConfigDataManager::
NodeConfigDataManager(char const* challerName, const NodeConfigMgrSetting* setting)
    : mCallerName(challerName)
    , mSensorId(0)
    , mSensorIdMain2(2)
{
    if(setting != NULL)
    {
        mSensorId = setting->sensorId;
        mSensorIdMain2= setting->sensorIdMain2;
        init(setting->cfgData);

        MY_LOGD("mSensorId(%d), mSensorIdMain2(%d)", mSensorId, mSensorIdMain2);
    }
    else {
        MY_LOGE("Input Invalid Config Setting!!");
    }
}
//
NodeConfigDataManager::
~NodeConfigDataManager()
{
    MY_LOGD("dcot(%x)", this);
}

void
NodeConfigDataManager::
init(const MINT32* data)
{
    // add root node
    mNodeSet.add(data[0]);
    // build node edge
    buildNodeEdge(data);
    // create node config data instance and add to mvNodeDataConfigContainer.
    MINT32 const* temp = data;
    while(*temp != -1)
    {
        createNodeActor(*temp, mvNodeDataConfigContainer);
        temp++;
    }
    MY_LOGD("mvNodeDataConfigContainer.size(%d)", mvNodeDataConfigContainer.size());
}
//
void
NodeConfigDataManager::
buildNodeEdge(const MINT32* data)
{
    MINT32 const* pNodeConnectData = data;
    while(*pNodeConnectData!=-1)
    {
        if((*(pNodeConnectData) == -1 || *(pNodeConnectData + 1)== -1))
        {
            break;
        }
        mNodeEdgeSet.addEdge(*pNodeConnectData, *(pNodeConnectData+1));
        pNodeConnectData += 2;
    }
}
//
void
NodeConfigDataManager::
createNodeActor(NodeId_T nodeId, DefaultKeyedVector<NodeId_T, sp<INodeConfigBase> >& nodeList)
{
    if(eNODEID_TPNode == nodeId)
        nodeList.add(nodeId, new TPNodeConfigData(mSensorId, nodeId, "TP"));
    else if(eNODEID_JpegNode_TP == nodeId)
        nodeList.add(nodeId, new JpegEnc_TP_JpegNodeConfigData(mSensorId, nodeId, "Jpeg_TP"));
    else if(eNODEID_JpegNode_TP0 == nodeId)
        nodeList.add(nodeId, new JpegEnc_TP0_JpegNodeConfigData(mSensorId, nodeId, "Jpeg_TP0"));
    else if(eNODEID_JpegNode_Clean == nodeId)
        nodeList.add(nodeId, new JpegEnc_CleanImageNodeConfigData(mSensorId, nodeId, "Jpg_CleanImage"));
#if defined (MTKCAM_STEREO_SUPPORT)
    else if(eNODEID_P1Node == nodeId)
        nodeList.add(nodeId, new StereoP1NodeConfigData(mSensorId, nodeId, "StereoP1_Main1"));
    else if(eNODEID_P1Node_main2 == nodeId)
        nodeList.add(nodeId, new StereoP1NodeMain2ConfigData(mSensorIdMain2, nodeId, "StereoP1_Main2"));
    else if(eNODEID_P2Node == nodeId)
        nodeList.add(nodeId, new StereoP2NodeConfigData(mSensorId, nodeId, "StereoP2"));
    else if(eNODEID_P2Node_main2 == nodeId)
        nodeList.add(nodeId, new StereoP2NodeMain2ConfigData(mSensorIdMain2, nodeId, "StereoP2_Main2"));
    else if(eNODEID_StereoRootNode == nodeId)
        nodeList.add(nodeId, new StereoRootNodeConfigData(mSensorId, nodeId, "StereoRootNode"));
    else if(eNODEID_RAW16Out == nodeId)
        nodeList.add(nodeId, new Raw16NodeConfigData(mSensorId, nodeId, "Raw16Node"));
    else if(eNODEID_Dual3rdPartyNode == nodeId)
        nodeList.add(nodeId, new Dual3rdPartyNodeConfigData(mSensorId, nodeId, "Dual3rdNode"));
#endif
#if defined (MTKCAM_STEREO_VSDOF_SUPPORT)
    else if(eNODEID_DepthMapNode == nodeId)
        nodeList.add(nodeId, new DepthMapNodeConfigData(mSensorId, nodeId, "DepthMapNode"));
    else if(eNODEID_BokehNode == nodeId)
        nodeList.add(nodeId, new BokehNodeConfigData(mSensorId, nodeId, "BokehNode"));
    else if(eNODEID_JpegNode_Bokeh == nodeId)
        nodeList.add(nodeId, new JpegEnc_BokehNodeConfigData(mSensorId, nodeId, "Jpg_Bokeh"));
    else if(eNODEID_JpegNode_JPS == nodeId)
        nodeList.add(nodeId, new JpegEnc_JPSNodeConfigData(mSensorId, nodeId, "Jpg_JPS"));
    else if(eNODEID_JpegNode == nodeId)
        nodeList.add(nodeId, new JpegEnc_JpegNodeConfigData(mSensorId, nodeId, "Jpg_Image"));
    else if(eNODEID_DualITNode == nodeId)
        nodeList.add(nodeId, new DualImageTransformNodeConfigData(mSensorId, nodeId, "DualITNode"));
    else if(eNODEID_SWBokehNode == nodeId)
        nodeList.add(nodeId, new BokehNodeConfigData(mSensorId, nodeId, "SWBokeh"));
    else if(eNODEID_HWBokehNode == nodeId)
        nodeList.add(nodeId, new BokehNodeConfigData(mSensorId, nodeId, "HWBokeh"));
    else if(eNODEID_VendorBokehNode == nodeId)
        nodeList.add(nodeId, new BokehNodeConfigData(mSensorId, nodeId, "VendorBokeh"));
#endif
    else
        MY_LOGE("NodeId is not support (%d).", nodeId);
    return;
}
//
void
NodeConfigDataManager::
configNode(
    sp<MetaStreamManager> metaManager,
    sp<ImageStreamManager> imageManager,
    sp<NSCam::StereoBasicParameters> userParams,
    sp<PipelineContext>& pipelineContext)
{
    for(int i=0 ;i<mvNodeDataConfigContainer.size();++i)
    {
        mvNodeDataConfigContainer.valueAt(i)->configNode(metaManager.get(),
                                                         imageManager.get(),
                                                         userParams.get(),
                                                         pipelineContext.get());
    }
}
//
sp<INodeActor>
NodeConfigDataManager::
getNodeInstance(NodeId_T nodeId)
{
    ssize_t index = mvNodeDataConfigContainer.indexOfKey(nodeId);
    if(index>=0)
    {
        return mvNodeDataConfigContainer.valueAt(index)->getNode();
    }
    return 0;
}
//
NodeId_T
NodeConfigDataManager::
getNodeIdByDstStreamId(StreamId_T streamId)
{
    sp<INodeConfigBase> nodeBase = NULL;
    for(MUINT32 i = 0;i<mvNodeDataConfigContainer.size();++i)
    {
        nodeBase = mvNodeDataConfigContainer.valueAt(i);
        if(nodeBase->isDstStreamId(streamId))
        {
            return mvNodeDataConfigContainer.keyAt(i);
        }
    }
    MY_LOGE("Cannot find streamId(%d)", streamId);
    return 0;
}
//
MUINT32
NodeConfigDataManager::
getSize()
{
    return mvNodeDataConfigContainer.size();
}
//
NodeId_T
NodeConfigDataManager::
getNodeId(MUINT32 index)
{
    return mvNodeDataConfigContainer.keyAt(index);
}
//
StreamSet
NodeConfigDataManager::
getInStreamSet(NodeId_T nodeId)
{
    ssize_t index = mvNodeDataConfigContainer.indexOfKey(nodeId);
    if(index>=0)
    {
        return mvNodeDataConfigContainer.valueAt(index)->getInStreamSet();
    }
    MY_LOGD("CKH: return default value.");
    return StreamSet();
}
//
StreamSet
NodeConfigDataManager::
getOutStreamSet(NodeId_T nodeId)
{
    ssize_t index = mvNodeDataConfigContainer.indexOfKey(nodeId);
    if(index>=0)
    {
        return mvNodeDataConfigContainer.valueAt(index)->getOutStreamSet();
    }
    MY_LOGD("CKH: return default value.");
    return StreamSet();
}
//
DefaultKeyedVector<StreamId_T, MINT32 >
NodeConfigDataManager::
getStreamUsageSet(NodeId_T nodeId)
{
    ssize_t index = mvNodeDataConfigContainer.indexOfKey(nodeId);
    if(index>=0)
    {
        return mvNodeDataConfigContainer.valueAt(index)->getStreamUsageSet();
    }
    MY_LOGD("CKH: return default value.");
    return DefaultKeyedVector<StreamId_T, MINT32 >();
}
//
IOMapSet
NodeConfigDataManager::
getImageIOMap(NodeId_T nodeId)
{
    ssize_t index = mvNodeDataConfigContainer.indexOfKey(nodeId);
    if(index>=0)
    {
        return mvNodeDataConfigContainer.valueAt(index)->getImageIOMaps();
    }
    MY_LOGD("CKH: return default value.");
    return IOMapSet();
}
//
IOMapSet
NodeConfigDataManager::
getMetaIOMap(NodeId_T nodeId)
{
    ssize_t index = mvNodeDataConfigContainer.indexOfKey(nodeId);
    if(index>=0)
    {
        return mvNodeDataConfigContainer.valueAt(index)->getMetaIOMaps();
    }
    MY_LOGD("CKH: return default value.");
    return IOMapSet();
}
//
NSPipelineContext::NodeSet
NodeConfigDataManager::
getRootNodeId()
{
    return mNodeSet;
}
//
NSPipelineContext::NodeEdgeSet
NodeConfigDataManager::
getNodeEdgeSet()
{
    return mNodeEdgeSet;
}
//
void
NodeConfigDataManager::
dump()
{
    MUINT32 size = mvNodeDataConfigContainer.size();
    if(size>0)
    {
        for(MUINT32 i=0;i<size;++i)
        {
            MY_LOGD("NodeId(%d) Data(0x%x)",
                    mvNodeDataConfigContainer.keyAt(i),
                    mvNodeDataConfigContainer.valueAt(i).get());
            // dump node
            mvNodeDataConfigContainer.valueAt(i)->dump();
        }
    }
}

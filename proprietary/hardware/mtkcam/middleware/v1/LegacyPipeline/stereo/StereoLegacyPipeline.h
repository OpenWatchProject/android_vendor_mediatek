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

#ifndef _MTK_HARDWARE_MTKCAM_LEGACYPIPELINE_STEREOLEGACYPIPELINE_H_
#define _MTK_HARDWARE_MTKCAM_LEGACYPIPELINE_STEREOLEGACYPIPELINE_H_
//

using namespace android;

namespace NSCam {
namespace v3{
class ImageStreamManager;
class MetaStreamManager;
class NodeConfigDataManager;
};
};
/******************************************************************************
 *
 ******************************************************************************/
//
namespace NSCam {
namespace v1 {
namespace NSLegacyPipeline {

class StereoLegacyPipeline
    : public ILegacyPipeline
{
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Create
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:
    static sp<StereoLegacyPipeline>     createInstance(
                                            MINT32 const openId,
                                            MINT32 const mode
                                        );
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Interface.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:
                                        StereoLegacyPipeline(
                                            MINT32 const openId,
                                            MINT32 const mode
                                        );

                                        ~StereoLegacyPipeline();

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  ILegacyPipeline Interface.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:     ////

    virtual MERROR                      flush() {
                                            return mpPipelineContext.get() ?
                                                mpPipelineContext->flush() : OK;
                                        }

    virtual MERROR                      waitUntilDrained() {
                                            return mpPipelineContext.get() ?
                                                mpPipelineContext->waitUntilDrained() : OK;
                                        }

    virtual MERROR                      waitUntilDrainedAndFlush() {
                                            MERROR err = waitUntilDrained();
                                            return ( err == OK ) ?
                                                flush() : err;
                                        }

    virtual wp<ResultProcessor>         getResultProcessor() {
                                            return mpResultProcessor;
                                        }

    virtual wp<TimestampProcessor>      getTimestampProcessor() {
                                            return mpTimestampProcessor;
                                        }

    virtual MERROR                      submitSetting(
                                            MINT32    const requestNo,
                                            IMetadata& appMeta,
                                            IMetadata& halMeta,
                                            ResultSet* pResultSet
                                        );

    virtual MERROR                      submitRequest(
                                            MINT32    const requestNo,
                                            IMetadata& appMeta,
                                            IMetadata& halMeta,
                                            Vector<BufferSet> vDstStreams,
                                            ResultSet* pResultSet
                                        );

    virtual MERROR getFrameNo( MINT32 &frameNo){ /* not support */  return UNKNOWN_ERROR; }

    virtual MERROR                      submitRequest(
                                            MINT32    const requestNo,
                                            IMetadata& appMeta,
                                            IMetadata& halMeta,
                                            Vector<BufferSet> vDstStreams,
                                            MINT32 type
                                        );

public:     ////

    virtual sp<IImageStreamInfo>        queryImageStreamInfo(StreamId const streamId);

    virtual sp<IMetaStreamInfo>         queryMetaStreamInfo(StreamId const streamId);

    virtual sp<StreamBufferProvider>    queryProvider(StreamId const streamId);

    virtual MVOID                       setRequestBuilder(
                                        sp<RequestBuilder> const pRequestBuilder
                                        ){
                                            mpRequestBuilder = pRequestBuilder;
                                        }

    virtual MVOID                       getMetaIOMapInfo(KeyedVector< NSCam::v3::Pipeline_NodeId_T, NSCam::v3::NSPipelineContext::IOMapSet >& vInfo)
                                        {
                                            vInfo = vIOMapInfo;
                                        }

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  RefBase Interface.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:
    virtual MVOID                       onLastStrongRef( const void* /*id*/);

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Interface.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:     //// for legacy pipeline builder

    MVOID                               setPipelineContext(
                                            sp<PipelineContext> const pPipelineContext
                                        ){
                                            mpPipelineContext = pPipelineContext;
                                        }



    MVOID                               setResultProcessor(
                                            sp<ResultProcessor> const pResultProcessor
                                        ){
                                            mpResultProcessor = pResultProcessor;
                                        }

    MVOID                               setTimestampProcessor(
                                            sp<TimestampProcessor> const pTimestampProcessor
                                        ){
                                            mpTimestampProcessor = pTimestampProcessor;
                                        }

    MVOID                               setImageStreamManager(
                                            sp<ImageStreamManager> const pImageStreamManager
                                        ){
                                            mpImageStreamManager = pImageStreamManager;
                                        }

    MVOID                               setMetaStreamManager(
                                            sp<MetaStreamManager> const pMetaStreamManager
                                        ){
                                            mpMetaStreamManager = pMetaStreamManager;
                                        }

    MVOID                               setNodeConfigDataManager(
                                            sp<NodeConfigDataManager> const pNodeConfigDataManager
                                        ){
                                            mpNodeConfigDataManager = pNodeConfigDataManager;
                                        }

    MVOID                               setNodeConfigDataManagers(
                                            Vector< sp<NodeConfigDataManager> > const vNodeConfigDataManager
                                        ){
                                            mvNodeConfigDataManagers = vNodeConfigDataManager;
                                        }

    MVOID                               setMetaStreamId(
                                            StreamId const halMetaStreamId,
                                            StreamId const appMetaStreamId
                                        ){
                                            mHalMetaStreamId = halMetaStreamId;
                                            mAppMetaStreamId = appMetaStreamId;
                                        }

    MVOID                               setMetaIOMap(
                                            NSCam::v3::Pipeline_NodeId_T            const  nodeId,
                                            NSCam::v3::NSPipelineContext::IOMapSet const& metaIOMap
                                        ){
                                            vIOMapInfo.add( nodeId, metaIOMap );
                                        }

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Implementations.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
protected:  ////
    sp<PipelineContext>                 mpPipelineContext       = NULL;
    sp<ResultProcessor>                 mpResultProcessor       = NULL;
    sp<TimestampProcessor>              mpTimestampProcessor    = NULL;
    sp<RequestBuilder>                  mpRequestBuilder        = NULL;
    Vector< sp<NodeConfigDataManager> > mvNodeConfigDataManagers;
    sp<ImageStreamManager>              mpImageStreamManager    = NULL;
    sp<MetaStreamManager>               mpMetaStreamManager     = NULL;
    sp<NodeConfigDataManager>           mpNodeConfigDataManager = NULL;
    //
    StreamId                            mHalMetaStreamId        = 0;
    StreamId                            mAppMetaStreamId        = 0;

    KeyedVector< NSCam::v3::Pipeline_NodeId_T, NSCam::v3::NSPipelineContext::IOMapSet >
                                        vIOMapInfo;
private:
    MINT32                              miOpenId                = -1;
    MINT32                              miMode                  = -1;
    MINT32                              mTimestamp              = 0;
    MINT32                              miOpenId_main1          = -1;
    MINT32                              miOpenId_main2          = -1;
};


};  //namespace NSLegacyPipeline
};  //namespace v1
};  //namespace NSCam
#endif  //_MTK_HARDWARE_MTKCAM_LEGACYPIPELINE_LEGACYPIPELINEPREVIEW_H_


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

#ifndef _MTK_HARDWARE_MTKCAM_LEGACYPIPELINE_LEGACYPIPELINEBASE_H_
#define _MTK_HARDWARE_MTKCAM_LEGACYPIPELINE_LEGACYPIPELINEBASE_H_
//

using namespace android;

/******************************************************************************
 *
 ******************************************************************************/
//
namespace NSCam {
namespace v1 {
namespace NSLegacyPipeline {

class LegacyPipelineBase
    : public ILegacyPipeline
{
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Interface.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:
                                        LegacyPipelineBase();


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  ILegacyPipeline Interface.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:     ////

    virtual MERROR                      flush();
    virtual MERROR                      flushForReuse();
    virtual MERROR                      flushNode(NodeId_T const nodeId);

    virtual MERROR                      waitUntilDrained() {
                                            return mpPipelineContext.get() ?
                                                mpPipelineContext->waitUntilDrained() : OK;
                                        }

    virtual MERROR                      waitNodeUntilDrained(NodeId_T const nodeId);

    virtual wp<ResultProcessor>         getResultProcessor() {
                                            return mpResultProcessor;
                                        }

    virtual wp<TimestampProcessor>      getTimestampProcessor() {
                                            return mpTimestampProcessor;
                                        }

public:     ////

    virtual sp<IImageStreamInfo>        queryImageStreamInfo(StreamId const streamId) {
                                            return ( vImageStreamInfo.indexOfKey(streamId) < 0) ? NULL :
                                                vImageStreamInfo.editValueFor(streamId);
                                        }

    virtual sp<IMetaStreamInfo>         queryMetaStreamInfo(StreamId const streamId) {
                                            return ( vMetaStreamInfo.indexOfKey(streamId) < 0) ? NULL :
                                                vMetaStreamInfo.editValueFor(streamId);
                                        }

    virtual sp<StreamBufferProvider>    queryProvider(StreamId const streamId) {
                                            return ( vStreamBufferProvider.indexOfKey(streamId) < 0) ? NULL :
                                                vStreamBufferProvider.editValueFor(streamId);
                                        }

    virtual MVOID                       setRequestBuilder(
                                            sp<RequestBuilder> const pRequestBuilder
                                        ){
                                            Mutex::Autolock _l(mLock);
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
//  LegacyPipelineBase Interface.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:     //// for legacy pipeline builder

    virtual MVOID                       setPipelineContext(
                                            sp<PipelineContext> const pPipelineContext
                                        ){
                                            mpPipelineContext = pPipelineContext;
                                        }

    virtual MVOID                       setResultProcessor(
                                            sp<ResultProcessor> const pResultProcessor
                                        ){
                                            mpResultProcessor = pResultProcessor;
                                        }

    virtual MVOID                       setTimestampProcessor(
                                            sp<TimestampProcessor> const pTimestampProcessor
                                        ){
                                            mpTimestampProcessor = pTimestampProcessor;
                                        }

    virtual MVOID                       setImageStreamInfo(
                                            sp<IImageStreamInfo> const pImageStreamInfo
                                        );

    virtual MVOID                       setMetaStreamInfo(
                                            sp<IMetaStreamInfo> const pMetaStreamInfo
                                        );

    virtual MVOID                       setStreamBufferProvider(
                                            sp<StreamBufferProvider> const pStreamBufferProvider
                                        );

    virtual MVOID                       setMetaIOMap(
                                            NSCam::v3::Pipeline_NodeId_T            const  nodeId,
                                            NSCam::v3::NSPipelineContext::IOMapSet const& metaIOMap
                                        )
                                        {
                                            vIOMapInfo.add( nodeId, metaIOMap );
                                        }

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Implementations.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:
    MINT32                              getOpenId() const { return mOpenId; }
    virtual MERROR                      getFrameNo( MINT32 &frameNo);

protected:  ////
    template <typename _Node_>
    MERROR                              flushNode(NodeId_T const nodeId, android::sp< NodeActor<_Node_> >& pNodeActor);

protected:  ////
    sp<PipelineContext>                 mpPipelineContext;
    sp<ResultProcessor>                 mpResultProcessor;
    sp<TimestampProcessor>              mpTimestampProcessor;
    sp<RequestBuilder>                  mpRequestBuilder;
    KeyedVector< NSCam::v3::Pipeline_NodeId_T, NSCam::v3::NSPipelineContext::IOMapSet >
                                        vIOMapInfo;

protected:  ////
    KeyedVector< StreamId_T, sp<StreamBufferProvider> >
                                        vStreamBufferProvider;
    KeyedVector< StreamId_T, sp<IImageStreamInfo> >
                                        vImageStreamInfo;
    KeyedVector< StreamId_T, sp<IMetaStreamInfo> >
                                        vMetaStreamInfo;
    //
    mutable android::Mutex              mLock;

protected:  ////
    MINT32                              mOpenId;
    MINT32                              mLogLevel;
    MINT32                              mTimestamp;

};


};  //namespace NSLegacyPipeline
};  //namespace v1
};  //namespace NSCam
#endif  //_MTK_HARDWARE_MTKCAM_LEGACYPIPELINE_LEGACYPIPELINEBASE_H_


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

#ifndef _MTK_HARDWARE_MTKCAM_INCLUDE_MTKCAM_PIPELINE_PIPELINE_COMMONCAPTUREPIPELINE_H_
#define _MTK_HARDWARE_MTKCAM_INCLUDE_MTKCAM_PIPELINE_PIPELINE_COMMONCAPTUREPIPELINE_H_

// MTKCAM
#include <mtkcam/middleware/v1/ICommonCapturePipeline.h>

/******************************************************************************
 *
 ******************************************************************************/
namespace NSCam {


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Common Capture Pipeline
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
class CommonCapturePipeline
    : public ICommonCapturePipeline
{
public:
                       CommonCapturePipeline(PipelineConfig &config);
                       ~CommonCapturePipeline(){};

public:
    virtual MBOOL      isPipelineExist();
    virtual MBOOL      isNeedToReconstruct(
                           PipelineConfig &config
                       );

    virtual MERROR     reconfigPipeline(
                           PipelineConfig &config
                       );
    virtual MERROR     unregistAndReturnBufferToPool();

private:
    virtual ResultType checkAndUpdateImageStreamInfo(
                           sp<IImageStreamInfo>      pInfo_old,
                           sp<IImageStreamInfo>&     pInfo_new,
                           bool isRaw = false
                       );

    virtual MBOOL      checkLPBConfigParams(
                           PipelineConfig &config
                       );

    virtual MBOOL      checkAndUpdateSensorParams(
                           PipelineConfig &config
                       );

    virtual MBOOL      checkScenarioControlParams(
                           PipelineConfig &config
                       );

    virtual ResultType reconfigCallback(
                           PipelineConfig &config
                       );
    virtual MERROR     reconfigStreamInfo(
                           IStreamInfo::StreamId_T streamId,
                           PipelineConfig &config
                       );
    virtual ResultType reconfigBufferPool(
                           IStreamInfo::StreamId_T streamId,
                           PipelineConfig &config,
                           ResultType checkResult
                       );
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  RefBase Interfaces.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
protected:  ////         Operations.
    virtual void         onLastStrongRef(const void* id);

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Pipeline Control Interfaces.
//  for sp<NSCam::v1::NSLegacyPipeline::ILegacyPipeline> mpPipeline;
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:
    virtual MERROR                      flush(MBOOL cleanRss = MFALSE);

    virtual MERROR                      waitUntilDrained();

    virtual MERROR                      waitUntilDrainedAndFlush();

    virtual MERROR                      waitUntilP1DrainedAndFlush();

    virtual MERROR                      waitUntilP2JpegDrainedAndFlush();


    virtual wp<ResultProcessor>         getResultProcessor();

    virtual wp<TimestampProcessor>      getTimestampProcessor();

    virtual MERROR                      submitSetting(
                                            MINT32    const requestNo,
                                            IMetadata& appMeta,
                                            IMetadata& halMeta,
                                            v1::NSLegacyPipeline::ILegacyPipeline::ResultSet* pResultSet = nullptr
                                        );

    virtual MERROR                      submitRequest(
                                            MINT32    const   requestNo,
                                            IMetadata&        appMeta,
                                            IMetadata&        halMeta,
                                            Vector<BufferSet> vDstStreams,
                                            v1::NSLegacyPipeline::ILegacyPipeline::ResultSet* pResultSet = nullptr
                                        );

    virtual sp<IImageStreamInfo>        queryImageStreamInfo(StreamId const streamId);

    virtual sp<IMetaStreamInfo>         queryMetaStreamInfo(StreamId const streamId);

    virtual sp<StreamBufferProvider>    queryProvider(StreamId const streamId);

    virtual MVOID                       setRequestBuilder(
                                            sp<RequestBuilder> const pRequestBuilder);

    virtual MVOID                       getMetaIOMapInfo(
                                            KeyedVector<NSCam::v3::Pipeline_NodeId_T,
                                            NSCam::v3::NSPipelineContext::IOMapSet >& vInfo);

    virtual MERROR                      getFrameNo(MINT32 &frameNo);

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Interfaces for CallbackBufferPool
//  for sp<BufferCallbackHandler> mpCallbackHandler;
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:
    virtual android::sp<CallbackBufferPool>     queryBufferPool(StreamId_T id);

private:
    PipelineConfig       mCurPipelineConfig;
    mutable Mutex        mCCPipelineLock;
    mutable Condition    mCCPipelineCond;

    // pipeline and callback handler instance
    sp<NSCam::v1::NSLegacyPipeline::ILegacyPipeline> mpPipeline;
    sp<StreamBufferProviderFactory> mpFactory;
    sp<BufferCallbackHandler> mpCallbackHandler;
    sp<SImageCallback> mpImageCallback;
    sp<SMetadataListener> mpMetadataListener;
    sp<SMetadataListener> mpMetadataListenerFull;

    // reuse buffer pool
    sp<CallbackBufferPool> mpPool_FullRaw;
    sp<CallbackBufferPool> mpPool_ResizedRaw;
    sp<CallbackBufferPool> mpPool_LcsoRaw;
    sp<CallbackBufferPool> mpPool_Yuv;
    sp<CallbackBufferPool> mpPool_YuvPostview;
    sp<CallbackBufferPool> mpPool_YuvThumbnail;
    sp<CallbackBufferPool> mpPool_Jpeg;

    // debug flag
    MUINT32                mDebug;
};

/******************************************************************************
 *
 ******************************************************************************/
};  //namespace NSCam
#endif  //_MTK_HARDWARE_MTKCAM_INCLUDE_MTKCAM_PIPELINE_PIPELINE_COMMONCAPTUREPIPELINE_H_


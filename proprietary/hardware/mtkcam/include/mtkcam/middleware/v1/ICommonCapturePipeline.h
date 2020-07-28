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

#ifndef _MTK_HARDWARE_MTKCAM_INCLUDE_PIPELINE_PIPELINE_ICOMMONCAPTUREPIPELINE_H_
#define _MTK_HARDWARE_MTKCAM_INCLUDE_PIPELINE_PIPELINE_ICOMMONCAPTUREPIPELINE_H_
// AOSP
#include <utils/String8.h>
#include <utils/RefBase.h>
// MTKCAM
#include <mtkcam/def/common.h>
#include <mtkcam/middleware/v1/LegacyPipeline/StreamId.h>
#include <mtkcam/middleware/v1/LegacyPipeline/ILegacyPipeline.h>
#include <mtkcam/middleware/v1/LegacyPipeline/IResourceContainer.h>
#include <mtkcam/middleware/v1/LegacyPipeline/LegacyPipelineUtils.h>
#include <mtkcam/middleware/v1/LegacyPipeline/LegacyPipelineBuilder.h>
#include <mtkcam/middleware/v1/LegacyPipeline/buffer/StreamBufferProviderFactory.h>
#include <mtkcam/middleware/v1/LegacyPipeline/buffer/Selector.h>
#include <mtkcam/middleware/v1/LegacyPipeline/buffer/BufferPoolImp.h>
#include <mtkcam/middleware/v1/camshot/BufferCallbackHandler.h>

using namespace android;

/******************************************************************************
 *
 ******************************************************************************/
namespace NSCam {

/******************************************************************************
 *
 ******************************************************************************/
class SCallbackListener
{
public:
    virtual         ~SCallbackListener() {}
public:
    virtual MVOID   onMetaReceived(
                        MUINT32         const requestNo,
                        StreamId_T      const streamId,
                        MBOOL           const errorResult,
                        IMetadata       const result
                    )                                       = 0;
    virtual MVOID   onDataReceived(
                        MUINT32 const requestNo,
                        StreamId_T const streamId,
                        MBOOL const errorResult,
                        android::sp<IImageBuffer>& pBuffer
                    )                                       = 0;
};

/*******************************************************************************
*
********************************************************************************/
class SImageCallback
    : public IImageCallback
{
public:
                                        SImageCallback(
                                            SCallbackListener* pListener,
                                            MUINT32 const data
                                            )
                                            : mpListener(pListener)
                                            , mData(data)
                                        {}
public:   ////    interface of IImageCallback
    /**
     *
     * Received result buffer.
     *
     * @param[in] RequestNo : request number.
     *
     * @param[in] pBuffer : IImageBuffer.
     *
     */
    virtual MERROR                      onResultReceived(
                                            MUINT32 const              RequestNo,
                                            StreamId_T const           streamId,
                                            MBOOL   const              errorBuffer,
                                            android::sp<IImageBuffer>& pBuffer
                                        ) {
                                            if( mpListener )
                                                mpListener->onDataReceived(
                                                    RequestNo, streamId, errorBuffer, pBuffer
                                                    );
                                            return OK;
                                        }
protected:
    SCallbackListener*                  mpListener;
    MUINT32 const                       mData;
};

/*******************************************************************************
*
********************************************************************************/
class SMetadataListener
    : public ResultProcessor::IListener
{
public:
                                        SMetadataListener(
                                            SCallbackListener* pListener
                                            )
                                            : mpListener(pListener)
                                        {}
public:   ////    interface of IListener
    virtual void                        onResultReceived(
                                            MUINT32         const requestNo,
                                            StreamId_T      const streamId,
                                            MBOOL           const errorResult,
                                            IMetadata       const result
                                        ) {
                                            if( mpListener )
                                                mpListener->onMetaReceived(
                                                    requestNo, streamId, errorResult, result
                                                    );
                                        }

    virtual void                       onFrameEnd(
                                            MUINT32         const /*requestNo*/
                                        ) {};

    virtual String8                     getUserName() { return String8(LOG_TAG); }

protected:
    SCallbackListener*                  mpListener;
};
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Common Capture Pipeline create & query interfaces
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
class ICommonCapturePipeline
    : public virtual android::RefBase
{
public:
    enum ResultType {
        RESULT_OK = 0,
        RESULT_NO_NEED_CHANGE = 0,
        // buffer pool result
        RESULT_RECONFIG_STREAM_INFO,
        RESULT_ADD_BUFFER,
        RESULT_RELOAD_ALL_BUFFER,
        RESULT_RECONFIG_BUFFER_POOL,
        // callback result
        RESULT_RECONFIG_CALLBACK,

        RESULT_UNKNOWN_ERROR = 0xFF
    };

    struct PipelineConfig
    {
        String8    userName;
        MINT32     openId;
        MBOOL      isZsdMode;
        // non-zsd requirement
        PipelineSensorParam       sensorParam;
        IScenarioControl::ControlParam scenarioControlParam;
        // all requirement
        sp<IImageStreamInfo>      pInfo_FullRaw;
        sp<IImageStreamInfo>      pInfo_ResizedRaw;
        sp<IImageStreamInfo>      pInfo_LcsoRaw;
        sp<IImageStreamInfo>      pInfo_Yuv;
        sp<IImageStreamInfo>      pInfo_YuvPostview;
        sp<IImageStreamInfo>      pInfo_YuvThumbnail;
        sp<IImageStreamInfo>      pInfo_Jpeg;
        SCallbackListener         *pCallbackListener;
        LegacyPipelineBuilder::ConfigParams LPBConfigParams;
        PipelineConfig()
            :userName("Unknow"),
             openId(-1),
             isZsdMode(MTRUE),
             // non-zsd requirement
             sensorParam(PipelineSensorParam()),
             scenarioControlParam(IScenarioControl::ControlParam()),
             // all requirement
             pInfo_FullRaw(nullptr),
             pInfo_ResizedRaw(nullptr),
             pInfo_LcsoRaw(nullptr),
             pInfo_Yuv(nullptr),
             pInfo_YuvPostview(nullptr),
             pInfo_YuvThumbnail(nullptr),
             pInfo_Jpeg(nullptr),
             pCallbackListener(nullptr),
             LPBConfigParams(LegacyPipelineBuilder::ConfigParams())
        {}
    };

public:
    // for flow control developer
    static sp<ICommonCapturePipeline> createCommonCapturePipeline(
                     MINT32 openId,
                     sp<IParamsManagerV3> pParamsManagerV3
                 );

    static MBOOL removeCommonCapturePipeline(
                     MINT32 openId
                 );
    static MERROR getDefaultConfig(
                     MINT32 openId,
                     sp<IParamsManagerV3> pParamsManagerV3,
                     PipelineConfig &config
                 );
    static MERROR returnCommonCapturePipeline(
                     MINT32 openId
                 );

    // for shot developer
    static sp<ICommonCapturePipeline> queryCommonCapturePipeline(
                     PipelineConfig &config
                 );

    // for debug
    static void dumpPipelineConfig(
                     PipelineConfig &config
                );
    static void dumpImageStreamInfo(
                     sp<IImageStreamInfo> pInfo,
                     const char* pTxt = nullptr
                );

private:
    static MBOOL removeCommonCapturePipelineNoLock(
                     MINT32 openId
                 );
    static void addPipelineToMap(
                     MINT32 openId,
                     sp<ICommonCapturePipeline> pCCPipeline
                 );
    static void removePipelineFromMap(
                     MINT32 openId
                 );
    static void addUserToMap(
                     MINT32 openId,
                     String8 userName
                 );
    static void removeUserFromMap(
                     MINT32 openId
                 );
    static sp<ICommonCapturePipeline> getPipelineFromMap(
                     MINT32 openId
                 );

    // for debug
    static void dumpUsage();

public:
    virtual MBOOL      isPipelineExist()                       = 0;
    virtual MBOOL      isNeedToReconstruct(
                           PipelineConfig &config
                       )                                       = 0;
    virtual MERROR     reconfigPipeline(
                           PipelineConfig &config
                       )                                       = 0;

    // for shot developer
    virtual MERROR     unregistAndReturnBufferToPool()         = 0;

private:
    virtual ResultType checkAndUpdateImageStreamInfo(
                           sp<IImageStreamInfo>      pInfo_old,
                           sp<IImageStreamInfo>&     pInfo_new,
                           bool isRaw = false
                       )                                       = 0;
    virtual MBOOL      checkLPBConfigParams(
                           PipelineConfig &config
                       )                                       = 0;
    virtual MBOOL      checkAndUpdateSensorParams(
                           PipelineConfig &config
                       )                                       = 0;
    virtual MBOOL      checkScenarioControlParams(
                           PipelineConfig &config
                       )                                       = 0;
    virtual ResultType reconfigCallback(
                           PipelineConfig &config
                       )                                       = 0;
    virtual MERROR     reconfigStreamInfo(
                           IStreamInfo::StreamId_T streamId,
                           PipelineConfig &config
                       )                                       = 0;
    virtual ResultType reconfigBufferPool(
                           IStreamInfo::StreamId_T streamId,
                           PipelineConfig &config,
                           ResultType checkResult
                       )                                       = 0;

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Interfaces for pipeline control
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:
    virtual MERROR                      flush(MBOOL cleanRss = MFALSE)=0;

    virtual MERROR                      waitUntilDrained()=0;

    virtual MERROR                      waitUntilDrainedAndFlush()=0;

    virtual MERROR                      waitUntilP1DrainedAndFlush() {return OK;}

    virtual MERROR                      waitUntilP2JpegDrainedAndFlush() { return OK;}


    virtual wp<ResultProcessor>         getResultProcessor() = 0;

    virtual wp<TimestampProcessor>      getTimestampProcessor() = 0;

    virtual MERROR                      submitSetting(
                                            MINT32    const requestNo,
                                            IMetadata& appMeta,
                                            IMetadata& halMeta,
                                            v1::NSLegacyPipeline::ILegacyPipeline::ResultSet* pResultSet = nullptr
                                        ) = 0;

    virtual MERROR                      submitRequest(
                                            MINT32    const   requestNo,
                                            IMetadata&        appMeta,
                                            IMetadata&        halMeta,
                                            Vector<BufferSet> vDstStreams,
                                            v1::NSLegacyPipeline::ILegacyPipeline::ResultSet* pResultSet = nullptr
                                        ) = 0;

    virtual sp<IImageStreamInfo>        queryImageStreamInfo(StreamId const streamId) = 0;

    virtual sp<IMetaStreamInfo>         queryMetaStreamInfo(StreamId const streamId) = 0;

    virtual sp<StreamBufferProvider>    queryProvider(StreamId const streamId) = 0;

    virtual MVOID                       setRequestBuilder(
                                            sp<RequestBuilder> const pRequestBuilder) = 0;

    virtual MVOID                       getMetaIOMapInfo(
                                            KeyedVector<NSCam::v3::Pipeline_NodeId_T,
                                            NSCam::v3::NSPipelineContext::IOMapSet >& vInfo) = 0;

    virtual MERROR                      getFrameNo(MINT32 &frameNo) = 0;

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Interfaces for CallbackBufferPool
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    virtual android::sp<CallbackBufferPool>     queryBufferPool(StreamId_T id) = 0;

};

};  //namespace NSCam
#endif  //_MTK_HARDWARE_MTKCAM_INCLUDE_PIPELINE_PIPELINE_ICOMMONCAPTUREPIPELINE_H_

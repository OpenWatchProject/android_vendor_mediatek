/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 */
/* MediaTek Inc. (C) 2015. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER ON
 * AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
 * NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
 * SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
 * SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES TO LOOK ONLY TO SUCH
 * THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. RECEIVER EXPRESSLY ACKNOWLEDGES
 * THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES
 * CONTAINED IN MEDIATEK SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK
 * SOFTWARE RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND
 * CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,
 * AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE,
 * OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY RECEIVER TO
 * MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek Software")
 * have been modified by MediaTek Inc. All revisions are subject to any receiver's
 * applicable license agreements with MediaTek Inc.
 */

#ifndef _MTK_HARDWARE_MTKCAM_LEGACYPIPELINE_LEGACYPIPELINE_MFC_H_
#define _MTK_HARDWARE_MTKCAM_LEGACYPIPELINE_LEGACYPIPELINE_MFC_H_

#include "Defs.h"
#include <mtkcam/middleware/v1/LegacyPipeline/ILegacyPipeline.h>

// ---------------------------------------------------------------------------

namespace NS3Av3 {
    class CaptureParam_T;
};

// ---------------------------------------------------------------------------

namespace NSCam {
namespace v1 {
namespace NSLegacyPipeline {

class LegacyPipelineMFC
    : public ILegacyPipeline
{
public:
    LegacyPipelineMFC();
    virtual ~LegacyPipelineMFC();

    /* ------------------------------------------------------------------------
     * ILegacyPipeline interface
     */
    inline MERROR flush()
    {
        return mPipelineContext.get() ?
            mPipelineContext->flush() : OK;
    }

    inline MERROR waitUntilDrained()
    {
        return mPipelineContext.get() ?
            mPipelineContext->waitUntilDrained() : OK;
    }

    MERROR waitUntilDrainedAndFlush();

    inline wp<ResultProcessor> getResultProcessor()
    {
        return mResultProcessor;
    }

    inline wp<TimestampProcessor> getTimestampProcessor()
    {
        return mTimestampProcessor;
    }


    virtual MERROR getFrameNo( MINT32 &frameNo){ /* not support */  return UNKNOWN_ERROR; }

    MERROR submitSetting(
            MINT32    const requestNo,
            IMetadata& appMeta,
            IMetadata& halMeta,
            ResultSet* pResultSet);

    MERROR submitRequest(
            MINT32    const requestNo,
            IMetadata& appMeta,
            IMetadata& halMeta,
            Vector<BufferSet> vDstStreams,
            ResultSet* pResultSet);

    inline sp<IImageStreamInfo> queryImageStreamInfo(StreamId const streamId)
    {
        return mvImageStreamInfo.editValueFor(streamId);
    }

    inline sp<IMetaStreamInfo> queryMetaStreamInfo(StreamId const streamId)
    {
        return mvMetaStreamInfo.editValueFor(streamId);
    }

    inline sp<StreamBufferProvider> queryProvider(StreamId const streamId)
    {
        return mvStreamBufferProvider.editValueFor(streamId);
    }

    inline MVOID setRequestBuilder(sp<RequestBuilder> const pRequestBuilder);

    inline MVOID getMetaIOMapInfo(KeyedVector<NSCam::v3::Pipeline_NodeId_T,
            NSCam::v3::NSPipelineContext::IOMapSet >&) {}

    /* ------------------------------------------------------------------------
     * RefBase interface
     */
    MVOID onLastStrongRef(const void* id);

    /* ------------------------------------------------------------------------
     * designed for legacy pipeline builder
     */
    inline MVOID setPipelineContext(
            const sp<PipelineContext>& pPipelineContext)
    {
        mPipelineContext = pPipelineContext;
    }

    inline MVOID setRequestBuilder(
            const Vector< sp<RequestBuilder> >& pvRequestBuilder)
    {
        mvRequestBuilder = pvRequestBuilder;
    }

    inline MVOID setResultProcessor(
            const sp<ResultProcessor>& pResultProcessor)
    {
        mResultProcessor = pResultProcessor;
    }

    inline MVOID setTimestampProcessor(
            const sp<TimestampProcessor>& pTimestampProcessor)
    {
        mTimestampProcessor = pTimestampProcessor;
    }

    MVOID setConfiguration(const sp<Configuration>& config);

    MVOID setImageStreamInfo(
            const sp<IImageStreamInfo>& pImageStreamInfo);

    MVOID setMetaStreamInfo(
            const sp<IMetaStreamInfo>& pMetaStreamInfo);

    MVOID setStreamBufferProvider(
            const sp<StreamBufferProvider>& pStreamBufferProvider);

private:
    sp<PipelineContext> mPipelineContext;
    sp<ResultProcessor> mResultProcessor;
    sp<TimestampProcessor> mTimestampProcessor;
    Vector< sp<RequestBuilder> >   mvRequestBuilder;
    sp<Configuration>   mConfiguration;
    MINT32 mTimestamp;

    KeyedVector< StreamId_T, sp<IImageStreamInfo> >
        mvImageStreamInfo;
    KeyedVector< StreamId_T, sp<IMetaStreamInfo> >
        mvMetaStreamInfo;
    KeyedVector< StreamId_T, sp<StreamBufferProvider> >
        mvStreamBufferProvider;

    sp<IPipelineFrame> createRequest(
            const MINT32 requestNo,
            const IMetadata& appMeta,
            const IMetadata& halMeta);

    template <typename TNode>
    MERROR drainAndFlushNode(const NodeId_T nodeId);
};

} // NSLegacyPipeline
} // namespace v1
} // namespace NSCam

#endif // _MTK_HARDWARE_MTKCAM_LEGACYPIPELINE_LEGACYPIPELINE_MFC_H_

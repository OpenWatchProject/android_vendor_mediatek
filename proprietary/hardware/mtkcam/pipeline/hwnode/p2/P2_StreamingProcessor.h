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

#ifndef _MTKCAM_HWNODE_P2_STREAMING_PROCESSOR_H_
#define _MTKCAM_HWNODE_P2_STREAMING_PROCESSOR_H_

#include <mtkcam/feature/featurePipe/IStreamingFeaturePipe.h>
#include <mtkcam/feature/3dnr/util_3dnr.h>

#include "P2_Processor.h"
#include "P2_Util.h"

namespace P2
{

class StreamingProcessor : virtual public Processor<P2InitParam, P2ConfigParam, sp<P2Request>>
{
public:
    StreamingProcessor();
    virtual ~StreamingProcessor();

protected:
    virtual MBOOL onInit(const P2InitParam &param);
    virtual MVOID onUninit();
    virtual MVOID onThreadStart();
    virtual MVOID onThreadStop();
    virtual MBOOL onConfig(const P2ConfigParam &param);
    virtual MBOOL onEnque(const sp<P2Request> &request);
    virtual MVOID onNotifyFlush();
    virtual MVOID onWaitFlush();
    virtual MVOID onIdle();

private:
    class Payload;
    typedef NSCam::NSCamFeature::NSFeaturePipe::FeaturePipeParam FeaturePipeParam;

private:
    NSCam::NSCamFeature::NSFeaturePipe::IStreamingFeaturePipe::UsageHint getFeatureUsageHint(const P2ConfigParam &config);
    MBOOL needReConfig(const P2ConfigParam &oldConfig, const P2ConfigParam &newConfig);
    MBOOL initFeaturePipe(const P2ConfigParam &config);
    MVOID uninitFeaturePipe();
    MBOOL init3A();
    MVOID uninit3A();
    MVOID process3A(const sp<Payload> &payload, P2MetaSet &metaset);
    MVOID prepareFeatureParam(const sp<Payload> &payload);
    MBOOL processP2(sp<Payload> &payload);
    MVOID onFPipeCB(FeaturePipeParam::MSG_TYPE msg, FeaturePipeParam &param, const sp<Payload> &payload);
    static MBOOL sFPipeCB(FeaturePipeParam::MSG_TYPE msg, FeaturePipeParam &param);

    MVOID incPayloadCount(const Logger &logger);
    MVOID decPayloadCount(const Logger &logger);

private:
    sp<P2Info> mP2Info;
    P2ConfigParam mConfigParam;
    NSCam::NSCamFeature::NSFeaturePipe::IStreamingFeaturePipe *mFeaturePipe;
    IHal3A_T *mHal3A;
    MBOOL mConfigVencStream;
    MUINT32 mTuningSize;
    android::Mutex mRssoHolderMutex;
    sp<P2Img> mRssoHolder;

private:
    class Payload : virtual public android::RefBase, virtual public LoggerHolder
    {
    public:
        Payload(StreamingProcessor *parent, const sp<P2Request> &request);
        virtual ~Payload();
        StreamingProcessor *mParent;
        sp<P2Request> mRequest;
        FeaturePipeParam mFeatureParam;
        TuningParam mTuning;
        P2Util::SimpleIO mIO;
        sp<P2Img> mCurrRsso;
        sp<P2Img> mPrevRsso;
    };

    android::Mutex mPayloadCountMutex;
    MUINT32 mPayloadCount = 0;
    android::Mutex mPayloadMutex;
    android::Condition mPayloadCondition;
    std::list<sp<Payload>> mPayloadList;

    MVOID incPayload(const sp<Payload> &payload);
    MBOOL decPayload(FeaturePipeParam &param, sp<Payload> &payload, MBOOL checkOrder);
    MVOID waitFeaturePipeDone();
    sp<P2Img> updateRsso(const sp<P2Img> &rsso);

private:
    NSCam::NSCamFeature::NSFeaturePipe::IStreamingFeaturePipe::UsageHint mPipeUsageHint;
    MINT32 m3dnrDebugLevel;
    android::sp<Util3dnr> mUtil3dnr;

    MBOOL prepareCommon(const sp<Payload> &payload);

    void init3DNR();
    void uninit3DNR();
    MBOOL isAPEnabled_3DNR(MINT32 force3DNR, IMetadata *appInMeta, IMetadata *halInMeta);
    MBOOL getInputCrop3DNR(
        MBOOL &isEIS4K, MBOOL &isIMGO, MRect &inputCrop,
        const sp<Payload> &payload, P2MetaSet &metaSet, const Logger &logger);
    MBOOL prepare3DNR(const sp<Payload> &payload, P2MetaSet &metaSet, const Logger &logger);
    MBOOL prepare3DNR_FeatureData(
        MBOOL appEn3DNR, MBOOL isEIS4K, MBOOL isIMGO,
        const sp<Payload> &payload, P2MetaSet &metaSet, MUINT8 ispProfile, const Logger &logger);
    MVOID prepare3DNR_ISPData(
        MBOOL canEnable3dnr, const MRect &inputCrop,
        const sp<Payload> &payload, P2MetaSet &metaSet, MUINT8 ispProfile, const Logger &logger);
    MINT32 get3DNRIsoThreshold(MUINT8 ispProfile);

    MBOOL isEIS12() const;
    MBOOL isAdvEIS() const;
    MBOOL prepareEIS(const sp<Payload> &payload);
    MRectF getEISRRZOMargin(const Cropper &cropper);
    MBOOL prepareEISVar(FeaturePipeParam &featureParam, const sp<P2Request> &request, const Cropper &cropper, const MRectF &eisMargin);
    MBOOL getEISExpTime(MINT32 &expTime, MINT32 &longExpTime, const LMVInfo &lmvInfo, const sp<P2Meta> &inHal);
    MBOOL prepareEISMask(FeaturePipeParam &featureParam);
    MBOOL prepareEISOutMeta(P2MetaSet& metaSet);

    MBOOL isNeedRSSO(const sp<Payload> &payload);
    MBOOL prepareRSSO(const sp<Payload> &payload);
    MINT64 getTSQ(const sp<Payload> &payload, const FeaturePipeParam &param);
    MVOID processTSQ(const sp<P2Request> &request, MINT64 ts = 0);
};

} // namespace P2

#endif // _MTKCAM_HWNODE_P2_STREAMING_PROCESSOR_H_

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

#ifndef _MTKCAM_HWNODE_P2_CAPTURE_PROCESSOR_H_
#define _MTKCAM_HWNODE_P2_CAPTURE_PROCESSOR_H_

#include "P2_Processor.h"
#include "P2_Util.h"
#include <mtkcam/utils/std/ULog.h>


#include <mtkcam3/feature/featurePipe/ICaptureFeaturePipe.h>
// feature index for pipeline plugin
#include <mtkcam3/3rdparty/mtk/mtk_feature_type.h>
#include <mtkcam3/3rdparty/customer/customer_feature_type.h>

using NSCam::NSCamFeature::NSFeaturePipe::NSCapture::ICaptureFeaturePipe;
using NSCam::NSCamFeature::NSFeaturePipe::NSCapture::RequestCallback;
using NSCam::NSCamFeature::NSFeaturePipe::NSCapture::ICaptureFeatureRequest;

namespace P2
{
class CaptureProcessor;
class CaptureRequestCallback
    : virtual public RequestCallback
{
public:
    CaptureRequestCallback(CaptureProcessor*);

    virtual void onMetaResultAvailable(android::sp<ICaptureFeatureRequest> pCapRequest, IMetadata* partialMeta);
    virtual void onContinue(sp<ICaptureFeatureRequest> pCapRequest);
    virtual void onAborted(sp<ICaptureFeatureRequest> pCapRequest);
    virtual void onRestored(sp<ICaptureFeatureRequest> pCapRequest);
    virtual void onCompleted(sp<ICaptureFeatureRequest> pCapRequest, NSCam::MERROR);
    virtual ~CaptureRequestCallback() { };

private:
    CaptureProcessor* mpProcessor;
};

class CaptureProcessor
    : virtual public Processor<P2InitParam, P2ConfigParam, sp<P2FrameRequest>>
{
friend CaptureRequestCallback;

public:
    CaptureProcessor();
    virtual ~CaptureProcessor();

public:
    virtual MVOID notifyRelease(MINT32 requestNo, MINT32 frameNo);
    virtual MVOID releaseImage(sp<P2Request> pRequest, ID_IMG imgId);
    virtual MVOID releaseMeta(sp<P2Request> pRequest, ID_META metaId);

protected:
    virtual MBOOL onInit(const P2InitParam &param);
    virtual MVOID onUninit();
    virtual MVOID onThreadStart();
    virtual MVOID onThreadStop();
    virtual MBOOL onConfig(const P2ConfigParam &param);
    virtual MBOOL onEnque(const sp<P2FrameRequest> &request);
    virtual MVOID onNotifyFlush();
    virtual MVOID onWaitFlush();
    virtual std::string onGetStatus();
    virtual MVOID abortRequest(MINT32 requestNo);


private:
    MVOID                   waitRequestDone();
    MVOID                   waitRequestAborted();

protected:

private:
    using CaptureFeaturePipeTable = std::map<MUINT32, sp<ICaptureFeaturePipe>>;

private:
    struct RequestPair {
        sp<IP2Frame>                            mpP2Frame;
        sp<P2Request>                           mpDetachP2Request;
        sp<ICaptureFeatureRequest>              mpCapRequest;
        MBOOL                                   mAborting       = MFALSE;
        MBOOL                                   mRestored       = MFALSE;
        MBOOL                                   mDetached       = MFALSE;
    };

    sp<RequestCallback>                         mpCallback;
    Vector<RequestPair>                         mRequestPairs;

    mutable Mutex                               mPairLock;
    android::Condition                          mPairCondition;

    ILog                                        mLog;
    P2Info                                      mP2Info;
    sp<ICaptureFeaturePipe>                     mpFeaturePipe;

    MINT32                                      mDebugDrop;
    MINT32                                      mAbortingRequestNo;
    MINT32                                      mLastRequestNo;
    MINT32                                      mLastFrameCount;
    MINT32                                      mLastFrameIndex;
    MUINT32                                     mLastMasterIndex;
};

} // namespace P2

#endif // _MTKCAM_HWNODE_P2_CAPTURE_PROCESSOR_H_

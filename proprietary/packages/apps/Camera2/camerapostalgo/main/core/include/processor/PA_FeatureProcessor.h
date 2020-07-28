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

#include "Processor.h"
#include <header_base.h>
#include <IFeaturePipe.h>
#include <IStreamManager.h>
#include "FeatureSettingPolicy.h"
#include "PA_Param.h"
#include "PA_Request.h"
#include <utils/metadata/IMetadata.h>
#include <mtk/mtk_platform_metadata_tag.h>

using com::mediatek::campostalgo::NSFeaturePipe::IFeaturePipe;
using com::mediatek::campostalgo::NSFeaturePipe::RequestCallback;
using com::mediatek::campostalgo::NSFeaturePipe::IFeatureRequest;


namespace NSPA
{

struct  ConfigParams
{
    ConfigParams()
        : uCustomOption(0)
        {};
#if 0
    /**
     * A pointer to a set of input app meta stream info.
     */
    android::sp<IMetaStreamInfo>  pInAppMeta;

    /**
     * A pointer to a set of input app result meta stream info.
     */
    android::sp<IMetaStreamInfo>  pInAppRetMeta;
    android::sp<IMetaStreamInfo>  pInAppRetMeta2;

    /**
     * A pointer to a set of input hal meta stream info.
     */
    android::sp<IMetaStreamInfo>  pInHalMeta;
    android::sp<IMetaStreamInfo>  pInHalMeta2;

    /**
     * A pointer to a set of output app meta stream info.
     */
    android::sp<IMetaStreamInfo>  pOutAppMeta;

    /**
     * A pointer to a set of output hal meta stream info.
     */
    android::sp<IMetaStreamInfo>  pOutHalMeta;

    /**
     * A pointer to a full-size raw input image stream info.
     */
    android::sp<IImageStreamInfo> pInFullRaw;
    android::sp<IImageStreamInfo> pInFullRaw2;

    /**
     * A pointer to a full-size raw input image stream info.
     */
    android::sp<IImageStreamInfo> pInResizedRaw;
    android::sp<IImageStreamInfo> pInResizedRaw2;

    /**
     * A pointer to input image stream info. (full-zsl input port)
     */
    android::Vector<android::sp<IImageStreamInfo>>
                                  vpInOpaqueRaws;

    /**
     * A pointer to YUV reprocessing image stream info.
     */
    android::sp<IImageStreamInfo> pInFullYuv;

    /**
     * A pointer to a lcso raw input image stream info.
     */
    android::sp<IImageStreamInfo> pInLcsoRaw;
    android::sp<IImageStreamInfo> pInLcsoRaw2;

    /**
     * A set of pointer to output image stream info.
     */
    android::Vector<android::sp<IImageStreamInfo>>
                                  vpOutImages;

    /**
     * A pointer to JPEG YUV image stream info.
     */
    android::sp<IImageStreamInfo> pOutJpegYuv;

    /**
     * A pointer to post view image stream info.
     */
    android::sp<IImageStreamInfo> pOutPostViewYuv;

    /**
     * A pointer to clean image stream info.
     */
    android::sp<IImageStreamInfo> pOutCleanYuv;

    /**
     * A pointer to depth stream info.
     */
    android::sp<IImageStreamInfo> pOutDepth;

    /**
     * A pointer to thumbnail image stream info.
     */
    android::sp<IImageStreamInfo> pOutThumbnailYuv;
#endif
    /**
     * customize option
     */
    MUINT32 uCustomOption;
};

class FeatureProcessor;

class CaptureRequestCallback
    : virtual public RequestCallback
{
public:
    CaptureRequestCallback(FeatureProcessor*);

    virtual void onContinue(sp<IFeatureRequest> pCapRequest);
    virtual void onAborted(sp<IFeatureRequest> pCapRequest);
    virtual void onRestored(sp<IFeatureRequest> pCapRequest);
    virtual void onCompleted(sp<IFeatureRequest> pCapRequest, int);
    virtual ~CaptureRequestCallback() { };

private:
    FeatureProcessor* mpProcessor;
};

class FeatureProcessor
    : virtual public Processor<FeatureInitParam, FeatureConfigParam, sp<PA_Request>>
{
friend CaptureRequestCallback;

public:
    FeatureProcessor(const std::string& name);
    virtual ~FeatureProcessor();

public:
    virtual MVOID notifyRelease(MINT32 requestNo, MINT32 frameNo);
    virtual MVOID releaseImage(sp<PA_Request> pRequest, ID_IMG imgId);
    virtual MVOID releaseMeta(sp<PA_Request> pRequest, ID_META metaId);

protected:
    virtual MBOOL onInit(const FeatureInitParam &param);
    virtual MVOID onUninit();
    virtual MVOID onThreadStart();
    virtual MVOID onThreadStop();
    virtual MBOOL onConfig(const FeatureConfigParam &param);
    virtual MBOOL onEnque(const sp<PA_Request> &request);
    virtual MVOID onNotifyFlush();
    virtual MVOID onWaitFlush();
    virtual std::string onGetStatus();
    virtual MVOID abortRequest(MINT32 requestNo);
    status_t evaluateConfiguration(const FeatureConfigParam &param);
    status_t evaluateRequest(const sp<PA_Request>& request);
    status_t evaluateMetadata(const sp<PA_Request>& request);
    status_t evaluateIOBuffer(const sp<PA_Request>& request);
//    MVOID setupIOBufferInPlace(const sp<PA_Request>& request);
    FeatureFeatureID convertAlgoType2FeatureId(AlgoType type);


private:
    MVOID                   waitRequestDone();
    MVOID                   waitRequestAborted();
    sp<IFeaturePipe> getFeaturePipe(MINT32 sensorId);

protected:

private:
//    using CaptureFeaturePipeTable = std::map<MINT32, sp<IFeaturePipe>>;
    using CaptureFeaturePipeTable = std::vector<sp<IFeaturePipe>>;

private:
    /*struct RequestPair {
        sp<IP2Frame>                            mpP2Frame;
        sp<P2Request>                           mpDetachP2Request;
        sp<IFeatureRequest>              mpCapRequest;
        MBOOL                                   mAborting       = MFALSE;
        MBOOL                                   mRestored       = MFALSE;
        MBOOL                                   mDetached       = MFALSE;
    };*/

    struct RequestPair {
    };

    wp<IStreamManager> mpStreamManager = nullptr;
    sp<FeatureSettingPolicy> mpFeatureSettingPolicy = nullptr;
    sp<RequestCallback>                         mpCallback;

//    mutable Mutex                               mPairLock;
//    android::Condition                          mPairCondition;

    CaptureFeaturePipeTable                     mFeaturePipeTable;

    MINT32                                      mDebugDrop;
    MINT32                                      mAbortingRequestNo;
    MINT32                                      mLastRequestNo;
    MINT32                                      mLastFrameCount;
    MINT32                                      mLastFrameIndex;
    MINT32                                      mLastMasterIndex;

    mutable Mutex mConfigInfoLock;
    StreamType                                  mType;
    int32_t                                     mInterfaceId;
    mutable Mutex                               mRequestLock;
    std::unordered_map<uint32_t, sp<PA_Request>> mRequests;
    std::vector<AlgoType> mStreamAlgos;

    //sp<FeatureParam> mpFeatureParam;

    sp<MetadataPack> mConfigMeta;
    enum {
        PAS_AUTORAMA_KNOWN = -1,
        PAS_AUTORAMA_PROCESS,
        PAS_AUTORAMA_MERGE
    };

    volatile int32_t mAutoRamaState = AUTORAMA_UNKNOWN;
    volatile int32_t mJpegOrientation = 0;

};

} // namespace NSPA

#endif // _MTKCAM_HWNODE_P2_CAPTURE_PROCESSOR_H_

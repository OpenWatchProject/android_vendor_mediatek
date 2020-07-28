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
 * MediaTek Inc. (C) 2018. All rights reserved.
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

#ifndef _MTK_CAMERA_STREAMING_FEATURE_PIPE_DEPTH_NODE_H_
#define _MTK_CAMERA_STREAMING_FEATURE_PIPE_DEPTH_NODE_H_

//
#define LOG_TAG "DepthNode"
#include <mtkcam/utils/metadata/hal/mtk_platform_metadata_tag.h>
#include <mtkcam3/feature/stereo/pipe/IDepthMapPipe.h>
#include <mtkcam3/feature/stereo/pipe/IDepthMapEffectRequest.h>
#include <mtkcam3/feature/stereo/pipe/IDualFeatureRequest.h>
#if (SUPPORT_ISP_VER == 60)
#include <mtkcam3/feature/stereo/pipe/DepthPipeHolder.h>
#endif
//
#include "StreamingFeatureNode.h"
#include "NullNode.h"

//=======================================================================================
#if SUPPORT_VSDOF
//=======================================================================================
namespace NSCam {
namespace NSCamFeature {
namespace NSFeaturePipe {

using namespace NSFeaturePipe_DepthMap;

struct inputImgData{
    NodeBufferSetting    param;
    NSCam::IImageBuffer* buf;
};
struct inputMetaData{
    NodeBufferSetting param;
    IMetadata*        meta;
};

class DepthEnqueData
{
public:
    RequestPtr mRequest;
    DepthImg   mOut;
};

class DepthNode : public StreamingFeatureNode
{
public:
    DepthNode(const char *name);
    virtual ~DepthNode();

    MVOID setOutputBufferPool(const android::sp<IBufferPool> &pool, MUINT32 allocate = 0);

    MVOID onFlush();
public:
    virtual MBOOL onData(DataID id, const RequestPtr &data);
    virtual IOPolicyType getIOPolicy(StreamType stream, const StreamingReqInfo &reqInfo) const;

protected:
    virtual MBOOL onInit();
    virtual MBOOL onUninit();
    virtual MBOOL onThreadStart();
    virtual MBOOL onThreadStop();
    virtual MBOOL onThreadLoop();

private:
    MVOID handleResultData(const RequestPtr &request, const DepthEnqueData &data);

    WaitQueue<RequestPtr> mRequests;
    MUINT32 mYuvImgPoolAllocateNeed      = 0;
    MUINT32 mDMBGImgPoolAllocateNeed     = 0;
    MUINT32 mDepthMapImgPoolAllocateNeed = 0;
    android::sp<IBufferPool> mYuvImgPool      = NULL;
    android::sp<IBufferPool> mDMBGImgPool     = NULL;
    android::sp<IBufferPool> mDepthMapImgPool = NULL;
#if (SUPPORT_ISP_VER == 60)
    android::sp<SmartDepthMapPipe> mpDepthMapPipe;
#else
    IDepthMapPipe* mpDepthMapPipe = nullptr;
#endif

    static MVOID onPipeReady(MVOID* tag, NSDualFeature::ResultState state,
                                        sp<IDualFeatureRequest>& request);

    MUINT16 fillIntoDepthMapPipe(sp<IDepthMapEffectRequest> pDepMapReq,
                                 vector<inputImgData>& vImgs, vector<inputMetaData>& vMetas);

    MUINT32 setInputData(MUINT32 sensorID,
                         sp<IDepthMapEffectRequest> pDepMapReq,
                         const SFPSensorInput &data);

    MUINT32 prepareMain1Data(const SFPSensorInput &data);

    MUINT32 prepareMain2Data(const SFPSensorInput &data);

    MUINT32 setOutputData(sp<IDepthMapEffectRequest> pDepMapReq, DepthImg& out,
                                    const SFPIOMap &generalIO);

    void updateMetadata(const RequestPtr &request,
                        const SFPSensorInput& input,const SFPIOMap& output);
private:
    enum cameraIdentiy {
        MAINCAM = 0,
        SUBCAM  = 1,
    };

    android::Mutex mLock;
    int miLogEnable = 0;
    std::vector<MUINT8> mSensorID;

    typedef struct {
        std::shared_ptr<DepthEnqueData> depEnquePack;
        sp<IDepthMapEffectRequest>      depEffectPack;
    } DepthNodePackage;
    KeyedVector< MINT32, DepthNodePackage > mvDepthNodePack;
};

} // namespace NSFeaturePipe
} // namespace NSCamFeature
} // namespace NSCam

//=======================================================================================
#else //SUPPORT_VSDOF
//=======================================================================================

namespace NSCam {
namespace NSCamFeature {
namespace NSFeaturePipe {
    typedef NullNode DepthNode;
} // namespace NSFeaturePipe
} // namespace NSCamFeature
} // namespace NSCam
//=======================================================================================
#endif //SUPPORT_VSDOF
//=======================================================================================
#endif // _MTK_CAMERA_STREAMING_FEATURE_PIPE_DEPTH_NODE_H_

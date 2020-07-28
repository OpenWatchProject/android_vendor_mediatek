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

#ifndef _MTK_CAMERA_STREAMING_FEATURE_PIPE_FOV_WARP_NODE_H_
#define _MTK_CAMERA_STREAMING_FEATURE_PIPE_FOV_WARP_NODE_H_

#include <mtkcam/drv/iopipe/PostProc/INormalStream.h>

#if !(SUPPORT_WPE)
#include "WarpStreamBase.h"
#endif

#include <camera_custom_dualzoom.h>
#include <utils/Mutex.h>
#include <semaphore.h>
#include "StreamingFeatureNode.h"
#include "MDPWrapper.h"

#if SUPPORT_WPE
#include <mtkcam/drv/iopipe/PostProc/IHalWpePipe.h>
#endif

namespace NSCam {
namespace NSCamFeature {
namespace NSFeaturePipe {

using NSCam::NSIoPipe::NSPostProc::INormalStream;
using NSCam::NSIoPipe::FrameParams;
using NSCam::NSIoPipe::PortID;
#if SUPPORT_WPE
using NSCam::NSIoPipe::NSWpe::WPEQParams;
#endif

class FOVWarpNode;

#if !(SUPPORT_WPE)
struct EnqueCookie
{
    EnqueCookie() : request(nullptr), pNode(nullptr) {}
    EnqueCookie(RequestPtr req, FOVWarpNode* node, FOVResult result, BasicImg buffer)
    : request(req), pNode(node), fovResult(result), fullOutImg(buffer){}
    //
    RequestPtr request;
    FOVWarpNode* pNode;
    FOVResult fovResult;
    BasicImg fullOutImg;
    BasicImg fovOutImg;
};
#endif

#if SUPPORT_WPE
class FOVWarpNode : public StreamingFeatureNode
#else
class FOVWarpNode : public virtual StreamingFeatureNode, public virtual WarpStreamBase<EnqueCookie>
#endif
{
public:
    #if SUPPORT_WPE
    struct EnqueCookie
    {
        EnqueCookie() : request(nullptr), pNode(nullptr) {}
        EnqueCookie(RequestPtr req, FOVWarpNode* node, FOVResult result, BasicImg buffer)
        : request(req), pNode(node), fovResult(result), fullOutImg(buffer){}
        //
        RequestPtr request;
        FOVWarpNode* pNode;
        FOVResult fovResult;
        BasicImg fullOutImg;
        BasicImg fovOutImg;
    };
    #endif
public:
    FOVWarpNode(const char *name);
    virtual ~FOVWarpNode();

    MVOID setOutputBufferPool(const android::sp<IBufferPool> &pool);

public:
    virtual MBOOL onData(DataID id, const BasicIOImgData &data);
    virtual MBOOL onData(DataID id, const FOVData &data);

#if SUPPORT_WPE
    static MVOID onWPEDone(QParams& rParams);
    MVOID onHandleWPEDone(QParams& rParams);
#endif
protected:
    virtual MBOOL onInit();
    virtual MBOOL onUninit();
    virtual MBOOL onThreadStart();
    virtual MBOOL onThreadStop();
    virtual MBOOL onThreadLoop();

private:
#if SUPPORT_WPE
    MBOOL processFOVWarp(
            const RequestPtr &request,
            const FOVResult &warp,
            BasicIOImg &fullIOImg);

    MBOOL prepareYUVOutput(
            const RequestPtr &request,
            const FOVResult &fovResult,
            BasicIOImg &fullIOImg,
            FrameParams &rFrameParam);

    MBOOL prepareWPEParam(
            const RequestPtr &request,
            const FOVResult &fovResult,
            const MSize& inputBufSize,
            WPEQParams& rWPEParam);
#else
    MBOOL processGPUWarp(
            const RequestPtr &request,
            const FOVResult &fovResult,
            BasicIOImg &fullIOImg);

    MBOOL prepareMDPOutput(
            const RequestPtr &request,
            const FOVResult &fovResult,
            BasicImg &fullOutImg,
            WarpParam &param);

    virtual MVOID onWarpStreamBaseCB(
            const WarpParam &param,
            const EnqueCookie &data);
#endif
    MVOID bufferDump(
            const RequestPtr &request,
            IImageBuffer* pImgBuf,
            const char* name);
    MVOID printIO(
            const RequestPtr &request,
            const BasicIOImgData &fullIOImg,
            const FOVData &fovData);
    MVOID handleResultData(
            const RequestPtr &request,
            const BasicImg &fullImg,
            const BasicImg &eisImg);
    MVOID applyFOVCrop(
            const float &scale,
            const MPoint &shift,
            MCropRect &crop,
            const MSize maxSize);

#if SUPPORT_WPE
    MVOID debugWPEParam(const WPEQParams& rWPEParam);
#endif
    double getFOVWarpRatio(const RequestPtr &request);
private:
    WaitQueue<FOVData>   mFOVDatas;
    WaitQueue<BasicIOImgData> mFullIOImgDatas;
#if SUPPORT_WPE
    INormalStream* mpINormalStream = nullptr;
#endif
    android::sp<IBufferPool> mOutputBufferPool;

    MDPWrapper mMDP;
    sem_t       mWaitWPESem;
#if !SUPPORT_WPE
    // for GPU warpping
    WarpStream *mWarpStream;
    android::sp<IBufferPool> mWarpOutBufferPool;
#endif
};

} // namespace NSFeaturePipe
} // namespace NSCamFeature
} // namespace NSCam

#endif // _MTK_CAMERA_STREAMING_FEATURE_PIPE_FOV_WARP_NODE_H_

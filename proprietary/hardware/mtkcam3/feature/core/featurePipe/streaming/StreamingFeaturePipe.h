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

#ifndef _MTK_CAMERA_STREAMING_FEATURE_PIPE_STREAMING_FEATURE_PIPE_H_
#define _MTK_CAMERA_STREAMING_FEATURE_PIPE_STREAMING_FEATURE_PIPE_H_

#include <list>
#include <utils/Mutex.h>

#include <featurePipe/core/include/CamPipe.h>
#include <featurePipe/core/include/IOUtil.h>
#include <mtkcam3/feature/featurePipe/util/FeaturePipeDebugee.h>
#include "StreamingFeatureNode.h"
#include "ImgBufferStore.h"
#include "StreamingFeaturePipeUsage.h"
#include "P2CamContext.h"

#include "RootNode.h"
#include "P2ANode.h"
#include "P2AMDPNode.h"
#include "P2SMNode.h"
#include "P2NRNode.h"
#include "VNRNode.h"
#include "WarpNode.h"
#include "EISNode.h"
#include "RSCNode.h"
#include "HelperNode.h"
#include "VendorMDPNode.h"
#include "DepthNode.h"
#include "BokehNode.h"
#include "TPINode.h"
#include "TPI_AsyncNode.h"
#include "TPI_DispNode.h"

#include <mtkcam3/feature/utils/p2/DIPStream.h>

namespace NSCam {
namespace NSCamFeature {
namespace NSFeaturePipe {

class StreamingFeaturePipe : public CamPipe<StreamingFeatureNode>, public StreamingFeatureNode::Handler_T, public IStreamingFeaturePipe
{
public:
    StreamingFeaturePipe(MUINT32 sensorIndex, const UsageHint &usageHint);
    virtual ~StreamingFeaturePipe();

public:
    // IStreamingFeaturePipe Members
    virtual void setSensorIndex(MUINT32 sensorIndex);
    virtual MBOOL init(const char *name=NULL);
    virtual MBOOL uninit(const char *name=NULL);
    virtual MBOOL enque(const FeaturePipeParam &param);
    virtual MVOID notifyFlush();
    virtual MBOOL flush();
    virtual MBOOL setJpegParam(NSCam::NSIoPipe::NSPostProc::EJpgCmd cmd, int arg1, int arg2);
    virtual MBOOL setFps(MINT32 fps);
    virtual MUINT32 getRegTableSize();
    virtual MBOOL sendCommand(NSCam::NSIoPipe::NSPostProc::ESDCmd cmd, MINTPTR arg1=0, MINTPTR arg2=0, MINTPTR arg3=0);
    virtual MBOOL addMultiSensorID(MUINT32 sensorID);

public:
    virtual MVOID sync();

protected:
    typedef CamPipe<StreamingFeatureNode> PARENT_PIPE;
    virtual MBOOL onInit();
    virtual MVOID onUninit();

protected:
    virtual MBOOL onData(DataID id, const RequestPtr &data);

private:
    MBOOL earlyInit();
    MVOID lateUninit();
    MBOOL initNodes();
    MBOOL uninitNodes();

    MBOOL prepareDebugSetting();
    MBOOL prepareGeneralPipe();
    MBOOL prepareNodeSetting();
    MBOOL prepareNodeConnection();
    MBOOL prepareIOControl();
    MBOOL prepareBuffer();
    MBOOL prepareCamContext();
    MVOID prepareFeatureRequest(const FeaturePipeParam &param);
    MVOID prepareEISQControl(const FeaturePipeParam &param);
    MVOID prepareIORequest(const RequestPtr &request);
    MVOID prepareIORequest(const RequestPtr &request, std::set<StreamType> &generalStreams, MUINT32 sensorID);

    android::sp<IBufferPool> createFullImgPool(const char* name, MSize size);
    android::sp<IBufferPool> createImgPool(const char* name, MSize size, EImageFormat fmt);
    android::sp<IBufferPool> createWarpOutputPool(const char* name, MSize size);

    MVOID releaseGeneralPipe();
    MVOID releaseNodeSetting();
    MVOID releaseBuffer();
    MVOID releaseCamContext();

    MVOID applyMaskOverride(const RequestPtr &request);
    MVOID applyVarMapOverride(const RequestPtr &request);

private:
    MUINT32 mForceOnMask;
    MUINT32 mForceOffMask;
    MUINT32 mSensorIndex;
    StreamingFeaturePipeUsage mPipeUsage;
    MUINT32 mCounter = 0;
    MUINT32 mRecordCounter;
    FPSCounter mDisplayFPSCounter;
    FPSCounter mFrameFPSCounter;

    MINT32 mDebugDump;
    MINT32 mDebugDumpCount;
    MBOOL mDebugDumpByRecordNo;
    MBOOL mForceIMG3O;
    MBOOL mForceWarpPass;
    MUINT32 mForceGpuOut;
    MBOOL mForceGpuRGBA;
    MBOOL mUsePerFrameSetting;
    MBOOL mForcePrintIO;
    MBOOL mEarlyInited;

    RootNode mRootNode;
    P2ANode mP2A;
    P2AMDPNode mP2AMDP;
    P2SMNode mP2SM;
    P2NRNode mP2NR;
    VNRNode mVNR;
    TPI_AsyncNode mAsync;
    TPI_DispNode mDisp;
    WarpNode mWarp;
    DepthNode mDepth;
    BokehNode mBokeh;

    EISNode mEIS;
    RSCNode mRSC;
    HelperNode mHelper;
    VendorMDPNode mVMDP;
    std::vector<TPINode*> mTPIs;

    android::sp<IBufferPool> mFullImgPool;
    android::sp<IBufferPool> mDepthYuvOutPool;
    android::sp<IBufferPool> mBokehOutPool;
    android::sp<IBufferPool> mEisFullImgPool;
    android::sp<IBufferPool> mWarpOutputPool;
    android::sp<IBufferPool> mTPIBufferPool;
    android::sp<SharedBufferPool> mTPISharedBufferPool;
    android::sp<IBufferPool> mVNRInputPool;
    android::sp<IBufferPool> mVNROutputPool;

    Feature::P2Util::DIPStream *mDIPStream = NULL;

    typedef std::list<StreamingFeatureNode*> NODE_LIST;
    NODE_LIST mNodes;

    NODE_LIST mDisplayPath;
    NODE_LIST mRecordPath;
    NODE_LIST mPhysicalPath;
    NODE_LIST mPreviewCallbackPath;
    NODE_LIST mAsyncPath;

    android::sp<NodeSignal> mNodeSignal;
    TPIMgr *mTPIMgr = NULL;

    EISQControl mEISQControl;

    android::Mutex mContextMutex;
    MBOOL mContextCreated[P2CamContext::SENSOR_INDEX_MAX];

    IOControl<StreamingFeatureNode, StreamingReqInfo> mIOControl;
    std::vector<MUINT32> mAllSensorIDs;

    android::sp<FeaturePipeDebugee<StreamingFeaturePipe>> mFPDebugee;
};


} // NSFeaturePipe
} // NSCamFeature
} // NSCam

#endif // _MTK_CAMERA_STREAMING_FEATURE_PIPE_STREAMING_FEATURE_PIPE_H_

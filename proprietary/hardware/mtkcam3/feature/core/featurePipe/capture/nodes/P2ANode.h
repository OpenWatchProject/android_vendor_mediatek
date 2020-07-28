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

#ifndef _MTK_CAMERA_CAPTURE_FEATURE_PIPE_P2A_NODE_H_
#define _MTK_CAMERA_CAPTURE_FEATURE_PIPE_P2A_NODE_H_

#include "CaptureFeatureNode.h"

#include <featurePipe/vsdof/util/P2Operator.h>
#include <featurePipe/core/include/CamThreadNode.h>
#include <featurePipe/core/include/Timer.h>

#include <mtkcam/aaa/IHalISP.h>
#include <thread>
#include <future>
#include <mtkcam3/feature/utils/ISPProfileMapper.h>

using namespace NS3Av3;

namespace NSCam {
namespace NSCamFeature {
namespace NSFeaturePipe {
namespace NSCapture {

class P2ANode : public CaptureFeatureNode {
public:
    P2ANode(NodeID_T nid, const char *name, MINT32 policy = SCHED_NORMAL, MINT32 priority = DEFAULT_CAMTHREAD_PRIORITY);
    virtual ~P2ANode();
    MVOID setBufferPool(const android::sp<CaptureBufferPool>& pool);
    MVOID setSensorFmt(MINT32 sensorIndex, MINT32 sensorIndex2);

public:
    virtual MBOOL onData(DataID id, const RequestPtr& pRequest);

protected:
    virtual MBOOL onInit();
    virtual MBOOL onUninit();
    virtual MBOOL onThreadStart();
    virtual MBOOL onThreadStop();
    virtual MBOOL onThreadLoop();
    virtual MERROR evaluate(NodeID_T nodeId, CaptureFeatureInferenceData& rInference);


public:

    // P2 callbacks
    static MVOID onP2SuccessCallback(QParams& rParams);
    static MVOID onP2FailedCallback(QParams& rParams);

    struct RequestHolder : public IHolder
    {
        RequestHolder(RequestPtr pRequest)
            : mpRequest(pRequest)
            , mStatus(OK)
        {};

        ~RequestHolder() {};

        Vector<android::sp<IIBuffer>>   mpBuffers;
        RequestPtr                      mpRequest;
        std::shared_ptr<RequestHolder>  mpPrecedeOver;
        MERROR                          mStatus;

        std::vector<std::unique_ptr<IHolder>> mpHolders;
    };

    struct SharedData
    {
        // tuning param
        SmartTuningBuffer imgoDualSynData = NULL;
        SmartTuningBuffer rrzoDualSynData = NULL;
    };

    struct P2Input
    {
        IImageBuffer* mpBuf         = NULL;
        BufferID_T    mBufId        = NULL_BUFFER;
        MBOOL         mPureRaw      = MFALSE;
    };

    struct P2Output
    {
        IImageBuffer* mpBuf         = NULL;
        BufferID_T    mBufId        = NULL_BUFFER;
        TypeID_T      mTypeId       = NULL_TYPE;
        MBOOL         mHasCrop      = MFALSE;
        MRect         mCropRegion   = MRect(0,0);
        MBOOL         mEnableCZ     = MFALSE;
        MBOOL         mEnableHFG    = MFALSE;
        MUINT32       mTrans        = 0;
        MUINT32       mScaleRatio   = 0;
        MBOOL         mEarlyRelease = MFALSE;
    };

    struct MDPOutput : P2Output
    {
        MDPOutput() : P2Output() { }

        IImageBuffer* mpSource      = NULL;
        MRect         mSourceCrop   = MRect(0,0);
        MUINT32       mSourceTrans  = 0;
    };

    enum ScaleType {
        eScale_None,
        eScale_Down,
        eScale_Up,
    };

    struct P2EnqueData
    {
        NodeID_T    mNodeId         = NULL_NODE;
        P2Input     mIMGI           = P2Input();
        P2Input     mLCEI           = P2Input();
        P2Input     mDCESI          = P2Input();
        P2Output    mIMG2O          = P2Output();
        P2Output    mWDMAO          = P2Output();
        P2Output    mWROTO          = P2Output();
        P2Output    mIMG3O          = P2Output();
        P2Output    mDCESO          = P2Output();
        P2Output    mTIMGO          = P2Output();
        // Using MDP Copy
        MDPOutput   mCopy1          = MDPOutput();
        MDPOutput   mCopy2          = MDPOutput();

        IMetadata*  mpIMetaApp      = NULL;
        IMetadata*  mpIMetaHal      = NULL;
        IMetadata*  mpIMetaDynamic  = NULL;
        IMetadata*  mpOMetaHal      = NULL;
        IMetadata*  mpOMetaApp      = NULL;

        MINT32      mSensorId       = 0;
        MBOOL       mResized        = MFALSE;
        MBOOL       mYuvRep         = MFALSE;
        MINT32      mScaleType      = eScale_None;
        MSize       mScaleSize      = MSize(0,0);
        MSize       mOriSize        = MSize(0,0);
        MBOOL       mEnableMFB      = MFALSE;
        MBOOL       mEnableDRE      = MFALSE;
        MBOOL       mEnableDCE      = MFALSE;
        MBOOL       mEnableVSDoF    = MFALSE;
        MBOOL       mDebugLoadIn    = MFALSE;
        MBOOL       mDebugDump      = MFALSE;
        MUINT32     mDebugDumpFilter
                                    = 0;
        MBOOL       mDebugUnpack    = MFALSE;
        MBOOL       mDebugDumpMDP   = MFALSE;
        MBOOL       mDumpWithMargin = MFALSE;
        MBOOL       mTimeSharing    = MFALSE;
        MINT32      mIspProfile     = -1;
        MINT32      mUniqueKey      = 0;
        MINT32      mRequestNo      = 0;
        MINT32      mFrameNo        = 0;
        MINT32      mFrameIndex     = 0;
        MINT32      mTaskId         = 0;
        MINT32      mRound          = 1;
        //
        std::shared_ptr<Mutex>      mpLockDCES;
        //
        std::shared_ptr<SharedData> mpSharedData;
        std::shared_ptr<RequestHolder>
                    mpHolder;
    };



    MBOOL enqueISP(
        RequestPtr& request,
        std::shared_ptr<P2EnqueData>& pEnqueData
    );

    // The debug buffer size will be allocated according to IMGI
    MVOID addDebugBuffer(
        std::shared_ptr<RequestHolder>& pHolder,
        std::shared_ptr<P2EnqueData>& pEnqueData
    );

    // image processes
    MBOOL onRequestProcess(NodeID_T nodeId, RequestPtr&);


    // routines
    MVOID onRequestFinish(NodeID_T nodeId, RequestPtr&);


private:
    // request meta update
    MVOID updateFovCropRegion(RequestPtr& pRequest, MINT32 sensorIndex, MetadataID_T metaId, MetadataID_T outMetaId);
    MVOID updateSensorCropRegion(RequestPtr& pRequest, MRect cropRegion);
    // check if request contains yuv output
    MBOOL checkHasYUVOutput(sp<CaptureFeatureNodeRequest> pNodeReq);
    // direct buffer copy by using CPU
    MERROR directBufCopy(const IImageBuffer* pSrcBuf, IImageBuffer* pDstBuf, QParams& rParams);

    //
    inline MBOOL hasSubSensor() {
        return mSensorIndex2 >= 0;
    }

    struct RequestPack {
        RequestPtr                          mpRequest;
        NodeID_T                            mNodeId = NULL_NODE;
    };

    WaitQueue<RequestPack>                  mRequestPacks;

    struct ISPOperator {
        IHalISP*                            mIspHal = NULL;
        sp<P2Operator>                      mP2Opt  = NULL;
    };

    std::map<MINT32, ISPOperator>           mISPOperators;
    MUINT                                   mSensorFmt;
    MUINT                                   mSensorFmt2;

    android::sp<CaptureBufferPool>          mpBufferPool;

    std::map<EDIPInfoEnum, MUINT32>         mDipInfo;
    MUINT32                                 mDipVer;
    MBOOL                                   mSupportDRE;
    MBOOL                                   mSupportCZ;
    MBOOL                                   mSupportHFG;
    MBOOL                                   mSupportDCE;
    MBOOL                                   mSupportMDPQoS;
    MBOOL                                   mSupport10Bits;
    MBOOL                                   mSupportDS;
    MBOOL                                   mISP3_0;
    MBOOL                                   mISP4_0;
    MBOOL                                   mISP5_0;
    MBOOL                                   mISP6_0;
    MINT32                                  mDebugPerFrame;
    MINT32                                  mDebugDS;
    MINT32                                  mDebugDSRatio_dividend;
    MINT32                                  mDebugDSRatio_divisor;
    MINT32                                  mDebugCZ;
    MINT32                                  mDebugDRE;
    MINT32                                  mDebugDCE;
    MINT32                                  mDebugHFG;
    MBOOL                                   mDebugLoadIn = MFALSE;
    MBOOL                                   mDebugDump = MFALSE;
    MBOOL                                   mDebugImg2o = MFALSE;
    MBOOL                                   mDebugImg3o = MFALSE;
    MBOOL                                   mDebugTimgo = MFALSE;
    MUINT32                                 mDebugTimgoType = 0;
    MBOOL                                   mDumpIMGI = MFALSE;
    MBOOL                                   mDumpLCEI = MFALSE;
    MUINT32                                 mDebugDumpFilter;
    MBOOL                                   mDebugUnpack = MFALSE;
    MBOOL                                   mDebugDumpMDP = MFALSE;
    MBOOL                                   mDumpMFNRYuv = MFALSE;

    // ISP6.0
    MSize                                   mDCES_Size;
    MUINT32                                 mDCES_Format;
    // ISP5.0, ISP6.0
    MUINT32                                 mDualSyncInfo_Size;

    std::weak_ptr<RequestHolder>            mpLastHolder;

    // Postview Delay
    wp<CaptureFeatureRequest>               mwpDelayRequest;

    SmartTuningBuffer                       mpKeepTuningData;
    IMetadata                               mKeepMetaApp;
    IMetadata                               mKeepMetaHal;

    ISPProfileMapper*                       mpISPProfileMapper;
private:

    struct EnquePackage :public Timer
    {
    public:
        std::shared_ptr<P2EnqueData>        mpEnqueData;
        PQParam*                            mpPQParam;
        ModuleInfo*                         mpModuleSRZ4;
        ModuleInfo*                         mpModuleSRZ3;
        SmartTuningBuffer                   mpTuningData;
        android::sp<IIBuffer>               mUnpackBuffer;
        P2ANode*                            mpNode;
        MINT32                              miSensorIdx;
        MBOOL                               mbNeedToDump;

        EnquePackage()
            : mpPQParam(NULL)
            , mpModuleSRZ4(NULL)
            , mpModuleSRZ3(NULL)
            , mpTuningData(NULL)
            , mpNode(NULL)
            , miSensorIdx(-1)
            , mbNeedToDump(MTRUE)
        {
        }

        ~EnquePackage();

    };

public:
    static MBOOL copyBuffers(EnquePackage* pkg);
    static MVOID unpackRawBuffer(IImageBuffer* pPkInBuf, IImageBuffer* pUpkOutBuf );

};

} // NSCapture
} // namespace NSFeaturePipe
} // namespace NSCamFeature
} // namespace NSCam

#endif // _MTK_CAMERA_CAPTURE_FEATURE_PIPE_P2A_NODE_H_

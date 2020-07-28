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

#ifndef _MTK_CAMERA_STREAMING_FEATURE_PIPE_P2A_NODE_H_
#define _MTK_CAMERA_STREAMING_FEATURE_PIPE_P2A_NODE_H_

#include "StreamingFeatureNode.h"
#include "DIPStreamBase.h"

#include "P2CamContext.h"
#include "TuningHelper.h"
#include "DSDNCfg.h"

#include <mtkcam/drv/iopipe/PostProc/IHalPostProcPipe.h>
#include <mtkcam3/feature/3dnr/util_3dnr.h>
#include <common/3dnr/3dnr_hal_base.h>
#include <featurePipe/core/include/CamThreadNode.h>
#include <featurePipe/core/include/ThreadSafeQueue.h>
#include <mtkcam3/feature/DualCam/FOVHal.h>
#include <mtkcam/aaa/IHal3A.h>
#include <mtkcam/aaa/IHalISP.h>
#include <mtkcam3/feature/utils/p2/P2IO.h>
#include <mtkcam3/feature/utils/p2/P2Util.h>
#include <mtkcam3/feature/utils/SecureBufferControlUtils.h>
#include "TimgoHal.h"

#include <mtkcam3/feature/utils/p2/DIPStream.h>

using namespace NS3Av3;
using NSCam::NR3D::NR3DHALResult;

class EisHal;

namespace NSCam {
namespace NSCamFeature {
namespace NSFeaturePipe {

class DCESORecord
{
public :
    MINT32      mMagic3A = -1;
    ImgBuffer   mImg;

    IImageBuffer* getBufferPtr() const { return (mImg != NULL) ? mImg->getImageBufferPtr() : NULL;}
};
typedef android::sp<ThreadSafeQueue<DCESORecord>> DCE_QUEUE_PTR;
typedef std::unordered_map<MUINT32, DCE_QUEUE_PTR> DCE_QUEUE_MAP;

class P2ARunData : public virtual android::RefBase
{
public:
    P2ARunData(const char * name) : mName(name) {}
    virtual ~P2ARunData(){}

    std::string                 mName;
    MUINT32                     mSensorID = INVALID_SENSOR;
    SmartTuningBuffer           mTuningBuf;
    Feature::P2Util::P2ISPObj   mIspObj;
    DCESORecord                 mDCESORec;
    MINT32                      mPreDCEMagic = -1;
    DCE_QUEUE_PTR               mDCESOQueue;
};

class P2ADummyData : public virtual android::RefBase
{
public:
    P2ADummyData() {}
    virtual ~P2ADummyData(){}
    Feature::P2Util::P2Obj   mP2Obj;
};

class P2AEnqueData
{
public:
    RequestPtr  mRequest;
    BasicImg    mFullImg;
    ImgBuffer   mPrevFullImg; // VIPI use
    BasicImg    mNextFullImg;
    BasicImg    mSlaveFullImg;
    BasicImg    mSlaveNextFullImg;
    ImgBuffer   mAsyncImg;
    ImgBuffer   mFE1Img;
    ImgBuffer   mFE2Img;
    ImgBuffer   mFE3Img;
    ImgBuffer   mTimgo;
    MUINT32     mTimgoType = 0;
    BasicImg    mDS1Img;

    // 3dnr MV info
    NR3D::NR3DMVInfo m3dnrMvInfo;

    std::vector<P2IO>  mRemainingOutputs; // Master may Need additional MDP to generate output
    std::list<android::sp<P2ARunData>> mRunDatas;
    std::list<SmartTuningBuffer> mTuningBufs;
    std::list<android::sp<P2ADummyData>> mDummyDatas;
};

class P2ATuningExtra
{
public:
    DCE_QUEUE_PTR mDCESOQueue;
    IMetadata *mExtraAppOut = NULL;
    TuningHelper::Scene mScene = TuningHelper::Tuning_Normal;
    SmartTuningBuffer mSyncTuning;

    P2ATuningExtra()
    {}

    P2ATuningExtra(const DCE_QUEUE_PTR &dceQ)
    : mDCESOQueue(dceQ)
    {}

    P2ATuningExtra(const DCE_QUEUE_PTR &dceQ, IMetadata *extraAppOut)
    :  mDCESOQueue(dceQ)
    ,  mExtraAppOut(extraAppOut)
    {}

    P2ATuningExtra(const DCE_QUEUE_PTR &dceQ, IMetadata *extraAppOut, TuningHelper::Scene scen)
    :  mDCESOQueue(dceQ)
    ,  mExtraAppOut(extraAppOut)
    ,  mScene(scen)
    {}
};

class P2ATuningIndex
{
public:
    MINT32 mGenMaster = -1;
    MINT32 mGenSlave = -1;
    MINT32 mPhyMaster = -1;
    MINT32 mPhySlave = -1;
    MINT32 mLargeMaster = -1;
    MINT32 mLargeSlave = -1;

    MBOOL isGenMasterValid() const {return mGenMaster >= 0;}
    MBOOL isGenSlaveValid() const {return mGenSlave >= 0;}
    MBOOL isPhyMasterValid() const {return mPhyMaster >= 0;}
    MBOOL isPhySlaveValid() const {return mPhySlave >= 0;}
    MBOOL isLargeMasterValid() const {return mLargeMaster >= 0;}
    MBOOL isLargeSlaveValid() const {return mLargeSlave >= 0;}

    MBOOL isMasterMainValid() const
    {
        return isGenMasterValid() || isPhyMasterValid();
    }
    MUINT32 getMasterMainIndex() const
    {
        return isGenMasterValid() ? mGenMaster : mPhyMaster;
    }
    MBOOL isSlaveMainValid() const
    {
        return isGenSlaveValid() || isPhySlaveValid();
    }
    MUINT32 getSlaveMainIndex() const
    {
        return isGenSlaveValid() ? mGenSlave : mPhySlave;
    }
};

class P2ANode :
    public StreamingFeatureNode,
    public DIPStreamBase<P2AEnqueData>
{
public:
    P2ANode(const char *name);
    virtual ~P2ANode();

    MVOID setDIPStream(Feature::P2Util::DIPStream *stream);

    MVOID setFullImgPool(const android::sp<IBufferPool> &pool, MUINT32 allocate = 0);

public:
    virtual MBOOL onData(DataID id, const RequestPtr &data);
    virtual MBOOL onData(DataID id, const RSCData &data);
    virtual IOPolicyType getIOPolicy(StreamType stream, const StreamingReqInfo &reqInfo) const;

protected:
    virtual MBOOL onInit();
    virtual MBOOL onUninit();
    virtual MBOOL onThreadStart();
    virtual MBOOL onThreadStop();
    virtual MBOOL onThreadLoop();

protected:
    virtual MVOID onDIPStreamBaseCB(const Feature::P2Util::DIPParams &dipParams, const P2AEnqueData &request);

private:
    MVOID handleResultData(const RequestPtr &request, const P2AEnqueData &data);
    MVOID handleRunData(const RequestPtr &request, const P2AEnqueData &data);
    MBOOL initP2();
    MVOID uninitP2();
    MBOOL processP2A(const RequestPtr &request, const RSCData &rscData);
    MBOOL prepareFullImgFromInput(const RequestPtr &request, P2AEnqueData &data);
    MBOOL prepareNonMDPIO(Feature::P2Util::DIPParams &params, const RequestPtr &request, P2AEnqueData &data, const P2ATuningIndex &tuningIndex, MBOOL nextFullByIMG3O);
    MBOOL prepareMasterMDPOuts(Feature::P2Util::DIPParams &params, const RequestPtr &request, P2AEnqueData &data, const P2ATuningIndex &tuningIndex, MBOOL nextFullByIMG3O);
    MBOOL prepareSlaveOuts(Feature::P2Util::DIPParams &params, const RequestPtr &request, P2AEnqueData &data, const P2ATuningIndex &tuningIndex, MBOOL nextFullByIMG3O);
    MBOOL prepareLargeMDPOuts(Feature::P2Util::DIPParams &params, const RequestPtr &request, MUINT32 frameIndex, MUINT32 sensorID);
    MVOID prepareVIPI(Feature::P2Util::DIPFrameParams &frame, const RequestPtr &request, P2AEnqueData &data);
    MVOID prepareTimgo(Feature::P2Util::DIPFrameParams &frame, const RequestPtr &request, P2AEnqueData &data, MUINT32 sensorID);
    MVOID prepareFDImg(Feature::P2Util::DIPFrameParams &frame, const RequestPtr &request, P2AEnqueData &data);
    MVOID prepareDs1Img(Feature::P2Util::DIPFrameParams &frame, const RequestPtr &request, const FrameInInfo &inInfo, P2AEnqueData &data);
    MVOID prepareFullImg(Feature::P2Util::DIPFrameParams &frame, const RequestPtr &request, BasicImg &outImg, const FrameInInfo &inInfo, MUINT32 sensorID, BasicImg *nextFullImg);
    MVOID prepareNextFullOut(P2IO &output, const RequestPtr &request, BasicImg &outImg, const FrameInInfo &inInfo, P2AEnqueData &data, MUINT32 sensorID);
    MBOOL prepareExtraMDPCrop(const BasicImg &fullImg, std::vector<P2IO> &leftOutList);
    MVOID enqueFeatureStream(Feature::P2Util::DIPParams &params, P2AEnqueData &data, const P2ATuningIndex &tuningIndex);
    MBOOL needFullForExtraOut(std::vector<P2IO> &outList);
    MBOOL needNextFullByIMG3O(const RequestPtr &request, MUINT32 sensorID);

private:

    MBOOL init3A();
    MVOID uninit3A();
    MBOOL prepare3A(Feature::P2Util::DIPParams &params, const RequestPtr &request);
    // Tuning
    MBOOL prepareRawTuning(Feature::P2Util::DIPParams &params, const RequestPtr &request, P2AEnqueData &data, P2ATuningIndex &tuningIndex);
    MINT32 prepareOneRawTuning( const  char*             pathName,
                                       MUINT32           sensorID,
                                const  SFPIOMap          &ioMap,
                                       Feature::P2Util::DIPParams           &params,
                                const  RequestPtr        &request,
                                       P2AEnqueData      &data,
                                       P2ATuningExtra    &extra);
    MBOOL needNormalYuv(MUINT32 sensorID, const RequestPtr &request);

    MBOOL createDCESOs();
    MBOOL initDCEQueue(DCE_QUEUE_MAP &dceQMap);
    MVOID destroyDCESOs();
    MVOID popDCESO(DCE_QUEUE_PTR &queue, android::sp<P2ARunData> &runData);
    MVOID updateDCESO(const RequestPtr &request, const android::sp<P2ARunData> &runData, MUINT32 sensorID, MBOOL isDcesEnque);
    MBOOL returnDCESO(const RequestPtr &request, const android::sp<P2ARunData> &runData);

    MBOOL createTuningBufs();
    SmartTuningBuffer requestSyncTuningBuf();
    MVOID destroyTuningBufs();

private:
    struct eis_region
    {
        MUINT32 x_int;
        MUINT32 x_float;
        MUINT32 y_int;
        MUINT32 y_float;
        MSize s;
        MINT32 gmvX;
        MINT32 gmvY;
        MINT32 confX;
        MINT32 confY;
    };

    MBOOL prepare3DNRMvInfo(Feature::P2Util::DIPParams &params, const RequestPtr &request, MUINT32 sensorID, const RSCData &rscData);
    MBOOL prepare3DNR(Feature::P2Util::DIPParams &params, const RequestPtr &request, MUINT32 sensorID, const RSCData &rscData);
    MVOID dump_DIPparam(Feature::P2Util::DIPParams& rParams, const char *pSep);
    MVOID dump_vOutImageBuffer(const Feature::P2Util::DIPParams & params);
    MVOID dump_imgiImageBuffer(const Feature::P2Util::DIPParams & params);

    MBOOL do3dnrFlow(
        Feature::P2Util::DIPParams &enqueParams,
        const RequestPtr &request,
        const MRect &dst_resizer_rect,
        const MSize &resize_size,
        eis_region &eisInfo,
        MINT32 iso,
        MINT32 isoThreshold,
        MUINT32 requestNo,
        const RSCData &rscData,
        const P2ATuningIndex &tuningIndex
        );

    MINT32 m3dnrLogLevel;
    NR3DHALResult mNr3dHalResult;
    IHal3A *mp3A = NULL;

private:
    Feature::SecureBufferControl mSecBufCtrl;

private:
    WaitQueue<RequestPtr> mRequests;
    WaitQueue<RSCData> mRSCDatas;

    Feature::P2Util::DIPStream *mDIPStream = NULL;
    android::Mutex mEnqueMutex;

    MUINT32 mFullImgPoolAllocateNeed;

    TimgoHal mTimgoHal;

    DSDNCfg mDSDNCfg;

    android::sp<IBufferPool> mFullImgPool;
    android::sp<TuningBufferPool> mDynamicTuningPool;
    android::sp<TuningBufferPool> mSyncTuningPool;
    android::sp<ImageBufferPool> mDCESOImgPool;
    android::sp<ImageBufferPool> mTimgoPool;
    android::sp<IBufferPool> mDS1ImgPool;

    DCE_QUEUE_MAP mDCEQueueMap;
    DCE_QUEUE_MAP mPhyDCEQueueMap;
    DCE_QUEUE_MAP mLargeDCEQueueMap;

    MUINT32   mEisMode;
    MBOOL     mLastDualParamValid;
    MUINT32   mForceExpectMS = 0;

};

} // namespace NSFeaturePipe
} // namespace NSCamFeature
} // namespace NSCam

#endif // _MTK_CAMERA_STREAMING_FEATURE_PIPE_P2A_NODE_H_

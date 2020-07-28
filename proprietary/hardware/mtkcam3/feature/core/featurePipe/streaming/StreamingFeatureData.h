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

#ifndef _MTK_CAMERA_STREAMING_FEATURE_PIPE_STREAMING_FEATURE_DATA_H_
#define _MTK_CAMERA_STREAMING_FEATURE_PIPE_STREAMING_FEATURE_DATA_H_

#include <unordered_map>
#include "MtkHeader.h"
//#include <mtkcam3/feature/featurePipe/IStreamingFeaturePipe.h>
//#include <mtkcam/common/faces.h>
//#include <mtkcam3/featureio/eis_type.h>
#include <camera_custom_eis.h>

#include <utils/RefBase.h>
#include <utils/Vector.h>

#include <featurePipe/core/include/CamNodeULog.h>
#include <featurePipe/core/include/WaitQueue.h>
#include <featurePipe/core/include/IIBuffer.h>
#include <featurePipe/core/include/IOUtil.h>
#include <featurePipe/core/include/TuningBufferPool.h>

#include "StreamingFeatureTimer.h"
#include "StreamingFeaturePipeUsage.h"

#include "OutputControl.h"
#include "EISQControl.h"
#include "MDPWrapper.h"

#include "tpi/TPIMgr.h"

#include <featurePipe/core/include/CamNodeULog.h>

#include <mtkcam3/feature/utils/p2/DIPStream.h>

namespace NSCam {
namespace NSCamFeature {
namespace NSFeaturePipe {

typedef android::sp<IIBuffer> ImgBuffer;

enum Domain { RRZO_DOMAIN, WARP_DOMAIN };

enum IO_TYPE
{
    IO_TYPE_DISPLAY,
    IO_TYPE_RECORD,
    IO_TYPE_EXTRA,
    IO_TYPE_FD,
    IO_TYPE_PHYSICAL,
};

class StreamingFeatureNode;
class StreamingReqInfo;
class SrcCropInfo // This class will do copy from user, please keep size as small as possible
{
public:
    MRect mSrcCrop = MRect(0, 0);
    MBOOL mIMGOin = MFALSE;
    MBOOL mIsSrcCrop = MFALSE;
    MSize mRRZOSize = MSize(0, 0);
    MSize mIMGOSize = MSize(0, 0);
};

class DomainTransform
{
public:
    MPointF   mOffset;
    MSizeF    mScale = MSizeF(1.0f, 1.0f);
    MVOID accumulate(const char* name, const Feature::ILog &log, const MSize &inSize, const MRectF &crop, const MSize &outSize);
    MVOID accumulate(const char* name, const Feature::ILog &log, const MSizeF &inSize, const MRectF &crop, const MSizeF &outSize);
    MVOID accumulate(const char* name, const Feature::ILog &log, const MRectF &inZoomROI, const MRectF &outZoomROI, const MSizeF &outSize);
    MRectF applyTo(const MRectF &crop) const;
};

class OutputInfo
{
public:
    MRectF mCropRect;
    MSize  mOutSize;
};

enum HelperMsg
{
    HMSG_UNKNOWN,
    HMSG_FRAME_DONE,
    HMSG_DISPLAY_DONE,
    HMSG_PMDP_DONE,
    HMSG_ASYNC_DONE,
    HMSG_META_DONE,
    HMSG_COUNT,          //  must be the last entry
};

FeaturePipeParam::MSG_TYPE toFPPMsg(HelperMsg msg);

class HelperRWData
{
public:
    MVOID markReady(HelperMsg msg);
    MVOID markDone(HelperMsg msg);
    MBOOL isReady(HelperMsg msg) const;
    MBOOL isReadyOrDone(HelperMsg msg) const;
    MBOOL isReadyOrDone(const std::set<HelperMsg> &msgs) const;

private:
    enum
    {
        HMSG_STATE_EMPTY  = 0,
        HMSG_STATE_READY  = 1 << 0,
        HMSG_STATE_DONE   = 1 << 1,
    };

private:
    MUINT32 mMsg[HMSG_COUNT] = { HMSG_STATE_EMPTY };
};

class BasicImg
{
public:
    class SensorClipInfo
    {
    public:
        MUINT32 mSensorID = INVALID_SENSOR_ID;
        MRectF  mSensorCrop;
        MSize   mSensorSize;
        SensorClipInfo();
        SensorClipInfo(MUINT32 sensorID, const MRectF &crop, const MSize &sensorSize);
        SensorClipInfo(MUINT32 sensorID, const MRect &crop, const MSize &sensorSize);
        MVOID accumulate(const char* name, const Feature::ILog &log, const MSize &inputSize, const MRectF &cropInInput);
        MVOID accumulate(const char* name, const Feature::ILog &log, const MSizeF &inputSize, const MRectF &cropInInput);
    };
    android::sp<IIBuffer> mBuffer;
    DomainTransform       mTransform;
    SensorClipInfo        mSensorClipInfo;
    MBOOL                 mIsReady = MTRUE;

    BasicImg();
    BasicImg(const android::sp<IIBuffer> &img, MBOOL isReady = MTRUE);
    BasicImg(const android::sp<IIBuffer> &img, MUINT32 sensorID, const MRectF &crop, const MSize &sensorSize, MBOOL isReady = MTRUE);

    MBOOL isValid() const;
    MVOID setAllInfo(const BasicImg &img, const MSize &size = MSize(0,0));
    MBOOL syncCache(NSCam::eCacheCtrl ctrl);
    MVOID accumulate(const char* name, const Feature::ILog &log, const MSize &inSize, const MRectF &crop, const MSize &outSize);
    MVOID accumulate(const char* name, const Feature::ILog &log, const MSizeF &inSize, const MRectF &crop, const MRectF &inZoomROI, const MRectF &outZoomROI, const MSizeF &outSize);
};

class WarpImg
{
public:
    android::sp<IIBuffer> mBuffer;
    MSizeF                mInputSize;
    MRectF                mInputCrop;

    WarpImg();
    WarpImg(const android::sp<IIBuffer> &img, const MSizeF &targetInSize, const MRectF &targetCrop);
};

struct EISSourceDumpInfo
{
    enum DUMP_TYPE
    {
        TYPE_RECORD,
        TYPE_EXTRA,
    };
    MBOOL needDump = false;
    MBOOL markFrame = false;
    MUINT32 dumpIdx = 0;
    DUMP_TYPE dumpType;
    std::string filePath;
};

class WarpController
{
public:
    enum WARPMAP_TYPE
    {
        TYPE_DISPLAY,
        TYPE_RECORD,
        TYPE_COUNT,
    };
    MVOID setWarpmap(WARPMAP_TYPE type, const android::sp<IIBuffer>& warpmap, const MSizeF &targetInSize, const MRectF &targetCrop);
    WarpImg getWarpmap(WARPMAP_TYPE type) const;
    MBOOL hasWarpmap(WARPMAP_TYPE type) const;
private:
    android::sp<IIBuffer> mDisWarpmap = NULL;
    android::sp<IIBuffer> mRecWarpmap = NULL;
    MSizeF                mDisTargetInSize;
    MSizeF                mRecTargetInSize;
    MRectF                mDisTargetCrop;
    MRectF                mRecTargetCrop;
};

class TPIRes;

class StreamingFeatureRequest : public virtual android::RefBase
{
private:
    // must allocate extParam before everything else
    FeaturePipeParam mExtParam;
    // alias members, do not change initialize order
    Feature::P2Util::DIPParams &mDIPParams;
    std::vector<Feature::P2Util::DIPFrameParams> &mvDIPFrameParams;
public:
    // alias members, do not change initialize order
    const StreamingFeaturePipeUsage &mPipeUsage;
    VarMap<SFP_VAR> &mVarMap;
    const Feature::P2Util::P2Pack &mP2Pack;
    Feature::ILog mLog;
    SFPIOManager &mSFPIOManager;
    MUINT32 mSlaveID = INVALID_SENSOR;
    MUINT32 mMasterID = INVALID_SENSOR;
    std::unordered_map<MUINT32, IORequest<StreamingFeatureNode, StreamingReqInfo>> mIORequestMap;
    IORequest<StreamingFeatureNode, StreamingReqInfo> &mMasterIOReq;

    MUINT32 mFeatureMask;
    const MUINT32 mRequestNo;
    const MUINT32 mRecordNo;
    const MUINT32 mMWFrameNo;
    IStreamingFeaturePipe::eAppMode mAppMode;
    StreamingFeatureTimer mTimer;
    //NSCam::NSIoPipe::QParams mP2A_QParams;
    //NSCam::NSIoPipe::QParams mN3D_P2Node_QParams;
    MINT32 mDebugDump;
    Feature::P2Util::P2DumpType mP2DumpType = Feature::P2Util::P2_DUMP_NONE;;
    HelperRWData mHelperNodeData; // NOTE can only access by helper node thread
    MBOOL mDispNodeData = MTRUE;
    WarpController mWarpController;
    TPIOFrame mTPIFrame;

public:
    StreamingFeatureRequest(const StreamingFeaturePipeUsage &pipeUsage, const FeaturePipeParam &extParam, MUINT32 requestNo, MUINT32 recordNo, const EISQState &eisQ);
    ~StreamingFeatureRequest();

    MVOID setDisplayFPSCounter(FPSCounter *counter);
    MVOID setFrameFPSCounter(FPSCounter *counter);
    MVOID calSizeInfo();
    MVOID checkBufferHoldByReq() const;

    MBOOL updateResult(MBOOL result);
    MBOOL doExtCallback(FeaturePipeParam::MSG_TYPE msg);

    IImageBuffer* getMasterInputBuffer();
    IImageBuffer* getDisplayOutputBuffer();
    IImageBuffer* getRecordOutputBuffer();

    MSize getMasterInputSize();
    MSize getDisplaySize() const;
    MRectF getRecordCrop() const;
    MRectF getZoomROI() const;

    MBOOL popDisplayOutput(const StreamingFeatureNode *node, NSCam::Feature::P2Util::P2IO &output);
    MBOOL popRecordOutput(const StreamingFeatureNode *node, NSCam::Feature::P2Util::P2IO &output);
    MBOOL popRecordOutputs(const StreamingFeatureNode *node, std::vector<NSCam::Feature::P2Util::P2IO> &outList);
    MBOOL popExtraOutputs(const StreamingFeatureNode *node, std::vector<NSCam::Feature::P2Util::P2IO> &outList);
    MBOOL popPhysicalOutput(const StreamingFeatureNode *node, MUINT32 sensorID, NSCam::Feature::P2Util::P2IO &output);
    MBOOL popLargeOutputs(const StreamingFeatureNode *node, MUINT32 sensorID, std::vector<NSCam::Feature::P2Util::P2IO> &outList);
    MBOOL popFDOutput(const StreamingFeatureNode *node, NSCam::Feature::P2Util::P2IO &output);
    MBOOL getOutputInfo(IO_TYPE ioType, OutputInfo &bufInfo);
    EISSourceDumpInfo getEISDumpInfo();

    ImgBuffer popAsyncImg(const StreamingFeatureNode *node);
    ImgBuffer getAsyncImg(const StreamingFeatureNode *node) const;
    ImgBuffer requestNextFullImg(const StreamingFeatureNode *node, MUINT32 sensorID, NextFullInfo &info);
    NextFullInfo getNextFullInfo(const StreamingFeatureNode *node, MUINT32 sensorID) const;

    MBOOL needDisplayOutput(const StreamingFeatureNode *node);
    MBOOL needRecordOutput(const StreamingFeatureNode *node);
    MBOOL needExtraOutput(const StreamingFeatureNode *node);
    MBOOL needFullImg(const StreamingFeatureNode *node, MUINT32 sensorID);
    MBOOL needNextFullImg(const StreamingFeatureNode *node, MUINT32 sensorID);
    MBOOL needPhysicalOutput(const StreamingFeatureNode *node, MUINT32 sensorID);

    MBOOL hasGeneralOutput() const;
    MBOOL hasDisplayOutput() const;
    MBOOL hasFDOutput() const;
    MBOOL hasRecordOutput() const;
    MBOOL hasExtraOutput() const;
    MBOOL hasPhysicalOutput(MUINT32 sensorID) const;
    MBOOL hasLargeOutput(MUINT32 sensorID) const;

    MSize getEISInputSize() const;
    SrcCropInfo getSrcCropInfo(MUINT32 sensorID);

    DECLARE_VAR_MAP_INTERFACE(mVarMap, SFP_VAR, setVar, getVar, tryGetVar, clearVar);

    MVOID setDumpProp(MINT32 start, MINT32 count, MBOOL byRecordNo);
    MVOID setForceIMG3O(MBOOL forceIMG3O);
    MVOID setForceWarpPass(MBOOL forceWarpPass);
    MVOID setForceGpuOut(MUINT32 forceGpuOut);
    MVOID setForceGpuRGBA(MBOOL forceGpuRGBA);
    MVOID setForcePrintIO(MBOOL forcePrintIO);
    MVOID setEISDumpInfo(const EISSourceDumpInfo& info);

    MBOOL isForceIMG3O() const;
    MBOOL hasSlave(MUINT32 sensorID) const;
    MBOOL isSlaveParamValid() const;
    FeaturePipeParam& getSlave(MUINT32 sensorID);
    const SFPSensorInput& getSensorInput() const ;
    const SFPSensorInput& getSensorInput(MUINT32 sensorID) const ;
    VarMap<SFP_VAR>& getSensorVarMap(MUINT32 sensorID);

    MBOOL getMasterFrameBasic(Feature::P2Util::DIPFrameParams &output);
    BasicImg::SensorClipInfo getSensorClipInfo(MUINT32 sensorID) const;
    MUINT32 getExtraOutputCount() const;

    // Legacy code for Hal1, w/o dynamic tuning & P2Pack
    MBOOL getMasterFrameTuning(Feature::P2Util::DIPFrameParams &output);
    MBOOL getMasterFrameInput(Feature::P2Util::DIPFrameParams &output);
    // -----

    const char* getFeatureMaskName() const;
    MBOOL need3DNR() const;
    MBOOL needVHDR() const;
    MBOOL needP2NR() const;
    MBOOL needDSDN20() const;
    MBOOL needEIS() const;
    MBOOL needPreviewEIS() const;
    MBOOL needTPIYuv() const;
    MBOOL needTPIAsync() const;
    MBOOL needVendorMDP() const;
    MBOOL needEarlyFSCVendorFullImg() const;
    MBOOL needWarp() const;
    MBOOL needEarlyDisplay() const;
    MBOOL needP2AEarlyDisplay() const;
    MBOOL skipMDPDisplay() const;
    MBOOL needRSC() const;
    MBOOL needFSC() const;
    MBOOL needDump() const;
    MBOOL needNddDump() const;
    MBOOL needRegDump() const;
    MBOOL is4K2K() const;
    MBOOL isP2ACRZMode() const;
    EISQ_ACTION getEISQAction() const;
    MUINT32 getEISQCounter() const;
    MBOOL useWarpPassThrough() const;
    MBOOL useDirectGpuOut() const;
    MBOOL needPrintIO() const;
    MUINT32 getMasterID() const;
    MBOOL needTOF() const;

    MUINT32 needTPILog() const;
    MUINT32 needTPIDump() const;
    MUINT32 needTPIScan() const;
    MUINT32 needTPIBypass() const;
    MUINT32 needTPIBypass(unsigned tpiNodeID) const;

    MSize getFSCMaxMarginPixel() const;
    MSizeF getEISMarginPixel() const;
    MRectF getEISCropRegion() const;
    MUINT32 getAppFPS() const;
    MUINT32 getNodeCycleTimeMs() const;

    IMetadata* getAppMeta() const;
    MVOID getTPIMeta(TPIRes &res) const;

    static NSCam::Utils::ULog::RequestTypeId getULogRequestTypeId() {
        return NSCam::Utils::ULog::REQ_STR_FPIPE_REQUEST;
    }

    NSCam::Utils::ULog::RequestSerial getULogRequestSerial() const {
        return mRequestNo;
    }

private:
    MVOID checkEISQControl();

    MBOOL getCropInfo(NSCam::NSIoPipe::EPortCapbility cap, MUINT32 defCropGroup, NSCam::NSIoPipe::MCrpRsInfo &crop);
    MVOID calNonLargeSrcCrop(SrcCropInfo &info, MUINT32 sensorID);
    MVOID calGeneralZoomROI();

    void appendEisTag(string& str, MUINT32 mFeatureMask) const;
    void append3DNRTag(string& str, MUINT32 mFeatureMask) const;
    void appendRSCTag(string& str, MUINT32 mFeatureMask) const;
    void appendDSDNTag(string& str, MUINT32 mFeatureMask) const;
    void appendFSCTag(string& str, MUINT32 mFeatureMask) const;
    void appendVendorTag(string& str, MUINT32 mFeatureMask) const;
    void appendNoneTag(string& str, MUINT32 mFeatureMask) const;
    void appendDefaultTag(string& str, MUINT32 mFeatureMask) const;

private:
    std::unordered_map<MUINT32, FeaturePipeParam> &mSlaveParamMap;
    std::unordered_map<MUINT32, SrcCropInfo> mNonLargeSrcCrops;
    MRectF mGeneralZoomROI;
    MSize mFullImgSize;
    MBOOL mHasGeneralOutput = MFALSE;

    FPSCounter *mDisplayFPSCounter;
    FPSCounter *mFrameFPSCounter;

    static std::unordered_map<MUINT32, std::string> mFeatureMaskNameMap;

    MBOOL mResult;
    MBOOL mNeedDump;
    MBOOL mForceIMG3O;
    MBOOL mForceWarpPass;
    MUINT32 mForceGpuOut;
    MBOOL mForceGpuRGBA;
    MBOOL mForcePrintIO;
    MBOOL mIs4K2K;
    MBOOL mIsP2ACRZMode;
    EISQState mEISQState;
    EISSourceDumpInfo mEisDumpInfo;
    OutputControl mOutputControl;

    MUINT32 mTPILog = 0;
    MUINT32 mTPIDump = 0;
    MUINT32 mTPIScan = 0;
    MUINT32 mTPIBypass = 0;

    MUINT32 mAppFPS = 30;
    MUINT32 mNodeCycleTimeMs = 0;
}; // class StreamingFeatureRequest end
typedef android::sp<StreamingFeatureRequest> RequestPtr;
typedef std::unordered_map<MUINT32, android::sp<IBufferPool>> PoolMap;

template <typename T>
class Data
{
public:
    T mData;
    RequestPtr mRequest;

    // lower value will be process first
    MUINT32 mPriority;

    Data()
        : mPriority(0)
    {}

    virtual ~Data() {}

    Data(const T &data, const RequestPtr &request, MINT32 nice = 0)
        : mData(data)
        , mRequest(request)
      , mPriority(request->mRequestNo)
    {
        if( nice > 0 )
        {
            // TODO: watch out for overflow
            mPriority += nice;
        }
    }

    T& operator->()
    {
        return mData;
    }

    const T& operator->() const
    {
        return mData;
    }

    class IndexConverter
    {
    public:
        IWaitQueue::Index operator()(const Data &data) const
        {
            return IWaitQueue::Index(data.mRequest->mRequestNo,
                                     data.mPriority);
        }
        static unsigned getID(const Data &data)
        {
            return data.mRequest->mRequestNo;
        }
        static unsigned getPriority(const Data &data)
        {
            return data.mPriority;
        }
    };

    static NSCam::Utils::ULog::RequestTypeId getULogRequestTypeId() {
        return NSCam::Utils::ULog::REQ_STR_FPIPE_REQUEST;
    }

    NSCam::Utils::ULog::RequestSerial getULogRequestSerial() const {
        return mRequest->mRequestNo;
    }
};


class MyFace
{
public:
    MtkCameraFaceMetadata mMeta;
    MyFace()
    {
        mMeta.number_of_faces = 0;
        mMeta.ImgWidth = 0;
        mMeta.ImgHeight = 0;
    }

    MyFace(const MyFace &src)
    {
        mMeta = src.mMeta;
    }

    MyFace operator=(const MyFace &src)
    {
        mMeta = src.mMeta;
        return *this;
    }
};

class RSCResult
{
public:
    union RSC_STA_0
    {
        MUINT32 value;
        struct
        {
            MUINT16 sta_gmv_x; // regard RSC_STA as composition of gmv_x and gmv_y
            MUINT16 sta_gmv_y;
        };
        RSC_STA_0()
            : value(0)
        {
        }
    };

    ImgBuffer mMV;
    ImgBuffer mBV;
    MSize     mRssoSize;
    RSC_STA_0 mRscSta; // gmv value of RSC
    MBOOL     mIsValid;

    RSCResult();
    RSCResult(const ImgBuffer &mv, const ImgBuffer &bv, const MSize& rssoSize, const RSC_STA_0& rscSta, MBOOL valid);
};

class DSDNImg
{
public:
    DSDNImg();
    DSDNImg(const BasicImg &full);

public:
    BasicImg mFullImg;
    BasicImg mSlaveImg;
    BasicImg mDS1Img;
    ImgBuffer mDS2Img;
};

class DualBasicImg
{
public:
    BasicImg mMaster;
    BasicImg mSlave;
    DualBasicImg();
    DualBasicImg(const BasicImg &master);
    DualBasicImg(const BasicImg &master, const BasicImg &slave);
};

class VMDPReq
{
public:
    enum class SRC_TYPE
    {
        GENERAL,
        RECORD,
        DROP,
    };

    class RecordInfo
    {
    public:
        SRC_TYPE                mSrcType = SRC_TYPE::GENERAL;
        MBOOL                   mOverride = MFALSE;
        Feature::P2Util::P2Pack mOverrideP2Pack;
        MRectF                  mOverrideCrop;
    };
    BasicImg        mGeneralInput;
    BasicImg        mRecordInput;
    RecordInfo      mRecordInfo;

public:
    VMDPReq();
    VMDPReq(const BasicImg &full);
    MBOOL isRecordOverride() const { return mRecordInfo.mOverride; }
    SRC_TYPE getRecSrcType() const { return mRecordInfo.mSrcType; }
};

class P2AMDPReq
{
public:
    BasicImg                                    mMDPIn;
    std::vector<NSCam::Feature::P2Util::P2IO>   mMDPOuts;
    std::list<SmartTuningBuffer>                mTuningBufs;
};

class HelpReq
{
public:
    FeaturePipeParam::MSG_TYPE mCBMsg = FeaturePipeParam::MSG_INVALID;
    HelperMsg mHelperMsg              = HMSG_UNKNOWN;
    HelpReq();
    HelpReq(FeaturePipeParam::MSG_TYPE msg);
    HelpReq(FeaturePipeParam::MSG_TYPE msg, HelperMsg helperMsg);
    HelperMsg toHelperMsg() const;
};

class DepthImg
{
public:
    BasicImg  mCleanYuvImg;
    ImgBuffer mDMBGImg;
    ImgBuffer mDepthMapImg;
    ImgBuffer mDepthIntensity;
    MBOOL     mDepthSucess = MFALSE;
    std::shared_ptr<IMetadata> mpCopyHalOut = nullptr;
    std::shared_ptr<IMetadata> mpCopyAppOut = nullptr;
};

class TPIRes
{
public:
    using BasicImgMap = std::map<unsigned, BasicImg>;
    BasicImgMap mSFP;
    BasicImgMap mTP;
    std::map<unsigned, IMetadata*> mMeta;

public:
    TPIRes();
    TPIRes(const BasicImg &yuv);
    TPIRes(const DualBasicImg &dual);
    MVOID add(const DepthImg &depth);
    MVOID setZoomROI(const MRectF &roi);
    MRectF getZoomROI() const;
    BasicImg getSFP(unsigned id) const;
    BasicImg getTP(unsigned id) const;
    IMetadata* getMeta(unsigned id) const;
    MVOID setSFP(unsigned id, const BasicImg &img);
    MVOID setTP(unsigned id, const BasicImg &img);
    MVOID setMeta(unsigned id, IMetadata *meta);
    MUINT32 getImgArray(TPI_Image imgs[], unsigned count) const;
    MUINT32 getMetaArray(TPI_Meta metas[], unsigned count) const;
    MVOID updateViewInfo(const char* name, const Feature::ILog &log, const TPI_Image imgs[], unsigned count);

private:
    TPI_ViewInfo makeViewInfo(unsigned id, const BasicImg &img) const;
    MVOID updateViewInfo(const char* name, const Feature::ILog &log, const TPI_Image &img, unsigned srcID, unsigned dstID);

private:
    MRectF mZoomROI;
};

typedef Data<ImgBuffer> ImgBufferData;
typedef Data<EIS_HAL_CONFIG_DATA> EisConfigData;
typedef Data<MyFace> FaceData;
typedef Data<FeaturePipeParam::MSG_TYPE> CBMsgData;
typedef Data<HelpReq> HelperData;
typedef Data<RSCResult> RSCData;
typedef Data<DSDNImg> DSDNData;
typedef Data<BasicImg> BasicImgData;
typedef Data<DualBasicImg> DualBasicImgData;
typedef Data<DepthImg> DepthImgData;
typedef Data<P2AMDPReq> P2AMDPReqData;
typedef Data<TPIRes> TPIData;
typedef Data<VMDPReq> VMDPReqData;
typedef Data<WarpController> WarpControlData;

BasicImgData toBasicImgData(const DualBasicImgData &data);
VMDPReqData toVMDPReqData(const DualBasicImgData &data);

} // namespace NSFeaturePipe
} // namespace NSCamFeature
} // namespace NSCam

#endif // _MTK_CAMERA_STREAMING_FEATURE_PIPE_STREAMING_FEATURE_DATA_H_

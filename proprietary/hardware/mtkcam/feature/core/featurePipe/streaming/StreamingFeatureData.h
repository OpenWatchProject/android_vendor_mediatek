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
//#include <mtkcam/feature/featurePipe/IStreamingFeaturePipe.h>
//#include <mtkcam/common/faces.h>
//#include <mtkcam/featureio/eis_type.h>
#include <camera_custom_eis.h>

#include <utils/RefBase.h>
#include <utils/Vector.h>

#include <featurePipe/core/include/WaitQueue.h>
#include <featurePipe/core/include/IIBuffer.h>
#include <featurePipe/vsdof/util/TuningBufferPool.h>

#include "StreamingFeatureTimer.h"
#include "StreamingFeaturePipeUsage.h"

#include "EISQControl.h"
#include <mtkcam/feature/fsc/fsc_defs.h>

namespace NSCam {
namespace NSCamFeature {
namespace NSFeaturePipe {

enum Domain { RRZO_DOMAIN, WARP_DOMAIN };

class StreamingFeatureRequest : public virtual android::RefBase
{
private:
    // must allocate extParam before everything else
    FeaturePipeParam mExtParam;

public:
    // alias members, do not change initialize order
    VarMap &mVarMap;
    NSCam::NSIoPipe::QParams &mQParams;
    android::Vector<NSCam::NSIoPipe::FrameParams> &mvFrameParams;

    const StreamingFeaturePipeUsage &mPipeUsage;
    MUINT32 mFeatureMask;
    const MUINT32 mRequestNo;
    const MUINT32 mRecordNo;
    IStreamingFeaturePipe::eAppMode mAppMode;
    StreamingFeatureTimer mTimer;
    NSCam::NSIoPipe::QParams mP2A_QParams;
    MSize mFullImgSize;
    NSCam::NSIoPipe::MCropRect mFullImgCrop;
    NSCam::NSIoPipe::MCropRect mMDPCrop;
    NSCam::NSIoPipe::QParams mN3D_P2Node_QParams;
    MINT32 mDebugDump;
    NSCam::FSC::FSC_CROPPING_RESULT_STRUCT fsc_cropping_result;

public:
    StreamingFeatureRequest(const StreamingFeaturePipeUsage &pipeUsage, const FeaturePipeParam &extParam, MUINT32 requestNo, MUINT32 recordNo, const EISQState &eisQ);
    ~StreamingFeatureRequest();

    MVOID setDisplayFPSCounter(FPSCounter *counter);
    MVOID setFrameFPSCounter(FPSCounter *counter);

    MBOOL updateResult(MBOOL result);
    MBOOL doExtCallback(FeaturePipeParam::MSG_TYPE msg);

    MSize getMaxOutSize() const;
    MSize getInputSize() const;
    NSCam::NSIoPipe::MCropRect getP2Crop() const;
    IImageBuffer* getInputBuffer();
    MBOOL getDisplayOutput(NSCam::NSIoPipe::Output &output);
    MBOOL getRecordOutput(NSCam::NSIoPipe::Output &output);
    MBOOL getExtraOutput(NSCam::NSIoPipe::Output &output);
    MBOOL getFDOutput(NSCam::NSIoPipe::Output &output);
    IImageBuffer* getRecordOutputBuffer();

    MBOOL getDisplayCrop(NSCam::NSIoPipe::MCrpRsInfo &crop, Domain domain=RRZO_DOMAIN, double cropRatio=0.0, MRectF *cropF = NULL);
    MBOOL getRecordCrop(NSCam::NSIoPipe::MCrpRsInfo &crop, Domain domain=RRZO_DOMAIN, double cropRatio=0.0, MRectF *cropF = NULL);
    MBOOL getExtraCrop(NSCam::NSIoPipe::MCrpRsInfo &crop, Domain domain=RRZO_DOMAIN, double cropRatio=0.0, MRectF *cropF = NULL);
    MBOOL getFDCrop(NSCam::NSIoPipe::MCrpRsInfo &crop, Domain domain=RRZO_DOMAIN, double cropRatio=0.0);

    MSize getEISInputSize() const;
    MSize getFOVAlignSize() const;

    MBOOL getWDMADpPqParam(DpPqParam &dpPqParam) const;
    MBOOL getWROTDpPqParam(DpPqParam &dpPqParam) const;
    MBOOL getPQParam(NSCam::NSIoPipe::PQParam &pqParam) const;

    DECLARE_VAR_MAP_INTERFACE(mVarMap, setVar, getVar, tryGetVar, clearVar);

    MVOID setDumpProp(MINT32 start, MINT32 count, MBOOL byRecordNo);
    MVOID setForceIMG3O(MBOOL forceIMG3O);
    MVOID setForceWarpPass(MBOOL forceWarpPass);
    MVOID setForceGpuOut(MUINT32 forceGpuOut);
    MVOID setForceGpuRGBA(MBOOL forceGpuRGBA);
    MVOID setForcePrintIO(MBOOL forcePrintIO);

    const char* getFeatureMaskName() const;
    MBOOL need3DNR() const;
    MBOOL needVHDR() const;
    MBOOL needVFB() const;
    MBOOL needExVFB() const;
    MBOOL needEIS() const;
    MBOOL needEIS22() const;
    MBOOL needEIS25() const;
    MBOOL needEIS30() const;
    MBOOL needVendor() const;
    MBOOL needVendorMDP() const;
    MBOOL needVendorFOV() const;
    MBOOL needVendorFullImg() const;
    MBOOL needEarlyFSCVendorFullImg() const;
    MBOOL needWarp() const;
    MBOOL needFullImg() const;
    MBOOL needDsImg() const;
    MBOOL needFEFM() const;
    MBOOL needEarlyDisplay() const;
    MBOOL needP2AEarlyDisplay() const;
    MBOOL skipMDPDisplay() const;
    MBOOL needRSC() const;
    MBOOL needFSC() const;
    MBOOL needDump() const;
    MBOOL isLastNodeP2A() const;
    MBOOL is4K2K() const;
    MBOOL isP2ACRZMode() const;
    EISQ_ACTION getEISQAction() const;
    MUINT32 getEISQCounter() const;
    MBOOL useWarpPassThrough() const;
    MBOOL useDirectGpuOut() const;
    MBOOL needPrintIO() const;
    MUINT32 getMasterID() const;
    MBOOL needFOV() const;
    MBOOL needFOVFEFM() const;
    MBOOL isOnFOVSensor() const;
    MBOOL needEISFullImg() const;
    MBOOL needP2AEarlyEISFullImg() const;
    MBOOL needVMDPEISFullImg() const;
    MBOOL needHWFOVWarp() const;

    MSize getFOVMarginPixel() const;
    MSize getFSCMaxMarginPixel() const;
    MSizeF getEISMarginPixel() const;
    MRectF getEISCropRegion() const;
    MSizeF getFSCMarginPixel() const;
    MRectF getFSCCropRegion() const;
    MBOOL needN3D() const;
    MBOOL needVendorFOVFullImg() const;


private:
    MVOID checkEISQControl();

    MBOOL getCropInfo(NSCam::NSIoPipe::EPortCapbility cap, MUINT32 defCropGroup, NSCam::NSIoPipe::MCrpRsInfo &crop, MRectF *cropF = NULL);
    NSCam::NSIoPipe::MCrpRsInfo applyCropRatio(NSCam::NSIoPipe::MCrpRsInfo crop, Domain domain, double ratio, const MSize &maxSize = MSize(0, 0));

    void appendEisTag(string& str, MUINT32 mFeatureMask) const;
    void append3DNRTag(string& str, MUINT32 mFeatureMask) const;
    void appendFSCTag(string& str, MUINT32 mFeatureMask) const;
    void appendVendorTag(string& str, MUINT32 mFeatureMask) const;
    void appendFOVTag(string& str, MUINT32 mFeatureMask) const;
    void appendN3DTag(string& str, MUINT32 mFeatureMask) const;
    void appendNoneTag(string& str, MUINT32 mFeatureMask) const;
    void appendDefaultTag(string& str, MUINT32 mFeatureMask) const;
    void appendVendorFOVTag(string& str, MUINT32 mFeatureMask) const;

private:
    FPSCounter *mDisplayFPSCounter;
    FPSCounter *mFrameFPSCounter;

    static std::unordered_map<MUINT32, std::string> mFeatureMaskNameMap;

private:
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
};
typedef android::sp<StreamingFeatureRequest> RequestPtr;
typedef android::sp<IIBuffer> ImgBuffer;

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

class VFBResult
{
public:
    ImgBuffer mDsImg;
    ImgBuffer mAlphaCL;
    ImgBuffer mAlphaNR;
    ImgBuffer mPCA;

    VFBResult();
    VFBResult(const ImgBuffer &dsImg, const ImgBuffer &alphaCL, const ImgBuffer &alphaNR, const ImgBuffer &pca);
};

class FEFMGroup
{
public:
    ImgBuffer High;
    ImgBuffer Medium;
    ImgBuffer Low;

    ImgBuffer Register_High;
    ImgBuffer Register_Medium;
    ImgBuffer Register_Low;

    MVOID clear();
    MBOOL isValid() const;
};

class FMResult
{
public:
    FEFMGroup FM_A;
    FEFMGroup FM_B;
    FEFMGroup FE;
    FEFMGroup PrevFE;
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

class FovP2AResult
{
public:
    ImgBuffer mFEO_Master;
    ImgBuffer mFEO_Slave;
    ImgBuffer mFMO_MtoS;
    ImgBuffer mFMO_StoM;
    SmartTuningBuffer mFMTuningBuf0;
    SmartTuningBuffer mFMTuningBuf1;
    MSize mFEInSize_Master;
    MSize mFEInSize_Slave;
};

class FOVResult
{
public:
    ImgBuffer mWarpMap;
    MSize     mWarpMapSize;
    MSize     mWPESize;
    MRect     mDisplayCrop;
    MRect     mRecordCrop;
    MRect     mExtraCrop;
    MSize     mSensorBaseMargin;
    MSize     mRRZOBaseMargin;
    MPoint    mFOVShift;
    float     mFOVScale;
};

class BasicImg
{
public:
    ImgBuffer    mBuffer;
    MSizeF       mDomainOffset;
    MBOOL        mIsReady;

    BasicImg();
    BasicImg(const ImgBuffer &img);
    BasicImg(const ImgBuffer &img, const MSizeF &offset);
    BasicImg(const ImgBuffer &img, const MSizeF &offset, const MBOOL &isReady);
};

class BasicIOImg
{
public:
    BasicImg mIn;
    BasicImg mOut;

    BasicIOImg();
    BasicIOImg(const BasicImg &in);
    BasicIOImg(const BasicImg &in, const BasicImg &out);
};

class N3DResult
{
public:
    ImgBuffer mFEBInputImg_Master;
    ImgBuffer mFEBInputImg_Slave;
    ImgBuffer mFECInputImg_Master;
    ImgBuffer mFECInputImg_Slave;
    ImgBuffer mFEBO_Master;
    ImgBuffer mFEBO_Slave;
    ImgBuffer mFECO_Master;
    ImgBuffer mFECO_Slave;
    ImgBuffer mFMBO_MtoS;
    ImgBuffer mFMBO_StoM;
    ImgBuffer mFMCO_MtoS;
    ImgBuffer mFMCO_StoM;
    ImgBuffer mCCin_Master;
    ImgBuffer mCCin_Slave;
    ImgBuffer mRectin_Master;
    ImgBuffer mRectin_Slave;
    SmartTuningBuffer tuningBuf1;
    SmartTuningBuffer tuningBuf2;
    SmartTuningBuffer tuningBuf3;
    SmartTuningBuffer tuningBuf4;
    SmartTuningBuffer tuningBuf5;
    SmartTuningBuffer tuningBuf6;
    SmartTuningBuffer tuningBuf7;
    SmartTuningBuffer tuningBuf8;
};

class DualBasicIOImg
{
public:
    BasicIOImg mMaster;
    BasicIOImg mSlave;
    DualBasicIOImg();
    DualBasicIOImg(const BasicIOImg &master, const BasicIOImg &slave);
};

typedef Data<ImgBuffer> ImgBufferData;
typedef Data<EIS_HAL_CONFIG_DATA> EisConfigData;
typedef Data<MyFace> FaceData;
typedef Data<VFBResult> VFBData;
typedef Data<FMResult> FMData;
typedef Data<FeaturePipeParam::MSG_TYPE> CBMsgData;
typedef Data<RSCResult> RSCData;
typedef Data<FovP2AResult> FOVP2AData;
typedef Data<FOVResult> FOVData;
typedef Data<BasicImg> BasicImgData;
typedef Data<BasicIOImg> BasicIOImgData;
typedef Data<N3DResult> N3DData;
typedef Data<DualBasicIOImg> DualBasicIOImgData;

} // namespace NSFeaturePipe
} // namespace NSCamFeature
} // namespace NSCam

#endif // _MTK_CAMERA_STREAMING_FEATURE_PIPE_STREAMING_FEATURE_DATA_H_

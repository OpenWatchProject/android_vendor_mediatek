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

#include "P2ANode.h"

#define PIPE_CLASS_TAG "P2ANode"
#define PIPE_TRACE TRACE_P2A_NODE
#include <featurePipe/core/include/PipeLog.h>

#include <mtkcam/aaa/IIspMgr.h>
#include <isp_tuning/isp_tuning.h>
#include <mtkcam/utils/metadata/hal/mtk_platform_metadata_tag.h>

#include <mtkcam/drv/iopipe/SImager/IImageTransform.h>
#include <mtkcam/drv/iopipe/PostProc/INormalStream.h>
#include <mtkcam/drv/IHalSensor.h>
#include <mtkcam/drv/def/Dip_Notify_datatype.h>
#include <mtkcam/utils/TuningUtils/FileReadRule.h>
#include <mtkcam/utils/exif/DebugExifUtils.h>
#include <mtkcam/utils/TuningUtils/CommonRule.h>
//
#include <mtkcam3/feature/lcenr/lcenr.h>
//
#include <cmath>
#include <camera_custom_nvram.h>
#include <debug_exif/cam/dbg_cam_param.h>
//
#include "../thread/CaptureTaskQueue.h"
#include <MTKDngOp.h>
//
#include <mtkcam3/feature/utils/ISPProfileMapper.h>

CAM_ULOG_DECLARE_MODULE_ID(MOD_CAPTURE_P2A);

using namespace NSCam::NSCamFeature::NSFeaturePipe;
using namespace NSCam::NSIoPipe;
using namespace NSCam::TuningUtils;
using namespace NSCam::NSIoPipe::NSSImager;
using namespace NSCam::NSPipelinePlugin;
using namespace NSIspTuning;

#define ISP30_RULE01_CROP_OFFSET         (196)
#define ISP30_RULE02_CROP_OFFSET         (128)
#define ISP30_RULE02_RESIZE_RATIO        (8)

#if (MTK_CAPTURE_ISP_VERSION == 6)
#include <isp_tuning.h>
#endif

namespace NSCam {
namespace NSCamFeature {
namespace NSFeaturePipe {
namespace NSCapture {

// flag for workround that resize yuv force use the time sharing mode
static MBOOL USEING_TIME_SHARING = MTRUE;

enum DeubgMode {
    AUTO    = -1,
    OFF     = 0,
    ON      = 1,
};

enum P2Task {
    TASK_M1_RRZO,
    TASK_M1_IMGO_RAW,
    TASK_M1_IMGO_PRE,
    TASK_M1_IMGO,
    TASK_M2_RRZO,
    TASK_M2_IMGO,
};

// copy buffer by using cpu
MERROR P2ANode::directBufCopy(const IImageBuffer* pSrcBuf, IImageBuffer* pDstBuf, QParams& rParams)
{
    CAM_TRACE_BEGIN("p2a:directBufCopy");
    if(pSrcBuf->getBufSizeInBytes(0) > pDstBuf->getBufSizeInBytes(0))
    {
        MY_LOGE("directBufCopy fail due to source buffer larger than destination, pSrcBuf/pDstBuf: %zu/%zu", pSrcBuf->getBufSizeInBytes(0), pDstBuf->getBufSizeInBytes(0));
        this->onP2FailedCallback(rParams);
        CAM_TRACE_END();
        return BAD_VALUE;
    }
    size_t src_stride = pSrcBuf->getBufStridesInBytes(0);
    size_t dst_stride = pDstBuf->getBufStridesInBytes(0);
    MY_LOGD("directBufCopy start, pSrcBuf/stride: %p/%zu, pDstBuf/strid: %p/%zu", pSrcBuf, src_stride, pDstBuf, dst_stride);
    if(src_stride == dst_stride)
    {
        memcpy(reinterpret_cast<void*>(pDstBuf->getBufVA(0)), reinterpret_cast<void*>(pSrcBuf->getBufVA(0)), pSrcBuf->getBufSizeInBytes(0));
    }
    else
    {
        char* psrcBufferVa = (char *) (pSrcBuf->getBufVA(0));
        char* pdstBufferVa = (char *) (pDstBuf->getBufVA(0));
        for(int i=0 ; i<pSrcBuf->getImgSize().h ; i++)
        {
            memcpy(reinterpret_cast<void*>(pdstBufferVa + i*dst_stride), reinterpret_cast<void*>(psrcBufferVa + i*src_stride), src_stride);
        }
    }
    pDstBuf->syncCache(eCACHECTRL_FLUSH);
    MY_LOGD("directBufCopy finished");
    this->onP2SuccessCallback(rParams);
    CAM_TRACE_END();
    return OK;
}

template<typename T>
static inline MBOOL isMulticamMode(T& t)
{
    const MUINT32 dualMode = t.mDualMode;
    const MBOOL isMulticam = (dualMode & v1::Stereo::E_STEREO_FEATURE_MULTI_CAM);
    return isMulticam;
}

MBOOL getIsSupportCroppedFSYUV(CaptureFeatureInferenceData& rInfer)
{
    static const MBOOL isForceEnable = []()
    {
        const char* prop = "vendor.debug.camera.capture.enable.croppedfsyuv";
        MBOOL ret = property_get_int32(prop, 0);
        MY_LOGD("prop:%s, value:%d", prop, ret);
        return (ret != 0);
    }();

    MBOOL ret = MFALSE;
    // TODO: determine witch feature we need to do croping for FSRAW to dFSYUV in P2ANode
    // ex: const MBOOL isFeatureEnable = rInfer.hasFeature(FID_NR);
    const MBOOL isFeatureEnable = 0;
    if (isFeatureEnable || isForceEnable) {
        MY_LOGD("is need cropped FSYUV, isFeatureEnable:%d, isForceEnable:%d", isFeatureEnable, isForceEnable);
        ret = MTRUE;
    }
    return ret;
}

template<typename T>
static inline MBOOL isVSDoFMode(T& t)
{
    const MUINT32 dualMode = t.mDualMode;
    const MBOOL isVsdof = (dualMode & v1::Stereo::E_STEREO_FEATURE_CAPTURE);
    return isVsdof;
}

MVOID P2ANode::unpackRawBuffer(IImageBuffer* pInPackBuf, IImageBuffer* pOutUnpackBuf )
{
    DngOpResultInfo dngResult;
    DngOpImageInfo dngImage;

    int i4ImgWidth  = pInPackBuf->getImgSize().w;
    int i4ImgHeight = pInPackBuf->getImgSize().h;
    int i4BufSize   = pInPackBuf->getBufSizeInBytes(0);
    int i4ImgStride = pInPackBuf->getBufStridesInBytes(0);
    MUINT32 u4BufSize = DNGOP_BUFFER_SIZE(i4ImgWidth * 2, i4ImgHeight);

    // unpack algorithm
    MY_LOGD("Unpack +");
    MTKDngOp *pDngOp = MTKDngOp::createInstance(DRV_DNGOP_UNPACK_OBJ_SW);
    dngImage.Width = i4ImgWidth;
    dngImage.Height = i4ImgHeight;
    dngImage.Stride_src = i4ImgStride;
    dngImage.Stride_dst = pOutUnpackBuf->getBufStridesInBytes(0);
    dngImage.BIT_NUM = 10;
    dngImage.BIT_NUM_DST = 10;
    dngImage.Buff_size = u4BufSize;
    dngImage.srcAddr     = (void *) (pInPackBuf->getBufVA(0));
    dngResult.ResultAddr = (void *) (pOutUnpackBuf->getBufVA(0));
    pDngOp->DngOpMain((void*)&dngImage, (void*)&dngResult);
    pDngOp->destroyInstance(pDngOp);
    MY_LOGD("Unpack -");
    MY_LOGD("unpack processing. va[in]:%p, va[out]:%p", dngImage.srcAddr, dngResult.ResultAddr);
    MY_LOGD("img size(%dx%d) src stride(%d) bufSize(%d) -> dst stride(%d) bufSize(%zu)", i4ImgWidth, i4ImgHeight,
             dngImage.Stride_src,i4BufSize, dngImage.Stride_dst , pOutUnpackBuf->getBufSizeInBytes(0));
}

P2ANode::P2ANode(NodeID_T nid, const char* name, MINT32 policy, MINT32 priority)
    : CamNodeULogHandler(Utils::ULog::MOD_CAPTURE_P2A)
    , CaptureFeatureNode(nid, name, 0, policy, priority)
    , mSensorFmt(-1)
    , mSensorFmt2(-1)
    , mDipVer(0)
    , mSupportDRE(MFALSE)
    , mSupportCZ(MFALSE)
    , mSupportHFG(MFALSE)
    , mSupportDCE(MFALSE)
    , mSupportMDPQoS(MFALSE)
    , mSupportDS(MFALSE)
    , mISP3_0(MFALSE)
    , mISP4_0(MFALSE)
    , mISP5_0(MFALSE)
    , mISP6_0(MFALSE)
    , mDebugDCE(-1)
    , mDebugHFG(-1)
    , mDCES_Size(0, 0)
    , mDCES_Format(0)
    , mDualSyncInfo_Size(0)
    , mpISPProfileMapper(nullptr)
{
    TRACE_FUNC_ENTER();
    this->addWaitQueue(&mRequestPacks);

    mDebugPerFrame          = property_get_int32("vendor.debug.camera.p2c.perframe", 0);
    mDebugDS                = property_get_int32("vendor.debug.camera.ds.enable", -1);
    mDebugDSRatio_dividend  = property_get_int32("vendor.debug.camera.ds.ratio_dividend", 8);
    mDebugDSRatio_divisor   = property_get_int32("vendor.debug.camera.ds.ratio_divisor", 16);
    mDebugDRE               = property_get_int32("vendor.debug.camera.dre.enable", -1);
    mDebugCZ                = property_get_int32("vendor.debug.camera.cz.enable", -1);
    mSupport10Bits          = property_get_int32("vendor.debug.p2c.10bits.enable", 1);
    mSupportDS              = property_get_int32("persist.vendor.camera.ds.enable", 1);

#ifdef SUPPORT_HFG
    mDebugHFG               = property_get_int32("vendor.debug.camera.hfg.enable", -1);
#endif

#ifdef SUPPORT_DCE
    mDebugDCE               = property_get_int32("vendor.debug.camera.dce.enable", -1);
#endif
    mDebugLoadIn            = (property_get_int32("vendor.debug.camera.dumpin.en", 0) == 2);

    mDebugDump              = property_get_int32("vendor.debug.camera.p2.dump", 0) > 0;
    mDebugTimgo             = property_get_int32("vendor.debug.camera.timgo.dump", 0) > 0;
    mDebugTimgoType         = property_get_int32("vendor.debug.camera.timgo.type", 0);
    mDebugImg2o             = property_get_int32("vendor.debug.camera.img2o.dump", 0) > 0;
    mDebugImg3o             = property_get_int32("vendor.debug.camera.img3o.dump", 0) > 0;
    mDumpIMGI              = property_get_int32("vendor.debug.camera.imgi.dump", 0) > 0;
    mDumpLCEI              = property_get_int32("vendor.debug.camera.lcei.dump", 0) > 0;
    mDumpMFNRYuv           = property_get_int32("vendor.debug.camera.mfnr.yuv.dump", 0) > 0;
    // Enable Dump
    if (mDebugDump) {
        mDebugDumpFilter    = property_get_int32("vendor.debug.camera.p2.dump.filter", 0xFFFF);
        mDebugUnpack        = property_get_int32("vendor.debug.camera.upkraw.dump", 0) > 0;
    } else {
        mDebugDumpFilter    = 0;
        if (mDebugImg3o) {
            mDebugDumpFilter |= DUMP_ROUND1_IMG3O;
            mDebugDumpFilter |= DUMP_ROUND2_IMG3O;
            mDebugDump      = MTRUE;
        }

        if (mDebugImg2o) {
            mDebugDumpFilter |= DUMP_ROUND1_IMG2O;
            mDebugDumpFilter |= DUMP_ROUND2_IMG2O;
            mDebugDump      = MTRUE;
        }

        if (mDebugTimgo) {
            mDebugDumpFilter |= DUMP_ROUND1_TIMGO;
            mDebugDumpFilter |= DUMP_ROUND2_TIMGO;
            mDebugDump      = MTRUE;

        }
        mDebugUnpack        = 0;
    }

#if MTKCAM_TARGET_BUILD_VARIANT != 'u'
    mDebugDumpMDP           = MTRUE;
#else
    mDebugDumpMDP           = property_get_int32("vendor.debug.camera.mdp.dump", 0) > 0;
#endif

    TRACE_FUNC_EXIT();
}

P2ANode::~P2ANode()
{
    TRACE_FUNC_ENTER();

    TRACE_FUNC_EXIT();
}

MVOID P2ANode::setBufferPool(const android::sp<CaptureBufferPool> &pool)
{
    TRACE_FUNC_ENTER();
    mpBufferPool = pool;
    TRACE_FUNC_EXIT();
}

MVOID P2ANode::setSensorFmt(MINT32 sensorIndex, MINT32 sensorIndex2)
{
    mSensorFmt = (sensorIndex >= 0) ? getSensorRawFmt(sensorIndex) : SENSOR_RAW_FMT_NONE;
    mSensorFmt2 = (sensorIndex2 >= 0) ? getSensorRawFmt(sensorIndex2) : SENSOR_RAW_FMT_NONE;
    MY_LOGD("sensorIndex:%d -> sensorFmt:%#09x, sensorIndex2:%d -> sensorFmt2:%#09x",
        sensorIndex, mSensorFmt, sensorIndex2, mSensorFmt2);
}

MBOOL P2ANode::checkHasYUVOutput(sp<CaptureFeatureNodeRequest> pNodeReq)
{
    // check if there is any output in request
    const TypeID_T output_typeIds[] = {
            TID_MAN_FULL_YUV,
            TID_MAN_FULL_PURE_YUV,
            TID_JPEG,
            TID_MAN_CROP1_YUV,
            TID_MAN_CROP2_YUV,
            TID_MAN_CROP3_YUV,
            TID_MAN_SPEC_YUV,
            TID_MAN_FD_YUV,
            TID_POSTVIEW,
            TID_THUMBNAIL,
            TID_MAN_CLEAN
    };

    MBOOL hasYUVOutput = MFALSE;
    for (TypeID_T typeId : output_typeIds) {
        BufferID_T bufId = pNodeReq->mapBufferID(typeId, OUTPUT);
        if (bufId == NULL_BUFFER)
          continue;

        IImageBuffer* pBuf = pNodeReq->acquireBuffer(bufId);
        if (pBuf != NULL) {
            MY_LOGI("request has yuv output. type:%d, buf:%d",typeId, bufId);
            hasYUVOutput = MTRUE;
            break;
        }
    }
    if(!hasYUVOutput) {
        MY_LOGD("request has no yuv output");
    }

    return hasYUVOutput;
}

MBOOL P2ANode::onInit()
{
    TRACE_FUNC_ENTER();
    CaptureFeatureNode::onInit();

    for (size_t i = 0; i < mSensorList.size(); i++) {
        MINT32 sensorId = mSensorList[i];
        std::string userName = "CFP" + to_string(sensorId);
        ISPOperator opt;
        opt.mIspHal = MAKE_HalISP(sensorId, userName.c_str());
        opt.mP2Opt = new P2Operator(LOG_TAG, sensorId);
        mISPOperators[sensorId] = opt;
    }

    if (!NSIoPipe::NSPostProc::INormalStream::queryDIPInfo(mDipInfo)) {
        MY_LOGE("queryDIPInfo Error! Please check the error msg above!");
    }

    std::map<DP_ISP_FEATURE_ENUM, bool> mdpFeatures;
    DpIspStream::queryISPFeatureSupport(mdpFeatures);
    mSupportCZ = mdpFeatures[ISP_FEATURE_CLEARZOOM];
    mSupportDRE = mdpFeatures[ISP_FEATURE_DRE];
    mSupportHFG = mdpFeatures[ISP_FEATURE_HFG];
    if (mSupportDRE) {
        mSupportDRE = property_get_int32("persist.vendor.camera.dre.enable", 1);
    }

    mDipVer = mDipInfo[EDIPINFO_DIPVERSION];
    mISP3_0 = (mDipVer == EDIPHWVersion_30);
    mISP4_0 = (mDipVer == EDIPHWVersion_40);
    mISP5_0 = (mDipVer == EDIPHWVersion_50);
    mISP6_0 = (mDipVer == EDIPHWVersion_60);

    if (mISP6_0) {
        NS3Av3::Buffer_Info info;
        if (mISPOperators[mSensorList[0]].mIspHal->queryISPBufferInfo(info) && info.DCESO_Param.bSupport)
        {
            mSupportDCE  = MTRUE;
            mDCES_Size   = info.DCESO_Param.size;
            mDCES_Format = info.DCESO_Param.format;
            mDualSyncInfo_Size = info.u4DualSyncInfoSize;
        }
    } else if (mISP5_0) {
        NS3Av3::IIspMgr* ispMgr = MAKE_IspMgr();
        if (ispMgr != NULL) {
            mDualSyncInfo_Size = ispMgr->queryDualSyncInfoSize();
        }
    }

#ifdef SUPPORT_MDPQoS
    mSupportMDPQoS = MTRUE;
    MY_LOGD("support MDP QoS %d", mSupportMDPQoS);
#else
    if (mISP6_0) {
        mSupportMDPQoS = MTRUE;
    }
#endif

    MY_LOGD("Support CZ(%d) DRE(%d) HFG(%d) DCE(%d)",
            mSupportCZ, mSupportDRE, mSupportHFG, mSupportDCE);

    TRACE_FUNC_EXIT();
    return MTRUE;
}

MBOOL P2ANode::onUninit()
{
    TRACE_FUNC_ENTER();

    for (size_t i = 0; i < mSensorList.size(); i++) {
        std::string userName = "CFP" + to_string(mSensorList[i]);
        ISPOperator opt = mISPOperators[mSensorList[i]];
        if (opt.mIspHal) {
            opt.mIspHal->destroyInstance(userName.c_str());
            opt.mIspHal = NULL;
        }
    }
    mISPOperators.clear();

    TRACE_FUNC_EXIT();
    return MTRUE;
}

/*******************************************************************************
 *
 ********************************************************************************/
MBOOL
P2ANode::
enqueISP(RequestPtr& pRequest, shared_ptr<P2EnqueData>& pEnqueData)
{
    TRACE_FUNC_ENTER();

    MERROR ret = OK;
    P2EnqueData& enqueData = *pEnqueData.get();

    // Trigger Dump
    enqueData.mDebugDump = mDebugDump;
    enqueData.mDebugDumpMDP = mDebugDumpMDP;
    enqueData.mDebugUnpack = mDebugUnpack;
    enqueData.mDebugDumpFilter = mDebugDumpFilter;
    MINT32& frameNo = enqueData.mFrameNo;
    MINT32& requestNo = enqueData.mRequestNo;

    MBOOL master = (enqueData.mSensorId == pRequest->getMainSensorIndex());
    MBOOL bYuvTunning = enqueData.mRound == 2 || enqueData.mYuvRep;
    const MBOOL isMulticam = isMulticamMode(mUsageHint);

    sp<CaptureFeatureNodeRequest> pNodeReq = pRequest->getNodeRequest(pEnqueData->mNodeId);

    auto GetBuffer = [&](IImageBuffer*& rpBuf, BufferID_T bufId) -> IImageBuffer* {
        if (rpBuf)
            return rpBuf;
        if (bufId != NULL_BUFFER) {
            rpBuf = pNodeReq->acquireBuffer(bufId);
        }
        return rpBuf;
    };

    EnquePackage* pPackage = NULL;

#define ASSERT(predict, mesg)     \
    do {                          \
        if (!(predict)) {         \
            if (pPackage != NULL) \
                delete pPackage;  \
            MY_LOGE(mesg);        \
            return MFALSE;        \
        }                         \
    } while(0)

    // 1.input & output data
    IImageBuffer* pIMG2O = GetBuffer(enqueData.mIMG2O.mpBuf, enqueData.mIMG2O.mBufId);
    IImageBuffer* pIMG3O = GetBuffer(enqueData.mIMG3O.mpBuf, enqueData.mIMG3O.mBufId);
    IImageBuffer* pTIMGO = GetBuffer(enqueData.mTIMGO.mpBuf, enqueData.mTIMGO.mBufId);
    IImageBuffer* pDCESO = GetBuffer(enqueData.mDCESO.mpBuf, enqueData.mDCESO.mBufId);
    IImageBuffer* pWROTO = GetBuffer(enqueData.mWROTO.mpBuf, enqueData.mWROTO.mBufId);
    IImageBuffer* pWDMAO = GetBuffer(enqueData.mWDMAO.mpBuf, enqueData.mWDMAO.mBufId);
    IImageBuffer* pCopy1 = GetBuffer(enqueData.mCopy1.mpBuf, enqueData.mCopy1.mBufId);
    IImageBuffer* pCopy2 = GetBuffer(enqueData.mCopy2.mpBuf, enqueData.mCopy2.mBufId);

    ASSERT(!!pIMG2O || !!pIMG3O || !!pWROTO || !!pWDMAO || !!pTIMGO, "do not acquire a output buffer");

    IMetadata* pIMetaDynamic    = enqueData.mpIMetaDynamic;
    IMetadata* pIMetaApp        = enqueData.mpIMetaApp;
    IMetadata* pIMetaHal        = enqueData.mpIMetaHal;
    IMetadata* pOMetaApp        = enqueData.mpOMetaApp;
    IMetadata* pOMetaHal        = enqueData.mpOMetaHal;

    IImageBuffer* pIMGI = GetBuffer(enqueData.mIMGI.mpBuf, enqueData.mIMGI.mBufId);
    IImageBuffer* pLCEI = GetBuffer(enqueData.mLCEI.mpBuf, enqueData.mLCEI.mBufId);
    IImageBuffer* pDCESI = GetBuffer(enqueData.mDCESI.mpBuf, enqueData.mDCESI.mBufId);

    ASSERT(!!pIMGI, "do not acquire a input buffer");
    //
    MBOOL bIMGIRawFmt = NSCam::isHalRawFormat((EImageFormat)pIMGI->getImgFormat());
    MBOOL bHasOutYuv = pIMG2O || pIMG3O || pWROTO || pWDMAO;
    MINT32 isMainSensor = (pRequest->getMainSensorIndex() == enqueData.mSensorId);
    // Enable the on-line device tuning only in Raw2Yuv stage && only for main sensor
    enqueData.mDebugLoadIn = mDebugLoadIn && bIMGIRawFmt && bHasOutYuv && isMainSensor;
    MY_LOGD_IF(mLogLevel, "Online device tunine:%d, mDebugLoadIn:%d bIMGIRawFmt:%d bHasOutYuv:%d isMainSensor:%d",
                enqueData.mDebugLoadIn, mDebugLoadIn, bIMGIRawFmt, bHasOutYuv, isMainSensor);
    // read data for online-device tuning
    if (enqueData.mDebugLoadIn) {
        FileReadRule rule;
        String8 profileName;
        MINT32  index;
        if (pRequest->hasFeature(FID_AINR) || pRequest->hasFeature(FID_AINR_YHDR)) {
            // P2B is for single path, P2A is for Main path
            MY_LOGD("AINR Main path");
            index = 0;
            profileName = String8::format("EIspProfile_AINR_Main");
            pLCEI = pNodeReq->acquireBuffer(enqueData.mLCEI.mBufId);
            rule.getFile_LCSO(index, profileName.string(), pLCEI, "P2Node", enqueData.mSensorId);
        } else {
            if (pRequest->hasFeature(FID_MFNR)) { // MFNR
                profileName = String8::format("MFNR");
                index       = pRequest->getParameter(PID_FRAME_INDEX_FORCE_BSS);
            } else {
                profileName = String8::format("single_capture");
                index       = 0;
            }
            rule.getFile_RAW(index, profileName.string(), pIMGI, "P2Node", enqueData.mSensorId);
            pLCEI = pNodeReq->acquireBuffer(enqueData.mLCEI.mBufId);
            rule.getFile_LCSO(index, profileName.string(), pLCEI, "P2Node", enqueData.mSensorId);
        }
    }
    // to invalidate raw buffer because it is HW write before P2ANode
    if (mDebugDump)
        pIMGI->syncCache(eCACHECTRL_INVALID);

    // 2. Prepare enque package
    SmartTuningBuffer pBufTuning;
    IMetadata metaApp;
    IMetadata metaHal;
    MBOOL bMainFrame = MFALSE;
    MBOOL bSubFrame  = MFALSE;
    // For MFNR, All subframe's APPMeta & HalMeta are copy from the main frame
    if (enqueData.mEnableMFB && enqueData.mTaskId == TASK_M1_IMGO) {
        MUINT8 uTuningMode = 0;
        tryGetMetadata<MUINT8>(pIMetaHal, MTK_ISP_P2_TUNING_UPDATE_MODE, uTuningMode);

        switch (uTuningMode) {
            case 0:
                pBufTuning = mpBufferPool->getTuningBuffer();
                mpKeepTuningData = pBufTuning;

                bMainFrame       = MTRUE;
                break;
            case 2:
                if (mpKeepTuningData == NULL)
                    MY_LOGE("Should have a previous tuning data");
                else {
                    pBufTuning = mpBufferPool->getTuningBuffer();
                    // copy tuning buffer
                    memcpy((void*)pBufTuning->mpVA, (void*)mpKeepTuningData->mpVA, mpBufferPool->getTuningBufferSize());
                    // copy metadata
                    metaApp = mKeepMetaApp;
                    metaHal = mKeepMetaHal;
                    // overwrite its own magic number
                    MINT32 magicNum = -1;
                    if(tryGetMetadata<MINT32>(pIMetaHal, MTK_P1NODE_PROCESSOR_MAGICNUM, magicNum))
                        trySetMetadata<MINT32>(&metaHal, MTK_P1NODE_PROCESSOR_MAGICNUM, magicNum);
                    else
                        MY_LOGW("Failed to get MTK_P1NODE_PROCESSOR_MAGICNUM of subframes");

                    bSubFrame = MTRUE;
                }
                break;
            default:
                mpKeepTuningData = NULL;
                break;
        }
    }

    if (pBufTuning == NULL)
        pBufTuning = mpBufferPool->getTuningBuffer();

    PQParam* pPQParam = new PQParam();
    pPackage = new EnquePackage();
    pPackage->mpEnqueData = pEnqueData;
    pPackage->mpTuningData = pBufTuning;
    pPackage->mpPQParam = pPQParam;
    pPackage->mpNode = this;
    pPackage->mbNeedToDump = this->needToDump(pRequest);
    if (mDebugUnpack) {
        pPackage->mUnpackBuffer = mpBufferPool->getImageBuffer(
                enqueData.mIMGI.mpBuf->getImgSize(), eImgFmt_BAYER10_UNPAK);
    }

    // 3. Crop Calculation & add log
    const MSize& rImgiSize = pIMGI->getImgSize();
    String8 strEnqueLog;
    strEnqueLog += String8::format("Sensor(%d) Resized(%d) R/F Num: %d/%d, EnQ: IMGI Fmt(0x%x) Size(%dx%d) VA/PA(%#" PRIxPTR "/%#" PRIxPTR ")",
               enqueData.mSensorId,
               enqueData.mResized,
               pRequest->getRequestNo(),
               pRequest->getFrameNo(),
               pIMGI->getImgFormat(),
               rImgiSize.w, rImgiSize.h,
               pIMGI->getBufVA(0),
               pIMGI->getBufPA(0));

    sp<CropCalculator::Factor> pFactor;
    if (enqueData.mWDMAO.mHasCrop ||
        enqueData.mWROTO.mHasCrop ||
        enqueData.mIMG2O.mHasCrop ||
        enqueData.mCopy1.mHasCrop ||
        enqueData.mCopy2.mHasCrop)
    {
        pFactor = mpCropCalculator->getFactor(pIMetaApp, pIMetaHal, enqueData.mSensorId);
        ASSERT(pFactor != NULL, "can not get crop factor!");

        if (pOMetaApp != NULL) {
            MRect cropRegion = pFactor->mActiveCrop;
            if (pFactor->mEnableEis) {
                cropRegion.p.x += pFactor->mActiveEisMv.p.x;
                cropRegion.p.y += pFactor->mActiveEisMv.p.y;
            }
            // Update crop region to output app metadata
            trySetMetadata<MRect>(pOMetaApp, MTK_SCALER_CROP_REGION, cropRegion);
        }
    }

    auto GetCropRegion = [&](const char* sPort, P2Output& rOut, IImageBuffer* pImg) mutable {
        if (pImg == NULL)
            return;

        if (rOut.mHasCrop) {
            // Pass if already have
            if (!rOut.mCropRegion.s) {
                MSize cropSize = pImg->getImgSize();
                if (rOut.mTrans & eTransform_ROT_90)
                    swap(cropSize.w, cropSize.h);
                if (pRequest->hasParameter(PID_IGNORE_CROP)) {
                    CropCalculator::evaluate(rImgiSize, cropSize, rOut.mCropRegion);
                }
                else if (pRequest->hasParameter(PID_REPROCESSING)) {
                    CropCalculator::evaluate(rImgiSize, cropSize, pFactor->mActiveCrop, rOut.mCropRegion);
                } else {
                    mpCropCalculator->evaluate(pFactor, cropSize, rOut.mCropRegion, enqueData.mResized, MTRUE);
                }
                if (enqueData.mScaleType == eScale_Up) {
                    simpleTransform tranTG2DS(MPoint(0,0), enqueData.mScaleSize, rImgiSize);
                    rOut.mCropRegion = transform(tranTG2DS, rOut.mCropRegion);
                }
            }
        } else
            rOut.mCropRegion = MRect(rImgiSize.w, rImgiSize.h);

        // output buffer no need to fulfill
        const MBOOL isForceMargin = (property_get_int32("vendor.debug.camera.p2capture.forceMargin", 0) > 0);
        MBOOL isSetNoMargin = MTRUE;
        if (pIMetaApp != NULL) {
            tryGetMetadata<MINT32>(pIMetaApp, MTK_CONTROL_CAPTURE_YUV_NO_MARGIN, isSetNoMargin);
        }
        if ((!isSetNoMargin || isForceMargin) && pRequest->isTargetBuffer(rOut.mBufId)) {
            MY_LOGD("buffer Id(%d) is target buffer, setNoMargin(%d), forceMargin(%d)", rOut.mBufId, isSetNoMargin, isForceMargin);
            enqueData.mOriSize = pImg->getImgSize();
            enqueData.mDumpWithMargin = MTRUE;
            enqueData.mpHolder->mpHolders.push_back(IHolder::createResizerInstance(pImg, rOut.mCropRegion.s));
            updateSensorCropRegion(pRequest, rOut.mCropRegion);
        }

        strEnqueLog += String8::format(", %s Rot(%d) Crop(%d,%d)(%dx%d) Size(%dx%d) VA/PA(%#" PRIxPTR "/%#" PRIxPTR ") Format(%x)",
            sPort, rOut.mTrans,
            rOut.mCropRegion.p.x, rOut.mCropRegion.p.y,
            rOut.mCropRegion.s.w, rOut.mCropRegion.s.h,
            pImg->getImgSize().w, pImg->getImgSize().h,
            pImg->getBufVA(0), pImg->getBufPA(0), pImg->getImgFormat());
    };

    GetCropRegion("WDMAO", enqueData.mWDMAO, pWDMAO);
    GetCropRegion("WROTO", enqueData.mWROTO, pWROTO);
    GetCropRegion("IMG2O", enqueData.mIMG2O, pIMG2O);
    GetCropRegion("IMG3O", enqueData.mIMG3O, pIMG3O);
    GetCropRegion("TIMGO", enqueData.mTIMGO, pTIMGO);
    GetCropRegion("DCESO", enqueData.mDCESO, pDCESO);
    GetCropRegion("COPY1", enqueData.mCopy1, pCopy1);
    GetCropRegion("COPY2", enqueData.mCopy2, pCopy2);



    // 3.1 ISP tuning
    TuningParam tuningParam = {NULL, NULL};
    const MBOOL isSupportedDualSyn = (mISP5_0 || mISP6_0);
    const MBOOL isNeedDualSynData = (isSupportedDualSyn && enqueData.mEnableVSDoF);
    MBOOL bdirectRawBufCopy = MFALSE;
    {
        // For NDD
        trySetMetadata<MINT32>(pIMetaHal, MTK_PIPELINE_FRAME_NUMBER, frameNo);
        trySetMetadata<MINT32>(pIMetaHal, MTK_PIPELINE_REQUEST_NUMBER, requestNo);

        {
            const MBOOL isImgo = !enqueData.mResized;
            const MBOOL isMasterImgo = (master && isImgo);
            const MBOOL isBayerBayer = (mSensorFmt != SENSOR_RAW_MONO) && (mSensorFmt2 != SENSOR_RAW_MONO);
            const MBOOL isBayerMono = (mSensorFmt != SENSOR_RAW_MONO) && (mSensorFmt2 == SENSOR_RAW_MONO);
            if (!isMasterImgo && (enqueData.mYuvRep || enqueData.mScaleType == eScale_Up || enqueData.mEnableMFB)) {
                MY_LOGW("no combined of non-MasterImgo with yuvRep(%d) or scaleUp(%d) or enableMFB(%d)",
                    enqueData.mYuvRep, enqueData.mScaleType == eScale_Up, enqueData.mEnableMFB);
            }
#if MTKCAM_ENABLE_ISPPROFILE_MAPPER == 1
            IspProfileInfo profileInfo(getIspProfileName((EIspProfile_T)enqueData.mIspProfile), enqueData.mIspProfile);
            if (enqueData.mScaleType == eScale_Up) {
                MINT32 resolution = rImgiSize.w | rImgiSize.h << 16;
                trySetMetadata<MINT32>(pIMetaHal, MTK_ISP_P2_IN_IMG_RES_REVISED, resolution);
                MY_LOGD("apply profile(MultiPass_HWNR or DSDN) revised resolution: 0x%x", resolution);
            }
#else
            IspProfileInfo profileInfo = MAKE_ISP_PROFILE_INFO(EIspProfile_Capture);
            if (enqueData.mIspProfile >= 0) {
                profileInfo.mValue = enqueData.mIspProfile;
                profileInfo.mName = " ";
            } else if (enqueData.mYuvRep) {
                profileInfo = MAKE_ISP_PROFILE_INFO(EIspProfile_YUV_Reprocess);
            } else if (enqueData.mScaleType == eScale_Up) {
#ifdef SUPPORT_DSDN_20
                profileInfo = MAKE_ISP_PROFILE_INFO(EIspProfile_Capture_DSDN);
#else
                profileInfo = MAKE_ISP_PROFILE_INFO(EIspProfile_Capture_MultiPass_HWNR);
#endif
                MINT32 resolution = rImgiSize.w | rImgiSize.h << 16;
                trySetMetadata<MINT32>(pIMetaHal, MTK_ISP_P2_IN_IMG_RES_REVISED, resolution);
                MY_LOGD("apply profile(MultiPass_HWNR or DSDN) revised resolution: 0x%x", resolution);
            } else
#ifdef SUPPORT_DCE
            if (enqueData.mEnableDCE) { // DCE ONLY, not DSDN
                profileInfo = MAKE_ISP_PROFILE_INFO(EIspProfile_Capture_DCE);
            } else
#endif
            if (enqueData.mEnableMFB) {
                profileInfo = MAKE_ISP_PROFILE_INFO(EIspProfile_MFNR_Before_Blend);
            // dualcam vsdof part
            } else if (enqueData.mEnableVSDoF) {
                if( !isBayerBayer && !isBayerMono ) {
                    MY_LOGW("unknown sensorFmt, sensorIndex:%d -> sensorFmt:%u, sensorIndex2:%d -> sensorFmt2:%u",
                            pRequest->getMainSensorIndex(), mSensorFmt, pRequest->getSubSensorIndex(), mSensorFmt2);
                } else {
                    IspProfileHint hint;
                    hint.mSensorAliasName = master ? eSAN_Master : eSAN_Sub_01;
                    hint.mSensorConfigType = isBayerBayer ? eSCT_BayerBayer : eSCT_BayerMono;
                    hint.mRawImageType = isImgo ? eRIT_Imgo : eRIT_Rrzo;
                    profileInfo = IspProfileManager::get(hint);
                }
            } else if (isMulticam) {
                profileInfo = MAKE_ISP_PROFILE_INFO(EIspProfile_N3D_Capture);
            }
#endif
            // The value should be used
            MINT32 iP2InputFormat = 0;
            // The value stored in HAL metadata. Should update HAL metadata if the two values are no equal
            MINT32 iP2CurrentFormat = 0;
            // set MTK_ISP_P2_IN_IMG_FMT to 1 for YUV2YUV processing
            if (bYuvTunning) {
                iP2InputFormat = 1;
            }
            // 0/Empty: RAW->YUV, 1: YUV->YUV
            tryGetMetadata<MINT32>(pIMetaHal, MTK_ISP_P2_IN_IMG_FMT, iP2CurrentFormat);
            if (iP2CurrentFormat != iP2InputFormat)
                trySetMetadata<MINT32>(pIMetaHal, MTK_ISP_P2_IN_IMG_FMT, iP2InputFormat);

            trySetMetadata<MUINT8>(pIMetaHal, MTK_3A_ISP_PROFILE, profileInfo.mValue);
            MY_LOGD("set ispProfile, R/F Num: %d/%d, taskId:%d, profile:%d(%s), master:%d, imgo:%d, yuvRep:%d, scale:%d, mfb:%d, vsdof:%d, bb:%d, bm:%d",
                requestNo, frameNo, enqueData.mTaskId,
                profileInfo.mValue, profileInfo.mName,
                master, isImgo,
                enqueData.mYuvRep, enqueData.mScaleType, enqueData.mEnableMFB, enqueData.mEnableVSDoF,
                isBayerBayer, isBayerMono);
        }

        // check if the Raw buffer can be copied directly (pure raw to pure raw with same format)
        {
            if(mISP6_0 && pTIMGO != nullptr && !mDebugTimgo) {
                MINT infmt = pTIMGO->getImgFormat();
                MINT outfmt = pEnqueData->mIMGI.mpBuf->getImgFormat();
                MINT32 iAPRawType = -1;
                tryGetMetadata<MINT32>(pIMetaApp, MTK_CONTROL_CAPTURE_PROCESS_RAW_ENABLE, iAPRawType);
                if (isHalRawFormat(static_cast<EImageFormat>(infmt)) && (iAPRawType <= 0) && infmt == outfmt)
                {
                        MY_LOGD("bdirectRawBufCopy is setting to MTRUE");
                        bdirectRawBufCopy = MTRUE;
                }
            }
        }

        // Consturct tuning parameter
        {
            tuningParam.pRegBuf = reinterpret_cast<void*>(pBufTuning->mpVA);

            // LCEI
            if (pLCEI != NULL) {
                ASSERT(!enqueData.mResized, "use LCSO for RRZO buffer, should not happended!");
                tuningParam.pLcsBuf = reinterpret_cast<void*>(pLCEI);
            }

            // DCES
            if (pDCESI != NULL) {
                ASSERT(!enqueData.mResized, "use DCESI for RRZO buffer, should not happended!");

                MINT32 iMagicNum = 0;
                if (!tryGetMetadata<MINT32>(pIMetaHal, MTK_P1NODE_PROCESSOR_MAGICNUM, iMagicNum))
                    MY_LOGW("can not get magic number");
                tuningParam.pDcsBuf = reinterpret_cast<void*>(pDCESI);
                tuningParam.i4DcsMagicNo = iMagicNum;
            }

            // Dual sync info
            if (isNeedDualSynData)
            {
                tuningParam.bSlave = !master;
                if (!tuningParam.bSlave) // master
                {
                    if (enqueData.mTaskId == TASK_M1_IMGO) {
                        if (enqueData.mpSharedData->imgoDualSynData == NULL) {
                            enqueData.mpSharedData->imgoDualSynData = mpBufferPool->getTuningBuffer(mDualSyncInfo_Size);
                            tuningParam.pDualSynInfo = reinterpret_cast<void*>(enqueData.mpSharedData->imgoDualSynData->mpVA);
                            MY_LOGD("create imgo dual sync data, R/F Num: %d/%d, addr:%p, size:%u, taskId:%d",
                                requestNo, frameNo,
                                tuningParam.pDualSynInfo, mDualSyncInfo_Size, enqueData.mTaskId);
                        } else {
                            MY_LOGW("imgo dual sync data is not null, R/F Num: %d/%d, addr:%p, taskId:%d",
                                requestNo, frameNo,
                                enqueData.mpSharedData->imgoDualSynData->mpVA, enqueData.mTaskId);
                        }
                    } else if (enqueData.mTaskId == TASK_M1_RRZO) {
                        if (enqueData.mpSharedData->rrzoDualSynData == NULL) {
                            enqueData.mpSharedData->rrzoDualSynData = mpBufferPool->getTuningBuffer(mDualSyncInfo_Size);
                            tuningParam.pDualSynInfo = reinterpret_cast<void*>(enqueData.mpSharedData->rrzoDualSynData->mpVA);
                            MY_LOGD("create rrzo dual sync data, R/F Num: %d/%d, addr:%p, size:%u, taskId:%d",
                                requestNo, frameNo,
                                tuningParam.pDualSynInfo, mDualSyncInfo_Size, enqueData.mTaskId);
                        } else {
                            MY_LOGD("rrzo dual sync data is not null, R/F Num: %d/%d, addr:%p, taskId:%d",
                                requestNo, frameNo,
                                enqueData.mpSharedData->rrzoDualSynData->mpVA, enqueData.mTaskId);
                        }
                    }
                } else { // slave
                    if (enqueData.mTaskId == TASK_M2_IMGO) {
                        if (enqueData.mpSharedData->imgoDualSynData != NULL) {
                            tuningParam.pDualSynInfo = reinterpret_cast<void*>(enqueData.mpSharedData->imgoDualSynData->mpVA);
                            MY_LOGD("set imgo dual sync data, R/F Num: %d/%d, addr:%p, taskId:%d",
                                requestNo, frameNo,
                                tuningParam.pDualSynInfo, enqueData.mTaskId);
                        } else {
                            MY_LOGW("failed to set imgo dual sync data, R/F Num: %d/%d, taskId:%d", requestNo, frameNo, enqueData.mTaskId);
                        }
                    }
                    else if (enqueData.mTaskId == TASK_M2_RRZO) {
                        if (enqueData.mpSharedData->rrzoDualSynData != NULL) {
                            tuningParam.pDualSynInfo = reinterpret_cast<void*>(enqueData.mpSharedData->rrzoDualSynData->mpVA);
                            MY_LOGD("set rrzo dual sync data, R/F Num: %d/%d, addr:%p, taskId:%d",
                                requestNo, frameNo,
                                tuningParam.pDualSynInfo, enqueData.mTaskId);
                        } else {
                            MY_LOGW("failed to set rrzo dual sync data, R/F Num: %d/%d, taskId:%d", requestNo, frameNo, enqueData.mTaskId);
                        }
                    }
                }
            }

            // USE resize raw-->set PGN 0
            if (enqueData.mResized) {
                trySetMetadata<MUINT8>(pIMetaHal, MTK_3A_PGN_ENABLE, 0);
            } else {
                trySetMetadata<MUINT8>(pIMetaHal, MTK_3A_PGN_ENABLE, 1);
            }
            MetaSet_T inMetaSet;
            if (bSubFrame) {
                // For MFNR,
                // sub frame uses same metadata and tuning data as main frame,
                // but must restore some metadata of sub frame
                trySetMetadata<MINT32>(&metaHal, MTK_PIPELINE_FRAME_NUMBER, frameNo);
                trySetMetadata<MUINT8>(&metaHal, MTK_ISP_P2_TUNING_UPDATE_MODE, 2);
                trySetMetadata<MINT32>(&metaHal, MTK_HAL_REQUEST_INDEX_BSS, pRequest->getParameter(PID_FRAME_INDEX_FORCE_BSS));
                inMetaSet.appMeta = metaApp;
                inMetaSet.halMeta = metaHal;
            } else {
                inMetaSet.appMeta = *pIMetaApp;
                inMetaSet.halMeta = *pIMetaHal;
            }
            MBOOL bIMGIRawFmt = NSCam::isHalRawFormat((EImageFormat)pIMGI->getImgFormat());
            MBOOL needOutMeta = (pOMetaApp != NULL || pOMetaHal != NULL) &&
                                (enqueData.mRound == 1) &&
                                (enqueData.mTaskId == TASK_M1_IMGO_PRE || enqueData.mTaskId == TASK_M1_IMGO) &&
                                (bIMGIRawFmt || !mUsageHint.mIsHidlIsp);
            MetaSet_T outMetaSet;
            MetaSet_T* outMetaPtr = nullptr;
            MY_LOGD_IF(mLogLevel, "Generate output meta(%d) imgiRaw=%d TaskId=%d pOMetaApp:%d pOMetaHal:%d",
                        needOutMeta, bIMGIRawFmt, enqueData.mTaskId, pOMetaApp != NULL, pOMetaHal != NULL);
            if(needOutMeta)
            {
                outMetaSet.appMeta = *pOMetaApp;
                outMetaSet.halMeta = *pOMetaHal;
                outMetaPtr = &outMetaSet;
            }

            {
                MBOOL bLockDCESI = (enqueData.mpLockDCES != NULL && pDCESI != NULL);
                if (bLockDCESI)
                    enqueData.mpLockDCES->lock();

                auto DumpReg = [&](MBOOL after) -> MVOID {
                    if (pBufTuning == NULL)
                        return;

                    unsigned char* pBuffer = NULL;
#if (MFLL_MF_TAG_VERSION == 9)
                    pBuffer = static_cast<unsigned char*>(pBufTuning->mpVA);
                    pBuffer += 4;
#elif (MFLL_MF_TAG_VERSION == 11)
                    pBuffer = static_cast<unsigned char*>(pBufTuning->mpVA);
                    pBuffer += 4112;
#endif
                    if (pBuffer != NULL) {
                        unsigned int* pTuning = (unsigned int*)pBuffer;
                        MY_LOGD("After(%d) Dump Tuning Reg: 0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x",
                                after, pTuning[0], pTuning[1], pTuning[2], pTuning[3], pTuning[4], pTuning[5]);
                    }
                    MY_LOGD("After(%d) pFaceAlphaBuf(%p), pLsc2Buf(%p), pBpc2Buf(%p)",
                        after, tuningParam.pFaceAlphaBuf, tuningParam.pLsc2Buf, tuningParam.pBpc2Buf);
                };

                DumpReg(MFALSE);

                CAM_TRACE_BEGIN("p2a:setIsp");
#if (MTK_CAPTURE_ISP_VERSION == 6)
                // check profile to set identity
                if(enqueData.mIspProfile == ISP_PROFILE_WITH_IDENTITY_EFFECT)
                    trySetMetadata<MUINT8>(&inMetaSet.halMeta, MTK_ISP_P2_TUNING_UPDATE_MODE, EIdenditySetting);
#endif
                //setP2Isp
                if(!bdirectRawBufCopy) {
                    ret = mISPOperators[enqueData.mSensorId].mIspHal->setP2Isp(0, inMetaSet, &tuningParam, outMetaPtr);
                }
                CAM_TRACE_END();

                DumpReg(MTRUE);

                if (bMainFrame) {
                    mKeepMetaApp = *pIMetaApp;
                    mKeepMetaHal = *pIMetaHal;
                }

                if (bLockDCESI)
                    enqueData.mpLockDCES->unlock();
            }
            ASSERT(ret == OK, "fail to set ISP");
            if (needOutMeta) {
                (*pOMetaHal) += outMetaSet.halMeta;
#if (MTKCAM_ISP_SUPPORT_COLOR_SPACE)
                // If flag on, the NVRAM will always prepare tuning data for P3 color space
                trySetMetadata<MINT32>(pOMetaHal, MTK_ISP_COLOR_SPACE, MTK_ISP_COLOR_SPACE_DISPLAY_P3);
#endif
            }
            else {
                MY_LOGD_IF(mLogLevel, "No need output metadata.");
            }
            if (pOMetaApp != NULL)
                (*pOMetaApp) += outMetaSet.appMeta;
        }
    }

    // 3.2 Fill PQ Param
    {
        MINT32 iIsoValue = 0;
        if (!tryGetMetadata<MINT32>(pIMetaDynamic, MTK_SENSOR_SENSITIVITY, iIsoValue))
            MY_LOGW("can not get iso value");

        MINT32 iMagicNum = 0;
        if (!tryGetMetadata<MINT32>(pIMetaHal, MTK_P1NODE_PROCESSOR_MAGICNUM, iMagicNum))
            MY_LOGW("can not get magic number");

        MINT32 iRealLv = 0;
        if (!tryGetMetadata<MINT32>(pIMetaHal, MTK_REAL_LV, iRealLv))
            MY_LOGW("can not get read lv");

        auto fillPQ = [&](DpPqParam& rPqParam, PortID port, MBOOL enableCZ, MBOOL enableHFG) {
            String8 strEnqueLog;
            strEnqueLog += String8::format("Port(%d) Timestamp:%d",port.index ,enqueData.mUniqueKey);

            rPqParam.enable           = false;
            rPqParam.scenario         = MEDIA_ISP_CAPTURE;
            rPqParam.u.isp.iso        = iIsoValue;
            rPqParam.u.isp.lensId     = enqueData.mSensorId;
            rPqParam.u.isp.LV         = iRealLv;
            rPqParam.u.isp.timestamp  = enqueData.mUniqueKey;
            rPqParam.u.isp.frameNo    = enqueData.mFrameNo;
            rPqParam.u.isp.requestNo  = enqueData.mRequestNo;

            if (enqueData.mRound > 1) {
                strncpy(rPqParam.u.isp.userString, "round2", sizeof("round2"));
            }

            if (mSupportCZ && enableCZ) {
                rPqParam.enable = (PQ_ULTRARES_EN);
                ClearZoomParam& rCzParam = rPqParam.u.isp.clearZoomParam;

                rCzParam.captureShot = CAPTURE_SINGLE;

                // Pass this part if user load
                // Be here only if don't apply DRE.
                if (mDebugDumpMDP) {
                    rPqParam.u.isp.p_mdpSetting = new MDPSetting();
                    rPqParam.u.isp.p_mdpSetting->size = MDPSETTING_MAX_SIZE;
                    rPqParam.u.isp.p_mdpSetting->buffer = ::malloc(MDPSETTING_MAX_SIZE);
                    rPqParam.u.isp.p_mdpSetting->offset = 0;
                    memset(rPqParam.u.isp.p_mdpSetting->buffer, 0xFF, MDPSETTING_MAX_SIZE);
                }

                MUINT32 idx = 0;
                rCzParam.p_customSetting = (void*)getTuningFromNvram(enqueData.mSensorId, idx, iMagicNum, NVRAM_TYPE_CZ, pIMetaHal, false, mUsageHint.mIsHidlIsp);
                strEnqueLog += String8::format(" CZ Capture:%d idx:0x%x",
                    rCzParam.captureShot,
                    idx);
            }

            if (mSupportDRE && enqueData.mEnableDRE) {
                rPqParam.enable |= (PQ_DRE_EN);
                rPqParam.scenario = MEDIA_ISP_CAPTURE;

                DpDREParam& rDreParam = rPqParam.u.isp.dpDREParam;
                rDreParam.cmd         = DpDREParam::Cmd::Initialize | DpDREParam::Cmd::Generate;
                rDreParam.userId      = generatePQUserID(rPqParam.u.isp.frameNo, rPqParam.u.isp.lensId, pRequest->isPhysicalStream());
                rDreParam.buffer      = nullptr;
                MUINT32 idx = 0;
                rDreParam.p_customSetting = (void*)getTuningFromNvram(enqueData.mSensorId, idx, iMagicNum, NVRAM_TYPE_DRE, pIMetaHal, false, mUsageHint.mIsHidlIsp);
                rDreParam.customIndex     = idx;
                strEnqueLog += String8::format(" DRE cmd:0x%x userId:%llu buffer:%p p_customSetting:%p idx:%d",
                                rDreParam.cmd, rDreParam.userId, rDreParam.buffer, rDreParam.p_customSetting, idx);

            }

            if (mSupportHFG & enableHFG) {
                rPqParam.enable |= (PQ_HFG_EN);
                DpHFGParam& rHfgParam = rPqParam.u.isp.dpHFGParam;

                MUINT32 idx = 0;
                rHfgParam.p_lowerSetting = (void*)getTuningFromNvram(enqueData.mSensorId, idx, iMagicNum, NVRAM_TYPE_HFG, pIMetaHal, false, mUsageHint.mIsHidlIsp);
                rHfgParam.p_upperSetting = rHfgParam.p_lowerSetting ;
                rHfgParam.lowerISO = iIsoValue;
                rHfgParam.upperISO = iIsoValue;
                rHfgParam.p_slkParam = tuningParam.pRegBuf;

                strEnqueLog += String8::format(" HFG idx:%d", idx);
            }

            strEnqueLog += String8::format(" PQ Enable:0x%x scenario:%d iso:%d",
                    rPqParam.enable,
                    rPqParam.scenario,
                    iIsoValue);

            MY_LOGD("%s", strEnqueLog.string());
        };

        // WROT
        if (pWROTO != NULL) {
            DpPqParam* pIspParam_WROT = new DpPqParam();
            fillPQ(*pIspParam_WROT, PORT_WROTO, enqueData.mWROTO.mEnableCZ, enqueData.mWROTO.mEnableHFG);
            pPQParam->WROTPQParam = static_cast<void*>(pIspParam_WROT);
        }

        // WDMA
        if (pWDMAO != NULL) {
            DpPqParam* pIspParam_WDMA = new DpPqParam();
            fillPQ(*pIspParam_WDMA, PORT_WDMAO, enqueData.mWDMAO.mEnableCZ, enqueData.mWDMAO.mEnableHFG);
            pPQParam->WDMAPQParam = static_cast<void*>(pIspParam_WDMA);
        }

    }

    // 3.3 Srz tuning for Imgo (LCE not applied to rrzo)
    if (enqueData.mScaleType != eScale_Up && !enqueData.mResized) {
        auto fillSRZ4 = [&]() -> ModuleInfo* {
            if (mDipVer < EDIPHWVersion_50) {
                MY_LOGD_IF(0, "isp version(0x%x) < 5.0, dont need SrzInfo", mDipVer);
                return NULL;
            }
            // srz4 config
            // ModuleInfo srz4_module;
            ModuleInfo* p = new ModuleInfo();
            p->moduleTag = EDipModule_SRZ4;
            p->frameGroup=0;

            _SRZ_SIZE_INFO_* pSrzParam = new _SRZ_SIZE_INFO_;
            if (pLCEI) {
                pSrzParam->in_w = pLCEI->getImgSize().w;
                pSrzParam->in_h = pLCEI->getImgSize().h;
                pSrzParam->crop_w = pLCEI->getImgSize().w;
                pSrzParam->crop_h = pLCEI->getImgSize().h;
            }
            if (pIMGI) {
                pSrzParam->out_w = pIMGI->getImgSize().w;
                pSrzParam->out_h = pIMGI->getImgSize().h;
            }

            p->moduleStruct = reinterpret_cast<MVOID*> (pSrzParam);

            return p;
        };
        pPackage->mpModuleSRZ4 = fillSRZ4();
    }

    // 3.4 Srz3 tuning
    if (tuningParam.pFaceAlphaBuf) {
        auto fillSRZ3 = [&]() -> ModuleInfo* {
            // srz3 config
            // ModuleInfo srz3_module;
            ModuleInfo* p = new ModuleInfo();
            p->moduleTag = EDipModule_SRZ3;
            p->frameGroup=0;

            _SRZ_SIZE_INFO_* pSrzParam = new _SRZ_SIZE_INFO_;

            FACENR_IN_PARAMS in;
            FACENR_OUT_PARAMS out;
            IImageBuffer *facei = (IImageBuffer*)tuningParam.pFaceAlphaBuf;
            in.p2_in    = rImgiSize;
            in.face_map = facei->getImgSize();
            calculateFACENRConfig(in, out);
            *pSrzParam = out.srz3Param;
            p->moduleStruct = reinterpret_cast<MVOID*>(pSrzParam);
            return p;
        };
        pPackage->mpModuleSRZ3 = fillSRZ3();
    }

    // 4.create enque param
    NSIoPipe::QParams qParam;

    // 4.1 QParam template
    MINT32 iFrameNum = 0;
    QParamTemplateGenerator qPTempGen = QParamTemplateGenerator(
            iFrameNum, enqueData.mSensorId,
            enqueData.mTimeSharing ? ENormalStreamTag_Vss : ENormalStreamTag_Cap);

    qPTempGen.addInput(PORT_IMGI);

    if (pPackage->mpModuleSRZ3 != NULL) {
        qPTempGen.addModuleInfo(EDipModule_SRZ3,  pPackage->mpModuleSRZ3->moduleStruct);
        qPTempGen.addInput(PORT_YNR_FACEI);
    }

    if (!bYuvTunning && !enqueData.mResized && tuningParam.pLsc2Buf != NULL) {
        if (mISP6_0) {
            qPTempGen.addInput(PORT_LSCI);
        } else if (mDipVer == EDIPHWVersion_50) {
            qPTempGen.addInput(PORT_IMGCI);
        } else if (mDipVer == EDIPHWVersion_40) {
            qPTempGen.addInput(PORT_DEPI);
        } else {
            MY_LOGE("should not have LSC buffer in ISP 3.0");
        }
    }

    if (!bYuvTunning && !enqueData.mResized && pLCEI != NULL) {
        qPTempGen.addInput(PORT_LCEI);
        if (pPackage->mpModuleSRZ4 != NULL) {
            qPTempGen.addModuleInfo(EDipModule_SRZ4,  pPackage->mpModuleSRZ4->moduleStruct);
            if (mISP6_0)
                qPTempGen.addInput(PORT_YNR_LCEI);
            else
                qPTempGen.addInput(PORT_DEPI);
        }
    }

    if (!bYuvTunning && tuningParam.pBpc2Buf != NULL) {
         if (mISP6_0)
            qPTempGen.addInput(PORT_BPCI);
        else if (mDipVer == EDIPHWVersion_50)
            qPTempGen.addInput(PORT_IMGBI);
        else if (mDipVer == EDIPHWVersion_40)
            qPTempGen.addInput(PORT_DMGI);
    }

    if (pIMG2O != NULL) {
        qPTempGen.addOutput(PORT_IMG2O, 0);
        qPTempGen.addCrop(eCROP_CRZ, enqueData.mIMG2O.mCropRegion.p, enqueData.mIMG2O.mCropRegion.s, pIMG2O->getImgSize());
    }

    if (pIMG3O != NULL) {
        qPTempGen.addOutput(PORT_IMG3O, 0);
    }

    if (pTIMGO != NULL) {
        qPTempGen.addOutput(PORT_TIMGO, 0);
    }

    if (pDCESO != NULL) {
        qPTempGen.addOutput(PORT_DCESO, 0);
    }

    if (pWROTO != NULL) {
        qPTempGen.addOutput(PORT_WROTO, enqueData.mWROTO.mTrans);
        qPTempGen.addCrop(eCROP_WROT, enqueData.mWROTO.mCropRegion.p, enqueData.mWROTO.mCropRegion.s, pWROTO->getImgSize());
    }

    if (pWDMAO != NULL) {
        qPTempGen.addOutput(PORT_WDMAO, 0);
        qPTempGen.addCrop(eCROP_WDMA, enqueData.mWDMAO.mCropRegion.p, enqueData.mWDMAO.mCropRegion.s, pWDMAO->getImgSize());
    }

    qPTempGen.addExtraParam(EPIPE_MDP_PQPARAM_CMD, (MVOID*)pPQParam);
    MUINT32 TimgoType = EDIPTimgoDump_NONE;
    if (mDebugTimgo) {
        qPTempGen.addExtraParam(EPIPE_TIMGO_DUMP_SEL_CMD, (MVOID*)&mDebugTimgoType);
    } else if (pTIMGO != nullptr) {
        MINT fmt = pTIMGO->getImgFormat();
        if(mISP6_0 && isHalRawFormat((EImageFormat)fmt)) {
            TimgoType = EDIPTimgoDump_AFTER_LTM;
            MUINT32 TimgoType = EDIPTimgoDump_AFTER_LTM;
            qPTempGen.addExtraParam(EPIPE_TIMGO_DUMP_SEL_CMD, (MVOID*)&TimgoType);
            MY_LOGD("Raw format(%x), set TimgoType(%u)", fmt, TimgoType);
        } else {
            MY_LOGW("nonRaw format(%x) is set", fmt);
        }
    }


    ret = qPTempGen.generate(qParam) ? OK : BAD_VALUE;
    ASSERT(ret == OK, "fail to create QParams");

    // 4.2 QParam filler
    QParamTemplateFiller qParamFiller(qParam);
    qParamFiller.insertInputBuf(iFrameNum, PORT_IMGI, pIMGI);
    if(mISP6_0 && pTIMGO != nullptr && !mDebugTimgo) {
        // check the current RAW type got from P1
        MINT32 iP1RawType = -1;
        tryGetMetadata<MINT32>(pIMetaHal, MTK_P1NODE_RAW_TYPE, iP1RawType);
        // check the returned RAW type set by AP
        MINT32 iAPRawType = -1;
        tryGetMetadata<MINT32>(pIMetaApp, MTK_CONTROL_CAPTURE_PROCESS_RAW_ENABLE, iAPRawType);
        // exclude the case that pTIMGO is set by dump debug TIMGO,
        // which tuning setiing should not be changed.
        MINT fmt = pTIMGO->getImgFormat();
#if (MTK_CAPTURE_ISP_VERSION == 6)
        if (isHalRawFormat((EImageFormat)fmt)){
            // this case should not be occurred
            if (iP1RawType == ERawType_Proc && iAPRawType == 0){
                MY_LOGE("wrong APPMeta MTK_CONTROL_CAPTURE_PROCESS_RAW_ENABLE might be used ");
            }
            // processed raw is needed
            else if (iAPRawType > 0){
                qParamFiller.insertTuningBuf(iFrameNum, pBufTuning->mpVA);
                trySetMetadata<MINT32>(pOMetaHal, MTK_P1NODE_RAW_TYPE, ERawType_Proc);
            }
            // pure raw
            else{
                qParamFiller.insertTuningBuf(iFrameNum, nullptr);
            }
        }else{
            MY_LOGW("nonRaw format(%x) is set", fmt);
            qParamFiller.insertTuningBuf(iFrameNum, nullptr);
        }
#endif
    } else {
        qParamFiller.insertTuningBuf(iFrameNum, pBufTuning->mpVA);
    }

    if (pPackage->mpModuleSRZ3 != NULL) {
        qParamFiller.insertInputBuf(iFrameNum, PORT_YNR_FACEI, static_cast<IImageBuffer*>(tuningParam.pFaceAlphaBuf));
        strEnqueLog += String8::format(" in:YNR_FACEI fmt:%x", static_cast<IImageBuffer*>(tuningParam.pFaceAlphaBuf)->getImgFormat());
    }
    if (!bYuvTunning && !enqueData.mResized && tuningParam.pLsc2Buf != NULL) {
        if (mISP6_0)
        {
            qParamFiller.insertInputBuf(iFrameNum,  PORT_LSCI,  static_cast<IImageBuffer*>(tuningParam.pLsc2Buf));
            strEnqueLog += String8::format(" in:LSCI fmt:%x", static_cast<IImageBuffer*>(tuningParam.pLsc2Buf)->getImgFormat());
        }
        else if (mDipVer == EDIPHWVersion_50)
        {
            qParamFiller.insertInputBuf(iFrameNum,  PORT_IMGCI, static_cast<IImageBuffer*>(tuningParam.pLsc2Buf));
            strEnqueLog += String8::format(" in:IMGCI");
        }
        else if (mDipVer == EDIPHWVersion_40)
        {
            qParamFiller.insertInputBuf(iFrameNum,  PORT_DEPI,  static_cast<IImageBuffer*>(tuningParam.pLsc2Buf));
            strEnqueLog += String8::format(" in:DEPI");
        }
    }

    if (!bYuvTunning && !enqueData.mResized && pLCEI != NULL) {
        qParamFiller.insertInputBuf(iFrameNum, PORT_LCEI, pLCEI);
        strEnqueLog += String8::format(" in:LCEI format:%x", pLCEI->getImgFormat());
        if (pPackage->mpModuleSRZ4 != NULL) {
            if (mISP6_0)
                qParamFiller.insertInputBuf(iFrameNum, PORT_YNR_LCEI, pLCEI);
            else
                qParamFiller.insertInputBuf(iFrameNum, PORT_DEPI, pLCEI);
        }
    }

    if (!bYuvTunning && tuningParam.pBpc2Buf != NULL) {
        if (mISP6_0)
        {
            qParamFiller.insertInputBuf(iFrameNum,  PORT_BPCI,  static_cast<IImageBuffer*>(tuningParam.pBpc2Buf));
            strEnqueLog += String8::format(" in:BPCI fmt:%x", static_cast<IImageBuffer*>(tuningParam.pBpc2Buf)->getImgFormat());
        }
        else if (mDipVer == EDIPHWVersion_50)
        {
            qParamFiller.insertInputBuf(iFrameNum,  PORT_IMGBI, static_cast<IImageBuffer*>(tuningParam.pBpc2Buf));
            strEnqueLog += String8::format(" in:IMGBI");
        }
        else if (mDipVer == EDIPHWVersion_40)
        {
            qParamFiller.insertInputBuf(iFrameNum,  PORT_DMGI,  static_cast<IImageBuffer*>(tuningParam.pBpc2Buf));
            strEnqueLog += String8::format(" in:DMGI");
        }

    }

    MY_LOGD("%s", strEnqueLog.string());

    if (pIMG2O != NULL) {
        qParamFiller.insertOutputBuf(iFrameNum, PORT_IMG2O, pIMG2O);
    }

    if (pIMG3O != NULL) {
        qParamFiller.insertOutputBuf(iFrameNum, PORT_IMG3O, pIMG3O);
    }

    if (pTIMGO != NULL) {
        qParamFiller.insertOutputBuf(iFrameNum, PORT_TIMGO, pTIMGO);
    }

    if (pDCESO != NULL) {
        qParamFiller.insertOutputBuf(iFrameNum, PORT_DCESO, pDCESO);
    }

    if (pWROTO != NULL) {
        qParamFiller.insertOutputBuf(iFrameNum, PORT_WROTO, pWROTO);
    }

    if (pWDMAO != NULL) {
        qParamFiller.insertOutputBuf(iFrameNum, PORT_WDMAO, pWDMAO);
    }

    QParamTemplateFiller::FrameInfo info = {.FrameNo=static_cast<MUINT32>(frameNo),
                                            .RequestNo=static_cast<MUINT32>(requestNo),
                                            .Timestamp=static_cast<MUINT32>(enqueData.mTaskId),
                                            .UniqueKey=enqueData.mUniqueKey, .IspProfile=enqueData.mIspProfile,
                                            .SensorID=enqueData.mSensorId};
    auto scenario = pPackage->mbNeedToDump ? DUMP_FEATURE_CAPTURE : DUMP_FEATURE_OFF;
    qParamFiller.setInfo(iFrameNum, info, scenario);


    ret = qParamFiller.validate() ? OK : BAD_VALUE;
    ASSERT(ret == OK, "fail to create QParamFiller");

    // 5. prepare rest buffers using mdp copy
    auto IsCovered = [](P2Output& rSrc, MDPOutput& rDst) {
        if (rSrc.mpBuf == NULL || rDst.mpBuf == NULL)
            return MFALSE;

        // Ignore gary image
        MINT srcFmt = rSrc.mpBuf->getImgFormat();
        if (srcFmt == eImgFmt_Y8)
            return MFALSE;

        MSize& srcCrop = rSrc.mCropRegion.s;
        MSize& dstCrop = rDst.mCropRegion.s;
        // Make sure that the source FOV covers the destination FOV
#define FOV_THRES (1)
        if (srcCrop.w * (100 + FOV_THRES) < dstCrop.w * 100 ||
            srcCrop.h * (100 + FOV_THRES) < dstCrop.h * 100)
            return MFALSE;
#undef FOV_THRES

        MSize srcSize = rSrc.mpBuf->getImgSize();
        MSize dstSize = rDst.mpBuf->getImgSize();
        if (rSrc.mTrans & eTransform_ROT_90)
            swap(srcSize.w, srcSize.h);

        if (rDst.mTrans & eTransform_ROT_90)
            swap(dstSize.w, dstSize.h);

        // Make sure that the source image is bigger than destination image
#define SIZE_THRES (5)
        if (srcSize.w * (100 + SIZE_THRES) < dstSize.w * 100 ||
            srcSize.h * (100 + SIZE_THRES) < dstSize.h * 100)
            return MFALSE;
#undef SIZE_THRES

        simpleTransform tranCrop2SrcBuf(rSrc.mCropRegion.p, rSrc.mCropRegion.s, srcSize);
        MRect& rCropRegion = rDst.mSourceCrop;
        rCropRegion = transform(tranCrop2SrcBuf, rDst.mCropRegion);

        // Boundary Check for FOV tolerance
        if (rCropRegion.p.x < 0)
            rCropRegion.p.x = 0;
        if (rCropRegion.p.y < 0)
            rCropRegion.p.y = 0;

        if ((rCropRegion.p.x + rCropRegion.s.w) > srcSize.w)
            rCropRegion.s.w = srcSize.w - rCropRegion.p.x;
        if ((rCropRegion.p.y + rCropRegion.s.h) > srcSize.h)
            rCropRegion.s.h = srcSize.h - rCropRegion.p.y;

        // Make 2 bytes alignment
        rCropRegion.s.w &= ~(0x1);
        rCropRegion.s.h &= ~(0x1);

        if (rSrc.mTrans & eTransform_ROT_90) {
            swap(rCropRegion.p.x, rCropRegion.p.y);
            swap(rCropRegion.s.w, rCropRegion.s.h);
        }

        rDst.mpSource = rSrc.mpBuf;

        MUINT32& rSrcTrans = rSrc.mTrans;
        MUINT32& rDstTrans = rDst.mTrans;
        // Use XOR to calucate the transform, where the source image has a rotation or flip
        rDst.mSourceTrans = rSrcTrans ^ rDstTrans;
        // MDP does image rotation after flip.
        // If the source has a rotation and the destinatio doesn't, do the exceptional rule
        if (rSrcTrans & eTransform_ROT_90 && ~rDstTrans & eTransform_ROT_90) {
            if ((rSrcTrans & eTransform_ROT_180) == (rDstTrans & eTransform_ROT_180))
                rDst.mSourceTrans = eTransform_ROT_90 | eTransform_FLIP_V | eTransform_FLIP_H;
            else if ((rSrcTrans & eTransform_ROT_180) == (~rDstTrans & eTransform_ROT_180))
                rDst.mSourceTrans = eTransform_ROT_90;
        }

        return MTRUE;
    };

    // select a buffer source for MDP copy
    P2Output* pFirstHit = NULL;
    if (pCopy1 != NULL) {
        if (IsCovered(enqueData.mIMG2O, enqueData.mCopy1))
            pFirstHit = &enqueData.mIMG2O;
        else if (IsCovered(enqueData.mWROTO, enqueData.mCopy1))
            pFirstHit = &enqueData.mWROTO;
        else if (IsCovered(enqueData.mWDMAO, enqueData.mCopy1))
            pFirstHit = &enqueData.mWDMAO;
        else
            MY_LOGE("Copy1's FOV is not covered by P2 first round output");
    }

    if (pCopy2 != NULL) {
        if (pFirstHit != NULL && IsCovered(*pFirstHit, enqueData.mCopy2)) {
            MY_LOGD("Use the same output to serve two MDP outputs");
        } else if (pFirstHit != &enqueData.mIMG2O && IsCovered(enqueData.mIMG2O, enqueData.mCopy2)) {
            if (!IsCovered(enqueData.mIMG2O, enqueData.mCopy1))
                MY_LOGD("Use different output to serve two MDP outputs");
        } else if (pFirstHit != &enqueData.mWROTO && IsCovered(enqueData.mWROTO, enqueData.mCopy2)) {
            if (IsCovered(enqueData.mWROTO, enqueData.mCopy1))
                MY_LOGD("Use different output to serve two MDP outputs");
        } else if (pFirstHit != &enqueData.mWDMAO && IsCovered(enqueData.mWDMAO, enqueData.mCopy2)) {
            if (!IsCovered(enqueData.mWDMAO, enqueData.mCopy1))
                MY_LOGD("Use different output to serve two MDP outputs");
        } else {
            MY_LOGE("Copy2's FOV is not covered by P2 first round output");
        }
    }



    // 6.enque
    pPackage->start();

    // callbacks
    qParam.mpfnCallback = onP2SuccessCallback;
    qParam.mpfnEnQFailCallback = onP2FailedCallback;
    qParam.mpCookie = (MVOID*) pPackage;

    CAM_TRACE_ASYNC_BEGIN("P2_Enque", (enqueData.mFrameNo << 3) + enqueData.mTaskId);

    // !!!!
    MBOOL bLockDCESO = (enqueData.mpLockDCES != NULL && pDCESO != NULL);
    if (bLockDCESO)
        enqueData.mpLockDCES->lock();

    // 7. MDP QoS: using full performance
    if (mSupportMDPQoS) {
        struct timeval current;
        gettimeofday(&current, NULL);
        for (size_t i = 0, n = qParam.mvFrameParams.size(); i < n; ++i)
            qParam.mvFrameParams.editItemAt(i).ExpectedEndTime = current;
    }
    // dump QParams
    dumpQParams(mLogLevel, qParam);
    // p2 enque
    if(bdirectRawBufCopy) {
        ret = this->directBufCopy(pEnqueData->mIMGI.mpBuf, pEnqueData->mTIMGO.mpBuf, qParam);
        if(ret != OK)
        {
            MY_LOGE("fail to copy buffer");
            return MFALSE;
        }
    }
    else{
        ret = mISPOperators[enqueData.mSensorId].mP2Opt->enque(qParam, LOG_TAG);
        ASSERT(ret == OK, "fail to enque P2");
    }


    TRACE_FUNC_EXIT();
    return MTRUE;

#undef ASSERT

}

MBOOL P2ANode::onThreadStart()
{
    TRACE_FUNC_ENTER();

    mpISPProfileMapper = ISPProfileMapper::getInstance();

    TRACE_FUNC_EXIT();
    return MTRUE;
}

MBOOL P2ANode::onThreadStop()
{
    TRACE_FUNC_ENTER();

    TRACE_FUNC_EXIT();
    return MTRUE;
}

MBOOL P2ANode::onData(DataID id, const RequestPtr& pRequest)
{
    TRACE_FUNC_ENTER();
    MY_LOGD_IF(mLogLevel, "Frame %d: %s arrived", pRequest->getRequestNo(), PathID2Name(id));
    MBOOL ret = MTRUE;

    NodeID_T srcNodeId;
    NodeID_T dstNodeId;
    ret = GetPath(id, srcNodeId, dstNodeId);

    if (!ret) {
        MY_LOGD("Can not find the path: %d", id);
        return ret;
    }

    if (dstNodeId != NID_P2A && dstNodeId != NID_P2B) {
        MY_LOGE("Unexpected dstNode node: %s", NodeID2Name(dstNodeId));
        return MFALSE;
    }

    if (pRequest->isReadyFor(dstNodeId)) {
        RequestPack pack = {
            .mNodeId = dstNodeId,
            .mpRequest = pRequest
        };
        MY_LOGD_IF(mLogLevel, "Frame %d: %s enqued", pRequest->getRequestNo(), PathID2Name(id));
        mRequestPacks.enque(pack);
    }
    TRACE_FUNC_EXIT();
    return ret;
}

MBOOL P2ANode::onThreadLoop()
{
    TRACE_FUNC("Waitloop");
    RequestPack pack;

    CAM_TRACE_CALL();

    if (!waitAllQueue()) {
        return MFALSE;
    }

    if (!mRequestPacks.deque(pack)) {
        MY_LOGE("Request deque out of sync");
        return MFALSE;
    } else if (pack.mpRequest == NULL) {
        MY_LOGE("Request out of sync");
        return MFALSE;
    }

    RequestPtr pRequest = pack.mpRequest;

    TRACE_FUNC_ENTER();

    MBOOL ret = onRequestProcess(pack.mNodeId, pRequest);
    if (ret == MFALSE) {
        pRequest->addParameter(PID_FAILURE, 1);
    }

    // Default timing of next capture is P2 start
    if (pRequest->getParameter(PID_ENABLE_NEXT_CAPTURE) > 0)
    {
        const MBOOL isTimingNotFound = !pRequest->hasParameter(PID_THUMBNAIL_TIMING);
        const MBOOL isP2BeginTiming = (pRequest->getParameter(PID_THUMBNAIL_TIMING) == NSPipelinePlugin::eTiming_P2_Begin);
        const MBOOL isNotCShot = !pRequest->hasParameter(PID_CSHOT_REQUEST);
        if ((isTimingNotFound || isP2BeginTiming) && isNotCShot)
        {
            const MINT32 frameCount = pRequest->getActiveFrameCount();
            const MINT32 frameIndex = pRequest->getActiveFrameIndex();

            if ((pRequest->isSingleFrame() && pRequest->isActiveFirstFrame()) || pRequest->isActiveLastFrame())
            {
                if (pRequest->mpCallback != NULL) {
                    MY_LOGI("Notify next capture at P2 beginning(%d|%d), isTimingNotFound:%d, isP2BeginTiming:%d, isNotCShot:%d",
                        frameIndex, frameCount, isTimingNotFound, isP2BeginTiming, isNotCShot);
                    pRequest->mpCallback->onContinue(pRequest);
                } else {
                    MY_LOGW("have no request callback instance!, isTimingNotFound:%d, isP2BeginTiming:%d, isNotCShot:%d",
                        isTimingNotFound, isP2BeginTiming, isNotCShot);
                }
            }
        }
    }

    TRACE_FUNC_EXIT();
    return MTRUE;
}

/*******************************************************************************
 *
 ********************************************************************************/
MVOID
P2ANode::
onP2SuccessCallback(QParams& rParams)
{
    EnquePackage* pPackage = (EnquePackage*) (rParams.mpCookie);
    P2EnqueData& rData = *pPackage->mpEnqueData.get();
    P2ANode* pNode = pPackage->mpNode;

    CAM_TRACE_ASYNC_END("P2_Enque", (rData.mFrameNo << 3) + rData.mTaskId);
    pPackage->stop();

    MY_LOGI("(%d) R/F/S Num: %d/%d/%d, task:%d, timeconsuming: %dms",
            rData.mNodeId,
            rData.mRequestNo,
            rData.mSensorId,
            rData.mFrameNo,
            rData.mTaskId,
            pPackage->getElapsed());
    // !!!!
    MBOOL bLockDCESO = (rData.mpLockDCES != NULL && rData.mDCESO.mpBuf != NULL);
    if (bLockDCESO)
        rData.mpLockDCES->unlock();

    if (pPackage->mbNeedToDump && rData.mDebugDump ) {
        char filename[256] = {0};
        FILE_DUMP_NAMING_HINT hint;
        hint.UniqueKey          = rData.mUniqueKey;
        hint.RequestNo          = rData.mRequestNo;
        hint.FrameNo            = rData.mFrameNo;

        extract_by_SensorOpenId(&hint, rData.mSensorId);

        auto DumpYuvBuffer = [&](IImageBuffer* pImgBuf, YUV_PORT port, const char* pStr, MBOOL bCfg) -> MVOID {
            if (pImgBuf == NULL)
                return;
            {
                extract(&hint, pImgBuf);

                if (rData.mDumpWithMargin) {
                    MY_LOGD("reset dump size");
                    hint.ImgWidth = rData.mOriSize.w;
                    hint.ImgHeight = rData.mOriSize.h;
                }

                // reset FrameNo, UniqueKey and RequestNo
                if (bCfg) {
                    FileReadRule rule;
                    MINT32 index = 0;
                    tryGetMetadata<MINT32>(rData.mpIMetaHal, MTK_HAL_REQUEST_INDEX_BSS, index);
                    rule.on_device_dump_file_rename(index, rData.mEnableMFB ? "MFNR" : "single_capture", &hint, NULL);

                    trySetMetadata<MINT32>(rData.mpIMetaHal, MTK_PIPELINE_DUMP_UNIQUE_KEY,     hint.UniqueKey);
                    trySetMetadata<MINT32>(rData.mpIMetaHal, MTK_PIPELINE_DUMP_REQUEST_NUMBER, hint.RequestNo);
                    trySetMetadata<MINT32>(rData.mpIMetaHal, MTK_PIPELINE_DUMP_FRAME_NUMBER,   hint.FrameNo);
                }
                if (pStr == NULL) {
                    genFileName_YUV(filename, sizeof(filename), &hint, port);
                } else {
                    genFileName_YUV(filename, sizeof(filename), &hint, port, pStr);
                }

                // restore FrameNo, UniqueKey and RequestNo
                if (bCfg) {
                    hint.UniqueKey = rData.mUniqueKey;
                    hint.RequestNo = rData.mRequestNo;
                    hint.FrameNo   = rData.mFrameNo;
                }

                pImgBuf->saveToFile(filename);
                MY_LOGD("Dump YUV: %s", filename);
            }
        };

        auto DumpLcsBuffer = [&](IImageBuffer* pImgBuf, const char* pStr) -> MVOID {
            if (pImgBuf == NULL)
                return;

            extract(&hint, pImgBuf);
            genFileName_LCSO(filename, sizeof(filename), &hint, pStr);
            pImgBuf->saveToFile(filename);
            MY_LOGD("Dump LCEI: %s", filename);
        };

        auto DumpRawBuffer = [&](IImageBuffer* pImgBuf, RAW_PORT port, const char* pStr) -> MVOID {
            if (pImgBuf == NULL)
                return;

            extract(&hint, pImgBuf);
            genFileName_RAW(filename, sizeof(filename), &hint, port, pStr);
            pImgBuf->saveToFile(filename);
            MY_LOGD("Dump RAW: %s", filename);
        };

        if (rData.mEnableMFB) {
            String8 str;
            if (rData.mIMG3O.mTypeId != TID_MAN_FULL_YUV) {// node's working for dump
                DumpYuvBuffer(rData.mIMG3O.mpBuf, YUV_PORT_IMG3O, NULL, rData.mDebugLoadIn);
            }

            // do NOT show sensor name for MFNR naming
            hint.SensorDev = -1;

            MINT32 iso = 0;
            MINT64 exp = 0;
            tryGetMetadata<MINT32>(rData.mpIMetaDynamic, MTK_SENSOR_SENSITIVITY, iso);
            tryGetMetadata<MINT64>(rData.mpIMetaDynamic, MTK_SENSOR_EXPOSURE_TIME, exp);

            // convert ns into us
            exp /= 1000;

            auto DumpMFNRBuffer = [&](P2Output& o) {

                switch (o.mTypeId) {
                    case TID_MAN_FULL_YUV:
                        if(!pNode->mDumpMFNRYuv) return;
                        str = String8::format("mfll-iso-%d-exp-%" PRId64 "-bfbld-yuv", iso, exp);
                        break;
                    case TID_MAN_SPEC_YUV:
                        str = String8::format("mfll-iso-%d-exp-%" PRId64 "-bfbld-qyuv", iso, exp);
                        break;
                    default:
                        return;
                }
                DumpYuvBuffer(o.mpBuf, YUV_PORT_NULL, str.string(), rData.mDebugLoadIn);
            };

            if (rData.mDebugDumpFilter & DUMP_ROUND1_IMG3O)
                DumpMFNRBuffer(rData.mIMG3O);

            if (rData.mDebugDumpFilter & DUMP_ROUND1_WDMAO)
                DumpMFNRBuffer(rData.mWDMAO);

            if (rData.mDebugDumpFilter & DUMP_ROUND1_WROTO)
                DumpMFNRBuffer(rData.mWROTO);

            if (rData.mDebugDumpFilter & DUMP_ROUND1_IMG2O)
                DumpYuvBuffer(rData.mIMG2O.mpBuf, YUV_PORT_IMG2O, NULL, rData.mDebugLoadIn);

            if (rData.mDebugDumpFilter & DUMP_ROUND1_LCEI)
            {
                IImageBuffer* pLCEI = rData.mLCEI.mpBuf;
                if (pLCEI != NULL) {
                    str = String8::format("mfll-iso-%d-exp-%" PRId64 "-bfbld-lcso", iso, exp);
                    DumpLcsBuffer(pLCEI, str.string());
                }
            }

            if (rData.mDebugDumpFilter & DUMP_ROUND1_IMGI) {
                str = String8::format("mfll-iso-%d-exp-%" PRId64 "-bfbld-raw", iso, exp);
                if (rData.mDebugUnpack) {
                    android::sp<IIBuffer> pWorkingBuffer = pPackage->mUnpackBuffer;
                    unpackRawBuffer(rData.mIMGI.mpBuf,pWorkingBuffer->getImageBufferPtr());
                    DumpRawBuffer(pWorkingBuffer->getImageBufferPtr(), RAW_PORT_NULL, str.string());
                } else {
                    DumpRawBuffer(rData.mIMGI.mpBuf, RAW_PORT_NULL, str.string());
                }
            }
        } else {

            String8 str;

            if (rData.mNodeId == NID_P2B) {
                str += "single";
            }
            if (rData.mRound > 1) {
                if (str.length() != 0)
                    str += "-";

                str += "round2";
            }

            const char* pStr = str.string();
            if ((rData.mRound < 2 && rData.mDebugDumpFilter & DUMP_ROUND1_IMG3O) ||
                (rData.mRound == 2 && rData.mDebugDumpFilter & DUMP_ROUND2_IMG3O))
            {
                DumpYuvBuffer(rData.mIMG3O.mpBuf, YUV_PORT_IMG3O, pStr, rData.mDebugLoadIn);
            }

            if ((rData.mRound < 2 && rData.mDebugDumpFilter & DUMP_ROUND1_IMG2O) ||
                (rData.mRound == 2 && rData.mDebugDumpFilter & DUMP_ROUND2_IMG2O))
            {
                DumpYuvBuffer(rData.mIMG2O.mpBuf, YUV_PORT_IMG2O, pStr, rData.mDebugLoadIn);
            }

            if ((rData.mRound < 2 && rData.mDebugDumpFilter & DUMP_ROUND1_WDMAO) ||
                (rData.mRound == 2 && rData.mDebugDumpFilter & DUMP_ROUND2_WDMAO))
            {
                DumpYuvBuffer(rData.mWDMAO.mpBuf, YUV_PORT_WDMAO, pStr, rData.mDebugLoadIn);
            }

            if ((rData.mRound < 2 && rData.mDebugDumpFilter & DUMP_ROUND1_WROTO) ||
                (rData.mRound == 2 && rData.mDebugDumpFilter & DUMP_ROUND2_WROTO))
            {
                DumpYuvBuffer(rData.mWROTO.mpBuf, YUV_PORT_WROTO, pStr, rData.mDebugLoadIn);
            }

            if ((rData.mRound < 2 && rData.mDebugDumpFilter & DUMP_ROUND1_TIMGO) ||
                (rData.mRound == 2 && rData.mDebugDumpFilter & DUMP_ROUND2_TIMGO))
            {
                DumpYuvBuffer(rData.mTIMGO.mpBuf, YUV_PORT_TIMGO, pStr, rData.mDebugLoadIn);
            }

            if (rData.mRound < 2 && rData.mDebugDumpFilter & DUMP_ROUND1_LCEI)
            {
                DumpLcsBuffer(rData.mLCEI.mpBuf, NULL);
            }

            if (!rData.mYuvRep && rData.mRound < 2 && rData.mDebugDumpFilter & DUMP_ROUND1_IMGI) {
               if (rData.mDebugUnpack) {
                    android::sp<IIBuffer> pWorkingBuffer = pPackage->mUnpackBuffer;
                    unpackRawBuffer(rData.mIMGI.mpBuf,pWorkingBuffer->getImageBufferPtr());
                    DumpRawBuffer(
                           pWorkingBuffer->getImageBufferPtr(),
                           rData.mResized ? RAW_PORT_RRZO : RAW_PORT_IMGO,
                           NULL);
                } else {
                    DumpRawBuffer(
                           rData.mIMGI.mpBuf,
                           rData.mResized ? RAW_PORT_RRZO : RAW_PORT_IMGO,
                           NULL);
               }
            } else if (rData.mYuvRep && rData.mDebugDumpFilter & DUMP_ROUND1_IMGI) {
                   DumpYuvBuffer(rData.mIMGI.mpBuf, YUV_PORT_IMGI, pStr, rData.mDebugLoadIn);
            }
        }
    }

    // Update EXIF Metadata
    if (rData.mDebugDumpMDP && rData.mpOMetaHal) {

        auto GetMdpSetting = [] (void* p) -> MDPSetting*
        {
            if (p != nullptr) {
                DpPqParam* pParam = static_cast<DpPqParam*>(p);
                MDPSetting* pSetting = pParam->u.isp.p_mdpSetting;
                if (pSetting != nullptr)
                    return pSetting;
            }
            return NULL;
        };

        auto pSetting = GetMdpSetting(pPackage->mpPQParam->WDMAPQParam);

        if (pSetting == NULL)
            pSetting = GetMdpSetting(pPackage->mpPQParam->WROTPQParam);

        if (pSetting || rData.mScaleType == eScale_Down) {
            IMetadata exifMeta;
            tryGetMetadata<IMetadata>(rData.mpOMetaHal, MTK_3A_EXIF_METADATA, exifMeta);
            // Append MDP EXIF if required
            if (pSetting) {
                unsigned char* pBuffer = static_cast<unsigned char*>(pSetting->buffer);
                MUINT32 size = pSetting->size;
                if (DebugExifUtils::setDebugExif(
                        DebugExifUtils::DebugExifType::DEBUG_EXIF_RESERVE3,
                        static_cast<MUINT32>(MTK_RESVC_EXIF_DBGINFO_KEY),
                        static_cast<MUINT32>(MTK_RESVC_EXIF_DBGINFO_DATA),
                        size, pBuffer, &exifMeta) == nullptr)
                {
                    MY_LOGW("fail to set mdp debug exif to metadata");
                }
                else
                    MY_LOGI("Update Mdp debug info: addr %p, size %u %d %d", pBuffer, size, *pBuffer, *(pBuffer+1));
            }
            // Append DSDN EXIF if required
            if (rData.mScaleType == eScale_Down) {
                std::map<MUINT32, MUINT32> debugInfo;
                {
                    using namespace dbg_cam_common_param_1;
                    debugInfo[CMN_TAG_DOWNSCALE_DENOISE_WIDTH] = rData.mScaleSize.w;
                    debugInfo[CMN_TAG_DOWNSCALE_DENOISE_HIGHT] = rData.mScaleSize.h ;
                }
                if (DebugExifUtils::setDebugExif(
                        DebugExifUtils::DebugExifType::DEBUG_EXIF_CAM,
                        static_cast<MUINT32>(MTK_CMN_EXIF_DBGINFO_KEY),
                        static_cast<MUINT32>(MTK_CMN_EXIF_DBGINFO_DATA),
                        debugInfo, &exifMeta) == nullptr)
                {
                    MY_LOGW("fail to set dsdn debug exif to metadata");
                }
            }

            trySetMetadata<IMetadata>(rData.mpOMetaHal, MTK_3A_EXIF_METADATA, exifMeta);
        }
    }

    MBOOL hasCopyTask = MFALSE;
    if (rData.mCopy1.mpBuf != NULL || rData.mCopy2.mpBuf != NULL) {
        pPackage->miSensorIdx = rData.mSensorId;
        sp<CaptureTaskQueue> pTaskQueue = pNode->mpTaskQueue;
        if (pTaskQueue != NULL) {

            std::function<void()> func =
                [pPackage]()
                {
                    copyBuffers(pPackage);
                    delete pPackage;
                };

            pTaskQueue->addTask(func);
            hasCopyTask = MTRUE;
        }
    }

    if (!hasCopyTask) {
        // Could early callback only if there is no copy task
        if (rData.mWROTO.mEarlyRelease || rData.mWDMAO.mEarlyRelease) {

            RequestPtr pRequest = rData.mpHolder->mpRequest;
            sp<CaptureFeatureNodeRequest> pNodeReq = pRequest->getNodeRequest(rData.mNodeId);

            if (rData.mWROTO.mBufId != NULL_BUFFER)
                pNodeReq->releaseBuffer(rData.mWROTO.mBufId);
            if (rData.mWDMAO.mBufId != NULL_BUFFER)
                pNodeReq->releaseBuffer(rData.mWDMAO.mBufId);
        }

        delete pPackage;
    }
}

/*******************************************************************************
 *
 ********************************************************************************/
MVOID
P2ANode::
onP2FailedCallback(QParams& rParams)
{
    EnquePackage* pPackage = (EnquePackage*) (rParams.mpCookie);
    P2EnqueData& rData = *pPackage->mpEnqueData.get();
    rData.mpHolder->mStatus = UNKNOWN_ERROR;

    CAM_TRACE_ASYNC_END("P2_Enque", (rData.mFrameNo << 3) + rData.mTaskId);
    pPackage->stop();

    MY_LOGE("R/F/S Num: %d/%d/%d, task:%d, timeconsuming: %dms",
            rData.mRequestNo,
            rData.mFrameNo,
            rData.mSensorId,
            rData.mTaskId,
            pPackage->getElapsed());

    delete pPackage;
}

/*******************************************************************************
 *
 ********************************************************************************/
MBOOL
P2ANode::
onRequestProcess(NodeID_T nodeId, RequestPtr& pRequest)
{
    MINT32 requestNo = pRequest->getRequestNo();
    MINT32 frameNo = pRequest->getFrameNo();
    MINT32 masterId = pRequest->getMainSensorIndex();
    MINT32 slaveId = pRequest->getSubSensorIndex();
    MBOOL isPhysical = pRequest->isPhysicalStream();

    setSensorFmt(masterId, slaveId);

    const MBOOL isCroppedFSYUV = (pRequest->hasParameter(PID_CROPPED_FSYUV) ? (pRequest->getParameter(PID_CROPPED_FSYUV) > 0) : MFALSE);
    MY_LOGD("cropped FSYUV info, isCroppedFSYUV:%d", isCroppedFSYUV);

    if (mDebugPerFrame) {
        mDebugDS                = property_get_int32("vendor.debug.camera.ds.enable", -1);
        mDebugDSRatio_dividend  = property_get_int32("vendor.debug.camera.ds.ratio_dividend", 8);
        mDebugDSRatio_divisor   = property_get_int32("vendor.debug.camera.ds.ratio_divisor", 16);
        mDebugDRE               = property_get_int32("vendor.debug.camera.dre.enable", -1);
        mDebugCZ                = property_get_int32("vendor.debug.camera.cz.enable", -1);

#ifdef SUPPORT_HFG
        mDebugHFG               = property_get_int32("vendor.debug.camera.hfg.enable", -1);
#endif

#ifdef SUPPORT_DCE
        mDebugDCE               = property_get_int32("vendor.debug.camera.dce.enable", -1);
#endif
    }

#ifdef GTEST
    MY_LOGD("run GTEST, return directly, request:%d", requestNo);
    dispatch(pRequest);
    return MTRUE;
#endif

    CAM_TRACE_FMT_BEGIN("p2a:process|r%df%ds%d", requestNo, frameNo, masterId);
    MY_LOGI("(%d) +, R/F/S Num: %d/%d/%d", nodeId, requestNo, frameNo, masterId);

    sp<CaptureFeatureNodeRequest> pNodeReq = pRequest->getNodeRequest(nodeId);
    MBOOL ret;

    // 0. Create request holder
    incExtThreadDependency();
    shared_ptr<RequestHolder> pHolder(new RequestHolder(pRequest),
            [=](RequestHolder* p) mutable
            {
                if (p->mStatus != OK)
                    p->mpRequest->addParameter(PID_FAILURE, 1);

                p->mpHolders.clear();

                onRequestFinish(nodeId, p->mpRequest);
                decExtThreadDependency();
                delete p;
            }
    );

    // Make sure that requests are finished in order
    {
        shared_ptr<RequestHolder> pLastHolder = mpLastHolder.lock();
        if (pLastHolder != NULL)
            pLastHolder->mpPrecedeOver = pHolder;

        mpLastHolder = pHolder;
    }

    //
    shared_ptr<SharedData> pSharedData = std::make_shared<SharedData>();

    // check drop frame
    if (pRequest->isCancelled())
    {
        MY_LOGD("Cancel, R/F/S Num: %d/%d/%d", requestNo, frameNo, masterId);
        return MFALSE;
    }

    // 0-1. Acquire Metadata
    IMetadata* pIMetaDynamic    = pNodeReq->acquireMetadata(MID_MAN_IN_P1_DYNAMIC);
    IMetadata* pIMetaApp        = pNodeReq->acquireMetadata(MID_MAN_IN_APP);
    IMetadata* pIMetaHal        = pNodeReq->acquireMetadata(MID_MAN_IN_HAL);
    IMetadata* pOMetaApp        = pNodeReq->acquireMetadata(MID_MAN_OUT_APP);
    IMetadata* pOMetaHal        = pNodeReq->acquireMetadata(MID_MAN_OUT_HAL);

    IMetadata* pIMetaHal2       = NULL;
    IMetadata* pIMetaDynamic2   = NULL;
    if (slaveId >= 0) {
        pIMetaHal2 = pNodeReq->acquireMetadata(MID_SUB_IN_HAL);
        pIMetaDynamic2 = pNodeReq->acquireMetadata(MID_SUB_IN_P1_DYNAMIC);
    }
    //
    const MBOOL isBayerBayer = (mSensorFmt != SENSOR_RAW_MONO) && (mSensorFmt2 != SENSOR_RAW_MONO);
    const MBOOL isBayerMono = (mSensorFmt != SENSOR_RAW_MONO) && (mSensorFmt2 == SENSOR_RAW_MONO);
    MBOOL bIsDualMode = ((mUsageHint.mDualMode != 0) && (slaveId >= 0));
    ESensorCombination sensorCom = (bIsDualMode && isBayerBayer) ? eSensorComb_BayerBayer :
                                   (bIsDualMode && isBayerMono) ? eSensorComb_BayerMono :
                                   (!bIsDualMode) ? eSensorComb_Single : eSensorComb_Invalid;
    // profile mapping key
    ProfileMapperKey mappingKey = mpISPProfileMapper->queryMappingKey(pIMetaApp, pIMetaHal, isPhysical, eMappingScenario_Capture, sensorCom);
    // ISP Profile mapper query
    auto fnQueryISPProfile = [&](const ProfileMapperKey& key, const EProfileMappingStages& stage) -> MINT32 {
#if MTKCAM_ENABLE_ISPPROFILE_MAPPER == 1
        EIspProfile_T ispProfile;
        if(!mpISPProfileMapper->mappingProfile(key, stage, ispProfile))
        {
            MY_LOGD("Failed to get ISP Profile, stage:%d, use EIspProfile_Capture as default", stage);
            ispProfile = EIspProfile_Capture;
        }
        return ispProfile;
#else
        return -1;
#endif
    };

    // 0-2. Get Data
    MINT32 uniqueKey = 0;
    tryGetMetadata<MINT32>(pIMetaHal, MTK_PIPELINE_UNIQUE_KEY, uniqueKey);

    MINT32 iIsoValue = 0;
    tryGetMetadata<MINT32>(pIMetaDynamic, MTK_SENSOR_SENSITIVITY, iIsoValue);

    // 0-3. Set Index
    if (pRequest->hasParameter(PID_FRAME_INDEX_FORCE_BSS)) {
        trySetMetadata<MINT32>(pIMetaHal, MTK_HAL_REQUEST_INDEX_BSS, pRequest->getParameter(PID_FRAME_INDEX_FORCE_BSS));
    }

    const MBOOL isVsdof = isVSDoFMode(mUsageHint);
    if (isVsdof && mpFOVCalculator->getIsEnable()) {
        pRequest->addParameter(PID_IGNORE_CROP, 1);
        MY_LOGD("R/F/S Num: %d/%d/%d set ignore crop", requestNo, frameNo, masterId);
    }

    if (!pIMetaHal->entryFor(MTK_P2NODE_CROP_REGION).isEmpty()) {
        pRequest->addParameter(PID_ENABLE_P2_CROP, 1);
    }

    // 1. Resized RAW of main sensor
    {
        BufferID_T uOResizedYuv = pNodeReq->mapBufferID(TID_MAN_RSZ_YUV, OUTPUT);
        BufferID_T uOPostview = mISP3_0 ? pNodeReq->mapBufferID(TID_POSTVIEW, OUTPUT) : NULL_BUFFER;
        BufferID_T uOThumbnail = mISP3_0 ? pNodeReq->mapBufferID(TID_THUMBNAIL, OUTPUT) : NULL_BUFFER;
        if ((uOResizedYuv & uOPostview & uOThumbnail) != NULL_BUFFER) {
            shared_ptr<P2EnqueData> pEnqueData = make_shared<P2EnqueData>();
            P2EnqueData& rEnqueData = *pEnqueData.get();

            rEnqueData.mpHolder             = pHolder;
            rEnqueData.mpSharedData         = pSharedData;
            rEnqueData.mIMGI.mBufId         = pNodeReq->mapBufferID(TID_MAN_RSZ_RAW, INPUT);
            rEnqueData.mWDMAO.mBufId        = uOResizedYuv;
            rEnqueData.mWROTO.mBufId        = uOPostview;
            rEnqueData.mWROTO.mHasCrop      = (uOPostview != NULL_BUFFER);
            rEnqueData.mWROTO.mEarlyRelease = (uOPostview != NULL_BUFFER);
            rEnqueData.mIMG2O.mBufId        = uOThumbnail;
            rEnqueData.mIMG2O.mHasCrop      = (uOThumbnail != NULL_BUFFER);
            rEnqueData.mpIMetaApp = pIMetaApp;
            rEnqueData.mpIMetaHal = pIMetaHal;
            rEnqueData.mSensorId  = masterId;
            rEnqueData.mResized   = MTRUE;
            rEnqueData.mEnableVSDoF = isVsdof;
            rEnqueData.mUniqueKey = uniqueKey;
            rEnqueData.mRequestNo = requestNo;
            rEnqueData.mFrameNo   = frameNo;
            rEnqueData.mTaskId    = TASK_M1_RRZO;
            rEnqueData.mNodeId    = nodeId;
            rEnqueData.mTimeSharing = USEING_TIME_SHARING;
            rEnqueData.mIspProfile = fnQueryISPProfile(mappingKey, eStage_RRZ_R2Y_Main);

            ret = enqueISP(
                pRequest,
                pEnqueData);

            if (!ret) {
                MY_LOGE("main sensor: resized yuv failed!");
                return MFALSE;
            }
        }
    }
    // 1-1. Full RAW of main sensor and output raw
    {
        BufferID_T uORawOut = pNodeReq->mapBufferID(TID_MAN_FULL_RAW, OUTPUT);
        if (uORawOut != NULL_BUFFER) {

            shared_ptr<P2EnqueData> pEnqueData = make_shared<P2EnqueData>();
            P2EnqueData& rEnqueData = *pEnqueData.get();

            rEnqueData.mpHolder             = pHolder;
            rEnqueData.mpSharedData         = pSharedData;
            rEnqueData.mIMGI.mBufId         = pNodeReq->mapBufferID(TID_MAN_FULL_RAW, INPUT);
            rEnqueData.mIMGI.mpBuf          = pNodeReq->acquireBuffer(rEnqueData.mIMGI.mBufId);
            rEnqueData.mpIMetaDynamic       = pIMetaDynamic;
            rEnqueData.mpIMetaApp           = pIMetaApp;
            rEnqueData.mpIMetaHal           = pIMetaHal;
            rEnqueData.mpOMetaApp           = pOMetaApp;
            rEnqueData.mpOMetaHal           = pOMetaHal;

            {   // output TIMGO port
                MSize fullSize = rEnqueData.mIMGI.mpBuf->getImgSize();
                auto addRawOutput = [&]() -> MBOOL {
                    TypeID_T typeId = TID_MAN_FULL_RAW;
                    BufferID_T bufId = pNodeReq->mapBufferID(typeId, OUTPUT);
                    if (bufId == NULL_BUFFER)
                        return MFALSE;

                    IImageBuffer* pBuf = pNodeReq->acquireBuffer(bufId);
                    if (pBuf == NULL) {
                        MY_LOGE("should not have null buffer. type:%d, buf:%d",typeId, bufId);
                        return MFALSE;
                    }

                    MUINT32 trans = pNodeReq->getImageTransform(bufId);
                    MRect cropRegion = MRect(fullSize.w, fullSize.h);
                    MUINT32 scaleRatio = 1;

                    P2Output &o = rEnqueData.mTIMGO;
                    o.mpBuf       = pBuf;
                    o.mBufId      = bufId;
                    o.mTypeId     = typeId;
                    o.mHasCrop    = MFALSE;
                    o.mTrans      = trans;
                    o.mCropRegion = cropRegion;
                    o.mScaleRatio = scaleRatio;

                    return MTRUE;
                };
                auto ret = addRawOutput();
                if(ret)
                    MY_LOGD("add RawImg Fmt(0x%x) VA/PA(%#" PRIxPTR "/%#" PRIxPTR ") to output port",
                        rEnqueData.mTIMGO.mpBuf->getImgFormat(),
                        rEnqueData.mTIMGO.mpBuf->getBufVA(0),
                        rEnqueData.mTIMGO.mpBuf->getBufPA(0));
            }

            rEnqueData.mSensorId  = masterId;
            rEnqueData.mUniqueKey = uniqueKey;
            rEnqueData.mRequestNo = requestNo;
            rEnqueData.mFrameNo   = frameNo;
            rEnqueData.mTaskId    = TASK_M1_IMGO_RAW;
            rEnqueData.mNodeId    = nodeId;
            rEnqueData.mTimeSharing = MTRUE;
            rEnqueData.mIspProfile = fnQueryISPProfile(mappingKey, eStage_R2YDCE_1stRun);

            ret = enqueISP(
                pRequest,
                pEnqueData);

            if (!ret) {
                MY_LOGE("main sensor: full raw out failed!");
                return MFALSE;
            }
        }
    }
    // 2. Full RAW of main sensor
    // YUV Reprocessing
    MBOOL isYuvRep = pNodeReq->mapBufferID(TID_MAN_FULL_YUV, INPUT) != NULL_BUFFER;
    // Down Scale: Only for IMGO
    MINT32 iDSRatio_dividend = 1;
    MINT32 iDSRatio_divisor  = 1;
    IImageBuffer* pRound1_Buf = NULL;

    std::shared_ptr<Mutex> pLockDCES;
    IImageBuffer* pDCES_Buf = NULL;
    MBOOL hasDCES = MFALSE;
    if (mISP6_0) {
        // [AINR] DCES from P2B
        BufferID_T uIBufDCE = pNodeReq->mapBufferID(TID_MAN_DCES, INPUT);
        if (uIBufDCE != NULL_BUFFER) {
            hasDCES = MTRUE;
            pDCES_Buf = pNodeReq->acquireBuffer(uIBufDCE);
        }
    }

    MSize fullSize;
    MSize downSize;

    MBOOL isRunDSDN = MFALSE;
    MBOOL isRunDCE = MFALSE;
    MBOOL hasYUVOutput = checkHasYUVOutput(pNodeReq);

    if(hasYUVOutput)
    {
        // only main frame need DCE
        if (mISP6_0 && pRequest->isMainFrame() &&
            (pRequest->hasFeature(FID_AINR) || pRequest->hasFeature(FID_AINR_YHDR))
        )  {
            isRunDCE = MTRUE;
        } else if (!mSupportDCE || !pRequest->isSingleFrame() || isYuvRep || mDebugDCE == OFF) {
            // do NOT execute DCE if multi-frame blending or YUV reprocessing or force off
        } else if (pRequest->hasFeature(FID_DCE) || mDebugDCE == ON) {
            isRunDCE = MTRUE;
        }

        if (mISP3_0 || isYuvRep || isVsdof || mDebugDS == OFF || mSupportDS == MFALSE || pRequest->hasParameter(PID_CSHOT_REQUEST) || isCroppedFSYUV) {
            if (mDebugDS == ON) {
                // do NOT execute down-scale if VSDOF or YUV reprocessing or force off or ISP30
                MY_LOGI("do NOT support DSDN due to (isYuvRep:%d, isVsdof:%d, mDebugDS:%d, mISP3_0:%d, isCroppedFSYUV:%d)",
                        isYuvRep, isVsdof, mDebugDS, mISP3_0, isCroppedFSYUV);
            }
            isRunDSDN  = MFALSE;
        } else if (mDebugDS == ON) {
            iDSRatio_dividend = mDebugDSRatio_dividend;
            iDSRatio_divisor  = mDebugDSRatio_divisor;
            MY_LOGD("force downscale ratio: (%d/%d)",iDSRatio_dividend, iDSRatio_divisor);
            isRunDSDN  = MTRUE;
        } else if (pRequest->isSingleFrame()){  /* TODO: set metadata if non-single feature needs to use DSDN */
    #if MTK_CAM_NEW_NVRAM_SUPPORT
            MINT32 iMagicNo = 0;
            MUINT32 idx = 1;
            tryGetMetadata<MINT32>(pIMetaHal, MTK_P1NODE_PROCESSOR_MAGICNUM, iMagicNo);
    #ifdef SUPPORT_DSDN_20
            FEATURE_NVRAM_DSDN_T* t = (FEATURE_NVRAM_DSDN_T*)
                getTuningFromNvram(masterId, idx, iMagicNo, NVRAM_TYPE_DSDN, pIMetaHal, mLogLevel > 0, mUsageHint.mIsHidlIsp, fnQueryISPProfile(mappingKey, eStage_DSDN20_2ndRun));
            if (t != NULL) {
                iDSRatio_dividend = t->dsdn_dividend;
                iDSRatio_divisor  = t->dsdn_divisor;
                MY_LOGD("downscale ratio: (%d/%d)",iDSRatio_dividend, iDSRatio_divisor);
                if (iDSRatio_divisor < 1) {
                    MY_LOGE("Has wrong downscale ratio: (%d,%d)", iDSRatio_dividend, iDSRatio_divisor);
                    iDSRatio_divisor  = 1;
                    iDSRatio_dividend = 1;
                }
            } else {
                MY_LOGE("fail to query nvram!");
            }

            isRunDSDN = iDSRatio_dividend != iDSRatio_divisor;
    #else
            NVRAM_CAMERA_FEATURE_SWNR_THRES_STRUCT* t = (NVRAM_CAMERA_FEATURE_SWNR_THRES_STRUCT*)
                getTuningFromNvram(masterId, idx, iMagicNo, NVRAM_TYPE_SWNR_THRES, pIMetaHal, mLogLevel > 0, mUsageHint.mIsHidlIsp);
            if (t != NULL) {
                iDSRatio_divisor  = t->downscale_ratio;
                isRunDSDN = t->downscale_thres <= iIsoValue;

                MY_LOGD("Decide downscale iso:%d thres:%d ratio: %d",iIsoValue, t->downscale_thres, iDSRatio_divisor);
                if (iDSRatio_divisor < 1) {
                    MY_LOGE("Has wrong downscale ratio: %d", iDSRatio_divisor);
                    iDSRatio_divisor  = 1;
                    iDSRatio_dividend = 1;
                }
            } else {
                MY_LOGE("fail to query nvram!");
            }
    #endif
    #endif
        }
        if (iDSRatio_divisor == iDSRatio_dividend) {
            isRunDSDN = MFALSE;
        }
    }

    // 2-1. Downscale De-noise or DCE
    if (isRunDSDN || (isRunDCE && !hasDCES)) {
        shared_ptr<P2EnqueData> pEnqueData = make_shared<P2EnqueData>();
        P2EnqueData& rEnqueData = *pEnqueData.get();
        rEnqueData.mpHolder  = pHolder;
        rEnqueData.mpSharedData = pSharedData;
        rEnqueData.mIMGI.mBufId = pNodeReq->mapBufferID(TID_MAN_FULL_RAW, INPUT);
        rEnqueData.mIMGI.mpBuf  = pNodeReq->acquireBuffer(rEnqueData.mIMGI.mBufId);
        rEnqueData.mLCEI.mBufId = pNodeReq->mapBufferID(TID_MAN_LCS, INPUT);
        rEnqueData.mpIMetaDynamic = pIMetaDynamic;
        rEnqueData.mpIMetaApp = pIMetaApp;
        rEnqueData.mpIMetaHal = pIMetaHal;
        rEnqueData.mpOMetaApp = pOMetaApp;
        rEnqueData.mpOMetaHal = pOMetaHal;

        fullSize = rEnqueData.mIMGI.mpBuf->getImgSize();

        if (isRunDCE) {
            pLockDCES = make_shared<Mutex>();
#ifdef SUPPORT_AINR
            if(pRequest->hasFeature(FID_AINR) || pRequest->hasFeature(FID_AINR_YHDR))
            {
                rEnqueData.mIspProfile = EIspProfile_AINR_Main;
            }
#endif
            BufferID_T uOBufDCE = pNodeReq->mapBufferID(TID_MAN_DCES, OUTPUT);
            if (uOBufDCE != NULL_BUFFER) {
                MY_LOGD("acquire DCE from pipe. nodeId:%d", nodeId);
                pDCES_Buf = pNodeReq->acquireBuffer(uOBufDCE);
            } else {
                android::sp<IIBuffer> pWorkingBuffer = mpBufferPool->getImageBuffer(mDCES_Size, (EImageFormat) mDCES_Format);
                pHolder->mpBuffers.push_back(pWorkingBuffer);
                pDCES_Buf = pWorkingBuffer->getImageBufferPtr();
            }
            if (pDCES_Buf) {
                for (size_t i = 0 ; i < pDCES_Buf->getPlaneCount() ; i++) {
                    ::memset((void *)pDCES_Buf->getBufVA(i), 0, pDCES_Buf->getBufSizeInBytes(i));
                }
                pDCES_Buf->syncCache(eCACHECTRL_FLUSH);
            }
            rEnqueData.mDCESO.mpBuf = pDCES_Buf;
            rEnqueData.mpLockDCES = pLockDCES;

            if (isRunDSDN) {
                downSize = MSize(((fullSize.w * iDSRatio_dividend)/ iDSRatio_divisor) & ~0x3, ((fullSize.h * iDSRatio_dividend)/ iDSRatio_divisor) & ~0x1);
                MY_LOGD("apply down-scale denoise: (%dx%d) -> (%dx%d)",
                        fullSize.w, fullSize.h, downSize.w, downSize.h);

                // Get working buffer
                android::sp<IIBuffer> pWorkingBuffer = mpBufferPool->getImageBuffer(downSize, eImgFmt_MTK_YUV_P010, MSize(16,4));
                // Push to resource holder
                pHolder->mpBuffers.push_back(pWorkingBuffer);
                pRound1_Buf = pWorkingBuffer->getImageBufferPtr();
                rEnqueData.mWDMAO.mpBuf = pRound1_Buf;
                rEnqueData.mWDMAO.mHasCrop = pRequest->hasParameter(PID_ENABLE_P2_CROP) ? MTRUE : MFALSE;
                rEnqueData.mScaleType = eScale_Down;
                rEnqueData.mScaleSize = downSize;
            } else {
                android::sp<IIBuffer> pWorkingBuffer = mpBufferPool->getImageBuffer(fullSize, eImgFmt_MTK_YUV_P010, MSize(16,4));
                // Push to resource holder
                pHolder->mpBuffers.push_back(pWorkingBuffer);
                pRound1_Buf = pWorkingBuffer->getImageBufferPtr();
                rEnqueData.mIMG3O.mpBuf = pRound1_Buf;
            }

        } else {
            // should use MDP to do down-scaling
            if (isRunDSDN) {
#ifdef SUPPORT_DSDN_20
                downSize = MSize(((fullSize.w * iDSRatio_dividend)/ iDSRatio_divisor) & ~0x3, ((fullSize.h * iDSRatio_dividend)/ iDSRatio_divisor) & ~0x1);
                MUINT32 format = eImgFmt_MTK_YUV_P010;
                MUINT32 alignW = 16;
#else
                downSize = MSize(((fullSize.w * iDSRatio_dividend)/ iDSRatio_divisor) & ~0x1, ((fullSize.h * iDSRatio_dividend)/ iDSRatio_divisor) & ~0x1);
                MUINT32 format = eImgFmt_NV12;
                MUINT32 alignW = 4;
#endif
                MY_LOGD("apply down-scale denoise: (%dx%d) -> (%dx%d)",
                        fullSize.w, fullSize.h, downSize.w, downSize.h);
                // Get working buffer
                android::sp<IIBuffer> pWorkingBuffer = mpBufferPool->getImageBuffer(downSize, format, MSize(alignW,4));
                // Push to resource holder
                pHolder->mpBuffers.push_back(pWorkingBuffer);
                pRound1_Buf = pWorkingBuffer->getImageBufferPtr();
                rEnqueData.mWDMAO.mpBuf = pRound1_Buf;
                rEnqueData.mScaleType = eScale_Down;
                rEnqueData.mScaleSize = downSize;
            }
        }

        rEnqueData.mSensorId    = masterId;
        rEnqueData.mUniqueKey   = uniqueKey;
        rEnqueData.mRequestNo   = requestNo;
        rEnqueData.mFrameNo     = frameNo;
        rEnqueData.mTaskId      = TASK_M1_IMGO_PRE;
        rEnqueData.mNodeId      = nodeId;
        rEnqueData.mTimeSharing = MTRUE;
        // ISP5.0 use DSDN 1.0, which has no feature id, need to use stage to classify
        EProfileMappingStages stage = (mISP5_0) ? eStage_DSDN10_1stRun : eStage_R2YDCE_1stRun;
        rEnqueData.mIspProfile = fnQueryISPProfile(mappingKey, stage);

        addDebugBuffer(pHolder, pEnqueData);

        ret = enqueISP(
            pRequest,
            pEnqueData);

        if (!ret) {
            MY_LOGE("main sensor: downsize failed!");
            return MFALSE;
        }
    }

    // 2-2. Upscale or Full-size enque
    if (hasYUVOutput)
    {
        shared_ptr<P2EnqueData> pEnqueData = make_shared<P2EnqueData>();
        P2EnqueData& rEnqueData = *pEnqueData.get();
        rEnqueData.mpHolder  = pHolder;
        rEnqueData.mpSharedData = pSharedData;
        MBOOL isPureRaw = MFALSE;
        MBOOL isOutputMeta = MFALSE;
        if (isRunDSDN || isRunDCE) {

            if (pRound1_Buf != NULL) {
                rEnqueData.mIMGI.mpBuf = pRound1_Buf;
                rEnqueData.mRound = 2;
            } else {
                // Enque RAW with LCS
                rEnqueData.mIMGI.mBufId = pNodeReq->mapBufferID(TID_MAN_FULL_RAW, INPUT);
                rEnqueData.mIMGI.mpBuf  = pNodeReq->acquireBuffer(rEnqueData.mIMGI.mBufId);
                rEnqueData.mLCEI.mBufId = pNodeReq->mapBufferID(TID_MAN_LCS, INPUT);
                rEnqueData.mLCEI.mpBuf = pNodeReq->acquireBuffer(rEnqueData.mLCEI.mBufId);
            }

            if (isRunDCE) {
                rEnqueData.mpLockDCES   = pLockDCES;
                rEnqueData.mDCESI.mpBuf = pDCES_Buf;
                rEnqueData.mEnableDCE   = MTRUE;
            }

            if (isRunDSDN) {
                rEnqueData.mScaleType = eScale_Up;
                rEnqueData.mScaleSize = fullSize;
            }

        } else {
            if (isYuvRep) {
                rEnqueData.mIMGI.mBufId = pNodeReq->mapBufferID(TID_MAN_FULL_YUV, INPUT);
                rEnqueData.mYuvRep = MTRUE;
            } else {
                rEnqueData.mIMGI.mBufId = pNodeReq->mapBufferID(TID_MAN_FULL_RAW, INPUT);
            }
            // Check Raw type
            auto IsPureRaw = [](IImageBuffer *pBuf) -> MBOOL {
                if (pBuf == NULL)
                    return MFALSE;
                MINT64 rawType;
                if (pBuf->getImgDesc(eIMAGE_DESC_ID_RAW_TYPE, rawType))
                    return rawType == eIMAGE_DESC_RAW_TYPE_PURE;
                return MFALSE;
            };
            rEnqueData.mIMGI.mpBuf     = pNodeReq->acquireBuffer(rEnqueData.mIMGI.mBufId);
            isPureRaw = IsPureRaw(rEnqueData.mIMGI.mpBuf);

            rEnqueData.mIMGI.mPureRaw  =  isPureRaw;
            rEnqueData.mLCEI.mBufId    = pNodeReq->mapBufferID(TID_MAN_LCS, INPUT);
            isOutputMeta = MTRUE;
        }

        MSize srcSize = rEnqueData.mIMGI.mpBuf->getImgSize();

        // the larger size has higher priority, the smaller size could using larger image to copy via MDP
        const TypeID_T typeIds[] = {
                TID_MAN_FULL_YUV,
                TID_MAN_FULL_PURE_YUV,
                TID_JPEG,
                TID_MAN_CROP1_YUV,
                TID_MAN_CROP2_YUV,
                TID_MAN_CROP3_YUV,
                TID_MAN_SPEC_YUV,
                TID_MAN_FD_YUV,
                TID_POSTVIEW,
                TID_THUMBNAIL,
                TID_MAN_CLEAN
        };

        MBOOL hasP2Resizer = !mISP3_0 || !isPureRaw;
        MBOOL hasCopyBuffer = MFALSE;
        auto UsedOutput = [&](P2Output& o) -> MBOOL {
            return o.mBufId != NULL_BUFFER || o.mpBuf != NULL;
        };

        // ISP 4.0/5.0: MFNR should use IMG3O for bit-true
        MBOOL bFullByImg3o = MFALSE;
        if (mISP4_0 || mISP5_0 || mISP6_0) {
            bFullByImg3o = pRequest->hasFeature(FID_MFNR) && (pRequest->getActiveFrameCount() > 1);
        }

        // ISP 3.0: Calculate cropping & scaling for limitation of different view angle
        sp<CropCalculator::Factor> pFactor = mpCropCalculator->getFactor(pIMetaApp, pIMetaHal, masterId);

        // check if cropping for YUV reprocessing
        if (isYuvRep) {
            pRequest->addParameter(PID_REPROCESSING, 1);

            if (!mUsageHint.mIsHidlIsp || !pFactor->mbExistActiveCrop) {   // disable cropping
                pRequest->addParameter(PID_IGNORE_CROP, 1);
            }
        }

        // use to count the PQ support size
        MINT8 curUsablePQSize = SUPPORT_PQ_SIZE;
        MBOOL hasOutput = MFALSE;
        for (TypeID_T typeId : typeIds) {
            if (mISP3_0) {
                if (typeId == TID_POSTVIEW || typeId == TID_THUMBNAIL)
                    continue;
            }

            BufferID_T bufId = pNodeReq->mapBufferID(typeId, OUTPUT);
            MY_LOGD("typeID=%d(%s) bufId == NULL:%d", typeId, TypeID2Name(typeId), bufId == NULL_BUFFER);
            if (bufId == NULL_BUFFER)
                continue;

            IImageBuffer* pBuf = pNodeReq->acquireBuffer(bufId);
            if (pBuf == NULL) {
                MY_LOGE("should not have null buffer. type:%d, buf:%d",typeId, bufId);
                continue;
            }

            MUINT32 trans   = pNodeReq->getImageTransform(bufId);
            MBOOL needCrop  = typeId == TID_JPEG ||
                              typeId == TID_MAN_CROP1_YUV ||
                              typeId == TID_MAN_CROP2_YUV ||
                              typeId == TID_MAN_CROP3_YUV ||
                              typeId == TID_POSTVIEW ||
                              typeId == TID_MAN_CLEAN ||
                              typeId == TID_THUMBNAIL;

            // disable second run cropping
            if (isRunDSDN && pRequest->hasParameter(PID_ENABLE_P2_CROP)) {
                needCrop = MFALSE;
            }
            auto GetScaleCrop = [&](MRect& rCrop, MUINT32& rScaleRatio) mutable {
                if (needCrop) {
                    MSize imgSize = pBuf->getImgSize();
                    if (trans & eTransform_ROT_90)
                        swap(imgSize.w, imgSize.h);

                    if (pRequest->hasParameter(PID_IGNORE_CROP)) {
                        CropCalculator::evaluate(srcSize, imgSize, rCrop);
                    } else if (pRequest->hasParameter(PID_REPROCESSING)) {
                        CropCalculator::evaluate(srcSize, imgSize, pFactor->mActiveCrop, rCrop);
                    } else {
                        mpCropCalculator->evaluate(pFactor, imgSize, rCrop, MFALSE);
                    }
                    if (rEnqueData.mScaleType == eScale_Up && !pRequest->hasParameter(PID_IGNORE_CROP)) {
                        simpleTransform tranTG2DS(MPoint(0,0), rEnqueData.mScaleSize, srcSize);
                        rCrop = transform(tranTG2DS, rCrop);
                    }
                    rScaleRatio = imgSize.w * 100 / rCrop.s.w;
                } else {
                    rCrop = MRect(srcSize.w, srcSize.h);
                    rScaleRatio = 1;
                }
            };

#if MTKCAM_EARLY_FLIP_SUPPORT == 1
            // Do the flip in P2A
            auto doFlipInP2A = [&](MUINT32& trans, MINT32 dvOri) -> MVOID {
                if (dvOri == 90 || dvOri == 270) {
                    MY_LOGD("Capture vertical");
                    trans = eTransform_FLIP_V;
                }
                if (dvOri == 0 || dvOri == 180) {
                    MY_LOGD("Capture horizontal");
                    trans = eTransform_FLIP_H;
                }
            };

            if(typeId == TID_MAN_FULL_YUV) {
                MINT32 jpegFlipProp = property_get_int32("vendor.debug.camera.Jpeg.flip", 0);
                MINT32 jpegFlip = 0;
                if(!tryGetMetadata<MINT32>(pIMetaApp, MTK_CONTROL_CAPTURE_JPEG_FLIP_MODE, jpegFlip))
                    MY_LOGD("cannot get MTK_CONTROL_CAPTURE_JPEG_FLIP_MODE");
                if (jpegFlip || jpegFlipProp) {
                    MINT32 jpegOri = 0;
                    tryGetMetadata<MINT32>(pIMetaApp, MTK_JPEG_ORIENTATION, jpegOri);
                    MY_LOGD("before trans 0x%u", trans);
                    doFlipInP2A(trans, jpegOri);
                    MY_LOGD("after trans 0x%u", trans);
                    trySetMetadata<MINT32>(pOMetaHal, MTK_FEATURE_FLIP_IN_P2A, 1);
                }
            }
#endif

            MRect cropRegion;
            MUINT32 scaleRatio = 1;
            // Apply customized crop region to full-size domain
            if (isVsdof && ((typeId == TID_MAN_FULL_YUV) || (typeId == TID_POSTVIEW) || (typeId == TID_MAN_CLEAN) || (typeId == TID_MAN_FULL_PURE_YUV)))
            {
                FovCalculator::FovInfo fovInfo;

                if (rEnqueData.mScaleType == eScale_Up) {
                    MY_LOGE("Can not combine VSDOF with DSDN");
                }
                else if (mpFOVCalculator->getIsEnable() && mpFOVCalculator->getFovInfo(masterId, fovInfo)) {
                    needCrop = MTRUE;
                    cropRegion.p = fovInfo.mFOVCropRegion.p;
                    cropRegion.s = fovInfo.mFOVCropRegion.s;

                    if (typeId == TID_POSTVIEW) {
                        // calculate crop region
                        const MSize srcSize = MSize(fovInfo.mFOVCropRegion.s.w, fovInfo.mFOVCropRegion.s.h);
                        const MSize imgSize = pBuf->getImgSize();
                        const MSize dstSize = MSize(imgSize.w, imgSize.h);

                        MRect srcCrop;
                        // pillarbox
                        if ((srcSize.w*dstSize.h) > (srcSize.h*dstSize.w)) {
                            srcCrop.s.w = div_round(srcSize.h*dstSize.w, dstSize.h);
                            srcCrop.s.h = srcSize.h;
                            srcCrop.p.x = ((srcSize.w - srcCrop.s.w) >> 1);
                            srcCrop.p.y = 0;
                        }
                        // letterbox
                        else {
                            srcCrop.s.w = srcSize.w;
                            srcCrop.s.h = div_round(srcSize.w*dstSize.h, dstSize.w);
                            srcCrop.p.x = 0;
                            srcCrop.p.y = ((srcSize.h - srcCrop.s.h) >> 1);
                        }
                        // add offset
                        cropRegion.p.x = fovInfo.mFOVCropRegion.p.x + srcCrop.p.x;
                        cropRegion.p.y = fovInfo.mFOVCropRegion.p.y + srcCrop.p.y;
                        cropRegion.s = srcCrop.s;
                    }

                    MY_LOGD("VSDOF Sensor(%d) FOV Crop(%d,%d)(%dx%d) TYPE Id(%s)",
                                masterId,
                                fovInfo.mFOVCropRegion.p.x,
                                fovInfo.mFOVCropRegion.p.y,
                                fovInfo.mFOVCropRegion.s.w,
                                fovInfo.mFOVCropRegion.s.h,
                                TypeID2Name(typeId));
                }
            } else if (isCroppedFSYUV && (typeId == TID_MAN_FULL_YUV)) {
                needCrop = MTRUE;
                MSize imgSize = pBuf->getImgSize();
                mpCropCalculator->evaluate(pFactor, imgSize, cropRegion, MFALSE);
                MY_LOGD("CropedFSYUV Sensor(%d) FOV Crop (%d, %d, %d, %d), dstSize:(%d, %d)",
                    mSensorIndex,
                    cropRegion.p.x, cropRegion.p.y,
                    cropRegion.s.w, cropRegion.s.h,
                    imgSize.w, imgSize.h);
            } else {
                GetScaleCrop(cropRegion, scaleRatio);
            }

            auto SetOutput = [&](P2Output& o, MBOOL isMDPDMA=MFALSE) {
                hasOutput = MTRUE;
                o.mpBuf = pBuf;
                o.mBufId = bufId;
                o.mTypeId = typeId;
                o.mHasCrop = needCrop;
                if (isMDPDMA && curUsablePQSize>0 &&
                    (typeId == TID_JPEG || typeId == TID_MAN_CROP1_YUV
                     || typeId == TID_MAN_CROP2_YUV || typeId == TID_MAN_CROP3_YUV)) {
                    o.mEnableCZ = pRequest->hasFeature(FID_CZ);
                    o.mEnableHFG = pRequest->hasFeature(FID_HFG);
                    curUsablePQSize--;
                }
                o.mTrans = trans;
                o.mCropRegion = cropRegion;
                o.mScaleRatio = scaleRatio;
            };

            auto BeyondCapability = [&]() -> MBOOL {
                if (!mISP3_0)
                    return MFALSE;

                MUINT32 maxRatio= scaleRatio;
                MUINT32 minRatio = maxRatio;
                MINT32 maxOffset = cropRegion.p.x;
                MINT32 minOffset = maxOffset;

                MY_LOGD("ratio:%d offset:%d", scaleRatio, cropRegion.p.x);
                auto UpdateStatistics = [&](P2Output &o) {
                    if (o.mpBuf == NULL)
                        return;

                    if (o.mCropRegion.p.x > maxOffset)
                        maxOffset = o.mCropRegion.p.x;
                    if (o.mCropRegion.p.x < minOffset)
                        minOffset = o.mCropRegion.p.x;

                    MUINT32 oRatio = o.mScaleRatio;
                    if (oRatio > maxRatio)
                        maxRatio = oRatio;
                    if (oRatio < minRatio)
                        minRatio = oRatio;
                };

                UpdateStatistics(rEnqueData.mWDMAO);
                UpdateStatistics(rEnqueData.mWROTO);
                UpdateStatistics(rEnqueData.mIMG2O);

                // It's not acceptible if the offset difference is over 196
                if (maxOffset - minOffset > ISP30_RULE01_CROP_OFFSET)
                    return MTRUE;

                // To use HW resizer has the following limitation:
                // - Don't allow that downscale ratio is over 8 and offset is over 128.
                MBOOL hitRatio = maxRatio > minRatio * ISP30_RULE02_RESIZE_RATIO;
                MBOOL hitOffset = maxOffset - minOffset > ISP30_RULE02_CROP_OFFSET;
                if (hitRatio && hitOffset)
                    return MTRUE;

                // - The offset difference multiplied by the ratio differnce must be smaller than 1024
                if (hitRatio || hitOffset) {
                    if ((maxOffset - minOffset) * maxRatio >
                        minRatio * ISP30_RULE02_CROP_OFFSET * ISP30_RULE02_RESIZE_RATIO) {
                        return MTRUE;
                    }
                }

                return MFALSE;
            };

            auto AcceptedByIMG3O = [&]() -> MBOOL {
                const MSize& rDstSize = pBuf->getImgSize();
                const MSize& rSrcSize = rEnqueData.mIMGI.mpBuf->getImgSize();
                return rSrcSize == rDstSize;
            };

            MBOOL limited  = BeyondCapability();
            // Use P2 resizer to serve FD or thumbnail buffer,
            // but do NOT use IMG2O to crop or resize in ISP 3.0 while enque pure raw

            if (!limited && !UsedOutput(rEnqueData.mIMG2O) && (typeId == TID_MAN_FD_YUV || typeId == TID_THUMBNAIL) && hasP2Resizer && pBuf->getImgFormat() == eImgFmt_YUY2)
            {
                SetOutput(rEnqueData.mIMG2O);
            } else if (typeId == TID_MAN_FULL_YUV && bFullByImg3o && AcceptedByIMG3O() && trans == 0) {
                SetOutput(rEnqueData.mIMG3O);
            } else if (!limited && !UsedOutput(rEnqueData.mWDMAO) && trans == 0) {
                SetOutput(rEnqueData.mWDMAO, MTRUE);
            } else if (!limited && !UsedOutput(rEnqueData.mWROTO)) {
                SetOutput(rEnqueData.mWROTO, MTRUE);
            } else if (!UsedOutput(rEnqueData.mCopy1)) {
                SetOutput(rEnqueData.mCopy1);
                hasCopyBuffer = MTRUE;
            } else if (!UsedOutput(rEnqueData.mCopy2)) {
                SetOutput(rEnqueData.mCopy2);
                hasCopyBuffer = MTRUE;
            } else {
                MY_LOGE("the buffer is not served, type:%s", TypeID2Name(typeId));
            }
        }

        // Create a smaller intermediate buffer for memory copying
        if (mISP3_0 && hasCopyBuffer) {
            if (!UsedOutput(rEnqueData.mWDMAO) || !UsedOutput(rEnqueData.mWROTO)) {
                P2Output& rUnusedOutput = UsedOutput(rEnqueData.mWDMAO) ? rEnqueData.mWROTO : rEnqueData.mWDMAO;

                MSize tempSize = MSize(srcSize.w / ISP30_RULE02_RESIZE_RATIO,
                                       srcSize.h / ISP30_RULE02_RESIZE_RATIO);
                tempSize.w &= ~(0x1);
                tempSize.h &= ~(0x1);

                MY_LOGW("Create an intermediate buffer(%d,%d), resized ratio:%d",
                    tempSize.w, tempSize.h, ISP30_RULE02_RESIZE_RATIO);

                // Should hit the hw limitation here. Create a temp buffer to avoid full size processing
                android::sp<IIBuffer> pTempBuffer = mpBufferPool->getImageBuffer(
                                    tempSize, eImgFmt_YUY2);

                pHolder->mpBuffers.push_back(pTempBuffer);
                rUnusedOutput.mpBuf = pTempBuffer->getImageBufferPtr();
                rUnusedOutput.mBufId = 200; // Magic No for Temp Buffer
                rUnusedOutput.mHasCrop = MTRUE;
            }
        }

        if (hasOutput) {
            rEnqueData.mEnableMFB = pRequest->hasFeature(FID_MFNR) && (pRequest->getActiveFrameCount() > 1);
            rEnqueData.mEnableDRE = !rEnqueData.mEnableMFB && pRequest->hasFeature(FID_DRE) && (pRequest->isActiveFirstFrame());
            rEnqueData.mpIMetaDynamic = pIMetaDynamic;
            rEnqueData.mpIMetaApp = pIMetaApp;
            rEnqueData.mpIMetaHal = pIMetaHal;
            rEnqueData.mpOMetaApp = (isOutputMeta) ? pOMetaApp : NULL;
            rEnqueData.mpOMetaHal = (isOutputMeta) ? pOMetaHal : NULL;
            rEnqueData.mSensorId  = masterId;
            rEnqueData.mEnableVSDoF = isVsdof;
            rEnqueData.mUniqueKey = uniqueKey;
            rEnqueData.mRequestNo = requestNo;
            rEnqueData.mFrameNo   = frameNo;
            rEnqueData.mTaskId    = TASK_M1_IMGO;
            rEnqueData.mNodeId    = nodeId;
            rEnqueData.mTimeSharing = MTRUE;
            addDebugBuffer(pHolder, pEnqueData);
            EProfileMappingStages stage;
            // ISP5.0 DSDN has no FID, need to check explicitly
            if(mISP5_0 && isRunDSDN)
                stage = eStage_DSDN10_2ndRun;
            else if(mISP6_0 && isRunDSDN)
                stage = eStage_DSDN20_2ndRun;
            else
            {
                stage = (isYuvRep) ? eStage_Y2Y_Main :
                        (isRunDCE) ?  eStage_Y2YDCE_2ndRun : eStage_R2Y_Main;
            }
            rEnqueData.mIspProfile = fnQueryISPProfile(mappingKey, stage);

            ret = enqueISP(
                pRequest,
                pEnqueData);

            if (!ret) {
                MY_LOGE("enqueISP failed!");
                return MFALSE;
            }
        }
    }

    // 3. Full RAW of sub sensor
    if (slaveId >= 0) {
        BufferID_T uOBufId = pNodeReq->mapBufferID(TID_SUB_FULL_YUV, OUTPUT);

        if (uOBufId != NULL_BUFFER) {
            shared_ptr<P2EnqueData> pEnqueData = make_shared<P2EnqueData>();
            P2EnqueData& rEnqueData = *pEnqueData.get();

            rEnqueData.mpHolder  = pHolder;
            rEnqueData.mpSharedData  = pSharedData;
            rEnqueData.mIMGI.mBufId  = pNodeReq->mapBufferID(TID_SUB_FULL_RAW, INPUT);
            rEnqueData.mLCEI.mBufId  = pNodeReq->mapBufferID(TID_SUB_LCS, INPUT);
            rEnqueData.mWDMAO.mBufId = uOBufId;
            rEnqueData.mpIMetaApp = pIMetaApp;
            rEnqueData.mpIMetaHal = pIMetaHal2;
            rEnqueData.mSensorId  = slaveId;
            rEnqueData.mEnableVSDoF = isVsdof;
            rEnqueData.mUniqueKey = uniqueKey;
            rEnqueData.mRequestNo = requestNo;
            rEnqueData.mFrameNo   = frameNo;
            rEnqueData.mTaskId    = TASK_M2_IMGO;
            rEnqueData.mNodeId    = nodeId;
            rEnqueData.mTimeSharing = MTRUE;
            rEnqueData.mIspProfile = fnQueryISPProfile(mappingKey, eStage_R2Y_Sub);
            if (isVsdof) {
                FovCalculator::FovInfo fovInfo;
                if (mpFOVCalculator->getIsEnable() && mpFOVCalculator->getFovInfo(slaveId, fovInfo)) {
                    rEnqueData.mWDMAO.mHasCrop = isVsdof;
                    rEnqueData.mWDMAO.mCropRegion.p = fovInfo.mFOVCropRegion.p;
                    rEnqueData.mWDMAO.mCropRegion.s = fovInfo.mFOVCropRegion.s;
                    MY_LOGD("VSDOF Sensor(%d) FOV Crop(%d,%d)(%dx%d)",
                                slaveId,
                                fovInfo.mFOVCropRegion.p.x,
                                fovInfo.mFOVCropRegion.p.y,
                                fovInfo.mFOVCropRegion.s.w,
                                fovInfo.mFOVCropRegion.s.h);
                }
            }

            ret = enqueISP(
                pRequest,
                pEnqueData);

            if (!ret) {
                MY_LOGE("sub sensor: full yuv failed!");
                return MFALSE;
            }
        }
    }

    // 4. Resized RAW of sub sensor
    if (slaveId >= 0) {
        BufferID_T uOBufId = pNodeReq->mapBufferID(TID_SUB_RSZ_YUV, OUTPUT);

        if (uOBufId != NULL_BUFFER) {
            shared_ptr<P2EnqueData> pEnqueData = make_shared<P2EnqueData>();
            P2EnqueData& rEnqueData = *pEnqueData.get();

            rEnqueData.mpHolder  = pHolder;
            rEnqueData.mpSharedData  = pSharedData;
            rEnqueData.mIMGI.mBufId  = pNodeReq->mapBufferID(TID_SUB_RSZ_RAW, INPUT);
            rEnqueData.mWDMAO.mBufId = uOBufId;
            rEnqueData.mpIMetaApp = pIMetaApp;
            rEnqueData.mpIMetaHal = pIMetaHal2;
            rEnqueData.mSensorId  = slaveId;
            rEnqueData.mResized   = MTRUE;
            rEnqueData.mEnableVSDoF = isVsdof;
            rEnqueData.mUniqueKey = uniqueKey;
            rEnqueData.mRequestNo = requestNo;
            rEnqueData.mFrameNo   = frameNo;
            rEnqueData.mTaskId    = TASK_M2_RRZO;
            rEnqueData.mNodeId    = nodeId;
            rEnqueData.mTimeSharing = USEING_TIME_SHARING;
            rEnqueData.mIspProfile = fnQueryISPProfile(mappingKey, eStage_RRZ_R2Y_Sub);
            ret = enqueISP(
                pRequest,
                pEnqueData);

            if (!ret) {
                MY_LOGE("sub sensor: resized yuv failed!");
                return MFALSE;
            }
        }
    }

    MY_LOGI("(%d) -, R/F/S Num: %d/%d/%d", nodeId, requestNo, frameNo, masterId);
    CAM_TRACE_FMT_END();
    return MTRUE;
}


/*******************************************************************************
 *
 ********************************************************************************/
MBOOL P2ANode::copyBuffers(EnquePackage* pPackage)
{
    P2EnqueData& rData = *pPackage->mpEnqueData.get();
    MINT32 requestNo = rData.mRequestNo;
    MINT32 frameNo = rData.mFrameNo;
    MINT32 masterId = rData.mSensorId;
    NodeID_T nodeId = rData.mNodeId;
    CAM_TRACE_FMT_BEGIN("p2a:copy|r%df%ds%d", requestNo, frameNo, masterId);
    MY_LOGD("(%d) +, R/F/S Num: %d/%d/%d", nodeId, requestNo, frameNo, masterId);


    IImageBuffer* pSource1  = rData.mCopy1.mpSource;
    IImageBuffer* pSource2  = rData.mCopy2.mpSource;
    IImageBuffer* pCopy1    = rData.mCopy1.mpBuf;
    IImageBuffer* pCopy2    = rData.mCopy2.mpBuf;
    MUINT32 uTrans1         = rData.mCopy1.mSourceTrans;
    MUINT32 uTrans2         = rData.mCopy2.mSourceTrans;
    MRect& rCrop1           = rData.mCopy1.mSourceCrop;
    MRect& rCrop2           = rData.mCopy2.mSourceCrop;
    MBOOL hasCopy2          = pCopy2 != NULL;
    MBOOL hasSameSrc        = hasCopy2 ? pSource1 == pSource2 : MFALSE;

    if (pSource1 == NULL || pCopy1 == NULL) {
        MY_LOGE("should have source1 & copy1 buffer here. src:%p, dst:%p", pSource1, pCopy1);
        return MFALSE;
    }

    if (hasCopy2 && pSource2 == NULL) {
        MY_LOGE("should have source2 buffer here. src:%p", pSource1);
        return MFALSE;
    }

    String8 strCopyLog;

    auto InputLog = [&](const char* sPort, IImageBuffer* pBuf) mutable {
        strCopyLog += String8::format("Sensor(%d) Resized(%d) R/F Num: %d/%d, Copy: %s Fmt(0x%x) Size(%dx%d) VA/PA(%#" PRIxPTR "/%#" PRIxPTR ")",
            rData.mSensorId,
            rData.mResized,
            requestNo,
            frameNo,
            sPort,
            pBuf->getImgFormat(),
            pBuf->getImgSize().w, pBuf->getImgSize().h,
            pBuf->getBufVA(0),
            pBuf->getBufPA(0));
    };

    auto OutputLog = [&](const char* sPort, MDPOutput& rOut) mutable {
        strCopyLog += String8::format(", %s Rot(%d) Crop(%d,%d)(%dx%d) Size(%dx%d) VA/PA(%#" PRIxPTR "/%#" PRIxPTR ")",
            sPort, rOut.mSourceTrans,
            rOut.mSourceCrop.p.x, rOut.mSourceCrop.p.y,
            rOut.mSourceCrop.s.w, rOut.mSourceCrop.s.h,
            rOut.mpBuf->getImgSize().w, rOut.mpBuf->getImgSize().h,
            rOut.mpBuf->getBufVA(0), rOut.mpBuf->getBufPA(0));
    };

    InputLog("SRC1", pSource1);
    OutputLog("COPY1", rData.mCopy1);

    if (hasCopy2) {
        if (!hasSameSrc) {
            MY_LOGD("%s", strCopyLog.string());
            strCopyLog.clear();
            InputLog("SRC2", pSource2);
        }
        OutputLog("COPY2", rData.mCopy2);
    }
    MY_LOGD("%s", strCopyLog.string());

    // use IImageTransform to handle image cropping
    std::unique_ptr<IImageTransform, std::function<void(IImageTransform*)>> transform(
            IImageTransform::createInstance(PIPE_CLASS_TAG, pPackage->miSensorIdx),
            [](IImageTransform *p)
            {
                if (p)
                    p->destroyInstance();
            }
        );

    if (transform.get() == NULL) {
        MY_LOGE("IImageTransform is NULL, cannot generate output");
        return MFALSE;
    }

    MBOOL ret = MTRUE;
    transform->setDumpInfo(IImageTransform::DumpInfos(requestNo, frameNo, 0));
    struct timeval current;
    gettimeofday(&current, NULL);
    ret = transform->execute(
            pSource1,
            pCopy1,
            (hasSameSrc) ? pCopy2 : 0,
            rCrop1,
            (hasSameSrc) ? rCrop2 : 0,
            uTrans1,
            (hasSameSrc) ? uTrans2 : 0,
            3000,
            &current,
            pPackage->mbNeedToDump
        );

    if (!ret) {
        MY_LOGE("fail to do image transform: first round");
        return MFALSE;
    }

    if (hasCopy2 && !hasSameSrc) {
        ret = transform->execute(
                pSource2,
                pCopy2,
                0,
                rCrop2,
                0,
                uTrans2,
                0,
                3000,
                &current,
                pPackage->mbNeedToDump
            );

        if (!ret) {
            MY_LOGE("fail to do image transform: second round");
            return MFALSE;
        }
    }

    CAM_TRACE_FMT_END();
    MY_LOGD("(%d) -, R/F Num: %d/%d", nodeId, requestNo, frameNo);
    return MTRUE;
}

MVOID P2ANode::onRequestFinish(NodeID_T nodeId, RequestPtr& pRequest)
{
    MINT32 requestNo = pRequest->getRequestNo();
    MINT32 frameNo = pRequest->getFrameNo();
    MINT32 masterId = pRequest->getMainSensorIndex();
    MINT32 slaveId = pRequest->getSubSensorIndex();
    CAM_TRACE_FMT_BEGIN("p2a:finish|r%df%ds%d)", requestNo, frameNo, masterId);
    MY_LOGI("(%d), R/F/S Num: %d/%d/%d", nodeId, requestNo, frameNo, masterId);

    const MINT32 frameCount = pRequest->getActiveFrameCount();
    const MINT32 frameIndex = pRequest->getActiveFrameIndex();
    const MBOOL isUnderBSS = pRequest->isUnderBSS();
    const MBOOL isFirstFrame = pRequest->isActiveFirstFrame();

    if (pRequest->getParameter(PID_ENABLE_NEXT_CAPTURE) > 0)
    {
        if ((pRequest->hasParameter(PID_THUMBNAIL_TIMING) &&
             pRequest->getParameter(PID_THUMBNAIL_TIMING) == NSPipelinePlugin::eTiming_P2)
          || pRequest->hasParameter(PID_CSHOT_REQUEST))
        {
            if (nodeId == NID_P2B && (pRequest->hasFeature(FID_AINR) || pRequest->hasFeature(FID_AINR_YHDR)) && pRequest->isMainFrame())
            {
                if (isFirstFrame)
                {
                    if (pRequest->mpCallback != NULL) {
                        MY_LOGI("[AINR] Notify next capture at P2 ending(%d|%d, isbss:%d)", frameIndex, frameCount, isUnderBSS);
                        pRequest->mpCallback->onContinue(pRequest);
                    } else {
                        MY_LOGW("[AINR] have no request callback instance!");
                    }
                }
            }
            else if ((pRequest->isSingleFrame() && isFirstFrame) || pRequest->isActiveLastFrame())
            {
                if (pRequest->mpCallback != NULL) {
                    MY_LOGI("Notify next capture at P2 ending(%d|%d, isbss:%d)", frameIndex, frameCount, isUnderBSS);
                    pRequest->mpCallback->onContinue(pRequest);
                } else {
                    MY_LOGW("have no request callback instance!");
                }
            }
        }
    }

    // Keep the delay request while user assigns a delay postview
    if (pRequest->hasParameter(PID_THUMBNAIL_DELAY)) {
        MINT32 delay = pRequest->getParameter(PID_THUMBNAIL_DELAY);
        if (delay != 0) {
            // Assign to delay request, and increate postview refernce count to keep the handle
            if (isFirstFrame) {
                sp<CaptureFeatureNodeRequest> pNodeReq = pRequest->getNodeRequest(nodeId);
                BufferID_T uPostView = pNodeReq->mapBufferID(TID_POSTVIEW, OUTPUT);

                if (uPostView != NULL_BUFFER) {
                    mwpDelayRequest = pRequest;
                    pRequest->incBufferRef(uPostView);
                    MY_LOGD("Delay Release: Postview(%d|%d)", frameIndex, frameCount);
                }
            }

            // Decrease the reference count of delayed buffer
            if (frameIndex == delay || frameCount == frameIndex + 1) {
                RequestPtr r = mwpDelayRequest.promote();
                if (r != NULL) {
                    mwpDelayRequest = NULL;

                    sp<CaptureFeatureNodeRequest> pNodeReq = r->getNodeRequest(nodeId);
                    BufferID_T uPostView = pNodeReq->mapBufferID(TID_POSTVIEW, OUTPUT);

                    if (uPostView != NULL_BUFFER) {
                        r->decBufferRef(uPostView);
                        MY_LOGD("Release: Postview(%d|%d)", frameIndex, frameCount);
                    }
                } else {
                    MY_LOGW("Release: Postview(%d|%d). But request released earlier", frameIndex, frameCount);
                }

            }
        }
    }

    if (isFirstFrame && mpFOVCalculator->getIsEnable()) {
        updateFovCropRegion(pRequest, masterId, MID_MAN_IN_HAL, MID_MAN_OUT_APP_PHY);
        updateFovCropRegion(pRequest, slaveId, MID_SUB_IN_HAL, MID_SUB_OUT_APP_PHY);
    }

    pRequest->decNodeReference(nodeId);

    dispatch(pRequest, nodeId);

    CAM_TRACE_FMT_END();
}

MVOID P2ANode::updateFovCropRegion(RequestPtr& pRequest, MINT32 sensorIndex, MetadataID_T metaId, MetadataID_T outMetaId)
{
    const MINT32 requestNo = pRequest->getRequestNo();
    const MINT32 frameNo = pRequest->getFrameNo();
    sp<MetadataHandle> metaPtr = pRequest->getMetadata(metaId);
    sp<MetadataHandle> outMetaPtr = pRequest->getMetadata(outMetaId);
    IMetadata* pMetaPtr = (metaPtr != nullptr) ? metaPtr->native() : nullptr;
    IMetadata* pOutMetaPtr = (outMetaPtr != nullptr) ? outMetaPtr->native() : nullptr;
    if (pMetaPtr == nullptr)
    {
        MY_LOGW("failed to get meta, R/F/S Num: %d/%d/%d, metaId:%d", requestNo, frameNo, sensorIndex, metaId);
        return;
    }

    FovCalculator::FovInfo fovInfo;
    if (!mpFOVCalculator->getFovInfo(sensorIndex, fovInfo))
    {
        MY_LOGW("failed to get fovInfo, R/F Num: %d/%d, sensorIndex:%d", requestNo, frameNo, sensorIndex);
        return;
    }

    auto updateCropRegion = [&](IMetadata::IEntry& entry, IMetadata* meta)
    {
        entry.push_back(fovInfo.mFOVCropRegion.p.x, Type2Type<MINT32>());
        entry.push_back(fovInfo.mFOVCropRegion.p.y, Type2Type<MINT32>());
        entry.push_back(fovInfo.mFOVCropRegion.s.w, Type2Type<MINT32>());
        entry.push_back(fovInfo.mFOVCropRegion.s.h, Type2Type<MINT32>());
        meta->update(entry.tag(), entry);
    };

    IMetadata::IEntry fovCropRegionEntry(MTK_STEREO_FEATURE_FOV_CROP_REGION);
    updateCropRegion(fovCropRegionEntry, pMetaPtr);
    MY_LOGD("update fov crop region, R/F Num: %d/%d, sensorIndex:%d, metaId:%d, region:(%d, %d)x(%d, %d)",
        requestNo, frameNo,
        sensorIndex, metaId,
        fovInfo.mFOVCropRegion.p.x, fovInfo.mFOVCropRegion.p.y,
        fovInfo.mFOVCropRegion.s.w, fovInfo.mFOVCropRegion.s.h);

    if (pOutMetaPtr == nullptr)
    {
        MY_LOGW("failed to get out meta, R/F/S Num: %d/%d/%d, metaId:%d", requestNo, frameNo, sensorIndex, outMetaId);
        return;
    }
    IMetadata::IEntry outFovCropRegionEntry(MTK_MULTI_CAM_FOV_CROP_REGION);
    updateCropRegion(outFovCropRegionEntry, pOutMetaPtr);
    MY_LOGD("update fov crop region R/F Num: %d/%d, sensorIndex:%d, metaId:%d", requestNo, frameNo, sensorIndex, outMetaId);
}

MVOID P2ANode::updateSensorCropRegion(RequestPtr& pRequest, MRect cropRegion)
{
    const MINT32 requestNo = pRequest->getRequestNo();
    const MINT32 frameNo = pRequest->getFrameNo();
    const MINT32 sensorId = pRequest->getMainSensorIndex();
    MetadataID_T metaId = (pRequest->isPhysicalStream()) ? MID_MAN_OUT_APP_PHY : MID_MAN_OUT_APP;
    sp<MetadataHandle> metaPtr = pRequest->getMetadata(metaId);
    IMetadata* pMetaPtr = (metaPtr != nullptr) ? metaPtr->native() : nullptr;
    if (pMetaPtr == nullptr)
    {
        MY_LOGW("failed to get meta, R/F/S Num: %d/%d/%d, metaId:%d", requestNo, frameNo, sensorId, metaId);
        return;
    }

    auto updateCropRegion = [&](IMetadata::IEntry& entry, IMetadata* meta)
    {
        entry.push_back(cropRegion.p.x, Type2Type<MINT32>());
        entry.push_back(cropRegion.p.y, Type2Type<MINT32>());
        entry.push_back(cropRegion.s.w, Type2Type<MINT32>());
        entry.push_back(cropRegion.s.h, Type2Type<MINT32>());
        meta->update(entry.tag(), entry);
    };

    IMetadata::IEntry sensorCropRegionEntry(MTK_MULTI_CAM_SENSOR_CROP_REGION);
    updateCropRegion(sensorCropRegionEntry, pMetaPtr);
    MY_LOGD("update sensor crop region, R/F Num: %d/%d, sensorIndex:%d, metaId:%d, region:(%d, %d)x(%d, %d)",
        requestNo, frameNo,
        sensorId, metaId,
        cropRegion.p.x, cropRegion.p.y,
        cropRegion.s.w, cropRegion.s.h);
}

MVOID P2ANode::addDebugBuffer(
    std::shared_ptr<RequestHolder>& pHolder,
    std::shared_ptr<P2EnqueData>& pEnqueData)
{
    const MSize& rSize = pEnqueData->mIMGI.mpBuf->getImgSize();
    // Debug: Dump TIMGO only for ISP 6.0
    if (mISP6_0 && mDebugTimgo) {
        MUINT32 timgoFormat = eImgFmt_UNKNOWN;
        if (mDebugTimgoType == EDIPTimgoDump_AFTER_LTM) {

            MINT fmt = pEnqueData->mIMGI.mpBuf->getImgFormat();
            if ((fmt & eImgFmt_RAW_START) == eImgFmt_RAW_START) {
                timgoFormat = eImgFmt_BAYER10;
            }
        }
        if (mDebugTimgoType == EDIPTimgoDump_AFTER_GGM)
            timgoFormat = eImgFmt_RGB48;

        if (timgoFormat != eImgFmt_UNKNOWN) {
            android::sp<IIBuffer> pDebugBuffer = mpBufferPool->getImageBuffer(rSize, timgoFormat, MSize(4, 0));
            pHolder->mpBuffers.push_back(pDebugBuffer);
            pEnqueData->mTIMGO.mpBuf = pDebugBuffer->getImageBufferPtr();
        }
    }

    if (mISP3_0) {
        // Debug: Dump IMG2O only for ISP 3.0
        if (mDebugImg2o && pEnqueData->mIMG2O.mpBuf == NULL) {
            android::sp<IIBuffer> pDebugBuffer = mpBufferPool->getImageBuffer(rSize, eImgFmt_YUY2);
            pHolder->mpBuffers.push_back(pDebugBuffer);
            pEnqueData->mIMG2O.mpBuf = pDebugBuffer->getImageBufferPtr();
        }
    } else {
        // Debug: Dump IMG3O or ISP 4.0+
        if (mDebugImg3o && pEnqueData->mIMG3O.mpBuf == NULL) {
            MUINT32 img3oFormat = mISP6_0 ? eImgFmt_MTK_YUYV_Y210 : eImgFmt_YUY2;
            MUINT32 img3oAlign = mISP6_0 ? 16 : 4;
            android::sp<IIBuffer> pDebugBuffer = mpBufferPool->getImageBuffer(rSize, img3oFormat, MSize(img3oAlign, 0));
            pHolder->mpBuffers.push_back(pDebugBuffer);
            pEnqueData->mIMG3O.mpBuf = pDebugBuffer->getImageBufferPtr();
        }
    }
}

P2ANode::EnquePackage::~EnquePackage()
{
    if (mpPQParam != nullptr)
    {
        auto DeletePqParam = [] (void* p)
        {
            if (p != nullptr) {
                DpPqParam* pParam = static_cast<DpPqParam*>(p);
                MDPSetting* pSetting = pParam->u.isp.p_mdpSetting;
                if (pSetting != nullptr) {
                    if (pSetting->buffer != nullptr)
                        free(pSetting->buffer);
                    delete pSetting;
                }
                delete pParam;
            }
        };

        DeletePqParam(mpPQParam->WDMAPQParam);
        DeletePqParam(mpPQParam->WROTPQParam);
        delete mpPQParam;
        mpPQParam = NULL;
    }

    auto DeleteModuleInfo = [] (ModuleInfo* pModuleInfo) {
        if (pModuleInfo != nullptr) {
            if (pModuleInfo->moduleStruct != nullptr)
                delete static_cast<_SRZ_SIZE_INFO_*>(pModuleInfo->moduleStruct);
            delete pModuleInfo;
        }
    };

    DeleteModuleInfo(mpModuleSRZ4);
    DeleteModuleInfo(mpModuleSRZ3);
}

MERROR P2ANode::evaluate(NodeID_T nodeId, CaptureFeatureInferenceData& rInfer)
{
    auto& rSrcData = rInfer.getSharedSrcData();
    auto& rDstData = rInfer.getSharedDstData();
    auto& rFeatures = rInfer.getSharedFeatures();
    auto& rMetadatas = rInfer.getSharedMetadatas();

    // Debug
    if (mDebugDRE == OFF || !mSupportDRE)
        rInfer.clearFeature(FID_DRE);
    else if (mDebugDRE == ON)
        rInfer.markFeature(FID_DRE);

    if (mDebugCZ == OFF || !mSupportCZ)
        rInfer.clearFeature(FID_CZ);
    else if (mDebugCZ == ON)
        rInfer.markFeature(FID_CZ);

    if (mDebugHFG == OFF || !mSupportHFG)
        rInfer.clearFeature(FID_HFG);
    else if (mDebugHFG == ON)
        rInfer.markFeature(FID_HFG);

    MBOOL isP2A = nodeId == NID_P2A;
    MBOOL hasMain = MFALSE;
    MBOOL hasSub = MFALSE;

    const MUINT8 reqtIndex = rInfer.getRequestIndex();
    const MUINT8 reqCount = rInfer.getRequestCount();
    const MBOOL isMainFrame = rInfer.isMainFrame();
    const MBOOL isBSS = rInfer.isUnderBSS();
    const MBOOL hasFeatureAINR = (mISP6_0 && (rInfer.hasFeature(FID_AINR) ||rInfer.hasFeature(FID_AINR_YHDR)));
    const MINT32 masterId = rInfer.mSensorIndex;
    const MINT32 slaveId = rInfer.mSensorIndex2;
    const MBOOL isVsdof = isVSDoFMode(mUsageHint);

    if (mDebugDCE == OFF || !mSupportDCE)
        rInfer.clearFeature(FID_DCE);
    else if (mDebugDCE == ON)
        rInfer.markFeature(FID_DCE);

    MY_LOGD("RequestIndex:%d RequestCount:%d isMainFrame:%d isBSS:%d hasAINR:%d",
            reqtIndex, reqCount, isMainFrame, isBSS, hasFeatureAINR);

    auto getFSYUVSize = [](NodeID_T nodeId, CaptureFeatureInferenceData& data) -> MSize
    {
        MSize ret = data.getSize(TID_MAN_FULL_RAW);
        if (getIsSupportCroppedFSYUV(data))
        {
            if ((data.mCroppedYUVSize.w > 0) && (data.mCroppedYUVSize.h > 0)) {
                ret = data.mCroppedYUVSize;
                data.markSupportCroppedFSYUV(nodeId);
                MY_LOGD("markSupportCroppedFSYUV and get the FSYUVSize:(%d, %d)", ret.w, ret.h);
            }
        }
        return ret;
    };

    // Reprocessing
    if (isP2A && rInfer.hasType(TID_MAN_FULL_YUV))
    {
        auto& src_0 = rSrcData.editItemAt(rSrcData.add());
        src_0.mTypeId = TID_MAN_FULL_YUV;
        src_0.mSizeId = SID_FULL;

        auto& dst_0 = rDstData.editItemAt(rDstData.add());
        dst_0.mTypeId = TID_MAN_FULL_YUV;
        dst_0.mSizeId = SID_FULL;
        dst_0.mSize = rInfer.getSize(TID_MAN_FULL_YUV);
        dst_0.setFormat(eImgFmt_YV12);
        dst_0.setAllFmtSupport(MTRUE);

        auto& dst_1 = rDstData.editItemAt(rDstData.add());
        dst_1.mTypeId = TID_MAN_CROP1_YUV;
        dst_1.setAllFmtSupport(MTRUE);

        auto& dst_2 = rDstData.editItemAt(rDstData.add());
        dst_2.mTypeId = TID_MAN_CROP2_YUV;
        dst_2.setAllFmtSupport(MTRUE);

        auto& dst_3 = rDstData.editItemAt(rDstData.add());
        dst_3.mTypeId = TID_MAN_CROP3_YUV;
        dst_3.setAllFmtSupport(MTRUE);

        auto& dst_4 = rDstData.editItemAt(rDstData.add());
        dst_4.mTypeId = TID_JPEG;
        dst_4.setAllFmtSupport(MTRUE);

        auto& dst_5 = rDstData.editItemAt(rDstData.add());
        dst_5.mTypeId = TID_THUMBNAIL;
        dst_5.setAllFmtSupport(MTRUE);

        auto& dst_6 = rDstData.editItemAt(rDstData.add());
        dst_6.mTypeId = TID_MAN_FD_YUV;
        dst_6.setAllFmtSupport(MTRUE);

        auto& dst_7 = rDstData.editItemAt(rDstData.add());
        dst_7.mTypeId = TID_POSTVIEW;
        dst_7.setAllFmtSupport(MTRUE);

        hasMain = MTRUE;
    }
    else if (isP2A && rInfer.hasType(TID_MAN_FULL_RAW))
    {
        if (!hasFeatureAINR || isMainFrame) {
            auto& src_0 = rSrcData.editItemAt(rSrcData.add());
            src_0.mTypeId = TID_MAN_FULL_RAW;
            src_0.mSizeId = SID_FULL;

            auto& src_1 = rSrcData.editItemAt(rSrcData.add());
            src_1.mTypeId = TID_MAN_LCS;
            src_1.mSizeId = SID_ARBITRARY;

            auto& dst_0 = rDstData.editItemAt(rDstData.add());
            dst_0.mTypeId = TID_MAN_FULL_YUV;
            dst_0.mSizeId = SID_FULL;
            // Use 10bit as default foramt for ISP 6.0
            if (mISP6_0 && rInfer.hasFeature(FID_DRE))
            {
                if(mSupport10Bits)
                    dst_0.setFormat(eImgFmt_MTK_YUV_P010);
                else
                    dst_0.setFormat(eImgFmt_YV12);
                dst_0.addSupportFormat(eImgFmt_YV12);
                dst_0.setAllFmtSupport(MTRUE);
            }
            else
            {
                dst_0.setFormat(eImgFmt_YV12);
                dst_0.setAllFmtSupport(MTRUE);
            }

            if (isVsdof && mpFOVCalculator->getIsEnable())
            {
                FovCalculator::FovInfo fovInfo;
                if (mpFOVCalculator->getFovInfo(masterId, fovInfo)) {
                    dst_0.mSize = fovInfo.mDestinationSize;
                } else {
                    dst_0.mSize = rInfer.getSize(TID_MAN_FULL_RAW);
                }
            } else {
                dst_0.mSize = getFSYUVSize(nodeId, rInfer);
            }

            // should only output working buffer if run DRE
            if (!(mSupportDRE && rInfer.hasFeature(FID_DRE))) {
                auto& dst_1 = rDstData.editItemAt(rDstData.add());
                dst_1.mTypeId = TID_MAN_CROP1_YUV;
                dst_1.setAllFmtSupport(MTRUE);

                auto& dst_2 = rDstData.editItemAt(rDstData.add());
                dst_2.mTypeId = TID_MAN_CROP2_YUV;
                dst_2.setAllFmtSupport(MTRUE);

                auto& dst_3 = rDstData.editItemAt(rDstData.add());
                dst_3.mTypeId = TID_MAN_CROP3_YUV;
                dst_3.setAllFmtSupport(MTRUE);
            }

            auto& dst_4 = rDstData.editItemAt(rDstData.add());
            dst_4.mTypeId = TID_MAN_SPEC_YUV;
            dst_4.setAllFmtSupport(MTRUE);

            auto& dst_5 = rDstData.editItemAt(rDstData.add());
            dst_5.mTypeId = TID_JPEG;
            dst_5.setAllFmtSupport(MTRUE);

            auto& dst_6 = rDstData.editItemAt(rDstData.add());
            dst_6.mTypeId = TID_THUMBNAIL;
            dst_6.setAllFmtSupport(MTRUE);

            auto& dst_7 = rDstData.editItemAt(rDstData.add());
            dst_7.mTypeId = TID_MAN_FD_YUV;
            dst_7.setAllFmtSupport(MTRUE);

            {
                auto& dst_8 = rDstData.editItemAt(rDstData.add());
                dst_8.mTypeId = TID_POSTVIEW;
                dst_8.setAllFmtSupport(MTRUE);
            }

            if (slaveId >= 0) {
                auto& dst_9 = rDstData.editItemAt(rDstData.add());
                dst_9.mTypeId = TID_MAN_CLEAN;
                dst_9.setAllFmtSupport(MTRUE);
            }

            if (rInfer.hasRawOut() && mISP6_0) {
                auto& dst_10 = rDstData.editItemAt(rDstData.add());
                dst_10.mTypeId = TID_MAN_FULL_RAW;
                dst_10.mSizeId = SID_FULL;
                dst_10.mSize = rInfer.getSize(TID_MAN_FULL_RAW);
            }

            hasMain = MTRUE;
        }
        else {
            MY_LOGD("no need to require TID_MAN_FULL_RAW buffer, hasFeatureAINR:%d, reqIndex:%d, reqCount:%u, isMainFrame:%d isUnderBSS:%d",
                hasFeatureAINR, reqtIndex, reqCount, isMainFrame, isBSS);
        }
    }
    else if (!isP2A && rInfer.hasType(TID_MAN_FULL_RAW))
    {
        // p2b node generate pure yuv when need yuv processing
        MUINT8 needYuvProc = MTK_FEATURE_CAP_YUV_PROCESSING_NOT_NEEDED;
        if (tryGetMetadata<MUINT8>(rInfer.mpIMetadataHal.get(), MTK_FEATURE_CAP_YUV_PROCESSING, needYuvProc))
        {
            MY_LOGD("request(index/count:%d/%d) needs YUV processing! isBSS:%d isAINR:%d.",
                        reqtIndex, reqCount, isBSS, hasFeatureAINR);
        }

        if (!hasFeatureAINR && isMainFrame)
        {
            auto& src_0 = rSrcData.editItemAt(rSrcData.add());
            src_0.mTypeId = TID_MAN_FULL_RAW;
            src_0.mSizeId = SID_FULL;

            auto& src_1 = rSrcData.editItemAt(rSrcData.add());
            src_1.mTypeId = TID_MAN_LCS;
            src_1.mSizeId = SID_ARBITRARY;

            auto& dst_0 = rDstData.editItemAt(rDstData.add());
            dst_0.mTypeId = TID_MAN_FULL_PURE_YUV;
            dst_0.mSize = getFSYUVSize(nodeId, rInfer);
            if (isVsdof && mpFOVCalculator->getIsEnable())
            {
                FovCalculator::FovInfo fovInfo;
                if (mpFOVCalculator->getFovInfo(masterId, fovInfo)) {
                    dst_0.mSize = fovInfo.mDestinationSize;
                } else {
                    dst_0.mSize = rInfer.getSize(TID_MAN_FULL_RAW);
                }
            } else {
                dst_0.mSize = rInfer.getSize(TID_MAN_FULL_RAW);
            }
            dst_0.setFormat(eImgFmt_YV12);
            dst_0.setAllFmtSupport(MTRUE);
            hasMain = MTRUE;
        }
        else if(needYuvProc == MTK_FEATURE_CAP_YUV_PROCESSING_NEEDED && !isMainFrame)
        {
            auto& src_0 = rSrcData.editItemAt(rSrcData.add());
            src_0.mTypeId = TID_MAN_FULL_RAW;
            src_0.mSizeId = SID_FULL;

            auto& src_1 = rSrcData.editItemAt(rSrcData.add());
            src_1.mTypeId = TID_MAN_LCS;
            src_1.mSizeId = SID_ARBITRARY;

            auto& dst_0 = rDstData.editItemAt(rDstData.add());
            dst_0.mTypeId = TID_MAN_FULL_PURE_YUV;
            dst_0.mSizeId = SID_FULL;
            dst_0.setFormat(eImgFmt_YV12);
            dst_0.setAllFmtSupport(MTRUE);
            dst_0.mSize = rInfer.getSize(TID_MAN_FULL_RAW);

            hasMain = MTRUE;
        }
        else {
            MY_LOGD("no need to require TID_MAN_FULL_RAW buffer, hasFeatureAINR:%d, reqIndex:%d, reqCount:%u, isMainFrame:%d isBSS:%d",
                hasFeatureAINR, reqtIndex, reqCount, isMainFrame, isBSS);
        }
    }

    if (isP2A && rInfer.hasType(TID_SUB_FULL_RAW))
    {
        auto& src_0 = rSrcData.editItemAt(rSrcData.add());
        src_0.mTypeId = TID_SUB_FULL_RAW;
        src_0.mSizeId = SID_FULL;

        auto& src_1 = rSrcData.editItemAt(rSrcData.add());
        src_1.mTypeId = TID_SUB_LCS;
        src_1.mSizeId = SID_ARBITRARY;

        auto& dst_0 = rDstData.editItemAt(rDstData.add());
        dst_0.mTypeId = TID_SUB_FULL_YUV;
        dst_0.mSizeId = SID_FULL;
        dst_0.setFormat(eImgFmt_YV12);
        dst_0.setAllFmtSupport(MTRUE);

        if (isVsdof && mpFOVCalculator->getIsEnable())
        {
            FovCalculator::FovInfo fovInfo;
            if(mpFOVCalculator->getFovInfo(slaveId, fovInfo)) {
                dst_0.mSize = fovInfo.mDestinationSize;
            } else {
                dst_0.mSize = rInfer.getSize(TID_SUB_FULL_RAW);
            }
        } else {
            dst_0.mSize = rInfer.getSize(TID_SUB_FULL_RAW);
        }

        hasSub = MTRUE;
    }

    if (isP2A && rInfer.hasType(TID_MAN_RSZ_RAW))
    {
        if (!hasFeatureAINR || isMainFrame) {
            auto& src_0 = rSrcData.editItemAt(rSrcData.add());
            src_0.mTypeId = TID_MAN_RSZ_RAW;
            src_0.mSizeId = SID_RESIZED;

            auto& dst_0 = rDstData.editItemAt(rDstData.add());
            dst_0.mTypeId = TID_MAN_RSZ_YUV;
            dst_0.mSizeId = SID_RESIZED;
            dst_0.mSize = rInfer.getSize(TID_MAN_RSZ_RAW);
            dst_0.setFormat(eImgFmt_YV12);
            dst_0.setAllFmtSupport(MTRUE);
            hasMain = MTRUE;
        } else {
            MY_LOGD("no need to require TID_MAN_RSZ_RAW buffer, hasFeatureAINR:%d, reqIndex:%d, reqCount:%u, isMainFrame:%d isBSS:%d",
                hasFeatureAINR, reqtIndex, reqCount, isMainFrame, isBSS);
        }
    }

    if (isP2A && rInfer.hasType(TID_SUB_RSZ_RAW))
    {
        auto& src_0 = rSrcData.editItemAt(rSrcData.add());
        src_0.mTypeId = TID_SUB_RSZ_RAW;
        src_0.mSizeId = SID_RESIZED;

        auto& dst_0 = rDstData.editItemAt(rDstData.add());
        dst_0.mTypeId = TID_SUB_RSZ_YUV;
        dst_0.mSizeId = SID_RESIZED;
        dst_0.mSize = rInfer.getSize(TID_SUB_RSZ_RAW);
        dst_0.setFormat(eImgFmt_YV12);
        dst_0.setAllFmtSupport(MTRUE);

        hasSub = MTRUE;
    }

    if (hasMain) {
        rMetadatas.push_back(MID_MAN_IN_P1_DYNAMIC);
        rMetadatas.push_back(MID_MAN_IN_APP);
        rMetadatas.push_back(MID_MAN_IN_HAL);
        rMetadatas.push_back(MID_MAN_OUT_APP);
        rMetadatas.push_back(MID_MAN_OUT_HAL);
        rMetadatas.push_back(MID_MAN_OUT_APP_PHY);

        if (isP2A  && !rInfer.hasFeature(FID_DRE)&& !rInfer.hasFeature(FID_DRE)) {
            if (rInfer.hasFeature(FID_CZ))
                rFeatures.push_back(FID_CZ);
            if (rInfer.hasFeature(FID_HFG))
                rFeatures.push_back(FID_HFG);
        }

    }

    if (isP2A && hasSub) {
        rMetadatas.push_back(MID_SUB_IN_P1_DYNAMIC);
        rMetadatas.push_back(MID_SUB_IN_HAL);
        rMetadatas.push_back(MID_SUB_OUT_APP_PHY);
    }

    if(!rInfer.addNodeIO(nodeId, rSrcData, rDstData, rMetadatas, rFeatures))
        return BAD_VALUE;
    else
        return OK;
}

} // NSCapture
} // namespace NSFeaturePipe
} // namespace NSCamFeature
} // namespace NSCam

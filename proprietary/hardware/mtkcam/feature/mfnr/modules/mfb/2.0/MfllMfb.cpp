/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 *
 * MediaTek Inc. (C) 2017. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER ON
 * AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
 * NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
 * SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
 * SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES TO LOOK ONLY TO SUCH
 * THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. RECEIVER EXPRESSLY ACKNOWLEDGES
 * THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES
 * CONTAINED IN MEDIATEK SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK
 * SOFTWARE RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND
 * CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,
 * AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE,
 * OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY RECEIVER TO
 * MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek Software")
 * have been modified by MediaTek Inc. All revisions are subject to any receiver's
 * applicable license agreements with MediaTek Inc.
 */
#define LOG_TAG "MfllCore/Mfb"

#include "MfllCore.h"
#include "MfllMfb.h"
#include <MfllLog.h>
#include "MfllProperty.h"
#include "MfllFeatureDump.h"
#include "MfllIspProfiles.h"
#include "MfllOperationSync.h"

// MTKCAM
#include <mtkcam/utils/metadata/hal/mtk_platform_metadata_tag.h>
#include <mtkcam/drv/iopipe/SImager/IImageTransform.h> // IImageTransform
#include <mtkcam/drv/def/Dip_Notify_datatype.h> // _SRZ_SIZE_INFO_
#include <mtkcam/aaa/INvBufUtil.h> // new NVRAM mechanism
#include <mtkcam/utils/mapping_mgr/cam_idx_mgr.h> // new NVRAM mechanism
#include <mtkcam/drv/IHalSensor.h> // MAKE_HalSensorList

// MTKCAM-driver (platform dependent)
#include <mfb_reg.h> // new MFB register table, only supports ISP ver 5 (and upon)

// DpFramework
#include <DpDataType.h>

// AOSP
#include <utils/Mutex.h> // android::Mutex
#include <cutils/compiler.h> //

// STL
#include <memory>
#include <tuple>

//
// To print more debug information of this module
// #define __DEBUG

// Enque frame timeout
#define MFLL_MFB_ENQUE_TIMEOUT_MS 1500

#if (MTKCAM_HAVE_AEE_FEATURE == 1)
#include <aee.h>
#include <aaa_log.h>
#include <aaa_error_code.h>
#include <aaa_common_custom.h>
#define AEE_ASSERT_MFB(error_log) \
        do { \
            aee_system_exception( \
                LOG_TAG, \
                NULL, \
                DB_OPT_DEFAULT|DB_OPT_FTRACE, \
                error_log); \
            mfllLogF("%s: %s", __FUNCTION__, error_log); \
        } while(0)
#else // MTKCAM_HAVE_AEE_FEATURE != 1
#define AEE_ASSERT_MFB(error_log) \
        do { \
            mfllLogE("%s: %s", __FUNCTION__, error_log); \
            *(volatile uint32_t*)(0x00000000) = 0xdeadbeef; \
        } while(0)
#endif // MTKCAM_HAVE_AEE_FEATURE

//
// To workaround device hang of MFB stage
// #define WORKAROUND_MFB_STAGE

// helper macro to check if ISP profile mapping ok or not
#define IS_WRONG_ISP_PROFILE(p) (p == static_cast<EIspProfile_T>(MFLL_ISP_PROFILE_ERROR))

/* platform dependent headers, which needs defines in MfllMfb.cpp */
#include "MfllMfb_platform.h"
#include "MfllMfb_algo.h"

static const char * const sStrSetIsp = "SetIsp";
static const char * const sStrAfterSetIsp = "AfterSetIsp";
static const char * const sStrRefineReg = "RefineReg";
static const char * const sStrBeforeP2Enque = "BeforeP2Enque";
static const char * const sStrP2 = "P2";
static const char * const sStrMfb = "MFB";

using namespace NSCam::NSIoPipe::NSPostProc;
using namespace NSCam;
using namespace NSCam::Utils::Format;
using namespace NS3Av3;
using namespace mfll;
using namespace NSCam::NSIoPipe;

using android::sp;
using android::Mutex;
using NSCam::IImageBuffer;
using NSCam::NSIoPipe::EPortType_Memory;
using NSCam::NSIoPipe::NSSImager::IImageTransform;
using NSCam::NSIoPipe::Input;
using NSCam::NSIoPipe::Output;
using NSCam::NSIoPipe::QParams;
using NSCam::NSIoPipe::PortID;

/* global option to enable DRE or not */
static MINT32 gSupportDRE = ::property_get_int32("camera.mdp.dre.enable", 0);

static void dumpQParams(const QParams& rQParam, bool bForcedDump = false)
{
    if ( !bForcedDump ) {
        return;
    }

    mfllLogI("%s: Frame size = %d", __FUNCTION__, rQParam.mvFrameParams.size());
    for(size_t index = 0; index < rQParam.mvFrameParams.size(); ++index) {
        auto& frameParam = rQParam.mvFrameParams.itemAt(index);
        mfllLogI("%s: =================================================", __FUNCTION__);
        mfllLogI("%s: Frame index = %d", __FUNCTION__, index);
        mfllLogI("%s: mStreamTag=%d mSensorIdx=%d", __FUNCTION__, frameParam.mStreamTag, frameParam.mSensorIdx);
        mfllLogI("%s: FrameNo=%d RequestNo=%d Timestamp=%d", __FUNCTION__, frameParam.FrameNo, frameParam.RequestNo, frameParam.Timestamp);
        mfllLogI("%s: === mvIn section ===", __FUNCTION__);
        for ( size_t index2 = 0; index2 < frameParam.mvIn.size(); ++index2 ) {
            Input data = frameParam.mvIn[index2];
            mfllLogI("%s: Index = %d", __FUNCTION__, index2);
            mfllLogI("%s: mvIn.PortID.index = %d", __FUNCTION__, data.mPortID.index);
            mfllLogI("%s: mvIn.PortID.type = %d", __FUNCTION__, data.mPortID.type);
            mfllLogI("%s: mvIn.PortID.inout = %d", __FUNCTION__, data.mPortID.inout);

            mfllLogI("%s: mvIn.mBuffer=%x", __FUNCTION__, data.mBuffer);
            if ( data.mBuffer != NULL ) {
                mfllLogI("%s: mvIn.mBuffer->getImgSize = %dx%d", __FUNCTION__, data.mBuffer->getImgSize().w, data.mBuffer->getImgSize().h);

                mfllLogI("%s: mvIn.mBuffer->getImgFormat = %x", __FUNCTION__, data.mBuffer->getImgFormat());
                mfllLogI("%s: mvIn.mBuffer->getImageBufferHeap = %X", __FUNCTION__, data.mBuffer->getImageBufferHeap());
                mfllLogI("%s: mvIn.mBuffer->getPlaneCount = %d", __FUNCTION__, data.mBuffer->getPlaneCount());
                for(int j = 0; j < data.mBuffer->getPlaneCount(); j++ ) {
                    mfllLogI("%s: mvIn.mBuffer->getBufVA(%d) = %X", __FUNCTION__, j, data.mBuffer->getBufVA(j));
                    mfllLogI("%s: mvIn.mBuffer->getBufPA(%d) = %X", __FUNCTION__, j, data.mBuffer->getBufPA(j));
                    mfllLogI("%s: mvIn.mBuffer->getBufStridesInBytes(%d) = %d", __FUNCTION__, j, data.mBuffer->getBufStridesInBytes(j));
                }
            }
            else {
                mfllLogI("%s: mvIn.mBuffer is NULL!!", __FUNCTION__);
            }
            mfllLogI("%s: mvIn.mTransform = %d", __FUNCTION__, data.mTransform);
        }

        mfllLogI("%s: === mvOut section ===", __FUNCTION__);
        for ( size_t index2 = 0; index2 < frameParam.mvOut.size(); index2++ ) {
            Output data = frameParam.mvOut[index2];
            mfllLogI("%s: Index = %d", __FUNCTION__, index2);

            mfllLogI("%s: mvOut.PortID.index = %d", __FUNCTION__, data.mPortID.index);
            mfllLogI("%s: mvOut.PortID.type = %d", __FUNCTION__, data.mPortID.type);
            mfllLogI("%s: mvOut.PortID.inout = %d", __FUNCTION__, data.mPortID.inout);

            mfllLogI("%s: mvOut.mBuffer=%x", __FUNCTION__, data.mBuffer);
            if( data.mBuffer != NULL ) {
                mfllLogI("%s: mvOut.mBuffer->getImgSize = %dx%d", __FUNCTION__, data.mBuffer->getImgSize().w, data.mBuffer->getImgSize().h);
                mfllLogI("%s: mvOut.mBuffer->getImgFormat = %x", __FUNCTION__, data.mBuffer->getImgFormat());
                mfllLogI("%s: mvOut.mBuffer->getImageBufferHeap = %X", __FUNCTION__, data.mBuffer->getImageBufferHeap());
                mfllLogI("%s: mvOut.mBuffer->getPlaneCount = %d", __FUNCTION__, data.mBuffer->getPlaneCount());
                for ( size_t j = 0; j < data.mBuffer->getPlaneCount(); j++ ) {
                    mfllLogI("%s: mvOut.mBuffer->getBufVA(%d) = %X", __FUNCTION__, j, data.mBuffer->getBufVA(j));
                    mfllLogI("%s: mvOut.mBuffer->getBufPA(%d) = %X", __FUNCTION__, j, data.mBuffer->getBufPA(j));
                    mfllLogI("%s: mvOut.mBuffer->getBufStridesInBytes(%d) = %d", __FUNCTION__, j, data.mBuffer->getBufStridesInBytes(j));
                }
            }
            else {
                mfllLogI("%s: mvOut.mBuffer is NULL!!", __FUNCTION__);
            }
            mfllLogI("%s: mvOut.mTransform = %d", __FUNCTION__, data.mTransform);
        }

        mfllLogI("%s: === mvCropRsInfo section ===", __FUNCTION__);
        for ( size_t i = 0; i < frameParam.mvCropRsInfo.size(); i++ ) {
            MCrpRsInfo data = frameParam.mvCropRsInfo[i];
            mfllLogI("%s: Index = %d", __FUNCTION__, i);
            mfllLogI("%s: CropRsInfo.mGroupID=%d", __FUNCTION__, data.mGroupID);
            mfllLogI("%s: CropRsInfo.mMdpGroup=%d", __FUNCTION__, data.mMdpGroup);
            mfllLogI("%s: CropRsInfo.mResizeDst=%dx%d", __FUNCTION__, data.mResizeDst.w, data.mResizeDst.h);
            mfllLogI("%s: CropRsInfo.mCropRect.p_fractional=(%d,%d)", __FUNCTION__, data.mCropRect.p_fractional.x, data.mCropRect.p_fractional.y);
            mfllLogI("%s: CropRsInfo.mCropRect.p_integral=(%d,%d)", __FUNCTION__, data.mCropRect.p_integral.x, data.mCropRect.p_integral.y);
            mfllLogI("%s: CropRsInfo.mCropRect.s=%dx%d ", __FUNCTION__, data.mCropRect.s.w, data.mCropRect.s.h);
        }

        mfllLogI("%s: === mvModuleData section ===", __FUNCTION__);
        for( size_t i = 0; i < frameParam.mvModuleData.size(); i++ ) {
            ModuleInfo data = frameParam.mvModuleData[i];
            mfllLogI("%s: Index = %d", __FUNCTION__, i);
            mfllLogI("%s: ModuleData.moduleTag=%d", __FUNCTION__, data.moduleTag);

            _SRZ_SIZE_INFO_ *SrzInfo = (_SRZ_SIZE_INFO_ *) data.moduleStruct;
            mfllLogI("%s: SrzInfo->in_w=%d", __FUNCTION__, SrzInfo->in_w);
            mfllLogI("%s: SrzInfo->in_h=%d", __FUNCTION__, SrzInfo->in_h);
            mfllLogI("%s: SrzInfo->crop_w=%d", __FUNCTION__, SrzInfo->crop_w);
            mfllLogI("%s: SrzInfo->crop_h=%d", __FUNCTION__, SrzInfo->crop_h);
            mfllLogI("%s: SrzInfo->crop_x=%d", __FUNCTION__, SrzInfo->crop_x);
            mfllLogI("%s: SrzInfo->crop_y=%d", __FUNCTION__, SrzInfo->crop_y);
            mfllLogI("%s: SrzInfo->crop_floatX=%d", __FUNCTION__, SrzInfo->crop_floatX);
            mfllLogI("%s: SrzInfo->crop_floatY=%d", __FUNCTION__, SrzInfo->crop_floatY);
            mfllLogI("%s: SrzInfo->out_w=%d", __FUNCTION__, SrzInfo->out_w);
            mfllLogI("%s: SrzInfo->out_h=%d", __FUNCTION__, SrzInfo->out_h);
        }
        mfllLogI("%s: TuningData=%x", __FUNCTION__, frameParam.mTuningData);
        mfllLogI("%s: === mvExtraData section ===", __FUNCTION__);
        for ( size_t i = 0; i < frameParam.mvExtraParam.size(); i++ ) {
            auto extraParam = frameParam.mvExtraParam[i];
            if ( extraParam.CmdIdx == EPIPE_FE_INFO_CMD ) {
                FEInfo *feInfo = (FEInfo*) extraParam.moduleStruct;
                mfllLogI("%s: mFEDSCR_SBIT=%d  mFETH_C=%d  mFETH_G=%d", __FUNCTION__, feInfo->mFEDSCR_SBIT, feInfo->mFETH_C, feInfo->mFETH_G);
                mfllLogI("%s: mFEFLT_EN=%d  mFEPARAM=%d  mFEMODE=%d", __FUNCTION__, feInfo->mFEFLT_EN, feInfo->mFEPARAM, feInfo->mFEMODE);
                mfllLogI("%s: mFEYIDX=%d  mFEXIDX=%d  mFESTART_X=%d", __FUNCTION__, feInfo->mFEYIDX, feInfo->mFEXIDX, feInfo->mFESTART_X);
                mfllLogI("%s: mFESTART_Y=%d  mFEIN_HT=%d  mFEIN_WD=%d", __FUNCTION__, feInfo->mFESTART_Y, feInfo->mFEIN_HT, feInfo->mFEIN_WD);

            }
            else if ( extraParam.CmdIdx == EPIPE_FM_INFO_CMD ) {
                FMInfo *fmInfo = (FMInfo*) extraParam.moduleStruct;
                mfllLogI("%s: mFMHEIGHT=%d  mFMWIDTH=%d  mFMSR_TYPE=%d", __FUNCTION__, fmInfo->mFMHEIGHT, fmInfo->mFMWIDTH, fmInfo->mFMSR_TYPE);
                mfllLogI("%s: mFMOFFSET_X=%d  mFMOFFSET_Y=%d  mFMRES_TH=%d", __FUNCTION__, fmInfo->mFMOFFSET_X, fmInfo->mFMOFFSET_Y, fmInfo->mFMRES_TH);
                mfllLogI("%s: mFMSAD_TH=%d  mFMMIN_RATIO=%d", __FUNCTION__, fmInfo->mFMSAD_TH, fmInfo->mFMMIN_RATIO);
            }
            else if( extraParam.CmdIdx == EPIPE_MDP_PQPARAM_CMD ) {
                PQParam* param = reinterpret_cast<PQParam*>(extraParam.moduleStruct);
                if ( param->WDMAPQParam != nullptr ) {
                    DpPqParam* dpPqParam = (DpPqParam*)param->WDMAPQParam;
                    DpIspParam& ispParam = dpPqParam->u.isp;
                    VSDOFParam& vsdofParam = dpPqParam->u.isp.vsdofParam;
                    mfllLogI("%s: WDMAPQParam %x enable = %d, scenario=%d", __FUNCTION__, dpPqParam, dpPqParam->enable, dpPqParam->scenario);
                    mfllLogI("%s: WDMAPQParam iso = %d, frameNo=%d requestNo=%d", __FUNCTION__, ispParam.iso , ispParam.frameNo, ispParam.requestNo);
                    mfllLogI("%s: WDMAPQParam lensId = %d, isRefocus=%d defaultUpTable=%d",
                             __FUNCTION__, ispParam.lensId , vsdofParam.isRefocus, vsdofParam.defaultUpTable);
                    mfllLogI("%s: WDMAPQParam defaultDownTable = %d, IBSEGain=%d", __FUNCTION__, vsdofParam.defaultDownTable, vsdofParam.IBSEGain);
                }
                if( param->WROTPQParam != nullptr ) {
                    DpPqParam* dpPqParam = (DpPqParam*)param->WROTPQParam;
                    DpIspParam&ispParam = dpPqParam->u.isp;
                    VSDOFParam& vsdofParam = dpPqParam->u.isp.vsdofParam;
                    mfllLogI("%s: WROTPQParam %x enable = %d, scenario=%d", __FUNCTION__, dpPqParam, dpPqParam->enable, dpPqParam->scenario);
                    mfllLogI("%s: WROTPQParam iso = %d, frameNo=%d requestNo=%d", __FUNCTION__, ispParam.iso , ispParam.frameNo, ispParam.requestNo);
                    mfllLogI("%s: WROTPQParam lensId = %d, isRefocus=%d defaultUpTable=%d",
                             __FUNCTION__, ispParam.lensId , vsdofParam.isRefocus, vsdofParam.defaultUpTable);
                    mfllLogI("%s: WROTPQParam defaultDownTable = %d, IBSEGain=%d", __FUNCTION__, vsdofParam.defaultDownTable, vsdofParam.IBSEGain);
                }
            }
        }
    }
}

/* Make sure pass 2 is thread-safe, basically it's not ... (maybe) */
static Mutex gMutexPass2Lock;

/* P2 callback Cookie typedef */
namespace MfllMfb_Imp {

    constexpr const uint32_t COOKIE_CHECKSUM1 = 0xC00C1E01;
    constexpr const uint32_t COOKIE_CHECKSUM2 = 0xC00C1E02;
    constexpr const uint32_t COOKIE_CHECKSUM3 = 0xC00C1E03;
    constexpr const uint32_t COOKIE_CHECKSUM4 = 0xC00C1E04;

    struct P2Cookie
    {
        // we do not use atomic or volatile to sepcify checksum because we want to
        // check visible side-effect problem too.
        uint32_t                    __checksum1;
        std::mutex                  __mtx;
        uint32_t                    __checksum2;
        std::condition_variable     __cv;
        uint32_t                    __checksum3;
        std::atomic<int>            __signaled;
        uint32_t                    __checksum4;
        //
        P2Cookie()
        {
            __checksum1 = COOKIE_CHECKSUM1;
            __checksum2 = COOKIE_CHECKSUM2;
            __checksum3 = COOKIE_CHECKSUM3;
            __checksum4 = COOKIE_CHECKSUM4;
            __signaled.store(0, std::memory_order_seq_cst);
        };
    };


    inline bool isValidCookie(const P2Cookie& c)
    {
        if (__builtin_expect( c.__checksum1 != COOKIE_CHECKSUM1, false)) {
            mfllLogE("checksum doesn't match, expected,real=%#x, %#x",
                    COOKIE_CHECKSUM1, c.__checksum1);
            return false;
        }
        if (__builtin_expect( c.__checksum2 != COOKIE_CHECKSUM2, false)) {
            mfllLogE("checksum doesn't match, expected,real=%#x, %#x",
                    COOKIE_CHECKSUM2, c.__checksum2);
            return false;
        }
        if (__builtin_expect( c.__checksum3 != COOKIE_CHECKSUM3, false)) {
            mfllLogE("checksum doesn't match, expected,real=%#x, %#x",
                    COOKIE_CHECKSUM3, c.__checksum3);
            return false;
        }
        if (__builtin_expect( c.__checksum4 != COOKIE_CHECKSUM4, false)) {
            mfllLogE("checksum doesn't match, expected,real=%#x, %#x",
                    COOKIE_CHECKSUM4, c.__checksum4);
            return false;
        }
        return true;
    }

};


template <class T>
static inline MVOID __P2Cb(T& rParams)
{
    mfllLogI("mpfnCallback [+]");
    std::atomic_thread_fence(std::memory_order_acquire); // LoadLoad

    using namespace MfllMfb_Imp;
    P2Cookie* p = static_cast<P2Cookie*>(rParams.mpCookie);
    if (__builtin_expect( p == nullptr, false )) {
        mfllLogE("%s: P2Cookie is nullptr", __FUNCTION__);
        *(volatile uint32_t*)(0x00000000) = 0xDEADBEEF;
        mfllLogI("mpfnCallback [-]");
        return;
    }

    mfllLogI("%s:mpCookie=%p", __FUNCTION__, rParams.mpCookie);

    // check if the P2Cookie is valid or not.
    if (__builtin_expect( !isValidCookie(*p), false )) {
        mfllLogE("%s: P2Cookie is invalid", __FUNCTION__);
        *(volatile uint32_t*)(0x00000000) = 0xDEADBEEF;
        return;
    }

    // P2Cookie is valid, but still may visible side-effect since we cannot
    // synchronize rParams.
    size_t triedCnt = 0;
    do {
        bool locked = p->__mtx.try_lock();
        if (locked == false) { // lock failed
            if (triedCnt >= 10) { // try 10 times
                mfllLogE("%s: try to lock P2Cookie's mutex failed, "\
                        "ignore lock and notify condition_variable",
                        __FUNCTION__);
            }
            else {
                mfllLogW("%s: acquire lock failed, may have performance issue, "\
                         "try again (cnt=%zu)", __FUNCTION__, triedCnt);
                triedCnt++;
                // wait a while and try again (10ms).
                std::this_thread::sleep_for( std::chrono::milliseconds(10) );
                continue; // continue loop
            }
        }

        p->__signaled.store(1, std::memory_order_seq_cst); // avoid visible side-effect
        p->__cv.notify_all();


        // unlock mutex if necessary
        if (__builtin_expect( locked, true )) {
            p->__mtx.unlock();
        }
        break; // break loop
    } while(true);

    mfllLogD3("P2 callback signaled");
    mfllLogI("mpfnCallback [-]");
}

template <class T>
static inline MVOID __P2FailCb(T& rParams)
{
    using namespace MfllMfb_Imp;
    P2Cookie* p = static_cast<P2Cookie*>(rParams.mpCookie);
    if (__builtin_expect( p == nullptr, false )) {
        mfllLogE("%s: P2Cookie is nullptr", __FUNCTION__);
        *(volatile uint32_t*)(0x00000000) = 0xDEADBEEF;
        return;
    }

    dumpQParams(rParams, true);
    AEE_ASSERT_MFB("FATAL: invalid QParams, P2 failed callback is triggered!");
}


static inline void updateP2RevisedTag(IImageBuffer* pSrc, IMetadata* pMetadata)
{
    MINT32 resolution = pSrc->getImgSize().w | (pSrc->getImgSize().h << 16);
    IMetadata::setEntry<MINT32>(
            pMetadata,
            MTK_ISP_P2_IN_IMG_RES_REVISED,
            resolution
            );

    mfllLogD3("P2 revised img size=%dx%d",
            resolution & 0x0000FFFF,
            resolution >> 16
            );
}
static inline void removeP2RevisedTag(IMetadata* pMetadata)
{
    pMetadata->remove(MTK_ISP_P2_IN_IMG_RES_REVISED);
}


static MRect calCrop(MRect const &rSrc, MRect const &rDst, uint32_t /* ratio */)
{
    #define ROUND_TO_2X(x) ((x) & (~0x1))

    MRect rCrop;

    // srcW/srcH < dstW/dstH
    if (rSrc.s.w * rDst.s.h < rDst.s.w * rSrc.s.h)
    {
        rCrop.s.w = rSrc.s.w;
        rCrop.s.h = rSrc.s.w * rDst.s.h / rDst.s.w;
    }
    // srcW/srcH > dstW/dstH
    else if (rSrc.s.w * rDst.s.h > rDst.s.w * rSrc.s.h)
    {
        rCrop.s.w = rSrc.s.h * rDst.s.w / rDst.s.h;
        rCrop.s.h = rSrc.s.h;
    }
    // srcW/srcH == dstW/dstH
    else
    {
        rCrop.s.w = rSrc.s.w;
        rCrop.s.h = rSrc.s.h;
    }

    rCrop.s.w =  ROUND_TO_2X(rCrop.s.w);
    rCrop.s.h =  ROUND_TO_2X(rCrop.s.h);

    rCrop.p.x = (rSrc.s.w - rCrop.s.w) / 2;
    rCrop.p.y = (rSrc.s.h - rCrop.s.h) / 2;

    rCrop.p.x += ROUND_TO_2X(rSrc.p.x);
    rCrop.p.y += ROUND_TO_2X(rSrc.p.y);

    #undef ROUND_TO_2X
    return rCrop;
}


static DpPqParam generateMdpDumpPQParam(
        const IMetadata* pMetadata
        )
{
    DpPqParam pq;
    pq.enable   = false; // we dont need pq here.
    pq.scenario = MEDIA_ISP_CAPTURE;

    // dump info
    MINT32 uniqueKey = 0;
    MINT32 frameNum  = 0;
    MINT32 requestNo = 0;

    if (!IMetadata::getEntry(pMetadata, MTK_PIPELINE_UNIQUE_KEY, uniqueKey)) {
        mfllLogW("get MTK_PIPELINE_UNIQUE_KEY failed, set to 0");
    }
    if (!IMetadata::getEntry(pMetadata, MTK_PIPELINE_FRAME_NUMBER, frameNum)) {
        mfllLogW("get MTK_PIPELINE_FRAME_NUMBER failed, set to 0");
    }
    if (!IMetadata::getEntry(pMetadata, MTK_PIPELINE_REQUEST_NUMBER, requestNo)) {
        mfllLogW("get MTK_PIPELINE_REQUEST_NUMBER failed, set to 0");
    }

    pq.u.isp.timestamp = static_cast<uint32_t>(uniqueKey);
    pq.u.isp.frameNo   = static_cast<uint32_t>(frameNum);
    pq.u.isp.requestNo = static_cast<uint32_t>(requestNo);

    return pq;
}


#if MTK_CAM_NEW_NVRAM_SUPPORT
// {{{
template <NSIspTuning::EModule_T module>
inline void* _getNVRAMBuf(NVRAM_CAMERA_FEATURE_STRUCT* /*pNvram*/, size_t /*idx*/)
{
    mfllLogE("_getNVRAMBuf: unsupport module(%d)", module);
    *(volatile uint32_t*)(0x00000000) = 0xDEADC0DE;
    return nullptr;
}

template<>
inline void* _getNVRAMBuf<NSIspTuning::EModule_CA_LTM>(
        NVRAM_CAMERA_FEATURE_STRUCT*    pNvram,
        size_t                          idx
        )
{
    return &(pNvram->CA_LTM[idx]);
}

// magicNo is from HAL metadata of MTK_P1NODE_PROCESSOR_MAGICNUM <MINT32>
template<NSIspTuning::EModule_T module>
inline std::tuple<void*, int>
getTuningFromNvram(MUINT32 openId, MUINT32 idx, MINT32 magicNo, MINT32 ispProfile = -1)
{
    mfllTraceCall();

    NVRAM_CAMERA_FEATURE_STRUCT *pNvram;
    void* pNRNvram              = nullptr;
    std::tuple<void*, int>      emptyVal(nullptr, -1);

    if (__builtin_expect( idx >= EISO_NUM, false )) {
        mfllLogE("wrong nvram idx %d", idx);
        return emptyVal;
    }

    // load some setting from nvram
    MUINT sensorDev = MAKE_HalSensorList()->querySensorDevIdx(openId);
    IdxMgr* pMgr = IdxMgr::createInstance(static_cast<ESensorDev_T>(sensorDev));
    CAM_IDX_QRY_COMB rMapping_Info;

    // query mapping info
    mfllLogD3("getMappingInfo with magicNo(%d)", magicNo);
    pMgr->getMappingInfo(static_cast<ESensorDev_T>(sensorDev), rMapping_Info, magicNo);

    // tricky: force set ISP profile since new NVRAM SW limitation
    if (ispProfile >= 0) {
        rMapping_Info.eIspProfile = static_cast<EIspProfile_T>( ispProfile );
        pMgr->setMappingInfo(static_cast<ESensorDev_T>(sensorDev), rMapping_Info, magicNo);
        mfllLogD3("%s: force set ISP profile to %#x", __FUNCTION__, ispProfile);
    }

    // query NVRAM index by mapping info
    idx = pMgr->query(static_cast<ESensorDev_T>(sensorDev), module, rMapping_Info, __FUNCTION__);
    mfllLogD3("query nvram DRE mappingInfo index: %d", idx);

    auto pNvBufUtil = MAKE_NvBufUtil();
    if (__builtin_expect( pNvBufUtil == NULL, false )) {
        mfllLogE("pNvBufUtil==0");
        return emptyVal;
    }

    auto result = pNvBufUtil->getBufAndRead(
        CAMERA_NVRAM_DATA_FEATURE,
        sensorDev, (void*&)pNvram);
    if (__builtin_expect( result != 0, false )) {
        mfllLogE("read buffer chunk fail");
        return emptyVal;
    }

    pNRNvram = _getNVRAMBuf<module>(pNvram, static_cast<size_t>(idx));
    return std::make_tuple( pNRNvram,  idx );
}
// }}}
#endif


static bool generateMdpDrePQParam(
        DpPqParam&          rMdpPqParam,
        int                 iso,
        int                 openId,
        const IMetadata*    pMetadata,
        EIspProfile_T       ispProfile
        )
{
    // dump info
    MINT32 uniqueKey = 0;
    MINT32 frameNum  = 0;
    MINT32 requestNo = 0;
    MINT32 magicNum  = 0;
    MINT32 lv_value  = 0;

    if (!IMetadata::getEntry(pMetadata, MTK_PIPELINE_UNIQUE_KEY, uniqueKey)) {
        mfllLogW("get MTK_PIPELINE_UNIQUE_KEY failed, set to 0");
    }
    if (!IMetadata::getEntry(pMetadata, MTK_PIPELINE_FRAME_NUMBER, frameNum)) {
        mfllLogW("get MTK_PIPELINE_FRAME_NUMBER failed, set to 0");
    }
    if (!IMetadata::getEntry(pMetadata, MTK_PIPELINE_REQUEST_NUMBER, requestNo)) {
        mfllLogW("get MTK_PIPELINE_REQUEST_NUMBER failed, set to 0");
    }
    if (!IMetadata::getEntry(pMetadata, MTK_REAL_LV, lv_value)) {
        mfllLogW("get MTK_REAL_LV failed, set to 0");
    }
    if (!IMetadata::getEntry(pMetadata, MTK_P1NODE_PROCESSOR_MAGICNUM, magicNum)) {
        mfllLogW("get MTK_P1NODE_PROCESSOR_MAGICNUM failed, cannot generate DRE's tuning "\
                "data, won't apply");
        return false;
    }

    /* check ISO */
    if (__builtin_expect( iso <= 0, false )) {
        mfllLogE("ISO information is wrong, set to 0");
        iso = 0;
    }

    /* PQ param */
    rMdpPqParam.enable     |= PQ_DRE_EN;
    rMdpPqParam.scenario    = MEDIA_ISP_CAPTURE;

    /* ISP param */
    DpIspParam  &ispParam = rMdpPqParam.u.isp;
    //
    ispParam.timestamp  = uniqueKey;
    ispParam.frameNo    = frameNum;
    ispParam.requestNo  = 1000; // use 1000 as MDP generating histogram
    ispParam.iso        = static_cast<uint32_t>(iso);
    ispParam.lensId     = static_cast<uint32_t>(openId);
    ispParam.LV         = static_cast<int>(lv_value);

    /* DRE param */
    DpDREParam &dreParam = rMdpPqParam.u.isp.dpDREParam;
    //
    dreParam.cmd        = DpDREParam::Cmd::Initialize | DpDREParam::Cmd::Generate;
    dreParam.userId     = static_cast<unsigned long long>(uniqueKey);
    dreParam.buffer     = nullptr; // no need buffer

    /* idx0: void*, buffer address, idx1: int, NVRAM index */
    const size_t I_BUFFER = 0, I_NVRAMIDX = 1;
    std::tuple<void*, int> nvramData = getTuningFromNvram<NSIspTuning::EModule_CA_LTM>(
            openId,
            0, // no need index
            magicNum,
            static_cast<MINT32>( ispProfile )
            );

    dreParam.p_customSetting = std::get<I_BUFFER>   (nvramData);
    dreParam.customIndex     = std::get<I_NVRAMIDX> (nvramData);

    mfllLogD3("generate DREParam with userId: %d, frmNum: %d, iso: %d, sensorId: %d, magicNum: %d",
            uniqueKey, frameNum, iso, openId, magicNum);
    return true;
}


static bool attachMDPPQParam2Pass2drv(
        PortID          portId,
        FrameParams*    pFrameParams,
        PQParam*        pP2Pq,
        DpPqParam*      pMdpPQ
        )
{
    if (portId == PORT_WDMAO) {
        pP2Pq->WDMAPQParam = static_cast<void*>(pMdpPQ);
    }
    else if (portId == PORT_WROTO) {
        pP2Pq->WROTPQParam = static_cast<void*>(pMdpPQ);
    }
    else {
        mfllLogD("%s: attach failed, the port is neither WDMAO nor WROTO", __FUNCTION__);
        return false;
    }

    ExtraParam extraP;
    extraP.CmdIdx = EPIPE_MDP_PQPARAM_CMD;
    extraP.moduleStruct = static_cast<void*>(pP2Pq);
    pFrameParams->mvExtraParam.push_back(extraP);

    return true;
}


// Set MTK_ISP_P2_IN_IMG_FMT to value and return the original value (if not exists returns 0).
//  @param halMeta      HAL metadata to set and query.
//  @param value        Value to set.
//  @param pRestorer    An unique_ptr<void*> with a custom deleter, the deleter is to reset
//                      MTK_ISP_P2_IN_IMG_FMT of halMeta back to original value, where the deleter
//                      is being invoked while pRestorer is being deleted.
//  @note While invoking the deleter of pRestorer, the reference of halMeta must be available,
//        or unexpected behavior will happen.
inline static MINT32
setIspP2ImgFmtTag(
        IMetadata&                                              halMeta,
        MINT32                                                  value,
        std::unique_ptr<void, std::function<void(void*)> >*     pRestorer = nullptr
        )
{
    MINT32 tagIspP2InFmt = 0;

    // try to retrieve MTK_ISP_P2_IN_IMG_FMT (for backup)
    IMetadata::getEntry<MINT32>(&halMeta, MTK_ISP_P2_IN_IMG_FMT, tagIspP2InFmt);

    // set MTK_ISP_P2_IN_IMG_FMT to 1 (1 means YUV->YUV)
    IMetadata::setEntry<MINT32>(&halMeta, MTK_ISP_P2_IN_IMG_FMT, value);
    mfllLogD3("set MTK_ISP_P2_IN_IMG_FMT to %d", value);

    if (pRestorer) {
        auto myRestorer = [&halMeta, tagIspP2InFmt](void*) mutable -> void
        {
            mfllLogD3("restore MTK_ISP_P2_IN_IMG_FMT back to the original one -> %d", tagIspP2InFmt);
            // restore metaset.halMeta's MTK_ISP_P2_IN_IMG_FMT back to the original one (default is 0)
            IMetadata::setEntry<MINT32>(&halMeta, MTK_ISP_P2_IN_IMG_FMT, tagIspP2InFmt);
        };

        // declare an unique_ptr and gives a value, the main purpose is invoking a custom deleter
        // while destroying _pRestorer (we will move _r to *pRestorer).
        std::unique_ptr<void, std::function<void(void*)> > _r(
                reinterpret_cast<void*>(0x00BADBAD), // we WON'T delete this pointer. Just gives it a value to invoke custom deleter
                std::move(myRestorer)
                );

        *pRestorer = std::move( _r );
    }

    return tagIspP2InFmt;
}


static bool isRawFormat(const EImageFormat fmt)
{
    /* RAW format is in range of eImgFmt_RAW_START -> eImgFmt_BLOB_START */
    /* see include/mtkcam/def/ImageFormat.h */
    return (fmt >= eImgFmt_RAW_START) && (fmt < eImgFmt_BLOB_START);
}

//-----------------------------------------------------------------------------
/**
 *  M F L L    M F B
 */
IMfllMfb* IMfllMfb::createInstance(void)
{
    return (IMfllMfb*)new MfllMfb();
}
//
//-----------------------------------------------------------------------------
//
void IMfllMfb::destroyInstance(void)
{
    decStrong((void*)this);
}
//
//-----------------------------------------------------------------------------
//
MfllMfb::MfllMfb(void)
    : m_sensorId(0)
    , m_shotMode((enum MfllMode)0)
    , m_nrType(NoiseReductionType_None)
    , m_syncPrivateData(nullptr)
    , m_syncPrivateDataSize(0)
    , m_pCore(nullptr)
    , m_encodeYuvCount(0)
    , m_blendCount(0)
    , m_bIsInited(false)
    , m_bExifDumpped{0}
    , m_pMainMetaApp(nullptr)
    , m_pMainMetaHal(nullptr)
    , m_pBPCI(nullptr)
{
    mfllAutoLogFunc();
}
//
//-----------------------------------------------------------------------------
//
MfllMfb::~MfllMfb(void)
{
    mfllAutoLogFunc();
    m_pNormalStream = nullptr;
    m_p3A = nullptr;
}
//
//-----------------------------------------------------------------------------
//
enum MfllErr MfllMfb::init(int sensorId)
{
    enum MfllErr err = MfllErr_Ok;
    Mutex::Autolock _l(m_mutex);

    mfllAutoLogFunc();

    if (m_bIsInited) { // do not init twice
        goto lbExit;
    }

    mfllLogD3("Init MfllMfb with sensor id --> %d", sensorId);
    m_sensorId = sensorId;

    /* RAII for INormalStream */
    m_pNormalStream = decltype(m_pNormalStream)(
        INormalStream::createInstance(sensorId),
        [](INormalStream* p) {
            if (!p) return;
            p->uninit(LOG_TAG);
            p->destroyInstance();
        }
    );

    if (CC_UNLIKELY( m_pNormalStream.get() == nullptr )) {
        mfllLogE("create INormalStream fail");
        err = MfllErr_UnexpectedError;
        goto lbExit;
    }
    else {
        MBOOL bResult = m_pNormalStream->init(LOG_TAG);
        if (CC_UNLIKELY(bResult == MFALSE)) {
            mfllLogE("init INormalStream returns MFALSE");
            err = MfllErr_UnexpectedError;
            goto lbExit;
        }
    }

    /* RAII for IHal3A instance */
    m_p3A = decltype(m_p3A)(
            MAKE_Hal3A(sensorId, LOG_TAG),
            [](IHal3A* p){ if (p) p->destroyInstance(LOG_TAG); }
    );

    if (CC_UNLIKELY( m_p3A.get() == nullptr )) {
        mfllLogE("create IHal3A fail");
        err = MfllErr_UnexpectedError;
        goto lbExit;
    }


    /* RAII for IEgnStream instance */
    m_pMfbStream = decltype(m_pMfbStream)(
            IEgnStream<MFBConfig>::createInstance(LOG_TAG),
            [](IEgnStream<MFBConfig>* p) {
                if (!p) return;
                p->uninit();
                p->destroyInstance(LOG_TAG);
            }
    );

    /* init IEgnStream */
    if (CC_UNLIKELY( m_pMfbStream.get() == nullptr )) {
        mfllLogE("create IEgnStream failed");
        err = MfllErr_UnexpectedError;
        goto lbExit;
    }
    else {
        MBOOL bResult = m_pMfbStream->init();
        if (CC_UNLIKELY( bResult == MFALSE )) {
            mfllLogE("init IEgnStream returns MFALSE");
            err = MfllErr_UnexpectedError;
            goto lbExit;
        }
    }


    /* mark as inited */
    m_bIsInited = true;

    /* create ISP tuning buffer chunks, these buffer chunks are supposed to be accessed w/o race condition */
    {
        size_t regTableSize = m_pNormalStream->getRegTableSize();
        if (__builtin_expect( regTableSize <= 0, false )) {
            mfllLogE("%s: INormalStream::getRegTableSize() returns 0", __FUNCTION__);
            *(uint32_t*)(0x00000000) = 0xdeadfeed;
            return MfllErr_UnexpectedError;
        }
        m_tuningBuf.data = std::make_unique<char[]>(regTableSize);
        m_tuningBuf.size = regTableSize;
    }

    /* create ISP(MFB) tuning buffer chunk */
    {
        constexpr const size_t MFB_TUNING_BUF_SIZE = sizeof( mfb_reg_t );
        static_assert( MFB_TUNING_BUF_SIZE > 0, "sizeof( mfb_reg_t ) is 0");
        m_tuningBufMfb.data = std::make_unique<char[]>(MFB_TUNING_BUF_SIZE);
        m_tuningBufMfb.size = MFB_TUNING_BUF_SIZE;
    }

lbExit:
    return err;
}
//
// ----------------------------------------------------------------------------
//
void MfllMfb::setMfllCore(
        IMfllCore* c
        )
{
    Mutex::Autolock _l(m_mutex);
    m_pCore = c;
}
//
// ----------------------------------------------------------------------------
//
void MfllMfb::setShotMode(
        const enum MfllMode& mode
        )
{
    Mutex::Autolock _l(m_mutex);
    m_shotMode = mode;
}
//
// ----------------------------------------------------------------------------
//
void MfllMfb::setPostNrType(
        const enum NoiseReductionType& type
        )
{
    Mutex::Autolock _l(m_mutex);
    m_nrType = type;
}
//
// ----------------------------------------------------------------------------
//
enum MfllErr MfllMfb::blend(IMfllImageBuffer *base, IMfllImageBuffer *ref, IMfllImageBuffer *out, IMfllImageBuffer *wt_in, IMfllImageBuffer *wt_out)
{
    mfllAutoLogFunc();

    /* It MUST NOT be NULL, so don't do error handling */
    if (base == NULL) {
        mfllLogE("%s: base is NULL", __FUNCTION__);
        return MfllErr_NullPointer;
    }

    if (ref == NULL) {
        mfllLogE("%s: ref is NULL", __FUNCTION__);
        return MfllErr_NullPointer;
    }

    if (out == NULL) {
        mfllLogE("%s: out is NULL", __FUNCTION__);
        return MfllErr_NullPointer;
    }

    if (wt_out == NULL) {
        mfllLogE("%s: wt_out is NULL", __FUNCTION__);
        return MfllErr_NullPointer;
    }

    return blend(
            (IImageBuffer*)base->getImageBuffer(),
            (IImageBuffer*)ref->getImageBuffer(),
            (IImageBuffer*)out->getImageBuffer(),
            wt_in ? (IImageBuffer*)wt_in->getImageBuffer() : NULL,
            (IImageBuffer*)wt_out->getImageBuffer()
            );
}
//
// ----------------------------------------------------------------------------
//
enum MfllErr MfllMfb::blend(
        IMfllImageBuffer* base,
        IMfllImageBuffer* ref,
        IMfllImageBuffer* conf,
        IMfllImageBuffer* out,
        IMfllImageBuffer* wt_in,
        IMfllImageBuffer* wt_out
        )
{
    mfllAutoLogFunc();

    /* It MUST NOT be NULL, so don't do error handling */
    if (base == NULL) {
        mfllLogE("%s: base is NULL", __FUNCTION__);
        return MfllErr_NullPointer;
    }

    if (ref == NULL) {
        mfllLogE("%s: ref is NULL", __FUNCTION__);
        return MfllErr_NullPointer;
    }

    if (conf == NULL) {
        mfllLogE("%s: confidence map is NULL", __FUNCTION__);
        return MfllErr_NullPointer;
    }

    if (out == NULL) {
        mfllLogE("%s: out is NULL", __FUNCTION__);
        return MfllErr_NullPointer;
    }

    if (wt_out == NULL) {
        mfllLogE("%s: wt_out is NULL", __FUNCTION__);
        return MfllErr_NullPointer;
    }

    return blend(
            (IImageBuffer*)base->getImageBuffer(),
            (IImageBuffer*)ref->getImageBuffer(),
            (IImageBuffer*)out->getImageBuffer(),
            wt_in ? (IImageBuffer*)wt_in->getImageBuffer() : NULL,
            (IImageBuffer*)wt_out->getImageBuffer(),
            (IImageBuffer*)conf->getImageBuffer()
            );
}


enum MfllErr MfllMfb::blend(
        IImageBuffer *base,
        IImageBuffer *ref,
        IImageBuffer *out,
        IImageBuffer *wt_in,
        IImageBuffer *wt_out,
        IImageBuffer *confmap /* = nullptr */)
{
    enum MfllErr err = MfllErr_Ok;
    MBOOL bRet = MTRUE;

    mfllAutoLogFunc();
    mfllAutoTraceFunc();

    /* get member resource here */
    m_mutex.lock();
    enum NoiseReductionType nrType = m_nrType;
    EIspProfile_T profile = (EIspProfile_T)0; // which will be set later.
    int sensorId = m_sensorId;
    size_t index = m_pCore->getIndexByNewIndex(m_blendCount);
    m_blendCount++;
    /* get metaset */
    if (index >= m_vMetaSet.size()) {
        mfllLogE("%s: index(%zu) is out of size of metaset(%zu)", __FUNCTION__, index, m_vMetaSet.size());
        m_mutex.unlock();
        return MfllErr_UnexpectedError;
    }
    MetaSet_T metaset = getMainMetasetLocked();
    m_mutex.unlock();

    /* determine ISP profile */
    if (isZsdMode(m_shotMode)) {
        profile = (nrType == NoiseReductionType_SWNR)
            ? get_isp_profile(eMfllIspProfile_Mfb_Zsd_Swnr, m_shotMode)
            : get_isp_profile(eMfllIspProfile_Mfb_Zsd, m_shotMode);
        if (IS_WRONG_ISP_PROFILE(profile)) {
            mfllLogE("%s: error isp profile mapping", __FUNCTION__);
        }
    }
    else {
        profile = (nrType == NoiseReductionType_SWNR)
            ? get_isp_profile(eMfllIspProfile_Mfb_Swnr, m_shotMode)
            : get_isp_profile(eMfllIspProfile_Mfb, m_shotMode);
        if (IS_WRONG_ISP_PROFILE(profile)) {
            mfllLogE("%s: error isp profile mapping", __FUNCTION__);
        }
    }


    /**
     * Retrieve MFB tuning data
     */
    if (!m_tuningBufMfb.isInited) {
        std::unique_ptr<char[]>& pMfbTuning = m_tuningBufMfb.data;

        // FIXME: no need tuning_reg here, but IHal3A needs to fix bug.
        size_t regTableSize = m_pNormalStream->getRegTableSize();
        std::unique_ptr<char[]> tuning_reg(new char[regTableSize]());

        TuningParam rTuningParam;
        rTuningParam.pMfbBuf = static_cast<void*>( m_tuningBufMfb.data.get() );
        rTuningParam.pRegBuf = static_cast<void*>( tuning_reg.get() ); // FIXME: remove this after SW8 patched

        /* update Metadata */
        IMetadata::setEntry<MUINT8>(&metaset.halMeta, MTK_3A_PGN_ENABLE, MTRUE);
        IMetadata::setEntry<MUINT8>(&metaset.halMeta, MTK_3A_ISP_PROFILE, profile);
        // to makes debug info different
        IMetadata::setEntry<MINT32>(&metaset.halMeta, MTK_PIPELINE_REQUEST_NUMBER, m_blendCount - 1);

        /* setIsp to retrieve tuning buffer */
        MetaSet_T rMetaSet; // saving output of setIsp
        mfllTraceBegin(sStrSetIsp);
        setIspP2ImgFmtTag(metaset.halMeta, 1); // set format to 1 (YUV->YUV)
        if (this->setIsp(0, metaset, &rTuningParam, &rMetaSet) == 0) {
            MfbPlatformConfig pltcfg;
            pltcfg[ePLATFORMCFG_STAGE]          = STAGE_MFB;
            pltcfg[ePLATFORMCFG_DIP_X_REG_T]    = reinterpret_cast<intptr_t>((volatile void*)rTuningParam.pMfbBuf);

            if (!refine_register(pltcfg)) {
                mfllLogE("%s: refine_register returns failed", __FUNCTION__);
            }
            else {
#ifdef __DEBUG
                mfllLogD3("%s: refine_register ok", __FUNCTION__);
#endif
            }

#if __DEBUG
            uint32_t* pRegData = static_cast<uint32_t*>(
                    static_cast<void*>(pMfbTuning.get())
                    );
            for (size_t i = 0; i < m_tuningBufMfb.size; i += 4, pRegData++) {
                mfllLogD("MFB reg: %#x , (offset %zu)", *pRegData, i);
            }
#endif
            /* update debug Exif */
            {
                mfllTraceBegin("dumpDbgInfo");
                dump_exif_info(m_pCore, pMfbTuning.get(), STAGE_MFB);
                mfllTraceEnd();
            }

            mfllTraceEnd(); // AfterSetIsp
        }
        else {
            mfllLogE("%s: set ISP profile(MFB) failed...", __FUNCTION__);
            err = MfllErr_UnexpectedError;
            mfllTraceEnd(); // AfterSetIsp
            goto lbExit;
        }

        m_tuningBufMfb.isInited = true; // marks initialized
    }


    /**
     *  Stand alone MFB
     *
     *  Configure input parameters and execute
     */
    {
        MFBConfig mfbCfg;

        /* Image format */
        mfbCfg.Mfb_format = [&]()
        {
            auto f = base->getImgFormat();
            switch (f) {
            case eImgFmt_YUY2:
                return FMT_MFB_YUV422;
            case eImgFmt_NV12:
            case eImgFmt_NV21:
                return FMT_MFB_YUV420_2P;
            default:
                mfllLogE("wrong data format(%#x) for MFB driver", f);
            }
            return FMT_MFB_YUV422;
        }();

        /* MFB configurations */
        {
            /**
             *  Check if the first time to blend.
             *   - 0: Lowlight mode, first pass (no blended weight read-in)
             *   - 1: Lowlight mode, following pass (requires blended weight read-in)
             */
            mfbCfg.Mfb_bldmode = ((m_blendCount - 1) <= 0) ? 0 : 1;

            /* passing tuning buffer to MFB driver */
            mfbCfg.Mfb_tuningBuf = static_cast<unsigned int*>(
                    (void*)m_tuningBufMfb.data.get()
                    );
        }

        /* input: source frame */
        {
            mfbCfg.Mfb_inbuf_baseFrame = base;
            mfllLogI("input(base): va/pa(%p/%p) fmt(%#x), size=(%d,%d), planes(%d), stride(%d)",
                    // va/pa
                    (void*)base->getBufVA(0), (void*)base->getBufPA(0),
                    // fmt
                    base->getImgFormat(),
                    // size
                    base->getImgSize().w, base->getImgSize().h,
                    // planes, stride
                    base->getPlaneCount(), base->getBufStridesInBytes(0)
                    );
        }

        /* input: reference frame */
        {
            mfbCfg.Mfb_inbuf_refFrame = ref;
            mfllLogI("input(ref): va/pa(%p/%p) fmt(%#x), size=(%d,%d), planes(%d), stride(%d)",
                    // va/pa
                    (void*)ref->getBufVA(0), (void*)ref->getBufPA(0),
                    // fmt
                    ref->getImgFormat(),
                    // size
                    ref->getImgSize().w, ref->getImgSize().h,
                    // planes, stride
                    ref->getPlaneCount(), ref->getBufStridesInBytes(0)
                    );
        }

        /* input: weighting table, for not the first blending, we need to give weighting map */
        if (wt_in && mfbCfg.Mfb_bldmode) {
            mfbCfg.Mfb_inbuf_blendWeight = wt_in;
            mfllLogI("input(wt_in): va/pa(%p/%p) fmt(%#x), size=(%d,%d), planes(%d), stride(%d)",
                    // va/pa
                    (void*)wt_in->getBufVA(0), (void*)wt_in->getBufPA(0),
                    // fmt
                    wt_in->getImgFormat(),
                    // size
                    wt_in->getImgSize().w, wt_in->getImgSize().h,
                    // planes, stride
                    wt_in->getPlaneCount(), wt_in->getBufStridesInBytes(0)
                    );
        }
        else if (wt_in == nullptr && mfbCfg.Mfb_bldmode == 0) {
            mfllLogD("input(wt_in): no need (blendCount:%d, mfbCfg.Mfb_bldmode:%d, wt_in:%p)",
                     m_blendCount, mfbCfg.Mfb_bldmode, wt_in);
        }
        else {
            mfllLogW("input(wt_in): invalid config (blendCount:%d, mfbCfg.Mfb_bldmode:%d, wt_in:%p)",
                     m_blendCount, mfbCfg.Mfb_bldmode, wt_in);
            mfbCfg.Mfb_bldmode = 0; // force mfb mode to 0 if wt_in and blend mode are unmatched.
        }


        /* output: blended frame */
        {
            mfbCfg.Mfb_outbuf_frame = out;
            mfllLogI("output(blended): va/pa(%p/%p) fmt(%#x), size=(%d,%d), planes(%d), stride(%d)",
                    // va/pa
                    (void*)out->getBufVA(0), (void*)out->getBufPA(0),
                    // fmt
                    out->getImgFormat(),
                    // size
                    out->getImgSize().w, out->getImgSize().h,
                    // planes, stride
                    out->getPlaneCount(), out->getBufStridesInBytes(0)
                    );
        }

        /* output: new weighting table */
        {
            mfbCfg.Mfb_outbuf_blendWeight = wt_out;
            mfllLogI("output(wt_out): va/pa(%p/%p) fmt(%#x), size=(%d,%d), planes(%d), stride(%d)",
                    // va/pa
                    (void*)wt_out->getBufVA(0), (void*)wt_out->getBufPA(0),
                    // fmt
                    wt_out->getImgFormat(),
                    // size
                    wt_out->getImgSize().w, wt_out->getImgSize().h,
                    // planes, stride
                    wt_out->getPlaneCount(), wt_out->getBufStridesInBytes(0)
                    );
        }

        /**
         *  Confidence map (since MFNR v2.0, new MFB hardware introduced) depends
         *  on platform, not all platform supports confidence map.
         */
#if MFB_SUPPORT_CONF_MAP
        do {
            if (CC_UNLIKELY( confmap == nullptr )) {
                mfbCfg.Mfb_srzEn = 0;
                mfllLogI("%s: no confidence map", __FUNCTION__);
                break; // break do-while loop.
            }

            mfbCfg.Mfb_srzEn = 1;
            /**
             * Note: Confidence map is generated in block-based(16x16) domain. And
             *       the image to be used is 1/4 size of full size image. Hence
             *       we need to consider that padding probem while reconstructing
             *       block-based confidence map to full size pixel-based confidence
             *       map.
             *
             *       First,
             *       we have a 1/4 image, and to calculate the confidence value
             *       block by block, where block size is fixed to 16x16.
             *
             *       Real image size (1/4 of full size)
             *       <--------->
             *       +---------------+---+
             *       |               |   |
             *       |               |   |
             *       |               |   |
             *       +---------------+   |
             *       |                   |
             *       +-------------------+
             *       <------------------->
             *       16 pixe aligned
             *
             *       While blending, we need to reconstruct the confidence map,
             *       where is based on block domain, to the full size pixel domain
             *       confidence map.
             *
             *       So the output size of pixel domain confidence map is necessary
             *       to be with 32 pixel alignment, or the map may have a little
             *       shifted.
             *
             *       Fortunately, p2 driver handled this, we only need to fill up
             *       the REAL output size, p2 driver will resize to 32 pixel aligned
             *       image and crop to the real size we need.
             */
            mfbCfg.Mfb_srz5Info.in_w    = confmap->getImgSize().w;
            mfbCfg.Mfb_srz5Info.in_h    = confmap->getImgSize().h;

            mfbCfg.Mfb_srz5Info.out_w   = base->getImgSize().w;
            mfbCfg.Mfb_srz5Info.out_h   = base->getImgSize().h;

            /* no cropping */
            mfbCfg.Mfb_srz5Info.crop_x  = 0;
            mfbCfg.Mfb_srz5Info.crop_y  = 0;
            mfbCfg.Mfb_srz5Info.crop_w  = confmap->getImgSize().w;
            mfbCfg.Mfb_srz5Info.crop_h  = confmap->getImgSize().h;

            mfbCfg.Mfb_inbuf_confidenceMap = confmap;

            mfllLogD3("%s: SRZ5, in(w,h)=(%d,%d), out(w,h)=(%d,%d)", __FUNCTION__,
                    mfbCfg.Mfb_srz5Info.in_w,   mfbCfg.Mfb_srz5Info.in_h,
                    mfbCfg.Mfb_srz5Info.out_w,  mfbCfg.Mfb_srz5Info.out_h
                    );
        } while(0);
#endif // MFB_SUPPORT_CONF_MAP

        /**
         *  Invoking stand alone MFB driver, this driver has an async call hence
         *  we need condition_variable to wait.
         */
        using MfllMfb_Imp::P2Cookie;
        P2Cookie cookie;

        EGNParams<MFBConfig> rMfbParams;
        rMfbParams.mEGNConfigVec.push_back(mfbCfg);
        rMfbParams.mpEngineID = eMFB;
        rMfbParams.mpCookie = reinterpret_cast<void*>(&cookie);
        rMfbParams.mpfnCallback = __P2Cb< EGNParams<MFBConfig> >;

        mfllLogD3("%s: MFB EGNenque", __FUNCTION__);
        mfllLogI("%s: mpCookie=%p", __FUNCTION__, rMfbParams.mpCookie);
        mfllTraceBegin(sStrMfb);
        if (CC_UNLIKELY( ! m_pMfbStream->EGNenque(rMfbParams) )) {
            mfllLogE("%s: pass 2 enque returns fail", __FUNCTION__);
            err = MfllErr_UnexpectedError;
            mfllTraceEnd();
            goto lbExit;
        }

        // to synchronize qParams
        std::atomic_thread_fence(std::memory_order_release);

        /* check if signaled, if not yet, we have to wait condition variable */
        mfllLogI("%s: wait MFB deque [+]", __FUNCTION__);
        {
            std::unique_lock<std::mutex> lk(cookie.__mtx);
            if (0 == cookie.__signaled.load(std::memory_order_seq_cst)) {
                mfllLogI("%s: start waiting cond", __FUNCTION__);
                /* wait for a second */
                auto status = cookie.__cv.wait_for(lk, std::chrono::milliseconds(MFLL_MFB_ENQUE_TIMEOUT_MS));
                if (CC_UNLIKELY( status == std::cv_status::timeout )) {
                    std::string _log = std::string("FATAL: wait timeout of dequing from MFB blend: ") + \
                                       std::to_string(MFLL_MFB_ENQUE_TIMEOUT_MS) + std::string("ms");
                    AEE_ASSERT_MFB(_log.c_str());
                }
            }
            else {
                mfllLogI("%s: already signaled", __FUNCTION__);
            }
        }
        mfllLogI("%s: wait MFB deque [-]", __FUNCTION__);
        mfllTraceEnd();

    }
lbExit:
    return err;
}
//
// ----------------------------------------------------------------------------
//
IMetadata* MfllMfb::getMainAppMetaLocked() const
{
    /* always use the best frame metadata */
    size_t index = m_pCore->getIndexByNewIndex(0);
    /* get metaset */
    if (__builtin_expect( index >= m_vMetaSet.size(), false )) {
        mfllLogE("%s: index(%zu) is out of size of metaset(%zu)", __FUNCTION__, index, m_vMetaSet.size());
        return nullptr;
    }
    return m_vMetaApp[index];
}
//
// ----------------------------------------------------------------------------
//
IMetadata* MfllMfb::getMainHalMetaLocked() const
{
    /* always use the best frame metadata */
    size_t index = m_pCore->getIndexByNewIndex(0);
    /* get metaset */
    if (__builtin_expect( index >= m_vMetaSet.size(), false )) {
        mfllLogE("%s: index(%zu) is out of size of metaset(%zu)", __FUNCTION__, index, m_vMetaSet.size());
        return nullptr;
    }
    return m_vMetaHal[index];
}
//
// ----------------------------------------------------------------------------
//
MetaSet_T MfllMfb::getMainMetasetLocked() const
{
    /* always use the best frame metadata */
    size_t index = m_pCore->getIndexByNewIndex(0);
    /* get metaset */
    if (__builtin_expect( index >= m_vMetaSet.size(), false )) {
        mfllLogE("%s: index(%zu) is out of size of metaset(%zu)", __FUNCTION__, index, m_vMetaSet.size());
        return MetaSet_T();
    }
    return m_vMetaSet[index];
}
//
// ----------------------------------------------------------------------------
//
MINT32 MfllMfb::setIsp(
        MINT32          flowType,
        MetaSet_T&      control,
        TuningParam*    pTuningBuf,
        MetaSet_T*      pResult /* = nullptr */,
        IImageBuffer*   pSrcImage /* = nullptr */)
{
    MINT32 r = MTRUE;

    if (__builtin_expect( m_p3A.get() == nullptr, false )) {
        mfllLogE("%s: m_p3A is nullptr", __FUNCTION__);
        mfllDumpStack(__FUNCTION__);
        return -1;
    }

    if (pSrcImage) {
        // since P2 input image may be not a full size image, the tuning manager
        // must know the input image resolution for updating the parameters from
        // full size domain to customized domain.
        updateP2RevisedTag(pSrcImage, &control.halMeta);
    }

    r = m_p3A->setIsp(flowType, control, pTuningBuf, pResult);

    if (pSrcImage) {
        // make sure these P2 revised tag has been removed to avoid ambiguous of
        // the following users.
        removeP2RevisedTag(&control.halMeta);
        if (pResult)
            removeP2RevisedTag(&pResult->halMeta);
    }
    return r;
}
//
// ----------------------------------------------------------------------------
//
enum MfllErr MfllMfb::mix(IMfllImageBuffer *base, IMfllImageBuffer *ref, IMfllImageBuffer *out, IMfllImageBuffer *wt)
{
    mfllAutoLogFunc();
    mfllAutoTraceFunc();

    enum MfllErr err = MfllErr_Ok;

    IImageBuffer *img_src = 0;
    IImageBuffer *img_ref = 0;
    IImageBuffer *img_dst = 0;
    IImageBuffer *img_wt = 0;

    /* check buffers */
    if (base == 0 || ref == 0 || out == 0 || wt == 0) {
        mfllLogE("%s: any argument cannot be NULL", __FUNCTION__);
        return MfllErr_BadArgument;
    }

    img_src = (IImageBuffer*)base->getImageBuffer();
    img_ref = (IImageBuffer*)ref->getImageBuffer();
    img_dst = (IImageBuffer*)out->getImageBuffer();
    img_wt =  (IImageBuffer*)wt->getImageBuffer();

    /* check buffers again (these buffers may be dereferenced */
    if (img_src == nullptr || img_dst == nullptr || img_wt == nullptr)
    {
        mfllLogE("%s: any image buffer cannot be nullptr", __FUNCTION__);
        return MfllErr_BadArgument;
    }

    /* check resolution */
    {
        MSize size_src = img_src->getImgSize();
        MSize size_dst = img_dst->getImgSize();
        MSize size_wt  = img_wt->getImgSize();

        if (size_src != size_dst || size_src != size_wt) {
            mfllLogD("%s: Resolution of images are not the same, src=%dx%d, dst=%dx%d, wt=%dx%d",
                    __FUNCTION__,
                    size_src.w, size_src.h,
                    size_dst.w, size_dst.h,
                    size_wt.w, size_wt.h);
        }
    }

    m_mutex.lock();
    int sensorId = m_sensorId;
    enum NoiseReductionType nrType = m_nrType;
    enum MfllMode shotMode = m_shotMode;
    EIspProfile_T profile = (EIspProfile_T)0; // which will be set later.
    void *privateData = m_syncPrivateData;
    size_t privateDataSize = m_syncPrivateDataSize;
    int index = m_pCore->getIndexByNewIndex(0);
    /* get metaset */
    if (index >= static_cast<int>(m_vMetaSet.size())) {
        mfllLogE("%s: index(%d) is out of size of metaset(%zu)", __FUNCTION__, index, m_vMetaSet.size());
        m_mutex.unlock();
        return MfllErr_UnexpectedError;
    }
    MetaSet_T metaset = m_vMetaSet[index];
    m_mutex.unlock();

    MBOOL ret = MTRUE;

    /**
     * P A S S 2
     */
    QParams     qParams;
    FrameParams params;
    PQParam     p2PqParam;
    DpPqParam   mdpPqDre;

    /* Mixing */
    params.mStreamTag = NSCam::NSIoPipe::NSPostProc::ENormalStreamTag_MFB_Mix;

    /* determine ISP profile */
    if (isZsdMode(shotMode)) {
        profile = (nrType == NoiseReductionType_SWNR)
            ? get_isp_profile(eMfllIspProfile_AfterBlend_Zsd_Swnr, m_shotMode)
            : get_isp_profile(eMfllIspProfile_AfterBlend_Zsd, m_shotMode);
        if (IS_WRONG_ISP_PROFILE(profile)) {
            mfllLogE("%s: error isp profile mapping", __FUNCTION__);
        }
    }
    else {
        profile = (nrType == NoiseReductionType_SWNR)
            ? get_isp_profile(eMfllIspProfile_AfterBlend_Swnr, m_shotMode)
            : get_isp_profile(eMfllIspProfile_AfterBlend, m_shotMode);
        if (IS_WRONG_ISP_PROFILE(profile)) {
            mfllLogE("%s: error isp profile mapping", __FUNCTION__);
        }
    }

    /* execute pass 2 operation */
    {
        // get size of dip_x_reg_t
        volatile size_t regTableSize = m_pNormalStream->getRegTableSize();
        if (regTableSize <= 0) {
            mfllLogE("%s: unexpected tuning buffer size: %zu", __FUNCTION__, regTableSize);
        }

        std::unique_ptr<char[]> tuning_reg(new char[regTableSize]());
        void *tuning_lsc;

        mfllLogD("%s: create tuning register data chunk with size %zu",
                __FUNCTION__, regTableSize);

        TuningParam rTuningParam;
        rTuningParam.pRegBuf = tuning_reg.get();

        IMetadata::setEntry<MUINT8>(&metaset.halMeta, MTK_3A_PGN_ENABLE, MTRUE);
        IMetadata::setEntry<MUINT8>(&metaset.halMeta, MTK_3A_ISP_PROFILE, profile);

        MetaSet_T rMetaSet;

        MfbPlatformConfig pltcfg;
        pltcfg[ePLATFORMCFG_STAGE] = STAGE_MIX;

        // MIX stage is always YUV->YUV
        std::unique_ptr<void, std::function<void(void*)> > _tagIspP2InFmtRestorer;
        MINT32 tagIspP2InFmt = setIspP2ImgFmtTag(
                metaset.halMeta,
                1,
                &_tagIspP2InFmtRestorer);

        mfllTraceBegin(sStrSetIsp);

        if (this->setIsp(0, metaset, &rTuningParam, &rMetaSet, img_src) == 0) {
            mfllTraceBegin(sStrAfterSetIsp);

            // restore MTK_ISP_P2_IN_IMG_FMT back
            IMetadata::setEntry<MINT32>(&rMetaSet.halMeta, MTK_ISP_P2_IN_IMG_FMT, tagIspP2InFmt);

            int expected_blend_cnt = m_pCore->getBlendFrameNum() - 1;
            int real_blend_cnt     = m_pCore->getFrameBlendedNum();
            mfllLogD3("%s: blend frames count, real=%d, expected=%d",
                    __FUNCTION__, real_blend_cnt, expected_blend_cnt);

            // refine postNR implementation
            int _enablePostNrRefine = MFLL_POST_NR_REFINE_ENABLE;
            int _enablePostNrRefineByProp = MfllProperty::readProperty
                (Property_PostNrRefine);

            if (CC_UNLIKELY( _enablePostNrRefineByProp >= 0 )) {
                _enablePostNrRefine = _enablePostNrRefineByProp;
            }

            if (CC_LIKELY( _enablePostNrRefine > 0 )) {
                mfllLogD3("%s: enable postNR refine", __FUNCTION__);
                mfllTraceBegin(sStrRefineReg);
                bss_refine_postnr_regs(
                        static_cast<void*>(tuning_reg.get()),
                        real_blend_cnt,
                        expected_blend_cnt);
                mfllTraceEnd(); // RefineReg
            }
            else {
                mfllLogD("%s: disable postNR refine", __FUNCTION__);
            }

            mfllLogD3("%s: get tuning data, reg=%p, lsc=%p",
                    __FUNCTION__, rTuningParam.pRegBuf, rTuningParam.pLsc2Buf);

            // refine register
            pltcfg[ePLATFORMCFG_DIP_X_REG_T] =
                    reinterpret_cast<intptr_t>((volatile void*)rTuningParam.pRegBuf);

            mfllTraceBegin(sStrRefineReg);
            if (!refine_register(pltcfg)) {
                mfllLogE("%s: refine_register returns failed", __FUNCTION__);
            }
            else {
#ifdef __DEBUG
                mfllLogD("%s: refine_register ok", __FUNCTION__);
#endif
            }
            mfllTraceEnd(); // RefineReg

            if (m_bExifDumpped[STAGE_MIX] == 0) {
                dump_exif_info(m_pCore, tuning_reg.get(), STAGE_MIX);
                m_bExifDumpped[STAGE_MIX] = 1;
            }

#ifdef __DEBUG
            debug_pass2_registers(tuning_reg.get(), STAGE_MIX);
#endif
            params.mTuningData = static_cast<MVOID*>(tuning_reg.get()); // adding tuning data
            mfllTraceEnd(); // AfterSetIsp
        }
        else {
            mfllLogE("%s: set ISP profile failed...", __FUNCTION__);
            err = MfllErr_UnexpectedError;
            return err;
        }
        mfllTraceEnd(); // setIsp
        mfllTraceBegin(sStrBeforeP2Enque);

        _tagIspP2InFmtRestorer = nullptr; // trigger restore MTK_ISP_P2_IN_IMG_FMT

        // input: blended frame [IMGI port]
        {
            Input p;
            p.mPortID       = MIX_PORT_IN_BASE_FRAME; // should be IMGI port
            p.mPortID.group = 0;
            p.mBuffer       = img_src;
            params.mvIn.push_back(p);

            /* cropping info */
            // Even though no cropping necessary, but we have to pass in a crop info
            // with group id = 1 because pass 2 driver MUST need it
            MCrpRsInfo crop;
            crop.mGroupID       = 1;
            crop.mCropRect.p_integral.x = 0; // position pixel, in integer
            crop.mCropRect.p_integral.y = 0;
            crop.mCropRect.p_fractional.x = 0; // always be 0
            crop.mCropRect.p_fractional.y = 0;
            crop.mCropRect.s.w  = img_src->getImgSize().w;
            crop.mCropRect.s.h  = img_src->getImgSize().h;
            crop.mResizeDst.w   = img_src->getImgSize().w;
            crop.mResizeDst.h   = img_src->getImgSize().h;
            crop.mFrameGroup    = 0;
            params.mvCropRsInfo.push_back(crop);
        }

        // if disabled MIX3, we don't need golden frame
        // input: golden frame
        if (pltcfg[ePLATFORMCFG_ENABLE_MIX3] && pltcfg[ePLATFORMCFG_REFINE_OK])
        {
            Input p;
            p.mPortID       = MIX_PORT_IN_GOLDEN_FRAME;
            p.mPortID.group = 0;
            p.mBuffer       = img_ref;
            params.mvIn.push_back(p);
            /* this port is not support crop info */
        }
        else
        {
            mfllLogD("%s: no golden frame due to disabled MIX3", __FUNCTION__);
        }

        // input: weighting
        if (pltcfg[ePLATFORMCFG_ENABLE_MIX3] && pltcfg[ePLATFORMCFG_REFINE_OK])
        {
            Input p;
            p.mPortID       = MIX_PORT_IN_WEIGHTING;
            p.mPortID.group = 0;
            p.mBuffer       = img_wt;
            params.mvIn.push_back(p);
            /* this port is not support crop info */
        }
        else
        {
            mfllLogD("%s: no weighting input due to disabled MIX3", __FUNCTION__);
        }

        // output: main yuv
        {
            mfllLogD3("config main yuv during mixing");
            Output p;
            p.mBuffer          = img_dst;
            p.mPortID          = MIX_PORT_OUT_MAIN;
            p.mPortID.group    = 0; // always be 0
            params.mvOut.push_back(p);

            /**
             *  Cropping info, only works with input port is IMGI port.
             *  mGroupID should be the index of the MCrpRsInfo.
             */
            MCrpRsInfo crop;
            crop.mGroupID       = MIX_MAIN_GID_OUT;
            crop.mCropRect.p_integral.x = 0; // position pixel, in integer
            crop.mCropRect.p_integral.y = 0;
            crop.mCropRect.p_fractional.x = 0; // always be 0
            crop.mCropRect.p_fractional.y = 0;
            crop.mCropRect.s.w  = img_src->getImgSize().w;
            crop.mCropRect.s.h  = img_src->getImgSize().h;
            crop.mResizeDst.w   = img_src->getImgSize().w;
            crop.mResizeDst.h   = img_src->getImgSize().h;
            crop.mFrameGroup    = 0;
            params.mvCropRsInfo.push_back(crop);

            mfllLogD3("main: srcCrop (x,y,w,h)=(%d,%d,%d,%d)",
                    crop.mCropRect.p_integral.x, crop.mCropRect.p_integral.y,
                    crop.mCropRect.s.w, crop.mCropRect.s.h);
            mfllLogD3("main: dstSize (w,h)=(%d,%d)",
                    crop.mResizeDst.w, crop.mResizeDst.h);

            /* ask MDP to generate CA-LTM histogram */
            bool bEnCALTM = !!gSupportDRE;
            bool bIsMdpPort = (p.mPortID == PORT_WDMAO) || (p.mPortID == PORT_WROTO);
            if (bEnCALTM && bIsMdpPort ) {
                bool bGeneratedMdpParam = generateMdpDrePQParam(
                        mdpPqDre,
                        m_pCore->getCurrentIso(),
                        m_sensorId,
                        &m_vMetaSet[0].halMeta,// use the first metadata for DRE, due to
                                               // others use main metadata to apply DRE.
                        profile
                        );
                if ( ! bGeneratedMdpParam ) {
                    mfllLogE("%s: generate DRE DRE param failed", __FUNCTION__);
                }
                else {
                    mfllLogD3("%s: generate DRE histogram.", __FUNCTION__);
                    attachMDPPQParam2Pass2drv(p.mPortID, &params, &p2PqParam, &mdpPqDre);
                }
            }
            else {
                mfllLogD3("%s: DRE disabled or port is not MDP port", __FUNCTION__);
            }
        }
        mfllTraceEnd(); // BeforeP2Enque

        Mutex::Autolock _l(gMutexPass2Lock);

        /**
         *  Finally, we're going to invoke P2 driver to encode Raw. Notice that,
         *  we must use async function call to trigger P2 driver, which means
         *  that we have to give a callback function to P2 driver.
         *
         *  E.g.:
         *      QParams::mpfnCallback = CALLBACK_FUNCTION
         *      QParams::mpCookie --> argument that CALLBACK_FUNCTION will get
         *
         *  Due to we wanna make the P2 driver as a synchronized flow, here we
         *  simply using Mutex to sync async call of P2 driver.
         */
        using MfllMfb_Imp::P2Cookie;
        P2Cookie cookie;
        //
        qParams.mpfnCallback        = __P2Cb<QParams>;
        qParams.mpfnEnQFailCallback = __P2FailCb<QParams>;
        qParams.mpCookie        = reinterpret_cast<void*>(&cookie);
        qParams.mvFrameParams   .push_back(params);
        //
        mfllLogD3("%s: P2 enque", __FUNCTION__);
        mfllLogI("%s: mpCookie=%p", __FUNCTION__, qParams.mpCookie);
        mfllTraceBegin(sStrP2);
        if (CC_UNLIKELY( ! m_pNormalStream->enque(qParams) )) {
            mfllLogE("%s: pass 2 enque returns fail", __FUNCTION__);
            err = MfllErr_UnexpectedError;
            mfllTraceEnd();
            goto lbExit;
        }

        // to synchronize qParams
        std::atomic_thread_fence(std::memory_order_release);

        /* wait unitl P2 deque */
        mfllLogI("%s: wait P2(MIX) deque [+]", __FUNCTION__);
        {
            std::unique_lock<std::mutex> lk(cookie.__mtx);
            if (0 == cookie.__signaled.load(std::memory_order_seq_cst)) {
                /* wait for a second */
                auto status = cookie.__cv.wait_for(lk, std::chrono::milliseconds(MFLL_MFB_ENQUE_TIMEOUT_MS));
                if (CC_UNLIKELY( status == std::cv_status::timeout )) {
                    std::string _log = std::string("FATAL: wait timeout of dequing from MIX: ") \
                                       + std::to_string(MFLL_MFB_ENQUE_TIMEOUT_MS) + std::string("ms");
                    AEE_ASSERT_MFB(_log.c_str());
                }
            }
        }
        mfllLogI("%s: wait P2(MIX) deque [-]", __FUNCTION__);
        mfllTraceEnd();
    }

lbExit:
    return err;
}
//
// ----------------------------------------------------------------------------
//
enum MfllErr MfllMfb::mix(
        IMfllImageBuffer *base,
        IMfllImageBuffer *ref,
        IMfllImageBuffer *out_main,
        IMfllImageBuffer *out_thumbnail,
        IMfllImageBuffer *wt,
        const MfllRect_t& output_thumb_crop
        )
{
    mfllAutoLogFunc();
    mfllAutoTraceFunc();

    enum MfllErr err = MfllErr_Ok;

    IImageBuffer *img_src = 0;
    IImageBuffer *img_ref = 0;
    IImageBuffer *img_dst = 0;
    IImageBuffer *img_thumb = 0;
    IImageBuffer *img_wt = 0;

    /* check buffers */
    if (base == 0 || ref == 0 || out_main == 0 || out_thumbnail == 0 || wt == 0) {
        mfllLogE("%s: any argument cannot be NULL", __FUNCTION__);
        return MfllErr_BadArgument;
    }

    img_src = (IImageBuffer*)base->getImageBuffer();
    img_ref = (IImageBuffer*)ref->getImageBuffer();
    img_dst = (IImageBuffer*)out_main->getImageBuffer();
    img_thumb = (IImageBuffer*)out_thumbnail->getImageBuffer();
    img_wt =  (IImageBuffer*)wt->getImageBuffer();

    /* check buffers again (these buffers may be dereferenced */
    if (img_src == nullptr || img_dst == nullptr ||
        img_thumb == nullptr || img_wt == nullptr)
    {
        mfllLogE("%s: any image buffer cannot be nullptr", __FUNCTION__);
        return MfllErr_BadArgument;
    }

    /* check resolution */
    {
        MSize size_src = img_src->getImgSize();
        MSize size_dst = img_dst->getImgSize();
        MSize size_thumb = img_thumb->getImgSize();
        MSize size_wt  = img_wt->getImgSize();

        if (size_src != size_dst || size_src != size_wt) {
            mfllLogD("%s: Resolution of images are not the same, src=%dx%d, dst=%dx%d, wt=%dx%d",
                    __FUNCTION__,
                    size_src.w, size_src.h,
                    size_dst.w, size_dst.h,
                    size_wt.w, size_wt.h);
        }
    }

    m_mutex.lock();
    int sensorId = m_sensorId;
    enum NoiseReductionType nrType = m_nrType;
    enum MfllMode shotMode = m_shotMode;
    EIspProfile_T profile = (EIspProfile_T)0; // which will be set later.
    void *privateData = m_syncPrivateData;
    size_t privateDataSize = m_syncPrivateDataSize;
    int index = m_pCore->getIndexByNewIndex(0);
    /* get metaset */
    if (index >= static_cast<int>(m_vMetaSet.size())) {
        mfllLogE("%s: index(%d) is out of size of metaset(%zu)", __FUNCTION__, index, m_vMetaSet.size());
        m_mutex.unlock();
        return MfllErr_UnexpectedError;
    }
    MetaSet_T metaset = m_vMetaSet[index];
    m_mutex.unlock();

    MBOOL ret = MTRUE;

    /**
     * P A S S 2
     */
    QParams     qParams;
    FrameParams params;
    PQParam     p2PqParam;
    DpPqParam   mdpPqDre;

    /* Mixing */
    params.mStreamTag = NSCam::NSIoPipe::NSPostProc::ENormalStreamTag_MFB_Mix;

    /* determine ISP profile */
    if (isZsdMode(shotMode)) {
        profile = (nrType == NoiseReductionType_SWNR)
            ? get_isp_profile(eMfllIspProfile_AfterBlend_Zsd_Swnr, m_shotMode)
            : get_isp_profile(eMfllIspProfile_AfterBlend_Zsd, m_shotMode);
        if (IS_WRONG_ISP_PROFILE(profile)) {
            mfllLogE("%s: error isp profile mapping", __FUNCTION__);
        }
    }
    else {
        profile = (nrType == NoiseReductionType_SWNR)
            ? get_isp_profile(eMfllIspProfile_AfterBlend_Swnr, m_shotMode)
            : get_isp_profile(eMfllIspProfile_AfterBlend, m_shotMode);
        if (IS_WRONG_ISP_PROFILE(profile)) {
            mfllLogE("%s: error isp profile mapping", __FUNCTION__);
        }
    }

    /* execute pass 2 operation */
    {
        // get size of dip_x_reg_t
        volatile size_t regTableSize = m_pNormalStream->getRegTableSize();
        if (regTableSize <= 0) {
            mfllLogE("%s: unexpected tuning buffer size: %zu", __FUNCTION__, regTableSize);
        }

        std::unique_ptr<char[]> tuning_reg(new char[regTableSize]());
        void *tuning_lsc;

        mfllLogD("%s: create tuning register data chunk with size %zu",
                __FUNCTION__, regTableSize);

        TuningParam rTuningParam;
        rTuningParam.pRegBuf = tuning_reg.get();

        IMetadata::setEntry<MUINT8>(&metaset.halMeta, MTK_3A_PGN_ENABLE, MTRUE);
        IMetadata::setEntry<MUINT8>(&metaset.halMeta, MTK_3A_ISP_PROFILE, profile);

        MetaSet_T rMetaSet;

        MfbPlatformConfig pltcfg;
        pltcfg[ePLATFORMCFG_STAGE] = STAGE_MIX;

        // MIX stage is always YUV->YUV
        std::unique_ptr<void, std::function<void(void*)> > _tagIspP2InFmtRestorer;
        MINT32 tagIspP2InFmt = setIspP2ImgFmtTag(
                metaset.halMeta,
                1,
                &_tagIspP2InFmtRestorer);

        mfllTraceBegin(sStrSetIsp);
        if (this->setIsp(0, metaset, &rTuningParam, &rMetaSet, img_src) == 0) {
            mfllTraceBegin(sStrAfterSetIsp);

            // restore MTK_ISP_P2_IN_IMG_FMT back
            IMetadata::setEntry<MINT32>(&rMetaSet.halMeta, MTK_ISP_P2_IN_IMG_FMT, tagIspP2InFmt);

            int expected_blend_cnt = m_pCore->getBlendFrameNum() - 1;
            int real_blend_cnt     = m_pCore->getFrameBlendedNum();
            mfllLogD3("%s: blend frames count, real=%d, expected=%d",
                    __FUNCTION__, real_blend_cnt, expected_blend_cnt);

            // refine postNR implementation
            int _enablePostNrRefine = MFLL_POST_NR_REFINE_ENABLE;
            int _enablePostNrRefineByProp = MfllProperty::readProperty
                (Property_PostNrRefine);

            if (CC_UNLIKELY( _enablePostNrRefineByProp >= 0 )) {
                _enablePostNrRefine = _enablePostNrRefineByProp;
            }

            if (CC_LIKELY( _enablePostNrRefine > 0 )) {
                mfllLogD3("%s: enable postNR refine", __FUNCTION__);
                mfllTraceBegin(sStrRefineReg);
                bss_refine_postnr_regs(
                        static_cast<void*>(tuning_reg.get()),
                        real_blend_cnt,
                        expected_blend_cnt);
                mfllTraceEnd(); // RefineReg
            }
            else {
                mfllLogD("%s: disable postNR refine", __FUNCTION__);
            }

            mfllLogD3("%s: get tuning data, reg=%p, lsc=%p",
                    __FUNCTION__, rTuningParam.pRegBuf, rTuningParam.pLsc2Buf);

            // refine register
            pltcfg[ePLATFORMCFG_DIP_X_REG_T] =
                    reinterpret_cast<intptr_t>((volatile void*)rTuningParam.pRegBuf);

            mfllTraceBegin(sStrRefineReg);
            if (!refine_register(pltcfg)) {
                mfllLogE("%s: refine_register returns failed", __FUNCTION__);
            }
            else {
#ifdef __DEBUG
                mfllLogD("%s: refine_register ok", __FUNCTION__);
#endif
            }
            mfllTraceEnd(); // RefineReg

            if (m_bExifDumpped[STAGE_MIX] == 0) {
                dump_exif_info(m_pCore, tuning_reg.get(), STAGE_MIX);
                m_bExifDumpped[STAGE_MIX] = 1;
            }

#ifdef __DEBUG
            debug_pass2_registers(tuning_reg.get(), STAGE_MIX);
#endif
            params.mTuningData = static_cast<MVOID*>(tuning_reg.get()); // adding tuning data
            mfllTraceEnd(); // AfterSetIsp
        }
        else {
            mfllLogE("%s: set ISP profile failed...", __FUNCTION__);
            err = MfllErr_UnexpectedError;
            return err;
        }
        mfllTraceEnd(); // setIsp
        mfllTraceBegin(sStrBeforeP2Enque);

        _tagIspP2InFmtRestorer = nullptr; // trigger restore MTK_ISP_P2_IN_IMG_FMT

        // input: blended frame [IMGI port]
        {
            Input p;
            p.mPortID       = MIX_PORT_IN_BASE_FRAME; // should be IMGI port
            p.mPortID.group = 0;
            p.mBuffer       = img_src;
            params.mvIn.push_back(p);

            /* cropping info */
            // Even though no cropping necessary, but we have to pass in a crop info
            // with group id = 1 because pass 2 driver MUST need it
            MCrpRsInfo crop;
            crop.mGroupID       = 1;
            crop.mCropRect.p_integral.x = 0; // position pixel, in integer
            crop.mCropRect.p_integral.y = 0;
            crop.mCropRect.p_fractional.x = 0; // always be 0
            crop.mCropRect.p_fractional.y = 0;
            crop.mCropRect.s.w  = img_src->getImgSize().w;
            crop.mCropRect.s.h  = img_src->getImgSize().h;
            crop.mResizeDst.w   = img_src->getImgSize().w;
            crop.mResizeDst.h   = img_src->getImgSize().h;
            crop.mFrameGroup    = 0;
            params.mvCropRsInfo.push_back(crop);
        }

        // if disabled MIX3, we don't need golden frame
        // input: golden frame
        if (pltcfg[ePLATFORMCFG_ENABLE_MIX3] && pltcfg[ePLATFORMCFG_REFINE_OK])
        {
            Input p;
            p.mPortID       = MIX_PORT_IN_GOLDEN_FRAME;
            p.mPortID.group = 0;
            p.mBuffer       = img_ref;
            params.mvIn.push_back(p);
            /* this port is not support crop info */
        }
        else
        {
            mfllLogD("%s: no golden frame due to disabled MIX3", __FUNCTION__);
        }

        // input: weighting
        if (pltcfg[ePLATFORMCFG_ENABLE_MIX3] && pltcfg[ePLATFORMCFG_REFINE_OK])
        {
            Input p;
            p.mPortID       = MIX_PORT_IN_WEIGHTING;
            p.mPortID.group = 0;
            p.mBuffer       = img_wt;
            params.mvIn.push_back(p);
            /* this port is not support crop info */
        }
        else
        {
            mfllLogD("%s: no weighting input due to disabled MIX3", __FUNCTION__);
        }

        // output: main yuv
        {
            mfllLogD3("config main yuv during mixing");
            Output p;
            p.mBuffer          = img_dst;
            p.mPortID          = MIX_PORT_OUT_MAIN;
            p.mPortID.group    = 0; // always be 0
            params.mvOut.push_back(p);

            /**
             *  Cropping info, only works with input port is IMGI port.
             *  mGroupID should be the index of the MCrpRsInfo.
             */
            MCrpRsInfo crop;
            crop.mGroupID       = MIX_MAIN_GID_OUT;
            crop.mCropRect.p_integral.x = 0; // position pixel, in integer
            crop.mCropRect.p_integral.y = 0;
            crop.mCropRect.p_fractional.x = 0; // always be 0
            crop.mCropRect.p_fractional.y = 0;
            crop.mCropRect.s.w  = img_src->getImgSize().w;
            crop.mCropRect.s.h  = img_src->getImgSize().h;
            crop.mResizeDst.w   = img_src->getImgSize().w;
            crop.mResizeDst.h   = img_src->getImgSize().h;
            crop.mFrameGroup    = 0;
            params.mvCropRsInfo.push_back(crop);

            mfllLogD3("main: srcCrop (x,y,w,h)=(%d,%d,%d,%d)",
                    crop.mCropRect.p_integral.x, crop.mCropRect.p_integral.y,
                    crop.mCropRect.s.w, crop.mCropRect.s.h);
            mfllLogD3("main: dstSize (w,h)=(%d,%d)",
                    crop.mResizeDst.w, crop.mResizeDst.h);

            /* ask MDP to generate CA-LTM histogram */
            bool bEnCALTM = !!gSupportDRE;
            bool bIsMdpPort = (p.mPortID == PORT_WDMAO) || (p.mPortID == PORT_WROTO);
            if (bEnCALTM && bIsMdpPort ) {
                bool bGeneratedMdpParam = generateMdpDrePQParam(
                        mdpPqDre,
                        m_pCore->getCurrentIso(),
                        m_sensorId,
                        &m_vMetaSet[0].halMeta,// use the first metadata for DRE, due to
                                               // others use main metadata to apply DRE.
                        profile
                        );
                if ( ! bGeneratedMdpParam ) {
                    mfllLogE("%s: generate DRE DRE param failed", __FUNCTION__);
                }
                else {
                    mfllLogD3("%s: generate DRE histogram.", __FUNCTION__);
                    attachMDPPQParam2Pass2drv(p.mPortID, &params, &p2PqParam, &mdpPqDre);
                }
            }
            else {
                mfllLogD3("%s: DRE disabled or port is not MDP port", __FUNCTION__);
            }
        }

        // output: thumbnail yuv
        if (img_thumb &&  output_thumb_crop.size() > 0) {
            mfllLogD3("config thumnail during mixing");
            Output p;
            p.mBuffer          = img_thumb;
            p.mPortID          = MIX_PORT_OUT_THUMBNAIL;
            p.mPortID.group    = 0; // always be 0
            params.mvOut.push_back(p);

            /**
             *  Check resolution between input and output, if the ratio is different,
             *  a cropping window should be applied.
             */

            MRect srcCrop = MRect(
                    MPoint(output_thumb_crop.x, output_thumb_crop.y),
                    MSize(output_thumb_crop.w, output_thumb_crop.h) );
            srcCrop = calCrop(
                    srcCrop,
                    MRect(MPoint(0,0), img_thumb->getImgSize()),
                    100);

            /**
             *  Cropping info, only works with input port is IMGI port.
             *  mGroupID should be the index of the MCrpRsInfo.
             */
            MCrpRsInfo crop;
            crop.mGroupID       = MIX_THUMBNAIL_GID_OUT;
            crop.mCropRect.p_integral.x = srcCrop.p.x; // position pixel, in integer
            crop.mCropRect.p_integral.y = srcCrop.p.y;
            crop.mCropRect.p_fractional.x = 0; // always be 0
            crop.mCropRect.p_fractional.y = 0;
            crop.mCropRect.s.w  = srcCrop.s.w;
            crop.mCropRect.s.h  = srcCrop.s.h;
            crop.mResizeDst.w   = img_thumb->getImgSize().w;
            crop.mResizeDst.h   = img_thumb->getImgSize().h;
            crop.mFrameGroup    = 0;
            params.mvCropRsInfo.push_back(crop);

            mfllLogD3("thumnail: srcCrop (x,y,w,h)=(%d,%d,%d,%d)",
                    crop.mCropRect.p_integral.x, crop.mCropRect.p_integral.y,
                    crop.mCropRect.s.w, crop.mCropRect.s.h);
            mfllLogD3("thumbnail: dstSize (w,h)=(%d,%d)",
                    crop.mResizeDst.w, crop.mResizeDst.h);

            /* ask MDP to generate CA-LTM histogram */
            bool bEnCALTM = !!gSupportDRE;
            bool bIsMdpPort = (p.mPortID == PORT_WDMAO) || (p.mPortID == PORT_WROTO);
            if (bEnCALTM && bIsMdpPort ) {
                bool bGeneratedMdpParam = generateMdpDrePQParam(
                        mdpPqDre,
                        m_pCore->getCurrentIso(),
                        m_sensorId,
                        &m_vMetaSet[0].halMeta,// use the first metadata for DRE, due to
                                               // others use main metadata to apply DRE.
                        profile
                        );
                if ( ! bGeneratedMdpParam ) {
                    mfllLogE("%s: generate DRE DRE param failed", __FUNCTION__);
                }
                else {
                    mfllLogD3("%s: generate DRE histogram.", __FUNCTION__);
                    attachMDPPQParam2Pass2drv(p.mPortID, &params, &p2PqParam, &mdpPqDre);
                }
            }
            else {
                mfllLogD3("%s: DRE disabled or port is not MDP port", __FUNCTION__);
            }
        }
        mfllTraceEnd(); // BeforeP2Enque

        Mutex::Autolock _l(gMutexPass2Lock);

        /**
         *  Finally, we're going to invoke P2 driver to encode Raw. Notice that,
         *  we must use async function call to trigger P2 driver, which means
         *  that we have to give a callback function to P2 driver.
         *
         *  E.g.:
         *      QParams::mpfnCallback = CALLBACK_FUNCTION
         *      QParams::mpCookie --> argument that CALLBACK_FUNCTION will get
         *
         *  Due to we wanna make the P2 driver as a synchronized flow, here we
         *  simply using Mutex to sync async call of P2 driver.
         */
        using MfllMfb_Imp::P2Cookie;
        P2Cookie cookie;
        //
        qParams.mpfnCallback        = __P2Cb<QParams>;
        qParams.mpfnEnQFailCallback = __P2FailCb<QParams>;
        qParams.mpCookie        = reinterpret_cast<void*>(&cookie);
        qParams.mvFrameParams   .push_back(params);
        //
        mfllLogD3("%s: P2 enque", __FUNCTION__);
        mfllLogI("%s: mpCookie=%p", __FUNCTION__, qParams.mpCookie);
        mfllTraceBegin(sStrP2);
        if (CC_UNLIKELY( ! m_pNormalStream->enque(qParams) )) {
            mfllLogE("%s: pass 2 enque returns fail", __FUNCTION__);
            err = MfllErr_UnexpectedError;
            mfllTraceEnd();
            goto lbExit;
        }

        // to synchronize qParams
        std::atomic_thread_fence(std::memory_order_release);

        /* wait unitl P2 deque */
        mfllLogI("%s: wait P2(MIX) deque [+]", __FUNCTION__);
        {
            std::unique_lock<std::mutex> lk(cookie.__mtx);
            if (0 == cookie.__signaled.load(std::memory_order_seq_cst)) {
                /* wait for a second */
                auto status = cookie.__cv.wait_for(lk, std::chrono::milliseconds(MFLL_MFB_ENQUE_TIMEOUT_MS));
                if (CC_UNLIKELY( status == std::cv_status::timeout )) {
                    std::string _log = std::string("FATAL: wait timeout of dequing from MIX: ") + \
                                       std::to_string(MFLL_MFB_ENQUE_TIMEOUT_MS) + std::string("ms");
                    AEE_ASSERT_MFB(_log.c_str());
                }
            }
        }
        mfllLogI("%s: wait P2(MIX) deque [-]", __FUNCTION__);
        mfllTraceEnd();
    }

lbExit:
    return err;
}
//
// ----------------------------------------------------------------------------
//
enum MfllErr MfllMfb::setSyncPrivateData(const std::deque<void*>& dataset)
{
    mfllLogD3("dataset size=%zu", dataset.size());
    if (dataset.size() <= 0) {
        mfllLogW("set sync private data receieve NULL, ignored");
        return MfllErr_Ok;
    }

    Mutex::Autolock _l(m_mutex);

    /**
     *  dataset is supposed to be contained two address, the first one is the
     *  address of App IMetadata, the second one is Hal. Please make sure the
     *  caller also send IMetadata addresses.
     */
    if (dataset.size() < 3) {
        mfllLogE(
                "%s: missed data in dataset, dataset size is %zu, " \
                "and it's supposed to be: "                         \
                "[0]: addr of app metadata, "                       \
                "[1]: addr of hal metadata, "                       \
                "[2]: addr of an IImageBuffer of LCSO",
                __FUNCTION__, dataset.size());
        return MfllErr_BadArgument;
    }
    IMetadata *pAppMeta = static_cast<IMetadata*>(dataset[0]);
    IMetadata *pHalMeta = static_cast<IMetadata*>(dataset[1]);
    IImageBuffer *pLcsoImg = static_cast<IImageBuffer*>(dataset[2]);

    MetaSet_T m;
    m.halMeta = *pHalMeta;
    m.appMeta = *pAppMeta;
    m_vMetaSet.push_back(m);
    m_vMetaApp.push_back(pAppMeta);
    m_vMetaHal.push_back(pHalMeta);
    m_vImgLcsoRaw.push_back(pLcsoImg);
    mfllLogD3("saves MetaSet_T, size = %zu", m_vMetaSet.size());
    return MfllErr_Ok;
}

/******************************************************************************
 * encodeRawToYuv
 *
 * Interface for encoding a RAW buffer to an YUV buffer
 *****************************************************************************/
enum MfllErr MfllMfb::encodeRawToYuv(IMfllImageBuffer *input, IMfllImageBuffer *output, const enum YuvStage &s)
{
    return encodeRawToYuv(
            input,
            output,
            NULL,
            MfllRect_t(),
            MfllRect_t(),
            s);
}

/******************************************************************************
 * encodeRawToYuv
 *
 * Interface for encoding a RAW buffer to two YUV buffers
 *****************************************************************************/
enum MfllErr MfllMfb::encodeRawToYuv(
            IMfllImageBuffer *input,
            IMfllImageBuffer *output,
            IMfllImageBuffer *output_q,
            const MfllRect_t& output_crop,
            const MfllRect_t& output_q_crop,
            enum YuvStage s /* = YuvStage_RawToYuy2 */)
{
    enum MfllErr err = MfllErr_Ok;
    IImageBuffer *iimgOut2 = NULL;

    if (input == NULL) {
        mfllLogE("%s: input buffer is NULL", __FUNCTION__);
        err = MfllErr_BadArgument;
    }
    if (output == NULL) {
        mfllLogE("%s: output buffer is NULL", __FUNCTION__);
        err = MfllErr_BadArgument;
    }
    if (err != MfllErr_Ok)
        goto lbExit;

    if (output_q) {
        iimgOut2 = (IImageBuffer*)output_q->getImageBuffer();
    }

    err = encodeRawToYuv(
            (IImageBuffer*)input->getImageBuffer(),
            (IImageBuffer*)output->getImageBuffer(),
            iimgOut2,
            NULL, // dst3
            output_crop,
            output_q_crop,
            output_q_crop,
            s);

lbExit:
    return err;

}

/******************************************************************************
 * encodeRawToYuv 3
 *
 * Interface for encoding a RAW buffer to two 3 YUV buffers
 *****************************************************************************/
enum MfllErr MfllMfb::encodeRawToYuv(
            IMfllImageBuffer *input,
            IMfllImageBuffer *output,
            IMfllImageBuffer *output2,
            IMfllImageBuffer *output3,
            const MfllRect_t& output2_crop,
            const MfllRect_t& output3_crop,
            enum YuvStage s /* = YuvStage_RawToYuy2 */)
{
    enum MfllErr err = MfllErr_Ok;

    if (input == NULL || output == NULL || output2 == NULL) {
        mfllLogE("%s: input or output buffer is NULL", __FUNCTION__);
        return MfllErr_BadArgument;
    }

    if (input->getWidth() != output->getWidth() ||
        input->getHeight() != output->getHeight()) {
        mfllLogD("%s: the size between input and output is not the same", __FUNCTION__);
    }


    MfllRect_t r;

    err = encodeRawToYuv(
            (IImageBuffer*)input    ->getImageBuffer(),
            (IImageBuffer*)output   ->getImageBuffer(),
            (IImageBuffer*)output2  ->getImageBuffer(),
            output3 ? (IImageBuffer*)output3  ->getImageBuffer() : NULL,
            r,
            output2_crop,
            output3_crop,
            s);

    return err;
}
/******************************************************************************
 * encodeRawToYuv
 *
 * Real implmentation that to control PASS 2 drvier for encoding RAW to YUV
 *****************************************************************************/
enum MfllErr MfllMfb::encodeRawToYuv(
        IImageBuffer *src,
        IImageBuffer *dst,
        IImageBuffer *dst2,
        IImageBuffer *dst3,
        const MfllRect_t& output_crop,
        const MfllRect_t& output_q_crop,
        const MfllRect_t& output_3_crop,
        const enum YuvStage &s)
{
    enum MfllErr err = MfllErr_Ok;
    MBOOL bRet = MTRUE;
    EIspProfile_T profile = get_isp_profile(eMfllIspProfile_BeforeBlend, m_shotMode);

    mfllAutoLogFunc();
    mfllTraceCall();

    /* If it's encoding base RAW ... */
    const bool bBaseYuvStage = (s == YuvStage_BaseYuy2 || s == YuvStage_GoldenYuy2) ? true : false;

    m_mutex.lock();
    int index = m_encodeYuvCount++; // describes un-sorted index
    int sensorId = m_sensorId;
    IImageBuffer *pInput = src;
    IImageBuffer *pOutput = dst;
    IImageBuffer *pOutputQ = dst2;
    IImageBuffer *pOutput3 = dst3;
    IImageBuffer *pImgLcsoRaw = nullptr;
    /* Do not increase YUV stage if it's encoding base YUV or golden YUV */
    if (bBaseYuvStage) {
        m_encodeYuvCount--;
        index = m_pCore->getIndexByNewIndex(0); // Use the first metadata
        pImgLcsoRaw = m_vImgLcsoRaw[index]; // use the mapped LCSO buffer
    }
    else {
        /* RAW may be sorted, retrieve the new order by index */
        pImgLcsoRaw = m_vImgLcsoRaw[m_pCore->getIndexByNewIndex(index)];
    }

    /* check if metadata is ok */
    if ((size_t)index >= m_vMetaSet.size()) {
        mfllLogE("%s: index(%d) is out of metadata set size(%zu) ",
                __FUNCTION__,
                index,
                m_vMetaSet.size());
        m_mutex.unlock();
        return MfllErr_UnexpectedError;
    }
    MetaSet_T metaset = getMainMetasetLocked(); // uses main frame's metadata set.
    IMetadata *pHalMeta = getMainHalMetaLocked(); // uses main frame's HAL metadata.
    m_mutex.unlock();

    bool bIsYuv2Yuv = !isRawFormat(static_cast<EImageFormat>(pInput->getImgFormat()));

    /**
     *  Select profile based on Stage:
     *  1) Raw to Yv16
     *  2) Encoding base YUV
     *  3) Encoding golden YUV
     *
     */
    mfllTraceBegin("get_isp_profile");
    switch(s) {
    case YuvStage_RawToYuy2:
    case YuvStage_RawToYv16:
    case YuvStage_BaseYuy2:
        if (isZsdMode(m_shotMode)) {
            profile = get_isp_profile(eMfllIspProfile_BeforeBlend_Zsd, m_shotMode);// Not related with MNR
            if (IS_WRONG_ISP_PROFILE(profile)) {
                mfllLogE("%s: error isp profile mapping", __FUNCTION__);
            }
        }
        else {
            profile = get_isp_profile(eMfllIspProfile_BeforeBlend, m_shotMode);
            if (IS_WRONG_ISP_PROFILE(profile)) {
                mfllLogE("%s: error isp profile mapping", __FUNCTION__);
            }
        }
        break;
    case YuvStage_GoldenYuy2:
        if (isZsdMode(m_shotMode)) {
            profile = (m_nrType == NoiseReductionType_SWNR)
                ? get_isp_profile(eMfllIspProfile_Single_Zsd_Swnr, m_shotMode)
                : get_isp_profile(eMfllIspProfile_Single_Zsd, m_shotMode);
            if (IS_WRONG_ISP_PROFILE(profile)) {
                mfllLogE("%s: error isp profile mapping", __FUNCTION__);
            }
        }
        else {
            profile = (m_nrType == NoiseReductionType_SWNR)
                ? get_isp_profile(eMfllIspProfile_Single_Swnr, m_shotMode)
                : get_isp_profile(eMfllIspProfile_Single, m_shotMode);
            if (IS_WRONG_ISP_PROFILE(profile)) {
                mfllLogE("%s: error isp profile mapping", __FUNCTION__);
            }
        }
        break;
    default:
        // do nothing
        break;
    } // switch
    mfllTraceEnd(); // get_isp_profile

    /* query input/output port mapping */
    MfllMfbPortInfoMap portMap;
    if (!get_port_map(m_pCore, s, portMap)) {
        mfllLogE("%s: cannot get correct port mapping for stage(%d)", __FUNCTION__, s);
        return MfllErr_UnexpectedError;
    }
    else{
        if(portMap.inputPorts.size() < 1) {
            mfllLogE("%s: input port map size(%zu) is invalid for stage(%d)",
                     __FUNCTION__, portMap.inputPorts.size(), s);
            return MfllErr_UnexpectedError;
        }

        if(portMap.outputPorts.size() < 3) {
            mfllLogE("%s: output port map size(%zu) is invalid for stage(%d)",
                     __FUNCTION__, portMap.outputPorts.size(), s);
            return MfllErr_UnexpectedError;
        }
    }


    /**
     *  Ok, we're going to configure P2 driver
     */
    QParams     qParams;
    FrameParams params;
    PQParam     p2PqParam;
    std::unique_ptr<_SRZ_SIZE_INFO_> srzParam(new _SRZ_SIZE_INFO_); // for SRZ4


    /* If using ZSD mode, to use Vss profile can improve performance */
    params.mStreamTag = (isZsdMode(m_shotMode)
            ? NSCam::NSIoPipe::NSPostProc::ENormalStreamTag_Vss
            : NSCam::NSIoPipe::NSPostProc::ENormalStreamTag_Normal
            );

    /* get ready to operate P2 driver, it's a long stroy ... */
    {
        std::unique_ptr<char[]>& tuning_reg = m_tuningBuf.data;


        MUINT8 ispP2TuningUpdateMode = 0;
        {
            if (IMetadata::getEntry<MUINT8>(&metaset.halMeta, MTK_ISP_P2_TUNING_UPDATE_MODE, ispP2TuningUpdateMode)) {
                mfllLogD3("%s: ISP P2 Tuning Update Mode=%#x", __FUNCTION__, ispP2TuningUpdateMode);
            }
            else {
                mfllLogW("%s: ISP P2 Tuning Update Mode is nullptr, ISP manager is supposed to use default one", __FUNCTION__);
            }
        }

        // Fix BFBLD parameter
        if (CC_LIKELY( !bBaseYuvStage )) {
            if (CC_LIKELY( m_tuningBuf.isInited )) {
                // MTK_ISP_P2_TUNING_UPDATE_MODE = 2: keep all existed parameters (force mode)
                IMetadata::setEntry<MUINT8>(&metaset.halMeta, MTK_ISP_P2_TUNING_UPDATE_MODE, 2);
                mfllLogD("%s: set MTK_ISP_P2_TUNING_UPDATE_MODE to 2", __FUNCTION__);
            }
            else {
                mfllLogD("%s: no need to set MTK_ISP_P2_TUNING_UPDATE_MODE since m_tuningBuf not inited", __FUNCTION__);
            }
            m_tuningBuf.isInited = true;
        }

        void *tuning_lsc;

        MfbPlatformConfig pltcfg;
        pltcfg[ePLATFORMCFG_STAGE] = [s]
        {
            switch (s) {
            case YuvStage_RawToYuy2:
            case YuvStage_RawToYv16:
            case YuvStage_BaseYuy2:     return STAGE_BASE_YUV;
            case YuvStage_GoldenYuy2:   return STAGE_GOLDEN_YUV;
            case YuvStage_Unknown:
            case YuvStage_Size:
                mfllLogE("Not support YuvStage %d", s);
            }
            return 0;
        }();
        pltcfg[ePLATFORMCFG_INDEX] = index;

        {
            TuningParam rTuningParam;
            rTuningParam.pRegBuf = tuning_reg.get();
            rTuningParam.pLcsBuf = pImgLcsoRaw; // gives LCS buffer, even though it's NULL.
            rTuningParam.pBpc2Buf= m_pBPCI; // always use the previouse BPCI buffer
            IMetadata::setEntry<MUINT8>(
                    &metaset.halMeta,
                    MTK_3A_PGN_ENABLE,
                    MTRUE);

            /* makes .tuning different */
            IMetadata::setEntry<MINT32>(
                    &metaset.halMeta,
                    MTK_PIPELINE_REQUEST_NUMBER,
                    index);

            // if shot mode is single frame, do not use MFNR related ISP profile
            mfllLogD3("%s: shotMode=%#x", __FUNCTION__, m_shotMode);
            if (m_shotMode & (1 << MfllMode_Bit_SingleFrame))
                mfllLogD("%s: do not use Mfll related ISP profile for SF",__FUNCTION__);
            else
                IMetadata::setEntry<MUINT8>(&metaset.halMeta, MTK_3A_ISP_PROFILE, profile);

            // tell ISP manager that this stage is YUV->YUV
            std::unique_ptr<void, std::function<void(void*)> > _tagIspP2InFmtRestorer;
            MINT32 tagIspP2InFmt = 0;
            if (bIsYuv2Yuv) {
                tagIspP2InFmt = setIspP2ImgFmtTag(metaset.halMeta, 1, &_tagIspP2InFmtRestorer);
            }

            // debug {{{
            {
                MUINT8 __profile = 0;
                if (IMetadata::getEntry<MUINT8>(&metaset.halMeta, MTK_3A_ISP_PROFILE, __profile)) {
                    mfllLogD3("%s: isp profile=%#x", __FUNCTION__, __profile);
                }
                else {
                    mfllLogW("%s: isp profile is nullptr, ISP manager is supposed to use default one", __FUNCTION__);
                }
            }
            // }}}

            MetaSet_T rMetaSet;

            mfllTraceBegin(sStrSetIsp);
            auto ret_setIsp = this->setIsp(0, metaset, &rTuningParam, &rMetaSet, pInput);
            if (CC_LIKELY( m_tuningBuf.isInited )) {
                IMetadata::setEntry<MUINT8>(&metaset.halMeta, MTK_ISP_P2_TUNING_UPDATE_MODE, ispP2TuningUpdateMode);
                IMetadata::setEntry<MUINT8>(&rMetaSet.halMeta, MTK_ISP_P2_TUNING_UPDATE_MODE, ispP2TuningUpdateMode);
            }
            // check rTuningParam.pBpc2Buf first
            if (__builtin_expect( rTuningParam.pBpc2Buf == nullptr, false )) {
                mfllLogW("%s: rTuningParam.pBpc2Buf is null", __FUNCTION__);
            }

            // update BPC buffer address at the first time
            if (m_pBPCI == nullptr)
                m_pBPCI = rTuningParam.pBpc2Buf;

            if (ret_setIsp == 0) {

                mfllLogD3("%s: get tuning data, reg=%p, lsc=%p",
                        __FUNCTION__, rTuningParam.pRegBuf, rTuningParam.pLsc2Buf);

                // restore MTK_ISP_P2_IN_IMG_FMT first
                IMetadata::setEntry<MINT32>(&rMetaSet.halMeta, MTK_ISP_P2_IN_IMG_FMT, tagIspP2InFmt);

                // refine register
                pltcfg[ePLATFORMCFG_DIP_X_REG_T] =
                    reinterpret_cast<intptr_t>((volatile void*)rTuningParam.pRegBuf);

                mfllTraceBegin(sStrRefineReg);
                if (!refine_register(pltcfg)) {
                    mfllLogE("%s: refine_register returns fail", __FUNCTION__);
                }
                else {
#ifdef __DEBUG
                    mfllLogD("%s: refine_register ok", __FUNCTION__);
#endif
                }
                mfllTraceEnd(); // sStrRefineReg

                // update Exif info
                switch (s) {
                case YuvStage_BaseYuy2: // stage bfbld
                    if (m_bExifDumpped[STAGE_BASE_YUV] == 0) {
                        dump_exif_info(m_pCore, tuning_reg.get(), STAGE_BASE_YUV);
                        m_bExifDumpped[STAGE_BASE_YUV] = 1;
                    }
                    break;

                case YuvStage_RawToYuy2: // stage bfbld
                case YuvStage_RawToYv16: // stage bfbld
                    if (m_bExifDumpped[STAGE_BASE_YUV] == 0) {
                        dump_exif_info(m_pCore, tuning_reg.get(), STAGE_BASE_YUV);
                        m_bExifDumpped[STAGE_BASE_YUV] = 1;
                        /* update metadata within the one given by IHal3A only in stage bfbld only once */
                        *pHalMeta += rMetaSet.halMeta;
                    }
                    break;

                case YuvStage_GoldenYuy2: // stage sf
                    if (m_bExifDumpped[STAGE_GOLDEN_YUV] == 0) {
                        dump_exif_info(m_pCore, tuning_reg.get(), STAGE_GOLDEN_YUV);
                        m_bExifDumpped[STAGE_GOLDEN_YUV] = 1;
                    }
                    // if single frame, update metadata
                    if (m_shotMode & (1 << MfllMode_Bit_SingleFrame))
                        *pHalMeta += rMetaSet.halMeta;

                    break;

                default:
                    // do nothing
                    break;
                }

                params.mTuningData = static_cast<MVOID*>(tuning_reg.get()); // adding tuning data

                /* LSC tuning data is constructed as an IImageBuffer, and we don't need to release */
                IImageBuffer* pSrc = static_cast<IImageBuffer*>(rTuningParam.pLsc2Buf);
                if ((bIsYuv2Yuv == false) && (pSrc != NULL)) {
                    Input __src;
                    __src.mPortID         = PORT_IMGCI;
                    __src.mPortID.group   = 0;
                    __src.mBuffer         = pSrc;
                    params.mvIn.push_back(__src);
                }
                else {
                    // check if it's process RAW, if yes, there's no LSC.
                    MINT32 __rawType = NSIspTuning::ERawType_Pure;
                    if (!IMetadata::getEntry<MINT32>(
                                &metaset.halMeta,
                                MTK_P1NODE_RAW_TYPE,
                                __rawType)) {
                        mfllLogW("get p1 node RAW type fail, assume it's pure RAW");
                    }

                    // check if LSC table should exists or not.
                    if (__rawType == NSIspTuning::ERawType_Proc) {
                        mfllLogD3("process RAW, no LSC table");
                    }
                    else if (bIsYuv2Yuv) {
                        mfllLogD3("Yuv->Yuv, no LSC table");
                    }
                    else {
                        mfllLogE("create LSC image buffer fail");
                        err = MfllErr_UnexpectedError;
                    }
                }

                IImageBuffer* pBpcBuf = static_cast<IImageBuffer*>(rTuningParam.pBpc2Buf);
                if ((bIsYuv2Yuv == false) && (pBpcBuf != nullptr)) {
                    Input __src;
                    __src.mPortID         = PORT_IMGBI;
                    __src.mPortID.group   = 0;
                    __src.mBuffer         = pBpcBuf;
                    params.mvIn.push_back(__src);
                }
                else {
                    // BPC buffer may be NULL even for pure RAW processing
                    mfllLogI("No need set Bpc buffer if No BPC buffer(%p) or It's yuv to yuv stage(%d)",
                            pBpcBuf, bIsYuv2Yuv);
                }

                // LCSO buffer may be removed by ISP manager
                // only needed while RAW->YUV
                IImageBuffer* pLcsoBuf = static_cast<IImageBuffer*>(rTuningParam.pLcsBuf);
                if ((bIsYuv2Yuv == false) && (pLcsoBuf != nullptr)) {
                    Input __src;
                    __src.mPortID         = RAW2YUV_PORT_LCE_IN;
                    __src.mPortID.group   = 0;
                    __src.mBuffer         = pLcsoBuf;
                    params.mvIn.push_back(__src);

                    /* one more port needs LCSO buffer since ISP5.0 */
                    __src.mPortID  = PORT_DEPI;
                    params.mvIn.push_back(__src);

                    /* this SRZ4 info is for PORT_DEPI */
                    ModuleInfo srz4_module;
                    srz4_module.moduleTag       = EDipModule_SRZ4;
                    srz4_module.frameGroup      = 0;
                    srzParam->in_w              = pLcsoBuf->getImgSize().w; // resolution of LCSO
                    srzParam->in_h              = pLcsoBuf->getImgSize().h; // ^^^^^^^^^^^^^^^^^^
                    srzParam->crop_floatX       = 0;
                    srzParam->crop_floatY       = 0;
                    srzParam->crop_x            = 0;
                    srzParam->crop_y            = 0;
                    srzParam->crop_w            = pLcsoBuf->getImgSize().w; // resolution of LCSO
                    srzParam->crop_h            = pLcsoBuf->getImgSize().h; // ^^^^^^^^^^^^^^^^^^
                    srzParam->out_w             = pInput->getImgSize().w; // input raw width
                    srzParam->out_h             = pInput->getImgSize().h; // input raw height
                    srz4_module.moduleStruct    = static_cast<MVOID*>(srzParam.get());
                    //
                    params.mvModuleData.push_back(srz4_module);

                    /* no cropping info need */
                    mfllLogD3("enable LCEI port for LCSO buffer, size = %d x %d",
                            pImgLcsoRaw->getImgSize().w,
                            pImgLcsoRaw->getImgSize().h);
                }

            }
            else {
                mfllLogE("%s: set ISP profile failed...", __FUNCTION__);
                err = MfllErr_UnexpectedError;
                goto lbExit;
            }
            mfllTraceEnd(); // sStrSetIsp

            // restore MTK_ISP_P2_IN_IMG_FMT
            _tagIspP2InFmtRestorer = nullptr;
        }

        DpPqParam   mdpPqDump = generateMdpDumpPQParam(&metaset.halMeta);

        mfllTraceBegin(sStrBeforeP2Enque);
        /* input: source frame [IMGI port] */
        {
            Input p;
            p.mBuffer          = pInput;
            p.mPortID          = portMap.inputPorts[0].portId;
            p.mPortID.group    = 0; // always be 0
            params.mvIn.push_back(p);
            mfllLogI("input: va/pa(%p/%p) fmt(%#x), size=(%d,%d), srcCrop(x,y,w,h)=(%d,%d,%d,%d), YuvStage=(%d)",
                    // va/pa
                    (void*)pInput->getBufVA(0), (void*)pInput->getBufPA(0),
                    // fmt
                    pInput->getImgFormat(),
                    // size
                    pInput->getImgSize().w, pInput->getImgSize().h,
                    // srcCrop
                    0, 0,
                    pInput->getImgSize().w, pInput->getImgSize().h,
                    // Yuv Stage
                    s);
        }

        /* output: destination frame [WDMAO port] */
        {
            Output p;
            p.mBuffer          = pOutput;
            p.mPortID          = portMap.outputPorts[0].portId;
            p.mPortID.group    = 0; // always be 0
            params.mvOut.push_back(p);

            /**
             *  Check resolution between input and output, if the ratio is different,
             *  a cropping window should be applied.
             */
            MRect srcCrop =
                (
                 output_crop.size() <= 0
                 // original size of source
                 ? MRect(MPoint(0, 0), pInput->getImgSize())
                 // user specified cropping window
                 : MRect(MPoint(output_crop.x, output_crop.y),
                     MSize(output_crop.w, output_crop.h))
                );

            srcCrop = calCrop(
                    srcCrop,
                    MRect(MPoint(0,0), pOutput->getImgSize()),
                    100);

            /**
             *  Cropping info, only works with input port is IMGI port.
             *  mGroupID should be the index of the MCrpRsInfo.
             */
            MCrpRsInfo crop;
            crop.mGroupID       = portMap.outputPorts[0].groupId;
            crop.mCropRect.p_integral.x = srcCrop.p.x; // position pixel, in integer
            crop.mCropRect.p_integral.y = srcCrop.p.y;
            crop.mCropRect.p_fractional.x = 0; // always be 0
            crop.mCropRect.p_fractional.y = 0;
            crop.mCropRect.s.w  = srcCrop.s.w;
            crop.mCropRect.s.h  = srcCrop.s.h;
            crop.mResizeDst.w   = pOutput->getImgSize().w;
            crop.mResizeDst.h   = pOutput->getImgSize().h;
            crop.mFrameGroup    = 0;
            params.mvCropRsInfo.push_back(crop);
            mfllLogI("output: va/pa(%p/%p) fmt(%#x), size=(%d,%d), srcCrop(x,y,w,h)=(%d,%d,%d,%d), dstSize(w,h)=(%d,%d), YuvStage=(%d)",
                    // va/pa
                    (void*)pOutput->getBufVA(0), (void*)pOutput->getBufPA(0),
                    // fmt
                    pOutput->getImgFormat(),
                    // size
                    pOutput->getImgSize().w, pOutput->getImgSize().h,
                    // srcCrop
                    crop.mCropRect.p_integral.x, crop.mCropRect.p_integral.y,
                    crop.mCropRect.s.w, crop.mCropRect.s.h,
                    // dstSize
                    crop.mResizeDst.w, crop.mResizeDst.h,
                    // yuv stage
                    s);
        }

        /* output2: destination frame [WROTO prot] */
        if (pOutputQ) {
            Output p;
            p.mBuffer          = pOutputQ;
            p.mPortID          = portMap.outputPorts[1].portId;
            p.mPortID.group    = 0; // always be 0
            params.mvOut.push_back(p);

            /**
             *  Check resolution between input and output, if the ratio is different,
             *  a cropping window should be applied.
             */

            MRect srcCrop =
                (
                 output_q_crop.size() <= 0
                 // original size of source
                 ? MRect(MPoint(0, 0), pInput->getImgSize())
                 // user specified crop
                 : MRect(MPoint(output_q_crop.x, output_q_crop.y),
                     MSize(output_q_crop.w, output_q_crop.h))
                );
            srcCrop = calCrop(
                    srcCrop,
                    MRect(MPoint(0,0), pOutputQ->getImgSize()),
                    100);

            /**
             *  Cropping info, only works with input port is IMGI port.
             *  mGroupID should be the index of the MCrpRsInfo.
             */
            MCrpRsInfo crop;
            crop.mGroupID       = portMap.outputPorts[1].groupId;
            crop.mCropRect.p_integral.x = srcCrop.p.x; // position pixel, in integer
            crop.mCropRect.p_integral.y = srcCrop.p.y;
            crop.mCropRect.p_fractional.x = 0; // always be 0
            crop.mCropRect.p_fractional.y = 0;
            crop.mCropRect.s.w  = srcCrop.s.w;
            crop.mCropRect.s.h  = srcCrop.s.h;
            crop.mResizeDst.w   = pOutputQ->getImgSize().w;
            crop.mResizeDst.h   = pOutputQ->getImgSize().h;
            crop.mFrameGroup    = 0;
            params.mvCropRsInfo.push_back(crop);

            mfllLogI("output2: va/pa(%p/%p) fmt(%#x), size=(%d,%d), srcCrop(x,y,w,h)=(%d,%d,%d,%d), dstSize(w,h)=(%d,%d), YuvStage=(%d)",
                    // va/pa
                    (void*)pOutputQ->getBufVA(0), (void*)pOutputQ->getBufPA(0),
                    // fmt
                    pOutputQ->getImgFormat(),
                    // size
                    pOutputQ->getImgSize().w, pOutputQ->getImgSize().h,
                    // srcCrop
                    crop.mCropRect.p_integral.x, crop.mCropRect.p_integral.y,
                    crop.mCropRect.s.w, crop.mCropRect.s.h,
                    // dstSize
                    crop.mResizeDst.w, crop.mResizeDst.h,
                    // Yuv stage
                    s);

            /* If pOutputQ is resized Y8, dump MDP info */
            if (pOutputQ->getImgFormat() == eImgFmt_Y8) {
                attachMDPPQParam2Pass2drv(p.mPortID, &params, &p2PqParam, &mdpPqDump);
            }
        }

        /* output3: destination frame*/
        if (pOutput3) {
            Output p;
            p.mBuffer          = pOutput3;
            p.mPortID          = portMap.outputPorts[2].portId;
            p.mPortID.group    = 0; // always be 0
            params.mvOut.push_back(p);

            /**
             *  Check resolution between input and output, if the ratio is different,
             *  a cropping window should be applied.
             */

            MRect srcCrop;
            if ((output_3_crop.size() <= 0) ||
                (pInput->getImgSize().h < output_3_crop.h) ||
                (pInput->getImgSize().w < output_3_crop.w)) {
                // original size of source
                srcCrop = MRect(MPoint(0, 0), pInput->getImgSize());
                mfllLogW("output3: invalid!! crop(w,h)=(%d,%d), pInput:(w,h)=(%d,%d)",
                          output_3_crop.w, output_3_crop.h, pInput->getImgSize().w, pInput->getImgSize().h);
            }
            else {
                // user specified crop
                srcCrop = MRect(MPoint(output_3_crop.x, output_3_crop.y),
                                MSize(output_3_crop.w, output_3_crop.h));
            }
            srcCrop = calCrop(
                    srcCrop,
                    MRect(MPoint(0,0), pOutput3->getImgSize()),
                    100);

            /**
             *  Cropping info, only works with input port is IMGI port.
             *  mGroupID should be the index of the MCrpRsInfo.
             */
            MCrpRsInfo crop;
            crop.mGroupID       = portMap.outputPorts[2].groupId;
            crop.mCropRect.p_integral.x = srcCrop.p.x; // position pixel, in integer
            crop.mCropRect.p_integral.y = srcCrop.p.y;
            crop.mCropRect.p_fractional.x = 0; // always be 0
            crop.mCropRect.p_fractional.y = 0;
            crop.mCropRect.s.w  = srcCrop.s.w;
            crop.mCropRect.s.h  = srcCrop.s.h;
            crop.mResizeDst.w   = pOutput3->getImgSize().w;
            crop.mResizeDst.h   = pOutput3->getImgSize().h;
            crop.mFrameGroup    = 0;
            params.mvCropRsInfo.push_back(crop);

            mfllLogI("output3: va/pa(%p/%p) fmt(%#x), size=(%d,%d), srcCrop(x,y,w,h)=(%d,%d,%d,%d), dstSize(w,h)=(%d,%d), YuvStage=(%d)",
                    // va/pa
                    (void*)pOutput3->getBufVA(0), (void*)pOutput3->getBufPA(0),
                    // fmt
                    pOutput3->getImgFormat(),
                    // size
                    pOutput3->getImgSize().w, pOutput3->getImgSize().h,
                    // srcCrop
                    crop.mCropRect.p_integral.x, crop.mCropRect.p_integral.y,
                    crop.mCropRect.s.w, crop.mCropRect.s.h,
                    // dstSize
                    crop.mResizeDst.w, crop.mResizeDst.h,
                    // Yuv Stage
                    s);
        }
        mfllTraceEnd(); // sStrBeforeP2Enque


        Mutex::Autolock _l(gMutexPass2Lock);
        /**
         *  Finally, we're going to invoke P2 driver to encode Raw. Notice that,
         *  we must use async function call to trigger P2 driver, which means
         *  that we have to give a callback function to P2 driver.
         *
         *  E.g.:
         *      QParams::mpfnCallback = CALLBACK_FUNCTION
         *      QParams::mpCookie --> argument that CALLBACK_FUNCTION will get
         *
         *  Due to we wanna make the P2 driver as a synchronized flow, here we
         *  simply using Mutex to sync async call of P2 driver.
         */
        using MfllMfb_Imp::P2Cookie;
        P2Cookie cookie;
        //
        qParams.mpfnCallback        = __P2Cb<QParams>;
        qParams.mpfnEnQFailCallback = __P2FailCb<QParams>;
        qParams.mpCookie        = reinterpret_cast<void*>(&cookie);
        qParams.mvFrameParams   .push_back(params);
        //
        mfllLogD3("%s: P2 enque", __FUNCTION__);
        mfllLogI("%s: mpCookie=%p", __FUNCTION__, qParams.mpCookie);
        mfllTraceBegin(sStrP2);
        if (CC_UNLIKELY( ! m_pNormalStream->enque(qParams) )) {
            mfllLogE("%s: pass 2 enque fail", __FUNCTION__);
            err = MfllErr_UnexpectedError;
            mfllTraceEnd();
            goto lbExit;
        }

        // to synchronize qParams
        std::atomic_thread_fence(std::memory_order_release);

        /* wait unitl P2 deque */
        mfllLogI("%s: wait P2(BFBLD/SF) deque [+]", __FUNCTION__);
        {
            std::unique_lock<std::mutex> lk(cookie.__mtx);
            if (0 == cookie.__signaled.load(std::memory_order_seq_cst)) {
                /* wait for a second */
                auto status = cookie.__cv.wait_for(lk, std::chrono::milliseconds(MFLL_MFB_ENQUE_TIMEOUT_MS));
                if (CC_UNLIKELY( status == std::cv_status::timeout )) {

                    mfllLogE("%s: p2 deque timed out (%d ms), index=%d, yuv stage=%d, manually dump QParams:", __FUNCTION__,
                            MFLL_MFB_ENQUE_TIMEOUT_MS, index, s);

                    __P2FailCb<QParams>(qParams);


                    std::string _log = std::string("FATAL: wait timeout of dequing from BFBLD or SF: ") + \
                                       std::to_string(MFLL_MFB_ENQUE_TIMEOUT_MS) + std::string("ms");
                    AEE_ASSERT_MFB(_log.c_str());
                }
            }
        }
        mfllLogI("%s: wait P2(BFBLD/SF) deque [-]", __FUNCTION__);
        mfllTraceEnd();
    }

lbExit:
    return err;
}


enum MfllErr MfllMfb::convertYuvFormatByMdp(
        IMfllImageBuffer* input,
        IMfllImageBuffer* output,
        IMfllImageBuffer* output_q,
        const MfllRect_t& output_crop,
        const MfllRect_t& output_q_crop,
        enum YuvStage s /* = YuvStage_Unknown */)
{
    mfllAutoLog("convertYuvFormatByMdp_itr");

    if (input == nullptr || output == nullptr) {
        mfllLogE("%s: input or output image buffer is NULL", __FUNCTION__);
        return MfllErr_BadArgument;
    }

    // use full size crop if cropping window is zero
    if (output_crop.w <= 0 || output_crop.h <= 0) {
        mfllLogE("%s: cropping window is 0 (invalid)", __FUNCTION__);
        return MfllErr_BadArgument;
    }

    if (output_q && (output_q_crop.w <= 0 || output_q_crop.h <= 0)) {
        mfllLogE("%s: cropping window 2 is 0 (invalid)", __FUNCTION__);
        return MfllErr_BadArgument;
    }

    return convertYuvFormatByMdp(
            reinterpret_cast<IImageBuffer*>(input->getImageBuffer()),
            reinterpret_cast<IImageBuffer*>(output->getImageBuffer()),
            (output_q == nullptr) ? nullptr : reinterpret_cast<IImageBuffer*>(output_q->getImageBuffer()),
            output_crop,
            output_q_crop,
            s);
}


enum MfllErr MfllMfb::convertYuvFormatByMdp(
        IImageBuffer* src,
        IImageBuffer* dst,
        IImageBuffer* dst2,
        const MfllRect_t& output_crop,
        const MfllRect_t& output_q_crop,
        const enum YuvStage s)
{
    mfllAutoLogFunc();

    MfllErr err = MfllErr_Ok;
    MRect crop = MRect(MPoint(output_crop.x, output_crop.y), MSize(output_crop.w, output_crop.h));
    MRect crop2 = MRect(MPoint(output_q_crop.x, output_q_crop.y), MSize(output_q_crop.w, output_q_crop.h));

    // check arguments
    if (src == nullptr || dst == nullptr) {
        mfllLogE("%s: source or dst image is NULL", __FUNCTION__);
        return MfllErr_BadArgument;
    }

    // create IImageTransform
    std::unique_ptr<IImageTransform, std::function<void(IImageTransform*)>> t(
            IImageTransform::createInstance(),
            [](IImageTransform *p)mutable->void { if (p) p->destroyInstance(); });

    if (t.get() == nullptr) {
        mfllLogE("%s: create IImageTransform failed", __FUNCTION__);
        return MfllErr_UnexpectedError;
    }

    auto bRet = (dst2 == nullptr)
        ? t->execute(src, dst, nullptr, crop, 0, 3000)
        : t->execute(src, dst, dst2, crop, crop2, 0, 0, 3000);

    if (!bRet) {
        mfllLogE("%s: IImageTransform::execute returns fail", __FUNCTION__);
        return MfllErr_UnexpectedError;
    }

    // At this stage, we need to save Debug Exif
    bool bUpdateExifBfbld = (s == YuvStage_BaseYuy2);

    if (bUpdateExifBfbld) {
        // {{{
        auto index = m_pCore->getIndexByNewIndex(0); // Use the first metadata
        MetaSet_T metaset;
        IMetadata* pHalMeta = nullptr;
        IImageBuffer* pImgLcsoRaw = nullptr;
        EIspProfile_T profile = static_cast<EIspProfile_T>(MFLL_ISP_PROFILE_ERROR);

        // retrieve data
        {
            Mutex::Autolock __l(m_mutex);

            // check if metadata is ok
            if (static_cast<size_t>(index) >= m_vMetaSet.size()) {
                mfllLogE("%s: index(%d) is out of metadata set size(%zu) ",
                        __FUNCTION__, index, m_vMetaSet.size());
                return MfllErr_UnexpectedError;
            }

            metaset = getMainMetasetLocked();
            pHalMeta = getMainHalMetaLocked();
            pImgLcsoRaw = m_vImgLcsoRaw[index]; // get LCSO RAW
        }

        // get profile
        if (isZsdMode(m_shotMode)) {
            profile = get_isp_profile(eMfllIspProfile_BeforeBlend_Zsd, m_shotMode);// Not related with MNR
            if (IS_WRONG_ISP_PROFILE(profile)) {
                mfllLogE("%s: error isp profile mapping", __FUNCTION__);
            }
        }
        else {
            profile = get_isp_profile(eMfllIspProfile_BeforeBlend, m_shotMode);
            if (IS_WRONG_ISP_PROFILE(profile)) {
                mfllLogE("%s: error isp profile mapping", __FUNCTION__);
            }
        }

        // get size of dip_x_reg_t
        size_t regTableSize = m_pNormalStream->getRegTableSize();
        if (regTableSize <= 0) {
            mfllLogE("%s: unexpected tuning buffer size: %zu", __FUNCTION__, regTableSize);
            return MfllErr_UnexpectedError;
        }

        /* add tuning data is necessary */
        std::unique_ptr<char[]> tuning_reg(new char[regTableSize]());
        void *tuning_lsc = nullptr;
        TuningParam rTuningParam;
        rTuningParam.pRegBuf = tuning_reg.get();
        rTuningParam.pLcsBuf = pImgLcsoRaw; // gives LCS buffer, even though it's NULL.
        // PGN enable due to BFBLD stage is RAW->YUV
        IMetadata::setEntry<MUINT8>(&metaset.halMeta, MTK_3A_PGN_ENABLE, MTRUE);
        IMetadata::setEntry<MUINT8>(&metaset.halMeta, MTK_3A_ISP_PROFILE, profile);

        // Set ISP to retrieve dip_x_reg_t
        if (this->setIsp(0, metaset, &rTuningParam, nullptr, src) == 0) {
#ifdef __DEBUG
            mfllLogD("%s: get tuning data, reg=%p, lsc=%p",
                    __FUNCTION__, rTuningParam.pRegBuf, rTuningParam.pLsc2Buf);
#endif

            MfbPlatformConfig pltcfg;
            pltcfg[ePLATFORMCFG_STAGE] = STAGE_BASE_YUV;
            pltcfg[ePLATFORMCFG_INDEX] = 0;
            pltcfg[ePLATFORMCFG_DIP_X_REG_T] =
                reinterpret_cast<intptr_t>((volatile void*)rTuningParam.pRegBuf);

            if (!refine_register(pltcfg)) {
                mfllLogE("%s: refine_register returns fail", __FUNCTION__);
            }
            else {
#ifdef __DEBUG
                mfllLogD("%s: refine_register ok", __FUNCTION__);
#endif
            }
            // update Exif info
            if (m_bExifDumpped[STAGE_BASE_YUV] == 0) {
                dump_exif_info(m_pCore, tuning_reg.get(), STAGE_BASE_YUV);
                m_bExifDumpped[STAGE_BASE_YUV] = 1;
            }
        }
        else {
            mfllLogE("%s: setIsp returns fail, Exif may be lost", __FUNCTION__);
        }
        // }}}
    } // fi (YuvStage_BaseYuy2)

    return err;
}

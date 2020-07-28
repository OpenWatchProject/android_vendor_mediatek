/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 *
 * MediaTek Inc. (C) 2015. All rights reserved.
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
#include <mtkcam3/feature/mfnr/MfllLog.h>
#include <mtkcam3/feature/mfnr/MfllProperty.h>
#include "MfllFeatureDump.h"
#include "MfllOperationSync.h"

// MTKCAM
#include <mtkcam/utils/metadata/hal/mtk_platform_metadata_tag.h>
#include <mtkcam/drv/iopipe/SImager/IImageTransform.h> // IImageTransform
#include <mtkcam/drv/def/Dip_Notify_datatype.h> // _SRZ_SIZE_INFO_

// AOSP
#include <utils/Mutex.h> // android::Mutex
#include <cutils/compiler.h> //

// STL
#include <memory>

//
// To print more debug information of this module
// #define __DEBUG


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


/*
 * Mutex for pass 2 operation
 *
 * make sure pass 2 is thread-safe, basically it's not ...
 */
static Mutex& gMutexPass2Lock = *new Mutex();

/* For P2 driver callback usage */
#define P2CBPARAM_MAGICNUM 0xABC
typedef struct _p2cbParam {
    MfllMfb         *self;
    Mutex           *mutex;
    unsigned int    magicNum;
    _p2cbParam ()
    {
        self = NULL;
        mutex = NULL;
        magicNum = P2CBPARAM_MAGICNUM;
    };
    _p2cbParam(MfllMfb *s, Mutex *m)
    {
        self = s;
        mutex = m;
        magicNum = P2CBPARAM_MAGICNUM;
    }
} p2cbParam_t;

static MVOID pass2CbFunc(QParams& rParams)
{
    mfllAutoLogFunc();
    p2cbParam_t *p = reinterpret_cast<p2cbParam_t*>(rParams.mpCookie);
    /* check Magic NUM */
    if (p->magicNum != P2CBPARAM_MAGICNUM) {
        mfllLogE("%s: pass2 param is weird, magic num is different", __FUNCTION__);
    }
    p->mutex->unlock();
}

template <typename T>
inline MVOID updateEntry(IMetadata* pMetadata, MUINT32 const tag, T const& val)
{
    if (pMetadata == NULL)
    {
        mfllLogE("pMetadata is NULL");
        return;
    }

    IMetadata::IEntry entry(tag);
    entry.push_back(val, Type2Type<T>());
    pMetadata->update(tag, entry);
}

template <class T>
inline MBOOL tryGetMetaData(IMetadata *pMetadata, MUINT32 const tag, T &rVal)
{
    if (pMetadata == NULL) {
        return MFALSE;
    }

    IMetadata::IEntry entry = pMetadata->entryFor(tag);
    if (!entry.isEmpty()) {
        rVal = entry.itemAt(0, Type2Type<T>());
        return MTRUE;
    }

    return MFALSE;
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

void IMfllMfb::destroyInstance(void)
{
    decStrong((void*)this);
}

MfllMfb::MfllMfb(void)
{
    m_pNormalStream = NULL;
    m_pNormalStream_vss = NULL;
    m_p3A = NULL;
    m_shotMode = (enum MfllMode)0;
    m_syncPrivateData = NULL;
    m_syncPrivateDataSize = 0;
    m_encodeYuvCount = 0;
    m_blendCount = 0;
    m_bIsInited = false;
    m_nrType = NoiseReductionType_None;
    m_sensorId = 0;
    m_pCore = NULL;
    for (size_t i = 0; i < STAGE_SIZE; i++)
        m_bExifDumpped[i] = 0;
}

MfllMfb::~MfllMfb(void)
{
    mfllAutoLogFunc();

    if (m_pNormalStream) {
        m_pNormalStream->uninit(LOG_TAG);
        m_pNormalStream->destroyInstance();
        m_pNormalStream = NULL;
    }
    if (m_pNormalStream_vss) {
        m_pNormalStream_vss->uninit(LOG_TAG);
        m_pNormalStream_vss->destroyInstance();
        m_pNormalStream_vss = NULL;
    }

    if (m_p3A) {
        m_p3A->destroyInstance(LOG_TAG);
    }
}

enum MfllErr MfllMfb::init(int sensorId)
{
    enum MfllErr err = MfllErr_Ok;
    Mutex::Autolock _l(m_mutex);
    mfllAutoLogFunc();

    if (m_bIsInited) { // do not init twice
        goto lbExit;
    }

    mfllLogD("Init MfllMfb with sensor id --> %d", sensorId);
    m_sensorId = sensorId;

    m_pNormalStream = INormalStream::createInstance(sensorId);
    if (m_pNormalStream == NULL) {
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

    m_pNormalStream_vss = INormalStream::createInstance(sensorId);
    if (m_pNormalStream_vss == NULL) {
        mfllLogE("create INormalStream_vss fail");
        err = MfllErr_UnexpectedError;
        goto lbExit;
    }
    else {
        MBOOL bResult = m_pNormalStream_vss->init(LOG_TAG);
        if (CC_UNLIKELY(bResult == MFALSE)) {
            mfllLogE("init INormalStream_vss returns MFALSE");
            err = MfllErr_UnexpectedError;
            goto lbExit;
        }
    }

    m_p3A = MAKE_Hal3A(sensorId, LOG_TAG);
    if (m_p3A == NULL) {
        mfllLogE("create IHal3A fail");
        err = MfllErr_UnexpectedError;
        goto lbExit;
    }
    /* mark as inited */
    m_bIsInited = true;

lbExit:
    return err;
}

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
    void *privateData = m_syncPrivateData;
    size_t privateDataSize = m_syncPrivateDataSize;
    size_t index = m_pCore->getIndexByNewIndex(m_blendCount);
    m_blendCount++;
    /* get metaset */
    if (index >= m_vMetaSet.size()) {
        mfllLogE("%s: index(%zu) is out of size of metaset(%zu)", __FUNCTION__, index, m_vMetaSet.size());
        m_mutex.unlock();
        return MfllErr_UnexpectedError;
    }
    MetaSet_T metaset = m_vMetaSet[index];
    m_mutex.unlock();

    /**
     *  P A S S 2
     *
     *  Configure input parameters
     */
    QParams qParams;
    FrameParams params;

    params.mStreamTag = NSCam::NSIoPipe::NSPostProc::ENormalStreamTag_MFB_Bld;


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

    /* execute pass 2 operation */
    {
        // get size of dip_x_reg_t
        volatile size_t regTableSize = m_pNormalStream->getRegTableSize();
        if (regTableSize <= 0) {
            mfllLogE("%s: unexpected tuning buffer size: %zu", __FUNCTION__, regTableSize);
        }

        /* add tuning data is necessary */
        std::unique_ptr<char[]> tuning_reg(new char[regTableSize]());
        void *tuning_lsc;

        mfllLogD("%s: create tuning register data chunk with size %zu",
                __FUNCTION__, regTableSize);

        TuningParam rTuningParam;
        rTuningParam.pRegBuf = tuning_reg.get();

        updateEntry<MUINT8>(&metaset.halMeta, MTK_3A_PGN_ENABLE, MTRUE);
        updateEntry<MUINT8>(&metaset.halMeta, MTK_3A_ISP_PROFILE, profile);

        MetaSet_T rMetaSet;

        MfbPlatformConfig pltcfg;
        pltcfg[ePLATFORMCFG_STAGE] = STAGE_MFB;
        pltcfg[ePLATFORMCFG_INDEX] = (m_blendCount - 1);

        // BLEND stage is always YUV->YUV
        std::unique_ptr<void, std::function<void(void*)> > _tagIspP2InFmtRestorer;
        MINT32 tagIspP2InFmt = setIspP2ImgFmtTag(
                metaset.halMeta,
                1,
                &_tagIspP2InFmtRestorer);

        mfllTraceBegin("SetIsp");
        if (m_p3A->setIsp(0, metaset, &rTuningParam, &rMetaSet) == 0) {
            mfllTraceBegin("AfterSetIsp");

            // restore MTK_ISP_P2_IN_IMG_FMT back
            IMetadata::setEntry<MINT32>(&rMetaSet.halMeta, MTK_ISP_P2_IN_IMG_FMT, tagIspP2InFmt);

            mfllLogD("%s: get tuning data, reg=%p, lsc=%p",
                    __FUNCTION__, rTuningParam.pRegBuf, rTuningParam.pLsc2Buf);

            {
                pltcfg[ePLATFORMCFG_DIP_X_REG_T] =
                    reinterpret_cast<intptr_t>((volatile void*)rTuningParam.pRegBuf);

                pltcfg[ePLATFORMCFG_FULL_SIZE_MC] =
                    (base->getImgSize() == ref->getImgSize() ? 1 : 0);

                pltcfg[ePLATFORMCFG_SRC_WIDTH]  = wt_out->getImgSize().w;
                pltcfg[ePLATFORMCFG_SRC_HEIGHT] = wt_out->getImgSize().h;


                mfllTraceBegin("RefineReg");
                if (!refine_register(pltcfg)) {
                    mfllLogE("%s: refine_register returns false", __FUNCTION__);
                }
                else {
#ifdef __DEBUG
                    mfllLogD("%s: refine_register ok", __FUNCTION__);
#endif
                }
                mfllTraceEnd(); // RefineReg

                workaround_MFB_stage(tuning_reg.get());

                if (m_bExifDumpped[STAGE_MFB] == 0) {
                    dump_exif_info(m_pCore, tuning_reg.get(), STAGE_MFB);
                    m_bExifDumpped[STAGE_MFB] = 1;
                }
#ifdef __DEBUG
                debug_pass2_registers(tuning_reg.get(), STAGE_MFB);
#endif
            }
            // mfllLogE("[%s]  dumpIsp", __FUNCTION__);
            m_p3A->dumpIsp(0, metaset, &rTuningParam, &rMetaSet);
            params.mTuningData = static_cast<MVOID*>(tuning_reg.get()); // adding tuning data

            mfllTraceEnd(); // AfterSetIsp
        }
        else {
            mfllLogE("%s: set ISP profile failed...", __FUNCTION__);
            err = MfllErr_UnexpectedError;
            goto lbExit;
        }
        mfllTraceEnd(); // setIsp
        Mutex mutex;
        std::shared_ptr<_SRZ_SIZE_INFO_> pSrzSizeInfo;
        p2cbParam_t p2cbPack(this, &mutex);
        mfllTraceBegin("BeforeP2Enque");

        _tagIspP2InFmtRestorer = nullptr; // trigger restore MTK_ISP_P2_IN_IMG_FMT


        /* input: source frame, base frame [IMGI port] */
        {
            Input p;
            p.mBuffer       = base;
            p.mPortID       = MFB_PORT_IN_BASE_FRAME;
            p.mPortID.group = 0;
            params.mvIn.push_back(p);
            /* IMGI in this stage doesn't supports cropping */
        }

        /* input: reference frame */
        {
            Input p;
            p.mBuffer       = ref;
            p.mPortID       = MFB_PORT_IN_REF_FRAME;
            p.mPortID.group = 0;
            params.mvIn.push_back(p);
            /* This input port in this stage doesn't support crop */
        }

        /* input: weighting table, for not the first blending, we need to give weighting map */
        if (wt_in) {
            Input p;
            p.mBuffer       = wt_in;
            p.mPortID       = MFB_PORT_IN_WEIGHTING;
            p.mPortID.group = 0;
            params.mvIn.push_back(p);
            /* This intput port doesn't support crop */
        }

        /* output: blended frame */
        {
            Output p;
            p.mBuffer       = out;
            p.mPortID       = MFB_PORT_OUT_FRAME;
            p.mPortID.group = 0;
            params.mvOut.push_back(p);

            /* crop info */
            MCrpRsInfo crop;
            crop.mGroupID                   = MFB_GID_OUT_FRAME;
            crop.mCropRect.p_integral.x     = 0; // position of cropping window, in pixel, integer
            crop.mCropRect.p_integral.y     = 0;
            crop.mCropRect.p_fractional.x   = 0;
            crop.mCropRect.p_fractional.y   = 0;
            crop.mCropRect.s.w              = base->getImgSize().w;
            crop.mCropRect.s.h              = base->getImgSize().h;
            crop.mResizeDst.w               = out->getImgSize().w;
            crop.mResizeDst.h               = out->getImgSize().h;
            crop.mFrameGroup                = 0;
            params.mvCropRsInfo.push_back(crop);
        }

        /* output: new weighting table */
        {
            Output p;
            p.mBuffer       = wt_out;
            p.mPortID       = MFB_PORT_OUT_WEIGHTING;
            p.mPortID.group = 0;
            params.mvOut.push_back(p);
            /* This port doesn't support cropping */
        }

        /**
         *  Confidence map (since MFNR v2.0, new MFB hardware introduced) depends
         *  on platform, not all platform supports confidence map.
         */
#if MFB_SUPPORT_CONF_MAP
        do {
            if (confmap == nullptr)
                break;

            Input p;
            p.mBuffer       = confmap;
            p.mPortID       = MFB_PORT_IN_CONF_MAP;
            p.mPortID.group = 0;
            params.mvIn.push_back(p);
            /* This intput port doesn't support crop */

            /* SRZ5 setting */
            ModuleInfo srzModule;
            srzModule.moduleTag = EDipModule_SRZ5;
            srzModule.frameGroup = 0;
            pSrzSizeInfo = std::shared_ptr<_SRZ_SIZE_INFO_>(new _SRZ_SIZE_INFO_);
            if (CC_UNLIKELY(pSrzSizeInfo.get() == nullptr)) {
                mfllLogE("unexpected error, create _SRZ_SIZE_INFO_ failed, giveup SRZ5");
                break;
            }

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
            pSrzSizeInfo->out_w         = base->getImgSize().w;
            pSrzSizeInfo->out_h         = base->getImgSize().h;

            pSrzSizeInfo->crop_w        = confmap->getImgSize().w;
            pSrzSizeInfo->crop_h        = confmap->getImgSize().h;
            pSrzSizeInfo->in_w          = confmap->getImgSize().w;
            pSrzSizeInfo->in_h          = confmap->getImgSize().h;

            mfllLogD("%s: SRZ5, in(w,h)=(%d,%d), out(w,h)=(%d,%d)", __FUNCTION__,
                    pSrzSizeInfo->in_w,     pSrzSizeInfo->in_h,
                    pSrzSizeInfo->out_w,    pSrzSizeInfo->out_h
                    );

            srzModule.moduleStruct = reinterpret_cast<MVOID*>(pSrzSizeInfo.get());
            params.mvModuleData.push_back(srzModule);
        } while(0);
#endif // MFB_SUPPORT_CONF_MAP


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
        mutex.lock();

        qParams.mpfnCallback = pass2CbFunc;
        qParams.mpCookie = reinterpret_cast<void*>(&p2cbPack);

        // add QParams::FrameParams to QParams
        qParams.mvFrameParams.push_back(params);

        mfllTraceEnd(); // BeforeP2Enque
        mfllTraceBegin("P2");
        mfllLogD("%s: enque pass 2", __FUNCTION__);

        if (!m_pNormalStream->enque(qParams)) {
            mfllLogE("%s: pass 2 enque returns fail", __FUNCTION__);
            err = MfllErr_UnexpectedError;
            goto lbExit;
        }

        mfllLogD("%s: deque pass 2", __FUNCTION__);

        /* lock again, and wait */
        mutex.lock(); // locked, and wait unlock from pass2CbFunc.
        mutex.unlock(); // unlock.
        mfllTraceEnd();

        mfllLogD("mfb dequed");
    }
lbExit:
    return err;
}

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

    /* check resolution */
    {
        MSize size_src = img_src->getImgSize();
        MSize size_dst = img_dst->getImgSize();
        MSize size_wt  = img_wt->getImgSize();

        if (size_src != size_dst || size_src != size_wt) {
            mfllLogE("%s: Resolution of images are not the same", __FUNCTION__);
            mfllLogE("%s: src=%dx%d, dst=%dx%d, wt=%dx%d",
                    __FUNCTION__,
                    size_src.w, size_src.h,
                    size_dst.w, size_dst.h,
                    size_wt.w, size_wt.h);
            return MfllErr_BadArgument;
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
    QParams qParams;
    FrameParams params;

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

        updateEntry<MUINT8>(&metaset.halMeta, MTK_3A_PGN_ENABLE, MTRUE);
        updateEntry<MUINT8>(&metaset.halMeta, MTK_3A_ISP_PROFILE, profile);

        MetaSet_T rMetaSet;

        MfbPlatformConfig pltcfg;
        pltcfg[ePLATFORMCFG_STAGE] = STAGE_MIX;

        // MIX stage is always YUV->YUV
        std::unique_ptr<void, std::function<void(void*)> > _tagIspP2InFmtRestorer;
        MINT32 tagIspP2InFmt = setIspP2ImgFmtTag(
                metaset.halMeta,
                1,
                &_tagIspP2InFmtRestorer);

        mfllTraceBegin("SetIsp");
        if (m_p3A->setIsp(0, metaset, &rTuningParam, &rMetaSet) == 0) {
            mfllTraceBegin("AfterSetIsp");

            // restore MTK_ISP_P2_IN_IMG_FMT back
            IMetadata::setEntry<MINT32>(&rMetaSet.halMeta, MTK_ISP_P2_IN_IMG_FMT, tagIspP2InFmt);

            int expected_blend_cnt = m_pCore->getBlendFrameNum() - 1;
            int real_blend_cnt     = m_pCore->getFrameBlendedNum();
            mfllLogD("%s: blend frames count, real=%d, expected=%d",
                    __FUNCTION__, real_blend_cnt, expected_blend_cnt);

            // refine postNR implementation
            int _enablePostNrRefine = MFLL_POST_NR_REFINE_ENABLE;
            int _enablePostNrRefineByProp = MfllProperty::readProperty
                (Property_PostNrRefine);

            if (CC_UNLIKELY( _enablePostNrRefineByProp >= 0 )) {
                _enablePostNrRefine = _enablePostNrRefineByProp;
            }

            if (CC_LIKELY( _enablePostNrRefine > 0 )) {
                mfllLogD("%s: enable postNR refine", __FUNCTION__);
                mfllTraceBegin("RefineReg");
                bss_refine_postnr_regs(
                        static_cast<void*>(tuning_reg.get()),
                        real_blend_cnt,
                        expected_blend_cnt);
                mfllTraceEnd(); // RefineReg
            }
            else {
                mfllLogD("%s: disable postNR refine", __FUNCTION__);
            }

            mfllLogD("%s: get tuning data, reg=%p, lsc=%p",
                    __FUNCTION__, rTuningParam.pRegBuf, rTuningParam.pLsc2Buf);

            // refine register
            pltcfg[ePLATFORMCFG_DIP_X_REG_T] =
                    reinterpret_cast<intptr_t>((volatile void*)rTuningParam.pRegBuf);

            mfllTraceBegin("RefineReg");
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
            m_p3A->dumpIsp(0, metaset, &rTuningParam, &rMetaSet);
            params.mTuningData = static_cast<MVOID*>(tuning_reg.get()); // adding tuning data
            mfllTraceEnd(); // AfterSetIsp
        }
        else {
            mfllLogE("%s: set ISP profile failed...", __FUNCTION__);
            err = MfllErr_UnexpectedError;
            return err;
        }
        mfllTraceEnd(); // setIsp
        Mutex mutex;
        p2cbParam_t p2cbPack(this, &mutex);
        mfllTraceBegin("BeforeP2Enque");

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

        // output: a frame
        {
            Output p;
            p.mPortID       = MIX_PORT_OUT_FRAME;
            p.mPortID.group = 0;
            p.mBuffer       = img_dst;
            params.mvOut.push_back(p);
            /* this port is not support crop info */
        }

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
        mutex.lock();

        qParams.mpfnCallback = pass2CbFunc;
        qParams.mpCookie = reinterpret_cast<void*>(&p2cbPack);

        // push QParams::FrameParams into QParams
        qParams.mvFrameParams.push_back(params);

        mfllTraceEnd(); // BeforeP2Enque
        mfllTraceBegin("P2");
        if (!m_pNormalStream->enque(qParams)) {
            mfllLogE("%s: pass 2 enque returns fail", __FUNCTION__);
            err = MfllErr_UnexpectedError;
            goto lbExit;
        }

        /* lock again, and wait */
        mutex.lock(); // locked, and wait unlock from pass2CbFunc.
        mutex.unlock(); // unlock.

        // post-fix mix problem
        bool bFixedMixYuv2 =
            fix_mix_yuy2(
                static_cast<unsigned char*>((void*)img_dst->getBufVA(0)),
                static_cast<int>(img_dst->getImgSize().w),
                static_cast<int>(img_dst->getImgSize().h)
                );

        // sync cache if necessary
        if ( bFixedMixYuv2 ) {
            mfllLogD("fix_mix_yuy2 syncCache [+]");
            mfllTraceBegin("syncCache");
            img_dst->syncCache();
            mfllTraceEnd();
            mfllLogD("fix_mix_yuy2 syncCache [-]");
        }

        mfllTraceEnd();
        mfllLogD("mix dequed");
    }

lbExit:
    return err;
}

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
            mfllLogE("%s: Resolution of images are not the same", __FUNCTION__);
            mfllLogE("%s: src=%dx%d, dst=%dx%d, wt=%dx%d",
                    __FUNCTION__,
                    size_src.w, size_src.h,
                    size_dst.w, size_dst.h,
                    size_wt.w, size_wt.h);
            return MfllErr_BadArgument;
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
    QParams qParams;
    FrameParams params;

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

        updateEntry<MUINT8>(&metaset.halMeta, MTK_3A_PGN_ENABLE, MTRUE);
        updateEntry<MUINT8>(&metaset.halMeta, MTK_3A_ISP_PROFILE, profile);

        MetaSet_T rMetaSet;

        MfbPlatformConfig pltcfg;
        pltcfg[ePLATFORMCFG_STAGE] = STAGE_MIX;

        // MIX stage is always YUV->YUV
        std::unique_ptr<void, std::function<void(void*)> > _tagIspP2InFmtRestorer;
        MINT32 tagIspP2InFmt = setIspP2ImgFmtTag(
                metaset.halMeta,
                1,
                &_tagIspP2InFmtRestorer);

        mfllTraceBegin("SetIsp");
        if (m_p3A->setIsp(0, metaset, &rTuningParam, &rMetaSet) == 0) {
            mfllTraceBegin("AfterSetIsp");

            // restore MTK_ISP_P2_IN_IMG_FMT back
            IMetadata::setEntry<MINT32>(&rMetaSet.halMeta, MTK_ISP_P2_IN_IMG_FMT, tagIspP2InFmt);

            int expected_blend_cnt = m_pCore->getBlendFrameNum() - 1;
            int real_blend_cnt     = m_pCore->getFrameBlendedNum();
            mfllLogD("%s: blend frames count, real=%d, expected=%d",
                    __FUNCTION__, real_blend_cnt, expected_blend_cnt);

            // refine postNR implementation
            int _enablePostNrRefine = MFLL_POST_NR_REFINE_ENABLE;
            int _enablePostNrRefineByProp = MfllProperty::readProperty
                (Property_PostNrRefine);

            if (CC_UNLIKELY( _enablePostNrRefineByProp >= 0 )) {
                _enablePostNrRefine = _enablePostNrRefineByProp;
            }

            if (CC_LIKELY( _enablePostNrRefine > 0 )) {
                mfllLogD("%s: enable postNR refine", __FUNCTION__);
                mfllTraceBegin("RefineReg");
                bss_refine_postnr_regs(
                        static_cast<void*>(tuning_reg.get()),
                        real_blend_cnt,
                        expected_blend_cnt);
                mfllTraceEnd(); // RefineReg
            }
            else {
                mfllLogD("%s: disable postNR refine", __FUNCTION__);
            }

            mfllLogD("%s: get tuning data, reg=%p, lsc=%p",
                    __FUNCTION__, rTuningParam.pRegBuf, rTuningParam.pLsc2Buf);

            // refine register
            pltcfg[ePLATFORMCFG_DIP_X_REG_T] =
                    reinterpret_cast<intptr_t>((volatile void*)rTuningParam.pRegBuf);

            mfllTraceBegin("RefineReg");
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
        Mutex mutex;
        p2cbParam_t p2cbPack(this, &mutex);
        mfllTraceBegin("BeforeP2Enque");

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
            mfllLogD("config main yuv during mixing");
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
            crop.mCropRect.s.w  = img_dst->getImgSize().w;
            crop.mCropRect.s.h  = img_dst->getImgSize().h;
            crop.mResizeDst.w   = img_dst->getImgSize().w;
            crop.mResizeDst.h   = img_dst->getImgSize().h;
            crop.mFrameGroup    = 0;
            params.mvCropRsInfo.push_back(crop);

            mfllLogD("main: srcCrop (x,y,w,h)=(%d,%d,%d,%d)",
                    crop.mCropRect.p_integral.x, crop.mCropRect.p_integral.y,
                    crop.mCropRect.s.w, crop.mCropRect.s.h);
            mfllLogD("main: dstSize (w,h)=(%d,%d",
                    crop.mResizeDst.w, crop.mResizeDst.h);
        }

        // output: thumbnail yuv
        if (img_thumb &&  output_thumb_crop.size() > 0) {
            mfllLogD("config thumnail during mixing");
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

            mfllLogD("thumnail: srcCrop (x,y,w,h)=(%d,%d,%d,%d)",
                    crop.mCropRect.p_integral.x, crop.mCropRect.p_integral.y,
                    crop.mCropRect.s.w, crop.mCropRect.s.h);
            mfllLogD("thumbnail: dstSize (w,h)=(%d,%d",
                    crop.mResizeDst.w, crop.mResizeDst.h);
        }

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
        mutex.lock();

        qParams.mpfnCallback = pass2CbFunc;
        qParams.mpCookie = reinterpret_cast<void*>(&p2cbPack);

        // push QParams::FrameParams into QParams
        qParams.mvFrameParams.push_back(params);

        mfllTraceEnd(); // BeforeP2Enque
        mfllTraceBegin("P2");
        if (!m_pNormalStream->enque(qParams)) {
            mfllLogE("%s: pass 2 enque returns fail", __FUNCTION__);
            err = MfllErr_UnexpectedError;
            goto lbExit;
        }

        /* lock again, and wait */
        mutex.lock(); // locked, and wait unlock from pass2CbFunc.
        mutex.unlock(); // unlock.
        mfllTraceEnd();

        mfllLogD("mix dequed");
    }

lbExit:
    return err;
}

enum MfllErr MfllMfb::setSyncPrivateData(const std::deque<void*>& dataset)
{
    mfllLogD("dataset size=%zu", dataset.size());
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
    mfllLogD("saves MetaSet_T, size = %zu", m_vMetaSet.size());
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
        mfllLogE("%s: the size between input and output must be the same", __FUNCTION__);
        return MfllErr_BadArgument;
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
    bool bBaseYuvStage = (s == YuvStage_BaseYuy2 || s == YuvStage_GoldenYuy2) ? true : false;

    m_mutex.lock();
    int index = m_encodeYuvCount++;
    int sensorId = m_sensorId;
    IImageBuffer *pInput = src;
    IImageBuffer *pOutput = dst;
    IImageBuffer *pOutputQ = dst2;
    IImageBuffer *pOutput3 = dst3;
    /* Do not increase YUV stage if it's encoding base YUV or golden YUV */
    if (bBaseYuvStage) {
        m_encodeYuvCount--;
        index = m_pCore->getIndexByNewIndex(0); // Use the first metadata
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
    MetaSet_T metaset = m_vMetaSet[index]; // using copy
    IMetadata *pHalMeta = m_vMetaHal[index]; // get address of IMetadata
    IImageBuffer *pImgLcsoRaw = m_vImgLcsoRaw[index]; // get LCSO RAW
    m_mutex.unlock();

    bool bIsYuv2Yuv = !isRawFormat(static_cast<EImageFormat>(pInput->getImgFormat()));

    /**
     *  Select profile based on Stage:
     *  1) Raw to Yv16
     *  2) Encoding base YUV
     *  3) Encoding golden YUV
     *
     */
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

    /**
     *  Ok, we're going to configure P2 driver
     */
    QParams qParams;
    FrameParams params;

    /* If using ZSD mode, to use Vss profile can improve performance */
    params.mStreamTag = (isZsdMode(m_shotMode)
            ? NSCam::NSIoPipe::NSPostProc::ENormalStreamTag_Vss
            : NSCam::NSIoPipe::NSPostProc::ENormalStreamTag_Normal
            );

    /* wait JOB MFB done */
    do {
        auto status = MfllOperationSync::getInstance()->waitJob(MfllOperationSync::JOB_MFB); // wait
        if (status == std::cv_status::timeout) {
            mfllLogW("wait timeout(200ms) of job MFB, ignored");
        }
    } while(0);

    /* get ready to operate P2 driver, it's a long stroy ... */
    {
        // get size of dip_x_reg_t
        volatile size_t regTableSize = m_pNormalStream_vss->getRegTableSize();
        if (regTableSize <= 0) {
            mfllLogE("%s: unexpected tuning buffer size: %zu", __FUNCTION__, regTableSize);
        }

        /* add tuning data is necessary */
        std::unique_ptr<char[]> tuning_reg(new char[regTableSize]());
        void *tuning_lsc;

        mfllLogD("%s: create tuning register data chunk with size %zu",
                __FUNCTION__, regTableSize);

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
            updateEntry<MUINT8>(
                    &metaset.halMeta,
                    MTK_3A_PGN_ENABLE,
                    MTRUE);

            // if shot mode is single frame, do not use MFNR related ISP profile
            mfllLogD("%s: shotMode=%#x", __FUNCTION__, m_shotMode);
            if (m_shotMode & (1 << MfllMode_Bit_SingleFrame))
                mfllLogD("%s: do not use Mfll related ISP profile for SF",__FUNCTION__);
            else
                updateEntry<MUINT8>(&metaset.halMeta, MTK_3A_ISP_PROFILE, profile);

            // tell ISP manager that this stage is YUV->YUV
            std::unique_ptr<void, std::function<void(void*)> > _tagIspP2InFmtRestorer;
            MINT32 tagIspP2InFmt = 0;
            if (bIsYuv2Yuv) {
                tagIspP2InFmt = setIspP2ImgFmtTag(metaset.halMeta, 1, &_tagIspP2InFmtRestorer);
            }

            // debug {{{
            {
                MUINT8 __profile = 0;
                if (tryGetMetaData<MUINT8>(&metaset.halMeta, MTK_3A_ISP_PROFILE, __profile)) {
                    mfllLogD("%s: isp profile=%#x", __FUNCTION__, __profile);
                }
                else {
                    mfllLogD("%s: isp profile=NULL", __FUNCTION__);
                }
            }
            // }}}

            MetaSet_T rMetaSet;

            mfllTraceBegin("SetIsp");
            if (m_p3A->setIsp(0, metaset, &rTuningParam, &rMetaSet) == 0) {
                mfllTraceBegin("AfterSetIsp");

                mfllLogD("%s: get tuning data, reg=%p, lsc=%p",
                        __FUNCTION__, rTuningParam.pRegBuf, rTuningParam.pLsc2Buf);

                // restore MTK_ISP_P2_IN_IMG_FMT first
                IMetadata::setEntry<MINT32>(&rMetaSet.halMeta, MTK_ISP_P2_IN_IMG_FMT, tagIspP2InFmt);

                // refine register
                pltcfg[ePLATFORMCFG_DIP_X_REG_T] =
                    reinterpret_cast<intptr_t>((volatile void*)rTuningParam.pRegBuf);

                mfllTraceBegin("RefineReg");
                if (!refine_register(pltcfg)) {
                    mfllLogE("%s: refine_register returns fail", __FUNCTION__);
                }
                else {
#ifdef __DEBUG
                    mfllLogD("%s: refine_register ok", __FUNCTION__);
#endif
                }
                mfllTraceEnd(); // RefineReg

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
                    /* update metadata within the one given by IHal3A only in stage bfbld*/
                    *pHalMeta += rMetaSet.halMeta;
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
                m_p3A->dumpIsp(0, metaset, &rTuningParam, &rMetaSet);
                params.mTuningData = static_cast<MVOID*>(tuning_reg.get()); // adding tuning data

                /* LSC tuning data is constructed as an IImageBuffer, and we don't need to release */
                IImageBuffer* pSrc = static_cast<IImageBuffer*>(rTuningParam.pLsc2Buf);
                if (pSrc != NULL) {
                    Input __src;
                    __src.mPortID         = PORT_DEPI;
                    __src.mPortID.group   = 0;
                    __src.mBuffer         = pSrc;
                    params.mvIn.push_back(__src);
                }
                else {
                    // check if it's process RAW, if yes, there's no LSC.
                    MINT32 __rawType = NSIspTuning::ERawType_Pure;
                    if (!tryGetMetaData<MINT32>(
                                &metaset.halMeta,
                                MTK_P1NODE_RAW_TYPE,
                                __rawType)) {
                        mfllLogW("get p1 node RAW type fail, assume it's pure RAW");
                    }

                    // check if LSC table should exists or not.
                    if (__rawType == NSIspTuning::ERawType_Proc) {
                        mfllLogD("process RAW, no LSC table");
                    }
                    else if (bIsYuv2Yuv) {
                        mfllLogD("Yuv->Yuv, no LSC table");
                    }
                    else {
                        mfllLogE("create LSC image buffer fail");
                        err = MfllErr_UnexpectedError;
                    }
                }

                IImageBuffer* pBpcBuf = static_cast<IImageBuffer*>(rTuningParam.pBpc2Buf);
                if (pBpcBuf != NULL) {
                    Input __src;
                    __src.mPortID         = PORT_DMGI;
                    __src.mPortID.group   = 0;
                    __src.mBuffer         = pBpcBuf;
                    params.mvIn.push_back(__src);
                }
                else {
                    // BPC buffer may be NULL even for pure RAW processing
                    mfllLogD("No BPC buffer");
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
                    /* no cropping info need */
                    mfllLogD("enable LCEI port for LCSO buffer, size = %d x %d",
                            pImgLcsoRaw->getImgSize().w,
                            pImgLcsoRaw->getImgSize().h);
                }

                mfllTraceEnd(); // AfterSetIsp
            }
            else {
                mfllLogE("%s: set ISP profile failed...", __FUNCTION__);
                err = MfllErr_UnexpectedError;
                goto lbExit;
            }
            mfllTraceEnd(); // setIsp

            // restore MTK_ISP_P2_IN_IMG_FMT
            _tagIspP2InFmtRestorer = nullptr;
        }
        Mutex mutex;
        p2cbParam_t p2cbPack(this, &mutex);
        mfllTraceBegin("BeforeP2Enque");

        /* input: source frame [IMGI port] */
        {
            Input p;
            p.mBuffer          = pInput;
            p.mPortID          = RAW2YUV_PORT_IN;
            p.mPortID.group    = 0; // always be 0
            params.mvIn.push_back(p);
        }

        /* output: destination frame [WDMAO port] */
        {
            bool bNeedResizer =
                (pInput->getImgSize().w != pOutput->getImgSize().w) ||
                (pInput->getImgSize().h != pOutput->getImgSize().h) ;

            if (!bNeedResizer) {
                mfllLogD("%s: using non-MDP port", __FUNCTION__);
            }

            Output p;
            p.mBuffer          = pOutput;
            p.mPortID          = (bNeedResizer ? RAW2YUV_PORT_OUT : RAW2YUV_PORT_OUT_NO_CRZ);
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
            if (bNeedResizer) {
                MCrpRsInfo crop;
                crop.mGroupID       = RAW2YUV_GID_OUT;
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
                mfllLogD("output: srcCrop (x,y,w,h)=(%d,%d,%d,%d)",
                        crop.mCropRect.p_integral.x, crop.mCropRect.p_integral.y,
                        crop.mCropRect.s.w, crop.mCropRect.s.h);
                mfllLogD("output: dstSize (w,h)=(%d,%d",
                        crop.mResizeDst.w, crop.mResizeDst.h);
            }
        }

        /* output2: destination frame [WROTO prot] */
        if (pOutputQ) {
            Output p;
            p.mBuffer          = pOutputQ;
            p.mPortID          = RAW2YUV_PORT_OUT2;
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
            crop.mGroupID       = RAW2YUV_GID_OUT2;
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

            mfllLogD("output2: srcCrop (x,y,w,h)=(%d,%d,%d,%d)",
                    crop.mCropRect.p_integral.x, crop.mCropRect.p_integral.y,
                    crop.mCropRect.s.w, crop.mCropRect.s.h);
            mfllLogD("output2: dstSize (w,h)=(%d,%d",
                    crop.mResizeDst.w, crop.mResizeDst.h);
        }

        /* output3: destination frame*/
        if (pOutput3) {
            Output p;
            p.mBuffer          = pOutput3;
            p.mPortID          = RAW2YUV_PORT_OUT3;
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
            crop.mGroupID       = RAW2YUV_GID_OUT3;
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

            mfllLogD("output3: srcCrop (x,y,w,h)=(%d,%d,%d,%d)",
                    crop.mCropRect.p_integral.x, crop.mCropRect.p_integral.y,
                    crop.mCropRect.s.w, crop.mCropRect.s.h);
            mfllLogD("output3: dstSize (w,h)=(%d,%d",
                    crop.mResizeDst.w, crop.mResizeDst.h);
        }

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
        mutex.lock();

        qParams.mpfnCallback = pass2CbFunc;
        qParams.mpCookie = reinterpret_cast<void*>(&p2cbPack);

        // push QParams::FrameParams into QParams
        qParams.mvFrameParams.push_back(params);

        mfllLogD("%s: enque", __FUNCTION__);

#ifdef __DEBUG
        if (pOutputQ) {
            mfllLogD("%s: VA input=%p,output1=%p,output2=%p", __FUNCTION__,
                    pInput->getBufVA(0), pOutput->getBufVA(0), pOutputQ->getBufVA(0));
            mfllLogD("%s: PA input=%p,output1=%p,output2=%p", __FUNCTION__,
                    pInput->getBufPA(0), pOutput->getBufPA(0), pOutputQ->getBufPA(0));
        }
        else {
            mfllLogD("%s: VA input=%p,output1=%p", __FUNCTION__,
                    pInput->getBufVA(0), pOutput->getBufVA(0));
            mfllLogD("%s: PA input=%p,output1=%p", __FUNCTION__,
                    pInput->getBufPA(0), pOutput->getBufPA(0));
        }
#endif

        mfllTraceEnd(); // BeforeP2Enque
        mfllTraceBegin("P2");
        if (!m_pNormalStream_vss->enque(qParams)) {
            mfllLogE("%s: pass 2 enque fail", __FUNCTION__);
            err = MfllErr_UnexpectedError;
            goto lbExit;
        }
        mfllLogD("%s: dequed", __FUNCTION__);

        /* lock again, and wait */
        mutex.lock(); // locked, and wait unlock from pass2CbFunc.
        mutex.unlock(); // unlock.
        mfllTraceEnd();

        mfllLogD("p2 dequed");
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
    mfllTraceCall();

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

            metaset = m_vMetaSet[index]; // using copy
            pHalMeta = m_vMetaHal[index]; // get address of IMetadata
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
        updateEntry<MUINT8>(&metaset.halMeta, MTK_3A_PGN_ENABLE, MTRUE);
        updateEntry<MUINT8>(&metaset.halMeta, MTK_3A_ISP_PROFILE, profile);

        // Set ISP to retrieve dip_x_reg_t
        mfllTraceBegin("SetIsp");
        if (m_p3A->setIsp(0, metaset, &rTuningParam, nullptr) == 0) {
            mfllTraceBegin("AfterSetIsp");
#ifdef __DEBUG
            mfllLogD("%s: get tuning data, reg=%p, lsc=%p",
                    __FUNCTION__, rTuningParam.pRegBuf, rTuningParam.pLsc2Buf);
#endif

            MfbPlatformConfig pltcfg;
            pltcfg[ePLATFORMCFG_STAGE] = STAGE_BASE_YUV;
            pltcfg[ePLATFORMCFG_INDEX] = 0;
            pltcfg[ePLATFORMCFG_DIP_X_REG_T] =
                reinterpret_cast<intptr_t>((volatile void*)rTuningParam.pRegBuf);

            mfllTraceBegin("RefineReg");
            if (!refine_register(pltcfg)) {
                mfllLogE("%s: refine_register returns fail", __FUNCTION__);
            }
            else {
#ifdef __DEBUG
                mfllLogD("%s: refine_register ok", __FUNCTION__);
#endif
            }
            mfllTraceEnd(); // RefineReg
            // update Exif info
            if (m_bExifDumpped[STAGE_BASE_YUV] == 0) {
                dump_exif_info(m_pCore, tuning_reg.get(), STAGE_BASE_YUV);
                m_bExifDumpped[STAGE_BASE_YUV] = 1;
            }
            mfllTraceEnd(); // AfterSetIsp
        }
        else {
            mfllLogE("%s: setIsp returns fail, Exif may be lost", __FUNCTION__);
        }
        mfllTraceEnd(); // setIsp
        // }}}
    } // fi (YuvStage_BaseYuy2)

    return err;
}

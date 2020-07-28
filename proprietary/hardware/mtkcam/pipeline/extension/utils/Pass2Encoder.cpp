/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 *
 * MediaTek Inc. (C) 2016. All rights reserved.
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
#define __DEBUG // enable debug
// #define __SCOPE_TIMER // enable log of scope timer
//
#define LOG_TAG "Pass2Encoder"
static const char* __CALLERNAME__ = LOG_TAG;

#include "Pass2Encoder.h"

// MTKCAM
#include <mtkcam/utils/std/Log.h> // CAM_LOGD, CAM_LOGV ...
#include <mtkcam/utils/std/StlUtils.h> // ScopeWorker
#include <mtkcam/utils/metadata/hal/mtk_platform_metadata_tag.h> // TAG
#include <mtkcam/drv/iopipe/PostProc/IHalPostProcPipe.h> // QParams
#include <mtkcam/aaa/aaa_hal_common.h> // TuningParam
#include <mtkcam/drv/iopipe/PortMap.h> // PORT_DEPI ...

// CUSTOM
#include <isp_tuning/isp_tuning.h> // EIspProfile_T, from custom folder

// AOSP
#include <cutils/compiler.h>
#include <utils/Errors.h>

// STL
#include <map>

//
// Macros
//
#define MY_LOGV(fmt, arg...)        CAM_LOGV("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGD(fmt, arg...)        CAM_LOGD("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGI(fmt, arg...)        CAM_LOGI("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGW(fmt, arg...)        CAM_LOGW("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGE(fmt, arg...)        CAM_LOGE("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGA(fmt, arg...)        CAM_LOGA("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGF(fmt, arg...)        CAM_LOGF("[%s] " fmt, __FUNCTION__, ##arg)
// FUNCTION_SCOPE
#ifdef __DEBUG
#define FUNCTION_SCOPE          auto __scope_logger__ = create_scope_logger(__FUNCTION__)
#include <memory>
#include <functional>
static std::shared_ptr<char> create_scope_logger(const char* functionName)
{
    char* pText = const_cast<char*>(functionName);
    CAM_LOGD("[%s] + ", pText);
    return std::shared_ptr<char>(pText, [](char* p){ CAM_LOGD("[%s] -", p); });
}
#else
#define FUNCTION_SCOPE          do{}while(0)
#endif
// SCOPE_TIMER
#ifdef __SCOPE_TIMER
#define SCOPE_TIMER(VAR, TEXT)  auto VAR = create_scope_timer(TEXT)
#include <memory>
#include <functional>
#include <utils/SystemClock.h> // elapsedRealtime(), uptimeMillis()
static std::unique_ptr<const char, std::function<void(const char*)> >
create_scope_timer(const char* text)
{
    auto t1 = android::elapsedRealtimeNano();
    return std::unique_ptr<const char, std::function<void(const char*)> >
        (
            text,
            [t1](const char* t)->void
            {
                auto t2 = android::elapsedRealtimeNano();
                MY_LOGD("%s --> duration(ns): %" PRId64 "", t, (t2 -t1));
            }
        );
}
#else
#define SCOPE_TIMER(VAR, TEXT)  do{}while(0)
#endif

using namespace android;
using namespace NSCam;
using namespace NSCam::plugin;
using namespace NSIoPipe;
using namespace NSCam::NSIoPipe::NSPostProc;
using namespace NSCam::NSIoPipe;
using namespace NS3Av3;
using namespace NSIspTuning;

//
// Pass2DataPack
//
class Pass2DataPack final
{
public:
    Pass2DataPack(size_t buf_size) : size(buf_size)
    {
        if (CC_LIKELY(buf_size > 0)) {
            this->data = std::unique_ptr<char[]>(new char[buf_size]{0});
        }
    }
    ~Pass2DataPack() = default;


public:
    inline std::unique_lock<std::mutex> uniqueLock()
    {
        return std::unique_lock<std::mutex>(mx);
    }
    inline std::mutex& getLocker() { return mx; }
    inline void lock()      { mx.lock(); }
    inline void unlock()    { mx.unlock(); }
    inline void notifyOne()  { cv.notify_one(); }
    inline void notifyAll()  { cv.notify_all(); }
    inline void wait(std::unique_lock<std::mutex>&& l) { cv.wait(l); }

    inline char* getBuf()   { return data.get(); }
    inline size_t getSize() const { return size; }


private:
    size_t                  size;
    std::unique_ptr<char[]> data;
    // for thread sync
    std::mutex              mx;
    std::condition_variable cv;
};


// ----------------------------------------------------------------------------
// ConfigRaw2Yuv
// ----------------------------------------------------------------------------

Pass2Encoder::ConfigRaw2Yuv::ConfigRaw2Yuv() noexcept
    : pSrcRaw(nullptr)
    , pLcsoRaw(nullptr)
    , pAppMeta(nullptr)
    , pHalMeta(nullptr)
    , pOutput(nullptr)
    , pOutput2(nullptr)
    , pOutAppMeta(nullptr)
    , pOutHalMeta(nullptr)
    , isZsd(MFALSE)
    , ispProfile(static_cast<int>(EIspProfile_Capture))
    , rotation1(0)
    , rotation2(0)
{
}


bool Pass2Encoder::ConfigRaw2Yuv::isValid() const noexcept
{
#define CHECK_ATTR(VAR, TEXT)  \
    if (CC_UNLIKELY(VAR == nullptr)) {           \
        MY_LOGE(TEXT);              \
        bOK = false;                \
    }

    bool bOK = true;
    CHECK_ATTR(pSrcRaw,     "Source RAW cannot be NULL");
    CHECK_ATTR(pAppMeta,    "App metadata cannot be NULL");
    CHECK_ATTR(pHalMeta,    "Hal metadata cannot be NULL");
    CHECK_ATTR(pOutput,     "Output cannot be NULL");
    if (CC_UNLIKELY(rotation1 != 0 && rotation2 != 0)) {
        MY_LOGE("Rotation 1 and 2 cannot be non-zero at the same time");
        bOK = false;
    }

    return bOK;
#undef CHECK_ATTR
}


//
// Pass2Encoder
//
Pass2Encoder::Pass2Encoder(MINT32 sensorId)
    : m_sensorId(sensorId)
    , m_regTableSize(0)
{

    // create INormalStream instance
    m_pNormalStream =
        std::unique_ptr< INormalStream, std::function<void(INormalStream*)> >
        (
            INormalStream::createInstance(m_sensorId),
            [](INormalStream* p) { if(CC_LIKELY(p)) p->destroyInstance(); }
        );

    if (CC_UNLIKELY(m_pNormalStream.get() == nullptr)) {
        MY_LOGE("create INormalStream failed");
        return;
    }

    m_regTableSize = m_pNormalStream->getRegTableSize();
    if (CC_UNLIKELY(m_regTableSize <= 0)) {
        MY_LOGE("get dip_x_reg_t size = 0");
        return;
    }

    // create IHal3A
    m_pHal3A =
        std::unique_ptr< IHal3A, std::function<void(IHal3A*)> >
        (
            MAKE_Hal3A(m_sensorId, __CALLERNAME__),
            [](IHal3A* p) { if(CC_LIKELY(p)) p->destroyInstance(__CALLERNAME__); }
        );

    if (CC_UNLIKELY(m_pHal3A.get() == nullptr)) {
        MY_LOGE("create IHal3A failed");
        return;
    }
}


Pass2Encoder::~Pass2Encoder()
{
    {
        std::lock_guard<std::mutex> __l(m_pHal3ALock);
        m_pHal3A = nullptr;
    }
    {
        std::lock_guard<std::mutex> __l(m_pNormalStreamLock);
        m_pNormalStream = nullptr;
    }
}


MERROR Pass2Encoder::encodeRaw2Yuv(const ConfigRaw2Yuv* cfg) const
{
    // check config
    if (CC_UNLIKELY(!cfg->isValid())) {
        MY_LOGE("ConfigRaw2Yuv is invalid");
        return BAD_VALUE;
    }

    //For ISP 5.0 Driver modification
    std::map<EDIPInfoEnum, MUINT32> pDipInfo;

    pDipInfo[EDIPINFO_DIPVERSION] = EDIPHWVersion_40;
    MBOOL r = NSCam::NSIoPipe::NSPostProc::INormalStream::queryDIPInfo(pDipInfo);
    if(!r) MY_LOGE("queryDIPInfo fail!");

    // MetaSet_T
    MetaSet_T metaset, rMetaSet;
    metaset.appMeta = *(cfg->pAppMeta); // copy if edited
    metaset.halMeta = *(cfg->pHalMeta); // copy if edited

    // update pHalMeta
    IMetadata::setEntry<MUINT8>(&metaset.halMeta, MTK_3A_PGN_ENABLE, MTRUE);
    IMetadata::setEntry<MUINT8>(&metaset.halMeta, MTK_3A_ISP_PROFILE, cfg->ispProfile);

    // prepare parameters for INormalStream.
    QParams qParams;
    FrameParams params;
    params.mStreamTag = (cfg->isZsd == MTRUE)
            ? ENormalStreamTag_Vss
            : ENormalStreamTag_Normal;

    // create a Pass2DataPack, including tuning buffer, and threa sync mechanism
    Pass2DataPack pass2data(m_regTableSize);

    //
    // retrieve ISP tuning parameters.
    //
    {
        TuningParam rTuningParam;
        rTuningParam.pRegBuf = pass2data.getBuf();
        rTuningParam.pLcsBuf = cfg->pLcsoRaw; // add LCSO raw to ISP mgr

        auto rSetIsp = [&] {
            bool bNeedRetrieveMetadata =
                (cfg->pOutHalMeta != nullptr) ||
                (cfg->pOutAppMeta != nullptr) ;

            std::lock_guard<std::mutex> __l(m_pHal3ALock);
            if (CC_UNLIKELY(m_pHal3A.get() == nullptr)) {
                MY_LOGE("IHal3A instance is NULL");
                return -1;
            }
            return m_pHal3A->setIsp(
                    0,
                    metaset,
                    &rTuningParam,
                    bNeedRetrieveMetadata ? &metaset : nullptr);
        }();

        if (CC_UNLIKELY(rSetIsp != 0)) {
            MY_LOGE("setIsp returns fail");
            return UNKNOWN_ERROR;
        }

        // add tuning data to QParams
        params.mTuningData = static_cast<MVOID*>(pass2data.getBuf());
        // LSC tuning
        IImageBuffer* pLsc2Buf = static_cast<IImageBuffer*>(rTuningParam.pLsc2Buf);
        if (CC_LIKELY(pLsc2Buf != nullptr)) {
            Input __src;
            __src.mPortID           = pDipInfo[EDIPINFO_DIPVERSION] == EDIPHWVersion_50 ? PORT_IMGCI : PORT_DEPI;
            __src.mPortID.group     = 0;
            __src.mBuffer           = pLsc2Buf;
            params.mvIn.push_back(__src);
        }
        else {
            MY_LOGW("No LSC buffer found. If you are process raw please igored");
        }

        // LCSO buffer, it may be disabled by ISP mgr event we gave it
        IImageBuffer* pLcsoRaw = static_cast<IImageBuffer*>(rTuningParam.pLcsBuf);
        if (pLcsoRaw != nullptr) {
            Input __src;
            __src.mPortID         = PORT_LCEI;
            __src.mPortID.group   = 0;
            __src.mBuffer         = pLcsoRaw;
            params.mvIn.push_back(__src);
        }

        // BPC buffer
        IImageBuffer* pBpc2Buf = static_cast<IImageBuffer*>(rTuningParam.pBpc2Buf);
        if (pBpc2Buf != nullptr) {
            Input __src;
            __src.mPortID         = pDipInfo[EDIPINFO_DIPVERSION] == EDIPHWVersion_50 ? PORT_IMGBI : PORT_DMGI;
            __src.mPortID.group   = 0;
            __src.mBuffer         = pBpc2Buf;
            params.mvIn.push_back(__src);
        }
    }


    //
    // prepare parameters for pass 2 driver
    //

    // to select output port by rotation.
#   define CROP_GID_IMG3O   1
#   define CROP_GID_WDMAO   2
#   define CROP_GID_WROTO   3
    PortID port1 = PORT_IMG3O;
    PortID port2 = PORT_WROTO;
    MUINT32 cropGid1 = CROP_GID_IMG3O;
    MUINT32 cropGid2 = CROP_GID_WROTO;

    // if crz is happened at output 1, do not use IMG3O.
    if ((cfg->pSrcRaw->getImgSize() != cfg->pOutput->getImgSize()) ||
        (cfg->cropWindow1.s.w != 0 && cfg->cropWindow1.s.h != 0))
    {
        port1 = PORT_WDMAO;
        cropGid1 = CROP_GID_WDMAO;
    }

    // if rotation happens at output1
    if (cfg->rotation1 != 0) {
        port1 = PORT_WROTO;
        port2 = PORT_WDMAO;
        //
        cropGid1 = CROP_GID_WROTO;
        cropGid2 = CROP_GID_WDMAO;
    }

    // Input source RAW
    do {
        Input p;
        p.mBuffer           = cfg->pSrcRaw;
        p.mPortID           = PORT_IMGI;
        p.mPortID.group     = 0;
        params.mvIn.push_back(p);
    } while(0);

    // Output 1
    do {
        Output p;
        p.mBuffer           = cfg->pOutput;
        p.mPortID           = port1;
        p.mPortID.group     = 0;
        params.mvOut.push_back(p);

        MRect srcCrop;
        if (cfg->cropWindow1.s.w == 0 || cfg->cropWindow1.s.h == 0)
            srcCrop = MRect(MPoint(0, 0), cfg->pSrcRaw->getImgSize());
        else
            srcCrop = cfg->cropWindow1;

        MCrpRsInfo crop;
        crop.mGroupID       = cropGid1;
        crop.mCropRect.p_integral.x = srcCrop.p.x; // position pixel, in integer
        crop.mCropRect.p_integral.y = srcCrop.p.y;
        crop.mCropRect.p_fractional.x = 0; // always be 0
        crop.mCropRect.p_fractional.y = 0;
        crop.mCropRect.s.w  = srcCrop.s.w;
        crop.mCropRect.s.h  = srcCrop.s.h;
        crop.mResizeDst.w   = cfg->pOutput->getImgSize().w;
        crop.mResizeDst.h   = cfg->pOutput->getImgSize().h;
        crop.mFrameGroup    = 0;
        params.mvCropRsInfo.push_back(crop);
    } while(0);

    // Output 2 (optional)
    if (cfg->pOutput2) {
        Output p;
        p.mBuffer           = cfg->pOutput2;
        p.mPortID           = port2;
        p.mPortID.group     = 0;
        params.mvOut.push_back(p);

        MRect srcCrop;
        if (cfg->cropWindow2.s.w == 0 || cfg->cropWindow2.s.h == 0)
            srcCrop = MRect(MPoint(0, 0), cfg->pSrcRaw->getImgSize());
        else
            srcCrop = cfg->cropWindow2;

        MCrpRsInfo crop;
        crop.mGroupID       = cropGid2; // for PROT_WROTO
        crop.mCropRect.p_integral.x = srcCrop.p.x; // position pixel, in integer
        crop.mCropRect.p_integral.y = srcCrop.p.y;
        crop.mCropRect.p_fractional.x = 0; // always be 0
        crop.mCropRect.p_fractional.y = 0;
        crop.mCropRect.s.w  = srcCrop.s.w;
        crop.mCropRect.s.h  = srcCrop.s.h;
        crop.mResizeDst.w   = cfg->pOutput2->getImgSize().w;
        crop.mResizeDst.h   = cfg->pOutput2->getImgSize().h;
        crop.mFrameGroup    = 0;
        params.mvCropRsInfo.push_back(crop);
    }


    // make as synchronized call
    qParams.mpCookie = static_cast<void*>(&pass2data);
    qParams.mvFrameParams.push_back(params);
    qParams.mpfnCallback = [](QParams& rParams)->MVOID
    {
        if (CC_UNLIKELY(rParams.mpCookie == nullptr))
            return;

        Pass2DataPack* pData = static_cast<Pass2DataPack*>(rParams.mpCookie);
        std::lock_guard<std::mutex> __l(pData->getLocker());
        pData->notifyOne();
    };


    // enque
    {
        auto __l = pass2data.uniqueLock();
        MBOOL bEnqueResult = MTRUE;
        {
            std::lock_guard<std::mutex> __locker(m_pNormalStreamLock);
            if (CC_UNLIKELY(m_pNormalStream.get() == nullptr)) {
                MY_LOGE("INormalStream instance is NULL");
                return UNKNOWN_ERROR;
            }
            bEnqueResult = m_pNormalStream->enque(qParams);
        }

        if (CC_UNLIKELY(!bEnqueResult)) {
            MY_LOGE("%s: pass2 enque fail", __FUNCTION__);
            return UNKNOWN_ERROR;
        }
        else {
            pass2data.wait(std::move(__l));
        }
    }

    // update output metadata if speicified.
    if (cfg->pOutAppMeta) *cfg->pOutAppMeta += metaset.appMeta;
    if (cfg->pOutHalMeta) *cfg->pOutHalMeta += metaset.halMeta;

    return OK;
}


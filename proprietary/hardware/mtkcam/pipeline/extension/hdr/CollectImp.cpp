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
#define LOG_TAG "CollectVendorImp"
static const char* __CALLERNAME__ = LOG_TAG;

#include "CollectImp.h"

#include "../utils/VendorUtils.h"
#include "../utils/RequestFrame.h"
#include "../utils/ControllerContainer.h"
#include "../utils/BaseController.h"
#include "../utils/Pass2Encoder.h"

// LINUX
#include <sys/prctl.h> // prctl(PR_SET_NAME)

// MTKCAM
#include <mtkcam/utils/std/Log.h>
#include <mtkcam/utils/std/StlUtils.h>
#include <mtkcam/aaa/IHal3A.h> // setIsp, CaptureParam_T
#include <mtkcam/utils/metadata/hal/mtk_platform_metadata_tag.h>
#include <mtkcam/utils/std/Time.h>

// CUSTOM
#include <aaa/ae_param.h>

// AOSP
#include <cutils/compiler.h>
#include <cutils/properties.h> // property_get

// STL
#include <memory> // std::shared_ptr
#include <deque> // std::deque
#include <mutex> // std::mutex
#include <algorithm> // std::find
#include <thread>
#include <chrono>
#include <map> // std::map

// ----------------------------------------------------------------------------
// definitions
// ----------------------------------------------------------------------------
#define THREAD_SLOT_SIZE    1

// ----------------------------------------------------------------------------
// MY_LOG
// ----------------------------------------------------------------------------
#define MY_LOGV(fmt, arg...)        CAM_LOGV("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGD(fmt, arg...)        CAM_LOGD("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGI(fmt, arg...)        CAM_LOGI("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGW(fmt, arg...)        CAM_LOGW("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGE(fmt, arg...)        CAM_LOGE("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGA(fmt, arg...)        CAM_LOGA("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGF(fmt, arg...)        CAM_LOGF("[%s] " fmt, __FUNCTION__, ##arg)
//
#define MY_LOGV_IF(cond, ...)       do { if ( (cond) ) { MY_LOGV(__VA_ARGS__); } }while(0)
#define MY_LOGD_IF(cond, ...)       do { if ( (cond) ) { MY_LOGD(__VA_ARGS__); } }while(0)
#define MY_LOGI_IF(cond, ...)       do { if ( (cond) ) { MY_LOGI(__VA_ARGS__); } }while(0)
#define MY_LOGW_IF(cond, ...)       do { if ( (cond) ) { MY_LOGW(__VA_ARGS__); } }while(0)
#define MY_LOGE_IF(cond, ...)       do { if ( (cond) ) { MY_LOGE(__VA_ARGS__); } }while(0)
#define MY_LOGA_IF(cond, ...)       do { if ( (cond) ) { MY_LOGA(__VA_ARGS__); } }while(0)
#define MY_LOGF_IF(cond, ...)       do { if ( (cond) ) { MY_LOGF(__VA_ARGS__); } }while(0)
//
#ifdef __DEBUG
#define FUNCTION_SCOPE          auto __scope_logger__ = create_scope_logger(__FUNCTION__)
#include <memory>
static std::shared_ptr<char> create_scope_logger(const char* functionName)
{
    char* pText = const_cast<char*>(functionName);
    CAM_LOGD("[%s] + ", pText);
    return std::shared_ptr<char>(pText, [](char* p){ CAM_LOGD("[%s] -", p); });
}
#else
#define FUNCTION_SCOPE
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
#endif // SCOPE_TIMER



using namespace android;
using namespace NSCam::plugin;

using NS3Av3::CaptureParam_T;
using NS3Av3::IHal3A;
//
using NS3Av3::E3ACtrl_GetEvCapture;
using NS3Av3::E3ACtrl_EnableFlareInManualCtrl;
using NS3Av3::E3ACtrl_GetRTParamsInfo;

using NSCam::Utils::TimeTool;

// ----------------------------------------------------------------------------
// FrameDescriptor
// ----------------------------------------------------------------------------
static std::map<MINT32, std::shared_ptr<CollectVendor::FrameDescriptor> >  s_frameDescriptors;
static std::mutex                                                       s_frameDescriptorsLocker;

std::shared_ptr<CollectVendor::FrameDescriptor>
CollectVendor::
takeFrameDescriptor(MINT32 requestNo)
{
    std::shared_ptr<CollectVendor::FrameDescriptor> d;
    {
        std::lock_guard<std::mutex> locker(s_frameDescriptorsLocker);
        auto itr = s_frameDescriptors.find(requestNo);
        if (itr != s_frameDescriptors.end()) { // found
            d = std::move(itr->second);
            s_frameDescriptors.erase(itr);
        }
    }
    return d;
}

void
CollectVendor::
addFrameDescriptor(
        MINT32 requestNo,
        const std::shared_ptr<CollectVendor::FrameDescriptor>& d
        )
{
    std::lock_guard<std::mutex> locker(s_frameDescriptorsLocker);
    s_frameDescriptors[requestNo] = d;
}

void
CollectVendor::
clearFrameDescriptors()
{
    std::lock_guard<std::mutex> locker(s_frameDescriptorsLocker);
    s_frameDescriptors.clear();
}


// ----------------------------------------------------------------------------
static
MBOOL
retrieveShutterAndIso(const IMetadata* pHalMeta, MINT32& _iso, MINT& _shutter)
{
    IMetadata exifMeta;
    MBOOL result = MFALSE;
    struct T {
        MINT32 val;
        MBOOL result;
        T() : val(-1), result(MFALSE) {};
    } iso, exp;

    result = IMetadata::getEntry<IMetadata>(
            const_cast<IMetadata*>(pHalMeta),
            MTK_3A_EXIF_METADATA,
            exifMeta);
    if (result == MFALSE) {
        CAM_LOGE("get MTK_3A_EXIF_METADATA fail");
        goto lbExit;
    }

    iso.result = IMetadata::getEntry<MINT32>(&exifMeta, MTK_3A_EXIF_AE_ISO_SPEED,        iso.val);
    exp.result = IMetadata::getEntry<MINT32>(&exifMeta, MTK_3A_EXIF_CAP_EXPOSURE_TIME,   exp.val);

    if (iso.result) _iso = iso.val;
    if (exp.result) _shutter = exp.val;

lbExit:
    return result;
}

namespace collect_raw
{
using NSCam::plugin::BaseController;
class MyWorker : public BaseController
{
public:
    MyWorker(const char* callerName, int sensorId) : BaseController(callerName)
    {
        m_frameNum = property_get_int32("debug.collect.num", 3);
        m_sensorId = sensorId;
        m_p2Encoder = std::shared_ptr<Pass2Encoder>( new Pass2Encoder(sensorId) );

    };
    virtual ~MyWorker() = default;

public:
    intptr_t job(intptr_t, intptr_t) override
    {
        FUNCTION_SCOPE;

        MINT32 uniqueKey = 0, frameKey = 0;
        MINT64 timeStampNs = 0; // nanosecond

        std::deque<std::shared_ptr<RequestFrame>> usedFrames;
        std::shared_ptr<RequestFrame> mainFrame;

        for (int i = 0; i < m_frameNum; i++) {
            MY_LOGD("dequeFrame [+]");
            auto frame = dequeFrame();
            MY_LOGD("dequeFrame [-]");

            if ( CC_UNLIKELY(frame.get() == nullptr) ) {
                MY_LOGE("dequed a frame but it's null");
                continue;
            }

            if (i == 0) {
                mainFrame = frame;
            }

            // images
            auto pImgSrc = frame->getImageBuffer(RequestFrame::eRequestImg_FullSrc);
            auto pImgLcs = frame->getImageBuffer(RequestFrame::eRequestImg_LcsoRaw);
            auto pImgDst = mainFrame->getImageBuffer(RequestFrame::eRequestImg_FullOut);
            // metadata
            auto pAppMeta = frame->getMetadata(RequestFrame::eRequestMeta_InAppRequest);
            auto pHalMeta = frame->getMetadata(RequestFrame::eRequestMeta_InHalP1);
            auto pAppOut  = frame->getMetadata(RequestFrame::eRequestMeta_OutAppResult);
            auto pHalOut  = frame->getMetadata(RequestFrame::eRequestMeta_OutHalResult);

            if ( CC_UNLIKELY(pImgSrc == nullptr) ){
                MY_LOGE("no full size RAW");
                continue;
            }

            if ( CC_UNLIKELY(pImgDst == nullptr) ) {
                MY_LOGE("no output YUV");
                continue;
            }

            if (pImgLcs == nullptr) {
                MY_LOGD("no lcso raw");
            }

            if(i == 0)
            {
                IMetadata::getEntry<MINT32>(pHalMeta, MTK_PIPELINE_UNIQUE_KEY, uniqueKey);
                IMetadata::getEntry<MINT32>(pHalMeta, MTK_PIPELINE_FRAME_NUMBER, frameKey);
            }
            else
            {
                IMetadata::setEntry<MINT32>(pHalMeta, MTK_PIPELINE_UNIQUE_KEY, uniqueKey);
                IMetadata::setEntry<MINT32>(pHalMeta, MTK_PIPELINE_FRAME_NUMBER, frameKey);
            }

            // retrieve timeStamp
            if ( ! IMetadata::getEntry<MINT64>(pHalMeta, MTK_P1NODE_FRAME_START_TIMESTAMP, timeStampNs) ) {
                MY_LOGW("cannot get timestamp from in hal metadata with tag MTK_SENSOR_TIMESTAMP");
            }
            else {
                MY_LOGD("Timestamp: %" PRId64 "", timeStampNs);
            }

            MY_LOGD("UniqueKey(%d)", uniqueKey);

            IMetadata::setEntry<MINT32>(pHalMeta, MTK_PIPELINE_REQUEST_NUMBER, i);

            // encode RAW to YUV
            Pass2Encoder::ConfigRaw2Yuv _cfg;
            // src
            _cfg.pSrcRaw        = pImgSrc;
            _cfg.pLcsoRaw       = pImgLcs;
            _cfg.pAppMeta       = pAppMeta;
            _cfg.pHalMeta       = pHalMeta;
            // output
            _cfg.pOutput        = pImgDst;
            _cfg.pOutAppMeta    = pAppOut;
            _cfg.pOutHalMeta    = pHalOut;
            // config
            _cfg.isZsd          = MTRUE;

            if (!retrieveScalerCropRgn(pAppMeta, _cfg.cropWindow1)) {
                // set crop region to full size
                _cfg.cropWindow1.p = MPoint(0, 0);
                _cfg.cropWindow1.s = pImgSrc->getImgSize();
                MY_LOGW("no MTK_SCALER_CROP_REGION, set crop region to full size %dx%d",
                _cfg.cropWindow1.s.w, _cfg.cropWindow1.s.h);
            }
            else {
                _cfg.cropWindow1 = calCrop(
                    _cfg.cropWindow1,
                    MRect(MPoint(0, 0), pImgSrc->getImgSize())
                    );
            }

            // go!
            auto err = m_p2Encoder->encodeRaw2Yuv(&_cfg);
            if ( CC_UNLIKELY(err != OK) ) {
                MY_LOGE("encode RAW to YUV failed");
                continue;
            }

            MINT32 iso = 0, shutter = 0;
            if (!retrieveShutterAndIso(pHalMeta, iso, shutter)) {
                MY_LOGE("cannot retrieve shutter and ISO");
            }
            else {
                MY_LOGD("frame iso = %d", iso);
                MY_LOGD("frame exp = %d", shutter);

                // add frame descriptor to CollectVendor
                CollectVendor::FrameDescriptor descriptor;
                descriptor.requestNo = frame->getRequestNo();
                descriptor.iso = iso;
                descriptor.exp = shutter;
                descriptor.isLast = ((i + 1) == m_frameNum) ? MTRUE : MFALSE;

                CollectVendor::addFrameDescriptor(
                        descriptor.requestNo,
                        std::make_shared<CollectVendor::FrameDescriptor>(descriptor) );
            }
            MY_LOGD("output yuv=%p", pImgDst);
            // dump
           auto dump_and_savefile = [&](const char *fileName, IImageBuffer *lImgSrc){
                static std::mutex __m;
                std::lock_guard<std::mutex> __l(__m);
                static int __serial = 0;
                static char __fileName[512];

                if (lImgSrc == nullptr) {
                    MY_LOGD("ignore dump %s, because it's nullptr", fileName);
                    return;
                }

                snprintf(
                        __fileName,
                        sizeof(__fileName),
                        "%s%s-%09d-%04i-%04i-%d-%04i-%d__%dx%d-iso_%d_exp_%d_time_%" PRId64 ".%s",
                        "/sdcard/DCIM/Camera/",
                        "IMG",
                        uniqueKey,
                        0, //TODO
                        i,
                        (m_sensorId+1), //SensorID
                        frameKey,
                        2, //ISP ENUM
                        lImgSrc->getImgSize().w, lImgSrc->getImgSize().h
                        , iso, shutter, (timeStampNs / 1000) /* NS --> US */,
                        fileName
                        );
                __serial++;
                lImgSrc->saveToFile(__fileName);
            };

            dump_and_savefile("raw", pImgSrc);
            dump_and_savefile("unknown", pImgLcs);
            dump_and_savefile("yuy2", pImgDst);

            // move to usedFrames stack
            usedFrames.push_back(frame);
        }

        // use MDP to encode thumbnailYuv
        do {
            // create JPEG YUV and thumbnail YUV using MDP (IImageTransform)
            std::unique_ptr<IImageTransform, std::function<void(IImageTransform*)>> transform(
                    IImageTransform::createInstance(), // constructor
                    [](IImageTransform *p){ if (p) p->destroyInstance(); } // deleter
                    );

            if (transform.get() == nullptr) {
                MY_LOGE("IImageTransform is NULL, cannot generate thumbnail");
                break;
            }

            if (CC_UNLIKELY( mainFrame.get() == nullptr )) {
                MY_LOGE("MainRequestFrame is NULL, cannot generate thumbnail");
                break;

            }
            auto _FullOutYuv = mainFrame->getImageBuffer(RequestFrame::eRequestImg_FullOut);
            auto _ThumbnailOut = mainFrame->getImageBuffer(RequestFrame::eRequestImg_ThumbnailOut);

            if (CC_UNLIKELY( _FullOutYuv == nullptr || _ThumbnailOut == nullptr )) {
                MY_LOGW("FullOutYuv or ThumbnailYuv is null, cannot generate thumbnail");
                break;
            }

            MRect cropRect = calCrop(
                    MRect(MPoint(0, 0), _FullOutYuv->getImgSize()),
                    MRect(MPoint(0, 0), _ThumbnailOut->getImgSize())
                    );

            auto thumbnailTransform = [&]()-> MUINT32{
                auto _t = mainFrame->getOrientation(RequestFrame::eRequestImg_ThumbnailOut);
                switch (_t) {
                case eTransform_ROT_90:
                    return eTransform_ROT_270;
                case eTransform_ROT_270:
                    return eTransform_ROT_90;
                default:
                    return _t;
                }
                return _t;
            }();

            transform->execute(
                    _FullOutYuv,
                    _ThumbnailOut,
                    nullptr,
                    cropRect,
                    thumbnailTransform,
                    3000);
        } while(0);

        onNextCaptureReady();

        mainFrame = nullptr;
        usedFrames.clear();

        m_p2Encoder = nullptr;
        return 0;
    }

// Attributes
public:
    std::shared_ptr<Pass2Encoder>   m_p2Encoder;
    int                             m_frameNum;
    int                             m_sensorId;
}; // class MyWorker
}; // namespace collect_raw

// ----------------------------------------------------------------------------
// Implementations of IVendor
// ----------------------------------------------------------------------------
CollectVendorImp::CollectVendorImp(
        char const*  pcszName,
        MINT32 const i4OpenId,
        MINT64 const vendorMode
    ) noexcept
    : BaseVendor(i4OpenId, vendorMode, String8::format("Collect vendor"))
    , mZsdFlow(MTRUE)
    , m_frame_num(0)
    , m_manual_type(0)
{
    FUNCTION_SCOPE;
    MY_LOGD("[%d]create vendor %" PRId64 " from %s", i4OpenId, vendorMode, pcszName);

    setThreadShotsSize(THREAD_SLOT_SIZE);

    try {
        mHal3A = std::unique_ptr< IHal3A, std::function<void(IHal3A*)> >
        (
            MAKE_Hal3A(i4OpenId, __CALLERNAME__),
            [](IHal3A* p){ if (p) p->destroyInstance(__CALLERNAME__); }
        );
    }
    catch (std::exception&) {
        MY_LOGE("MAKE_Hal3A throws exception");
        mHal3A = nullptr;
    }
}

CollectVendorImp::~CollectVendorImp()
{
    try {
        CollectVendor::clearFrameDescriptors();
    } catch (...) {
        MY_LOGE("clearFrameDescriptors throws exception");
    }
    mHal3A = nullptr;
}

status_t CollectVendorImp::get(
        MINT32           openId         __attribute__((unused)),
        const InputInfo& in             __attribute__((unused)),
        FrameInfoSet&    out            __attribute__((unused)))
{
    FUNCTION_SCOPE;
    SCOPE_TIMER(__t0, __FUNCTION__);

    //Get Manaul frame number
    m_frame_num = property_get_int32("debug.collect.num", 3);
    MY_LOGD("Data collect frame number(%d)", m_frame_num);

    //Get manual type
    m_manual_type = property_get_int32("debug.collect.type", 0);
    MY_LOGD("Data collect manual type(%d)", m_manual_type);

    int frameCount = m_frame_num;
    int frameDelay = COLLECT_FRAME_DELAY;

    {
        auto _set_manual_ev = [&]()
        {
            if(m_manual_type == eManual_EV)
            {
                mvEvSetting.resize(m_frame_num);

                for(int i = 0; i < frameCount; i++)
                {
                    static char get_prop_str[256];

                    snprintf(
                            get_prop_str,
                            sizeof(get_prop_str),
                            "debug.collect.ev%d", i
                            );
                    mvEvSetting[i] = property_get_int32(get_prop_str, 0);
                    MY_LOGD("Get ev[%d]=(%d)", i, mvEvSetting[i]);
                    MY_LOGD("String get(%s)", get_prop_str);
                }
            }
            else if(m_manual_type == eManual_ISO_EXP)
            {
                mvManualExpSetting.resize(m_frame_num);

                static char get_prop_iso_str[256];
                static char get_prop_ex_str[256];

                for(int i = 0; i < frameCount; i++)
                {
                    snprintf(
                            get_prop_iso_str,
                            sizeof(get_prop_iso_str),
                            "debug.collect.iso%d", i
                            );
                    snprintf(
                            get_prop_ex_str,
                            sizeof(get_prop_ex_str),
                            "debug.collect.exp%d", i
                            );
                    mvManualExpSetting[i].m_iso = property_get_int32(get_prop_iso_str, 0);
                    mvManualExpSetting[i].m_shutterUs = property_get_int32(get_prop_ex_str, 0);
                    MY_LOGD("Get iso[%d]=(%d)", i, mvManualExpSetting[i].m_iso);
                    MY_LOGD("Get shutterUs[%d]=(%" PRId64 ")", i, mvManualExpSetting[i].m_shutterUs);
                    MY_LOGD("String iso get(%s)", get_prop_iso_str);
                    MY_LOGD("String shutter get(%s)", get_prop_ex_str);
                }
            }
            else{
                MY_LOGE("Type is should be eManual_EV(0) or eManual_ISO_EXP(1)");
            }
        };
        _set_manual_ev();
    }

    for (int i = 0; i < frameCount; i++) {
        bool bLastFrame = ((i + 1) == frameCount);

        // default, copy from InputInfo.
        MetaItem item;
        item.setting.appMeta = in.appCtrl;
        item.setting.halMeta = in.halCtrl;

        // modify app control metadata
        IMetadata& appSetting(item.setting.appMeta);
        {
            auto _set_manual_app_control = [&]()
            {
                MINT32 iso;
                MINT64 shutterUs;

                if(m_manual_type == eManual_EV)
                {
                    // get the current AE PLINE index
                    NS3Av3::FrameOutputParam_T _paramT;
                    mHal3A->send3ACtrl(E3ACtrl_GetRTParamsInfo, (MINTPTR)&_paramT, 0);
                    int _plineIdx = static_cast<int>(_paramT.u4AEIndex);
                    MY_LOGD("E3ACtrl_GetRTParamsInfo: index(%d)", _plineIdx);
                    strAEOutput _aeOutput;
                    mHal3A->send3ACtrl(
                            E3ACtrl_GetEvCapture,
                            mvEvSetting[i],
                            (MINTPTR)&_aeOutput);
                    // get ISO from the certain index
                    iso = _aeOutput.u4ISO;
                    shutterUs = (MINT64)_aeOutput.EvSetting.u4Eposuretime * 1000; // ms->us
                    // debug
                    MY_LOGD("E3ACtrl_GetEvCapture: ev(%d), iso(%d), shutter(%" PRId64 ")",
                            mvEvSetting[i],
                            iso,
                            shutterUs
                           );

                }
                else if(m_manual_type == eManual_ISO_EXP)
                {
                    iso = mvManualExpSetting[i].m_iso;
                    shutterUs = mvManualExpSetting[i].m_shutterUs * 1000;
                    MY_LOGD("ManualExpSetting: iso(%d), shutter(%" PRId64 ")",
                            iso,
                            shutterUs
                           );
                }
                // manual exposure mode
                IMetadata::setEntry<MUINT8>(&appSetting, MTK_CONTROL_AE_MODE, MTK_CONTROL_AE_MODE_OFF);
                IMetadata::setEntry<MINT32>(&appSetting, MTK_SENSOR_SENSITIVITY, iso);
                IMetadata::setEntry<MINT64>(&appSetting, MTK_SENSOR_EXPOSURE_TIME, shutterUs); // ms->us
                IMetadata::setEntry<MUINT8>(&appSetting, MTK_CONTROL_AWB_LOCK, MTRUE);
            };
            if ( i < frameCount) {
                // lock AWB
                _set_manual_app_control();
            }
        }

        // modify hal control metadata
        IMetadata& halSetting(item.setting.halMeta);
        {
            // pause AF for (N - 1) frames and resume for the last frame
            IMetadata::setEntry<MUINT8>(
                    &halSetting, MTK_FOCUS_PAUSE,
                    bLastFrame ? 0 : 1);
            IMetadata::setEntry<MUINT8>(
                    &halSetting, MTK_HAL_REQUEST_REQUIRE_EXIF, 1);
            IMetadata::setEntry<MUINT8>(
                    &halSetting, MTK_HAL_REQUEST_DUMP_EXIF, 1);
        }

        out.settings.push_back(item);
    }

    // Step 2.2: add delay frames.
    for (int i = 0; i < frameDelay; i++) {
        // default, copy from InputInfo.
        MetaItem item;
        item.setting.appMeta = in.appCtrl;
        item.setting.halMeta = in.halCtrl;
        out.settings.push_back(item);
    }

    // Step 2.3: configure it
    out.table.vendorMode    = mMode;
    out.table.inCategory    = FORMAT_RAW;
    out.table.outCategory   = FORMAT_YUV;
    out.table.bufferType    = BUFFER_NO_WORKING_OUTPUT;
    out.frameCount          = out.settings.size();

    // for manual exposure mode, we need ask for flare offset
    MY_LOGD("ask flare offset -> yes");
    mHal3A->send3ACtrl(E3ACtrl_EnableFlareInManualCtrl, 1, 0);

    // Step 3: create StreamId Map
    // create StreamId Map for RequestFrame
    {
        // {{{
        SCOPE_TIMER (__t1, "Create StreamID map");
        RequestFrame::StreamIdMap_Img map;
        if (CC_LIKELY(in.fullRaw.get())) {
            map[RequestFrame::eRequestImg_FullSrc] = in.fullRaw->getStreamId();
        }
        else {
            MY_LOGE("no full size RAW");
        }

        if (in.resizedRaw.get()) {
            map[RequestFrame::eRequestImg_ResizedSrc] = in.resizedRaw->getStreamId();
        }
        else {
            MY_LOGD("no rrzo");
        }

        // lcso
        if (CC_LIKELY(in.lcsoRaw.get())) {
            map[RequestFrame::eRequestImg_LcsoRaw] = in.lcsoRaw->getStreamId();
        }
        else {
            MY_LOGD("no lcso");
        }

        if (CC_LIKELY(in.jpegYuv.get())) {
            map[RequestFrame::eRequestImg_FullOut] = in.jpegYuv->getStreamId();
        }
        else {
            MY_LOGE("no output YUV");
        }

        if (CC_LIKELY(in.thumbnailYuv.get())) {
            map[RequestFrame::eRequestImg_ThumbnailOut] = in.thumbnailYuv->getStreamId();
        }
        else {
            MY_LOGW("no thumbnail info");
        }

        if (CC_LIKELY(in.workingbuf.get())) {
            map[RequestFrame::eRequestImg_WorkingBufferOut] = in.workingbuf->getStreamId();
        }
        else {
            MY_LOGD("no working buffer info");
        }

        if ( in.vYuv.size() + RequestFrame::eRequestImg_ReservedOut1 >= RequestFrame::eRequestImg_Out_End )
            MY_LOGF("output yuv too much.");

        for(size_t i = 0; i < in.vYuv.size(); ++i) {
            map[RequestFrame::eRequestImg_ReservedOut1 + i] = in.vYuv[i]->getStreamId();
        }
        m_streamIdMap_img = std::move(map);
        // }}}
    }

    return OK;
}


status_t CollectVendorImp::set(
        MINT32              openId              __attribute__((unused)),
        const InputSetting& in                  __attribute__((unused)))
{
    FUNCTION_SCOPE;

    // create a worker
    std::shared_ptr<BaseController> pController(
            new collect_raw::MyWorker(__CALLERNAME__, openId)
            );

    // set request no to worker
    for (MUINT32 i = 0; i < m_frame_num; i++) {
        pController->setBelong(in.vFrame[i].frameNo);
    }
    pController->setMessageCallback(in.pMsgCb);
    pController->setStreamIdMapImg(m_streamIdMap_img);

    // add to stack
    m_controllers.push_back(pController);

    // if the number of used thread slot reaches to max,
    // this method will be blocked until there's a free thread slot.
    asyncRun([this, pController]() mutable {
            ::prctl(PR_SET_NAME, "doTHDR", 0, 0, 0);
            pController->execute();
            pController->waitExecution();
            m_controllers.remove(pController);
            pController = nullptr;
        });

    return OK;
}


status_t CollectVendorImp::queue(
        MINT32  const              openId           __attribute__((unused)),
        MUINT32 const              requestNo        __attribute__((unused)),
        wp<IVendor::IDataCallback> cb               __attribute__((unused)),
        BufferParam                bufParam         __attribute__((unused)),
        MetaParam                  metaParam        __attribute__((unused)))
{
    FUNCTION_SCOPE;

#ifdef __DEBUG
    MY_LOGI("queue frame, requestNo=%u", requestNo);
#endif

    auto pController = m_controllers.belong_to(requestNo);
    if ( CC_UNLIKELY(pController == nullptr) ) {
        MY_LOGW("this frame is not belong to anyone");
        drop(openId, cb, bufParam, metaParam);
        return OK;
    }

    MY_LOGD("enque frame");
    pController->enqueFrame(
            openId,
            requestNo,
            bufParam,
            metaParam,
            cb);

    if (pController->getEnquedTimes() == m_frame_num) {
        pController->onShutter();
        // flare offset off
        MY_LOGD("ask flare offset -> no");
        mHal3A->send3ACtrl(E3ACtrl_EnableFlareInManualCtrl, 0, 0);
    }

    return OK;
}


status_t CollectVendorImp::beginFlush(MINT32 /*openId*/)
{
    FUNCTION_SCOPE;
    drain(); // cancel flow, and drain.
    return OK;
}


status_t CollectVendorImp::endFlush(MINT32 /*openId*/)
{
    FUNCTION_SCOPE;
    return OK;
}


status_t CollectVendorImp::sendCommand(
        MINT32      /* cmd */,
        MINT32      /* openId */,
        MetaItem&   /* meta */,
        MINT32&     /* arg1 */,
        MINT32&     /* arg2 */,
        void*       /* arg3 */
    )
{
    FUNCTION_SCOPE;
    return OK;
}


status_t CollectVendorImp::dump(MINT32 /* openId */)
{
    FUNCTION_SCOPE;
    return OK;
}

void CollectVendorImp::drain(bool bCancel /* = true */)
{
    syncAllThread();
    auto CC = std::move(m_controllers);
    CC.clear();
}


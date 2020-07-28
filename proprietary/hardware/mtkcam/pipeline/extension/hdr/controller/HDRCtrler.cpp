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

#define LOG_TAG "HdrController"
static const char* __CALLERNAME__ = LOG_TAG;

#include "HDRCtrler.h"
#include "../utils/VendorUtils.h"
#include "../utils/Pass2Encoder.h"

// LINUX
#include <sys/prctl.h> // prctl(PR_SET_NAME)

// MTKCAM
#include <mtkcam/utils/std/Log.h>
#include <mtkcam/utils/exif/DebugExifUtils.h>
#include <custom/aaa/AEPlinetable.h> // strAETable
#include <mtkcam/drv/IHalSensor.h> // sensor type
#include <mtkcam/utils/metastore/IMetadataProvider.h> // static metadata
#include <mtkcam/feature/utils/ImageBufferUtils.h>
#include <mtkcam/custom/ExifFactory.h>
#include <cutils/properties.h>

// STL
#include <atomic>
#include <cstdint>
#include <functional> // std::function
#include <mutex>


#define HDRCTRLER_DEQUE_FRAME_TIMEOUT1                  3000 // 1st deque time out (1.5s)
#define HDRCTRLER_DEQUE_FRAME_TIMEOUT2                  500  // following deque time out (0.15s)


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
using namespace NSCam::HDR2;
using namespace NS3Av3;

// ----------------------------------------------------------------------------
// Static function
// ----------------------------------------------------------------------------


template<typename T>
static MBOOL GET_ENTRY_ARRAY(const NSCam::IMetadata& metadata, MINT32 entry_tag, T* array, MUINT32 size)
{
    NSCam::IMetadata::IEntry entry = metadata.entryFor(entry_tag);
    if (entry.tag() != NSCam::IMetadata::IEntry::BAD_TAG &&
        entry.count() == size)
    {
        for (MUINT32 i = 0; i < size; i++)
        {
            *array++ = entry.itemAt(i, NSCam::Type2Type< T >());
        }
        return MTRUE;
    }
    return MFALSE;
}

// ----------------------------------------------------------------------------
// HDRCtrler
// ----------------------------------------------------------------------------
HDRCtrler::HDRCtrler(int sensorId, bool enableLog, bool zsdFlow)
    : BaseController(__CALLERNAME__)
    , m_openId(sensorId)
    , m_captureNum(0)
    , m_enableLog(enableLog)
    , m_zsdFlow(zsdFlow)
    , m_bFired(false)
    , m_delayFrame(0)
    , m_droppedFrameNum(0)
    , m_bNeedExifInfo(0)
    , m_hdrDone(false)
    , mAlgoMode(0)
{
    FUNCTION_SCOPE;
    SCOPE_TIMER(__t0, __FUNCTION__);

    MY_LOGD("init HDR proc");

    // initialize HDR proc
    IHDRProc2& hdrProc(HDRProc2Factory::getIHDRProc2());

    // get HDR handle and set complete callback
    if (hdrProc.init(m_openId, m_HDRHandle) != MTRUE)
    {
        MY_LOGE("init HDR proc failed");
        return;
    }

    hdrProc.setCompleteCallback(m_HDRHandle, HDRProcCompleteCallback, this);

    mAlgoMode = hdrProc.queryHDRAlgoType(m_HDRHandle);

    MY_LOGD("HDR algo type(%d)", mAlgoMode);

    if ( !getCurrentCaptureParam(m_zsdFlow) ) {
        MY_LOGE("get capture param fail.");
        return;
    }
}


HDRCtrler::~HDRCtrler()
{
    FUNCTION_SCOPE;
}


intptr_t HDRCtrler::job(
        intptr_t arg1 /* = 0 */     __attribute__((unused)),
        intptr_t arg2 /* = 0 */     __attribute__((unused)))
{
    doHdr();
    return 0;
}


void HDRCtrler::makesDebugInfo(
        const std::map<unsigned int, uint32_t>& data,
        IMetadata*                              pMetadata)
{
    FUNCTION_SCOPE;
    SCOPE_TIMER(__t0, __FUNCTION__);

    if (pMetadata == nullptr) {
        MY_LOGE("%s: pMetadata is NULL", __FUNCTION__);
        return;
    }

    IMetadata exifMeta;
    {
        IMetadata::IEntry entry = pMetadata->entryFor(MTK_3A_EXIF_METADATA);
        if (entry.isEmpty()) {
            MY_LOGW("%s: no MTK_3A_EXIF_METADATA can be used", __FUNCTION__);
            return;
        }

        exifMeta = entry.itemAt(0, Type2Type<IMetadata>());
    }

    /* set debug information into debug Exif metadata */
    DebugExifUtils::setDebugExif(
            DebugExifUtils::DebugExifType::DEBUG_EXIF_MF,
            static_cast<MUINT32>(MTK_MF_EXIF_DBGINFO_MF_KEY),
            static_cast<MUINT32>(MTK_MF_EXIF_DBGINFO_MF_DATA),
            data,
            &exifMeta);

    /* update debug Exif metadata */
    IMetadata::IEntry entry_exif(MTK_3A_EXIF_METADATA);
    entry_exif.push_back(exifMeta, Type2Type<IMetadata>());
    pMetadata->update(entry_exif.tag(), entry_exif);
}

void HDRCtrler::calCropRegin(std::shared_ptr<RequestFrame> pFrame, MRect& crop, MSize dstSize)
{
    // query active array size
    MRect activeArray;
    sp<IMetadataProvider> pMetadataProvider = NSMetadataProviderManager::valueFor(getOpenId());
    IMetadata static_meta = pMetadataProvider->getMtkStaticCharacteristics();
    if (!IMetadata::getEntry<MRect>(
                &static_meta, MTK_SENSOR_INFO_ACTIVE_ARRAY_REGION, activeArray)) {
        MY_LOGE("no ective array size.");
        return;
    }

    IMetadata* pAppMeta = pFrame->getMetadata(RequestFrame::eRequestMeta_InAppRequest);
    IMetadata* pHalMeta = pFrame->getMetadata(RequestFrame::eRequestMeta_InHalP1);

    MRect origCropRegion;
    if (!retrieveScalerCropRgn(pAppMeta, pHalMeta, origCropRegion)) {
        // set crop region to full size
        origCropRegion.p = MPoint(0, 0);
        origCropRegion.s = activeArray.s;
        MY_LOGW("no MTK_SCALER_CROP_REGION, set crop region to full size %dx%d",
        origCropRegion.s.w, origCropRegion.s.h);
    }

    simpleTransform tranActive2Sensor =
        simpleTransform(MPoint(0, 0), activeArray.s, dstSize);

    // apply transform
    crop = transform(tranActive2Sensor, origCropRegion);
}

bool HDRCtrler::prepareSingleFrameHDR(std::shared_ptr<RequestFrame> pFrame)
{
    FUNCTION_SCOPE;
    SCOPE_TIMER(__t0, __FUNCTION__);

    // main yuv pool
    {
        IImageBuffer* pImageBuffer = pFrame->getImageBuffer(RequestFrame::eRequestImg_FullSrc);
        MSize size        = pImageBuffer->getImgSize();
        MINT format       = eImgFmt_YV12;
        MUINT const usage = 0; //not necessary here
        MUINT32 transform = 0;
        sp<v3::IImageStreamInfo> pStreamInfo =
            createImageStreamInfo(
                "hdrVendor:MainYuv",
                0, eSTREAMTYPE_IMAGE_INOUT,
                getCaptureNum(), getCaptureNum(),
                usage, format, size, transform
                );
        m_mainYuvPool = new WorkPool(true);
        m_mainYuvPool->allocate(pStreamInfo);
    }

    return true;
}

bool HDRCtrler::prepareMultiFrameHDR(std::shared_ptr<RequestFrame> pFrame)
{
    FUNCTION_SCOPE;
    SCOPE_TIMER(__t0, __FUNCTION__);

    /* retrieve metadata */
    IMetadata* pAppMeta = pFrame->getMetadata(RequestFrame::eRequestMeta_InAppRequest);
    IMetadata* pHalMeta = pFrame->getMetadata(RequestFrame::eRequestMeta_InHalP1);

    // sensor size
    MSize size_sensor;
    IMetadata::getEntry<MSize>(
            pHalMeta, MTK_HAL_REQUEST_SENSOR_SIZE, size_sensor);
    MY_LOGD("hal request sensor size(%dx%d)", size_sensor.w, size_sensor.h);

    IHDRProc2& hdrProc(HDRProc2Factory::getIHDRProc2());
    const HDRHandle& hdrHandle(m_HDRHandle);

    // set pipeline's unique key to hdrproc for debug purpose
    MINT32 uniqueKey = 0;
    if ( IMetadata::getEntry<MINT32>( pHalMeta, MTK_PIPELINE_UNIQUE_KEY, uniqueKey) ) {
        hdrProc.setParam(
            hdrHandle,
            HDRProcParam_Set_SequenceNumber,
            uniqueKey, 0);
    }

    // crop
    {
        MRect cropRegion;
        // sensor size
        MSize size_sensor;
        IMetadata::getEntry<MSize>(
                pHalMeta, MTK_HAL_REQUEST_SENSOR_SIZE, size_sensor);
        MY_LOGD("hal request sensor size(%dx%d)", size_sensor.w, size_sensor.h);
        calCropRegin(pFrame, cropRegion, size_sensor);

        // TODO: set the corresponding postview size
        MSize postviewSize(800, 600);
        hdrProc.setShotParam(hdrHandle, size_sensor, postviewSize, cropRegion);
    }

    hdrProc.setParam(
            hdrHandle,
            HDRProcParam_Set_sensor_size, size_sensor.w, size_sensor.w);
    // TODO sensor type
    hdrProc.setParam(hdrHandle, HDRProcParam_Set_sensor_type, SENSOR_TYPE_RAW, 0);
    hdrProc.prepare(hdrHandle);
    //
    MUINT32 uSrcMainFormat = 0;
    MUINT32 uSrcMainWidth = 0;
    MUINT32 uSrcMainHeight = 0;
    MUINT32 uSrcSmallFormat = 0;
    MUINT32 uSrcSmallWidth = 0;
    MUINT32 uSrcSmallHeight = 0;
    MUINT32 empty = 0;

    hdrProc.getParam(
            hdrHandle,
            HDRProcParam_Get_src_main_format, uSrcMainFormat, empty);
    hdrProc.getParam(
            hdrHandle,
            HDRProcParam_Get_src_main_size, uSrcMainWidth, uSrcMainHeight);
    hdrProc.getParam(
            hdrHandle,
            HDRProcParam_Get_src_small_format, uSrcSmallFormat, empty);
    hdrProc.getParam(
            hdrHandle,
            HDRProcParam_Get_src_small_size, uSrcSmallWidth, uSrcSmallHeight);

    {
        MSize size        = MSize(uSrcMainWidth, uSrcMainHeight);
        MINT format       = uSrcMainFormat;
        MUINT const usage = 0; //not necessary here
        MUINT32 transform = 0;
        sp<v3::IImageStreamInfo> pStreamInfo =
            createImageStreamInfo(
                "hdrVendor:MainYuvBuffer",
                0, eSTREAMTYPE_IMAGE_INOUT,
                getCaptureNum(), getCaptureNum(),
                usage, format, size, transform
                );
        m_mainYuvPool = new WorkPool(true);
        m_mainYuvPool->allocateFromBlob(pStreamInfo);
    }

    // MTK flow. Allocate y8 pool
    if(!mAlgoMode)
    {
        MSize size        = MSize(uSrcSmallWidth, uSrcSmallHeight);
        MINT format       = uSrcSmallFormat;
        MUINT const usage = 0; //not necessary here
        MUINT32 transform = 0;
        sp<v3::IImageStreamInfo> pStreamInfo =
            createImageStreamInfo(
                "hdrVendor:Y8",
                0, eSTREAMTYPE_IMAGE_INOUT,
                getCaptureNum(), getCaptureNum(),
                usage, format, size, transform
                );
        m_Y8Pool = new WorkPool(true);
        m_Y8Pool->allocateFromBlob(pStreamInfo);

    }

    return true;
}

bool HDRCtrler::checkFrame(std::shared_ptr<RequestFrame> pFrame)
{
    /* retrieve metadata */
    IMetadata* pAppMeta         = pFrame->getMetadata(RequestFrame::eRequestMeta_InAppRequest);
    IMetadata* pHalMeta         = pFrame->getMetadata(RequestFrame::eRequestMeta_InHalP1);
    /* retrieve image buffer that we care */
    IImageBuffer* pImageBuffer  = pFrame->getImageBuffer(RequestFrame::eRequestImg_FullSrc);
    IImageBuffer* pImageLcsoRaw = pFrame->getImageBuffer(RequestFrame::eRequestImg_LcsoRaw);

    // check these metadata and image buffer
    if (pAppMeta == nullptr) {
        MY_LOGD("no in app request metadata");
        // it's ok if no AppMeta...maybe.
    }
    if (pHalMeta == nullptr) {
        MY_LOGE("no in hal P1 metadata, fatal error");
        m_droppedFrameNum++;
        return false;
    }
    if (pImageBuffer == nullptr) {
        MY_LOGE("no src image (full size), fatal error");
        m_droppedFrameNum++;
        return false;
    }
    if (pImageLcsoRaw == nullptr) {
        MY_LOGD("no src image (lcso), it's ok");
    }

    // MTK flow check if LTM configration is sent
    if(!mAlgoMode)
    {
        MINT32 bypassLTM = 0;
        if ( !IMetadata::getEntry<MINT32>(
                pHalMeta, MTK_3A_ISP_BYPASS_LCE, bypassLTM) ) {
            MY_LOGD("no MTK_3A_ISP_BYPASS_LCE.");
            return false;
        }
        MY_LOGD("bypassLTM[%u](%d)", pFrame->getRequestNo(), bypassLTM);
    }

    return true;
}

bool HDRCtrler::doHdr()
{
    FUNCTION_SCOPE;
    SCOPE_TIMER(__t0, __FUNCTION__);

    if (m_bFired) {
        MY_LOGE("HDR has been fired, cannot fire twice");
        return false;
    }

    m_bFired = true;

    sp<Ctrl> mainCtrl  = new Ctrl();
    sp<Ctrl> smallCtrl = new Ctrl();
    IHDRProc2& hdrProc(HDRProc2Factory::getIHDRProc2());

    std::deque<std::shared_ptr<RequestFrame>> usedFrames;

    bool isSingleFrame = false;
    // deque frames
    for (size_t i = 0; i < static_cast<size_t>(getCaptureNum()); i++) {
        MY_LOGD("deque RequestFrame %zu", i);

        // deque frame with a timeout (in ms)
        std::shared_ptr<RequestFrame> pFrame;
        const int timeoutMs = [i](){
            return (i == 0) ? HDRCTRLER_DEQUE_FRAME_TIMEOUT1 : HDRCTRLER_DEQUE_FRAME_TIMEOUT2;
        }();
        MY_LOGD("dequeFrame(%d)[+]", i);
        auto status = HDRCtrler::dequeFrame(pFrame, timeoutMs);
        if (CC_UNLIKELY( status == std::cv_status::timeout )) {
            MY_LOGE("deque frame(%d) timeout (%dms), ignore this frame", i, timeoutMs);
            pFrame = nullptr;
        }
        MY_LOGD("dequeFrame(%d)[-]", i);


        // check frame.
        if (pFrame.get() == nullptr || pFrame->isBroken()) {
            m_droppedFrameNum++;
            if (pFrame.get() == nullptr) {
                MY_LOGE("dequed a empty RequestFrame, idx=%zu", i);
            }
            else if (pFrame->isBroken()) {
                MY_LOGE("dequed a RequestFrame but marked as error, idx=%zu", i);
            }

            // Drop frame is not allowed in HDR capture.
            m_droppedFrameNum = getCaptureNum();
            break;
        }

        if ( !checkFrame(pFrame) ) continue;

        /* retrieve metadata */
        IMetadata* pAppMeta         = pFrame->getMetadata(RequestFrame::eRequestMeta_InAppRequest);
        IMetadata* pHalMeta         = pFrame->getMetadata(RequestFrame::eRequestMeta_InHalP1);
        IMetadata* pAppRawMeta      = pFrame->getMetadata(RequestFrame::eRequestMeta_InFullRaw);
        IMetadata* pAppMetaOut      = pFrame->getMetadata(RequestFrame::eRequestMeta_OutAppResult);
        IMetadata* pHalMetaOut      = pFrame->getMetadata(RequestFrame::eRequestMeta_OutHalResult);
        /* retrieve image buffer that we care */
        IImageBuffer* pImageBuffer  = pFrame->getImageBuffer(RequestFrame::eRequestImg_FullSrc);
        IImageBuffer* pImageLcsoRaw = pFrame->getImageBuffer(RequestFrame::eRequestImg_LcsoRaw);

        if(i == 0) {
            generateFileInfo(pFrame);
        }

        if(i == 1) {
            //Get AWB RGB gain for arcsoft algo
            MRational NeutralColorPt[3];
            HDR_AWB_Gain awbGain;

            GET_ENTRY_ARRAY(*pAppRawMeta, MTK_SENSOR_NEUTRAL_COLOR_POINT, NeutralColorPt, 3);

            awbGain.rGain = NeutralColorPt[0].denominator;
            awbGain.gGain = NeutralColorPt[1].denominator;
            awbGain.bGain = NeutralColorPt[2].denominator;

            MY_LOGD("AWB gain(%d, %d, %d)" , NeutralColorPt[0].denominator
                                           , NeutralColorPt[1].denominator
                                           , NeutralColorPt[2].denominator);

            hdrProc.setParam(m_HDRHandle, HDRProcParam_Set_AWB_Gain, (MUINTPTR)&awbGain, 0);
        }

        if ( !m_mainYuvPool.get() ) {
            if ( getCaptureNum() > 1 ) {
                if ( !prepareMultiFrameHDR(pFrame) ) {
                    MY_LOGE("prepareMultiFrameHDR fail.");
                    continue;
                }
            } else {
                if ( !prepareSingleFrameHDR(pFrame) ) {
                    MY_LOGE("prepareSingleFrameHDR fail.");
                    continue;
                }
            }
        }

        // first frame is always a main frame.
        if (m_mainRequestFrame.get() == nullptr) {
            bool isMain = false;
            int begin = static_cast<int>(RequestFrame::eRequestImg_Out_Start);
            int end   = static_cast<int>(RequestFrame::eRequestImg_Out_End);
            for (int i = begin; i < end; i++){
                auto reqId = static_cast<RequestFrame::eRequestImg>(i);
                if ( pFrame->getImageBuffer(reqId) != nullptr ) {
                    isMain = true;
                    break;
                }
            }
            if ( isMain ) {
                m_mainRequestFrame = pFrame;

                // update request Exif
                MUINT8 isRequestExif = 0;
                tryGetMetadata<MUINT8>(
                        pFrame->getMetadata(RequestFrame::eRequestMeta_InHalP1),
                        MTK_HAL_REQUEST_REQUIRE_EXIF,
                        isRequestExif);

                m_bNeedExifInfo = (isRequestExif != 0);
                //
                onShutter(); //
            }
        } // fi (frame 0)

        isSingleFrame = processSingleFrame(pFrame);

        if(CC_UNLIKELY(isSingleFrame)) {
            MY_LOGD("Its single frame");
            break;
        }

        //YUV domain HDR convert raw to yuv
        {
            if(!mAlgoMode)
            {
                Pass2Encoder p2Enc(m_openId);
                Pass2Encoder::ConfigRaw2Yuv config;
                // input
                config.pSrcRaw     = pImageBuffer;
                config.pLcsoRaw    = pImageLcsoRaw;
                config.pAppMeta    = pAppMeta;
                config.pHalMeta    = pHalMeta;

                // output
                config.pOutput     = mainCtrl->get(m_mainYuvPool);
                config.pOutput2    = smallCtrl->get(m_Y8Pool);
                config.pOutAppMeta = pAppMetaOut;
                config.pOutHalMeta = pHalMetaOut;

                if ( p2Enc.encodeRaw2Yuv(&config) != OK ) {
                    MY_LOGE("p2 encode fail.");
                    continue;
                }

                MINT32 dumpBuffer = property_get_int32("debug.camera.vendor.dump", 0);
                if( dumpBuffer != 0 ) {
                    String8 filename = String8::format("/sdcard/DCIM/Camera/I420_%d_%dx%d_.yuv",
                            pFrame->getRequestNo(), config.pOutput->getImgSize().w, config.pOutput->getImgSize().h);
                    String8 filename2 = String8::format("/sdcard/DCIM/Camera/y8_%d_%dx%d_.yuv",
                            pFrame->getRequestNo(), config.pOutput2->getImgSize().w, config.pOutput2->getImgSize().h);
                    MY_LOGD("save main yuv %s", filename.string());
                    config.pOutput->saveToFile(filename);
                    MY_LOGD("save small yuv %s", filename2.string());
                    config.pOutput2->saveToFile(filename2);
                }

                //Release imagebuffer back to pipeline
                releasePartialImages(pFrame, true, true);

                if( getCaptureNum() > 1 ) {
                    size_t index = i << 1;
                    hdrProc.addInputFrame(m_HDRHandle, index+0, config.pOutput);
                    hdrProc.addInputFrame(m_HDRHandle, index+1, config.pOutput2);

                } else {
                    setResultBuffer(config.pOutput);
                }
            }
            else
            {
                // move to usedFrames stack to keep image frames
                usedFrames.push_back(pFrame);
                hdrProc.addInputFrame(m_HDRHandle, i, pImageBuffer);
            }

        }

    } // for-loop: for every frames

    // do HDR
    {
        // bad request set
        if ( m_droppedFrameNum > 0 || isSingleFrame) {
            cleanUp();
            return true;
        }

        //
        hdrProc.start(m_HDRHandle);
        //

        // wait Hdr done
        std::unique_lock<std::mutex> locker(m_HdrDoneMx);
        MY_LOGD("Wait Hdr");
        if(!m_hdrDone)
        {
            m_HdrDoneCond.wait(locker);
        }
        onNextCaptureReady();
        MY_LOGD("Hdr gogo");

        // handle output yuv buffer
        processMixedYuv();

        // return working buffer to pool after HDR done
        mainCtrl  = nullptr;
        smallCtrl = nullptr;

        if(mAlgoMode)
        {
            // Release
            usedFrames.clear();
        }
        // clear resources.
        cleanUp();
    }

    return true;
}

void HDRCtrler::releasePartialImages(
    std::shared_ptr<RequestFrame> pFrame,
    bool sourceRelease,
    bool extraRelease
)
{
    FUNCTION_SCOPE;
    SCOPE_TIMER(__t0, __FUNCTION__);

    if ( sourceRelease ) {
        // partial release all source images
        if (CC_LIKELY(pFrame.get())) {
            int begin = static_cast<int>(RequestFrame::eRequestImg_Src_Start);
            int end   = static_cast<int>(RequestFrame::eRequestImg_Src_End);

            for (int i = begin; i < end; i++){
                auto reqId = static_cast<RequestFrame::eRequestImg>(i);
                pFrame->releaseImageBuffer(reqId);
            }
        }
    }
    if ( extraRelease ) {
        if (CC_LIKELY(pFrame.get())) {
            int begin = static_cast<int>(RequestFrame::eRequestImg_Out_Extra_Start);
            int end   = static_cast<int>(RequestFrame::eRequestImg_Out_Extra_End);

            for (int i = begin; i < end; i++){
                auto reqId = static_cast<RequestFrame::eRequestImg>(i);
                pFrame->markError(reqId);
                pFrame->releaseImageBuffer(reqId);
            }
        }
    }
    return;
}

MBOOL HDRCtrler::HDRProcCompleteCallback(
        void* user, const sp<IImageBuffer>& hdrResult, MBOOL ret)
{
    HDRCtrler* self = reinterpret_cast<HDRCtrler*>(user);
    if (NULL == self) {
        MY_LOGE("HDRProcCompleteCallback with NULL user");
        return MFALSE;
    }

    MY_LOGD("HDRProcCompleteCallback ret(%d)", ret);

    self->setResultBuffer(hdrResult);

    return MTRUE;
}

void HDRCtrler::setResultBuffer(sp<IImageBuffer> b)
{
    FUNCTION_SCOPE;
    SCOPE_TIMER(__t0, __FUNCTION__);

    std::lock_guard<std::mutex> __l(m_HdrDoneMx);
    m_HdrResult = b;
    m_hdrDone = true;
    m_HdrDoneCond.notify_one();
}

void HDRCtrler::processMixedYuv()
{
    FUNCTION_SCOPE;
    SCOPE_TIMER(__t0, __FUNCTION__);

    // retrieve mixed YUV image buffer
    IImageBuffer* pHDRResult = m_HdrResult.get();
    //Used for raw domain HDR
    sp<Ctrl> yuvCtrl  = new Ctrl();
    auto workingYuv = yuvCtrl->get(m_mainYuvPool);
    //HDR result for yuv domain
    sp<IImageBuffer> pResult = nullptr;


    if (pHDRResult == nullptr) {
        MY_LOGE("IMfllImageBuffer is not NULL but result in it is NULL");
        return;
    }

    // process pResult to output YUV.
    if (m_mainRequestFrame.get() == nullptr) {
        MY_LOGE("main request frame is NULL");
        return;
    }

    auto appMetaIn     = m_mainRequestFrame->getMetadata(RequestFrame::eRequestMeta_InAppRequest);
    auto halMetaIn     = m_mainRequestFrame->getMetadata(RequestFrame::eRequestMeta_InHalP1);
    auto appMetaOut    = m_mainRequestFrame->getMetadata(RequestFrame::eRequestMeta_OutAppResult);
    auto halMetaOut    = m_mainRequestFrame->getMetadata(RequestFrame::eRequestMeta_OutHalResult);
    //
    auto pFrameYuvJpeg = m_mainRequestFrame->getImageBuffer(RequestFrame::eRequestImg_FullOut);
    auto pFrameYuvThub = m_mainRequestFrame->getImageBuffer(RequestFrame::eRequestImg_ThumbnailOut);
    auto pFrameWorking = m_mainRequestFrame->getImageBuffer(RequestFrame::eRequestImg_WorkingBufferOut);
    auto outOrientaion = m_mainRequestFrame->getOrientation(RequestFrame::eRequestImg_FullOut);
    auto requestNo     = m_mainRequestFrame->getRequestNo();


    //Used for dump buffer
    String8 fileResultname;
    MINT32 dumpBuffer = property_get_int32("debug.camera.vendor.dump", 0);

    if(mAlgoMode)
    {
        //Raw2YUV
        Pass2Encoder p2Enc(m_openId);
        Pass2Encoder::ConfigRaw2Yuv config;

        // if using RAW, we have to give the bayer order info
        {
            // Get sensor format
            IHalSensorList *const pIHalSensorList = MAKE_HalSensorList(); // singleton, no need to release
            if (pIHalSensorList) {
                MUINT32 sensorDev = (MUINT32) pIHalSensorList->querySensorDevIdx(m_openId);
                NSCam::SensorStaticInfo sensorStaticInfo;
                memset(&sensorStaticInfo, 0, sizeof(NSCam::SensorStaticInfo));
                pIHalSensorList->querySensorStaticInfo(sensorDev, &sensorStaticInfo);
                pHDRResult->setColorArrangement(static_cast<MINT32>(sensorStaticInfo.sensorFormatOrder));
            }
        }
        // input
        config.pSrcRaw     = pHDRResult;
        config.pAppMeta    = appMetaIn;
        config.pHalMeta    = halMetaIn;

        // output
        config.pOutput     = workingYuv;
        config.pOutAppMeta = appMetaOut;
        config.pOutHalMeta = halMetaOut;

        pResult = workingYuv;

        if ( p2Enc.encodeRaw2Yuv(&config) != OK ) {
            MY_LOGE("p2 encode fail.");
        }

        //For debug
        fileResultname = String8::format("%s_%s_%dx%d_.raw"
            , m_filename
            , "AlgoResultRaw"
            , m_HdrResult->getImgSize().w
            , m_HdrResult->getImgSize().h);
    }
    else
    {
        //For debug
        fileResultname = String8::format("%s_%dx%d_.yuv"
            , m_filename
            , m_HdrResult->getImgSize().w
            , m_HdrResult->getImgSize().h);

        pResult = pHDRResult;
    }

    // handle yuv output
    bool ret = true;
    IImageBuffer* dst1 = (pFrameYuvJpeg == nullptr) ? pFrameWorking : pFrameYuvJpeg;
    IImageBuffer* dst2 = pFrameYuvThub;
    std::string str = (pFrameYuvJpeg == nullptr) ? "Process output working buffer."
                                                 : "Process output capture buffer.";
    bool needCrop = (pFrameYuvJpeg == nullptr) ? false : true;
    bool needClearZoom = (pFrameYuvJpeg == nullptr) ? false : true;
    ret  = handleYuvOutput(str,
                pResult.get(), dst1, dst2,
                appMetaIn, halMetaIn,
                getOpenId(), outOrientaion, requestNo, needClearZoom, needCrop);
    if (!ret) MY_LOGE("handle yuv output fail.");

    if( dumpBuffer != 0 ) {
        MY_LOGD("HDR save hdr result %s", fileResultname.string());
        m_HdrResult->saveToFile(fileResultname);

        //For debug MDP result
        String8 fileResultYuvname;

        fileResultYuvname = String8::format("%s_%s_%dx%d_.yuv"
            , m_filename
            , "AlgoResultYuv"
            , pFrameYuvJpeg->getImgSize().w
            , pFrameYuvJpeg->getImgSize().h);
        pFrameYuvJpeg->saveToFile(fileResultYuvname);
    }

    // append debug exif if need
    if (m_bNeedExifInfo) {
        // add HDR image flag
        auto const inst = MAKE_DebugExif();
        std::map<MUINT32, MUINT32> debugInfoList;
        debugInfoList[inst->getTagId_MF_TAG_IMAGE_HDR()] = 1;
        makesDebugInfo(debugInfoList, halMetaOut);
    }
}

void HDRCtrler::cleanUp()
{
    FUNCTION_SCOPE;
    SCOPE_TIMER(__t0, __FUNCTION__);

    // temp
    if(getCaptureNum() > 1)
        ImageBufferUtils::getInstance().deallocBuffer(m_HdrResult);

    invalidate();

    m_HdrResult        = nullptr;
    m_mainYuvPool      = nullptr;
    m_Y8Pool           = nullptr;
    m_mainRequestFrame = nullptr;

    // release hdrproc
    IHDRProc2& hdrProc(HDRProc2Factory::getIHDRProc2());
    const HDRHandle& hdrHandle(m_HDRHandle);

    hdrProc.release(m_HDRHandle);
    hdrProc.uninit(m_HDRHandle);
}

void HDRCtrler::doCancel()
{
    FUNCTION_SCOPE;
    // wait until HDR finished
    waitExecution();
}


bool HDRCtrler::checkHdrParam()
{
    FUNCTION_SCOPE;
    SCOPE_TIMER(__t0, __FUNCTION__);

    // check source resolution is valid or not.
    if (getSizeSrc().w <= 0 || getSizeSrc().h <= 0) {
        MY_LOGE("source resolution is invalid (w,h)=(%d,%d)",
                getSizeSrc().w, getSizeSrc().h);
        return false;
    }

    // check capture number
    if (getCaptureNum() <= 0) {
        MY_LOGE("capture number is wrong (0)");
        return false;
    }

    return true;
}

bool HDRCtrler::getCurrentCaptureParam(bool bZsdFlow)
{
    FUNCTION_SCOPE;
    SCOPE_TIMER(__t0, __FUNCTION__);

    std::unique_ptr <
                    IHal3A,
                    std::function<void(IHal3A*)>
                    > hal3a
            (
                MAKE_Hal3A(m_openId, __CALLERNAME__),
                [](IHal3A* p){ if (p) p->destroyInstance(__CALLERNAME__); }
            );

    if (hal3a.get() == nullptr) {
        MY_LOGE("create IHal3A instance failed");
        return false;
    }

    {
        static std::mutex __locker;
        std::lock_guard<std::mutex> __l(__locker);
    }

    // copy hdr handle
    HDRHandle& hdrHandle(m_HDRHandle);

    if (hdrHandle.isExposureConfigured)
    {
        MY_LOGE("exposure setting has been configured");
        return false;
    }

    // get exposure setting from 3A
    ExpSettingParam_T rExpSetting;
    hal3a->send3ACtrl(
            E3ACtrl_GetExposureInfo, reinterpret_cast<MINTPTR>(&rExpSetting), 0);

    IHDRProc2& hdrProc(HDRProc2Factory::getIHDRProc2());

    // set exposure setting to HDR proc
    hdrProc.setParam(
            hdrHandle,
            HDRProcParam_Set_AOEMode, rExpSetting.u4AOEMode, 0);
    hdrProc.setParam(
            hdrHandle,
            HDRProcParam_Set_MaxSensorAnalogGain, rExpSetting.u4MaxSensorAnalogGain, 0);
    hdrProc.setParam(
            hdrHandle,
            HDRProcParam_Set_MaxAEExpTimeInUS, rExpSetting.u4MaxAEExpTimeInUS, 0);
    hdrProc.setParam(
            hdrHandle,
            HDRProcParam_Set_MinAEExpTimeInUS, rExpSetting.u4MinAEExpTimeInUS, 0);
    hdrProc.setParam(
            hdrHandle,
            HDRProcParam_Set_ShutterLineTime, rExpSetting.u4ShutterLineTime, 0);
    hdrProc.setParam(
            hdrHandle,
            HDRProcParam_Set_MaxAESensorGain, rExpSetting.u4MaxAESensorGain, 0);
    hdrProc.setParam(
            hdrHandle,
            HDRProcParam_Set_MinAESensorGain, rExpSetting.u4MinAESensorGain, 0);
    hdrProc.setParam(
            hdrHandle,
            HDRProcParam_Set_ExpTimeInUS0EV, rExpSetting.u4ExpTimeInUS0EV, 0);
    hdrProc.setParam(
            hdrHandle,
            HDRProcParam_Set_SensorGain0EV, rExpSetting.u4SensorGain0EV, 0);
    hdrProc.setParam(
            hdrHandle,
            HDRProcParam_Set_FlareOffset0EV, rExpSetting.u1FlareOffset0EV, 0);
    hdrProc.setParam(
            hdrHandle,
            HDRProcParam_Set_GainBase0EV, rExpSetting.i4GainBase0EV, 0);
    hdrProc.setParam(
            hdrHandle,
            HDRProcParam_Set_LE_LowAvg, rExpSetting.i4LE_LowAvg, 0);
    hdrProc.setParam(
            hdrHandle,
            HDRProcParam_Set_SEDeltaEVx100, rExpSetting.i4SEDeltaEVx100, 0);
    hdrProc.setParam(
            hdrHandle,
            HDRProcParam_Set_DetectFace, rExpSetting.bDetectFace, 0);
    hdrProc.setParam(
            hdrHandle,
            HDRProcParam_Set_Histogram,
            reinterpret_cast<MUINTPTR>(rExpSetting.u4Histogram), 0);
    hdrProc.setParam(
            hdrHandle,
            HDRProcParam_Set_FlareHistogram,
            reinterpret_cast<MUINTPTR>(rExpSetting.u4FlareHistogram), 0);
    {
        // get AEPlineTable
        MINT32 aeTableCurrentIndex;
        strAETable aePlineTable;
        hal3a->send3ACtrl(
                E3ACtrl_GetAECapPLineTable,
                reinterpret_cast<MINTPTR>(&aeTableCurrentIndex),
                reinterpret_cast<MINTPTR>(&aePlineTable));

        hdrProc.setParam(
                hdrHandle,
                HDRProcParam_Set_PLineAETable,
                reinterpret_cast<MUINTPTR>(&aePlineTable), aeTableCurrentIndex);

        MY_LOGD_IF( m_enableLog, "tableCurrentIndex(%d) eID(%d) u4TotalIndex(%d)",
            aeTableCurrentIndex, aePlineTable.eID, aePlineTable.u4TotalIndex);
    }

    // get HDR capture information from HDR proc
    MINT32& inputFrameCount(hdrHandle.inputFrameCount);
    std::vector<HDRCaptureParam> vCapParam;

    hdrProc.getHDRCapInfo(hdrHandle, vCapParam);

    inputFrameCount = vCapParam.size();

    // query the current 3A information
    HDRCaptureParam tmpCap3AParam;
    hal3a->send3ACtrl(
            E3ACtrl_GetExposureParam,
            reinterpret_cast<MINTPTR>(&tmpCap3AParam.exposureParam), 0);

    MUINT32 delayedFrames = 0;
    if ( bZsdFlow && inputFrameCount>1 ) {
        hal3a->send3ACtrl(
                E3ACtrl_GetCaptureDelayFrame,
                reinterpret_cast<MINTPTR>(&delayedFrames), 0);
    }


    MY_LOGD("HDR input frames(%d) delayed frames(%u)",
        inputFrameCount, delayedFrames);

    dumpCaptureParam(tmpCap3AParam, "Original ExposureParam");

    // update 3A information with information from HDR proc
    m_CurrentCapParams.resize(inputFrameCount);
    for (MINT32 i = 0; i < inputFrameCount; i++)
    {
        HDRCaptureParam& captureParam(m_CurrentCapParams.at(i));

        // copy original capture parameter
        captureParam = tmpCap3AParam;

        // adjust exposure settings
        CaptureParam_T& modifiedCapExpParam(captureParam.exposureParam);
        const CaptureParam_T& capExpParam(vCapParam.at(i).exposureParam);
        modifiedCapExpParam.u4Eposuretime  = capExpParam.u4Eposuretime;
        modifiedCapExpParam.u4AfeGain      = capExpParam.u4AfeGain;
        modifiedCapExpParam.u4IspGain      = capExpParam.u4IspGain;
        modifiedCapExpParam.u4FlareOffset  = capExpParam.u4FlareOffset;

        // update ISP conditions
        captureParam.ltmMode = vCapParam.at(i).ltmMode;

        String8 str;
        str.appendFormat("Modified ExposureParam[%d]", i);
        dumpCaptureParam(captureParam, str.string());

        m_CurrentCapParams.push_back(captureParam);
    }
    for (MUINT32 i = 0; i < delayedFrames; i++)
        m_DelayCapParams.push_back(tmpCap3AParam);

    // raise exposure flag
    hdrHandle.isExposureConfigured = MTRUE;

    setCaptureNum(inputFrameCount);
    setDelayFrameNum(delayedFrames);

    return true;
}

void HDRCtrler::generateFileInfo(std::shared_ptr<RequestFrame> pFrame)
{
    if(CC_LIKELY(pFrame.get())) {
        IHDRProc2& hdrProc(HDRProc2Factory::getIHDRProc2());

        /* retrieve metadata */
        IMetadata* pAppMeta         = pFrame->getMetadata(RequestFrame::eRequestMeta_InAppRequest);
        IMetadata* pHalMeta         = pFrame->getMetadata(RequestFrame::eRequestMeta_InHalP1);

        extract(&m_dumpNamingHint, pHalMeta);
        m_dumpNamingHint.SensorDev = m_openId;
        m_dumpNamingHint.IspProfile = 2; //EIspProfile_Capture;
        genFileName_TUNING(m_filename, sizeof(m_filename), &m_dumpNamingHint);

        hdrProc.setParam(m_HDRHandle, HDRProcParam_Set_Debug_Name_Info, (MUINTPTR)&m_dumpNamingHint, 0);

    }
    else {
        MY_LOGE("generateFileInfo error because of pFrame null");
    }
}
void HDRCtrler::dumpCaptureParam(
        const HDRCaptureParam& captureParam, const char* msg)
{
    if(!m_enableLog) return;
    if (msg) MY_LOGD("========= %s =========", msg);

    const CaptureParam_T& capExpParam(captureParam.exposureParam);

    MY_LOGD("u4ExposureMode  (%u)", capExpParam.u4ExposureMode);
    MY_LOGD("u4Eposuretime   (%u)", capExpParam.u4Eposuretime);
    MY_LOGD("u4AfeGain       (%u)", capExpParam.u4AfeGain);
    MY_LOGD("u4IspGain       (%u)", capExpParam.u4IspGain);
    MY_LOGD("u4RealISO       (%u)", capExpParam.u4RealISO);
    MY_LOGD("u4FlareGain     (%u)", capExpParam.u4FlareGain);
    MY_LOGD("u4FlareOffset   (%u)", capExpParam.u4FlareOffset);
    MY_LOGD("i4LightValue_x10(%d)", capExpParam.i4LightValue_x10);
    MY_LOGD("bypassLTM       (%d)", captureParam.bypassLTM());

}

bool HDRCtrler::processSingleFrame(std::shared_ptr<RequestFrame> pFrame)
{
    /* retrieve metadata */
    IMetadata* pAppMeta         = pFrame->getMetadata(RequestFrame::eRequestMeta_InAppRequest);
    IMetadata* pHalMeta         = pFrame->getMetadata(RequestFrame::eRequestMeta_InHalP1);
    /* retrieve image buffer that we care */
    IImageBuffer* pImageBuffer  = pFrame->getImageBuffer(RequestFrame::eRequestImg_FullSrc);
    IImageBuffer* pImageLcsoRaw = pFrame->getImageBuffer(RequestFrame::eRequestImg_LcsoRaw);

    // check these metadata and image buffer
    //m_mainRequestFrame
    MUINT8 isUseSingleFrame = 0;
    IMetadata::getEntry<MUINT8>(pHalMeta,
                MTK_HAL_REQUEST_ERROR_FRAME, isUseSingleFrame);

    if(!isUseSingleFrame) {
        return false;
    }

    //
    auto pFrameYuvJpeg = m_mainRequestFrame->getImageBuffer(RequestFrame::eRequestImg_FullOut);
    auto pFrameYuvThub = m_mainRequestFrame->getImageBuffer(RequestFrame::eRequestImg_ThumbnailOut);
    auto pFrameWorking = m_mainRequestFrame->getImageBuffer(RequestFrame::eRequestImg_WorkingBufferOut);
    auto outOrientaion = m_mainRequestFrame->getOrientation(RequestFrame::eRequestImg_FullOut);
    auto requestNo     = m_mainRequestFrame->getRequestNo();

    //Raw2YUV
    Pass2Encoder p2Enc(m_openId);
    Pass2Encoder::ConfigRaw2Yuv config;
    sp<Ctrl> yuvCtrl = new Ctrl();
    auto workingYuv = yuvCtrl->get(m_mainYuvPool);

    // if using RAW, we have to give the bayer order info
    {
        // Get sensor format
        IHalSensorList *const pIHalSensorList = MAKE_HalSensorList(); // singleton, no need to release
        if (pIHalSensorList) {
            MUINT32 sensorDev = (MUINT32) pIHalSensorList->querySensorDevIdx(m_openId);
            NSCam::SensorStaticInfo sensorStaticInfo;
            memset(&sensorStaticInfo, 0, sizeof(NSCam::SensorStaticInfo));
            pIHalSensorList->querySensorStaticInfo(sensorDev, &sensorStaticInfo);
            pImageBuffer->setColorArrangement(static_cast<MINT32>(sensorStaticInfo.sensorFormatOrder));
        }
    }

    // input
    config.pSrcRaw     = pImageBuffer;
    config.pAppMeta    = pAppMeta;
    config.pHalMeta    = pHalMeta;

    // output
    config.pOutput     = workingYuv;
    config.pOutAppMeta = pAppMeta;
    config.pOutHalMeta = pHalMeta;

    MY_LOGD("EncodeRaw2Yuv+");
    if ( p2Enc.encodeRaw2Yuv(&config) != OK ) {
        MY_LOGE("p2 encode fail.");
    }
    MY_LOGD("EncodeRaw2Yuv-");

    //Final buffer cropping
    // handle yuv output
    bool ret = true;
    IImageBuffer* dst1 = (pFrameYuvJpeg == nullptr) ? pFrameWorking : pFrameYuvJpeg;
    IImageBuffer* dst2 = pFrameYuvThub;
    std::string str = (pFrameYuvJpeg == nullptr) ? "Process output working buffer."
                                                 : "Process output capture buffer.";
    bool needCrop = (pFrameYuvJpeg == nullptr) ? false : true;
    bool needClearZoom = (pFrameYuvJpeg == nullptr) ? false : true;
    ret  = handleYuvOutput(str,
                workingYuv, dst1, dst2,
                pAppMeta, pHalMeta,
                getOpenId(), outOrientaion, requestNo, needClearZoom, needCrop);


    return true;
}


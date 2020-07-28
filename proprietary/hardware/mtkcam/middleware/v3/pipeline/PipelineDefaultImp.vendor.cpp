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

#define LOG_TAG "MtkCam/HwPipeline.vendor"
//
#include "PipelineDefaultImp.h"
//
#include <mtkcam/pipeline/extension/MFNR.h>
//
#include <mtkcam/utils/metadata/hal/mtk_platform_metadata_tag.h>
//
#include <mtkcam/feature/hdrDetection/Defs.h>
//
#include <mtkcam/aaa/IHal3A.h>
// STL
#include <type_traits> //static_assert
// CUSTOM
#include <custom/feature/mfnr/camera_custom_mfll.h>
using namespace NS3Av3;
//
#define  SUPPORT_IVENDOR                    (1)

using namespace android;
using namespace NSCam;
using namespace NSCam::v3;
using namespace NSCam::Utils;
using namespace NSCam::v3::Utils;
using namespace NSCam::v3::NSPipelineContext;

/******************************************************************************
 *
 ******************************************************************************/
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

#if 1
#define FUNC_START     MY_LOGD("+")
#define FUNC_END       MY_LOGD("-")
#else
#define FUNC_START
#define FUNC_END
#endif

#define CHECK_ERROR(_err_)                                \
    do {                                                  \
        MERROR const err = (_err_);                       \
        if( err != OK ) {                                 \
            MY_LOGE("err:%d(%s)", err, ::strerror(-err)); \
            return err;                                   \
        }                                                 \
    } while(0)

/******************************************************************************
 *
 ******************************************************************************/
MBOOL
PipelineDefaultImp::
refinePluginRequestMetaStreamBuffersLocked(
    evaluateRequestResult&      result,
    evaluateSubRequestResult&   subResult
)
{
#if MTKCAM_HAVE_IVENDOR_SUPPORT
    CAM_TRACE_NAME(__FUNCTION__);
    //
    plugin::InputInfo input;
    CHECK_ERROR( updateCombination(input, result) );
    //
    if ( input.combination.isEmpty() ) {
        MY_LOGD_IF( mLogLevel>=2, "-");
        return MFALSE;
    }
    //
    {
        // set plugin info
        input.fullRaw      = mpHalImage_P1_Raw;
        input.resizedRaw   = mpHalImage_P1_ResizerRaw;
        input.jpegYuv      = mpHalImage_Jpeg_YUV;
        input.thumbnailYuv = mpHalImage_Thumbnail_YUV;
        input.jpeg         = mpAppImage_Jpeg;
        for ( size_t i = 0; i < mvAppYuvImage.size(); ++i ) {
            // app yuv image
            input.vYuv.push_back(mvAppYuvImage.editValueAt(i));
        }
        if (mpHalImage_FD_YUV.get()) input.vYuv.push_back(mpHalImage_FD_YUV);
    }
    //
    // plugin result
    {
        sp<IMetaStreamBuffer> pAppStreamBuffer = result.vAppMetaBuffers.valueFor(mpAppMeta_Control->getStreamId());
        sp<IMetaStreamBuffer> pHalStreamBuffer = result.vHalMetaBuffers.valueFor(mpHalMeta_Control->getStreamId());
        IMetadata* pAppMetadata = pAppStreamBuffer->tryWriteLock(LOG_TAG);
        IMetadata* pHalMetadata = pHalStreamBuffer->tryWriteLock(LOG_TAG);
        //
        if ( pAppMetadata == nullptr || pHalMetadata == nullptr ) {
            MY_LOGE("lock meta fail. app:%p hal:%p", pAppMetadata, pHalMetadata);
            return MFALSE;
        }
        //
        plugin::OutputInfo out;
        CHECK_ERROR( mpVendorMgr->get(mUserId, input, out) );
        //
        subResult.subRequetNumber = out.frameCount - 1;
        // update p2 control
        {
            MINT64 p2_setting = MTK_P2_ISP_PROCESSOR|MTK_P2_MDP_PROCESSOR|MTK_P2_CAPTURE_REQUEST;
            if (out.inCategory == plugin::FORMAT_RAW && out.outCategory == plugin::FORMAT_YUV) {
                p2_setting = MTK_P2_RAW_PROCESSOR;
                if ( mpHalImage_P1_Raw.get() ) p2_setting |= MTK_P2_CAPTURE_REQUEST;
                if ( mpHalImage_P1_ResizerRaw.get() ) p2_setting |= MTK_P2_PREVIEW_REQUEST;
            }
            else if (out.inCategory == plugin::FORMAT_YUV && out.outCategory == plugin::FORMAT_YUV)
                p2_setting = MTK_P2_ISP_PROCESSOR|MTK_P2_YUV_PROCESSOR|MTK_P2_MDP_PROCESSOR|MTK_P2_CAPTURE_REQUEST;
            else if (out.inCategory == plugin::FORMAT_RAW && out.outCategory == plugin::FORMAT_RAW)
                p2_setting = MTK_P2_RAW_PROCESSOR|MTK_P2_ISP_PROCESSOR|MTK_P2_MDP_PROCESSOR|MTK_P2_CAPTURE_REQUEST;;

            IMetadata::IEntry entry(MTK_PLUGIN_P2_COMBINATION);
            entry.push_back(p2_setting, Type2Type< MINT64 >());
            pHalMetadata->update(entry.tag(), entry);
        }
        //
        // sub frame
        for ( int i = 1; i < out.frameCount; ++i ) {
            // control meta
            IMetadata appMeta = *pAppMetadata;
            IMetadata halMeta = *pHalMetadata;
            //
            appMeta += out.settings.editItemAt(i).setting.appMeta;
            halMeta += out.settings.editItemAt(i).setting.halMeta;
            //
            evaluateRequestResult sub;
            //
            sub.vAppMetaBuffers.setCapacity(1);
            sp<HalMetaStreamBuffer> pAppBuffer =
                HalMetaStreamBufferAllocatorT(mpAppMeta_Control.get())(appMeta);
            sub.vAppMetaBuffers.add(mpAppMeta_Control->getStreamId(), pAppBuffer);
            //
            sub.vHalMetaBuffers.setCapacity(1);
            sp<HalMetaStreamBuffer> pHalBuffer =
                HalMetaStreamBufferAllocatorT(mpHalMeta_Control.get())(halMeta);
            sub.vHalMetaBuffers.add(mpHalMeta_Control->getStreamId(), pHalBuffer);

            // iomap
            {
                IOMap p1_Image_IOMap;
                IOMapSet p2_Image_IOMap_Set;
                if ( mpHalImage_P1_Raw.get() ) {
                    p1_Image_IOMap.addOut(mpHalImage_P1_Raw->getStreamId());
                    p2_Image_IOMap_Set.add(
                        IOMap().addIn(mpHalImage_P1_Raw->getStreamId())
                    );

                    //
                    sub.vHalImageBuffers.add(
                            mpHalImage_P1_Raw->getStreamId(),
                            result.vHalImageBuffers.valueFor(mpHalImage_P1_Raw->getStreamId())
                        );
                }
                if ( mpHalImage_P1_ResizerRaw.get() ) {
                    p1_Image_IOMap.addOut(mpHalImage_P1_ResizerRaw->getStreamId());
                    p2_Image_IOMap_Set.add(
                        IOMap().addIn(mpHalImage_P1_ResizerRaw->getStreamId())
                    );
                    //
                    sub.vHalImageBuffers.add(
                            mpHalImage_P1_ResizerRaw->getStreamId(),
                            result.vHalImageBuffers.valueFor(mpHalImage_P1_ResizerRaw->getStreamId())
                        );
                }
                //
                sub.nodeIOMapImage.add(eNODEID_P1Node, IOMapSet().add( p1_Image_IOMap ));
                sub.nodeIOMapImage.add( eNODEID_P2Node, p2_Image_IOMap_Set );
                sub.nodeIOMapMeta.add(
                    eNODEID_P1Node,
                    IOMapSet().add(IOMap()
                        .addIn(mpAppMeta_Control->getStreamId())
                        .addIn(mpHalMeta_Control->getStreamId())
                        .addOut(mpHalMeta_DynamicP1->getStreamId())
                        .addOut(mpAppMeta_DynamicP1->getStreamId())
                    )
                );
                sub.nodeIOMapMeta.add(
                    eNODEID_P2Node,
                    IOMapSet().add(IOMap()
                        .addIn(mpAppMeta_Control->getStreamId())
                        .addIn(mpHalMeta_DynamicP1->getStreamId())
                        .addIn(mpAppMeta_DynamicP1->getStreamId())
                        .addOut(mpHalMeta_DynamicP2->getStreamId())
                        .addOut(mpAppMeta_DynamicP2->getStreamId())
                    )
                );
            }
            //
            subResult.subRequestList.push_back(sub);
        }
        // main frame
        if ( !out.settings.isEmpty() ) {
            *pAppMetadata += out.settings.editItemAt(0).setting.appMeta;
            *pHalMetadata += out.settings.editItemAt(0).setting.halMeta;
        }
        //
        pAppStreamBuffer->unlock(LOG_TAG, pAppMetadata);
        pHalStreamBuffer->unlock(LOG_TAG, pHalMetadata);
    }
#endif
    return MTRUE;
}


/******************************************************************************
 *
 ******************************************************************************/
MERROR
PipelineDefaultImp::
updateCombination(
    plugin::InputInfo&     input,
    evaluateRequestResult& result
)
{
#if MTKCAM_HAVE_IVENDOR_SUPPORT
    {
        // parse plugin setting
        sp<IMetaStreamBuffer> pAppStreamBuffer = result.vAppMetaBuffers.valueFor(mpAppMeta_Control->getStreamId());
        sp<IMetaStreamBuffer> pHalStreamBuffer = result.vHalMetaBuffers.valueFor(mpHalMeta_Control->getStreamId());
        IMetadata* pAppMetadata = pAppStreamBuffer->tryReadLock(LOG_TAG);
        IMetadata* pHalMetadata = pHalStreamBuffer->tryReadLock(LOG_TAG);
        if ( pAppMetadata == nullptr || pHalMetadata == nullptr )
        {
            MY_LOGE("lock meta fail. app:%p hal:%p", pAppMetadata, pHalMetadata);
            return NAME_NOT_FOUND;
        }
        //
        input.appCtrl = *pAppMetadata;
        input.halCtrl = *pHalMetadata;
        //
        IMetadata::IEntry const eCaptureIntent = pAppMetadata->entryFor(MTK_CONTROL_CAPTURE_INTENT);
        IMetadata::IEntry const eMfb           = pAppMetadata->entryFor(MTK_MFNR_FEATURE_MFB_MODE);
        IMetadata::IEntry const eAis           = pAppMetadata->entryFor(MTK_MFNR_FEATURE_AIS_MODE);
        IMetadata::IEntry const eSceneMode     = pAppMetadata->entryFor(MTK_CONTROL_SCENE_MODE);
        IMetadata::IEntry const eHDRMode       = pAppMetadata->entryFor(MTK_HDR_FEATURE_HDR_MODE);

        MBOOL isHDRScene = (! eSceneMode.isEmpty()) &&
                   (eSceneMode.itemAt(0, Type2Type<MUINT8>()) == MTK_CONTROL_SCENE_MODE_HDR);
        HDRMode hdrMode = eHDRMode.isEmpty() ? HDRMode::OFF : static_cast<HDRMode>(eHDRMode.itemAt(0, Type2Type<MINT32>()));
        MBOOL runVHDR = (hdrMode == HDRMode::VIDEO_ON) || (hdrMode == HDRMode::VIDEO_AUTO);
        MBOOL allowSWHDR = (hdrMode == HDRMode::OFF && isHDRScene)
                            || (hdrMode == HDRMode::ON)
                            || (hdrMode == HDRMode::AUTO && 1/*TODO check P1 app result meta*/);
        //
        // capture feature
        if ( ! eCaptureIntent.isEmpty() &&
               eCaptureIntent.itemAt(0, Type2Type<MUINT8>()) == MTK_CONTROL_CAPTURE_INTENT_STILL_CAPTURE )
        {
            #define HANDLE_EXTRA_YUV(_MODE_) {                                          \
                plugin::VendorInfo info;                                                \
                info.vendorMode = _MODE_;                                               \
                for ( size_t i = 0; i < mvAppYuvImage.size(); ++i )                     \
                    info.appOut.push_back(mvAppYuvImage.editValueAt(i)->getStreamId()); \
                if (mpHalImage_FD_YUV.get())                                            \
                    info.appOut.push_back(mpHalImage_FD_YUV->getStreamId());            \
                result.vVendorInfo.add(_MODE_, info);                                   \
            }
#if MTKCAM_HAVE_HDR_SUPPORT
            // HDR
            if ( isHDRScene && allowSWHDR )
            {
                input.combination.push_back(MTK_PLUGIN_MODE_HDR);
                input.combination.push_back(MTK_PLUGIN_MODE_NR);
                input.combination.push_back(MTK_PLUGIN_MODE_COPY);
                //
                HANDLE_EXTRA_YUV(MTK_PLUGIN_MODE_HDR);
                //
                goto lbEndParse;
            }
#endif
#if MTKCAM_HAVE_MFB_SUPPORT // link MFNR extension if MTKCAM_HAVE_MFB_SUPPORT > 0
            do {
                // If enable DNG, do not use MFNRVendor
                if (mParams.mbHasRaw) {
                    MY_LOGD("Disable MFNR due to enable DNG");
                    break;
                }
                // Check flash LED status
                int isFlashOn = 0;
                IHal3A* pHal3a = MAKE_Hal3A(getOpenId(), LOG_TAG);
                if (pHal3a)
                {
                    pHal3a->send3ACtrl(NS3Av3::E3ACtrl_GetIsFlashOnCapture, (MINTPTR)&isFlashOn, 0);
                    pHal3a->destroyInstance(LOG_TAG);
                    MY_LOGD("isFlashOn:%d", isFlashOn);
                }
                if (isFlashOn) { // If enable flash LED on, do not use MFNRVendor
                    MY_LOGD("Disable MFNR due to enable Flash LED");
                    break;
                }

                // MFNR HAL3 only supports IVendor mechanism
                int mfbMode = [&]()
                {
                    // if the feature has combined with SWHDR, set MFB off (0)
                    if ( isHDRScene && allowSWHDR ) {
                        return MTK_MFB_MODE_OFF;
                    }
                    // If MTK specific parameter AIS on or MFB mode is AIS, set to AIS mode (2)
                    else if (( ! eMfb.isEmpty() && eMfb.itemAt(0, Type2Type<MINT32>()) == MTK_MFNR_FEATURE_MFB_AIS)  ||
                             ( ! eAis.isEmpty() && eAis.itemAt(0, Type2Type<MINT32>()) == MTK_MFNR_FEATURE_AIS_ON)) {
                        return MTK_MFB_MODE_AIS;
                    }
                    // Scene mode is Night or MFB mode is MFLL, set to MFLL mode (1)
                    else if (( ! eMfb.isEmpty() && eMfb.itemAt(0, Type2Type<MINT32>()) == MTK_MFNR_FEATURE_MFB_MFLL) ||
                             ( ! eSceneMode.isEmpty() && eSceneMode.itemAt(0, Type2Type<MUINT8>()) == MTK_CONTROL_SCENE_MODE_NIGHT)) {
                        return MTK_MFB_MODE_MFLL;
                    }
                    else if (( ! eMfb.isEmpty() && eMfb.itemAt(0, Type2Type<MINT32>()) == MTK_MFNR_FEATURE_MFB_AUTO)) {
#ifdef CUST_MFLL_AUTO_MODE
                        static_assert( ((CUST_MFLL_AUTO_MODE >= MTK_MFB_MODE_OFF)&&(CUST_MFLL_AUTO_MODE < MTK_MFB_MODE_NUM)),
                                       "CUST_MFLL_AUTO_MODE is invalid in custom/feature/mfnr/camera_custom_mfll.h" );

                        MY_LOGD("CUST_MFLL_AUTO_MODE:%d", CUST_MFLL_AUTO_MODE);
                        return static_cast<mtk_platform_metadata_enum_mfb_mode>(CUST_MFLL_AUTO_MODE);
#else
#error "CUST_MFLL_AUTO_MODE is no defined in custom/feature/mfnr/camera_custom_mfll.h"
#endif
                    }
                    // Otherwise, set MFB off (0)
                    else {
                        return MTK_MFB_MODE_OFF;
                    }
                }();

#if MTKCAM_HAVE_MFB_BUILDIN_SUPPORT
                if (mfbMode == MTK_MFB_MODE_OFF) {
                    mfbMode = MTKCAM_HAVE_MFB_BUILDIN_SUPPORT;
                    MY_LOGD("APK set mfbMode to 0, but MTKCAM_HAVE_MFB_BUILDIN_SUPPORT " \
                            "has been set to %d.", MTKCAM_HAVE_MFB_BUILDIN_SUPPORT);
                }
#endif

                // query feature info
                plugin::MFNRVendor::ConfigParams params;
                params.isZSDMode = MTRUE;
                plugin::MFNRVendor::FeatureInfo info =
                    plugin::MFNRVendor::queryFeatureInfo(
                            getOpenId()/*sensorId*/,
                            mfbMode /*mfbMode*/,
                            params /*rParams*/
                            );

                // if no need MFNR, break is scope.
                if (info.doMfb == 0)
                    break;

                // combine iVendors
                input.combination.push_back(MTK_PLUGIN_MODE_MFNR);
                input.combination.push_back(MTK_PLUGIN_MODE_NR);
                input.combination.push_back(MTK_PLUGIN_MODE_COPY);
                //
                HANDLE_EXTRA_YUV(MTK_PLUGIN_MODE_MFNR);
                //
                goto lbEndParse;
            } while(0);
#else
            MY_LOGI("MTKCAM_HAVE_MFB_SUPPORT has been disabled");
#endif // MTKCAM_HAVE_MFB_SUPPORT

            #undef HANDLE_EXTRA_YUV

            // NR
            {
                input.combination.push_back(MTK_PLUGIN_MODE_NR);
                goto lbEndParse;
            }
        }
        //
        //
lbEndParse:
        pAppStreamBuffer->unlock(LOG_TAG, pAppMetadata);
        pHalStreamBuffer->unlock(LOG_TAG, pHalMetadata);
    }
#endif
    return OK;
}

/******************************************************************************
 *
 ******************************************************************************/
MERROR
PipelineDefaultImp::
setPluginResult(
    MUINT32 startFrameNo,
    evaluateRequestResult&    result,
    evaluateSubRequestResult& subResult
)
{
#if MTKCAM_HAVE_IVENDOR_SUPPORT
    CAM_TRACE_NAME(__FUNCTION__);
    //
    plugin::InputSetting in;

#define setFrameInfo( _frameNo_, _data_ )                                             \
    do {                                                                              \
        sp<IMetaStreamBuffer> pAppStreamBuffer =                                      \
            _data_.vAppMetaBuffers.valueFor(mpAppMeta_Control->getStreamId());        \
        sp<IMetaStreamBuffer> pHalStreamBuffer =                                      \
            _data_.vHalMetaBuffers.valueFor(mpHalMeta_Control->getStreamId());        \
        IMetadata* pAppMetadata = pAppStreamBuffer->tryReadLock(LOG_TAG);             \
        IMetadata* pHalMetadata = pHalStreamBuffer->tryReadLock(LOG_TAG);             \
                                                                                      \
        plugin::FrameInfo info;                                                       \
        info.frameNo = _frameNo_;                                                     \
        info.curAppControl = *pAppMetadata;                                           \
        info.curHalControl = *pHalMetadata;                                           \
        info.vVendorInfo   = _data_.vVendorInfo;                                      \
                                                                                      \
        in.vFrame.push_back(info);                                                    \
                                                                                      \
        pAppStreamBuffer->unlock(LOG_TAG, pAppMetadata);                              \
        pHalStreamBuffer->unlock(LOG_TAG, pHalMetadata);                              \
    } while(0)
    //
    // main frame
    setFrameInfo(startFrameNo, result);
    // sub frame
    for( size_t i = 1; i <= subResult.subRequetNumber; ++i ) {
        setFrameInfo(startFrameNo+i, subResult.subRequestList.editItemAt(i-1));
    }
#undef setFrameInfo
    //
    return mpVendorMgr->set(mUserId, in);
#endif
    return OK;
}


/******************************************************************************
 *
 ******************************************************************************/
sp<IPipelineFrame>
PipelineDefaultImp::
buildSubPipelineFrameLocked(
    MUINT32                requestNo,
    evaluateRequestResult& evaluateResult
)
{
    CAM_TRACE_NAME(__FUNCTION__);
    //
    RequestBuilder builder;
    // root
    builder.setRootNode(
        NodeSet().add(eNODEID_P1Node)
    );
    //
    // not support multiple frame vss
    builder.setNodeEdges(
        NodeEdgeSet().addEdge(eNODEID_P1Node, eNODEID_P2Node)
    );
    //
#define try_setIOMap(_nodeId_)                                                        \
    do {                                                                              \
        ssize_t idx_image = evaluateResult.nodeIOMapImage.indexOfKey(_nodeId_);       \
        ssize_t idx_meta  = evaluateResult.nodeIOMapMeta.indexOfKey(_nodeId_);        \
        builder.setIOMap(                                                             \
                _nodeId_,                                                             \
                (0 <= idx_image ) ?                                                   \
                evaluateResult.nodeIOMapImage.valueAt(idx_image) : IOMapSet::empty(), \
                (0 <= idx_meta ) ?                                                    \
                evaluateResult.nodeIOMapMeta.valueAt(idx_meta) : IOMapSet::empty()    \
                );                                                                    \
    } while(0)
    //
    try_setIOMap(eNODEID_P1Node);
    try_setIOMap(eNODEID_P2Node);
    //
#undef try_setIOMap
    //
#define setStreamBuffers(_sb_type_, _type_, _vStreamBuffer_, _builder_)    \
    do {                                                                   \
        for (size_t i = 0; i < _vStreamBuffer_.size(); i++ )               \
        {                                                                  \
            StreamId_T streamId                = _vStreamBuffer_.keyAt(i); \
            sp<_sb_type_> buffer = _vStreamBuffer_.valueAt(i);             \
            _builder_.set##_type_##StreamBuffer(streamId, buffer);         \
        }                                                                  \
    } while(0)
    //
    setStreamBuffers(HalImageStreamBuffer, Image, evaluateResult.vHalImageBuffers, builder);
    setStreamBuffers(IMetaStreamBuffer   , Meta , evaluateResult.vAppMetaBuffers , builder);
    setStreamBuffers(HalMetaStreamBuffer , Meta , evaluateResult.vHalMetaBuffers , builder);
#undef setStreamBuffers
    //
    sp<IPipelineFrame> pFrame = builder
        .updateFrameCallback(nullptr)
        .build(requestNo, mpPipelineContext);
    //
    return pFrame;
}

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

#define LOG_TAG "MtkCam/FusionShot"

#include <mtkcam/utils/std/Log.h>
#include <mtkcam/def/common.h>
#include <mtkcam/utils/std/Trace.h>

#include <mtkcam/middleware/v1/IShot.h>

#include "ImpShot.h"
#include "FusionShot.h"

#include <mtkcam/middleware/v1/LegacyPipeline/StreamId.h>
#include <mtkcam/utils/metadata/hal/mtk_platform_metadata_tag.h>

#include <mtkcam/utils/hw/HwInfoHelper.h>

#include <mtkcam/middleware/v1/LegacyPipeline/LegacyPipelineUtils.h>
#include <mtkcam/middleware/v1/LegacyPipeline/LegacyPipelineBuilder.h>
#include <mtkcam/middleware/v1/LegacyPipeline/buffer/StreamBufferProviderFactory.h>

#include <mtkcam/feature/stereo/hal/stereo_setting_provider.h>
#include <camera_custom_stereo.h>
#define BM_DENOISE_PORTER_KEY "bmdenoise.porter.dump"

using namespace NSShot::NSFusionShot;
using namespace NSCam;
using namespace NSCam::v1;
using namespace NSCam::v1::NSLegacyPipeline;
using namespace NSCamHW;

#define CHECK_OBJECT(x)  do{                                        \
    if (x == nullptr) { MY_LOGE("Null %s Object", #x); return MFALSE;} \
} while(0)

/******************************************************************************
 *
 ******************************************************************************/
#define MY_LOGV(fmt, arg...)        CAM_LOGV("(%d)(%d)(%s)[%s] " fmt, ::gettid(), getOpenId(), getShotName(), __FUNCTION__, ##arg)
#define MY_LOGD(fmt, arg...)        CAM_LOGD("(%d)(%d)(%s)[%s] " fmt, ::gettid(), getOpenId(), getShotName(), __FUNCTION__, ##arg)
#define MY_LOGI(fmt, arg...)        CAM_LOGI("(%d)(%d)(%s)[%s] " fmt, ::gettid(), getOpenId(), getShotName(), __FUNCTION__, ##arg)
#define MY_LOGW(fmt, arg...)        CAM_LOGW("(%d)(%d)(%s)[%s] " fmt, ::gettid(), getOpenId(), getShotName(), __FUNCTION__, ##arg)
#define MY_LOGE(fmt, arg...)        CAM_LOGE("(%d)(%d)(%s)[%s] " fmt, ::gettid(), getOpenId(), getShotName(), __FUNCTION__, ##arg)
#define MY_LOGA(fmt, arg...)        CAM_LOGA("(%d)(%d)(%s)[%s] " fmt, ::gettid(), getOpenId(), getShotName(), __FUNCTION__, ##arg)
#define MY_LOGF(fmt, arg...)        CAM_LOGF("(%d)(%d)(%s)[%s] " fmt, ::gettid(), getOpenId(), getShotName(), __FUNCTION__, ##arg)

#define MYU_LOGE(fmt, arg...)       CAM_LOGE("(%d)[%s] " fmt, ::gettid(), __FUNCTION__, ##arg)

//
#define MY_LOGV_IF(cond, ...)       do { if ( (cond) ) { MY_LOGV(__VA_ARGS__); } }while(0)
#define MY_LOGD_IF(cond, ...)       do { if ( (cond) ) { MY_LOGD(__VA_ARGS__); } }while(0)
#define MY_LOGI_IF(cond, ...)       do { if ( (cond) ) { MY_LOGI(__VA_ARGS__); } }while(0)
#define MY_LOGW_IF(cond, ...)       do { if ( (cond) ) { MY_LOGW(__VA_ARGS__); } }while(0)
#define MY_LOGE_IF(cond, ...)       do { if ( (cond) ) { MY_LOGE(__VA_ARGS__); } }while(0)
#define MY_LOGA_IF(cond, ...)       do { if ( (cond) ) { MY_LOGA(__VA_ARGS__); } }while(0)
#define MY_LOGF_IF(cond, ...)       do { if ( (cond) ) { MY_LOGF(__VA_ARGS__); } }while(0)
#define FUNC_START                  MY_LOGD("+")
#define FUNC_END                    MY_LOGD("-")

#define DEFAULT_MAX_FRAME_NUM       6

/******************************************************************************
 *
 ******************************************************************************/
extern
sp<IShot>
createInstance_FusionShot(
    char const*const    pszShotName,
    uint32_t const      u4ShotMode,
    int32_t const       i4OpenId
)
{
    sp<IShot>  pShot            = NULL;
    sp<FusionShot>  pImpShot = NULL;
    //
    //  (1.1) new Implementator.
    pImpShot = new FusionShot(pszShotName, u4ShotMode, i4OpenId, true);
    if  ( pImpShot == 0 ) {
        CAM_LOGE("[%s] new FusionShot", __FUNCTION__);
        goto lbExit;
    }
    //
    //  (1.2) initialize Implementator if needed.
    if  ( ! pImpShot->onCreate() ) {
        CAM_LOGE("[%s] onCreate()", __FUNCTION__);
        goto lbExit;
    }
    //
    //  (2)   new Interface.
    pShot = new IShot(pImpShot);
    if  ( pShot == 0 ) {
        CAM_LOGE("[%s] new IShot", __FUNCTION__);
        goto lbExit;
    }
    //
lbExit:
    //
    //  Free all resources if this function fails.
    if  ( pShot == 0 && pImpShot != 0 ) {
        pImpShot->onDestroy();
        pImpShot = NULL;
    }
    //
    return  pShot;
}

/******************************************************************************
 *
 ******************************************************************************/
FusionShot::
FusionShot(
    char const*const pszShotName,
    uint32_t const   u4ShotMode,
    int32_t const    i4OpenId,
    bool const       isZsd
)
    : SmartShot(pszShotName, u4ShotMode, i4OpenId, isZsd)
{
    mFusionShotPorterFlag  = ::property_get_int32(BM_DENOISE_PORTER_KEY, 0);
    if( mFusionShotPorterFlag ) {
        MY_LOGD("enable mFusionShotPorterFlag flag %d", mFusionShotPorterFlag);
    }
}

/******************************************************************************
 *
 ******************************************************************************/
FusionShot::
~FusionShot()
{
    MY_LOGD("~FusionShot()");
    if( mResultMetadataSetMap.size() > 0 )
    {
        int n = mResultMetadataSetMap.size();
        for(int i=0; i<n; i++)
        {
            MY_LOGW("requestNo(%d) doesn't clear before FusionShot destroyed",mResultMetadataSetMap.keyAt(i));
            mResultMetadataSetMap.editValueAt(i).selectorGetBufs.clear();
        }
    }
    mResultMetadataSetMap.clear();
}

/******************************************************************************
 *
 ******************************************************************************/
bool
FusionShot::
onCmd_capture()
{
    CAM_TRACE_NAME("FusionShot onCmd_capture");
    mbIgnoreJpegStream = isNeedToIgnoreJpegStream();
    //
    mpManager = NSVendorManager::get(getOpenId());
    CHECK_OBJECT(mpManager);

    // applyRawBufferSettings and doCreateSterams need it
    mpConsumer = IResourceContainer::getInstance(getOpenId())->queryConsumer(eSTREAMID_IMAGE_PIPE_RAW_OPAQUE);
    mJpegsize  = (mShotParam.mu4Transform & eTransform_ROT_90) ? MSize(mShotParam.mi4PictureHeight, mShotParam.mi4PictureWidth):
                                                                 MSize(mShotParam.mi4PictureWidth, mShotParam.mi4PictureHeight);
    //
    if (!mpPipeline.get()) doCreateStreams();
    //
    MY_LOGD("UpdateVendor+");
    //
    // ImageRatio must be set before Vendor->get, featurepipe needs it
    // update StereoSetting for image ratio
    // get preview size
    int preview_width = mShotParam.mi4PictureWidth;
    int preview_height = mShotParam.mi4PictureHeight;
    MY_LOGD("width(%d) height(%d)", preview_width, preview_height);
    double ratio_4_3 = 4.0/3.0;
    double preview_ratio = ((double)preview_width) / ((double)preview_height);
    if(preview_ratio == ratio_4_3)
    {
        MY_LOGD("set to 4:3");
        StereoSettingProvider::setImageRatio(eRatio_4_3);
    }
    else
    {
        MY_LOGD("set to 16:9");
        StereoSettingProvider::setImageRatio(eRatio_16_9);
    }

    plugin::InputInfo inputInfo;
    MINT64 mode = [ this ](MINT32 const type) -> MINT64 {
        switch(type)
        {
            case eShotMode_FusionShot: {
                return MTK_PLUGIN_MODE_FUSION_3rdParty;
            }
        }
        MY_LOGW("no mapped vendor for type %d", type);
        return MTK_PLUGIN_MODE_COMBINATION;
    }(getShotMode());

    // set Input
    if(!makeVendorCombination(inputInfo, mode)){
        MY_LOGE("make vendor combination failed!");
        return false;
    }

    inputInfo.appCtrl      = mShotParam.mAppSetting;
    inputInfo.halCtrl      = mShotParam.mHalSetting;
    //
    inputInfo.fullRaw      = mpInfo_FullRaw;
    inputInfo.resizedRaw   = mpInfo_ResizedRaw;
    inputInfo.lcsoRaw      = mpInfo_LcsoRaw;
    inputInfo.jpegYuv      = mpInfo_Yuv;
    inputInfo.thumbnailYuv = mpInfo_YuvThumbnail;
    if(!mbIgnoreJpegStream){
        inputInfo.postview     = mpInfo_YuvPostview;
        inputInfo.jpeg         = mpInfo_Jpeg;
    }

    plugin::OutputInfo outputInfo;
    CAM_TRACE_FMT_BEGIN("FusionShot(%d):Vendor:Get", getOpenId());
    MY_LOGD("Vendor:Get");
    mpManager->get(plugin::CALLER_SHOT_SMART, inputInfo, outputInfo);
    CAM_TRACE_FMT_END();
    // User defined
    plugin::VendorInfo vInfo;
    vInfo.vendorMode = mode;
    //
    plugin::InputSetting inputSetting;
    inputSetting.pMsgCb = this;
    for ( int i = 0; i < outputInfo.frameCount; ++i ) {
        plugin::FrameInfo info;
        info.frameNo = mCapReqNo + i;
        info.curAppControl = mShotParam.mAppSetting;
        info.curHalControl = mShotParam.mHalSetting;
        info.curAppControl += outputInfo.settings[i].setting.appMeta;
        info.curHalControl += outputInfo.settings[i].setting.halMeta;
        info.vVendorInfo.add(mode, vInfo);
        inputSetting.vFrame.push_back(info);
    }
    // update p2 control
    updateSetting(outputInfo, inputSetting);
    //
    CAM_TRACE_FMT_BEGIN("FusionShot(%d):Vendor:Set", getOpenId());
    MY_LOGD("Vendor:Set");
    mpManager->set(plugin::CALLER_SHOT_SMART, inputSetting);
    CAM_TRACE_FMT_END();
    MY_LOGD("UpdateVendor-");
    MY_LOGD("SyncDualCam()+");
    SmartShotUtil::getInstance()->SyncDualCam(getOpenId());
    MY_LOGD("SyncDualCam()-");

    //
    MINT32 shotCount = inputSetting.vFrame.size();
    MY_LOGD("shotCount:%d", shotCount);
    if ( mbZsdFlow ) {
        // submit to zsd preview pipeline
        Vector<SettingSet> vSettings;
        for ( size_t i = 0; i < shotCount; ++i ) {
            SettingSet s;
            s.appSetting = inputSetting.vFrame[i].curAppControl;
            s.halSetting = inputSetting.vFrame[i].curHalControl;
            vSettings.push_back(s);
        }
        applyRawBufferSettings(vSettings, inputSetting.vFrame.size());
        // Since the number of capture requests sent to capture pipeline is not always the same as we sent to preview pipeline,
        // we have to update the capture count here.
        shotCount    = getShotMode() == eShotMode_FusionShot ? getBMDNCaptureCnt() :
                       getShotMode() == eShotMode_MFHRShot ? getMFHRCaptureCnt() : shotCount;
        MY_LOGD("[dual cam]shotCount(cap):%d", shotCount);
    }

    if ( createNewPipeline()) {
        CAM_TRACE_FMT_BEGIN("FusionShot(%d):CreatePipeline", getOpenId());
        MINT32 runtimeAllocateCount = 0;
        beginCapture( runtimeAllocateCount );
        //
        constructCapturePipeline();
        CAM_TRACE_FMT_END();
    }

    // get multiple raw buffer and send to capture pipeline
    for ( MINT32 i = 0; i < shotCount; ++i ) {
        IMetadata halSetting = mShotParam.mHalSetting;
        if ( mbZsdFlow ) {
            // get Selector Data (buffer & metadata)
            status_t status = OK;
            android::sp<IImageBuffer> pBuf = NULL; // full raw buffer
            IMetadata selectorAppMetadata; // app setting for this raw buffer. Ex.3A infomation
            status = getSelectorData(
                        selectorAppMetadata,
                        halSetting,
                        pBuf
                    );
            if( status != OK ) {
                MY_LOGE("GetSelectorData Fail!");
                return MFALSE;
            }
            //
            {
                Mutex::Autolock _l(mResultMetadataSetLock);
                mResultMetadataSetMap.editValueFor(mCapReqNo).selectorAppMetadata = selectorAppMetadata;
            }
        } else {
            halSetting += inputSetting.vFrame[i].curHalControl;
            IMetadata::IEntry entry(MTK_HAL_REQUEST_SENSOR_SIZE);
            entry.push_back(mSensorSize, Type2Type< MSize >());
            halSetting.update(entry.tag(), entry);
            {
                Mutex::Autolock _l(mResultMetadataSetLock);
                IMetadata resultAppMetadata;
                IMetadata resultHalMetadata;
                IMetadata selectorAppMetadata;
                Vector<ISelector::BufferItemSet> bufferSet;
                mResultMetadataSetMap.add(
                        static_cast<MUINT32>(mCapReqNo),
                        ResultSet_T{static_cast<MUINT32>(mCapReqNo), resultAppMetadata, resultHalMetadata, selectorAppMetadata, bufferSet, NONE_CB_DONE}
                        );
            }
        }
        // submit setting to capture pipeline
        if (OK != submitCaptureSetting(
                    i == 0,
                    inputSetting.vFrame[i].curAppControl,
                    halSetting) ) {
            MY_LOGE("Submit capture setting fail.");
            return MFALSE;
        }
        //
        mCapReqNo++;
    }
    // 1. wait pipeline done
    // 2. set selector back to default zsd selector
    // 3. set full raw buffer count back to normal
    endCapture();
    //
    return MTRUE;
}

/******************************************************************************
 *
 ******************************************************************************/
static
MVOID
PREPARE_STREAM(BufferList& vDstStreams, StreamId id, MBOOL criticalBuffer)
{
    vDstStreams.push_back(
        BufferSet{
            .streamId       = id,
            .criticalBuffer = criticalBuffer,
        }
    );
}

MBOOL
FusionShot::
applyRawBufferSettings(
    Vector< SettingSet > vSettings,
    MINT32 shotCount
)
{
    CAM_TRACE_CALL();
    MY_LOGD("Apply user's setting.");
    //
    sp<StreamBufferProvider> pProvider = mpConsumer.promote();
    if(pProvider == NULL) {
        MY_LOGE("pProvider is NULL!");
        return MFALSE;
    }
    //
    sp<IFeatureFlowControl> pFeatureFlowControl = IResourceContainer::getInstance(getOpenId())->queryFeatureFlowControl();
    if(pFeatureFlowControl == NULL) {
        MY_LOGE("IFeatureFlowControl is NULL!");
        return MFALSE;
    }
    //
    HwInfoHelper helper(getOpenId());
    MSize sensorSize;
    if( ! helper.updateInfos() ) {
        MY_LOGE("cannot properly update infos!");
        return MFALSE;
    }
    //
    if( ! helper.getSensorSize( mu4Scenario, sensorSize) ) {
        MY_LOGE("cannot get params about sensor!");
        return MFALSE;
    }
    MY_LOGD("sensorMode(%d), sensorSize(%d,%d)", mu4Scenario, sensorSize.w, sensorSize.h);
    //
    for ( size_t i = 0; i < vSettings.size(); ++i ) {
        IMetadata::IEntry entry(MTK_HAL_REQUEST_SENSOR_SIZE);
        entry.push_back(sensorSize, Type2Type< MSize >());
        vSettings.editItemAt(i).halSetting.update(entry.tag(), entry);
    }
    //
    BufferList           vDstStreams;
    Vector< MINT32 >     vRequestNo;

    Vector<sp<IImageStreamInfo>> rawInputInfos;
    pProvider->querySelector()->queryCollectImageStreamInfo( rawInputInfos, MTRUE );
    BufferList vDstStreams_disp;
    MBOOL bRrzo = MFALSE;
    for( size_t i = 0; i < rawInputInfos.size() ; i++) {
        PREPARE_STREAM(vDstStreams, rawInputInfos[i]->getStreamId(),  true);
        if (rawInputInfos[i]->getStreamId() == eSTREAMID_IMAGE_PIPE_RAW_RESIZER) bRrzo = MTRUE;
    }
    // prepare RRZO, however we dont need it but LMV does.
    if (!bRrzo)
        PREPARE_STREAM(vDstStreams, eSTREAMID_IMAGE_PIPE_RAW_RESIZER, false);

    if( OK != pFeatureFlowControl->submitRequest(
                vSettings,
                vDstStreams,
                vRequestNo
                )
      )
    {
        MY_LOGE("submitRequest failed");
        return MFALSE;
    }
    //
    Vector< MINT32 >     vActualRequestNo;
    if ( shotCount < vRequestNo.size() ) {
        for ( MINT32 i = 0; i < shotCount; ++i ) vActualRequestNo.push_back(vRequestNo[i]);
    } else {
        vActualRequestNo = vRequestNo;
    }
    //
    status_t status;

    sp<DualCamSelector> pSelector = new DualCamSelector(getOpenId());
    pSelector->setRequests(vActualRequestNo);
    MINT32 cnt = getShotMode() == eShotMode_FusionShot ? getBMDNCaptureCnt() : getMFHRCaptureCnt();
    if (getShotMode() == eShotMode_FusionShot) {
        cnt = shotCount;
    }
    pSelector->setExpectedCount(cnt);
    status = pProvider->switchSelector(pSelector);
    MY_LOGD("DualCamSelector setExpectedCount %d", cnt);
    // TODO add LCSO consumer set the same Selector
    if(status != OK)
    {
        MY_LOGE("change selector Fail!");
        return MFALSE;
    }
    MY_LOGD("change selector Success");
    //
    return MTRUE;
}
/******************************************************************************
*
*******************************************************************************/
MBOOL
FusionShot::
createStreams()
{
  // do nothing, do createStreams prior to Vendor get/set
  // doCreateStreams();
  return MTRUE;
}
/******************************************************************************
*
*******************************************************************************/
MBOOL
FusionShot::
doCreateStreams()
{
    CAM_TRACE_CALL();
    //
    mu4Scenario = SENSOR_SCENARIO_ID_NORMAL_CAPTURE;
    mu4Bitdepth = getShotRawBitDepth(); // ImpShot
    //
    MBOOL const bEnablePostview = (mShotParam.miPostviewClientFormat != eImgFmt_UNKNOWN && !mbIgnoreJpegStream) ? MTRUE : MFALSE;
    MSize const postviewSize    = MSize(mShotParam.mi4PostviewWidth, mShotParam.mi4PostviewHeight);
    MINT const  postviewFmt     = static_cast<EImageFormat>(mShotParam.miPostviewClientFormat);
    MINT       yuvfmt_main      = eImgFmt_YUY2;
    MINT       yuvfmt_thumbnail = eImgFmt_YUY2;
#if (MTKCAM_HAVE_DUAL_ZOOM_FUSION_SUPPORT == 1)
    if (StereoSettingProvider::getStereoShotMode() == eShotMode_FusionShot)
    {
        yuvfmt_main      = eImgFmt_NV21;
        yuvfmt_thumbnail = eImgFmt_NV21;
    }
#endif
    //
    MSize const thumbnailsize = MSize(mJpegParam.mi4JpegThumbWidth, mJpegParam.mi4JpegThumbHeight);
    //
    if(mbZsdFlow)
    {
        Vector<sp<IImageStreamInfo>> rawInputInfos;
        sp<StreamBufferProvider> pConsumer = mpConsumer.promote();
        if( !pConsumer.get() ) return UNKNOWN_ERROR;
        pConsumer->querySelector()->queryCollectImageStreamInfo( rawInputInfos, MTRUE );
        for(size_t i = 0; i < rawInputInfos.size() ; i++)
        {
            if(rawInputInfos[i]->getStreamId() == eSTREAMID_IMAGE_PIPE_RAW_LCSO)
                mpInfo_LcsoRaw = rawInputInfos[i];
            else if(rawInputInfos[i]->getStreamId() == eSTREAMID_IMAGE_PIPE_RAW_OPAQUE)
                mpInfo_FullRaw = rawInputInfos[i];
            else if(rawInputInfos[i]->getStreamId() == eSTREAMID_IMAGE_PIPE_RAW_RESIZER)
            {
                MY_LOGD("eSTREAMID_IMAGE_PIPE_RAW_RESIZER");
                mpInfo_ResizedRaw = rawInputInfos[i];
            }
        }
    }
    else
    {
        MY_LOGE("non-zsd flow not support!");
        return MFALSE;
    }

    MINT32 yuvBufCnt = 2;
    MY_LOGD("bmdnPhases:%d mfhrPhases:%d", getBMDNCapturePhaseCnt(), getMFHRCapturePhaseCnt());
    yuvBufCnt = std::max(getBMDNCapturePhaseCnt(), getMFHRCapturePhaseCnt());
    MY_LOGD("yuvBufCnt:%d", yuvBufCnt);
    //
    // postview YUV
    if (bEnablePostview)
    {
        MSize size        = postviewSize;
        MINT format       = postviewFmt;
        MUINT const usage = 0; //not necessary here
        MUINT32 transform = 0;
        sp<IImageStreamInfo>
            pStreamInfo = createImageStreamInfo(
                    "SingleShot:Postview",
                    eSTREAMID_IMAGE_PIPE_YUV_00,
                    eSTREAMTYPE_IMAGE_INOUT,
                    yuvBufCnt, 1,
                    usage, format, size, transform
                    );
        if(pStreamInfo == nullptr) return BAD_VALUE;
        //
        mpInfo_YuvPostview = pStreamInfo;
    }
    //
    // Yuv
    {
        MSize size        = mJpegsize;
        MINT format       = yuvfmt_main;
        MUINT const usage = 0; //not necessary here
        MUINT32 transform = mShotParam.mu4Transform;
        sp<IImageStreamInfo>
            pStreamInfo = createImageStreamInfo(
                    "SmartShot:MainYuv",
                    eSTREAMID_IMAGE_PIPE_YUV_JPEG,
                    eSTREAMTYPE_IMAGE_INOUT,
                    yuvBufCnt, 1,
                    usage, format, size, transform
                    );
        if(pStreamInfo == nullptr) return BAD_VALUE;
        //
        mpInfo_Yuv = pStreamInfo;
    }
    //
    // Thumbnail Yuv
    {
        MSize size        = thumbnailsize;
        MINT format       = yuvfmt_thumbnail;
        MUINT const usage = 0; //not necessary here
        MUINT32 transform = 0;
        sp<IImageStreamInfo>
            pStreamInfo = createImageStreamInfo(
                    "SmartShot:ThumbnailYuv",
                    eSTREAMID_IMAGE_PIPE_YUV_THUMBNAIL,
                    eSTREAMTYPE_IMAGE_INOUT,
                    yuvBufCnt, 1,
                    usage, format, size, transform
                    );
        if(pStreamInfo == nullptr) return BAD_VALUE;
        //
        mpInfo_YuvThumbnail = pStreamInfo;
    }
    //
    // Jpeg
    if(!mbIgnoreJpegStream)
    {
        MSize size        = mJpegsize;
        MINT format       = eImgFmt_BLOB;
        MUINT const usage = 0; //not necessary here
        MUINT32 transform = 0;
        sp<IImageStreamInfo>
            pStreamInfo = createImageStreamInfo(
                    "SmartShot:Jpeg",
                    eSTREAMID_IMAGE_JPEG,
                    eSTREAMTYPE_IMAGE_INOUT,
                    1, 1,
                    usage, format, size, transform
                    );
        if(pStreamInfo == nullptr) return BAD_VALUE;
        //
        mpInfo_Jpeg = pStreamInfo;
    }
    //
    return MTRUE;
}

/******************************************************************************
*
*******************************************************************************/
MBOOL
FusionShot::
createPipeline()
{
    CAM_TRACE_CALL();
    FUNC_START;
    LegacyPipelineBuilder::ConfigParams LPBConfigParams;
    LPBConfigParams.mode = getLegacyPipelineMode();
    LPBConfigParams.enableEIS = MFALSE;
    LPBConfigParams.enableLCS = mShotParam.mbEnableLtm;
    LPBConfigParams.pluginUser = plugin::CALLER_SHOT_SMART;
    LPBConfigParams.dualcamMode = StereoSettingProvider::getStereoFeatureMode();

    if ( mbZsdFlow ) {
        Vector<sp<IImageStreamInfo>> rawInputInfos;
        sp<StreamBufferProvider> pConsumer = mpConsumer.promote();
        if( !pConsumer.get() ) return UNKNOWN_ERROR;
        pConsumer->querySelector()->queryCollectImageStreamInfo( rawInputInfos, MTRUE );
        for(size_t i = 0; i < rawInputInfos.size() ; i++)
        {
            if(rawInputInfos[i]->getStreamId() == eSTREAMID_IMAGE_PIPE_RAW_LCSO)
                LPBConfigParams.enableLCS = MTRUE;
        }
    }
    //
    HwInfoHelper helper(getOpenId());
    if( ! helper.updateInfos() ) {
        MY_LOGE("cannot properly update infos");
        return BAD_VALUE;
    }
    //
    if (helper.getDualPDAFSupported(mu4Scenario))
    {
        LPBConfigParams.enableDualPD = MTRUE;
    }
    //
    sp<LegacyPipelineBuilder> pBuilder = LegacyPipelineBuilder::createInstance(
                                            getOpenId(),
                                            "SmartShot",
                                            LPBConfigParams);
    CHECK_OBJECT(pBuilder);

    mpImageCallback = new SImageCallback(this, 0);
    CHECK_OBJECT(mpImageCallback);

    sp<BufferCallbackHandler> pCallbackHandler = new BufferCallbackHandler(getOpenId());
    CHECK_OBJECT(pCallbackHandler);
    pCallbackHandler->setImageCallback(mpImageCallback);
    mpCallbackHandler = pCallbackHandler;

    sp<StreamBufferProviderFactory> pFactory =
                StreamBufferProviderFactory::createInstance();
    //
    Vector<PipelineImageParam> vImageParam;
    //
    {
        MY_LOGD("createPipeline for BMDN shot");
        if ( mbZsdFlow ) {
            Vector<sp<IImageStreamInfo>> rawInputInfos;
            sp<StreamBufferProvider> pConsumer = mpConsumer.promote();
            if( !pConsumer.get() ) return UNKNOWN_ERROR;
            pConsumer->querySelector()->queryCollectImageStreamInfo( rawInputInfos, MTRUE );
            //
            Vector<PipelineImageParam> vImgSrcParams;
            sp<IImageStreamInfo> pStreamInfo = NULL;
            for(size_t i = 0; i < rawInputInfos.size() ; i++) {
                pStreamInfo = rawInputInfos[i];
                sp<CallbackBufferPool> pPool = new CallbackBufferPool(pStreamInfo);
                //pPool->addBuffer(pBuf);
                mpCallbackHandler->setBufferPool(pPool);
                //
                pFactory->setImageStreamInfo(pStreamInfo);
                pFactory->setUsersPool(
                        mpCallbackHandler->queryBufferPool(pStreamInfo->getStreamId())
                    );
                PipelineImageParam imgParam = {
                    .pInfo     = pStreamInfo,
                    .pProvider = pFactory->create(false),
                    .usage     = 0
                };
                vImgSrcParams.push_back(imgParam);
            }
            //
            if( OK != pBuilder->setSrc(vImgSrcParams) ) {
                MY_LOGE("setSrc failed");
                return MFALSE;
            //
            }
        } else {
            MY_LOGE("non-zsd mode not support!");
            return MFALSE;
        }
    }
    //
    {
        if( mpInfo_Yuv.get() )
        {
            sp<IImageStreamInfo> pStreamInfo = mpInfo_Yuv;
            //
            sp<CallbackBufferPool> pPool = new CallbackBufferPool(pStreamInfo);
            {
                pPool->allocateBuffer(
                                  pStreamInfo->getStreamName(),
                                  pStreamInfo->getMaxBufNum(),
                                  pStreamInfo->getMinInitBufNum()
                                  );
                mpCallbackHandler->setBufferPool(pPool);
            }
            //
            pFactory->setImageStreamInfo(pStreamInfo);
            pFactory->setUsersPool(
                        mpCallbackHandler->queryBufferPool(pStreamInfo->getStreamId())
                    );
            //
            PipelineImageParam imgParam = {
                .pInfo     = pStreamInfo,
                .pProvider = pFactory->create(false),
                .usage     = 0
            };
            vImageParam.push_back(imgParam);
        }
        //
        if( mpInfo_YuvPostview.get() )
        {
            sp<IImageStreamInfo> pStreamInfo = mpInfo_YuvPostview;
            //
            sp<CallbackBufferPool> pPool = new CallbackBufferPool(pStreamInfo);
            {
                pPool->allocateBuffer(
                                  pStreamInfo->getStreamName(),
                                  pStreamInfo->getMaxBufNum(),
                                  pStreamInfo->getMinInitBufNum()
                                  );
            }
            mpCallbackHandler->setBufferPool(pPool);
            //
            pFactory->setImageStreamInfo(pStreamInfo);
            pFactory->setUsersPool(
                        mpCallbackHandler->queryBufferPool(pStreamInfo->getStreamId())
                    );
            //
            PipelineImageParam imgParam = {
                .pInfo     = pStreamInfo,
                .pProvider = pFactory->create(false),
                .usage     = 0
            };
            vImageParam.push_back(imgParam);
        }
        //
        if( mpInfo_YuvThumbnail.get() )
        {
            sp<IImageStreamInfo> pStreamInfo = mpInfo_YuvThumbnail;
            //
            sp<CallbackBufferPool> pPool = new CallbackBufferPool(pStreamInfo);
            {
                pPool->allocateBuffer(
                                  pStreamInfo->getStreamName(),
                                  pStreamInfo->getMaxBufNum(),
                                  pStreamInfo->getMinInitBufNum()
                                  );
            }
            mpCallbackHandler->setBufferPool(pPool);
            //
            pFactory->setImageStreamInfo(pStreamInfo);
            pFactory->setUsersPool(
                        mpCallbackHandler->queryBufferPool(pStreamInfo->getStreamId())
                    );
            //
            PipelineImageParam imgParam = {
                .pInfo     = pStreamInfo,
                .pProvider = pFactory->create(false),
                .usage     = 0
            };
            vImageParam.push_back(imgParam);
        }
        //
        if( mpInfo_Jpeg.get() )
        {
            CAM_TRACE_NAME("set mpInfo_Jpeg");
            sp<IImageStreamInfo> pStreamInfo = mpInfo_Jpeg;
            //
            sp<CallbackBufferPool> pPool = new CallbackBufferPool(pStreamInfo);
            {
                CAM_TRACE_NAME("allocateBuffer");
                pPool->allocateBuffer(
                                  pStreamInfo->getStreamName(),
                                  pStreamInfo->getMaxBufNum(),
                                  pStreamInfo->getMinInitBufNum()
                                  );
            }
            mpCallbackHandler->setBufferPool(pPool);
            //
            pFactory->setImageStreamInfo(pStreamInfo);
            pFactory->setUsersPool(
                        mpCallbackHandler->queryBufferPool(pStreamInfo->getStreamId())
                    );
            //
            PipelineImageParam imgParam = {
                .pInfo     = pStreamInfo,
                .pProvider = pFactory->create(false),
                .usage     = 0
            };
            vImageParam.push_back(imgParam);
        }
        //
        if( OK != pBuilder->setDst(vImageParam) ) {
            MY_LOGE("setDst failed");
            return MFALSE;
        }
    }
    //
    mpPipeline = pBuilder->create();
    //
    FUNC_END;
    return MTRUE;

}

/******************************************************************************
*
*******************************************************************************/
MERROR
FusionShot::
submitCaptureSetting(
    MBOOL mainFrame,
    IMetadata appSetting,
    IMetadata halSetting
)
{
    CAM_TRACE_CALL();
    ILegacyPipeline::ResultSet resultSet;
    if ( mbZsdFlow ){
        Mutex::Autolock _l(mResultMetadataSetLock);
        resultSet.add(eSTREAMID_META_APP_DYNAMIC_P1, mResultMetadataSetMap.editValueFor(mCapReqNo).selectorAppMetadata);
    }
    auto pPipeline = mpPipeline;
    if( !pPipeline.get() )
    {
        MY_LOGW("get pPipeline fail");
        return UNKNOWN_ERROR;
    }
    //
    if ( mainFrame ) {
        if( OK != pPipeline->submitSetting(
                    mCapReqNo,
                    appSetting,
                    halSetting,
                    &resultSet
                    )
          )
        {
            MY_LOGE("submitRequest failed");
            return UNKNOWN_ERROR;
        }
    } else {
        BufferList vDstStreams;
        if ( !mbZsdFlow ) {
            PREPARE_STREAM(vDstStreams, eSTREAMID_IMAGE_PIPE_RAW_OPAQUE, false);
            if ( mShotParam.mbEnableLtm )
                PREPARE_STREAM(vDstStreams, eSTREAMID_IMAGE_PIPE_RAW_LCSO, false);
        } else {
            Vector<sp<IImageStreamInfo>> rawInputInfos;
            sp<StreamBufferProvider> pConsumer = mpConsumer.promote();
            if( !pConsumer.get() ) return UNKNOWN_ERROR;
            pConsumer->querySelector()->queryCollectImageStreamInfo( rawInputInfos, MTRUE );
            for( size_t i = 0; i < rawInputInfos.size() ; i++) {
                PREPARE_STREAM(vDstStreams, rawInputInfos[i]->getStreamId(),  false);
            }
        }
        if( OK != pPipeline->submitRequest(
                    mCapReqNo,
                    appSetting,
                    halSetting,
                    vDstStreams,
                    &resultSet
                    )
          )
        {
            MY_LOGE("submitRequest failed");
            return UNKNOWN_ERROR;
        }
    }


    return OK;
}

/******************************************************************************
*
*******************************************************************************/
MBOOL
FusionShot::
isNeedToIgnoreJpegStream(
)
{
    MBOOL ret = MFALSE;
    if(StereoSettingProvider::StereoSettingProvider::isDualCamMode())
    {
        if(StereoSettingProvider::getStereoFeatureMode() == NSCam::v1::Stereo::E_STEREO_FEATURE_DENOISE
            ||
            (StereoSettingProvider::getStereoShotMode() == eShotMode_FusionShot))
        {
            MINT32 main1OpenId, main2OpenId;
            if(!StereoSettingProvider::getStereoSensorIndex(main1OpenId, main2OpenId))
            {
                MY_LOGW("cannot get dual cam open id");
                goto lbExit;
            }
            if(getOpenId() == main2OpenId)
            {
                // for denoise/fusion, main2 always does not output jpeg.
                ret = MTRUE;
            }
            goto lbExit;
        }
    }
    else
    {
        goto lbExit;
    }
lbExit:
    return ret;
}

/******************************************************************************
*
*******************************************************************************/
MBOOL
FusionShot::
handleJpegData(MUINT32 const requestNo, IImageBuffer* pJpeg)
{
    CAM_TRACE_CALL();
    //
    class scopedVar
    {
    public:
                    scopedVar(IImageBuffer* pBuf)
                    : mBuffer(pBuf) {
                        if( mBuffer )
                            mBuffer->lockBuf(LOG_TAG, eBUFFER_USAGE_SW_READ_MASK);
                    }
                    ~scopedVar() {
                        if( mBuffer )
                            mBuffer->unlockBuf(LOG_TAG);
                    }
    private:
        IImageBuffer* mBuffer;
    } _local(pJpeg);
    //
    uint8_t const* puJpegBuf = (uint8_t const*)pJpeg->getBufVA(0);
    MUINT32 u4JpegSize = pJpeg->getBitstreamSize();

    MY_LOGD("+ (puJpgBuf, jpgSize) = (%p, %d)",
            puJpegBuf, u4JpegSize);

    if(mFusionShotPorterFlag == 1)
    {
        auto now = chrono::system_clock::now();
        auto duration = now.time_since_epoch();
        auto millis = chrono::duration_cast<chrono::milliseconds>(duration).count();
        doPorter(pJpeg, requestNo, millis, MTRUE);
    }

    // dummy raw callback
    mpShotCallback->onCB_RawImage(0, 0, NULL);

    // Jpeg callback
    mpShotCallback->onCB_CompressedImage_packed(0,
                                         u4JpegSize,
                                         puJpegBuf,
                                         0,                       //callback index
                                         true                     //final image
                                         );
    MY_LOGD("-");

    return MTRUE;
}


/******************************************************************************
*
*******************************************************************************/
MERROR
FusionShot::
updateSetting(
    plugin::OutputInfo& param,
    plugin::InputSetting& setting
)
{
    MINT64 p2_setting = MTK_P2_ISP_PROCESSOR|MTK_P2_YUV_PROCESSOR|MTK_P2_CAPTURE_REQUEST;
    if (StereoSettingProvider::getStereoShotMode() == eShotMode_FusionShot)
    {
    }
    else
    {
        MY_LOGE("Error Configure StereoShotMode:%d" , StereoSettingProvider::getStereoShotMode());
        exit(1);
    }

    for( size_t i = 0; i < setting.vFrame.size(); ++i ) {
        IMetadata::IEntry entry(MTK_PLUGIN_P2_COMBINATION);
        entry.push_back(p2_setting, Type2Type< MINT64 >());
        setting.vFrame.editItemAt(i).curHalControl.update(entry.tag(), entry);
    }
    MY_LOGD("cathy %" PRId64 , p2_setting);

    return OK;
    // check flash needed
    // yes --> apply raw
    // no --> normal
}



/*******************************************************************************
*
********************************************************************************/
MVOID
FusionShot::
doPorter(android::sp<IImageBuffer> pBuf, MINT32 reqNo, MUINT64 millis, MBOOL removeOriginalData)
{
    MSize size = pBuf->getImgSize();
    string msFilename = "/sdcard/bmdenoise/"+to_string(reqNo)+"_"+to_string(millis);
    string msTuningFilename = msFilename + "/0";
    NSCam::Utils::makePath(msTuningFilename.c_str(), 0660);
    // move other tuning data to time-stamp specific folder
    string msCMD = "cp -R /sdcard/bmdenoise/"+to_string(reqNo)+"/* "+msTuningFilename;
    MY_LOGD("Move tuning data, cmd:%s", msCMD.c_str());
    system(msCMD.c_str());
    // jpeg
    string saveFileName = msFilename +
                               "/"+
                               "JPG_DENOISE"+
                               "_"+
                               to_string(millis)+
                               ".jpg";
    MY_LOGD("Move jpeg, cmd:%s", msFilename.c_str());
    pBuf->saveToFile(saveFileName.c_str());
    // 3a debug bin
    string ms3aFilename = msFilename + "/debug";
    NSCam::Utils::makePath(ms3aFilename.c_str(), 0660);
    msCMD = "cp -R /sdcard/bmdenoise/"+to_string(reqNo)+"/debug/* "+msFilename+"/debug";
    MY_LOGD("Move 3a debug bin, cmd:%s", msCMD.c_str());
    system(msCMD.c_str());
    msCMD = "rm -rf "+msTuningFilename+"/debug/";
    MY_LOGD("remove 3a debug bin in the new folder, cmd:%s", msCMD.c_str());
    system(msCMD.c_str());
    // CModelData
    string msCModelFilePath = msFilename + "/CModelData";
    NSCam::Utils::makePath(msCModelFilePath.c_str(), 0660);
    msCMD = "cp -R /sdcard/bmdenoise/CModelData/* " + msCModelFilePath;
    MY_LOGD("Move CModelDatam, cmd:%s", msCMD.c_str());
    system(msCMD.c_str());
    // Raw
    msCMD = "cp -R /sdcard/bmdenoise/"+to_string(reqNo)+"_raw/* "+msFilename+"/";
    MY_LOGD("Move raw, cmd:%s", msCMD.c_str());
    system(msCMD.c_str());
    if(removeOriginalData){
        // remove original data
        msCMD = "rm -rf /sdcard/bmdenoise/"+to_string(reqNo);
        MY_LOGD("remove original data, cmd:%s", msCMD.c_str());
        system(msCMD.c_str());
        // remove original CModelData
        msCMD = "rm -rf /sdcard/bmdenoise/CModelData";
        MY_LOGD("remove original CModelData, cmd:%s", msCMD.c_str());
        system(msCMD.c_str());
        // remove original raw
        msCMD = "rm -rf /sdcard/bmdenoise/"+to_string(reqNo)+"_raw";
        MY_LOGD("remove original raw, cmd:%s", msCMD.c_str());
        system(msCMD.c_str());
    }
}

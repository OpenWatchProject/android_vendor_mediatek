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

#define LOG_TAG "MtkCam/BMDNShot"

#include <sstream>
#include <dirent.h>

#include <mtkcam/utils/std/Log.h>
#include <mtkcam/def/common.h>
#include <mtkcam/utils/std/Trace.h>

#include <mtkcam/middleware/v1/IShot.h>

#include "ImpShot.h"
#include "BMDNShot.h"

#include <mtkcam/middleware/v1/LegacyPipeline/StreamId.h>
#include <mtkcam/utils/metadata/hal/mtk_platform_metadata_tag.h>

#include <mtkcam/utils/hw/HwInfoHelper.h>

#include <mtkcam/middleware/v1/LegacyPipeline/LegacyPipelineUtils.h>
#include <mtkcam/middleware/v1/LegacyPipeline/LegacyPipelineBuilder.h>
#include <mtkcam/middleware/v1/LegacyPipeline/buffer/StreamBufferProviderFactory.h>

#include <mtkcam/feature/stereo/hal/stereo_setting_provider.h>
#include <camera_custom_stereo.h>

#include <mtkcam/aaa/IHal3A.h>

#define BM_DENOISE_PORTER_KEY "vendor.debug.bmdenoise.porter.dump"
#define BM_DENOISE_PURE_ZSD_KEY "vendor.debug.bmdenoise.purezsd"

using namespace NSShot::NSBMDNShot;
using namespace NSCam;
using namespace NSCam::v1;
using namespace NSCam::v1::NSLegacyPipeline;
using namespace NSCamHW;

using namespace NS3Av3;
using NS3Av3::IHal3A;

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
template <typename T>
inline MBOOL
tryGetMetadata(
    IMetadata const* const pMetadata,
    MUINT32 const tag,
    T & rVal
)
{
    if( pMetadata == NULL ) {
        //MY_LOGE("pMetadata == NULL");
        return MFALSE;
    }

    IMetadata::IEntry entry = pMetadata->entryFor(tag);
    if( !entry.isEmpty() ) {
        rVal = entry.itemAt(0, Type2Type<T>());
        return MTRUE;
    }
    return MFALSE;
}
/******************************************************************************
 *
 ******************************************************************************/
template <class T>
inline MBOOL
trySetMetadata(
        IMetadata& metadata,
        MUINT32 const tag,
        T const& val)
{
    IMetadata::IEntry entry(tag);
    entry.push_back(val, Type2Type<T>());
    if (OK == metadata.update(entry.tag(), entry)) {
        return MTRUE;
    }
    //
    return MFALSE;
}

/******************************************************************************
 *
 ******************************************************************************/
extern
sp<IShot>
createInstance_BMDNShot(
    char const*const    pszShotName,
    uint32_t const      u4ShotMode,
    int32_t const       i4OpenId
)
{
    sp<IShot>  pShot            = NULL;
    sp<BMDNShot>  pImpShot = NULL;
    //
    //  (1.1) new Implementator.
    pImpShot = new BMDNShot(pszShotName, u4ShotMode, i4OpenId, true);
    if  ( pImpShot == 0 ) {
        CAM_LOGE("[%s] new BMDNShot", __FUNCTION__);
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
BMDNShot::
BMDNShot(
    char const*const pszShotName,
    uint32_t const   u4ShotMode,
    int32_t const    i4OpenId,
    bool const       isZsd
)
    : SmartShot(pszShotName, u4ShotMode, i4OpenId, isZsd)
{
    mBMDNShotPorterFlag  = ::property_get_int32(BM_DENOISE_PORTER_KEY, 0);
    mForcePureZSD        = ::property_get_int32(BM_DENOISE_PURE_ZSD_KEY, -1);

    MY_LOGD("mBMDNShotPorterFlag(%d), mForcePureZSD(%d)",
        mBMDNShotPorterFlag,
        mForcePureZSD
    );

    mvPlugInKey.clear();
    mvPlugIn_P2_Comb.clear();

    if(StereoSettingProvider::getStereoSensorIndex(mMain1OpenId, mMain2OpenId)){
        MY_LOGD("mMain1OpenId(%d), mMain2OpenId(%d)", mMain1OpenId, mMain2OpenId);
    }else{
        MY_LOGE("cannot get dual cam open id");
    }
}

/******************************************************************************
 *
 ******************************************************************************/
BMDNShot::
~BMDNShot()
{
    MY_LOGD("~BMDNShot()");
    if( mResultMetadataSetMap.size() > 0 )
    {
        int n = mResultMetadataSetMap.size();
        for(int i=0; i<n; i++)
        {
            MY_LOGW("requestNo(%d) doesn't clear before BMDNShot destroyed",mResultMetadataSetMap.keyAt(i));
            mResultMetadataSetMap.editValueAt(i).selectorGetBufs.clear();
        }
    }
    mResultMetadataSetMap.clear();

    if(!mvPlugInKey.empty()){
        MY_LOGW("mvPlugInKey not empty!");
        mvPlugInKey.clear();
    }

    if(!mvPlugIn_P2_Comb.empty()){
        MY_LOGW("mvPlugIn_P2_Comb not empty!");
        mvPlugIn_P2_Comb.clear();
    }

    // always do this to avoid 3A locked
    if(getOpenId() == mMain1OpenId){
        afterCapture();
    }
}

/******************************************************************************
 *
 ******************************************************************************/
bool
BMDNShot::
onCmd_capture()
{
    CAM_TRACE_NAME("BMDNShot onCmd_capture");
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
    plugin::InputInfo inputInfo;
    MINT64 mode = [ this ](MINT32 const type) -> MINT64 {
        switch(type)
        {
            case eShotMode_MFHRShot: {
                mPureZSD = getIsPureZSD_MFHR();
                return MTK_PLUGIN_MODE_MFHR;
            }
            case eShotMode_BMDNShot: {
                mPureZSD = getIsPureZSD_BMDN();
                return MTK_PLUGIN_MODE_BMDN;
            }
        }
        MY_LOGW("no mapped vendor for type %d", type);
        return MTK_PLUGIN_MODE_COMBINATION;
    }(getShotMode());

    if(mForcePureZSD != -1){
        mPureZSD = mForcePureZSD;
        MY_LOGW("mPureZSD = mForcePureZSD:%d", mPureZSD);
    }else{
        MY_LOGW("mPureZSD:%d", mPureZSD);
    }

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
    CAM_TRACE_FMT_BEGIN("BMDNShot(%d):Vendor:Get", getOpenId());
    MY_LOGD("Vendor:Get+");
    mpManager->get(plugin::CALLER_SHOT_SMART, inputInfo, outputInfo);
    CAM_TRACE_FMT_END();
    MY_LOGD("Vendor:Get-");
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

        // plug-in combination key
        MINT64 pluginKey = -1;
        if ( !tryGetMetadata<MINT64>( (IMetadata*)&outputInfo.settings[i].setting.halMeta, MTK_PLUGIN_COMBINATION_KEY, pluginKey ) ) {
            MY_LOGW("MTK_PLUGIN_COMBINATION_KEY not found.");
            return false;
        }
        mvPlugInKey.push_back(pluginKey);
    }
    // update p2 control
    updateSetting(outputInfo, inputSetting);

    // plug-in P2 combination
    MINT64 plugin_P2_comb = -1;
    for( size_t i = 0; i < inputSetting.vFrame.size(); ++i ) {
        if ( !tryGetMetadata<MINT64>( (IMetadata*)&inputSetting.vFrame.editItemAt(i).curHalControl, MTK_PLUGIN_P2_COMBINATION, plugin_P2_comb ) ) {
            MY_LOGW("MTK_PLUGIN_P2_COMBINATION not found.");
            return false;
        }
        mvPlugIn_P2_Comb.push_back(plugin_P2_comb);
    }

    CAM_TRACE_FMT_BEGIN("BMDNShot(%d):Vendor:Set", getOpenId());
    MY_LOGD("Vendor:Set+");
    mpManager->set(plugin::CALLER_SHOT_SMART, inputSetting);
    CAM_TRACE_FMT_END();
    MY_LOGD("Vendor:Set-");

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
        SettingSet s;
        for ( size_t i = 0; i < shotCount; ++i ) {
            s.appSetting = inputSetting.vFrame[i].curAppControl;
            s.halSetting = inputSetting.vFrame[i].curHalControl;
            vSettings.push_back(s);
        }
        for ( size_t i = 0; i < getHWSyncStableCnt(); ++i ){
            vSettings.push_back(s);
        }
        applyRawBufferSettings(vSettings, inputSetting.vFrame.size());
        MY_LOGD("[dual cam]shotCount:%d hwSyncCount:%d", shotCount, getHWSyncStableCnt());
    }

    if ( createNewPipeline()) {
        CAM_TRACE_FMT_BEGIN("BMDNShot(%d):CreatePipeline", getOpenId());
        MINT32 runtimeAllocateCount = 0;
        beginCapture( runtimeAllocateCount );
        //
        constructCapturePipeline();
        CAM_TRACE_FMT_END();
    }

    // get multiple raw buffer and send to capture pipeline
    for ( MINT32 i = 0; i < shotCount; ++i ) {

        MY_LOGD("getSelectorData and submitCaptureSetting %d/%d mCapReqNo(%u)",
            i, shotCount, mCapReqNo);

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
        if(i == 0 && getOpenId() == mMain1OpenId){
            MY_LOGD("insert req(%u)", mCapReqNo);
            Mutex::Autolock _l(mReqSetLock);
            if(mReqSet.empty()){
                beforeCapture();
            }
            mReqSet.insert(mCapReqNo);
        }
        mCapReqNo++;
    }

    if(mPureZSD){
        sp<StreamBufferProvider> pConsumer = mpConsumer.promote();
        if( !pConsumer.get() ){
            MY_LOGE("cannot promote mpConsumer");
            return MFALSE;
        }
        pConsumer->setSelector(mspOriSelector);
        mspOriSelector = nullptr;
    }else{
        endCapture();
    }
    return MTRUE;
    }

/******************************************************************************
 *
 ******************************************************************************/
void
BMDNShot::
onCmd_cancel()
{
    FUNC_START;
    auto pPipeline = mpPipeline;
    //
    if( pPipeline.get() )
    {
        pPipeline->waitUntilDrained();
        pPipeline->flush();
        mpPipeline = nullptr;
        }

    // always do this when cancel capture to avoid 3A locked
    if(getOpenId() == mMain1OpenId){
        afterCapture();
    }
    FUNC_END;
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
BMDNShot::
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

    status_t status;
    sp<DualCamSelector> pSelector = new DualCamSelector(getOpenId());
    pSelector->setExpectedCount(shotCount);

    if(mPureZSD){
        mspOriSelector = pProvider->querySelector();
        pProvider->querySelector()->copyPoolsToSelector(pSelector);
        pProvider->querySelector()->sendCommand(ISelector::eCmd_setAllBuffersTransferring, MTRUE, 0, 0);
        pProvider->querySelector()->transferResults(pSelector);
        pProvider->querySelector()->sendCommand(ISelector::eCmd_setAllBuffersTransferring, MFALSE, 0, 0);
    }else{
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
        pSelector->setRequests(vRequestNo);
    }
    MY_LOGD("shotCount(%d) / vRequestNo(%d)", shotCount, vRequestNo.size());
    MY_LOGD("DualCamSelector");
    status = pProvider->switchSelector(pSelector);
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
BMDNShot::
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
BMDNShot::
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
    MINT const yuvfmt_main      = eImgFmt_YUY2;
    MINT const yuvfmt_thumbnail = eImgFmt_YUY2;
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
        MUINT32 transform = mShotParam.mu4Transform;
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
        MSize size        = (mbIgnoreJpegStream) ? MSize(1,1) : mJpegsize;
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
        MSize size        = (mbIgnoreJpegStream) ? MSize(1,1) : thumbnailsize;
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
BMDNShot::
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
        mpConsumer = IResourceContainer::getInstance(getOpenId())->queryConsumer( eSTREAMID_IMAGE_PIPE_RAW_OPAQUE );
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
BMDNShot::
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

    // update plug-in key
    if(mvPlugInKey.empty()){
        MY_LOGE("mvPlugInKey is empty!");
        return UNKNOWN_ERROR;
    }else{
        MINT64 pluginKey = mvPlugInKey.front();
        mvPlugInKey.pop_front();

        trySetMetadata<MINT64>(halSetting, MTK_PLUGIN_COMBINATION_KEY, pluginKey);
    }

    // update plug-in P2 combination
    if(mvPlugIn_P2_Comb.empty()){
        MY_LOGE("mvPlugIn_P2_Comb is empty!");
        return UNKNOWN_ERROR;
    }else{
        MINT64 plugin_P2_comb = mvPlugIn_P2_Comb.front();
        mvPlugIn_P2_Comb.pop_front();

        trySetMetadata<MINT64>(halSetting, MTK_PLUGIN_P2_COMBINATION, plugin_P2_comb);
    }

    // update isp gain in pureZSD mode
    if(mPureZSD){
        MINT32 shotModeIdx = -1;
        switch(getShotMode()){
            case eShotMode_MFHRShot:
                shotModeIdx = 2;
                break;
            case eShotMode_BMDNShot:
                shotModeIdx = 1;
                break;
            default:
                shotModeIdx = 1;
                break;
        }
        MY_LOGD("update denoise shot mode index(%d) for p2 isp gain", shotModeIdx);
        trySetMetadata<MINT32>(halSetting, MTK_STEREO_FEATURE_DENOISE_MODE, shotModeIdx);
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
BMDNShot::
isNeedToIgnoreJpegStream(
)
{
    MBOOL ret = MFALSE;
    if(StereoSettingProvider::StereoSettingProvider::isDualCamMode())
    {
        if(StereoSettingProvider::getStereoFeatureMode() == NSCam::v1::Stereo::E_STEREO_FEATURE_DENOISE)
        {
            if(getOpenId() == mMain2OpenId)
            {
                // for denoise, main2 always does not output jpeg.
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
BMDNShot::
handleJpegData(MUINT32 const requestNo, IImageBuffer* pJpeg)
{
    CAM_TRACE_CALL();
    FUNC_START;
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

    if(mBMDNShotPorterFlag == 1)
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
                                         true,                     //final image
                                         MTK_CAMERA_MSG_EXT_DATA_COMPRESSED_IMAGE,
                                         getShotMode()
                                         );
    if(mBMDNShotPorterFlag == 1){
        mpShotCallback->onCB_P2done();
    }

    if(getOpenId() == mMain1OpenId){
        MY_LOGD("remove req(%d)", requestNo);
        Mutex::Autolock _l(mReqSetLock);
        mReqSet.erase(requestNo);
        if(mReqSet.empty()){
            afterCapture();
        }
    }

    FUNC_END;
    return MTRUE;
}

/*******************************************************************************
*
********************************************************************************/
/**
 * The function arrange the directorise base on the directorise of BMDN shot dump data
 *
 * The original directory architecture is:
 *
 * ├── sdcard
 * │   ├── bmdenoise
 * │   │   ├── 0
 * │   │   │   ├── 0
 * │   │   │   │   └──debug
 * │   │   │   ├── 1
 * │   │   │   └── 2
 * │   │   ├── 0_raw
 * │   │   │   └─ *.raw
 * │   │   ├── 3
 * │   │   │   ├── 3
 * │   │   │   │   └──debug
 * │   │   │   ├── 4
 * │   │   │   └── 5
 * │   │   ├── 1_raw
 * │   │   │   └─ *.raw
 * │   │   └── CModelData
 *
 * The arranged directory architecture is:
 *
 * ├── sdcard
 * │   ├── bmdenoise
 * │   │   ├── clock_time_01
 * │   │   │   ├── 0
 * │   │   │   │   ├── 0
 * │   │   │   │   ├── 1
 * │   │   │   │   └── 2
 * │   │   │   ├── *.raw
 * │   │   │   ├── debug
 * │   │   │   ├── CModeData
 * │   │   │   └── JPG_DENOISE_clock_time_01.jpg
 * │   │   │
 * │   │   ├── clock_time_02
 * │   │   │   ├── 0
 * │   │   │   │   ├── 0
 * │   │   │   │   ├── 1
 * │   │   │   │   └── 2
 * │   │   │   ├── *.raw
 * │   │   │   ├── debug
 * │   │   │   ├── CModeData
 * │   │   │   └── JPG_DENOISE_clock_time_02.jpg
 * │   │   └── CModelData
 *
*/
MVOID
BMDNShot::
doPorter(android::sp<IImageBuffer> pBuf, MINT32 reqNo, MUINT64 millis, MBOOL removeOriginalData)
{
    FUNC_START;

    class Path
    {
    public:
        Path(const string& root) : msstream(root){
            msstream << root;
        }

        Path& addSub(const string& sub) {
            msstream << '/';
            msstream << sub;
            return *this;
        }

        Path& addSub(const string& n1, const string& n2) {
            msstream << '/';
            msstream << n1;
            msstream << '_';
            msstream << n2;
            return *this;
        }

        Path& addExtension(const string& extension) {
            msstream << '.';
            msstream << extension;
            return *this;
        }

        string str(){
            return msstream.str();
        }
    private:
        std::stringstream msstream;
    };

    auto getCMD = [this] (const string& cmd, const string& arg, const string& src, const string& dst) -> string {
        std::stringstream outsstream;
        outsstream << cmd;
        if(!arg.empty()) outsstream << " " << arg;
        outsstream << " " << src;
        if(!dst.empty()) outsstream << " " << dst;
        return outsstream.str();
    };
    auto executeMoveCMD = [this, getCMD] (const string& src, const string& dst) {
        string cmdTmp = getCMD("mv", "", src, dst);
        MY_LOGD("[doPorter] execure move cmd: %s", cmdTmp.c_str());
        system(cmdTmp.c_str());
    };
    auto executeRemoveCMD = [this, getCMD] (const string& src) {
        string cmdTmp = getCMD("rm", "-rf", src, "");
        MY_LOGD("[doPorter] execure remove cmd: %s", cmdTmp.c_str());
        system(cmdTmp.c_str());
    };
    auto executeMakeDirCMD = [this] (const string& dir) {
        MY_LOGD("[doPorter] make dir.: %s", dir.c_str());
        NSCam::Utils::makePath(dir.c_str(), 0660);
    };
    auto getFileNames = [this] (const string& rootDir) {
        std::list<string> fileNames;
        DIR *dir;
        struct dirent *entry;
        if ((dir = ::opendir(rootDir.c_str())) == NULL) {
            MY_LOGW("[doPorter] opendir error: %s", rootDir.c_str());
        }
        else {
            while ((entry = readdir(dir)) != NULL) {
                string fileName = entry->d_name;
                if(fileName != "." && fileName != "..") {
                    fileNames.push_back(fileName);
                }
            }
            closedir(dir);
        }
        return fileNames;
    };


    const string rootDir = "/sdcard/bmdenoise";
    const string reqNoStr = to_string(reqNo);
    const string timeStr = to_string(millis);
    const string srcRootDir = Path(rootDir).addSub(reqNoStr).str();
    const string dstRootDir = Path(rootDir).addSub(timeStr).str();

    string cmdTmp;
    string srcTemp;
    string dstTemp;

    // [1] move CModelData
    srcTemp = Path(rootDir).addSub("CModelData").addSub("*").str();
    dstTemp = Path(dstRootDir).addSub("CModelData").str();
    executeMakeDirCMD(dstTemp);
    executeMoveCMD(srcTemp, dstTemp);

    // [2] move raw data
    srcTemp = Path(rootDir).addSub(reqNoStr, "raw").addSub("*").str();
    dstTemp = Path(dstRootDir).str();
    executeMoveCMD(srcTemp, dstTemp);
    srcTemp = Path(rootDir).addSub(reqNoStr, "raw").str();
    executeRemoveCMD(srcTemp);

    // [3] move debug profile
    srcTemp = Path(srcRootDir).addSub(reqNoStr).addSub("debug").str();
    dstTemp = Path(dstRootDir).addSub("debug").str();
    executeMoveCMD(srcTemp, dstTemp);

    // [4] save jpeg file
    string jpegFileName = Path(dstRootDir).addSub("JPG_DENOISE", timeStr).addExtension("jpg").str();
    MY_LOGD("save jpeg file, path: %s", jpegFileName.c_str());
    pBuf->saveToFile(jpegFileName.c_str());

    // [5-1] move request dump data
    srcTemp = Path(srcRootDir).str();
    dstTemp = Path(dstRootDir).addSub("0").str();
    executeMoveCMD(srcTemp, dstTemp);

    // [5-2] rename request dump data
    string reqDataRootDir = dstTemp;
    std::list<string> fileNames = getFileNames(reqDataRootDir);
    if(fileNames.size() > 0)
    {
        int index = 0;
        for(auto fileName : fileNames) {
            srcTemp = Path(reqDataRootDir).addSub(fileName).str();
            dstTemp = Path(reqDataRootDir).addSub(to_string(index++)).str();
            executeMoveCMD(srcTemp, dstTemp);
        }
    }

    FUNC_END;
}
/******************************************************************************
*
*******************************************************************************/
MVOID
BMDNShot::
beforeCapture()
{
    FUNC_START;

    if(!mPureZSD){
        MY_LOGD("do nothing when not in pureZSD");
        return;
    }

    std::unique_ptr <
                    IHal3A,
                    std::function<void(IHal3A*)>
                    > hal3a_main1
            (
                MAKE_Hal3A(mMain1OpenId, LOG_TAG),
                [](IHal3A* p){ if (p) p->destroyInstance(LOG_TAG); }
            );
    if (hal3a_main1.get() == nullptr) {
        MY_LOGE("create IHal3A instance failed");
        return;
    }

    std::unique_ptr <
                    IHal3A,
                    std::function<void(IHal3A*)>
                    > hal3a_main2
            (
                MAKE_Hal3A(mMain2OpenId, LOG_TAG),
                [](IHal3A* p){ if (p) p->destroyInstance(LOG_TAG); }
            );
    if (hal3a_main2.get() == nullptr) {
        MY_LOGE("create IHal3A instance failed");
        return;
    }

    // lock AE
    hal3a_main1->send3ACtrl(E3ACtrl_EnableDisableAE,  (MINTPTR)0, 0);
    hal3a_main2->send3ACtrl(E3ACtrl_EnableDisableAE,  (MINTPTR)0, 0);
    FUNC_END;
    return;
}
/******************************************************************************
*
*******************************************************************************/
MVOID
BMDNShot::
afterCapture()
{
    FUNC_START;

    if(!mPureZSD){
        MY_LOGD("do nothing when not in pureZSD");
        return;
    }

    std::unique_ptr <
                    IHal3A,
                    std::function<void(IHal3A*)>
                    > hal3a_main1
            (
                MAKE_Hal3A(mMain1OpenId, LOG_TAG),
                [](IHal3A* p){ if (p) p->destroyInstance(LOG_TAG); }
            );
    if (hal3a_main1.get() == nullptr) {
        MY_LOGE("create IHal3A instance failed");
        return;
    }

    std::unique_ptr <
                    IHal3A,
                    std::function<void(IHal3A*)>
                    > hal3a_main2
            (
                MAKE_Hal3A(mMain2OpenId, LOG_TAG),
                [](IHal3A* p){ if (p) p->destroyInstance(LOG_TAG); }
            );
    if (hal3a_main2.get() == nullptr) {
        MY_LOGE("create IHal3A instance failed");
        return;
    }
    // unlock AE
    hal3a_main1->send3ACtrl(E3ACtrl_EnableDisableAE,  (MINTPTR)1, 0);
    hal3a_main2->send3ACtrl(E3ACtrl_EnableDisableAE,  (MINTPTR)1, 0);
    FUNC_END;
    return;
}

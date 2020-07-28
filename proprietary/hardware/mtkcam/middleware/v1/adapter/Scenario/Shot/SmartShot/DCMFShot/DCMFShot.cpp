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

#define LOG_TAG "MtkCam/DCMFShot"

#include <sstream>
#include <dirent.h>

#include <mtkcam/def/common.h>
#include <mtkcam/utils/std/Log.h>
#include <mtkcam/utils/std/Trace.h>

#include <mtkcam/utils/metadata/hal/mtk_platform_metadata_tag.h>
#include <mtkcam/utils/metastore/IMetadataProvider.h>
#include <mtkcam/utils/hw/HwInfoHelper.h>
#include <mtkcam/utils/hw/IScenarioControl.h>

#include <mtkcam/middleware/v1/IShot.h>
#include <mtkcam/middleware/v1/LegacyPipeline/StreamId.h>
#include <mtkcam/middleware/v1/camshot/_params.h>

#include <mtkcam/feature/effectHalBase/ICallbackClientMgr.h>
#include <mtkcam/feature/stereo/hal/stereo_setting_provider.h>
#include <mtkcam/feature/stereo/hal/stereo_size_provider.h>

#include <camera_custom_stereo.h>

#include <mtkcam/aaa/IHal3A.h>

#include "ImpShot.h"
#include "DCMFShot.h"

#define DCMF_HDR_RETRY_MAX 2

using namespace NSShot::NSDCMFShot;
using namespace NSCam;
using namespace NSCam::v1;
using namespace NSCam::v1::NSLegacyPipeline;
using namespace NSCamHW;
using namespace android::NSPostProc;

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
//
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

#define FUNCTION_SCOPE_D            ScopeLogger __scope_logger(::gettid(), getOpenId(), true,  mDebugLog, getShotName(), __FUNCTION__)
#define FUNCTION_SCOPE_I            ScopeLogger __scope_logger(::gettid(), getOpenId(), false, mDebugLog, getShotName(), __FUNCTION__)

/******************************************************************************
 * static menber initialization
 ******************************************************************************/
Mutex                                              DCMFShot::gCapCBHandlerLock;
Condition                                          DCMFShot::gCondCapCBHandlerLock;
std::map<MINT32, sp<DCMFShot::CaptureCBHandler> >  DCMFShot::gvCapCBHandlers;

Mutex                                              DCMFShot::gValidInsLock;
MINT32                                             DCMFShot::gValidIns = 0;

DCMFShot::DualOperation                            DCMFShot::gDualOperation;

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
createInstance_DCMFShot(
    char const*const    pszShotName,
    uint32_t const      u4ShotMode,
    int32_t const       i4OpenId
)
{
    sp<IShot>  pShot            = NULL;
    sp<DCMFShot>  pImpShot = NULL;
    //
    //  (1.1) new Implementator.
    pImpShot = DCMFShot::createInstance(pszShotName, u4ShotMode, i4OpenId, true);
    if  ( pImpShot == 0 ) {
        CAM_LOGE("[%s] new DCMFShot", __FUNCTION__);
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
 *  This function is invoked when this object is ready to destryoed in the
 *  destructor. All resources must be released before this returns.
 ******************************************************************************/
void
DCMFShot::
onDestroy()
{
    FUNCTION_SCOPE_I;

    onCmd_cancel();

    exitHighPerf();

    Thread::requestExit();
    mCondCapReqLock.signal();
    Thread::requestExitAndWait();

    if( mFuture.valid() ){
        MY_LOGD("wait future job +");
        mFuture.get();
        MY_LOGD("wait future job -");
    }
}

/******************************************************************************
 *
 ******************************************************************************/
DCMFShot*
DCMFShot::
createInstance(
    char const*const pszShotName,
    uint32_t const   u4ShotMode,
    int32_t const    i4OpenId,
    bool const       isZsd
    )
{
    CAM_LOGI("[%s] +", __FUNCTION__);
    DCMFShot* ret = nullptr;
    {
        Mutex::Autolock _l(gValidInsLock);
        ret = new DCMFShot(pszShotName, u4ShotMode, i4OpenId, isZsd);
        CAM_LOGI("[%s] create DCMFShot, addr:%p, pszShotName:%s, u4ShotMode:%ld, i4OpenId:%d isZsd:%d",
            __FUNCTION__, ret, pszShotName, u4ShotMode, i4OpenId, isZsd);
    }
    CAM_LOGI("[%s] -", __FUNCTION__);

    return ret;
}

DCMFShot::
DCMFShot(
    char const*const pszShotName,
    uint32_t const   u4ShotMode,
    int32_t const    i4OpenId,
    bool const       isZsd
)
    : SmartShot(pszShotName, u4ShotMode, i4OpenId, isZsd)
#if MTKCAM_HAVE_COMMON_CAPTURE_PIPELINE_SUPPORT && CCP_NOT_SUPPORT
    , mpPipeline(nullptr)
    , mpImageCallback(nullptr)
    , mpCallbackHandler(nullptr)
    , mpMetadataListener(nullptr)
    , mpMetadataListenerFull(nullptr)
#endif
{
    mvPlugInKey.clear();
    mvPlugIn_P2_Comb.clear();

    if(StereoSettingProvider::getStereoSensorIndex(mMain1OpenId, mMain2OpenId)){
        MY_LOGI("mMain1OpenId(%d), mMain2OpenId(%d)", mMain1OpenId, mMain2OpenId);
    }else{
        MY_LOGE("cannot get dual cam open id");
    }

    mDumpFlag = ::property_get_int32(DUMP_KEY, 0);
    if( mDumpFlag ) {
        mkdir(DUMP_PATH, S_IRWXU | S_IRWXG | S_IRWXO);
    }

    mDebugLog = ::property_get_int32(DEBUG_LOG, 0);

    mbDumpCali = ::property_get_int32(DUMP_CALI_KEY, 0);

    switch( getShotMode() ){
        case eShotMode_DCMFShot:
            MY_LOGI("MFNR+Bokeh");
            break;
        case eShotMode_DCMFHdrShot:
            MY_LOGI("HDR+Bokeh");
            break;
        default:
            MY_LOGE("unknown shotmode(%d)", getShotMode());
    }

    mHDRSync = ::property_get_int32(HDR_FRAME_SYNC, 1);

    sp<IMetadataProvider> pMetadataProvider = NSMetadataProviderManager::valueFor(getOpenId());
    if( pMetadataProvider.get() ){
        IMetadata const& staticMetadata = pMetadataProvider->getMtkStaticCharacteristics();
        if( !IMetadata::getEntry<MRect>(&staticMetadata, MTK_SENSOR_INFO_ACTIVE_ARRAY_REGION, mActiveArray) ){
            MY_LOGE("no static info: MTK_SENSOR_INFO_ACTIVE_ARRAY_REGION");
        }
    }else{
        MY_LOGE("there is no metadata provider!");
    }

    ++gValidIns;

    MY_LOGI("CCP(mtkcam:%d dcmf not support:%d) dump(0x%x) cali(%d) debug(%d) HDRsync(%d) activeArray(%d,%d,%dx%d) gValidIns(%d)",
        MTKCAM_HAVE_COMMON_CAPTURE_PIPELINE_SUPPORT, CCP_NOT_SUPPORT,
        mDumpFlag, mbDumpCali,
        mDebugLog,
        mHDRSync,
        mActiveArray.p.x, mActiveArray.p.y, mActiveArray.s.w, mActiveArray.s.h,
        gValidIns
    );

// #if MTKCAM_HAVE_COMMON_CAPTURE_PIPELINE_SUPPORT
#if 0
    gDualOperation.registerShotInstance(this);
#else
    // need a worker thread to release capture requests
    run(LOG_TAG);
#endif
}

/******************************************************************************
 *
 ******************************************************************************/
DCMFShot::
~DCMFShot()
{
    MY_LOGI("~DCMFShot()");
// #if MTKCAM_HAVE_COMMON_CAPTURE_PIPELINE_SUPPORT
#if 0
    gDualOperation.unRegisterShotInstance(this);
#endif
}

/******************************************************************************
 *
 ******************************************************************************/
status_t
DCMFShot::
onMsgReceived(
    MINT32 cmd, /*NOTIFY_MSG*/
    MINT32 /*arg1*/, MINT32 /*arg2*/,
    void*  /*arg3*/
)
{
    switch  (cmd)
    {
        case plugin::MSG_ON_NEXT_CAPTURE_READY:{
                Mutex::Autolock _l(mOnGoingPPLock);
                mP2DoneCnt ++;
            }
            if(mbZsdFlow) handleP2Done();
            break;
        case plugin::MSG_ON_SHUTTER_CALLBACK:
            mpShotCallback->onCB_Shutter(true,0,getShotMode());
            break;
    }

    return OK;
}

/******************************************************************************
 *
 ******************************************************************************/
bool
DCMFShot::
onCmd_capture()
{
    CAM_TRACE_NAME("DCMFShot onCmd_capture");
    MY_LOGI("+");
    Mutex::Autolock _l(mAPILock);

    // both capture start in same timing
    MY_LOGD("SyncDualCam() stage 0+");
    SmartShotUtil::getInstance()->SyncDualCam(getOpenId());
    MY_LOGD("SyncDualCam() stage 0-");

    {
        Mutex::Autolock _l(gValidInsLock);
        if( gValidIns != 2 ){
            MY_LOGW("capture cmd without enough valid instances (%d != 2), ignore", gValidIns);
            MY_LOGI("-");
            return true;
        }
    }

    if(!colletGlobalSettings()){
        MY_LOGE("colletGlobalSettings failed!");
        return false;
    }

    plugin::InputInfo inputInfo;
    if(!configVendor(inputInfo)){
        MY_LOGE("updateVendor failed!");
        return false;
    }

    plugin::OutputInfo outputInfo;
    if(!getVendorInfo(inputInfo, outputInfo)){
        MY_LOGE("getVendorInfo failed!");
        return false;
    }

    plugin::InputSetting inputSetting;
    MINT32 shotCount = 0;
    if(!setVendorInputSetting(outputInfo, inputSetting, shotCount)){
        MY_LOGE("getVendorInputSetting failed!");
        return false;
    }

    collectOneShotInfo();

    // to ensure vendor is ready
    MY_LOGD("SyncDualCam() stage 1+");
    SmartShotUtil::getInstance()->SyncDualCam(getOpenId());
    MY_LOGD("SyncDualCam() stage 1-");

    if(!submitRequestToPreviewPipeline(inputSetting)){
        MY_LOGE("submitRequestToPreviewPipeline failed!");
        return false;
    }

    // create capture pipeline here for for better performance
    if (createNewPipeline()){
        CAM_TRACE_FMT_BEGIN("DCMFShot(%d):CreatePipeline", getOpenId());
        MINT32 runtimeAllocateCount = 0;
        beginCapture( runtimeAllocateCount );

        // Since SmartShot::beginCapture() updates this valuse with rotation, we have to reset it.
        mJpegsize  = MSize(mShotParam.mi4PictureWidth, mShotParam.mi4PictureHeight); // no rotation

        constructCapturePipeline();

        CAM_TRACE_FMT_END();
    }

    MUINT32 mainReqNo = mCapReqNo;

    // make the bundle of all capture request belong to one shot
    if( isMaster() ){
        Mutex::Autolock _l(mOnGoingPPLock);
        vector<MUINT32> vReqSet;

        for(auto i=0 ; i<shotCount ; ++i){
            vReqSet.push_back(mCapReqNo + i);
            mvOnGoingPP.insert(mCapReqNo + i);
        }
        mvCapShotBundle.push_back(vReqSet);

        String8 str = String8::format("add new mCapShotBundle:");
        for (auto &bundle : mvCapShotBundle){
            std::stringstream s;
            std::copy(bundle.begin(), bundle.end(), std::ostream_iterator<MUINT32>(s, ","));
            str = str + String8::format(" [%s]", s.str().c_str());
        }
        MY_LOGD("%s\n", str.c_str());
    }

    for(MINT32 i = 0; i < shotCount; ++i){

        MY_LOGD("getSelectorData and submitCaptureRequest %d/%d mCapReqNo(%u)",
            i, shotCount, mCapReqNo);

        IMetadata shotAppMetadata = inputSetting.vFrame[i].curAppControl;
        IMetadata selectorAppMetadata;
        IMetadata selectorHalMetadata;
        Vector<PipelineImageParam> vImgDstParams;
        if(!prepareCaptureRequests(
            mainReqNo, i, shotAppMetadata,
            selectorAppMetadata, selectorHalMetadata, vImgDstParams
        )){
            MY_LOGE("prepareCaptureRequest failed!");
            return false;
        }

        MY_LOGD("SyncDualCam() stage 2+");
        SmartShotUtil::getInstance()->SyncDualCam(getOpenId());
        MY_LOGD("SyncDualCam() stage 2-");

        if (OK != submitCaptureRequest(
                i == 0,
                shotAppMetadata,
                selectorHalMetadata,
                vImgDstParams)
        ){
            MY_LOGE("submitCaptureRequest failed!");
            return false;
        }

        mCapReqNo++;
    }

    if( !isPureZsd() ){
        sp<StreamBufferProvider> pConsumer = mpConsumer.promote();
        if( !pConsumer.get() ){
            MY_LOGE("cannot promote mpConsumer");
            return false;
        }
        pConsumer->switchSelector(mspOriSelector);
        mspOriSelector = nullptr;
    }else{
        endCapture();
    }

    MY_LOGI("-");
    return MTRUE;
}

/******************************************************************************
 *
 ******************************************************************************/
void
DCMFShot::
onCmd_cancel()
{
    FUNCTION_SCOPE_I;
    Mutex::Autolock _l(mAPILock);

    if( mbVaild ){
        mbVaild = MFALSE;
        Mutex::Autolock _l(gValidInsLock);
        --gValidIns;
        MY_LOGI("valid instances %d->%d", gValidIns+1, gValidIns);
    }else{
        return;
    }

    std::chrono::nanoseconds maxWaitTimeNS(std::chrono::seconds(3));
    {
        Mutex::Autolock _l(mCapReqLock);
        while(mvCaptureRequests.size() > 0){
            MY_LOGW("mvCaptureRequests not empty(%lu), wait for it +", mvCaptureRequests.size());
            auto ret = mCondCapReqErased.waitRelative(mCapReqLock, maxWaitTimeNS.count());
            MY_LOGW("mvCaptureRequests not empty(%lu), wait for it -", mvCaptureRequests.size());
            if(ret != OK){
                MY_LOGE("mvCaptureRequests wait timeout!");
                break;
            }
        }
    }

    {
        Mutex::Autolock _l(gCapCBHandlerLock);
        while(gvCapCBHandlers.size() > 0){
            MY_LOGW("gvCapCBHandlers not empty(%lu), wait for it +", gvCapCBHandlers.size());
            auto ret = gCondCapCBHandlerLock.waitRelative(gCapCBHandlerLock, maxWaitTimeNS.count());
            MY_LOGW("gvCapCBHandlers not empty(%lu), wait for it -", gvCapCBHandlers.size());
            if(ret != OK){
                MY_LOGE("gvCapCBHandlers wait timeout!");
                break;
            }
        }
    }

    {
        std::chrono::nanoseconds maxWaitTimeForBG(std::chrono::seconds(15));
        MBOOL isNeedAOSPCallback= !mbBGService;
        if( isNeedAOSPCallback ){
            Mutex::Autolock _l(mOnGoingPPLock);
            while(mvOnGoingPP.size() > 0){
                MY_LOGW("mvOnGoingPP not empty(%lu), wait for it +", mvOnGoingPP.size());
                auto ret = mCondOnGoingPPLock.waitRelative(mOnGoingPPLock, maxWaitTimeNS.count());
                MY_LOGW("mvOnGoingPP not empty(%lu), wait for it -", mvOnGoingPP.size());
                if(ret != OK){
                    MY_LOGE("mvOnGoingPP wait timeout!");
                    break;
                }
            }
        }
    }

    auto pPipeline = mpPipeline;
    //
    if( pPipeline.get() )
    {
        pPipeline->waitUntilDrained();
        pPipeline->flush();
        mpPipeline = nullptr;
    }

    {
        Mutex::Autolock _l(mResultMetadataSetLock);
        if( mResultMetadataSetMap.size() > 0 ){
            int n = mResultMetadataSetMap.size();
            for(int i=0; i<n; i++){
                MY_LOGW("requestNo(%d) doesn't clear before DCMFShot destroyed",mResultMetadataSetMap.keyAt(i));
                mResultMetadataSetMap.editValueAt(i).selectorGetBufs.clear();
            }
        }
        mResultMetadataSetMap.clear();
    }

    if(!mvPlugInKey.empty()){
        MY_LOGW("mvPlugInKey not empty!");
        mvPlugInKey.clear();
    }

    if(!mvPlugIn_P2_Comb.empty()){
        MY_LOGW("mvPlugIn_P2_Comb not empty!");
        mvPlugIn_P2_Comb.clear();
    }

    // always do this when cancel capture to avoid 3A locked
    if(getOpenId() == mMain1OpenId){
        afterCapture();
    }

    exitHighPerf();
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

/******************************************************************************
 *
 ******************************************************************************/
static
MVOID
PREPARE_STREAM(BufferList& vDstStreams, StreamId id, MBOOL criticalBuffer, sp<StreamBufferProvider> spProvider)
{
    vDstStreams.push_back(
        BufferSet{
            .streamId       = id,
            .criticalBuffer = criticalBuffer,
            .provider       = spProvider
        }
    );
}

/******************************************************************************
 *
 ******************************************************************************/
MBOOL
DCMFShot::
applyRawBufferSettings_dcmf(
    Vector< SettingSet > vSettings,
    MINT32 shotCount,
    MINT32 delayCount
)
{
    CAM_TRACE_CALL();
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

    if( getShotMode() == eShotMode_DCMFHdrShot ){
        if( mHDRSync ){
            pSelector->setExpectedCount(1, 1);
        }else{
            pSelector->setExpectedCount(shotCount);
        }
    }else{
        pSelector->setExpectedCount(shotCount);
    }

    mspOriSelector = pProvider->querySelector();
    pSelector->setOriginSelector(mspOriSelector);
    pProvider->querySelector()->copyPoolsToSelector(pSelector);

    if( isPureZsd() ){
        MY_LOGD("grab zsd buffer");
        pProvider->querySelector()->sendCommand(ISelector::eCmd_setAllBuffersTransferring, MTRUE, 0, 0);
        pProvider->querySelector()->transferResults(pSelector);
        pProvider->querySelector()->sendCommand(ISelector::eCmd_setAllBuffersTransferring, MFALSE, 0, 0);
    }else{
        // get original priority
        struct sched_param sched_p;
        ::sched_getparam(0, &sched_p);
        int const priority  = sched_p.sched_priority -4;
        MY_LOGD("set priority:(origin, new)=(%d, %d)",
            sched_p.sched_priority,
            priority
        );
        // raise priority
        ::setpriority(PRIO_PROCESS, 0, priority);

        // to ensure the submit is synchronized
        MY_LOGD("SyncDualCam() stage 1.1+");
        SmartShotUtil::getInstance()->SyncDualCam(getOpenId());
        MY_LOGD("SyncDualCam() stage 1.1-");

        MY_LOGD("submit pass1 requests");
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
        // remove delay frames becuase they are not supposed to be used by anyone
        if( vRequestNo.size() < shotCount + delayCount){
            MY_LOGE("vReq(%d) < shot(%d) + delay(%d), should not happened!", vRequestNo.size(), shotCount, delayCount);
            return MFALSE;
        }else{
            // for ( int i = 0; i < vRequestNo.size(); ++i ) {MY_LOGD("before pop vReq[%d] = %d", i, vRequestNo.itemAt(i));}
            for ( int i = 0; i < delayCount; ++i){
                vRequestNo.pop();
            }
            // for ( int i = 0; i < vRequestNo.size(); ++i ) {MY_LOGD("after pop vReq[%d] = %d", i, vRequestNo.itemAt(i));}
        }

        if( getShotMode() == eShotMode_DCMFHdrShot ){
            if( mHDRSync ){
                Vector< MINT32 > vRequestNo_group1;
                Vector< MINT32 > vRequestNo_group2;

                std::copy(vRequestNo.begin(), vRequestNo.end() - 1, std::back_inserter(vRequestNo_group1));
                std::copy(vRequestNo.end() - 1, vRequestNo.end(), std::back_inserter(vRequestNo_group2));

                {
                    std::stringstream s;
                    std::copy(vRequestNo.begin(), vRequestNo.end(), std::ostream_iterator<MINT32>(s, " "));
                    MY_LOGD("vRequestNo: %s", s.str().c_str());
                }

                {
                    std::stringstream s;
                    std::copy(vRequestNo_group1.begin(), vRequestNo_group1.end(), std::ostream_iterator<MINT32>(s, " "));
                    MY_LOGD("vRequestNo_group1: %s", s.str().c_str());
                }

                {
                    std::stringstream s;
                    std::copy(vRequestNo_group2.begin(), vRequestNo_group2.end(), std::ostream_iterator<MINT32>(s, " "));
                    MY_LOGD("vRequestNo_group2: %s", s.str().c_str());
                }
                pSelector->setRequests(vRequestNo_group1, vRequestNo_group2);
            }else{
                pSelector->setRequests(vRequestNo);
            }
        }else{
            pSelector->setRequests(vRequestNo);
        }

        // restore priority
        MY_LOGD("restore priority:(origin)=(%d)",
            sched_p.sched_priority
        );
        ::setpriority(PRIO_PROCESS, 0, sched_p.sched_priority);
    }
    MY_LOGD("shotCount(%d) / vRequestNo(%zu)", shotCount, vRequestNo.size());

    status = pProvider->switchSelector(pSelector);

    if(status != OK)
    {
        MY_LOGE("change selector Fail!");
        return MFALSE;
    }

    return MTRUE;
}

/******************************************************************************
*
*******************************************************************************/
MBOOL
DCMFShot::
createStreams()
{
  // Due to performance tuning, do nothing.
  // Will doCreateStreams prior to Vendor get/set.
  return MTRUE;
}

/******************************************************************************
*
*******************************************************************************/
MBOOL
DCMFShot::
doCreateStreams()
{
    CAM_TRACE_CALL();

    mu4Scenario = SENSOR_SCENARIO_ID_NORMAL_CAPTURE;
    mu4Bitdepth = getShotRawBitDepth(); // ImpShot

    MBOOL const bEnablePostview = (mShotParam.miPostviewClientFormat != eImgFmt_UNKNOWN && isMaster()) ? MTRUE : MFALSE;

    MINT const  postviewFmt     = static_cast<EImageFormat>(mShotParam.miPostviewClientFormat);
    MSize postviewSize = StereoSizeProvider::getInstance()->postViewSize();
    if(mShotParam.mu4Transform & eTransform_ROT_90){
        swap(postviewSize.w, postviewSize.h);
    }

    MINT const yuvfmt_main      = eImgFmt_NV21;

    MINT const yuvfmt_thumbnail = eImgFmt_NV21;
    MSize const thumbnailSize   = getRRZOSize();

    MINT const yuvfmt_small     = eImgFmt_Y8;
    MSize const smyuvSize       = getSMSize();

    Vector<sp<IImageStreamInfo>> rawInputInfos;
    sp<StreamBufferProvider> pConsumer = mpConsumer.promote();
    if( !pConsumer.get() ) return UNKNOWN_ERROR;
    pConsumer->querySelector()->queryCollectImageStreamInfo( rawInputInfos, MTRUE );
    for(size_t i = 0; i < rawInputInfos.size() ; i++)
    {
        if(rawInputInfos[i]->getStreamId() == eSTREAMID_IMAGE_PIPE_RAW_LCSO){
            mpInfo_LcsoRaw = rawInputInfos[i];
        }
        else if(rawInputInfos[i]->getStreamId() == eSTREAMID_IMAGE_PIPE_RAW_OPAQUE){
            mpInfo_FullRaw = rawInputInfos[i];
        }
        else if(rawInputInfos[i]->getStreamId() == eSTREAMID_IMAGE_PIPE_RAW_RESIZER){
            mpInfo_ResizedRaw = rawInputInfos[i];
        }else{
            MY_LOGE("unknown stream [%" PRId64 "] !", rawInputInfos[i]->getStreamId());
        }
    }
// #if MTKCAM_HAVE_COMMON_CAPTURE_PIPELINE_SUPPORT
#if 0
    MINT32 yuvCount = getDCMFCaptureCnt() * 2; // *2 for fast S2S (P2_Done shot2shot);
#else
    MINT32 yuvCount = 1;
#endif

    MY_LOGD("yuv alloc count (%d)", yuvCount);

    //
    // postview YUV
    if(bEnablePostview)
    {
        MSize size        = postviewSize;
        MINT format       = postviewFmt;
        MUINT const usage = 0; //not necessary here
        MUINT32 transform = 0;
        sp<IImageStreamInfo>
            pStreamInfo = createImageStreamInfo(
                    "DCMFShot:Postview",
                    eSTREAMID_IMAGE_PIPE_YUV_00,
                    eSTREAMTYPE_IMAGE_INOUT,
                    yuvCount, 1,
                    usage, format, size, transform
                    );
        if(pStreamInfo == nullptr) return BAD_VALUE;
        //
        mpInfo_YuvPostview = pStreamInfo;
    }
    //
    // Yuv, for FS_YUV (IMGO produced full-size YUV)
    {
        MSize size        = getIMGOSize();
        MINT format       = yuvfmt_main;
        MUINT const usage = 0; //not necessary here
        MUINT32 transform = 0; //mShotParam.mu4Transform;
        sp<IImageStreamInfo>
            pStreamInfo = createImageStreamInfo(
                    "DCMFShot:MainYuv",
                    eSTREAMID_IMAGE_PIPE_YUV_JPEG,
                    eSTREAMTYPE_IMAGE_INOUT,
                    yuvCount, 1,
                    usage, format, size, transform
                    );
        if(pStreamInfo == nullptr) return BAD_VALUE;
        //
        mpInfo_Yuv = pStreamInfo;
    }
    //
    // Thumbnail Yuv, for RS_YUV (RRZO produced small-size YUV)
    {
        MSize size        = thumbnailSize;
        MINT format       = yuvfmt_thumbnail;
        MUINT const usage = 0; //not necessary here
        MUINT32 transform = 0;
        sp<IImageStreamInfo>
            pStreamInfo = createImageStreamInfo(
                    "DCMFShot:ThumbnailYuv",
                    eSTREAMID_IMAGE_PIPE_YUV_THUMBNAIL,
                    eSTREAMTYPE_IMAGE_INOUT,
                    yuvCount, 1,
                    usage, format, size, transform
                    );
        if(pStreamInfo == nullptr) return BAD_VALUE;
        //
        mpInfo_YuvThumbnail = pStreamInfo;
    }
    //
    // Small Yuv, for SM_YUV (IMGO produced very small-size YUV)
    {
        MSize size        = smyuvSize;
        MINT format       = yuvfmt_small;
        MUINT const usage = 0; //not necessary here
        MUINT32 transform = 0;
        sp<IImageStreamInfo>
            pStreamInfo = createImageStreamInfo(
                    "DCMFShot:SmallYuv",
                    eSTREAMID_IMAGE_PIPE_YUV_01,
                    eSTREAMTYPE_IMAGE_INOUT,
                    yuvCount, 1,
                    usage, format, size, transform
                    );
        if(pStreamInfo == nullptr) return BAD_VALUE;
        //
        mpInfo_YuvSmall = pStreamInfo;
    }
    //
    return MTRUE;
}

/******************************************************************************
*
*******************************************************************************/
MBOOL
DCMFShot::
colletGlobalSettings(){
    mbBGService = mShotParam.mbIsUsingBackgroundService;

    mpManager = NSVendorManager::get(getOpenId());
    CHECK_OBJECT(mpManager);

    mpConsumer = IResourceContainer::getInstance(getOpenId())->queryConsumer(eSTREAMID_IMAGE_PIPE_RAW_OPAQUE);
    mJpegsize  = MSize(mShotParam.mi4PictureWidth, mShotParam.mi4PictureHeight); // no rotation

    if (mpPipeline.get() == nullptr){
        doCreateStreams();
    }

    mVendorMode = [ this ](MINT32 const type) -> MINT64 {
        switch(type)
        {
            case eShotMode_DCMFShot:
            case eShotMode_DCMFHdrShot:
                return MTK_PLUGIN_MODE_DCMF_3RD_PARTY;
                break;
            default:
                MY_LOGW("no mapped vendor for type %d", type);
                return MTK_PLUGIN_MODE_COMBINATION;
        }
    }(getShotMode());

    if( getShotMode() == eShotMode_DCMFShot){
        mPureZSD = ::property_get_int32(DCMF_PURE_ZSD_KEY, getIsPureZSD_DCMF());
    }else{
        mPureZSD = MFALSE;
        MY_LOGD("dcmf hdr, pureZSD always %d", mPureZSD);
    }

    MY_LOGI("canSupportBGService(%d) pureZSD(%d)", mbBGService, mPureZSD);
    return MTRUE;
}

/******************************************************************************
*
*******************************************************************************/
MBOOL
DCMFShot::
configVendor(plugin::InputInfo& inputInfo){
    FUNCTION_SCOPE_D;

    // set Input
    if(!makeVendorCombination(inputInfo, mVendorMode)){
        MY_LOGE("make vendor combination failed!");
        return MFALSE;;
    }
    // metadata
    inputInfo.appCtrl      = mShotParam.mAppSetting;
    inputInfo.halCtrl      = mShotParam.mHalSetting;
    // image buffer
    inputInfo.fullRaw      = mpInfo_FullRaw;
    inputInfo.resizedRaw   = mpInfo_ResizedRaw;
    inputInfo.lcsoRaw      = mpInfo_LcsoRaw;
    inputInfo.jpegYuv      = mpInfo_Yuv;
    inputInfo.thumbnailYuv = mpInfo_YuvThumbnail;
    if(isMaster()){
        inputInfo.postview     = mpInfo_YuvPostview;
        inputInfo.vYuv.push_back(mpInfo_YuvSmall);
    }

    updateVendorFeatureTag(inputInfo);
    return MTRUE;
}

/******************************************************************************
*
*******************************************************************************/
MBOOL
DCMFShot::
getVendorInfo(plugin::InputInfo& inputInfo, plugin::OutputInfo& outputInfo){
    FUNCTION_SCOPE_D;
    CAM_TRACE_FMT_BEGIN("DCMFShot(%d):Vendor:Get", getOpenId());;

    mpManager->get(plugin::CALLER_SHOT_SMART, inputInfo, outputInfo);

    CAM_TRACE_FMT_END();
    return MTRUE;
}

/******************************************************************************
*
*******************************************************************************/
MBOOL
DCMFShot::
setVendorInputSetting(
    plugin::OutputInfo& outputInfo,
    plugin::InputSetting& inputSetting,
    MINT32& rShotCount
){
    FUNC_START;

    plugin::VendorInfo vInfo;
    vInfo.vendorMode = mVendorMode;

    inputSetting.pMsgCb = this;
    for ( int i = 0; i < outputInfo.frameCount; ++i ) {
        plugin::FrameInfo info;
        info.frameNo = mCapReqNo + i;
        info.curAppControl = mShotParam.mAppSetting;
        info.curHalControl = mShotParam.mHalSetting;
        info.curAppControl += outputInfo.settings[i].setting.appMeta;
        info.curHalControl += outputInfo.settings[i].setting.halMeta;
        info.vVendorInfo.add(vInfo.vendorMode, vInfo);
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

    CAM_TRACE_FMT_BEGIN("DCMFShot(%d):Vendor:Set", getOpenId());
    MY_LOGD("Vendor:Set+");
    mpManager->set(plugin::CALLER_SHOT_SMART, inputSetting);
    CAM_TRACE_FMT_END();
    MY_LOGD("Vendor:Set-");

    rShotCount = inputSetting.vFrame.size();

    FUNC_END;
    return MTRUE;
}

/******************************************************************************
*
*******************************************************************************/
MBOOL
DCMFShot::
submitRequestToPreviewPipeline(
    plugin::InputSetting& inputSetting
){
    FUNC_START;
    MBOOL ret = MTRUE;

    // submit to zsd preview pipeline

    // real frame-per-capture
    MINT32 shotCount = inputSetting.vFrame.size();
    Vector<SettingSet> vSettings;
    SettingSet s;
    for ( MINT32 i = 0; i < shotCount; ++i ) {
        s.appSetting = inputSetting.vFrame[i].curAppControl;
        s.halSetting = inputSetting.vFrame[i].curHalControl;
        vSettings.push_back(s);
    }

    SettingSet first;
    first.appSetting = inputSetting.vFrame[0].curAppControl;
    first.halSetting = inputSetting.vFrame[0].curHalControl;

    // hwsync stable counts
    MINT32 hwSyncCount = getHWSyncStableCnt();
    if( getShotMode() == eShotMode_DCMFHdrShot ){
        if( mHDRSync ){
            // in HDR mode, hwsync buffer is placed in the front,
            // to ensure +0 EV buffer pair is synchronized
            for ( decltype(hwSyncCount) i = 0; i < hwSyncCount; ++i ){
                vSettings.push_front(first);
            }
        }else{
            hwSyncCount = 0;
        }
    }else{
        // in other mode, hwsync buffer is placed in the back
        for ( decltype(hwSyncCount) i = 0; i < hwSyncCount; ++i ){
            vSettings.push_back(s);
        }
    }

    // delay frames, use first frame as input to recover 3A settings
    MINT32 delayCount = 0;
    if(tryGetMetadata<MINT32>(&first.halSetting, MTK_STEREO_DELAY_FRAME_COUNT, delayCount)){
        MY_LOGD("get delayCount(%d)", delayCount);
    }
    for ( decltype(delayCount) i = 0; i < delayCount; ++i ){
        vSettings.push_back(first);
    }

    MY_LOGD("shotCount:%d hwSyncCount:%zu delayCount:%d", shotCount, hwSyncCount, delayCount);

    if( getShotMode() == eShotMode_DCMFHdrShot ){
        // hdr may need to retry submit request
        for(int tryCount = 0 ; tryCount < DCMF_HDR_RETRY_MAX ; ++tryCount){

            MY_LOGD("try submit request (try:%d/max:%d)", tryCount, DCMF_HDR_RETRY_MAX);

            applyRawBufferSettings_dcmf(vSettings, inputSetting.vFrame.size(), delayCount);

            DCMFHdrSync::getInstance()->notifyMySelectResult(
                getOpenId(),
                getSelectorDataAndStore(shotCount)
            );

            if( !DCMFHdrSync::getInstance()->getDualSelectResult(getOpenId()) ){
                if(tryCount < DCMF_HDR_RETRY_MAX - 1){
                    MY_LOGW("try submit request (try:%d/max:%d) failed, re-trying", tryCount, DCMF_HDR_RETRY_MAX);
                    returnStoredSelectorData();
                }else{
                    MY_LOGW("try submit request (try:%d/max:%d) reach max, capture failed and handle dummy callbacks", tryCount, DCMF_HDR_RETRY_MAX);
                    returnStoredSelectorData();
                    handleDummyCallbacks();
                    ret = MFALSE;
                    break;
                }
            }else{
                MY_LOGD("try submit request (try:%d/max:%d) success!", tryCount, DCMF_HDR_RETRY_MAX);
                mDebugTryCount = 0;
                break;
            }
        }
    }else{
        // zsd mode, simply get buffer from selector
        applyRawBufferSettings_dcmf(vSettings, inputSetting.vFrame.size(), delayCount);
    }

    FUNC_END;
    return ret;
}

/******************************************************************************
*
*******************************************************************************/
MBOOL
DCMFShot::
prepareCaptureRequests(
    const MUINT32 mainReqNo,
    const MUINT32 shotIdx,
    IMetadata& shotAppSetting,
    IMetadata& selectorAppSetting,
    IMetadata& selectorHalSetting,
    Vector<PipelineImageParam>& rvImageParam
){
    FUNC_START;

    selectorHalSetting = mShotParam.mHalSetting;

    // get Selector Data (buffer & metadata)
    status_t status = OK;
    android::sp<IImageBuffer> pBuf = NULL; // full raw buffer
    if( getShotMode() == eShotMode_DCMFHdrShot ){
        status = getStoredSelectorData(
                    selectorAppSetting,
                    selectorHalSetting,
                    pBuf
                );
    }else{
        status = getSelectorData(
                    selectorAppSetting,
                    selectorHalSetting,
                    pBuf
                );
    }

    if( status != OK ) {
        MY_LOGE("GetSelectorData Fail! Suicide: kill DCMFShot - raise(SIGINT)");
        ::raise(SIGINT);
        return MFALSE;
    }

    {
        Mutex::Autolock _l(mResultMetadataSetLock);
        mResultMetadataSetMap.editValueFor(mCapReqNo).selectorAppMetadata = selectorAppSetting;
    }

// #if MTKCAM_HAVE_COMMON_CAPTURE_PIPELINE_SUPPORT
#if 0
    // no need to create BufferCallbackHandler because it is held by common capture pipeline
    sp<BufferCallbackHandler> pCallbackHandler = nullptr;
#else
    // by-request create cb handler since the buffer provider are also created with each request
    sp<BufferCallbackHandler> pCallbackHandler = new BufferCallbackHandler(getOpenId());
    CHECK_OBJECT(pCallbackHandler);
    pCallbackHandler->setImageCallback(mpImageCallback);
#endif

    CaptureRequest newCapReq;
    newCapReq.mCapReqNo = mCapReqNo;
    newCapReq.spCbHandler = pCallbackHandler;
    Vector<StreamId> vDstMetadata;
    vDstMetadata.push_back(eSTREAMID_META_APP_CONTROL);
    vDstMetadata.push_back(eSTREAMID_META_HAL_DYNAMIC_P2);

    if( ! prepareImgBufProvider_Dst(rvImageParam, pCallbackHandler, (shotIdx == 0) ? MTRUE : MFALSE) ){
        MY_LOGE("prepare dst buffer failed");
        return MFALSE;
    }

    newCapReq.spImgStreamMgr = createImgStreamMgr(rvImageParam);
    newCapReq.spCbImgBufMgr = new CallbackImageBufferManager(DCMF_PIPE_ID, newCapReq.spImgStreamMgr);
    MINT64 timestamp = -1;
    getTimestamp(&selectorAppSetting, timestamp);
    newCapReq.spCbImgBufMgr->setTimestamp(mCapReqNo, timestamp);
    for(auto &e : rvImageParam){
        newCapReq.vDstStreams[e.pInfo->getStreamId()] = nullptr;
    }

    {
        Mutex::Autolock _l(mCapReqLock);
        if(shotIdx == 0){
            if(mvCaptureRequests.empty()){
                beforeCapture(rvImageParam);
            }
        }
        MY_LOGD("insert cap req(%u)", mCapReqNo);
        mvCaptureRequests[mCapReqNo] = newCapReq;
    }
    {
        Mutex::Autolock _l(gCapCBHandlerLock);
        // 1.for main request:
        // create a new CaptureCBHandler if no existing one with mainReqNo == mCapReqNo
        if(mCapReqNo == mainReqNo){
            if(gvCapCBHandlers.count(mainReqNo) == 0){

                MINT64 genTS = ICallbackClientMgr::generateTimestamp();

                gvCapCBHandlers[mainReqNo] = new CaptureCBHandler(
                    mainReqNo, genTS, mShotParam, mJpegParam,
                    mMain1OpenId, mMain2OpenId
                );

                gvCapCBHandlers[mainReqNo]->setIShotCB(mpShotCallback);

                gvCapCBHandlers[mainReqNo]->setBGServiceSupport(mbBGService);
            }
        }
        // 2.for main and other requests: register to CaptureCBHandler
        gvCapCBHandlers[mainReqNo]->registerCapReq(getOpenId(), newCapReq, rvImageParam, vDstMetadata);
        gvCapCBHandlers[mainReqNo]->setIPostProcCB(getOpenId(), this);

        const std::vector<MINT32>& vAllPPReqs = gvCapCBHandlers[mainReqNo]->getAllBelongReq(getOpenId());
        for(auto &e : vAllPPReqs){
            // mapping to final request
            mvPPReqMapping[e] = vAllPPReqs.back();
        }

        // 3.update some metadata that will not be returned by capture pipeline
        updateAppMeta(&selectorAppSetting, &selectorHalSetting, &shotAppSetting);
        gvCapCBHandlers[mainReqNo]->onMetaFinished(getOpenId(), mCapReqNo, eSTREAMID_META_APP_CONTROL, shotAppSetting);

        // 4.check whether need foreground postview
        gvCapCBHandlers[mainReqNo]->setForegroundPostviewSupport(
            supportForegroundPostview(&shotAppSetting)
        );
    }

    FUNC_END;
    return MTRUE;
}

/******************************************************************************
*
*******************************************************************************/
MBOOL
DCMFShot::
prepareImgBufProvider_Src(Vector<PipelineImageParam>& rvImageParam, sp<BufferCallbackHandler> pCbHandler)
{
    CAM_TRACE_CALL();
    MY_LOGD("");

    Vector<sp<IImageStreamInfo>> rawInputInfos;
    sp<StreamBufferProvider> pConsumer = mpConsumer.promote();
    if( !pConsumer.get() ) return UNKNOWN_ERROR;
    pConsumer->querySelector()->queryCollectImageStreamInfo( rawInputInfos, MTRUE );

    sp<IImageStreamInfo> pStreamInfo = nullptr;

    sp<StreamBufferProviderFactory> pFactory = StreamBufferProviderFactory::createInstance();
    for(size_t i = 0; i < rawInputInfos.size() ; i++) {
        pStreamInfo = rawInputInfos[i];
        sp<CallbackBufferPool> pPool = new CallbackBufferPool(pStreamInfo);
        pCbHandler->setBufferPool(pPool);
        //
        pFactory->setImageStreamInfo(pStreamInfo);
        pFactory->setUsersPool(
                pCbHandler->queryBufferPool(pStreamInfo->getStreamId())
            );
        PipelineImageParam imgParam = {
            .pInfo     = pStreamInfo,
            .pProvider = pFactory->create(false),
            .usage     = 0
        };
        rvImageParam.push_back(imgParam);
    }
    return MTRUE;
}

/******************************************************************************
*
*******************************************************************************/
MBOOL
DCMFShot::
prepareImgBufProvider_Dst(Vector<PipelineImageParam>& rvImageParam, sp<BufferCallbackHandler> pCbHandler, MBOOL isMainFrame)
{
    CAM_TRACE_CALL();
    MY_LOGD("");

// #if MTKCAM_HAVE_COMMON_CAPTURE_PIPELINE_SUPPORT
#if 0
    // no need to create provider, use common capture pipeline directly
    auto prepareStream = [&](sp<IImageStreamInfo> pStreamInfo){
        PipelineImageParam imgParam = {
            .pInfo     = pStreamInfo,
            .pProvider = mpPipeline->queryProvider(pStreamInfo->getStreamId()),
            .usage     = 0
        };
        rvImageParam.push_back(imgParam);
    };
#else
    // create new providers for each output
    sp<StreamBufferProviderFactory> pFactory = StreamBufferProviderFactory::createInstance();

    auto prepareStream = [&](sp<IImageStreamInfo> pStreamInfo){
        sp<CallbackBufferPool> pPool = new CallbackBufferPool(pStreamInfo);
        pPool->setExtraParam(
            MTRUE // continuous plane
        );
        {
            pPool->allocateBuffer(
                              pStreamInfo->getStreamName(),
                              pStreamInfo->getMaxBufNum(),
                              pStreamInfo->getMinInitBufNum()
                              );
            pCbHandler->setBufferPool(pPool);
        }

        pFactory->setImageStreamInfo(pStreamInfo);
        pFactory->setUsersPool(
                    pCbHandler->queryBufferPool(pStreamInfo->getStreamId())
                );

        PipelineImageParam imgParam = {
            .pInfo     = pStreamInfo,
            .pProvider = pFactory->create(false),
            .usage     = 0
        };
        rvImageParam.push_back(imgParam);
    };
#endif

    MBOOL need_FS_YUV = MFALSE;
    MBOOL need_RS_YUV = MFALSE;
    MBOOL need_SM_YUV = MFALSE;
    MBOOL need_postview = MFALSE;

    if( isMaster() ){
        if(DCMF_CAMERA_OUTPUT_LIST.count(DCMF_CAMERA_OUTPUTS::eFullSize_YUV_1) > 0){
            need_FS_YUV = MTRUE;
        }
        if( isMainFrame ){
            if(DCMF_CAMERA_OUTPUT_LIST.count(DCMF_CAMERA_OUTPUTS::eReSized_YUV_1) > 0){
                need_RS_YUV = MTRUE;
            }
            need_postview = MTRUE;
        }
        need_SM_YUV = MTRUE;
    }else{
        if( isMainFrame ){
            if(DCMF_CAMERA_OUTPUT_LIST.count(DCMF_CAMERA_OUTPUTS::eReSized_YUV_2) > 0){
                need_RS_YUV = MTRUE;
            }
            if(DCMF_CAMERA_OUTPUT_LIST.count(DCMF_CAMERA_OUTPUTS::eFullSize_YUV_2) > 0){
                need_FS_YUV = MTRUE;
            }
        }
    }

    MY_LOGD("Camera Outputs: FS(%d) RS(%d) Postview(%d) SM(%d)",
        need_FS_YUV, need_RS_YUV, need_postview, need_SM_YUV);

    if(need_FS_YUV){
        if( mpInfo_Yuv.get() ){
            prepareStream(mpInfo_Yuv);
        }
    }

    if(need_RS_YUV){
        if( mpInfo_YuvThumbnail.get() ){
            prepareStream(mpInfo_YuvThumbnail);
        }
    }

    if(need_postview){
        if( mpInfo_YuvPostview.get() && isMainFrame){
            prepareStream(mpInfo_YuvPostview);
        }
    }

    if(need_SM_YUV){
        if( mpInfo_YuvSmall.get()){
            prepareStream(mpInfo_YuvSmall);
        }
    }

    return MTRUE;
}

/******************************************************************************
*
*******************************************************************************/
sp<ImageStreamManager>
DCMFShot::
createImgStreamMgr(const Vector<PipelineImageParam>& vImageParam)
{
    CAM_TRACE_CALL();
    MY_LOGD("");

    // add one tail element for ending
    ImageStreamManager::image_stream_info_pool_setting* streamTbl =
        new ImageStreamManager::image_stream_info_pool_setting[vImageParam.size() + 1];

    for(size_t i=0 ; i<vImageParam.size() ; i++){
        streamTbl[i] = {
            vImageParam[i].pInfo->getStreamName(), vImageParam[i].pInfo->getStreamId(), vImageParam[i].pInfo->getStreamType(),
            vImageParam[i].pInfo->getMaxBufNum(), vImageParam[i].pInfo->getMinInitBufNum(),
            eBUFFER_USAGE_SW_READ_OFTEN |eBUFFER_USAGE_HW_CAMERA_READWRITE| eBUFFER_USAGE_SW_WRITE_OFTEN,
            vImageParam[i].pInfo->getImgFormat(), vImageParam[i].pInfo->getImgSize(),
            0,0, // stride, transform, not used
            eStreamType_IMG_HAL_PROVIDER,
            vImageParam[i].pProvider // although pProvider has been fiiled in the table, we have to manually updateBufProvider later
        };
    }

    // ending
    streamTbl[vImageParam.size()] = {"", 0, 0, 0, 0, 0, 0, MSize(), 0, 0, eStreamType(), nullptr};

    sp<ImageStreamManager> pImageStreamManager =
            ImageStreamManager::create(streamTbl);

    delete [] streamTbl;

    if(pImageStreamManager == nullptr){
        MY_LOGE("Create ImageStreamManager fail.");
        return nullptr;
    }

    // update buffer providers
    // TODO: can be done automatically in ImageStreamManager::init() ?
    for(size_t i=0 ; i<vImageParam.size() ; i++){
        pImageStreamManager->updateBufProvider(
            vImageParam[i].pInfo->getStreamId(),
            vImageParam[i].pProvider,
            Vector<StreamId_T>()
        );
    }

    pImageStreamManager->dump();
    return pImageStreamManager;
}

/******************************************************************************
*
*******************************************************************************/
// #if MTKCAM_HAVE_COMMON_CAPTURE_PIPELINE_SUPPORT
#if 0
MBOOL
DCMFShot::
createPipeline()
{
    CAM_TRACE_CALL();
    FUNC_START;

    if( ! mbZsdFlow ){
        MY_LOGE("non-zsd flow not supported!");
        return MFALSE;
    }

    ICommonCapturePipeline::PipelineConfig pipelineConfig;
    pipelineConfig.userName = LOG_TAG;
    pipelineConfig.openId = getOpenId();
    pipelineConfig.isZsdMode = mbZsdFlow;

    // LegacyPipelineBuilder::ConfigParams
    pipelineConfig.LPBConfigParams.mode = getLegacyPipelineMode();
    pipelineConfig.LPBConfigParams.enableEIS = MFALSE;
    pipelineConfig.LPBConfigParams.enableLCS = mShotParam.mbEnableLtm;
    pipelineConfig.LPBConfigParams.pluginUser = plugin::CALLER_SHOT_SMART;
    pipelineConfig.LPBConfigParams.dualcamMode = StereoSettingProvider::getStereoFeatureMode();

    Vector<sp<IImageStreamInfo>> rawInputInfos;
    sp<StreamBufferProvider> pConsumer = mpConsumer.promote();
    if( ! pConsumer.get() ){
        return MFALSE;
    }
    pConsumer->querySelector()->queryCollectImageStreamInfo( rawInputInfos, MTRUE );
    for(size_t i = 0; i < rawInputInfos.size() ; i++){
        if( rawInputInfos[i]->getStreamId() == eSTREAMID_IMAGE_PIPE_RAW_LCSO ){
            pipelineConfig.LPBConfigParams.enableLCS = MTRUE;
        }
    }

    HwInfoHelper helper(getOpenId());
    if( ! helper.updateInfos() ) {
        MY_LOGE("cannot properly update infos");
        return MFALSE;
    }

    if (helper.getDualPDAFSupported(mu4Scenario)){
        pipelineConfig.LPBConfigParams.enableDualPD = MTRUE;
    }

    MUINT32 pipeBit;
    MINT ImgFmt = eImgFmt_BAYER12;
    if( helper.getLpModeSupportBitDepthFormat(ImgFmt, pipeBit) ){
        using namespace NSCam::NSIoPipe::NSCamIOPipe;
        if(pipeBit & CAM_Pipeline_14BITS){
            pipelineConfig.LPBConfigParams.pipeBit = CAM_Pipeline_14BITS;
        }
        else{
            pipelineConfig.LPBConfigParams.pipeBit = CAM_Pipeline_12BITS;
        }
    }

    // provide pCallbackListener
    pipelineConfig.pCallbackListener = this;

    // provide sp<IImageStreamInfo>
    // fullraw
    if( mpInfo_FullRaw.get() ){
        pipelineConfig.pInfo_FullRaw = mpInfo_FullRaw;
    }
    // resized raw
    if( mpInfo_ResizedRaw.get() ){
        pipelineConfig.pInfo_ResizedRaw = mpInfo_ResizedRaw;
    }
    // lcso
    if( mpInfo_LcsoRaw.get() ){
        pipelineConfig.pInfo_LcsoRaw = mpInfo_LcsoRaw;
    }
    // jpeg main yuv
    if( mpInfo_Yuv.get() ){
        pipelineConfig.pInfo_Yuv = mpInfo_Yuv;
    }
    // postview yuv
    if( mpInfo_YuvPostview.get() ){
        pipelineConfig.pInfo_YuvPostview = mpInfo_YuvPostview;
    }
    // jpeg thumbnail yuv
    if( mpInfo_YuvThumbnail.get() ){
        pipelineConfig.pInfo_YuvThumbnail = mpInfo_YuvThumbnail;
    }
    // jpeg
    if( mpInfo_Jpeg.get() ){
        pipelineConfig.pInfo_Jpeg = mpInfo_Jpeg;
    }

    // creat and query common capture pipeline
    mpPipeline = ICommonCapturePipeline::queryCommonCapturePipeline(pipelineConfig);
    CHECK_OBJECT(mpPipeline);

    FUNC_END;
    return MTRUE;
}

#else

MBOOL
DCMFShot::
createPipeline()
{
    CAM_TRACE_CALL();
    FUNCTION_SCOPE_I;

    if( !mbZsdFlow ){
        MY_LOGE("non-zsd flow not supported!");
        return MFALSE;
    }

    LegacyPipelineBuilder::ConfigParams LPBConfigParams;
    LPBConfigParams.mode = getLegacyPipelineMode();
    LPBConfigParams.enableEIS = MFALSE;
    LPBConfigParams.enableLCS = mShotParam.mbEnableLtm;
    LPBConfigParams.pluginUser = plugin::CALLER_SHOT_SMART;
    LPBConfigParams.dualcamMode = StereoSettingProvider::getStereoFeatureMode();

    Vector<sp<IImageStreamInfo>> rawInputInfos;
    sp<StreamBufferProvider> pConsumer = mpConsumer.promote();
    if( !pConsumer.get() ) return UNKNOWN_ERROR;
    pConsumer->querySelector()->queryCollectImageStreamInfo( rawInputInfos, MTRUE );
    for(size_t i = 0; i < rawInputInfos.size() ; i++)
    {
        if(rawInputInfos[i]->getStreamId() == eSTREAMID_IMAGE_PIPE_RAW_LCSO)
            LPBConfigParams.enableLCS = MTRUE;
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
                                            "DCMFShot",
                                            LPBConfigParams);
    CHECK_OBJECT(pBuilder);

    mpImageCallback = new ImageCallback(this, 0);
    CHECK_OBJECT(mpImageCallback);

    sp<BufferCallbackHandler> pCallbackHandler = new BufferCallbackHandler(getOpenId());
    CHECK_OBJECT(pCallbackHandler);
    pCallbackHandler->setImageCallback(mpImageCallback);
    mpCallbackHandler = pCallbackHandler; // for src buffers, use the same cb handler

    {
        Vector<PipelineImageParam> vImgSrcParams;
        if(!prepareImgBufProvider_Src(vImgSrcParams, pCallbackHandler)){
            MY_LOGE("prepare src buffer failed");
            return MFALSE;
        }

        if( OK != pBuilder->setSrc(vImgSrcParams) ) {
            MY_LOGE("setSrc failed");
            return MFALSE;

        }
    }

    {
        // dst buffer providers are temporaily created at init stage, will be replace later
        Vector<PipelineImageParam> vImgDstParams;
        if(!prepareImgBufProvider_Dst(vImgDstParams, pCallbackHandler, MTRUE)){
            MY_LOGE("prepare dst buffer failed");
            return MFALSE;
        }

        if( OK != pBuilder->setDst(vImgDstParams) ) {
            MY_LOGE("setDst failed");
            return MFALSE;
        }
    }

    mpPipeline = pBuilder->create();
    return MTRUE;
}

#endif

/******************************************************************************
*
*******************************************************************************/
MVOID
DCMFShot::
endCapture()
{
    FUNCTION_SCOPE_D;
    if ( mbZsdFlow) {
        if (mSelectorUsage.load () != ISelector::SelectorType_ZsdSelector){
            // change to ZSD selector
            mpConsumer = IResourceContainer::getInstance(getOpenId())->queryConsumer( eSTREAMID_IMAGE_PIPE_RAW_OPAQUE );
            sp<StreamBufferProvider> pConsumer = mpConsumer.promote();
            if( pConsumer != NULL) {
                if(mspOriSelector != nullptr) {
                    MY_LOGD("restore original selector");
                    pConsumer->setSelector(mspOriSelector);
                    mspOriSelector = nullptr;
                }else{
                    MY_LOGD("create new selector");
                    sp<ZsdSelector> pSelector = new ZsdSelector();
                    pConsumer->switchSelector(pSelector);
                }
            }
        }
    }
    else {
        MY_LOGE("non-zsd mode not support, should not happened!");
    }
}

/*******************************************************************************
*
********************************************************************************/
MBOOL
DCMFShot::
constructCapturePipeline()
{
    CAM_TRACE_CALL();

    //
    MY_LOGD_IF(mDebugLog, "zsd raw stream %#" PRIx64 ", (%s) size(%dx%d), fmt 0x%x",
            mpInfo_FullRaw->getStreamId(),
            mpInfo_FullRaw->getStreamName(),
            mpInfo_FullRaw->getImgSize().w,
            mpInfo_FullRaw->getImgSize().h,
            mpInfo_FullRaw->getImgFormat()
           );
    // create new pipeline
    if ( ! createPipeline() ) {
        MY_LOGE("createPipeline failed");
        return MFALSE;
    }
    //
    auto pPipeline = mpPipeline;
    CHECK_OBJECT(pPipeline);
    //
#if CCP_NOT_SUPPORT
    if( !pPipeline.get() )
    {
        MY_LOGW("get pPipeline fail");
        return MFALSE;
    }
    sp<ResultProcessor> pResultProcessor = pPipeline->getResultProcessor().promote();
    CHECK_OBJECT(pResultProcessor);

    // metadata
    mpMetadataListener = new MetadataListener(this);
    pResultProcessor->registerListener( std::numeric_limits<MUINT32>::min(), std::numeric_limits<MUINT32>::max(), true, mpMetadataListener);

    mpMetadataListenerFull = new MetadataListener(this);
    pResultProcessor->registerListener( std::numeric_limits<MUINT32>::min(), std::numeric_limits<MUINT32>::max(), false, mpMetadataListenerFull);

#endif
    return MTRUE;
}

/******************************************************************************
*
*******************************************************************************/
status_t
DCMFShot::
getSelectorData(
    IMetadata& rAppSetting,
    IMetadata& rHalSetting,
    android::sp<IImageBuffer>& pBuffer
)
{
    CAM_TRACE_CALL();
    //
    sp<StreamBufferProvider> pConsumer = mpConsumer.promote();
    if(pConsumer == NULL) {
        MY_LOGE("mpConsumer is NULL!");
        return UNKNOWN_ERROR;
    }
    //
    sp< ISelector > pSelector = pConsumer->querySelector();
    if(pSelector == NULL) {
        MY_LOGE("can't find Selector in Consumer");
        return UNKNOWN_ERROR;
    }
    //
    auto pPipeline = mpPipeline;
    if(pPipeline == NULL) {
        MY_LOGE("pPipeline is nullptr");
        return UNKNOWN_ERROR;
    }
    //
    status_t status = OK;
    MINT32 rRequestNo;
    Vector<ISelector::MetaItemSet> rvResultMeta;
    Vector<ISelector::BufferItemSet> rvBufferSet;
    pSelector->sendCommand(ISelector::eCmd_setAllBuffersTransferring, MTRUE, 0, 0);
    status = pSelector->getResult(rRequestNo, rvResultMeta, rvBufferSet);
    pSelector->sendCommand(ISelector::eCmd_setAllBuffersTransferring, MFALSE, 0, 0);

    // check
    MBOOL foundFullRaw = MFALSE;
    MBOOL foundResizedRaw = MFALSE;
    for ( size_t i = 0; i < rvBufferSet.size(); ++i ) {
        if (rvBufferSet[i].id == eSTREAMID_IMAGE_PIPE_RAW_OPAQUE) {
            foundFullRaw = MTRUE;
        }
        if (rvBufferSet[i].id == eSTREAMID_IMAGE_PIPE_RAW_RESIZER) {
            foundResizedRaw = MTRUE;
        }
        if ( foundFullRaw && foundResizedRaw ) {
            MY_LOGD_IF(mDebugLog, "found full raw and resized raw");
            break;
        }
    }

    if ( !foundFullRaw || !foundResizedRaw || rvBufferSet.isEmpty() ) {
        MY_LOGE("Selector get input raw buffer failed! bufferSetSize(%zu). foundFullRaw(%d), foundResizedRaw(%d)",
                rvBufferSet.size(), foundFullRaw, foundResizedRaw);
        return UNKNOWN_ERROR;
    }

    if(status != OK) {
        MY_LOGE("Selector getResult Fail!");
        return UNKNOWN_ERROR;
    }

    if(rvResultMeta.size() < 2) {
        MY_LOGE("ZsdSelect getResult rvResultMeta(%zu)", rvResultMeta.size());
        return UNKNOWN_ERROR;
    }

    // get app & hal metadata
    if(    rvResultMeta.editItemAt(0).id == eSTREAMID_META_APP_DYNAMIC_P1
        && rvResultMeta.editItemAt(1).id == eSTREAMID_META_HAL_DYNAMIC_P1 )
    {
        rAppSetting  = rvResultMeta.editItemAt(0).meta;
        rHalSetting += rvResultMeta.editItemAt(1).meta;
    }
    else
    if(    rvResultMeta.editItemAt(1).id == eSTREAMID_META_APP_DYNAMIC_P1
        && rvResultMeta.editItemAt(0).id == eSTREAMID_META_HAL_DYNAMIC_P1 )
    {
        rAppSetting  = rvResultMeta.editItemAt(1).meta;
        rHalSetting += rvResultMeta.editItemAt(0).meta;
    }
    else {
        MY_LOGE("Something wrong for selector metadata.");
        return UNKNOWN_ERROR;
    }

    for ( size_t i = 0; i < rvBufferSet.size(); ++i ) {
        pBuffer = rvBufferSet[i].heap->createImageBuffer();
        //
        if(pBuffer == NULL) {
            MY_LOGE("get buffer is NULL!");
            return UNKNOWN_ERROR;
        }

       //update p1 buffer to pipeline pool
#if 0
        sp<CallbackBufferPool> pPool = pPipeline->queryBufferPool( rvBufferSet[i].id );
#else
        sp<CallbackBufferPool> pPool = mpCallbackHandler->queryBufferPool( rvBufferSet[i].id );
#endif
        if( pPool == NULL) {
            MY_LOGE("query Pool Fail!");
            return UNKNOWN_ERROR;
        }
        pPool->addBuffer(pBuffer);
    }
    //
    {
        Mutex::Autolock _l(mResultMetadataSetLock);

        IMetadata resultAppMetadata;
        IMetadata resultHalMetadata;
        mResultMetadataSetMap.add(
                static_cast<MUINT32>(mCapReqNo),
                ResultSet_T{static_cast<MUINT32>(mCapReqNo), resultAppMetadata, resultHalMetadata, rAppSetting, rvBufferSet, NONE_CB_DONE }
                );
    }
    return OK;
}

/******************************************************************************
*
*******************************************************************************/
MBOOL
DCMFShot::
getTimestamp(
    IMetadata* metadata,
    MINT64 &timestamp
){
    if(metadata == nullptr){
        MY_LOGW("metadata is nullptr, set to 0");
        timestamp = 0;
        return MFALSE;
    }

    if(tryGetMetadata<MINT64>(metadata, MTK_SENSOR_TIMESTAMP, timestamp)){
        MY_LOGD("timestamp_P1:%" PRId64 "", timestamp);
        return MTRUE;
    }
    else{
        MY_LOGW("Can't get timestamp from metadata, set to 0");
        timestamp = 0;
        return MFALSE;
    }
}

/******************************************************************************
*
*******************************************************************************/
MERROR
DCMFShot::
submitCaptureRequest(
    __attribute__((unused)) MBOOL mainFrame,
    IMetadata appSetting,
    IMetadata halSetting,
    const Vector<PipelineImageParam>& vImageParam
)
{
    CAM_TRACE_CALL();
    ILegacyPipeline::ResultSet resultSet;

    {
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

    // assign fov cropping to isp pipe
    if( mFovCrop.s.w != 0 || mFovCrop.s.h != 0){
        IMetadata::IEntry entry(MTK_STEREO_FEATURE_FOV_CROP_REGION);
        entry.push_back(mFovCrop.p.x, Type2Type<MINT32>());
        entry.push_back(mFovCrop.p.y, Type2Type<MINT32>());
        entry.push_back(mFovCrop.s.w, Type2Type<MINT32>());
        entry.push_back(mFovCrop.s.h, Type2Type<MINT32>());
        halSetting.update(entry.tag(), entry);
    }

    if( isPureZsd() ){
        // since buffer and meta are acquire from zsd selector, we have to update speific hal control which
        // are not exsist in preview pipeine
        updateVendorFeatureTag(&halSetting);
    }

    //
    BufferList vDstStreams;
    Vector<sp<IImageStreamInfo>> rawInputInfos;
    sp<StreamBufferProvider> pConsumer = mpConsumer.promote();
    if( !pConsumer.get() ){
        return UNKNOWN_ERROR;
    }
    // src buffer
    pConsumer->querySelector()->queryCollectImageStreamInfo( rawInputInfos, MTRUE );
    for( size_t i = 0; i < rawInputInfos.size() ; i++) {
        PREPARE_STREAM(vDstStreams, rawInputInfos[i]->getStreamId(),  false);
    }
    // dst buffer, critical, new buffer providers are provided for each capture request
    for( size_t i = 0; i < vImageParam.size() ; i++) {
        PREPARE_STREAM(vDstStreams, vImageParam[i].pInfo->getStreamId(),  true, vImageParam[i].pProvider);
    }

    if( OK != pPipeline->submitRequest(
                mCapReqNo,
                appSetting,
                halSetting,
                vDstStreams,
                &resultSet
                )
      ){
        MY_LOGE("submitRequest failed");
        return UNKNOWN_ERROR;
    }
    return OK;
}

/******************************************************************************
*
*******************************************************************************/
MBOOL
DCMFShot::
isMaster() const
{
    MBOOL ret = MFALSE;
    if(StereoSettingProvider::StereoSettingProvider::isDualCamMode()){
        if(getOpenId() == mMain1OpenId){
            ret = MTRUE;
        }
    }else{
        MY_LOGE("NOT in dual cam mode?");
    }
    return ret;
}

/******************************************************************************
*
*******************************************************************************/
MVOID
DCMFShot::
beforeCapture(Vector<PipelineImageParam>& rvImageParam)
{
    FUNCTION_SCOPE_D;

    if( isMaster() ){
        enterHighPerf();
    }

    if( !isPureZsd() ){
        MY_LOGD("no need to lock AE in pureZSD");
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

// #if MTKCAM_HAVE_COMMON_CAPTURE_PIPELINE_SUPPORT
#if 0
    // set no need return to be true because the buffer will be returned manually by DCMFShot itself
    for(auto &e : rvImageParam){

        sp<CallbackBufferPool> pPool = mpPipeline->queryBufferPool( e.pInfo->getStreamId() );

        pPool->setForceNoNeedReturnBuffer(MTRUE);

        mVCurrentUsingDst.insert(e.pInfo->getStreamId());
    }
#endif

    return;
}
/******************************************************************************
*
*******************************************************************************/
MVOID
DCMFShot::
afterCapture()
{
    FUNCTION_SCOPE_D;

    if( !isPureZsd() ){
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

// #if MTKCAM_HAVE_COMMON_CAPTURE_PIPELINE_SUPPORT
#if 0
    // set no need return to be true because the buffer will be returned manually by DCMFShot
    for(auto &e : mVCurrentUsingDst){

        sp<CallbackBufferPool> pPool = mpPipeline->queryBufferPool( e );

        pPool->setForceNoNeedReturnBuffer(MFALSE);
    }

    mVCurrentUsingDst.clear();
#endif

    return;
}

/******************************************************************************
*
*******************************************************************************/
MSize
DCMFShot::
getRRZOSize() const
{
    MSize ret = MSize(0,0);
#if 0
    bool bRet = StereoSizeProvider::getInstance()->getcustomYUVSize(
            (getOpenId() == mMain1OpenId) ? (StereoHAL::eSTEREO_SENSOR_MAIN1):(StereoHAL::eSTEREO_SENSOR_MAIN2),
            NSImageio::NSIspio::EPortIndex_RRZO,
            ret
        );
#else
    MRect imageCrop = MRect(0,0);
    size_t stride = 0;

    bool bRet = StereoSizeProvider::getInstance()->getPass1Size(
            (getOpenId() == mMain1OpenId) ? (StereoHAL::eSTEREO_SENSOR_MAIN1):(StereoHAL::eSTEREO_SENSOR_MAIN2),
            (EImageFormat)eImgFmt_FG_BAYER10,
            NSImageio::NSIspio::EPortIndex_RRZO,
            (StereoHAL::ENUM_STEREO_SCENARIO)StereoHAL::eSTEREO_SCENARIO_CAPTURE,
            (MRect&)imageCrop,
            (MSize&)ret,
            (MUINT32&)stride
        );
#endif

    if(bRet){
        MY_LOGD("getcustomYUVSize - RRZOYUV (%dx%d)", ret.w, ret.h);
    }else{
        MY_LOGE("getcustomYUVSize - RRZOYUV Failed!");
    }

    return ret;
}

/******************************************************************************
*
*******************************************************************************/
MSize
DCMFShot::
getIMGOSize()
{
    MSize ret = MSize(0,0);
    MSize sensorSize = MSize(0,0);
    Pass2SizeInfo sizeInfo;
    bool bRet = true;

    NSCamHW::HwInfoHelper helper(getOpenId());
    if( !helper.updateInfos() ) {
        MY_LOGE("cannot properly update infos!");
        return ret;
    }
    if( !helper.getSensorSize( SENSOR_SCENARIO_ID_NORMAL_CAPTURE, sensorSize) ) {
        MY_LOGE("cannot get params about sensor!");
        return ret;
    }

    if( mbDumpCali ){
        MY_LOGD("calibration mode, get sensor fullsize - IMGOYUV (%dx%d)", sensorSize.w, sensorSize.h);
        return sensorSize;
    }else{
        if( isMaster() ){
            bRet = StereoSizeProvider::getInstance()->getPass2SizeInfo(
                PASS2A_CROP,
                eSTEREO_SCENARIO_CAPTURE,
                sizeInfo
            );

            if(bRet){
                ret.w = mFovCrop.s.w = sizeInfo.areaWDMA.size.w - sizeInfo.areaWDMA.padding.w;
                ret.h = mFovCrop.s.h = sizeInfo.areaWDMA.size.h - sizeInfo.areaWDMA.padding.h;
                MY_LOGD("original(size:%dx%d, pad:%dx%d) => mFovCrop(%dx%d)",
                    sizeInfo.areaWDMA.size.w, sizeInfo.areaWDMA.size.h, sizeInfo.areaWDMA.padding.w, sizeInfo.areaWDMA.padding.h,
                    mFovCrop.s.w, mFovCrop.s.h
                );

                // 16-alignment
                // Warning: turnkey MFLL algorithm need 16-aligned yuv as input, do not change to false with MFLL combined
                static bool have16Align = true;
                if( have16Align ){
                    ret.w = mFovCrop.s.w = ((MINT32)ret.w/16)*16;
                    ret.h = mFovCrop.s.h = ((MINT32)ret.h/16)*16;
                    MY_LOGD("mFovCrop(%dx%d) 16-aligned", mFovCrop.s.w, mFovCrop.s.h);
                }

                // get start point
                static bool haveCenterShift = true;
                if( haveCenterShift ){
                    mFovCrop.p.x = (sizeInfo.areaWDMA.size.w - mFovCrop.s.w)/2;
                    mFovCrop.p.y = (sizeInfo.areaWDMA.size.h - mFovCrop.s.h)/2;
                    // store mFOVOffset in active array domain
                    mFOVOffset.w = (mFovCrop.p.x - sizeInfo.areaWDMA.startPt.x)*(float)mActiveArray.s.w/sensorSize.w;
                    mFOVOffset.h = (mFovCrop.p.y - sizeInfo.areaWDMA.startPt.y)*(float)mActiveArray.s.h/sensorSize.h;
                }else{
                    mFovCrop.p.x = sizeInfo.areaWDMA.startPt.x;
                    mFovCrop.p.y = sizeInfo.areaWDMA.startPt.y;
                }
                MY_LOGD("mFovCrop(%d,%d,%dx%d) center-shifted(%d)",
                        mFovCrop.p.x, mFovCrop.p.y, mFovCrop.s.w, mFovCrop.s.h, haveCenterShift);

                MY_LOGD("getPass2SizeInfo - IMGOYUV (%d,%d)", ret.w, ret.h);
            }else{
                MY_LOGE("getPass2SizeInfo - IMGOYUV Failed!");
            }
        }else{
            bRet = StereoSizeProvider::getInstance()->getcustomYUVSize(
                (getOpenId() == mMain1OpenId) ? (StereoHAL::eSTEREO_SENSOR_MAIN1):(StereoHAL::eSTEREO_SENSOR_MAIN2),
                NSImageio::NSIspio::EPortIndex_IMGO,
                ret
            );

            if(bRet){
                MY_LOGD("getcustomYUVSize - IMGOYUV (%dx%d)", ret.w, ret.h);
            }else{
                MY_LOGE("getcustomYUVSize - IMGOYUV Failed!");
            }
        }
    }
    return ret;
}

/******************************************************************************
*
*******************************************************************************/
MSize
DCMFShot::
getSMSize()
{
    MSize ret = getIMGOSize();

    ret.w = ret.w/4;
    ret.h = ret.h/4;

    MY_LOGD("getSMSize - SM YUV (%dx%d)", ret.w, ret.h);

    return ret;
}

/******************************************************************************
*
*******************************************************************************/
MVOID
DCMFShot::
updateAppMeta(
    const IMetadata* const appMeta_src,
    const IMetadata* const halMeta_src,
    IMetadata* const appMeta_dst) const
{
    MINT32 dofLevel       = -1;
    MUINT8 afStatus       = -1;
    MINT32 afType         = 0;
    MINT32 afROINum       = 0;
    MINT32 afTopLeftX     = 0;
    MINT32 afTopLeftY     = 0;
    MINT32 afBottomRightX = 0;
    MINT32 afBottomRightY = 0;
    MRect  prv_3a_crop    = MRect(0,0);
    MINT32 afTopLeftX_new = 0;
    MINT32 afTopLeftY_new = 0;
    MINT32 afBottomRightX_new = 0;
    MINT32 afBottomRightY_new = 0;
    MSize  dstBufferSize = mpInfo_Yuv->getImgSize();


    // dof/Bokeh level
    {
        sp<IParamsManager> pParamsMgr = mShotParam.mpParamsMgr.promote();
        if ( pParamsMgr.get() == nullptr ) {
            MY_LOGW("cannot promote IParamsManager, use -1 as default dof level");
        }else{
            dofLevel = pParamsMgr->getInt(MtkCameraParameters::KEY_STEREO_DOF_LEVEL);
        }
        trySetMetadata<MINT32>((*appMeta_dst), MTK_STEREO_FEATURE_DOF_LEVEL, dofLevel);
    }

    // AF status
    {
        IMetadata::IEntry entry = appMeta_src->entryFor(MTK_CONTROL_AF_STATE);
        if(!entry.isEmpty()){
            afStatus = entry.itemAt(0, Type2Type<MUINT8>());
            appMeta_dst->update(entry.tag(), entry);
        }else{
            MY_LOGW("Can't get afStatus from metadata, set to 0");
            trySetMetadata<MUINT8>((*appMeta_dst), MTK_CONTROL_AF_STATE, 0);
        }
    }

    // AF ROI
    {
        IMetadata::IEntry entry = appMeta_src->entryFor(MTK_3A_FEATURE_AF_ROI);
        if( !entry.isEmpty() ){
            afType           = entry.itemAt(0, Type2Type<MINT32>());
            afROINum         = entry.itemAt(1, Type2Type<MINT32>());

            if( afROINum == 1 || afROINum == 2){
                // dont ask me why, as AF team...
                MINT32 afValStart = (afROINum == 1) ? 2 : 7;
                afTopLeftX        = entry.itemAt(afValStart,   Type2Type<MINT32>());
                afTopLeftY        = entry.itemAt(afValStart+1, Type2Type<MINT32>());
                afBottomRightX    = entry.itemAt(afValStart+2, Type2Type<MINT32>());
                afBottomRightY    = entry.itemAt(afValStart+3, Type2Type<MINT32>());
            }else{
                MY_LOGW("af roi(%d)?", afROINum);
                afTopLeftX = afTopLeftY = afBottomRightX = afBottomRightY = 0;
            }

            appMeta_dst->update(entry.tag(), entry);

            // active region transform
            if( mpInfo_Yuv != nullptr){
                if( tryGetMetadata<MRect>(halMeta_src, MTK_3A_PRV_CROP_REGION, prv_3a_crop) ){
                    MY_LOGD("MTK_3A_PRV_CROP_REGION(%d,%d,%dx%d)", prv_3a_crop.p.x, prv_3a_crop.p.y, prv_3a_crop.s.w, prv_3a_crop.s.h);
                }
                else{
                    MY_LOGW("Can't get MTK_3A_PRV_CROP_REGION from metadata");
                    prv_3a_crop = mActiveArray;
                }

                MINT32 TLX_in_prv_crop = afTopLeftX - prv_3a_crop.p.x - mFOVOffset.w;
                MINT32 TLY_in_prv_crop = afTopLeftY - prv_3a_crop.p.y - mFOVOffset.h;
                MINT32 BRX_in_prv_crop = afBottomRightX - prv_3a_crop.p.x - mFOVOffset.w;
                MINT32 BRY_in_prv_crop = afBottomRightY - prv_3a_crop.p.y - mFOVOffset.h;

                afTopLeftX_new     = TLX_in_prv_crop*(float)dstBufferSize.w/prv_3a_crop.s.w;
                afTopLeftY_new     = TLY_in_prv_crop*(float)dstBufferSize.h/prv_3a_crop.s.h;
                afBottomRightX_new = BRX_in_prv_crop*(float)dstBufferSize.w/prv_3a_crop.s.w;
                afBottomRightY_new = BRY_in_prv_crop*(float)dstBufferSize.h/prv_3a_crop.s.h;

                auto boundaryCheck = [](MINT32& val, MINT32 min, MINT32 max){
                    val = std::max(val, min);
                    val = std::min(val, max);
                };
                boundaryCheck(afTopLeftX_new,     0, dstBufferSize.w - 1);
                boundaryCheck(afTopLeftY_new,     0, dstBufferSize.h - 1);
                boundaryCheck(afBottomRightX_new, 0, dstBufferSize.w - 1);
                boundaryCheck(afBottomRightY_new, 0, dstBufferSize.h - 1);

                IMetadata::IEntry entryNew(MTK_3A_FEATURE_AF_ROI);
                entryNew.push_back(afType, Type2Type<MINT32>());
                entryNew.push_back(afROINum, Type2Type<MINT32>());
                entryNew.push_back(afTopLeftX_new, Type2Type<MINT32>());
                entryNew.push_back(afTopLeftY_new, Type2Type<MINT32>());
                entryNew.push_back(afBottomRightX_new, Type2Type<MINT32>());
                entryNew.push_back(afBottomRightY_new, Type2Type<MINT32>());

                appMeta_dst->update(entryNew.tag(), entryNew);
            }
        }else{
            MY_LOGW("Get AF roi fail, set to 0");
            IMetadata::IEntry entry(MTK_3A_FEATURE_AF_ROI);
            for(int i=0 ; i < 4 ; i ++){
                entry.push_back(0, Type2Type< MINT32 >());
            }
            appMeta_dst->update(entry.tag(), entry);
        }
    }
#if 0 // no portrait light support in internel branch
    // portrait light
    {
        MINT32 portraitLightMode = 0;
        sp<IParamsManager> pParamsMgr = mShotParam.mpParamsMgr.promote();
        if ( pParamsMgr.get() == nullptr ) {
            MY_LOGW("cannot promote IParamsManager, use 0 as default portrait light mode");
        }else{
            portraitLightMode = pParamsMgr->getInt(MtkCameraParameters::KEY_STEREO_PORTRAIT_LIGHT_MODE);
        }

        MY_LOGD("portraitLightMode(%d)", portraitLightMode);
        IMetadata::IEntry entry(MTK_STEREO_PORTRAIT_LIGHT_MODE);
        entry.push_back(portraitLightMode, Type2Type< MINT32 >());
        appMeta_dst->update(entry.tag(), entry);
    }
#endif
    // stereo warning
    {
        IMetadata::IEntry entry(MTK_STEREO_FEATURE_WARNING);
        entry.push_back(mStereoWarning, Type2Type< MINT32 >());
        appMeta_dst->update(entry.tag(), entry);
    }

    MY_LOGD("dofUI(%d), AF status(%d) TopLeft(%d,%d) BotRight(%d,%d), mFOVOffset(%dx%d) prv_3a_crop(%d,%d,%dx%d) AF new TopLeft(%d,%d) BotRight(%d,%d),",
        dofLevel,
        afStatus, afTopLeftX, afTopLeftY, afBottomRightX, afBottomRightY,
        mFOVOffset.w, mFOVOffset.h,
        prv_3a_crop.p.x, prv_3a_crop.p.y, prv_3a_crop.s.w, prv_3a_crop.s.h,
        afTopLeftX_new, afTopLeftY_new, afBottomRightX_new, afBottomRightY_new
    );
}

/******************************************************************************
*
*******************************************************************************/
MBOOL
DCMFShot::
supportForegroundPostview(const IMetadata* const appMeta) const
{
    MBOOL ret = MTRUE;

    MINT32 portraitLightMode = 0;
#if 0 // no portrait light support in internel branch
    IMetadata::IEntry entry = appMeta->entryFor(MTK_STEREO_PORTRAIT_LIGHT_MODE);
    if(!entry.isEmpty()){
        portraitLightMode = entry.itemAt(0, Type2Type<MINT32>());

        // portrait light does not use foreground postview because the after effect is too strong
        ret = ( portraitLightMode > 0 ) ? MFALSE : MTRUE;
    }
#endif
    ret = ::property_get_int32(FG_POSTVIEW_KEY, ret);

    MY_LOGD("portraitLightMode(%d) ret(%d)", portraitLightMode, ret);
    return ret;
}

/******************************************************************************
*
*******************************************************************************/
MVOID
DCMFShot::
enterHighPerf()
{
    MY_LOGD("");

    auto scenarioCtrlEnter = [this] () {
        sp<IScenarioControl> pScenarioCtrl = IScenarioControl::create(mMain1OpenId);
        if( pScenarioCtrl.get() == nullptr ){
            MY_LOGE("get Scenario Control fail");
            return MFALSE;
        }
        else{
            pScenarioCtrl->enterScenario(IScenarioControl::Scenario_ContinuousShot);
        }
        return MTRUE;
    };

    if( mFuture.valid() ){
        MY_LOGD("wait future job +");
        mFuture.get();
        MY_LOGD("wait future job -");
    }

    mFuture = std::async(std::launch::async, scenarioCtrlEnter);
}

/******************************************************************************
*
*******************************************************************************/
MVOID
DCMFShot::
exitHighPerf()
{
    MY_LOGD("");

    auto scenarioCtrlExit = [this] () {
        sp<IFeatureFlowControl> pFeatureFlowControl = IResourceContainer::getInstance(mMain1OpenId)->queryFeatureFlowControl();
        if( pFeatureFlowControl == nullptr ){
            MY_LOGE("IFeatureFlowControl is NULL!");
            return MFALSE;
        }else{
            pFeatureFlowControl->changeToPreviewStatus();
        }
        return MTRUE;
    };

    if( mFuture.valid() ){
        MY_LOGD("wait future job +");
        mFuture.get();
        MY_LOGD("wait future job -");
    }

    mFuture = std::async(std::launch::async, scenarioCtrlExit);
}

/******************************************************************************
*
*******************************************************************************/
MVOID
DCMFShot::
updateVendorFeatureTag(plugin::InputInfo& inputInfo) const
{
    updateVendorFeatureTag(&inputInfo.halCtrl);
}

/******************************************************************************
*
*******************************************************************************/
MVOID
DCMFShot::
updateVendorFeatureTag(IMetadata* const halMeta) const
{
    MINT32 vendorFeatureTag = 0;

    switch(getShotMode()){
        case eShotMode_DCMFShot:
            vendorFeatureTag = MTK_DCMF_FEATURE_BOKEH;
            break;
        case eShotMode_DCMFHdrShot:
            vendorFeatureTag = MTK_DCMF_FEATURE_HDR_BOKEH;
            break;
        default:
            vendorFeatureTag = MTK_DCMF_FEATURE_BOKEH;
    }

    IMetadata::IEntry entry(MTK_STEREO_DCMF_FEATURE_MODE);
    entry.push_back(vendorFeatureTag, Type2Type<MINT32>());
    halMeta->update(entry.tag(), entry);
    MY_LOGD("vendorFeatureTag(%d)", vendorFeatureTag);
}

/******************************************************************************
*
*******************************************************************************/
MVOID
DCMFShot::
collectOneShotInfo()
{
    sp<IFrameInfo> pFrameInfo = IResourceContainer::getInstance(getOpenId())->queryLatestFrameInfo();
    MY_LOGW_IF((pFrameInfo == nullptr), "Can't query Latest FrameInfo!");
    IMetadata metadata;
    if (pFrameInfo.get()){
        pFrameInfo->getFrameMetadata(eSTREAMID_META_APP_DYNAMIC_P2, metadata);
    }

    // get stereo warning from p2 app matadata
    {
        IMetadata::IEntry entry = metadata.entryFor(MTK_STEREO_FEATURE_WARNING);
        if(!entry.isEmpty()) {
            mStereoWarning = entry.itemAt(0, Type2Type<MINT32>());
        }else{
            MY_LOGW("Cannot get stereo warning result");
        }
        MY_LOGD("mStereoWarning(%d)", mStereoWarning);
    }
}

/******************************************************************************
*
*******************************************************************************/
MVOID
DCMFShot::
returnStoredSelectorData()
{
    FUNC_START;
    while( !mvStoredSelectorData.empty() ){
        StoredSelectorData theSet = mvStoredSelectorData.front();
        mvStoredSelectorData.pop_front();

        for ( size_t i = 0; i < theSet.bufferSet.size(); ++i ) {
            if (theSet.bufferSet[i].id == eSTREAMID_IMAGE_PIPE_RAW_OPAQUE) {
                sp<StreamBufferProvider> pProvider = IResourceContainer::getInstance(getOpenId())->queryConsumer( theSet.bufferSet[i].id );
                MY_LOGD("Query Consumer OpenID(%d) StreamID[%" PRId64 "]", getOpenId(), theSet.bufferSet[i].id);
                if( pProvider == nullptr) {
                    MY_LOGE("can't find StreamBufferProvider in ConsumerContainer");
                }else{
                    sp< ISelector > pSelector = pProvider->querySelector();
                    if(pSelector == nullptr) {
                        MY_LOGE("can't find Selector in Consumer");
                    } else {
                        for(size_t j = 0; j < theSet.bufferSet.size() ; j++)
                            pSelector->returnBuffer(theSet.bufferSet.editItemAt(j));
                    }
                }
            }else{
                // rrzo, imgo, lcso are return together with imgo
            }
        }
    }
    FUNC_END;
}

/******************************************************************************
*
*******************************************************************************/
MVOID
DCMFShot::
handleDummyCallbacks()
{
    if( !isMaster() ){
        return;
    }
#if 0
    MINT64  timestamp = 0;
    timestamp = ICallbackClientMgr::generateTimestamp();
    MY_LOGD("handleDummyCallbacks with TS(%lld) +", timestamp);

    // shutter
    mpShotCallback->onCB_Shutter(true,0,getShotMode());

    // p2done
    mpShotCallback->onCB_P2done(
        mbBGService ? 1 : 0 // callback together with MTK_CAMERA_MSG_EXT_NOTIFY_CAPTURE_DONE
    );

    // postview
    // alloc dummy postview
    sp<CallbackBufferPool> pPool = new CallbackBufferPool(mpInfo_YuvPostview);
    pPool->setExtraParam(
        MTRUE // continuous plane
    );
    {
        pPool->allocateBuffer(
                          mpInfo_YuvPostview->getStreamName(),
                          mpInfo_YuvPostview->getMaxBufNum(),
                          mpInfo_YuvPostview->getMinInitBufNum()
                          );
    }

    sp<IImageBufferHeap> pHeap = nullptr;
    MINT32 rRequestNo = 0;
    MUINT32 trans = 0;

    if( pPool->acquireFromPool(LOG_TAG, rRequestNo, pHeap, trans) != OK){
        MY_LOGE("can not acquire DummyPostview from pool");
        return;
    }

    IImageBuffer* pImgBuffer = pHeap->createImageBuffer();
    pImgBuffer->lockBuf(LOG_TAG, eBUFFER_USAGE_SW_READ_MASK|eBUFFER_USAGE_SW_WRITE_MASK);

    // make it black
    for(auto i=0 ; i<pImgBuffer->getPlaneCount() ; ++i){
        memset((uint8_t*)pImgBuffer->getBufVA(i), 0, pImgBuffer->getBufSizeInBytes(i));
    }

    mpShotCallback->onCB_PostviewClient(
        timestamp,
        pImgBuffer,
        0, // never ask for offset
        mbBGService
    );

    pImgBuffer->unlockBuf(LOG_TAG);

    // jpeg
    {
        static MINT32 dummyJpegSize = 512;
        uint8_t* puJpegBuf = new uint8_t[dummyJpegSize];
        MUINT32 u4JpegSize = dummyJpegSize;
        MY_LOGD("dummy (puJpgBuf, jpgSize) = (%p, %d)",
                puJpegBuf, u4JpegSize);

        memset(puJpegBuf, 0, dummyJpegSize);

        // mpShotCallback->onCB_CompressedImage_packed(
        //     timestamp,
        //     u4JpegSize,
        //     puJpegBuf,
        //     0,                       //callback index
        //     true,                    //final image
        //     MTK_CAMERA_MSG_EXT_DATA_COMPRESSED_IMAGE,
        //     getShotMode()
        // );

        ICallbackClientMgr::getInstance()->onCB_CompressedImage_packed(
                                                    timestamp,
                                                    u4JpegSize,
                                                    puJpegBuf,
                                                    0,
                                                    true,
                                                    MTK_CAMERA_MSG_EXT_DATA_COMPRESSED_IMAGE);
        delete[] puJpegBuf;
    }

    MY_LOGD("handleDummyCallbacks with TS(%lld) -", timestamp);
#else
    MY_LOGE("temp handleDummyCallbacks! Suicide: kill DCMFShot - raise(SIGINT)");
    ::raise(SIGINT);
#endif
}

/******************************************************************************
*
*******************************************************************************/
MBOOL
DCMFShot::
getSelectorDataAndStore(MINT32 const shotCount)
{
    MBOOL ret = MTRUE;
    CAM_TRACE_CALL();

    sp<StreamBufferProvider> pConsumer = mpConsumer.promote();
    if(pConsumer == NULL) {
        MY_LOGE("mpConsumer is NULL!");
        return MFALSE;
    }

    sp< ISelector > pSelector = pConsumer->querySelector();
    if(pSelector == NULL) {
        MY_LOGE("can't find Selector in Consumer");
        return MFALSE;
    }

    for(int shot = 0 ; shot < shotCount ; ++shot){
        MY_LOGD("getSelectorDataAndStore (%d/%d)", shot, shotCount);

        status_t status = OK;
        MINT32 rRequestNo;
        Vector<ISelector::MetaItemSet>   rvResultMeta;
        Vector<ISelector::BufferItemSet> rvBufferSet;
        pSelector->sendCommand(ISelector::eCmd_setAllBuffersTransferring, MTRUE, 0, 0);
        status = pSelector->getResult(rRequestNo, rvResultMeta, rvBufferSet);
        pSelector->sendCommand(ISelector::eCmd_setAllBuffersTransferring, MFALSE, 0, 0);

        MBOOL foundFullRaw = MFALSE;
        MBOOL foundResizedRaw = MFALSE;
        for ( size_t i = 0; i < rvBufferSet.size(); ++i ) {
            if (rvBufferSet[i].id == eSTREAMID_IMAGE_PIPE_RAW_OPAQUE) {
                foundFullRaw = MTRUE;
            }
            if (rvBufferSet[i].id == eSTREAMID_IMAGE_PIPE_RAW_RESIZER) {
                foundResizedRaw = MTRUE;
            }
            if ( foundFullRaw && foundResizedRaw ) {
                MY_LOGD_IF(mDebugLog, "found full raw and resized raw");
                break;
            }
        }

        if ( !foundFullRaw || !foundResizedRaw || rvBufferSet.isEmpty() ) {
            MY_LOGE("Selector get input raw buffer failed! bufferSetSize(%zu). foundFullRaw(%d), foundResizedRaw(%d)",
                    rvBufferSet.size(), foundFullRaw, foundResizedRaw);
            return MFALSE;
        }

        if(status != OK) {
            MY_LOGE("Selector getResult Fail!");
            return MFALSE;
        }

        if(rvResultMeta.size() < 2) {
            MY_LOGE("ZsdSelect getResult rvResultMeta(%zu)", rvResultMeta.size());
            return MFALSE;
        }

        StoredSelectorData newSet = {rvResultMeta, rvBufferSet};
        mvStoredSelectorData.push_back(newSet);
        MY_LOGD("mvStoredSelectorData.size(%d)", mvStoredSelectorData.size());
    }

    // debugging
    {
        mDebugTryCount ++;
        static MINT32 forceFailTryCount = property_get_int32("vendor.dcmf.selfailtrycount", -1);
        if(mDebugTryCount <= forceFailTryCount){
            MY_LOGW("debugTry(%d) forceFail(%d), force return failed!", mDebugTryCount, forceFailTryCount);
            return MFALSE;
        }
    }

    return ret;
}

/******************************************************************************
*
*******************************************************************************/
status_t
DCMFShot::
getStoredSelectorData(
    IMetadata& rAppSetting,
    IMetadata& rHalSetting,
    android::sp<IImageBuffer>& pBuffer
){
    status_t ret = OK;

    if( mvStoredSelectorData.empty() ){
        MY_LOGE("mvStoredSelectorData is empty!");
        return UNKNOWN_ERROR;
    }

    StoredSelectorData theSet = mvStoredSelectorData.front();
    mvStoredSelectorData.pop_front();

    // get app & hal metadata
    if(    theSet.metaSet.editItemAt(0).id == eSTREAMID_META_APP_DYNAMIC_P1
        && theSet.metaSet.editItemAt(1).id == eSTREAMID_META_HAL_DYNAMIC_P1 )
    {
        rAppSetting  = theSet.metaSet.editItemAt(0).meta;
        rHalSetting += theSet.metaSet.editItemAt(1).meta;
    }
    else
    if(    theSet.metaSet.editItemAt(1).id == eSTREAMID_META_APP_DYNAMIC_P1
        && theSet.metaSet.editItemAt(0).id == eSTREAMID_META_HAL_DYNAMIC_P1 )
    {
        rAppSetting  = theSet.metaSet.editItemAt(1).meta;
        rHalSetting += theSet.metaSet.editItemAt(0).meta;
    }
    else {
        MY_LOGE("Something wrong for selector metadata.");
        return UNKNOWN_ERROR;
    }

    for ( size_t i = 0; i < theSet.bufferSet.size(); ++i ) {
        pBuffer = theSet.bufferSet[i].heap->createImageBuffer();
        //
        if(pBuffer == nullptr) {
            MY_LOGE("get buffer is NULL!");
            return UNKNOWN_ERROR;
        }

       //update p1 buffer to pipeline pool
        sp<CallbackBufferPool> pPool = mpCallbackHandler->queryBufferPool( theSet.bufferSet[i].id );
        if( pPool == nullptr) {
            MY_LOGE("query Pool Fail!");
            return UNKNOWN_ERROR;
        }
        pPool->addBuffer(pBuffer);
    }
    //
    {
        Mutex::Autolock _l(mResultMetadataSetLock);

        IMetadata resultAppMetadata;
        IMetadata resultHalMetadata;
        mResultMetadataSetMap.add(
                static_cast<MUINT32>(mCapReqNo),
                ResultSet_T{static_cast<MUINT32>(mCapReqNo), resultAppMetadata, resultHalMetadata, rAppSetting, theSet.bufferSet, NONE_CB_DONE }
                );
    }

    return ret;
}

/******************************************************************************
*
*******************************************************************************/
DCMFShot::DualOperation::
DualOperation()
{
    mOptable[eOp::RETURN_CAP_BUFFERS] = &DCMFShot::returnCapReqBuffers;
}

/******************************************************************************
*
*******************************************************************************/
DCMFShot::DualOperation::
~DualOperation()
{
    CAM_LOGD("dtor");
}

/******************************************************************************
*
*******************************************************************************/
MBOOL
DCMFShot::DualOperation::
registerShotInstance(sp<DCMFShot> pShot)
{
    Mutex::Autolock _l(mLock);
    if(mvShots.count( pShot->getOpenId() ) > 0){
        CAM_LOGW("Already registered? Overwrite!");
    }
    mvShots[pShot->getOpenId()] = pShot;
    return MTRUE;
}

/******************************************************************************
*
*******************************************************************************/
MBOOL
DCMFShot::DualOperation::
unRegisterShotInstance(sp<DCMFShot> pShot)
{
    Mutex::Autolock _l(mLock);
    if(mvShots.count( pShot->getOpenId() ) == 0){
        CAM_LOGW("Not exist?");
    }else{
        mvShots.erase( pShot->getOpenId() );
    }
    return MTRUE;
}

/******************************************************************************
*
*******************************************************************************/
MBOOL
DCMFShot::DualOperation::
perform(DualOperation::eOp opName, MUINT32 arg)
{
    Mutex::Autolock _l(mLock);
    for( auto &e : mvShots ){
        sp<DCMFShot> spShot = e.second.promote();

        if(spShot == nullptr){
            CAM_LOGW("failed to promote shot (%d)", e.first);
            continue;
        }

        if( mOptable.count(opName) == 0){
            CAM_LOGW("no such op (%d)", opName);
            continue;
        }

        Op op = mOptable.at(opName);

        DCMFShot* pShot = spShot.get();

        (pShot->*op)(arg);
    }
    return MTRUE;
}

/*******************************************************************************
*
********************************************************************************/
DCMFHdrSync*
DCMFHdrSync::
getInstance()
{
    static DCMFHdrSync dualHdrSync;
    return &dualHdrSync;
}
/*******************************************************************************
*
********************************************************************************/
DCMFHdrSync::
DCMFHdrSync()
{
}
/*******************************************************************************
*
********************************************************************************/
DCMFHdrSync::
~DCMFHdrSync()
{
}
/*******************************************************************************
*
********************************************************************************/
void
DCMFHdrSync::
notifyMySelectResult(
    int const openId, bool const selectResult
)
{
    Mutex::Autolock _l(mLock);

    CAM_LOGD("openId(%d) SelectResult(%d)", openId, selectResult);
    mSelectResultMap[openId] = selectResult;
}
/*******************************************************************************
*
********************************************************************************/
bool
DCMFHdrSync::
getDualSelectResult(
    int const openId
)
{
    Mutex::Autolock _l(mLock);

    if( mSelectResultMap.size() == 2){
        // check whether both shot have OK select result
        mDualSelectResult = true;
        for(auto &itr:mSelectResultMap){
            if(itr.second == false){
                CAM_LOGW("openId(%d)'s select result is(%d)", itr.first, itr.second);
                mDualSelectResult = false;
            }
        }
        mSelectResultMap.clear();
        mCond.signal();
    }else{
        // wait
        CAM_LOGD("openId(%d) wait +", openId);
        std::chrono::nanoseconds maxWaitTimeNS(std::chrono::seconds(10));
        auto ret = mCond.waitRelative(mLock, maxWaitTimeNS.count());
        if(ret != OK){
            CAM_LOGE("mCond wait timeout!");
            return false;
        }
        CAM_LOGD("openId(%d) wait -", openId);
    }

    CAM_LOGD("openId(%d) dualSelectResult(%d)", openId, mDualSelectResult);
    return mDualSelectResult;
}
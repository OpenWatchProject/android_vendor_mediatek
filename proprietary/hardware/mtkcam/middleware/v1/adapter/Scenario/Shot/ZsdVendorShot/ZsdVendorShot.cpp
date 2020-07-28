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

#define LOG_TAG "MtkCam/ZsdVendorShot"
//
#include <mtkcam/utils/std/Log.h>
#include <mtkcam/def/common.h>
#include <mtkcam/utils/std/Trace.h>
//


//
#include <mtkcam/drv/IHalSensor.h>
#include <mtkcam/drv/iopipe/PostProc/INormalStream.h>
#include <mtkcam/drv/iopipe/SImager/IImageTransform.h>
//
#include <mtkcam/middleware/v1/camshot/ICamShot.h>
#include <mtkcam/middleware/v1/camshot/ISingleShot.h>
#include <mtkcam/middleware/v1/camshot/ISmartShot.h>
//
#include <mtkcam/middleware/v1/IShot.h>
//
#include "ImpShot.h"
#include "ZsdVendorShot.h"
#include <ExifJpegUtils.h>
//
#include <mtkcam/utils/hw/CamManager.h>
using namespace NSCam::Utils;
//
#include <mtkcam/middleware/v1/LegacyPipeline/StreamId.h>
#include <mtkcam/middleware/v1/camshot/_params.h>
//
#include <mtkcam/utils/metadata/client/mtk_metadata_tag.h>
//
#include <mtkcam/middleware/v1/LegacyPipeline/ILegacyPipeline.h>
#include <mtkcam/utils/hw/HwInfoHelper.h>
#include <mtkcam/utils/metadata/hal/mtk_platform_metadata_tag.h>
#include <utils/Vector.h>
#include <sys/stat.h>
//
#include <mtkcam/aaa/IDngInfo.h>
//
#include <mtkcam/middleware/v1/LegacyPipeline/LegacyPipelineUtils.h>
#include <mtkcam/middleware/v1/LegacyPipeline/LegacyPipelineBuilder.h>
#include <mtkcam/middleware/v1/LegacyPipeline/buffer/StreamBufferProviderFactory.h>

#include <mtkcam/aaa/IHal3A.h>
#include <isp_tuning/isp_tuning.h>

// postprocess
#include <mtkcam/middleware/v1/camutils/IDetachJobManager.h>
#include <mtkcam/middleware/v1/camutils/PostProcessJobQueue.h>
#include <mtkcam/feature/effectHalBase/ICallbackClientMgr.h>

using namespace android;
using namespace NSShot;
using namespace ZvShot;
using namespace NSCam::v1;
using namespace NSCam::v1::NSLegacyPipeline;
using namespace NSCamHW;
using namespace NS3Av3;
using namespace NSCam::NSIoPipe;
using namespace NSCam::NSIoPipe::NSPostProc;
using namespace NSIspTuning;
using NSCam::NSIoPipe::EPortType_Memory;
using NSCam::NSIoPipe::Input;
using NSCam::NSIoPipe::Output;
using NSCam::NSIoPipe::QParams;
using NSCam::NSIoPipe::PortID;
using NSCam::NSIoPipe::NSSImager::IImageTransform;

#define CHECK_OBJECT(x)  do{                                        \
    if (x == nullptr) { MY_LOGE("Null %s Object", #x); return MFALSE;} \
} while(0)

#include <cutils/properties.h>
#include <sched.h>

#define DUMP_KEY  "debug.zsdvendorshot.dump"
#define DUMP_PATH "/sdcard/zsdvendorshot"
#define VENDORSHOT_SOURCE_COUNT     (8)
#define FLASH_DUMMY_FRAMES_FRONT    (2)
#define FLASH_DUMMY_FRAMES_BACK     (3)
/******************************************************************************
 *
 ******************************************************************************/
#define MY_LOGV(fmt, arg...)        CAM_LOGV("(%d)(%s)[%s] " fmt, ::gettid(), getShotName(), __FUNCTION__, ##arg)
#define MY_LOGD(fmt, arg...)        CAM_LOGD("(%d)(%s)[%s] " fmt, ::gettid(), getShotName(), __FUNCTION__, ##arg)
#define MY_LOGI(fmt, arg...)        CAM_LOGI("(%d)(%s)[%s] " fmt, ::gettid(), getShotName(), __FUNCTION__, ##arg)
#define MY_LOGW(fmt, arg...)        CAM_LOGW("(%d)(%s)[%s] " fmt, ::gettid(), getShotName(), __FUNCTION__, ##arg)
#define MY_LOGE(fmt, arg...)        CAM_LOGE("(%d)(%s)[%s] " fmt, ::gettid(), getShotName(), __FUNCTION__, ##arg)
#define MY_LOGA(fmt, arg...)        CAM_LOGA("(%d)(%s)[%s] " fmt, ::gettid(), getShotName(), __FUNCTION__, ##arg)
#define MY_LOGF(fmt, arg...)        CAM_LOGF("(%d)(%s)[%s] " fmt, ::gettid(), getShotName(), __FUNCTION__, ##arg)
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


#include    "ZsdVendorShot.SwnrHelper.hpp"

wp<ScenarioCtrl> ZsdVendorShot::m_wpScenarioCtrl = nullptr;

/******************************************************************************
 *
 ******************************************************************************/

static MBOOL enableForceToBigCore()
{
    MBOOL ret;
    pid_t tid = gettid();
    cpu_set_t cpuset;
    int s;
    unsigned int Msk, cpu_no;

    // use PerfServeice to enable 4 big core to highest freq
    int cpu_msk = 0xF0;
    // put current thread to big cores
    CPU_ZERO(&cpuset);
    for (Msk=1, cpu_no=0; Msk<0xFF; Msk<<=1, cpu_no++) {
        if (Msk&cpu_msk) {
            CPU_SET(cpu_no, &cpuset);
        }
    }
    s = sched_setaffinity(tid, sizeof(cpu_set_t), &cpuset);
    if (s != 0) {
        CAM_LOGE("mt_sched_setaffinity error!!");
        ret = MFALSE;
        goto lbExit;
    }
    ret = MTRUE;
lbExit:
    return ret;
}

/******************************************************************************
*
*******************************************************************************/
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

extern
sp<IShot>
createInstance_ZsdVendorShot(
    char const*const    pszShotName,
    uint32_t const      u4ShotMode,
    int32_t const       i4OpenId
)
{
    sp<IShot>  pShot            = NULL;
    sp<ZsdVendorShot>  pImpShot = NULL;
    //
    //  (1.1) new Implementator.
    pImpShot = new ZsdVendorShot(pszShotName, u4ShotMode, i4OpenId);
    if  ( pImpShot == 0 ) {
        CAM_LOGE("[%s] new ZsdVendorShot", __FUNCTION__);
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

/*******************************************************************************
*
********************************************************************************/
static void onFakeJpegCallback(const sp<IShotCallback>& callback)
{
    if (callback == NULL)
    {
        CAM_LOGE("[onFakeJpegCallback] shot callback is null");
        return;
    }

    CAM_LOGW("[onFakeJpegCallback] send dummy jpeg callback");

    const uint32_t jpegSize = 512;
    const uint8_t  jpegBuffer[jpegSize] {};
    const uint32_t exifHeaderSize = 512;
    const uint8_t  exifHeaderBuffer[exifHeaderSize] {};

    callback->onCB_CompressedImage(
            0, jpegSize, jpegBuffer, exifHeaderSize, exifHeaderBuffer, 0, true);
}

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

template<typename T>
inline MINT32 UPDATE_ENTRY(IMetadata& metadata, MINT32 entry_tag, T value)
{
    IMetadata::IEntry entry(entry_tag);
    entry.push_back(value, Type2Type< T >());
    return metadata.update(entry_tag, entry);
}

/******************************************************************************
 *  This function is invoked when this object is firstly created.
 *  All resources can be allocated here.
 ******************************************************************************/
bool
ZsdVendorShot::
onCreate()
{
    bool ret = true;
    return ret;
}


/******************************************************************************
 *  This function is invoked when this object is ready to destryoed in the
 *  destructor. All resources must be released before this returns.
 ******************************************************************************/
void
ZsdVendorShot::
onDestroy()
{

}

/******************************************************************************
 *
 ******************************************************************************/
ZsdVendorShot::
ZsdVendorShot(
    char const*const pszShotName,
    uint32_t const u4ShotMode,
    int32_t const i4OpenId
)
    : ImpShot(pszShotName, u4ShotMode, i4OpenId)
    , mu4Scenario(0)
    , mu4Bitdepth(0)
    //

#if MTKCAM_HAVE_COMMON_CAPTURE_PIPELINE_SUPPORT
    , mpPipeline(NULL)
#else
    , mpPipeline(NULL)
    , mpImageCallback(NULL)
    , mpCallbackHandler(NULL)
    , mpMetadataListener(NULL)
#endif
    //
    , mpJpegPool()
    , mCapReqNo(0)
    , mEncDoneCond()
    , mEncJobLock()
    , mEncJob()
    , mEncJobDone(MFALSE)
    , m_spScenarioCtrl(nullptr)
    , muYuvBufferCount(1)
{
    mDumpFlag = ::property_get_int32(DUMP_KEY, 0);
    if( mDumpFlag ) {
        MY_LOGD("enable dump flag 0x%x", mDumpFlag);
        MINT32 err = mkdir(DUMP_PATH, S_IRWXU | S_IRWXG | S_IRWXO);
        if(err != 0)
        {
            MY_LOGE("mkdir failed");
        }
    }

    mpCpuCtrl = CpuCtrl::createInstance();
    if(mpCpuCtrl)
    {
        mpCpuCtrl->init();
    }
}

/******************************************************************************
 *
 ******************************************************************************/
ZsdVendorShot::
~ZsdVendorShot()
{
    MY_LOGD("~ZsdVendorShot()");
    if( mResultMetadataSetMap.size() > 0 )
    {
        int n = mResultMetadataSetMap.size();
        for(int i=0; i<n; i++)
        {
            MY_LOGW("requestNo(%d) doesn't clear before ZsdVendorShot destroyed",mResultMetadataSetMap.keyAt(i));
            mResultMetadataSetMap.editValueAt(i).selectorGetBufs.clear();
        }
    }
    mResultMetadataSetMap.clear();
}


/******************************************************************************
 *
 ******************************************************************************/
bool
ZsdVendorShot::
sendCommand(
    uint32_t const  cmd,
    MUINTPTR const  arg1,
    uint32_t const  arg2,
    uint32_t const  arg3
)
{
    bool ret = true;
    //
    switch  (cmd)
    {
    //  This command is to reset this class. After captures and then reset,
    //  performing a new capture should work well, no matter whether previous
    //  captures failed or not.
    //
    //  Arguments:
    //          N/A
    case eCmd_reset:
        ret = onCmd_reset();
        break;

    //  This command is to perform capture.
    //
    //  Arguments:
    //          N/A
    case eCmd_capture:
        ret = onCmd_capture();
        break;

    //  This command is to perform cancel capture.
    //
    //  Arguments:
    //          N/A
    case eCmd_cancel:
        onCmd_cancel();
        break;
    //
    default:
        ret = ImpShot::sendCommand(cmd, arg1, arg2, arg3);
    }
    //
    return ret;
}


/******************************************************************************
 *
 ******************************************************************************/
bool
ZsdVendorShot::
onCmd_reset()
{
    bool ret = true;
    return ret;
}

/******************************************************************************
 *
 ******************************************************************************/
bool
ZsdVendorShot::
onCmd_capture()
{
    CAM_TRACE_NAME("Zsd vendor Capture");

    // TODO: decide different shot. Ex. zsh with flash light, HDR..
    MINT32 type;
    decideSettingType(type);

    // prepare necessary setting for capture
    // 1. get sensor size
    // 2. get streamBufferProvider
    // 3. runtime allocate full raw buffer [optional]
    // TODO
    MINT32 runtimeAllocateCount = 0;
    beginCapture( runtimeAllocateCount );
    //
    // decide raw buffer source & count
    // TODO: add different setting here [optional]
    //DRE update
    MINT32 supportDRE = property_get_int32("camera.mdp.dre.enable", 0);
    if(supportDRE != 0)
    {
        MBOOL bEnableCALTM = false;
        IMetadata::setEntry<MBOOL>(&mShotParam.mHalSetting, MTK_P2NODE_CTRL_CALTM_ENABLE, bEnableCALTM);
    }
    // BackgroundService
    if (supportBackgroundService()) {
        /* add a sync token if using BackgroundService */
        IMetadata::setEntry<MINT64>(
                &mShotParam.mHalSetting,
                MTK_PLUGIN_DETACT_JOB_SYNC_TOKEN,
                getInstanceId() );

        /* start capturing, add token to job manager */
        IDetachJobManager::getInstance()->registerToken( getInstanceId(), LOG_TAG );
    }

    //
    MINT32 shotCount = VENDORSHOT_SOURCE_COUNT;
    muYuvBufferCount = shotCount;
    Vector<NS3Av3::CaptureParam_T> vHdrCaptureParams;
    mEncJob.setSourceCnt(shotCount);
    Vector< SettingSet > vSettings;
    {
        MBOOL res = MTRUE;
        //
        switch(type) {
            // zsd + flash light
            case SETTING_FLASH_ACQUIRE:
                {
                    shotCount = getFlashCaptureCount(
                            &mShotParam.mAppSetting,
                            &mShotParam.mHalSetting
                            );
                    MY_LOGD("SETTING_FLASH_ACQUIRE, shotCount=%d", shotCount);
                    mEncJob.setSourceCnt(shotCount);

                    // add front dummy frame for flash stable
                    SettingSet setting_dummy_front;
                    UPDATE_ENTRY(setting_dummy_front.appSetting, MTK_CONTROL_AE_MODE,      (MUINT8)MTK_CONTROL_AE_MODE_OFF);
                    UPDATE_ENTRY(setting_dummy_front.appSetting, MTK_SENSOR_EXPOSURE_TIME, (MINT64)33333333);
                    UPDATE_ENTRY(setting_dummy_front.appSetting, MTK_SENSOR_SENSITIVITY,   (MINT32)1000);
                    for ( int i = 0; i < FLASH_DUMMY_FRAMES_FRONT; i++ ) {
                        vSettings.push_back(setting_dummy_front);
                    }

                    // capture frame setting
                    for (size_t i = 0; i < shotCount; i++) {
                        vSettings.push_back(
                                SettingSet{
                                .appSetting = mShotParam.mAppSetting,
                                .halSetting = mShotParam.mHalSetting
                            }
                        );
                    }

                    // add back dummy frames for preview stable
                    SettingSet setting_dummy_back;
                    for ( int i = 0; i < FLASH_DUMMY_FRAMES_BACK; i++ ) {
                        vSettings.push_back(setting_dummy_back);
                    }

                    // submit to zsd preview pipeline
                    res = applyRawBufferSettings( vSettings, shotCount, FLASH_DUMMY_FRAMES_FRONT );
                }
                break;
            case SETTING_HDR:
                {
                    updateCaptureParams(shotCount, vHdrCaptureParams);
                    mEncJob.setSourceCnt(shotCount);
                    for ( size_t i = 0; i < vHdrCaptureParams.size(); ++i) {
                        IMetadata appSetting = mShotParam.mAppSetting;
                        IMetadata halSetting = mShotParam.mHalSetting;
                        {
                            IMetadata::Memory capParams;
                            capParams.resize(sizeof(NS3Av3::CaptureParam_T));
                            memcpy(capParams.editArray(), &vHdrCaptureParams[i],
                                    sizeof(NS3Av3::CaptureParam_T));

                            IMetadata::IEntry entry(MTK_3A_AE_CAP_PARAM);
                            entry.push_back(capParams, Type2Type< IMetadata::Memory >());
                            halSetting.update(entry.tag(), entry);
                        }
                        {
                            // pause AF for (N - 1) frames and resume for the last frame
                            IMetadata::IEntry entry(MTK_FOCUS_PAUSE);
                            entry.push_back(((i+1)==shotCount) ? 0 : 1, Type2Type< MUINT8 >());
                            halSetting.update(entry.tag(), entry);
                        }
                        {
                            IMetadata::IEntry entry(MTK_CONTROL_SCENE_MODE);
                            entry.push_back(MTK_CONTROL_SCENE_MODE_HDR, Type2Type< MUINT8 >());
                            appSetting.update(entry.tag(), entry);
                        }
                        //
                        vSettings.push_back(
                            SettingSet{
                                appSetting,
                                halSetting
                            }
                        );
                        //
                        // submit to zsd preview pipeline
                        res = applyRawBufferSettings( vSettings, shotCount );
                    }
                } break;
            // default zsd flow
            default:
                break;
        }

        if(res == MFALSE){
            MY_LOGE("applySettings Fail!");
            endFailedCapture();
            return MFALSE;
        }
        //
    }

    /**
     *  Add a control flow to get the continuous multiple RAWs from ZSD buffer
     */
     mpConsumer = IResourceContainer::getInstance(getOpenId())->queryConsumer( eSTREAMID_IMAGE_PIPE_RAW_OPAQUE );
    sp<StreamBufferProvider> pProvider = mpConsumer.promote();
    MBOOL isUsingZsdBuffer = [&pProvider](){
        if (pProvider.get() == nullptr)
            return MFALSE;
        sp< ISelector > pSelector = pProvider->querySelector();
        // check if selector is ZsdSelector, if yes, using ZSD buffer
        return pSelector->getSelectorType() == ISelector::SelectorType_ZsdSelector
            ? MTRUE
            : MFALSE
            ;
    }();

    status_t st = OK;
    if (isUsingZsdBuffer) {
        sp< ISelector > mSelector = pProvider->querySelector();
        ZsdSelector* pZsdSelector = static_cast<ZsdSelector*>(mSelector.get());
        MINT32 Selector_count = pZsdSelector->getContainerSize();
        if (shotCount > Selector_count) {
            pZsdSelector->setContainerSize(shotCount);
        }
        st  = runGetSelectorDataMultipleFlow(shotCount, type);
        if ( st != OK )
        {
            MY_LOGE("runGetSelectorDataMultipleFlow fail:(%d)", st);
            return MFALSE; // no need to invoke endCapture again if return as MFALSE.
        }
        if (shotCount > Selector_count) {
            pZsdSelector->setContainerSize(Selector_count);
        }
    }
    else {
        st = runSubmitRequestAndGetSelectorDataFlow(shotCount, vSettings, type);
        if ( st != OK )
        {
            MY_LOGE("runSubmitRequestAndGetSelectorDataFlow fail:(%d)", st);
            return MFALSE; // no need to invoke endCapture again if return as MFALSE.
        }
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
void
ZsdVendorShot::
onCmd_cancel()
{
}

/******************************************************************************
*
*******************************************************************************/
MBOOL
ZsdVendorShot::
handlePostViewData(MUINT8* const /*puBuf*/, MUINT32 const /*u4Size*/)
{
#if 0
    MY_LOGD("+ (puBuf, size) = (%p, %d)", puBuf, u4Size);
    mpShotCallback->onCB_PostviewDisplay(0,
                                         u4Size,
                                         reinterpret_cast<uint8_t const*>(puBuf)
                                        );

    MY_LOGD("-");
#endif
    return  MTRUE;
}

/******************************************************************************
*
*******************************************************************************/
MBOOL
ZsdVendorShot::
handleJpegData(IImageBuffer* pJpeg)
{
    CAM_TRACE_CALL();
    //
    mpShotCallback->onCB_Shutter(true,0,getShotMode());
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

    // dummy raw callback
    mpShotCallback->onCB_RawImage(0, 0, NULL);

    // Jpeg callback
    mpShotCallback->onCB_CompressedImage_packed(getInstanceId(),
                                             u4JpegSize,
                                             puJpegBuf,
                                             0,                       //callback index
                                             true,                     //final image
                                             MTK_CAMERA_MSG_EXT_DATA_COMPRESSED_IMAGE,
                                             getShotMode(),
                                             supportBackgroundService()
                                             );

    {
        Mutex::Autolock _l(mEncJobLock);
        mEncJob.clear();
    }

    MY_LOGD("-");

    return MTRUE;
}

/******************************************************************************
*
*******************************************************************************/
MVOID
ZsdVendorShot::
decideSettingType( MINT32& type )
{
    sp<IFrameInfo> pFrameInfo = IResourceContainer::getInstance(getOpenId())->queryLatestFrameInfo();
    if(pFrameInfo == NULL)
    {
        MY_LOGW("Can't query Latest FrameInfo!");
    }
    else
    {
        //The FlashRequired condition must be the same with PreCapture required condition in CamAdapter::takePicture()
        IMetadata meta;
        pFrameInfo->getFrameMetadata(eSTREAMID_META_APP_DYNAMIC_P1, meta);
        MUINT8 AeMode = 0;
        tryGetMetadata< MUINT8 >(&meta, MTK_CONTROL_AE_MODE, AeMode);
        //
        IMetadata halMeta;
        pFrameInfo->getFrameMetadata(eSTREAMID_META_HAL_DYNAMIC_P1, halMeta);
        MBOOL isBvTrigger = MFALSE;
        tryGetMetadata< MBOOL >(&halMeta, MTK_3A_AE_BV_TRIGGER, isBvTrigger);
        //
        if( AeMode == MTK_CONTROL_AE_MODE_ON_ALWAYS_FLASH ||
            (AeMode == MTK_CONTROL_AE_MODE_ON_AUTO_FLASH && isBvTrigger) )
        {
            type = SETTING_FLASH_ACQUIRE;
            MY_LOGD("Acquire flash");
        }
        /*else if (1)
            type = SETTING_HDR;*/
        else
            type = SETTING_NONE;
        MY_LOGD("query AE mode(%d) type(%d)", AeMode, type);
    }
}


/******************************************************************************
*
*******************************************************************************/
MERROR
ZsdVendorShot::
updateCaptureParams(
    MINT32 shotCount,
    Vector<CaptureParam_T>& vHdrCaptureParams
)
{
    IHal3A *hal3A = MAKE_Hal3A(
                    getOpenId(), LOG_TAG);
    if ( hal3A == NULL ) {
        MY_LOGE("null 3a.");
        return UNKNOWN_ERROR;
    }
    //
    ExpSettingParam_T rExpSetting;
    hal3A->send3ACtrl( E3ACtrl_GetExposureInfo,
        reinterpret_cast<MINTPTR>(&rExpSetting), 0);
    CaptureParam_T tmpCap3AParam;
    hal3A->send3ACtrl( E3ACtrl_GetExposureParam,
        reinterpret_cast<MINTPTR>(&tmpCap3AParam), 0);
    //
    MUINT32 delayedFrames = 0;
    hal3A->send3ACtrl( E3ACtrl_GetCaptureDelayFrame,
            reinterpret_cast<MINTPTR>(&delayedFrames), 0);
    //
    for (MINT32 i = 0; i < shotCount; i++)
    {
        // copy original capture parameter
        CaptureParam_T modifiedCap3AParam = tmpCap3AParam;
#warning "modify EV settings here"
        modifiedCap3AParam.u4Eposuretime  = tmpCap3AParam.u4Eposuretime;
        modifiedCap3AParam.u4AfeGain      = tmpCap3AParam.u4AfeGain;
        //
        modifiedCap3AParam.u4IspGain      = 1024; // fix ISP gain to 1x
        modifiedCap3AParam.u4FlareOffset  = tmpCap3AParam.u4FlareOffset;
        MY_LOGD_IF( 1, "Modified ExposureParam[%d] w/ Exp(%d) Gain(%d)",
                    i, tmpCap3AParam.u4Eposuretime, tmpCap3AParam.u4AfeGain);
        vHdrCaptureParams.push_back(modifiedCap3AParam);
    }
    //
    hal3A->destroyInstance(LOG_TAG);
    //
    //  append another blank EVs in tails, for AE converge
    for ( size_t i = 0; i < delayedFrames; ++i) {
        vHdrCaptureParams.push_back(tmpCap3AParam);
    }
    //
    return OK;
}

/******************************************************************************
*
*******************************************************************************/
static
MVOID
prepare_stream(BufferList& vDstStreams, StreamId id, MBOOL criticalBuffer)
{
    vDstStreams.push_back(
        BufferSet{
            .streamId       = id,
            .criticalBuffer = criticalBuffer,
        }
    );
}

MBOOL
ZsdVendorShot::
applyRawBufferSettings(
    Vector< SettingSet > vSettings,
    MINT32 shotCount,
    MINT32 frontDummyCount,
    MBOOL  bNeedStoreSelector
)
{
    MY_LOGD("Apply user's setting. vSettings.size()==%zu",vSettings.size());
    //
    mpConsumer = IResourceContainer::getInstance(getOpenId())->queryConsumer( eSTREAMID_IMAGE_PIPE_RAW_OPAQUE );
    sp<StreamBufferProvider> pProvider = mpConsumer.promote();
    if(pProvider == NULL) {
        MY_LOGE("pProvider is NULL!");
        return MFALSE;
    }
    //
    if(pProvider->querySelector() == NULL)
    {
        MY_LOGE("pProvider's Selector is NULL!");
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
    //prepare_stream(vDstStreams, eSTREAMID_IMAGE_PIPE_RAW_OPAQUE,  true);
    //prepare_stream(vDstStreams, eSTREAMID_IMAGE_PIPE_RAW_RESIZER, false);
    //prepare_stream(vDstStreams, eSTREAMID_IMAGE_PIPE_YUV_00,      false);
    //if( mShotParam.mbEnableLtm )
    //    prepare_stream(vDstStreams, eSTREAMID_IMAGE_PIPE_RAW_LCSO,  true);
    Vector<sp<IImageStreamInfo>> rawInputInfos;
    pProvider->querySelector()->queryCollectImageStreamInfo( rawInputInfos, MTRUE );
    for( size_t i = 0; i < rawInputInfos.size() ; i++) {
        prepare_stream(vDstStreams, rawInputInfos[i]->getStreamId(),  false);
    }
    //
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
    if ( shotCount < (int)vRequestNo.size() ) {
        for ( MINT32 i = 0; i < shotCount; ++i ) {
            MINT32 capIndex;
            if (frontDummyCount > 0) {
                capIndex = i + frontDummyCount;
            }
            else {
                capIndex = i;
            }

            if (capIndex > vRequestNo.size()) {
                MY_LOGW("capIndex:%d out of vRequestNo[%zu]",
                        capIndex, vRequestNo.size());
            }
            else {
                vActualRequestNo.push_back(vRequestNo[capIndex]);
                MY_LOGD("vRequestNo[%d]:%d", capIndex, vRequestNo[capIndex]);
            }
        }
    } else {
        vActualRequestNo = vRequestNo;
    }
    //
    if(mspOriSelector.get() == nullptr) {
        MY_LOGD("Keep orginal selector");
        mspOriSelector = pProvider->querySelector();
    }
    //
    if(bNeedStoreSelector)
    {
        mspOriSelector = pProvider->querySelector();
        MY_LOGD("need store original selector");
    }

    sp<ZsdRequestSelector> pSelector = new ZsdRequestSelector();
    pSelector->setWaitRequestNo(vActualRequestNo);
    status_t status = pProvider->switchSelector(pSelector);// If other consumer exist, they will auto switch.
    // TODO add LCSO consumer set the same Selector
    if(status != OK)
    {
        MY_LOGE("change to ZSD Flash selector Fail!");
        return MFALSE;
    }
    MY_LOGD("change to ZSD Flash selector");
    //
    return MTRUE;
}

/******************************************************************************
*
*******************************************************************************/
MERROR
ZsdVendorShot::
submitCaptureSetting(
    IMetadata appSetting,
    IMetadata halSetting
)
{

    IMetadata::IEntry entry(MTK_HAL_REQUEST_SENSOR_SIZE);
    entry.push_back(mSensorSize, Type2Type< MSize >());
    halSetting.update(entry.tag(), entry);
    //
    ILegacyPipeline::ResultSet resultSet;
    {
        Mutex::Autolock _l(mResultMetadataSetLock);
        resultSet.add(eSTREAMID_META_APP_DYNAMIC_P1, mResultMetadataSetMap.editValueFor(mCapReqNo).selectorAppMetadata);
    }
    // submit setting to capture pipeline
    MY_LOGD("submitSetting %d",mCapReqNo);

    {
        BufferList vDstStreams;
        // JPEG
        //if( mpInfo_Jpeg.get() ) {
        //    PREPARE_STREAM(vDstStreams, eSTREAMID_IMAGE_JPEG, false);
        //}
        // YUV
        if( mpInfo_Yuv.get() ) {
            PREPARE_STREAM(vDstStreams, eSTREAMID_IMAGE_PIPE_YUV_JPEG, false);
        }
        if( mpInfo_YuvThumbnail.get() ) {
            PREPARE_STREAM(vDstStreams, eSTREAMID_IMAGE_PIPE_YUV_THUMBNAIL, false);
        }
        if( mpInfo_YuvPostview.get() ) {
            PREPARE_STREAM(vDstStreams, eSTREAMID_IMAGE_PIPE_YUV_00, false);
        }

        // RAW
        Vector<sp<IImageStreamInfo>> rawInputInfos;
        sp<StreamBufferProvider> pConsumer = mpConsumer.promote();
        if( !pConsumer.get() ) return UNKNOWN_ERROR;
        pConsumer->querySelector()->queryCollectImageStreamInfo( rawInputInfos );
        for( size_t i = 0; i < rawInputInfos.size() ; i++) {
            PREPARE_STREAM(vDstStreams, rawInputInfos[i]->getStreamId(),  false);
        }
        //
        IMetadata::setEntry<MUINT8>(&halSetting, MTK_3A_ISP_PROFILE, EIspProfile_Capture);
        if( OK != mpPipeline->submitRequest(
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
MVOID
ZsdVendorShot::
endCapture()
{
    // 1. change to ZSD selector
    mpConsumer = IResourceContainer::getInstance(getOpenId())->queryConsumer( eSTREAMID_IMAGE_PIPE_RAW_OPAQUE );
    sp<StreamBufferProvider> pConsumer = mpConsumer.promote();
    if(pConsumer != NULL) {
        if(mspOriSelector != nullptr) {
            MY_LOGD("restore original selector");
            pConsumer->setSelector(mspOriSelector);
            mspOriSelector = nullptr;
        }else{
            sp<ISelector> pSelector = pConsumer->querySelector();
            if ( pSelector == nullptr ||
                 (pSelector != nullptr && pSelector->getSelectorType() != ISelector::SelectorType_ZsdSelector)) {
                MY_LOGD("create new selector");
                sp<ZsdSelector> pSelector = new ZsdSelector();
                pConsumer->switchSelector(pSelector);
            }
        }
    }

    /* declare a job to wait until pipeline drain */
    auto _jobWaitUntilDrain = [this](sp<ImpShot> shot)
    {
        const char* FUNCTION = "_jobWaitUntilDrain";
        MY_LOGD("%s: [+], uid=%" PRId64 "", FUNCTION, getInstanceId());
        /* wait enc job done */
        {
            Mutex::Autolock _l(mEncJobLock);
            MY_LOGD("%s: wait EncJob done [+], uid=%" PRId64 "", FUNCTION, getInstanceId());
            if (mEncJobDone == MFALSE) {
                mEncDoneCond.wait(mEncJobLock);
            }
            mEncJobDone = MTRUE;
            MY_LOGD("%s: wait EncJob done [-], uid=%" PRId64 "", FUNCTION, getInstanceId());
        }

        /* clear futures */
        mvFutures.clear();

        /* wait pipeline drain */
        mpPipeline->waitUntilDrained();

        /* disable CPU control */
        if(mpCpuCtrl)
        {
            mpCpuCtrl->disable();
            mpCpuCtrl->uninit();
            mpCpuCtrl->destroyInstance();
            mpCpuCtrl = NULL;
        }

        MY_LOGD("%s: [-], uid=%" PRId64 "", FUNCTION, getInstanceId());
        shot = nullptr;
    };

    /* if result metadata set map is not empty, we have to wait Enc job done */
    if ( !mResultMetadataSetMap.isEmpty() ) {
        /* if using BackgroundService, wait at the other thread */
        if (supportBackgroundService()) {
            sp<ImpShot> self = this; // extend shot instance life.
            MY_LOGI("adding _jobWaitUntilDrain to PostProcessJobQueue, uid=%" PRId64 "", getInstanceId());
            PostProcessJobQueue::addJob( std::bind(
                        std::move( _jobWaitUntilDrain ),
                        self
                        ));
        }
        else {
            _jobWaitUntilDrain(nullptr);
        }
    }
    else {
        mvFutures.clear();
        mpPipeline->waitUntilDrained();

        if(mpCpuCtrl)
        {
            mpCpuCtrl->disable();
            mpCpuCtrl->uninit();
            mpCpuCtrl->destroyInstance();
            mpCpuCtrl = NULL;
        }
    }
}

/******************************************************************************
*
*******************************************************************************/
MVOID
ZsdVendorShot::
endFailedCapture()
{
    // 1. callback dump jpeg
    onFakeJpegCallback(mpShotCallback);
    // 2. change to ZSD selector
    mpConsumer = IResourceContainer::getInstance(getOpenId())->queryConsumer( eSTREAMID_IMAGE_PIPE_RAW_OPAQUE );
    sp<StreamBufferProvider> pConsumer = mpConsumer.promote();
    if(pConsumer != NULL) {
        sp<ZsdSelector> pSelector = new ZsdSelector();
        pConsumer->setSelector(pSelector);
    }
    // 3. wait pipeline done
    if( mpPipeline != NULL ) {
        mpPipeline->waitUntilDrained();
    }
}


/******************************************************************************
*
*******************************************************************************/
MBOOL
ZsdVendorShot::
createStreams()
{
    CAM_TRACE_CALL();
    MUINT32 const openId        = getOpenId();
    MUINT32 const sensorMode    = mu4Scenario;
    MUINT32 const bitDepth      = mu4Bitdepth;
    //
    MSize const previewsize     = MSize(mShotParam.mi4PostviewWidth, mShotParam.mi4PostviewHeight);
    MINT const previewfmt       = static_cast<EImageFormat>(mShotParam.miPostviewDisplayFormat);
    #if MTK_CAM_YUV420_JPEG_ENCODE_SUPPORT
    MINT const yuvfmt_main      = eImgFmt_NV21;
    #else
    MINT const yuvfmt_main      = eImgFmt_YUY2;
    #endif
    MINT const yuvfmt_thumbnail = eImgFmt_YUY2;
    //
    MSize const thumbnailsize = MSize(mJpegParam.mi4JpegThumbWidth, mJpegParam.mi4JpegThumbHeight);
    //
    //
#if 0
    // postview
    if( isDataMsgEnabled(ECamShot_DATA_MSG_POSTVIEW) )
    {
        MSize size        = previewsize;
        MINT format       = previewfmt;
        MUINT const usage = 0; //not necessary here
        MUINT32 transform = 0;
        sp<IImageStreamInfo>
            pStreamInfo = createImageStreamInfo(
                    "SingleShot:Postview",
                    eSTREAMID_IMAGE_PIPE_YUV_00,
                    eSTREAMTYPE_IMAGE_INOUT,
                    1, 1,
                    usage, format, size, transform
                    );
        if( pStreamInfo == nullptr ) {
            return BAD_VALUE;
        }
        //
        mpInfo_YuvPostview = pStreamInfo;
    }
#endif
    //
    // Yuv
    {
        MSize size        = mJpegsize;
        MINT format       = yuvfmt_main;
        MUINT const usage = 0; //not necessary here
        MUINT32 transform = mShotParam.mu4Transform;
        sp<IImageStreamInfo>
            pStreamInfo = createImageStreamInfo(
                    "ZsdVendorShot:MainYuv",
                    eSTREAMID_IMAGE_PIPE_YUV_JPEG,
                    eSTREAMTYPE_IMAGE_INOUT,
                    muYuvBufferCount, 0,
                    usage, format, size, transform,
                    MSize(16, 16) // alignment
                    );
        if( pStreamInfo == nullptr ) {
            return BAD_VALUE;
        }
        //
        mpInfo_Yuv = pStreamInfo;
        MY_LOGD("zsd Yuv stream %#" PRIx64 "(%s) size(%dx%d), fmt 0x%x",
                static_cast<uint64_t>(mpInfo_Yuv->getStreamId()),
                mpInfo_Yuv->getStreamName(),
                mpInfo_Yuv->getImgSize().w,
                mpInfo_Yuv->getImgSize().h,
                mpInfo_Yuv->getImgFormat()
               );
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
                    "ZsdVendorShot:ThumbnailYuv",
                    eSTREAMID_IMAGE_PIPE_YUV_THUMBNAIL,
                    eSTREAMTYPE_IMAGE_INOUT,
                    muYuvBufferCount, 0,
                    usage, format, size, transform
                    );
        if( pStreamInfo == nullptr ) {
            return BAD_VALUE;
        }
        //
        mpInfo_YuvThumbnail = pStreamInfo;
        //
        MY_LOGD("zsd YuvThumbnail stream %#" PRIx64 "(%s) size(%dx%d), fmt 0x%x",
                static_cast<uint64_t>(mpInfo_YuvThumbnail->getStreamId()),
                mpInfo_YuvThumbnail->getStreamName(),
                mpInfo_YuvThumbnail->getImgSize().w,
                mpInfo_YuvThumbnail->getImgSize().h,
                mpInfo_YuvThumbnail->getImgFormat()
               );
    }
    //
    // Jpeg
    {
        MSize size        = mJpegsize;
        MINT format       = eImgFmt_BLOB;
        MUINT const usage = 0; //not necessary here
        MUINT32 transform = 0;
        sp<IImageStreamInfo>
            pStreamInfo = createImageStreamInfo(
                    "ZsdVendorShot:Jpeg",
                    eSTREAMID_IMAGE_JPEG,
                    eSTREAMTYPE_IMAGE_INOUT,
                    1, 0,
                    usage, format, size, transform
                    );
        if( pStreamInfo == nullptr ) {
            return BAD_VALUE;
        }
        //
        mpInfo_Jpeg = pStreamInfo;
        //
        MY_LOGD("zsd Jpeg stream %#" PRIx64 "(%s) size(%dx%d), fmt 0x%x",
                static_cast<uint64_t>(mpInfo_Jpeg->getStreamId()),
                mpInfo_Jpeg->getStreamName(),
                mpInfo_Jpeg->getImgSize().w,
                mpInfo_Jpeg->getImgSize().h,
                mpInfo_Jpeg->getImgFormat()
               );
    }
    //
    return MTRUE;

}

/******************************************************************************
*
*******************************************************************************/
#if MTKCAM_HAVE_COMMON_CAPTURE_PIPELINE_SUPPORT
MBOOL
ZsdVendorShot::
createPipeline(Vector<BufferItemSet>& rBufferSet)
{
    CAM_TRACE_CALL();
    FUNC_START;

    ICommonCapturePipeline::PipelineConfig pipelineConfig;
    pipelineConfig.userName = LOG_TAG;
    pipelineConfig.openId = getOpenId();
    pipelineConfig.isZsdMode = MTRUE;

    // LegacyPipelineBuilder::ConfigParams
    pipelineConfig.LPBConfigParams.mode = getLegacyPipelineMode();
    pipelineConfig.LPBConfigParams.enableEIS = MFALSE;
    pipelineConfig.LPBConfigParams.enableLCS = mShotParam.mbEnableLtm;
    pipelineConfig.LPBConfigParams.pluginUser = plugin::CALLER_SHOT_GENERAL;
    //
    HwInfoHelper helper(getOpenId());
    if( ! helper.updateInfos() ) {
        MY_LOGE("cannot properly update infos");
        return BAD_VALUE;
    }
    //
    if (helper.getDualPDAFSupported(mu4Scenario))
    {
        pipelineConfig.LPBConfigParams.enableDualPD = MTRUE;
    }
    //
    MUINT32 pipeBit;
    MINT ImgFmt = eImgFmt_BAYER12;
    if( helper.getLpModeSupportBitDepthFormat(ImgFmt, pipeBit) )
    {
        using namespace NSCam::NSIoPipe::NSCamIOPipe;
        if(pipeBit & CAM_Pipeline_14BITS)
        {
            pipelineConfig.LPBConfigParams.pipeBit = CAM_Pipeline_14BITS;
        }
        else
        {
            pipelineConfig.LPBConfigParams.pipeBit = CAM_Pipeline_12BITS;
        }
    }

    // provide pCallbackListener
    pipelineConfig.pCallbackListener = this;

    // provide sp<IImageStreamInfo>
    // fullraw
    if( mpInfo_FullRaw.get() )
    {
        pipelineConfig.pInfo_FullRaw = mpInfo_FullRaw;
    }
    // resized raw
    if( mpInfo_ResizedRaw.get() )
    {
        pipelineConfig.pInfo_ResizedRaw = mpInfo_ResizedRaw;
    }
    // jpeg main yuv
    if( mpInfo_Yuv.get() )
    {
        pipelineConfig.pInfo_Yuv = mpInfo_Yuv;
    }
    // postview yuv
    if( mpInfo_YuvPostview.get() )
    {
        pipelineConfig.pInfo_YuvPostview = mpInfo_YuvPostview;
    }
    // jpeg thumbnail yuv
    if( mpInfo_YuvThumbnail.get() )
    {
        pipelineConfig.pInfo_YuvThumbnail = mpInfo_YuvThumbnail;
    }
#if 0
    // jpeg
    if( mpInfo_Jpeg.get() )
    {
        pipelineConfig.pInfo_Jpeg = mpInfo_Jpeg;
    }
#endif

    // creat and query common capture pipeline
    mpPipeline = ICommonCapturePipeline::queryCommonCapturePipeline(pipelineConfig);
    CHECK_OBJECT(mpPipeline);
    //
    //
    FUNC_END;
    return MTRUE;
}

#else // not MTKCAM_HAVE_COMMON_CAPTURE_PIPELINE_SUPPORT

MBOOL
ZsdVendorShot::
createPipeline(Vector<BufferItemSet>& rBufferSet)
{
    CAM_TRACE_CALL();
    FUNC_START;
    LegacyPipelineBuilder::ConfigParams LPBConfigParams;
    LPBConfigParams.mode = getLegacyPipelineMode();
    LPBConfigParams.enableEIS = MFALSE;
    LPBConfigParams.enableLCS = mShotParam.mbEnableLtm;
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
    MUINT32 pipeBit;
    MINT ImgFmt = eImgFmt_BAYER12;
    if( helper.getLpModeSupportBitDepthFormat(ImgFmt, pipeBit) )
    {
        using namespace NSCam::NSIoPipe::NSCamIOPipe;
        if(pipeBit & CAM_Pipeline_14BITS)
        {
            LPBConfigParams.pipeBit = CAM_Pipeline_14BITS;
        }
        else
        {
            LPBConfigParams.pipeBit = CAM_Pipeline_12BITS;
        }
    }
    //
    sp<LegacyPipelineBuilder> pBuilder = LegacyPipelineBuilder::createInstance(
                                            getOpenId(),
                                            "ZsdVendorShot",
                                            LPBConfigParams);
    CHECK_OBJECT(pBuilder);

    mpImageCallback = new ImageCallback(this, 0);
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
        MY_LOGD("createPipeline for ZSD vendor shot");
        // zsd flow
        Vector<PipelineImageParam> vImgSrcParams;
        sp<IImageStreamInfo> pStreamInfo = NULL;
        for(size_t i = 0; i < rBufferSet.size() ; i++)
        {
            pStreamInfo = rBufferSet[i].streamInfo;
            //
            if(pStreamInfo == NULL)
            {
                MY_LOGE("Can not find stream info (0x%#" PRIx64 "),(%p)", static_cast<uint64_t>(rBufferSet[i].id), rBufferSet[i].heap.get());
                return MFALSE;
            }
            sp<IImageBuffer> pBuf = rBufferSet[i].heap->createImageBuffer();
            if(pBuf == NULL)
            {
                MY_LOGE("Can not build image buffer (0x%#" PRIx64 "),(%p)", static_cast<uint64_t>(rBufferSet[i].id), rBufferSet[i].heap.get());
                return MFALSE;
            }
            sp<CallbackBufferPool> pPool = new CallbackBufferPool(pStreamInfo);
            //update p1 buffer to pipeline pool
            pPool->addBuffer(pBuf);
            pCallbackHandler->setBufferPool(pPool);
            //
            pFactory->setImageStreamInfo(pStreamInfo);
            pFactory->setUsersPool(
                    pCallbackHandler->queryBufferPool(pStreamInfo->getStreamId())
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
    }
    //
    {
        if( mpInfo_Yuv.get() )
        {
            sp<IImageStreamInfo> pStreamInfo = mpInfo_Yuv;
            //
            sp<CallbackBufferPool> pPool = new CallbackBufferPool(pStreamInfo);
            {
                /**
                 * Since these YUV will be kept after invoked CallbackBufferPool::releaseToPool,
                 * we have to set this CallbackBufferPool as a pool never reuses the buffer
                 * if caller invokes CallbackBufferPool::releaseToPool.
                 */
                pPool->setForceNoNeedReturnBuffer( true );

                pPool->allocateBuffer(
                                  pStreamInfo->getStreamName(),
                                  pStreamInfo->getMaxBufNum(),
                                  pStreamInfo->getMinInitBufNum()
                                  );
                pCallbackHandler->setBufferPool(pPool);
            }
            //
            pFactory->setImageStreamInfo(pStreamInfo);
            pFactory->setUsersPool(
                        pCallbackHandler->queryBufferPool(pStreamInfo->getStreamId())
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
            pCallbackHandler->setBufferPool(pPool);
            //
            pFactory->setImageStreamInfo(pStreamInfo);
            pFactory->setUsersPool(
                        pCallbackHandler->queryBufferPool(pStreamInfo->getStreamId())
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
            pCallbackHandler->setBufferPool(pPool);
            //
            pFactory->setImageStreamInfo(pStreamInfo);
            pFactory->setUsersPool(
                        pCallbackHandler->queryBufferPool(pStreamInfo->getStreamId())
                    );
            //
            PipelineImageParam imgParam = {
                .pInfo     = pStreamInfo,
                .pProvider = pFactory->create(false),
                .usage     = 0
            };
            vImageParam.push_back(imgParam);
        }
#if 0
        //
        if( mpInfo_Jpeg.get() )
        {
            sp<IImageStreamInfo> pStreamInfo = mpInfo_Jpeg;
            //
            sp<CallbackBufferPool> pPool = new CallbackBufferPool(pStreamInfo);
            {
                pPool->allocateBuffer(
                                  pStreamInfo->getStreamName(),
                                  pStreamInfo->getMaxBufNum(),
                                  pStreamInfo->getMinInitBufNum()
                                  );
            }
            pCallbackHandler->setBufferPool(pPool);
            //
            pFactory->setImageStreamInfo(pStreamInfo);
            pFactory->setUsersPool(
                        pCallbackHandler->queryBufferPool(pStreamInfo->getStreamId())
                    );
            //
            PipelineImageParam imgParam = {
                .pInfo     = pStreamInfo,
                .pProvider = pFactory->create(),
                .usage     = 0
            };
            vImageParam.push_back(imgParam);
        }
#endif
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
#endif // MTKCAM_HAVE_COMMON_CAPTURE_PIPELINE_SUPPORT

/******************************************************************************
*
*******************************************************************************/
MINT
ZsdVendorShot::
getLegacyPipelineMode(void)
{
    int shotMode = getShotMode();
    EPipelineMode pipelineMode = getPipelineMode();
    int legacyPipeLineMode = LegacyPipelineMode_T::PipelineMode_Capture;
    switch(shotMode)
    {
        default:
            legacyPipeLineMode = (pipelineMode == ePipelineMode_Feature) ?
                LegacyPipelineMode_T::PipelineMode_Feature_Capture :
                LegacyPipelineMode_T::PipelineMode_Capture;
            break;
    }
    return legacyPipeLineMode;
}

/******************************************************************************
*
*******************************************************************************/
status_t
ZsdVendorShot::
getSelectorData(
    IMetadata& rAppSetting,
    IMetadata& rHalSetting,
    Vector<BufferItemSet>& rvBufferSet
)
{
    CAM_TRACE_CALL();
    //
    mpConsumer = IResourceContainer::getInstance(getOpenId())->queryConsumer( eSTREAMID_IMAGE_PIPE_RAW_OPAQUE );
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
    // to notify selector that capture already started:
    //     if AF mode is AUTO and still not receive AutoFocus,
    //     then don't need to wait AF done
    pSelector->sendCommand(ISelector::eCmd_setAlreadyStartedCapture, 0, 0, 0);
    //
    status_t status = OK;
    MINT32 rRequestNo;
    Vector<ISelector::MetaItemSet> rvResultMeta;
    status = pSelector->getResult(rRequestNo, rvResultMeta, rvBufferSet);

    if(rvBufferSet.size() <=0)
    {
        MY_LOGE("Selector get input raw buffer failed! bufferSetSize(%zu)", rvBufferSet.size());
        return UNKNOWN_ERROR;
    }

    if(status != OK) {
        MY_LOGE("Selector getResult Fail!");
        return UNKNOWN_ERROR;
    }

    if(rvResultMeta.size() < 2) {
        MY_LOGE("ZsdSelect getResult rvResultMeta(%d)", rvResultMeta.size());
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
    //
    {
        Mutex::Autolock _l(mResultMetadataSetLock);

        IMetadata resultAppMetadata;
        IMetadata resultHalMetadata;
        IMetadata selectorAppMetadata = rAppSetting;
        mResultMetadataSetMap.add(static_cast<MUINT32>(mCapReqNo),ResultSet_T{static_cast<MUINT32>(mCapReqNo), resultAppMetadata, resultHalMetadata, selectorAppMetadata, rvBufferSet});
    }
    return OK;
}


/******************************************************************************
*
*******************************************************************************/
status_t
ZsdVendorShot::
getSelectorDataMultiple(
        Vector< IMetadata >&                rAppSettings,
        Vector< IMetadata >&                rHalSettings,
        Vector< Vector<BufferItemSet> >&    rBufferSets,
        MINT32                              frameNo
)
{
    CAM_TRACE_CALL();
    //
    mpConsumer = IResourceContainer::getInstance(getOpenId())->queryConsumer( eSTREAMID_IMAGE_PIPE_RAW_OPAQUE );
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
    sp<IParamsManager> pParamsMgr = mShotParam.mpParamsMgr.promote();
    if (CC_UNLIKELY(pParamsMgr.get() == nullptr)) {
        MY_LOGE("cannot get params manager, assume need to reconstruct");
    } else {
        String8 const mFocusModeStr = pParamsMgr->getStr(CameraParameters::KEY_FOCUS_MODE);
        MUINT32 Mcompensation = pParamsMgr->getInt(CameraParameters::KEY_EXPOSURE_COMPENSATION);
        if (((!mFocusModeStr.isEmpty())
            && (strcmp(mFocusModeStr.string(), CameraParameters::FOCUS_MODE_AUTO) == 0))
            || (Mcompensation)) {
            pSelector->sendCommand(ISelector::eCmd_setNoWaitAfDone, 0, 0, 0);
            MY_LOGD("eCmd_setNoWaitAfDone mFocusMode:%s, Mcompensation:%d",
                mFocusModeStr.string(), Mcompensation);
        }
    }
    //
    auto __getSelectorData = [this](
            Vector< IMetadata >&                 _rAppSetting,
            Vector< IMetadata >&                 _rHalSetting,
            Vector< Vector<BufferItemSet> >&     _rBufferSets,
            MINT32                               _frameNo
    )
    {
        status_t err = OK;
        for (MINT32 i = 0; i < _frameNo; i++) {
            err = getSelectorData(
                    _rAppSetting.editItemAt(i),
                    _rHalSetting.editItemAt(i),
                    _rBufferSets.editItemAt(i)
                    );
            if (err != OK)
                return err;
        }
        return err;
    };

    // check if selector is ZsdSelector
    if (CC_UNLIKELY( pSelector->getSelectorType() != ISelector::SelectorType_ZsdSelector )) {
        MY_LOGE("wrong selector type(%#x) for this method", pSelector->getSelectorType());
        // using getSelectorData N times
        return __getSelectorData(
                rAppSettings,
                rHalSettings,
                rBufferSets,
                frameNo
            );
    }

    ZsdSelector* pZsdSelector = static_cast<ZsdSelector*>(pSelector.get());

    status_t status = OK;
    Vector< Vector<ISelector::MetaItemSet> >    vvResultMeta;
    Vector< Vector<ISelector::BufferItemSet> >  vvBufferSet;
    Vector< MINT32 >                            vRequestNo;

    vRequestNo.resize(frameNo);

    // get the continuous results
    status = pZsdSelector->getContinuousResults(
            vRequestNo,
            vvResultMeta,
            vvBufferSet,
            5000 // ms
            );

    // if timed out, try again, take any frame (w/o AF OK)
    if (status == TIMED_OUT) {
        pZsdSelector->sendCommand(ISelector::eCmd_setNoWaitAfDone, 0, 0, 0, 0);
        MY_LOGD("sendCommand(eCmd_setNoWaitAfDone,0,0,0)");
        MY_LOGW("wait continuous result timed out (%d), try again for any buffer", 5000);
        pZsdSelector->sendCommand(ISelector::eCmd_setAllBuffersTransferring, MTRUE, 0, 0);
        status = pZsdSelector->getContinuousResults(
                vRequestNo,
                vvResultMeta,
                vvBufferSet,
                5000 // ms
                );
        pZsdSelector->sendCommand(ISelector::eCmd_setAllBuffersTransferring, MFALSE, 0, 0);
        pZsdSelector->sendCommand(ISelector::eCmd_setNeedWaitAfDone, 0, 0, 0, 0); // set back
        MY_LOGD("sendCommand(eCmd_setNeedWaitAfDone,0,0,0)");
    }

    // check status
    if (CC_UNLIKELY( status != OK )) {
        MY_LOGE("Selector getResult failed(%#x)", status);
        pZsdSelector->sendCommand(ISelector::eCmd_setNoWaitAfDone, 0, 0, 0, 0);
        status = pZsdSelector->getContinuousResults(
                vRequestNo,
                vvResultMeta,
                vvBufferSet,
                5000 // ms
                );
        pZsdSelector->sendCommand(ISelector::eCmd_setNeedWaitAfDone, 0, 0, 0, 0);
        if(status != OK)
        {
            MY_LOGD("retry getContinuousResults fail!");
            return UNKNOWN_ERROR;
        }
    }
    for (int i = 0; i < frameNo; i++) {
        if(vvBufferSet.editItemAt(i).size() <=0)
        {
            MY_LOGE("Selector get input buffer failed! bufferSetSize(%zu)", vvBufferSet.editItemAt(i).size());
            return UNKNOWN_ERROR;
        }

        if(vvResultMeta.editItemAt(0).size() < 2) {
            MY_LOGE("ZsdSelect getResult vvResultMeta(%zu)", vvResultMeta.editItemAt(0).size());
            return UNKNOWN_ERROR;
        }

        // get app & hal metadata
        if( vvResultMeta.editItemAt(i).editItemAt(0).id == eSTREAMID_META_APP_DYNAMIC_P1
            && vvResultMeta.editItemAt(i).editItemAt(1).id == eSTREAMID_META_HAL_DYNAMIC_P1 )
        {
            rAppSettings.editItemAt(i)  = vvResultMeta.editItemAt(i).editItemAt(0).meta;
            rHalSettings.editItemAt(i) += vvResultMeta.editItemAt(i).editItemAt(1).meta;
        }
        else
        {
            if( vvResultMeta.editItemAt(i).editItemAt(1).id == eSTREAMID_META_APP_DYNAMIC_P1
                && vvResultMeta.editItemAt(i).editItemAt(0).id == eSTREAMID_META_HAL_DYNAMIC_P1 )
            {
                rAppSettings.editItemAt(i)  = vvResultMeta.editItemAt(i).editItemAt(1).meta;
                rHalSettings.editItemAt(i) += vvResultMeta.editItemAt(i).editItemAt(0).meta;
            }
            else {
                MY_LOGE("Something wrong for selector metadata.");
                return UNKNOWN_ERROR;
            }
        }

        // assign buffer set to return argument
        rBufferSets.editItemAt(i) = vvBufferSet.editItemAt(i);

        // push back to metaset container
        {
            Mutex::Autolock _l(mResultMetadataSetLock);

            IMetadata resultAppMetadata;
            IMetadata resultHalMetadata;
            IMetadata selectorAppMetadata = rAppSettings.editItemAt(i);
            mResultMetadataSetMap.add(
                    static_cast<MUINT32>(mCapReqNo+i),
                    ResultSet_T{static_cast<MUINT32>(mCapReqNo+i),
                    resultAppMetadata, resultHalMetadata,
                    selectorAppMetadata,
                    vvBufferSet.editItemAt(i)});
        }
    }

    return OK;
}


/*******************************************************************************
*
********************************************************************************/
MVOID
ZsdVendorShot::
onMetaReceived(
    MUINT32         const requestNo,
    StreamId_T      const streamId,
    MBOOL           const errorResult,
    IMetadata       const result
)
{
    CAM_TRACE_FMT_BEGIN("onMetaReceived No%d,StreamID %#" PRIx64, requestNo, static_cast<uint64_t>(streamId) );
    MY_LOGD("requestNo %d, stream %#" PRIx64", errResult:%d", requestNo, static_cast<uint64_t>(streamId), errorResult);
    //
    {
        Mutex::Autolock _l(mResultMetadataSetLock);
        int idx = mResultMetadataSetMap.indexOfKey(requestNo);
        if(idx < 0 )
        {
            MY_LOGE("mResultMetadataSetMap can't find requestNo(%d)",requestNo);
            for ( size_t i=0; i<mResultMetadataSetMap.size(); i++) {
                MY_LOGD( "mResultMetadataSetMap(%d/%d)  requestNo(%d) buf(%p)",
                         i, mResultMetadataSetMap.size(), mResultMetadataSetMap[i].requestNo,
                         mResultMetadataSetMap[i].selectorGetBufs[0].heap.get() );
            }
            return;
        }
    }
    //
    //Need to modify if pipeline change
    if (streamId == eSTREAMID_META_HAL_DYNAMIC_P2) {
        // mpShotCallback->onCB_Shutter(true,0);
        //
        //
        {
            Mutex::Autolock _l(mResultMetadataSetLock);
            mResultMetadataSetMap.editValueFor(requestNo).halResultMetadata = result;
        }
        {
            Mutex::Autolock _l(mEncJobLock);
            mEncJob.add(requestNo, EncJob::STREAM_HAL_META, result);
            checkStreamAndEncodeLocked(requestNo);
        }
    }
    else if (streamId == eSTREAMID_META_APP_DYNAMIC_P2) {
        IMetadata tmp;
        {
            Mutex::Autolock _l(mResultMetadataSetLock);
            // update appResultMetadata
            mResultMetadataSetMap.editValueFor(requestNo).appResultMetadata += result;
            // update tmp metadata
            tmp = mResultMetadataSetMap.valueFor(requestNo).selectorAppMetadata;
            tmp += result;
        }
        {
            Mutex::Autolock _l(mEncJobLock);
            mEncJob.add(requestNo, EncJob::STREAM_APP_META, tmp);
            checkStreamAndEncodeLocked(requestNo);
        }
    }
    //
    CAM_TRACE_FMT_END();
}

/*******************************************************************************
*
********************************************************************************/
MVOID
ZsdVendorShot::
onDataReceived(
    MUINT32               const requestNo,
    StreamId_T            const streamId,
    MBOOL                 const errorBuffer,
    android::sp<IImageBuffer>&  pBuffer
)
{
    CAM_TRACE_FMT_BEGIN("onDataReceived No%d,streamId%d",requestNo,streamId);
    MY_LOGD("requestNo %d, streamId 0x%x, buffer %p", requestNo, streamId, pBuffer.get());
    //
    if( pBuffer != 0 )
    {
        if (streamId == eSTREAMID_IMAGE_PIPE_YUV_00)
        {
        }
        else if (streamId == eSTREAMID_IMAGE_PIPE_RAW_OPAQUE)
        {
            //
            sp<StreamBufferProvider> pProvider = IResourceContainer::getInstance(getOpenId())->queryConsumer( eSTREAMID_IMAGE_PIPE_RAW_OPAQUE );
            MY_LOGD("Query Consumer OpenID(%d) StreamID(%d)", getOpenId(), eSTREAMID_IMAGE_PIPE_RAW_OPAQUE);
            if( pProvider == NULL) {
                MY_LOGE("can't find StreamBufferProvider in ConsumerContainer");
            }
            else {
                Vector<ISelector::BufferItemSet> bufferSet;
                {
                    Mutex::Autolock _l(mResultMetadataSetLock);
                    bufferSet = mResultMetadataSetMap.editValueFor(requestNo).selectorGetBufs;
                };

                sp< ISelector > pSelector = pProvider->querySelector();
                if(pSelector == NULL) {
                    MY_LOGE("can't find Selector in Consumer when p2done");
                } else {
                    for(size_t i = 0; i < bufferSet.size() ; i++)
                        pSelector->returnBuffer(bufferSet.editItemAt(i));
                }
            }
            //
            {
                Mutex::Autolock _l(mResultMetadataSetLock);
                for (auto& itr : mResultMetadataSetMap.editValueFor(requestNo).selectorGetBufs) {
                    itr = ISelector::BufferItemSet();
                }
            }

        }
        else if (streamId == eSTREAMID_IMAGE_PIPE_YUV_THUMBNAIL)
        {
            {
                Mutex::Autolock _l(mEncJobLock);
                mEncJob.add(requestNo, EncJob::STREAM_IMAGE_THUMB, pBuffer);
                checkStreamAndEncodeLocked(requestNo);
            }
        }
        else if (streamId == eSTREAMID_IMAGE_PIPE_YUV_JPEG)
        {
            //
            {
                Mutex::Autolock _l(mEncJobLock);
                mEncJob.add(requestNo, EncJob::STREAM_IMAGE_MAIN, pBuffer);
                checkStreamAndEncodeLocked(requestNo);
            }
        }
        //
        // debug
        MINT32 data = NSCamShot::ECamShot_DATA_MSG_NONE;
        //
        switch (streamId)
        {
            case eSTREAMID_IMAGE_PIPE_RAW_OPAQUE:
                data = NSCamShot::ECamShot_DATA_MSG_RAW;
                break;
            case eSTREAMID_IMAGE_PIPE_YUV_JPEG:
                data = NSCamShot::ECamShot_DATA_MSG_YUV;
                break;
            case eSTREAMID_IMAGE_PIPE_YUV_00:
                data = NSCamShot::ECamShot_DATA_MSG_POSTVIEW;
                break;
            case eSTREAMID_IMAGE_JPEG:
                data = NSCamShot::ECamShot_DATA_MSG_JPEG;
                break;
            default:
                data = NSCamShot::ECamShot_DATA_MSG_NONE;
                break;
        }
        //

        if( mDumpFlag & data )
        {
            int t;
            struct timespec ts;
            clock_gettime(CLOCK_MONOTONIC, &ts);
            t = (ts.tv_sec*1000+ts.tv_nsec/1000000);

            String8 filename = String8::format("%s/ZsdVendorShot_%dx%d",
                    DUMP_PATH, pBuffer->getImgSize().w, pBuffer->getImgSize().h);
            switch( data )
            {
                case NSCamShot::ECamShot_DATA_MSG_RAW:
                    filename += String8::format("_%zd_%d_%d.raw", pBuffer->getBufStridesInBytes(0), t, requestNo);
                    break;
                case NSCamShot::ECamShot_DATA_MSG_YUV:
                case NSCamShot::ECamShot_DATA_MSG_POSTVIEW:
                    filename += String8(".yuv");
                    break;
                case NSCamShot::ECamShot_DATA_MSG_JPEG:
                    filename += String8(".jpeg");
                    break;
                default:
                    break;
            }
            pBuffer->lockBuf(LOG_TAG, eBUFFER_USAGE_SW_READ_MASK);
            pBuffer->saveToFile(filename.string());
            pBuffer->unlockBuf(LOG_TAG);
            //
            MY_LOGD("dump buffer in %s", filename.string());
        }
    }
    CAM_TRACE_FMT_END();
}

/*******************************************************************************
*
********************************************************************************/
MERROR
ZsdVendorShot::
checkStreamAndEncodeLocked( MUINT32 const requestNo)
{
    FUNC_START;
    if ( ! mEncJob.isReady() )
    {
        FUNC_END;
        return NAME_NOT_FOUND;
    }
    //
    MY_LOGD("requestNo:%d Job is Ready",requestNo);
    //
    mvFutures.push_back(
        std::async(std::launch::async,
            [ requestNo, this ]( ) -> MERROR {
                MERROR err = postProcessing();
                if ( OK != err )
                {
                    FUNC_END;
                    return err;
                }

                //
                sp<IImageBufferHeap> dstBuffer;
                MUINT32              transform;
                mpJpegPool->acquireFromPool(
                                LOG_TAG,
                                0,
                                dstBuffer,
                                transform
                            );
                if ( ! dstBuffer.get() )
                    MY_LOGE("no destination");
                // encode jpeg here
                {
                    Mutex::Autolock _l(mEncJobLock);
                    mEncJob.setTarget(dstBuffer);
                    sp<ExifJpegUtils> pExifJpegUtils = ExifJpegUtils::createInstance(
                        getOpenId(), mEncJob.HalMetadata, mShotParam.mAppSetting,
                        mEncJob.mpDst, mEncJob.pSrc_main, mEncJob.pSrc_thumbnail );
                    if ( !pExifJpegUtils.get() ) {
                        MY_LOGE("create exif jpeg encode utils fail");
                        FUNC_END;
                        return -ENODEV;
                    }
                    err = pExifJpegUtils->execute();
                    if ( OK != err) {
                        MY_LOGE("Exif Jpeg encode utils: (%d)", err);
                        FUNC_END;
                        return err;
                    }
                }
                //
                sp<IImageBuffer> pBuf = dstBuffer->createImageBuffer();
                handleJpegData(pBuf.get());
                //clear result metadata
                {
                    Mutex::Autolock _l(mResultMetadataSetLock);
                    mResultMetadataSetMap.clear();
                }
                //
                mpJpegPool->releaseToPool(
                                LOG_TAG,
                                0,
                                dstBuffer,
                                0,
                                false
                                );
                //
                {
                    Mutex::Autolock _l(mEncJobLock);
                    mEncJobDone = MTRUE;
                    mEncDoneCond.broadcast();
                }
                FUNC_END;
                return err;
            }
        )
    );
    //
    FUNC_END;
    return OK;
}

/*******************************************************************************
*
********************************************************************************/
MERROR
ZsdVendorShot::
postProcessing()
{
    CAM_TRACE_NAME("Zsd vendor postProcessing");
    MY_LOGD("+");
    // MUST to fill final metadata/imagebuffer for encode
    // ex. fetch sources for post processing
    // for ( ssize_t i=0; i<mEncJob.mvSource.size(); i++)
    // {
    // //    mEncJob.pSrc_main =
    // //           GET_GRAY_IMAGE(mEncJob.mvSource[index].pSrc_main);
    // }
    //
    // ex. directly assign last source for encode
    // MY_LOGD("Ready to notify p2done & Shutter");
    this->onP2done();

    /* if using BackgroundService, wait syncToken before algorithm start */
    if (supportBackgroundService()) {
        MY_LOGI("wait syncToken (%" PRId64 ") [+]", getInstanceId());
        IDetachJobManager::getInstance()->waitTrigger(getInstanceId(), INT_MAX);
        MY_LOGI("wait syncToken (%" PRId64 ") [-]", getInstanceId());
    }

    MINT32 index = 0;//mEncJob.mSrcCnt - 1;

    //Forced big cores
    enableForceToBigCore();
    //
    mEncJob.HalMetadata      = mEncJob.mvSource[index].HalMetadata;
    mEncJob.AppMetadata      = mEncJob.mvSource[index].AppMetadata;
    mEncJob.pSrc_main        = mEncJob.mvSource[index].pSrc_main;
    mEncJob.pSrc_thumbnail   = mEncJob.mvSource[index].pSrc_thumbnail;
    MY_LOGD("-");
    return OK;
}

MERROR ZsdVendorShot::
postEeAnr(
        sp<IImageBuffer> img_src,
        sp<IImageBuffer> img_dst,
        IMetadata* pHalMeta,
        IMetadata* pAppMeta
)
{
    CAM_TRACE_CALL();

    /* determine ISP profile */
    EIspProfile_T profile = EIspProfile_Capture; // TODO: change ISP profile here
    MERROR err = OK;

    /* check buffers */
    if (img_src.get() == 0 || img_dst.get() == 0 || pHalMeta == 0 || pAppMeta == 0) {
        MY_LOGE("all argument cannot be NULL, img_src(%p), img_dst(%p), pHalMeta(%p), pAppMeta(%p)",
                img_src.get(), img_dst.get(), pHalMeta, pAppMeta);
        return INVALID_OPERATION;
    }
    else if (img_src == img_dst) {
        MY_LOGI("process by inplace buffer, img_src(%p), img_dst(%p)", img_src.get(), img_dst.get());
    }

    MetaSet_T metaset;
    metaset.halMeta = *pHalMeta;
    metaset.appMeta = *pAppMeta;

    int sensorId = getOpenId();
    IHal3A *pHal3A = NULL;
    INormalStream *pNormalStream = NULL;

    /* check resolution */
    {
        MSize size_src = img_src->getImgSize();
        MSize size_dst = img_dst->getImgSize();

        if (size_src != size_dst) {
            MY_LOGE("Resolution of images are not the same, src=%dx%d, dst=%dx%d",
                    size_src.w, size_src.h,
                    size_dst.w, size_dst.h);
            return BAD_VALUE;
        }
    }

    /**
     * P A S S 2
     */
    MY_LOGD("pass2 for post ee and anr");
    QParams qParams;
    FrameParams params;

    params.mStreamTag = NSCam::NSIoPipe::NSPostProc::ENormalStreamTag_Normal;

    // create Hal3A and NormalStream
    pHal3A = MAKE_Hal3A(sensorId, LOG_TAG);
    if (pHal3A == NULL) {
        MY_LOGE("create IHal3A fail");
        err = UNKNOWN_ERROR;
        goto lbExit;
    }

    pNormalStream = INormalStream::createInstance(sensorId);
    if (pNormalStream == NULL) {
        MY_LOGE("create INormalStream fail");
        err = UNKNOWN_ERROR;
        goto lbExit;
    }
    else {
        MBOOL bResult = pNormalStream->init(LOG_TAG);
        if (CC_UNLIKELY(bResult == MFALSE)) {
            MY_LOGE("init INormalStream returns MFALSE");
            err = UNKNOWN_ERROR;
            goto lbExit;
        }
    }

    /* execute pass 2 operation */
    {
        // get size of dip_x_reg_t
        volatile size_t regTableSize = pNormalStream->getRegTableSize();
        if (regTableSize <= 0) {
            MY_LOGE("unexpected tuning buffer size: %zu", regTableSize);
        }

        std::unique_ptr<char[]> tuning_reg(new char[regTableSize]());
        void *tuning_lsc;

        MY_LOGD("create tuning register data chunk with size %zu",
                regTableSize);

        TuningParam rTuningParam;
        rTuningParam.pRegBuf = tuning_reg.get();

        /* Enable NR for this P2 round */
        IMetadata::setEntry<MBOOL> (&metaset.halMeta, MTK_3A_ISP_DISABLE_NR, MFALSE);

        IMetadata::setEntry<MUINT8>(&metaset.halMeta, MTK_3A_PGN_ENABLE, MTRUE);
        IMetadata::setEntry<MUINT8>(&metaset.halMeta, MTK_3A_ISP_PROFILE, profile);

        ATRACE_BEGIN("setIsp");
        if (pHal3A->setIsp(0, metaset, &rTuningParam, nullptr) == 0) {
            MY_LOGD("get tuning data, reg=%p, lsc=%p",
                    rTuningParam.pRegBuf, rTuningParam.pLsc2Buf);

            params.mTuningData = static_cast<MVOID*>(tuning_reg.get()); // adding tuning data
        }
        else {
            MY_LOGE("set ISP profile failed...");
            err = UNKNOWN_ERROR;
            goto lbExit;
        }
        ATRACE_END(); // setIsp
        MY_LOGD("set isp done");

        ATRACE_BEGIN("BeforeP2Enque");
        // input: blended frame [IMGI port]
        {
            Input p;
            p.mPortID       = PORT_IMGI; // should be IMGI port
            p.mPortID.group = 0;
            p.mBuffer       = img_src.get();
            params.mvIn.push_back(p);

            /* source image cropping info */
            MCrpRsInfo crop;
            crop.mGroupID       = 1; // CRZ, group ID 1
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

        // output: a frame
        {
            Output p;
            p.mPortID       = PORT_WDMAO;
            p.mPortID.group = 0;
            p.mBuffer       = img_dst.get();
            params.mvOut.push_back(p);

            /* destination cropping info */
            MCrpRsInfo crop;
            crop.mGroupID       = 2; // WDMAO using group ID 2
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
        }

        Mutex       mutex;
        Condition   cond;

        struct MyCookie
        {
            Mutex       mtx;
            Condition   cond;
            int         fired;
            //
            MyCookie() : fired(0) {}
        } _cookie;


        auto _cb = [](QParams& param)
        {
            MyCookie* pCookie = static_cast<MyCookie*>(param.mpCookie);
            if (pCookie == nullptr) {
                CAM_LOGE("p2 callback but mpCookie is nullptr");
                return;
            }
            Mutex::Autolock lk( pCookie->mtx );
            pCookie->fired = 1;
            pCookie->cond.signal();
        };

        qParams.mpfnCallback = _cb;
        qParams.mpCookie = static_cast<void*>( &_cookie );

        // push QParams::FrameParams into QParams
        qParams.mvFrameParams.push_back(params);

        ATRACE_END(); // BeforeP2Enque
        ATRACE_BEGIN("Pass2Enque");
        MY_LOGD("pass2 enque");
        if (!pNormalStream->enque(qParams)) {
            MY_LOGE("pass 2 enque returns fail");
            err = UNKNOWN_ERROR;
            goto lbExit;
        }

        /* wait until P2 done */
        {
            status_t _status = OK;
            Mutex::Autolock lk( _cookie.mtx );
            if (_cookie.fired == 0) {
                /* wait 500 ms */
                _status = _cookie.cond.waitRelative( _cookie.mtx, (nsecs_t)500000000000LL );
            }
            /* check status */
            if (_status != OK ) {
                MY_LOGE("wait P2 deque failed, err=%d", _status);
            }
        }

        ATRACE_END(); //Pass2Enque

        MY_LOGD("pass2 dequed");
    }

lbExit:
    // destroy Hal3A and NormalStream
    if (pNormalStream) {
        pNormalStream->uninit(LOG_TAG);
        pNormalStream->destroyInstance();
        pNormalStream = NULL;
    }

    if (pHal3A) {
        pHal3A->destroyInstance(LOG_TAG);
        pHal3A = NULL;
    }
    return err;
}

/*******************************************************************************
*
********************************************************************************/
MVOID
ZsdVendorShot::
beginCapture(
    MINT32 rAllocateCount
)
{
    CAM_TRACE_CALL();
    //
    mu4Scenario = SENSOR_SCENARIO_ID_NORMAL_CAPTURE;
    mu4Bitdepth = 10;
    //
     HwInfoHelper helper(getOpenId());
     if( ! helper.updateInfos() ) {
         MY_LOGE("cannot properly update infos");
     }
     if( ! helper.getSensorSize( mu4Scenario, mSensorSize) ) {
         MY_LOGE("cannot get params about sensor");
     }
     //
     mJpegsize      = (mShotParam.mu4Transform & eTransform_ROT_90) ?
         MSize(mShotParam.mi4PictureHeight, mShotParam.mi4PictureWidth):
         MSize(mShotParam.mi4PictureWidth, mShotParam.mi4PictureHeight);

     MY_LOGD("transform=%d, jpegsize=%dx%d", mShotParam.mu4Transform, mJpegsize.w, mJpegsize.h);

    //
    // TODO add other consumer
    mpConsumer = IResourceContainer::getInstance(getOpenId())->queryConsumer( eSTREAMID_IMAGE_PIPE_RAW_OPAQUE );
    MY_LOGD("Query Consumer OpenID(%d) StreamID(%d)", getOpenId(), eSTREAMID_IMAGE_PIPE_RAW_OPAQUE);
    sp<StreamBufferProvider> pProvider = mpConsumer.promote();
    if( pProvider == NULL) {
        MY_LOGE("can't find StreamBufferProvider in ConsumerContainer");
    }
    //
    sp<IImageStreamInfo> pInfo = pProvider->queryImageStreamInfo();
    pProvider->updateBufferCount("ZsdVendorShot", pInfo->getMaxBufNum() + rAllocateCount);

    mBoostThread = std::async(std::launch::async, [this](){
        //mmdvfs
        {
            std::lock_guard<std::mutex> __oplocker(m_opScenarioCtrlMx);

            m_spScenarioCtrl = ZsdVendorShot::m_wpScenarioCtrl.promote();
            if( m_spScenarioCtrl.get() == nullptr )
            {
                MY_LOGW("no scenario Control exist, create a new one");
                ZsdVendorShot::m_wpScenarioCtrl
                    = new ScenarioCtrl(getOpenId(), mu4Scenario, IScenarioControl::Scenario_ContinuousShot);
                m_spScenarioCtrl = ZsdVendorShot::m_wpScenarioCtrl.promote();
            }
            else{
                MY_LOGD("keep ScenarioCtrl, ref count:%u", m_spScenarioCtrl->getStrongCount());
            }
        }
        //Cpu Boost
        if(mpCpuCtrl)
        {
            mpCpuCtrl->cpuPerformanceMode(3);
        }
   });
}

/*******************************************************************************
*
********************************************************************************/
MBOOL
ZsdVendorShot::
constructCapturePipeline(Vector<BufferItemSet>& rBufSet)
{
    CAM_TRACE_CALL();
    //
    //set yuv & jpeg format / size / buffer in createStreams()
    if ( ! createStreams() ) {
        MY_LOGE("createStreams failed");
        return MFALSE;
    }
    //
    mpConsumer = IResourceContainer::getInstance(getOpenId())->queryConsumer( eSTREAMID_IMAGE_PIPE_RAW_OPAQUE );
    sp<StreamBufferProvider> pProvider = mpConsumer.promote();
    CHECK_OBJECT(pProvider);
    //
    // zsd flow: query IImageStreamInfo of raw stream from provider
    Vector<sp<IImageStreamInfo>> rawInputInfos;
    pProvider->querySelector()->queryCollectImageStreamInfo( rawInputInfos, MTRUE );
    //
    for(size_t i = 0 ; i < rawInputInfos.size() ; i++)
    {
        MY_LOGD("zsd raw(%zu) stream %#" PRIx64 " (%s) size(%dx%d), fmt 0x%x",
            i,
            rawInputInfos[i]->getStreamId(),
            rawInputInfos[i]->getStreamName(),
            rawInputInfos[i]->getImgSize().w,
            rawInputInfos[i]->getImgSize().h,
            rawInputInfos[i]->getImgFormat()
           );
    }
    //
    mpJpegPool = new BufferPoolImp(mpInfo_Jpeg);
    mpJpegPool->allocateBuffer(LOG_TAG, mpInfo_Jpeg->getMaxBufNum(), mpInfo_Jpeg->getMinInitBufNum());
    // create new pipeline
    if ( ! createPipeline(rBufSet) ) {
        MY_LOGE("createPipeline failed");
        return MFALSE;
    }
    CHECK_OBJECT(mpPipeline);
    //
#if (MTKCAM_HAVE_COMMON_CAPTURE_PIPELINE_SUPPORT == 0)
    sp<ResultProcessor> pResultProcessor = mpPipeline->getResultProcessor().promote();
    CHECK_OBJECT(pResultProcessor);

    // metadata
    mpMetadataListener = new ZvMetadataListener(this);
    pResultProcessor->registerListener( 0, UINT_MAX, true, mpMetadataListener);
#endif

    return MTRUE;
}

ScenarioCtrl::ScenarioCtrl(
    const MINT32 openId,
    const MUINT32 sensorMode,
    const MINT32 scenario
)
    : m_openId(openId)
    , m_sensorMode(sensorMode)
    , m_scenario(scenario)
    , m_enterResult(MFALSE)
    , m_scenarioControl(nullptr)
{
    if( m_scenarioControl.get() == nullptr )
    {
        CAM_LOGD("no scenario Control exist, create a new one");
        m_scenarioControl = IScenarioControl::create(m_openId);

        if( CC_UNLIKELY(m_scenarioControl.get() == nullptr) ) {
            CAM_LOGE("get Scenario Control fail");
            return;
        }
        else {
            HwInfoHelper helper(m_openId);
            if( ! helper.updateInfos() ) {
                CAM_LOGE("cannot properly update infos");
                return;
            }
            MSize sensorSize;
            MINT32 sensorFps;

            // check sensor mode, for development stage, assert it if it's undefined
            if (CC_UNLIKELY( m_sensorMode == SENSOR_SCENARIO_ID_UNNAMED_START )) {
#if (MTKCAM_LOG_LEVEL_DEFAULT >= 3)
                CAM_LOGE("invalid sensor mode, assert");
                assert(0);
#else
                CAM_LOGW("invalid sensor mode, force set to capture sensor mode");
                m_sensorMode = SENSOR_SCENARIO_ID_NORMAL_CAPTURE;
#endif
            }

            if( ! helper.getSensorSize( m_sensorMode, sensorSize) ||
                ! helper.getSensorFps( m_sensorMode, sensorFps) ) {
                CAM_LOGE("cannot get params about sensor");
                return;
            }
            //
            IScenarioControl::ControlParam param;
            param.scenario   = m_scenario;
            param.sensorSize = sensorSize;
            param.sensorFps  = sensorFps;

            if (helper.getDualPDAFSupported(m_sensorMode)) {
                FEATURE_CFG_ENABLE_MASK(param.featureFlag,IScenarioControl::FEATURE_DUAL_PD);
            }

            m_scenarioControl->enterScenario(param);
            m_enterResult = MTRUE;
            CAM_LOGD("enter Scenario Control: %u, ref count:%u", param.scenario, m_scenarioControl->getStrongCount());
        }
    }
    else {
        CAM_LOGD("keep Scenario Control, ref count:%u", m_scenarioControl->getStrongCount());
    }
}


ScenarioCtrl::~ScenarioCtrl()
{
    if( m_scenarioControl.get() != nullptr ) {
        if (m_enterResult) {
            m_scenarioControl->enterScenario(IScenarioControl::Scenario_ZsdPreview);
            CAM_LOGD("resume Scenario Control: Scenario_ZsdPreview");
        }

        CAM_LOGD("release Scenario Control, ref count:%u", m_scenarioControl->getStrongCount());
        m_scenarioControl = nullptr;
        m_enterResult = MFALSE;
    }
}

/*******************************************************************************
*
********************************************************************************/
status_t
ZsdVendorShot::
returnSelectorBuffer(Vector<BufferItemSet>& rvBufferSet)
{
    status_t st = UNKNOWN_ERROR;
    mpConsumer = IResourceContainer::getInstance(getOpenId())->queryConsumer( eSTREAMID_IMAGE_PIPE_RAW_OPAQUE );
    sp<StreamBufferProvider> pConsumer = mpConsumer.promote();
    if(pConsumer != NULL) {
        sp< ISelector > pSelector = pConsumer->querySelector();
        if(pSelector != NULL) {
        for(size_t i = 0; i < rvBufferSet.size() ; i++)
            st = pSelector->returnBuffer(rvBufferSet.editItemAt(i));
        }
    }
    return st;
}


/*******************************************************************************
*
********************************************************************************/
size_t
ZsdVendorShot::
getFlashCaptureCount(
        const IMetadata*    appMeta     __attribute__((unused)),
        const IMetadata*    halMeta     __attribute__((unused))
)
{
    // TODO: time to decide frame number
    return 3;
}


/*******************************************************************************
*
********************************************************************************/
status_t
ZsdVendorShot::
runSubmitRequestAndGetSelectorDataFlow(
    MINT32 shotCount,
    Vector< SettingSet > &vSettings,
    MINT32 Cap_type
)
{
    MY_LOGD("runSubmitRequestAndGetSelectorDataFlow()");
    status_t status = OK;
    // get multiple raw buffer and send to capture pipeline
    MBOOL res = MTRUE;

    // requests have the same unique key
    MINT32 uniqueKey = static_cast<MINT32>(NSCam::Utils::TimeTool::getReadableTime());
    for ( MINT32 i = 0; i < shotCount; ++i ) {
        // get Selector Data (buffer & metadata)
        status = OK;
        Vector<BufferItemSet> rvBufferSet;
        IMetadata selectorAppMetadata; // app setting for this raw buffer. Ex.3A infomation
        status = getSelectorData(
                    selectorAppMetadata,
                    mShotParam.mHalSetting,
                    rvBufferSet
                );
        //
        if( status != OK ) {
            MY_LOGW("i(%d) getSelectorData Fail, status(%d), need submitRequest and getSelectorData again",i,status);
            //
            res = applyRawBufferSettings( vSettings, 1, 0, MFALSE);
            //
            if(res == MFALSE){
                MY_LOGE("applySettings Fail!");
                returnSelectorBuffer(rvBufferSet);
                endFailedCapture();
                return status;
            }
            //
            status = getSelectorData(
                        selectorAppMetadata,
                        mShotParam.mHalSetting,
                        rvBufferSet);
            if( status != OK && status != TIMED_OUT) {
                MY_LOGE("i(%d) getSelectorData Fail again, status(%d), capture fail!",i,status);
                returnSelectorBuffer(rvBufferSet);
                endFailedCapture();
                return status;
            }
        }
        //
        if( i==0 )
        {
            if ( !constructCapturePipeline(rvBufferSet) )
            {
                MY_LOGE("constructCapturePipeline fail.");
                endFailedCapture();
                return UNKNOWN_ERROR;
            }
        }
#if (MTKCAM_HAVE_COMMON_CAPTURE_PIPELINE_SUPPORT == 0)
        else
#endif
        {
            for(size_t i =0 ; i < rvBufferSet.size() ; i++)
            {
                rvBufferSet[i].log();
#if MTKCAM_HAVE_COMMON_CAPTURE_PIPELINE_SUPPORT
                sp<CallbackBufferPool> pPool = mpPipeline->queryBufferPool( rvBufferSet[i].id );
#else
                sp<CallbackBufferPool> pPool = mpCallbackHandler->queryBufferPool( rvBufferSet[i].id );
#endif
                if( pPool == NULL)
                {
                    MY_LOGE("query Pool Fail!");
                    return UNKNOWN_ERROR;
                }
                sp<IImageBuffer> pBuf = rvBufferSet[i].heap->createImageBuffer();
                if(pBuf == NULL)
                {
                    MY_LOGE("Can not build image buffer (0x%x),(%p)", rvBufferSet[i].id, rvBufferSet[i].heap.get());
                    return UNKNOWN_ERROR;
                }
                pPool->addBuffer(pBuf);
            }
        }
        //
        // TODO: modify capture setting here! [optional] ex. diff EV
        IMetadata appSetting = mShotParam.mAppSetting;
        IMetadata halSetting = mShotParam.mHalSetting;
        // update hal metadata
        UPDATE_ENTRY<MINT32>(halSetting, MTK_PIPELINE_UNIQUE_KEY, uniqueKey);
        // submit setting to capture pipeline
        if (OK != submitCaptureSetting(appSetting, halSetting) ) {
            MY_LOGE("Submit capture setting fail.");
            endFailedCapture();
            return UNKNOWN_ERROR;
        }
        //
        mCapReqNo++;
    }
    return OK;
}

/*******************************************************************************
*
********************************************************************************/
status_t
ZsdVendorShot::
runGetSelectorDataMultipleFlow(
    MINT32 shotCount,
    MINT32 Cap_type
)
{
    MY_LOGD("runGetSelectorDataMultipleFlow()");
    status_t status = OK;

    // get Selector Data (buffer & metadata)
    Vector<Vector<BufferItemSet>> rvBufferSets;
    Vector<IMetadata> rvSelectorAppMetadatas; // app setting for this raw buffer. Ex.3A infomation
    Vector<IMetadata> rvSelectorHalMetadatas; // app setting for this raw buffer. Ex.3A infomation
    rvSelectorAppMetadatas.resize(shotCount);
    rvSelectorHalMetadatas.resize(shotCount);
    rvBufferSets.resize(shotCount);
    for ( MINT32 i = 0; i < shotCount; ++i ) {
        rvSelectorHalMetadatas.editItemAt(i) = mShotParam.mHalSetting;
    }
    status = getSelectorDataMultiple(
                rvSelectorAppMetadatas,
                rvSelectorHalMetadatas,
                rvBufferSets,
                shotCount
            );

    // requests have the same unique key
    MINT32 uniqueKey = static_cast<MINT32>(NSCam::Utils::TimeTool::getReadableTime());

    for ( MINT32 i = 0; i < shotCount; ++i ) {
        if( i==0 )
        {
            if ( !constructCapturePipeline(rvBufferSets.editItemAt(i)) )
            {
                MY_LOGE("constructCapturePipeline fail.");
                endFailedCapture();
                return UNKNOWN_ERROR;
            }
        }
#if (MTKCAM_HAVE_COMMON_CAPTURE_PIPELINE_SUPPORT == 0)
        else
#endif
        {
            for(size_t j =0 ; j < rvBufferSets.editItemAt(i).size() ; j++)
            {
                String8 str = rvBufferSets.editItemAt(i).editItemAt(j).log();
#if MTKCAM_HAVE_COMMON_CAPTURE_PIPELINE_SUPPORT
                sp<CallbackBufferPool> pPool = mpPipeline->queryBufferPool( rvBufferSets.editItemAt(i).editItemAt(j).id );
#else
                sp<CallbackBufferPool> pPool = mpCallbackHandler->queryBufferPool( rvBufferSets.editItemAt(i).editItemAt(j).id );
#endif
                if( pPool == NULL)
                {
                    MY_LOGE("query Pool Fail!");
                    return UNKNOWN_ERROR;
                }
                sp<IImageBuffer> pBuf = rvBufferSets.editItemAt(i).editItemAt(j).heap->createImageBuffer();
                if(pBuf == NULL)
                {
                    MY_LOGE("Can not build image buffer (0x%x),(%p)", rvBufferSets.editItemAt(i).editItemAt(j).id, rvBufferSets.editItemAt(i).editItemAt(j).heap.get());
                    return UNKNOWN_ERROR;
                }
                pPool->addBuffer(pBuf);
            }
        }
        //
        // TODO: modify capture setting here! [optional] ex. diff EV
        IMetadata appSetting = rvSelectorAppMetadatas.editItemAt(i);
        IMetadata halSetting = rvSelectorHalMetadatas.editItemAt(i);
        // update hal metadata
        UPDATE_ENTRY<MINT32>(halSetting, MTK_PIPELINE_UNIQUE_KEY, uniqueKey);
        // submit setting to capture pipeline
        if (OK != submitCaptureSetting(appSetting, halSetting) ) {
            MY_LOGE("Submit capture setting fail.");
            endFailedCapture();
            return UNKNOWN_ERROR;
        }
        //
        mCapReqNo++;
    }
    //
    return OK;
}

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

#define LOG_TAG "MtkCam/Shot"
//
#include <mtkcam/utils/std/Log.h>
#include <mtkcam/def/common.h>
#include <mtkcam/utils/std/common.h>
#include <mtkcam/utils/std/JobQueue.h>
#include <mtkcam/utils/sys/MemoryInfo.h>
//

#include <mtkcam/aaa/IHal3A.h>
using namespace NS3Av3;
//
#include <mtkcam/drv/IHalSensor.h>
//
#include <mtkcam/middleware/v1/IShot.h>
//
#include <mtkcam/middleware/v1/camshot/_params.h>
//
#include <mtkcam/middleware/v1/LegacyPipeline/StreamId.h> // eSTREAMID_IMAGE_PIPE_RAW_OPAQUE
//
#include "inc/ImpShot.h"
//
#include <cutils/properties.h>
//
#include <mtkcam/utils/metadata/hal/mtk_platform_metadata_tag.h>
#include <camera_custom_capture_nr.h>
//
#include <mtkcam/pipeline/extension/IVendorManager.h>
//
#include <mtkcam/middleware/v1/camshot/CamShotUtils.h>
//
#include <mtkcam/feature/effectHalBase/ICallbackClientMgr.h>
#include <mtkcam/middleware/v1/camutils/IDetachJobManager.h>
//#include <camera_custom_capture_rwb.h>
//
//
#define SUPPORT_3A (1)
//
#if SUPPORT_3A
#include <mtkcam/aaa/IHal3A.h>
using namespace NS3Av3;
#include <isp_tuning/isp_tuning.h>  //EIspProfile_T, EOperMode_*
using namespace NSIspTuning;
#endif
//
#include <mtkcam/utils/hw/CamManager.h>
//
#include <mtkcam/utils/hw/HwInfoHelper.h>
#include <cutils/atomic.h>
//
#include <mutex>
#include <memory>
using namespace NSCamHW;
using namespace NSCam::Utils;
//
using namespace android;
using namespace NSShot;


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


////////////////////////////////////////////////////////////////////////////////
//  IShot
////////////////////////////////////////////////////////////////////////////////

/******************************************************************************
 *
 ******************************************************************************/
template<typename T>
void
updateSingleEntry(IMetadata* pMeta, MUINT32 const tag, T const val)
{
    IMetadata::IEntry entry(tag);
    entry.push_back(val, Type2Type<T>());
    pMeta->update(tag, entry);
}

/******************************************************************************
 *
 ******************************************************************************/
IShot::
IShot(sp<ImpShot> pImpShot)
    : mpImpShot(pImpShot)
{
}


/******************************************************************************
 *
 ******************************************************************************/
IShot::
~IShot()
{
    MY_LOGD("");
    mpImpShot->onDestroy();
    mpImpShot = NULL;
}


/******************************************************************************
 *
 ******************************************************************************/
char const*
IShot::
getShotName() const
{
    return  mpImpShot->getShotName();
}


/******************************************************************************
 *
 ******************************************************************************/
uint32_t
IShot::
getShotMode() const
{
    return  mpImpShot->getShotMode();
}


/******************************************************************************
 *
 ******************************************************************************/
EPipelineMode
IShot::
getPipelineMode() const
{
    return  mpImpShot->getPipelineMode();
}


/******************************************************************************
 *
 ******************************************************************************/
int32_t
IShot::
getOpenId() const
{
    return  mpImpShot->getOpenId();
}


/******************************************************************************
 *
 ******************************************************************************/
bool
IShot::
setCallback(sp<IShotCallback> pShotCallback)
{
    return  mpImpShot->setCallback(pShotCallback);
}


/******************************************************************************
*
*******************************************************************************/
bool
IShot::
isDataMsgEnabled(MINT32 const i4MsgTypes)
{
    return  mpImpShot->isDataMsgEnabled(i4MsgTypes);
}

/******************************************************************************
*
*******************************************************************************/
void
IShot::
enableDataMsg(MINT32 const i4MsgTypes)
{
    mpImpShot->enableDataMsg(i4MsgTypes);
}

/******************************************************************************
*
*******************************************************************************/
void
IShot::
disableDataMsg(MINT32 const i4MsgTypes)
{
    mpImpShot->disableDataMsg(i4MsgTypes);
}


/******************************************************************************
 *
 ******************************************************************************/
bool
IShot::
sendCommand(
    ECommand const  cmd,
    MUINTPTR const  arg1 /*= 0*/,
    uint32_t const  arg2 /*= 0*/,
    uint32_t const  arg3
)
{
    return  mpImpShot->sendCommand(cmd, arg1, arg2, arg3);
}


////////////////////////////////////////////////////////////////////////////////
//  ImpShot
////////////////////////////////////////////////////////////////////////////////


/******************************************************************************
 *
 ******************************************************************************/
std::atomic<int32_t>  ImpShot::sVendorManagerId(0xFF); // avoid starting from 0 since there're VendorManagers already exists (by sensor index)
ImpShot::
ImpShot(
    char const*const pszShotName,
    uint32_t const u4ShotMode,
    int32_t const i4OpenId
)
    : ms8ShotName(String8(pszShotName))
    , mu4ShotMode(u4ShotMode)
    , mi4OpenId(i4OpenId)
    , mPipelineMode(ePipelineMode_Null)
    , mpShotCallback(NULL)
    , mShotParam()
    , mJpegParam()
    , mi4DataMsgSet(0)
    , mPluginUser(plugin::CALLER_ID::CALLER_SHOT_GENERAL)
    , mVendorManagerId(0)
{
    mInstanceId = ICallbackClientMgr::generateTimestamp();
    /* acquireJobQueue while creating ImpShot */
    acquireJobQueue();
}


/******************************************************************************
 *
 ******************************************************************************/
ImpShot::
~ImpShot()
{
    MY_LOGD("+");
    /* release VendorManager if mVendorManagerId > 0 */
    if  ( mVendorManagerId > 0 ) {
        plugin::NSVendorManager::remove(getVendorManagerId());
    }
    /* unregister syncToken */
    IDetachJobManager::getInstance()->unregisterToken(getInstanceId());

    if  ( mpShotCallback != 0 ) {
        MY_LOGD("mpShotCallback.get(%p), mpShotCallback->getStrongCount(%d)", mpShotCallback.get(), mpShotCallback->getStrongCount());
    }
    MY_LOGD("-");
}


/******************************************************************************
 *
 ******************************************************************************/
bool
ImpShot::
setCallback(sp<IShotCallback>& rpShotCallback)
{
    MY_LOGD("+ rpShotCallback(%p), rpShotCallback->getStrongCount(%d)", rpShotCallback.get(), rpShotCallback->getStrongCount());
    mpShotCallback = rpShotCallback;
    return  (mpShotCallback != 0);
}

/******************************************************************************
 *
 ******************************************************************************/
void
ImpShot::
acquireJobQueue()
{
    static std::mutex       sResLock;
    static std::weak_ptr<
        NSCam::JobQueue<void()>
    >                       sResJobQ;

    // acquire resource from weak_ptr
    std::lock_guard<std::mutex> lk(sResLock);
    mMinorJobQueue = sResJobQ.lock();
    if (mMinorJobQueue.get() == nullptr) {
        mMinorJobQueue = std::shared_ptr< NSCam::JobQueue<void()> >(
                new NSCam::JobQueue<void()>("ImpShot_MinorJob")
                );
        sResJobQ = mMinorJobQueue;
    }
    return;
}


/******************************************************************************
 *
 ******************************************************************************/
bool
ImpShot::
setShotParam(void const* pParam, size_t const size)
{
    if  ( ! pParam )
    {
        MY_LOGE("Null pointer to ShotParam");
        return  false;
    }
    //
    if  ( size != sizeof(ShotParam) )
    {
        MY_LOGE("size[%zu] != sizeof(ShotParam)[%zu]; please fully build source codes", size, sizeof(ShotParam));
        return  false;
    }
    //
    mShotParam = *reinterpret_cast<ShotParam const*>(pParam);
    return  true;
}


/******************************************************************************
 *
 ******************************************************************************/
bool
ImpShot::
setJpegParam(void const* pParam, size_t const size)
{
    if  ( ! pParam )
    {
        MY_LOGE("Null pointer to JpegParam");
        return  false;
    }
    //
    if  ( size != sizeof(JpegParam) )
    {
        MY_LOGE("size[%zu] != sizeof(JpegParam)[%zu]; please fully build source codes", size, sizeof(JpegParam));
        return  false;
    }
    //
    mJpegParam = *reinterpret_cast<JpegParam const*>(pParam);
    return  true;
}


/******************************************************************************
 *
 ******************************************************************************/
bool
ImpShot::
sendCommand(
    uint32_t const  cmd,
    MUINTPTR const  arg1,
    uint32_t const  arg2,
    uint32_t const  arg3
)
{
    switch  (cmd)
    {
    //  This command is to set shot-related parameters.
    //
    //  Arguments:
    //      arg1
    //          [I] Pointer to ShotParam (i.e. ShotParam const*)
    //      arg2
    //          [I] sizeof(ShotParam)
    case eCmd_setShotParam:
        return  setShotParam(reinterpret_cast<void const*>(arg1), arg2);

    //  This command is to set jpeg-related parameters.
    //
    //  Arguments:
    //      arg1
    //          [I] Pointer to JpegParam (i.e. JpegParam const*)
    //      arg2
    //          [I] sizeof(JpegParam)
    case eCmd_setJpegParam:
        return  setJpegParam(reinterpret_cast<void const*>(arg1), arg2);

    //  This command is to set pipeline mode.
    //
    //  Arguments:
    //      arg2
    //          [I] pipeline mode
    case eCmd_setPipelineMode:
        mPipelineMode = static_cast<EPipelineMode>(arg2);
        return true;

    default:
        break;
    }
    MY_LOGW("Do nothing (cmd, arg1, arg2, arg3)=(%x, %#" PRIxPTR ", %u, %u)", cmd, arg1, arg2, arg3);
    return  false;
}


/******************************************************************************
*
*******************************************************************************/
bool
ImpShot::
isDataMsgEnabled(MINT32 const i4MsgTypes)
{
    return (i4MsgTypes & mi4DataMsgSet);
}

/******************************************************************************
*
*******************************************************************************/
void
ImpShot::
enableDataMsg(MINT32 const i4MsgTypes)
{
    ::android_atomic_or(i4MsgTypes, &mi4DataMsgSet);
}

/******************************************************************************
*
*******************************************************************************/
void
ImpShot::
disableDataMsg(MINT32 const i4MsgTypes)
{
    ::android_atomic_and(~i4MsgTypes, &mi4DataMsgSet);
}

/******************************************************************************
*
*******************************************************************************/
bool
ImpShot::
updatePostNRSetting(MINT32 nrtype, bool const isMfll, IMetadata* pMeta)
{
    NSCam::IHalSensorList* pHalSensorList = MAKE_HalSensorList();
    if( pHalSensorList->queryType(getOpenId()) == NSCam::NSSensorType::eYUV ) {
        // yuv sensor not support multi-pass NR
        updateSingleEntry<MINT32>(pMeta, MTK_NR_MODE, MTK_NR_MODE_OFF);
        return true;
    }
    MUINT const sensorDev = pHalSensorList->querySensorDevIdx(getOpenId());
    //
    int hw_threshold = -1;
    int swnr_threshold = -1;
    // get threshold from custom folder
    get_capture_nr_th(
            sensorDev, getShotMode(), isMfll,
            &hw_threshold, &swnr_threshold);
    //
    { // debug
        MINT32 type = property_get_int32("vendor.debug.shot.forcenr", -1);
        if( type != -1 ) {
            MY_LOGD("force use nrType: %d", type);
            nrtype = type;
        }
    }
    //
    MY_LOGD("shot mode(%d), nrtype(%d), threshold(%d/%d)",
            getShotMode(), nrtype, hw_threshold, swnr_threshold);
    //
    updateSingleEntry<MINT32>(pMeta, MTK_NR_MODE, nrtype);
    updateSingleEntry<MINT32>(pMeta, MTK_NR_MNR_THRESHOLD_ISO, hw_threshold);
    updateSingleEntry<MINT32>(pMeta, MTK_NR_SWNR_THRESHOLD_ISO, swnr_threshold);
    //
    return true;
}

/******************************************************************************
*
*******************************************************************************/
bool
ImpShot::
updatePostNRSetting(NRParam& param)
{
    NSCam::IHalSensorList* pHalSensorList = MAKE_HalSensorList();
    if( pHalSensorList->queryType(getOpenId()) == NSCam::NSSensorType::eYUV ) {
        // yuv sensor not support multi-pass NR
        updateSingleEntry<MINT32>(param.mpHalMeta, MTK_NR_MODE, MTK_NR_MODE_OFF);
        return true;
    }
    //
    if ( param.mFullRawSize.w == 0 || param.mFullRawSize.h == 0 ) {
        MY_LOGE("raw size not correct. %dx%d", param.mFullRawSize.w, param.mFullRawSize.h);
        return false;
    }
    //
    if ( param.mi4NRmode != -1 ) {
        updateSingleEntry<MINT32>(param.mpHalMeta, MTK_NR_MODE, param.mi4NRmode);
    }

    // if Vendor Manager ID is greater than 0, use customized ID.
    android::sp<NSCam::plugin::IVendorManager> pManager = [this](){
        if (getVendorManagerId() > 0)
            return NSCam::plugin::NSVendorManager::get(getVendorManagerId());

        return NSCam::plugin::NSVendorManager::get(getOpenId());
    }();

    if( !pManager.get() ) {
        MY_LOGE("null manager.");
        return false;
    }
    // set Input
    NSCam::plugin::InputInfo inputInfo;
    inputInfo.combination.push_back(MTK_PLUGIN_MODE_NR);
    inputInfo.halCtrl      = *(param.mpHalMeta);
    {
        MSize size        = param.mFullRawSize;
        MINT format       = 0; //not necessary here
        MUINT const usage = 0; //not necessary here
        MUINT32 transform = 0;
        inputInfo.fullRaw = NSCamShot::createImageStreamInfo(
                    "NR fullRaw",
                    0,
                    eSTREAMTYPE_IMAGE_INOUT,
                    1, 1,
                    usage, format, size, transform
                    );
        if( inputInfo.fullRaw == nullptr ) {
            return false;
        }
    }

    NSCam::plugin::OutputInfo outputInfo;
    pManager->get(param.mPluginUser, inputInfo, outputInfo);

    NSCam::plugin::InputSetting inputSetting;
    NSCam::plugin::FrameInfo info;
    info.frameNo = 0;
    *(param.mpHalMeta) += outputInfo.settings[0].setting.halMeta;

    // get plugin P2 combination first, and add necessary processor
    {
        typedef MINT64 T;
        T combination = 0;
        IMetadata::getEntry<T>(param.mpHalMeta, MTK_PLUGIN_P2_COMBINATION, combination);
        IMetadata::setEntry<T>(param.mpHalMeta, MTK_PLUGIN_P2_COMBINATION,
                combination | MTK_P2_ISP_PROCESSOR | MTK_P2_YUV_PROCESSOR |
                MTK_P2_MDP_PROCESSOR | MTK_P2_CAPTURE_REQUEST);
    }

    info.curHalControl = *(param.mpHalMeta);
    inputSetting.vFrame.push_back(info);
    //
    pManager->set(param.mPluginUser, inputSetting);

    return true;
}


bool
ImpShot::
insertRawPlugin(
        int64_t     pluginUser,
        uint64_t    pluginModeId,
        IMetadata*  pAppMeta,
        IMetadata*  pHalMeta,
        MSize       rawSize,
        int         rawStreamId
        )
{
#if MTKCAM_HAVE_IVENDOR_SUPPORT
    if (pluginModeId <= 0)
        return false;

    // if Vendor Manager ID is greater than 0, use customized ID.
    android::sp<NSCam::plugin::IVendorManager> pManager = [this](){
        if (getVendorManagerId() > 0)
            return NSCam::plugin::NSVendorManager::get(getVendorManagerId());

        return NSCam::plugin::NSVendorManager::get(getOpenId());
    }();

    if( !pManager.get() ) {
        MY_LOGE("null manager.");
        return false;
    }

    MY_LOGD("insertRawPlugin %#x", static_cast<uint32_t>(pluginModeId));

    NSCam::plugin::InputInfo inputInfo;

    inputInfo.appCtrl     = *(pAppMeta);
    inputInfo.halCtrl     = *(pHalMeta);
    {
        MSize size        = rawSize;
        MINT format       = NSCam::plugin::FORMAT_RAW;
        MUINT const usage = 0;
        MUINT32 transform = 0;

        // create IStreamInfo
        auto streamInfo = NSCamShot::createImageStreamInfo(
                "raw",
                rawStreamId,
                eSTREAMTYPE_IMAGE_INOUT,
                1, 1, usage, format, size, transform);

        switch (rawStreamId) {
        case eSTREAMID_IMAGE_PIPE_RAW_OPAQUE:
            inputInfo.fullRaw = streamInfo;
            break;
        case eSTREAMID_IMAGE_PIPE_RAW_RESIZER:
            inputInfo.resizedRaw = streamInfo;
            break;
        default:
            MY_LOGE("Not support RAW stream ID");
            return false;
        }
    }

    inputInfo.combination.push_back(pluginModeId);

    NSCam::plugin::OutputInfo outputInfo;
    pManager->get(pluginUser, inputInfo, outputInfo);

    NSCam::plugin::InputSetting inputSetting;
    NSCam::plugin::FrameInfo info;
    info.frameNo = 0;
    (*pAppMeta) += outputInfo.settings[0].setting.appMeta;
    (*pHalMeta) += outputInfo.settings[0].setting.halMeta;

    // get plugin P2 combination first
    {
        typedef MINT64 T;
        T combination = 0;
        IMetadata::getEntry<T>( pHalMeta, MTK_PLUGIN_P2_COMBINATION, combination);
        IMetadata::setEntry<T>( pHalMeta, MTK_PLUGIN_P2_COMBINATION,
                combination | MTK_P2_RAW_PROCESSOR );
    }

    info.curAppControl = *(pAppMeta);
    info.curHalControl = *(pHalMeta);
    inputSetting.vFrame.push_back(info);
    //
    pManager->set(pluginUser, inputSetting);
#else
    MY_LOGW("Not support if IVENDOR SUPPORT is 0");
#endif
    return true;
}

/******************************************************************************
 *
 ******************************************************************************/
template <typename T>
inline MVOID
updateEntry(
    IMetadata* pMetadata,
    MUINT32 const tag,
    T const& val
)
{
    if( pMetadata == NULL ) {
        //MY_LOGE("pMetadata == NULL");
        return;
    }

    IMetadata::IEntry entry(tag);
    entry.push_back(val, Type2Type<T>());
    pMetadata->update(tag, entry);
}


/******************************************************************************
 *
 ******************************************************************************/
bool
ImpShot::
setManualAEControl(IMetadata* pAppSetting, MUINT32 u4ExpTime, MUINT32 u4ExpGain, MUINT32 u4IspGain)
{
    MY_LOGD("setManualAEControl: u4ExpTime(%ld),u4ExpGain(%d),u4IspGain(%ld)", u4ExpTime, u4ExpGain, u4IspGain);
    //
    MINT64 isoBase = 100;
    //
    IHal3A* pHal3a = MAKE_Hal3A(getOpenId(), LOG_TAG);
    if ( ! pHal3a )
    {
        MY_LOGE("pHal3a == NULL");
        return MFALSE;
    }
    else
    {
        //
        FrameOutputParam_T RTParams;
        if ( ! pHal3a->send3ACtrl(NS3Av3::E3ACtrl_GetRTParamsInfo, reinterpret_cast<MINTPTR>(&RTParams), 0) )
        {
            MY_LOGE("E3ACtrl_GetRTParamsInfo fail");
        }
        else
        {
            isoBase = (MINT64)RTParams.u4RealISOValue* 1024 / (MINT64)RTParams.u4PreviewSensorGain_x1024* 1024 / (MINT64)RTParams.u4PreviewISPGain_x1024;
            MY_LOGD("setManualAEControl: isoBase(%" PRId64 "),u4RealISOValue(%ld),u4PreviewSensorGain_x1024(%ld),u4PreviewISPGain_x1024(%ld)",
                    isoBase, RTParams.u4RealISOValue, RTParams.u4PreviewSensorGain_x1024, RTParams.u4PreviewISPGain_x1024);
        }
        pHal3a->destroyInstance(LOG_TAG);
    }
    //
    {
        MUINT8 aeMode = MTK_CONTROL_AE_MODE_OFF;
        updateEntry<MUINT8>(pAppSetting , MTK_CONTROL_AE_MODE , aeMode);
        MY_LOGD("set MTK_CONTROL_AE_MODE (OFF)");
    }
    //
    {
        if((getShotMode()==eShotMode_EngShot)||(property_get_int32("debug.enable.normalAEB", 0)))
        {
            MINT32 sensorGain =( MINT32 )( ( MINT64 ) u4ExpGain*u4IspGain*isoBase/1024/1024);
            updateEntry<MINT32>(pAppSetting , MTK_SENSOR_SENSITIVITY , sensorGain);
            MY_LOGD("set MTK_SENSOR_SENSITIVITY (%d)", sensorGain);
        }
        else
        {
            updateEntry<MINT32>(pAppSetting , MTK_SENSOR_SENSITIVITY , u4ExpGain);
            MY_LOGD("set MTK_SENSOR_SENSITIVITY (%d)", u4ExpGain);
        }
    }
    //
    {
        MINT64 shutterSpeed = ((MINT64)u4ExpTime)*1000;
        updateEntry<MINT64>(pAppSetting , MTK_SENSOR_EXPOSURE_TIME , shutterSpeed);
        MY_LOGD("set MTK_SENSOR_EXPOSURE_TIME (%" PRId64 ")", shutterSpeed);
    }
    return MTRUE;
}

/******************************************************************************
*
*******************************************************************************/
int
ImpShot::
getShotRawBitDepth()
{
    //return getRawBitDepth();
    MINT32 bitDepth = 10;
    HwInfoHelper helper(getOpenId());
    if( ! helper.updateInfos() ) {
        MY_LOGE("cannot properly update infos");
        return bitDepth;
    }
    //
    helper.getRecommendRawBitDepth(bitDepth);
    return bitDepth;

}

/******************************************************************************
*
*******************************************************************************/
MBOOL
ImpShot::
supportBackgroundService() const
{
    return mShotParam.mbIsUsingBackgroundService;
}

/******************************************************************************
*
*******************************************************************************/
MBOOL
ImpShot::
onP2done()
{
    if (__builtin_expect( mpShotCallback.get() == nullptr, false )) {
        return MFALSE;
    }

    if (waitP2DoneReady()) {
        // If using BackgroundService, send MTK_CAMERA_MSG_EXT_NOTIFY_CAPTURE_DONE
        // while sending P2done too.
        return mpShotCallback->onCB_P2done(
                mShotParam.mbIsUsingBackgroundService ? 1 : 0
                );
    }

    return MTRUE;
}

/******************************************************************************
*
*******************************************************************************/
MBOOL
ImpShot::
waitP2DoneReady()
{
    constexpr const int64_t million     = 1024 * 1024;
    // Configurations
    constexpr const int64_t freeMemTh   = 500L * million; // 500MB, the free memory count we have at least.
    constexpr const size_t  sizeToCheck = 1; // if token size is greater than is value, check avaiable memory.
    constexpr const int     timeoutSec  = 10; // wait condition in 10 seconds, or an exception will be thrown
    constexpr const size_t  limitedSize = 5;  // if token size is greater than this value, need to wait until
                                              // token size <= limitedSize.

    // Check if BackgroundService disabled
    if ( ! mShotParam.mbIsUsingBackgroundService)
        return MTRUE;

    auto condition = [](size_t tokenSize)->bool
    {
        /* query available memory */
        int64_t avaiableMem = NSCam::NSMemoryInfo::getFreeMemorySize();

        CAM_LOGD("waitP2DoneReady: curr(size=%zu,mem=%d), threshold(size,mem)=(%zu, %d)",
                tokenSize, (int)(avaiableMem / million),
                sizeToCheck, (int)(freeMemTh / million)
                );

        /* 0. check token size, if exceeds limitedSize, need to wait */
        if (tokenSize >= limitedSize)
            return true;

        /* 1. if syncToken size is smaller than th, no need to wait */
        if (tokenSize <= sizeToCheck)
            return false;

        /* 2. if available memory is smaller than freeMemTh, need to wait */
        if (avaiableMem < freeMemTh) {
            CAM_LOGW("waitP2DoneReady: mem not enough, wait until enough...");
            return true;
        }

        /* Final: enough memory, no need to wait */
        return false;
    };

    /* Wait condition twice here.
     * 1. Wait with timeout 0 seconds, check if we have to "wait" or not.
     * 2. If #1 is yes, wait P2 done ready at mMinorJobQueue
     */
    std::cv_status status = std::cv_status::no_timeout;
    status = IDetachJobManager::getInstance()->waitTrigger(condition, 0);

    if (status == std::cv_status::timeout) {
        MY_LOGD("need to wait p2done in my minor thread");

        /* declare a job to wait P2 done ready and signal p2 done cb */
        auto _job =
            [condition, timeoutSec]
            (int64_t uid, sp<IShotCallback> cb, String8 caller) -> void
        {
            CAM_LOGD("waitP2DoneReady: wait p2done ready, uid=%" PRId64 " [+]", uid);
            auto __r = IDetachJobManager::getInstance()->waitTrigger(condition, timeoutSec*1000);
            CAM_LOGD("waitP2DoneReady: wait p2done ready, uid=%" PRId64 " [-]", uid);
            if (__builtin_expect( __r == std::cv_status::timeout, false )) {
                CAM_LOGF("waitP2DoneReady: wait condition timed on in %d secs, " \
                         "something wrong, invoke exception. caller is:%s",
                         timeoutSec,
                         caller.string()
                         );
                *(volatile uint32_t*)(0) = 0xdead10cc;
                return;
            }

            if (cb.get()) {
                // The first argument of onCB_P2done is:
                //  to send or not to send(as default) MTK_CAMERA_MSG_EXT_NOTIFY_CAPTURE_DONE.
                cb->onCB_P2done(1); // BackgroundService always send
            }

        };

        if (__builtin_expect( mMinorJobQueue.get() == nullptr, false )) {
            acquireJobQueue();
        }

        mMinorJobQueue->addJob( std::bind( _job, getInstanceId(), mpShotCallback, ms8ShotName ));
        return MFALSE; // wait failed
    }

    return MTRUE;
}


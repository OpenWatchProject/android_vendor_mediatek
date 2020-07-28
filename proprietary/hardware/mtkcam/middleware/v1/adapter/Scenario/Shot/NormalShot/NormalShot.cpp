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
#include <mtkcam/utils/std/Trace.h>
#include <mtkcam/def/common.h>
#include <cutils/properties.h>
//

//
#include <mtkcam/aaa/IHal3A.h>
//
#include <custom/aaa/AEPlinetable.h>
//
using namespace NS3Av3;
//
#include <isp_tuning/isp_tuning.h>  // EIspProfile_*
using namespace NSIspTuning;


//
#include <mtkcam/drv/IHalSensor.h>
//
#include <mtkcam/middleware/v1/camshot/ICamShot.h>
#include <mtkcam/middleware/v1/camshot/ISingleShot.h>
#include <mtkcam/middleware/v1/camshot/ISmartShot.h>

#include <mtkcam/middleware/v1/camutils/IDetachJobManager.h>
#include <mtkcam/feature/effectHalBase/ICallbackClientMgr.h>
//
#include <mtkcam/middleware/v1/IShot.h>
//
#define DNG_META_CB 1
//
#include "ImpShot.h"
#include "NormalShot.h"
//
#include <mtkcam/utils/hw/CamManager.h>
using namespace NSCam::Utils;
//
#include <mtkcam/utils/metadata/hal/mtk_platform_metadata_tag.h>
//
using namespace android;
using namespace NSShot;
//
#include <mtkcam/utils/hw/HwInfoHelper.h>
using namespace NSCamHW;

#include <mtkcam/pipeline/extension/IVendorManager.h>
//
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


/******************************************************************************
 *
 ******************************************************************************/
extern
sp<IShot>
createInstance_NormalShot(
    char const*const    pszShotName,
    uint32_t const      u4ShotMode,
    int32_t const       i4OpenId
)
{
    sp<IShot>       pShot = NULL;
    sp<NormalShot>  pImpShot = NULL;
    //
    //  (1.1) new Implementator.
    pImpShot = new NormalShot(pszShotName, u4ShotMode, i4OpenId);
    if  ( pImpShot == 0 ) {
        CAM_LOGE("[%s] new NormalShot", __FUNCTION__);
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
 *  This function is invoked when this object is firstly created.
 *  All resources can be allocated here.
 ******************************************************************************/
bool
NormalShot::
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
NormalShot::
onDestroy()
{
}


/******************************************************************************
 *
 ******************************************************************************/
NormalShot::
NormalShot(
    char const*const pszShotName,
    uint32_t const u4ShotMode,
    int32_t const i4OpenId
)
    : ImpShot(pszShotName, u4ShotMode, i4OpenId)
    , mbShotStoped(false)
    , mShotStopMtx()
{
    mpSingleShotDeleter = [](NSCamShot::ICamShot *p)
    {
        if (p)
            p->destroyInstance();
    };
}


/******************************************************************************
 *
 ******************************************************************************/
NormalShot::
~NormalShot()
{
}


/******************************************************************************
 *
 ******************************************************************************/
bool
NormalShot::
sendCommand(
    uint32_t const  cmd,
    MUINTPTR const  arg1,
    uint32_t const  arg2,
    uint32_t const  arg3
)
{
    CAM_TRACE_CALL();
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
NormalShot::
onCmd_reset()
{
    bool ret = true;
    return ret;
}

/******************************************************************************
 *
 ******************************************************************************/
bool
NormalShot::
onCmd_capture()
{
    CAM_TRACE_CALL();
    MBOOL ret = MTRUE;
    MBOOL isMfbShot = MFALSE;
    std::shared_ptr<NSCamShot::ICamShot> pSingleShot;
    {
        Mutex::Autolock lock(mShotStopMtx);

        if(isDataMsgEnabled(NSShot::EIShot_DATA_MSG_RAW))
        {
            MY_LOGD("%s :SingleShot", __FUNCTION__);
            pSingleShot = std::shared_ptr<NSCamShot::ICamShot>(
                    // constructor
                    NSCamShot::ISingleShot::createInstance(
                        static_cast<EShotMode>(mu4ShotMode),
                        "NormalShot"
                        ),
                    // deleter
                    mpSingleShotDeleter
                    );
        }
        else
        {
            pSingleShot = std::shared_ptr<NSCamShot::ICamShot>(
                    // constructor
                    NSCamShot::ISmartShot::createInstance(
                        static_cast<EShotMode>(mu4ShotMode),
                        "NormalShot",
                        getOpenId(),
                        mShotParam.mu4MultiFrameBlending,
                        &isMfbShot
                        ),
                    // deleter
                    mpSingleShotDeleter
                    );
        }
        //
        mpSingleShot = pSingleShot;
        //
        mWaitCreateShotLock.signal();
    }
    //
    pSingleShot->init();
    //
    pSingleShot->enableNotifyMsg( NSCamShot::ECamShot_NOTIFY_MSG_EOF );
    pSingleShot->enableNotifyMsg( NSCamShot::ECamShot_NOTIFY_MSG_P2DONE );
    //
    //[customize] 4-cell sensor update scenario sensor mode
    HwInfoHelper helper(getOpenId());
    if( ! helper.updateInfos() ) {
        MY_LOGE("cannot properly update infos");
        return BAD_VALUE;
    }
    //
    if((helper.get4CellSensorSupported()) && ((!mShotParam.mbEnableRrzoCapture)) || //4cell do P2done_callback before last img data callback
        mShotParam.mbNeedP1DoneCb)
    {
        MY_LOGD("enableNotifyMsg( NSCamShot::ECamShot_NOTIFY_MSG_P1DONE )");
        pSingleShot->enableNotifyMsg( NSCamShot::ECamShot_NOTIFY_MSG_P1DONE );
    }
    //
    if(mShotParam.mbNeedP2DoneCb)
    {
        MY_LOGD("enableNotifyMsg( NSCamShot::ECamShot_NOTIFY_MSG_P2DONE )");
        pSingleShot->enableNotifyMsg( NSCamShot::ECamShot_NOTIFY_MSG_P2DONE );
    }
    //
    EImageFormat ePostViewFmt =
        static_cast<EImageFormat>(mShotParam.miPostviewClientFormat);

    pSingleShot->enableDataMsg(
            NSCamShot::ECamShot_DATA_MSG_JPEG
            |((isDataMsgEnabled(NSShot::EIShot_DATA_MSG_RAW))?
            NSCamShot::ECamShot_DATA_MSG_RAW : NSCamShot::ECamShot_DATA_MSG_NONE)
#if DNG_META_CB
            |((isDataMsgEnabled(NSShot::EIShot_DATA_MSG_RAW))?
            NSCamShot::ECamShot_DATA_MSG_APPMETA : NSCamShot::ECamShot_DATA_MSG_NONE)
#endif
            | ((ePostViewFmt != eImgFmt_UNKNOWN) ?
             NSCamShot::ECamShot_DATA_MSG_POSTVIEW : NSCamShot::ECamShot_DATA_MSG_NONE)
            );

    // update LCS open in P1 or not
    MINT32 lcsOpen = mShotParam.mbEnableLtm;

    // manual exp time / sensor gain / iso control
    if(mu4ShotMode==eShotMode_EngShot)
    {
        if (mShotParam.muSensorSpeed != 0 && mShotParam.muSensorGain != 0)
        {
            ret = setManualAEControl(&mShotParam.mAppSetting, mShotParam.muSensorSpeed*1000, mShotParam.muSensorGain);
        }
    }
    else
    {
        if(mShotParam.mbEnableManualCapture)
        {
            if (mShotParam.muSensorSpeed != 0 && mShotParam.muRealIso != 0)
            {
                ret = setManualAEControl(&mShotParam.mAppSetting, mShotParam.muSensorSpeed*1000, mShotParam.muRealIso);
            }
            else
            {
                MY_LOGD("mbEnableManualCapture == TRUE, but muSensorSpeed(%d) or muRealIso(%d) == 0, it is not Manual Capture flow",
                    mShotParam.muSensorSpeed,mShotParam.muRealIso);
            }
        }
    }
    //EV breacket
    if (property_get_int32("debug.enable.normalAEB", 0))
    {
        strEvSetting evSetting;
        IHal3A* pHal3A = MAKE_Hal3A(getOpenId(), LOG_TAG);
        pHal3A->send3ACtrl(E3ACtrl_GetEvSetting, (MINT8)((mShotParam.miCurrentEV)/10), (MINTPTR)&evSetting); //D0Nenable, 0Ndisable
        MY_LOGD("send3ACtrl ev-value: %d ", (mShotParam.miCurrentEV/10));
        ret = setManualAEControl(&mShotParam.mAppSetting, evSetting.u4Eposuretime, evSetting.u4AfeGain, evSetting.u4IspGain);
        pHal3A->destroyInstance(LOG_TAG);
    }


    CamManager* pCamMgr = CamManager::getInstance();
    MUINT32 scenario = (( pCamMgr->isMultiDevice() && (pCamMgr->getFrameRate(getOpenId()) == 0) ) ||
                         (mShotParam.mbEnableRrzoCapture) ||
                         mShotParam.muSensorMode == SENSOR_SCENARIO_ID_NORMAL_PREVIEW)?
                         SENSOR_SCENARIO_ID_NORMAL_PREVIEW : SENSOR_SCENARIO_ID_NORMAL_CAPTURE;

    // sensor param
    NSCamShot::SensorParam rSensorParam(
            getOpenId(),                            //sensor idx
            scenario,                               //Scenaio
            getShotRawBitDepth(),                   //bit depth
            MFALSE,                                 //bypass delay
            MFALSE                                  //bypass scenario
            );

    MBOOL bBackgroundService = supportBackgroundService();
    if (bBackgroundService) {
        // add a sync token if using BackgroundService
        IMetadata::setEntry<MINT64>(
                &mShotParam.mHalSetting,
                MTK_PLUGIN_DETACT_JOB_SYNC_TOKEN,
                getInstanceId() );

        using namespace NSCam::plugin;

        // generate an unique VendorManager ID, and create a VendorManager
        genVendorManagerId();
        auto vmgr = IVendorManager::createInstance(LOG_TAG, getOpenId());
        NSVendorManager::add(getVendorManagerId(), vmgr);

        // start capturing, add token to job manager
        IDetachJobManager::getInstance()->registerToken( getInstanceId(), LOG_TAG );

        // extend SingleShot lifetime, correspond to NormalShot
        mpSingleShotLifeExtender = pSingleShot;
    }

    //
    NRParam nrParam;
    ret = pSingleShot->getRawSize(rSensorParam, nrParam.mFullRawSize);
    // update post nr settings
    nrParam.mpHalMeta    = &mShotParam.mHalSetting;
    nrParam.mPluginUser  = mPluginUser;
    updatePostNRSetting(nrParam);

    // shot param
    NSCamShot::ShotParam rShotParam(
            #if MTK_CAM_YUV420_JPEG_ENCODE_SUPPORT
            eImgFmt_NV21,                    //yuv format
            #else
            eImgFmt_YUY2,                    //yuv format
            #endif
            mShotParam.mi4PictureWidth,      //picutre width
            mShotParam.mi4PictureHeight,     //picture height
            mShotParam.mu4Transform,         //picture transform
            ePostViewFmt,                    //postview format
            mShotParam.mi4PostviewWidth,     //postview width
            mShotParam.mi4PostviewHeight,    //postview height
            0,                               //postview transform
            mShotParam.mu4ZoomRatio,         //zoom
            mShotParam.mAppSetting,
            mShotParam.mHalSetting,
            (lcsOpen > 0),                    // P1 open LCS to enqueue to LCE
            (mShotParam.mbEnableRrzoCapture),
            mShotParam.mbNeedP1DoneCb,
            mShotParam.mbNeedP2DoneCb
            );

    if (bBackgroundService) {
        // update NSCamShot::ShotParam..
        //
        // since NormalShot is using virtual inheritance of RefBase,
        // we MUST use dynamic_cast, or stack mapping would be wrong.
        rShotParam.extParamBGserv.impShot = static_cast<void*>(
                dynamic_cast<android::RefBase*>(this)
                );
        rShotParam.extParamBGserv.vendorManagerId = getVendorManagerId();
        rShotParam.extParamBGserv.instanceId      = getInstanceId();
    }

    // jpeg param
    NSCamShot::JpegParam rJpegParam(
            NSCamShot::ThumbnailParam(
                mJpegParam.mi4JpegThumbWidth,
                mJpegParam.mi4JpegThumbHeight,
                mJpegParam.mu4JpegThumbQuality,
                MTRUE),
            mJpegParam.mu4JpegQuality,         //Quality
            MFALSE                             //isSOI
            );

    //
    pSingleShot->setCallbacks(fgCamShotNotifyCb, fgCamShotDataCb, this);
    //
    ret = pSingleShot->setShotParam(rShotParam);
    //
    ret = pSingleShot->setJpegParam(rJpegParam);
    //
    std::future<MBOOL> _futureStop;
    {
        Mutex::Autolock lock(mShotStopMtx);
        if(mbShotStoped)
        {
            auto stopT = [this, pSingleShot] () {
                MY_LOGD("run mpSingleShot->stop() Thread +");
                MY_LOGD("run mpSingleShot->stop()");
                pSingleShot->stop();
                MY_LOGD("run mpSingleShot->stop() Thread -");
                return MTRUE;
            };
            _futureStop = std::async(std::launch::async, stopT);
        }
    }
    //
    ret = pSingleShot->startOne(rSensorParam);

    // If using BackgroundService, no need to uninit or wait
    if ( ! bBackgroundService) {
        ret = pSingleShot->uninit();
        //
        if (_futureStop.valid())
        {
            MY_LOGD("wait _futureStop done +");
            _futureStop.wait();
            MY_LOGD("wait _futureStop done -");
        }
    }
    //
    return ret;
}


/******************************************************************************
 *
 ******************************************************************************/
void
NormalShot::
onCmd_cancel()
{
    MY_LOGD("+ ");

    /* no need to cancel */
    if (supportBackgroundService()) {
        MY_LOGD("- ");
        return;
    }

    //
    bool shotStoped = false;
    {
        Mutex::Autolock lock(mShotStopMtx);
        if(!mbShotStoped)
        {
            mbShotStoped = false;
            shotStoped = true;
        }
    }
    //
    if(shotStoped)
    {
        auto pSingleShot = mpSingleShot.lock(); // acquire resource

        // if pSingleShot is not ready, wait for 2 seconds
        if( pSingleShot == NULL )
        {
            Mutex::Autolock lock(mShotStopMtx);
            nsecs_t timeout = 2000000000LL; // wait for most 2 secs
            MY_LOGD("mpSingleShot is NULL, need to wait (%" PRIx64 ") ns +", timeout);
            mWaitCreateShotLock.waitRelative(mShotStopMtx,timeout);
            MY_LOGD("mpSingleShot is NULL, need to wait (%" PRIx64 ") ns -", timeout);
            // acquire again
            pSingleShot = mpSingleShot.lock();
        }
        //
        if(pSingleShot != NULL)
        {
            MY_LOGD("run mpSingleShot->stop()");
            pSingleShot->stop();
        }
        else
        {
            MY_LOGI("mpSingleShot not exist, set mbShotStoped = true");
            Mutex::Autolock lock(mShotStopMtx);
            mbShotStoped = true;
        }
    }
    //
    MY_LOGD("- ");
}


/******************************************************************************
 *
 ******************************************************************************/
MBOOL
NormalShot::
fgCamShotNotifyCb(MVOID* user, NSCamShot::CamShotNotifyInfo const msg)
{
    CAM_TRACE_CALL();
    NormalShot *pNormalShot = reinterpret_cast <NormalShot *>(user);
    if (NULL != pNormalShot)
    {
        if ( NSCamShot::ECamShot_NOTIFY_MSG_EOF == msg.msgType)
        {
            pNormalShot->mpShotCallback->onCB_Shutter(true,
                                                      0,
                                                     pNormalShot->getShotMode());
        }
        if ( NSCamShot::ECamShot_NOTIFY_MSG_P1DONE == msg.msgType)
        {
            pNormalShot->mpShotCallback->onCB_P1done();
        }
        if ( NSCamShot::ECamShot_NOTIFY_MSG_P2DONE == msg.msgType)
        {

            pNormalShot->onP2done();
        }
    }
    return MTRUE;
}


/******************************************************************************
 *
 ******************************************************************************/
MBOOL
NormalShot::
fgCamShotDataCb(MVOID* user, NSCamShot::CamShotDataInfo const msg)
{
    CAM_TRACE_CALL();
    NormalShot *pNormalShot = reinterpret_cast<NormalShot *>(user);
    if (NULL != pNormalShot)
    {
        if (NSCamShot::ECamShot_DATA_MSG_POSTVIEW == msg.msgType)
        {
            pNormalShot->handlePostViewData( msg.pBuffer );
        }
        else if (NSCamShot::ECamShot_DATA_MSG_JPEG == msg.msgType)
        {
            pNormalShot->handleJpegData(msg.pBuffer);
        }
#if DNG_META_CB
        else if (NSCamShot::ECamShot_DATA_MSG_RAW == msg.msgType)//raw data callback to adapter
        {
            msg.pBuffer->lockBuf(LOG_TAG, eBUFFER_USAGE_SW_READ_MASK);
            pNormalShot->handleRaw16CB(msg.pBuffer);
            msg.pBuffer->unlockBuf(LOG_TAG);
        }
        else if (NSCamShot::ECamShot_DATA_MSG_APPMETA == msg.msgType)//raw data callback to adapter
        {
            pNormalShot->handleDNGMetaCB(msg.ext1);//metadata structure
        }
#endif
    }

    return MTRUE;
}


/******************************************************************************
*
*******************************************************************************/
MBOOL
NormalShot::
handlePostViewData(IImageBuffer *pImgBuffer)
{
    CAM_TRACE_CALL();
#if 1
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
    } _local(pImgBuffer);

    mpShotCallback->onCB_PostviewClient(
        getInstanceId(),
        pImgBuffer,
        0, // never ask for offset
        supportBackgroundService()
        );

    MY_LOGD("-");
#endif
    return  MTRUE;
}

/******************************************************************************
*
*******************************************************************************/
MBOOL
NormalShot::
handleRaw16CB(IImageBuffer const *pImgBuffer)
{
    MY_LOGD("+ (pImgBuffer) = (%p)", pImgBuffer);
    mpShotCallback->onCB_Raw16Image(reinterpret_cast<IImageBuffer const*>(pImgBuffer));
    return  MTRUE;
}

/******************************************************************************
*
*******************************************************************************/
MBOOL
NormalShot::
handleDNGMetaCB(MUINTPTR const dngMetadata)
{
    mpShotCallback->onCB_DNGMetaData(dngMetadata);
    return  MTRUE;
}

/******************************************************************************
*
*******************************************************************************/
MBOOL
NormalShot::
handleJpegData(IImageBuffer* pJpeg)
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
    MY_LOGD("-");

    return MTRUE;
}



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
//


//
#include <mtkcam/aaa/IHal3A.h>
using namespace NS3Av3;
//
#include <mtkcam/drv/IHalSensor.h>
//
#include <mtkcam/middleware/v1/camshot/ICamShot.h>
#include <mtkcam/middleware/v1/camshot/IMultiShot.h>
//
#include <mtkcam/utils/exif/IBaseCamExif.h>
//
#include <mtkcam/middleware/v1/IShot.h>
//
#include "ImpShot.h"
#include "ContinuousShot.h"
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


#define MAX_JPEG_QUALITY  (90)
/******************************************************************************
 *
 ******************************************************************************/
extern
sp<IShot>
createInstance_ContinuousShot(
    char const*const    pszShotName,
    uint32_t const      u4ShotMode,
    int32_t const       i4OpenId
)
{
    sp<IShot>       pShot = NULL;
    sp<ContinuousShot>  pImpShot = NULL;
    //
    //  (1.1) new Implementator.
    pImpShot = new ContinuousShot(pszShotName, u4ShotMode, i4OpenId);
    if  ( pImpShot == 0 ) {
        CAM_LOGE("[%s] new ContinuousShot", __FUNCTION__);
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
ContinuousShot::
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
ContinuousShot::
onDestroy()
{
}


/******************************************************************************
 *
 ******************************************************************************/
ContinuousShot::
ContinuousShot(
    char const*const pszShotName,
    uint32_t const u4ShotMode,
    int32_t const i4OpenId
)
    : ImpShot(pszShotName, u4ShotMode, i4OpenId)
    , mpMultiShot(NULL)
    , mu4ShotConut(0)
    , mbLastImage(false)
    , mbShotStoped(false)
    , mShotStopMtx()
    , semMShotEnd()
    , mCallbackMtx()
    , mbCbShutterMsg(true)
    , mbNormalCShot(MFALSE)
{
}


/******************************************************************************
 *
 ******************************************************************************/
ContinuousShot::
~ContinuousShot()
{
}


/******************************************************************************
 *
 ******************************************************************************/
bool
ContinuousShot::
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
    //  This command is to perform set continuous shot speed.
    //
    //  Arguments:
    //          N/A
    case eCmd_setCShotSpeed:
        ret = onCmd_setCShotSpeed(arg1);
        break;
    //  This command is to perform set preview metadata.
    //
    //  Arguments:
    //      arg1
    //          [I] Pointer to PreviewMetadata (i.e. PreviewMetadata const*)
    case eCmd_setPreviewMetadata:
        ret = onCmd_setPreviewMetadata(reinterpret_cast<void const*>(arg1));
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
ContinuousShot::
onCmd_reset()
{
    bool ret = true;
    return ret;
}

/******************************************************************************
 *
 ******************************************************************************/
bool
ContinuousShot::
onCmd_capture()
{
    MY_LOGD("+ ");
    MBOOL ret = true;
    mbCbShutterMsg = true;

    {
        Mutex::Autolock lock(mShotStopMtx);
        mpMultiShot = NSCamShot::IMultiShot::createInstance(static_cast<EShotMode>(mu4ShotMode), "ContinuousShot");
        mWaitCreateShotLock.signal();
        //
        if(mbShotStoped)
        {
            MY_LOGD("run MultiShot->stop()");
            mpMultiShot->stop();
        }
    }

    mpMultiShot->init();

    mpMultiShot->enableNotifyMsg(NSCamShot::ECamShot_NOTIFY_MSG_EOF      |
                                 NSCamShot::ECamShot_NOTIFY_MSG_SHOTS_END );

    EImageFormat ePostViewFmt = static_cast<EImageFormat>(mShotParam.miPostviewDisplayFormat);

    mpMultiShot->enableDataMsg(NSCamShot::ECamShot_DATA_MSG_JPEG);

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
            mShotParam.mbEnableLtm           // P1 open LCS to enqueue to LCE
            );

    if( mJpegParam.mu4JpegQuality > MAX_JPEG_QUALITY ) {
            MY_LOGW("limit jpeg quality from %d to %d", mJpegParam.mu4JpegQuality, MAX_JPEG_QUALITY);
            mJpegParam.mu4JpegQuality = MAX_JPEG_QUALITY;
    }

    // jpeg param
    NSCamShot::JpegParam rJpegParam(
            NSCamShot::ThumbnailParam(
                mJpegParam.mi4JpegThumbWidth,
                mJpegParam.mi4JpegThumbHeight,
                mJpegParam.mu4JpegThumbQuality,
                MTRUE),
            mJpegParam.mu4JpegQuality,       //Quality
            MFALSE                           //isSOI
            );

    // sensor param
    NSCamShot::SensorParam rSensorParam(
        getOpenId(),                             //sensor idx
        SENSOR_SCENARIO_ID_NORMAL_CAPTURE,       //Scenaio
        getShotRawBitDepth(),                    //bit depth
        MFALSE,                                  //bypass delay
        MFALSE                                   //bypass scenario
        );
    //
    mpMultiShot->setCallbacks(fgCamShotNotifyCb, fgCamShotDataCb, this);
    //
    ret = mpMultiShot->setShotParam(rShotParam);
    //
    ret = mpMultiShot->setJpegParam(rJpegParam);
    //
    if( mbNormalCShot )
    {
        ret = mpMultiShot->sendCommand( NSCamShot::ECamShot_CMD_SET_PRV_META, 0, 0, 0, (MVOID*)&mPreviewMetadata );
    }
    //
    mu4ShotConut = 0;
    ::sem_init(&semMShotEnd, 0, 0);
    //
    ret = ret & mpMultiShot->start(rSensorParam, mShotParam.mu4ShotCount);

    ::sem_wait(&semMShotEnd);

    {
        Mutex::Autolock lock(mShotStopMtx);
        mbShotStoped = true;
        //
        ret = mpMultiShot->uninit();
        //
        mpMultiShot->destroyInstance();
        //
        mpMultiShot = NULL;
    }

    MY_LOGD("- ");
    return ret;
}


/******************************************************************************
 *
 ******************************************************************************/
void
ContinuousShot::
onCmd_cancel()
{
    MY_LOGD("+ ");
    //
    Mutex::Autolock lock(mShotStopMtx);
    if(!mbShotStoped)
    {
        if( mpMultiShot == NULL )
        {
            nsecs_t timeout = 3000000000LL; // wait for most 3 secs
            MY_LOGD("mpMultiShot is NULL, need to wait (%lld) ns +",timeout);
            mWaitCreateShotLock.waitRelative(mShotStopMtx,timeout);
            MY_LOGD("mpMultiShot is NULL, need to wait (%lld) ns -",timeout);
        }
        //
        if(mpMultiShot != NULL)
        {
            MY_LOGD("run MultiShot->stop()");
            mpMultiShot->stop();
        }
        else
        {
            MY_LOGD("MultiShot not exist, set mbShotStoped = true");
        }
        mbShotStoped = true;
    }
    //
    MY_LOGD("- ");
}

/******************************************************************************
 *
 ******************************************************************************/
bool
ContinuousShot::
onCmd_setCShotSpeed(uint32_t u4CShotSpeed)
{
    bool ret = true;
    Mutex::Autolock lock(mShotStopMtx);
    if(!mbShotStoped && mpMultiShot!=NULL)
    {
        MY_LOGD("set continuous shot speed: %d", u4CShotSpeed);
        ret = mpMultiShot->sendCommand(NSCamShot::ECamShot_CMD_SET_CSHOT_SPEED, u4CShotSpeed, 0, 0);
    }
    return ret;
}


/******************************************************************************
 *
 ******************************************************************************/
bool
ContinuousShot::
onCmd_setPreviewMetadata(void const* pParam)
{
    if  ( ! pParam )
    {
        MY_LOGE("Null pointer to PreviewMetadata");
        return  false;
    }
    //
    MY_LOGD("onCmd_setPreviewMetadata");
    mPreviewMetadata.mPreviewAppSetting = (*reinterpret_cast<PreviewMetadata const*>(pParam)).mPreviewAppSetting;
    mPreviewMetadata.mPreviewHalSetting = (*reinterpret_cast<PreviewMetadata const*>(pParam)).mPreviewHalSetting;
    mbNormalCShot = MTRUE;
    return  true;
}


/******************************************************************************
 *
 ******************************************************************************/
MBOOL
ContinuousShot::
fgCamShotNotifyCb(MVOID* user, NSCamShot::CamShotNotifyInfo const msg)
{
    ContinuousShot *pContinuousShot = reinterpret_cast <ContinuousShot *>(user);
    if (NULL != pContinuousShot)
    {
        pContinuousShot->handleNotifyCb(msg);
    }

    return MTRUE;
}
//[CS]+
/******************************************************************************
 *
 ******************************************************************************/
MBOOL
ContinuousShot::handleNotifyCb(NSCamShot::CamShotNotifyInfo const msg)
{
    Mutex::Autolock lock(mCallbackMtx);
    MY_LOGD("+ (msgType, ext1, ext2), (%d, %d, %d)", msg.msgType, msg.ext1, msg.ext2);

    switch(msg.msgType)
    {
        case NSCamShot::ECamShot_NOTIFY_MSG_EOF:
            if(mbCbShutterMsg)
            {
                mbCbShutterMsg = false;
                mpShotCallback->onCB_Shutter(false, mu4ShotConut, mu4ShotMode);
            }
            break;

        case NSCamShot::ECamShot_NOTIFY_MSG_SHOTS_END:
            mbLastImage = true;
            break;

        default:
            break;
    }

   return MTRUE;
}
//[CS]-


/******************************************************************************
 *
 ******************************************************************************/
MBOOL
ContinuousShot::
fgCamShotDataCb(MVOID* user, NSCamShot::CamShotDataInfo const msg)
{
    ContinuousShot *pContinuousShot = reinterpret_cast<ContinuousShot *>(user);
    if (NULL != pContinuousShot)
    {
        if (NSCamShot::ECamShot_DATA_MSG_POSTVIEW == msg.msgType)
        {
            //pContinuousShot->handlePostViewData( msg.puData, msg.u4Size);
        }
        else if (NSCamShot::ECamShot_DATA_MSG_JPEG == msg.msgType)
        {
            pContinuousShot->handleJpegData((IImageBuffer*)msg.pBuffer);
        }
    }

    return MTRUE;
}


/******************************************************************************
*
*******************************************************************************/
MBOOL
ContinuousShot::
handlePostViewData(MUINT8* const puBuf, MUINT32 const u4Size)
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
ContinuousShot::
handleJpegData(IImageBuffer* pJpeg)
{
    Mutex::Autolock lock(mCallbackMtx);
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
    mu4ShotConut++; // shot count from 1

    uint8_t const* puJpegBuf = (uint8_t const*)pJpeg->getBufVA(0);
    MUINT32 u4JpegSize = pJpeg->getBitstreamSize();
    MY_LOGD("+ (puJpgBuf, jpgSize) = (%p, %d), shot count:%d",
            puJpegBuf, u4JpegSize, mu4ShotConut);

    // dummy raw callback
    mpShotCallback->onCB_RawImage(0, 0, NULL);

    // Jpeg callback
    mpShotCallback->onCB_CompressedImage_packed( 0,
                                                 u4JpegSize,
                                                 puJpegBuf,
                                                 mu4ShotConut,            //callback index
                                                 mbLastImage,              //final image
                                                 MTK_CAMERA_MSG_EXT_DATA_COMPRESSED_IMAGE,
                                                 mu4ShotMode
                                                 );

    if(mbLastImage)
    {
        MY_LOGD("CShot end, post end sem");
        ::sem_post(&semMShotEnd);
    }

    MY_LOGD("-");

    return MTRUE;
}



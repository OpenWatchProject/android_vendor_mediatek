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
#include <mtkcam/drv/IHalSensor.h>
//
#include <mtkcam/middleware/v1/camshot/ICamShot.h>
#include <mtkcam/middleware/v1/camshot/ISingleShot.h>
#include <mtkcam/middleware/v1/camshot/ISmartShot.h>
//
#include <mtkcam/middleware/v1/IShot.h>
//
#include "ImpShot.h"
#include "VssShot.h"
//
#include <mtkcam/utils/hw/CamManager.h>
using namespace NSCam::Utils;
//
#include <mtkcam/middleware/v1/LegacyPipeline/buffer/StreamBufferProvider.h>
#include <mtkcam/middleware/v1/LegacyPipeline/IResourceContainer.h>
#include <mtkcam/middleware/v1/LegacyPipeline/StreamId.h>
#include <mtkcam/middleware/v1/camshot/_params.h>
//
#include <mtkcam/utils/metadata/hal/mtk_platform_metadata_tag.h>
//
#define FEATURE_MODIFY (1)
//
#if FEATURE_MODIFY
#include <mtkcam/feature/utils/FeatureProfileHelper.h>
#endif // FEATURE_MODIFY
//
#include <mtkcam/utils/hw/HwInfoHelper.h>
//
#include <mtkcam/aaa/IHal3A.h>
//
using namespace NSCamHW;
//
using namespace android;
using namespace NSShot;
using namespace NSCam::v1;
using namespace NS3Av3;

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
extern
sp<IShot>
createInstance_VssShot(
    char const*const    pszShotName,
    uint32_t const      u4ShotMode,
    int32_t const       i4OpenId,
    uint32_t const      u4VHDRMode,
    bool const          bIsAutoVHDR
)
{
    sp<IShot>       pShot = NULL;
    sp<VssShot>  pImpShot = NULL;
    //
    //  (1.1) new Implementator.
    pImpShot = new VssShot(pszShotName, u4ShotMode, i4OpenId, u4VHDRMode, bIsAutoVHDR);
    if  ( pImpShot == 0 ) {
        CAM_LOGE("[%s] new VssShot", __FUNCTION__);
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
VssShot::
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
VssShot::
onDestroy()
{

}


/******************************************************************************
 *
 ******************************************************************************/
VssShot::
VssShot(
    char const*const pszShotName,
    uint32_t const u4ShotMode,
    int32_t const i4OpenId,
    uint32_t const u4VHDRMode,
    bool const bIsAutoHDR
)
    : ImpShot(pszShotName, u4ShotMode, i4OpenId),
    mVHDRMode(u4VHDRMode),
    mIsAutoHDR(bIsAutoHDR)
{
}


/******************************************************************************
 *
 ******************************************************************************/
VssShot::
~VssShot()
{
}


/******************************************************************************
 *
 ******************************************************************************/
bool
VssShot::
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
VssShot::
onCmd_reset()
{
    bool ret = true;
    return ret;
}


/******************************************************************************
 *
 ******************************************************************************/
bool
VssShot::
onCmd_capture()
{
    MBOOL ret = MTRUE;
    MBOOL isMfbShot = MFALSE;
    NSCamShot::ICamShot *pSingleShot = NSCamShot::ISmartShot::createInstance(static_cast<EShotMode>(mu4ShotMode)
                                                                            , "VssShot"
                                                                            , getOpenId()
                                                                            , mShotParam.mu4MultiFrameBlending
                                                                            , &isMfbShot
                                                                            , getPipelineMode()
                                                                            );
    //
    pSingleShot->init();
    //
    pSingleShot->enableNotifyMsg( NSCamShot::ECamShot_NOTIFY_MSG_EOF );
    //
    EImageFormat ePostViewFmt =
        static_cast<EImageFormat>(mShotParam.miPostviewDisplayFormat);

    pSingleShot->enableDataMsg(
            NSCamShot::ECamShot_DATA_MSG_JPEG
            );

    // sensor param
    MINT32 sensorMode = SENSOR_SCENARIO_ID_NORMAL_VIDEO;
    sp<IFeatureFlowControl> spFlowControl = IResourceContainer::getInstance(getOpenId())->queryFeatureFlowControl();
    if( spFlowControl == NULL )
    {
        MY_LOGW("failed to queryFeatureFlowControl");
    }
    else
    {
        sensorMode = spFlowControl->getSensorMode();
    }
    NSCamShot::SensorParam rSensorParam(
            getOpenId(),                            //sensor idx
            sensorMode,                             //Scenaio
            getShotRawBitDepth(),                   //bit depth
            MFALSE,                                 //bypass delay
            MFALSE                                  //bypass scenario
            );
    //
    //get sesnor size
    HwInfoHelper helper(getOpenId());
    MSize sensorSize;
    if( ! helper.updateInfos() ) {
        MY_LOGE("cannot properly update infos!");
        return MFALSE;
    }
    //
    if( ! helper.getSensorSize( sensorMode, sensorSize) ) {
        MY_LOGE("cannot get params about sensor!");
        return MFALSE;
    }
    MY_LOGD("sensorMode(%d), sensorSize(%d,%d)", sensorMode, sensorSize.w, sensorSize.h);
    //
    //DRE update
    MINT32 supportDRE = property_get_int32("camera.mdp.dre.enable", 0);
    if(supportDRE != 0)
    {
        MBOOL bEnableCALTM = false;
        IMetadata::setEntry<MBOOL>(&mShotParam.mHalSetting, MTK_P2NODE_CTRL_CALTM_ENABLE, bEnableCALTM);
    }
    //
    //vhdr update ispprofile halmeta
    IMetadata isp_profile;
    {
        // Prepare query Feature VSSShot ISP Profile
        // Prepare query Feature Streaming ISP Profile
        ProfileParam profileParam(
            sensorSize,
            mVHDRMode,
            sensorMode,
            ProfileParam::FLAG_NONE,
            (mIsAutoHDR ? ProfileParam::FMASK_AUTO_HDR_ON : ProfileParam::FMASK_NONE)
        );

        MUINT8 featureProf = 0;
        if (FeatureProfileHelper::getVSSProf(featureProf, profileParam))
        {
            updateEntry<MUINT8>(&mShotParam.mHalSetting , MTK_3A_ISP_PROFILE , featureProf);
            //
            IMetadata::IEntry entry(MTK_3A_ISP_PROFILE);
            entry.push_back(featureProf, Type2Type< MUINT8 >());
            isp_profile.update(MTK_3A_ISP_PROFILE, entry);
            //
        }
    }
    //
    //
    sp<StreamBufferProvider> pTempConsumer =
        IResourceContainer::getInstance(getOpenId())->queryConsumer(eSTREAMID_IMAGE_PIPE_RAW_OPAQUE );
    MY_LOGD("Query Consumer OpenID(%d) StreamID(%d)", getOpenId(), eSTREAMID_IMAGE_PIPE_RAW_OPAQUE);
    if(pTempConsumer == NULL)
    {
        MY_LOGE("can't find StreamBufferProvider in ConsumerContainer");
        return MFALSE;
    }
    // update post nr settings
    NRParam nrParam;
    nrParam.mpHalMeta    = &mShotParam.mHalSetting;
    nrParam.mFullRawSize = pTempConsumer->queryImageStreamInfo()->getImgSize();
    nrParam.mPluginUser  = mPluginUser;
    updatePostNRSetting(nrParam);
    //
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
            MFALSE,                          // P1 don't need LCS generate because VSS don't need P1.
            MTRUE                            // mbEnableRrzoCapture, always no need to do remosaic
            );

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
    // Let 3a flow know capture begin
    IHal3A *hal3A = MAKE_Hal3A(
                    getOpenId(), LOG_TAG);
    if ( hal3A == NULL ) {
        MY_LOGW("NULL 3A!");
    }
    else
    {
        hal3A->send3ACtrl(E3ACtrl_SetIsZsdCapture, 1,0);
        hal3A->destroyInstance(getShotName());
        hal3A = NULL;
    }
    MY_LOGD("Single start one - E");
    MY_LOGD("VSS: provider(%p)", pTempConsumer.get());
    ret = pSingleShot->startOne(rSensorParam, pTempConsumer.get(), &isp_profile);
    MY_LOGD("Single start one - E");
    //
    #if 0
    pSelector->returnBuffer(rpHeap);
    #endif
    //
    ret = pSingleShot->uninit();
    //
    pSingleShot->destroyInstance();


    return ret;
}


/******************************************************************************
 *
 ******************************************************************************/
void
VssShot::
onCmd_cancel()
{

}


/******************************************************************************
 *
 ******************************************************************************/
MBOOL
VssShot::
fgCamShotNotifyCb(MVOID* user, NSCamShot::CamShotNotifyInfo const msg)
{
    VssShot *pVssShot = reinterpret_cast <VssShot *>(user);
    if (NULL != pVssShot)
    {
        if ( NSCamShot::ECamShot_NOTIFY_MSG_EOF == msg.msgType)
        {
            pVssShot->mpShotCallback->onCB_Shutter(true,
                                                      0,
                                                      pVssShot->getShotMode()
                                                     );
        }
    }

    return MTRUE;
}


/******************************************************************************
 *
 ******************************************************************************/
MBOOL
VssShot::
fgCamShotDataCb(MVOID* user, NSCamShot::CamShotDataInfo const msg)
{
    VssShot *pVssShot = reinterpret_cast<VssShot *>(user);
    if (NULL != pVssShot)
    {
        if (NSCamShot::ECamShot_DATA_MSG_POSTVIEW == msg.msgType)
        {
            //pVssShot->handlePostViewData( msg.puData, msg.u4Size);
        }
        else if (NSCamShot::ECamShot_DATA_MSG_JPEG == msg.msgType)
        {
            pVssShot->handleJpegData(msg.pBuffer);
        }
    }

    return MTRUE;
}


/******************************************************************************
*
*******************************************************************************/
MBOOL
VssShot::
handlePostViewData(MUINT8* const puBuf __attribute__((__unused__)), MUINT32 const u4Size __attribute__((__unused__)))
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
VssShot::
handleJpegData(IImageBuffer* pJpeg)
{
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
    mpShotCallback->onCB_CompressedImage_packed(0,
                                         u4JpegSize,
                                         puJpegBuf,
                                         0,                       //callback index
                                         true,
                                         MTK_CAMERA_MSG_EXT_DATA_COMPRESSED_IMAGE,
                                         getShotMode());

    // Let 3a flow know capture end
    IHal3A *hal3A = MAKE_Hal3A(
                    getOpenId(), LOG_TAG);
    if ( hal3A == NULL ) {
        MY_LOGW("NULL 3A!");
    }
    else
    {
        hal3A->send3ACtrl(E3ACtrl_SetIsZsdCapture, 0,0);
        hal3A->destroyInstance(getShotName());
        hal3A = NULL;
    }
    //
    MY_LOGD("-");

    return MTRUE;
}



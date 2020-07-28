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

#define LOG_TAG "MtkCam/VendorShot"
//
#include <mtkcam/utils/std/Log.h>
#include <mtkcam/utils/std/Trace.h>
#include <mtkcam/def/common.h>
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
#include "VendorShot.h"
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

using namespace android;
using namespace NSShot;
using namespace VShot;
using namespace NSCam::v1;
using namespace NSCam::v1::NSLegacyPipeline;
using namespace NSCamHW;
using namespace NS3Av3;

#define CHECK_OBJECT(x)  do{                                        \
    if (x == nullptr) { MY_LOGE("Null %s Object", #x); return MFALSE;} \
} while(0)

#include <cutils/properties.h>
#define DUMP_KEY  "debug.vendorshot.dump"
#define DUMP_PATH "/sdcard/vendorshot"
#define VENDORSHOT_SOURCE_COUNT     (1)
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

/******************************************************************************
 *
 ******************************************************************************/
extern
sp<IShot>
createInstance_VendorShot(
    char const*const    pszShotName,
    uint32_t const      u4ShotMode,
    int32_t const       i4OpenId
)
{
    sp<IShot>  pShot            = NULL;
    sp<VendorShot>  pImpShot = NULL;
    //
    //  (1.1) new Implementator.
    pImpShot = new VendorShot(pszShotName, u4ShotMode, i4OpenId);
    if  ( pImpShot == 0 ) {
        CAM_LOGE("[%s] new VendorShot", __FUNCTION__);
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
 *  This function is invoked when this object is firstly created.
 *  All resources can be allocated here.
 ******************************************************************************/
bool
VendorShot::
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
VendorShot::
onDestroy()
{

}

/******************************************************************************
 *
 ******************************************************************************/
VendorShot::
VendorShot(
    char const*const pszShotName,
    uint32_t const u4ShotMode,
    int32_t const i4OpenId
)
    : ImpShot(pszShotName, u4ShotMode, i4OpenId)
    , mSensorFps(0)
    , mPixelMode(0)
    , mu4Scenario(0)
    , mu4Bitdepth(0)
    //
    , mpPipeline(NULL)
    , mpImageCallback(NULL)
    , mpCallbackHandler(NULL)
    , mpMetadataListener(NULL)
    //
    , mpJpegPool()
    , mCapReqNo(0)
    , mEncDoneCond()
    , mEncJobLock()
    , mEncJob()
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
}


/******************************************************************************
 *
 ******************************************************************************/
VendorShot::
~VendorShot()
{
    MY_LOGD("~VendorShot()");

    auto pPipeline = mpPipeline;

    if( pPipeline.get() )
    {
        if( mShotParam.mbNeedP1DoneCb ) {
            // fast shot to preview
            pPipeline->waitUntilP1DrainedAndFlush();
            // do P1 flush and done callback to cam1device for resume preview
            mpShotCallback->onCB_P1done();
            pPipeline->waitUntilP2JpegDrainedAndFlush();
        }
        else {
            pPipeline->waitUntilDrained();
        }
        pPipeline->flush();
        mpPipeline = nullptr;
    }

    if( mResultMetadataSetMap.size() > 0 )
    {
        int n = mResultMetadataSetMap.size();
        for(int i = 0; i < n; i++)
        {
            MY_LOGW("requestNo(%d) doesn't clear before VendorShot destroyed",mResultMetadataSetMap.keyAt(i));
        }
    }
    mResultMetadataSetMap.clear();
}


/******************************************************************************
 *
 ******************************************************************************/
bool
VendorShot::
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
VendorShot::
onCmd_reset()
{
    bool ret = true;
    return ret;
}

/******************************************************************************
 *
 ******************************************************************************/
bool
VendorShot::
onCmd_capture()
{
    CAM_TRACE_NAME("Zsd vendor Capture");

    // TODO: decide different shot. Ex. zsh with flash light, HDR..
    MINT32 type;
    decideSettingType(type);

    // prepare necessary setting for capture
    // 1. get sensor size
    // TODO
    beginCapture();
    //
    // TODO: add different setting here [optional]
    //DRE update
    MINT32 supportDRE = property_get_int32("camera.mdp.dre.enable", 0);
    if(supportDRE != 0)
    {
        MBOOL bEnableCALTM = false;
        IMetadata::setEntry<MBOOL>(&mShotParam.mHalSetting, MTK_P2NODE_CTRL_CALTM_ENABLE, bEnableCALTM);
    }
    //
    MINT32 shotCount = VENDORSHOT_SOURCE_COUNT;
    MINT32 YuvBufferCount = shotCount;
    Vector<NS3Av3::CaptureParam_T> vHdrCaptureParams;
    Vector<NS3Av3::CaptureParam_T> vOrigCaptureParams;
    mEncJob.setSourceCnt(shotCount);
    {
        MBOOL res = MTRUE;
        //
        Vector< SettingSet > vSettings;
        switch( type ) {
            case SETTING_HDR:
                {
                    updateCaptureParams(shotCount,
                        vHdrCaptureParams, &vOrigCaptureParams);
                    //append another blank EVs in tails
                    shotCount = vHdrCaptureParams.size();
                    mEncJob.setSourceCnt(shotCount);
                } break;
            // default capture flow
            default:
                break;
        }

        if(res == MFALSE){
            MY_LOGE("applySettings Fail!");
            return MFALSE;
        }
        //
    }
    //
#warning "TODO: set yuv & jpeg format / size / buffer in createStreams()"
    if ( ! createStreams(YuvBufferCount) ) {
        MY_LOGE("createStreams failed");
        return MFALSE;
    }
#warning "TODO: create jpeg buffer here"
    mpJpegPool = new BufferPoolImp(mpInfo_Jpeg);
    mpJpegPool->allocateBuffer(LOG_TAG, mpInfo_Jpeg->getMaxBufNum(), mpInfo_Jpeg->getMinInitBufNum());

    constructCapturePipeline();

    // get multiple raw buffer and send to capture pipeline
    for ( MINT32 i = 0; i < shotCount; ++i ) {
        status_t status = OK;
        //
        // TODO: modify capture setting here! [optional] ex. diff EV
        IMetadata appSetting = mShotParam.mAppSetting;
        IMetadata halSetting = mShotParam.mHalSetting;
        if ( type == SETTING_HDR )
        {
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
                entry.push_back( (i+1) != shotCount , Type2Type< MUINT8 >());
                halSetting.update(entry.tag(), entry);
            }
        }

        // submit setting to capture pipeline
        if (OK != submitCaptureSetting(appSetting, halSetting) ) {
            MY_LOGE("Submit capture setting fail.");
            return MFALSE;
        }
        //
        mCapReqNo++;
    }

    // 1. wait encode done
    // 2. wait pipeline done
    endCapture();
    //
    return MTRUE;
}


/******************************************************************************
 *
 ******************************************************************************/
void
VendorShot::
onCmd_cancel()
{
}

/******************************************************************************
*
*******************************************************************************/
MBOOL
VendorShot::
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
VendorShot::
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
    mpShotCallback->onCB_CompressedImage_packed(0,
                                         u4JpegSize,
                                         puJpegBuf,
                                         0,                       //callback index
                                         true,                     //final image
                                         MTK_CAMERA_MSG_EXT_DATA_COMPRESSED_IMAGE,
                                         getShotMode()
                                         );
    MY_LOGD("-");

    return MTRUE;
}

/******************************************************************************
*
*******************************************************************************/
MVOID
VendorShot::
decideSettingType( MINT32& type )
{
    type = SETTING_NONE;
    // type = SETTING_HDR;
}


/******************************************************************************
*
*******************************************************************************/
MERROR
VendorShot::
updateCaptureParams(
    MINT32 shotCount,
    Vector<CaptureParam_T>& vHdrCaptureParams,
    Vector<CaptureParam_T>* vOrigCaptureParams
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
    if (vOrigCaptureParams)
    {
        hal3A->send3ACtrl( E3ACtrl_GetCaptureDelayFrame,
            reinterpret_cast<MINTPTR>(&delayedFrames), 0);
        // resize to the amount of delayed frames
        vOrigCaptureParams->resize(delayedFrames);
        for (MUINT32 i = 0; i < delayedFrames; i++)
            vOrigCaptureParams->editItemAt(i) = tmpCap3AParam;
    }
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
    return OK;
}

/******************************************************************************
*
*******************************************************************************/
MERROR
VendorShot::
submitCaptureSetting(
    IMetadata appSetting,
    IMetadata halSetting
)
{
    {
        Mutex::Autolock _l(mResultMetadataSetLock);

        IMetadata resultAppMetadata;
        IMetadata resultHalMetadata;
        mResultMetadataSetMap.add(  mCapReqNo,
                                    ResultSet_T
                                    {
                                        .requestNo         = mCapReqNo,
                                        .appResultMetadata = resultAppMetadata,
                                        .halResultMetadata = resultHalMetadata
                                    }
                                );
    }
    IMetadata::IEntry entry(MTK_HAL_REQUEST_SENSOR_SIZE);
    entry.push_back(mSensorSize, Type2Type< MSize >());
    halSetting.update(entry.tag(), entry);

    // submit setting to capture pipeline
    MY_LOGD("submitSetting %d",mCapReqNo);
    if( OK != mpPipeline->submitSetting(
                mCapReqNo,
                appSetting,
                halSetting
                )
      )
    {
        MY_LOGE("submitRequest failed");
        return UNKNOWN_ERROR;
    }

    return OK;
}

/******************************************************************************
*
*******************************************************************************/
MVOID
VendorShot::
endCapture()
{
    // 1. wait encode done
    if ( !mResultMetadataSetMap.isEmpty() ) {
        Mutex::Autolock _l(mEncJobLock);
        mEncDoneCond.wait(mEncJobLock);
    }
    // 2. wait pipeline done
    auto pPipeline = mpPipeline;
    if( pPipeline.get() ) {
        if( mShotParam.mbNeedP1DoneCb ) {
            // fast shot to preview
            pPipeline->waitUntilP1DrainedAndFlush();
            // do P1 flush and done callback to cam1device for resume preview
            mpShotCallback->onCB_P1done();
            pPipeline->waitUntilP2JpegDrainedAndFlush();
        }
        else {
            pPipeline->waitUntilDrained();
        }
        mpPipeline = nullptr;
    }
}

/******************************************************************************
*
*******************************************************************************/
MBOOL
VendorShot::
createStreams( MINT32 aBufferCount )
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
    HwInfoHelper helper(openId);
    if( ! helper.updateInfos() ) {
        MY_LOGE("cannot properly update infos");
        return MFALSE;
    }
    //
    if( ! helper.getSensorSize( sensorMode, mSensorSize) ||
            ! helper.getSensorFps( sensorMode, mSensorFps) ||
            ! helper.queryPixelMode( sensorMode, mSensorFps, mPixelMode)
      ) {
        MY_LOGE("cannot get params about sensor");
        return MFALSE;
    }
    //
    // Fullsize Raw
    {
        MSize size = mSensorSize;
        MINT format;
        size_t stride;
        MUINT const usage = 0; //not necessary here
        if( ! helper.getImgoFmt(bitDepth, format) ||
                ! helper.alignPass1HwLimitation(mPixelMode, format, true, size, stride) )
        {
            MY_LOGE("wrong params about imgo");
            return MFALSE;
        }
        //
        sp<IImageStreamInfo>
            pStreamInfo = createRawImageStreamInfo(
                    "SingleShot:Fullraw",
                    eSTREAMID_IMAGE_PIPE_RAW_OPAQUE,
                    eSTREAMTYPE_IMAGE_INOUT,
                    aBufferCount, aBufferCount,
                    usage, format, size, stride
                    );
        if( pStreamInfo == nullptr ) {
            return MFALSE;
        }
        mpInfo_FullRaw = pStreamInfo;
    }
    //
#if 0
    // Resize Raw
    if( helper.isRaw() && !! previewsize )
    {
        MSize size;
        MINT format;
        size_t stride;
        MUINT const usage = 0; //not necessary here
        if( ! helper.getRrzoFmt(bitDepth, format) ||
                ! helper.alignRrzoHwLimitation(previewsize, mSensorSize, size) ||
                ! helper.alignPass1HwLimitation(mPixelMode, format, false, size, stride) )
        {
            MY_LOGE("wrong params about imgo");
            return MFALSE;
        }
        //
        sp<IImageStreamInfo>
            pStreamInfo = createRawImageStreamInfo(
                    "SingleShot:Resizedraw",
                    eSTREAMID_IMAGE_PIPE_RAW_RESIZER,
                    eSTREAMTYPE_IMAGE_INOUT,
                    1, 1,
                    usage, format, size, stride
                    );
        if( pStreamInfo == nullptr ) {
            return MFALSE;
        }
        mpInfo_ResizedRaw = pStreamInfo;
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
                    "VendorShot:MainYuv",
                    eSTREAMID_IMAGE_PIPE_YUV_JPEG,
                    eSTREAMTYPE_IMAGE_INOUT,
                    aBufferCount, aBufferCount,
                    usage, format, size, transform
                    );
        if( pStreamInfo == nullptr ) {
            return BAD_VALUE;
        }
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
                    "VendorShot:ThumbnailYuv",
                    eSTREAMID_IMAGE_PIPE_YUV_THUMBNAIL,
                    eSTREAMTYPE_IMAGE_INOUT,
                    aBufferCount, aBufferCount,
                    usage, format, size, transform
                    );
        if( pStreamInfo == nullptr ) {
            return BAD_VALUE;
        }
        //
        mpInfo_YuvThumbnail = pStreamInfo;
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
                    "VendorShot:Jpeg",
                    eSTREAMID_IMAGE_JPEG,
                    eSTREAMTYPE_IMAGE_INOUT,
                    1, 1,
                    usage, format, size, transform
                    );
        if( pStreamInfo == nullptr ) {
            return BAD_VALUE;
        }
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
VendorShot::
createPipeline()
{
    CAM_TRACE_CALL();
    FUNC_START;
    LegacyPipelineBuilder::ConfigParams LPBConfigParams;
    LPBConfigParams.mode = getLegacyPipelineMode();
    LPBConfigParams.enableEIS = MFALSE;
    LPBConfigParams.enableLCS = MFALSE;
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
                                            "VendorShot",
                                            LPBConfigParams);
    CHECK_OBJECT(pBuilder);

    mpImageCallback = new ImageCallback(this, 0);
    CHECK_OBJECT(mpImageCallback);

    sp<BufferCallbackHandler> pCallbackHandler = new BufferCallbackHandler(getOpenId());
    pCallbackHandler->setImageCallback(mpImageCallback);
    mpCallbackHandler = pCallbackHandler;

    sp<StreamBufferProviderFactory> pFactory =
                StreamBufferProviderFactory::createInstance();
    //
    Vector<PipelineImageParam> vImageParam;
    //
    {
        MY_LOGD("createPipeline for Normal");
        // normal flow
        // CamManager* pCamMgr = CamManager::getInstance();
        // MUINT32 scenario = ( pCamMgr->isMultiDevice() && (pCamMgr->getFrameRate(getOpenId()) == 0) )
        //             ? SENSOR_SCENARIO_ID_NORMAL_PREVIEW : SENSOR_SCENARIO_ID_NORMAL_CAPTURE;

        PipelineSensorParam sensorParam(
            /*.mode      = */SENSOR_SCENARIO_ID_NORMAL_CAPTURE, //scenario
            /*.rawType   = */0x0000,
            /*.size      = */mSensorSize,
            /*.fps       = */(MUINT)mSensorFps,
            /*.pixelMode = */mPixelMode,
            /*.vhdrMode  = */MFALSE
            );
        //
        if( OK != pBuilder->setSrc(sensorParam) ) {
            MY_LOGE("setSrc failed");
            return MFALSE;
        }
        //
        sp<IScenarioControl> pScenarioCtrl = IScenarioControl::create(getOpenId());
        if( pScenarioCtrl.get() == NULL )
        {
            MY_LOGE("get Scenario Control fail");
            return MFALSE;
        }
        IScenarioControl::ControlParam param;
        param.scenario   = IScenarioControl::Scenario_Capture;
        param.sensorSize = mSensorSize;
        param.sensorFps  = mSensorFps;
        if(LPBConfigParams.enableDualPD)
            FEATURE_CFG_ENABLE_MASK(param.featureFlag,IScenarioControl::FEATURE_DUAL_PD);

        pScenarioCtrl->enterScenario(param);
        pBuilder->setScenarioControl(pScenarioCtrl);
        //
        // fullraw
        if( mpInfo_FullRaw.get() )
        {
            sp<IImageStreamInfo> pStreamInfo = mpInfo_FullRaw;
            //
            sp<CallbackBufferPool> pPool = new CallbackBufferPool(pStreamInfo);
            pPool->allocateBuffer(
                              pStreamInfo->getStreamName(),
                              pStreamInfo->getMaxBufNum(),
                              pStreamInfo->getMinInitBufNum()
                              );
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
        // resized raw
        if( mpInfo_ResizedRaw.get() )
        {
            sp<IImageStreamInfo> pStreamInfo = mpInfo_ResizedRaw;
            //
            PipelineImageParam imgParam = {
                .pInfo     = pStreamInfo,
                .pProvider = nullptr,
                .usage     = 0
            };
            vImageParam.push_back(imgParam);
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
                .pProvider = pFactory->create(),
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
                .pProvider = pFactory->create(),
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
                .pProvider = pFactory->create(),
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
MINT
VendorShot::
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

/*******************************************************************************
*
********************************************************************************/
MVOID
VendorShot::
onMetaReceived(
    MUINT32         const requestNo,
    StreamId_T      const streamId,
    MBOOL           const errorResult,
    IMetadata       const result
)
{
    CAM_TRACE_FMT_BEGIN("onMetaReceived No%d,StreamID %#" PRIxPTR, requestNo,streamId);
    MY_LOGD("requestNo %d, stream %#" PRIxPTR ", errResult:%d", requestNo, streamId, errorResult);
    //
    {
        Mutex::Autolock _l(mResultMetadataSetLock);
        int idx = mResultMetadataSetMap.indexOfKey(requestNo);
        if(idx < 0 )
        {
            MY_LOGE("mResultMetadataSetMap can't find requestNo(%d)",requestNo);
            for ( size_t i = 0; i < mResultMetadataSetMap.size(); i++) {
                MY_LOGD( "mResultMetadataSetMap(%d/%d)  requestNo(%d)",
                         i, mResultMetadataSetMap.size(), mResultMetadataSetMap[i].requestNo );
            }
            return;
        }
    }
    //
#warning "Need to modify if pipeline change"
    if (streamId == eSTREAMID_META_HAL_DYNAMIC_P2) {
        // MY_LOGD("Ready to notify p2done & Shutter");
        // mpShotCallback->onCB_P2done();
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
        {
            Mutex::Autolock _l(mResultMetadataSetLock);
            mResultMetadataSetMap.editValueFor(requestNo).appResultMetadata += result;
        }
        {
            Mutex::Autolock _l(mEncJobLock);
            mEncJob.add(requestNo, EncJob::STREAM_APP_META, mResultMetadataSetMap.editValueFor(requestNo).appResultMetadata);
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
VendorShot::
onDataReceived(
    MUINT32 const               requestNo,
    StreamId_T const            streamId,
    android::sp<IImageBuffer>&  pBuffer
)
{
    CAM_TRACE_FMT_BEGIN("onDataReceived No%d,streamId%d", requestNo, (MINT32)streamId);
    MY_LOGD("requestNo %d, streamId 0x%x, buffer %p", requestNo, streamId, pBuffer.get());
    //
    if( pBuffer != 0 )
    {
        if (streamId == eSTREAMID_IMAGE_PIPE_YUV_00)
        {
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
            Mutex::Autolock _l(mEncJobLock);
            mEncJob.add(requestNo, EncJob::STREAM_IMAGE_MAIN, pBuffer);
            checkStreamAndEncodeLocked(requestNo);
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
            String8 filename = String8::format("%s/VendorShot_%dx%d",
                    DUMP_PATH, pBuffer->getImgSize().w, pBuffer->getImgSize().h);
            switch( data )
            {
                case NSCamShot::ECamShot_DATA_MSG_RAW:
                    filename += String8::format("_%d.raw", (MINT32)pBuffer->getBufStridesInBytes(0));
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
            pBuffer->saveToFile(filename);
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
VendorShot::
checkStreamAndEncodeLocked( MUINT32 const requestNo)
{
    if ( ! mEncJob.isReady() )
        return NAME_NOT_FOUND;
    //
    if ( OK != postProcessing() )
        return MFALSE;
    //
    sp<IImageBufferHeap> dstBuffer;
    MUINT32              transform;
    mpJpegPool->acquireFromPool(
                    LOG_TAG,
                    requestNo,
                    dstBuffer,
                    transform
                );
    if ( ! dstBuffer.get() )
        MY_LOGE("no destination");
    // encode jpeg here
    //mEncJob.pDst = dstBuffer;
    mEncJob.setTarget(dstBuffer);
    {
        sp<ExifJpegUtils> pExifJpegUtils = ExifJpegUtils::createInstance(
            getOpenId(), mEncJob.HalMetadata, mShotParam.mAppSetting,
            mEncJob.mpDst, mEncJob.pSrc_main, mEncJob.pSrc_thumbnail );
        if ( !pExifJpegUtils.get() ) {
            MY_LOGE("create exif jpeg encode utils fail");
            return DEAD_OBJECT;
        }
        MERROR err = pExifJpegUtils->execute();
        if ( OK != err) {
            MY_LOGE("Exif Jpeg encode utils: (%d)", err);
            return err;
        }
    }
    //
    sp<IImageBuffer> pBuf = dstBuffer->createImageBuffer();
    handleJpegData(pBuf.get());
    //clear result metadata
    {
        Mutex::Autolock _l(mResultMetadataSetLock);
        for ( size_t i = 0; i < mResultMetadataSetMap.size(); ++i )
            mResultMetadataSetMap.removeItemsAt(0);
    }
    mEncDoneCond.signal();
    //
    return OK;
}

/*******************************************************************************
*
********************************************************************************/
MERROR
VendorShot::
postProcessing()
{
    // MUST to fill final metadata/imagebuffer for encode
    // ex. fetch sources for post processing
    // for ( ssize_t i=0; i<mEncJob.mvSource.size(); i++)
    // {
    // //    mEncJob.pSrc_main =
    // //           GET_GRAY_IMAGE(mEncJob.mvSource[index].pSrc_main);
    // }
    //
    // ex. directly assign last source for encode
    MINT32 index = 0;//mEncJob.mSrcCnt - 1;
    mEncJob.HalMetadata      = mEncJob.mvSource[index].HalMetadata;
    mEncJob.AppMetadata      = mEncJob.mvSource[index].AppMetadata;
    mEncJob.pSrc_main        = mEncJob.mvSource[index].pSrc_main;
    mEncJob.pSrc_thumbnail   = mEncJob.mvSource[index].pSrc_thumbnail;
    return OK;
}

/*******************************************************************************
*
********************************************************************************/
MVOID
VendorShot::
beginCapture()
{
    CAM_TRACE_CALL();
    //
    mu4Scenario = SENSOR_SCENARIO_ID_NORMAL_CAPTURE;
    mu4Bitdepth = getShotRawBitDepth();
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
}

/*******************************************************************************
*
********************************************************************************/
MBOOL
VendorShot::
constructCapturePipeline()
{
    CAM_TRACE_CALL();
    // create new pipeline
    if ( ! createPipeline() ) {
        MY_LOGE("createPipeline failed");
        return MFALSE;
    }
    CHECK_OBJECT(mpPipeline);
    //
    sp<ResultProcessor> pResultProcessor = mpPipeline->getResultProcessor().promote();
    CHECK_OBJECT(pResultProcessor);

    // metadata
    mpMetadataListener = new MetadataListener(this);
    pResultProcessor->registerListener( 0, 1000, true, mpMetadataListener);

    return MTRUE;
}

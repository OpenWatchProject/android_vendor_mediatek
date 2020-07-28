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

#define LOG_TAG "MtkCam/HwInfoHelper"
//
#include <string>

#include <cutils/compiler.h>
#include <cutils/properties.h>

#include <mtkcam/utils/std/Log.h>
#include <mtkcam/utils/std/ULog.h>
#define MY_LOGV(fmt, arg...)        CAM_ULOGMV("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGD(fmt, arg...)        CAM_ULOGMD("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGI(fmt, arg...)        CAM_ULOGMI("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGW(fmt, arg...)        CAM_ULOGMW("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGE(fmt, arg...)        CAM_ULOGME("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGA(fmt, arg...)        CAM_LOGA("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGF(fmt, arg...)        CAM_LOGF("[%s] " fmt, __FUNCTION__, ##arg)
//
#define MY_LOGV_IF(cond, ...)       do { if (            (cond) ) { MY_LOGV(__VA_ARGS__); } }while(0)
#define MY_LOGD_IF(cond, ...)       do { if (            (cond) ) { MY_LOGD(__VA_ARGS__); } }while(0)
#define MY_LOGI_IF(cond, ...)       do { if (            (cond) ) { MY_LOGI(__VA_ARGS__); } }while(0)
#define MY_LOGW_IF(cond, ...)       do { if ( CC_UNLIKELY(cond) ) { MY_LOGW(__VA_ARGS__); } }while(0)
#define MY_LOGE_IF(cond, ...)       do { if ( CC_UNLIKELY(cond) ) { MY_LOGE(__VA_ARGS__); } }while(0)
#define MY_LOGA_IF(cond, ...)       do { if ( CC_UNLIKELY(cond) ) { MY_LOGA(__VA_ARGS__); } }while(0)
#define MY_LOGF_IF(cond, ...)       do { if ( CC_UNLIKELY(cond) ) { MY_LOGF(__VA_ARGS__); } }while(0)

#define FUNC_START                  MY_LOGD("+")
#define FUNC_END                    MY_LOGD("-")
//
#include <mtkcam/drv/iopipe/CamIO/INormalPipe.h>
#include <mtkcam/drv/iopipe/CamIO/Cam_QueryDef.h>
#include <mtkcam/drv/def/ispio_port_index.h>
#include <mtkcam/drv/IHalSensor.h>
#include <kd_imgsensor_define.h>
#include <mtkcam/utils/hw/HwInfoHelper.h>
#if (MTKCAM_HAVE_VSDOF_SUPPORT == 1) || (MTKCAM_HAVE_STEREO_CAMERA_SUPPORT == 1)
#if (3 == MTKCAM_HAL_VERSION)
#include <mtkcam3/feature/stereo/StereoCamEnum.h>
#include <mtkcam3/feature/stereo/hal/stereo_setting_provider.h>
#include <mtkcam3/feature/stereo/hal/stereo_size_provider.h>
#else
#include <mtkcam/feature/stereo/StereoCamEnum.h>
#include <mtkcam/feature/stereo/hal/stereo_setting_provider.h>
#include <mtkcam/feature/stereo/hal/stereo_size_provider.h>

#endif
#define UNUSED(expr) do { (void)(expr); } while (0)
#endif

CAM_ULOG_DECLARE_MODULE_ID(MOD_UTILITY);
//

using namespace NSCamHW;
using namespace NSCam;

using namespace NSCam::NSIoPipe;
using namespace NSCam::NSIoPipe::NSCamIOPipe;


/******************************************************************************
 * get the isp major version
 *
 * @return
 *      < 0 : unknown
 *      >= 0: a valid isp major version
 ******************************************************************************/
static int32_t getIspVersionMajor()
{
    static int32_t major = [](){
        char const* key = "ro.vendor.camera.isp-version.major";
        int32_t value = ::property_get_int32(key, -1);
        CAM_LOGD_IF(value>=0, "isp-version(major)=%d", value);
        return value;
    }();

    return major;
}


/******************************************************************************
 *
 ******************************************************************************/
static bool gQueryUFOStrides(size_t stride[3], int format, MSize const& imgSize)
{
    auto pModule = INormalPipeModule::get();
    if  (CC_UNLIKELY( ! pModule )) {
        MY_LOGE("Fail on INormalPipeModule::get()");
        return false;
    }
    auto portIndex = 0;
    switch (format)
    {
        case eImgFmt_UFO_BAYER8:
        case eImgFmt_UFO_BAYER10:
        case eImgFmt_UFO_BAYER12:
        case eImgFmt_UFO_BAYER14:
            portIndex = PORT_IMGO.index;
            break;
        case eImgFmt_UFO_FG_BAYER8:
        case eImgFmt_UFO_FG_BAYER10:
        case eImgFmt_UFO_FG_BAYER12:
        case eImgFmt_UFO_FG_BAYER14:
            portIndex = PORT_RRZO.index;
            break;
        default:
            MY_LOGE("Not UFO format!");
            return false;
    }
    NormalPipe_QueryInfo queryRst;
    NormalPipe_QueryIn input;
    input.width = imgSize.w; // pixMode as default
    MBOOL ret = pModule->query(
        portIndex,
        ENPipeQueryCmd_STRIDE_BYTE,
        format,
        input,
        queryRst
    );
    if (CC_UNLIKELY( ! ret )) {
        MY_LOGE("Fail on INormalPipeModule::query()");
        return false;
    }
    for (int i = 0; i < 3; i++) {
        stride[i] = (size_t)queryRst.stride_B[i];
    }
    return true;
}


/******************************************************************************
 *
 ******************************************************************************/
static bool gQueryDefaultBufPlanes_Pass1(
    BufPlanes& bufPlanes,
    int format,
    MSize const& imgSize,
    size_t stride
)
{
#define addBufPlane(idx, planes, height, stride)            \
            do{                                             \
                planes[idx] = {                             \
                        .sizeInBytes = (height * stride),   \
                        .rowStrideInBytes = stride,         \
                    };                                      \
            }while(0)

    switch( format )
    {
    case eImgFmt_YUY2: // direct yuv
    case eImgFmt_BAYER10:
    case eImgFmt_BAYER12:
    case eImgFmt_BAYER10_UNPAK:
    case eImgFmt_BAYER12_UNPAK:
    case eImgFmt_FG_BAYER10:
    case eImgFmt_FG_BAYER12:
    case eImgFmt_BAYER8: // LCSO
    case eImgFmt_STA_BYTE:
    case eImgFmt_STA_2BYTE: // LCSO with LCE3.0
    case eImgFmt_BAYER16_APPLY_LSC: // for AF input raw in ISP6.0
        bufPlanes.count = 1;
        addBufPlane(0, bufPlanes.planes, imgSize.h, stride);
        break;
    case eImgFmt_NV16: // direct yuv 2 plane
        bufPlanes.count = 2;
        addBufPlane(0, bufPlanes.planes, imgSize.h, stride);
        addBufPlane(1, bufPlanes.planes, imgSize.h, stride);
        break;
    case eImgFmt_UFO_BAYER8:
    case eImgFmt_UFO_BAYER10:
    case eImgFmt_UFO_BAYER12:
    case eImgFmt_UFO_BAYER14:
    case eImgFmt_UFO_FG_BAYER8:
    case eImgFmt_UFO_FG_BAYER10:
    case eImgFmt_UFO_FG_BAYER12:
    case eImgFmt_UFO_FG_BAYER14:{

        size_t ufoStride[3] = {0};
        auto ret = gQueryUFOStrides(ufoStride, format, imgSize);
        if ((CC_UNLIKELY( ! ret ))) {
            MY_LOGF("Fail on queryUFOStride()");
            return false;
        }
        bufPlanes.count = 3;
        addBufPlane(0, bufPlanes.planes, imgSize.h, stride);
        addBufPlane(1, bufPlanes.planes, imgSize.h, ufoStride[1]);
        addBufPlane(2, bufPlanes.planes, imgSize.h, ufoStride[2]);

        }break;

    default:
        MY_LOGF("unsupported format:%d - imgSize:%dx%d stride:%zu",
            format, imgSize.w, imgSize.h, stride);
        return false;
        break;
    }
#undef  addBufPlane
    return true;
}


/******************************************************************************
 *
 ******************************************************************************/
class HwInfoHelper::Implementor
{
protected:
    MINT32 const                mOpenId;
    SensorStaticInfo            mSensorStaticInfo;
    MINT32                      mUseUFO;
    MINT32                      mUseUFO_IMGO;
    MINT32                      mUseUFO_RRZO;

public:
                                Implementor(MINT32 const openId);
    virtual                     ~Implementor() {}

public:
    MBOOL                       updateInfos();
    MBOOL                       isRaw() const { return mSensorStaticInfo.sensorType == NSCam::SENSOR_TYPE_RAW; }
    MBOOL                       isYuv() const { return mSensorStaticInfo.sensorType == NSCam::SENSOR_TYPE_YUV; }
    MBOOL                       getSensorSize(MUINT32 const sensorMode, NSCam::MSize& size) const;
    MBOOL                       getSensorOutputSize(MUINT32 const sensorMode, NSCam::MSize& size) const;
    MBOOL                       getSensorFps(MUINT32 const sensorMode, MINT32& fps) const;
    MBOOL                       getSensorVBTime(MUINT32 sensorMode, MUINT32& VBTime) const;
    MBOOL                       getSensorPDSize(MUINT32 sensorMode, MUINT32& PDSize) const;
    MBOOL                       getImgoFmt(MUINT32 const bitDepth, MINT& fmt, MBOOL forceUFO, MBOOL useUnpakFmt, MBOOL isSecure) const;
    MBOOL                       getRrzoFmt(MUINT32 const bitDepth, MINT& fmt, MBOOL forceUFO, MBOOL isSecure) const;
    MBOOL                       getLpModeSupportBitDepthFormat(MINT& fmt, MUINT32& pipeBit) const;
    MBOOL                       getRecommendRawBitDepth(MINT32& bitDepth) const;
    MBOOL                       getSensorPowerOnPredictionResult(MBOOL& isPowerOnSuccess)const;
    //
    MBOOL                       queryPixelMode(
                                    MUINT32 const sensorMode,
                                    MINT32 const fps,
                                    MUINT32& pixelMode
                                ) const;
    virtual MBOOL               alignPass1HwLimitation(
                                    MINT const imgFormat,
                                    MBOOL isImgo,
                                    NSCam::MSize& size,
                                    size_t& stride
                                ) const;
    virtual MBOOL               alignRrzoHwLimitation(
                                    NSCam::MSize const targetSize,
                                    NSCam::MSize const sensorSize,
                                    NSCam::MSize& result,
                                    MUINT32 const recommendResizeRatio = 0
                                ) const;
    MBOOL                       querySupportVHDRMode(
                                    MUINT32 const sensorMode,
                                    MUINT32& vhdrMode
                                ) const;
    MBOOL                       quertMaxRrzoWidth(MINT32 &maxWidth) const;
    MBOOL                       getPDAFSupported(MUINT32 const sensorMode) const;
    MBOOL                       isType3PDSensorWithoutPDE(MUINT32 const sensorMode, MBOOL const checkSensorOnly) const;
    MBOOL                       getDualPDAFSupported(MUINT32 const sensorMode) const;
    MBOOL                       get4CellSensorSupported() const;
    e4CellSensorPattern         get4CellSensorPattern() const;
    MBOOL                       getSensorRawFmtType(MUINT32 &u4RawFmtType) const;
    MBOOL                       queryUFOStride(MINT const imgFormat, MSize const imgSize, size_t* stride) const;
    MBOOL                       getShutterDelayFrameCount(MINT32& shutterDelayCnt) const;
    MBOOL                       shrinkCropRegion(NSCam::MSize const sensorSize, NSCam::MRect& cropRegion, MINT32 shrinkPx = 2) const;

    MBOOL                       querySupportResizeRatio(MUINT32& rPrecentage) const;
    MBOOL                       querySupportBurstNum(MUINT32& rBitField) const;
    MBOOL                       querySupportRawPattern(MUINT32& rBitField) const;

    static MBOOL                getDynamicTwinSupported();
    static MBOOL                getSecureSensorModeSupported(MUINT32 sensorMode);
    static MUINT32              getCameraSensorPowerOnCount();
    static MBOOL                getPorcessRawInfo(MINT& fmt);
    static MBOOL                getLMVSupported(const MUINT32 &idx, const MUINT32 &rawType);
};
#define IMPL_CLASSNAME   HwInfoHelper::Implementor
//
#if (MTKCAM_HAVE_VSDOF_SUPPORT == 1) || (MTKCAM_HAVE_STEREO_CAMERA_SUPPORT == 1)
/******************************************************************************
 *
 ******************************************************************************/
class HwInfoHelper::DualImplementor : public HwInfoHelper::Implementor
{
public:
    DualImplementor() = delete;
    DualImplementor(MINT32 const openId);
    virtual ~DualImplementor() {}
public:
    MBOOL                       alignPass1HwLimitation(
                                    MINT const imgFormat,
                                    MBOOL isImgo,
                                    NSCam::MSize& size,
                                    size_t& stride
                                ) const override;
    MBOOL                       alignRrzoHwLimitation(
                                    NSCam::MSize const targetSize,
                                    NSCam::MSize const sensorSize,
                                    NSCam::MSize& result,
                                    MUINT32 const recommendResizeRatio = 0
                                ) const override;
};
#define DUAL_IMPL_CLASSNAME   HwInfoHelper::DualImplementor
#endif

/******************************************************************************
 *
 ******************************************************************************/
IMPL_CLASSNAME::
Implementor(MINT32 const openId)
    : mOpenId(openId)
#ifdef USE_UFO
    , mUseUFO(1)
    , mUseUFO_IMGO(1)
    , mUseUFO_RRZO(1)
#else
    , mUseUFO(0)
    , mUseUFO_IMGO(0)
    , mUseUFO_RRZO(0)
#endif
{
    ::memset(&mSensorStaticInfo, 0, sizeof(SensorStaticInfo));
    MINT disableUFO = ::property_get_int32("vendor.debug.camera.ufo_off", 0);
    MINT disableUFO_IMGO = ::property_get_int32("vendor.debug.camera.ufo_off.imgo", 1);
    MINT disableUFO_RRZO = ::property_get_int32("vendor.debug.camera.ufo_off.rrzo", 1);
    if (disableUFO) {
        mUseUFO = 0;
        mUseUFO_IMGO = 0;
        mUseUFO_RRZO = 0;
    } else {
        if (disableUFO_IMGO)
            mUseUFO_IMGO = 0;
        if (disableUFO_RRZO)
            mUseUFO_RRZO = 0;
    }
}


/******************************************************************************
 *
 ******************************************************************************/
MBOOL
HwInfoHelper::
updateInfos()
{
    return  mpImp->updateInfos();
}

MBOOL
IMPL_CLASSNAME::
updateInfos()
{
    IHalSensorList* pSensorList = MAKE_HalSensorList();
    if( ! pSensorList ) {
        MY_LOGE("cannot get sensorlist");
        return MFALSE;
    }
    MUINT32 sensorDev = pSensorList->querySensorDevIdx(mOpenId);
    pSensorList->querySensorStaticInfo(sensorDev, &mSensorStaticInfo);
    return MTRUE;
}


/******************************************************************************
 *
 ******************************************************************************/
MBOOL
HwInfoHelper::
isRaw() const
{
    return  mpImp->isRaw();
}


/******************************************************************************
 *
 ******************************************************************************/
MBOOL
HwInfoHelper::
isYuv() const
{
    return  mpImp->isYuv();
}


/******************************************************************************
 *
 ******************************************************************************/
MBOOL
HwInfoHelper::
getSensorSize(MUINT32 const sensorMode, MSize& size) const
{
    return  mpImp->getSensorSize(sensorMode, size);
}

MBOOL
IMPL_CLASSNAME::
getSensorSize(MUINT32 const sensorMode, MSize& size) const
{
    switch(sensorMode)
    {
#define scenario_case(scenario, KEY, size_var)          \
        case scenario:                                  \
            size_var.w = mSensorStaticInfo.KEY##Width;  \
            size_var.h = mSensorStaticInfo.KEY##Height; \
            break;
        scenario_case( SENSOR_SCENARIO_ID_NORMAL_PREVIEW, preview, size );
        scenario_case( SENSOR_SCENARIO_ID_NORMAL_CAPTURE, capture, size );
        scenario_case( SENSOR_SCENARIO_ID_NORMAL_VIDEO, video, size );
        scenario_case( SENSOR_SCENARIO_ID_SLIM_VIDEO1, video1, size );
        scenario_case( SENSOR_SCENARIO_ID_SLIM_VIDEO2, video2, size );
        scenario_case( SENSOR_SCENARIO_ID_CUSTOM1, SensorCustom1, size);
        scenario_case( SENSOR_SCENARIO_ID_CUSTOM2, SensorCustom2, size);
        scenario_case( SENSOR_SCENARIO_ID_CUSTOM3, SensorCustom3, size);
        scenario_case( SENSOR_SCENARIO_ID_CUSTOM4, SensorCustom4, size);
        scenario_case( SENSOR_SCENARIO_ID_CUSTOM5, SensorCustom5, size);
#undef scenario_case
        default:
            MY_LOGE("not support sensor scenario(0x%x)", sensorMode);
            return MFALSE;
    }
    return MTRUE;
}

/******************************************************************************
 *
 ******************************************************************************/
MBOOL
HwInfoHelper::
getSensorOutputSize(MUINT32 const sensorMode, MSize& size) const
{
    return  mpImp->getSensorOutputSize(sensorMode, size);
}

MBOOL
IMPL_CLASSNAME::
getSensorOutputSize(MUINT32 const sensorMode, MSize& size) const
{
    SensorCropWinInfo rSensorCropInfo;
    IHalSensor* pSensorHalObj = NULL;
    IHalSensorList* const pHalSensorList = MAKE_HalSensorList();
    //
    if( !pHalSensorList ) { MY_LOGE("pHalSensorList == NULL"); return MFALSE; }

    pSensorHalObj = pHalSensorList->createSensor(LOG_TAG, mOpenId);
    if( pSensorHalObj == NULL ) { MY_LOGE("pSensorHalObj is NULL"); return MFALSE; }

    pSensorHalObj->sendCommand(
            pHalSensorList->querySensorDevIdx(mOpenId),
            SENSOR_CMD_GET_SENSOR_CROP_WIN_INFO,
            (MUINTPTR)(&sensorMode),
            (MUINTPTR)(&rSensorCropInfo),
            (MUINTPTR)0);
    MY_LOGD("w1 x h1 = %u x %u\n", rSensorCropInfo.w1_size, rSensorCropInfo.h1_size);
    pSensorHalObj->destroyInstance(LOG_TAG);
    size.w = rSensorCropInfo.w1_size;
    size.h = rSensorCropInfo.h1_size;
    return MTRUE;
}

/******************************************************************************
 *
 ******************************************************************************/
MBOOL
HwInfoHelper::
getSensorFps(MUINT32 const sensorMode, MINT32& fps) const
{
    return  mpImp->getSensorFps(sensorMode, fps);
}

MBOOL
IMPL_CLASSNAME::
getSensorFps(MUINT32 const sensorMode, MINT32& fps) const
{
    switch(sensorMode)
    {
#define scenario_case(scenario, KEY, fps_var)              \
        case scenario:                                     \
            fps_var = mSensorStaticInfo.KEY##FrameRate/10; \
            break;
        scenario_case( SENSOR_SCENARIO_ID_NORMAL_PREVIEW, preview, fps );
        scenario_case( SENSOR_SCENARIO_ID_NORMAL_CAPTURE, capture, fps );
        scenario_case( SENSOR_SCENARIO_ID_NORMAL_VIDEO, video, fps );
        scenario_case( SENSOR_SCENARIO_ID_SLIM_VIDEO1, video1, fps );
        scenario_case( SENSOR_SCENARIO_ID_SLIM_VIDEO2, video2, fps );
        scenario_case( SENSOR_SCENARIO_ID_CUSTOM1, custom1, fps);
        scenario_case( SENSOR_SCENARIO_ID_CUSTOM2, custom2, fps);
        scenario_case( SENSOR_SCENARIO_ID_CUSTOM3, custom3, fps);
        scenario_case( SENSOR_SCENARIO_ID_CUSTOM4, custom4, fps);
        scenario_case( SENSOR_SCENARIO_ID_CUSTOM5, custom5, fps);

#undef scenario_case
        default:
            MY_LOGE("not support sensor scenario(0x%x)", sensorMode);
            return MFALSE;
    }
    return MTRUE;
}

/******************************************************************************
 *
 ******************************************************************************/
MBOOL
HwInfoHelper::
getSensorVBTime(MUINT32 sensorMode, MUINT32& VBTime) const
{
    return  mpImp->getSensorVBTime(sensorMode, VBTime);
}

MBOOL
IMPL_CLASSNAME::
getSensorVBTime(MUINT32 sensorMode, MUINT32& VBTime) const
{
    IHalSensor* pSensorHalObj = NULL;
    IHalSensorList* const pHalSensorList = MAKE_HalSensorList();
    MY_LOGD("WillDBG2 before vbtime : %d", VBTime);
    //
    if( !pHalSensorList ) { MY_LOGE("pHalSensorList == NULL"); return MFALSE; }

    pSensorHalObj = pHalSensorList->createSensor(LOG_TAG, mOpenId);
    if( pSensorHalObj == NULL ) { MY_LOGE("pSensorHalObj is NULL"); return MFALSE; }

    pSensorHalObj->sendCommand(
            pHalSensorList->querySensorDevIdx(mOpenId),
            SENSOR_CMD_GET_VERTICAL_BLANKING,
            (MUINTPTR)(&VBTime),
            (MUINTPTR)(&sensorMode),
            (MUINTPTR)0);
    pSensorHalObj->destroyInstance(LOG_TAG);
    MY_LOGD("vbtime : %d", VBTime);
    return MTRUE;
}

/******************************************************************************
 *
 ******************************************************************************/
MBOOL
HwInfoHelper::
getSensorPDSize(MUINT32 sensorMode, MUINT32& PDSize) const
{
    return  mpImp->getSensorPDSize(sensorMode, PDSize);
}

MBOOL
IMPL_CLASSNAME::
getSensorPDSize(MUINT32 sensorMode, MUINT32& PDSize) const
{
    MBOOL ret = MFALSE;

    //
    MINT32 SensorModeSupportPD = 0;
    IHalSensor* pSensorHalObj = NULL;
    IHalSensorList* pHalSensorList = MAKE_HalSensorList();

    //
    if( !pHalSensorList ) {
        MY_LOGE("pHalSensorList == NULL");
        return ret;
    }

    //
    MUINT32 sensorDev = pHalSensorList->querySensorDevIdx(mOpenId);

    //
    pSensorHalObj = pHalSensorList->createSensor(LOG_TAG, mOpenId);
    if( pSensorHalObj == NULL ) {
        MY_LOGE("pSensorHalObj is NULL");
        return ret;
    }

    /* PDAF capacity enable or not */
    pSensorHalObj->sendCommand(
            sensorDev,
            SENSOR_CMD_GET_SENSOR_PDAF_CAPACITY,
            (MUINTPTR)(&sensorMode),
            (MUINTPTR)(&SensorModeSupportPD),
            0);

    pSensorHalObj->destroyInstance(LOG_TAG);

    // DMA access bandwidth
    SensorStaticInfo sensorStaticInfo;
    memset( &sensorStaticInfo, 0, sizeof(SensorStaticInfo));
    if(SensorModeSupportPD)
    {
        pHalSensorList->querySensorStaticInfo(sensorDev, &sensorStaticInfo);

        switch(sensorStaticInfo.PDAF_Support)
        {
            case PDAF_SUPPORT_CAMSV:
            case PDAF_SUPPORT_CAMSV_LEGACY:
            case PDAF_SUPPORT_CAMSV_DUALPD:
                {
                    SensorVCInfo2 sensor_vc_info_v2;
                    memset( &sensor_vc_info_v2, 0, sizeof(SensorVCInfo2));
                    MINT32 err = pSensorHalObj->sendCommand( sensorDev,
                                                             SENSOR_CMD_GET_SENSOR_VC_INFO2,
                                                             (MUINTPTR)&(sensor_vc_info_v2)/*Output*/,
                                                             (MUINTPTR)&(sensorMode)/*input*/,
                                                             0);

                    if(err==0)
                    {
                        for(MUINT32 i=0; i<sensor_vc_info_v2.vcInfo2s.size(); i++)
                        {
                            if(sensor_vc_info_v2.vcInfo2s[i].VC_FEATURE == VC_PDAF_STATS)
                            {
                                PDSize = sensor_vc_info_v2.vcInfo2s[i].VC_SIZEH_BYTE * sensor_vc_info_v2.vcInfo2s[i].VC_SIZEV;
                            }
                        }
                    }
                }
                break;

            case PDAF_SUPPORT_RAW_DUALPD:
                PDSize = 0;
                MY_LOGD("PDAF_SUPPORT_RAW_DUALPD is not supported any more");
                break;

            case PDAF_SUPPORT_RAW:
                {
                    SET_PD_BLOCK_INFO_T pd_block_info;
                    memset( &pd_block_info, 0, sizeof(SET_PD_BLOCK_INFO_T));
                    MINT32 err __unused = pSensorHalObj->sendCommand( pHalSensorList->querySensorDevIdx(mOpenId),
                                                             SENSOR_CMD_GET_SENSOR_PDAF_INFO,
                                                             (MINTPTR )&(sensorMode)/*input*/,
                                                             (MINTPTR )&(pd_block_info)/*Output*/,
                                                             0);

                    if( pd_block_info.i4SubBlkW && pd_block_info.i4SubBlkH)
                    {
                       PDSize = pd_block_info.i4BlockNumX * (pd_block_info.i4PitchX / pd_block_info.i4SubBlkW) *
                                pd_block_info.i4BlockNumY * (pd_block_info.i4PitchY / pd_block_info.i4SubBlkH) *
                                sizeof(unsigned short);
                    }
                }
                break;

            case PDAF_SUPPORT_RAW_LEGACY:
            case PDAF_SUPPORT_NA:
            default:
                PDSize = 0;
                break;

        }
    }
    else
    {
        PDSize = 0;
    }

    MY_LOGD("pd sensor info:(%d/%d). DMA bandwidth for pd:(%d byte)", sensorStaticInfo.PDAF_Support, SensorModeSupportPD, PDSize);

    return MTRUE;
}

/******************************************************************************
 *
 ******************************************************************************/
MBOOL
HwInfoHelper::
getImgoFmt(MUINT32 const bitDepth, MINT& fmt, MBOOL forceUFO, MBOOL useUnpakFmt, MBOOL isSecure) const
{
    return  mpImp->getImgoFmt(bitDepth, fmt, forceUFO, useUnpakFmt, isSecure);
}

MBOOL
IMPL_CLASSNAME::
getImgoFmt(MUINT32 const bitDepth, MINT& fmt, MBOOL forceUFO, MBOOL useUnpakFmt, MBOOL isSecure) const
{
#define case_Format( condition, mappedfmt, fmt_var) \
        case condition:                             \
            fmt_var = mappedfmt;                    \
        break;

    if( isYuv() )
    {
        switch( mSensorStaticInfo.sensorFormatOrder )
        {
            case_Format( SENSOR_FORMAT_ORDER_UYVY, eImgFmt_UYVY, fmt);
            case_Format( SENSOR_FORMAT_ORDER_VYUY, eImgFmt_VYUY, fmt);
            case_Format( SENSOR_FORMAT_ORDER_YUYV, eImgFmt_YUY2, fmt);
            case_Format( SENSOR_FORMAT_ORDER_YVYU, eImgFmt_YVYU, fmt);
            default:
            MY_LOGE("formatOrder not supported: 0x%x", mSensorStaticInfo.sensorFormatOrder);
            return MFALSE;
        }
    }
    else if( isRaw() )
    {
        MBOOL useAfRaw = MFALSE;
        {
            // for debug flow
            MINT32 unpak = ::property_get_int32("debug.camera.rawunpak", -1);
            if ( unpak > 0 )
            {
                MY_LOGI("debug.camera.rawunpak = %d, refer useUnpakFmt = %d, forced to use unpak format", unpak, useUnpakFmt);
                useUnpakFmt = MTRUE;
            }
            else if( unpak == 0 )
            {
                MY_LOGI("debug.camera.rawunpak = %d,  refer useUnpakFmt = %d, don't use unpak format", unpak, useUnpakFmt);
                useUnpakFmt = MFALSE;
            }
            else
            {
                MY_LOGI("debug.camera.rawunpak = %d,  useUnpakFmt = %d", unpak, useUnpakFmt);
            }

            // for dump AF input raw in ISP6.0 platform
            MINT32 AfRawDump = ::property_get_int32("vendor.debug.camera.afraw", -1);
            if ( AfRawDump > 0 )
            {
                MY_LOGI("vendor.debug.camera.afraw = %d,useUnpakFmt = %d, forced to use af raw format", AfRawDump, useUnpakFmt);
                useAfRaw = MTRUE;
            }
            else
            {
                MY_LOGI("vendor.debug.camera.afraw = %d, don't use Af Raw", AfRawDump);
            }
        }
        //
        NormalPipe_QueryInfo queryRst;
        NormalPipe_QueryIn input;
        MINT imgFmt[2]; //imgFmt[0]: 1st priority request format  //imgFmt[1]: 2nd priority request format
        auto pModule = INormalPipeModule::get();
        MBOOL ret = MTRUE;

        if  ( ! pModule )
        {
            MY_LOGE("INormalPipeModule::get() fail");
            return MFALSE;
        }

        sCAM_QUERY_QUERY_FMT res;
        res.QueryInput.portId = PORT_IMGO.index;
        res.QueryInput.SecOn = isSecure;
        ret = pModule->query(
            ENPipeQueryCmd_QUERY_FMT,
            (MUINTPTR)(&res));
        if (!ret) {
            MY_LOGE("Cannot query ENPipeQueryCmd_QUERY_FMT");
            return MFALSE;
        }

        // (1) need to fill imgFmt[0] for first request format
        // (2) need to fill imgFmt[1] for backup format
        // (3) if ISP don't support the imgFmt[0] format, then the imgFmt[1] will be applied
        // (4) if imgFmt[0] & imgFmt[1] all are not supported by ISP, the function will return false
        if (useAfRaw) {
            imgFmt[0] = eImgFmt_BAYER16_APPLY_LSC;
            imgFmt[1] = bitDepth == 8 ? eImgFmt_BAYER8:
                bitDepth == 10 ? eImgFmt_BAYER10   :
                bitDepth == 12 ? eImgFmt_BAYER12   :
                bitDepth == 14 ? eImgFmt_BAYER14   : eImgFmt_UNKNOWN;
        }
        else if (useUnpakFmt) {
            imgFmt[0] = bitDepth == 8 ? eImgFmt_BAYER8_UNPAK :
                bitDepth == 10 ? eImgFmt_BAYER10_UNPAK :
                bitDepth == 12 ? eImgFmt_BAYER12_UNPAK :
                bitDepth == 14 ? eImgFmt_BAYER14_UNPAK :
                bitDepth == 15 ? eImgFmt_BAYER15_UNPAK : eImgFmt_UNKNOWN;
            imgFmt[1] = bitDepth == 8 ? eImgFmt_BAYER8:
                    bitDepth == 10 ? eImgFmt_BAYER10   :
                    bitDepth == 12 ? eImgFmt_BAYER12   :
                    bitDepth == 14 ? eImgFmt_BAYER14   : eImgFmt_UNKNOWN;
        }
        else
        {

            // TODO: Need  to remove this hard code
            #if MTKCAM_HAVE_AINR_SUPPORT
                MY_LOGD("Not support UFO when AINR on because of conflict");
                forceUFO = false;
            #endif

            if (forceUFO) {
                imgFmt[0] = (!mUseUFO || !mUseUFO_IMGO) ? eImgFmt_UNKNOWN:
                    bitDepth == 8  ? eImgFmt_UFO_BAYER8  :
                    bitDepth == 10 ? eImgFmt_UFO_BAYER10 :
                    bitDepth == 12 ? eImgFmt_UFO_BAYER12 :
                    bitDepth == 14 ? eImgFmt_UFO_BAYER14 : eImgFmt_UNKNOWN;
                imgFmt[1] = bitDepth == 8 ? eImgFmt_BAYER8:
                    bitDepth == 10 ? eImgFmt_BAYER10   :
                    bitDepth == 12 ? eImgFmt_BAYER12   :
                    bitDepth == 14 ? eImgFmt_BAYER14   : eImgFmt_UNKNOWN;
            } else {
                imgFmt[0] = eImgFmt_UNKNOWN;
                imgFmt[1] = bitDepth == 8 ? eImgFmt_BAYER8:
                    bitDepth == 10 ? eImgFmt_BAYER10   :
                    bitDepth == 12 ? eImgFmt_BAYER12   :
                    bitDepth == 14 ? eImgFmt_BAYER14   : eImgFmt_UNKNOWN;
            }
        }

        if (res.QueryOutput.size() > 0)
        {
            std::vector<NSCam::EImageFormat>::iterator it;
            fmt = eImgFmt_UNKNOWN;
            for (int i = 0 ; i < 2 ; i++)
            {
                if (imgFmt[i] == eImgFmt_UNKNOWN) continue;

                for (it = res.QueryOutput.begin(); it != res.QueryOutput.end(); ++it)
                {
                    if (imgFmt[i] == *it)
                    {
                        break;
                    }
                }
                if (it != res.QueryOutput.end())
                {
                    fmt = imgFmt[i];
                    MY_LOGD("imgFmt[0]:(0x%x) imgFmt[1]:(0x%x) i:(%d) fmt:(0x%x)", imgFmt[0], imgFmt[1], i, fmt);
                    break ;
                }
            }
        }
        else
        {
            fmt = imgFmt[0] != eImgFmt_UNKNOWN ? imgFmt[0] : imgFmt[1];
            MY_LOGD("QueryOutput.size() == 0, imgFmt[0]:(0x%x) imgFmt[1]:(0x%x) fmt:(0x%x)", imgFmt[0], imgFmt[1], fmt);
        }

        if (fmt == eImgFmt_UNKNOWN)
        {
            MY_LOGE("bitdepth not supported: %d", bitDepth);
            return MFALSE;
        }

    }
    else
    {
        MY_LOGE("sensorType not supported yet(0x%x)", mSensorStaticInfo.sensorType);
        return MFALSE;
    }
#undef case_Format
    return MTRUE;
}


/******************************************************************************
 *
 ******************************************************************************/
MBOOL
HwInfoHelper::
getRrzoFmt(MUINT32 const bitDepth, MINT& fmt, MBOOL forceUFO, MBOOL isSecure) const
{
    return  mpImp->getRrzoFmt(bitDepth, fmt, forceUFO, isSecure);
}

MBOOL
IMPL_CLASSNAME::
getRrzoFmt(MUINT32 const bitDepth, MINT& fmt, MBOOL forceUFO, MBOOL isSecure) const
{
    if( isYuv() )
    {
        return MFALSE;
    }
    else if( isRaw() )
    {
        MINT imgFmt[2];
        auto pModule = INormalPipeModule::get();
        if  ( ! pModule ) {
            MY_LOGE("INormalPipeModule::get() fail");
            return MFALSE;
        }
        sCAM_QUERY_QUERY_FMT queryRst;
        queryRst.QueryInput.portId = PORT_RRZO.index;
        queryRst.QueryInput.SecOn = isSecure;
        MBOOL ret = MTRUE;
        ret = pModule->query(
            ENPipeQueryCmd_QUERY_FMT,
            (MUINTPTR)(&queryRst));
        if (!ret) {
            MY_LOGE("Cannot query ENPipeQueryCmd_QUERY_FMT");
            return MFALSE;
        }

        if(forceUFO) {
            imgFmt[0] =  (!mUseUFO || !mUseUFO_RRZO) ? eImgFmt_UNKNOWN :
                bitDepth == 8  ? eImgFmt_UFO_FG_BAYER8  :
                bitDepth == 10 ? eImgFmt_UFO_FG_BAYER10 :
                bitDepth == 12 ? eImgFmt_UFO_FG_BAYER12 :
                bitDepth == 14 ? eImgFmt_UFO_FG_BAYER14 : eImgFmt_UNKNOWN;
            imgFmt[1] =  bitDepth == 8  ? eImgFmt_FG_BAYER8    :
                bitDepth == 10 ? eImgFmt_FG_BAYER10   :
                bitDepth == 12 ? eImgFmt_FG_BAYER12   :
                bitDepth == 14 ? eImgFmt_FG_BAYER14   : eImgFmt_UNKNOWN;
        } else {
            imgFmt[0] = eImgFmt_UNKNOWN;
            imgFmt[1] =  bitDepth == 8  ? eImgFmt_FG_BAYER8    :
                bitDepth == 10 ? eImgFmt_FG_BAYER10   :
                bitDepth == 12 ? eImgFmt_FG_BAYER12   :
                bitDepth == 14 ? eImgFmt_FG_BAYER14   : eImgFmt_UNKNOWN;
        }

        if (queryRst.QueryOutput.size() > 0)
        {
            std::vector<NSCam::EImageFormat>::iterator it;
            fmt = eImgFmt_UNKNOWN;
            for (int i = 0 ; i < 2 ; i++)
            {
                if (imgFmt[i] == eImgFmt_UNKNOWN) continue;

                for (it = queryRst.QueryOutput.begin(); it != queryRst.QueryOutput.end(); ++it)
                {
                    if (imgFmt[i] == *it) break;
                }
                if (it != queryRst.QueryOutput.end())
                {
                    fmt = imgFmt[i];
                    break ;
                }
            }
        }
        else
        {
            fmt = imgFmt[0] != eImgFmt_UNKNOWN ? imgFmt[0] : imgFmt[1];
        }

        if (fmt == eImgFmt_UNKNOWN)
        {
            MY_LOGE("bitdepth not supported: %d", bitDepth);
            return MFALSE;
        }

    }
    else
    {
        MY_LOGE("sensorType not supported yet(0x%x)", mSensorStaticInfo.sensorType);
        return MFALSE;
    }
    return MTRUE;

}

/******************************************************************************
 *
 ******************************************************************************/
MBOOL
HwInfoHelper::
getLpModeSupportBitDepthFormat(MINT& fmt, MUINT32& pipeBit) const

{
    return  mpImp->getLpModeSupportBitDepthFormat(fmt, pipeBit);
}

MBOOL
IMPL_CLASSNAME::
getLpModeSupportBitDepthFormat(MINT& fmt, MUINT32& pipeBit) const
{
    int pipebitdepth = property_get_int32("debug.camera.pipebitdepth", -1);
    if( pipebitdepth >= 0 )
    {
        pipeBit = (MUINT32)pipebitdepth;
        MY_LOGD("(For Debug)Force get LP mode supprot bit depth format (0x%x) !",pipeBit);
        return MTRUE;
    }

    auto pModule = INormalPipeModule::get();
    if  ( ! pModule ) {
        MY_LOGE("INormalPipeModule::get() fail");
        return MFALSE;
    }

    NormalPipe_QueryInfo queryRst;
    NormalPipe_QueryIn input;
    pModule->query(
        NSCam::NSIoPipe::PORT_IMGO.index,
        ENPipeQueryCmd_PIPELINE_BITDEPTH,
        fmt,
        input,
        queryRst
    );
    pipeBit = queryRst.pipelinebitdepth;
    MY_LOGD("get LP mode supprot bit depth format (0x%x) !",pipeBit);
    return MTRUE;
}


/******************************************************************************
 *
 ******************************************************************************/
MBOOL
HwInfoHelper::
getRecommendRawBitDepth(MINT32& bitDepth) const
{
    return  mpImp->getRecommendRawBitDepth(bitDepth);
}

MBOOL
IMPL_CLASSNAME::
getRecommendRawBitDepth(MINT32& bitDepth) const
{
    bitDepth = property_get_int32("debug.camera.raw.bitdepth", -1);
    if(bitDepth == 10)
    {
        MY_LOGD("force set raw bit 10 bits");
        return MTRUE;
    }
    else if(bitDepth == 12)
    {
        MY_LOGD("force set raw bit 12 bits");
        return MTRUE;
    }

    if( isYuv() )
    {
        bitDepth = 10;
        MY_LOGD("isYuv => recommend raw bit 10 bits");
        return MFALSE;
    }
    else if( isRaw() )
    {
        auto pModule = INormalPipeModule::get();
        if  ( ! pModule ) {
            MY_LOGE("INormalPipeModule::get() fail");
            return MFALSE;
        }

        MINT32 imgFormat = eImgFmt_BAYER12;
        MUINT32 queryLpBitFmt = CAM_Pipeline_12BITS;

        getLpModeSupportBitDepthFormat(imgFormat, queryLpBitFmt);

        if(queryLpBitFmt & CAM_Pipeline_14BITS)
        {
            bitDepth = 12;
            MY_LOGD("pipeline bit depth support 14 bits => recommend raw bit 12 bits");
        }
        else
        {
            bitDepth = 10;
            MY_LOGD("recommend raw bit 10 bits");
        }
    }
    else
    {
        MY_LOGE("sensorType not supported yet(0x%x)", mSensorStaticInfo.sensorType);
        return MFALSE;
    }
    return MTRUE;

}

/******************************************************************************
 *
 ******************************************************************************/
MBOOL
HwInfoHelper::
getPorcessRawInfo(MINT& fmt)
{
    return  IMPL_CLASSNAME::getPorcessRawInfo(fmt);
}

MBOOL
IMPL_CLASSNAME::
getPorcessRawInfo(MINT& fmt)
{
    #if 0
    {
        auto pModule = INormalPipeModule::get();
        MBOOL ret = MFALSE;
        if  ( ! pModule ) {
            MY_LOGE("INormalPipeModule::get() fail");
            return MFALSE;
        }

        sCAM_QUERY_PROCESS_FMT res;
        res.QueryInput.rawOutFmt = EPipe_PROCESSED_RAW;
        res.QueryOutput.clear();
        ret = pModule->query(
            ENPipeQueryCmd_PROCESS_FMT,
            (MUINTPTR)(&res));
        if (!ret || res.QueryOutput.size() == 0) {
            MY_LOGI("Cannot query ENPipeQueryCmd_PROCESS_FMT");
            return MFALSE;
        }
        MINT32 foundIdx = -1;
        for (int i = 0; i < res.QueryOutput.size(); i++)
        {
            if (res.QueryOutput[i] == fmt)
            {
                foundIdx = i;
                break;
            }
        }
        if (foundIdx == -1)
        {
            MY_LOGW("request format(0x%X) is not support, use driver default(0x%X)", fmt, res.QueryOutput[0]);
            fmt = res.QueryOutput[0];
            return MTRUE;
        }
        else if (foundIdx != 0 && isHalUnpackRawFormat((EImageFormat)fmt))
        {
            MY_LOGI("request format(0x%X) is not driver default, use driver default(0x%X)", fmt, res.QueryOutput[0]);
            fmt = res.QueryOutput[0];
            return MTRUE;
        }
        else
        {
            MY_LOGD("request format(0x%X) is found, driver index(%d)", fmt, foundIdx);
        }
    }
    #endif

    return MTRUE;
}

MBOOL
HwInfoHelper::
getLMVSupported(const MUINT32 &idx, const MUINT32 &type)
{
    return IMPL_CLASSNAME::getLMVSupported(idx, type);

}

MBOOL
IMPL_CLASSNAME::
getLMVSupported(const MUINT32 &idx, const MUINT32 &type)
{
    if( getIspVersionMajor() >= 6) {
        MBOOL isSupportHWLMV = MFALSE;
        MBOOL isSupportVirtualLMV = MFALSE;
        auto pModule = INormalPipeModule::get();
        if( !pModule ) {
            MY_LOGE("INormalPipeModule::get() fail");
            return MFALSE;
        }
        if( !pModule->queryLMVSupported(static_cast<E_INPUT>(type), isSupportHWLMV, isSupportVirtualLMV) ){
            MY_LOGE("queryLMVSupported fail");
            return MFALSE;
        }
        if( isSupportHWLMV || isSupportVirtualLMV ) {
            return MTRUE;
        }
        MY_LOGD("No config LMV: RawType(%d),isSupportHWLMV(%d),isSupportVirtualLMV(%d)",
                 type, isSupportHWLMV, isSupportVirtualLMV);
    }
    else if( idx == 0 )
    {
        return MTRUE;
    }

    return MFALSE;
}

/******************************************************************************
 *
 ******************************************************************************/
MBOOL
HwInfoHelper::
getSensorPowerOnPredictionResult(MBOOL& isPowerOnSuccess) const
{
    return  mpImp->getSensorPowerOnPredictionResult(isPowerOnSuccess);
}
MBOOL
IMPL_CLASSNAME::
getSensorPowerOnPredictionResult(MBOOL& isPowerOnSuccess) const
{
    auto pModule = INormalPipeModule::get();
    if  ( ! pModule ) {
        MY_LOGE("INormalPipeModule::get() fail");
        return MFALSE;
    }
    sCAM_QUERY_ISP_RES  QueryIn;
    QueryIn.QueryInput.sensorIdx = mOpenId;
    QueryIn.QueryInput.scenarioId = SENSOR_SCENARIO_ID_NORMAL_CAPTURE;
    QueryIn.QueryInput.rrz_out_w = DISPLAY_WIDTH;
    QueryIn.QueryInput.pattern = eCAM_NORMAL;
    QueryIn.QueryInput.bin_off = getPDAFSupported(SENSOR_SCENARIO_ID_NORMAL_CAPTURE);
    if(!pModule->query(ENPipeQueryCmd_ISP_RES,(MUINTPTR)&QueryIn)){
        MY_LOGE("ISP Query is not supported");
        isPowerOnSuccess = MTRUE;
        return MFALSE;
    }
    MY_LOGD("SensorId: %d SensorOnPredictionResult: %d",mOpenId,QueryIn.QueryOutput);
    isPowerOnSuccess = QueryIn.QueryOutput;
    return MTRUE;
}
/******************************************************************************
 *
 ******************************************************************************/
MBOOL
HwInfoHelper::
queryPixelMode(
    MUINT32 const sensorMode,
    MINT32 const fps,
    MUINT32& pixelMode
) const
{
    return  mpImp->queryPixelMode(sensorMode, fps, pixelMode);
}

MBOOL
IMPL_CLASSNAME::
queryPixelMode(
    MUINT32 const sensorMode,
    MINT32 const fps,
    MUINT32& pixelMode
) const
{
    IHalSensor* pSensorHalObj = NULL;
    IHalSensorList* const pHalSensorList = MAKE_HalSensorList();
    //
    if( !pHalSensorList ) { MY_LOGE("pHalSensorList == NULL"); return MFALSE; }

    pSensorHalObj = pHalSensorList->createSensor(LOG_TAG, mOpenId);
    if( pSensorHalObj == NULL ) { MY_LOGE("pSensorHalObj is NULL"); return MFALSE; }

    pSensorHalObj->sendCommand(
            pHalSensorList->querySensorDevIdx(mOpenId),
            SENSOR_CMD_GET_SENSOR_PIXELMODE,
            (MUINTPTR)(&sensorMode),
            (MUINTPTR)(&fps),
            (MUINTPTR)(&pixelMode));

    pSensorHalObj->destroyInstance(LOG_TAG);

    if( pixelMode != 0 &&
        pixelMode != 1 &&
        pixelMode != 2 ) {
        MY_LOGE("Un-supported pixel mode %d", pixelMode);
        return MFALSE;
    }

    return MTRUE;
}


/******************************************************************************
 *
 ******************************************************************************/
MBOOL
HwInfoHelper::
alignPass1HwLimitation(
    MUINT32 const pixelMode __unused,
    MINT const imgFormat,
    MBOOL isImgo,
    MSize& size,
    size_t& stride
) const
{
    return  mpImp->alignPass1HwLimitation(imgFormat, isImgo, size, stride);
}


MBOOL
HwInfoHelper::
alignPass1HwLimitation(
    MINT const imgFormat,
    MBOOL isImgo,
    MSize& size,
    size_t& stride
) const
{
    return  mpImp->alignPass1HwLimitation(imgFormat, isImgo, size, stride);
}


MBOOL
IMPL_CLASSNAME::
alignPass1HwLimitation(
    MINT const imgFormat,
    MBOOL isImgo,
    MSize& size,
    size_t& stride
) const
{
    using namespace NSCam::NSIoPipe;
    using namespace NSCam::NSIoPipe::NSCamIOPipe;

    auto pModule = INormalPipeModule::get();
    if  ( ! pModule ) {
        MY_LOGE("INormalPipeModule::get() fail");
        return MFALSE;
    }

    NormalPipe_QueryInfo queryRst;
    NormalPipe_QueryIn input;
    input.width = size.w;
    //input.pixMode = (pixelMode == 0) ? _1_PIX_MODE : _2_PIX_MODE;
    pModule->query(
        isImgo ? PORT_IMGO.index : PORT_RRZO.index,
        ENPipeQueryCmd_X_PIX|
        ENPipeQueryCmd_STRIDE_PIX|
        ENPipeQueryCmd_STRIDE_BYTE,
        imgFormat,
        input,
        queryRst
    );

    // Because of AINR require 64 alignment imgo stride for warping
    // We need to make sure stride is 64 alignment.
#if MTKCAM_HAVE_AINR_SUPPORT
    if(isImgo && imgFormat == eImgFmt_BAYER10) {
        #define ALIGN_PIXEL(w, a) (((w + (a-1)) / a) * a)
        size_t strideInPixel = ALIGN_PIXEL(size.w, 64);
        queryRst.stride_byte = strideInPixel * 1.25;
        #undef ALIGN_PIXEL
        MY_LOGD("Make stride 64 pixel alignemnt (%d)", queryRst.stride_byte);
    }
#endif

    size.w = queryRst.x_pix;
    size.h = ((size.h + 1) & (~1));
    stride = queryRst.stride_byte;
    MY_LOGD("rrzo size %dx%d, stride %zu", size.w, size.h, stride);
    return MTRUE;
}


/******************************************************************************
 *
 ******************************************************************************/
MBOOL
HwInfoHelper::
alignRrzoHwLimitation(
    MSize const targetSize,
    MSize const sensorSize,
    MSize& result,
    MUINT32 const recommendResizeRatio
) const
{
    return  mpImp->alignRrzoHwLimitation(targetSize, sensorSize, result, recommendResizeRatio);
}

MBOOL
IMPL_CLASSNAME::
alignRrzoHwLimitation(
    MSize const targetSize,
    MSize const sensorSize,
    MSize& result,
    MUINT32 const recommendResizeRatio
) const
{

    MUINT32 SupportRatio = 40;
    bool scaledUp = false;
    result = targetSize;

    // figure out the crop region size
    MSize usedRegionSize;
    usedRegionSize.w = sensorSize.w;
    usedRegionSize.h = sensorSize.h;
    // check if the edges are beyond hardware scale limitation(crop region edge * scale ratio)
    // scale up to cope with the limitation, if needed
    querySupportResizeRatio(SupportRatio);

    if (recommendResizeRatio > SupportRatio) {
        MY_LOGD("Modify resize ratio (%u) to (%u) for isp quality", SupportRatio, recommendResizeRatio);
        SupportRatio = recommendResizeRatio;
    }

#define ROUND_UP(x, div) (((x) + (div - 1)) / div)
#define ALIGN16(x) x = (((x) + 15) & ~(15))

    // check the width
    if( (uint32_t)result.w < ROUND_UP(usedRegionSize.w * SupportRatio, 100) )
    {
        result = MSize(ROUND_UP(usedRegionSize.w * SupportRatio, 100),
                result.h * ROUND_UP(usedRegionSize.w * SupportRatio, 100) / result.w);
        scaledUp = true;
        ALIGN16(result.w);
        ALIGN16(result.h);
        MY_LOGD("width is beyond scale limitation, modified size: %dx%d, original target size: %dx%d, crop size: %dx%d", result.w, result.h, targetSize.w, targetSize.h, usedRegionSize.w, usedRegionSize.h);
    }

    // check the height
    if( (uint32_t)result.h < ROUND_UP(usedRegionSize.h * SupportRatio, 100) )
    {
        result = MSize(result.w * ROUND_UP(usedRegionSize.h * SupportRatio, 100) / result.h,
                ROUND_UP(usedRegionSize.h * SupportRatio, 100));
        scaledUp = true;
        ALIGN16(result.w);
        ALIGN16(result.h);
        MY_LOGD("height is beyond scale limitation, modified size: %dx%d, original target size: %dx%d, crop size: %dx%d", result.w, result.h, targetSize.w, targetSize.h, usedRegionSize.w, usedRegionSize.h);
    }

    if( !scaledUp )
    {
        // we don't attempt to scale down if scaledUp is true,
        // since it means at least one edge is at the limit
        MSize temp = result;
        if( temp.w > sensorSize.w )
        {
            temp = MSize(sensorSize.w,
                   temp.h * sensorSize.w / temp.w);
        }

        if( temp.h > sensorSize.h )
        {
            temp = MSize(temp.w * sensorSize.h / temp.h,
                   sensorSize.h);
        }

        if( (uint32_t)temp.w > usedRegionSize.w * SupportRatio / 100 && (uint32_t)temp.h > usedRegionSize.h * SupportRatio / 100 )
        {
            result = temp;
            MY_LOGD("exceeding sensor size, modified size: %dx%d, original target size: %dx%d, crop size: %dx%d", result.w, result.h, targetSize.w, targetSize.h, usedRegionSize.w, usedRegionSize.h);
        }
    }
#undef ROUND_UP
#undef ALIGN16
    return MTRUE;
}

/******************************************************************************
 *
 ******************************************************************************/
MBOOL
HwInfoHelper::
quertMaxRrzoWidth(
    MINT32 &maxWidth
) const
{
    return  mpImp->quertMaxRrzoWidth(maxWidth);
}

MBOOL
IMPL_CLASSNAME::
quertMaxRrzoWidth(
    MINT32 &maxWidth
) const
{
#define MAX_RRZO_W    (2560)

    using namespace NSCam::NSIoPipe;
    using namespace NSCam::NSIoPipe::NSCamIOPipe;

    maxWidth = MAX_RRZO_W;
    MUINT32 ret = true;

    auto pModule = INormalPipeModule::get();
    if  ( ! pModule )
    {
        MY_LOGE("INormalPipeModule::get() fail");
        return MFALSE;
    }

    sCAM_QUERY_MAX_PREVIEW_SIZE MaxWidth;
    MaxWidth.QueryOutput = MAX_RRZO_W;
    ret = pModule->query(
        ENPipeQueryCmd_MAX_PREVIEW_SIZE,
        (MUINTPTR)&MaxWidth
    );

    if (!ret)
    {
        MY_LOGW("this platform not support ENPipeQueryCmd_MAX_PREVIEW_SIZE, use default value : %d", MAX_RRZO_W);
        maxWidth = MAX_RRZO_W;
        return MFALSE;
    }
    maxWidth = MaxWidth.QueryOutput;

    return MTRUE;
#undef MAX_RRZO_W
}

/*******************************************************************************
*
********************************************************************************/
MBOOL
HwInfoHelper::
querySupportVHDRMode(
    MUINT32 const sensorMode,
    MUINT32& vhdrMode
) const
{
    return  mpImp->querySupportVHDRMode(sensorMode, vhdrMode);
}

MBOOL
IMPL_CLASSNAME::
querySupportVHDRMode(
    MUINT32 const sensorMode,
    MUINT32& vhdrMode
) const
{

    IHalSensor* pSensorHalObj = NULL;
    IHalSensorList* const pHalSensorList = MAKE_HalSensorList();
    //
    if( !pHalSensorList ) { MY_LOGE("pHalSensorList == NULL"); return MFALSE; }

    pSensorHalObj = pHalSensorList->createSensor(LOG_TAG, mOpenId);
    if( pSensorHalObj == NULL ) { MY_LOGE("pSensorHalObj is NULL"); return MFALSE; }

    pSensorHalObj->sendCommand(
            pHalSensorList->querySensorDevIdx(mOpenId),
            SENSOR_CMD_GET_SENSOR_HDR_CAPACITY,
            (MUINTPTR)(&sensorMode),
            (MUINTPTR)(&vhdrMode),
            (MUINTPTR)0);

    return MTRUE;

}


/******************************************************************************
*
*
********************************************************************************/
MBOOL
HwInfoHelper::
getPDAFSupported(MUINT32 const sensorMode)
const
{
    return  mpImp->getPDAFSupported(sensorMode);
}

MBOOL
IMPL_CLASSNAME::
getPDAFSupported(MUINT32 const sensorMode)
const
{
    MINT32 PDAFSupport = 0;
    /*0: NO PDAF, 1: PDAF Raw Data mode, 2:PDAF VC mode*/
    if (mSensorStaticInfo.PDAF_Support == 1)
    {
        IHalSensor* pSensorHalObj = NULL;
        IHalSensorList* const pHalSensorList = MAKE_HalSensorList();
        //
        if( !pHalSensorList ) { MY_LOGE("pHalSensorList == NULL"); return
MFALSE; }

        pSensorHalObj = pHalSensorList->createSensor(LOG_TAG, mOpenId);
        if( pSensorHalObj == NULL ) { MY_LOGE("pSensorHalObj is NULL");
return MFALSE; }

        /* PDAF capacity enable or not */
        pSensorHalObj->sendCommand(
                pHalSensorList->querySensorDevIdx(mOpenId),
                SENSOR_CMD_GET_SENSOR_PDAF_CAPACITY,
                (MUINTPTR)(&sensorMode),
                (MUINTPTR)(&PDAFSupport),
                0);

        pSensorHalObj->destroyInstance(LOG_TAG);
    }

    return PDAFSupport;

}


/******************************************************************************
*
*
********************************************************************************/
MBOOL
HwInfoHelper::
isType3PDSensorWithoutPDE(MUINT32 const sensorMode, MBOOL const checkSensorOnly)
const
{
    return  mpImp->isType3PDSensorWithoutPDE(sensorMode, checkSensorOnly);
}

MBOOL
IMPL_CLASSNAME::
isType3PDSensorWithoutPDE(MUINT32 const sensorMode, MBOOL const checkSensorOnly)
const
{
    MBOOL ret = MFALSE;
    MINT32 PDAFSupport = 0;

    /*  1: PDAF Raw Data mode, 6: PDAF Raw Legacy Data Mode */
    if (mSensorStaticInfo.PDAF_Support == 1 || mSensorStaticInfo.PDAF_Support == 6)
    {
        if (checkSensorOnly)
        {
            PDAFSupport = 1;
        }
        else
        {
            IHalSensor* pSensorHalObj = NULL;
            IHalSensorList* const pHalSensorList = MAKE_HalSensorList();
            //
            if( !pHalSensorList ) {
                MY_LOGE("pHalSensorList == NULL");
                return ret;
            }

            pSensorHalObj = pHalSensorList->createSensor(LOG_TAG, mOpenId);
            if( pSensorHalObj == NULL ) {
                MY_LOGE("pSensorHalObj is NULL");
                return ret;
            }

            /* PDAF capacity enable or not */
            pSensorHalObj->sendCommand(
                    pHalSensorList->querySensorDevIdx(mOpenId),
                    SENSOR_CMD_GET_SENSOR_PDAF_CAPACITY,
                    (MUINTPTR)(&sensorMode),
                    (MUINTPTR)(&PDAFSupport),
                    0);

            pSensorHalObj->destroyInstance(LOG_TAG);
        }
    }
    // check P1 has PDE or not
    if (PDAFSupport)
    {
        auto pModule = INormalPipeModule::get();
        if  ( ! pModule ) {
            MY_LOGE("INormalPipeModule::get() fail");
            return MFALSE;
        }
        NSCam::NSIoPipe::NSCamIOPipe::sCAM_QUERY_PDO_AVAILABLE pdoAvaiQuery;
        ret = pModule->query((MUINT32)NSCam::NSIoPipe::NSCamIOPipe::ENPipeQueryCmd_PDO_AVAILABLE, (MUINTPTR)&pdoAvaiQuery);
        if (!ret) {
            MY_LOGW("Cannot query sCAM_QUERY_PDO_AVAILABLE from DRV");
        } else {
            ret = pdoAvaiQuery.QueryOutput == MFALSE ? MTRUE : MFALSE;
        }
    }

    MY_LOGD("checkSensorOnly %d, sensorMode %d, mSensorStaticInfo.PDAF_Support %d, PDAFSupport %d, ret %d",
             checkSensorOnly, sensorMode, mSensorStaticInfo.PDAF_Support, PDAFSupport, ret);

    return ret;
}

/******************************************************************************
*
*
********************************************************************************/
MBOOL
HwInfoHelper::
queryUFOStride(
    MINT const imgFormat,
    MSize const imgSize,
    size_t stride[3]
) const
{
    return  mpImp->queryUFOStride(imgFormat, imgSize, stride);
}

MBOOL
IMPL_CLASSNAME::
queryUFOStride(
    MINT const imgFormat __attribute__((__unused__)),
    MSize const imgSize,
    size_t stride[3]
) const
{
    return gQueryUFOStrides(stride, imgFormat, imgSize);
}


/******************************************************************************
*
*
********************************************************************************/
MBOOL
HwInfoHelper::
getDualPDAFSupported(MUINT32 const sensorMode)
const
{
    return  mpImp->getDualPDAFSupported(sensorMode);
}

MBOOL
IMPL_CLASSNAME::
getDualPDAFSupported(MUINT32 const sensorMode)
const
{
    MINT32 PDAFSupport = 0;
    /*0: NO PDAF, 1: PDAF Raw Data mode, 2:PDAF VC mode(Full), 3:PDAF VC mode(
Binning), 4: PDAF DualPD Raw Data mode, 5: PDAF DualPD VC mode*/
    if (mSensorStaticInfo.PDAF_Support == 4)
    {
        IHalSensor* pSensorHalObj = NULL;
        IHalSensorList* const pHalSensorList = MAKE_HalSensorList();
        //
        if( !pHalSensorList ) { MY_LOGE("pHalSensorList == NULL"); return
MFALSE; }

        pSensorHalObj = pHalSensorList->createSensor(LOG_TAG, mOpenId);
        if( pSensorHalObj == NULL ) { MY_LOGE("pSensorHalObj is NULL");
return MFALSE; }

        /* PDAF capacity enable or not */
        pSensorHalObj->sendCommand(
                pHalSensorList->querySensorDevIdx(mOpenId),
                SENSOR_CMD_GET_SENSOR_PDAF_CAPACITY,
                (MUINTPTR)(&sensorMode),
                (MUINTPTR)(&PDAFSupport),
                0);

        pSensorHalObj->destroyInstance(LOG_TAG);
    }

    MY_LOGD("SensorStaticInfo PDAF_Support(%d) DualPDAFSupported(%d)",
        mSensorStaticInfo.PDAF_Support,PDAFSupport);

    return PDAFSupport;

}

/******************************************************************************
*
*
********************************************************************************/
MBOOL
HwInfoHelper::
get4CellSensorSupported()
const
{
    return  mpImp->get4CellSensorSupported();
}


HwInfoHelper::e4CellSensorPattern
HwInfoHelper::
get4CellSensorPattern()
const
{
    return  mpImp->get4CellSensorPattern();
}


MBOOL
IMPL_CLASSNAME::
get4CellSensorSupported()
const
{

   SensorStaticInfo sensorStaticInfo;
    IHalSensorList* const pHalSensorList = MAKE_HalSensorList();
    if( !pHalSensorList )
    {
        MY_LOGE("pHalSensorList == NULL");
        return MFALSE;
    }
    //
    MUINT32 sensorDev = pHalSensorList->querySensorDevIdx(mOpenId);
    pHalSensorList->querySensorStaticInfo(sensorDev,&sensorStaticInfo);
    MUINT32 u4RawFmtType = sensorStaticInfo.rawFmtType;
    MBOOL support = ((u4RawFmtType == SENSOR_RAW_4CELL || u4RawFmtType == SENSOR_RAW_4CELL_BAYER || u4RawFmtType == SENSOR_RAW_4CELL_HW_BAYER) ? MTRUE : MFALSE);
    MY_LOGD("SensorStaticInfo 4CellSensor_Support(type:%d) (support:%d)", u4RawFmtType,support);

    return support;

}

HwInfoHelper::e4CellSensorPattern
IMPL_CLASSNAME::
get4CellSensorPattern()
const
{
    SensorStaticInfo sensorStaticInfo;
    IHalSensorList* const pHalSensorList = MAKE_HalSensorList();
    if( !pHalSensorList )
    {
        MY_LOGE("pHalSensorList == NULL");
        return e4CellSensorPattern_Unknown;
    }
    //
    MUINT32 sensorDev = pHalSensorList->querySensorDevIdx(mOpenId);
    pHalSensorList->querySensorStaticInfo(sensorDev,&sensorStaticInfo);
    MUINT32 u4RawFmtType = sensorStaticInfo.rawFmtType;
    //
    e4CellSensorPattern pattern = e4CellSensorPattern_Unknown;
    const char *pStr = "Not 4cell sensor";
    switch (u4RawFmtType) {
        case SENSOR_RAW_4CELL:
        case SENSOR_RAW_4CELL_BAYER:
            pStr = "Unpacked";
            pattern = e4CellSensorPattern_Unpacked;
            break;

        case SENSOR_RAW_4CELL_HW_BAYER:
            pStr = "Packed";
            pattern = e4CellSensorPattern_Packed;
            break;

        default:
            pattern = e4CellSensorPattern_Unknown;
            break;
    }
    MY_LOGD("SensorStaticInfo 4CellSensor_Pattern(type:%d) (pattern:%s)", u4RawFmtType, pStr);
    return pattern;
}

/******************************************************************************
*
*
********************************************************************************/
MBOOL
HwInfoHelper::
getSensorRawFmtType(MUINT32 &u4RawFmtType)
const
{
    return  mpImp->getSensorRawFmtType(u4RawFmtType);
}

MBOOL
IMPL_CLASSNAME::
getSensorRawFmtType(MUINT32 &u4RawFmtType)
const
{

   SensorStaticInfo sensorStaticInfo;
    IHalSensorList* const pHalSensorList = MAKE_HalSensorList();
    if( !pHalSensorList )
    {
        MY_LOGE("pHalSensorList == NULL");
        return MFALSE;
    }
    //
    MUINT32 sensorDev = pHalSensorList->querySensorDevIdx(mOpenId);
    pHalSensorList->querySensorStaticInfo(sensorDev,&sensorStaticInfo);
    u4RawFmtType = sensorStaticInfo.rawFmtType;
    MY_LOGD("SensorStaticInfo SensorRawFmtType(%d)", u4RawFmtType);

    return MTRUE;

}

/******************************************************************************
 *
 ******************************************************************************/
MBOOL
HwInfoHelper::
getShutterDelayFrameCount(MINT32& shutterDelayCnt)
const
{
    return  mpImp->getShutterDelayFrameCount(shutterDelayCnt);
}
MBOOL
IMPL_CLASSNAME::
getShutterDelayFrameCount(MINT32& shutterDelayCnt)
const
{
    shutterDelayCnt = 0;
    SensorStaticInfo sensorStaticInfo;
    IHalSensorList* const pHalSensorList = MAKE_HalSensorList();
    if( !pHalSensorList )
    {
        MY_LOGE("pHalSensorList == NULL");
        return MFALSE;
    }
    //
    MUINT32 sensorDev = pHalSensorList->querySensorDevIdx(mOpenId);
    pHalSensorList->querySensorStaticInfo(sensorDev,&sensorStaticInfo);
    int i4AeShutDelayFrame = sensorStaticInfo.aeShutDelayFrame;
    int i4AeISPGainDelayFrame = sensorStaticInfo.aeISPGainDelayFrame;
    shutterDelayCnt = (i4AeISPGainDelayFrame - i4AeShutDelayFrame);
    //
    MY_LOGD("i4AeISPGainDelayFrame(%d) i4AeShutDelayFrame(%d) shutterDelayCnt(%d)",
        i4AeISPGainDelayFrame, i4AeShutDelayFrame, shutterDelayCnt);
    //
    return MTRUE;
}

/******************************************************************************
 *
 ******************************************************************************/
MBOOL
HwInfoHelper::
shrinkCropRegion(NSCam::MSize const sensorSize, NSCam::MRect& cropRegion, MINT32 shrinkPx)
const
{
    return  mpImp->shrinkCropRegion(sensorSize, cropRegion, shrinkPx);
}
MBOOL
IMPL_CLASSNAME::
shrinkCropRegion(NSCam::MSize const sensorSize, NSCam::MRect& cropRegion, MINT32 shrinkPx)
const
{
    if((sensorSize.w-shrinkPx) <= cropRegion.width())
    {
        cropRegion.p.x = shrinkPx;
        cropRegion.s.w = (sensorSize.w-shrinkPx*2);
    }
    if((sensorSize.h-shrinkPx) <= cropRegion.height())
    {
        cropRegion.p.y = shrinkPx;
        cropRegion.s.h = (sensorSize.h-shrinkPx*2);
    }
    return MTRUE;
}

/******************************************************************************
 *
 ******************************************************************************/
MBOOL
HwInfoHelper::
querySupportResizeRatio(MUINT32& rPrecentage)
const
{
    MBOOL ret = mpImp->querySupportResizeRatio(rPrecentage);
    return ret;
}

MBOOL
IMPL_CLASSNAME::
querySupportResizeRatio(MUINT32& rPrecentage)
const
{
    using namespace NSCam::NSIoPipe;
    using namespace NSCam::NSIoPipe::NSCamIOPipe;

    auto pModule = INormalPipeModule::get();
    rPrecentage = 40;
    if  ( ! pModule ) {
        MY_LOGE("INormalPipeModule::get() fail, default value = 40");
        return MFALSE;
    }

    MBOOL ret = MFALSE;
    NSCam::NSIoPipe::NSCamIOPipe::sCAM_QUERY_BS_RATIO info;
    info.QueryInput.portId = NSImageio::NSIspio::EPortIndex_RRZO;
    ret = pModule->query(
        ENPipeQueryCmd_BS_RATIO,
        (MUINTPTR)&info);
    if (!ret) {
        MY_LOGW("Cannot query ENPipeQueryCmd_BS_RATIO from DRV, default value = 40");
    } else {
        rPrecentage = info.QueryOutput;
        MY_LOGD("Support Resize-Ratio-Percentage: %d",
            rPrecentage);
    }
    return ret;
}

/******************************************************************************
 *
 ******************************************************************************/
MBOOL
HwInfoHelper::
querySupportBurstNum(MUINT32& rBitField)
const
{
    MBOOL ret = mpImp->querySupportBurstNum(rBitField);
    return ret;
}

MBOOL
IMPL_CLASSNAME::
querySupportBurstNum(MUINT32& rBitField)
const
{
    auto pModule = INormalPipeModule::get();
    if  ( ! pModule ) {
        MY_LOGE("INormalPipeModule::get() fail");
        return MFALSE;
    }

    MBOOL ret = MFALSE;
    sCAM_QUERY_BURST_NUM res;
    res.QueryOutput = 0x0;
    ret = pModule->query(
        NSCam::NSIoPipe::NSCamIOPipe::ENPipeQueryCmd_BURST_NUM,
        (MUINTPTR)(&res));
    if (!ret) {
        MY_LOGW("Cannot query ENPipeQueryCmd_BURST_NUM from DRV");
    } else {
        rBitField = res.QueryOutput;
        rBitField |= 0x1; // always support BurstNum=1
        MY_LOGD("Support Burst-Num-Set: 0x%X (0x%X)",
            rBitField, res.QueryOutput);
    }
    return ret;
}

/******************************************************************************
 *
 ******************************************************************************/
MBOOL
HwInfoHelper::
querySupportRawPattern(MUINT32& rBitField)
const
{
    MBOOL ret = mpImp->querySupportRawPattern(rBitField);
    return ret;
}

MBOOL
IMPL_CLASSNAME::
querySupportRawPattern(MUINT32& rBitField)
const
{
    auto pModule = INormalPipeModule::get();
    if  ( ! pModule ) {
        MY_LOGE("INormalPipeModule::get() fail");
        return MFALSE;
    }

    MBOOL ret = MFALSE;
    sCAM_QUERY_SUPPORT_PATTERN res;
    res.QueryOutput = 0x0;
    ret = pModule->query(
        NSCam::NSIoPipe::NSCamIOPipe::ENPipeQueryCmd_SUPPORT_PATTERN,
        (MUINTPTR)(&res));
    if (!ret) {
        MY_LOGW("Cannot query ENPipeQueryCmd_SUPPORT_PATTERN from DRV");
    } else {
        rBitField = res.QueryOutput;
        MY_LOGD("Support Raw-Pattern-Set: 0x%X",
            rBitField);
    }
    return ret;
}

/******************************************************************************
*
*
********************************************************************************/
MBOOL
HwInfoHelper::
getDynamicTwinSupported()
{
    static MBOOL ret = IMPL_CLASSNAME::getDynamicTwinSupported();
    return ret;
}
MBOOL
IMPL_CLASSNAME::
getDynamicTwinSupported()
{
    auto pModule = INormalPipeModule::get();
    if  ( ! pModule ) {
        MY_LOGE("INormalPipeModule::get() fail");
        return MFALSE;
    }

    MBOOL ret = MFALSE;
    NSCam::NSIoPipe::NSCamIOPipe::NormalPipe_QueryInfo info;
    pModule->query(0, NSCam::NSIoPipe::NSCamIOPipe::ENPipeQueryCmd_D_Twin, 0, 0, info);
    ret = info.D_TWIN;

    MY_LOGD("[%s] is support dynamic twin: %s", __FUNCTION__, ret ? "true" : "false");
    return ret;
}

/******************************************************************************
*
*
********************************************************************************/
MBOOL
HwInfoHelper::
getSecureSensorModeSupported(MUINT32 sensorMode)
{
    return IMPL_CLASSNAME::getSecureSensorModeSupported(sensorMode);
}
MBOOL
IMPL_CLASSNAME::
getSecureSensorModeSupported(MUINT32 sensorMode)
{
    if ((sensorMode != SENSOR_SCENARIO_ID_NORMAL_CAPTURE) &&
        (sensorMode != SENSOR_SCENARIO_ID_NORMAL_PREVIEW))
    {
        MY_LOGW("This API is a temporary solution for checking " \
                "image sensor mode capability under secure flow");
        return MFALSE;
    }

    // NOTE: preview mode is always supported
    if (sensorMode == SENSOR_SCENARIO_ID_NORMAL_PREVIEW)
    {
        MY_LOGD("support secure sensor mode(%u)", sensorMode);
        return MTRUE;
    }

    // check if capture mode is supported or not
    auto pModule = INormalPipeModule::get();
    if  (  !pModule ) {
        MY_LOGE("INormalPipeModule::get() failed");
        return MFALSE;
    }

    sCAM_QUERY_SEC_CAP sSecureCaptureCapability;
    if (!pModule->query(
            static_cast<MUINT32>(ENPipeQueryCmd_SEC_CAP),
            reinterpret_cast<MUINTPTR>(&sSecureCaptureCapability)))
    {
        MY_LOGE("query ENPipeQueryCmd_SEC_CAP failed");
        return MFALSE;
    }

    MY_LOGD("%s support secure sensor mode(%u)",
            sSecureCaptureCapability.QueryOutput ? "" : "does not", sensorMode);

    return sSecureCaptureCapability.QueryOutput;
}

/******************************************************************************
*
*
********************************************************************************/

MUINT32
HwInfoHelper::
getCameraSensorPowerOnCount()
{
    return IMPL_CLASSNAME::getCameraSensorPowerOnCount();
}
MUINT32
IMPL_CLASSNAME::
getCameraSensorPowerOnCount()
{
    IHalSensorList* const pHalSensorList = MAKE_HalSensorList();
    if( !pHalSensorList )
    {
        MY_LOGE("pHalSensorList == NULL");
        return MFALSE;
    }
    // get total physic camera sensor.
    MUINT sensorCount = pHalSensorList->queryNumberOfSensors();
    IHalSensor* pSensorHalObj = nullptr;
    MUINT32 powerOnResult = 0;
    MUINT32 powerOnCount = 0;
    for(MUINT32 i=0;i<sensorCount;++i)
    {
        pSensorHalObj = pHalSensorList->createSensor(LOG_TAG, i);
        if( pSensorHalObj == NULL ) { MY_LOGE("pSensorHalObj is NULL"); return MFALSE; }

        pSensorHalObj->sendCommand(
                pHalSensorList->querySensorDevIdx(i),
                SENSOR_CMD_GET_SENSOR_POWER_ON_STETE,
                (MUINTPTR)(&powerOnResult),
                0,
                0);
        if(powerOnResult > 0) powerOnCount++;
        pSensorHalObj->destroyInstance(LOG_TAG);
    }
    MY_LOGD("powerOnCount(%d) sensorCount(%d)", powerOnCount, sensorCount);
    return powerOnCount;
}
#if (MTKCAM_HAVE_VSDOF_SUPPORT == 1) || (MTKCAM_HAVE_STEREO_CAMERA_SUPPORT == 1)
/******************************************************************************
 *
 ******************************************************************************/
DUAL_IMPL_CLASSNAME::
DualImplementor(MINT32 const openId)
    : IMPL_CLASSNAME(openId)
{
}
/******************************************************************************
 *
 ******************************************************************************/
MBOOL
DUAL_IMPL_CLASSNAME::
alignPass1HwLimitation(
    MINT const imgFormat,
    MBOOL isImgo,
    NSCam::MSize& size,
    size_t& stride
) const
{
    bool bRet = MFALSE;
#if (3 != MTKCAM_HAL_VERSION)
    //
    MINT32 main1Id = -1, main2Id = -1;
    MBOOL isMain1 = MFALSE;
    {
        // get Main1 & Main2 id
        if(!StereoSettingProvider::getStereoSensorIndex(main1Id, main2Id))
        {
            MY_LOGE("fail to get sensor id");
            return MFALSE;
        }
        (main1Id == mOpenId) ? isMain1 = MTRUE : isMain1 = MFALSE;
    }
#endif
    MRect imageCrop;
    // size provider use MUINT32 as stride fmt.
    MUINT32 q_stride;
    if(isImgo)
    {
        bRet = StereoSizeProvider::getInstance()->getPass1Size(
#if (3 == MTKCAM_HAL_VERSION)
                            mOpenId,
#else
                            (isMain1)?(StereoHAL::eSTEREO_SENSOR_MAIN1):(StereoHAL::eSTEREO_SENSOR_MAIN2),
#endif
                            (EImageFormat)imgFormat,
                            NSImageio::NSIspio::EPortIndex_IMGO,
                            StereoHAL::eSTEREO_SCENARIO_CAPTURE, // in this mode, stereo only support zsd.
                            (MRect&)imageCrop,
                            size,
                            q_stride);
        if(!bRet) {
            MY_LOGE("[IMGO] Get Pass1 Size Fail.");
            return MFALSE;
        }
    #if MTKCAM_HAVE_AINR_SUPPORT
        if(imgFormat == eImgFmt_BAYER10) {
            #define ALIGN_PIXEL(w, a) (((w + (a-1)) / a) * a)
            size_t strideInPixel = ALIGN_PIXEL(size.w, 64);
            q_stride = strideInPixel * 1.25;
            MY_LOGD("Make stride 64 alignemnt (%d)", q_stride);
            #undef ALIGN_PIXEL
        }
    #endif
    }
    else
    {
        bRet = StereoSizeProvider::getInstance()->getPass1Size(
#if (3 == MTKCAM_HAL_VERSION)
                            mOpenId,
#else
                            (isMain1)?(StereoHAL::eSTEREO_SENSOR_MAIN1):(StereoHAL::eSTEREO_SENSOR_MAIN2),
#endif
                            (EImageFormat)imgFormat,
                            NSImageio::NSIspio::EPortIndex_RRZO,
                            StereoHAL::eSTEREO_SCENARIO_CAPTURE, // in this mode, stereo only support zsd.
                            (MRect&)imageCrop,
                            size,
                            q_stride);
        if(!bRet) {
            MY_LOGE("[RRZO] Get Pass1 Size Fail.");
            return MFALSE;
        }
    }
    stride = q_stride;
    return MTRUE;
}
/******************************************************************************
 *
 ******************************************************************************/
MBOOL
DUAL_IMPL_CLASSNAME::
alignRrzoHwLimitation(
    NSCam::MSize const targetSize,
    NSCam::MSize const sensorSize,
    NSCam::MSize& result,
    MUINT32 const recommendResizeRatio
) const
{
    // by pass rrzo check, stereo already check in alignPass1HwLimitation
    UNUSED(targetSize);
    UNUSED(sensorSize);
    UNUSED(result);
    UNUSED(recommendResizeRatio);
    return MTRUE;
}
#endif
/******************************************************************************
 *
 ******************************************************************************/
HwInfoHelper::
HwInfoHelper(MINT32 const openId)
{
#if (MTKCAM_HAVE_VSDOF_SUPPORT == 1) || (MTKCAM_HAVE_STEREO_CAMERA_SUPPORT == 1)
    std::string value;
    // if vsdof mode, HWInfoHelper needs create specific one.
    if(StereoSettingProvider::getStereoFeatureMode())
    {
        value = "DualCam HWInfoHelper.";
        mpImp = new DualImplementor(openId);
    }
    else
    {
        value = "Default HWInfoHelper.";
        mpImp = new Implementor(openId);
    }
#if 0
    MY_LOGI("%s ctor(%p)", value.c_str(), mpImp);
#endif
#else
    mpImp = new Implementor(openId);
#endif
}


/******************************************************************************
 *
 ******************************************************************************/
HwInfoHelper::
~HwInfoHelper()
{
#if (MTKCAM_HAVE_VSDOF_SUPPORT == 1) || (MTKCAM_HAVE_STEREO_CAMERA_SUPPORT == 1)
#if 0
    MY_LOGI("dtor(%p)", mpImp);
#endif
#endif
    delete mpImp;
}


/******************************************************************************
 *
 ******************************************************************************/
class HwInfoHelperImpl : public IHwInfoHelper
{
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Data Members.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
protected:  ////
    MINT32 const            mSensorId;
    NSCamHW::HwInfoHelper   mHwInfoHelper;

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Implementation.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:     ////


/**
 *
 */
HwInfoHelperImpl(int32_t const sensorId)
    : mSensorId(sensorId)
    , mHwInfoHelper(sensorId)
{
    MY_LOGD("sensorId:%d", mSensorId);
}


/**
 *
 */
auto
init() -> bool
{
    if (CC_UNLIKELY( ! mHwInfoHelper.updateInfos() )) {
        MY_LOGE("sensorId:%d: Fail on HwInfoHelper::updateInfos()", mSensorId);
        return false;
    }
    return true;
}


/**
 *
 */
auto
getDefaultStride_Pass1(
    size_t& stride,
    int format,
    MSize const& imgSize,
    bool isImgo /*imgo: true, rrzo: false*/
) const -> bool
{
    size_t tmpStride = 0;
    MSize tmpImgSize = imgSize;
    auto ret = mHwInfoHelper.alignPass1HwLimitation(
                    format, isImgo,
                    tmpImgSize/*out*/, tmpStride/*out*/);
    if (CC_UNLIKELY( ! ret )) {
        MY_LOGE("sensorId:%d: Fail on HwInfoHelper::alignPass1HwLimitation()", mSensorId);
        return false;
    }

    if (CC_UNLIKELY( imgSize != tmpImgSize )) {
        MY_LOGE("sensorId:%d: imgSize(%dx%d) != tmpImgSize(%dx%d)",
            mSensorId, imgSize.w, imgSize.h, tmpImgSize.w, tmpImgSize.h);
        return false;
    }

    if (CC_UNLIKELY( tmpStride < (size_t)imgSize.w )) {
        MY_LOGE("sensorId:%d: bad stride:%zu < imgSize.w:%d",
            mSensorId, tmpStride, imgSize.w);
        return false;
    }

    stride = tmpStride;
    return true;
}


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  IHwInfoHelper Interface.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:     ////


/**
 *
 */
auto
getDefaultBufPlanes_Pass1(
    BufPlanes& bufPlanes,
    int format,
    MSize const& imgSize,
    size_t stride
) const -> bool
{
    return gQueryDefaultBufPlanes_Pass1(bufPlanes, format, imgSize, stride);
}


/**
 *
 */
auto
getDefaultBufPlanes_Imgo(
    BufPlanes& bufPlanes,
    int format,
    MSize const& imgSize
) const -> bool
{
    size_t tmpStride = 0;
    return getDefaultStride_Pass1(tmpStride, format, imgSize, true/*isImgo*/)
        && getDefaultBufPlanes_Pass1(bufPlanes, format, imgSize, tmpStride)
            ;
}


/**
 *
 */
auto
getDefaultBufPlanes_Rrzo(
    BufPlanes& bufPlanes,
    int format,
    MSize const& imgSize
) const -> bool
{
    size_t tmpStride = 0;
    return getDefaultStride_Pass1(tmpStride, format, imgSize, false/*isImgo*/)
        && getDefaultBufPlanes_Pass1(bufPlanes, format, imgSize, tmpStride)
            ;
}


/**
 *
 */
auto
queryRawTypeSupport(QueryRawTypeSupportParams& params) const -> void
{
    int32_t majorIspVersion = getIspVersionMajor();
    if ( majorIspVersion >= 0 && majorIspVersion <= 3 ) {
        params.isPostProcRawSupport = false;
        params.isProcImgoSupport = true;
        MY_LOGD("[ISP%d] post-processing raw is unsupported", majorIspVersion);
        return;
    }


    // Check to see whether it's supported or not via querying P1 driver.
    sCAM_QUERY_IQ_LEVEL sIQ;
    sIQ.QueryOutput = false;
    sIQ.QueryInput.vInData.clear();
    for (auto const& in : params.in)
    {
        QueryInData_t queryInData;
        queryInData.sensorIdx = in.sensorId;
        queryInData.scenarioId = in.sensorMode;
        queryInData.rrz_out_w = in.rrzoImgSize.w;
        queryInData.minCamProcessedFps = in.minProcessingFps * 10; //unit x10
        queryInData.pattern = eCAM_NORMAL;
        sIQ.QueryInput.vInData.push_back(queryInData);
        MY_LOGD_IF(1, "sensorId:%d sensorMode:%d minProcessingFps:%d(x10) rrzo:%dx%d",
            in.sensorId, in.sensorMode, in.minProcessingFps,
            in.rrzoImgSize.w, in.rrzoImgSize.h);
    }

    auto pModule = INormalPipeModule::get();
    MY_LOGF_IF(pModule==nullptr, "INormalPipeModule::get() fail");
    pModule->query((MUINT32)ENPipeQueryCmd_IQ_LEVEL, (MUINTPTR)&sIQ);

    if ( ! sIQ.QueryOutput ) {
        params.isPostProcRawSupport = true;
        params.isProcImgoSupport = true;
        MY_LOGW("INormalPipeModule::query(ENPipeQueryCmd_IQ_LEVEL) is unsupported. Maybe it's ISP5 or below.");
        return;
    }


    // [single-cam] IQlv: H, L
    // [multi-cam ] IQlv: H+H, L+H, H+L, L+L
    //
    // Notes:
    //   H: High (non binning => high quality)
    //   L: Low  (binning => low quality)
    bool isProcImgoSupport = false;
    for (size_t i = 0; i < sIQ.QueryInput.vOutData.size(); i++) {
        auto const& vQueryOutData = sIQ.QueryInput.vOutData[i];

        bool isAllHighIQ = true;
        bool isAllSupport = true;

        // [single-cam] vQueryOutData.size()=1
        // [multi-cam ] vQueryOutData.size()=N
        for (size_t j = 0; j < vQueryOutData.size(); j++) {
            auto const& queryOutData = vQueryOutData[j];
            MY_LOGD_IF(1, "IQ Table[%zu/%zu][%zu/%zu] sensorIdx:%d isTwin:%d IQlv:%d clklv:%d result:%d",
                i, sIQ.QueryInput.vOutData.size(),
                j, vQueryOutData.size(),
                queryOutData.sensorIdx,
                queryOutData.isTwin,
                queryOutData.IQlv,
                queryOutData.clklv,
                queryOutData.result);
            isAllHighIQ = isAllHighIQ && queryOutData.IQlv;
            isAllSupport = isAllSupport && queryOutData.result;
        }

        // supported or not?
        //
        // [single-cam] IQlv: H   --> result == true for the device in vQueryOutData.
        // [multi-cam ] IQlv: H+H --> result == true for all devices in vQueryOutData.
        if ( isAllHighIQ ) {
            isProcImgoSupport = isAllSupport;
            break;
        }
    }

    params.isPostProcRawSupport = true;
    params.isProcImgoSupport = isProcImgoSupport;
    MY_LOGD_IF(!isProcImgoSupport, "imgo cannot outpu processed raw");
    return;
}


/**
 *
 */
auto
queryP1DrvIspCapability(QueryP1DrvIspParams& params) const -> void
{
    // Check to see whether it's supported or not via querying P1 driver.
    sCAM_QUERY_IQ_LEVEL sIQ;
    sIQ.QueryOutput = false;
    sIQ.QueryInput.vInData.clear();
    for (auto const& in : params.in)
    {
        QueryInData_t queryInData;
        queryInData.sensorIdx = in.sensorId;
        queryInData.scenarioId = in.sensorMode;
        queryInData.rrz_out_w = in.rrzoImgSize.w;
        queryInData.minCamProcessedFps = in.minProcessingFps * 10; //unit x10
        queryInData.pattern = eCAM_NORMAL;
        sIQ.QueryInput.vInData.push_back(queryInData);
        MY_LOGD_IF(1, "sensorId:%d sensorMode:%d minProcessingFps:%d(x10) rrzo:%dx%d",
            in.sensorId, in.sensorMode, in.minProcessingFps,
            in.rrzoImgSize.w, in.rrzoImgSize.h);
    }

    auto pModule = INormalPipeModule::get();
    MY_LOGF_IF(pModule==nullptr, "INormalPipeModule::get() fail");
    auto bRet = pModule->query((MUINT32)ENPipeQueryCmd_IQ_LEVEL, (MUINTPTR)&sIQ);

    if ( ! sIQ.QueryOutput || (bRet == false) ) {
        MY_LOGE("isp quality function not support in this platform, query from stereo_setting_provider");
        NSCam::IHwInfoHelper::QueryP1DrvIspParams::P1DrvIspOutputParams outParam;
#if (MTKCAM_HAVE_VSDOF_SUPPORT == 1) || (MTKCAM_HAVE_STEREO_CAMERA_SUPPORT == 1)
#if (3 == MTKCAM_HAL_VERSION)
        auto resizeQuality = StereoSettingProvider::getVSDoFP1ResizeQuality();
        if(resizeQuality == NSCam::v3::P1Node::RESIZE_QUALITY_L){
            outParam.eIqLevel = eCamIQ_L;
        }
        else if(resizeQuality == NSCam::v3::P1Node::RESIZE_QUALITY_H){
            outParam.eIqLevel = eCamIQ_H;
        }
        else{
            outParam.eIqLevel = eCamIQ_MAX;
        }
#else
        outParam.eIqLevel = eCamIQ_MAX;
#endif
#endif

        for(size_t i=0;i<params.in.size();i++) {
            params.out.push_back(outParam);
        }
        return;
    }

    auto getMainSuitableQuality = [&sIQ]() {
        std::vector<uint32_t> vIndex;
        MUINT32 minClk = std::numeric_limits<MUINT32>::max();
        if(sIQ.QueryInput.vOutData.size() == 0) return vIndex;
        // step 1, query rule: main->H, min clk
        auto p1DrvCapability_main = sIQ.QueryInput.vOutData[0];
        for(size_t i=0;i<sIQ.QueryInput.vOutData.size();i++) {
            if(sIQ.QueryInput.vOutData[i].size() == 0) continue;
            if(sIQ.QueryInput.vOutData[i][0].IQlv == eCamIQ_H) {
                if(sIQ.QueryInput.vOutData[i][0].clklv < minClk) {
                    if(sIQ.QueryInput.vOutData[i][0].result == true) {
                        // clear previous index
                        vIndex.clear();
                        vIndex.push_back(i);
                        minClk = sIQ.QueryInput.vOutData[i][0].clklv;
                    }
                }
                else if(sIQ.QueryInput.vOutData[i][0].clklv == minClk) {
                    vIndex.push_back(i);
                }
            }
        }
        return vIndex;
    };

    auto getP1IspIqFromResult = [&sIQ] (
                                    uint32_t sensorIndex,
                                    uint32_t index,
                                    E_CamIQLevel &IQresult,
                                    CAM_RESCONFIG &camResConfig,
                                    MBOOL &result) {
        if(index >= sIQ.QueryInput.vOutData.size())
            return false;
        if(sensorIndex >= sIQ.QueryInput.vOutData[index].size())
            return false;
        IQresult = sIQ.QueryInput.vOutData[index][sensorIndex].IQlv;
        camResConfig = sIQ.QueryInput.vOutData[index][sensorIndex].camResConfig;
        result = sIQ.QueryInput.vOutData[index][sensorIndex].result;
        return true;
    };

    // multi-cam quality decision
    if(sIQ.QueryInput.vInData.size() > 1)
    {
        // get main cam suitable iq level index.
        // condition: IQ=H, min clk
        auto acceptMainIndex = getMainSuitableQuality();
        if(acceptMainIndex.size() == 0) {
            MY_LOGE("please check flow.");
            return;
        }
        // get max H count from acceptMainIndex list.
        uint32_t max_h_count = 0;
        uint32_t max_index = acceptMainIndex[0];
        // main cam no needs to check, index start from 1.
        for(auto&& index:acceptMainIndex) {
            uint32_t h_count = 0;
            for(size_t i=1;i<sIQ.QueryInput.vInData.size();i++) {
                E_CamIQLevel IQresult = eCamIQ_MAX;
                MBOOL result = false;
                CAM_RESCONFIG resConfig;
                if(getP1IspIqFromResult(i, index, IQresult, resConfig, result)) {
                    MY_LOGD("Table index(%zu) select(%d) IQ(%d) tg(%d) result(%d)"
                        , i, index, IQresult, resConfig.Bits.targetTG, result);
                    if(IQresult == eCamIQ_H && result == true)
                    {
                        h_count++;
                    }
                }
            }
            if(h_count >= max_h_count) {
                max_index = index;
                max_h_count = h_count;
            }
        }
        //
        NSCam::IHwInfoHelper::QueryP1DrvIspParams::P1DrvIspOutputParams outParam;
        for(size_t i=0;i<sIQ.QueryInput.vInData.size();i++) {
            E_CamIQLevel IQresult = eCamIQ_MAX;
            CAM_RESCONFIG resConfig;
            MBOOL result = false;
            if(getP1IspIqFromResult(i, max_index, IQresult, resConfig, result)) {
                outParam.eIqLevel = IQresult;
                outParam.eResConfig = resConfig;
                params.out.push_back(outParam);
                MY_LOGD("index(%zu) select(%d) IQ(%d) tg(%d)", i, max_index, IQresult, resConfig.Bits.targetTG);
            }
            else
            {
                outParam.eIqLevel = IQresult;
                outParam.eResConfig = resConfig;
                params.out.push_back(outParam);
                MY_LOGD("n index(%zu) IQ(%d) tg(%d)", i, IQresult, resConfig.Bits.targetTG);
            }
        }
    }
    else
    {
        NSCam::IHwInfoHelper::QueryP1DrvIspParams::P1DrvIspOutputParams outParam;
        E_CamIQLevel IQresult = eCamIQ_MAX;
        CAM_RESCONFIG resConfig;
        uint32_t sensorIndex = 0;
        uint32_t index = 0;
        MBOOL result = false;
        if(getP1IspIqFromResult(sensorIndex, index, IQresult, resConfig, result)) {
            outParam.eIqLevel = IQresult;
            outParam.eResConfig = resConfig;
            params.out.push_back(outParam);
            MY_LOGD("index(%d) select(%d) IQ(%d) tg(%d)", sensorIndex, index, IQresult, resConfig.Bits.targetTG);
        }
        else
        {
            outParam.eIqLevel = IQresult;
            outParam.eResConfig = resConfig;
            params.out.push_back(outParam);
            MY_LOGW("n index(%zu) IQ(%d) tg(%d)", sensorIndex, IQresult, resConfig.Bits.targetTG);
        }
    }
    return;
}


};//HwInfoHelperImpl


/******************************************************************************
 *
 ******************************************************************************/
class HwInfoHelperManagerImpl : public IHwInfoHelperManager
{
protected:  ////    Data Members.
    mutable std::map<int32_t, std::shared_ptr<HwInfoHelperImpl>>
                        mInstanceMap;
    mutable std::mutex  mInstanceMapLock;

public:     ////    Interfaces.

    virtual auto    getHwInfoHelper(
                        int32_t const sensorId
                    ) const -> std::shared_ptr<IHwInfoHelper>
                    {
                        std::lock_guard<std::mutex> _l(mInstanceMapLock);

                        auto it = mInstanceMap.find(sensorId);
                        if (CC_LIKELY( it != mInstanceMap.end() )) {
                            return std::static_pointer_cast<IHwInfoHelper>(it->second);
                        }

                        // first time.
                        auto pHwInfoHelper = std::make_shared<HwInfoHelperImpl>(sensorId);
                        if  (CC_UNLIKELY( pHwInfoHelper==nullptr )) {
                            MY_LOGE("sensorId:%d: Fail on std::make_shared<HwInfoHelperImpl>", sensorId);
                            return nullptr;
                        }
                        if  (CC_UNLIKELY( ! pHwInfoHelper->init() )) {
                            MY_LOGE("sensorId:%d: Fail on init()", sensorId);
                            pHwInfoHelper = nullptr;
                            return nullptr;
                        }

                        /**
                         * [FIXME] We have to create a new HwInfoHelperImpl instance every time,
                         * since its member could be different
                         * (HwInfoHelper::Implementor or HwInfoHelper::DualImplementor)
                         * depending on the session configuration.
                         */
                        //mInstanceMap[sensorId] = pHwInfoHelper;
                        return std::static_pointer_cast<IHwInfoHelper>(pHwInfoHelper);
                    }


/**
 *
 */
virtual auto
getDefaultBufPlanes_JpegYuv(
    BufPlanes& bufPlanes,
    int format,
    MSize const& imgSize
) const -> bool
{
    #define ALIGN(_value_, _align_) (((_value_) + (_align_-1)) & ~(_align_-1))

    auto addBufPlane = [](size_t idx, BufPlanes& bufPlanes, size_t height, size_t stride){
        bufPlanes.planes[idx] = {
                .sizeInBytes = (height * stride),
                .rowStrideInBytes = stride,
            };
    };

    MY_LOGF_IF(imgSize.w<=0 || imgSize.h<= 0, "bad imgSize:%dx%d", imgSize.w, imgSize.h);
    switch (format)
    {
    case eImgFmt_NV12:
    case eImgFmt_NV21:{
        // 16x16 byte alignment for the constraint of yuv input to Jpeg HW
        auto w = ALIGN(imgSize.w, 16);
        auto h = ALIGN(imgSize.h, 16);
        bufPlanes.count = 2;
        addBufPlane(0, bufPlanes, h      , w);
        addBufPlane(1, bufPlanes, h >> 1 , w);
        }break;

    case eImgFmt_YUY2:{
        // 32x8 byte alignment for the constraint of yuv input to Jpeg HW
        bufPlanes.count = 1;
        addBufPlane(0, bufPlanes, ALIGN(imgSize.h, 8), ALIGN(imgSize.w<<1, 32));
        }break;

    case eImgFmt_YV12:{
        bufPlanes.count = 3;
        addBufPlane(0, bufPlanes, imgSize.h      , imgSize.w);
        addBufPlane(1, bufPlanes, imgSize.h >> 1 , imgSize.w >> 1);
        addBufPlane(2, bufPlanes, imgSize.h >> 1 , imgSize.w >> 1);
        }break;

    default:
        MY_LOGE("unsupported format:%#x", format);
        return false;
        break;
    }

    return true;
    #undef ALIGN
}


/**
 *
 */
virtual auto
getDefaultBufPlanes_Pass1(
    BufPlanes& bufPlanes,
    int format,
    MSize const& imgSize,
    size_t stride
) const -> bool
{
    return gQueryDefaultBufPlanes_Pass1(bufPlanes, format, imgSize, stride);
}


};  //class HwInfoHelperManagerImpl


auto IHwInfoHelperManager::get() -> IHwInfoHelperManager*
{
    static HwInfoHelperManagerImpl inst;
    return &inst;
}


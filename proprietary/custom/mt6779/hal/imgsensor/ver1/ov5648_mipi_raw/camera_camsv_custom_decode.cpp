#include <utils/Log.h>
#include <fcntl.h>
#include <cutils/log.h>
#include <string.h>

#include "camera_custom_nvram.h"
#include "camera_custom_sensor.h"
#include "image_sensor.h"
#include "kd_imgsensor_define.h"

//camera_info_header
#include "camera_info_ov5648mipiraw.h"

#if MTK_CAM_NEW_NVRAM_SUPPORT
    #include INCLUDE_FILENAME_TUNING_MAPPING
#endif

typedef NSFeature::CamsvCustomDecode<SENSOR_ID, MODULE_INDEX> SensorInfoSingleton_T;

namespace NSFeature {

CAMSV_CUSTOM_BUFFER_INFO getAeBufSize_ov5648mipiraw();
CAMSV_CUSTOM_BUFFER_INFO getFlkBufsize_ov5648mipiraw();
UINT32 DecodeAe_ov5648mipiraw(VOID* const pData);
UINT32 DecodeFlk_ov5648mipiraw(VOID* const pData);

template <>
CAMSV_CUSTOM_BUFFER_INFO
SensorInfoSingleton_T::
impCamsvBufSize(CAMSV_CUSTOM_DECODE_ENUM const CamsvCusDecodeFeature) const
{
    CAMSV_CUSTOM_BUFFER_INFO buf_size = {0};
    switch(CamsvCusDecodeFeature)
    {
        case CAMSV_CUSTOM_DECODE_AE:
            buf_size = getAeBufSize_ov5648mipiraw();
            break;
        case CAMSV_CUSTOM_DECODE_FLK:
            buf_size = getFlkBufsize_ov5648mipiraw();
            break;
        default:
            ALOGD("[impCamsvBufSize] undefined function type");
            break;
    }
    return buf_size;
}

CAMSV_CUSTOM_BUFFER_INFO getAeBufSize_ov5648mipiraw()
{
    return {0};
}

CAMSV_CUSTOM_BUFFER_INFO getFlkBufsize_ov5648mipiraw()
{
    return {0};
}

template <>
UINT32
SensorInfoSingleton_T::
impCamsvCustomDecode(CAMSV_CUSTOM_DECODE_ENUM const CamsvCusDecodeFeature, VOID* const pData) const
{
    switch(CamsvCusDecodeFeature)
    {
        case CAMSV_CUSTOM_DECODE_AE:
            DecodeAe_ov5648mipiraw(pData);
            break;
        case CAMSV_CUSTOM_DECODE_FLK:
            DecodeFlk_ov5648mipiraw(pData);
            break;
        default:
            ALOGD("[impCamsvCustomDecode] undefined function type");
            break;
    }
    return 0;
}

UINT32 DecodeAe_ov5648mipiraw(VOID* const pData)
{
    // to do for AE decoding
    return 0;
}

UINT32 DecodeFlk_ov5648mipiraw(VOID* const pData)
{
    // to do for flicker decoding
    return 0;
}

};  //  NSFeature


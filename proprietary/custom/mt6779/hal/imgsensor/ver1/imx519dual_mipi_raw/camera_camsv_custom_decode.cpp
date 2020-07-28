#include <cstdio>
#include <cstdlib>
#include <utils/Log.h>
#include <fcntl.h>
#include <cutils/log.h>
#include <string.h>
#include <cutils/properties.h>
#include <unordered_map>

#include "camera_custom_ae_nvram.h"
#include "camera_custom_nvram.h"
#include "camera_custom_sensor.h"
#include "image_sensor.h"
#include "kd_imgsensor_define.h"

//camera_info_header
#include "camera_info_imx519dualmipiraw.h"

#if MTK_CAM_NEW_NVRAM_SUPPORT
    #include INCLUDE_FILENAME_TUNING_MAPPING
#endif

#define MVHDR3EXPO_WIDTH  8
#define MVHDR3EXPO_HEIGHT 6
#define CLAMP(x,min,max) (((x) > (max)) ? (max) : (((x) < (min)) ? (min) : (x)))
#define MY_LOGD(fmt, arg...)    ALOGD(fmt, ##arg)
#define MY_LOGE(fmt, arg...)    ALOGE(fmt, ##arg)
#define MY_LOG_IF(cond, ...)    do { if ( (cond) ) { MY_LOGD(__VA_ARGS__); } }while(0)

typedef NSFeature::CamsvCustomDecode<SENSOR_ID, MODULE_INDEX> SensorInfoSingleton_T;

namespace NSFeature {

CAMSV_CUSTOM_BUFFER_INFO getAeBufSize_imx519dualmipiraw();
CAMSV_CUSTOM_BUFFER_INFO getFlkBufsize_imx519dualmipiraw();
UINT32 DecodeAe_imx519dualmipiraw(VOID* const pData);
UINT32 DecodeFlk_imx519dualmipiraw(VOID* const pData);

template <>
CAMSV_CUSTOM_BUFFER_INFO
SensorInfoSingleton_T::
impCamsvBufSize(CAMSV_CUSTOM_DECODE_ENUM const CamsvCusDecodeFeature) const
{
    CAMSV_CUSTOM_BUFFER_INFO buf_size = {0};
    switch(CamsvCusDecodeFeature)
    {
        case CAMSV_CUSTOM_DECODE_AE:
            buf_size = getAeBufSize_imx519dualmipiraw();
            break;
        case CAMSV_CUSTOM_DECODE_FLK:
            buf_size = getFlkBufsize_imx519dualmipiraw();
            break;
        default:
            ALOGD("[impCamsvBufSize] undefined function type");
            break;
    }
    return buf_size;
}

CAMSV_CUSTOM_BUFFER_INFO getAeBufSize_imx519dualmipiraw()
{
    return {0};
}

CAMSV_CUSTOM_BUFFER_INFO getFlkBufsize_imx519dualmipiraw()
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
            DecodeAe_imx519dualmipiraw(pData);
            break;
        case CAMSV_CUSTOM_DECODE_FLK:
            DecodeFlk_imx519dualmipiraw(pData);
            break;
        default:
            ALOGD("[impCamsvCustomDecode] undefined function type");
            break;
    }
    return 0;
}

UINT32 DecodeAe_imx519dualmipiraw(VOID* const pData)
{
    return 0;
}

UINT32 DecodeFlk_imx519dualmipiraw(VOID* const pData)
{
    // to do for flicker decoding
    return 0;
}

};  //  NSFeature


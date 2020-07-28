#include <utils/Log.h>
#include <fcntl.h>
#include <math.h>
#include <cutils/log.h>
#include <string.h>

#include "camera_custom_nvram.h"
#include "camera_info_ov5648mipiraw.h"
#include "camera_custom_sensor.h"
#include "image_sensor.h"
#include "kd_imgsensor_define.h"
#include "camera_AE_PLineTable_ov5648mipiraw.h"
#include "camera_AWB_Sync_param_ov5648mipiraw.h"
#include "camera_info_ov5648mipiraw.h"
#include <custom/aaa/AEPlinetable.h>
#include "camera_custom_tsf_tbl.h"

#if MTK_CAM_NEW_NVRAM_SUPPORT
    #include INCLUDE_FILENAME_TUNING_MAPPING
#endif

#include "ov5648mipiraw_Info.h"

#define NVRAM_TUNING_PARAM_NUM  6181001

extern "C" fptrDefault getDefaultDataFunc(MUINT32 sensorType)
{
    if(sensorType==0)
        return NSFeature::RAWSensorInfo<SENSOR_ID, MODULE_INDEX>::getDefaultData;
    else
        return NSFeature::YUVSensorInfo<SENSOR_ID, MODULE_INDEX>::getDefaultData;
}

const NVRAM_CAMERA_ISP_PARAM_STRUCT CAMERA_ISP_DEFAULT_VALUE =
{{
    //Version
    .Version= NVRAM_CAMERA_PARA_FILE_VERSION,

    //SensorId
    .SensorId= SENSOR_ID,
    .ISPComm={
      {
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      }
    },
    .ISPRegs={
#include INCLUDE_FILENAME_ISP_COMM_PARAM
#include INCLUDE_FILENAME_ISP_REGS_PARAM
    },
    .ISPColorTbl= {
#include INCLUDE_FILENAME_ISP_COLORTBL_PARAM
    },
    .ISPToneMap={
#include INCLUDE_FILENAME_ISP_TONEMAP_PARAM
    },
    .DngMetadata=
    {
        0,  //i4RefereceIlluminant1
        3,  //i4RefereceIlluminant2
        {
            // rNoiseProfile[4]
            {
                {3.530980e-6, 3.357493e-4},
                {8.098436e-8, -2.336529e-4},
            },
            {
                {3.530980e-6, 3.357493e-4},
                {8.098436e-8, -2.336529e-4},
            },
            {
                {3.530980e-6, 3.357493e-4},
                {8.098436e-8, -2.336529e-4},
            },
            {
                {3.530980e-6, 3.357493e-4},
                {8.098436e-8, -2.336529e-4},
            },
        },
    }
}};

const NVRAM_CAMERA_3A_STRUCT CAMERA_3A_NVRAM_DEFAULT_VALUE =
{
    #include INCLUDE_FILENAME_3A_PARA
    .AWBSYNC = {
        g_rAWBSyncTuningParam,
    },
};

const NVRAM_LENS_PARA_STRUCT CAMERA_AF_NVRAM_DEFAULT_VALUE =
{
    #include INCLUDE_FILENAME_AF_PARA
};

#include INCLUDE_FILENAME_ISP_LSC_PARAM
//};  //  namespace

const CAMERA_TSF_TBL_STRUCT CAMERA_TSF_DEFAULT_VALUE =
{
    {
        1,  // isTsfEn
        2,  // tsfCtIdx
        {20, 2000, -110, -110, 512, 512, 512, 0}    // rAWBInput[8]
    },

#include INCLUDE_FILENAME_TSF_PARA
#include INCLUDE_FILENAME_TSF_DATA
};

const NVRAM_CAMERA_FEATURE_STRUCT CAMERA_FEATURE_DEFAULT_VALUE =
{
    #include INCLUDE_FILENAME_GIS_PARAM
    #include INCLUDE_FILENAME_FEATURE_PARA
};

const NVRAM_CAMERA_STROBE_STRUCT CAMERA_FLASH_DEFAULT_VALUE =
{
#include INCLUDE_FILENAME_FLASH_PARA
};

const NVRAM_CAMERA_FLASH_CALIBRATION_STRUCT CAMERA_FLASH_CALIBRATION_DEFAULT_VALUE =
{
#include INCLUDE_FILENAME_FLASH_CALIBRATION
};

typedef NSFeature::RAWSensorInfo<SENSOR_ID, MODULE_INDEX> SensorInfoSingleton_T;


namespace NSFeature {
  template <>
  UINT32
  SensorInfoSingleton_T::
  impGetDefaultData(CAMERA_DATA_TYPE_ENUM const CameraDataType, VOID*const pDataBuf, UINT32 const size) const
  {
    UINT32 dataSize[CAMERA_DATA_TYPE_NUM] = {sizeof(NVRAM_CAMERA_ISP_PARAM_STRUCT),
        sizeof(NVRAM_CAMERA_3A_STRUCT),
        sizeof(NVRAM_CAMERA_SHADING_STRUCT),
        sizeof(NVRAM_LENS_PARA_STRUCT),
        sizeof(AE_PLINETABLE_T),
        sizeof(NVRAM_CAMERA_STROBE_STRUCT),
        sizeof(CAMERA_TSF_TBL_STRUCT),
        0,
        0,
        0,
#if MTK_CAM_NEW_NVRAM_SUPPORT
        sizeof(NVRAM_CAMERA_IDX_STRUCT),
#endif
        sizeof(NVRAM_CAMERA_FEATURE_STRUCT),
        0,
        sizeof(NVRAM_CAMERA_FLASH_CALIBRATION_STRUCT)
    };

    if (CameraDataType > CAMERA_NVRAM_DATA_FLASH_CALIBRATION || NULL == pDataBuf || (size != dataSize[CameraDataType]))
    {
      ALOGE("Wrong tuning structure mapping table, please check");
      return 1;
    }

    // ALOGI("Muse debug %d", CAMERA_ISP_DEFAULT_VALUE.ISPRegs.UDM[0].intp_crs.val);

    switch(CameraDataType)
    {
      case CAMERA_NVRAM_DATA_ISP:
        memcpy(pDataBuf,&CAMERA_ISP_DEFAULT_VALUE,sizeof(NVRAM_CAMERA_ISP_PARAM_STRUCT));
        break;
      case CAMERA_NVRAM_DATA_3A:
        memcpy(pDataBuf,&CAMERA_3A_NVRAM_DEFAULT_VALUE,sizeof(NVRAM_CAMERA_3A_STRUCT));
        break;
      case CAMERA_NVRAM_DATA_LENS:
        memcpy(pDataBuf,&CAMERA_AF_NVRAM_DEFAULT_VALUE,sizeof(NVRAM_LENS_PARA_STRUCT));
        break;
      case CAMERA_NVRAM_DATA_SHADING:
        memcpy(pDataBuf,&CAMERA_SHADING_DEFAULT_VALUE,sizeof(NVRAM_CAMERA_SHADING_STRUCT));
        break;
      case CAMERA_DATA_AE_PLINETABLE:
        memcpy(pDataBuf,&g_PlineTableMapping,sizeof(AE_PLINETABLE_T));
        break;
      case CAMERA_NVRAM_DATA_STROBE:
        memcpy(pDataBuf,&CAMERA_FLASH_DEFAULT_VALUE,sizeof(NVRAM_CAMERA_STROBE_STRUCT));
        break;
      case CAMERA_DATA_TSF_TABLE:
        memcpy(pDataBuf,&CAMERA_TSF_DEFAULT_VALUE,sizeof(CAMERA_TSF_TBL_STRUCT));
        break;
      case CAMERA_NVRAM_DATA_FEATURE:
        memcpy(pDataBuf,&CAMERA_FEATURE_DEFAULT_VALUE,sizeof(NVRAM_CAMERA_FEATURE_STRUCT));
        break;
      case CAMERA_NVRAM_DATA_FLASH_CALIBRATION:
        memcpy(pDataBuf,&CAMERA_FLASH_CALIBRATION_DEFAULT_VALUE,sizeof(NVRAM_CAMERA_FLASH_CALIBRATION_STRUCT));
        break;
#if MTK_CAM_NEW_NVRAM_SUPPORT
      case CAMERA_NVRAM_IDX_TBL:
        memcpy(pDataBuf,&idx_mod_array,sizeof(NVRAM_CAMERA_IDX_STRUCT));
        break;
#endif
      default:
        break;
    }
    return 0;
  }};  //  NSFeature



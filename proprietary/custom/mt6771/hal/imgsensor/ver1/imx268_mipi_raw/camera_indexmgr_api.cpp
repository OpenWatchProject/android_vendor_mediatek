#include <utils/Log.h>
#include <fcntl.h>
#include <math.h>
#include <cutils/log.h>
#include <string.h>

#include "camera_custom_nvram.h"
#include "camera_custom_sensor.h"
#include "image_sensor.h"
#include "kd_imgsensor_define.h"

//camera_info_header
#include "camera_info_imx268mipiraw.h"


#if MTK_CAM_NEW_NVRAM_SUPPORT
    #include INCLUDE_FILENAME_TUNING_MAPPING
#endif

//#include "imx338mipiraw_Info.h"

//#define NVRAM_TUNING_PARAM_NUM  6181001


typedef NSFeature::SensorInfoIdxMgr<SENSOR_ID, MODULE_INDEX> SensorInfoSingleton_T;


namespace NSFeature {
  template <>
  UINT32
  SensorInfoSingleton_T::
  impGetDefaultData(CAMERA_DATA_TYPE_ENUM const CameraDataType, VOID*const pDataBuf, UINT32 const size) const
  {
    if (CameraDataType > CAMERA_NVRAM_DATA_FLASH_CALIBRATION || NULL == pDataBuf || (size != sizeof(NVRAM_CAMERA_IDX_STRUCT)))
    {
      ALOGE("Wrong tuning structure mapping table, please check");
      return 1;
    }

    switch(CameraDataType)
    {
#if MTK_CAM_NEW_NVRAM_SUPPORT
      case CAMERA_NVRAM_IDX_TBL:
        ALOGE("Read CAMERA_NVRAM_IDX_TBL");
        memcpy(pDataBuf,&idx_mod_array,sizeof(NVRAM_CAMERA_IDX_STRUCT));
        break;
#endif
      default:
        ALOGE("Undefine data type");
        break;
    }
    return 0;
  }};  //  NSFeature



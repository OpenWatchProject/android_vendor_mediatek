#include "camera_custom_nvram.h"
#include "string.h"

int cust_NvramNeedCombineDef(int id)
{
    if((id==CAMERA_NVRAM_DATA_3A) || (id==CAMERA_NVRAM_DATA_STROBE) || (id == CAMERA_NVRAM_DATA_LENS))
    {
        return 1;
        //return 1;
    }
    else
    {
        return 0;
    }
}

//def: bin or default of data
//nvram: read from nvram if it exists.
//purpose: combine def and nvram and output to def for data accessing
void cust_NvramCombine(int id, void* def, void* nvram)
{
  if(id==CAMERA_NVRAM_DATA_3A)
  {
    memcpy(& ((NVRAM_CAMERA_3A_STRUCT*)def)->rFlashAWBNVRAM.rCalibrationData, &((NVRAM_CAMERA_3A_STRUCT*)nvram)->rFlashAWBNVRAM.rCalibrationData, sizeof(FLASH_AWB_CALIBRATION_DATA_STRUCT));
  }
  else if(id==CAMERA_NVRAM_DATA_STROBE)
  {
#ifdef FLASHLIGHT_OFFSET_CALI
    memcpy( ((NVRAM_CAMERA_STROBE_STRUCT*)def)->engTab.dutyOffsetMA, ((NVRAM_CAMERA_STROBE_STRUCT*)nvram)->engTab.dutyOffsetMA, 40*sizeof(short));
    memcpy( ((NVRAM_CAMERA_STROBE_STRUCT*)def)->engTab.yTab, ((NVRAM_CAMERA_STROBE_STRUCT*)nvram)->engTab.yTab, 40*39*sizeof(short));
#else
    memcpy( ((NVRAM_CAMERA_STROBE_STRUCT*)def)->engTab.yTab, ((NVRAM_CAMERA_STROBE_STRUCT*)nvram)->engTab.yTab, 40*40*sizeof(short));
#endif
  }

  else if(id==CAMERA_NVRAM_DATA_LENS)
  {
    int i;
    for (i = 0; i < AF_CAM_SCENARIO_NUM; i++)
    {
        memcpy( &((NVRAM_LENS_PARA_STRUCT*)def)->rLENSNVRAM[i].rPDNVRAM.rCaliData, &((NVRAM_LENS_PARA_STRUCT*)nvram)->rLENSNVRAM[i].rPDNVRAM.rCaliData, sizeof(PD_CALIBRATION_DATA_T));
    }
  }
}

  





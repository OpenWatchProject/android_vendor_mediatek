#include "camera_custom_flicker.h"
#include "kd_imgsensor.h"
#include "kd_imgsensor_define.h"
#include "camera_custom_flicker_para.h"

void cust_getFlickerHalPara(int* defaultHz, int* maxDetExpUs)
{
	*defaultHz = 50;
	*maxDetExpUs = 70000;
}

int cust_getFlickerDetectFrequency()
{
    // 10 frames/time of flicker detection
    return 10;
}

int cust_getMaxAttachNum()
{
    return 1;
}

int cust_getSensorBinningRatio(int sensorID, int sensorMode)
{
    (void)sensorMode;
    switch (sensorID) {
/*
    case 0x8044 : 
        if (sensorMode == e_sensorModePreview) 
          return 2;
        else if (sensorMode == e_sensorModeVideoPreview)
          return 2;
        else
          return 1;
*/
    default:
        return 1;
    }
}

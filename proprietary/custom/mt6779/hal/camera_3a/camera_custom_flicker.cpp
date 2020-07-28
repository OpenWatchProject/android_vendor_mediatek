#include "camera_custom_flicker.h"

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


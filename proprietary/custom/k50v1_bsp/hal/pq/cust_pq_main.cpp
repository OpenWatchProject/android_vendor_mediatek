#define LOG_TAG "PQCust"

#define MTK_LOG_ENABLE 1
#include <cutils/log.h>
#include <stdint.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <unistd.h>

#include "ddp_drv.h"

// MUST HAVE parameters
//extern int BrightnessLevel;
//extern int BrighteningSpeedLevel;
//extern int ReadabilityLevel;
//extern int DarkeningSpeedLevel;
//extern int SmartBacklightLevel;

void checkVariableNames(void)
{
    // If any link error here, means the cust_pqdc.cpp is not configured properly.
    // May be file lost(not linked) or incorrect variable name
//    ALOGI("Levels = %d %d %d %d %d",
//        BrightnessLevel, BrighteningSpeedLevel, ReadabilityLevel, DarkeningSpeedLevel, SmartBacklightLevel);
}


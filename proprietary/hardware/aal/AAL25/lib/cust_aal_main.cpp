#define LOG_TAG "AALCust"

#define MTK_LOG_ENABLE 1
#include <log/log.h>
#include <stdint.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <unistd.h>
#include <ddp_drv.h>
#include <cmath>


extern "C" { // All symbols should be exported as C signature

extern int LCM_COUNT;

int getLcmIndex(void)
{
    static int lcmIdx = -1;

    if (lcmIdx == -1) {
        int drvID = open("/dev/mtk_disp_mgr", O_RDONLY, 0);
        if (drvID >= 0) {
            ioctl(drvID, DISP_IOCTL_GET_LCMINDEX, &lcmIdx);
            close(drvID);
            if (lcmIdx < 0 || LCM_COUNT <= lcmIdx)
            {
                ALOGE("Invalid LCM index %d, LCM count %d", lcmIdx, LCM_COUNT);
                lcmIdx = 0;
            }
        }
        else {
            ALOGE("Fail to open disp driver!");
            lcmIdx = 0;
        }
    }

    ALOGI("LCM index: %d/%d", lcmIdx, LCM_COUNT);
    return lcmIdx;
}


#ifdef MTK_ULTRA_DIMMING_SUPPORT

#define EXTEND_8_TO_12(V8) (((V8) << 4) | ((V8) >> 4))

// Brightness value of full on
// Must sync with PowerManager.BRIGHTNESS_ON
static const int BRIGHTNESS8_FULL_ON = 255;

// Boundary to convert virtual brightness to physical brightness.
// Must sync with PowerManager.java
static const int ULTRA_DIMMING_VIRTUAL8_CONTROL = 80;
static const int ULTRA_DIMMING_PHYSICAL8_CONTROL = 10;


static const int BRIGHTNESS12_FULL_ON = EXTEND_8_TO_12(BRIGHTNESS8_FULL_ON);

static const int ULTRA_DIMMING_PHYSICAL12_CONTROL =
        EXTEND_8_TO_12(ULTRA_DIMMING_PHYSICAL8_CONTROL);

static const int ULTRA_DIMMING_VIRTUAL12_CONTROL =
        EXTEND_8_TO_12(ULTRA_DIMMING_VIRTUAL8_CONTROL);

static const float DimmingGammaHigh =
        log((float)ULTRA_DIMMING_PHYSICAL12_CONTROL / (float)BRIGHTNESS12_FULL_ON) /
        log((float)ULTRA_DIMMING_VIRTUAL8_CONTROL / (float)BRIGHTNESS8_FULL_ON);


// We cache the result here to reduce duplicated calculations
static unsigned short Virtual8ToPhysical12ResultCache[BRIGHTNESS8_FULL_ON + 1] = { 0 };


int dimmingVirtual10ToPhysical12(void *, void *, int virtualValue10)
{
    if (virtualValue10 <= 0)
        return 0;

    int virtualValue8 = virtualValue10 >> 2;

    if (virtualValue8 >= BRIGHTNESS8_FULL_ON)
        return BRIGHTNESS12_FULL_ON;

    if (Virtual8ToPhysical12ResultCache[virtualValue8] == 0) {
        int physicalValue12 = 0;
        if (virtualValue8 <= ULTRA_DIMMING_VIRTUAL8_CONTROL) {
            physicalValue12 = (int)( ((float)virtualValue8 / (float)ULTRA_DIMMING_VIRTUAL8_CONTROL) *
                    (float)ULTRA_DIMMING_PHYSICAL12_CONTROL + 0.5f );
        } else {
            physicalValue12 = (int)( pow((float)virtualValue8 / (float)BRIGHTNESS8_FULL_ON, DimmingGammaHigh) *
                    (float)BRIGHTNESS12_FULL_ON + 0.5f );
        }

        Virtual8ToPhysical12ResultCache[virtualValue8] = (unsigned short)physicalValue12;
    }

    return Virtual8ToPhysical12ResultCache[virtualValue8];
}

#endif



// MUST HAVE parameters
extern int ReadabilityLevel;
extern int SmartBacklightStrength;
extern int SmartBacklightRange;


void checkVariableNames(void)
{
    // If any link error here, means the cust_aal.cpp is not configured properly.
    // May be file lost(not linked) or incorrect variable name
    ALOGI("Levels = %d %d %d",
        ReadabilityLevel, SmartBacklightStrength, SmartBacklightRange);
}

} // end of extern "C"


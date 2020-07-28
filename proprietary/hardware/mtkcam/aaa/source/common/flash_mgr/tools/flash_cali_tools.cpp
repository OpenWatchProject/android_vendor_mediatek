#define LOG_TAG "FlashCaliTools"

#include <stdio.h>
#include "flash_cali_tools.h"
#include "flash_cali_utils.h"
#include "flash_cali_xml.h"
#include "flash_custom_utils.h"

int main()
{
    NVRAM_CAMERA_FLASH_CALIBRATION_STRUCT nvFlash;

    if (readFlashCali("flash_cali.bin", &nvFlash))
        logE("Error: failed to read.");
    //if (readFlashCali("flash_cali_quick2.bin", &nvFlash))
    //	logE("Error: failed to read.");


    if (verifyYTab(nvFlash.yTab, 26, 26, 1))
        logE("Error: invalid energy table.");

    //showFlashCali(&nvFlash);
    showFlashCali(&nvFlash, 26, 26);

#if 0
    writeCaliAeXml("caliAe.xml", DUAL_CAMERA_MAIN_SENSOR, 1, 10000, 1024, 1024);
    writeEngTabXml("engTab.xml", &nvFlash, DUAL_CAMERA_MAIN_SENSOR, 1, 26, 26);
    writeFwbGainXml("fwbGain.xml", &nvFlash, DUAL_CAMERA_MAIN_SENSOR, 1, 26, 26);

    dqCaliData dq1, dq2, dq3;
    for (int i = 0; i < 10; i++) {
        CaliData tmp;
        tmp.duty = i;
        tmp.dutyLT = i;
        tmp.exp = 10000;
        tmp.afe = 1024;
        tmp.isp = 1024;
        for (int j = 0; j < CCT_FLASH_CALCULATE_FRAME; j++) {
            tmp.y[j] = 512;
            tmp.r[j] = 513;
            tmp.g[j] = 514;
            tmp.b[j] = 515;
            tmp.t[j] = i * 10 + j;
        }
        dq1.push_back(tmp);
    }
    dq2 = dq1;
    dq3 = dq1;
    writeEngAllXml("engAll.xml", dq1, dq2, dq3, DUAL_CAMERA_MAIN_SENSOR, 1);
#endif
    return 0;
}


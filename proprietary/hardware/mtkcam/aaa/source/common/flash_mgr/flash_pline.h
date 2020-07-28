#pragma once

#include "camera_custom_awb_nvram.h"
#include "flash_param.h" // needed by FlashAlg.h
#include "flash_awb_param.h" // needed by FlashAlg.h
#include "FlashAlg.h"

#if (CAM3_3A_ISP_30_EN || CAM3_3A_ISP_40_EN)
using namespace NS3A;
#endif

class FlashPline
{
    public:
        FlashPline();
        ~FlashPline();
        int convertAePlineToPline(PLine *p, strAETable *pAePline, int maxExp, int flickerMode);
    private:
        evSetting *pEvSetting;
};

class FlashPlineTool
{
    public:
        static int searchAePlineIndex(int *ind, strAETable *pAePline, int exp, int afe, int isp);
        static int dumpAePline(const char *fname, strAETable *pAePline);
        static int dumpPline(const char *fname, PLine *pPline);
};


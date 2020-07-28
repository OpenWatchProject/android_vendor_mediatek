#define LOG_TAG "FlashPlineTool"

#include <stdlib.h>
#include <cmath>

/* custom common headers */
#include "mtkcam/def/BuiltinTypes.h" // needed by AEPlinetable.h
#include "custom/aaa/AEPlinetable.h"

/* camera common headers */
#include "flash_pline.h"
#include "flash_utils.h"
#include "log_utils.h"

/* camera headers */
#include "flicker_hal_if.h"
#include "flash_platform_adapter.h"

#define FLASH_PATH_AE_PLINE "/data/vendor/flash/ae_pline.txt"
#define FLASH_PATH_FLASH_ALGO_PLINE "/data/vendor/flash/flash_algo_pline.txt"

#define PLINE_AFE_GAIN_BASE 1024.0
#define PLINE_ISP_GAIN_BASE 1024.0

/***********************************************************
 * FlashPline
 **********************************************************/
FlashPline::FlashPline()
{
    pEvSetting = NULL;
}

FlashPline::~FlashPline()
{
    if (pEvSetting) {
        delete [] pEvSetting;
        pEvSetting = NULL;
    }
}

int FlashPline::convertAePlineToPline(PLine *p, strAETable *pAePline, int maxExp, int flickerMode)
{
    logI("convertAePlineToPline().");

    int i;
    int len = pAePline->u4TotalIndex;
    int ispGainBase = FlashPlatformAdapter::getIspGainBase();
    int afeGainBase = FlashPlatformAdapter::getAfeGainBase();

    if (len <= 0){
        logI("len is not valid");
        return -1;
    }

    /* allocate flash algo pline table */
    if (pEvSetting)
        delete [] pEvSetting;
    pEvSetting = new evSetting[len];

    /* choose AE pline table with flicker mode */
    if (flickerMode == HAL_FLICKER_AUTO_60HZ)
        pAePline->pCurrentTable = &pAePline->sTable60Hz;
    else
        pAePline->pCurrentTable = &pAePline->sTable50Hz;

    /* check if over the max exposure time */
    double evModify = 0;
    double maxGainLevel = 0;
    int maxAfe = 0;
    int maxIsp = 0;
    if (maxExp) {
        if ((int)pAePline->pCurrentTable->sPlineTable[len - 1].u4Eposuretime > maxExp) {
            evModify = 10 * std::log2(pAePline->pCurrentTable->sPlineTable[len - 1].u4Eposuretime / (double)maxExp);
            maxAfe = pAePline->pCurrentTable->sPlineTable[len - 1].u4AfeGain;
            maxIsp = pAePline->pCurrentTable->sPlineTable[len - 1].u4IspGain*ispGainBase/PLINE_ISP_GAIN_BASE;
            maxGainLevel = (double)maxAfe * maxIsp;
        }
    }

    /* flash algo pline */
    p->i4MaxBV = pAePline->i4MaxBV;
    p->i4MinBV = pAePline->i4MinBV + evModify;
    p->u4TotalIndex = pAePline->u4TotalIndex;
    p->i4StrobeTrigerBV = pAePline->i4StrobeTrigerBV;
    p->pCurrentTable = pEvSetting;
    for (i = 0; i < len; i++) {
        int exp = pAePline->pCurrentTable->sPlineTable[i].u4Eposuretime;
        int afe = pAePline->pCurrentTable->sPlineTable[i].u4AfeGain;
        int isp = pAePline->pCurrentTable->sPlineTable[i].u4IspGain*ispGainBase/PLINE_ISP_GAIN_BASE;
        if (!maxExp) {
            p->pCurrentTable[i].u4Eposuretime = exp;
            p->pCurrentTable[i].u4AfeGain = afe;
            p->pCurrentTable[i].u4IspGain = isp;
        } else if (exp <= maxExp) {
            p->pCurrentTable[i].u4Eposuretime = exp;
            p->pCurrentTable[i].u4AfeGain = afe;
            p->pCurrentTable[i].u4IspGain = isp;
        } else if (exp > maxExp) {
            double gainLevel = (double)exp * afe * isp / maxExp;
            p->pCurrentTable[i].u4Eposuretime = maxExp;
            if (gainLevel > maxGainLevel) {
                p->pCurrentTable[i].u4AfeGain = maxAfe;
                p->pCurrentTable[i].u4IspGain = maxIsp;
            } else if ((gainLevel / afeGainBase) > maxAfe && maxAfe > 0) {
                p->pCurrentTable[i].u4AfeGain = maxAfe;
                p->pCurrentTable[i].u4IspGain = (double)exp * afe * isp / maxExp / maxAfe;
            } else {
                p->pCurrentTable[i].u4AfeGain = gainLevel / afeGainBase;
                p->pCurrentTable[i].u4IspGain = ispGainBase;
            }
        }
        p->pCurrentTable[i].uIris = pAePline->pCurrentTable->sPlineTable[i].uIris;
        p->pCurrentTable[i].uSensorMode = pAePline->pCurrentTable->sPlineTable[i].uSensorMode;
        p->pCurrentTable[i].uFlag = pAePline->pCurrentTable->sPlineTable[i].uFlag;
    }

    return 0;
}


/***********************************************************
 * FlashPlineTool
 **********************************************************/
int FlashPlineTool::searchAePlineIndex(
        int *index, strAETable *pAePline, int exp, int afe, int isp)
{
    /* verify arguments */
    if (!index || !pAePline)
        return -1;

    /* search index */
    int ispGainBase = FlashPlatformAdapter::getIspGainBase();
    int afeGainBase = FlashPlatformAdapter::getAfeGainBase();
    int i;
    double expLevelTarget = (double)exp * afe * isp / (afeGainBase * ispGainBase);
    double expLevel;
    double diff;
    double minDiff = expLevelTarget;
    for (i = 0; i < (int)pAePline->u4TotalIndex; i++) {
        expLevel = (double)pAePline->pCurrentTable->sPlineTable[i].u4Eposuretime *
            pAePline->pCurrentTable->sPlineTable[i].u4AfeGain *
            pAePline->pCurrentTable->sPlineTable[i].u4IspGain / (PLINE_AFE_GAIN_BASE * PLINE_ISP_GAIN_BASE);
        diff = std::abs(expLevel - expLevelTarget);
        if (minDiff > diff) {
            minDiff = diff;
            *index = i;
        }
    }
    logI("searchAePlineIndex(): index(%d).", *index);

    return 0;
}

int FlashPlineTool::dumpAePline(const char *fname, strAETable *pAePline)
{
    int i;
    FILE *fp;

    /* verify arguments */
    if (!fname || !pAePline)
        return -1;

    fp = fopen(fname, "wt");
    if (!fp)
        return -1;

    for (i = 0;i < (int)pAePline->u4TotalIndex; i++) {
        fprintf(fp, "%d\t%d\t%d\n",
                pAePline->pCurrentTable->sPlineTable[i].u4Eposuretime,
                pAePline->pCurrentTable->sPlineTable[i].u4AfeGain,
                pAePline->pCurrentTable->sPlineTable[i].u4IspGain);
    }
    fclose(fp);

    return 0;
}

int FlashPlineTool::dumpPline(const char *fname, PLine *pPline)
{
    int i;
    FILE *fp;

    /* verify arguments */
    if (!fname || !pPline)
        return -1;

    fp = fopen(fname, "wt");
    if (!fp)
        return -1;

    for (i = 0; i < (int)pPline->u4TotalIndex; i++) {
        fprintf(fp, "%d\t%d\t%d\n",
                pPline->pCurrentTable[i].u4Eposuretime,
                pPline->pCurrentTable[i].u4AfeGain,
                pPline->pCurrentTable[i].u4IspGain);
    }
    fclose(fp);

    return 0;
}


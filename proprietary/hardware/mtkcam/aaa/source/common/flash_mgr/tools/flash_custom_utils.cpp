#define LOG_TAG "FlashCustomUtils"

#include <stdio.h>
#include <string.h>

#ifdef LINUX
#include "flash_cali_tools.h"
#else
#include "camera_custom_nvram.h"
#include "flash_utils.h"
#endif


template <class T>
int diffYTab(const T *yTab, const T *yTabTarget, int dutyNum, int dutyNumLt)
{
    int i;
    for (i = 0; i < dutyNum * dutyNumLt; i++)
        if (yTab[i] != yTabTarget[i])
            return 1;
    return 0;
}
template int diffYTab<short>(const short *yTab, const short *yTabTarget, int dutyNum, int dutyNumLt);
template int diffYTab<float>(const float *yTab, const float *yTabTarget, int dutyNum, int dutyNumLt);

template <class T>
void dumpYTab(const T *yTab, int dutyNum, int dutyNumLt)
{
    char engStr[640];
    char engItem[16];

    if (!yTab)
        return;

    logI("dumpYTab():");

    for (int j = 0; j < dutyNumLt; j++) {
        memset(engStr, '\0', sizeof(engStr));
        for (int i = 0; i < dutyNum; i++) {
            snprintf(engItem, sizeof(engItem), "%d ", (short)yTab[i + j * dutyNumLt]);
            strncat(engStr, engItem, sizeof(engStr) - strlen(engStr) - 1);
        }
        logI("\t%s", engStr);
    }
}
template void dumpYTab<short>(const short *yTab, int dutyNum, int dutyNumLt);
template void dumpYTab<float>(const float *yTab, int dutyNum, int dutyNumLt);

void dumpEngTab(NVRAM_FLASH_CCT_ENG_TABLE *engTab, int dutyNum, int dutyNumLt)
{
    /* verify arguments */
    if (!engTab)
        return;

    if (dutyNum < 0 || dutyNum > FLASH_CUSTOM_MAX_DUTY_NUM_HT ||
            dutyNumLt < 0 || dutyNumLt > FLASH_CUSTOM_MAX_DUTY_NUM_LT)
        return;

    logI("dumpEngTab(): exp(%d) afe(%d) isp(%d) distiance(%d).",
            engTab->exp, engTab->afe_gain, engTab->isp_gain, engTab->distance);
    dumpYTab(engTab->yTab, dutyNum, dutyNumLt);
}

template <class T>
int verifyYTab(const T *yTab, int dutyNum, int dutyNumLt, int isDual)
{
    int i, j;
    int isInvalid = 0;
    int index;
    int indexCompare;
    /* verify arguments */
    if (dutyNum < 0 || dutyNum >= FLASH_CUSTOM_MAX_DUTY_NUM_HT ||
            dutyNumLt < 0 || dutyNumLt >= FLASH_CUSTOM_MAX_DUTY_NUM_LT) {
        logE("verifyYTab(): invalid dutyNum(%d) or dutyNumLt(%d).",
                dutyNum, dutyNumLt);
        return -1;
    }
    /* y table from single led doesn't contain duty(-1) */
    if (!isDual) {
        dutyNum--;
        dutyNumLt = 0;
    }

    /* should be strictly increment in horizontal direction */
    for (j = -1; j < dutyNumLt; j++) {
        if (isInvalid)
            break;
        for (i = 0; i < dutyNum; i++) {
            index = (j + 1) * (dutyNum + 1) + (i + 1);
            indexCompare = index - 1;
            if (yTab[index] == -1 || yTab[indexCompare] == -1)
                continue;

            if (yTab[index] < yTab[indexCompare]) {
                isInvalid = 1;
                logE("verifyYTab(): invalid with %d(%d) < %d(%d).",
                        yTab[index], index,
                        yTab[indexCompare], indexCompare);
                break;
            }
        }
    }

    /* should be strictly increment in vertical direction */
    for (i = -1; i < dutyNum; i++) {
        if (isInvalid)
            break;
        for (j = 0; j < dutyNumLt; j++) {
            index = (j + 1) * (dutyNum + 1) + (i + 1);
            indexCompare = index - (dutyNum + 1);
            if (yTab[index] == -1 || yTab[indexCompare] == -1)
                continue;

            if (yTab[index] < yTab[indexCompare]) {
                isInvalid = 1;
                logE("verifyYTab(): invalid with %d(%d) < %d(%d).",
                        yTab[index], index,
                        yTab[indexCompare], indexCompare);
                break;
            }
        }
    }

    return isInvalid;
}
template int verifyYTab<short>(const short *yTab, int dutyNum, int dutyNumLt, int isDual);

void dumpFwbGain(AWB_GAIN_T *flashWBGain, int dutyNum, int dutyNumLt)
{
    if (!flashWBGain)
        return;

    logI("dumpFwbGain():");

    for (int j = 0; j < dutyNumLt; j++)
        for (int i = 0; i < dutyNum; i++)
            logI("\t(%d,%d): %d %d %d", i, j,
                    flashWBGain[i + j * dutyNumLt].i4R,
                    flashWBGain[i + j * dutyNumLt].i4G,
                    flashWBGain[i + j * dutyNumLt].i4B);
}


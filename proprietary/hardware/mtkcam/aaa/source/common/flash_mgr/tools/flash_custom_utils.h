#pragma once

#ifdef LINUX
#include "flash_cali_tools.h"
#else
#include "camera_custom_nvram.h"
#endif

template <class T>
int diffYTab(const T *yTab, const T *yTabTarget, int dutyNum, int dutyNumLt);

template <class T>
void dumpYTab(const T *yTab, int dutyNum, int dutyNumLt);

void dumpEngTab(NVRAM_FLASH_CCT_ENG_TABLE *engTab, int dutyNum, int dutyNumLt);

template <class T>
int verifyYTab(const T *yTab, int dutyNum, int dutyNumLt, int isDual);

void dumpFwbGain(AWB_GAIN_T *flashWBGain, int dutyNum, int dutyNumLt);


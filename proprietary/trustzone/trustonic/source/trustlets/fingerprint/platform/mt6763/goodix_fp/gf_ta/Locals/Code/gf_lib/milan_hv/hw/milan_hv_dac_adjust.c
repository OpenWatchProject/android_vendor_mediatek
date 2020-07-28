/*
 * Copyright (C) 2013-2017, Shenzhen Huiding Technology Co., Ltd.
 * All Rights Reserved.
 */
#include "milan_hv_dac_adjust.h"

#define MILAN_E_WHITE_DIFF_THR 3800
#define ShakeNum 3

unsigned short DAC_Cali_Flag = 0;
unsigned short DAC_Cali_Cnt = 0;
unsigned short Shake_High_Cnt = 0;
unsigned short Shake_Low_Cnt = 0;

static void CalulateWhiteThr(const unsigned short *pksRaw, const unsigned short *pksBase,
        unsigned short nRows, unsigned short nCols, signed int* pnWhiteThr, signed int* pnCount);
void HVDacDynamicAdjust(const unsigned short *pksRaw, const unsigned short *pksBase,
        unsigned short nRows, unsigned short nCols, unsigned short* DacDataHDynamic,
        unsigned short TcodeData, unsigned short DacDataHOtp) {
    int n, p, q, m;
    int nPixelNum = 0;
    unsigned long Rawdata_Aver = 0;
    float percent_Effective = 0.0;
    float percent_NoneZero = 0.0;

    unsigned short DAC_Cali = 0;

    float K_Dac_Tcode = 0.0;
    signed int nWhiteThr = 0;
    signed int nCount = 0;

    nPixelNum = nRows * nCols;
    K_Dac_Tcode = TcodeData * 0.283;
    DAC_Cali = *DacDataHDynamic;

    CalulateWhiteThr(pksRaw, pksBase, nRows, nCols, &nWhiteThr, &nCount);

    for (n = 0, p = 0, q = 0, m = 0; n < nPixelNum; n++) {
        if ((n / nCols != 0) && (n / nCols != (nRows - 1)) && (n % nCols != 0)
                && (n % nCols != (nCols - 1))) {
            if (pksRaw[n] != 0) {
                p++;
            }
            if (((pksBase[n] - pksRaw[n] < nWhiteThr) || (0 == nWhiteThr)
                    || ((nCount * 10 > nPixelNum) && (pksRaw[n] - pksBase[n] > 0)))
                    && pksRaw[n] > 3800) {
                m++;
            } else {
                Rawdata_Aver = Rawdata_Aver + pksRaw[n];
                q++;
            }
        }
    }
    percent_NoneZero = (float) p / (float) ((nRows - 2) * (nCols - 2));
    percent_Effective = (float) q / (float) ((nRows - 2) * (nCols - 2));
    if (q == 0) {
        Rawdata_Aver = 0;
    } else {
        Rawdata_Aver = Rawdata_Aver / q;
    }

    if (((percent_NoneZero > 0.95) && (percent_Effective > 0.4)) || (percent_NoneZero <= 0.95)) {
        unsigned short temp_middle = 0;
        unsigned short temp_cali = 0;
        if (Rawdata_Aver > 3200) {
            if (Shake_Low_Cnt < ShakeNum) {
                Shake_Low_Cnt = 0;
            } else {
                DAC_Cali_Flag = 0;
            }

            if (Shake_High_Cnt < ShakeNum) {
                Shake_High_Cnt++;
            }

            if (Shake_High_Cnt >= ShakeNum) {
                Shake_Low_Cnt = 0;
                if (DAC_Cali_Flag == 0) {
                    DAC_Cali_Flag = 0x01;
                    DAC_Cali_Cnt = 0;
                }

                temp_middle = 0.25 * (Rawdata_Aver - 2000) / K_Dac_Tcode;
                if (temp_middle == 0)
                    temp_middle = 0x1;
                if ((DAC_Cali & 0x1FF0) + (16 * temp_middle) > 16 * (DacDataHOtp + 0x0D + 0x11)) {
                    temp_cali = (DAC_Cali & 0xE00F) + 16 * (DacDataHOtp + 0x0D + 0x11);
                } else {
                    temp_cali = DAC_Cali + (16 * temp_middle);
                }
                DAC_Cali = (DAC_Cali & 0xE00F) + (temp_cali & 0x1FF0);
                DAC_Cali_Cnt++;
            }

        } else if (Rawdata_Aver < 1500) {
            if (Shake_High_Cnt < ShakeNum) {
                Shake_High_Cnt = 0;
            } else {
                DAC_Cali_Flag = 0;
            }

            if (Shake_Low_Cnt < ShakeNum) {
                Shake_Low_Cnt++;
            }

            if (Shake_Low_Cnt >= ShakeNum) {
                Shake_High_Cnt = 0;
                if (DAC_Cali_Flag == 0) {
                    DAC_Cali_Flag = 0x01;
                    DAC_Cali_Cnt = 0;
                }

                temp_middle = 0.25 * (2000 - Rawdata_Aver) / K_Dac_Tcode;
                if (temp_middle == 0)
                    temp_middle = 0x1;
                if ((DAC_Cali & 0x1FF0) < 16 * (DacDataHOtp + 0x0D - 0x11) + (16 * temp_middle)) {
                    temp_cali = (DAC_Cali & 0xE00F) + 16 * (DacDataHOtp + 0x0D - 0x11);
                } else {
                    temp_cali = DAC_Cali - (16 * temp_middle);
                }
                DAC_Cali = (DAC_Cali & 0xE00F) + (temp_cali & 0x1FF0);
                DAC_Cali_Cnt++;
            }

        } else {
            if (DAC_Cali_Flag == 0x01) {
                if ((Rawdata_Aver > 2100) || (Rawdata_Aver < 1900)) {
                    DAC_Cali_Cnt++;
                    if (Rawdata_Aver > 2000) {
                        temp_middle = 0.25 * (Rawdata_Aver - 2000) / K_Dac_Tcode;
                        if (temp_middle == 0)
                            temp_middle = 0x1;
                        if (((DAC_Cali & 0x1FF0) + (16 * temp_middle))
                                > 16 * (DacDataHOtp + 0x0D + 0x11)) {
                            temp_cali = (DAC_Cali & 0xE00F) + 16 * (DacDataHOtp + 0x0D + 0x11);
                        } else {
                            temp_cali = DAC_Cali + (16 * temp_middle);
                        }
                    } else {
                        temp_middle = 0.25 * (2000 - Rawdata_Aver) / K_Dac_Tcode;
                        if (temp_middle == 0)
                            temp_middle = 0x1;
                        if ((DAC_Cali & 0x1FF0)
                                < 16 * (DacDataHOtp + 0x0D - 0x12) + (16 * temp_middle)) {
                            temp_cali = (DAC_Cali & 0xE00F) + 16 * (DacDataHOtp + 0x0D - 0x11);
                        } else {
                            temp_cali = DAC_Cali - (16 * temp_middle);
                        }
                    }
                    DAC_Cali = (DAC_Cali & 0xE00F) + (temp_cali & 0x1FF0);
                } else {
                    DAC_Cali_Flag = 0x00;
                    Shake_High_Cnt = 0;
                    Shake_Low_Cnt = 0;
                }
            } else {
                if (Shake_High_Cnt < ShakeNum) {
                    Shake_High_Cnt = 0;
                }
                if (Shake_Low_Cnt < ShakeNum) {
                    Shake_Low_Cnt = 0;
                }
            }
        }
    }

    *DacDataHDynamic = DAC_Cali;
}

void CalulateWhiteThr(const unsigned short *pksRaw, const unsigned short *pksBase,
        unsigned short nRows, unsigned short nCols, signed int* pnWhiteThr, signed int* pnCount) {
    const signed int nkPixelNum = nRows * nCols;
    signed int nCountNobase = 0;
    signed int nAvgNobase = 0;

    signed int nCountNobase1 = 0;
    signed int nOfAve = 0;

    signed int nThrBase = 0;

    nThrBase = MILAN_E_WHITE_DIFF_THR;

    for (signed int n = 0; n < nkPixelNum; ++n) {
        if (pksBase[n] - pksRaw[n] > nThrBase/*WHITE_DIFF_THRESHOLD*/) {
            nCountNobase++;
            nAvgNobase += pksBase[n] - pksRaw[n];
        } else {
            nCountNobase1++;
            nOfAve += (pksBase[n] - pksRaw[n]);
        }
        if ((pksRaw[n] - pksBase[n]) > 0) {
            (*pnCount)++;
        }
    }
    if (nCountNobase > 0) {
        nAvgNobase /= nCountNobase;
        if (nCountNobase1 > 0) {
            nOfAve /= nCountNobase1;
        }
        (*pnWhiteThr) = nThrBase - 1 * (nAvgNobase - nOfAve) / 4;
    }
}


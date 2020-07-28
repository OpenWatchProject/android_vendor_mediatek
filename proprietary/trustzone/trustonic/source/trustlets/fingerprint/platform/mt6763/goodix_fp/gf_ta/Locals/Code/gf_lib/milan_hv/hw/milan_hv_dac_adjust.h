/*
 * Copyright (C) 2013-2016, Shenzhen Huiding Technology Co., Ltd.
 * All Rights Reserved.
 */
#ifndef __MILAN_HV_DAC_ADJUST_H__
#define __MILAN_HV_DAC_ADJUST_H__

#ifdef __cplusplus
extern "C" {
#endif

extern void HVDacDynamicAdjust(const unsigned short *pksRaw, const unsigned short *pksBase,  // NOLINT
        unsigned short nRows, unsigned short nCols, unsigned short *DacDataHDynamic,  // NOLINT
        unsigned short TcodeData, unsigned short DacDataHOtp);  // NOLINT

#ifdef __cplusplus
}
#endif

#endif  // __MILAN_HV_DAC_ADJUST_H__


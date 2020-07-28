/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 */
/* MediaTek Inc. (C) 2010. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER ON
 * AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
 * NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
 * SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
 * SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES TO LOOK ONLY TO SUCH
 * THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. RECEIVER EXPRESSLY ACKNOWLEDGES
 * THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES
 * CONTAINED IN MEDIATEK SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK
 * SOFTWARE RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND
 * CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,
 * AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE,
 * OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY RECEIVER TO
 * MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek Software")
 * have been modified by MediaTek Inc. All revisions are subject to any receiver's
 * applicable license agreements with MediaTek Inc.
 */

/********************************************************************************************
 *     LEGAL DISCLAIMER
 *
 *     (Header of MediaTek Software/Firmware Release or Documentation)
 *
 *     BY OPENING OR USING THIS FILE, BUYER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 *     THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE") RECEIVED
 *     FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO BUYER ON AN "AS-IS" BASIS
 *     ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES, EXPRESS OR IMPLIED,
 *     INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR
 *     A PARTICULAR PURPOSE OR NONINFRINGEMENT. NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY
 *     WHATSOEVER WITH RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY,
 *     INCORPORATED IN, OR SUPPLIED WITH THE MEDIATEK SOFTWARE, AND BUYER AGREES TO LOOK
 *     ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. MEDIATEK SHALL ALSO
 *     NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE RELEASES MADE TO BUYER'S SPECIFICATION
 *     OR TO CONFORM TO A PARTICULAR STANDARD OR OPEN FORUM.
 *
 *     BUYER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND CUMULATIVE LIABILITY WITH
 *     RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE, AT MEDIATEK'S OPTION,
TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE
 *     FEES OR SERVICE CHARGE PAID BY BUYER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 *     THE TRANSACTION CONTEMPLATED HEREUNDER SHALL BE CONSTRUED IN ACCORDANCE WITH THE LAWS
 *     OF THE STATE OF CALIFORNIA, USA, EXCLUDING ITS CONFLICT OF LAWS PRINCIPLES.
 ************************************************************************************************/

#include "camera_custom_mvhdr3expo.h"
#include <math.h>
#include <cstdio>
#include <cstdlib>
#include <cutils/properties.h>
#include <cutils/log.h>
#include <utils/Errors.h>
#include <string.h>

#define LOG_TAG "camera_custom_mvhdr3expo"

#define CLAMP(x,min,max) (((x) > (max)) ? (max) : (((x) < (min)) ? (min) : (x)))

#define MY_LOGD(fmt, arg...)    ALOGD(fmt, ##arg)
#define MY_LOGE(fmt, arg...)    ALOGE(fmt, ##arg)

#define MY_LOG_IF(cond, ...)    do { if ( (cond) ) { MY_LOGD(__VA_ARGS__); } }while(0)

MUINT32 getMVHDR3ExpoBufSize()
{
    return (MVHDR3EXPO_WIDTH * MVHDR3EXPO_HEIGHT * 2 + 256 * 2); // AEY_LE + AEY_SE + AEHist_LE + AEHist_SE
}

MVOID decodeMVHDR3ExpoStatistic(MINT32 i4SensorDev, MVOID *pAEYDataPointer, MVOID *pAEHistDataPointer, MVOID *pEmbDataPointer, MVOID *pOutputDataPointer, MUINT32 &u4MVHDRRatio_x100)
{
    char value[PROPERTY_VALUE_MAX] = {'\0'};
    property_get("debug.mvhdr3ExpoL.custom.enable", value, "0");
    MBOOL bDebugLEnable = (MBOOL)atoi(value);
    property_get("debug.mvhdr3ExpoH.custom.enable", value, "0");
    MBOOL bDebugHEnable = (MBOOL)atoi(value);
    memset(pOutputDataPointer, 0, getMVHDR3ExpoBufSize() * sizeof(MUINT32));
    MUINT32 *pOutput     = reinterpret_cast<MUINT32 *>(pOutputDataPointer);
    MUINT8  *pAEYStat    = reinterpret_cast<MUINT8 *>(pAEYDataPointer);
    MUINT8  *pAEHistStat = reinterpret_cast<MUINT8 *>(pAEHistDataPointer);
    MUINT8  *pAEEBDData = reinterpret_cast<MUINT8 *>(pEmbDataPointer);
    MUINT32 u4AEHistTotalSum = 6209280; // Default for IMX576

    /* Embeded Data => HDR Ratio */
    if (pAEEBDData != NULL)
    {
        const MUINT32 u4AEEBDDataOffset  = 5; // 5 Bytes
        MUINT16 u2UpXSize, u2LowXSize, u2UpYSize, u2LowYSize;
        MUINT16 u2UpLELine, u2LowLELine, u2UpSELine, u2LowSELine, u2UpMELine, u2LowMELine, u2UpLEGain, u2LowLEGain, u2UpSEGain, u2LowSEGain, u2UpMEGain, u2LowMEGain;
        MUINT32 u4LELine = 0, u4SELine = 0, u4MELine = 0, u4CurRatio, u4XSize = 0, u4YSize = 0;
        MFLOAT fLEGain = 0, fSEGain = 0, fMEGain = 0;
        u2UpXSize  = pAEEBDData[39*u4AEEBDDataOffset+2];
        u2LowXSize = pAEEBDData[40*u4AEEBDDataOffset+0];
        u2UpYSize  = pAEEBDData[40*u4AEEBDDataOffset+2];
        u2LowYSize = pAEEBDData[41*u4AEEBDDataOffset+0];
        u4XSize += (u2UpXSize  << 8);
        u4XSize += (u2LowXSize << 0);
        u4YSize += (u2UpYSize  << 8);
        u4YSize += (u2LowYSize << 0);
        u4AEHistTotalSum = u4XSize * u4YSize;

        u2UpLELine  = pAEEBDData[11*u4AEEBDDataOffset+2];
        u2LowLELine = pAEEBDData[12*u4AEEBDDataOffset+0];
        u2UpLEGain  = pAEEBDData[12*u4AEEBDDataOffset+2];
        u2LowLEGain = pAEEBDData[13*u4AEEBDDataOffset+0];
        u2UpSELine  = pAEEBDData[22*u4AEEBDDataOffset+2];
        u2LowSELine = pAEEBDData[23*u4AEEBDDataOffset+0];
        u2UpSEGain  = pAEEBDData[18*u4AEEBDDataOffset+0];
        u2LowSEGain = pAEEBDData[18*u4AEEBDDataOffset+2];
        u2UpMELine  = pAEEBDData[(u4XSize/4+40)*u4AEEBDDataOffset+2];
        u2LowMELine = pAEEBDData[(u4XSize/4+41)*u4AEEBDDataOffset+0];
        u2UpMEGain  = pAEEBDData[(u4XSize/4+41)*u4AEEBDDataOffset+2];
        u2LowMEGain = pAEEBDData[(u4XSize/4+42)*u4AEEBDDataOffset+0];
        u4LELine += (u2UpLELine  << 8);
        u4LELine += (u2LowLELine << 0);
        fLEGain  += (u2UpLEGain  << 8);
        fLEGain  += (u2LowLEGain << 0);
        u4SELine += (u2UpSELine  << 8);
        u4SELine += (u2LowSELine << 0);
        fSEGain  += (u2UpSEGain  << 8);
        fSEGain  += (u2LowSEGain << 0);
        u4MELine += (u2UpMELine  << 8);
        u4MELine += (u2LowMELine << 0);
        fMEGain  += (u2UpMEGain  << 8);
        fMEGain  += (u2LowMEGain << 0);
        fLEGain = 1024 / (1024-fLEGain);
        fSEGain = 1024 / (1024-fSEGain);
        fMEGain = 1024 / (1024-fMEGain);
        u4MVHDRRatio_x100 = u4CurRatio = (u4LELine*fLEGain)*100 / (u4SELine*fSEGain);
        if (u4MVHDRRatio_x100 < 100) u4MVHDRRatio_x100 = 100;

        MY_LOG_IF(bDebugLEnable, "[%s] EBDData i4SensorDev[%d] XY[%d*%d=%d][0x%x 0x%x 0x%x 0x%x] HDRMode[0x%x] Ratio[%d] LE[%d %f][0x%x 0x%x 0x%x 0x%x] SE[%d %f][0x%x 0x%x 0x%x 0x%x] ME[%d %f][0x%x 0x%x 0x%x 0x%x]", __FUNCTION__,
	    i4SensorDev, u4XSize, u4YSize, u4AEHistTotalSum, pAEEBDData[39*u4AEEBDDataOffset+2], pAEEBDData[40*u4AEEBDDataOffset+0], pAEEBDData[40*u4AEEBDDataOffset+2], pAEEBDData[41*u4AEEBDDataOffset+0], pAEEBDData[20*u4AEEBDDataOffset+2], 
        u4CurRatio, u4LELine, fLEGain,
        pAEEBDData[11*u4AEEBDDataOffset+2], pAEEBDData[12*u4AEEBDDataOffset+0], pAEEBDData[12*u4AEEBDDataOffset+2], pAEEBDData[13*u4AEEBDDataOffset+0],
        u4SELine, fSEGain,
        pAEEBDData[22*u4AEEBDDataOffset+2], pAEEBDData[23*u4AEEBDDataOffset+0], pAEEBDData[18*u4AEEBDDataOffset+0], pAEEBDData[18*u4AEEBDDataOffset+2],
        u4MELine, fMEGain,
        pAEEBDData[(u4XSize/4+40)*u4AEEBDDataOffset+2], pAEEBDData[(u4XSize/4+41)*u4AEEBDDataOffset+0], pAEEBDData[(u4XSize/4+41)*u4AEEBDDataOffset+2], pAEEBDData[(u4XSize/4+42)*u4AEEBDDataOffset+0]);
    }
    else MY_LOG_IF(bDebugLEnable, "[%s] EBDData = NULL.", __FUNCTION__);

    /* YHIST => AEY_LE & AEY_SE */
    MUINT32 u4AEYMaxVal = (1024 * u4MVHDRRatio_x100 / 100)-1;
	const MUINT32 u4AEYBinSet = 2;                       // 1 Set = 2 Bins
    const MUINT32 u4AEYBinOffset  = 5;                   // 5 Bytes = 1Set = 2 Bins
    const MUINT32 u4AEYAreaOffset = 10 * u4AEYBinOffset; // 20 bins
    const MUINT32 u4AEYBinNumber = 18;
	MUINT16 u2UpDataPre, u2LowDataPre, u2UpData1, u2LowData1, u2UpData2, u2LowData2;
    MUINT16 u2BinValue[u4AEYBinNumber];

    // Set value for each Bin
    for (MINT32 i = u4AEYBinNumber-1; i >= 0; i--)
    {
        if (i > 0)
        {
            if (u4AEYMaxVal <= 1023)
            {
                u2BinValue[i] = u4AEYMaxVal;
                u4AEYMaxVal = u4AEYMaxVal / 2;
            }
            else
            {
                u2BinValue[i] = 1023;
                u4AEYMaxVal = u4AEYMaxVal / 2;
            }
        }
        else
        {
            u2BinValue[i] = 0;
        }
	}

    for (MUINT32 u4AEYAreaIdx = 0; u4AEYAreaIdx < (MVHDR3EXPO_WIDTH * MVHDR3EXPO_HEIGHT); u4AEYAreaIdx ++) { // calculate area[0] ~ area[47]
        MFLOAT fAEYFullAreaY = 0;
        for (MUINT32 u4AEYBinSetIdx = 0; u4AEYBinSetIdx < 9; u4AEYBinSetIdx ++) { // calculate bin[0] ~ bin[17]
            MUINT32 u4AEYBinSetCur = u4AEYBinSetIdx * u4AEYBinOffset;
            MFLOAT fPrevAreaY1 = 0, fAreaY1 = 0, fPrevAreaY2 = 0, fAreaY2 = 0;
            u2UpData1  = pAEYStat[u4AEYAreaIdx * u4AEYAreaOffset + u4AEYBinSetCur + 0];
            u2LowData1 = pAEYStat[u4AEYAreaIdx * u4AEYAreaOffset + u4AEYBinSetCur + 1];
            u2UpData2  = pAEYStat[u4AEYAreaIdx * u4AEYAreaOffset + u4AEYBinSetCur + 2];
            u2LowData2 = pAEYStat[u4AEYAreaIdx * u4AEYAreaOffset + u4AEYBinSetCur + 3];
            u2UpData1  = ((u2UpData1 << 1) & 0x01FE);
            u2LowData1 = ((u2LowData1 & 0x0080) >> 7);
            u2UpData2  = ((u2UpData2 << 1) & 0x01FE);
            u2LowData2 = ((u2LowData2 & 0x0080) >> 7);

            fPrevAreaY2 = fAreaY1 = (u2UpData1 + u2LowData1);
            fAreaY2 = (u2UpData2 + u2LowData2);
            fPrevAreaY2 = fAreaY1 = fAreaY1 / 256;
            fAreaY2 = fAreaY2 / 256;

            if(u4AEYBinSetIdx > 0)
            {
                u2UpDataPre  = pAEYStat[u4AEYAreaIdx * u4AEYAreaOffset + u4AEYBinSetCur - 3];
                u2LowDataPre = pAEYStat[u4AEYAreaIdx * u4AEYAreaOffset + u4AEYBinSetCur - 2];
                u2UpDataPre = ((u2UpDataPre << 1) & 0x01FE);
                u2LowDataPre = ((u2LowDataPre & 0x0080) >> 7);
                fPrevAreaY1 = (u2UpDataPre + u2LowDataPre);
                fPrevAreaY1 = fPrevAreaY1 / 256;
            }

            double tempVal1 = u2BinValue[2*u4AEYBinSetIdx];
            double tempVal2 = u2BinValue[2*u4AEYBinSetIdx+1];
            if (u4AEYBinSetIdx == 0) tempVal1 = 0;
            fAEYFullAreaY += (fAreaY1 - fPrevAreaY1) * (tempVal1); // non-linear
            fAEYFullAreaY += (fAreaY2 - fPrevAreaY2) * (tempVal2); // non-linear
            if (u4AEYBinSetIdx == 8 && fAreaY2 != 0) fAEYFullAreaY = fAEYFullAreaY / fAreaY2;

            MY_LOG_IF(bDebugHEnable, "[%s] AEY Area[%d] BinSet[%d][%d]=[0x%x 0x%x][0x%x 0x%x]", __FUNCTION__, u4AEYAreaIdx, u4AEYBinSetIdx,
			(u4AEYAreaIdx * u4AEYAreaOffset + u4AEYBinSetCur),
            pAEYStat[u4AEYAreaIdx * u4AEYAreaOffset + u4AEYBinSetCur + 0], pAEYStat[u4AEYAreaIdx * u4AEYAreaOffset + u4AEYBinSetCur + 1],
            pAEYStat[u4AEYAreaIdx * u4AEYAreaOffset + u4AEYBinSetCur + 2], pAEYStat[u4AEYAreaIdx * u4AEYAreaOffset + u4AEYBinSetCur + 3]);
            MY_LOG_IF(bDebugHEnable, "[%s] AEY Area[%d] BinSet[%d]=[%f %f] VAR[%f %f] Sum %f", __FUNCTION__, u4AEYAreaIdx, u4AEYBinSetIdx, fAreaY1, fAreaY2, tempVal1, tempVal2, fAEYFullAreaY);

        }
        MUINT32 u4AEYLEIdx = u4AEYAreaIdx;
        MUINT32 u4AEYSEIdx = u4AEYAreaIdx + (MVHDR3EXPO_WIDTH * MVHDR3EXPO_HEIGHT);
        pOutput[u4AEYLEIdx] = CLAMP((MUINT32)fAEYFullAreaY, 0, 1023) / 4;        // LE Y
        pOutput[u4AEYSEIdx] = ((MUINT32)fAEYFullAreaY * 1023 / u4AEYMaxVal) / 4; // SE Y
/*
        if ((u4AEYAreaIdx > 0) && (u4AEYAreaIdx % (MVHDR3EXPO_WIDTH - 1) == 0)) {
            MY_LOG_IF(bDebugHEnable, "[%s] AEY[%d] LE: %d,%d,%d,%d,%d,%d,%d,%d SE: %d,%d,%d,%d,%d,%d,%d,%d", __FUNCTION__, u4AEYAreaIdx / MVHDR3EXPO_WIDTH,
            pOutput[u4AEYLEIdx - 7], pOutput[u4AEYLEIdx - 6], pOutput[u4AEYLEIdx - 5], pOutput[u4AEYLEIdx - 4], pOutput[u4AEYLEIdx - 3], pOutput[u4AEYLEIdx - 2], pOutput[u4AEYLEIdx - 1], pOutput[u4AEYLEIdx - 0],
            pOutput[u4AEYSEIdx - 7], pOutput[u4AEYSEIdx - 6], pOutput[u4AEYSEIdx - 5], pOutput[u4AEYSEIdx - 4], pOutput[u4AEYSEIdx - 3], pOutput[u4AEYSEIdx - 2], pOutput[u4AEYSEIdx - 1], pOutput[u4AEYSEIdx - 0]);
        }
*/
    }

    /* AEHIST_Linear => AEHist_LE */
    const MUINT32 u4AEHistBinOffset = 5; // 5 Bytes
    const MUINT32 u4AEHistLogOffset = 128*u4AEHistBinOffset;
    // const MUINT32 u4AEHistTotalSum = 6209280;
    MUINT32 u4AEHistSum = 0;
    for (MUINT32 u4AEHistBinIdx = 0; u4AEHistBinIdx < 128; u4AEHistBinIdx ++) { // calculate linear hist[0] ~ hist[128]
        MUINT32 u4AEHistLEIdx = (MVHDR3EXPO_WIDTH * MVHDR3EXPO_HEIGHT) * 2 + u4AEHistBinIdx * 2 + 1;
        MUINT32 u4AEHistSEIdx = (MVHDR3EXPO_WIDTH * MVHDR3EXPO_HEIGHT) * 2 + u4AEHistBinIdx * 2 + 1 + 256;
        MUINT32 u4AEHistLE = 0;
        MUINT32 u4UpData, u4MidData, u4LowData;
        u4UpData  = pAEHistStat[u4AEHistBinIdx * u4AEHistBinOffset];
        u4MidData = pAEHistStat[u4AEHistBinIdx * u4AEHistBinOffset+1];
        u4LowData = pAEHistStat[u4AEHistBinIdx * u4AEHistBinOffset+2];
        u4AEHistLE += ((u4UpData  & 0x00FF) << 15);
        u4AEHistLE += ((u4MidData & 0x00FF) << 7 );
        u4AEHistLE += ((u4LowData & 0x00FE) >> 1 );
        u4AEHistSum += u4AEHistLE;
        pOutput[u4AEHistLEIdx] = u4AEHistLE; // LE Hist
        if (u4AEHistBinIdx == 127 && u4AEHistSum < u4AEHistTotalSum) pOutput[u4AEHistLEIdx] += (u4AEHistTotalSum - u4AEHistSum);

        MY_LOG_IF(bDebugHEnable, "[%s] RegAEHIST_linear Bin[%d]=[%d][0x%x 0x%x 0x%x 0x%x 0x%x]", __FUNCTION__, u4AEHistBinIdx, u4AEHistLE,
        pAEHistStat[u4AEHistBinIdx * u4AEHistBinOffset]  , pAEHistStat[u4AEHistBinIdx * u4AEHistBinOffset+1],
        pAEHistStat[u4AEHistBinIdx * u4AEHistBinOffset+2], pAEHistStat[u4AEHistBinIdx * u4AEHistBinOffset+3]  , pAEHistStat[u4AEHistBinIdx * u4AEHistBinOffset+4]);

        if ((u4AEHistBinIdx+1) % 16 == 0) {
            MY_LOG_IF(bDebugHEnable, "[%s] ValAEHIST_linear[%d] LE: %d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d", __FUNCTION__, u4AEHistBinIdx / 16,
            pOutput[u4AEHistLEIdx -30], pOutput[u4AEHistLEIdx -28], pOutput[u4AEHistLEIdx -26], pOutput[u4AEHistLEIdx -24], pOutput[u4AEHistLEIdx -22], pOutput[u4AEHistLEIdx -20], pOutput[u4AEHistLEIdx -18], pOutput[u4AEHistLEIdx -16],
            pOutput[u4AEHistLEIdx -14], pOutput[u4AEHistLEIdx -12], pOutput[u4AEHistLEIdx -10], pOutput[u4AEHistLEIdx - 8], pOutput[u4AEHistLEIdx - 6], pOutput[u4AEHistLEIdx - 4], pOutput[u4AEHistLEIdx - 2], pOutput[u4AEHistLEIdx - 0]);
        }
    }
    /* AEHIST_Log => AEHist_SE */
    const MUINT32 u4AEHISTLogBinNumber = 16;
    MUINT32 u4AEHISTMaxVal = 65535, u4AEHISTLogAve = 0, u4AEHISTBinSum = 0;
    MUINT16 u2AEHISTBinValue[u4AEHISTLogBinNumber];
    // Set value for each Bin
    for (MINT32 i = u4AEHISTLogBinNumber-1; i >= 0; i--)
    {
        if (i > 0)
        {
            u2AEHISTBinValue[i] = u4AEHISTMaxVal / 256;
            u4AEHISTMaxVal = u4AEHISTMaxVal / 2;
        }
        else
        {
            u2AEHISTBinValue[i] = 0;
        }
    }

    for (MUINT32 u4AEHistBinIdx = 0; u4AEHistBinIdx < 16; u4AEHistBinIdx ++) { // calculate log hist[0] ~ hist[16]
        MUINT32 u4AEHistSEIdx = (MVHDR3EXPO_WIDTH * MVHDR3EXPO_HEIGHT) * 2 + 256 + u2AEHISTBinValue[u4AEHistBinIdx];
        MUINT32 u4AEHistSE = 0;
        MUINT32 u4UpData, u4MidData, u4LowData;
        u4UpData  = pAEHistStat[u4AEHistLogOffset + u4AEHistBinIdx * u4AEHistBinOffset];
        u4MidData = pAEHistStat[u4AEHistLogOffset + u4AEHistBinIdx * u4AEHistBinOffset+1];
        u4LowData = pAEHistStat[u4AEHistLogOffset + u4AEHistBinIdx * u4AEHistBinOffset+2];
        u4AEHistSE += ((u4UpData  & 0x00FF) << 15);
        u4AEHistSE += ((u4MidData & 0x00FF) << 7 );
        u4AEHistSE += ((u4LowData & 0x00FE) >> 1 );
        pOutput[u4AEHistSEIdx] += u4AEHistSE; // SE Hist

        MY_LOG_IF(bDebugHEnable, "[%s] RegAEHIST_log Bin[%d]=[0x%x 0x%x 0x%x 0x%x 0x%x]", __FUNCTION__, u4AEHistBinIdx,
        pAEHistStat[u4AEHistLogOffset + u4AEHistBinIdx * u4AEHistBinOffset]  , pAEHistStat[u4AEHistLogOffset + u4AEHistBinIdx * u4AEHistBinOffset+1],
        pAEHistStat[u4AEHistLogOffset + u4AEHistBinIdx * u4AEHistBinOffset+2], pAEHistStat[u4AEHistLogOffset + u4AEHistBinIdx * u4AEHistBinOffset+3]  , pAEHistStat[u4AEHistLogOffset + u4AEHistBinIdx * u4AEHistBinOffset+4]);

        MY_LOG_IF(bDebugHEnable, "[%s] ValAEHIST_log[%d] SE[%d]=[%d]", __FUNCTION__, u4AEHistBinIdx, u2AEHISTBinValue[u4AEHistBinIdx], pOutput[u4AEHistSEIdx]);
    }

}

MVOID getMVHDR3Expo_AEInfo(const MINT32 i4Ratio, MINT32 &i4SEDeltaEVx100)
{
    i4SEDeltaEVx100 = 100; // temp for test
}


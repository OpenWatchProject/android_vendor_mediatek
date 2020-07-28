#include <cstdio>
#include <cstdlib>
#include <utils/Log.h>
#include <fcntl.h>
#include <cutils/log.h>
#include <string.h>
#include <cutils/properties.h>
#include <unordered_map>

#include "camera_custom_ae_nvram.h"
#include "camera_custom_nvram.h"
#include "camera_custom_sensor.h"
#include "image_sensor.h"
#include "kd_imgsensor_define.h"

//camera_info_header
#include "camera_info_imx576mipiraw.h"

#if MTK_CAM_NEW_NVRAM_SUPPORT
    #include INCLUDE_FILENAME_TUNING_MAPPING
#endif

#define MVHDR3EXPOFLK_WIDTH  4
#define MVHDR3EXPOFLK_HEIGHT 96
#define MVHDR3EXPOFLK_OUTSIZE (768*10/8)
#define MVHDR3EXPO_WIDTH  8
#define MVHDR3EXPO_HEIGHT 6
#define CLAMP(x,min,max) (((x) > (max)) ? (max) : (((x) < (min)) ? (min) : (x)))
#define MY_LOGD(fmt, arg...)    ALOGD(fmt, ##arg)
#define MY_LOGE(fmt, arg...)    ALOGE(fmt, ##arg)
#define MY_LOG_IF(cond, ...)    do { if ( (cond) ) { MY_LOGD(__VA_ARGS__); } }while(0)

typedef NSFeature::CamsvCustomDecode<SENSOR_ID, MODULE_INDEX> SensorInfoSingleton_T;

namespace NSFeature {

CAMSV_CUSTOM_BUFFER_INFO getAeBufSize_imx576mipiraw();
CAMSV_CUSTOM_BUFFER_INFO getFlkBufsize_imx576mipiraw();
UINT32 DecodeAe_imx576mipiraw(VOID* const pData);
UINT32 DecodeFlk_imx576mipiraw(VOID* const pData);

template <>
CAMSV_CUSTOM_BUFFER_INFO
SensorInfoSingleton_T::
impCamsvBufSize(CAMSV_CUSTOM_DECODE_ENUM const CamsvCusDecodeFeature) const
{
    CAMSV_CUSTOM_BUFFER_INFO buf_size = {0};
    switch(CamsvCusDecodeFeature)
    {
        case CAMSV_CUSTOM_DECODE_AE:
            buf_size = getAeBufSize_imx576mipiraw();
            break;
        case CAMSV_CUSTOM_DECODE_FLK:
            buf_size = getFlkBufsize_imx576mipiraw();
            break;
        default:
            ALOGD("[impCamsvBufSize] undefined function type");
            break;
    }
    return buf_size;
}

CAMSV_CUSTOM_BUFFER_INFO getAeBufSize_imx576mipiraw()
{
    CAMSV_CUSTOM_BUFFER_INFO buf;
    buf.width = MVHDR3EXPO_WIDTH;
    buf.height = MVHDR3EXPO_HEIGHT;
    buf.size = MVHDR3EXPO_WIDTH * MVHDR3EXPO_HEIGHT * 2 + 256 * 2; // AEY_LE + AEY_SE + AEHist_LE + AEHist_SE
    return buf;
}

CAMSV_CUSTOM_BUFFER_INFO getFlkBufsize_imx576mipiraw()
{
    CAMSV_CUSTOM_BUFFER_INFO buf;
    buf.width = MVHDR3EXPOFLK_WIDTH;
    buf.height = MVHDR3EXPOFLK_HEIGHT;
    buf.size = MVHDR3EXPOFLK_WIDTH * MVHDR3EXPOFLK_HEIGHT;
    return buf;
}

template <>
UINT32
SensorInfoSingleton_T::
impCamsvCustomDecode(CAMSV_CUSTOM_DECODE_ENUM const CamsvCusDecodeFeature, VOID* const pData) const
{
    switch(CamsvCusDecodeFeature)
    {
        case CAMSV_CUSTOM_DECODE_AE:
            DecodeAe_imx576mipiraw(pData);
            break;
        case CAMSV_CUSTOM_DECODE_FLK:
            DecodeFlk_imx576mipiraw(pData);
            break;
        default:
            ALOGD("[impCamsvCustomDecode] undefined function type");
            break;
    }
    return 0;
}

UINT32 DecodeAe_imx576mipiraw(VOID* const pData)
{
    CAMSV_STAT_T* input = reinterpret_cast<CAMSV_STAT_T *>(pData);

    char value[PROPERTY_VALUE_MAX] = {'\0'};
    property_get("vendor.debug.mvhdr3ExpoL.custom.enable", value, "0");
    MBOOL bDebugLEnable = (MBOOL)atoi(value);
    property_get("vendor.debug.mvhdr3ExpoH.custom.enable", value, "0");
    MBOOL bDebugHEnable = (MBOOL)atoi(value);
    MUINT32 *pOutput     = reinterpret_cast<MUINT32 *>(input->pAeOutputDataPointer);
    MUINT8  *pAEYStat    = reinterpret_cast<MUINT8 *>(input->mapCamsvDataPointer[VC_3HDR_Y]);
    MUINT8  *pAEHistStat = reinterpret_cast<MUINT8 *>(input->mapCamsvDataPointer[VC_3HDR_AE]);
    MUINT8  *pAEEBDData = reinterpret_cast<MUINT8 *>(input->mapCamsvDataPointer[VC_3HDR_EMBEDDED]);
    CAMSV_CUSTOM_BUFFER_INFO buf = getAeBufSize_imx576mipiraw();
    memset(pOutput, 0, buf.size * sizeof(MUINT32));
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
        input->u4MVHDRRatio_x100 = u4CurRatio = (u4LELine*fLEGain)*100 / (u4SELine*fSEGain);
        if (input->u4MVHDRRatio_x100 < 100) input->u4MVHDRRatio_x100 = 100;

        MY_LOG_IF(bDebugLEnable, "[%s] EBDData i4SensorDev[%d] XY[%d*%d=%d][0x%x 0x%x 0x%x 0x%x] HDRMode[0x%x] Ratio[%d] LE[%d %f][0x%x 0x%x 0x%x 0x%x] SE[%d %f][0x%x 0x%x 0x%x 0x%x] ME[%d %f][0x%x 0x%x 0x%x 0x%x]", __FUNCTION__,
        input->i4SensorDev, u4XSize, u4YSize, u4AEHistTotalSum, pAEEBDData[39*u4AEEBDDataOffset+2], pAEEBDData[40*u4AEEBDDataOffset+0], pAEEBDData[40*u4AEEBDDataOffset+2], pAEEBDData[41*u4AEEBDDataOffset+0], pAEEBDData[20*u4AEEBDDataOffset+2],
        u4CurRatio, u4LELine, fLEGain,
        pAEEBDData[11*u4AEEBDDataOffset+2], pAEEBDData[12*u4AEEBDDataOffset+0], pAEEBDData[12*u4AEEBDDataOffset+2], pAEEBDData[13*u4AEEBDDataOffset+0],
        u4SELine, fSEGain,
        pAEEBDData[22*u4AEEBDDataOffset+2], pAEEBDData[23*u4AEEBDDataOffset+0], pAEEBDData[18*u4AEEBDDataOffset+0], pAEEBDData[18*u4AEEBDDataOffset+2],
        u4MELine, fMEGain,
        pAEEBDData[(u4XSize/4+40)*u4AEEBDDataOffset+2], pAEEBDData[(u4XSize/4+41)*u4AEEBDDataOffset+0], pAEEBDData[(u4XSize/4+41)*u4AEEBDDataOffset+2], pAEEBDData[(u4XSize/4+42)*u4AEEBDDataOffset+0]);
    }
    else MY_LOG_IF(bDebugLEnable, "[%s] EBDData = NULL.", __FUNCTION__);

    /* YHIST => AEY_LE & AEY_SE */
    MUINT32 u4AEYMaxVal = (1024 * input->u4MVHDRRatio_x100 / 100)-1;
    // const MUINT32 u4AEYBinSet = 2;                       // 1 Set = 2 Bins
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
        // MUINT32 u4AEHistSEIdx = (MVHDR3EXPO_WIDTH * MVHDR3EXPO_HEIGHT) * 2 + u4AEHistBinIdx * 2 + 1 + 256;
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
    MUINT32 u4AEHISTMaxVal = 65535; // , u4AEHISTLogAve = 0, u4AEHISTBinSum = 0;
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

    return 0;
}

UINT32 DecodeFlk_imx576mipiraw(VOID* const pData)
{
    CAMSV_STAT_T* input = reinterpret_cast<CAMSV_STAT_T *>(pData);
    MUINT32 *pOutput    = reinterpret_cast<MUINT32 *>(input->pFlkOutputDataPointer);
    MUINT8 *pFLKStat    = reinterpret_cast<MUINT8 *>(input->mapCamsvDataPointer[VC_3HDR_FLICKER]);

    // Decode data
    MUINT32 mode = 0;
    if( NULL == pFLKStat || NULL == pOutput )
    {
        if( NULL == pFLKStat)
            MY_LOGE("[%s] pFLKStat is NULL", __FUNCTION__);
        else
            MY_LOGD("[%s] pOutput is NULL", __FUNCTION__);
        return 0;
    }

    char value[PROPERTY_VALUE_MAX] = {'\0'};
    property_get("vendor.debug.mvhdr3expoflk.custom.enable", value, "0");
    MBOOL bDebugEnable = (MBOOL)atoi(value);
    CAMSV_CUSTOM_BUFFER_INFO buf = getFlkBufsize_imx576mipiraw();
    memset(pOutput, 0, buf.size * sizeof(MUINT32));

    MY_LOG_IF(bDebugEnable, "[%s] stat W/H: %d/%d u4MVHDRFLK_Mode: %d", __FUNCTION__, MVHDR3EXPOFLK_WIDTH, MVHDR3EXPOFLK_HEIGHT, mode);

    for (MUINT32 u4FLKIdx = 0; u4FLKIdx < (MVHDR3EXPOFLK_WIDTH * MVHDR3EXPOFLK_HEIGHT); u4FLKIdx ++) { // initialization
        pOutput[u4FLKIdx] = 0;
    }

    /* DECODE */
    for (MUINT32 u4FLKIdx = 0, u4FLKOutIdx = 0; u4FLKIdx < MVHDR3EXPOFLK_OUTSIZE; u4FLKIdx +=5) {
        pOutput[u4FLKOutIdx++] = (
        //((pFLKStat[u4FLKIdx+2] & 0x0E ) >> 2 ) |// 1~0
        //((pFLKStat[u4FLKIdx  ] >>   2 ) << 2 ) |// 7~2
        //((pFLKStat[u4FLKIdx+1] & 0x03 ) << 8 )  // 8~9
        ((pFLKStat[u4FLKIdx  ]        ) << 2 ) |// 9:2
        ((pFLKStat[u4FLKIdx+1] & 0xc0 ) >> 6 )  // 1:0
        );

        pOutput[u4FLKOutIdx++] = (
        ((pFLKStat[u4FLKIdx+2]        ) << 2 ) |// 9:2
        ((pFLKStat[u4FLKIdx+3] & 0xc0 ) >> 6 )  // 1:0
        );

        MY_LOG_IF(bDebugEnable, "[%s] pFLKStat[%03d-%03d] %d,%d,%d,%d,%d", __FUNCTION__,
        u4FLKIdx, u4FLKIdx+4,
        pFLKStat[u4FLKIdx],
        pFLKStat[u4FLKIdx + 1],
        pFLKStat[u4FLKIdx + 2],
        pFLKStat[u4FLKIdx + 3],
        pFLKStat[u4FLKIdx + 4]);
//        MY_LOG_IF(bDebugEnable, "[%s] pOutput[%03d-%03d] %d,%d", __FUNCTION__, u4FLKOutIdx-2, u4FLKOutIdx-1, pOutput[u4FLKOutIdx - 2], pOutput[u4FLKOutIdx - 1]);
    }

    for (MUINT32 u4FLKIdx = 0; u4FLKIdx < (MVHDR3EXPOFLK_WIDTH * MVHDR3EXPOFLK_HEIGHT); u4FLKIdx +=4) { // initialization
        MY_LOG_IF(bDebugEnable, "[%s] pOutput[%03d-%03d] %3d,%3d,%3d,%3d", __FUNCTION__,
        u4FLKIdx, u4FLKIdx+3,
        pOutput[u4FLKIdx],
        pOutput[u4FLKIdx+1],
        pOutput[u4FLKIdx+2],
        pOutput[u4FLKIdx+3] );
    }

    return 0;
}

};  //  NSFeature


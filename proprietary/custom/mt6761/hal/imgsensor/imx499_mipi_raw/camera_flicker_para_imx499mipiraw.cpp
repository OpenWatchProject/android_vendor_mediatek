//[Sensor]
//name = imx499mipiraw
//bin_ratio = 1,2
//
//[Preview]
//read_freq = 280000000
//pixel_line = 5120
//column_length = 1721
//
//[Capture]
//read_freq = 564000000
//pixel_line = 5120
//column_length = 3467
//
//[Video]
//read_freq = 420000000
//pixel_line = 5120
//column_length = 2583
//
//[Video1]
//read_freq = 580000000
//pixel_line = 5120
//column_length = 695
//
//[Video2]
//read_freq = 120000000
//pixel_line = 5120
//column_length = 695
//
#define LOG_TAG "imx499mipiraw"

#include <utils/Log.h>
#include <fcntl.h>
#include <math.h>
#include "camera_custom_nvram.h"
#include "camera_custom_sensor.h"
#include "image_sensor.h"
#include "kd_imgsensor_define.h"
#include "camera_AE_PLineTable_imx499mipiraw.h"
#include "camera_info_imx499mipiraw.h"
#include "camera_custom_AEPlinetable.h"
#include "camera_custom_flicker_para.h"
#include <cutils/log.h>


extern "C" fptrFlicker getFlickerParaFunc(MUINT32 sensorType)
{
    if(sensorType==0)
        return NSFeature::RAWSensorInfo<SENSOR_ID>::getFlickerPara;
    else
        return NSFeature::YUVSensorInfo<SENSOR_ID>::getNullFlickerPara;
}

static void get_flicker_para_by_Preview_Bin1(FLICKER_CUST_PARA* para)
{
    int freq[9] = {70, 80, 90, 100, 120, 130, 140, 160, 170};
    FLICKER_CUST_STATISTICS EV50_L50 = {-194, 4721, 381, -766};
    FLICKER_CUST_STATISTICS EV50_L60 = {1103, 421, 787, -397};
    FLICKER_CUST_STATISTICS EV60_L50 = {1315, 444, 1172, -471};
    FLICKER_CUST_STATISTICS EV60_L60 = {-162, 2898, 247, -642};
    for (int i = 0; i < 9; i++)
        para->flickerFreq[i] = freq[i];
    para->flickerGradThreshold = 26;
    para->flickerSearchRange = 28;
    para->minPastFrames = 3;
    para->maxPastFrames = 14;
    para->EV50_L50 = EV50_L50;
    para->EV50_L60 = EV50_L60;
    para->EV60_L50 = EV60_L50;
    para->EV60_L60 = EV60_L60;
    para->EV50_thresholds[0] = -30;
    para->EV50_thresholds[1] = 12;
    para->EV60_thresholds[0] = -30;
    para->EV60_thresholds[1] = 12;
    para->freq_feature_index[0] = 4;
    para->freq_feature_index[1] = 3;
}

static void get_flicker_para_by_Preview_Bin2(FLICKER_CUST_PARA* para)
{
    int freq[9] = {70, 80, 90, 100, 120, 130, 140, 170, 230};
    FLICKER_CUST_STATISTICS EV50_L50 = {-194, 4721, 381, -766};
    FLICKER_CUST_STATISTICS EV50_L60 = {1052, 441, 825, -409};
    FLICKER_CUST_STATISTICS EV60_L50 = {1255, 465, 1228, -483};
    FLICKER_CUST_STATISTICS EV60_L60 = {-162, 2898, 247, -642};
    for (int i = 0; i < 9; i++)
        para->flickerFreq[i] = freq[i];
    para->flickerGradThreshold = 32;
    para->flickerSearchRange = 12;
    para->minPastFrames = 3;
    para->maxPastFrames = 14;
    para->EV50_L50 = EV50_L50;
    para->EV50_L60 = EV50_L60;
    para->EV60_L50 = EV60_L50;
    para->EV60_L60 = EV60_L60;
    para->EV50_thresholds[0] = -30;
    para->EV50_thresholds[1] = 12;
    para->EV60_thresholds[0] = -30;
    para->EV60_thresholds[1] = 12;
    para->freq_feature_index[0] = 4;
    para->freq_feature_index[1] = 3;
}

static void get_flicker_para_by_Capture_Bin1(FLICKER_CUST_PARA* para)
{
    int freq[9] = {70, 80, 90, 100, 120, 130, 140, 160, 170};
    FLICKER_CUST_STATISTICS EV50_L50 = {-194, 4721, 381, -766};
    FLICKER_CUST_STATISTICS EV50_L60 = {1104, 421, 786, -397};
    FLICKER_CUST_STATISTICS EV60_L50 = {1316, 443, 1171, -471};
    FLICKER_CUST_STATISTICS EV60_L60 = {-162, 2898, 247, -642};
    for (int i = 0; i < 9; i++)
        para->flickerFreq[i] = freq[i];
    para->flickerGradThreshold = 24;
    para->flickerSearchRange = 56;
    para->minPastFrames = 3;
    para->maxPastFrames = 14;
    para->EV50_L50 = EV50_L50;
    para->EV50_L60 = EV50_L60;
    para->EV60_L50 = EV60_L50;
    para->EV60_L60 = EV60_L60;
    para->EV50_thresholds[0] = -30;
    para->EV50_thresholds[1] = 12;
    para->EV60_thresholds[0] = -30;
    para->EV60_thresholds[1] = 12;
    para->freq_feature_index[0] = 4;
    para->freq_feature_index[1] = 3;
}

static void get_flicker_para_by_Capture_Bin2(FLICKER_CUST_PARA* para)
{
    int freq[9] = {70, 80, 90, 100, 120, 130, 140, 160, 170};
    FLICKER_CUST_STATISTICS EV50_L50 = {-194, 4721, 381, -766};
    FLICKER_CUST_STATISTICS EV50_L60 = {1073, 433, 809, -404};
    FLICKER_CUST_STATISTICS EV60_L50 = {1279, 456, 1205, -478};
    FLICKER_CUST_STATISTICS EV60_L60 = {-162, 2898, 247, -642};
    for (int i = 0; i < 9; i++)
        para->flickerFreq[i] = freq[i];
    para->flickerGradThreshold = 25;
    para->flickerSearchRange = 28;
    para->minPastFrames = 3;
    para->maxPastFrames = 14;
    para->EV50_L50 = EV50_L50;
    para->EV50_L60 = EV50_L60;
    para->EV60_L50 = EV60_L50;
    para->EV60_L60 = EV60_L60;
    para->EV50_thresholds[0] = -30;
    para->EV50_thresholds[1] = 12;
    para->EV60_thresholds[0] = -30;
    para->EV60_thresholds[1] = 12;
    para->freq_feature_index[0] = 4;
    para->freq_feature_index[1] = 3;
}

static void get_flicker_para_by_Video_Bin1(FLICKER_CUST_PARA* para)
{
    int freq[9] = {70, 80, 90, 100, 120, 130, 140, 160, 170};
    FLICKER_CUST_STATISTICS EV50_L50 = {-194, 4721, 381, -766};
    FLICKER_CUST_STATISTICS EV50_L60 = {1106, 420, 785, -396};
    FLICKER_CUST_STATISTICS EV60_L50 = {1319, 442, 1169, -471};
    FLICKER_CUST_STATISTICS EV60_L60 = {-162, 2898, 247, -642};
    for (int i = 0; i < 9; i++)
        para->flickerFreq[i] = freq[i];
    para->flickerGradThreshold = 24;
    para->flickerSearchRange = 44;
    para->minPastFrames = 3;
    para->maxPastFrames = 14;
    para->EV50_L50 = EV50_L50;
    para->EV50_L60 = EV50_L60;
    para->EV60_L50 = EV60_L50;
    para->EV60_L60 = EV60_L60;
    para->EV50_thresholds[0] = -30;
    para->EV50_thresholds[1] = 12;
    para->EV60_thresholds[0] = -30;
    para->EV60_thresholds[1] = 12;
    para->freq_feature_index[0] = 4;
    para->freq_feature_index[1] = 3;
}

static void get_flicker_para_by_Video_Bin2(FLICKER_CUST_PARA* para)
{
    int freq[9] = {70, 80, 90, 100, 120, 130, 140, 160, 170};
    FLICKER_CUST_STATISTICS EV50_L50 = {-194, 4721, 381, -766};
    FLICKER_CUST_STATISTICS EV50_L60 = {1065, 436, 815, -406};
    FLICKER_CUST_STATISTICS EV60_L50 = {1270, 459, 1213, -480};
    FLICKER_CUST_STATISTICS EV60_L60 = {-162, 2898, 247, -642};
    for (int i = 0; i < 9; i++)
        para->flickerFreq[i] = freq[i];
    para->flickerGradThreshold = 27;
    para->flickerSearchRange = 20;
    para->minPastFrames = 3;
    para->maxPastFrames = 14;
    para->EV50_L50 = EV50_L50;
    para->EV50_L60 = EV50_L60;
    para->EV60_L50 = EV60_L50;
    para->EV60_L60 = EV60_L60;
    para->EV50_thresholds[0] = -30;
    para->EV50_thresholds[1] = 12;
    para->EV60_thresholds[0] = -30;
    para->EV60_thresholds[1] = 12;
    para->freq_feature_index[0] = 4;
    para->freq_feature_index[1] = 3;
}

static void get_flicker_para_by_Video1_Bin1(FLICKER_CUST_PARA* para)
{
    int freq[9] = {70, 80, 100, 120, 160, 170, 190, 210, 230};
    FLICKER_CUST_STATISTICS EV50_L50 = {-194, 4721, 381, -766};
    FLICKER_CUST_STATISTICS EV50_L60 = {153, 3029, 5657, -902};
    FLICKER_CUST_STATISTICS EV60_L50 = {183, 3190, 8422, -976};
    FLICKER_CUST_STATISTICS EV60_L60 = {-162, 2898, 247, -642};
    for (int i = 0; i < 9; i++)
        para->flickerFreq[i] = freq[i];
    para->flickerGradThreshold = 25;
    para->flickerSearchRange = 8;
    para->minPastFrames = 3;
    para->maxPastFrames = 14;
    para->EV50_L50 = EV50_L50;
    para->EV50_L60 = EV50_L60;
    para->EV60_L50 = EV60_L50;
    para->EV60_L60 = EV60_L60;
    para->EV50_thresholds[0] = -30;
    para->EV50_thresholds[1] = 12;
    para->EV60_thresholds[0] = -30;
    para->EV60_thresholds[1] = 12;
    para->freq_feature_index[0] = 3;
    para->freq_feature_index[1] = 2;
}

static void get_flicker_para_by_Video1_Bin2(FLICKER_CUST_PARA* para)
{
    int freq[9] = {70, 80, 100, 120, 160, 170, 190, 210, 230};
    FLICKER_CUST_STATISTICS EV50_L50 = {-194, 4721, 381, -766};
    FLICKER_CUST_STATISTICS EV50_L60 = {152, 3057, 5710, -904};
    FLICKER_CUST_STATISTICS EV60_L50 = {181, 3219, 8499, -978};
    FLICKER_CUST_STATISTICS EV60_L60 = {-162, 2898, 247, -642};
    for (int i = 0; i < 9; i++)
        para->flickerFreq[i] = freq[i];
    para->flickerGradThreshold = 27;
    para->flickerSearchRange = 4;
    para->minPastFrames = 3;
    para->maxPastFrames = 14;
    para->EV50_L50 = EV50_L50;
    para->EV50_L60 = EV50_L60;
    para->EV60_L50 = EV60_L50;
    para->EV60_L60 = EV60_L60;
    para->EV50_thresholds[0] = -30;
    para->EV50_thresholds[1] = 12;
    para->EV60_thresholds[0] = -30;
    para->EV60_thresholds[1] = 12;
    para->freq_feature_index[0] = 3;
    para->freq_feature_index[1] = 2;
}

static void get_flicker_para_by_Video2_Bin1(FLICKER_CUST_PARA* para)
{
    int freq[9] = {80, 90, 100, 120, 130, 140, 170, 210, 230};
    FLICKER_CUST_STATISTICS EV50_L50 = {-194, 4721, 381, -766};
    FLICKER_CUST_STATISTICS EV50_L60 = {1025, 453, 847, -416};
    FLICKER_CUST_STATISTICS EV60_L50 = {1222, 477, 1261, -490};
    FLICKER_CUST_STATISTICS EV60_L60 = {-162, 2898, 247, -642};
    for (int i = 0; i < 9; i++)
        para->flickerFreq[i] = freq[i];
    para->flickerGradThreshold = 35;
    para->flickerSearchRange = 8;
    para->minPastFrames = 3;
    para->maxPastFrames = 14;
    para->EV50_L50 = EV50_L50;
    para->EV50_L60 = EV50_L60;
    para->EV60_L50 = EV60_L50;
    para->EV60_L60 = EV60_L60;
    para->EV50_thresholds[0] = -30;
    para->EV50_thresholds[1] = 12;
    para->EV60_thresholds[0] = -30;
    para->EV60_thresholds[1] = 12;
    para->freq_feature_index[0] = 3;
    para->freq_feature_index[1] = 2;
}

static void get_flicker_para_by_Video2_Bin2(FLICKER_CUST_PARA* para)
{
    int freq[9] = {80, 90, 100, 120, 130, 140, 170, 210, 230};
    FLICKER_CUST_STATISTICS EV50_L50 = {-194, 4721, 381, -766};
    FLICKER_CUST_STATISTICS EV50_L60 = {1019, 456, 852, -417};
    FLICKER_CUST_STATISTICS EV60_L50 = {1215, 480, 1268, -492};
    FLICKER_CUST_STATISTICS EV60_L60 = {-162, 2898, 247, -642};
    for (int i = 0; i < 9; i++)
        para->flickerFreq[i] = freq[i];
    para->flickerGradThreshold = 56;
    para->flickerSearchRange = 4;
    para->minPastFrames = 3;
    para->maxPastFrames = 14;
    para->EV50_L50 = EV50_L50;
    para->EV50_L60 = EV50_L60;
    para->EV60_L50 = EV60_L50;
    para->EV60_L60 = EV60_L60;
    para->EV50_thresholds[0] = -30;
    para->EV50_thresholds[1] = 12;
    para->EV60_thresholds[0] = -30;
    para->EV60_thresholds[1] = 12;
    para->freq_feature_index[0] = 3;
    para->freq_feature_index[1] = 2;
}

// using [Preview_Bin1] parameters for [Custom1_Bin1]
static void get_flicker_para_by_Custom1_Bin1(FLICKER_CUST_PARA* para)
{
    int freq[9] = {70, 80, 90, 100, 120, 130, 140, 160, 170};
    FLICKER_CUST_STATISTICS EV50_L50 = {-194, 4721, 381, -766};
    FLICKER_CUST_STATISTICS EV50_L60 = {1103, 421, 787, -397};
    FLICKER_CUST_STATISTICS EV60_L50 = {1315, 444, 1172, -471};
    FLICKER_CUST_STATISTICS EV60_L60 = {-162, 2898, 247, -642};
    for (int i = 0; i < 9; i++)
        para->flickerFreq[i] = freq[i];
    para->flickerGradThreshold = 26;
    para->flickerSearchRange = 28;
    para->minPastFrames = 3;
    para->maxPastFrames = 14;
    para->EV50_L50 = EV50_L50;
    para->EV50_L60 = EV50_L60;
    para->EV60_L50 = EV60_L50;
    para->EV60_L60 = EV60_L60;
    para->EV50_thresholds[0] = -30;
    para->EV50_thresholds[1] = 12;
    para->EV60_thresholds[0] = -30;
    para->EV60_thresholds[1] = 12;
    para->freq_feature_index[0] = 4;
    para->freq_feature_index[1] = 3;
}

// using [Preview_Bin1] parameters for [Custom1_Bin2]
static void get_flicker_para_by_Custom1_Bin2(FLICKER_CUST_PARA* para)
{
    int freq[9] = {70, 80, 90, 100, 120, 130, 140, 160, 170};
    FLICKER_CUST_STATISTICS EV50_L50 = {-194, 4721, 381, -766};
    FLICKER_CUST_STATISTICS EV50_L60 = {1103, 421, 787, -397};
    FLICKER_CUST_STATISTICS EV60_L50 = {1315, 444, 1172, -471};
    FLICKER_CUST_STATISTICS EV60_L60 = {-162, 2898, 247, -642};
    for (int i = 0; i < 9; i++)
        para->flickerFreq[i] = freq[i];
    para->flickerGradThreshold = 26;
    para->flickerSearchRange = 28;
    para->minPastFrames = 3;
    para->maxPastFrames = 14;
    para->EV50_L50 = EV50_L50;
    para->EV50_L60 = EV50_L60;
    para->EV60_L50 = EV60_L50;
    para->EV60_L60 = EV60_L60;
    para->EV50_thresholds[0] = -30;
    para->EV50_thresholds[1] = 12;
    para->EV60_thresholds[0] = -30;
    para->EV60_thresholds[1] = 12;
    para->freq_feature_index[0] = 4;
    para->freq_feature_index[1] = 3;
}

// using [Preview_Bin1] parameters for [Custom2_Bin1]
static void get_flicker_para_by_Custom2_Bin1(FLICKER_CUST_PARA* para)
{
    int freq[9] = {70, 80, 90, 100, 120, 130, 140, 160, 170};
    FLICKER_CUST_STATISTICS EV50_L50 = {-194, 4721, 381, -766};
    FLICKER_CUST_STATISTICS EV50_L60 = {1103, 421, 787, -397};
    FLICKER_CUST_STATISTICS EV60_L50 = {1315, 444, 1172, -471};
    FLICKER_CUST_STATISTICS EV60_L60 = {-162, 2898, 247, -642};
    for (int i = 0; i < 9; i++)
        para->flickerFreq[i] = freq[i];
    para->flickerGradThreshold = 26;
    para->flickerSearchRange = 28;
    para->minPastFrames = 3;
    para->maxPastFrames = 14;
    para->EV50_L50 = EV50_L50;
    para->EV50_L60 = EV50_L60;
    para->EV60_L50 = EV60_L50;
    para->EV60_L60 = EV60_L60;
    para->EV50_thresholds[0] = -30;
    para->EV50_thresholds[1] = 12;
    para->EV60_thresholds[0] = -30;
    para->EV60_thresholds[1] = 12;
    para->freq_feature_index[0] = 4;
    para->freq_feature_index[1] = 3;
}

// using [Preview_Bin1] parameters for [Custom2_Bin2]
static void get_flicker_para_by_Custom2_Bin2(FLICKER_CUST_PARA* para)
{
    int freq[9] = {70, 80, 90, 100, 120, 130, 140, 160, 170};
    FLICKER_CUST_STATISTICS EV50_L50 = {-194, 4721, 381, -766};
    FLICKER_CUST_STATISTICS EV50_L60 = {1103, 421, 787, -397};
    FLICKER_CUST_STATISTICS EV60_L50 = {1315, 444, 1172, -471};
    FLICKER_CUST_STATISTICS EV60_L60 = {-162, 2898, 247, -642};
    for (int i = 0; i < 9; i++)
        para->flickerFreq[i] = freq[i];
    para->flickerGradThreshold = 26;
    para->flickerSearchRange = 28;
    para->minPastFrames = 3;
    para->maxPastFrames = 14;
    para->EV50_L50 = EV50_L50;
    para->EV50_L60 = EV50_L60;
    para->EV60_L50 = EV60_L50;
    para->EV60_L60 = EV60_L60;
    para->EV50_thresholds[0] = -30;
    para->EV50_thresholds[1] = 12;
    para->EV60_thresholds[0] = -30;
    para->EV60_thresholds[1] = 12;
    para->freq_feature_index[0] = 4;
    para->freq_feature_index[1] = 3;
}

// using [Preview_Bin1] parameters for [Custom3_Bin1]
static void get_flicker_para_by_Custom3_Bin1(FLICKER_CUST_PARA* para)
{
    int freq[9] = {70, 80, 90, 100, 120, 130, 140, 160, 170};
    FLICKER_CUST_STATISTICS EV50_L50 = {-194, 4721, 381, -766};
    FLICKER_CUST_STATISTICS EV50_L60 = {1103, 421, 787, -397};
    FLICKER_CUST_STATISTICS EV60_L50 = {1315, 444, 1172, -471};
    FLICKER_CUST_STATISTICS EV60_L60 = {-162, 2898, 247, -642};
    for (int i = 0; i < 9; i++)
        para->flickerFreq[i] = freq[i];
    para->flickerGradThreshold = 26;
    para->flickerSearchRange = 28;
    para->minPastFrames = 3;
    para->maxPastFrames = 14;
    para->EV50_L50 = EV50_L50;
    para->EV50_L60 = EV50_L60;
    para->EV60_L50 = EV60_L50;
    para->EV60_L60 = EV60_L60;
    para->EV50_thresholds[0] = -30;
    para->EV50_thresholds[1] = 12;
    para->EV60_thresholds[0] = -30;
    para->EV60_thresholds[1] = 12;
    para->freq_feature_index[0] = 4;
    para->freq_feature_index[1] = 3;
}

// using [Preview_Bin1] parameters for [Custom3_Bin2]
static void get_flicker_para_by_Custom3_Bin2(FLICKER_CUST_PARA* para)
{
    int freq[9] = {70, 80, 90, 100, 120, 130, 140, 160, 170};
    FLICKER_CUST_STATISTICS EV50_L50 = {-194, 4721, 381, -766};
    FLICKER_CUST_STATISTICS EV50_L60 = {1103, 421, 787, -397};
    FLICKER_CUST_STATISTICS EV60_L50 = {1315, 444, 1172, -471};
    FLICKER_CUST_STATISTICS EV60_L60 = {-162, 2898, 247, -642};
    for (int i = 0; i < 9; i++)
        para->flickerFreq[i] = freq[i];
    para->flickerGradThreshold = 26;
    para->flickerSearchRange = 28;
    para->minPastFrames = 3;
    para->maxPastFrames = 14;
    para->EV50_L50 = EV50_L50;
    para->EV50_L60 = EV50_L60;
    para->EV60_L50 = EV60_L50;
    para->EV60_L60 = EV60_L60;
    para->EV50_thresholds[0] = -30;
    para->EV50_thresholds[1] = 12;
    para->EV60_thresholds[0] = -30;
    para->EV60_thresholds[1] = 12;
    para->freq_feature_index[0] = 4;
    para->freq_feature_index[1] = 3;
}

// using [Preview_Bin1] parameters for [Custom4_Bin1]
static void get_flicker_para_by_Custom4_Bin1(FLICKER_CUST_PARA* para)
{
    int freq[9] = {70, 80, 90, 100, 120, 130, 140, 160, 170};
    FLICKER_CUST_STATISTICS EV50_L50 = {-194, 4721, 381, -766};
    FLICKER_CUST_STATISTICS EV50_L60 = {1103, 421, 787, -397};
    FLICKER_CUST_STATISTICS EV60_L50 = {1315, 444, 1172, -471};
    FLICKER_CUST_STATISTICS EV60_L60 = {-162, 2898, 247, -642};
    for (int i = 0; i < 9; i++)
        para->flickerFreq[i] = freq[i];
    para->flickerGradThreshold = 26;
    para->flickerSearchRange = 28;
    para->minPastFrames = 3;
    para->maxPastFrames = 14;
    para->EV50_L50 = EV50_L50;
    para->EV50_L60 = EV50_L60;
    para->EV60_L50 = EV60_L50;
    para->EV60_L60 = EV60_L60;
    para->EV50_thresholds[0] = -30;
    para->EV50_thresholds[1] = 12;
    para->EV60_thresholds[0] = -30;
    para->EV60_thresholds[1] = 12;
    para->freq_feature_index[0] = 4;
    para->freq_feature_index[1] = 3;
}

// using [Preview_Bin1] parameters for [Custom4_Bin2]
static void get_flicker_para_by_Custom4_Bin2(FLICKER_CUST_PARA* para)
{
    int freq[9] = {70, 80, 90, 100, 120, 130, 140, 160, 170};
    FLICKER_CUST_STATISTICS EV50_L50 = {-194, 4721, 381, -766};
    FLICKER_CUST_STATISTICS EV50_L60 = {1103, 421, 787, -397};
    FLICKER_CUST_STATISTICS EV60_L50 = {1315, 444, 1172, -471};
    FLICKER_CUST_STATISTICS EV60_L60 = {-162, 2898, 247, -642};
    for (int i = 0; i < 9; i++)
        para->flickerFreq[i] = freq[i];
    para->flickerGradThreshold = 26;
    para->flickerSearchRange = 28;
    para->minPastFrames = 3;
    para->maxPastFrames = 14;
    para->EV50_L50 = EV50_L50;
    para->EV50_L60 = EV50_L60;
    para->EV60_L50 = EV60_L50;
    para->EV60_L60 = EV60_L60;
    para->EV50_thresholds[0] = -30;
    para->EV50_thresholds[1] = 12;
    para->EV60_thresholds[0] = -30;
    para->EV60_thresholds[1] = 12;
    para->freq_feature_index[0] = 4;
    para->freq_feature_index[1] = 3;
}

// using [Preview_Bin1] parameters for [Custom5_Bin1]
static void get_flicker_para_by_Custom5_Bin1(FLICKER_CUST_PARA* para)
{
    int freq[9] = {70, 80, 90, 100, 120, 130, 140, 160, 170};
    FLICKER_CUST_STATISTICS EV50_L50 = {-194, 4721, 381, -766};
    FLICKER_CUST_STATISTICS EV50_L60 = {1103, 421, 787, -397};
    FLICKER_CUST_STATISTICS EV60_L50 = {1315, 444, 1172, -471};
    FLICKER_CUST_STATISTICS EV60_L60 = {-162, 2898, 247, -642};
    for (int i = 0; i < 9; i++)
        para->flickerFreq[i] = freq[i];
    para->flickerGradThreshold = 26;
    para->flickerSearchRange = 28;
    para->minPastFrames = 3;
    para->maxPastFrames = 14;
    para->EV50_L50 = EV50_L50;
    para->EV50_L60 = EV50_L60;
    para->EV60_L50 = EV60_L50;
    para->EV60_L60 = EV60_L60;
    para->EV50_thresholds[0] = -30;
    para->EV50_thresholds[1] = 12;
    para->EV60_thresholds[0] = -30;
    para->EV60_thresholds[1] = 12;
    para->freq_feature_index[0] = 4;
    para->freq_feature_index[1] = 3;
}

// using [Preview_Bin1] parameters for [Custom5_Bin2]
static void get_flicker_para_by_Custom5_Bin2(FLICKER_CUST_PARA* para)
{
    int freq[9] = {70, 80, 90, 100, 120, 130, 140, 160, 170};
    FLICKER_CUST_STATISTICS EV50_L50 = {-194, 4721, 381, -766};
    FLICKER_CUST_STATISTICS EV50_L60 = {1103, 421, 787, -397};
    FLICKER_CUST_STATISTICS EV60_L50 = {1315, 444, 1172, -471};
    FLICKER_CUST_STATISTICS EV60_L60 = {-162, 2898, 247, -642};
    for (int i = 0; i < 9; i++)
        para->flickerFreq[i] = freq[i];
    para->flickerGradThreshold = 26;
    para->flickerSearchRange = 28;
    para->minPastFrames = 3;
    para->maxPastFrames = 14;
    para->EV50_L50 = EV50_L50;
    para->EV50_L60 = EV50_L60;
    para->EV60_L50 = EV60_L50;
    para->EV60_L60 = EV60_L60;
    para->EV50_thresholds[0] = -30;
    para->EV50_thresholds[1] = 12;
    para->EV60_thresholds[0] = -30;
    para->EV60_thresholds[1] = 12;
    para->freq_feature_index[0] = 4;
    para->freq_feature_index[1] = 3;
}


typedef NSFeature::RAWSensorInfo<SENSOR_ID> SensorInfoSingleton_T;
namespace NSFeature {
template <> UINT32 SensorInfoSingleton_T::
impGetFlickerPara(MINT32 sensorMode, MINT32 binRatio, MVOID* const pDataBuf) const
{
    ALOGD("impGetFlickerPara(): mode(%d), binRatio(%d).", sensorMode, binRatio);

    FLICKER_CUST_PARA* para;
    para = (FLICKER_CUST_PARA *)pDataBuf;

    if (sensorMode == e_sensorModePreview && binRatio == 1)
        get_flicker_para_by_Preview_Bin1(para);
    else if (sensorMode == e_sensorModePreview && binRatio == 2)
        get_flicker_para_by_Preview_Bin2(para);
    else if (sensorMode == e_sensorModeCapture && binRatio == 1)
        get_flicker_para_by_Capture_Bin1(para);
    else if (sensorMode == e_sensorModeCapture && binRatio == 2)
        get_flicker_para_by_Capture_Bin2(para);
    else if (sensorMode == e_sensorModeVideo && binRatio == 1)
        get_flicker_para_by_Video_Bin1(para);
    else if (sensorMode == e_sensorModeVideo && binRatio == 2)
        get_flicker_para_by_Video_Bin2(para);
    else if (sensorMode == e_sensorModeVideo1 && binRatio == 1)
        get_flicker_para_by_Video1_Bin1(para);
    else if (sensorMode == e_sensorModeVideo1 && binRatio == 2)
        get_flicker_para_by_Video1_Bin2(para);
    else if (sensorMode == e_sensorModeVideo2 && binRatio == 1)
        get_flicker_para_by_Video2_Bin1(para);
    else if (sensorMode == e_sensorModeVideo2 && binRatio == 2)
        get_flicker_para_by_Video2_Bin2(para);
    else if (sensorMode == e_sensorModeCustom1 && binRatio == 1)
        get_flicker_para_by_Custom1_Bin1(para);
    else if (sensorMode == e_sensorModeCustom1 && binRatio == 2)
        get_flicker_para_by_Custom1_Bin2(para);
    else if (sensorMode == e_sensorModeCustom2 && binRatio == 1)
        get_flicker_para_by_Custom2_Bin1(para);
    else if (sensorMode == e_sensorModeCustom2 && binRatio == 2)
        get_flicker_para_by_Custom2_Bin2(para);
    else if (sensorMode == e_sensorModeCustom3 && binRatio == 1)
        get_flicker_para_by_Custom3_Bin1(para);
    else if (sensorMode == e_sensorModeCustom3 && binRatio == 2)
        get_flicker_para_by_Custom3_Bin2(para);
    else if (sensorMode == e_sensorModeCustom4 && binRatio == 1)
        get_flicker_para_by_Custom4_Bin1(para);
    else if (sensorMode == e_sensorModeCustom4 && binRatio == 2)
        get_flicker_para_by_Custom4_Bin2(para);
    else if (sensorMode == e_sensorModeCustom5 && binRatio == 1)
        get_flicker_para_by_Custom5_Bin1(para);
    else if (sensorMode == e_sensorModeCustom5 && binRatio == 2)
        get_flicker_para_by_Custom5_Bin2(para);

    else {
        ALOGD("impGetFlickerPara(): failed to find prameters.");
        return -1;
    }

    return 0;
}
}

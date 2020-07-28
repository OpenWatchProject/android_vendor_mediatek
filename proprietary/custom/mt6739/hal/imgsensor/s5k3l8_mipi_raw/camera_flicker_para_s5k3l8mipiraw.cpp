//[Sensor]
//name = s5k3l8mipiraw
//bin_ratio = 1,2
//
//[Preview]
//read_freq = 560000000
//pixel_line = 5808
//column_length = 1535
//
//[Capture]
//read_freq = 560000000
//pixel_line = 5808
//column_length = 3095
//
//[Video]
//read_freq = 560000000
//pixel_line = 5808
//column_length = 3095
//
//[Video1]
//read_freq = 560000000
//pixel_line = 5808
//column_length = 455
//
//[Video2]
//read_freq = 560000000
//pixel_line = 5808
//column_length = 695
//
//[Custom1]
//read_freq = 440000000
//pixel_line = 4592
//column_length = 1527
//
//[Custom2]
//read_freq = 440000000
//pixel_line = 4592
//column_length = 1527
//
//[Custom3]
//read_freq = 440000000
//pixel_line = 4592
//column_length = 1527
//
//[Custom4]
//read_freq = 440000000
//pixel_line = 4592
//column_length = 1527
//
//[Custom5]
//read_freq = 440000000
//pixel_line = 4592
//column_length = 1527
//
#define LOG_TAG "s5k3l8mipiraw"

#include <utils/Log.h>
#include <fcntl.h>
#include <math.h>
#include "camera_custom_nvram.h"
#include "camera_custom_sensor.h"
#include "image_sensor.h"
#include "kd_imgsensor_define.h"
#include "camera_AE_PLineTable_s5k3l8mipiraw.h"
#include "camera_info_s5k3l8mipiraw.h"
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
    int freq[9] = {70, 100, 120, 140, 160, 170, 190, 210, 230};
    FLICKER_CUST_STATISTICS EV50_L50 = {-194, 4721, 381, -766};
    FLICKER_CUST_STATISTICS EV50_L60 = {914, 508, 949, -445};
    FLICKER_CUST_STATISTICS EV60_L50 = {1090, 535, 1413, -519};
    FLICKER_CUST_STATISTICS EV60_L60 = {-162, 2898, 247, -642};
    for (int i = 0; i < 9; i++)
        para->flickerFreq[i] = freq[i];
    para->flickerGradThreshold = 24;
    para->flickerSearchRange = 24;
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
    para->freq_feature_index[0] = 2;
    para->freq_feature_index[1] = 1;
}

static void get_flicker_para_by_Preview_Bin2(FLICKER_CUST_PARA* para)
{
    int freq[9] = {70, 100, 120, 140, 160, 170, 190, 210, 230};
    FLICKER_CUST_STATISTICS EV50_L50 = {-194, 4721, 381, -766};
    FLICKER_CUST_STATISTICS EV50_L60 = {910, 510, 953, -446};
    FLICKER_CUST_STATISTICS EV60_L50 = {1086, 537, 1419, -520};
    FLICKER_CUST_STATISTICS EV60_L60 = {-162, 2898, 247, -642};
    for (int i = 0; i < 9; i++)
        para->flickerFreq[i] = freq[i];
    para->flickerGradThreshold = 28;
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
    para->freq_feature_index[0] = 2;
    para->freq_feature_index[1] = 1;
}

static void get_flicker_para_by_Capture_Bin1(FLICKER_CUST_PARA* para)
{
    int freq[9] = {70, 80, 90, 100, 120, 130, 140, 160, 170};
    FLICKER_CUST_STATISTICS EV50_L50 = {-194, 4721, 381, -766};
    FLICKER_CUST_STATISTICS EV50_L60 = {1207, 385, 719, -374};
    FLICKER_CUST_STATISTICS EV60_L50 = {1440, 405, 1070, -448};
    FLICKER_CUST_STATISTICS EV60_L60 = {-162, 2898, 247, -642};
    for (int i = 0; i < 9; i++)
        para->flickerFreq[i] = freq[i];
    para->flickerGradThreshold = 25;
    para->flickerSearchRange = 52;
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
    FLICKER_CUST_STATISTICS EV50_L60 = {1161, 400, 747, -384};
    FLICKER_CUST_STATISTICS EV60_L50 = {1385, 421, 1113, -458};
    FLICKER_CUST_STATISTICS EV60_L60 = {-162, 2898, 247, -642};
    for (int i = 0; i < 9; i++)
        para->flickerFreq[i] = freq[i];
    para->flickerGradThreshold = 26;
    para->flickerSearchRange = 24;
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
    FLICKER_CUST_STATISTICS EV50_L60 = {1207, 385, 719, -374};
    FLICKER_CUST_STATISTICS EV60_L50 = {1440, 405, 1070, -448};
    FLICKER_CUST_STATISTICS EV60_L60 = {-162, 2898, 247, -642};
    for (int i = 0; i < 9; i++)
        para->flickerFreq[i] = freq[i];
    para->flickerGradThreshold = 24;
    para->flickerSearchRange = 52;
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
    FLICKER_CUST_STATISTICS EV50_L60 = {1161, 400, 747, -384};
    FLICKER_CUST_STATISTICS EV60_L50 = {1385, 421, 1113, -458};
    FLICKER_CUST_STATISTICS EV60_L60 = {-162, 2898, 247, -642};
    for (int i = 0; i < 9; i++)
        para->flickerFreq[i] = freq[i];
    para->flickerGradThreshold = 27;
    para->flickerSearchRange = 24;
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
    int freq[9] = {100, 120, 130, 140, 160, 170, 190, 210, 230};
    FLICKER_CUST_STATISTICS EV50_L50 = {-194, 4721, 381, -766};
    FLICKER_CUST_STATISTICS EV50_L60 = {285, 1632, 3048, -743};
    FLICKER_CUST_STATISTICS EV60_L50 = {339, 1719, 4538, -818};
    FLICKER_CUST_STATISTICS EV60_L60 = {-162, 2898, 247, -642};
    for (int i = 0; i < 9; i++)
        para->flickerFreq[i] = freq[i];
    para->flickerGradThreshold = 25;
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
    para->freq_feature_index[0] = 1;
    para->freq_feature_index[1] = 0;
}

static void get_flicker_para_by_Video1_Bin2(FLICKER_CUST_PARA* para)
{
    int freq[9] = {100, 120, 130, 140, 160, 170, 190, 210, 230};
    FLICKER_CUST_STATISTICS EV50_L50 = {-194, 4721, 381, -766};
    FLICKER_CUST_STATISTICS EV50_L60 = {262, 1774, 3314, -765};
    FLICKER_CUST_STATISTICS EV60_L50 = {312, 1869, 4933, -839};
    FLICKER_CUST_STATISTICS EV60_L60 = {-162, 2898, 247, -642};
    for (int i = 0; i < 9; i++)
        para->flickerFreq[i] = freq[i];
    para->flickerGradThreshold = 27;
    para->flickerSearchRange = 0;
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
    para->freq_feature_index[0] = 1;
    para->freq_feature_index[1] = 0;
}

static void get_flicker_para_by_Video2_Bin1(FLICKER_CUST_PARA* para)
{
    int freq[9] = {70, 100, 120, 140, 160, 170, 190, 210, 230};
    FLICKER_CUST_STATISTICS EV50_L50 = {-194, 4721, 381, -766};
    FLICKER_CUST_STATISTICS EV50_L60 = {235, 1971, 3683, -792};
    FLICKER_CUST_STATISTICS EV60_L50 = {281, 2076, 5482, -866};
    FLICKER_CUST_STATISTICS EV60_L60 = {-162, 2898, 247, -642};
    for (int i = 0; i < 9; i++)
        para->flickerFreq[i] = freq[i];
    para->flickerGradThreshold = 22;
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
    para->freq_feature_index[0] = 2;
    para->freq_feature_index[1] = 1;
}

static void get_flicker_para_by_Video2_Bin2(FLICKER_CUST_PARA* para)
{
    int freq[9] = {70, 80, 100, 120, 160, 170, 190, 210, 230};
    FLICKER_CUST_STATISTICS EV50_L50 = {-194, 4721, 381, -766};
    FLICKER_CUST_STATISTICS EV50_L60 = {194, 2397, 4478, -842};
    FLICKER_CUST_STATISTICS EV60_L50 = {231, 2525, 6666, -916};
    FLICKER_CUST_STATISTICS EV60_L60 = {-162, 2898, 247, -642};
    for (int i = 0; i < 9; i++)
        para->flickerFreq[i] = freq[i];
    para->flickerGradThreshold = 26;
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

static void get_flicker_para_by_Custom1_Bin1(FLICKER_CUST_PARA* para)
{
    int freq[9] = {70, 100, 120, 140, 160, 170, 190, 210, 230};
    FLICKER_CUST_STATISTICS EV50_L50 = {-194, 4721, 381, -766};
    FLICKER_CUST_STATISTICS EV50_L60 = {915, 508, 949, -445};
    FLICKER_CUST_STATISTICS EV60_L50 = {1091, 535, 1413, -519};
    FLICKER_CUST_STATISTICS EV60_L60 = {-162, 2898, 247, -642};
    for (int i = 0; i < 9; i++)
        para->flickerFreq[i] = freq[i];
    para->flickerGradThreshold = 24;
    para->flickerSearchRange = 24;
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
    para->freq_feature_index[0] = 2;
    para->freq_feature_index[1] = 1;
}

static void get_flicker_para_by_Custom1_Bin2(FLICKER_CUST_PARA* para)
{
    int freq[9] = {70, 100, 120, 140, 160, 170, 190, 210, 230};
    FLICKER_CUST_STATISTICS EV50_L50 = {-194, 4721, 381, -766};
    FLICKER_CUST_STATISTICS EV50_L60 = {910, 510, 953, -446};
    FLICKER_CUST_STATISTICS EV60_L50 = {1086, 537, 1419, -520};
    FLICKER_CUST_STATISTICS EV60_L60 = {-162, 2898, 247, -642};
    for (int i = 0; i < 9; i++)
        para->flickerFreq[i] = freq[i];
    para->flickerGradThreshold = 26;
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
    para->freq_feature_index[0] = 2;
    para->freq_feature_index[1] = 1;
}

static void get_flicker_para_by_Custom2_Bin1(FLICKER_CUST_PARA* para)
{
    int freq[9] = {70, 100, 120, 140, 160, 170, 190, 210, 230};
    FLICKER_CUST_STATISTICS EV50_L50 = {-194, 4721, 381, -766};
    FLICKER_CUST_STATISTICS EV50_L60 = {915, 508, 949, -445};
    FLICKER_CUST_STATISTICS EV60_L50 = {1091, 535, 1413, -519};
    FLICKER_CUST_STATISTICS EV60_L60 = {-162, 2898, 247, -642};
    for (int i = 0; i < 9; i++)
        para->flickerFreq[i] = freq[i];
    para->flickerGradThreshold = 26;
    para->flickerSearchRange = 24;
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
    para->freq_feature_index[0] = 2;
    para->freq_feature_index[1] = 1;
}

static void get_flicker_para_by_Custom2_Bin2(FLICKER_CUST_PARA* para)
{
    int freq[9] = {70, 100, 120, 140, 160, 170, 190, 210, 230};
    FLICKER_CUST_STATISTICS EV50_L50 = {-194, 4721, 381, -766};
    FLICKER_CUST_STATISTICS EV50_L60 = {910, 510, 953, -446};
    FLICKER_CUST_STATISTICS EV60_L50 = {1086, 537, 1419, -520};
    FLICKER_CUST_STATISTICS EV60_L60 = {-162, 2898, 247, -642};
    for (int i = 0; i < 9; i++)
        para->flickerFreq[i] = freq[i];
    para->flickerGradThreshold = 26;
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
    para->freq_feature_index[0] = 2;
    para->freq_feature_index[1] = 1;
}

static void get_flicker_para_by_Custom3_Bin1(FLICKER_CUST_PARA* para)
{
    int freq[9] = {70, 100, 120, 140, 160, 170, 190, 210, 230};
    FLICKER_CUST_STATISTICS EV50_L50 = {-194, 4721, 381, -766};
    FLICKER_CUST_STATISTICS EV50_L60 = {915, 508, 949, -445};
    FLICKER_CUST_STATISTICS EV60_L50 = {1091, 535, 1413, -519};
    FLICKER_CUST_STATISTICS EV60_L60 = {-162, 2898, 247, -642};
    for (int i = 0; i < 9; i++)
        para->flickerFreq[i] = freq[i];
    para->flickerGradThreshold = 24;
    para->flickerSearchRange = 24;
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
    para->freq_feature_index[0] = 2;
    para->freq_feature_index[1] = 1;
}

static void get_flicker_para_by_Custom3_Bin2(FLICKER_CUST_PARA* para)
{
    int freq[9] = {70, 100, 120, 140, 160, 170, 190, 210, 230};
    FLICKER_CUST_STATISTICS EV50_L50 = {-194, 4721, 381, -766};
    FLICKER_CUST_STATISTICS EV50_L60 = {910, 510, 953, -446};
    FLICKER_CUST_STATISTICS EV60_L50 = {1086, 537, 1419, -520};
    FLICKER_CUST_STATISTICS EV60_L60 = {-162, 2898, 247, -642};
    for (int i = 0; i < 9; i++)
        para->flickerFreq[i] = freq[i];
    para->flickerGradThreshold = 26;
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
    para->freq_feature_index[0] = 2;
    para->freq_feature_index[1] = 1;
}

static void get_flicker_para_by_Custom4_Bin1(FLICKER_CUST_PARA* para)
{
    int freq[9] = {70, 100, 120, 140, 160, 170, 190, 210, 230};
    FLICKER_CUST_STATISTICS EV50_L50 = {-194, 4721, 381, -766};
    FLICKER_CUST_STATISTICS EV50_L60 = {915, 508, 949, -445};
    FLICKER_CUST_STATISTICS EV60_L50 = {1091, 535, 1413, -519};
    FLICKER_CUST_STATISTICS EV60_L60 = {-162, 2898, 247, -642};
    for (int i = 0; i < 9; i++)
        para->flickerFreq[i] = freq[i];
    para->flickerGradThreshold = 25;
    para->flickerSearchRange = 24;
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
    para->freq_feature_index[0] = 2;
    para->freq_feature_index[1] = 1;
}

static void get_flicker_para_by_Custom4_Bin2(FLICKER_CUST_PARA* para)
{
    int freq[9] = {70, 100, 120, 140, 160, 170, 190, 210, 230};
    FLICKER_CUST_STATISTICS EV50_L50 = {-194, 4721, 381, -766};
    FLICKER_CUST_STATISTICS EV50_L60 = {910, 510, 953, -446};
    FLICKER_CUST_STATISTICS EV60_L50 = {1086, 537, 1419, -520};
    FLICKER_CUST_STATISTICS EV60_L60 = {-162, 2898, 247, -642};
    for (int i = 0; i < 9; i++)
        para->flickerFreq[i] = freq[i];
    para->flickerGradThreshold = 26;
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
    para->freq_feature_index[0] = 2;
    para->freq_feature_index[1] = 1;
}

static void get_flicker_para_by_Custom5_Bin1(FLICKER_CUST_PARA* para)
{
    int freq[9] = {70, 100, 120, 140, 160, 170, 190, 210, 230};
    FLICKER_CUST_STATISTICS EV50_L50 = {-194, 4721, 381, -766};
    FLICKER_CUST_STATISTICS EV50_L60 = {915, 508, 949, -445};
    FLICKER_CUST_STATISTICS EV60_L50 = {1091, 535, 1413, -519};
    FLICKER_CUST_STATISTICS EV60_L60 = {-162, 2898, 247, -642};
    for (int i = 0; i < 9; i++)
        para->flickerFreq[i] = freq[i];
    para->flickerGradThreshold = 24;
    para->flickerSearchRange = 24;
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
    para->freq_feature_index[0] = 2;
    para->freq_feature_index[1] = 1;
}

static void get_flicker_para_by_Custom5_Bin2(FLICKER_CUST_PARA* para)
{
    int freq[9] = {70, 100, 120, 140, 160, 170, 190, 210, 230};
    FLICKER_CUST_STATISTICS EV50_L50 = {-194, 4721, 381, -766};
    FLICKER_CUST_STATISTICS EV50_L60 = {910, 510, 953, -446};
    FLICKER_CUST_STATISTICS EV60_L50 = {1086, 537, 1419, -520};
    FLICKER_CUST_STATISTICS EV60_L60 = {-162, 2898, 247, -642};
    for (int i = 0; i < 9; i++)
        para->flickerFreq[i] = freq[i];
    para->flickerGradThreshold = 28;
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
    para->freq_feature_index[0] = 2;
    para->freq_feature_index[1] = 1;
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

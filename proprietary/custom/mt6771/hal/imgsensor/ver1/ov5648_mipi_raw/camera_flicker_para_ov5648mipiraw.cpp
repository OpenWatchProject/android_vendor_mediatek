//[Sensor]
//name = ov5648mipiraw
//bin_ratio = 1,2
//
//[Preview]
//read_freq = 84000000
//pixel_line = 2816
//column_length = 935
//
//[Capture]
//read_freq = 84000000
//pixel_line = 2816
//column_length = 1895
//
//[Video]
//read_freq = 84000000
//pixel_line = 2816
//column_length = 935
//
//[Video1]
//read_freq = 84000000
//pixel_line = 2500
//column_length = 1055
//
//[Video2]
//read_freq = 84000000
//pixel_line = 3728
//column_length = 695
//
//[Custom1]
//read_freq = 84000000
//pixel_line = 2816
//column_length = 935
//
//[Custom2]
//read_freq = 84000000
//pixel_line = 2816
//column_length = 935
//
//[Custom3]
//read_freq = 84000000
//pixel_line = 2816
//column_length = 935
//
//[Custom4]
//read_freq = 84000000
//pixel_line = 2816
//column_length = 935
//
//[Custom5]
//read_freq = 84000000
//pixel_line = 2816
//column_length = 935
//
#define LOG_TAG "ov5648mipiraw"

#include <utils/Log.h>
#include <fcntl.h>
#include <math.h>
#include "camera_custom_nvram.h"
#include "camera_custom_sensor.h"
#include "image_sensor.h"
#include "kd_imgsensor_define.h"
#include "camera_AE_PLineTable_ov5648mipiraw.h"
#include "camera_info_ov5648mipiraw.h"
#include "camera_custom_AEPlinetable.h"
#include "camera_custom_flicker_para.h"
#include <cutils/log.h>


extern "C" fptrFlicker getFlickerParaFunc(MUINT32 sensorType)
{
    if(sensorType==0)
        return NSFeature::RAWSensorInfo<SENSOR_ID, MODULE_INDEX>::getFlickerPara;
    else
        return NSFeature::YUVSensorInfo<SENSOR_ID, MODULE_INDEX>::getNullFlickerPara;
}

static void get_flicker_para_by_Preview_Bin1(FLICKER_CUST_PARA* para)
{
    int freq[9] = {70, 80, 90, 100, 120, 130, 140, 160, 170};
    FLICKER_CUST_STATISTICS EV50_L50 = {-194, 4721, 381, -766};
    FLICKER_CUST_STATISTICS EV50_L60 = {1084, 428, 800, -401};
    FLICKER_CUST_STATISTICS EV60_L50 = {1293, 451, 1192, -476};
    FLICKER_CUST_STATISTICS EV60_L60 = {-162, 2898, 247, -642};
    for (int i = 0; i < 9; i++)
        para->flickerFreq[i] = freq[i];
    para->flickerGradThreshold = 31;
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

static void get_flicker_para_by_Preview_Bin2(FLICKER_CUST_PARA* para)
{
    int freq[9] = {80, 90, 100, 120, 130, 140, 170, 210, 230};
    FLICKER_CUST_STATISTICS EV50_L50 = {-194, 4721, 381, -766};
    FLICKER_CUST_STATISTICS EV50_L60 = {1030, 451, 843, -414};
    FLICKER_CUST_STATISTICS EV60_L50 = {1228, 475, 1255, -489};
    FLICKER_CUST_STATISTICS EV60_L60 = {-162, 2898, 247, -642};
    for (int i = 0; i < 9; i++)
        para->flickerFreq[i] = freq[i];
    para->flickerGradThreshold = 47;
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

static void get_flicker_para_by_Capture_Bin1(FLICKER_CUST_PARA* para)
{
    int freq[9] = {70, 80, 90, 100, 110, 120, 130, 140, 170};
    FLICKER_CUST_STATISTICS EV50_L50 = {-194, 4721, 381, -766};
    FLICKER_CUST_STATISTICS EV50_L60 = {3197, 145, 271, -125};
    FLICKER_CUST_STATISTICS EV60_L50 = {3813, 153, 404, -199};
    FLICKER_CUST_STATISTICS EV60_L60 = {-162, 2898, 247, -642};
    for (int i = 0; i < 9; i++)
        para->flickerFreq[i] = freq[i];
    para->flickerGradThreshold = 30;
    para->flickerSearchRange = 32;
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
    para->freq_feature_index[0] = 5;
    para->freq_feature_index[1] = 3;
}

static void get_flicker_para_by_Capture_Bin2(FLICKER_CUST_PARA* para)
{
    int freq[9] = {70, 80, 90, 100, 110, 120, 130, 140, 170};
    FLICKER_CUST_STATISTICS EV50_L50 = {-194, 4721, 381, -766};
    FLICKER_CUST_STATISTICS EV50_L60 = {2975, 156, 292, -143};
    FLICKER_CUST_STATISTICS EV60_L50 = {3547, 164, 434, -217};
    FLICKER_CUST_STATISTICS EV60_L60 = {-162, 2898, 247, -642};
    for (int i = 0; i < 9; i++)
        para->flickerFreq[i] = freq[i];
    para->flickerGradThreshold = 46;
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
    para->freq_feature_index[0] = 5;
    para->freq_feature_index[1] = 3;
}

static void get_flicker_para_by_Video_Bin1(FLICKER_CUST_PARA* para)
{
    int freq[9] = {70, 80, 90, 100, 120, 130, 140, 160, 170};
    FLICKER_CUST_STATISTICS EV50_L50 = {-194, 4721, 381, -766};
    FLICKER_CUST_STATISTICS EV50_L60 = {1084, 428, 800, -401};
    FLICKER_CUST_STATISTICS EV60_L50 = {1293, 451, 1192, -476};
    FLICKER_CUST_STATISTICS EV60_L60 = {-162, 2898, 247, -642};
    for (int i = 0; i < 9; i++)
        para->flickerFreq[i] = freq[i];
    para->flickerGradThreshold = 31;
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

static void get_flicker_para_by_Video_Bin2(FLICKER_CUST_PARA* para)
{
    int freq[9] = {80, 90, 100, 120, 130, 140, 170, 210, 230};
    FLICKER_CUST_STATISTICS EV50_L50 = {-194, 4721, 381, -766};
    FLICKER_CUST_STATISTICS EV50_L60 = {1030, 451, 843, -414};
    FLICKER_CUST_STATISTICS EV60_L50 = {1228, 475, 1255, -489};
    FLICKER_CUST_STATISTICS EV60_L60 = {-162, 2898, 247, -642};
    for (int i = 0; i < 9; i++)
        para->flickerFreq[i] = freq[i];
    para->flickerGradThreshold = 46;
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

static void get_flicker_para_by_Video1_Bin1(FLICKER_CUST_PARA* para)
{
    int freq[9] = {70, 80, 90, 100, 120, 130, 140, 160, 170};
    FLICKER_CUST_STATISTICS EV50_L50 = {-194, 4721, 381, -766};
    FLICKER_CUST_STATISTICS EV50_L60 = {1092, 425, 795, -399};
    FLICKER_CUST_STATISTICS EV60_L50 = {1302, 448, 1183, -474};
    FLICKER_CUST_STATISTICS EV60_L60 = {-162, 2898, 247, -642};
    for (int i = 0; i < 9; i++)
        para->flickerFreq[i] = freq[i];
    para->flickerGradThreshold = 29;
    para->flickerSearchRange = 16;
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

static void get_flicker_para_by_Video1_Bin2(FLICKER_CUST_PARA* para)
{
    int freq[9] = {80, 90, 100, 120, 130, 140, 170, 210, 230};
    FLICKER_CUST_STATISTICS EV50_L50 = {-194, 4721, 381, -766};
    FLICKER_CUST_STATISTICS EV50_L60 = {1036, 448, 838, -413};
    FLICKER_CUST_STATISTICS EV60_L50 = {1236, 472, 1247, -487};
    FLICKER_CUST_STATISTICS EV60_L60 = {-162, 2898, 247, -642};
    for (int i = 0; i < 9; i++)
        para->flickerFreq[i] = freq[i];
    para->flickerGradThreshold = 43;
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

static void get_flicker_para_by_Video2_Bin1(FLICKER_CUST_PARA* para)
{
    int freq[9] = {80, 90, 100, 120, 130, 140, 160, 170, 230};
    FLICKER_CUST_STATISTICS EV50_L50 = {-194, 4721, 381, -766};
    FLICKER_CUST_STATISTICS EV50_L60 = {1045, 444, 830, -411};
    FLICKER_CUST_STATISTICS EV60_L50 = {1246, 468, 1236, -485};
    FLICKER_CUST_STATISTICS EV60_L60 = {-162, 2898, 247, -642};
    for (int i = 0; i < 9; i++)
        para->flickerFreq[i] = freq[i];
    para->flickerGradThreshold = 36;
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
    FLICKER_CUST_STATISTICS EV50_L60 = {1024, 453, 847, -416};
    FLICKER_CUST_STATISTICS EV60_L50 = {1222, 478, 1261, -490};
    FLICKER_CUST_STATISTICS EV60_L60 = {-162, 2898, 247, -642};
    for (int i = 0; i < 9; i++)
        para->flickerFreq[i] = freq[i];
    para->flickerGradThreshold = 57;
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
    int freq[9] = {70, 80, 90, 100, 120, 130, 140, 160, 170};
    FLICKER_CUST_STATISTICS EV50_L50 = {-194, 4721, 381, -766};
    FLICKER_CUST_STATISTICS EV50_L60 = {1084, 428, 800, -401};
    FLICKER_CUST_STATISTICS EV60_L50 = {1293, 451, 1192, -476};
    FLICKER_CUST_STATISTICS EV60_L60 = {-162, 2898, 247, -642};
    for (int i = 0; i < 9; i++)
        para->flickerFreq[i] = freq[i];
    para->flickerGradThreshold = 30;
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

static void get_flicker_para_by_Custom1_Bin2(FLICKER_CUST_PARA* para)
{
    int freq[9] = {80, 90, 100, 120, 130, 140, 170, 210, 230};
    FLICKER_CUST_STATISTICS EV50_L50 = {-194, 4721, 381, -766};
    FLICKER_CUST_STATISTICS EV50_L60 = {1030, 451, 843, -414};
    FLICKER_CUST_STATISTICS EV60_L50 = {1228, 475, 1255, -489};
    FLICKER_CUST_STATISTICS EV60_L60 = {-162, 2898, 247, -642};
    for (int i = 0; i < 9; i++)
        para->flickerFreq[i] = freq[i];
    para->flickerGradThreshold = 47;
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

static void get_flicker_para_by_Custom2_Bin1(FLICKER_CUST_PARA* para)
{
    int freq[9] = {70, 80, 90, 100, 120, 130, 140, 160, 170};
    FLICKER_CUST_STATISTICS EV50_L50 = {-194, 4721, 381, -766};
    FLICKER_CUST_STATISTICS EV50_L60 = {1084, 428, 800, -401};
    FLICKER_CUST_STATISTICS EV60_L50 = {1293, 451, 1192, -476};
    FLICKER_CUST_STATISTICS EV60_L60 = {-162, 2898, 247, -642};
    for (int i = 0; i < 9; i++)
        para->flickerFreq[i] = freq[i];
    para->flickerGradThreshold = 30;
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

static void get_flicker_para_by_Custom2_Bin2(FLICKER_CUST_PARA* para)
{
    int freq[9] = {80, 90, 100, 120, 130, 140, 170, 210, 230};
    FLICKER_CUST_STATISTICS EV50_L50 = {-194, 4721, 381, -766};
    FLICKER_CUST_STATISTICS EV50_L60 = {1030, 451, 843, -414};
    FLICKER_CUST_STATISTICS EV60_L50 = {1228, 475, 1255, -489};
    FLICKER_CUST_STATISTICS EV60_L60 = {-162, 2898, 247, -642};
    for (int i = 0; i < 9; i++)
        para->flickerFreq[i] = freq[i];
    para->flickerGradThreshold = 46;
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

static void get_flicker_para_by_Custom3_Bin1(FLICKER_CUST_PARA* para)
{
    int freq[9] = {70, 80, 90, 100, 120, 130, 140, 160, 170};
    FLICKER_CUST_STATISTICS EV50_L50 = {-194, 4721, 381, -766};
    FLICKER_CUST_STATISTICS EV50_L60 = {1084, 428, 800, -401};
    FLICKER_CUST_STATISTICS EV60_L50 = {1293, 451, 1192, -476};
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

static void get_flicker_para_by_Custom3_Bin2(FLICKER_CUST_PARA* para)
{
    int freq[9] = {80, 90, 100, 120, 130, 140, 170, 210, 230};
    FLICKER_CUST_STATISTICS EV50_L50 = {-194, 4721, 381, -766};
    FLICKER_CUST_STATISTICS EV50_L60 = {1030, 451, 843, -414};
    FLICKER_CUST_STATISTICS EV60_L50 = {1228, 475, 1255, -489};
    FLICKER_CUST_STATISTICS EV60_L60 = {-162, 2898, 247, -642};
    for (int i = 0; i < 9; i++)
        para->flickerFreq[i] = freq[i];
    para->flickerGradThreshold = 46;
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

static void get_flicker_para_by_Custom4_Bin1(FLICKER_CUST_PARA* para)
{
    int freq[9] = {70, 80, 90, 100, 120, 130, 140, 160, 170};
    FLICKER_CUST_STATISTICS EV50_L50 = {-194, 4721, 381, -766};
    FLICKER_CUST_STATISTICS EV50_L60 = {1084, 428, 800, -401};
    FLICKER_CUST_STATISTICS EV60_L50 = {1293, 451, 1192, -476};
    FLICKER_CUST_STATISTICS EV60_L60 = {-162, 2898, 247, -642};
    for (int i = 0; i < 9; i++)
        para->flickerFreq[i] = freq[i];
    para->flickerGradThreshold = 30;
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

static void get_flicker_para_by_Custom4_Bin2(FLICKER_CUST_PARA* para)
{
    int freq[9] = {80, 90, 100, 120, 130, 140, 170, 210, 230};
    FLICKER_CUST_STATISTICS EV50_L50 = {-194, 4721, 381, -766};
    FLICKER_CUST_STATISTICS EV50_L60 = {1030, 451, 843, -414};
    FLICKER_CUST_STATISTICS EV60_L50 = {1228, 475, 1255, -489};
    FLICKER_CUST_STATISTICS EV60_L60 = {-162, 2898, 247, -642};
    for (int i = 0; i < 9; i++)
        para->flickerFreq[i] = freq[i];
    para->flickerGradThreshold = 47;
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

static void get_flicker_para_by_Custom5_Bin1(FLICKER_CUST_PARA* para)
{
    int freq[9] = {70, 80, 90, 100, 120, 130, 140, 160, 170};
    FLICKER_CUST_STATISTICS EV50_L50 = {-194, 4721, 381, -766};
    FLICKER_CUST_STATISTICS EV50_L60 = {1084, 428, 800, -401};
    FLICKER_CUST_STATISTICS EV60_L50 = {1293, 451, 1192, -476};
    FLICKER_CUST_STATISTICS EV60_L60 = {-162, 2898, 247, -642};
    for (int i = 0; i < 9; i++)
        para->flickerFreq[i] = freq[i];
    para->flickerGradThreshold = 30;
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

static void get_flicker_para_by_Custom5_Bin2(FLICKER_CUST_PARA* para)
{
    int freq[9] = {80, 90, 100, 120, 130, 140, 170, 210, 230};
    FLICKER_CUST_STATISTICS EV50_L50 = {-194, 4721, 381, -766};
    FLICKER_CUST_STATISTICS EV50_L60 = {1030, 451, 843, -414};
    FLICKER_CUST_STATISTICS EV60_L50 = {1228, 475, 1255, -489};
    FLICKER_CUST_STATISTICS EV60_L60 = {-162, 2898, 247, -642};
    for (int i = 0; i < 9; i++)
        para->flickerFreq[i] = freq[i];
    para->flickerGradThreshold = 47;
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


typedef NSFeature::RAWSensorInfo<SENSOR_ID, MODULE_INDEX> SensorInfoSingleton_T;
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

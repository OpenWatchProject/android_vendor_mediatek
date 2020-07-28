//[Sensor]
//name = gc8c34mipiraw
//bin_ratio = 1,2
//
//[Preview]
//read_freq = 80000000
//pixel_line = 2116
//column_length = 1262
//
//[Video]
//read_freq = 80000000
//pixel_line = 2116
//column_length = 1262
//
//[Capture]
//read_freq = 80000000
//pixel_line = 2116
//column_length = 1262
//
//[Video1]
//read_freq = 80000000
//pixel_line = 2116
//column_length = 1262
//
//[Video2]
//read_freq = 80000000
//pixel_line = 2116
//column_length = 1262
//
#define LOG_TAG "gc8c34mipiraw"

#include <utils/Log.h>
#include <fcntl.h>
#include <math.h>
#include "camera_custom_nvram.h"
#include "camera_custom_sensor.h"
#include "image_sensor.h"
#include "kd_imgsensor_define.h"
#include "camera_AE_PLineTable_gc8c34mipiraw.h"
#include "camera_info_gc8c34mipiraw.h"
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
  int freq[9] = { 70, 90, 100, 110, 120, 140, 160, 190, 230};
  FLICKER_CUST_STATISTICS EV50_L50 = {-194, 4721, 381, -766};
  FLICKER_CUST_STATISTICS EV50_L60 = {884, 525, 982, -453};
  FLICKER_CUST_STATISTICS EV60_L50 = {1055, 553, 1461, -528};
  FLICKER_CUST_STATISTICS EV60_L60 = {-162, 2898, 247, -642};
    for (int i = 0; i < 9; i++)
        para->flickerFreq[i] = freq[i];
  para->flickerGradThreshold=28;
  para->flickerSearchRange=20;
  para->minPastFrames=3;
  para->maxPastFrames=14;
  para->EV50_L50=EV50_L50;
  para->EV50_L60=EV50_L60;
  para->EV60_L50=EV60_L50;
  para->EV60_L60=EV60_L60;
  para->EV50_thresholds[0]=-30;
  para->EV50_thresholds[1]=12;
  para->EV60_thresholds[0]=-30;
  para->EV60_thresholds[1]=12;
  para->freq_feature_index[0]=4;
  para->freq_feature_index[1]=2;
}

static void get_flicker_para_by_Preview_Bin2(FLICKER_CUST_PARA* para)
{
  int freq[9] = { 70, 90, 100, 110, 120, 140, 160, 190, 230};
  FLICKER_CUST_STATISTICS EV50_L50 = {-194, 4721, 381, -766};
  FLICKER_CUST_STATISTICS EV50_L60 = {884, 525, 982, -453};
  FLICKER_CUST_STATISTICS EV60_L50 = {1055, 553, 1461, -528};
  FLICKER_CUST_STATISTICS EV60_L60 = {-162, 2898, 247, -642};
    for (int i = 0; i < 9; i++)
        para->flickerFreq[i] = freq[i];
  para->flickerGradThreshold=28;
  para->flickerSearchRange=20;
  para->minPastFrames=3;
  para->maxPastFrames=14;
  para->EV50_L50=EV50_L50;
  para->EV50_L60=EV50_L60;
  para->EV60_L50=EV60_L50;
  para->EV60_L60=EV60_L60;
  para->EV50_thresholds[0]=-30;
  para->EV50_thresholds[1]=12;
  para->EV60_thresholds[0]=-30;
  para->EV60_thresholds[1]=12;
  para->freq_feature_index[0]=4;
  para->freq_feature_index[1]=2;
}

static void get_flicker_para_by_Capture_Bin1(FLICKER_CUST_PARA* para)
{
  int freq[9] =  { 70, 90, 100, 110, 120, 140, 160, 190, 230};
  FLICKER_CUST_STATISTICS EV50_L50 = {-194, 4721, 381, -766};
  FLICKER_CUST_STATISTICS EV50_L60 = {884, 525, 982, -453};
  FLICKER_CUST_STATISTICS EV60_L50 = {1055, 553, 1461, -528};
  FLICKER_CUST_STATISTICS EV60_L60 = {-162, 2898, 247, -642};
    for (int i = 0; i < 9; i++)
        para->flickerFreq[i] = freq[i];
  para->flickerGradThreshold=27;
  para->flickerSearchRange=20;
  para->minPastFrames=3;
  para->maxPastFrames=14;
  para->EV50_L50=EV50_L50;
  para->EV50_L60=EV50_L60;
  para->EV60_L50=EV60_L50;
  para->EV60_L60=EV60_L60;
  para->EV50_thresholds[0]=-30;
  para->EV50_thresholds[1]=12;
  para->EV60_thresholds[0]=-30;
  para->EV60_thresholds[1]=12;
  para->freq_feature_index[0]=4;
  para->freq_feature_index[1]=2;
}

static void get_flicker_para_by_Capture_Bin2(FLICKER_CUST_PARA* para)
{
  int freq[9] =  { 70, 90, 100, 110, 120, 140, 160, 190, 230};
  FLICKER_CUST_STATISTICS EV50_L50 = {-194, 4721, 381, -766};
  FLICKER_CUST_STATISTICS EV50_L60 = {884, 525, 982, -453};
  FLICKER_CUST_STATISTICS EV60_L50 = {1055, 553, 1461, -528};
  FLICKER_CUST_STATISTICS EV60_L60 = {-162, 2898, 247, -642};
    for (int i = 0; i < 9; i++)
        para->flickerFreq[i] = freq[i];
  para->flickerGradThreshold=27;
  para->flickerSearchRange=20;
  para->minPastFrames=3;
  para->maxPastFrames=14;
  para->EV50_L50=EV50_L50;
  para->EV50_L60=EV50_L60;
  para->EV60_L50=EV60_L50;
  para->EV60_L60=EV60_L60;
  para->EV50_thresholds[0]=-30;
  para->EV50_thresholds[1]=12;
  para->EV60_thresholds[0]=-30;
  para->EV60_thresholds[1]=12;
  para->freq_feature_index[0]=4;
  para->freq_feature_index[1]=2;
}

static void get_flicker_para_by_Video_Bin1(FLICKER_CUST_PARA* para)
{
  int freq[9] =  { 70, 90, 100, 110, 120, 140, 160, 190, 230};
  FLICKER_CUST_STATISTICS EV50_L50 = {-194, 4721, 381, -766};
  FLICKER_CUST_STATISTICS EV50_L60 = {884, 525, 982, -453};
  FLICKER_CUST_STATISTICS EV60_L50 = {1055, 553, 1461, -528};
  FLICKER_CUST_STATISTICS EV60_L60 = {-162, 2898, 247, -642};
    for (int i = 0; i < 9; i++)
        para->flickerFreq[i] = freq[i];
  para->flickerGradThreshold=28;
  para->flickerSearchRange=20;
  para->minPastFrames=3;
  para->maxPastFrames=14;
  para->EV50_L50=EV50_L50;
  para->EV50_L60=EV50_L60;
  para->EV60_L50=EV60_L50;
  para->EV60_L60=EV60_L60;
  para->EV50_thresholds[0]=-30;
  para->EV50_thresholds[1]=12;
  para->EV60_thresholds[0]=-30;
  para->EV60_thresholds[1]=12;
  para->freq_feature_index[0]=4;
  para->freq_feature_index[1]=2;
}

static void get_flicker_para_by_Video_Bin2(FLICKER_CUST_PARA* para)
{
  int freq[9] =  { 70, 90, 100, 110, 120, 140, 160, 190, 230};
  FLICKER_CUST_STATISTICS EV50_L50 = {-194, 4721, 381, -766};
  FLICKER_CUST_STATISTICS EV50_L60 = {884, 525, 982, -453};
  FLICKER_CUST_STATISTICS EV60_L50 = {1055, 553, 1461, -528};
  FLICKER_CUST_STATISTICS EV60_L60 = {-162, 2898, 247, -642};
    for (int i = 0; i < 9; i++)
        para->flickerFreq[i] = freq[i];
  para->flickerGradThreshold=28;
  para->flickerSearchRange=20;
  para->minPastFrames=3;
  para->maxPastFrames=14;
  para->EV50_L50=EV50_L50;
  para->EV50_L60=EV50_L60;
  para->EV60_L50=EV60_L50;
  para->EV60_L60=EV60_L60;
  para->EV50_thresholds[0]=-30;
  para->EV50_thresholds[1]=12;
  para->EV60_thresholds[0]=-30;
  para->EV60_thresholds[1]=12;
  para->freq_feature_index[0]=4;
  para->freq_feature_index[1]=2;
}

static void get_flicker_para_by_Video1_Bin1(FLICKER_CUST_PARA* para)
{
  int freq[9] =  { 70, 90, 100, 110, 120, 140, 160, 190, 230};
  FLICKER_CUST_STATISTICS EV50_L50 = {-194, 4721, 381, -766};
  FLICKER_CUST_STATISTICS EV50_L60 = {884, 525, 982, -453};
  FLICKER_CUST_STATISTICS EV60_L50 = {1055, 553, 1461, -528};
  FLICKER_CUST_STATISTICS EV60_L60 = {-162, 2898, 247, -642};
    for (int i = 0; i < 9; i++)
        para->flickerFreq[i] = freq[i];
  para->flickerGradThreshold=28;
  para->flickerSearchRange=20;
  para->minPastFrames=3;
  para->maxPastFrames=14;
  para->EV50_L50=EV50_L50;
  para->EV50_L60=EV50_L60;
  para->EV60_L50=EV60_L50;
  para->EV60_L60=EV60_L60;
  para->EV50_thresholds[0]=-30;
  para->EV50_thresholds[1]=12;
  para->EV60_thresholds[0]=-30;
  para->EV60_thresholds[1]=12;
  para->freq_feature_index[0]=4;
  para->freq_feature_index[1]=2;
}

static void get_flicker_para_by_Video1_Bin2(FLICKER_CUST_PARA* para)
{
  int freq[9] =  { 70, 90, 100, 110, 120, 140, 160, 190, 230};
  FLICKER_CUST_STATISTICS EV50_L50 = {-194, 4721, 381, -766};
  FLICKER_CUST_STATISTICS EV50_L60 = {884, 525, 982, -453};
  FLICKER_CUST_STATISTICS EV60_L50 = {1055, 553, 1461, -528};
  FLICKER_CUST_STATISTICS EV60_L60 = {-162, 2898, 247, -642};
    for (int i = 0; i < 9; i++)
        para->flickerFreq[i] = freq[i];
  para->flickerGradThreshold=28;
  para->flickerSearchRange=20;
  para->minPastFrames=3;
  para->maxPastFrames=14;
  para->EV50_L50=EV50_L50;
  para->EV50_L60=EV50_L60;
  para->EV60_L50=EV60_L50;
  para->EV60_L60=EV60_L60;
  para->EV50_thresholds[0]=-30;
  para->EV50_thresholds[1]=12;
  para->EV60_thresholds[0]=-30;
  para->EV60_thresholds[1]=12;
  para->freq_feature_index[0]=4;
  para->freq_feature_index[1]=2;
}

static void get_flicker_para_by_Video2_Bin1(FLICKER_CUST_PARA* para)
{
  int freq[9] =  { 70, 90, 100, 110, 120, 140, 160, 190, 230};
  FLICKER_CUST_STATISTICS EV50_L50 = {-194, 4721, 381, -766};
  FLICKER_CUST_STATISTICS EV50_L60 = {884, 525, 982, -453};
  FLICKER_CUST_STATISTICS EV60_L50 = {1055, 553, 1461, -528};
  FLICKER_CUST_STATISTICS EV60_L60 = {-162, 2898, 247, -642};
    for (int i = 0; i < 9; i++)
        para->flickerFreq[i] = freq[i];
  para->flickerGradThreshold=27;
  para->flickerSearchRange=20;
  para->minPastFrames=3;
  para->maxPastFrames=14;
  para->EV50_L50=EV50_L50;
  para->EV50_L60=EV50_L60;
  para->EV60_L50=EV60_L50;
  para->EV60_L60=EV60_L60;
  para->EV50_thresholds[0]=-30;
  para->EV50_thresholds[1]=12;
  para->EV60_thresholds[0]=-30;
  para->EV60_thresholds[1]=12;
  para->freq_feature_index[0]=4;
  para->freq_feature_index[1]=2;
}

static void get_flicker_para_by_Video2_Bin2(FLICKER_CUST_PARA* para)
{
  int freq[9] =  { 70, 90, 100, 110, 120, 140, 160, 190, 230};
  FLICKER_CUST_STATISTICS EV50_L50 = {-194, 4721, 381, -766};
  FLICKER_CUST_STATISTICS EV50_L60 = {884, 525, 982, -453};
  FLICKER_CUST_STATISTICS EV60_L50 = {1055, 553, 1461, -528};
  FLICKER_CUST_STATISTICS EV60_L60 = {-162, 2898, 247, -642};
    for (int i = 0; i < 9; i++)
        para->flickerFreq[i] = freq[i];
  para->flickerGradThreshold=27;
  para->flickerSearchRange=20;
  para->minPastFrames=3;
  para->maxPastFrames=14;
  para->EV50_L50=EV50_L50;
  para->EV50_L60=EV50_L60;
  para->EV60_L50=EV60_L50;
  para->EV60_L60=EV60_L60;
  para->EV50_thresholds[0]=-30;
  para->EV50_thresholds[1]=12;
  para->EV60_thresholds[0]=-30;
  para->EV60_thresholds[1]=12;
  para->freq_feature_index[0]=4;
  para->freq_feature_index[1]=2;
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
  int freq[9] =  { 70, 90, 100, 110, 120, 140, 160, 190, 230};
  FLICKER_CUST_STATISTICS EV50_L50 = {-194, 4721, 381, -766};
  FLICKER_CUST_STATISTICS EV50_L60 = {884, 525, 982, -453};
  FLICKER_CUST_STATISTICS EV60_L50 = {1055, 553, 1461, -528};
  FLICKER_CUST_STATISTICS EV60_L60 = {-162, 2898, 247, -642};
    for (int i = 0; i < 9; i++)
        para->flickerFreq[i] = freq[i];
  para->flickerGradThreshold=27;
  para->flickerSearchRange=20;
  para->minPastFrames=3;
  para->maxPastFrames=14;
  para->EV50_L50=EV50_L50;
  para->EV50_L60=EV50_L60;
  para->EV60_L50=EV60_L50;
  para->EV60_L60=EV60_L60;
  para->EV50_thresholds[0]=-30;
  para->EV50_thresholds[1]=12;
  para->EV60_thresholds[0]=-30;
  para->EV60_thresholds[1]=12;
  para->freq_feature_index[0]=4;
  para->freq_feature_index[1]=2;
}

// using [Preview_Bin1] parameters for [Custom2_Bin1]
static void get_flicker_para_by_Custom2_Bin1(FLICKER_CUST_PARA* para)
{
  int freq[9] =  { 70, 90, 100, 110, 120, 140, 160, 190, 230};
  FLICKER_CUST_STATISTICS EV50_L50 = {-194, 4721, 381, -766};
  FLICKER_CUST_STATISTICS EV50_L60 = {884, 525, 982, -453};
  FLICKER_CUST_STATISTICS EV60_L50 = {1055, 553, 1461, -528};
  FLICKER_CUST_STATISTICS EV60_L60 = {-162, 2898, 247, -642};
    for (int i = 0; i < 9; i++)
        para->flickerFreq[i] = freq[i];
  para->flickerGradThreshold=27;
  para->flickerSearchRange=20;
  para->minPastFrames=3;
  para->maxPastFrames=14;
  para->EV50_L50=EV50_L50;
  para->EV50_L60=EV50_L60;
  para->EV60_L50=EV60_L50;
  para->EV60_L60=EV60_L60;
  para->EV50_thresholds[0]=-30;
  para->EV50_thresholds[1]=12;
  para->EV60_thresholds[0]=-30;
  para->EV60_thresholds[1]=12;
  para->freq_feature_index[0]=4;
  para->freq_feature_index[1]=2;
}

// using [Preview_Bin1] parameters for [Custom2_Bin2]
static void get_flicker_para_by_Custom2_Bin2(FLICKER_CUST_PARA* para)
{
  int freq[9] =  { 70, 90, 100, 110, 120, 140, 160, 190, 230};
  FLICKER_CUST_STATISTICS EV50_L50 = {-194, 4721, 381, -766};
  FLICKER_CUST_STATISTICS EV50_L60 = {884, 525, 982, -453};
  FLICKER_CUST_STATISTICS EV60_L50 = {1055, 553, 1461, -528};
  FLICKER_CUST_STATISTICS EV60_L60 = {-162, 2898, 247, -642};
    for (int i = 0; i < 9; i++)
        para->flickerFreq[i] = freq[i];
  para->flickerGradThreshold=27;
  para->flickerSearchRange=20;
  para->minPastFrames=3;
  para->maxPastFrames=14;
  para->EV50_L50=EV50_L50;
  para->EV50_L60=EV50_L60;
  para->EV60_L50=EV60_L50;
  para->EV60_L60=EV60_L60;
  para->EV50_thresholds[0]=-30;
  para->EV50_thresholds[1]=12;
  para->EV60_thresholds[0]=-30;
  para->EV60_thresholds[1]=12;
  para->freq_feature_index[0]=4;
  para->freq_feature_index[1]=2;
}

// using [Preview_Bin1] parameters for [Custom3_Bin1]
static void get_flicker_para_by_Custom3_Bin1(FLICKER_CUST_PARA* para)
{
  int freq[9] =  { 70, 90, 100, 110, 120, 140, 160, 190, 230};
  FLICKER_CUST_STATISTICS EV50_L50 = {-194, 4721, 381, -766};
  FLICKER_CUST_STATISTICS EV50_L60 = {884, 525, 982, -453};
  FLICKER_CUST_STATISTICS EV60_L50 = {1055, 553, 1461, -528};
  FLICKER_CUST_STATISTICS EV60_L60 = {-162, 2898, 247, -642};
    for (int i = 0; i < 9; i++)
        para->flickerFreq[i] = freq[i];
  para->flickerGradThreshold=27;
  para->flickerSearchRange=20;
  para->minPastFrames=3;
  para->maxPastFrames=14;
  para->EV50_L50=EV50_L50;
  para->EV50_L60=EV50_L60;
  para->EV60_L50=EV60_L50;
  para->EV60_L60=EV60_L60;
  para->EV50_thresholds[0]=-30;
  para->EV50_thresholds[1]=12;
  para->EV60_thresholds[0]=-30;
  para->EV60_thresholds[1]=12;
  para->freq_feature_index[0]=4;
  para->freq_feature_index[1]=2;
}

// using [Preview_Bin1] parameters for [Custom3_Bin2]
static void get_flicker_para_by_Custom3_Bin2(FLICKER_CUST_PARA* para)
{
  int freq[9] =  { 70, 90, 100, 110, 120, 140, 160, 190, 230};
  FLICKER_CUST_STATISTICS EV50_L50 = {-194, 4721, 381, -766};
  FLICKER_CUST_STATISTICS EV50_L60 = {884, 525, 982, -453};
  FLICKER_CUST_STATISTICS EV60_L50 = {1055, 553, 1461, -528};
  FLICKER_CUST_STATISTICS EV60_L60 = {-162, 2898, 247, -642};
    for (int i = 0; i < 9; i++)
        para->flickerFreq[i] = freq[i];
  para->flickerGradThreshold=27;
  para->flickerSearchRange=20;
  para->minPastFrames=3;
  para->maxPastFrames=14;
  para->EV50_L50=EV50_L50;
  para->EV50_L60=EV50_L60;
  para->EV60_L50=EV60_L50;
  para->EV60_L60=EV60_L60;
  para->EV50_thresholds[0]=-30;
  para->EV50_thresholds[1]=12;
  para->EV60_thresholds[0]=-30;
  para->EV60_thresholds[1]=12;
  para->freq_feature_index[0]=4;
  para->freq_feature_index[1]=2;
}

// using [Preview_Bin1] parameters for [Custom4_Bin1]
static void get_flicker_para_by_Custom4_Bin1(FLICKER_CUST_PARA* para)
{
  int freq[9] =  { 70, 90, 100, 110, 120, 140, 160, 190, 230};
  FLICKER_CUST_STATISTICS EV50_L50 = {-194, 4721, 381, -766};
  FLICKER_CUST_STATISTICS EV50_L60 = {884, 525, 982, -453};
  FLICKER_CUST_STATISTICS EV60_L50 = {1055, 553, 1461, -528};
  FLICKER_CUST_STATISTICS EV60_L60 = {-162, 2898, 247, -642};
    for (int i = 0; i < 9; i++)
        para->flickerFreq[i] = freq[i];
  para->flickerGradThreshold=27;
  para->flickerSearchRange=20;
  para->minPastFrames=3;
  para->maxPastFrames=14;
  para->EV50_L50=EV50_L50;
  para->EV50_L60=EV50_L60;
  para->EV60_L50=EV60_L50;
  para->EV60_L60=EV60_L60;
  para->EV50_thresholds[0]=-30;
  para->EV50_thresholds[1]=12;
  para->EV60_thresholds[0]=-30;
  para->EV60_thresholds[1]=12;
  para->freq_feature_index[0]=4;
  para->freq_feature_index[1]=2;
}

// using [Preview_Bin1] parameters for [Custom4_Bin2]
static void get_flicker_para_by_Custom4_Bin2(FLICKER_CUST_PARA* para)
{
  int freq[9] =  { 70, 90, 100, 110, 120, 140, 160, 190, 230};
  FLICKER_CUST_STATISTICS EV50_L50 = {-194, 4721, 381, -766};
  FLICKER_CUST_STATISTICS EV50_L60 = {884, 525, 982, -453};
  FLICKER_CUST_STATISTICS EV60_L50 = {1055, 553, 1461, -528};
  FLICKER_CUST_STATISTICS EV60_L60 = {-162, 2898, 247, -642};
    for (int i = 0; i < 9; i++)
        para->flickerFreq[i] = freq[i];
  para->flickerGradThreshold=27;
  para->flickerSearchRange=20;
  para->minPastFrames=3;
  para->maxPastFrames=14;
  para->EV50_L50=EV50_L50;
  para->EV50_L60=EV50_L60;
  para->EV60_L50=EV60_L50;
  para->EV60_L60=EV60_L60;
  para->EV50_thresholds[0]=-30;
  para->EV50_thresholds[1]=12;
  para->EV60_thresholds[0]=-30;
  para->EV60_thresholds[1]=12;
  para->freq_feature_index[0]=4;
  para->freq_feature_index[1]=2;
}

// using [Preview_Bin1] parameters for [Custom5_Bin1]
static void get_flicker_para_by_Custom5_Bin1(FLICKER_CUST_PARA* para)
{
  int freq[9] =  { 70, 90, 100, 110, 120, 140, 160, 190, 230};
  FLICKER_CUST_STATISTICS EV50_L50 = {-194, 4721, 381, -766};
  FLICKER_CUST_STATISTICS EV50_L60 = {884, 525, 982, -453};
  FLICKER_CUST_STATISTICS EV60_L50 = {1055, 553, 1461, -528};
  FLICKER_CUST_STATISTICS EV60_L60 = {-162, 2898, 247, -642};
    for (int i = 0; i < 9; i++)
        para->flickerFreq[i] = freq[i];
  para->flickerGradThreshold=27;
  para->flickerSearchRange=20;
  para->minPastFrames=3;
  para->maxPastFrames=14;
  para->EV50_L50=EV50_L50;
  para->EV50_L60=EV50_L60;
  para->EV60_L50=EV60_L50;
  para->EV60_L60=EV60_L60;
  para->EV50_thresholds[0]=-30;
  para->EV50_thresholds[1]=12;
  para->EV60_thresholds[0]=-30;
  para->EV60_thresholds[1]=12;
  para->freq_feature_index[0]=4;
  para->freq_feature_index[1]=2;
}

// using [Preview_Bin1] parameters for [Custom5_Bin2]
static void get_flicker_para_by_Custom5_Bin2(FLICKER_CUST_PARA* para)
{
  int freq[9] =  { 70, 90, 100, 110, 120, 140, 160, 190, 230};
  FLICKER_CUST_STATISTICS EV50_L50 = {-194, 4721, 381, -766};
  FLICKER_CUST_STATISTICS EV50_L60 = {884, 525, 982, -453};
  FLICKER_CUST_STATISTICS EV60_L50 = {1055, 553, 1461, -528};
  FLICKER_CUST_STATISTICS EV60_L60 = {-162, 2898, 247, -642};
    for (int i = 0; i < 9; i++)
        para->flickerFreq[i] = freq[i];
  para->flickerGradThreshold=27;
  para->flickerSearchRange=20;
  para->minPastFrames=3;
  para->maxPastFrames=14;
  para->EV50_L50=EV50_L50;
  para->EV50_L60=EV50_L60;
  para->EV60_L50=EV60_L50;
  para->EV60_L60=EV60_L60;
  para->EV50_thresholds[0]=-30;
  para->EV50_thresholds[1]=12;
  para->EV60_thresholds[0]=-30;
  para->EV60_thresholds[1]=12;
  para->freq_feature_index[0]=4;
  para->freq_feature_index[1]=2;
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

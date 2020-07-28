//[Sensor]
//name = s5k4e6mipiraw
//
//[Preview]
//read_freq = 180000000
//pixel_line = 5650
//column_length = 955
//
//[Video]
//read_freq = 180000000
//pixel_line = 2920
//column_length = 1935
//
//[Capture]
//read_freq = 180000000
//pixel_line = 2920
//column_length = 1935
//
//[Video1]
//read_freq = 180000000
//pixel_line = 2600
//column_length = 463
//
//[Video2]
//read_freq = 180000000
//pixel_line = 5650
//column_length = 695
#include <utils/Log.h>
#include <fcntl.h>
#include <math.h>

#include "camera_custom_nvram.h"
#include "camera_custom_sensor.h"
#include "image_sensor.h"
#include "kd_imgsensor_define.h"
#include "camera_AE_PLineTable_s5k4e6mipiraw.h"
#include "camera_info_s5k4e6mipiraw.h"
#include <custom/aaa/AEPlinetable.h>
//#include "camera_custom_flicker_table.h"
#include "camera_custom_flicker_para.h"
#include <cutils/log.h>
#include <string.h>


static void get_flicker_para_by_Preview(FLICKER_CUST_PARA* para)
{
  int i;
  int freq[9] = { 80, 90, 100, 120, 130, 140, 170, 210, 230};
  FLICKER_CUST_STATISTICS EV50_L50 = {-194, 4721, 381, -766};
  FLICKER_CUST_STATISTICS EV50_L60 = {1023, 454, 849, -416};
  FLICKER_CUST_STATISTICS EV60_L50 = {1219, 478, 1264, -491};
  FLICKER_CUST_STATISTICS EV60_L60 = {-162, 2898, 247, -642};
  for(i=0;i<9;i++)
  {
    para->flickerFreq[i]=freq[i];
  }
  para->flickerGradThreshold=28;
  para->flickerSearchRange=16;
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
  para->freq_feature_index[0]=3;
  para->freq_feature_index[1]=2;
}

static void get_flicker_para_by_Video(FLICKER_CUST_PARA* para)
{
  int i;
  int freq[9] =  { 70, 80, 90, 100, 120, 130, 140, 160, 170};
  FLICKER_CUST_STATISTICS EV50_L50 = {-194, 4721, 381, -766};
  FLICKER_CUST_STATISTICS EV50_L60 = {1092, 425, 795, -400};
  FLICKER_CUST_STATISTICS EV60_L50 = {1302, 448, 1184, -474};
  FLICKER_CUST_STATISTICS EV60_L60 = {-162, 2898, 247, -642};

  for(i=0;i<9;i++)
  {
    para->flickerFreq[i]=freq[i];
  }
  para->flickerGradThreshold=25;
  para->flickerSearchRange=32;
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
  para->freq_feature_index[1]=3;
}

static void get_flicker_para_by_Capture(FLICKER_CUST_PARA* para)
{
  int i;
  int freq[9] =  { 70, 80, 90, 100, 120, 130, 140, 160, 170};
  FLICKER_CUST_STATISTICS EV50_L50 = {-194, 4721, 381, -766};
  FLICKER_CUST_STATISTICS EV50_L60 = {1092, 425, 795, -400};
  FLICKER_CUST_STATISTICS EV60_L50 = {1302, 448, 1184, -474};
  FLICKER_CUST_STATISTICS EV60_L60 = {-162, 2898, 247, -642};

  for(i=0;i<9;i++)
  {
    para->flickerFreq[i]=freq[i];
  }
  para->flickerGradThreshold=24;
  para->flickerSearchRange=32;
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
  para->freq_feature_index[1]=3;
}

static void get_flicker_para_by_Video1(FLICKER_CUST_PARA* para)
{
  int i;
  int freq[9] =  { 70, 80, 100, 120, 160, 170, 190, 210, 230};
  FLICKER_CUST_STATISTICS EV50_L50 = {-194, 4721, 381, -766};
  FLICKER_CUST_STATISTICS EV50_L60 = {175, 2647, 4944, -867};
  FLICKER_CUST_STATISTICS EV60_L50 = {209, 2787, 7359, -942};
  FLICKER_CUST_STATISTICS EV60_L60 = {-162, 2898, 247, -642};

  for(i=0;i<9;i++)
  {
    para->flickerFreq[i]=freq[i];
  }
  para->flickerGradThreshold=23;
  para->flickerSearchRange=4;
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
  para->freq_feature_index[0]=3;
  para->freq_feature_index[1]=2;
}

static void get_flicker_para_by_Video2(FLICKER_CUST_PARA* para)
{
  int i;
  int freq[9] =  { 70, 80, 100, 120, 140, 160, 170, 190, 230};
  FLICKER_CUST_STATISTICS EV50_L50 = {-194, 4721, 381, -766};
  FLICKER_CUST_STATISTICS EV50_L60 = {988, 470, 879, -425};
  FLICKER_CUST_STATISTICS EV60_L50 = {1178, 495, 1309, -499};
  FLICKER_CUST_STATISTICS EV60_L60 = {-162, 2898, 247, -642};

  for(i=0;i<9;i++)
  {
    para->flickerFreq[i]=freq[i];
  }
  para->flickerGradThreshold=30;
  para->flickerSearchRange=8;
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
  para->freq_feature_index[0]=3;
  para->freq_feature_index[1]=2;
}

typedef NSFeature::RAWSensorInfo<SENSOR_ID> SensorInfoSingleton_T;
namespace NSFeature {
template <>
UINT32
SensorInfoSingleton_T::
impGetFlickerPara(MINT32 sensorMode, MVOID*const pDataBuf) const
{
	ALOGD("impGetFlickerPara+ mode=%d", sensorMode);
	ALOGD("prv=%d, vdo=%d, cap=%d, zsd=%d",
	    (int)e_sensorModePreview, (int)e_sensorModeVideoPreview, (int)e_sensorModeCapture, (int)e_sensorModeZsd );
	FLICKER_CUST_PARA* para;
	para =  (FLICKER_CUST_PARA*)pDataBuf;
	if(sensorMode == e_sensorModePreview)
		get_flicker_para_by_Preview(para);

	else if(sensorMode == e_sensorModeVideo)
	{
		get_flicker_para_by_Video(para);
	}
	else if(sensorMode == e_sensorModeCapture)
	{
		get_flicker_para_by_Capture(para);
	}
	else if(sensorMode == e_sensorModeVideo1)
	{
		get_flicker_para_by_Video1(para);
	}
	else if(sensorMode == e_sensorModeVideo2)
	{
		get_flicker_para_by_Video2(para);
	}
	else
	{
		ALOGD("impGetFlickerPara ERROR ln=%d", __LINE__);
		return -1;
	}
	ALOGD("impGetFlickerPara-");
	return 0;
}
}


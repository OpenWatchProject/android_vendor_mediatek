//[Sensor]
//name = imx386mipiraw
//
//[Preview]
//read_freq = 233300000
//pixel_line = 4296
//column_length = 1483
//
//[Video]
//read_freq = 433300000
//pixel_line = 4296
//column_length = 2991
//
//[Capture]
//read_freq = 433300000
//pixel_line = 4296
//column_length = 2991
//
//[Video1]
//read_freq = 594000000
//pixel_line = 4296
//column_length = 711
//
//[Video2]
//read_freq = 600000000
//pixel_line = 4296
//column_length = 711
#define MTK_LOG_ENABLE 1
#include <log/log.h>
#include <fcntl.h>
#include <math.h>

#include "camera_custom_nvram.h"
#include "camera_custom_sensor.h"
#include "image_sensor.h"
#include "kd_imgsensor_define.h"
#include "camera_AE_PLineTable_imx386mipiraw.h"
#include "camera_info_imx386mipiraw.h"
#include "camera_custom_AEPlinetable.h"
//#include "camera_custom_flicker_table.h"
#include "camera_custom_flicker_para.h"
#include <log/log.h>


static void get_flicker_para_by_Preview(FLICKER_CUST_PARA* para)
{
  int i;
  int freq[9] = { 70, 90, 100, 120, 130, 160, 170, 190, 210};
  FLICKER_CUST_STATISTICS EV50_L50 = {-194, 4721, 381, -766};
  FLICKER_CUST_STATISTICS EV50_L60 = {1120, 415, 775, -393};
  FLICKER_CUST_STATISTICS EV60_L50 = {1336, 437, 1154, -467};
  FLICKER_CUST_STATISTICS EV60_L60 = {-162, 2898, 247, -642};
  for(i=0;i<9;i++)
  {
    para->flickerFreq[i]=freq[i];
  }
  para->flickerGradThreshold=26;
  para->flickerSearchRange=24;
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
  int freq[9] =  { 80, 90, 100, 120, 130, 140, 170, 210, 230};
  FLICKER_CUST_STATISTICS EV50_L50 = {-194, 4721, 381, -766};
  FLICKER_CUST_STATISTICS EV50_L60 = {1025, 453, 847, -416};
  FLICKER_CUST_STATISTICS EV60_L50 = {1222, 477, 1261, -490};
  FLICKER_CUST_STATISTICS EV60_L60 = {-162, 2898, 247, -642};

  for(i=0;i<9;i++)
  {
    para->flickerFreq[i]=freq[i];
  }
  para->flickerGradThreshold=23;
  para->flickerSearchRange=48;
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

static void get_flicker_para_by_Capture(FLICKER_CUST_PARA* para)
{
  int i;
  int freq[9] =  { 80, 90, 100, 120, 130, 140, 170, 210, 230};
  FLICKER_CUST_STATISTICS EV50_L50 = {-194, 4721, 381, -766};
  FLICKER_CUST_STATISTICS EV50_L60 = {1025, 453, 847, -416};
  FLICKER_CUST_STATISTICS EV60_L50 = {1222, 477, 1261, -490};
  FLICKER_CUST_STATISTICS EV60_L60 = {-162, 2898, 247, -642};

  for(i=0;i<9;i++)
  {
    para->flickerFreq[i]=freq[i];
  }
  para->flickerGradThreshold=24;
  para->flickerSearchRange=48;
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

static void get_flicker_para_by_Video1(FLICKER_CUST_PARA* para)
{
  int i;
  int freq[9] =  { 100, 120, 130, 140, 160, 170, 190, 210, 230};
  FLICKER_CUST_STATISTICS EV50_L50 = {-194, 4721, 381, -766};
  FLICKER_CUST_STATISTICS EV50_L60 = {292, 1591, 2972, -737};
  FLICKER_CUST_STATISTICS EV60_L50 = {348, 1676, 4424, -811};
  FLICKER_CUST_STATISTICS EV60_L60 = {-162, 2898, 247, -642};

  for(i=0;i<9;i++)
  {
    para->flickerFreq[i]=freq[i];
  }
  para->flickerGradThreshold=20;
  para->flickerSearchRange=12;
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
  para->freq_feature_index[0]=1;
  para->freq_feature_index[1]=0;
}

static void get_flicker_para_by_Video2(FLICKER_CUST_PARA* para)
{
  int i;
  int freq[9] =  { 100, 120, 130, 140, 160, 170, 190, 210, 230};
  FLICKER_CUST_STATISTICS EV50_L50 = {-194, 4721, 381, -766};
  FLICKER_CUST_STATISTICS EV50_L60 = {293, 1583, 2958, -736};
  FLICKER_CUST_STATISTICS EV60_L50 = {350, 1668, 4403, -810};
  FLICKER_CUST_STATISTICS EV60_L60 = {-162, 2898, 247, -642};

  for(i=0;i<9;i++)
  {
    para->flickerFreq[i]=freq[i];
  }
  para->flickerGradThreshold=23;
  para->flickerSearchRange=12;
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
  para->freq_feature_index[0]=1;
  para->freq_feature_index[1]=0;
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


//[Sensor]
//name = ov8856mipiraw
//
//[Preview]
//read_freq = 144000000
//pixel_line = 1928
//column_length = 1199
//
//[Video]
//read_freq = 144000000
//pixel_line = 1928
//column_length = 1199
//
//[Capture]
//read_freq = 144000000
//pixel_line = 1940
//column_length = 2423
//
//[Video1]
//read_freq = 72000000
//pixel_line = 2566
//column_length = 2423
//
//[Video2]
//read_freq = 72000000
//pixel_line = 1928
//column_length = 1199
//
#include <log/log.h>
#include <fcntl.h>
#include <math.h>

#include "camera_custom_nvram.h"
#include "camera_custom_sensor.h"
#include "image_sensor.h"
#include "kd_imgsensor_define.h"
#include "camera_AE_PLineTable_ov8856mipiraw.h"
#include "camera_info_ov8856mipiraw.h"
#include "camera_custom_AEPlinetable.h"
//#include "camera_custom_flicker_table.h"
#include "camera_custom_flicker_para.h"
#include <cutils/log.h>


static void get_flicker_para_by_Preview(FLICKER_CUST_PARA* para)
{
  int i;
  int freq[9] = { 70, 100, 120, 140, 160, 170, 190, 210, 230};
  FLICKER_CUST_STATISTICS EV50_L50 = {-194, 4721, 381, -766};
  FLICKER_CUST_STATISTICS EV50_L60 = {919, 505, 944, -444};
  FLICKER_CUST_STATISTICS EV60_L50 = {1096, 532, 1406, -518};
  FLICKER_CUST_STATISTICS EV60_L60 = {-162, 2898, 247, -642};
  for(i=0;i<9;i++)
  {
    para->flickerFreq[i]=freq[i];
  }
  para->flickerGradThreshold=24;
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
  para->freq_feature_index[0]=2;
  para->freq_feature_index[1]=1;
}

static void get_flicker_para_by_Video(FLICKER_CUST_PARA* para)
{
  int i;
  int freq[9] =  { 70, 100, 120, 140, 160, 170, 190, 210, 230};
  FLICKER_CUST_STATISTICS EV50_L50 = {-194, 4721, 381, -766};
  FLICKER_CUST_STATISTICS EV50_L60 = {919, 505, 944, -444};
  FLICKER_CUST_STATISTICS EV60_L50 = {1096, 532, 1406, -518};
  FLICKER_CUST_STATISTICS EV60_L60 = {-162, 2898, 247, -642};

  for(i=0;i<9;i++)
  {
    para->flickerFreq[i]=freq[i];
  }
  para->flickerGradThreshold=26;
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
  para->freq_feature_index[0]=2;
  para->freq_feature_index[1]=1;
}

static void get_flicker_para_by_Capture(FLICKER_CUST_PARA* para)
{
  int i;
  int freq[9] =  { 90, 100, 110, 120, 130, 170, 190, 210, 230};
  FLICKER_CUST_STATISTICS EV50_L50 = {-194, 4721, 381, -766};
  FLICKER_CUST_STATISTICS EV50_L60 = {850, 547, 1022, -464};
  FLICKER_CUST_STATISTICS EV60_L50 = {1013, 576, 1521, -538};
  FLICKER_CUST_STATISTICS EV60_L60 = {-162, 2898, 247, -642};

  for(i=0;i<9;i++)
  {
    para->flickerFreq[i]=freq[i];
  }
  para->flickerGradThreshold=25;
  para->flickerSearchRange=40;
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
  para->freq_feature_index[1]=1;
}

static void get_flicker_para_by_Video1(FLICKER_CUST_PARA* para)
{
  int i;
  int freq[9] =  { 70, 80, 90, 100, 110, 120, 130, 140, 160};
  FLICKER_CUST_STATISTICS EV50_L50 = {-194, 4721, 381, -766};
  FLICKER_CUST_STATISTICS EV50_L60 = {12234, 38, 71, 218};
  FLICKER_CUST_STATISTICS EV60_L50 = {14588, 40, 105, 144};
  FLICKER_CUST_STATISTICS EV60_L60 = {-162, 2898, 247, -642};

  for(i=0;i<9;i++)
  {
    para->flickerFreq[i]=freq[i];
  }
  para->flickerGradThreshold=31;
  para->flickerSearchRange=40;
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
  para->freq_feature_index[0]=5;
  para->freq_feature_index[1]=3;
}

static void get_flicker_para_by_Video2(FLICKER_CUST_PARA* para)
{
  int i;
  int freq[9] =  { 70, 80, 90, 100, 120, 130, 140, 160, 170};
  FLICKER_CUST_STATISTICS EV50_L50 = {-194, 4721, 381, -766};
  FLICKER_CUST_STATISTICS EV50_L60 = {1207, 385, 719, -374};
  FLICKER_CUST_STATISTICS EV60_L50 = {1439, 405, 1071, -448};
  FLICKER_CUST_STATISTICS EV60_L60 = {-162, 2898, 247, -642};

  for(i=0;i<9;i++)
  {
    para->flickerFreq[i]=freq[i];
  }
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
  para->freq_feature_index[1]=3;
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


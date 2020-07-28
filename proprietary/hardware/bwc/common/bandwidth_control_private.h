#ifndef __BANDWIDTH_CONTROL_PRIVATE_H__
#define __BANDWIDTH_CONTROL_PRIVATE_H__

#include "bandwidth_control.h"

/*=============================================================================
    Compile Flags
  =============================================================================*/
#define FLAG_SUPPORT_PROPERTY
//#define FLAG_SUPPORT_MODEM_SCALE
//#define FLAG_SUPPORT_SMI_SETTING


/*=============================================================================
    Header Files
  =============================================================================*/
#include    <stdio.h>   //printf()
#include    <unistd.h>  //gettid()
#include    <utils/Log.h>

/*=============================================================================
    MACRO
  =============================================================================*/

#ifdef LOG_TAG
  #undef LOG_TAG
#endif

#define  LOG_TAG    "BWC"

#define BWC_INFO(fmt, arg...)       { ALOGI(fmt, ##arg);   }
#define BWC_WARNING(fmt, arg...)    { ALOGW(fmt, ##arg);   }
#define BWC_ERROR(fmt, arg...)      { ALOGE(fmt, ##arg);   }


typedef enum {
    SMI_BWC_USER_INFO_CON_PROFILE = 0,
    SMI_BWC_USER_INFO_SENSOR_SIZE,
    SMI_BWC_USER_INFO_VIDEO_RECORD_SIZE,
    SMI_BWC_USER_INFO_DISP_SIZE,
    SMI_BWC_USER_INFO_TV_OUT_SIZE,
    SMI_BWC_USER_INFO_FPS,
    SMI_BWC_USER_INFO_VIDEO_ENCODE_CODEC,
    SMI_BWC_USER_INFO_VIDEO_DECODE_CODEC,
    SMI_BWC_USER_INFO_HW_OVL_LIMIT,
    SMI_BWC_USER_INFO_CNT
} MTK_SMI_BWC_USER_INFO_ID;


typedef struct {
    unsigned int flag; // Reserved
    int concurrent_profile;
    int sensor_size[2];
    int video_record_size[2];
    int display_size[2];
    int tv_out_size[2];
    int fps;
    int video_encode_codec;
    int video_decode_codec;
    int hw_ovl_limit;
} MTK_SMI_BWC_MM_INFO_USER;

class BWCHelper{
public:
    int set_bwc_mm_property( int propterty_id, int value1, int value2 );
    int get_bwc_mm_property( MTK_SMI_BWC_MM_INFO_USER* properties );
    void profile_add(
        MTK_SMI_BWC_MM_INFO_USER* properties,
        BWC_PROFILE_TYPE profile );
    void profile_remove(
        MTK_SMI_BWC_MM_INFO_USER* properties,
        BWC_PROFILE_TYPE profile );
    int profile_get( MTK_SMI_BWC_MM_INFO_USER* properties );
};

#endif  /*__BANDWIDTH_CONTROL_PRIVATE_H__*/


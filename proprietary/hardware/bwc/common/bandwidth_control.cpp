#include <stdio.h>
#include <string.h>
#include "bandwidth_control.h"
#include "bandwidth_control_private.h"

#ifdef FLAG_SUPPORT_PROPERTY
//Property
#include <cutils/properties.h>
#else
#define PROPERTY_VALUE_MAX  (512)
#endif

#define MAX_PROP_NAME_CHAR  (512)

/*******************************************************************************
    Bandwidth Control Limitation Table
 *******************************************************************************/
#if 0
const static BWC_SETTING BWCLIMIT_VR_1066(  BWC_SIZE( 3072, 2044 ),     //_sensor_size   //6M
                                            BWC_SIZE( 1920, 1080 ),     //_vr_size      //1080P
                                            BWC_SIZE( 1280,  768 ),     //_disp_size    //WXGA
                                            BWC_SIZE(    0,    0 ),     //_tv_size      //no tv out
                                            30,                         //_fps          //30 fps
                                            BWCVT_MPEG4,                //_venc_codec_type//MPEG4
                                            BWCVT_NONE                  //_vdec_codec_type//no decode
                                            );

const static BWC_SETTING BWCLIMIT_VT_1066(  BWC_SIZE( 1920, 1080 ),     //_sensor_size   //FHD = 2M
                                            BWC_SIZE( 1280,  720 ),     //_vr_size      //720P decode/decode
                                            BWC_SIZE( 1280,  768 ),     //_disp_size    //WXGA
                                            BWC_SIZE( 1280,  768 ),     //_tv_size      //WXGA tv out
                                            30,                         //_fps          //30 fps
                                            BWCVT_H264,                 //_venc_codec_type//H.264
                                            BWCVT_H264                  //_vdec_codec_type//H.264
                                            );

#endif

extern const char* BwcProfileType_GetStr( BWC_PROFILE_TYPE profile );
/*******************************************************************************
    Bandwidth Control Primitive Datatypes
 *******************************************************************************/
/*-----------------------------------------------------------------------------
    BWC_SIZE
  -----------------------------------------------------------------------------*/
void BWC_SIZE::LoadFromProperty( const char* property_name ){
    // Doesn't suppport
    // BWC.mm* system properties have been removed in GMP
    BWC_UNUSED(property_name);
    BWC_WARNING("BWC_SIZE::LoadFromProperty is not supported on this platform");
}

void BWC_SIZE::SetToProperty( const char* property_name ) const {
    // Doesn't suppport
    // BWC.mm* system properties have been removed in GMP
    BWC_UNUSED(property_name);
    BWC_WARNING("BWC_SIZE::SetToProperty is not supported on this platform");
}

/*-----------------------------------------------------------------------------
    BWC_INT
  -----------------------------------------------------------------------------*/

void BWC_INT::LoadFromProperty( const char* property_name ){
    // Doesn't suppport
    // BWC.mm* system properties have been removed in GMP
    BWC_UNUSED(property_name);
    BWC_WARNING("BWC_INT::LoadFromProperty is not supported on this platform");
}

void BWC_INT::SetToProperty( const char* property_name ) const {
    // Doesn't suppport
    // BWC.mm* system properties have been removed in GMP
    BWC_UNUSED(property_name);
    BWC_WARNING("BWC_INT::SetToProperty is not supported on this platform");
}

/*-----------------------------------------------------------------------------
    BWC_SETTING
  -----------------------------------------------------------------------------*/
unsigned long BWC_SETTING::CalcThroughput_VR( void ) const {
    unsigned long throughput = 0;

    const float bpp = 1.5;  //use yuv422 as limit calculation

    throughput  =   sensor_size.w * sensor_size.h * 2;  //sensor out + cdp in
    throughput +=   vr_size.w * vr_size.h * 2;          //cdp out + venc in
    throughput +=   disp_size.w * disp_size.h * 2;      //cdp out + disp in

    if( tv_size.w * tv_size.h ){
        throughput += tv_size.w * tv_size.h;    //TV in
    }

    return (unsigned long)(throughput*fps*bpp);

}

unsigned long BWC_SETTING::CalcThroughput_VT( void ) const {
    unsigned long throughput = 0;

    const float bpp = 1.5;  //use yuv422 as limit calculation

    throughput  =   sensor_size.w * sensor_size.h * 2;  //sensor out + cdp in
    throughput +=   vr_size.w * vr_size.h * 2;          //cdp out + venc in
    throughput +=   disp_size.w * disp_size.h * 2;      //cdp out + disp in

    throughput +=   vr_size.w * vr_size.h * 1;          //vdec out

    if( tv_size.w * tv_size.h ){
        throughput += tv_size.w * tv_size.h * 2;    //disp out + TV in
    }

    return (unsigned long)(throughput*fps*bpp);

}

void BWC_SETTING::DumpInfo( void ){
    #define _DUMP_SIZE( _field_name_ ) \
        BWC_INFO("%20s = %6ld x %6ld\n", #_field_name_, _field_name_.w , _field_name_.h );

    #define _DUMP_INT( _scalar_ ) \
        BWC_INFO("%20s = %6ld\n", #_scalar_, (long)_scalar_ );

    BWC_INFO("BWC_SETTING::DumpInfo-------\n\n");
    _DUMP_SIZE( sensor_size );
    _DUMP_SIZE( vr_size );
    _DUMP_SIZE( disp_size );
    _DUMP_SIZE( tv_size );

    _DUMP_INT(fps);
    _DUMP_INT(venc_codec_type);
    _DUMP_INT(vdec_codec_type);
    BWC_INFO("----------------------------\n\n");

}

/*******************************************************************************
    Bandwidth Control Change Profile
 *******************************************************************************/
/*-----------------------------------------------------------------------------
    BWC
  -----------------------------------------------------------------------------*/

#define MAX_STR_SIZE 128
int BWC::Profile_Change( BWC_PROFILE_TYPE profile_type, bool bOn ){
    BWC_SETTING mmsetting;
    BWCHelper bwcHelper;
    int return_code = 0;
    char emi_scen_str[MAX_STR_SIZE] = {0};
    char ddr_type_str[MAX_STR_SIZE] = {0};

    //GMP start
    MTK_SMI_BWC_MM_INFO_USER mm_info = { 0, 0, { 0, 0 }, { 0, 0 }, { 0, 0 }, {
        0, 0 }, 0, 0, 0, 0 };

    if (!bwcHelper.get_bwc_mm_property(&mm_info)) {
        mmsetting.sensor_size.w = mm_info.sensor_size[0];
        mmsetting.sensor_size.h = mm_info.sensor_size[1];

        mmsetting.vr_size.w = mm_info.video_record_size[0];
        mmsetting.vr_size.h = mm_info.video_record_size[1];

        mmsetting.disp_size.w = mm_info.display_size[0];
        mmsetting.disp_size.h = mm_info.display_size[1];

        mmsetting.tv_size.w = mm_info.tv_out_size[0];
        mmsetting.tv_size.h = mm_info.tv_out_size[1];

        mmsetting.fps = mm_info.fps;
        mmsetting.venc_codec_type = (BWC_VCODEC_TYPE) mm_info.video_encode_codec;
        mmsetting.vdec_codec_type = (BWC_VCODEC_TYPE) mm_info.video_decode_codec;
    }
    //GMP end

    //mmsetting.DumpInfo();

    /*Get DDR Type*/
    //BWC_INFO("DDR Type = %d\n", emi_ddr_type_get() );
    emi_ddr_type_string_get(ddr_type_str, MAX_STR_SIZE);

    /*Change SMI Setting*/
    return_code = smi_bw_ctrl_set(profile_type, mmsetting.venc_codec_type, bOn);
    if( return_code != 0 ){
        BWC_ERROR("Failed to switch SMI profile, error: %d", return_code);
    }

    /*Change EMI Setting*/
    return_code = emi_bw_ctrl_set(profile_type, mmsetting.venc_codec_type, bOn, emi_scen_str);
    if( return_code != 0 ){
        BWC_ERROR("Failed to switch EMI profile, error: %d", return_code);
    }

    if( bOn ){
        bwcHelper.profile_add(&mm_info, profile_type);
    }else{
        bwcHelper.profile_remove(&mm_info, profile_type);

}

    BWC_INFO("Profile_Change:[%s]:%s,current concurrency is 0x%x,DDR=%s,EMI scenario=%s\n",
        BwcProfileType_GetStr(profile_type), bOn ? "ON" : "OFF",
            bwcHelper.profile_get(&mm_info), ddr_type_str,
            emi_scen_str);
    return 0;

}

/*******************************************************************************
    Bandwidth Control Various Property
 *******************************************************************************/
//GMP start
void BWC::SensorSize_Set( const BWC_SIZE &sensor_size ){
    BWCHelper bwcHelper;
    bwcHelper.set_bwc_mm_property(SMI_BWC_USER_INFO_SENSOR_SIZE, sensor_size.w,
        sensor_size.h);
}

BWC_SIZE BWC::SensorSize_Get( void ){
    BWC_SIZE    sensor_size;
    BWCHelper bwcHelper;
    MTK_SMI_BWC_MM_INFO_USER mm_info = { 0, 0, { 0, 0 }, { 0, 0 }, { 0, 0 }, {
        0, 0 }, 0, 0, 0, 0 };

    if( bwcHelper.get_bwc_mm_property(&mm_info) == 0 ){
        sensor_size.w = mm_info.sensor_size[0];
        sensor_size.h = mm_info.sensor_size[1];
    }
    return sensor_size;
}

void BWC::VideoRecordSize_Set( const BWC_SIZE &vr_size ){
    BWCHelper bwcHelper;
    bwcHelper.set_bwc_mm_property(SMI_BWC_USER_INFO_VIDEO_RECORD_SIZE,
        vr_size.w, vr_size.h);
}

BWC_SIZE BWC::VideoRecordSize_Get( void ){
    BWC_SIZE    vr_size;
    MTK_SMI_BWC_MM_INFO_USER mm_info = { 0, 0, { 0, 0 }, { 0, 0 }, { 0, 0 }, {
        0, 0 }, 0, 0, 0, 0 };
    BWCHelper bwcHelper;
    if( bwcHelper.get_bwc_mm_property(&mm_info) == 0 ){
        vr_size.w = mm_info.video_record_size[0];
        vr_size.h = mm_info.video_record_size[1];
    }
    return vr_size;
}

void BWC::DisplaySize_Set( const BWC_SIZE &disp_size ){
    BWCHelper bwcHelper;
    bwcHelper.set_bwc_mm_property(SMI_BWC_USER_INFO_DISP_SIZE, disp_size.w,
        disp_size.h);
}

BWC_SIZE BWC::DisplaySize_Get( void ){
    BWC_SIZE    disp_size;
    MTK_SMI_BWC_MM_INFO_USER mm_info = { 0, 0, { 0, 0 }, { 0, 0 }, { 0, 0 }, {
        0, 0 }, 0, 0, 0, 0 };
    BWCHelper bwcHelper;
    if( bwcHelper.get_bwc_mm_property(&mm_info) == 0 ){
        disp_size.w = mm_info.display_size[0];
        disp_size.h = mm_info.display_size[1];
    }
    return disp_size;
}

void BWC::TvOutSize_Set( const BWC_SIZE &tv_size ){
    BWCHelper bwcHelper;
    bwcHelper.set_bwc_mm_property(SMI_BWC_USER_INFO_TV_OUT_SIZE, tv_size.w,
        tv_size.h);
}

BWC_SIZE BWC::TvOutSize_Get( void ){
    BWC_SIZE    tv_size;
    MTK_SMI_BWC_MM_INFO_USER mm_info = { 0, 0, { 0, 0 }, { 0, 0 }, { 0, 0 }, {
        0, 0 }, 0, 0, 0, 0 };
    BWCHelper bwcHelper;

    if( bwcHelper.get_bwc_mm_property(&mm_info) == 0 ){
        tv_size.w = mm_info.tv_out_size[0];
        tv_size.h = mm_info.tv_out_size[1];
    }
    return tv_size;
}

void BWC::Fps_Set( int fps ){
    BWCHelper bwcHelper;
    bwcHelper.set_bwc_mm_property(SMI_BWC_USER_INFO_FPS, fps, 0);
}

int BWC::Fps_Get( void ){
    MTK_SMI_BWC_MM_INFO_USER mm_info = { 0, 0, { 0, 0 }, { 0, 0 }, { 0, 0 }, {
        0, 0 }, 0, 0, 0, 0 };
    BWCHelper bwcHelper;

    if( bwcHelper.get_bwc_mm_property(&mm_info) == 0 ){
        return mm_info.fps;
    }else{
        return 0;
    }
}

void BWC::VideoEncodeCodec_Set( BWC_VCODEC_TYPE codec_type ){
    BWCHelper bwcHelper;

    bwcHelper.set_bwc_mm_property(SMI_BWC_USER_INFO_VIDEO_ENCODE_CODEC,
        codec_type, 0);
}

BWC_VCODEC_TYPE BWC::VideoEncodeCodec_Get( void ){
    MTK_SMI_BWC_MM_INFO_USER mm_info = { 0, 0, { 0, 0 }, { 0, 0 }, { 0, 0 }, {
        0, 0 }, 0, 0, 0, 0 };
    BWCHelper bwcHelper;
    if( bwcHelper.get_bwc_mm_property(&mm_info) == 0 ){
        return (BWC_VCODEC_TYPE) mm_info.video_encode_codec;
    }else{
        return (BWC_VCODEC_TYPE) 0;
    }
}

void BWC::VideoDecodeCodec_Set( BWC_VCODEC_TYPE codec_type ){
    BWCHelper bwcHelper;
    bwcHelper.set_bwc_mm_property(SMI_BWC_USER_INFO_VIDEO_DECODE_CODEC,
        codec_type, 0);
}

BWC_VCODEC_TYPE BWC::VideoDecodeCodec_Get( void ){
    MTK_SMI_BWC_MM_INFO_USER mm_info = { 0, 0, { 0, 0 }, { 0, 0 }, { 0, 0 }, {
        0, 0 }, 0, 0, 0, 0 };
    BWCHelper bwcHelper;
    if( bwcHelper.get_bwc_mm_property(&mm_info) == 0 ){
        return (BWC_VCODEC_TYPE) mm_info.video_decode_codec;
    }else{
        return (BWC_VCODEC_TYPE) 0;
    }
}

//GMP end


void BWC::_Profile_Set( BWC_PROFILE_TYPE profile ){
//Null function
    BWC_UNUSED(profile);
    BWC_WARNING(
        "BWC::_Profile_Set is not supported on this platform, use Profile_Change instead");
}

void BWC::_Profile_Add( BWC_PROFILE_TYPE profile ){
    BWCHelper bwcHelper;
    MTK_SMI_BWC_MM_INFO_USER mm_info = { 0, 0, { 0, 0 }, { 0, 0 }, { 0, 0 }, {
        0, 0 }, 0, 0, 0, 0 };
    if (bwcHelper.get_bwc_mm_property(&mm_info))
        return;
    bwcHelper.profile_add(&mm_info, profile);
    BWC_INFO("BWC::_Profile_Add should not be used outside BWC lib");
}

void BWC::_Profile_Remove( BWC_PROFILE_TYPE profile ){
    BWCHelper bwcHelper;
    MTK_SMI_BWC_MM_INFO_USER mm_info = { 0, 0, { 0, 0 }, { 0, 0 }, { 0, 0 }, {
        0, 0 }, 0, 0, 0, 0 };
    if (bwcHelper.get_bwc_mm_property(&mm_info))
        return;
    bwcHelper.profile_remove(&mm_info, profile);
    BWC_INFO("BWC::_Profile_Remove should not be used outside BWC lib");
}

int BWC::_Profile_Get( void ){
    BWCHelper bwcHelper;
    MTK_SMI_BWC_MM_INFO_USER mm_info = { 0, 0, { 0, 0 }, { 0, 0 }, { 0, 0 }, {
        0, 0 }, 0, 0, 0, 0 };
    if (bwcHelper.get_bwc_mm_property(&mm_info))
        return -1;
    BWC_INFO("BWC::_Profile_Get should not be used outside BWC lib");
    return bwcHelper.profile_get(&mm_info);
}

/*-----------------------------------------------------------------------------
    Auto generate property_name from given function name
  -----------------------------------------------------------------------------*/
int BWC::property_name_str_get( const char* function_name, char* prop_name ){
    // Doesn't suppport in GMP
    // BWC doesn't keep the information with system properties
    BWC_UNUSED(function_name);
    BWC_UNUSED(prop_name);
    BWC_WARNING("BWC::property_name_str_get is not supported on this platform");
    return 0;

}

unsigned int BWC_MONITOR::query_hwc_max_pixel(){
    unsigned int hwc_max_pixel = -1;
    hwc_max_pixel = this->get_smi_bw_state();
    //BWC_INFO("query_hwc_max_pixel: get_smi_bw_state return %d\n", hwc_max_pixel );
    if( hwc_max_pixel <= 0 ){
        return 1920 * 1080 * 7;
    }else{
        return hwc_max_pixel;
    }
}


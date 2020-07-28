#ifndef __BANDWIDTH_CONTROL_H__
#define __BANDWIDTH_CONTROL_H__

#define BWC_UNUSED(x) (void)(x)

#define BWC_VPU_DVFS_UNREQUEST (-1)

/*-----------------------------------------------------------------------------
 BWC Primitive Data Type : Size
 (Avoid having virtual table in structure, try not to use inheritance)
 -----------------------------------------------------------------------------*/
class BWC_SIZE{
public:
    long w;
    long h;

public:
    BWC_SIZE() :
        w(0), h(0){
    };

    BWC_SIZE( long _w, long _h ){
        w = _w;
        h = _h;
    };

    BWC_SIZE& operator=( const BWC_SIZE& rhs ){
        w = rhs.w;
        h = rhs.h;
        return *this;
    }

    bool operator==( const BWC_SIZE& rhs ) const {
        return ((w == rhs.w) && (h == rhs.h));
    }

    bool operator!=( const BWC_SIZE& rhs ) const {
        return !(*this == rhs);
    }

    void LoadFromProperty( const char* property_name );
    void SetToProperty( const char* property_name ) const;

};

/*-----------------------------------------------------------------------------
 BWC Primitive Data Type : Integer
 -----------------------------------------------------------------------------*/
class BWC_INT{
public:
    int value;

public:
    BWC_INT() :
        value(0){
    };

    BWC_INT( int _value ){
        value = _value;
    };

    BWC_INT& operator=( const BWC_INT& rhs ){
        value = rhs.value;
        return *this;
    }

    bool operator==( const BWC_INT& rhs ) const {
        return (value == rhs.value);
    }

    bool operator!=( const BWC_INT& rhs ) const {
        return !(*this == rhs);
    }

    void LoadFromProperty( const char* property_name );
    void SetToProperty( const char* property_name ) const;

};

/*-----------------------------------------------------------------------------
 BWC Operation Scenario
 -----------------------------------------------------------------------------*/
typedef enum {
    // Lowest Priority
    BWCPT_NONE = 0,
    BWCPT_VIDEO_NORMAL,
    BWCPT_CAMERA_PREVIEW,
    BWCPT_CAMERA_ZSD,
    BWCPT_CAMERA_CAPTURE,
    BWCPT_CAMERA_ICFP,
    BWCPT_VIDEO_SWDEC_PLAYBACK,
    BWCPT_VIDEO_PLAYBACK,
    BWCPT_VIDEO_PLAYBACK_HIGH_FPS,
    BWCPT_VIDEO_PLAYBACK_HIGH_RESOLUTION,
    BWCPT_VIDEO_TELEPHONY,
    BWCPT_VIDEO_RECORD,
    BWCPT_VIDEO_RECORD_CAMERA,
    BWCPT_VIDEO_RECORD_SLOWMOTION,
    BWCPT_VIDEO_SNAPSHOT,
    BWCPT_VIDEO_LIVE_PHOTO,
    BWCPT_VIDEO_WIFI_DISPLAY,
    BWCPT_FORCE_MMDVFS,
    // Highest Priority

} BWC_PROFILE_TYPE;

typedef enum {
    BWCVT_NONE,
    BWCVT_MPEG4,
    BWCVT_H264,
    BWCVT_HEVC,
    BWCVT_VP8,
    BWCVT_VC1,
    BWCVT_MPEG2,
    BWCVT_VP9
} BWC_VCODEC_TYPE;

/*-----------------------------------------------------------------------------
 BWC Setting : a combination of MM setting. In charge of calculating out
 bandwidth consumption
 -----------------------------------------------------------------------------*/
class BWC_SETTING{
public:
    BWC_SIZE sensor_size;
    BWC_SIZE vr_size;
    BWC_SIZE disp_size;
    BWC_SIZE tv_size;
    int fps;
    BWC_VCODEC_TYPE venc_codec_type;
    BWC_VCODEC_TYPE vdec_codec_type;

public:
    BWC_SETTING(){
        fps = 0;
        venc_codec_type = BWCVT_NONE;
        vdec_codec_type = BWCVT_NONE;
    };

    BWC_SETTING(
        const BWC_SIZE& _sensor_size,
        const BWC_SIZE& _vr_size,
        const BWC_SIZE& _disp_size,
        const BWC_SIZE& _tv_size,
        int _fps,
        BWC_VCODEC_TYPE _venc_codec_type,
        BWC_VCODEC_TYPE _vdec_codec_type ) :
            sensor_size(_sensor_size), vr_size(_vr_size), disp_size(_disp_size),
            tv_size(_tv_size), fps(_fps), venc_codec_type(_venc_codec_type),
            vdec_codec_type(_vdec_codec_type){
    };

public:
    unsigned long CalcThroughput_VR( void ) const;
    unsigned long CalcThroughput_VT( void ) const;

public:
    void DumpInfo( void );

};

/*******************************************************************************

 *******************************************************************************/
/*-----------------------------------------------------------------------------
 BWC Core Object: Interface to BWC
 -----------------------------------------------------------------------------*/
#define BWC_NO_OVL_LIMIT 0

class BWC_MONITOR{
public:
    int start( void );
    int stop( void );
    unsigned int query_hwc_max_pixel( void );
    BWC_MONITOR();
    ~BWC_MONITOR();
private:
    int smi_fd;
    unsigned int get_smi_bw_state( void );
};

// GMP  start
typedef struct {
    unsigned int flag; // Reserved
    int concurrent_profile;
    long sensor_size[2];
    long video_record_size[2];
    long display_size[2];
    long tv_out_size[2];
    int fps;
    int video_encode_codec;
    int video_decode_codec;
} MTK_SMI_BWC_MM_INFO_ADAPT;
// GMP  end

class BWC_EMI_Adaptor;
class BWC_SMI_Adaptor;

class BWC{
public:
    int Profile_Change( BWC_PROFILE_TYPE profile_type, bool bOn );


public:
    void SensorSize_Set( const BWC_SIZE &sensor_size );
    BWC_SIZE SensorSize_Get( void );

    void VideoRecordSize_Set( const BWC_SIZE &vr_size );
    BWC_SIZE VideoRecordSize_Get( void );

    void DisplaySize_Set( const BWC_SIZE &disp_size );
    BWC_SIZE DisplaySize_Get( void );

    void TvOutSize_Set( const BWC_SIZE &tv_size );
    BWC_SIZE TvOutSize_Get( void );

    void Fps_Set( int fps );
    int Fps_Get( void );

    void VideoEncodeCodec_Set( BWC_VCODEC_TYPE codec_type );
    BWC_VCODEC_TYPE VideoEncodeCodec_Get( void );

    void VideoDecodeCodec_Set( BWC_VCODEC_TYPE codec_type );
    BWC_VCODEC_TYPE VideoDecodeCodec_Get( void );


public:
    void _Profile_Set( BWC_PROFILE_TYPE profile );
    void _Profile_Add( BWC_PROFILE_TYPE profile );
    void _Profile_Remove( BWC_PROFILE_TYPE profile );
    int _Profile_Get( void );

private:
    int property_name_str_get( const char* function_name, char* prop_name ); //Auto generate property_name from given function name
    bool check_profile_change_valid( BWC_PROFILE_TYPE profile_type );

    // GMP start
    int set_bwc_mm_property( int propterty_id, long value1, long value2 );
    int get_bwc_mm_property( MTK_SMI_BWC_MM_INFO_ADAPT* properties );
    void profile_add(
        MTK_SMI_BWC_MM_INFO_ADAPT* properties,
        BWC_PROFILE_TYPE profile );
    void profile_remove(
        MTK_SMI_BWC_MM_INFO_ADAPT* properties,
        BWC_PROFILE_TYPE profile );
    int profile_get( MTK_SMI_BWC_MM_INFO_ADAPT* properties );
    // GMP end


private:
    // Platform-Depended Function
    int smi_bw_ctrl_set(
        BWC_PROFILE_TYPE profile_type,
        BWC_VCODEC_TYPE codec_type,
        bool bOn );
    int emi_bw_ctrl_set(
        BWC_PROFILE_TYPE profile_type,
        BWC_VCODEC_TYPE codec_type,
        bool bOn,
        char* out_str );

    typedef enum {
        EDT_LPDDR2 = 0,
        EDT_DDR3,
        EDT_LPDDR3,
        EDT_mDDR, EDT_NONE = -1,
    } EMI_DDR_TYPE;

    EMI_DDR_TYPE emi_ddr_type_get( void );

    typedef enum {
        MSP_NORMAL = 0,
        MSP_SCALE_DOWN,
    } MODEM_SPEED_PROFILE;

    int modem_speed_profile_set( MODEM_SPEED_PROFILE profile );
    void emi_ddr_type_string_get( char* out_str, int len );


};


/*
 * MMDVFS
 */
typedef enum
{
    MMDVFS_STEP_LOW = 0,
    MMDVFS_STEP_HIGH,

    MMDVFS_STEP_LOW2LOW,    /* LOW */
    MMDVFS_STEP_HIGH2LOW,   /* LOW */
    MMDVFS_STEP_LOW2HIGH,   /* HIGH */
    MMDVFS_STEP_HIGH2HIGH,  /* HIGH */
} mmdvfs_step_enum;

#define MMDVFS_IS_HIGH_STEP(x) (

typedef enum
{
    MMDVFS_SENSOR_SIZE,
    MMDVFS_SENSOR_FPS,
    MMDVFS_CAMERA_MODE_PIP,
    MMDVFS_CAMERA_MODE_VFB,
    MMDVFS_CAMERA_MODE_EIS_2_0,
    MMDVFS_CAMERA_MODE_IVHDR,
    MMDVFS_CAMERA_MODE_STEREO,
    MMDVFS_CAMERA_MODE_MVHDR,
    MMDVFS_CAMERA_MODE_ZVHDR,
    MMDVFS_CAMERA_MODE_DUAL_ZOOM,
    MMDVFS_VENC_SIZE,
    MMDVFS_PREVIEW_SIZE,
    MMDVFS_PARAMETER_COUNT,
    MMDVFS_PARAMETER_EOF = MMDVFS_PARAMETER_COUNT
} mmdvfs_parameter_enum;

typedef enum
{
    MMDVFS_QOS_MAX_CAM_BW,
    MMDVFS_QOS_PARAMETER_COUNT,
    MMDVFS_QOS_PARAMETER_EOF = MMDVFS_QOS_PARAMETER_COUNT
} mmdvfs_qos_parameter_enum;

typedef struct {
    unsigned int camera_id;
    unsigned int sensor_fps;
    unsigned int sensor_size;
    unsigned int pd_size;
    unsigned int video_size;
    double vb_ratio;
    double vb_time;     // ms level
    bool rawi_enable;
    bool yuv_enable;    // direct yuv
    double raw_bbp;     // byte per pixel
    bool is_smvr;
} MMDVFS_QOS_PARAMETER;

extern "C"
{
mmdvfs_step_enum mmdvfs_query(int scenario, ...);
int mmdvfs_set(int scenario, ...);
int mmdvfs_qos_set(int num, ...);
int mmdvfs_get_current_step(void);
int mmdvfs_get_current_mmclk(void);
void mmdvfs_platform_config(int boost_disable, int ddr_type);
int mmdvfs_direct_mmclk_step_set(int scenario, int step, int mmclk);
int mmdvfs_vpu_step_set(int step);
int mmdvfs_vpu_step_get(void);
}

#endif /*__BANDWIDTH_CONTROL_H__*/


#ifndef _Module_HDR_H
#define _Module_HDR_H

#ifdef __cplusplus
extern "C"
{
#endif

#define HDR_ANDROID_PLATFORM // open before release
// #define __Local_Sim__ // should be marked before platform release
#define IN
#define OUT
// #define _calman_dbg_
// #define _calman_dbg2_
// #define _smooth_test_
// #define _contour_solution_

#define before_tone_map_linear_bits 20
#define after_tone_map_linear_bits 16
#define after_gaincurve_linear_bits 16

#define Tone_Map_POINTS 16

#define Gain_Curve_POINTS 256
#define Gain_Curve_INT_BITS 3
#define Gain_Curve_FLT_BITS 13

#define GAMUT_COEF_INT_BITS 3
#define GAMUT_COEF_FLT_BITS 13

#define Y2R_INPUT_FORMAT_SELECT
    #define Y2R_BT2020_NON_CONSTANT_LUMA 0
    #define Y2R_BT2020_CONSTANT_LUMA 1
    #define Y2R_BT709 2
    #define Y2R_Progarmmalbel_Y2R 3

#define R2Y_OUTPUT_FORMAT_SELECT
    #define R2Y_BT709 0
    #define R2Y_BT2020_NON_CONSTANT_LUMA 1
    #define R2Y_Programmable_Matrix 2

#define EOTF_TBL_TYPE 4 // 0:only fix, 1: only programmalbe, 2: 1 fix + programmable, 3: selectable fix + prgrammable, 4: selectable fix
    #define ST2084_EOTF_IDX 0
    #define HLG_EOTF_IDX 1
    #define SDR_EOTF_IDX 2
    #define SRGB_EOTF_IDX 3 //4
    // #define SLD_EOTF_IDX 3 //4

#define OETF_TBL_TYPE 4 // 0:only fix, 1: only programmalbe, 2: 1 fix + programmable, 3: selectable fix + prgrammable, 4: selectable fix
    #define BT1886_OETF_IDX 0
    // #define HLG_OETF_IDX 1
    #define SRGB_OETF_IDX 1 //2
    #define BT709_OETF_IDX 2 //2

#define Relay
#define MOBILE_PROJECT

#ifdef _PQ_SUPPORT_HCML_
    #define _CRT_SECURE_NO_WARNINGS
#endif

#define hist_bins  57
#define fw_stat_bins 33
#define Edit_Panel_Luminance 4000

#ifdef HDR_ANDROID_PLATFORM // Android Phone
    #include <android/log.h>
    #include <math.h>
    #include <stdio.h>
    #include <stdlib.h>
    #include <string.h>
    #ifdef _WIN32
        #define HDR_LOGD(fp, fmt, ...) ALOGD("%s: " fmt, __FUNCTION__, __VA_ARGS__)
    #else
        // #define HDR_LOGD(fp, fmt, arg...) ALOGD("%s: " fmt, __FUNCTION__, ##arg)
        #define HDR_LOGD(fp, fmt, ...) __android_log_print(ANDROID_LOG_DEBUG, "PQ_HDR", "%s: " fmt, __FUNCTION__, ##__VA_ARGS__ )
    #endif
#else
    #ifdef __Local_Sim__ // SRE C model
        #include <stdio.h>
        #include <stdlib.h>
        #include <string.h>
        #include <math.h>
        #include <memory.h>
        #include <time.h>

        // #define LOG(fp, fmt, ...) fprintf(fp, fmt, __VA_ARGS__)
        // #define HDR_Trace_LOG(fp, fmt, ...) fprintf(fp, fmt, __VA_ARGS__)
        #define HDR_LOGD(fp, fmt, ...) fprintf(fp, fmt, ##__VA_ARGS__)
    #else  // Linux TV
        #include "general.h"
        #include "typedef.h"
        #include "x_debug.h"
        #include "x_util.h"
        #include "x_assert.h"
        #include "nptv_debug.h"
        #include "hw_sw.h"
        #include "hw_predown.h"
        #include "drv_hdr_int.h"
    #endif
#endif


typedef unsigned int UINT32;
typedef int INT32;
typedef unsigned long long UINT64;

#ifndef x_memcpy
#define x_memcpy memcpy
#endif

#ifndef x_memset
#define x_memset memset
#endif

#ifndef MAX
#define MAX(a,b)  (((a) > (b)) ? (a) : (b))
#endif

#ifndef MIN
#define MIN(a,b)  (((a) < (b)) ? (a) : (b))
#endif

#ifndef ABS
#define ABS(a) (((a) < 0) ? (-(a)) : (a))
#endif


#define SW_ST2084_EOTF 0
#define SW_HLG_EOTF 1
#define SW_SLD_EOTF 2
#define SW_709_EOTF 3
#define SW_UNKOWN_EOTF 7

#define SW_BT709_OETF 0
#define SW_BT1886_OETF 1
#define SW_UNKOWN_OETF 7



enum HDRDebugFlags {
    eHDRDebugDisabled          = 0x0,
    eHDRDebugInput             = 0x1,
    eHDRDebugOutput            = 0x2,
    eHDRDebugTime              = 0x4,
    eHDRDebugContent_Frame     = 0x8,
    eHDRDebugContent_Stream    = 0x10,
    eHDRDebugAll               = 0xFF
};

typedef enum
{
    TRANSFER_CHARACTOR_ITURBT709           = 1,
    TRANSFER_CHARACTOR_UNSPECIFIED         = 2,
    TRANSFER_CHARACTOR_GAMMA2_2CURVE       = 4,
    TRANSFER_CHARACTOR_GAMMA2_8CURVE       = 5,
    TRANSFER_CHARACTOR_SMPTE170M           = 6,
    TRANSFER_CHARACTOR_SMPTE240M           = 7,
    TRANSFER_CHARACTOR_LINEAR              = 8,
    TRANSFER_CHARACTOR_LOG                 = 9,
    TRANSFER_CHARACTOR_SQRT                = 10,
    TRANSFER_CHARACTOR_IEC61966_2_4        = 11,
    TRANSFER_CHARACTOR_ITURBT1361          = 12,
    TRANSFER_CHARACTOR_IEC61966_2_1        = 13,
    TRANSFER_CHARACTOR_ITURBT2020_10       = 14,
    TRANSFER_CHARACTOR_ITURBT2020_12       = 15,
    TRANSFER_CHARACTOR_SMPTEST2084         = 16,
    TRANSFER_CHARACTOR_SMPTEST428_1        = 17,
    TRANSFER_CHARACTOR_ARIBSTD_B67         = 18,
    TRANSFER_CHARACTOR_RESVERD,
}TRANSFER_CHARACTOR_E;

typedef enum
{
    MATRIX_COEF_GBR                = 0,
    MATRIX_COEF_BT709              = 1,
    MATRIX_COEF_UNSPECIFIED        = 2,
    MATRIX_COEF_RESVERD            = 3,
    MATRIX_COEF_FCC                = 4, //NTSC
    MATRIX_COEF_BT470BG            = 5, //
    MATRIX_COEF_SMPTE170M          = 6, //NTSC 601
    MATRIX_COEF_SMPTE240M          = 7, //PAL/SECAM 601
    MATRIX_COEF_YCOCG              = 8,
    MATRIX_COEF_BT2020_NonCON      = 9,
    MATRIX_COEF_BT2020_CON         = 10,
}MATRIX_COEFFICIENT_E;

typedef enum
{
    COLOR_PRIMARIES_BT601 = 0,
    COLOR_PRIMARIES_BT709 = 1,
    COLOR_PRIMARIES_BT2020= 9,
    COLOR_PRIMARIES_DISPLAY_P3 = 11,
    COLOR_PRIMARIES_Reserved,
}COLOR_PRIMARIES_E;

// provided by video decoder
typedef struct __VDEC_DRV_COLORDESC_T
{
    // comes from sequence VUI
    UINT32 u4ColorPrimaries;                  // colour_primaries emum, 1 = REC.709, 9 = BT. 2020
    UINT32 u4TransformCharacter;              // transfer_characteristics emume, 1 = BT. 709, 14 = BT. 2020 10b, 16 = ST2084, 18 = HLG
    UINT32 u4MatrixCoeffs;                    // matrix_coeffs emum, 9 = BT. 2020 non-constant luminance, 10 = BT.2020 c

    // comes from HDR static metadata
    UINT32 u4DisplayPrimariesX[3];            // display_primaries_x
    UINT32 u4DisplayPrimariesY[3];            // display_primaries_y
    UINT32 u4WhitePointX;                     // white_point_x
    UINT32 u4WhitePointY;                     // white_point_y
    UINT32 u4MaxDisplayMasteringLuminance;    // max_display_mastering_luminance //1=1 nits
    UINT32 u4MinDisplayMasteringLuminance;    // min_display_mastering_luminance// 1= 0.0001 nits
    UINT32 u4MaxCLL;                          // max_content_light_level
    UINT32 u4MaxFALL;                         // max frame-average light level

} __VDEC_DRV_COLORDESC_T;

typedef struct iHDRHWReg
{
#if EOTF_TBL_TYPE == 0   // 0: only fix
    UINT32 sdr_gamma;    // for BT.2020 only, not for HDR
    UINT32 BBC_gamma;    // for HDR, HLG
#elif EOTF_TBL_TYPE == 1 // 1: only programmalbe
#elif EOTF_TBL_TYPE == 2 // 2: 1 fix + programmable
    UINT32 sdr_gamma;    // for BT.2020 only, not for HDR
#elif EOTF_TBL_TYPE == 3 // selectable fix + prgrammable
    UINT32 input_gamma;
#elif EOTF_TBL_TYPE == 4 // selectable fix
    UINT32 input_gamma;
#endif

    UINT32 reg_hist_en;

    UINT32 lbox_det_en;
    UINT32 UPpos;
    UINT32 DNpos;
} iHDRHWReg;

// provided by customer's panel, project dependent
typedef enum GAMUT
{
    REC709 = 0,
    BT2020 = 1,
    Others = 2, // enum will be increased by customer's panel spec
} GAMUT;

typedef struct PANEL_SPEC
{
    UINT32 panel_nits;
    enum GAMUT gamut;
} PANEL_SPEC;

// =================== feedback parameters per frame ================== //
typedef struct HDRFWFeedbackMemberVariable
{

    INT32 gain_curve_idx_1;
    INT32 tgt_nits, tgt_nits_1, tgt_nits_2;
    INT32 fw_tgt_nits, fw_tgt_nits_1;
    INT32 panel_sdr_nits, panel_sdr_nits_1;

    INT32 sdr_avg_70_100_percent_nits[fw_stat_bins], sdr_avg_70_100_percent_nits_1[fw_stat_bins];
    INT32 hist_70_100_percent_nits[fw_stat_bins], hist_70_100_percent_nits_1[fw_stat_bins];

    INT32 hw_src_nits, hw_src_nits_1, hw_src_nits_2;
    INT32 scene_change_flag, scene_change_flag_1;

    UINT32 maxFCLL, maxFCLL_1, reference_maxCLL;
    UINT32 idx;
    UINT32 iCurrFrmNum_Fw;

    INT32 reg_p[Tone_Map_POINTS + 1];
    INT32 reg_slope[Tone_Map_POINTS]; // local buffer

} HDRFWFeedbackMemberVariable;

// =================== HDR SW registers ================== //
typedef struct HDRFWReg
{
    UINT32 DebugFlags;

    UINT32 dynamic_rendering; // SW register, 1b

    UINT32 static_mode_gain_curve_idx; // 4b, 0~8

    UINT32 min_histogram_window;

    UINT32 gain_curve_boost_speed; // 8b, 0~255

    UINT32 hlg_gain_curve_idx; // 4b, 0~9

    UINT32 tgt_hist_idx_array[10]; // 0~32, 6b.  {20, 22, 24, 24, 25, 26, 27, 28, 29, 29}; //SW register

    UINT32 sw_p[17]; // 16+3 = 19b sw_p[Tone_Map_POINTS+1]
    UINT32 sw_slope[16]; // 12+4 = 16b sw_slope[Tone_Map_POINTS]

    UINT32 high_bright; // 32b, 0~400000, SW register

    UINT32 dark_scene_slope1, dark_scene_slope2, dark_scene_darken_rate; // 9b, 0~256
    UINT32 dark_scene_p1, dark_scene_p2; // 32b, 0~400000

    UINT32 normal_scene_slope1, normal_scene_slope2, normal_scene_darken_rate; // 9b, 0~256
    UINT32 normal_scene_p1, normal_scene_p2; // 32b, 0~400000

    UINT32 bright_scene_slope1, bright_scene_slope2; // 9b, 0~256
    UINT32 bright_scene_p1, bright_scene_p2; // 32b, 0~400000

    UINT32 non_bright_scene_slope, non_bright_scene_lighten_rate; // 9b, 0~256
    UINT32 non_bright_scene_p1, non_bright_scene_p2; // 32b, 0~400000

    UINT32 panel_nits_change_rate; // SW register, 9b, 0.03 = 15/512
    UINT32 tgt_nits_change_step; // SW register, 4b, 0~15
    UINT32 fade_hist_change_rate; // SW register, 9b, 0.06 = 31/512
    UINT32 fade_tgt_nits_change_rate; // SW register, 9b, 0.1 = 51/512
    UINT32 tgt_nits_assign_factor; // SW register, 5b, 0~16
    UINT32 fade_tgt_nits_assign_factor; // SW register, 5b, 0~16

    UINT32 tgt_nits_upper_limit; // SW register, 12b, 100~4095

    UINT32 nr_strength_b; // 4b, NR strength
    UINT32 mode_weight; // 4b, maxRGB and Y bleng

    UINT32 test_nits;

    struct __VDEC_DRV_COLORDESC_T HDR2SDR_STMDInfo;
    struct PANEL_SPEC panel_spec;

    UINT32 tone_mapping_truncate_mode;
    UINT32 min_p_end;

    // 3x3 matrix (gamut) 2'complement
    UINT32 c00;
    UINT32 c01;
    UINT32 c02;
    UINT32 c10;
    UINT32 c11;
    UINT32 c12;
    UINT32 c20;
    UINT32 c21;
    UINT32 c22;
    UINT32 gamut_matrix_en;

    UINT32 dynamic_mode_fix_gain_curve_en;
    UINT32 dynamic_mode_fixed_gain_curve_idx;

    UINT32 sw_eotf_table; // 0:st2084, 1:HLG, 2:SDR, 3: 709, others: random for RTL
    UINT32 sw_oetf_table; // 0:709, 1: 2.4, others: random for RTl

    UINT32 hlg_no_system_gamma; // 1b

    UINT32 BT709_c00;   // for BT.709 to panel gamut > sRGB
    UINT32 BT709_c01;
    UINT32 BT709_c02;
    UINT32 BT709_c10;
    UINT32 BT709_c11;
    UINT32 BT709_c12;
    UINT32 BT709_c20;
    UINT32 BT709_c21;
    UINT32 BT709_c22;
    UINT32 BT2020_c00;  // for BT.2020 to panel gamut > sRGB
    UINT32 BT2020_c01;
    UINT32 BT2020_c02;
    UINT32 BT2020_c10;
    UINT32 BT2020_c11;
    UINT32 BT2020_c12;
    UINT32 BT2020_c20;
    UINT32 BT2020_c21;
    UINT32 BT2020_c22;

    UINT32 low_flicker_mode_en;
    UINT32 low_flicker_mode_scene_change_nits_diff;
    UINT32 low_flicker_mode_different_scene_light_decrease;
    UINT32 low_flicker_mode_different_scene_light_increase;
    UINT32 low_flicker_mode_fade_decrease;
    UINT32 low_flicker_mode_fade_increase;
    UINT32 low_flicker_mode_same_scene_chase_gap;
    UINT32 low_flicker_mode_same_scene_chase_converge_period;
    UINT32 low_flicker_mode_same_scene_chase_max_speed;

    UINT32 xpercent_histogram_tuning_en;
    UINT32 max_hist_70_100_percent_nits;
    UINT32 approach_saturate_region;

    UINT32 sce_HDR2SDR; // EPCOT only, //0: PQ2SDR1886, 1: PQ2SDR709, 2: PQ to BT2020 HLG (new request 3), 3:709 SDR to BT2020 HLG (new request 0) 4:BT2020 SDR to BT20202 HLG (new request 1) 5: PQ to BT2020 SDR (new request 2) 6: HLG_to_HLG (new request 4)

    UINT32 target_nits; // read only register for FW

    UINT32 tone_mapping_fw_refine_test_en; // tone_mapping_fw_refine
    UINT32 source_VUI_debug_en;

    UINT32 eotf_accuracy_control_en;

    UINT32 protect_rate_256;
    UINT32 protect_rate_512;
    UINT32 protect_rate_1024;
    UINT32 protect_rate_2048;
    UINT32 maxFCLL_Queue_size;

    UINT32 tone_mapping_fw_refine_slope_n; // tone_mapping_fw_refine

    UINT32 curve_blending_low_ratio;
    UINT32 curve_blending_high_ratio;

    UINT32 sdr_nits_factor_4000; // 0~64, husky only
    UINT32 sdr_nits_factor_2048;
    UINT32 sdr_nits_factor_1024;
    UINT32 sdr_nits_factor_512;

    UINT32 low_flicker_mode_different_scene_nits_diff;

    UINT32 ccorr_en;

}HDRFWReg;

// input information for each frame, HW registers & histogram, streaming resolution change
typedef struct DHDRINPUT
{
    struct iHDRHWReg iHWReg;
    UINT32 cwidth, cheight;
    UINT32 resolution_change; // for streaming resolution change
    UINT32 RGBmaxHistogram_1[hist_bins];
    struct __VDEC_DRV_COLORDESC_T HDR2SDR_STMDInfo;
    struct PANEL_SPEC panel_spec;
    bool ccorr_en;
    struct HDRFWFeedbackMemberVariable HDRFW_FBMV;
}DHDRINPUT;

// output information for each frame HW setting
typedef struct DHDROUTPUT
{
    UINT32 GainCurve[Gain_Curve_POINTS];

    UINT32 reg_p1;
    UINT32 reg_p2;
    UINT32 reg_p3;
    UINT32 reg_p4;
    UINT32 reg_p5;
    UINT32 reg_slope0;
    UINT32 reg_slope1;
    UINT32 reg_slope2;
    UINT32 reg_slope3;
    UINT32 reg_slope4;

#if Tone_Map_POINTS == 16
    UINT32 reg_p6;
    UINT32 reg_p7;
    UINT32 reg_p8;
    UINT32 reg_p9;
    UINT32 reg_p10;
    UINT32 reg_p11;
    UINT32 reg_p12;
    UINT32 reg_p13;
    UINT32 reg_p14;
    UINT32 reg_p15;
    UINT32 reg_p16;
    UINT32 reg_slope5;
    UINT32 reg_slope6;
    UINT32 reg_slope7;
    UINT32 reg_slope8;
    UINT32 reg_slope9;
    UINT32 reg_slope10;
    UINT32 reg_slope11;
    UINT32 reg_slope12;
    UINT32 reg_slope13;
    UINT32 reg_slope14;
    UINT32 reg_slope15;
#endif

    UINT32 hist_begin_y;
    UINT32 hist_end_y;
    UINT32 hist_begin_x;
    UINT32 hist_end_x;

    UINT32 in_hsize;
    UINT32 in_vsize;

  // YUV2RGB
#ifdef Y2R_INPUT_FORMAT_SELECT
    UINT32 y2r_input_format;

    UINT32 y2r_c00;
    UINT32 y2r_c01;
    UINT32 y2r_c02;
    UINT32 y2r_c10;
    UINT32 y2r_c11;
    UINT32 y2r_c12;
    UINT32 y2r_c20;
    UINT32 y2r_c21;
    UINT32 y2r_c22;
    UINT32 y2r_pre_add_0;
    UINT32 y2r_pre_add_1;
    UINT32 y2r_pre_add_2;
    UINT32 y2r_post_add_0;
    UINT32 y2r_post_add_1;
    UINT32 y2r_post_add_2;
#else
    UINT32 bt2020_in;
    UINT32 bt2020_const_luma;
#endif

    // 3x3 matrix (gamut) 2'complement
    UINT32 c00;
    UINT32 c01;
    UINT32 c02;
    UINT32 c10;
    UINT32 c11;
    UINT32 c12;
    UINT32 c20;
    UINT32 c21;
    UINT32 c22;
    UINT32 gamut_matrix_en;

    // lbox detection
    UINT32 UPpos;
    UINT32 DNpos;

    // histogram
    UINT32 reg_hist_en;

    // Tone Mapping
    UINT32 tone_map_en;

    // GainCurve
    UINT32 reg_luma_gain_en;

    // NR-B
    UINT32 reg_nr_strength;
    UINT32 reg_filter_no;
    UINT32 reg_NR_en;

    // Adaptive luminance control
    UINT32 reg_maxRGB_weight;

   // SCE
#ifdef R2Y_OUTPUT_FORMAT_SELECT
    UINT32 r2y_output_format;
    UINT32 r2y_c00;
    UINT32 r2y_c01;
    UINT32 r2y_c02;
    UINT32 r2y_c10;
    UINT32 r2y_c11;
    UINT32 r2y_c12;
    UINT32 r2y_c20;
    UINT32 r2y_c21;
    UINT32 r2y_c22;
    UINT32 r2y_pre_add_0;
    UINT32 r2y_pre_add_1;
    UINT32 r2y_pre_add_2;
    UINT32 r2y_post_add_0;
    UINT32 r2y_post_add_1;
    UINT32 r2y_post_add_2;
#endif

    UINT32 hdr_en;

#ifdef Relay
    UINT32 hdr_relay;
#else
    UINT32 hdr_bypass;
#endif

#if EOTF_TBL_TYPE == 0 // 0:only fix
    UINT32 BBC_gamma;
    UINT32 sdr_gamma;
#elif EOTF_TBL_TYPE == 1 // 1:only programmalbe
    UINT32 programmable_EOTF[3][EOTF_POINTS]; // should cooperate with reg: eotf_table_sep_mode = 0 to reduce bandwidth
    UINT32 last_eotf_elementR;
    UINT32 last_eotf_posR;
    UINT32 last_eotf_elementG;
    UINT32 last_eotf_posG;
    UINT32 last_eotf_elementB;
    UINT32 last_eotf_posB;
    UINT32 eotf_accuracy_control_P1;
    UINT32 eotf_accuracy_control_P2;
    UINT32 eotf_accuracy_control_P3;
    UINT32 eotf_accuracy_control_P4;
#elif EOTF_TBL_TYPE == 2 //2: 1 fix + programmable
    UINT32 sdr_gamma;
    UINT32 programmable_EOTF[EOTF_POINTS];
    UINT32 last_eotf_element;
    UINT32 last_eotf_pos;
#elif EOTF_TBL_TYPE == 3 //3: selectable fix + prgrammable
    UINT32 input_gamma;
    UINT32 programmable_EOTF[EOTF_POINTS];
    UINT32 last_eotf_element;
    UINT32 last_eotf_pos;
#elif EOTF_TBL_TYPE == 4 //4: selectable fix
    UINT32 input_gamma;
#endif


#if OETF_TBL_TYPE == 0 //0:only fix
#elif OETF_TBL_TYPE == 1 //1: only programmalbe
    UINT32 programmable_OETF[3][OETF_POINTS];
#elif OETF_TBL_TYPE == 2 //2: 1 fix + programmable
#elif OETF_TBL_TYPE == 3 //3: selectable fix + prgrammable
    UINT32 output_gamma;
    UINT32 programmable_OETF[OETF_POINTS];
#elif OETF_TBL_TYPE == 4 //4: selectable fix
    UINT32 output_gamma;
#endif

    UINT32 ccorr_en;
    struct HDRFWFeedbackMemberVariable HDRFW_FBMV;

} DHDROUTPUT;


/* ........begin Dynamic HDR Rendering, local parameters......... */
typedef struct HDRFWMemberVariable
{
    INT32 gain_curve_idx_1;
    INT32 tgt_nits, tgt_nits_1, tgt_nits_2;
    INT32 fw_tgt_nits, fw_tgt_nits_1;
    INT32 panel_sdr_nits, panel_sdr_nits_1;

    INT32 sdr_avg_70_100_percent_nits_buf0[fw_stat_bins];
    INT32 sdr_avg_70_100_percent_nits_buf1[fw_stat_bins];
    INT32 hist_70_100_percent_nits_buf0[fw_stat_bins];
    INT32 hist_70_100_percent_nits_buf1[fw_stat_bins];
    INT32 *sdr_avg_70_100_percent_nits, *sdr_avg_70_100_percent_nits_1;
    INT32 *hist_70_100_percent_nits, *hist_70_100_percent_nits_1;

    INT32 Hist_nits[hist_bins];
    INT32 Hist_pdf[hist_bins], Hist_cdf[hist_bins];
    INT32 Hist_avg_nits[hist_bins],Hist_sdr_avg_nits[hist_bins];
    INT32 *sdr_avg_70_100_percent_nits_tmp, *hist_70_100_percent_nits_tmp;

    INT32 width, height;
    INT32 panel_nits;
    INT32 hw_src_nits, hw_src_nits_1, hw_src_nits_2;
    INT32 scene_change_flag, scene_change_flag_1;

    UINT32 maxFCLL, maxFCLL_1, reference_maxCLL;
    UINT32 idx, maxFCLL_Queue[32];
    UINT32 iCurrFrmNum_Fw;

    UINT32 GainCurve[Gain_Curve_POINTS]; // local buffer
    INT32 reg_p[Tone_Map_POINTS+1];
    INT32 reg_slope[Tone_Map_POINTS]; // local buffer

    char out_buffer[600];
    char tmp_buffer[256];
    char out_buffer2[600];

} HDRFWMemberVariable;


#ifdef MOBILE_PROJECT

class CPQHDRFW
{
public:
#endif

#ifndef MOBILE_PROJECT
    void vDrvHDRInitCommon(struct HDRFWMemberVariable *pHDRFWMV, struct HDRFWReg *pHDRFWReg); // timing based, video
    void vDrvHDRInitPlatform(DHDRINPUT *input, DHDROUTPUT *output, struct HDRFWMemberVariable *pHDRFWMV, struct HDRFWReg *pHDRFWReg); //stream based
    void vDrvHDRCalculate(DHDRINPUT *input, DHDROUTPUT *output, struct HDRFWMemberVariable *pHDRFWMV, struct HDRFWReg *pHDRFWReg); //frame based, ISR
#else
    void onInitCommon(void);
    void onInitPlatform(DHDRINPUT *input, DHDROUTPUT *output);
    void onInitPlatform(void);  // for customer setting
    void onCalculate(DHDRINPUT *input, DHDROUTPUT *output);
#endif

    void setting_by_SWreg(DHDROUTPUT *output, struct HDRFWMemberVariable *pHDRFWMV, struct HDRFWReg *pHDRFWReg);
    void setting_by_feedbackMV(DHDRINPUT *input, struct HDRFWMemberVariable *pHDRFWMV);
    void feedbackMV_setting(struct HDRFWMemberVariable *pHDRFWMV, DHDROUTPUT *output);
    void tone_mapping_fw(DHDRINPUT *input, struct HDRFWMemberVariable *pHDRFWMV, struct HDRFWReg *pHDRFWReg);
    void tone_mapping_fw_refine(struct HDRFWMemberVariable *pHDRFWMV, struct HDRFWReg *pHDRFWReg);
    void tone_mapping_hw_setting(DHDROUTPUT *output, struct HDRFWMemberVariable *pHDRFWMV);
    void tone_mapping_sw_InitSetting(OUT DHDRINPUT *input, OUT DHDROUTPUT *output, IN struct HDRFWMemberVariable *pHDRFWMV);
    void gain_curve_gen_fw(DHDRINPUT *input, struct HDRFWMemberVariable *pHDRFWMV, struct HDRFWReg *pHDRFWReg);
    void gain_curve_hw_setting(DHDROUTPUT *output, struct HDRFWMemberVariable *pHDRFWMV);
    void histogram_window_fw(DHDRINPUT *input, DHDROUTPUT *output, struct HDRFWReg *pHDRFWReg);
    INT32 _set2sCompNum(INT32 val, INT32 bits);
    INT32 _Get2sCompNum(INT32 val, INT32 bits);

#if (EOTF_TBL_TYPE == 1 || EOTF_TBL_TYPE == 2 || EOTF_TBL_TYPE == 3)
    void st2084_eotf_table(DHDROUTPUT *output, INT32 factor);
    void st2084_eotf_shift_table(DHDROUTPUT *output); // 16bits LUT store 20b 4000 nits curve
    void HLG_eotf_table(DHDROUTPUT *output);
    void HLG_eotf_shift_table(DHDROUTPUT *output);
    void BT709_eotf_table(DHDROUTPUT *output);
    void BT709_eotf_shift_table(DHDROUTPUT *output);
    void BT709_oetf_table(DHDROUTPUT *output);
    void BT1886_oetf_table(DHDROUTPUT *output);
#endif

#ifdef MOBILE_PROJECT
    HDRFWMemberVariable *pHDRFWMV;
    HDRFWMemberVariable vHDRFWMV;

    HDRFWReg *pHDRFWReg;
    HDRFWReg vHDRFWReg;

    CPQHDRFW()
    {
        pHDRFWReg = &vHDRFWReg;
        pHDRFWMV = &vHDRFWMV;
        onInitCommon();
    }
    ~CPQHDRFW();

    void setDebugFlag(unsigned int debugFlag)
    {
        pHDRFWReg->DebugFlags = debugFlag;
    }

};
#endif


#ifdef __Local_Sim__
extern FILE *out_fp_stat;
extern FILE *out_fp_stat_scene;
#endif

#ifndef MOBILE_PROJECT
extern HDRFWReg *pHDRFWReg;
extern HDRFWMemberVariable *pHDRFWMV;
#ifndef __MAIN_PATH_ONLY__
extern HDRFWReg *pHDRFWReg_SUB;
extern HDRFWMemberVariable *pHDRFWMV_SUB;
#endif
#endif


#ifdef __cplusplus
}
#endif

#endif /* _Module_HDR_H */



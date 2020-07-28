#ifndef __PQDSIMPL_H__
#define __PQDSIMPL_H__

/*
* header files
*/
#include "PQCZTuning.h"

#define DSHP_ANDROID_PLATFORM

#define VER_6755  0
#define VER_6797  1
#define VER_6771  2

#ifndef DSHP_ANDROID_PLATFORM
  #define DYN_SHARP_VERSION VER_6771
#else
   //to be defined at MDP SW layer
#endif

#if (DYN_SHARP_VERSION == VER_6755)
   #define DS_BYPASS_EDS
   #define DS_BYPASS_2DGC
   #define DS_BYPASS_CB
   #define DS_BYPASS_POSTYLEV
#elif (DYN_SHARP_VERSION == VER_6797)
   #define DS_BYPASS_POSTYLEV
#elif (DYN_SHARP_VERSION == VER_6771)
#else
   #define DS_BYPASS_EDS
   #define DS_BYPASS_2DGC
   #define DS_BYPASS_CB
   #define DS_BYPASS_POSTYLEV
#endif

// for ClearZoom
#define ISP_SCENARIO_CNT (3)  // 0: Capture(single frame), 1: Capture(multi-frame), 2; preview
#define LENS_TYPE_CNT (4)
#define ISP_SCENARIOS (13) //ISP_SCENARIO_CNT*LENS_TYPE_CNT + 1(DPE)
#define UR_MAX_LEVEL (11)

enum DSDebugFlags {
    eDSDebugDisabled          = 0x0,
    eDSDebugEn                = 0x1,
    eDSDebugUR                = 0x2,
    eDSDebugIsoAdp            = 0x4,
    eDSDebugAll               = 0xFF
};

typedef unsigned int uint32_t;
typedef unsigned short uint16_t;
typedef unsigned char uint8_t;
typedef signed int int32_t;

// Initial register values to DS HW
struct DSInitReg {
    uint32_t useless; // To pass compilation
};

struct DSHWReg {
    uint32_t tdshp_softcoring_gain;
    uint32_t tdshp_gain_high;
    uint32_t tdshp_gain_mid;
    uint32_t tdshp_ink_sel;
    uint32_t tdshp_bypass_high;
    uint32_t tdshp_bypass_mid;
    uint32_t tdshp_en;
    uint32_t tdshp_limit_ratio;
    uint32_t tdshp_gain;
    uint32_t tdshp_coring_zero;
    uint32_t tdshp_coring_thr;
    uint32_t tdshp_coring_value;
    uint32_t tdshp_bound;
    uint32_t tdshp_limit;
    uint32_t tdshp_sat_proc;
    uint32_t tdshp_ac_lpf_coe;
    uint32_t tdshp_clip_thr;
    uint32_t tdshp_clip_ratio;
    uint32_t tdshp_clip_en;
    uint32_t tdshp_ylev_p048;
    uint32_t tdshp_ylev_p032;
    uint32_t tdshp_ylev_p016;
    uint32_t tdshp_ylev_p000;
    uint32_t tdshp_ylev_p112;
    uint32_t tdshp_ylev_p096;
    uint32_t tdshp_ylev_p080;
    uint32_t tdshp_ylev_p064;
    uint32_t tdshp_ylev_p176;
    uint32_t tdshp_ylev_p160;
    uint32_t tdshp_ylev_p144;
    uint32_t tdshp_ylev_p128;
    uint32_t tdshp_ylev_p240;
    uint32_t tdshp_ylev_p224;
    uint32_t tdshp_ylev_p208;
    uint32_t tdshp_ylev_p192;
    uint32_t tdshp_ylev_en;
    uint32_t tdshp_ylev_alpha;
    uint32_t tdshp_ylev_256;

    uint32_t pbc1_radius_r;
    uint32_t pbc1_theta_r;
    uint32_t pbc1_rslope_1;
    uint32_t pbc1_gain;
    uint32_t pbc1_lpf_en;
    uint32_t pbc1_en;
    uint32_t pbc1_lpf_gain;
    uint32_t pbc1_tslope;
    uint32_t pbc1_radius_c;
    uint32_t pbc1_theta_c;
    uint32_t pbc1_edge_slope;
    uint32_t pbc1_edge_thr;
    uint32_t pbc1_edge_en;
    uint32_t pbc1_conf_gain;
    uint32_t pbc1_rslope;

    uint32_t pbc2_radius_r;
    uint32_t pbc2_theta_r;
    uint32_t pbc2_rslope_1;
    uint32_t pbc2_gain;
    uint32_t pbc2_lpf_en;
    uint32_t pbc2_en;
    uint32_t pbc2_lpf_gain;
    uint32_t pbc2_tslope;
    uint32_t pbc2_radius_c;
    uint32_t pbc2_theta_c;
    uint32_t pbc2_edge_slope;
    uint32_t pbc2_edge_thr;
    uint32_t pbc2_edge_en;
    uint32_t pbc2_conf_gain;
    uint32_t pbc2_rslope;

    uint32_t pbc3_radius_r;
    uint32_t pbc3_theta_r;
    uint32_t pbc3_rslope_1;
    uint32_t pbc3_gain;
    uint32_t pbc3_lpf_en;
    uint32_t pbc3_en;
    uint32_t pbc3_lpf_gain;
    uint32_t pbc3_tslope;
    uint32_t pbc3_radius_c;
    uint32_t pbc3_theta_c;
    uint32_t pbc3_edge_slope;
    uint32_t pbc3_edge_thr;
    uint32_t pbc3_edge_en;
    uint32_t pbc3_conf_gain;
    uint32_t pbc3_rslope;

#ifndef DS_BYPASS_2DGC
    uint32_t  tdshp_mid_softlimit_ratio;
    uint32_t  tdshp_mid_coring_zero;
    uint32_t  tdshp_mid_coring_thr;
    uint32_t  tdshp_mid_softcoring_gain;
    uint32_t  tdshp_mid_coring_value;
    uint32_t  tdshp_mid_bound;
    uint32_t  tdshp_mid_limit;

    uint32_t  tdshp_high_softlimit_ratio;
    uint32_t  tdshp_high_coring_zero;
    uint32_t  tdshp_high_coring_thr;
    uint32_t  tdshp_high_softcoring_gain;
    uint32_t  tdshp_high_coring_value;
    uint32_t  tdshp_high_bound;
    uint32_t  tdshp_high_limit;
#endif //DS_BYPASS_2DGC

#ifndef DS_BYPASS_EDS
    uint32_t  edf_clip_ratio_inc;
    uint32_t  edf_edge_gain;
    uint32_t  edf_detail_gain;
    uint32_t  edf_flat_gain;
    uint32_t  edf_gain_en;

    uint32_t  edf_edge_th;
    uint32_t  edf_detail_fall_th;
    uint32_t  edf_detail_rise_th;
    uint32_t  edf_flat_th;

    uint32_t  edf_edge_slope;
    uint32_t  edf_detail_fall_slope;
    uint32_t  edf_detail_rise_slope;
    uint32_t  edf_flat_slope;

    uint32_t  edf_edge_mono_slope;
    uint32_t  edf_edge_mono_th;
    uint32_t  edf_edge_mag_slope;
    uint32_t  edf_edge_mag_th;

    uint32_t  edf_edge_trend_flat_mag;
    uint32_t  edf_edge_trend_slope;
    uint32_t  edf_edge_trend_th;

    uint32_t  edf_bld_wgt_mag;
    uint32_t  edf_bld_wgt_mono;
    uint32_t  edf_bld_wgt_trend;
#endif //DS_BYPASS_EDS

#ifndef DS_BYPASS_CB
    uint32_t  tdshp_cboost_lmt_u;
    uint32_t  tdshp_cboost_lmt_l;
    uint32_t  tdshp_cboost_en;
    uint32_t  tdshp_cboost_gain;
    uint32_t  tdshp_cboost_yconst;
    uint32_t  tdshp_cboost_yoffset_sel;
    uint32_t  tdshp_cboost_yoffset;
#endif

#ifndef DS_BYPASS_POSTYLEV
    uint32_t tdshp_post_ylev_p048;
    uint32_t tdshp_post_ylev_p032;
    uint32_t tdshp_post_ylev_p016;
    uint32_t tdshp_post_ylev_p000;
    uint32_t tdshp_post_ylev_p112;
    uint32_t tdshp_post_ylev_p096;
    uint32_t tdshp_post_ylev_p080;
    uint32_t tdshp_post_ylev_p064;
    uint32_t tdshp_post_ylev_p176;
    uint32_t tdshp_post_ylev_p160;
    uint32_t tdshp_post_ylev_p144;
    uint32_t tdshp_post_ylev_p128;
    uint32_t tdshp_post_ylev_p240;
    uint32_t tdshp_post_ylev_p224;
    uint32_t tdshp_post_ylev_p208;
    uint32_t tdshp_post_ylev_p192;
    uint32_t tdshp_post_ylev_en;
    uint32_t tdshp_post_ylev_alpha;
    uint32_t tdshp_post_ylev_256;
#endif //DS_BYPASS_POSTYLEV
};

// Fields collected from DS HW
struct DSInput {
    uint32_t SrcWidth;
    uint32_t SrcHeight;
    uint32_t DstWidth;
    uint32_t DstHeight;

    uint32_t VideoImgSwitch; // 0: Video, 1: Image

    uint32_t inISOSpeed;
    struct DSHWReg iHWReg;

    //ClearZoom input
    uint32_t iIspScenario; //Assign the current ISP scenario from the 12 candidates
    uint32_t iIsIspScenario;

    //DebugFlag
    uint32_t DebugFlagsDS;
};

// Fields which will be set to HW registers
struct DSOutput {
    struct DSHWReg iHWReg;
};

// DS FW registers
struct DSReg {
    int32_t DS_en;

    int32_t iUpSlope;        // Range from -64 to 63
    int32_t iUpThreshold;    // Range from 0 to 4095
    int32_t iDownSlope;      // Range from -64 to 63
    int32_t iDownThreshold;  // Range from 0 to 4095

    uint32_t iISO_en;
    uint32_t iISO_thr1;
    uint32_t iISO_thr0;
    uint32_t iISO_thr3;
    uint32_t iISO_thr2;
    uint32_t iISO_IIR_alpha;

    int32_t iCorZero_clip2; //Range from -255 to 255
    int32_t iCorZero_clip1; //Range from -255 to 255
    int32_t iCorZero_clip0; //Range from -255 to 255
    int32_t iCorThr_clip2; //Range from -255 to 255
    int32_t iCorThr_clip1; //Range from -255 to 255
    int32_t iCorThr_clip0; //Range from -255 to 255
    int32_t iCorGain_clip2; //Range from -255 to 255
    int32_t iCorGain_clip1; //Range from -255 to 255
    int32_t iCorGain_clip0; //Range from -255 to 255
    int32_t iGain_clip2; //Range from -255 to 255
    int32_t iGain_clip1; //Range from -255 to 255
    int32_t iGain_clip0; //Range from -255 to 255

#ifndef DS_BYPASS_2DGC
    int32_t iHighCorZero_clip2; //Range from -255 to 255
    int32_t iHighCorZero_clip1; //Range from -255 to 255
    int32_t iHighCorZero_clip0; //Range from -255 to 255
    int32_t iHighCorThr_clip2; //Range from -255 to 255
    int32_t iHighCorThr_clip1; //Range from -255 to 255
    int32_t iHighCorThr_clip0; //Range from -255 to 255
    int32_t iHighCorGain_clip2; //Range from -255 to 255
    int32_t iHighCorGain_clip1; //Range from -255 to 255
    int32_t iHighCorGain_clip0; //Range from -255 to 255

    int32_t iMidCorZero_clip2; //Range from -255 to 255
    int32_t iMidCorZero_clip1; //Range from -255 to 255
    int32_t iMidCorZero_clip0; //Range from -255 to 255
    int32_t iMidCorThr_clip2; //Range from -255 to 255
    int32_t iMidCorThr_clip1; //Range from -255 to 255
    int32_t iMidCorThr_clip0; //Range from -255 to 255
    int32_t iMidCorGain_clip2; //Range from -255 to 255
    int32_t iMidCorGain_clip1; //Range from -255 to 255
    int32_t iMidCorGain_clip0; //Range from -255 to 255
#endif //#ifndef DS_BYPASS_2DGC

#ifndef DS_BYPASS_EDS
    int32_t i_edf_flat_th_clip2; //Range from -255 to 255
    int32_t i_edf_flat_th_clip1; //Range from -255 to 255
    int32_t i_edf_flat_th_clip0; //Range from -255 to 255
    int32_t i_edf_detail_rise_th_clip2; //Range from -255 to 255
    int32_t i_edf_detail_rise_th_clip1; //Range from -255 to 255
    int32_t i_edf_detail_rise_th_clip0; //Range from -255 to 255
#endif //#ifndef DS_BYPASS_EDS

    //UltraResolution registers
    int32_t  iUltraRes_en; //u1.0
    int32_t  iUltraRes_ratio_thr2; //u16.0
    int32_t  iUltraRes_ratio_thr1; //u16.0
    int32_t  iUltraRes_ratio_thr0; //u16.0
    int32_t  iUltraRes_gain_mid_clip1; //9-bit s8.0
    int32_t  iUltraRes_gain_mid_clip0; //9-bit s8.0
    int32_t  iUltraRes_gain_high_clip1; //9-bit s8.0
    int32_t  iUltraRes_gain_high_clip0; //9-bit s8.0
#ifndef DS_BYPASS_EDS
    int32_t  iUltraRes_edf_edge_gain_clip1; //9-bit s8.0
    int32_t  iUltraRes_edf_edge_gain_clip0; //9-bit s8.0
    int32_t  iUltraRes_edf_detail_gain_clip1; //9-bit s8.0
    int32_t  iUltraRes_edf_detail_gain_clip0; //9-bit s8.0
    int32_t  iUltraRes_edf_flat_gain_clip1; //9-bit s8.0
    int32_t  iUltraRes_edf_flat_gain_clip0; //9-bit s8.0
#endif //#ifndef DS_BYPASS_EDS
    int32_t  iUltraRes_clip_thr_clip1; //9-bit s8.0
    int32_t  iUltraRes_clip_thr_clip0; //9-bit s8.0
    int32_t  iUltraRes_clip_ratio_clip1; //9-bit s8.0
    int32_t  iUltraRes_clip_ratio_clip0; //9-bit s8.0

};

// HFG FW compute I/O structure
struct HFGInput_tile
{
    // MDP tile imformation
    uint32_t tile_width;
    uint32_t tile_height;
    uint32_t tile_x_ofst;
    uint32_t tile_y_ofst;
};
struct HFGOutput_tile
{
    // [SL2 reg]
    uint32_t SL2_X_OFST;
    uint32_t SL2_Y_OFST;
    uint32_t SL2_TPIPE_WD;
    uint32_t SL2_TPIPE_HT;
    // [HFG_crop reg]
    uint32_t HFG_CROP_XSTART;
    uint32_t HFG_CROP_XEND;
    uint32_t HFG_CROP_YSTART;
    uint32_t HFG_CROP_YEND;
    // [HFG reg]
    uint32_t HFG_X_OFST;
    uint32_t HFG_Y_OFST;
    uint32_t HFG_TILE_WD;
    uint32_t HFG_TILE_HT;
};

struct HFGInput_frame
{
    uint32_t MDP_input_width;   // for MDP_resize_ratio compute, original image width
    uint32_t MDP_input_height;  // for MDP_resize_ratio compute, original image height
    uint32_t MDP_output_width;  // for MDP_resize_ratio compute, after MDP resizing image width  (HFG input image width)
    uint32_t MDP_output_height; // for MDP_resize_ratio compute, after MDP resizing image height (HFG input image height)
    uint32_t MDP_crop_x;        // MDP crop position on the x directions (just backup for debug, no use in functions)
    uint32_t MDP_crop_y;        // MDP crop position on the y directions (just backup for debug, no use in functions)
    // SLK reg (In HFG module, SL2 = SLK)
    uint32_t SLK_HRZ_COMP;
    uint32_t SLK_VRZ_COMP;
    uint32_t SLK_CENTR_X;
    uint32_t SLK_CENTR_Y;
    uint32_t SLK_R_0;
    uint32_t SLK_R_1;
    uint32_t SLK_R_2;
    uint32_t SLK_GAIN_0;
    uint32_t SLK_GAIN_1;
    uint32_t SLK_GAIN_2;
    uint32_t SLK_GAIN_3;
    uint32_t SLK_GAIN_4;
    uint32_t SLK_SLP_1;
    uint32_t SLK_SLP_2;
    uint32_t SLK_SLP_3;
    uint32_t SLK_SLP_4;
    uint32_t SLK_SET_ZERO;
    // HFG reg
    uint32_t HFG_NOISE_GSEED;
    uint32_t HFG_IMAGE_WD;
    uint32_t HFG_NOISE_SEED0;
    uint32_t HFG_NOISE_SEED1;
    uint32_t HFG_NOISE_SEED2;
    // HFC reg
    uint32_t HFC_NOISE_STD;
    uint32_t HFC_SL2_LINK_EN;
    uint32_t HFC_LUMA_CPX1;
    uint32_t HFC_LUMA_CPX2;
    uint32_t HFC_LUMA_CPX3;
    uint32_t HFC_LUMA_CPY0;
    uint32_t HFC_LUMA_CPY1;
    uint32_t HFC_LUMA_CPY2;
    uint32_t HFC_LUMA_CPY3;
    uint32_t HFC_LUMA_SP0;
    uint32_t HFC_LUMA_SP1;
    uint32_t HFC_LUMA_SP2;
    uint32_t HFC_LUMA_SP3;
    uint32_t HFC_SL2_CPX1;
    uint32_t HFC_SL2_CPX2;
    uint32_t HFC_SL2_CPX3;
    uint32_t HFC_SL2_CPY0;
    uint32_t HFC_SL2_CPY1;
    uint32_t HFC_SL2_CPY2;
    uint32_t HFC_SL2_CPY3;
    uint32_t HFC_SL2_SP0;
    uint32_t HFC_SL2_SP1;
    uint32_t HFC_SL2_SP2;
    uint32_t HFC_SL2_SP3;
};
struct HFGOutput_frame
{
    // [SL2 reg]
    uint32_t SL2_HRZ_COMP;
    uint32_t SL2_VRZ_COMP;
    // bypass
    uint32_t SL2_CENTR_X;
    uint32_t SL2_CENTR_Y;
    uint32_t SL2_R_0;
    uint32_t SL2_R_1;
    uint32_t SL2_R_2;
    uint32_t SL2_GAIN_0;
    uint32_t SL2_GAIN_1;
    uint32_t SL2_GAIN_2;
    uint32_t SL2_GAIN_3;
    uint32_t SL2_GAIN_4;
    uint32_t SL2_SLP_1;
    uint32_t SL2_SLP_2;
    uint32_t SL2_SLP_3;
    uint32_t SL2_SLP_4;
    uint32_t SL2_SET_ZERO;
    // [HFG reg]
    uint32_t HFG_NOISE_GSEED;
    uint32_t HFG_IMAGE_WD;
    // bypass
    uint32_t HFG_NOISE_SEED0;
    uint32_t HFG_NOISE_SEED1;
    uint32_t HFG_NOISE_SEED2;
    // [HFC reg] all bypass
    uint32_t HFC_NOISE_STD;
    uint32_t HFC_SL2_LINK_EN;
    uint32_t HFC_LUMA_CPX1;
    uint32_t HFC_LUMA_CPX2;
    uint32_t HFC_LUMA_CPX3;
    uint32_t HFC_LUMA_CPY0;
    uint32_t HFC_LUMA_CPY1;
    uint32_t HFC_LUMA_CPY2;
    uint32_t HFC_LUMA_CPY3;
    uint32_t HFC_LUMA_SP0;
    uint32_t HFC_LUMA_SP1;
    uint32_t HFC_LUMA_SP2;
    uint32_t HFC_LUMA_SP3;
    uint32_t HFC_SL2_CPX1;
    uint32_t HFC_SL2_CPX2;
    uint32_t HFC_SL2_CPX3;
    uint32_t HFC_SL2_CPY0;
    uint32_t HFC_SL2_CPY1;
    uint32_t HFC_SL2_CPY2;
    uint32_t HFC_SL2_CPY3;
    uint32_t HFC_SL2_SP0;
    uint32_t HFC_SL2_SP1;
    uint32_t HFC_SL2_SP2;
    uint32_t HFC_SL2_SP3;
};


///////////////////////////////////////////////////////////////////////////////
// DS FW Processing class
///////////////////////////////////////////////////////////////////////////////
class CPQDSFW
{

    /* ........Dynamic Sharpness Process, functions......... */
public:

#ifdef DSHP_ANDROID_PLATFORM
    void onCalculate(const DSInput *input, DSOutput *output);
    void onInitPlatform(void);
#else
    void vDrvDSProc_int(const DSInput * input, DSOutput * output);
#endif
    void onLoadTable(DSInput *input);
    void onLoadTable(DSInput *input, void* NvRamTable);


    void DSInitialize(void);
    void DSRegInit  (DSReg   *pDSReg  );
    void DSHWRegInit(DSHWReg *pDSHWReg);

    void DSGetScalingRatio(const DSInput *input);
    void DSCore(const DSInput *input, DSOutput * output);

    void UltraResolution(const DSInput *input, DSOutput * output);

    void ISOAdaptive(const DSInput *input, DSOutput * output);

    int  DSSyncCorGain(int gain_before, int gain_after, int cor_gain_before);

    //int32_t IsoAdpGetReg(int32_t delta_iso, int32_t reg, int32_t reg_clip, int32_t reg_slope, int32_t reg_shift, int32_t reg_min, int32_t reg_max);
    int max2(int data1, int data2);
    int min2(int data1, int data2);
    int AlphaBlend(int data1, int data2, int wgt, int wgt_max);

    // [HFG FW]
    void HFG_FW_compute_tile(const HFGInput_tile *input, HFGOutput_tile * output);
    void HFG_FW_compute_frame(const HFGInput_frame *input, HFGOutput_frame * output);
    uint32_t fw_SL2_HRZ_COMP;
    uint32_t fw_SL2_VRZ_COMP;

    int32_t UltraResGetReg(
                       int32_t in_scl_ratio,
                       int32_t in_value,
                       int32_t reg_ultra_res_thr0,
                       int32_t reg_ultra_res_thr1,
                       int32_t reg_ultra_res_thr2,
                       int32_t reg_clip1,
                       int32_t reg_clip2,
                       int32_t reg_min,
                       int32_t reg_max
                       );
    int32_t IsoAdpGetReg(
                  int32_t in_iso,
                  int32_t in_value,
                  int32_t reg_iso_thr0,
                  int32_t reg_iso_thr1,
                  int32_t reg_iso_thr2,
                  int32_t reg_iso_thr3,
                  int32_t reg_clip1,
                  int32_t reg_clip2,
                  int32_t reg_clip3,
                  int32_t reg_min,
                  int32_t reg_max
                  );
    int32_t IsoAdpGetCorValue(int32_t zero_thr, int32_t coring_thr, int32_t gain, int32_t cor_gain);
    int32_t IsoAdpAlphaBlending(int32_t data1, int32_t data2, int32_t alpha, int32_t bits);

    int IsoAdpGetCorGain(int gain, int cor_gain, int cor_zero, int cor_thr, int cor_value, int wgt_cor_gain);

    int DumpReg(void* RegBuffer, char* DumpBuffer, int& BufferCount,  int MaxBufferSize, int RegNum);

    CPQDSFW();
    ~CPQDSFW();

private:

    /* ........Dynamic Sharpness Process, variables......... */
public:
    DSReg * pDSReg;
    DSReg * pDSRegNormal;
    // for ClearZoom
    DSReg iDSRegEntry[UR_MAX_LEVEL];
    DSHWReg iDSHWRegEntry[UR_MAX_LEVEL];
    uint32_t iTdshpLevel[ISP_SCENARIOS];

private:
    DSInput oldInput;
    DSOutput oldOutput;
    DSReg * pDSRegOld;

    uint32_t iISOSpeedIIR;

    int iUpScalingRatioHoriz;
    int iDownScalingRatioHoriz;
    int iUpScalingRatioVert;
    int iDownScalingRatioVert;


#ifndef DSHP_ANDROID_PLATFORM
    FILE * ifp1;
#endif
};

#endif //__PQDSIMPL_H__

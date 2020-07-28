#ifndef MDPAALIMPL_H
#define MDPAALIMPL_H

/*
* header files
*/

//#define DRE_ANDROID_PLATFORM

/*
* standard header files
*/
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "mdpAALCommon.h"
#include "mdpAALImplCore.h"

#define DRE_SLOPE_BIT 15
#define DRE_SLOPE_BIT_ROUND ( ( 1 << ( DRE_SLOPE_BIT ) ) >> 1 )

#define DRE_STRENGTH_BIT 7
#define DRE_STRENGTH_BIT_ROUND ( ( 1 << ( DRE_STRENGTH_BIT ) ) >> 1 )

#define DRE_FLT_COEF_BIT 7
#define DRE_FLT_COEF_RANGE ( 1 << ( DRE_FLT_COEF_BIT ) )

#define DRE_GAIN_INDEX_FLOAT_BIT 8
#define DRE_GAIN_INDEX_INT_BIT 4
#define DRE_GAIN_INDEX_BIT ( ( DRE_GAIN_INDEX_FLOAT_BIT ) + ( DRE_GAIN_INDEX_INT_BIT ) )
#define DRE_GAIN_INDEX_BIT_RANGE ( 1 << ( DRE_GAIN_INDEX_BIT ) )
#define DRE_GAIN_INDEX_BIT_ROUND ( ( 1 << ( DRE_GAIN_INDEX_BIT ) ) >> 1 )
#define DRE_GAIN_INDEX_FLOAT_BIT_ROUND ( ( 1 << ( DRE_GAIN_INDEX_FLOAT_BIT ) ) >> 1 )

#define DRE_OVER_PXL_SHIFT_BIT 8
#define DRE_OVER_PXL_SHIFT_BIT_ROUND ( ( 1 << ( DRE_OVER_PXL_SHIFT_BIT ) ) >> 1 )
#define DRE_NOR_OVER_PXL_STRNGTH_MAX 256

#define DRE_DYNC_FLT_WGT_MAX_BIT 8
#define DRE_DYNC_FLT_WGT_MAX_BIT_ROUND ( ( 1 << ( DRE_DYNC_FLT_WGT_MAX_BIT ) ) >> 1 )
#define DRE_DYNC_FLT_WGT_MAX ( 1 << ( DRE_DYNC_FLT_WGT_MAX_BIT ) )
#define DRE_DYNC_FLT_SLOPE_BIT 4
#define DRE_DYNC_FLT_SLOPE_BIT_ROUND ( ( 1 << ( DRE_DYNC_FLT_SLOPE_BIT ) ) >> 1 )


// Event bits  // need to rename ???
enum MdpAALEvent {
    // Configuration changed
    eMdpAALEvtConfigChanged   = 0x10,
    // Some configuration field has been written
    eMdpAALEvtFieldWritten    = 0x20,
    // Intialize, set when boot or AAL restart
    eMdpAALEvtInit            = 0x30,
    // Some configuration field has been read
    eMdpAALEvtFieldRead       = 0x40,
};


#if (CALTM_VERSION == CALTM_VERSION_3_0)
struct DREInitParam {
    int frame_width;
    int frame_height;
    int tile_pxl_start;
    int tile_pxl_end;
    int act_win_x_start;
    int act_win_x_end;
    int isLastTile;
    int demo_win_x_start;
    int demo_win_x_end;
};
#elif (CALTM_VERSION == CALTM_VERSION_3_1)
struct DREInitParam {
    int frame_width;
    int frame_height;
    int tile_pxl_x_start;
    int tile_pxl_x_end;
    int tile_pxl_y_start;
    int tile_pxl_y_end;
    int act_win_x_start;
    int act_win_x_end;
    int act_win_y_start;
    int act_win_y_end;
    int isLastTile_x;
    int isLastTile_y;
    int demo_win_x_start;
    int demo_win_x_end;
    int demo_win_y_start;
    int demo_win_y_end;
};
#endif

struct DREInitReg {
    int dre_blk_x_num;
    int dre_blk_y_num;
    int dre_blk_height;
    int dre_blk_width;
    int dre_blk_area;
    int dre_x_alpha_base;
    int dre_x_alpha_shift_bit;
    int dre_y_alpha_base;
    int dre_y_alpha_shift_bit;
    int dre_flat_length_th;
    int dre_flat_length_slope;
    int dre_blk_area_min;
    int HSIZE;
    int VSIZE;
#if (CALTM_VERSION == CALTM_VERSION_3_0)
    int act_win_x_start;
    int act_win_x_end;
    int blk_num_start;
    int blk_num_end;
    int blk_cnt_start;
    int blk_cnt_end;
    int last_tile_flag;
    int win_x_start;
    int win_x_end;
#elif (CALTM_VERSION == CALTM_VERSION_3_1)
    int act_win_x_start;
    int act_win_x_end;
    int act_win_y_start;
    int act_win_y_end;
    int blk_num_x_start;
    int blk_num_x_end;
    int blk_cnt_x_start;
    int blk_cnt_x_end;
    int blk_num_y_start;
    int blk_num_y_end;
    int blk_cnt_y_start;
    int blk_cnt_y_end;
    int last_tile_x_flag;
    int last_tile_y_flag;
    int win_x_start;
    int win_x_end;
    int win_y_start;
    int win_y_end;
#endif
    int dre_s_lower;
    int dre_s_upper;
    int dre_y_lower;
    int dre_y_upper;
    int dre_h_lower;
    int dre_h_upper;
    int dre_bitplus_contour_range_th;
    int dre_bitplus_contour_range_slope;
    int dre_bitplus_diff_count_th;
    int dre_bitplus_diff_count_slope;
    int dre_bitplus_pxl_diff_th;
    int dre_bitplus_pxl_diff_slope;
    int dre_bitplus_noise_range_th;
    int dre_bitplus_noise_range_slope;
    int dre_bitplus_signchange_count_th;
    int dre_bitplus_signchange_count_slope;
    int dre_bitplus_high_lvl_out_oft;
    int dre_bitplus_high_lvl_pxl_slope;
    int dre_bitplus_high_lvl_pxl_th;
    int dre_bitplus_high_lvl_out_oft2;
    int dre_bitplus_high_lvl_pxl_slope2;
    int dre_bitplus_high_lvl_pxl_th2;
    int dre_bitplus_to_ali_wgt;
    int dre_pxl_diff_th_for_flat_pxl;
    int dre_pxl_diff_slope_for_flat_pxl;
    int dre_pxl_diff_th;
    int dre_pxl_diff_slope;
};

struct DREToolReg {
    int dre_fw_en;
    int dre_curve_en;
    int dre_gain_flt_en;
    int fw_ctrl_dre_s_lower;
    int fw_ctrl_dre_s_upper;
    int fw_ctrl_dre_y_lower;
    int fw_ctrl_dre_y_upper;
    int fw_ctrl_dre_h_lower;
    int fw_ctrl_dre_h_upper;
    int fw_ctrl_dre_bitplus_contour_range_th;
    int fw_ctrl_dre_bitplus_contour_range_slope;
    int fw_ctrl_dre_bitplus_diff_count_th;
    int fw_ctrl_dre_bitplus_diff_count_slope;
    int fw_ctrl_dre_bitplus_pxl_diff_th;
    int fw_ctrl_dre_bitplus_pxl_diff_slope;
    int fw_ctrl_dre_bitplus_noise_range_th;
    int fw_ctrl_dre_bitplus_noise_range_slope;
    int fw_ctrl_dre_bitplus_signchange_count_th;
    int fw_ctrl_dre_bitplus_signchange_count_slope;
    int fw_ctrl_dre_bitplus_high_lvl_out_oft;
    int fw_ctrl_dre_bitplus_high_lvl_pxl_slope;
    int fw_ctrl_dre_bitplus_high_lvl_pxl_th;
    int fw_ctrl_dre_bitplus_high_lvl_out_oft2;
    int fw_ctrl_dre_bitplus_high_lvl_pxl_slope2;
    int fw_ctrl_dre_bitplus_high_lvl_pxl_th2;
    int fw_ctrl_dre_bitplus_to_ali_wgt;
    int fw_ctrl_dre_pxl_diff_th_for_flat_pxl;
    int fw_ctrl_dre_pxl_diff_slope_for_flat_pxl;
    int fw_ctrl_dre_flat_length_th;
    int fw_ctrl_dre_flat_length_slope;
    int fw_ctrl_dre_pxl_diff_th;
    int fw_ctrl_dre_pxl_diff_slope;
    int fw_ctrl_dre_blk_area_min;
    int bADLWeight1;
    int bADLWeight2;
    int bADLWeight3;
    int bBSDCGain;
    int bBSACGain;
    int bBSLevel;
    int bMIDDCGain;
    int bMIDACGain;
    int bWSDCGain;
    int bWSACGain;
    int bWSLevel;
    int dre_dync_spike_wgt_min;
    int dre_dync_spike_wgt_max;
    int dre_dync_spike_th;
    int dre_dync_spike_slope;
    int bSpikeBlendmethod;
    int bSkinWgtSlope;
    int bSkinBlendmethod;
    int SkinDummy1;
    int Layer0_Ratio;
    int Layer1_Ratio;
    int Layer2_Ratio;
    int Dark_Ratio;
    int Bright_Ratio;
    int dre_dync_flt_coef_min;
    int dre_dync_flt_coef_max;
    int dre_dync_flt_ovr_pxl_th;
    int dre_dync_flt_ovr_pxl_slope;
    int dre_iir_force_range;
    int LLPValue;
    int LLPRatio;
    int APLCompRatioLow;
    int APLCompRatioHigh;
    int FltConfSlope;
    int FltConfTh;
    int BlkHistCountRatio;
    int BinIdxDiffSlope;
    int BinIdxDiffTh;
    int BinIdxDiffWgtOft;
    int APLTh;
    int APLSlope;
    int APLWgtOft;
    int APL2Th;
    int APL2Slope;
    int APL2WgtOft;
    int APL2WgtMax;
    int BlkSpaFltEn;
    int BlkSpaFltType;
    int LoadBlkCurveEn;
    int SaveBlkCurveEn;
    int Flat_Length_Th_Base;
    int Flat_Length_Slope_Base;
    int DZ_Fallback_En;
    int DZ_Size_Th;
    int DZ_Size_Slope;
#if (CALTM_VERSION == CALTM_VERSION_3_1)
    int bGreenWgtSlope;
    int dre_dync_compact_wgt_min;
    int dre_dync_compact_wgt_max;
    int dre_dync_compact_th;
    int dre_dync_compact_slope;
    int bCompactBlendmethod;
    int dre_dync_compact_LowBoundBin;
    int dre_dync_compact_HighBoundBin;
    int dre_dync_compact_BinMinWeight;
    int dre_dync_compact_BinMaxWeight;
#endif
};

// Fields collected from AAL HW
struct DRETopInput {
    unsigned int eventFlags;

    int DREMaxHisSet[mdpDRE_BLK_NUM_Y][mdpDRE_BLK_NUM_X][mdpDRE_LUMA_HIST_NUM_WEIGHT];

    int DRERGBMaxSum[mdpDRE_BLK_NUM_Y][mdpDRE_BLK_NUM_X];
#if (CALTM_VERSION == CALTM_VERSION_3_0)
    int DREChromaHist[mdpDRE_BLK_NUM_Y][mdpDRE_BLK_NUM_X]; //HW calculated Skin histogram for CALTM_VERSION_3_0;
#elif (CALTM_VERSION == CALTM_VERSION_3_1)
    int DREChromaHist[mdpDRE_BLK_NUM_Y][mdpDRE_BLK_NUM_X];  //Used as a virtual buffer for FD in CALTM_VERSION_3_1
    int DREChromaHist2[mdpDRE_BLK_NUM_Y][mdpDRE_BLK_NUM_X]; //HW calculated Green histogram for CALTM_VERSION_3_1;
#endif
    int DREFlatLineCountSet[mdpDRE_BLK_NUM_Y][mdpDRE_BLK_NUM_X];

    int DRELargeDiffCountSet[mdpDRE_BLK_NUM_Y][mdpDRE_BLK_NUM_X];

    int DREMaxDiffSet[mdpDRE_BLK_NUM_Y][mdpDRE_BLK_NUM_X];

    int dre_blk_x_num;
    int dre_blk_y_num;

    int IspScenario;

    int PreWidth;
    int PreHeight;
    int CurWidth;
    int CurHeight;

    int PreFloatCurve[mdpDRE_BLK_NUM_Y][mdpDRE_BLK_NUM_X][mdpDRE_LUMA_CURVE_NUM]; // 15-bit [0, 255*128]
};

// Fields which will be set to HW registers
struct DRETopOutput {
    int DRECurveSet[mdpDRE_BLK_NUM_Y][mdpDRE_BLK_NUM_X][mdpDRE_LUMA_CURVE_NUM];
    int CurFloatCurve[mdpDRE_BLK_NUM_Y][mdpDRE_BLK_NUM_X][mdpDRE_LUMA_CURVE_NUM]; // 15-bit [0, 255*128]
};


///////////////////////////////////////////////////////////////////////////////
// MDP AAL FW Processing class
///////////////////////////////////////////////////////////////////////////////
class CDRETopFW
{
private:
  unsigned int DebugFlags;

  int CurTgtCurve[mdpDRE_BLK_NUM_Y][mdpDRE_BLK_NUM_X][ mdpDRE_LUMA_CURVE_NUM ]; // 8-bit [0, 255]
  int CurFltCurve[mdpDRE_BLK_NUM_Y][mdpDRE_BLK_NUM_X][ mdpDRE_LUMA_CURVE_NUM ]; // 8-bit [0, 255]

  int DRESkinWgtSet[mdpDRE_BLK_NUM_Y][mdpDRE_BLK_NUM_X];

  int Tmpfinal_l_curve_wgt_set[mdpDRE_BLK_NUM_Y][mdpDRE_BLK_NUM_X];

  int Blk_Hist_Wgt[mdpDRE_BLK_SFLT_SIZE_MAX][mdpDRE_BLK_SFLT_SIZE_MAX];
  int Blk_LPF_W;
  int Blk_LPF_H;
  int tmp_shift_bit;
  int tmp_shift_bit_round;
  int cen_x;
  int cen_y;

  int IIRForceRange;

  char inname[1000];
  char integer_string[100];
  bool isFirstFrame;

  void EventTriggerSetting( const DRETopInput &input, DRETopOutput *output );
  void DREGainIdxCal( const DRETopInput &input );
  void DRECurveCalculation( const DRETopInput &input );
  void G_AdaptiveLuma_Block( int dre_blk_x_num, int dre_blk_y_num, int HistNorEn, int *HisBin, int RGBMaxSum, int ChromaHistCount, int ChromaHistCount2, int *iYFtn );
  void L_AdaptiveLuma_Block( int dre_blk_x_num, int dre_blk_y_num, int HistNorEn, int *HisBin, int RGBMaxSum, int ChromaHistCount, int *iYFtn, int ChromaHistCount2 );
  void DREGainIdxFlt( const DRETopInput &input, DRETopOutput *output );
  int DynCAdaptCoef( const int NorHist[ mdpDRE_LUMA_HIST_NUM_WEIGHT ], const int PreFloatCurve[ mdpDRE_LUMA_CURVE_NUM ], int CurTgtCurve[ mdpDRE_LUMA_CURVE_NUM ], int CoefBit );
  int IIR( int CurIn, int PreOutFloat, int & CurOutFloat, int CoefBit, int CurInCoef );
  void calIIRForceRange();
  void DREFinalGainCal( const DRETopInput &input, DRETopOutput *output );
  void BlkSPF( const DRETopInput &input, DRETopOutput *output );
  void BlkSTFP( const DRETopInput &input, DRETopOutput *output );
  void IspPreviewZoom( const DRETopInput &input, DRETopOutput *output );

  CDRECoreFW * pDRECoreFW;
  DRECoreInput * pDRECoreIn;
  DRECoreOutput * pDRECoreOut;

public:
  DREReg * pDREReg;
  DREInitReg * pDREInitReg;
  DREInternalReg * pDREInternalReg;
  DREToolReg * pDREToolReg;

  void setDebugFlags(unsigned int debug) {
    DebugFlags = debug;
    pDRECoreFW->setDebugFlags( DebugFlags );
  }

  int DREVersionNum;

  void calBlkSpaFltParam();
  void DREBlkHWRegCal( int Blk_Num, int Width, int &blk_width, int &dre_x_alpha_base, int &dre_x_alpha_shift_bit );
  void DREFrameHWRegCal( const DREInitParam &initParam );
  void DRETileHWRegCal( const DREInitParam &initParam );
  void LoadFWRegSettings();

#ifdef DRE_ANDROID_PLATFORM
  void onCalculate(const DRETopInput &input, DRETopOutput *output);
  void onInitCommon(const DREInitParam &initParam);

  // Initialization will call onInitCommon() -> onInitPlatform()
  // onInitCommon() performs common initialization.
  // onInitPlatform() will load customization parameters

#else
  void Main( const DRETopInput &input, DRETopOutput *output );
  void Initialize( const DREInitParam &initParam );

  FILE * ifp0;
  FILE * ifp2;
#endif


  CDRETopFW()
  {
    memset(this, 0, sizeof(*this));
    pDREReg = new DREReg;
    pDREInitReg = new DREInitReg;
    pDREInternalReg = new DREInternalReg;
    pDREToolReg = new DREToolReg;

    pDRECoreFW  = new CDRECoreFW;
    pDRECoreIn  = new DRECoreInput;
    pDRECoreOut = new DRECoreOutput;

    pDRECoreFW->pDREReg = pDREReg;
    pDRECoreFW->pDREInternalReg = pDREInternalReg;

    DebugFlags = 0;
  };

  ~CDRETopFW()
  {
    delete pDREReg;
    delete pDREInitReg;
    delete pDREInternalReg;
    delete pDREToolReg;

    delete pDRECoreFW;
    delete pDRECoreIn;
    delete pDRECoreOut;

#ifndef DRE_ANDROID_PLATFORM
  if( DebugFlags & eMdpAALDebugDRE )
  {
    fclose( ifp0 );
  }
#endif
  };
};

#endif

#ifndef __ADAPTIVECALTM_NVRAM_H__
#define __ADAPTIVECALTM_NVRAM_H__

#include "mdpAALCommon.h"

typedef unsigned int MUINT32;
typedef int MINT32;

typedef struct{
    MUINT32  ca_ltm_s_lower;
    MUINT32  ca_ltm_s_upper;
    MUINT32  ca_ltm_y_lower;
    MUINT32  ca_ltm_y_upper;
    MUINT32  ca_ltm_h_lower;
    MUINT32  ca_ltm_h_upper;
    MUINT32  ca_ltm_max_hist_mode;
    MUINT32  ca_ltm_bitplus_contour_range_th;
    MUINT32  ca_ltm_bitplus_contour_range_slope;
    MUINT32  ca_ltm_bitplus_diff_count_th;
    MUINT32  ca_ltm_bitplus_diff_count_slope;
    MUINT32  ca_ltm_bitplus_pxl_diff_th;
    MUINT32  ca_ltm_bitplus_pxl_diff_slope;
    MUINT32  ca_ltm_pxl_diff_th_for_flat_pxl;
    MUINT32  ca_ltm_pxl_diff_slope_for_flat_pxl;
    MUINT32  ca_ltm_pxl_diff_th;
    MUINT32  ca_ltm_pxl_diff_slope;
}FEATURE_NVRAM_CA_LTM_INITUNINGREG_T, *PFEATURE_NVRAM_CA_LTM_INITUNINGREG_T;

typedef struct{
    MINT32 ca_ltm_fw_en;
    MINT32 ca_ltm_curve_en;
    MINT32 ca_ltm_gain_flt_en;
    MINT32 ca_ltm_iir_force_range;
    MUINT32 bADLWeight1;
    MUINT32 bADLWeight2;
    MUINT32 bADLWeight3;
    MUINT32 bBSDCGain;
    MUINT32 bBSACGain;
    MUINT32 bBSLevel;
    MUINT32 bMIDDCGain;
    MUINT32 bMIDACGain;
    MUINT32 bWSDCGain;
    MUINT32 bWSACGain;
    MUINT32 bWSLevel;
    MUINT32 ca_ltm_dync_spike_wgt_min;
    MUINT32 ca_ltm_dync_spike_wgt_max;
    MUINT32 ca_ltm_dync_spike_th;
    MUINT32 ca_ltm_dync_spike_slope;
    MUINT32 bSpikeBlendmethod;
    MUINT32 bSkinWgtSlope;
    MUINT32 bSkinBlendmethod;
    MUINT32 ca_ltm_dync_flt_coef_min;
    MUINT32 ca_ltm_dync_flt_coef_max;
    MUINT32 ca_ltm_dync_flt_ovr_pxl_th;
    MUINT32 ca_ltm_dync_flt_ovr_pxl_slope;
    MINT32 LLPValue;
    MINT32 LLPRatio;
    MINT32 APLCompRatioLow;
    MINT32 APLCompRatioHigh;
    MINT32 FltConfSlope;
    MINT32 FltConfTh;
    MINT32 BlkHistCountRatio;
    MINT32 BinIdxDiffSlope;
    MINT32 BinIdxDiffTh;
    MINT32 BinIdxDiffWgtOft;
    MINT32 APLTh;
    MINT32 APLSlope;
    MINT32 APLWgtOft;
    MINT32 APL2Th;
    MINT32 APL2Slope;
    MINT32 APL2WgtOft;
    MINT32 APL2WgtMax;
    MINT32 BlkSpaFltEn;
    MINT32 BlkSpaFltType;
    MINT32 LoadBlkCurveEn;
    MINT32 SaveBlkCurveEn;
#if (CALTM_VERSION == CALTM_VERSION_3_1)
    MINT32 bGreenWgtSlope;
    MINT32 dre_dync_compact_wgt_min;
    MINT32 dre_dync_compact_wgt_max;
    MINT32 dre_dync_compact_th;
    MINT32 dre_dync_compact_slope;
    MINT32 bCompactBlendmethod;
    MINT32 dre_dync_compact_LowBoundBin;
    MINT32 dre_dync_compact_HighBoundBin;
    MINT32 dre_dync_compact_BinMinWeight;
    MINT32 dre_dync_compact_BinMaxWeight;
#endif
}FEATURE_NVRAM_CA_LTM_TUNINGREG_T, *PFEATURE_NVRAM_CA_LTM_TUNINGREG_T;

typedef struct{
    MINT32 Enabled;
    MINT32 Strength;
    MINT32 AdaptiveMethod;
    MINT32 AdaptiveType;
    MINT32 CustomParametersSearchMode;
}FEATURE_NVRAM_CA_LTM_ADAPTTUNINGREG_T, *PFEATURE_NVRAM_CA_LTM_ADAPTTUNINGREG_T;

typedef struct{
    FEATURE_NVRAM_CA_LTM_INITUNINGREG_T CA_LTM_INITUNINGRE;
    FEATURE_NVRAM_CA_LTM_TUNINGREG_T CA_LTM_TUNINGREG;
    FEATURE_NVRAM_CA_LTM_ADAPTTUNINGREG_T CA_LTM_ADAPTTUNINGREG;
}FEATURE_NVRAM_CA_LTM_ALLTUNINGREG_T, *PFEATURE_NVRAM_CA_LTM_ALLTUNINGREG_T;

#define NVRAM_ISO_LV_SEGMENTATION_SIZE 15

typedef struct{
    MINT32 ISO[1 + NVRAM_ISO_LV_SEGMENTATION_SIZE];
    MINT32 LV[1 + NVRAM_ISO_LV_SEGMENTATION_SIZE];
    FEATURE_NVRAM_CA_LTM_ALLTUNINGREG_T CA_LTM_ALLTUNINGREG[15];
} FEATURE_NVRAM_CA_LTM_T, *PFEATURE_NVRAM_CA_LTM_T;

#endif

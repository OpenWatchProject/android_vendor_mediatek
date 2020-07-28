#ifndef __PQSERVICECOMMON_H__
#define __PQSERVICECOMMON_H__

#include "ddp_pq.h"

typedef enum PQ_SCENARIO_TYPE_ENUM
{
    SCENARIO_UNKNOWN,
    SCENARIO_VIDEO,
    SCENARIO_PICTURE,
    SCENARIO_ISP_PREVIEW,
    SCENARIO_VIDEO_CODEC,
    SCENARIO_ISP_CAPTURE
} PQ_SCENARIO_TYPE_ENUM;

enum
{
    SET_PQ_SHP_GAIN = 0,
    SET_PQ_SAT_GAIN,
    SET_PQ_LUMA_ADJ,
    SET_PQ_HUE_ADJ_SKIN,
    SET_PQ_HUE_ADJ_GRASS,
    SET_PQ_HUE_ADJ_SKY,
    SET_PQ_SAT_ADJ_SKIN,
    SET_PQ_SAT_ADJ_GRASS,
    SET_PQ_SAT_ADJ_SKY,
    SET_PQ_CONTRAST,
    SET_PQ_BRIGHTNESS
};

enum
{
    MOD_DISPLAY =               0x0000,
    MOD_DYNAMIC_SHARPNESS =     0x0002,
    MOD_TDSHP_REG =             0x0003,
    MOD_ULTRARESOLUTION =       0x0004,
    MOD_DYNAMIC_CONTRAST =      0x0007,
    MOD_BLUE_LIGHT_ALGO =       0x0010,
    MOD_BLUE_LIGHT_INPUT =      0x0011,
    MOD_BLUE_LIGHT_OUTPUT =     0x0012,
    MOD_CHAMELEON_ALGO =        0x0020,
    MOD_CHAMELEON_INPUT =       0x0021,
    MOD_CHAMELEON_OUTPUT =      0x0022,
    MOD_TRANSITION_ALGO =       0x0030,
    MOD_TRANSITION_INPUT =      0x0031,
    MOD_TRANSITION_OUTPUT =     0x0032,
    MOD_DS_SWREG =              0x0110,
    MOD_DS_INPUT =              0x0111,
    MOD_DS_OUTPUT =             0x0112,
    MOD_DC_SWREG =              0x0120,
    MOD_DC_INPUT =              0x0121,
    MOD_DC_OUTPUT =             0x0122,
    MOD_RSZ_SWREG =             0x0130,
    MOD_RSZ_INPUT =             0x0131,
    MOD_RSZ_OUTPUT =            0x0132,
    MOD_COLOR_SWREG =           0x0140,
    MOD_COLOR_INPUT =           0x0141,
    MOD_COLOR_OUTPUT =          0x0142,
    MOD_HDR_SWREG =             0x0150,
    MOD_HDR_INPUT =             0x0151,
    MOD_HDR_OUTPUT =            0x0152,
    MOD_CCORR_SWREG =           0x0160,
    MOD_CCORR_INPUT =           0x0161,
    MOD_CCORR_OUTPUT =          0x0162,
    MOD_DRE_SWREG =             0x0170,
    MOD_DRE_INPUT =             0x0171,
    MOD_DRE_OUTPUT =            0x0172,
    MOD_HFG_INPUT =             0x0173,
    MOD_HFG_OUTPUT =            0x0174,
    MOD_RGB_GAIN =              0x0200,
};

typedef enum {
    TDSHP_GAIN_MID = 0,
    TDSHP_GAIN_HIGH,
    TDSHP_COR_GAIN,
    TDSHP_COR_THR,
    TDSHP_COR_ZERO,
    TDSHP_GAIN,
    TDSHP_COR_VALUE,

    TDSHP_HIGH_COR_ZERO,
    TDSHP_HIGH_COR_THR,
    TDSHP_HIGH_COR_GAIN,
    TDSHP_HIGH_COR_VALUE,
    TDSHP_MID_COR_ZERO,
    TDSHP_MID_COR_THR,
    TDSHP_MID_COR_GAIN,
    TDSHP_MID_COR_VALUE,
    EDF_EDGE_GAIN,
    EDF_FLAT_TH,
    EDF_DETAIL_RISE_TH,

    TDSHP_CLIP_RATIO,
    TDSHP_CLIP_THR,

    TDSHP_REG_BASE_SEL,
    TDSHP_REG_INDEX_MAX
} MDP_TDSHP_REG_EX_index_t;

typedef struct {

    int param[TDSHP_REG_INDEX_MAX];

} MDP_TDSHP_REG_EX;

typedef enum {
    PQDS_DS_en = 0,
    PQDS_iUpSlope,
    PQDS_iUpThreshold,
    PQDS_iDownSlope,
    PQDS_iDownThreshold,
    PQDS_iISO_en,
    PQDS_iISO_thr1,
    PQDS_iISO_thr0,
    PQDS_iISO_thr3,
    PQDS_iISO_thr2,
    PQDS_iISO_IIR_alpha,
    PQDS_iCorZero_clip2,
    PQDS_iCorZero_clip1,
    PQDS_iCorZero_clip0,
    PQDS_iCorThr_clip2,
    PQDS_iCorThr_clip1,
    PQDS_iCorThr_clip0,
    PQDS_iCorGain_clip2,
    PQDS_iCorGain_clip1,
    PQDS_iCorGain_clip0,
    PQDS_iGain_clip2,
    PQDS_iGain_clip1,
    PQDS_iGain_clip0,

    //6797 new reg
    PQDS_iHighCorZero_clip2,
    PQDS_iHighCorZero_clip1,
    PQDS_iHighCorZero_clip0,
    PQDS_iHighCorThr_clip2,
    PQDS_iHighCorThr_clip1,
    PQDS_iHighCorThr_clip0,
    PQDS_iHighCorGain_clip2,
    PQDS_iHighCorGain_clip1,
    PQDS_iHighCorGain_clip0,

    PQDS_iMidCorZero_clip2,
    PQDS_iMidCorZero_clip1,
    PQDS_iMidCorZero_clip0,
    PQDS_iMidCorThr_clip2,
    PQDS_iMidCorThr_clip1,
    PQDS_iMidCorThr_clip0,
    PQDS_iMidCorGain_clip2,
    PQDS_iMidCorGain_clip1,
    PQDS_iMidCorGain_clip0,

    PQDS_i_edf_flat_th_clip2,
    PQDS_i_edf_flat_th_clip1,
    PQDS_i_edf_flat_th_clip0,
    PQDS_i_edf_detail_rise_th_clip2,
    PQDS_i_edf_detail_rise_th_clip1,
    PQDS_i_edf_detail_rise_th_clip0,

    //UltraResolution registers
    PQDS_iUltraRes_en,
    PQDS_iUltraRes_ratio_thr2,
    PQDS_iUltraRes_ratio_thr1,
    PQDS_iUltraRes_ratio_thr0,
    PQDS_iUltraRes_gain_mid_clip1,
    PQDS_iUltraRes_gain_mid_clip0,
    PQDS_iUltraRes_gain_high_clip1,
    PQDS_iUltraRes_gain_high_clip0,
    PQDS_iUltraRes_edf_edge_gain_clip1,
    PQDS_iUltraRes_edf_edge_gain_clip0,
    PQDS_iUltraRes_clip_thr_clip1,
    PQDS_iUltraRes_clip_thr_clip0,
    PQDS_iUltraRes_clip_ratio_clip1,
    PQDS_iUltraRes_clip_ratio_clip0,
    PQDS_iUltraRes_edf_detail_gain_clip1,
    PQDS_iUltraRes_edf_detail_gain_clip0,
    PQDS_iUltraRes_edf_flat_gain_clip1,
    PQDS_iUltraRes_edf_flat_gain_clip0,

    PQDS_INDEX_MAX
} PQ_DS_EX_index_t;


typedef struct {

    int param[PQDS_INDEX_MAX];

} DISP_PQ_DS_PARAM_EX;

typedef enum {
    RSZ_tableMode = 0,
    RSZ_defaultUpTable,
    RSZ_defaultDownTable,
    RSZ_autoTableSelection,
    RSZ_autoAlignment,
    RSZ_autoAlgSelection,
    RSZ_IBSEEnable,
    RSZ_ultraResEnable,
    RSZ_autoCoefTrunc,

    /* algorithm-switch ratios for Rome mode */
    RSZ_switchRatio6Tap6nTap,
    RSZ_switchRatio6nTapAcc,

    /* Ultra Resolution */
    RSZ_dynIBSE_gain_ratio_thr0,
    RSZ_dynIBSE_gain_ratio_thr1,
    RSZ_dynIBSE_gain_ratio_thr2,
    RSZ_dynIBSE_gain_clip1,
    RSZ_dynIBSE_gain_clip2,
    RSZ_dynIBSE_gain_min,
    RSZ_dynIBSE_gain_max,

    RSZ_tapAdaptSlope,
    RSZ_tapAdapt_slope_ratio_thr0,
    RSZ_tapAdapt_slope_ratio_thr1,
    RSZ_tapAdapt_slope_ratio_thr2,
    RSZ_tapAdapt_slope_clip1,
    RSZ_tapAdapt_slope_clip2,
    RSZ_tapAdapt_slope_min,
    RSZ_tapAdapt_slope_max,

    RSZ_IBSE_clip_thr,
    RSZ_IBSE_clip_thr_ratio_thr0,
    RSZ_IBSE_clip_thr_ratio_thr1,
    RSZ_IBSE_clip_thr_ratio_thr2,
    RSZ_IBSE_clip_thr_clip1,
    RSZ_IBSE_clip_thr_clip2,
    RSZ_IBSE_clip_thr_min,
    RSZ_IBSE_clip_thr_max,

    RSZ_IBSE_clip_ratio,
    RSZ_IBSE_clip_ratio_ratio_thr0,
    RSZ_IBSE_clip_ratio_ratio_thr1,
    RSZ_IBSE_clip_ratio_ratio_thr2,
    RSZ_IBSE_clip_ratio_clip1,
    RSZ_IBSE_clip_ratio_clip2,
    RSZ_IBSE_clip_ratio_min,
    RSZ_IBSE_clip_ratio_max,

    RSZ_INDEX_MAX
} PQ_RSZ_index_t;


typedef struct {

    int param[RSZ_INDEX_MAX];

} PQ_RSZ_PARAM;

#define DISP_COLOR_SWREG_START              (0xFFFF0000)
#define DISP_COLOR_SWREG_COLOR_BASE         (DISP_COLOR_SWREG_START)  /* 0xFFFF0000 */
#define DISP_COLOR_SWREG_TDSHP_BASE         (DISP_COLOR_SWREG_COLOR_BASE + 0x1000)  /* 0xFFFF1000 */
#define DISP_COLOR_SWREG_TDSHP_REG_BASE     (DISP_COLOR_SWREG_COLOR_BASE + 0x1001)  /* 0xFFFF1001 */
#define DISP_COLOR_SWREG_PQDC_BASE          (DISP_COLOR_SWREG_TDSHP_BASE + 0x1000)  /* 0xFFFF2000 */
#define DISP_COLOR_SWREG_PQDS_BASE          (DISP_COLOR_SWREG_PQDC_BASE + 0x1000) /* 0xFFFF3000 */
#define DISP_COLOR_SWREG_MDP_COLOR_BASE     (DISP_COLOR_SWREG_PQDS_BASE + 0x1000) /* 0xFFFF4000 */
#define DISP_COLOR_SWREG_END                (DISP_COLOR_SWREG_MDP_COLOR_BASE + 0x1000)  /* 0xFFFF5000 */

#define SWREG_TDSHP_TUNING_MODE             (DISP_COLOR_SWREG_TDSHP_BASE + 0x0000)
#define SWREG_TDSHP_REG_GAIN_MID            (DISP_COLOR_SWREG_TDSHP_REG_BASE + TDSHP_GAIN_MID)
#define SWREG_TDSHP_REG_GAIN_HIGH           (DISP_COLOR_SWREG_TDSHP_REG_BASE + TDSHP_GAIN_HIGH)
#define SWREG_TDSHP_REG_COR_GAIN            (DISP_COLOR_SWREG_TDSHP_REG_BASE + TDSHP_COR_GAIN)
#define SWREG_TDSHP_REG_COR_THR             (DISP_COLOR_SWREG_TDSHP_REG_BASE + TDSHP_COR_THR)
#define SWREG_TDSHP_REG_COR_ZERO            (DISP_COLOR_SWREG_TDSHP_REG_BASE + TDSHP_COR_ZERO)
#define SWREG_TDSHP_REG_GAIN                (DISP_COLOR_SWREG_TDSHP_REG_BASE + TDSHP_GAIN)
#define SWREG_TDSHP_REG_COR_VALUE           (DISP_COLOR_SWREG_TDSHP_REG_BASE + TDSHP_COR_VALUE)

#define SWREG_TDSHP_REG_HIGH_COR_ZERO       (DISP_COLOR_SWREG_TDSHP_REG_BASE + TDSHP_HIGH_COR_ZERO)
#define SWREG_TDSHP_REG_HIGH_COR_THR        (DISP_COLOR_SWREG_TDSHP_REG_BASE + TDSHP_HIGH_COR_THR)
#define SWREG_TDSHP_REG_HIGH_COR_GAIN       (DISP_COLOR_SWREG_TDSHP_REG_BASE + TDSHP_HIGH_COR_GAIN)
#define SWREG_TDSHP_REG_HIGH_COR_VALUE      (DISP_COLOR_SWREG_TDSHP_REG_BASE + TDSHP_HIGH_COR_VALUE)
#define SWREG_TDSHP_REG_MID_COR_ZERO        (DISP_COLOR_SWREG_TDSHP_REG_BASE + TDSHP_MID_COR_ZERO)
#define SWREG_TDSHP_REG_MID_COR_THR         (DISP_COLOR_SWREG_TDSHP_REG_BASE + TDSHP_MID_COR_THR)
#define SWREG_TDSHP_REG_MID_COR_GAIN        (DISP_COLOR_SWREG_TDSHP_REG_BASE + TDSHP_MID_COR_GAIN)
#define SWREG_TDSHP_REG_MID_COR_VALUE       (DISP_COLOR_SWREG_TDSHP_REG_BASE + TDSHP_MID_COR_VALUE)
#define SWREG_TDSHP_REG_EDF_EDGE_GAIN       (DISP_COLOR_SWREG_TDSHP_REG_BASE + EDF_EDGE_GAIN)
#define SWREG_TDSHP_REG_EDF_FLAT_TH         (DISP_COLOR_SWREG_TDSHP_REG_BASE + EDF_FLAT_TH)
#define SWREG_TDSHP_REG_EDF_DETAIL_RISE_TH  (DISP_COLOR_SWREG_TDSHP_REG_BASE + EDF_DETAIL_RISE_TH)
#define SWREG_TDSHP_REG_CLIP_RATIO          (DISP_COLOR_SWREG_TDSHP_REG_BASE + TDSHP_CLIP_RATIO)
#define SWREG_TDSHP_REG_CLIP_THR            (DISP_COLOR_SWREG_TDSHP_REG_BASE + TDSHP_CLIP_THR)

#define SWREG_TDSHP_REG_END                 (DISP_COLOR_SWREG_TDSHP_REG_BASE + TDSHP_REG_INDEX_MAX)

#define SWREG_BLACK_EFFECT_ENABLE      (DISP_COLOR_SWREG_PQDC_BASE + BlackEffectEnable)
#define SWREG_WHITE_EFFECT_ENABLE      (DISP_COLOR_SWREG_PQDC_BASE + WhiteEffectEnable)
#define SWREG_STRONG_BLACK_EFFECT      (DISP_COLOR_SWREG_PQDC_BASE + StrongBlackEffect)
#define SWREG_STRONG_WHITE_EFFECT      (DISP_COLOR_SWREG_PQDC_BASE + StrongWhiteEffect)
#define SWREG_ADAPTIVE_BLACK_EFFECT    (DISP_COLOR_SWREG_PQDC_BASE + AdaptiveBlackEffect)
#define SWREG_ADAPTIVE_WHITE_EFFECT    (DISP_COLOR_SWREG_PQDC_BASE + AdaptiveWhiteEffect)
#define SWREG_SCENCE_CHANGE_ONCE_EN    (DISP_COLOR_SWREG_PQDC_BASE + ScenceChangeOnceEn)
#define SWREG_SCENCE_CHANGE_CONTROL_EN (DISP_COLOR_SWREG_PQDC_BASE + ScenceChangeControlEn)
#define SWREG_SCENCE_CHANGE_CONTROL    (DISP_COLOR_SWREG_PQDC_BASE + ScenceChangeControl)
#define SWREG_SCENCE_CHANGE_TH1        (DISP_COLOR_SWREG_PQDC_BASE + ScenceChangeTh1)
#define SWREG_SCENCE_CHANGE_TH2        (DISP_COLOR_SWREG_PQDC_BASE + ScenceChangeTh2)
#define SWREG_SCENCE_CHANGE_TH3        (DISP_COLOR_SWREG_PQDC_BASE + ScenceChangeTh3)
#define SWREG_CONTENT_SMOOTH1          (DISP_COLOR_SWREG_PQDC_BASE + ContentSmooth1)
#define SWREG_CONTENT_SMOOTH2          (DISP_COLOR_SWREG_PQDC_BASE + ContentSmooth2)
#define SWREG_CONTENT_SMOOTH3          (DISP_COLOR_SWREG_PQDC_BASE + ContentSmooth3)
#define SWREG_MIDDLE_REGION_GAIN1      (DISP_COLOR_SWREG_PQDC_BASE + MiddleRegionGain1)
#define SWREG_MIDDLE_REGION_GAIN2      (DISP_COLOR_SWREG_PQDC_BASE + MiddleRegionGain2)
#define SWREG_BLACK_REGION_GAIN1       (DISP_COLOR_SWREG_PQDC_BASE + BlackRegionGain1)
#define SWREG_BLACK_REGION_GAIN2       (DISP_COLOR_SWREG_PQDC_BASE + BlackRegionGain2)
#define SWREG_BLACK_REGION_RANGE       (DISP_COLOR_SWREG_PQDC_BASE + BlackRegionRange)
#define SWREG_BLACK_EFFECT_LEVEL       (DISP_COLOR_SWREG_PQDC_BASE + BlackEffectLevel)
#define SWREG_BLACK_EFFECT_PARAM1      (DISP_COLOR_SWREG_PQDC_BASE + BlackEffectParam1)
#define SWREG_BLACK_EFFECT_PARAM2      (DISP_COLOR_SWREG_PQDC_BASE + BlackEffectParam2)
#define SWREG_BLACK_EFFECT_PARAM3      (DISP_COLOR_SWREG_PQDC_BASE + BlackEffectParam3)
#define SWREG_BLACK_EFFECT_PARAM4      (DISP_COLOR_SWREG_PQDC_BASE + BlackEffectParam4)
#define SWREG_WHITE_REGION_GAIN1       (DISP_COLOR_SWREG_PQDC_BASE + WhiteRegionGain1)
#define SWREG_WHITE_REGION_GAIN2       (DISP_COLOR_SWREG_PQDC_BASE + WhiteRegionGain2)
#define SWREG_WHITE_REGION_RANGE       (DISP_COLOR_SWREG_PQDC_BASE + WhiteRegionRange)
#define SWREG_WHITE_EFFECT_LEVEL       (DISP_COLOR_SWREG_PQDC_BASE + WhiteEffectLevel)
#define SWREG_WHITE_EFFECT_PARAM1      (DISP_COLOR_SWREG_PQDC_BASE + WhiteEffectParam1)
#define SWREG_WHITE_EFFECT_PARAM2      (DISP_COLOR_SWREG_PQDC_BASE + WhiteEffectParam2)
#define SWREG_WHITE_EFFECT_PARAM3      (DISP_COLOR_SWREG_PQDC_BASE + WhiteEffectParam3)
#define SWREG_WHITE_EFFECT_PARAM4      (DISP_COLOR_SWREG_PQDC_BASE + WhiteEffectParam4)
#define SWREG_CONTRAST_ADJUST1         (DISP_COLOR_SWREG_PQDC_BASE + ContrastAdjust1)
#define SWREG_CONTRAST_ADJUST2         (DISP_COLOR_SWREG_PQDC_BASE + ContrastAdjust2)
#define SWREG_DC_CHANGE_SPEED_LEVEL    (DISP_COLOR_SWREG_PQDC_BASE + DCChangeSpeedLevel)
#define SWREG_PROTECT_REGION_EFFECT    (DISP_COLOR_SWREG_PQDC_BASE + ProtectRegionEffect)
#define SWREG_DC_CHANGE_SPEED_LEVEL2   (DISP_COLOR_SWREG_PQDC_BASE + DCChangeSpeedLevel2)
#define SWREG_PROTECT_REGION_WEIGHT    (DISP_COLOR_SWREG_PQDC_BASE + ProtectRegionWeight)
#define SWREG_DC_ENABLE                (DISP_COLOR_SWREG_PQDC_BASE + DCEnable)
#define SWREG_DC_END                   (DISP_COLOR_SWREG_PQDC_BASE + PQDC_INDEX_MAX)


#define SWREG_PQDS_DS_EN                    (DISP_COLOR_SWREG_PQDS_BASE + PQDS_DS_en)
#define SWREG_PQDS_UP_SLOPE                 (DISP_COLOR_SWREG_PQDS_BASE + PQDS_iUpSlope)
#define SWREG_PQDS_UP_THR                   (DISP_COLOR_SWREG_PQDS_BASE + PQDS_iUpThreshold)
#define SWREG_PQDS_DOWN_SLOPE               (DISP_COLOR_SWREG_PQDS_BASE + PQDS_iDownSlope)
#define SWREG_PQDS_DOWN_THR                 (DISP_COLOR_SWREG_PQDS_BASE + PQDS_iDownThreshold)
#define SWREG_PQDS_ISO_EN                   (DISP_COLOR_SWREG_PQDS_BASE + PQDS_iISO_en)
#define SWREG_PQDS_ISO_THR1                 (DISP_COLOR_SWREG_PQDS_BASE + PQDS_iISO_thr1)
#define SWREG_PQDS_ISO_THR0                 (DISP_COLOR_SWREG_PQDS_BASE + PQDS_iISO_thr0)
#define SWREG_PQDS_ISO_THR3                 (DISP_COLOR_SWREG_PQDS_BASE + PQDS_iISO_thr3)
#define SWREG_PQDS_ISO_THR2                 (DISP_COLOR_SWREG_PQDS_BASE + PQDS_iISO_thr2)
#define SWREG_PQDS_ISO_IIR                  (DISP_COLOR_SWREG_PQDS_BASE + PQDS_iISO_IIR_alpha)
#define SWREG_PQDS_COR_ZERO_2               (DISP_COLOR_SWREG_PQDS_BASE + PQDS_iCorZero_clip2)
#define SWREG_PQDS_COR_ZERO_1               (DISP_COLOR_SWREG_PQDS_BASE + PQDS_iCorZero_clip1)
#define SWREG_PQDS_COR_ZERO_0               (DISP_COLOR_SWREG_PQDS_BASE + PQDS_iCorZero_clip0)
#define SWREG_PQDS_COR_THR_2                (DISP_COLOR_SWREG_PQDS_BASE + PQDS_iCorThr_clip2)
#define SWREG_PQDS_COR_THR_1                (DISP_COLOR_SWREG_PQDS_BASE + PQDS_iCorThr_clip1)
#define SWREG_PQDS_COR_THR_0                (DISP_COLOR_SWREG_PQDS_BASE + PQDS_iCorThr_clip0)
#define SWREG_PQDS_COR_GAIN_2               (DISP_COLOR_SWREG_PQDS_BASE + PQDS_iCorGain_clip2)
#define SWREG_PQDS_COR_GAIN_1               (DISP_COLOR_SWREG_PQDS_BASE + PQDS_iCorGain_clip1)
#define SWREG_PQDS_COR_GAIN_0               (DISP_COLOR_SWREG_PQDS_BASE + PQDS_iCorGain_clip0)
#define SWREG_PQDS_GAIN_2                   (DISP_COLOR_SWREG_PQDS_BASE + PQDS_iGain_clip2)
#define SWREG_PQDS_GAIN_1                   (DISP_COLOR_SWREG_PQDS_BASE + PQDS_iGain_clip1)
#define SWREG_PQDS_GAIN_0                   (DISP_COLOR_SWREG_PQDS_BASE + PQDS_iGain_clip0)

//6797 new reg
#define SWREG_PQDS_HIGH_COR_ZERO_2          (DISP_COLOR_SWREG_PQDS_BASE + PQDS_iHighCorZero_clip2)
#define SWREG_PQDS_HIGH_COR_ZERO_1          (DISP_COLOR_SWREG_PQDS_BASE + PQDS_iHighCorZero_clip1)
#define SWREG_PQDS_HIGH_COR_ZERO_0          (DISP_COLOR_SWREG_PQDS_BASE + PQDS_iHighCorZero_clip0)
#define SWREG_PQDS_HIGH_COR_THR_2           (DISP_COLOR_SWREG_PQDS_BASE + PQDS_iHighCorThr_clip2)
#define SWREG_PQDS_HIGH_COR_THR_1           (DISP_COLOR_SWREG_PQDS_BASE + PQDS_iHighCorThr_clip1)
#define SWREG_PQDS_HIGH_COR_THR_0           (DISP_COLOR_SWREG_PQDS_BASE + PQDS_iHighCorThr_clip0)
#define SWREG_PQDS_HIGH_COR_GAIN_2          (DISP_COLOR_SWREG_PQDS_BASE + PQDS_iHighCorGain_clip2)
#define SWREG_PQDS_HIGH_COR_GAIN_1          (DISP_COLOR_SWREG_PQDS_BASE + PQDS_iHighCorGain_clip1)
#define SWREG_PQDS_HIGH_COR_GAIN_0          (DISP_COLOR_SWREG_PQDS_BASE + PQDS_iHighCorGain_clip0)

#define SWREG_PQDS_MID_COR_ZERO_2           (DISP_COLOR_SWREG_PQDS_BASE + PQDS_iMidCorZero_clip2)
#define SWREG_PQDS_MID_COR_ZERO_1           (DISP_COLOR_SWREG_PQDS_BASE + PQDS_iMidCorZero_clip1)
#define SWREG_PQDS_MID_COR_ZERO_0           (DISP_COLOR_SWREG_PQDS_BASE + PQDS_iMidCorZero_clip0)
#define SWREG_PQDS_MID_COR_THR_2            (DISP_COLOR_SWREG_PQDS_BASE + PQDS_iMidCorThr_clip2)
#define SWREG_PQDS_MID_COR_THR_1            (DISP_COLOR_SWREG_PQDS_BASE + PQDS_iMidCorThr_clip1)
#define SWREG_PQDS_MID_COR_THR_0            (DISP_COLOR_SWREG_PQDS_BASE + PQDS_iMidCorThr_clip0)
#define SWREG_PQDS_MID_COR_GAIN_2           (DISP_COLOR_SWREG_PQDS_BASE + PQDS_iMidCorGain_clip2)
#define SWREG_PQDS_MID_COR_GAIN_1           (DISP_COLOR_SWREG_PQDS_BASE + PQDS_iMidCorGain_clip1)
#define SWREG_PQDS_MID_COR_GAIN_0           (DISP_COLOR_SWREG_PQDS_BASE + PQDS_iMidCorGain_clip0)

#define SWREG_PQDS_EDF_FLAT_TH_2            (DISP_COLOR_SWREG_PQDS_BASE + PQDS_i_edf_flat_th_clip2)
#define SWREG_PQDS_EDF_FLAT_TH_1            (DISP_COLOR_SWREG_PQDS_BASE + PQDS_i_edf_flat_th_clip1)
#define SWREG_PQDS_EDF_FLAT_TH_0            (DISP_COLOR_SWREG_PQDS_BASE + PQDS_i_edf_flat_th_clip0)
#define SWREG_PQDS_EDF_DETAIL_RISE_TH_2     (DISP_COLOR_SWREG_PQDS_BASE + PQDS_i_edf_detail_rise_th_clip2)
#define SWREG_PQDS_EDF_DETAIL_RISE_TH_1     (DISP_COLOR_SWREG_PQDS_BASE + PQDS_i_edf_detail_rise_th_clip1)
#define SWREG_PQDS_EDF_DETAIL_RISE_TH_0     (DISP_COLOR_SWREG_PQDS_BASE + PQDS_i_edf_detail_rise_th_clip0)

//UltraResolution registers
#define SWREG_PQDS_ULTRARES_EN              (DISP_COLOR_SWREG_PQDS_BASE + PQDS_iUltraRes_en)
#define SWREG_PQDS_ULTRARES_RATIO_THR_2     (DISP_COLOR_SWREG_PQDS_BASE + PQDS_iUltraRes_ratio_thr2)
#define SWREG_PQDS_ULTRARES_RATIO_THR_1     (DISP_COLOR_SWREG_PQDS_BASE + PQDS_iUltraRes_ratio_thr1)
#define SWREG_PQDS_ULTRARES_RATIO_THR_0     (DISP_COLOR_SWREG_PQDS_BASE + PQDS_iUltraRes_ratio_thr0)
#define SWREG_PQDS_ULTRARES_GAIN_MID_1      (DISP_COLOR_SWREG_PQDS_BASE + PQDS_iUltraRes_gain_mid_clip1)
#define SWREG_PQDS_ULTRARES_GAIN_MID_0      (DISP_COLOR_SWREG_PQDS_BASE + PQDS_iUltraRes_gain_mid_clip0)
#define SWREG_PQDS_ULTRARES_GAIN_HIGH_1     (DISP_COLOR_SWREG_PQDS_BASE + PQDS_iUltraRes_gain_high_clip1)
#define SWREG_PQDS_ULTRARES_GAIN_HIGH_0     (DISP_COLOR_SWREG_PQDS_BASE + PQDS_iUltraRes_gain_high_clip0)
#define SWREG_PQDS_ULTRARES_EDF_EDGE_GAIN_1 (DISP_COLOR_SWREG_PQDS_BASE + PQDS_iUltraRes_edf_edge_gain_clip1)
#define SWREG_PQDS_ULTRARES_EDF_EDGE_GAIN_0 (DISP_COLOR_SWREG_PQDS_BASE + PQDS_iUltraRes_edf_edge_gain_clip0)
#define SWREG_PQDS_ULTRARES_CLIP_THR_1      (DISP_COLOR_SWREG_PQDS_BASE + PQDS_iUltraRes_clip_thr_clip1)
#define SWREG_PQDS_ULTRARES_CLIP_THR_0      (DISP_COLOR_SWREG_PQDS_BASE + PQDS_iUltraRes_clip_thr_clip0)
#define SWREG_PQDS_ULTRARES_CLIP_RATIO_1    (DISP_COLOR_SWREG_PQDS_BASE + PQDS_iUltraRes_clip_ratio_clip1)
#define SWREG_PQDS_ULTRARES_CLIP_RATIO_0    (DISP_COLOR_SWREG_PQDS_BASE + PQDS_iUltraRes_clip_ratio_clip0)

#define SWREG_PQDS_END                      (DISP_COLOR_SWREG_PQDS_BASE + PQDS_INDEX_MAX)

#define ASHMEM_SIZE (PQAshmemProxy::getAshmemSize())
#define EXTERNAL_PANEL_NITS_DEFAULT         (300)

#define PQFeatureID_DRE ((int)PQFeatureID::PQ_FEATURE_MAX + 1)
#define PQFeatureID_CCORR ((int)PQFeatureID::PQ_FEATURE_MAX + 2)

enum ASHMEM_ENUM
{
    ASHMEM_TUNING_FLAG = 0,
    DEBUG_PROPERTY,
    SHP_ENABLE,
    DSHP_ENABLE,
    VIDEO_CONTENT_COLOR_ENABLE,
    DC_ENABLE,
    ISO_SHP_ENABLE,
    UR_ENABLE,
    HFG_ENABLE,
    CONTENT_COLOR_ENABLE,
    HDR_VIDEO_ENABLE,
    DC_DEBUG_FLAG,
    HDR_DEBUG_FLAG,
    EXTERNAL_PANEL_NITS,
    HDR_DRIVER_DEBUG_FLAG,
    PQ_PARAM_SN,
    MDP_CCORR_DEBUG_FLAG,
    MDP_DRE_ENABLE,
    MDP_DRE_VP_ENABLE,
    MDP_DRE_HDR_VP_ENABLE,
    MDP_DRE_DEBUG_FLAG,
    ADAPTIVE_CALTM_DEBUG_FLAG,
    MDP_DREDRIVER_DEBUG_FLAG,
    MDP_DRE_DEMOWIN_X_FLAG,
    MDP_DREDRIVER_BLK_FLAG,
    MDP_DRE_ISPTUNING_FLAG,
    MDP_CZ_ISPTUNING_FLAG,
    MDP_HFG_ISPTUNING_FLAG,
};
#endif

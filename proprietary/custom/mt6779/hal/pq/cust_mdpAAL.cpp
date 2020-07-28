#include "mdpAALVPImpl.h"
#include <string.h>
extern "C" {

// 0~9 HDR
// 10~19 SCLTM

DREReg iDRERegEntry[DRE_MODE_NUM * SET_NUM] =
{
    // HDR
    // ###### 00 ######
    {
        .dre_fw_en           = 1,
        .dre_curve_en        = 1,
        .dre_gain_flt_en     = 1,

        .bADLWeight1 = 0x14,
        .bADLWeight2 = 0x2C,
        .bADLWeight3 = 0x00,

        .bBSDCGain = 0x06,
        .bBSACGain = 0x70,
        .bBSLevel  = 0x04,

        .bMIDDCGain = 0x06,
        .bMIDACGain = 0x25,

        .bWSDCGain = 0x06,
        .bWSACGain = 0x10,
        .bWSLevel  = 0x05,

        .dre_dync_spike_wgt_min = 0x0,
        .dre_dync_spike_wgt_max = 0xF5,
        .dre_dync_spike_th      = 0x60,
        .dre_dync_spike_slope   = 0x27,
        .bSpikeBlendmethod      = 0x2,

        .bSkinWgtSlope    = 0x80,
        .bSkinBlendmethod = 0x2,
        .SkinDummy1       = 0,
        .Layer0_Ratio     = 0x80,
        .Layer1_Ratio     = 0x10,
        .Layer2_Ratio     = 0x00,
        .Dark_Ratio       = 0x100,
        .Bright_Ratio     = 0x100,

        .dre_dync_flt_coef_min      = 0x03,
        .dre_dync_flt_coef_max      = 0xA0,
        .dre_dync_flt_ovr_pxl_th    = 0x2A,
        .dre_dync_flt_ovr_pxl_slope = 0x15,
        .dre_iir_force_range        = 0x01,

        .LLPValue = 0x0,
        .LLPRatio = 0x0,

        .APLCompRatioLow  = 0xFF,
        .APLCompRatioHigh = 0x02,
 
        .FltConfSlope = 0x880,
        .FltConfTh    = 0xF0,

        .BlkHistCountRatio = 0x3,
        .BinIdxDiffSlope   = 0x08,
        .BinIdxDiffTh      = 0x03,
        .BinIdxDiffWgtOft  = 0xF0,
        .APLTh             = 0x00,
        .APLSlope          = 0x7F,
        .APLWgtOft         = 0x100,
 
        .APL2Th     = 0xFF,
        .APL2Slope  = 0x80,
        .APL2WgtOft = 0x100,
        .APL2WgtMax = 0x100,

        .BlkSpaFltEn   = 1,
        .BlkSpaFltType = 0x1,
 
        .LoadBlkCurveEn = 0,
        .SaveBlkCurveEn = 0,
  
        .Flat_Length_Th_Base = 0,
        .Flat_Length_Slope_Base = 0xC8,
  
        .DZ_Fallback_En    = 1,
        .DZ_Size_Th        = 0x32,
        .DZ_Size_Slope     = 0x40,

        .bGreenWgtSlope    = 0x80,
        .dre_dync_compact_wgt_min = 0x0,
        .dre_dync_compact_wgt_max = 0xFF,
        .dre_dync_compact_th     = 0x40,
        .dre_dync_compact_slope   = 0x40,
        .bCompactBlendmethod      = 0x3,
        .dre_dync_compact_LowBoundBin = 0x7,
        .dre_dync_compact_HighBoundBin = 0x9,
        .dre_dync_compact_BinMinWeight = 0x100,
        .dre_dync_compact_BinMaxWeight = 0x280,
    },

    // HDR
    // ###### 01 ######
    {
        .dre_fw_en           = 1,
        .dre_curve_en        = 1,
        .dre_gain_flt_en     = 1,

        .bADLWeight1 = 0x14,
        .bADLWeight2 = 0x2C,
        .bADLWeight3 = 0x00,

        .bBSDCGain = 0x06,
        .bBSACGain = 0x70,
        .bBSLevel  = 0x04,

        .bMIDDCGain = 0x06,
        .bMIDACGain = 0x25,

        .bWSDCGain = 0x06,
        .bWSACGain = 0x10,
        .bWSLevel  = 0x05,

        .dre_dync_spike_wgt_min = 0x0,
        .dre_dync_spike_wgt_max = 0xF5,
        .dre_dync_spike_th      = 0x60,
        .dre_dync_spike_slope   = 0x27,
        .bSpikeBlendmethod      = 0x2,

        .bSkinWgtSlope    = 0x80,
        .bSkinBlendmethod = 0x2,
        .SkinDummy1       = 0,
        .Layer0_Ratio     = 0x80,
        .Layer1_Ratio     = 0x10,
        .Layer2_Ratio     = 0x00,
        .Dark_Ratio       = 0x100,
        .Bright_Ratio     = 0x100,

        .dre_dync_flt_coef_min      = 0x03,
        .dre_dync_flt_coef_max      = 0xA0,
        .dre_dync_flt_ovr_pxl_th    = 0x2A,
        .dre_dync_flt_ovr_pxl_slope = 0x15,
        .dre_iir_force_range        = 0x01,

        .LLPValue = 0x0,
        .LLPRatio = 0x0,

        .APLCompRatioLow  = 0xFF,
        .APLCompRatioHigh = 0x02,
 
        .FltConfSlope = 0x880,
        .FltConfTh    = 0xF0,

        .BlkHistCountRatio = 0x3,
        .BinIdxDiffSlope   = 0x08,
        .BinIdxDiffTh      = 0x03,
        .BinIdxDiffWgtOft  = 0xF0,
        .APLTh             = 0x00,
        .APLSlope          = 0x7F,
        .APLWgtOft         = 0x100,
 
        .APL2Th     = 0xFF,
        .APL2Slope  = 0x80,
        .APL2WgtOft = 0x100,
        .APL2WgtMax = 0x100,

        .BlkSpaFltEn   = 1,
        .BlkSpaFltType = 0x1,
 
        .LoadBlkCurveEn = 0,
        .SaveBlkCurveEn = 0,
  
        .Flat_Length_Th_Base = 0,
        .Flat_Length_Slope_Base = 0xC8,
  
        .DZ_Fallback_En    = 1,
        .DZ_Size_Th        = 0x32,
        .DZ_Size_Slope     = 0x40,

        .bGreenWgtSlope    = 0x80,
        .dre_dync_compact_wgt_min = 0x0,
        .dre_dync_compact_wgt_max = 0xFF,
        .dre_dync_compact_th     = 0x40,
        .dre_dync_compact_slope   = 0x40,
        .bCompactBlendmethod      = 0x3,
        .dre_dync_compact_LowBoundBin = 0x7,
        .dre_dync_compact_HighBoundBin = 0x9,
        .dre_dync_compact_BinMinWeight = 0x100,
        .dre_dync_compact_BinMaxWeight = 0x280,
    },

    // HDR
    // ###### 02 ######
    {
        .dre_fw_en           = 1,
        .dre_curve_en        = 1,
        .dre_gain_flt_en     = 1,

        .bADLWeight1 = 0x14,
        .bADLWeight2 = 0x2C,
        .bADLWeight3 = 0x00,

        .bBSDCGain = 0x06,
        .bBSACGain = 0x70,
        .bBSLevel  = 0x04,

        .bMIDDCGain = 0x06,
        .bMIDACGain = 0x25,

        .bWSDCGain = 0x06,
        .bWSACGain = 0x10,
        .bWSLevel  = 0x05,

        .dre_dync_spike_wgt_min = 0x0,
        .dre_dync_spike_wgt_max = 0xF5,
        .dre_dync_spike_th      = 0x60,
        .dre_dync_spike_slope   = 0x27,
        .bSpikeBlendmethod      = 0x2,

        .bSkinWgtSlope    = 0x80,
        .bSkinBlendmethod = 0x2,
        .SkinDummy1       = 0,
        .Layer0_Ratio     = 0x80,
        .Layer1_Ratio     = 0x10,
        .Layer2_Ratio     = 0x00,
        .Dark_Ratio       = 0x100,
        .Bright_Ratio     = 0x100,

        .dre_dync_flt_coef_min      = 0x03,
        .dre_dync_flt_coef_max      = 0xA0,
        .dre_dync_flt_ovr_pxl_th    = 0x2A,
        .dre_dync_flt_ovr_pxl_slope = 0x15,
        .dre_iir_force_range        = 0x01,

        .LLPValue = 0x0,
        .LLPRatio = 0x0,

        .APLCompRatioLow  = 0xFF,
        .APLCompRatioHigh = 0x02,
 
        .FltConfSlope = 0x880,
        .FltConfTh    = 0xF0,

        .BlkHistCountRatio = 0x3,
        .BinIdxDiffSlope   = 0x08,
        .BinIdxDiffTh      = 0x03,
        .BinIdxDiffWgtOft  = 0xF0,
        .APLTh             = 0x00,
        .APLSlope          = 0x7F,
        .APLWgtOft         = 0x100,
 
        .APL2Th     = 0xFF,
        .APL2Slope  = 0x80,
        .APL2WgtOft = 0x100,
        .APL2WgtMax = 0x100,

        .BlkSpaFltEn   = 1,
        .BlkSpaFltType = 0x1,
 
        .LoadBlkCurveEn = 0,
        .SaveBlkCurveEn = 0,
  
        .Flat_Length_Th_Base = 0,
        .Flat_Length_Slope_Base = 0xC8,
  
        .DZ_Fallback_En    = 1,
        .DZ_Size_Th        = 0x32,
        .DZ_Size_Slope     = 0x40,

        .bGreenWgtSlope    = 0x80,
        .dre_dync_compact_wgt_min = 0x0,
        .dre_dync_compact_wgt_max = 0xFF,
        .dre_dync_compact_th     = 0x40,
        .dre_dync_compact_slope   = 0x40,
        .bCompactBlendmethod      = 0x3,
        .dre_dync_compact_LowBoundBin = 0x7,
        .dre_dync_compact_HighBoundBin = 0x9,
        .dre_dync_compact_BinMinWeight = 0x100,
        .dre_dync_compact_BinMaxWeight = 0x280,
    },

    // HDR
    // ###### 03 ######
    {
        .dre_fw_en           = 1,
        .dre_curve_en        = 1,
        .dre_gain_flt_en     = 1,

        .bADLWeight1 = 0x14,
        .bADLWeight2 = 0x2C,
        .bADLWeight3 = 0x00,

        .bBSDCGain = 0x10,
        .bBSACGain = 0x80,
        .bBSLevel  = 0x00,

        .bMIDDCGain = 0x10,
        .bMIDACGain = 0x80,

        .bWSDCGain = 0x10,
        .bWSACGain = 0x80,
        .bWSLevel  = 0x0,

        .dre_dync_spike_wgt_min = 0x0,
        .dre_dync_spike_wgt_max = 0xFF,
        .dre_dync_spike_th      = 0x90,
        .dre_dync_spike_slope   = 0x40,
        .bSpikeBlendmethod      = 0x2,

        .bSkinWgtSlope    = 0x80,
        .bSkinBlendmethod = 0x2,
        .SkinDummy1       = 0,
        .Layer0_Ratio     = 0x80,
        .Layer1_Ratio     = 0x10,
        .Layer2_Ratio     = 0x00,
        .Dark_Ratio       = 0x100,
        .Bright_Ratio     = 0x100,

        .dre_dync_flt_coef_min      = 0x8,
        .dre_dync_flt_coef_max      = 0x20,
        .dre_dync_flt_ovr_pxl_th    = 0x4,
        .dre_dync_flt_ovr_pxl_slope = 0x32,
        .dre_iir_force_range        = 0x1,

        .LLPValue = 0x0,
        .LLPRatio = 0x0,

        .APLCompRatioLow  = 0xFF,
        .APLCompRatioHigh = 0xFF,
 
        .FltConfSlope = 0x12C,
        .FltConfTh    = 0x0,

        .BlkHistCountRatio = 0x3,
        .BinIdxDiffSlope   = 0xE0,
        .BinIdxDiffTh      = 0x0,
        .BinIdxDiffWgtOft  = 0x20,
        .APLTh             = 0x10,
        .APLSlope          = 0x80,
        .APLWgtOft         = 0x100,
 
        .APL2Th     = 0x10,
        .APL2Slope  = 0x80,
        .APL2WgtOft = 0x100,
        .APL2WgtMax = 0x100,

        .BlkSpaFltEn   = 1,
        .BlkSpaFltType = 0x1,
 
        .LoadBlkCurveEn = 0,
        .SaveBlkCurveEn = 0,
  
        .Flat_Length_Th_Base = 0,
        .Flat_Length_Slope_Base = 384,
  
        .DZ_Fallback_En    = 1,
        .DZ_Size_Th        = 0x32,
        .DZ_Size_Slope     = 0x40,

        .bGreenWgtSlope    = 0x80,
        .dre_dync_compact_wgt_min = 0x0,
        .dre_dync_compact_wgt_max = 0xFF,
        .dre_dync_compact_th     = 128,
        .dre_dync_compact_slope   = 0x40,
        .bCompactBlendmethod      = 0x2,
        .dre_dync_compact_LowBoundBin = 0x5,
        .dre_dync_compact_HighBoundBin = 0x7,
        .dre_dync_compact_BinMinWeight = 256,
        .dre_dync_compact_BinMaxWeight = 640,
    },

    // HDR
    // ###### 04 ######
    {
        .dre_fw_en           = 1,
        .dre_curve_en        = 1,
        .dre_gain_flt_en     = 1,

        .bADLWeight1 = 0x14,
        .bADLWeight2 = 0x2C,
        .bADLWeight3 = 0x00,

        .bBSDCGain = 0x10,
        .bBSACGain = 0x80,
        .bBSLevel  = 0x00,

        .bMIDDCGain = 0x10,
        .bMIDACGain = 0x80,

        .bWSDCGain = 0x10,
        .bWSACGain = 0x80,
        .bWSLevel  = 0x0,

        .dre_dync_spike_wgt_min = 0x0,
        .dre_dync_spike_wgt_max = 0xFF,
        .dre_dync_spike_th      = 0x90,
        .dre_dync_spike_slope   = 0x40,
        .bSpikeBlendmethod      = 0x2,

        .bSkinWgtSlope    = 0x80,
        .bSkinBlendmethod = 0x2,
        .SkinDummy1       = 0,
        .Layer0_Ratio     = 0x80,
        .Layer1_Ratio     = 0x10,
        .Layer2_Ratio     = 0x00,
        .Dark_Ratio       = 0x100,
        .Bright_Ratio     = 0x100,

        .dre_dync_flt_coef_min      = 0x8,
        .dre_dync_flt_coef_max      = 0x20,
        .dre_dync_flt_ovr_pxl_th    = 0x4,
        .dre_dync_flt_ovr_pxl_slope = 0x32,
        .dre_iir_force_range        = 0x1,

        .LLPValue = 0x0,
        .LLPRatio = 0x0,

        .APLCompRatioLow  = 0xFF,
        .APLCompRatioHigh = 0xFF,
 
        .FltConfSlope = 0x12C,
        .FltConfTh    = 0x0,

        .BlkHistCountRatio = 0x3,
        .BinIdxDiffSlope   = 0xE0,
        .BinIdxDiffTh      = 0x0,
        .BinIdxDiffWgtOft  = 0x20,
        .APLTh             = 0x10,
        .APLSlope          = 0x80,
        .APLWgtOft         = 0x100,
 
        .APL2Th     = 0x10,
        .APL2Slope  = 0x80,
        .APL2WgtOft = 0x100,
        .APL2WgtMax = 0x100,

        .BlkSpaFltEn   = 1,
        .BlkSpaFltType = 0x1,
 
        .LoadBlkCurveEn = 0,
        .SaveBlkCurveEn = 0,
  
        .Flat_Length_Th_Base = 0,
        .Flat_Length_Slope_Base = 384,
  
        .DZ_Fallback_En    = 1,
        .DZ_Size_Th        = 0x32,
        .DZ_Size_Slope     = 0x40,

        .bGreenWgtSlope    = 0x80,
        .dre_dync_compact_wgt_min = 0x0,
        .dre_dync_compact_wgt_max = 0xFF,
        .dre_dync_compact_th     = 128,
        .dre_dync_compact_slope   = 0x40,
        .bCompactBlendmethod      = 0x2,
        .dre_dync_compact_LowBoundBin = 0x5,
        .dre_dync_compact_HighBoundBin = 0x7,
        .dre_dync_compact_BinMinWeight = 256,
        .dre_dync_compact_BinMaxWeight = 640,
    },

    // HDR
    // ###### 05 ######
    {
        .dre_fw_en           = 1,
        .dre_curve_en        = 1,
        .dre_gain_flt_en     = 1,

        .bADLWeight1 = 0x14,
        .bADLWeight2 = 0x2C,
        .bADLWeight3 = 0x00,

        .bBSDCGain = 0x10,
        .bBSACGain = 0x80,
        .bBSLevel  = 0x00,

        .bMIDDCGain = 0x10,
        .bMIDACGain = 0x80,

        .bWSDCGain = 0x10,
        .bWSACGain = 0x80,
        .bWSLevel  = 0x0,

        .dre_dync_spike_wgt_min = 0x0,
        .dre_dync_spike_wgt_max = 0xFF,
        .dre_dync_spike_th      = 0x90,
        .dre_dync_spike_slope   = 0x40,
        .bSpikeBlendmethod      = 0x2,

        .bSkinWgtSlope    = 0x80,
        .bSkinBlendmethod = 0x2,
        .SkinDummy1       = 0,
        .Layer0_Ratio     = 0x80,
        .Layer1_Ratio     = 0x10,
        .Layer2_Ratio     = 0x00,
        .Dark_Ratio       = 0x100,
        .Bright_Ratio     = 0x100,

        .dre_dync_flt_coef_min      = 0x8,
        .dre_dync_flt_coef_max      = 0x20,
        .dre_dync_flt_ovr_pxl_th    = 0x4,
        .dre_dync_flt_ovr_pxl_slope = 0x32,
        .dre_iir_force_range        = 0x1,

        .LLPValue = 0x0,
        .LLPRatio = 0x0,

        .APLCompRatioLow  = 0xFF,
        .APLCompRatioHigh = 0xFF,
 
        .FltConfSlope = 0x12C,
        .FltConfTh    = 0x0,

        .BlkHistCountRatio = 0x3,
        .BinIdxDiffSlope   = 0xE0,
        .BinIdxDiffTh      = 0x0,
        .BinIdxDiffWgtOft  = 0x20,
        .APLTh             = 0x10,
        .APLSlope          = 0x80,
        .APLWgtOft         = 0x100,
 
        .APL2Th     = 0x10,
        .APL2Slope  = 0x80,
        .APL2WgtOft = 0x100,
        .APL2WgtMax = 0x100,

        .BlkSpaFltEn   = 1,
        .BlkSpaFltType = 0x1,
 
        .LoadBlkCurveEn = 0,
        .SaveBlkCurveEn = 0,
  
        .Flat_Length_Th_Base = 0,
        .Flat_Length_Slope_Base = 384,
  
        .DZ_Fallback_En    = 1,
        .DZ_Size_Th        = 0x32,
        .DZ_Size_Slope     = 0x40,

        .bGreenWgtSlope    = 0x80,
        .dre_dync_compact_wgt_min = 0x0,
        .dre_dync_compact_wgt_max = 0xFF,
        .dre_dync_compact_th     = 128,
        .dre_dync_compact_slope   = 0x40,
        .bCompactBlendmethod      = 0x2,
        .dre_dync_compact_LowBoundBin = 0x5,
        .dre_dync_compact_HighBoundBin = 0x7,
        .dre_dync_compact_BinMinWeight = 256,
        .dre_dync_compact_BinMaxWeight = 640,
    },

    // HDR
    // ###### 06 ######
    {
        .dre_fw_en           = 1,
        .dre_curve_en        = 1,
        .dre_gain_flt_en     = 1,

        .bADLWeight1 = 0x14,
        .bADLWeight2 = 0x2C,
        .bADLWeight3 = 0x00,

        .bBSDCGain = 0x10,
        .bBSACGain = 0x80,
        .bBSLevel  = 0x00,

        .bMIDDCGain = 0x10,
        .bMIDACGain = 0x80,

        .bWSDCGain = 0x10,
        .bWSACGain = 0x80,
        .bWSLevel  = 0x0,

        .dre_dync_spike_wgt_min = 0x0,
        .dre_dync_spike_wgt_max = 0xFF,
        .dre_dync_spike_th      = 0x90,
        .dre_dync_spike_slope   = 0x40,
        .bSpikeBlendmethod      = 0x2,

        .bSkinWgtSlope    = 0x80,
        .bSkinBlendmethod = 0x2,
        .SkinDummy1       = 0,
        .Layer0_Ratio     = 0x80,
        .Layer1_Ratio     = 0x10,
        .Layer2_Ratio     = 0x00,
        .Dark_Ratio       = 0x100,
        .Bright_Ratio     = 0x100,

        .dre_dync_flt_coef_min      = 0x8,
        .dre_dync_flt_coef_max      = 0x20,
        .dre_dync_flt_ovr_pxl_th    = 0x4,
        .dre_dync_flt_ovr_pxl_slope = 0x32,
        .dre_iir_force_range        = 0x1,

        .LLPValue = 0x0,
        .LLPRatio = 0x0,

        .APLCompRatioLow  = 0xFF,
        .APLCompRatioHigh = 0xFF,
 
        .FltConfSlope = 0x12C,
        .FltConfTh    = 0x0,

        .BlkHistCountRatio = 0x3,
        .BinIdxDiffSlope   = 0xE0,
        .BinIdxDiffTh      = 0x0,
        .BinIdxDiffWgtOft  = 0x20,
        .APLTh             = 0x10,
        .APLSlope          = 0x80,
        .APLWgtOft         = 0x100,
 
        .APL2Th     = 0x10,
        .APL2Slope  = 0x80,
        .APL2WgtOft = 0x100,
        .APL2WgtMax = 0x100,

        .BlkSpaFltEn   = 1,
        .BlkSpaFltType = 0x1,
 
        .LoadBlkCurveEn = 0,
        .SaveBlkCurveEn = 0,
  
        .Flat_Length_Th_Base = 0,
        .Flat_Length_Slope_Base = 384,
  
        .DZ_Fallback_En    = 1,
        .DZ_Size_Th        = 0x32,
        .DZ_Size_Slope     = 0x40,

        .bGreenWgtSlope    = 0x80,
        .dre_dync_compact_wgt_min = 0x0,
        .dre_dync_compact_wgt_max = 0xFF,
        .dre_dync_compact_th     = 128,
        .dre_dync_compact_slope   = 0x40,
        .bCompactBlendmethod      = 0x2,
        .dre_dync_compact_LowBoundBin = 0x5,
        .dre_dync_compact_HighBoundBin = 0x7,
        .dre_dync_compact_BinMinWeight = 256,
        .dre_dync_compact_BinMaxWeight = 640,
    },

    // HDR
    // ###### 07 ######
    {
        .dre_fw_en           = 1,
        .dre_curve_en        = 1,
        .dre_gain_flt_en     = 1,

        .bADLWeight1 = 0x14,
        .bADLWeight2 = 0x2C,
        .bADLWeight3 = 0x00,

        .bBSDCGain = 0x10,
        .bBSACGain = 0x80,
        .bBSLevel  = 0x00,

        .bMIDDCGain = 0x10,
        .bMIDACGain = 0x80,

        .bWSDCGain = 0x10,
        .bWSACGain = 0x80,
        .bWSLevel  = 0x0,

        .dre_dync_spike_wgt_min = 0x0,
        .dre_dync_spike_wgt_max = 0xFF,
        .dre_dync_spike_th      = 0x90,
        .dre_dync_spike_slope   = 0x40,
        .bSpikeBlendmethod      = 0x2,

        .bSkinWgtSlope    = 0x80,
        .bSkinBlendmethod = 0x2,
        .SkinDummy1       = 0,
        .Layer0_Ratio     = 0x80,
        .Layer1_Ratio     = 0x10,
        .Layer2_Ratio     = 0x00,
        .Dark_Ratio       = 0x100,
        .Bright_Ratio     = 0x100,

        .dre_dync_flt_coef_min      = 0x8,
        .dre_dync_flt_coef_max      = 0x20,
        .dre_dync_flt_ovr_pxl_th    = 0x4,
        .dre_dync_flt_ovr_pxl_slope = 0x32,
        .dre_iir_force_range        = 0x1,

        .LLPValue = 0x0,
        .LLPRatio = 0x0,

        .APLCompRatioLow  = 0xFF,
        .APLCompRatioHigh = 0xFF,
 
        .FltConfSlope = 0x12C,
        .FltConfTh    = 0x0,

        .BlkHistCountRatio = 0x3,
        .BinIdxDiffSlope   = 0xE0,
        .BinIdxDiffTh      = 0x0,
        .BinIdxDiffWgtOft  = 0x20,
        .APLTh             = 0x10,
        .APLSlope          = 0x80,
        .APLWgtOft         = 0x100,
 
        .APL2Th     = 0x10,
        .APL2Slope  = 0x80,
        .APL2WgtOft = 0x100,
        .APL2WgtMax = 0x100,

        .BlkSpaFltEn   = 1,
        .BlkSpaFltType = 0x1,
 
        .LoadBlkCurveEn = 0,
        .SaveBlkCurveEn = 0,
  
        .Flat_Length_Th_Base = 0,
        .Flat_Length_Slope_Base = 384,
  
        .DZ_Fallback_En    = 1,
        .DZ_Size_Th        = 0x32,
        .DZ_Size_Slope     = 0x40,

        .bGreenWgtSlope    = 0x80,
        .dre_dync_compact_wgt_min = 0x0,
        .dre_dync_compact_wgt_max = 0xFF,
        .dre_dync_compact_th     = 128,
        .dre_dync_compact_slope   = 0x40,
        .bCompactBlendmethod      = 0x2,
        .dre_dync_compact_LowBoundBin = 0x5,
        .dre_dync_compact_HighBoundBin = 0x7,
        .dre_dync_compact_BinMinWeight = 256,
        .dre_dync_compact_BinMaxWeight = 640,
    },

    // HDR
    // ###### 08 ######
    {
        .dre_fw_en           = 1,
        .dre_curve_en        = 1,
        .dre_gain_flt_en     = 1,

        .bADLWeight1 = 0x14,
        .bADLWeight2 = 0x2C,
        .bADLWeight3 = 0x00,

        .bBSDCGain = 0x10,
        .bBSACGain = 0x80,
        .bBSLevel  = 0x00,

        .bMIDDCGain = 0x10,
        .bMIDACGain = 0x80,

        .bWSDCGain = 0x10,
        .bWSACGain = 0x80,
        .bWSLevel  = 0x0,

        .dre_dync_spike_wgt_min = 0x0,
        .dre_dync_spike_wgt_max = 0xFF,
        .dre_dync_spike_th      = 0x90,
        .dre_dync_spike_slope   = 0x40,
        .bSpikeBlendmethod      = 0x2,

        .bSkinWgtSlope    = 0x80,
        .bSkinBlendmethod = 0x2,
        .SkinDummy1       = 0,
        .Layer0_Ratio     = 0x80,
        .Layer1_Ratio     = 0x10,
        .Layer2_Ratio     = 0x00,
        .Dark_Ratio       = 0x100,
        .Bright_Ratio     = 0x100,

        .dre_dync_flt_coef_min      = 0x8,
        .dre_dync_flt_coef_max      = 0x20,
        .dre_dync_flt_ovr_pxl_th    = 0x4,
        .dre_dync_flt_ovr_pxl_slope = 0x32,
        .dre_iir_force_range        = 0x1,

        .LLPValue = 0x0,
        .LLPRatio = 0x0,

        .APLCompRatioLow  = 0xFF,
        .APLCompRatioHigh = 0xFF,
 
        .FltConfSlope = 0x12C,
        .FltConfTh    = 0x0,

        .BlkHistCountRatio = 0x3,
        .BinIdxDiffSlope   = 0xE0,
        .BinIdxDiffTh      = 0x0,
        .BinIdxDiffWgtOft  = 0x20,
        .APLTh             = 0x10,
        .APLSlope          = 0x80,
        .APLWgtOft         = 0x100,
 
        .APL2Th     = 0x10,
        .APL2Slope  = 0x80,
        .APL2WgtOft = 0x100,
        .APL2WgtMax = 0x100,

        .BlkSpaFltEn   = 1,
        .BlkSpaFltType = 0x1,
 
        .LoadBlkCurveEn = 0,
        .SaveBlkCurveEn = 0,
  
        .Flat_Length_Th_Base = 0,
        .Flat_Length_Slope_Base = 384,
  
        .DZ_Fallback_En    = 1,
        .DZ_Size_Th        = 0x32,
        .DZ_Size_Slope     = 0x40,

        .bGreenWgtSlope    = 0x80,
        .dre_dync_compact_wgt_min = 0x0,
        .dre_dync_compact_wgt_max = 0xFF,
        .dre_dync_compact_th     = 128,
        .dre_dync_compact_slope   = 0x40,
        .bCompactBlendmethod      = 0x2,
        .dre_dync_compact_LowBoundBin = 0x5,
        .dre_dync_compact_HighBoundBin = 0x7,
        .dre_dync_compact_BinMinWeight = 256,
        .dre_dync_compact_BinMaxWeight = 640,
    },

    // HDR
    // ###### 09 ######
    {
        .dre_fw_en           = 1,
        .dre_curve_en        = 1,
        .dre_gain_flt_en     = 1,

        .bADLWeight1 = 0x14,
        .bADLWeight2 = 0x2C,
        .bADLWeight3 = 0x00,

        .bBSDCGain = 0x10,
        .bBSACGain = 0x80,
        .bBSLevel  = 0x00,

        .bMIDDCGain = 0x10,
        .bMIDACGain = 0x80,

        .bWSDCGain = 0x10,
        .bWSACGain = 0x80,
        .bWSLevel  = 0x0,

        .dre_dync_spike_wgt_min = 0x0,
        .dre_dync_spike_wgt_max = 0xFF,
        .dre_dync_spike_th      = 0x90,
        .dre_dync_spike_slope   = 0x40,
        .bSpikeBlendmethod      = 0x2,

        .bSkinWgtSlope    = 0x80,
        .bSkinBlendmethod = 0x2,
        .SkinDummy1       = 0,
        .Layer0_Ratio     = 0x80,
        .Layer1_Ratio     = 0x10,
        .Layer2_Ratio     = 0x00,
        .Dark_Ratio       = 0x100,
        .Bright_Ratio     = 0x100,

        .dre_dync_flt_coef_min      = 0x8,
        .dre_dync_flt_coef_max      = 0x20,
        .dre_dync_flt_ovr_pxl_th    = 0x4,
        .dre_dync_flt_ovr_pxl_slope = 0x32,
        .dre_iir_force_range        = 0x1,

        .LLPValue = 0x0,
        .LLPRatio = 0x0,

        .APLCompRatioLow  = 0xFF,
        .APLCompRatioHigh = 0xFF,
 
        .FltConfSlope = 0x12C,
        .FltConfTh    = 0x0,

        .BlkHistCountRatio = 0x3,
        .BinIdxDiffSlope   = 0xE0,
        .BinIdxDiffTh      = 0x0,
        .BinIdxDiffWgtOft  = 0x20,
        .APLTh             = 0x10,
        .APLSlope          = 0x80,
        .APLWgtOft         = 0x100,
 
        .APL2Th     = 0x10,
        .APL2Slope  = 0x80,
        .APL2WgtOft = 0x100,
        .APL2WgtMax = 0x100,

        .BlkSpaFltEn   = 1,
        .BlkSpaFltType = 0x1,
 
        .LoadBlkCurveEn = 0,
        .SaveBlkCurveEn = 0,
  
        .Flat_Length_Th_Base = 0,
        .Flat_Length_Slope_Base = 384,
  
        .DZ_Fallback_En    = 1,
        .DZ_Size_Th        = 0x32,
        .DZ_Size_Slope     = 0x40,

        .bGreenWgtSlope    = 0x80,
        .dre_dync_compact_wgt_min = 0x0,
        .dre_dync_compact_wgt_max = 0xFF,
        .dre_dync_compact_th     = 128,
        .dre_dync_compact_slope   = 0x40,
        .bCompactBlendmethod      = 0x2,
        .dre_dync_compact_LowBoundBin = 0x5,
        .dre_dync_compact_HighBoundBin = 0x7,
        .dre_dync_compact_BinMinWeight = 256,
        .dre_dync_compact_BinMaxWeight = 640,
    },

    //=======================================
    // SC-LTM
    // ###### 10 ######
    {
        .dre_fw_en           = 1,
        .dre_curve_en        = 1,
        .dre_gain_flt_en     = 1,

        .bADLWeight1 = 0x14,
        .bADLWeight2 = 0x2C,
        .bADLWeight3 = 0x00,

        .bBSDCGain = 0x6,
        .bBSACGain = 0x80,
        .bBSLevel  = 2,

        .bMIDDCGain = 0x6,
        .bMIDACGain = 0x20,

        .bWSDCGain = 0x4,
        .bWSACGain = 0xFF,
        .bWSLevel  = 4,

        .dre_dync_spike_wgt_min = 0x0,
        .dre_dync_spike_wgt_max = 0xF5,
        .dre_dync_spike_th      = 0xA0,
        .dre_dync_spike_slope   = 0x27,
        .bSpikeBlendmethod      = 0x2,

        .bSkinWgtSlope    = 0x80,
        .bSkinBlendmethod = 0x2,
        .SkinDummy1       = 0,
        .Layer0_Ratio     = 0x80,
        .Layer1_Ratio     = 0x10,
        .Layer2_Ratio     = 0x00,
        .Dark_Ratio       = 0x100,
        .Bright_Ratio     = 0x100,

        .dre_dync_flt_coef_min      = 0x3,
        .dre_dync_flt_coef_max      = 0xA0,
        .dre_dync_flt_ovr_pxl_th    = 0x2A,
        .dre_dync_flt_ovr_pxl_slope = 0x15,
        .dre_iir_force_range        = 0x1,

        .LLPValue = 0x0,
        .LLPRatio = 0x0,

        .APLCompRatioLow  = 0xFF,
        .APLCompRatioHigh = 0x02,

        .FltConfSlope = 0x880,
        .FltConfTh    = 0xF0,

        .BlkHistCountRatio = 0x3,
        .BinIdxDiffSlope   = 0x8,
        .BinIdxDiffTh      = 0x3,
        .BinIdxDiffWgtOft  = 0xF0,
        .APLTh             = 0x0,
        .APLSlope          = 0x7F,
        .APLWgtOft         = 0x100,

        .APL2Th     = 0xFF,
        .APL2Slope  = 0x80,
        .APL2WgtOft = 0x100,
        .APL2WgtMax = 0x100,

        .BlkSpaFltEn   = 1,
        .BlkSpaFltType = 0x1,

        .LoadBlkCurveEn = 0,
        .SaveBlkCurveEn = 0,

        .Flat_Length_Th_Base = 0,
        .Flat_Length_Slope_Base = 0xC8,

        .DZ_Fallback_En    = 1,
        .DZ_Size_Th        = 0x32,
        .DZ_Size_Slope     = 0x40,

        .bGreenWgtSlope    = 0x80,
        .dre_dync_compact_wgt_min = 0x0,
        .dre_dync_compact_wgt_max = 0xFF,
        .dre_dync_compact_th     = 0x40,
        .dre_dync_compact_slope   = 0x40,
        .bCompactBlendmethod      = 0x3,
        .dre_dync_compact_LowBoundBin = 0x7,
        .dre_dync_compact_HighBoundBin = 0x9,
        .dre_dync_compact_BinMinWeight = 0x100,
        .dre_dync_compact_BinMaxWeight = 0x280,
    },

    // SC-LTM
    // ###### 11 ######
    {       
        .dre_fw_en           = 1,
        .dre_curve_en        = 1,
        .dre_gain_flt_en     = 1,

        .bADLWeight1 = 0x14,
        .bADLWeight2 = 0x2C,
        .bADLWeight3 = 0x00,

        .bBSDCGain = 0x6,
        .bBSACGain = 0x80,
        .bBSLevel  = 2,

        .bMIDDCGain = 0x20,
        .bMIDACGain = 0x25,

        .bWSDCGain = 0x4,
        .bWSACGain = 0x20,
        .bWSLevel  = 4,

        .dre_dync_spike_wgt_min = 0x0,
        .dre_dync_spike_wgt_max = 0xF5,
        .dre_dync_spike_th      = 0x80,
        .dre_dync_spike_slope   = 0x42,
        .bSpikeBlendmethod      = 0x2,

        .bSkinWgtSlope    = 0x80,
        .bSkinBlendmethod = 0x2,
        .SkinDummy1       = 0,
        .Layer0_Ratio     = 0x80,
        .Layer1_Ratio     = 0x10,
        .Layer2_Ratio     = 0x00,
        .Dark_Ratio       = 0x100,
        .Bright_Ratio     = 0x100,

        .dre_dync_flt_coef_min      = 0x3,
        .dre_dync_flt_coef_max      = 0xA0,
        .dre_dync_flt_ovr_pxl_th    = 0x2A,
        .dre_dync_flt_ovr_pxl_slope = 0x15,
        .dre_iir_force_range        = 0x1,

        .LLPValue = 0x10,
        .LLPRatio = 0xFF,

        .APLCompRatioLow  = 0x2FF,
        .APLCompRatioHigh = 0x02,

        .FltConfSlope = 0x880,
        .FltConfTh    = 0xF0,

        .BlkHistCountRatio = 0x3,
        .BinIdxDiffSlope   = 0x8,
        .BinIdxDiffTh      = 0x3,
        .BinIdxDiffWgtOft  = 0xF0,
        .APLTh             = 0x0,
        .APLSlope          = 0x7F,
        .APLWgtOft         = 0x100,

        .APL2Th     = 0xFF,
        .APL2Slope  = 0x80,
        .APL2WgtOft = 0x100,
        .APL2WgtMax = 0x100,

        .BlkSpaFltEn   = 1,
        .BlkSpaFltType = 0x1,

        .LoadBlkCurveEn = 0,
        .SaveBlkCurveEn = 0,

        .Flat_Length_Th_Base = 0,
        .Flat_Length_Slope_Base = 0xC8,

        .DZ_Fallback_En    = 1,
        .DZ_Size_Th        = 0x32,
        .DZ_Size_Slope     = 0x40,

        .bGreenWgtSlope    = 0x80,
        .dre_dync_compact_wgt_min = 0x0,
        .dre_dync_compact_wgt_max = 0xFF,
        .dre_dync_compact_th     = 0x40,
        .dre_dync_compact_slope   = 0x40,
        .bCompactBlendmethod      = 0x3,
        .dre_dync_compact_LowBoundBin = 0x7,
        .dre_dync_compact_HighBoundBin = 0x9,
        .dre_dync_compact_BinMinWeight = 0x100,
        .dre_dync_compact_BinMaxWeight = 0x280,
    },

    // SC-LTM
    // ###### 12 ######
    { 
        .dre_fw_en           = 1,
        .dre_curve_en        = 1,
        .dre_gain_flt_en     = 1,

        .bADLWeight1 = 0x14,
        .bADLWeight2 = 0x2C,
        .bADLWeight3 = 0x00,

        .bBSDCGain = 0x6,
        .bBSACGain = 0x80,
        .bBSLevel  = 2,

        .bMIDDCGain = 0x20,
        .bMIDACGain = 0x25,

        .bWSDCGain = 0x4,
        .bWSACGain = 0x20,
        .bWSLevel  = 4,

        .dre_dync_spike_wgt_min = 0x0,
        .dre_dync_spike_wgt_max = 0xF5,
        .dre_dync_spike_th      = 0x80,
        .dre_dync_spike_slope   = 0x42,
        .bSpikeBlendmethod      = 0x2,

        .bSkinWgtSlope    = 0x80,
        .bSkinBlendmethod = 0x2,
        .SkinDummy1       = 0,
        .Layer0_Ratio     = 0x80,
        .Layer1_Ratio     = 0x10,
        .Layer2_Ratio     = 0x00,
        .Dark_Ratio       = 0x100,
        .Bright_Ratio     = 0x100,

        .dre_dync_flt_coef_min      = 0x3,
        .dre_dync_flt_coef_max      = 0xA0,
        .dre_dync_flt_ovr_pxl_th    = 0x2A,
        .dre_dync_flt_ovr_pxl_slope = 0x15,
        .dre_iir_force_range        = 0x1,

        .LLPValue = 0x10,
        .LLPRatio = 0xFF,

        .APLCompRatioLow  = 0x3FF,
        .APLCompRatioHigh = 0x02,

        .FltConfSlope = 0x880,
        .FltConfTh    = 0xF0,

        .BlkHistCountRatio = 0x3,
        .BinIdxDiffSlope   = 0x8,
        .BinIdxDiffTh      = 0x3,
        .BinIdxDiffWgtOft  = 0xF0,
        .APLTh             = 0x0,
        .APLSlope          = 0x7F,
        .APLWgtOft         = 0x100,

        .APL2Th     = 0xFF,
        .APL2Slope  = 0x80,
        .APL2WgtOft = 0x100,
        .APL2WgtMax = 0x100,

        .BlkSpaFltEn   = 1,
        .BlkSpaFltType = 0x1,

        .LoadBlkCurveEn = 0,
        .SaveBlkCurveEn = 0,

        .Flat_Length_Th_Base = 0,
        .Flat_Length_Slope_Base = 0xC8,

        .DZ_Fallback_En    = 1,
        .DZ_Size_Th        = 0x32,
        .DZ_Size_Slope     = 0x40,

        .bGreenWgtSlope    = 0x80,
        .dre_dync_compact_wgt_min = 0x0,
        .dre_dync_compact_wgt_max = 0xFF,
        .dre_dync_compact_th     = 0x40,
        .dre_dync_compact_slope   = 0x40,
        .bCompactBlendmethod      = 0x3,
        .dre_dync_compact_LowBoundBin = 0x7,
        .dre_dync_compact_HighBoundBin = 0x9,
        .dre_dync_compact_BinMinWeight = 0x100,
        .dre_dync_compact_BinMaxWeight = 0x280,
    },

    // SC-LTM
    // ###### 13 ######
    {
        .dre_fw_en           = 1,
        .dre_curve_en        = 1,
        .dre_gain_flt_en     = 1,

        .bADLWeight1 = 0x14,
        .bADLWeight2 = 0x2C,
        .bADLWeight3 = 0x00,

        .bBSDCGain = 0x6,
        .bBSACGain = 0x80,
        .bBSLevel  = 2,

        .bMIDDCGain = 0x6,
        .bMIDACGain = 0x20,

        .bWSDCGain = 0x4,
        .bWSACGain = 0xFF,
        .bWSLevel  = 4,

        .dre_dync_spike_wgt_min = 0x0,
        .dre_dync_spike_wgt_max = 0xF5,
        .dre_dync_spike_th      = 0xA0,
        .dre_dync_spike_slope   = 0x27,
        .bSpikeBlendmethod      = 0x2,
    
        .bSkinWgtSlope    = 0x80,
        .bSkinBlendmethod = 0x2,
        .SkinDummy1       = 0,
        .Layer0_Ratio     = 0x80,
        .Layer1_Ratio     = 0x10,
        .Layer2_Ratio     = 0x00,
        .Dark_Ratio       = 0x100,
        .Bright_Ratio     = 0x100,
           
        .dre_dync_flt_coef_min      = 0x3,
        .dre_dync_flt_coef_max      = 0xA0,
        .dre_dync_flt_ovr_pxl_th    = 0x2A,
        .dre_dync_flt_ovr_pxl_slope = 0x15,
        .dre_iir_force_range        = 0x1,
  
        .LLPValue = 0x0,
        .LLPRatio = 0x0,

        .APLCompRatioLow  = 0xFF,
        .APLCompRatioHigh = 0x02,

        .FltConfSlope = 0x880,
        .FltConfTh    = 0xF0,

        .BlkHistCountRatio = 0x3,
        .BinIdxDiffSlope   = 0x8,
        .BinIdxDiffTh      = 0x3,
        .BinIdxDiffWgtOft  = 0xF0,
        .APLTh             = 0x0,
        .APLSlope          = 0x7F,
        .APLWgtOft         = 0x100,

        .APL2Th     = 0xFF,
        .APL2Slope  = 0x80,
        .APL2WgtOft = 0x100,
        .APL2WgtMax = 0x100,

        .BlkSpaFltEn   = 1,
        .BlkSpaFltType = 0x1,

        .LoadBlkCurveEn = 0,
        .SaveBlkCurveEn = 0,

        .Flat_Length_Th_Base = 0,
        .Flat_Length_Slope_Base = 0xC8,

        .DZ_Fallback_En    = 1,
        .DZ_Size_Th        = 0x32,
        .DZ_Size_Slope     = 0x40,

        .bGreenWgtSlope    = 0x80,
        .dre_dync_compact_wgt_min = 0x0,
        .dre_dync_compact_wgt_max = 0xFF,
        .dre_dync_compact_th     = 0x80,
        .dre_dync_compact_slope   = 0x40,
        .bCompactBlendmethod      = 0x2,
        .dre_dync_compact_LowBoundBin = 0x5,
        .dre_dync_compact_HighBoundBin = 0x7,
        .dre_dync_compact_BinMinWeight = 0x100,
        .dre_dync_compact_BinMaxWeight = 0x280,
    },

    // SC-LTM
    // ###### 14 ######
    {
        .dre_fw_en           = 1,
        .dre_curve_en        = 1,
        .dre_gain_flt_en     = 1,

        .bADLWeight1 = 0x14,
        .bADLWeight2 = 0x2C,
        .bADLWeight3 = 0x00,

        .bBSDCGain = 0x6,
        .bBSACGain = 0x80,
        .bBSLevel  = 2,

        .bMIDDCGain = 0x6,
        .bMIDACGain = 0x20,

        .bWSDCGain = 0x4,
        .bWSACGain = 0xFF,
        .bWSLevel  = 4,

        .dre_dync_spike_wgt_min = 0x0,
        .dre_dync_spike_wgt_max = 0xF5,
        .dre_dync_spike_th      = 0xA0,
        .dre_dync_spike_slope   = 0x27,
        .bSpikeBlendmethod      = 0x2,
    
        .bSkinWgtSlope    = 0x80,
        .bSkinBlendmethod = 0x2,
        .SkinDummy1       = 0,
        .Layer0_Ratio     = 0x80,
        .Layer1_Ratio     = 0x10,
        .Layer2_Ratio     = 0x00,
        .Dark_Ratio       = 0x100,
        .Bright_Ratio     = 0x100,
           
        .dre_dync_flt_coef_min      = 0x3,
        .dre_dync_flt_coef_max      = 0xA0,
        .dre_dync_flt_ovr_pxl_th    = 0x2A,
        .dre_dync_flt_ovr_pxl_slope = 0x15,
        .dre_iir_force_range        = 0x1,
  
        .LLPValue = 0x0,
        .LLPRatio = 0x0,

        .APLCompRatioLow  = 0xFF,
        .APLCompRatioHigh = 0x02,

        .FltConfSlope = 0x880,
        .FltConfTh    = 0xF0,

        .BlkHistCountRatio = 0x3,
        .BinIdxDiffSlope   = 0x8,
        .BinIdxDiffTh      = 0x3,
        .BinIdxDiffWgtOft  = 0xF0,
        .APLTh             = 0x0,
        .APLSlope          = 0x7F,
        .APLWgtOft         = 0x100,

        .APL2Th     = 0xFF,
        .APL2Slope  = 0x80,
        .APL2WgtOft = 0x100,
        .APL2WgtMax = 0x100,

        .BlkSpaFltEn   = 1,
        .BlkSpaFltType = 0x1,

        .LoadBlkCurveEn = 0,
        .SaveBlkCurveEn = 0,

        .Flat_Length_Th_Base = 0,
        .Flat_Length_Slope_Base = 0xC8,

        .DZ_Fallback_En    = 1,
        .DZ_Size_Th        = 0x32,
        .DZ_Size_Slope     = 0x40,

        .bGreenWgtSlope    = 0x80,
        .dre_dync_compact_wgt_min = 0x0,
        .dre_dync_compact_wgt_max = 0xFF,
        .dre_dync_compact_th     = 0x80,
        .dre_dync_compact_slope   = 0x40,
        .bCompactBlendmethod      = 0x2,
        .dre_dync_compact_LowBoundBin = 0x5,
        .dre_dync_compact_HighBoundBin = 0x7,
        .dre_dync_compact_BinMinWeight = 0x100,
        .dre_dync_compact_BinMaxWeight = 0x280,
    },

    // SC-LTM
    // ###### 15 ######
    {
        .dre_fw_en           = 1,
        .dre_curve_en        = 1,
        .dre_gain_flt_en     = 1,

        .bADLWeight1 = 0x14,
        .bADLWeight2 = 0x2C,
        .bADLWeight3 = 0x00,

        .bBSDCGain = 0x6,
        .bBSACGain = 0x80,
        .bBSLevel  = 2,

        .bMIDDCGain = 0x6,
        .bMIDACGain = 0x20,

        .bWSDCGain = 0x4,
        .bWSACGain = 0xFF,
        .bWSLevel  = 4,

        .dre_dync_spike_wgt_min = 0x0,
        .dre_dync_spike_wgt_max = 0xF5,
        .dre_dync_spike_th      = 0xA0,
        .dre_dync_spike_slope   = 0x27,
        .bSpikeBlendmethod      = 0x2,
    
        .bSkinWgtSlope    = 0x80,
        .bSkinBlendmethod = 0x2,
        .SkinDummy1       = 0,
        .Layer0_Ratio     = 0x80,
        .Layer1_Ratio     = 0x10,
        .Layer2_Ratio     = 0x00,
        .Dark_Ratio       = 0x100,
        .Bright_Ratio     = 0x100,
           
        .dre_dync_flt_coef_min      = 0x3,
        .dre_dync_flt_coef_max      = 0xA0,
        .dre_dync_flt_ovr_pxl_th    = 0x2A,
        .dre_dync_flt_ovr_pxl_slope = 0x15,
        .dre_iir_force_range        = 0x1,
  
        .LLPValue = 0x0,
        .LLPRatio = 0x0,

        .APLCompRatioLow  = 0xFF,
        .APLCompRatioHigh = 0x02,

        .FltConfSlope = 0x880,
        .FltConfTh    = 0xF0,

        .BlkHistCountRatio = 0x3,
        .BinIdxDiffSlope   = 0x8,
        .BinIdxDiffTh      = 0x3,
        .BinIdxDiffWgtOft  = 0xF0,
        .APLTh             = 0x0,
        .APLSlope          = 0x7F,
        .APLWgtOft         = 0x100,

        .APL2Th     = 0xFF,
        .APL2Slope  = 0x80,
        .APL2WgtOft = 0x100,
        .APL2WgtMax = 0x100,

        .BlkSpaFltEn   = 1,
        .BlkSpaFltType = 0x1,

        .LoadBlkCurveEn = 0,
        .SaveBlkCurveEn = 0,

        .Flat_Length_Th_Base = 0,
        .Flat_Length_Slope_Base = 0xC8,

        .DZ_Fallback_En    = 1,
        .DZ_Size_Th        = 0x32,
        .DZ_Size_Slope     = 0x40,

        .bGreenWgtSlope    = 0x80,
        .dre_dync_compact_wgt_min = 0x0,
        .dre_dync_compact_wgt_max = 0xFF,
        .dre_dync_compact_th     = 0x80,
        .dre_dync_compact_slope   = 0x40,
        .bCompactBlendmethod      = 0x2,
        .dre_dync_compact_LowBoundBin = 0x5,
        .dre_dync_compact_HighBoundBin = 0x7,
        .dre_dync_compact_BinMinWeight = 0x100,
        .dre_dync_compact_BinMaxWeight = 0x280,
    },

    // SC-LTM
    // ###### 16 ######
    {
        .dre_fw_en           = 1,
        .dre_curve_en        = 1,
        .dre_gain_flt_en     = 1,

        .bADLWeight1 = 0x14,
        .bADLWeight2 = 0x2C,
        .bADLWeight3 = 0x00,

        .bBSDCGain = 0x6,
        .bBSACGain = 0x80,
        .bBSLevel  = 2,

        .bMIDDCGain = 0x6,
        .bMIDACGain = 0x20,

        .bWSDCGain = 0x4,
        .bWSACGain = 0xFF,
        .bWSLevel  = 4,

        .dre_dync_spike_wgt_min = 0x0,
        .dre_dync_spike_wgt_max = 0xF5,
        .dre_dync_spike_th      = 0xA0,
        .dre_dync_spike_slope   = 0x27,
        .bSpikeBlendmethod      = 0x2,
    
        .bSkinWgtSlope    = 0x80,
        .bSkinBlendmethod = 0x2,
        .SkinDummy1       = 0,
        .Layer0_Ratio     = 0x80,
        .Layer1_Ratio     = 0x10,
        .Layer2_Ratio     = 0x00,
        .Dark_Ratio       = 0x100,
        .Bright_Ratio     = 0x100,
           
        .dre_dync_flt_coef_min      = 0x3,
        .dre_dync_flt_coef_max      = 0xA0,
        .dre_dync_flt_ovr_pxl_th    = 0x2A,
        .dre_dync_flt_ovr_pxl_slope = 0x15,
        .dre_iir_force_range        = 0x1,
  
        .LLPValue = 0x0,
        .LLPRatio = 0x0,

        .APLCompRatioLow  = 0xFF,
        .APLCompRatioHigh = 0x02,

        .FltConfSlope = 0x880,
        .FltConfTh    = 0xF0,

        .BlkHistCountRatio = 0x3,
        .BinIdxDiffSlope   = 0x8,
        .BinIdxDiffTh      = 0x3,
        .BinIdxDiffWgtOft  = 0xF0,
        .APLTh             = 0x0,
        .APLSlope          = 0x7F,
        .APLWgtOft         = 0x100,

        .APL2Th     = 0xFF,
        .APL2Slope  = 0x80,
        .APL2WgtOft = 0x100,
        .APL2WgtMax = 0x100,

        .BlkSpaFltEn   = 1,
        .BlkSpaFltType = 0x1,

        .LoadBlkCurveEn = 0,
        .SaveBlkCurveEn = 0,

        .Flat_Length_Th_Base = 0,
        .Flat_Length_Slope_Base = 0xC8,

        .DZ_Fallback_En    = 1,
        .DZ_Size_Th        = 0x32,
        .DZ_Size_Slope     = 0x40,

        .bGreenWgtSlope    = 0x80,
        .dre_dync_compact_wgt_min = 0x0,
        .dre_dync_compact_wgt_max = 0xFF,
        .dre_dync_compact_th     = 0x80,
        .dre_dync_compact_slope   = 0x40,
        .bCompactBlendmethod      = 0x2,
        .dre_dync_compact_LowBoundBin = 0x5,
        .dre_dync_compact_HighBoundBin = 0x7,
        .dre_dync_compact_BinMinWeight = 0x100,
        .dre_dync_compact_BinMaxWeight = 0x280,
    },

    // SC-LTM
    // ###### 17 ######
    {
        .dre_fw_en           = 1,
        .dre_curve_en        = 1,
        .dre_gain_flt_en     = 1,

        .bADLWeight1 = 0x14,
        .bADLWeight2 = 0x2C,
        .bADLWeight3 = 0x00,

        .bBSDCGain = 0x6,
        .bBSACGain = 0x80,
        .bBSLevel  = 2,

        .bMIDDCGain = 0x6,
        .bMIDACGain = 0x20,

        .bWSDCGain = 0x4,
        .bWSACGain = 0xFF,
        .bWSLevel  = 4,

        .dre_dync_spike_wgt_min = 0x0,
        .dre_dync_spike_wgt_max = 0xF5,
        .dre_dync_spike_th      = 0xA0,
        .dre_dync_spike_slope   = 0x27,
        .bSpikeBlendmethod      = 0x2,
    
        .bSkinWgtSlope    = 0x80,
        .bSkinBlendmethod = 0x2,
        .SkinDummy1       = 0,
        .Layer0_Ratio     = 0x80,
        .Layer1_Ratio     = 0x10,
        .Layer2_Ratio     = 0x00,
        .Dark_Ratio       = 0x100,
        .Bright_Ratio     = 0x100,
           
        .dre_dync_flt_coef_min      = 0x3,
        .dre_dync_flt_coef_max      = 0xA0,
        .dre_dync_flt_ovr_pxl_th    = 0x2A,
        .dre_dync_flt_ovr_pxl_slope = 0x15,
        .dre_iir_force_range        = 0x1,
  
        .LLPValue = 0x0,
        .LLPRatio = 0x0,

        .APLCompRatioLow  = 0xFF,
        .APLCompRatioHigh = 0x02,

        .FltConfSlope = 0x880,
        .FltConfTh    = 0xF0,

        .BlkHistCountRatio = 0x3,
        .BinIdxDiffSlope   = 0x8,
        .BinIdxDiffTh      = 0x3,
        .BinIdxDiffWgtOft  = 0xF0,
        .APLTh             = 0x0,
        .APLSlope          = 0x7F,
        .APLWgtOft         = 0x100,

        .APL2Th     = 0xFF,
        .APL2Slope  = 0x80,
        .APL2WgtOft = 0x100,
        .APL2WgtMax = 0x100,

        .BlkSpaFltEn   = 1,
        .BlkSpaFltType = 0x1,

        .LoadBlkCurveEn = 0,
        .SaveBlkCurveEn = 0,

        .Flat_Length_Th_Base = 0,
        .Flat_Length_Slope_Base = 0xC8,

        .DZ_Fallback_En    = 1,
        .DZ_Size_Th        = 0x32,
        .DZ_Size_Slope     = 0x40,

        .bGreenWgtSlope    = 0x80,
        .dre_dync_compact_wgt_min = 0x0,
        .dre_dync_compact_wgt_max = 0xFF,
        .dre_dync_compact_th     = 0x80,
        .dre_dync_compact_slope   = 0x40,
        .bCompactBlendmethod      = 0x2,
        .dre_dync_compact_LowBoundBin = 0x5,
        .dre_dync_compact_HighBoundBin = 0x7,
        .dre_dync_compact_BinMinWeight = 0x100,
        .dre_dync_compact_BinMaxWeight = 0x280,
    },

    // SC-LTM
    // ###### 18 ######
    {
        .dre_fw_en           = 1,
        .dre_curve_en        = 1,
        .dre_gain_flt_en     = 1,

        .bADLWeight1 = 0x14,
        .bADLWeight2 = 0x2C,
        .bADLWeight3 = 0x00,

        .bBSDCGain = 0x6,
        .bBSACGain = 0x80,
        .bBSLevel  = 2,

        .bMIDDCGain = 0x6,
        .bMIDACGain = 0x20,

        .bWSDCGain = 0x4,
        .bWSACGain = 0xFF,
        .bWSLevel  = 4,

        .dre_dync_spike_wgt_min = 0x0,
        .dre_dync_spike_wgt_max = 0xF5,
        .dre_dync_spike_th      = 0xA0,
        .dre_dync_spike_slope   = 0x27,
        .bSpikeBlendmethod      = 0x2,
    
        .bSkinWgtSlope    = 0x80,
        .bSkinBlendmethod = 0x2,
        .SkinDummy1       = 0,
        .Layer0_Ratio     = 0x80,
        .Layer1_Ratio     = 0x10,
        .Layer2_Ratio     = 0x00,
        .Dark_Ratio       = 0x100,
        .Bright_Ratio     = 0x100,
           
        .dre_dync_flt_coef_min      = 0x3,
        .dre_dync_flt_coef_max      = 0xA0,
        .dre_dync_flt_ovr_pxl_th    = 0x2A,
        .dre_dync_flt_ovr_pxl_slope = 0x15,
        .dre_iir_force_range        = 0x1,
  
        .LLPValue = 0x0,
        .LLPRatio = 0x0,

        .APLCompRatioLow  = 0xFF,
        .APLCompRatioHigh = 0x02,

        .FltConfSlope = 0x880,
        .FltConfTh    = 0xF0,

        .BlkHistCountRatio = 0x3,
        .BinIdxDiffSlope   = 0x8,
        .BinIdxDiffTh      = 0x3,
        .BinIdxDiffWgtOft  = 0xF0,
        .APLTh             = 0x0,
        .APLSlope          = 0x7F,
        .APLWgtOft         = 0x100,

        .APL2Th     = 0xFF,
        .APL2Slope  = 0x80,
        .APL2WgtOft = 0x100,
        .APL2WgtMax = 0x100,

        .BlkSpaFltEn   = 1,
        .BlkSpaFltType = 0x1,

        .LoadBlkCurveEn = 0,
        .SaveBlkCurveEn = 0,

        .Flat_Length_Th_Base = 0,
        .Flat_Length_Slope_Base = 0xC8,

        .DZ_Fallback_En    = 1,
        .DZ_Size_Th        = 0x32,
        .DZ_Size_Slope     = 0x40,

        .bGreenWgtSlope    = 0x80,
        .dre_dync_compact_wgt_min = 0x0,
        .dre_dync_compact_wgt_max = 0xFF,
        .dre_dync_compact_th     = 0x80,
        .dre_dync_compact_slope   = 0x40,
        .bCompactBlendmethod      = 0x2,
        .dre_dync_compact_LowBoundBin = 0x5,
        .dre_dync_compact_HighBoundBin = 0x7,
        .dre_dync_compact_BinMinWeight = 0x100,
        .dre_dync_compact_BinMaxWeight = 0x280,
    },

    // SC-LTM
    // ###### 19 ######
    {
        .dre_fw_en           = 1,
        .dre_curve_en        = 1,
        .dre_gain_flt_en     = 1,

        .bADLWeight1 = 0x14,
        .bADLWeight2 = 0x2C,
        .bADLWeight3 = 0x00,

        .bBSDCGain = 0x6,
        .bBSACGain = 0x80,
        .bBSLevel  = 2,

        .bMIDDCGain = 0x6,
        .bMIDACGain = 0x20,

        .bWSDCGain = 0x4,
        .bWSACGain = 0xFF,
        .bWSLevel  = 4,

        .dre_dync_spike_wgt_min = 0x0,
        .dre_dync_spike_wgt_max = 0xF5,
        .dre_dync_spike_th      = 0xA0,
        .dre_dync_spike_slope   = 0x27,
        .bSpikeBlendmethod      = 0x2,
    
        .bSkinWgtSlope    = 0x80,
        .bSkinBlendmethod = 0x2,
        .SkinDummy1       = 0,
        .Layer0_Ratio     = 0x80,
        .Layer1_Ratio     = 0x10,
        .Layer2_Ratio     = 0x00,
        .Dark_Ratio       = 0x100,
        .Bright_Ratio     = 0x100,
           
        .dre_dync_flt_coef_min      = 0x3,
        .dre_dync_flt_coef_max      = 0xA0,
        .dre_dync_flt_ovr_pxl_th    = 0x2A,
        .dre_dync_flt_ovr_pxl_slope = 0x15,
        .dre_iir_force_range        = 0x1,
  
        .LLPValue = 0x0,
        .LLPRatio = 0x0,

        .APLCompRatioLow  = 0xFF,
        .APLCompRatioHigh = 0x02,

        .FltConfSlope = 0x880,
        .FltConfTh    = 0xF0,

        .BlkHistCountRatio = 0x3,
        .BinIdxDiffSlope   = 0x8,
        .BinIdxDiffTh      = 0x3,
        .BinIdxDiffWgtOft  = 0xF0,
        .APLTh             = 0x0,
        .APLSlope          = 0x7F,
        .APLWgtOft         = 0x100,

        .APL2Th     = 0xFF,
        .APL2Slope  = 0x80,
        .APL2WgtOft = 0x100,
        .APL2WgtMax = 0x100,

        .BlkSpaFltEn   = 1,
        .BlkSpaFltType = 0x1,

        .LoadBlkCurveEn = 0,
        .SaveBlkCurveEn = 0,

        .Flat_Length_Th_Base = 0,
        .Flat_Length_Slope_Base = 0xC8,

        .DZ_Fallback_En    = 1,
        .DZ_Size_Th        = 0x32,
        .DZ_Size_Slope     = 0x40,

        .bGreenWgtSlope    = 0x80,
        .dre_dync_compact_wgt_min = 0x0,
        .dre_dync_compact_wgt_max = 0xFF,
        .dre_dync_compact_th     = 0x80,
        .dre_dync_compact_slope   = 0x40,
        .bCompactBlendmethod      = 0x2,
        .dre_dync_compact_LowBoundBin = 0x5,
        .dre_dync_compact_HighBoundBin = 0x7,
        .dre_dync_compact_BinMinWeight = 0x100,
        .dre_dync_compact_BinMaxWeight = 0x280,
    }
};

}

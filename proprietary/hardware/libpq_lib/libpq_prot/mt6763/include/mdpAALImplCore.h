#ifndef __MDPAAL_IMPL_CORE_H__
#define __MDPAAL_IMPL_CORE_H__

/*
* header files
*/

#include "mdpAALCommon.h"

//#define DRE_ANDROID_PLATFORM

#define mdpDRE_LUMA_BIT            8

#define mdpDRE_LUMA_POINT_BIT      4 // For ( 17 = 2^4 + 1 ) points

#define mdpDRE_LUMA_INTERVAL_BIT       ( (mdpDRE_LUMA_BIT) - (mdpDRE_LUMA_POINT_BIT) ) // For 17 points
#define mdpDRE_LUMA_INTERVAL_BIT_ROUND ( ( 1 << ( mdpDRE_LUMA_INTERVAL_BIT ) ) >> 1 )
#define mdpDRE_LUMA_INTERVAL           ( 1 << ( mdpDRE_LUMA_INTERVAL_BIT ) ) // For 17 points
#define mdpDRE_LUMA_MAX                ( 1 << (mdpDRE_LUMA_BIT) )

#define mdpDRE_LUMA_HIST_NUM           ( 1 << ( mdpDRE_LUMA_POINT_BIT ) )
#define mdpDRE_LUMA_HIST_NUM_WEIGHT    ( mdpDRE_LUMA_HIST_NUM + 1 )
#define mdpDRE_LUMA_CURVE_NUM          ( mdpDRE_LUMA_HIST_NUM + 1 )

#define mdpDRE_LUMA_HIST_NORM_BASE_BIT     8
#define mdpDRE_LUMA_HIST_NORM_BASE         ( 1 << mdpDRE_LUMA_HIST_NORM_BASE_BIT )
#define mdpDRE_LUMA_HIST_NUM_TRUNCATE      mdpDRE_LUMA_HIST_NUM

#define mdpDRE_BLACK_WHITE_NUM    ( mdpDRE_LUMA_HIST_NUM / 2 )

#define mdpDRE_FIR_NUM             9
#define mdpDRE_FIR_NUM_WEIGHT      8

#define mdpDRE_SKIN_NORM_BASE                  256
#define mdpDRE_SKIN_WGT_SLOPE_BIT              4
#define mdpDRE_SKIN_WGT_BIT                    8
#define mdpDRE_SKIN_WGT_MAX                    ( 1 << ( mdpDRE_SKIN_WGT_BIT ) )
#define mdpDRE_SKIN_CHANGE_WITH_LUMA_BIT       ( ( mdpDRE_LUMA_POINT_BIT ) - 1 )
#define mdpDRE_SKIN_CHANGE_WITH_LUMA_BIT_ROUND ( 1 << ( mdpDRE_SKIN_CHANGE_WITH_LUMA_BIT - 1 ) )


#define mdpDRE_GREEN_NORM_BASE                  256
#define mdpDRE_GREEN_WGT_SLOPE_BIT              4
#define mdpDRE_GREEN_WGT_BIT                    8
#define mdpDRE_GREEN_WGT_MAX                    ( 1 << ( mdpDRE_GREEN_WGT_BIT ) )
#define mdpDRE_GREEN_CHANGE_WITH_LUMA_BIT       ( ( mdpDRE_LUMA_POINT_BIT ) - 1 )
#define mdpDRE_GREEN_CHANGE_WITH_LUMA_BIT_ROUND ( 1 << ( mdpDRE_GREEN_CHANGE_WITH_LUMA_BIT - 1 ) )


#define mdpDRE_DYNC_SPIKE_SLOPE_BIT         4
#define mdpDRE_DYNC_SPIKE_SLOPE_BIT_ROUND   ( ( 1 << ( mdpDRE_DYNC_SPIKE_SLOPE_BIT ) ) >> 1 )
#define mdpDRE_DYNC_COMPACT_SLOPE_BIT         4
#define mdpDRE_DYNC_COMPACT_SLOPE_BIT_ROUND   ( ( 1 << ( mdpDRE_DYNC_SPIKE_SLOPE_BIT ) ) >> 1 )


#define mdpDRE_BLK_NUM_X                    16
#define mdpDRE_BLK_NUM_Y                    8
#define mdpDRE_BLK_SFLT_SIZE_MAX            7

#define mdpDRE_SRAM_CURVE_POINT_BIT         8
#define mdpDRE_SRAM_CURVE_POINT_MAX         ( ( 1 << ( mdpDRE_SRAM_CURVE_POINT_BIT ) ) - 1 )
#define mdpDRE_LOCAL_MAX_WGT_BIT            8
#define mdpDRE_LOCAL_MAX_WGT_BIT_ROUND      ( ( 1 << ( mdpDRE_LOCAL_MAX_WGT_BIT ) ) >> 1 )
#define mdpDRE_LOCAL_MAX_WGT                ( 1 << ( mdpDRE_LOCAL_MAX_WGT_BIT ) )
#define mdpDRE_BLOCK_INFO_SLOPE_BIT         7 // 128 = 1x
#define mdpDRE_BLOCK_INFO_SLOPE_BIT_ROUND   ( ( 1 << ( mdpDRE_BLOCK_INFO_SLOPE_BIT ) ) >> 1 )
#define mdpDRE_BLOCK_INFO_SLOPE2_BIT        3 // 128 = 1x
#define mdpDRE_BLOCK_INFO_SLOPE2_BIT_ROUND  ( ( 1 << ( mdpDRE_BLOCK_INFO_SLOPE2_BIT ) ) >> 1 )
#define mdpDRE_BLK_INFO_NORMALIZE_BIT       8
#define mdpDRE_BLK_INFO_NORMALIZE_BIT_ROUND ( ( 1 << ( mdpDRE_BLK_INFO_NORMALIZE_BIT ) ) >> 1 )
#define mdpDRE_APL_COMP_RATIO_BIT           8
#define mdpDRE_APL_COMP_RATIO_BIT_ROUND     ( ( 1 << ( mdpDRE_APL_COMP_RATIO_BIT ) ) >> 1 )
#define mdpDRE_APL_COMP_UNIT_BIT            8
#define mdpDRE_APL_COMP_UNIT_VALUE          ( 1 << ( mdpDRE_APL_COMP_UNIT_BIT ) )
#define mdpDRE_APL_COMP_UNIT_BIT_ROUND      ( ( 1 << ( mdpDRE_APL_COMP_UNIT_BIT ) ) >> 1 )
#define mdpDRE_LLP_RATIO_BIT                8
#define mdpDRE_LLP_RATIO_BIT_ROUND          ( ( 1 << ( mdpDRE_LLP_RATIO_BIT ) ) >> 1 )
#define mdpDRE_LLP_BLEND_MAX_WGT            ( ( mdpDRE_LUMA_INTERVAL ) << 1 )
#define mdpDRE_LLP_BLEND_MAX_WGT_ROUND      ( ( mdpDRE_LLP_BLEND_MAX_WGT ) >> 1 )



// Fields collected from ADL HW
struct DRECoreInput {
    uint32_t LumaHist[mdpDRE_LUMA_HIST_NUM_WEIGHT];
    uint32_t LumaSum;
    uint32_t ColorHist;
    uint32_t ColorHist2;
    uint32_t BlkNum;
    uint32_t HistNorEn;
};

// Fields which will be set to HW registers
struct DRECoreOutput {
    uint16_t LumaCurve[mdpDRE_LUMA_CURVE_NUM];
    uint16_t SkinWgt;
};

///////////////////////////////////////////////////////////////////////////////
// ADL FW Processing class
///////////////////////////////////////////////////////////////////////////////
class CDRECoreFW
{
    /* ........Adaptive Luma Process, functions......... */
public:
  CDRECoreFW();
  ~CDRECoreFW();
  void vDrvADLProc_int( const DRECoreInput &input, DRECoreOutput * output);
  void ADLInitialize(void);
  void setDebugFlags(unsigned int debug) { DebugFlags = debug;}
  uint16_t * getNorHist() { return waHist; }

private:
  void BasicInfoCal( uint32_t LumaSum, uint32_t ColorHist, uint32_t * u4APL, uint32_t * u4SkinRatio, uint32_t BlkNum, uint32_t ColorHist2, uint32_t *u4GreenRatio);
  void u1DrvGetLumaHistWeight(const uint32_t wHist[mdpDRE_LUMA_HIST_NUM_WEIGHT], uint16_t waHistCur[mdpDRE_LUMA_HIST_NUM_WEIGHT], uint16_t u2NormBase, uint16_t BlkNum, uint16_t HistNorEn );
  void bHistogramNormalization(const uint32_t wHist[mdpDRE_LUMA_HIST_NUM_WEIGHT], uint16_t waHistCur[mdpDRE_LUMA_HIST_NUM_WEIGHT], uint8_t bDim, uint16_t wBase, uint16_t BlkNum, uint16_t HistNorEn );
  void vDrvGetBlackWhiteIndex(const uint16_t waHistCur[mdpDRE_LUMA_HIST_NUM_WEIGHT]);
  void LocalHistEqualized(const uint16_t waHist[mdpDRE_LUMA_HIST_NUM_WEIGHT], uint16_t waLumaCurve[mdpDRE_LUMA_CURVE_NUM]);
  void vDrvDynamicADLPara(void);
  void vDrvDynamicBSOffset(void);
  void vDrvDynamicWSOffset(void);
  void vDrvADLBuildDefault(uint16_t wCurve[mdpDRE_LUMA_CURVE_NUM]);
  void LowPassFilter(uint32_t u4FIR[mdpDRE_FIR_NUM]);
  void HistFIR(const uint16_t waHistCur[mdpDRE_LUMA_HIST_NUM_WEIGHT], uint16_t waLowPass[mdpDRE_LUMA_HIST_NUM], uint32_t u4FIR[mdpDRE_FIR_NUM] );
  void vBuildSecondLumaCurve(const uint16_t waCurCurve[mdpDRE_LUMA_CURVE_NUM], uint16_t waLumaCurve[mdpDRE_LUMA_CURVE_NUM], uint32_t u4SkinRatio, uint16_t &OutputSkinWgt);
  uint32_t vAdapSecondLumaWeight( uint32_t u4SkinRatio );
  void vSpikeLumaCurve(const uint16_t waCurCurve[mdpDRE_LUMA_CURVE_NUM], uint16_t waLumaCurve[mdpDRE_LUMA_CURVE_NUM], const uint16_t waHist[mdpDRE_LUMA_HIST_NUM_WEIGHT]);
  uint32_t vSpikeLumaWeight(const uint16_t waHist[mdpDRE_LUMA_HIST_NUM_WEIGHT]);
  void vCompactLumaCurve(const uint16_t waCurCurve[mdpDRE_LUMA_CURVE_NUM], uint16_t waLumaCurve[mdpDRE_LUMA_CURVE_NUM], const uint16_t waHist[mdpDRE_LUMA_HIST_NUM_WEIGHT], uint32_t u4GreenRatio);
  uint32_t vCompactLumaWeight(const uint16_t waHist[mdpDRE_LUMA_HIST_NUM_WEIGHT]);
  uint32_t vAdaptGreenWeight( uint32_t u4GreenRatio );
  void RGBMaxCurveMap( uint32_t RGBMaxIn, uint32_t &RGBMaxOut, const uint16_t DRECurvePoint[mdpDRE_LUMA_CURVE_NUM] );
  void printSWRegVal();
  void vLowLvlProtCurve( uint16_t waLumaCurve[mdpDRE_LUMA_CURVE_NUM] );
  void vAPLCompCurve(const uint16_t waLumaCurveSrc[mdpDRE_LUMA_CURVE_NUM], uint16_t waLumaCurveDst[mdpDRE_LUMA_CURVE_NUM] );

    /* ........Adaptive Luma Process, variables......... */
public:
    DREReg * pDREReg;
    DREInternalReg * pDREInternalReg;

//#ifndef DRE_ANDROID_PLATFORM
//    FILE * ifp1;
//#endif

private:

    uint32_t bALOnOff;
    uint32_t bBSOnOff;
    uint32_t bWSOnOff;
    uint32_t bStrongBSOffset;
    uint32_t bStrongWSOffset;
    uint32_t bBSAdaptiveLmt;
    uint32_t bWSAdaptiveLmt;
    uint32_t bLevelLowBnd;
    uint32_t bLevelHighBnd;
    uint32_t bBSRatio;
    uint32_t bBSOffsetThL;
    uint32_t bBSOffsetThH;
    uint32_t bBSOffsetL;
    uint32_t bBSOffsetH;
    uint32_t bWSRatio;
    uint32_t bWSOffsetThL;
    uint32_t bWSOffsetThH;
    uint32_t bWSOffsetL;
    uint32_t bWSOffsetH;
    uint32_t DebugFlags;
    uint16_t waHist[mdpDRE_LUMA_HIST_NUM_WEIGHT];
    uint16_t waCurCurve[mdpDRE_LUMA_CURVE_NUM];
    uint16_t waLumaArray[mdpDRE_LUMA_CURVE_NUM];
    uint16_t waLumaArray1[mdpDRE_LUMA_CURVE_NUM];
    uint16_t waLumaArray2[mdpDRE_LUMA_CURVE_NUM];
    uint16_t waLumaArray3[mdpDRE_LUMA_CURVE_NUM];
    uint32_t u4APL;
    uint32_t u4SkinRatio;
    uint32_t u4GreenRatio;
    uint8_t bParamChange;
    uint8_t bBlackIndex;
    uint8_t bWhiteIndex;
    uint8_t bBSOffset;
    uint8_t bWSOffset;
    char inname[500];
    char integer_string[32];
};

#endif //__MDPAAL_IMPL_CORE_H__

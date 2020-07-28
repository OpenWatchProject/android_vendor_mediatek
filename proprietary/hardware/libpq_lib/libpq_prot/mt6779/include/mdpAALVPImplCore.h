#ifndef __MDPAALVP_IMPL_CORE_H__
#define __MDPAALVP_IMPL_CORE_H__

/*
* header files
*/

#include "mdpAALCommon.h"
#include "mdpAALImplCore.h"

///////////////////////////////////////////////////////////////////////////////
// ADL FW Processing class
///////////////////////////////////////////////////////////////////////////////
class CDREVPCoreFW
{
    /* ........Adaptive Luma Process, functions......... */
public:
  CDREVPCoreFW();
  ~CDREVPCoreFW();
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
    char inname[1000];
    char integer_string[32];
};

#endif //__MDPAALVP_IMPL_CORE_H__

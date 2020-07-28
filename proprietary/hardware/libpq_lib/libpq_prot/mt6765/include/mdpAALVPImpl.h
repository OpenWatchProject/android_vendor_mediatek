#ifndef MDPAALVPIMPL_H
#define MDPAALVPIMPL_H

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
#include "mdpAALVPImplCore.h"
#include "mdpAALImpl.h"

#define DRE_MODE_NUM 2
#define SET_NUM 10

// Game PQ read-only FW registers
struct DREReg_RO {
    int g_hist_set01;
    int g_hist_set02;
    int g_hist_set03;
    int g_hist_set04;
    int g_hist_set05;
    int g_hist_set06;
    int g_hist_set07;
    int g_hist_set08;
    int g_hist_set09;
    int g_hist_set10;
    int g_hist_set11;
    int g_hist_set12;
    int g_hist_set13;
    int g_hist_set14;
    int g_hist_set15;
    int g_hist_set16;
    int g_hist_set17;
    int g_curve_set01;
    int g_curve_set02;
    int g_curve_set03;
    int g_curve_set04;
    int g_curve_set05;
    int g_curve_set06;
    int g_curve_set07;
    int g_curve_set08;
    int g_curve_set09;
    int g_curve_set10;
    int g_curve_set11;
    int g_curve_set12;
    int g_curve_set13;
    int g_curve_set14;
    int g_curve_set15;
    int g_curve_set16;
    int g_curve_set17;
};

// Game PQ share data for session switching
struct DREsharedata {
    unsigned char PreFloatCurve_game[mdpDRE_BLK_NUM_Y][mdpDRE_BLK_NUM_X][mdpDRE_LUMA_CURVE_NUM];
    unsigned char CurTgtCurve_game[mdpDRE_BLK_NUM_Y][mdpDRE_BLK_NUM_X][ mdpDRE_LUMA_CURVE_NUM ];
    unsigned char FinalCoef_game[mdpDRE_BLK_NUM_Y][mdpDRE_BLK_NUM_X];
    unsigned short DRESkinWgtSet_game[mdpDRE_BLK_NUM_Y][mdpDRE_BLK_NUM_X];
};


///////////////////////////////////////////////////////////////////////////////
// MDP AAL FW Processing class
///////////////////////////////////////////////////////////////////////////////
class CDREVPTopFW
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
  int pre_dre_dync_flt_coef_min = 0x3;
  int pre_dre_dync_flt_ovr_pxl_slope = 0x15;
  char inname[1000];
  char integer_string[100];
  bool isFirstFrame;
  bool isSecondFrame;
  bool isFilterFlat;
  int session_switch;
  int session_switch_skin;
  int isUICombined;
  int FrameCount;
  void EventTriggerSetting( const DRETopInput &input, DRETopOutput *output );
  void DREGainIdxCal( const DRETopInput &input);
  void DRECurveCalculation( const DRETopInput &input);
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

  CDREVPCoreFW * pDRECoreFW;
  DRECoreInput * pDRECoreIn;
  DRECoreOutput * pDRECoreOut;
  bool m_isHDR;

public:
  DREReg * pDREReg;
  DREInitReg * pDREInitReg;
  DREInternalReg * pDREInternalReg;
  DREToolReg * pDREToolReg;
  DREReg iDRERegEntry[DRE_MODE_NUM*SET_NUM];
  DREReg_RO * pDREReg_RO;
  DREsharedata * pDREsharedata;
void setDebugFlags(unsigned int debug) {
    DebugFlags = debug;
    pDRECoreFW->setDebugFlags( DebugFlags );
  }

  int DREVersionNum;
  void DREBlkNumCal(const DREInitParam &initParam, int wid, int hei);
  void calBlkSpaFltParam();
  void DREBlkHWRegCal( int Blk_Num, int Width, int &blk_width, int &dre_x_alpha_base, int &dre_x_alpha_shift_bit );
  void DREFrameHWRegCal( const DREInitParam &initParam );
  void DRETileHWRegCal( const DREInitParam &initParam );
  void LoadFWRegSettings();
  void onLoadTable(bool isHDR, int sel = 0);
  void initDRERegEntry();

  void setShareMemory( DREsharedata *sharedata);
  void getShareMemory( DREsharedata *sharedata);
  void initShareMemory( DREsharedata *sharedata);

#ifdef DRE_ANDROID_PLATFORM
  void onCalculate(const DRETopInput &input, DRETopOutput *output);
  void onInitCommon(const DREInitParam &initParam, const int wholefbwidth, const int wholefbheight);
  void onInitPlatform();
  void onInitPart(const DREInitParam &initParam, DRETopOutput *output);
  // Initialization will call onInitCommon() -> onInitPlatform()
  // onInitCommon() performs common initialization.
  // onInitPlatform() will load customization parameters
  // onInitPlay() performs part of initialization
#else
  void Main( const DRETopInput &input, DRETopOutput *output );
  void Initialize( const DREInitParam &initParam, const int wholefbwidth, const int wholefbheight );
  void ReInitialize( const DREInitParam &initParam, DRETopOutput *output );
  FILE * ifp0;
  FILE * ifp2;
#endif


  CDREVPTopFW()
  {
    memset(this, 0, sizeof(*this));
    pDREReg = new DREReg;
    pDREInitReg = new DREInitReg;
    pDREInternalReg = new DREInternalReg;
    pDREToolReg = new DREToolReg;
    pDREReg_RO = new DREReg_RO;
    pDRECoreFW  = new CDREVPCoreFW;
    pDRECoreIn  = new DRECoreInput;
    pDRECoreOut = new DRECoreOutput;
    pDREsharedata = new DREsharedata;
    pDRECoreFW->pDREReg = pDREReg;
    pDRECoreFW->pDREInternalReg = pDREInternalReg;

    DebugFlags = 0;
    isFirstFrame = false;
    isSecondFrame = false;
    isFilterFlat = false;
    session_switch = 32;
    session_switch_skin = 32;
    isUICombined = 1;
    FrameCount = 0;
    m_isHDR = 0;

    initDRERegEntry();
  };

  ~CDREVPTopFW()
  {
    delete pDREReg;
    delete pDREInitReg;
    delete pDREInternalReg;
    delete pDREToolReg;
    delete pDREReg_RO;
    delete pDRECoreFW;
    delete pDRECoreIn;
    delete pDRECoreOut;
    delete pDREsharedata;

#ifndef DRE_ANDROID_PLATFORM
  if( DebugFlags & eMdpAALDebugDRE )
  {
    fclose( ifp0 );
  }
#endif
  };
};

#endif

#ifndef __AE_STREAM_CORE__
#define __AE_STREAM_CORE__
#include "ccu_ext_interface/ccu_types.h"

#if !defined(AE_MGR_INCLUDING_CCU)
#include "algorithm/ae_feature.h"
#endif

#if !defined(AE_MGR_INCLUDING_CCU)
//custom\mt6797\hal\inc\camera_custom_AEPlinetable.h
typedef enum
{
    LIB3A_AE_SCENE_UNSUPPORTED           = -1,
    LIB3A_AE_SCENE_OFF                            =  0,
    LIB3A_AE_SCENE_AUTO                          =  1,
    LIB3A_AE_SCENE_NIGHT                         =  2,
    LIB3A_AE_SCENE_ACTION                       =  3,
    LIB3A_AE_SCENE_BEACH                         =  4,
    LIB3A_AE_SCENE_CANDLELIGHT             =  5,
    LIB3A_AE_SCENE_FIREWORKS                 = 6,
    LIB3A_AE_SCENE_LANDSCAPE                 = 7,
    LIB3A_AE_SCENE_PORTRAIT                   = 8,
    LIB3A_AE_SCENE_NIGHT_PORTRAIT       = 9,
    LIB3A_AE_SCENE_PARTY                          = 10,
    LIB3A_AE_SCENE_SNOW                           = 11,
    LIB3A_AE_SCENE_SPORTS                        = 12,
    LIB3A_AE_SCENE_STEADYPHOTO             = 13,
    LIB3A_AE_SCENE_SUNSET                        = 14,
    LIB3A_AE_SCENE_THEATRE                      = 15,
    LIB3A_AE_SCENE_ISO_ANTI_SHAKE        = 16,
    LIB3A_AE_SCENE_BACKLIGHT                 = 17,
    LIB3A_AE_SCENE_ISO100           = 100,
    LIB3A_AE_SCENE_ISO200           = 101,
    LIB3A_AE_SCENE_ISO400           = 102,
    LIB3A_AE_SCENE_ISO800           = 103,
    LIB3A_AE_SCENE_ISO1600         = 104,
    LIB3A_AE_SCENE_ISO3200         = 105,
    LIB3A_AE_SCENE_ISO6400         = 106,
    LIB3A_AE_SCENE_ISO12800         = 107,
} LIB3A_AE_SCENE_T;

typedef struct

{
    MUINT32 u4Eposuretime;
    MUINT32 u4AfeGain;
    MUINT32 u4IspGain;
    MUINT8  uIris;
    MUINT8  uSensorMode;
    MUINT8  uFlag;
}strEvSetting;

typedef struct
{
    strEvSetting sPlineTable[2000];   // Pline structure table
} strEvPline;

//vendor\mediatek\proprietary\custom\mt6757\hal\inc\aaa\ae_param.h
#define FLARE_SCALE_UNIT (512) // 1.0 = 512
#define FLARE_OFFSET_DOMAIN (4095) // 12bit domain
#define AE_BV_TARGET ((MUINT32)47)
typedef struct
{
    //B2T
    MUINT32 OverExpoThr; //245
    MUINT32 OverExpoRatio; //70%
    MUINT32 OverExpoResetAccEVTh; //200

    MUINT32 OverExpoSTD_Th[3];
    MUINT32 OverExpoSTD_Ratio[4];

    MUINT32 OverExpoAccDeltaEV_Th[3];
    MINT32 OverExpoAccDeltaEV_DeltaEV[4];

    MUINT32 OverExpoCountSTD_Th;

    MUINT32 OverExpoLowBound;
    MUINT32 OverExpLightAcc1;
    MUINT32 OverExpLightAcc2;
    MUINT32 OverExpLightAcc3;
    MUINT32 OverExpLightAcc4;
    //D2T
    MUINT32 UnderExpoThr; //10
    MUINT32 UnderExpoRatio; //95%
    MUINT32 UnderExpoResetAccEVTh; //20

    MUINT32 UnderExpoSTD_Th[3];
    MUINT32 UnderExpoSTD_Ratio[4];

    MUINT32 UnderExpoAccDeltaEV_Th[3];
    MINT32 UnderExpoAccDeltaEV_DeltaEV[4];

    MUINT32 UnderExpoCountSTD_Th;

} strAETgTuningPara;
#endif


typedef unsigned int FIELD;
// CAM_OBC_OFFST0 CAM+0500H: OB for B channel
typedef struct {
        FIELD OBC_OFST_B                : 13;
        FIELD rsv_13                    : 19;
} CCU_ISP_CAM_OBC_OFFST0_T;

typedef union {
    //enum { MASK     = 0x00001FFF };
    //typedef ISP_CAM_OBC_OFFST0_T reg_t;
    CCU_ISP_CAM_OBC_OFFST0_T   bits;
    MUINT32 val;
} CCU_ISP_NVRAM_OBC_OFFST0_T;

// CAM_OBC_OFFST1 CAM+0504H: OB for Gr channel
typedef struct {
        FIELD OBC_OFST_GR               : 13;
        FIELD rsv_13                    : 19;
} CCU_ISP_CAM_OBC_OFFST1_T;

typedef union {
    //enum { MASK     = 0x00001FFF };
    //typedef ISP_CAM_OBC_OFFST1_T reg_t;
    CCU_ISP_CAM_OBC_OFFST1_T   bits;
    MUINT32 val;
} CCU_ISP_NVRAM_OBC_OFFST1_T;

// CAM_OBC_OFFST2 CAM+0508H: OB for Gb channel
typedef struct {
        FIELD OBC_OFST_GB               : 13;
        FIELD rsv_13                    : 19;
} CCU_ISP_CAM_OBC_OFFST2_T;

typedef union {
    //enum { MASK     = 0x00001FFF };
    //typedef ISP_CAM_OBC_OFFST2_T reg_t;
    CCU_ISP_CAM_OBC_OFFST2_T   bits;
    MUINT32 val;
} CCU_ISP_NVRAM_OBC_OFFST2_T;

// CAM_OBC_OFFST3 CAM+050CH: OB for R channel
typedef struct {
        FIELD OBC_OFST_R                : 13;
        FIELD rsv_13                    : 19;
} CCU_ISP_CAM_OBC_OFFST3_T;

typedef union {
    //enum { MASK     = 0x00001FFF };
    //typedef ISP_CAM_OBC_OFFST3_T reg_t;
    CCU_ISP_CAM_OBC_OFFST3_T   bits;
    MUINT32 val;
} CCU_ISP_NVRAM_OBC_OFFST3_T;

// CAM_OBC_GAIN0 CAM+0510H: OB gain for B channel
typedef struct {
        FIELD OBC_GAIN_B                : 13;
        FIELD rsv_13                    : 19;
} CCU_ISP_CAM_OBC_GAIN0_T;

typedef union {
    //enum { MASK     = 0x00001FFF };
    //typedef ISP_CAM_OBC_GAIN0_T reg_t;
    CCU_ISP_CAM_OBC_GAIN0_T   bits;
    MUINT32 val;
} CCU_ISP_NVRAM_OBC_GAIN0_T;

// CAM_OBC_GAIN1 CAM+0514H: OB gain for Gr channel
typedef struct {
        FIELD OBC_GAIN_GR               : 13;
        FIELD rsv_13                    : 19;
} CCU_ISP_CAM_OBC_GAIN1_T;

typedef union {
    //enum { MASK     = 0x00001FFF };
    //typedef ISP_CAM_OBC_GAIN1_T reg_t;
    CCU_ISP_CAM_OBC_GAIN1_T   bits;
    MUINT32 val;
} CCU_ISP_NVRAM_OBC_GAIN1_T;

// CAM_OBC_GAIN2 CAM+0518H: OB gain for Gb channel
typedef struct {
        FIELD OBC_GAIN_GB               : 13;
        FIELD rsv_13                    : 19;
} CCU_ISP_CAM_OBC_GAIN2_T;

typedef union {
    //enum { MASK     = 0x00001FFF };
    //typedef ISP_CAM_OBC_GAIN2_T reg_t;
    CCU_ISP_CAM_OBC_GAIN2_T   bits;
    MUINT32 val;
} CCU_ISP_NVRAM_OBC_GAIN2_T;

// CAM_OBC_GAIN3 CAM+051CH: OB gain for R channel
typedef struct {
        FIELD OBC_GAIN_R                : 13;
        FIELD rsv_13                    : 19;
} CCU_ISP_CAM_OBC_GAIN3_T;

typedef union {
    //enum { MASK     = 0x00001FFF };
    //typedef ISP_CAM_OBC_GAIN3_T reg_t;
    CCU_ISP_CAM_OBC_GAIN3_T   bits;
    MUINT32 val;
} CCU_ISP_NVRAM_OBC_GAIN3_T;

//
typedef union {
    //enum { COUNT = 8 };
    MUINT32 set[8];
    struct {
        CCU_ISP_NVRAM_OBC_OFFST0_T  offst0; // Q.1.12
        CCU_ISP_NVRAM_OBC_OFFST1_T  offst1; // Q.1.12
        CCU_ISP_NVRAM_OBC_OFFST2_T  offst2; // Q.1.12
        CCU_ISP_NVRAM_OBC_OFFST3_T  offst3; // Q.1.12
        CCU_ISP_NVRAM_OBC_GAIN0_T   gain0; // Q.0.4.9
        CCU_ISP_NVRAM_OBC_GAIN1_T   gain1; // Q.0.4.9
        CCU_ISP_NVRAM_OBC_GAIN2_T   gain2; // Q.0.4.9
        CCU_ISP_NVRAM_OBC_GAIN3_T   gain3; // Q.0.4.9
    };
} CCU_ISP_NVRAM_OBC_T;

typedef struct {
    MUINT16 u2Length;
    MINT16 *IDX_Partition;
    MINT16 *IDX_Part_Middle;
} CCU_ISP_NVRAM_ISO_INTERVAL_STRUCT, *P_CCU_ISP_NVRAM_ISO_INTERVAL_STRUCT;

typedef struct
{
    strEvSetting  EvSetting;
    MINT32        Bv;
    MINT32        AoeCompBv;
    MINT32        i4EV;
    MUINT32       u4ISO;          //correspoing ISO , only use in capture
    MINT16        i2FaceDiffIndex;
    MINT32        i4AEidxCurrent;  // current AE idx
    MINT32        i4AEidxNext;  // next AE idx

} strCcuAeOutput;

//custom\mt6797\hal\imgsensor\imx219_mipi_raw\camera_AE_PLineTable_imx219mipiraw.h


/////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////CCU AE algo Interface//////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////
typedef enum
{
    AE_CTRL_SET_CPU_EXP_INFO,
    AE_CTRL_SET_VSYNC_INFO,
    AE_CTRL_SET_RUN_TIME_INFO,
    AE_CTRL_CLEAR_CPU_SHOULD_GET_DATA_INFO,
    AE_CTRL_SET_LAST_RESULT,
    AE_CTRL_GET_DATA_POINTER,
    AE_CTRL_MAX
}	AE_CTRL_ENUM;

typedef enum
{
    AE_CCU_ACTION_SET_I2C,
    AE_CCU_ACTION_SKIP_I2C,
    AE_CCU_ACTION_MAX
}	AE_CCU_ACTION_ENUM;

typedef enum
{
    AE_CPU_ACTION_DO_NORMAL_AE,
    AE_CPU_ACTION_DO_FULL_AE,
    AE_CPU_ACTION_SKIP_DO_AE,
    AE_CPU_ACTION_MAX
}	AE_CPU_ACTION_ENUM;

typedef enum
{
    AE_STATUS_1,
    AE_STATUS_2,
    AE_STATUS_3
}	AE_STATUS_ENUM;


typedef struct
{
    MUINT32 EndBankIdx;
    MUINT32 m_i4AEMaxBlockWidth;
    MUINT32 m_i4AEMaxBlockHeight;
    MUINT32 uTgBlockNumX;          //m_AeTuningParam->strAEParasetting.uTgBlockNumX
    MUINT32 uTgBlockNumY;          //m_AeTuningParam->strAEParasetting.uTgBlockNumX
    MUINT32 m_u4IndexMax;
    MUINT32 m_u4IndexMin;
    MBOOL   bSpeedupEscOverExp;   //m_AeTuningParam->bSpeedupEscOverExp
    MBOOL   bSpeedupEscUnderExp;  //m_AeTuningParam->bSpeedupEscUnderExp
    strAETgTuningPara pAETgTuningParam; //m_AeTuningParam->m_AeTuningParam
    MUINT32 pTgWeightTbl[9][12];
    MINT32 *pTgEVCompJumpRatioOverExp;  //m_AeTuningParam->pTgEVCompJumpRatioOverExp
    MINT32 *pTgEVCompJumpRatioUnderExp; //m_AeTuningParam->pTgEVCompJumpRatioUnderExp
    MUINT32 *LumLog2x1000;
    MUINT32 SzLumLog2x1000;
    strEvPline *pCurrentTable;         //m_pPreviewTableCurrent->pCurrentTable
    MINT32  i4MaxBV;       //m_pPreviewTableCurrent->i4MaxBV or instace in g_AE_PreviewAutoTable,custom\mt6757\hal\imgsensor\s5k2p8_mipi_raw\camera_AE_PLineTable_s5k2p8raw.h
    MINT32  i4BVOffset;    //m_AeCCTConfig.i4BVOffset
    MUINT32 u4MiniISOGain; //m_pAENVRAM->rDevicesInfo.u4MiniISOGain
    MINT32  iMiniBVValue;  //m_AeTuningParam->strAEParasetting.iMiniBVValue
    MUINT32 StatisticStable;
    MUINT32 EVBase;

    CCU_ISP_NVRAM_ISO_INTERVAL_STRUCT m_rIspIsoEnv;
    CCU_ISP_NVRAM_OBC_T OBC_Table[4];
    MINT32 OBC_ISO_IDX_Range[5];
}AE_CORE_INIT;

typedef struct
{
    MUINT8     *Buffer;
    MUINT32     YOffset;
    MUINT32     YLineNum;
    MUINT8      CrntBankIdx;
    MUINT8      *LogBuffer;

}AE_CORE_MAIN_IN;

typedef struct
{
    strCcuAeOutput a_Output;
    MUINT32 m_u4EffectiveIndex;
    MUINT32 m_u4Index;
    MINT32  m_i4RealBV;
    MINT32  m_i4RealBVx1000;
    MINT32  m_i4BV;
    MINT32  m_i4AoeCompBV;
    MINT32  m_i4EV;
    AE_CPU_ACTION_ENUM enumCpuAction;
    AE_CCU_ACTION_ENUM enumCcuAction;

    CCU_ISP_NVRAM_OBC_T ObcResult;
}AE_CORE_MAIN_OUT;


typedef struct
{
    AE_CORE_INIT *pInitPtr;

}AE_CORE_CTRL_DATA_POINTER;

typedef struct
{
    //MUINT32 SkipAEAlgo;
    MUINT32 u4CpuReady;
    MUINT32 m_u4EffectiveIndex;
    MINT32  m_i4deltaIndex;
    MUINT32 u4Eposuretime;
    MUINT32 u4AfeGain;
    MUINT32 u4IspGain;
    MUINT32 u4ISO;

}AE_CORE_CTRL_CPU_EXP_INFO;

typedef struct
{
    //ROI
    MUINT32 AEWinBlock_u4XHi;
    MUINT32 AEWinBlock_u4XLow;
    MUINT32 AEWinBlock_u4YHi;
    MUINT32 AEWinBlock_u4YLow;
    //EV Settings
    LIB3A_AE_EVCOMP_T eAEComp;

}AE_CORE_CTRL_CCU_VSYNC_INFO;


typedef struct
{
    MUINT32 m_u4IndexMax;
    MUINT32 m_u4IndexMin;
    strEvPline *pCurrentTable;

}AE_CORE_CTRL_RUN_TIME_INFO;


void AE_Core_Init(void);

void AE_Core_Ctrl(AE_CTRL_ENUM id, void *ctrl_in, void *ctrl_out);

void AE_Core_Main(AE_CORE_MAIN_IN *in, AE_CORE_MAIN_OUT *out);

void AE_Core_Reset(void);

int GetOBGain(MUINT32 real_iso, MUINT32 ISPGain, CCU_ISP_NVRAM_OBC_T *obc);
#endif

#ifndef __AE_STREAM_CORE__
#define __AE_STREAM_CORE__
#include "ccu_ext_interface/ccu_types.h"
#include "algorithm/ccu_ae_param.h"
#include "algorithm/ccu_ae_nvram.h"
#include "algorithm/ccu_AEPlinetable.h"
#include "algorithm/ccu_ae_algo_data.h"
#include "algorithm/ccu_n3d_ae_algo_data.h"


/////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////CCU AE algo Interface//////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////
typedef enum
{
    AE_CTRL_SET_VSYNC_INFO,
    AE_CTRL_SET_RUN_TIME_INFO,
    AE_CTRL_CLEAR_CPU_SHOULD_GET_DATA_INFO,
    AE_CTRL_SET_LAST_RESULT,
    AE_CTRL_SET_AF_NOTIFY,
    AE_CTRL_SET_FD_INFO,
    AE_CTRL_GET_INIT_DATA_POINTER_MAIN,
    AE_CTRL_GET_INIT_DATA_POINTER_MAIN2,
    AE_CTRL_SET_AE_SYNC_INFO,
    AE_CTRL_SET_AE_SYNC_DIRECT_MAP_INFO,
    AE_CTRL_GET_EXIF_PTR,
    AE_CTRL_MAX
} AE_CTRL_ENUM;

typedef enum
{
    AE_CORE_CAM_ID_MAIN,
    AE_CORE_CAM_ID_MAIN2,
    AE_CORE_CAM_ID_MAIN_SUB,
    AE_CORE_CAM_ID_MAIN_SUB2,
    AE_CORE_CAM_ID_MAIN3,
    AE_CORE_CAM_ID
} AE_CORE_CAM_ID_ENUM;

#define LumLog2x1000_TABLE_SIZE 4096

#define CCU_AE_TOTAL_BANK_CNT   (90)
#define CCU_AE_BANK_SIZE        (120)
#define CCU_AE_CUST_HIST_SIZE   (256)
typedef struct
{
    MUINT16 r[CCU_AE_BANK_SIZE*CCU_AE_TOTAL_BANK_CNT];
    MUINT16 g[CCU_AE_BANK_SIZE*CCU_AE_TOTAL_BANK_CNT];
    MUINT16 b[CCU_AE_BANK_SIZE*CCU_AE_TOTAL_BANK_CNT];
    MUINT16 y[CCU_AE_BANK_SIZE*CCU_AE_TOTAL_BANK_CNT];
    MUINT32 stat_width;
    MUINT32 stat_height;
    MUINT32 histFullR[CCU_AE_CUST_HIST_SIZE];
    MUINT32 histFullG[CCU_AE_CUST_HIST_SIZE];
    MUINT32 histFullB[CCU_AE_CUST_HIST_SIZE];
    MUINT32 histFullY[CCU_AE_CUST_HIST_SIZE];
    MUINT32 histFullRGB[CCU_AE_CUST_HIST_SIZE];
    MUINT32 histCentralY[CCU_AE_CUST_HIST_SIZE];
} CCU_AE_STAT;

typedef struct
{
    MBOOL Call_AE_Core_init;
    strAERealSetting InitAESetting;
    MUINT32 u4LastIdx;
    MUINT32 EndBankIdx;
    MUINT32 u4InitIndex;

    MINT32  m_i4AEMaxBlockWidth;
    MINT32  m_i4AEMaxBlockHeight;
    MUINT32 m_u4AETarget;

    MUINT32 m_LineWidth;
    MUINT32 m_u4IndexFMax;
    MUINT32 m_u4IndexFMin;
    MUINT32 m_u4IndexMax;
    MUINT32 m_u4IndexMin;
    MUINT32 m_u4FinerEVIdxBase; // Real index base
    MUINT32 m_u4AAO_AWBValueWinSize;            // bytes of AWB Value window
    MUINT32 m_u4AAO_AWBSumWinSize;              // bytes of AWB Sum window
    MUINT32 m_u4AAO_AEYWinSize;                 // bytes of AEY window
    MUINT32 m_u4AAO_AEYWinSizeSE;                 // bytes of AEY window
    MUINT32 m_u4AAO_AEOverWinSize;              // bytes of AEOverCnt window
    MUINT32 m_u4AAO_HistSize;                   // bytes of each Hist
    MUINT32 m_u4AAO_HistSizeSE;                   // bytes of each Hist
    MINT32 i4MaxBV;
    MINT32 i4MinBV;
    MINT32 i4BVOffset;
    MUINT16 u2Length;

    CCU_LIB3A_AE_METERING_MODE_T eAEMeteringMode;
    CCU_LIB3A_AE_SCENE_T eAEScene;
    CCU_LIB3A_AECAM_MODE_T eAECamMode;
    CCU_LIB3A_AE_FLICKER_MODE_T eAEFlickerMode;
    CCU_LIB3A_AE_FLICKER_AUTO_MODE_T eAEAutoFlickerMode;
    CCU_LIB3A_AE_EVCOMP_T eAEEVcomp;
    MUINT32 u4AEISOSpeed;
    MINT32 i4AEMaxFps;
    MINT32 i4AEMinFps;
    MINT32 i4SensorMode;
    MUINT32 u4ExpUnit;
    CCU_AE_SENSOR_DEV_T eSensorDev;
    CCU_CAM_SCENARIO_T eAEScenario;
    CCU_AE_TargetMODE InitAETargetMode;
    /*ISP 6.0*/
    CCU_AWBSTAT_CONFIG_BLK AWBStatConfig;
    CCU_AAOConfig_T AAOStatConfig;

    /*!!!!!!!!!!!!!!! PONITER MUST BE PUT AT BOTTOM MOST (due to CCU driver constraint) !!!!!!!!!!!!!!!!*/
    MINT32 *m_LumLog2x1000;
    CCU_strFinerEvPline *pCurrentTableF;
    CCU_strEvPline *pCurrentTable;
    CCU_strAEPLineGainList *pCurrentGainList;
    MUINT32 *pEVValueArray;
    CCU_strAEMovingRatio *pAETouchMovingRatio;
    CCU_AE_NVRAM_T* pAeNVRAM;
    CCU_AESYNC_NVRAM_T *prAeSyncNvram;
    MVOID *pCusAEParam;
} AE_CORE_INIT;

typedef struct
{
    MINT32 CamID;
    MINT32 MasterCamId;
    MUINT8 *pAEBuffer;
    MUINT8 *pHistBuffer;
    MUINT8 *pHistBufferSE;
    MVOID *pCustStatData;
    CCU_AE_STAT* p_ae_stat;

    MUINT32 YOffset;
    MUINT32 YLineNum;
    MUINT8 CrntBankIdx;

    /* HDR-AE INPUT DATA */
    CCU_AE_MHDR_STATS MHDRStatsData;
    MUINT32 u4CurHDRRatio;
    MUINT32 u4BufSizeX;
    MUINT32 u4BufSizeY;
    /* HDR-AE INPUT DATA */

    MUINT32 ae_cust_stat_enable;   // 0: normal flow, 1: bypass AE main algo
} AE_CORE_MAIN_IN;

typedef struct
{
    strAERealSetting RealOutput;
    MINT32 i4Bv;
    MINT32 i4AoeCompBv;
    MINT32 i4EV;
    MINT16 i2FaceDiffIndex;
    MUINT32 u4FracGain;    // stepless gain, lower than 0.1EV
    MBOOL bAEStable;      // Only used in Preview/Movie
    MUINT32 u4CWValue;
    MUINT32 u4AECondition;
    MINT32 i4DeltaBV;
    MINT32 i4PreDeltaBV;
    MUINT32 u4BayerY;
    MUINT32 u4MonoY;
    MINT32 i4RealBVx1000;
    MINT32 i4RealBV;
    MUINT32 u4CWRecommendStable;
    MUINT32 u4FaceFailCnt;
    MUINT32 u4FaceFoundCnt;
    MUINT32 u4FaceAEStable;
    MINT32 i4AEidxNext;
    MUINT32 u4EndBankIdx;
    MBOOL bIsAFLock;
    MBOOL bIsAPLock;
} AE_CORE_MAIN_OUT;

typedef struct
{
    AE_CORE_INIT *pInitPtr;
} AE_CORE_CTRL_DATA_POINTER;

typedef struct
{
    MUINT32 u4Cmd;
    MUINT32 u4Value;
} AE_ADB_RTT;

typedef struct
{
    MBOOL bEableManualTarget;
    MUINT32 u4ManualTarget;
    MBOOL bEableCalib;
    MBOOL bEnableRTT;
    MUINT32 u4RTTCnt;
    AE_ADB_RTT RTTList[AE_ADB_RTT_TOTAL_CNT];
} AE_ADB_CFG;

typedef struct
{
    //ROI
    CCU_AE_BLOCK_WINDOW_T m_AEWinBlock;
    // Face ROI
    CCU_AEMeteringArea_T m_eAEFDArea;
    CCU_AE_TOUCH_BLOCK_WINDOW_T m_AETOUCHWinBlock[CCU_MAX_AE_METER_AREAS];
    MUINT8 m_u4MeteringCnt;
    //EV Settings
    CCU_LIB3A_AE_EVCOMP_T eAEComp;
    /**
     * Other cmdset
     */
    CCU_AAO_PROC_INFO_T  pAAOProcInfo;
    CCU_LIB3A_AE_METERING_MODE_T eAEMeteringMode;
    CCU_LIB3A_AE_SCENE_T eAEScene;
    CCU_LIB3A_AECAM_MODE_T eAECamMode;
    CCU_LIB3A_AE_FLICKER_MODE_T eAEFlickerMode;
    CCU_LIB3A_AE_FLICKER_AUTO_MODE_T eAEAutoFlickerMode;
    MUINT32 u4AEISOSpeed;
    MINT32 i4AEMaxFps;
    MINT32 i4AEMinFps;
    MINT32 i4SensorMode;
    MINT32 i4DeltaSensitivityIdx;
    MINT32 i4DeltaBVIdx;
    AE_ADB_CFG ae_adb_cfg;
    MBOOL bIsoSpeedReal;
    //request number
    MUINT32 camReqNumber;

    /*ISP 6.0*/
    CCU_AE_CTRL AECtrl;
    MUINT32 u4CycleCnt;
    MUINT32 u4MagicNo;
    MBOOL bIsSlowMotion;
    MBOOL bIsLimitMode;
    MINT32 GyroAcc[3];
    MINT32 GyroRot[3];
    CCU_AE_ZOOM_WIN_INFO ZoomWinInfo;
} AE_CORE_CTRL_CCU_VSYNC_INFO;

typedef struct
{
    MINT32 i4MaxBV;
    MINT32 i4MinBV;
    MINT32 i4CapMaxBV;
    MINT32 i4CapMinBV;
    MUINT32 u4TotalIndex;
    MUINT32 u4CapTotalIndex;
    CCU_eAETableID  eID;
    CCU_eAETableID  eCapID;
    MUINT32 m_u4FinerEVIdxBase;
    MUINT32 u4UpdateLockIndex;
    MUINT32 u4IndexMax;
    MUINT32 u4IndexMin;
    MUINT32 u4NvramIdx;
    CCU_AE_PRIORITY_INFO PriorityInfo;

    AE_LTM_NVRAM_PARAM LtmNvramParam;
    CCU_strEvPline *pCurrentTable;
    CCU_strFinerEvPline *pCurrentTableF; /**< Specifies AE pline table of current scenario*/
    CCU_AE_NVRAM_T* pAeNVRAM;
    MVOID  *pCusAEParam;
} AE_CORE_CTRL_RUN_TIME_INFO;

typedef struct
{
    strAERealSetting *pLastResult;
    MBOOL bIsManualExp;
} AE_CORE_CTRL_LAST_RESULT;

typedef struct
{
    MBOOL bAELock;
} AE_CORE_CTRL_AF_NOTIFY;

typedef struct
{
    CCU_AEMeteringArea_T AEFDArea;
} AE_CORE_CTRL_FD_INFO;

typedef struct
{
    CCU_SYNC_AE_OUTPUT_T *pAeSyncInfo;
} AE_CORE_CTRL_AE_SYNC_INFO;


typedef struct
{
    CCU_SYNC_AE_DIRECT_MAP_RESULT_STRUCT *pAeDirectMapInfo;
} AE_CORE_CTRL_AE_DIRECT_MAP_INFO;


typedef struct
{
    AE_CORE_INIT *pInitPtr;
    AE_CORE_CTRL_CCU_VSYNC_INFO *pVsyncInfoPtr;
    CCU_AeAlgo *pAeAlgoDataPtr;
    CCU_SYNC_AE_INPUT_T *pAeSyncAlgoInPtr;
    CCU_SYNC_AE_OUTPUT_T *pAeSyncAlgoOutPtr;
} AE_CORE_CTRL_GET_EXIF_PTR;

typedef enum
{
    CCU_DUAL_CAM_SYNC_WIDE,
    CCU_DUAL_CAM_SYNC_TELE,
    CCU_DUAL_CAM_SYNC_BAYER,
    CCU_DUAL_CAM_SYNC_MONO
} CCU_DUAL_CAM_SYNC_MODE_ENUM;

#ifdef WIN32

#ifdef __cplusplus
extern "C" {
#endif

extern void AE_Core_Init(AE_CORE_CAM_ID_ENUM id);

extern void AE_Core_Stat(AE_CORE_MAIN_IN *in, AE_CORE_MAIN_OUT *out);

extern void AE_Core_Ctrl(AE_CTRL_ENUM id, void *ctrl_in, void *ctrl_out, AE_CORE_CAM_ID_ENUM cam_id);

extern void AE_Core_Main(AE_CORE_MAIN_IN *in, AE_CORE_MAIN_OUT *out);

extern void AE_Core_Reset(void);

#ifdef __cplusplus
}
#endif

#else
void AE_Core_Init(AE_CORE_CAM_ID_ENUM id);

void AE_Core_Ctrl(AE_CTRL_ENUM id, void *ctrl_in, void *ctrl_out, AE_CORE_CAM_ID_ENUM cam_id);

void AE_Core_Stat(AE_CORE_MAIN_IN *in, AE_CORE_MAIN_OUT *out);

void AE_Core_Main(AE_CORE_MAIN_IN *in, AE_CORE_MAIN_OUT *out);

void AE_Core_Reset(void);

#endif // WIN32

#endif // __AE_STREAM_CORE__

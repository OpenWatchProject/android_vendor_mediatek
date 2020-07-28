#ifndef __AE_STREAM_CORE__
#define __AE_STREAM_CORE__
#include "ccu_ext_interface/ccu_types.h"
#include "algorithm/ccu_ae_param.h"
#include "algorithm/ccu_ae_nvram.h"
#include "algorithm/ccu_AEPlinetable.h"
#include "algorithm/ccu_ae_algo_data.h"
#include "algorithm/ccu_n3d_ae_algo_data.h"

/*
typedef struct {
    MUINT16 u2Length;
    MINT16 *IDX_Partition;
    MINT16 *IDX_Part_Middle;
} CCU_ISP_NVRAM_ISO_INTERVAL_STRUCT, *P_CCU_ISP_NVRAM_ISO_INTERVAL_STRUCT;
*/
// typedef struct
// {
//     CCU_strEvSetting  EvSetting;
//     MINT32        Bv;
//     MINT32        AoeCompBv;
//     MINT32        i4EV;
//     MUINT32       u4ISO;          //correspoing ISO , only use in capture
//     MINT16        i2FaceDiffIndex;
//     MINT32        i4AEidxCurrent;  // current AE idx
//     MINT32        i4AEidxNext;  // next AE idx
//     MINT32        i4Index;        
//     MINT32        i4Cycle0FullDeltaIdx;
//     MINT32        i4FrameCnt;
//     MUINT32           u4FracGain;    // stepless gain, lower than 0.1EV
//     MUINT32       u4Prvflare;
//     MBOOL             bAEStable;      // Only used in Preview/Movie
//     MBOOL             bGammaEnable;   // gamma enable
//     MUINT32           u4CWValue;
//     MUINT32           u4AECondition;
//     MINT32            i4DeltaBV;
//     MINT32            i4PreDeltaBV;    
//     MUINT32           u4DeltaBVRatio[CCU_SYNC_AE_DUAL_CAM_DENOISE_MAX];
//     MUINT16           u2FrameRate;     // Calculate the frame
//     MINT16           i2FlareOffset;
//     MINT16           i2FlareGain;   // in 512 domain
//     MINT32           i4AEidxCurrentF;  // current AE idx
//     MINT32           i4AEidxNextF;   // next AE idx
//     MINT32           i4gammaidx;   // next gamma idx
//     MINT32           i4LESE_Ratio;    // LE/SE ratio
// } strCcuAeOutput;

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
    AE_CTRL_GET_INIT_DATA_POINTER_MAIN,
    AE_CTRL_GET_INIT_DATA_POINTER_MAIN2,
    AE_CTRL_SET_AE_SYNC_INFO,
    AE_CTRL_SET_AE_SYNC_DIRECT_MAP_INFO,
    AE_CTRL_GET_EXIF_PTR,
    AE_CTRL_SET_CYCLE_INFO,
    AE_CTRL_SET_CAM_INFO,
    AE_CTRL_MAX
}   AE_CTRL_ENUM;

typedef enum
{
    AE_CORE_CAM_ID_MAIN,
    AE_CORE_CAM_ID_MAIN2,
    AE_CORE_CAM_ID_MAIN_SUB,
    AE_CORE_CAM_ID
}   AE_CORE_CAM_ID_ENUM;

#define LumLog2x1000_TABLE_SIZE 1024

typedef struct
{
    MBOOL Call_AE_Core_init;
    MUINT32 u4LastIdx;
    MUINT32 EndBankIdx;
    MUINT32 u4InitIndex;


    MINT32  m_i4AEMaxBlockWidth;
    MINT32  m_i4AEMaxBlockHeight;
    // MUINT32 m_pWtTbl_W[5][5];
    // MBOOL   m_AECmdSet_bRotateWeighting;
    MINT32 OBC_ISO_IDX_Range[5];
    MUINT32 m_u4AETarget;

    /* OBC version2 */
    MUINT16 u2LengthV2;
    MINT16 IDX_PartitionV2[NVRAM_ISP_REGS_ISO_SUPPORT_NUM];
    CCU_ISP_NVRAM_OBC_T OBC_TableV2[NVRAM_ISP_REGS_ISO_SUPPORT_NUM];
    CCU_ISP_NVRAM_OBC_T OBC_Table[4];

    MUINT32 m_LineWidth;
    MUINT32 m_u4IndexFMax;
    MUINT32 m_u4IndexFMin;
    MUINT32 m_u4IndexMax;
    MUINT32 m_u4IndexMin;
    MUINT32 m_u4FinerEVIdxBase; // Real index base
    MUINT32 m_u4AAO_AWBValueWinSize;            // bytes of AWB Value window
    MUINT32 m_u4AAO_AWBSumWinSize;              // bytes of AWB Sum window 
    MUINT32 m_u4AAO_AEYWinSize;                 // bytes of AEY window
    MUINT32 m_u4AAO_AEOverWinSize;              // bytes of AEOverCnt window
    MUINT32 m_u4AAO_HistSize;                   // bytes of each Hist
    MUINT32 m_u4PSO_SE_AWBWinSize;              // bytes of AWB window
    MUINT32 m_u4PSO_LE_AWBWinSize;              // bytes of AWB window
    MUINT32 m_u4PSO_SE_AEYWinSize;              // bytes of AEY window  
    MUINT32 m_u4PSO_LE_AEYWinSize;              // bytes of AEY window
    MUINT32 m_u4PSO_SE_HistSize;                // bytes of each Hist
    MUINT32 m_u4PSO_LE_HistSize;                // bytes of each Hist
	MINT32 i4MaxBV;
    MINT32 i4MinBV;
	MINT32 i4BVOffset;
    MUINT16 u2Length;

    //CCU_AE_DEVICES_INFO_T rDevicesInfo;

    //AE_NVRAM_T rAENVRAM;         // AE NVRAM param
    //AE_PARAM_T rAEPARAM;
    //AE_PLINETABLE_T *rAEPlineTable;
    //AE_PLINEMAPPINGTABLE_T rAEPlineMapTable[30];
    //EZOOM_WINDOW_T rEZoomWin;
    //MINT32 i4AEMaxBlockWidth;  // AE max block width
    //MINT32 i4AEMaxBlockHeight; // AE max block height
    //MINT32 i4AAOLineByte;
    CCU_LIB3A_AE_METERING_MODE_T eAEMeteringMode;
    CCU_LIB3A_AE_SCENE_T eAEScene;
    CCU_LIB3A_AECAM_MODE_T eAECamMode;
    CCU_LIB3A_AE_FLICKER_MODE_T eAEFlickerMode;
    CCU_LIB3A_AE_FLICKER_AUTO_MODE_T eAEAutoFlickerMode;
    CCU_LIB3A_AE_EVCOMP_T eAEEVcomp;
    MUINT32 u4AEISOSpeed;
    MINT32    i4AEMaxFps;
    MINT32    i4AEMinFps;
    MINT32    i4SensorMode;
    CCU_AE_SENSOR_DEV_T eSensorDev;
    CCU_CAM_SCENARIO_T eAEScenario;

    /*!!!!!!!!!!!!!!! PONITER MUST BE PUT AT BOTTOM MOST (due to CCU driver constraint) !!!!!!!!!!!!!!!!*/
    MINT32 *m_LumLog2x1000;
    CCU_strFinerEvPline *pCurrentTableF;
    CCU_strEvPline *pCurrentTable;
    MUINT32 *pEVValueArray;
    MINT16 *IDX_Partition;
    MINT16 *IDX_Part_Middle;
    //Smooth
    CCU_strAEMovingRatio *rAEMovingRatio;
    CCU_strAEMovingRatio *pAETouchMovingRatio;
    CCU_strAEMovingRatio *rAEVideoMovingRatio;    // Video
    CCU_strAEMovingRatio *rAEFaceMovingRatio;     // Face AE
    CCU_strAEMovingRatio *rAETrackingMovingRatio; // Object Tracking
    CCU_AE_NVRAM_T* pAeNVRAM;
}AE_CORE_INIT;

typedef struct
{
    MINT32           CamID;
    MINT32           MasterCamId;
    MUINT8           *a_pAEBuffer;
    MUINT32          *a_pHistBuffer0;
    MUINT32          *a_pHistBuffer4;
    MUINT8           *a_pPSOuffer;


    MUINT32     YOffset;
    MUINT32     YLineNum;
    MUINT8      CrntBankIdx;

    //Smooth
    CCU_strAEInput aeInput;

}AE_CORE_MAIN_IN;

typedef struct
{
    CCU_strAEOutput a_Output;

    //AE_CPU_ACTION_ENUM enumCpuAction;
    //AE_CCU_ACTION_ENUM enumCcuAction;

    CCU_ISP_NVRAM_OBC_T ObcResult;



    //Smooth
    //CCU_strAEOutput aeOutput;
    MUINT32 EndBankIdx;
}AE_CORE_MAIN_OUT;

typedef struct
{
    AE_CORE_INIT *pInitPtr;

}AE_CORE_CTRL_DATA_POINTER;

typedef struct
{

    MUINT32 u4FaceFailCnt;
    MUINT32 u4FaceFoundCnt;
    MUINT32 u4FaceAEStable;

}FACE_AE_BACKUP;

typedef struct
{
    //MUINT32 SkipAEAlgo;
    MUINT32 u4CpuReady;
    MUINT32 m_u4EffectiveIndex;
    MUINT32 m_i4deltaIndex;
    MUINT32 m_u4Index;
    MUINT32 m_u4IndexF;
    MUINT32 u4Eposuretime;
    MUINT32 u4AfeGain;
    MUINT32 u4IspGain;
    MUINT32 u4ISO;
    FACE_AE_BACKUP face_ae_backup;
}AE_CORE_CTRL_CPU_EXP_INFO;


typedef struct 
{
    MBOOL bEableManualTarget;
    MUINT32 u4ManualTarget;
    MBOOL bEableCalib;

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
    MBOOL flare_disable;
    CCU_LIB3A_AE_METERING_MODE_T eAEMeteringMode;
    CCU_LIB3A_AE_SCENE_T eAEScene;
    CCU_LIB3A_AECAM_MODE_T eAECamMode;
    CCU_LIB3A_AE_FLICKER_MODE_T eAEFlickerMode;
    CCU_LIB3A_AE_FLICKER_AUTO_MODE_T eAEAutoFlickerMode;
    MUINT32 u4AEISOSpeed;
    MINT32    i4AEMaxFps;
    MINT32    i4AEMinFps;
    MINT32    i4SensorMode;
    CCU_eAESTATE AeState;
    MINT32 i4DeltaSensitivityIdx;
    MINT32 i4DeltaBVIdx;
    MUINT32 u4NvramIdx;
    AE_ADB_CFG ae_adb_cfg;
    MUINT32 u4IsCalAE;
}AE_CORE_CTRL_CCU_VSYNC_INFO;


typedef struct
{
    MUINT32 m_u4IndexMax;
    MUINT32 m_u4IndexMin;
    MUINT32 m_u4IndexFMax; /**< Specifies max AE pline index of current scenario*/
    MUINT32 m_u4IndexFMin; /**< Specifies minimum AE pline index of current scenario*/
    MUINT32 m_u4IndexF;
    MUINT32 m_u4Index;
    MINT32 i4MaxBV;
    MINT32 i4MinBV;
    /* OBC version2 */
    // MUINT16 u2LengthV2;
    // MINT16 IDX_PartitionV2[NVRAM_ISP_REGS_ISO_SUPPORT_NUM];
    CCU_ISP_NVRAM_OBC_T OBC_TableV2[NVRAM_ISP_REGS_ISO_SUPPORT_NUM];
    CCU_ISP_NVRAM_OBC_T OBC_Table[4];
    
    MUINT32 m_u4FinerEVIdxBase;
    MUINT32 m_u4Prvflare;
    MUINT32 u4UpdateLockIndex;
    CCU_strEvPline *pCurrentTable;
    CCU_strFinerEvPline *pCurrentTableF; /**< Specifies AE pline table of current scenario*/
    CCU_AE_NVRAM_T* pAeNVRAM;

}AE_CORE_CTRL_RUN_TIME_INFO;

typedef struct
{
    AE_CORE_INIT *pInitPtr;
    AE_CORE_CTRL_CCU_VSYNC_INFO *pVsyncInfoPtr;
    CCU_AeAlgo *pAeAlgoDataPtr;

    CCU_N3DAE_STRUCT *pN3dAePtr;
    CCU_strAEOutput **pN3dOutputPtr;
    CCU_N3D_AE_INIT_STRUCT *pN3dInitPtr;


}AE_CORE_CTRL_GET_EXIF_PTR;

typedef struct
{
    MUINT32 u4CycleCnt;
}AE_CORE_CTRL_CYCLE_INFO;

typedef struct
{
    CCU_SYNC_AE_OUTPUT_STRUCT *pAeSyncInfo;
} AE_CORE_CTRL_AE_SYNC_INFO;


typedef struct
{
    CCU_SYNC_AE_DIRECT_MAP_RESULT_STRUCT *pAeDirectMapInfo;
} AE_CORE_CTRL_AE_DIRECT_MAP_INFO;

typedef enum
{
    CCU_DUAL_CAM_SYNC_WIDE,
    CCU_DUAL_CAM_SYNC_TELE,
    CCU_DUAL_CAM_SYNC_BAYER,
    CCU_DUAL_CAM_SYNC_MONO
}CCU_DUAL_CAM_SYNC_MODE_ENUM;

void AE_Core_Init(AE_CORE_CAM_ID_ENUM id);

void AE_Core_Ctrl(AE_CTRL_ENUM id, void *ctrl_in, void *ctrl_out, AE_CORE_CAM_ID_ENUM cam_id);

void AE_Core_Main(AE_CORE_MAIN_IN *in, AE_CORE_MAIN_OUT *out);

void AE_Core_Reset(void);

int GetOBGain(MUINT32 real_iso, MUINT32 ISPGain, CCU_ISP_NVRAM_OBC_T *obc);
#endif



/********************************************************************************************
 *     LEGAL DISCLAIMER
 *
 *     (Header of MediaTek Software/Firmware Release or Documentation)
 *
 *     BY OPENING OR USING THIS FILE, BUYER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 *     THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE") RECEIVED
 *     FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO BUYER ON AN "AS-IS" BASIS
 *     ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES, EXPRESS OR IMPLIED,
 *     INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR
 *     A PARTICULAR PURPOSE OR NONINFRINGEMENT. NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY
 *     WHATSOEVER WITH RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY,
 *     INCORPORATED IN, OR SUPPLIED WITH THE MEDIATEK SOFTWARE, AND BUYER AGREES TO LOOK
 *     ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. MEDIATEK SHALL ALSO
 *     NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE RELEASES MADE TO BUYER'S SPECIFICATION
 *     OR TO CONFORM TO A PARTICULAR STANDARD OR OPEN FORUM.
 *
 *     BUYER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND CUMULATIVE LIABILITY WITH
 *     RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE, AT MEDIATEK'S OPTION,
 *     TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE
 *     FEES OR SERVICE CHARGE PAID BY BUYER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 *     THE TRANSACTION CONTEMPLATED HEREUNDER SHALL BE CONSTRUED IN ACCORDANCE WITH THE LAWS
 *     OF THE STATE OF CALIFORNIA, USA, EXCLUDING ITS CONFLICT OF LAWS PRINCIPLES.
 ************************************************************************************************/

#pragma once

#include <utils/threads.h>

/* camera headers */
#include "mtkcam/def/Modes.h"
#include "mtkcam/aaa/aaa_hal_common.h"
#include "camera_custom_awb_nvram.h"
#include "flash_param.h" // needed by FlashAlg.h
#include "flash_awb_param.h" // needed by FlashAlg.h
#include "ae_param.h"
#include "FlashAlg.h"
#include "flash_mgr.h"
#include "dbg_aaa_param.h"

/* Face headers */
#include <faces.h>

using namespace android;
#if (CAM3_3A_ISP_30_EN || CAM3_3A_ISP_40_EN)
using namespace NS3A;
#endif

#define FLASH_STATE_STACK_NUM 3

/* maximum face number*/
#define FACE_MAX_NUM 15

/** flash error return */
typedef enum {
    FL_ERR_FlashModeNotSupport = -100,
    FL_ERR_AFLampModeNotSupport = -100,
    FL_ERR_SetLevelFail = -101,
    FL_ERR_CCT_INPUT_SIZE_WRONG = -10001,
    FL_ERR_CCT_OUTPUT_SIZE_WRONG = -10002,
    FL_ERR_CCT_FILE_NOT_EXIST = -10003,
} FlashMgrEnum;

/** flash capture */
typedef struct {
    float digRatio;         /** digial zoom ratio (from API) */
    int aeScene;            /** ae scene (from AE management) */
    int flashMode;          /** flash internal mode (from API) */
    int vBat;               /** battery voltage in mV (from power sysfs) */
    int flickerMode;        /** filcker mode (from arguments of doPfOneFrame()) */
    float evComp;           /** EV compensation (from API) */
    int isBurst;            /** continue shot (from API) */

    /*
     * Defined in mtkcam/include/mtkcam/def/Modes.h
     * enum EAppMode
     * {
     *     eAppMode_DefaultMode = 0, // Default Mode
     *     eAppMode_EngMode,         // Engineer Mode
     *     eAppMode_AtvMode,         // ATV Mode
     *     eAppMode_StereoMode,      // Stereo Mode
     *     eAppMode_VtMode,          // VT Mode
     *     eAppMode_PhotoMode,       // Photo Mode
     *     eAppMode_VideoMode,       // Video Mode
     *     eAppMode_ZsdMode,         // ZSD Mode
     *     eAppMode_FactoryMode,     // Factory Mode
     *     eAppMode_DualCam,         // Dual Zoom Mode
     * };
     */
    int appMode;
    NS3Av3::EBitMode_T bitMode;
    int hasHw;
    int inCharge;
    int isBatLow;
    int isChargerReady;
    int manualExposureTime;
    int manualSensitivity;
    int subFlashState;
    int hasPanelFlash;
    int useFlashFlow;
} FlashCapInfo;

/** flash capture reply */
typedef struct {
    int capIso;
    int capAfeGain;
    int capIspGain;
    int capExp;
    int capDuty;
    int capDutyLT;
    int dutyNum;
    int dutyNumLT;

    int pfStartTime; /** pre-flash start time(ms) */
    int pfCoolTime;
    int pfRunTime;   /** pre-flash run time(ms) */
    int pfEndTime;   /** pre-flash end time(ms) */
    int pfRunStartFrm; /** pre-flash run frame number */
    int pfRunFrms;     /** pre-flash run frame count */
    int pfApplyFrm;
    int pfFrameCycle;
    int pfIteration;

    int pfState;
    int pfPreState;

    int isMainFlash;
    int isPreFlash;
    int pfAlgoDone;

    int errCnt;
    int err1;
    int err2;
    int err3;
    int errTime1;
    int errTime2;
    int errTime3;
} FlashCapRep;

/*Face Information for face flash algorithm*/
typedef struct
{
    int   x1;
    int   y1;
    int   x2;
    int   y2;
    float weight;
} FaceInformation;

/**
 * FlashMgr: the class is mainly for handling flash.
 */
class FlashMgrM : public FlashMgr
{
    public:
        /**
         * @brief Constructor
         */
        FlashMgrM(int sensorDev);

        /**
         * @brief Deconstructor
         */
        ~FlashMgrM();

        /**
         * @brief Get the handle of FlashMgr with singleton pattern
         */
        static FlashMgrM *getInstance(int sensorDev);


        /************************************************************
         * Life Cycle
         ***********************************************************/

        /**
         * @brief Initialize flash.
         *
         * Triggered at enter camera.
         */
        int init();

        /**
         * @brief Uninitialize flash.
         *
         * Triggered at exit camera.
         */
        int uninit();

        /**
         * @brief Start flash.
         *
         * Triggered at config or start image sensors.
         * It means that 3A starts to preview, capture, video record.
         * In other words, it usually comes with changing sensor mode.
         */
        int start();

        /**
         * @brief Stop flash.
         *
         * Triggered at stop image sensors.
         */
        int stop();


        /************************************************************
         * Attribute
         ***********************************************************/

        /**
         * @brief is instance avalible for operation
         * @return available status
         */
        int isAvailable();

        /**
         * @brief get in charge status
         * @return get the current instance is in charge or not
         */
        int getInCharge();

        /**
         * @brief set in charge
         * @param[in] inCharge: the current instance is in charge or not
         */
        int setInCharge(int inCharge);

        int getFlashSpMode();

        /**
         * @brief get flash mode.
         *
         * Could be two type of structure (with the same definition):
         *   AE_STROBE_T (kd_camera_feature_enum.h)
         *   LIB3A_FLASH_MODE_T (flash_feature.h)
         *
         * @return flash mode
         */
        int getFlashMode();

        /**
         * @brief get flash flow type.
         * @return 1: new flow, 0: legacy flow
         */
        int getFlashFlowType();

        /**
         * @brief set flash mode
         */
        int setFlashMode(int mode);

        /**
         * @brief set flash mode from Android metadata.
         *
         * Two related key of android metadata:
         *   CONTROL_AE_MODE (renamed MTK_CONTROL_AE_MODE)
         *   FLASH_MODE (renamed MTK_FLASH_MODE)
         *
         * Type of structure:
         *   mtk_camera_metadata_enum_android_control_ae_mode_t (mtk_metadata_tag.h)
         *   mtk_camera_metadata_enum_android_flash_mode_t (mtk_metadata_tag.h)
         */
        int setAeFlashMode(int aeMode, int flashMode);

        /**
         * @brief get af lamp mode
         */
        int getAfLampMode();

        /**
         * @brief Set camera mode.
         *
         * Type of structure:
         *   EAppMode (mtkcam/include/mtkcam/def/Modes.h)
         */
        int setCamMode(int mode);

        /**
         * @brief Set ISP bit mode.
         *
         * Type of structure:
         *   EBitMode_T (mtkcam/include/mtkcam/aaa/aaa_hal_common.h)
         */
        int setBitMode(NS3Av3::EBitMode_T mode);

        /**
         * @brief Set digital zoom.
         * @param[in] digx100: digital zoom ratio * 100
         */
        int setDigZoom(int digx100);

        /**
         * @brief Set EV compensation value.
         *
         * The EV compensation value = EV index * EV step.
         *
         * @param[in] index: index of EV compensation
         * @param[in] evStep: step of EV compensation
         */
        int setEvComp(int index, float evStep);

        /**
         * @brief Set manual exposure time from metadata
         * @param[in] i8ExposureTime: Exposure time in nanoseconds
         */
        int setManualExposureTime(MINT64 i8ExposureTime);

        /**
         * @brief Set manual sensitivity from metadata
         * @param[in] i4Sensitivity: Sensitivity (ISO) value
         */
        int setManualSensitivity(MINT32 i4Sensitivity);

        int setMultiCapture(int multiCapture);

        /**
         * @brief Set NVRAM index for parameter mapping.
         * @param[in] eNVRAM: enum specifying parameter group
         * @param[in] a_eNVRAMIndex: mapping index for scenario
         */
        int setNVRAMIndex(FLASH_NVRAM_ENUM eNVRAM, MUINT32 a_eNVRAMIndex);

        /**
         * @brief check if firing flash or not at capture.
         * @return 1: fire flash at capture, 0: NOT fire at capture.
         */
        int isFlashOnCapture();
        int setIsFlashOnCapture(int flashOnCapture);

        int isFlashOnCalibration();


        /************************************************************
         * Main Function
         ***********************************************************/

        /**
         * @brief update flash state.
         *
         * Related key of android metadata:
         *   FLASH_STATE (renamed MTK_FLASH_STATE)
         *
         * Type of structure:
         *   mtk_camera_metadata_enum_android_flash_state_t (mtk_metadata_tag.h)
         */
        int updateFlashState();

        /**
         * @brief get flash state.
         *
         * Related key of android metadata:
         *   FLASH_STATE (renamed MTK_FLASH_STATE)
         *
         * Type of structure:
         *   mtk_camera_metadata_enum_android_flash_state_t (mtk_metadata_tag.h)
         */
        int getFlashState();

        /**
         * @brief Do flash pre-capture flow.
         *
         * Called by 3A at each preview frame in pre-capture state.
         */
        int doPfOneFrame(FlashExePara *para, FlashExeRep *rep);
        int doPfOneFrameNormal(FlashExePara *para, FlashExeRep *rep);

        /**
         * @brief End flash pre-capture flow.
         *
         * Called at the end of preview frame in pre-capture state.
         */
        int endPrecapture();

        /**
         * @brief set flash AE parameters.
         *
         * Called at the end of pre-capture state or AF state.
         */
        int setCapPara();

        /**
         * @brief set flash AE Algo Exposure input .
         *
         * Called at the end of pre-capture state.
         */
        int setAlgoExpPara(int exp, int afe, int isp, int isFlashOn);

        /**
         * @brief Get debug information.
         */
        int getDebugInfo(FLASH_DEBUG_INFO_T *p);

        /**
         * @brief set/get Panel Flash State
         */
        int setSubFlashState(int state);
        int getSubFlashState();

        /************************************************************
         * CCT related function
         ***********************************************************/
        int cctSetSpModeLock(int lock);
        int cctSetSpModeCalibration();
        MBOOL cctIsSpModeCalibration();
        int cctSetSpModeNormal();
        int cctSetSpModeQuickCalibration2();
        int cctGetQuickCalibrationResult();
        int cctGetFlashMaxIDuty(int dutyNum, int dutyNumLt, int *duty, int *dutyLt);

        /**
         * @brief Set flash enable/disable.
		 *
		 * Called by CCT: ACDK_CCT_OP_FLASH_ENABLE, ACDK_CCT_OP_FLASH_DISABLE
         */
        int cctFlashEnable(int enable);

        /**
         * @brief do ratio calibration during precapture
         */
        int cctCalibration(FlashExePara *para, FlashExeRep *rep);
        int cctCalibrationQuick2(FlashExePara *para, FlashExeRep *rep);


        /**
         * @brief Test hardware compentnent.
         *
         * When calling the function, the flash will turn on and turn off for a while.
         * Called by CCT: ACDK_CCT_OP_FLASH_CONTROL
         */
        int cctFlashLightTest(void *pIn);

        /**
         * @brief Get the flash current status (on/off).
         *
         * Called by CCT: ACDK_CCT_OP_FLASH_GET_INFO
         */
        int cctGetFlashInfo(int* isOn);

        /**
         * @brief Write buf to nvram.
         *
         * Called by CCT: ACDK_CCT_OP_STROBE_WRITE_NVRAM
         */
        int cctWriteNvram();

        /**
         * @brief Read nvram and change to structure ACDK_STROBE_STRUCT.
         *
         * Called by CCT: ACDK_CCT_OP_STROBE_READ_NVRAM_TO_PC_META
         */
        int cctReadNvramToPcMeta(void *out, MUINT32 *realOutSize);

        /**
         * @brief Read ACDK_STROBE_STRUCT structure and save to nvram.
         *
         * Called by CCT: ACDK_CCT_OP_STROBE_SET_NVDATA_META
         */
        int cctSetNvdataMeta(void *in, int inSize);

        /**
         * @brief Manual set flash duty
         *
         * Called by CCT: ACDK_CCT_OP_FLASH_SET_MANUAL_FLASH
         */
        int setManualFlash(int duty, int dutyLt);

        /**
         * @brief Manual clear flash
         *
         * Called by CCT: ACDK_CCT_OP_FLASH_CLEAR_MANUAL_FLASH
         */
        int clearManualFlash();

        int dumpMainFlashAAO(void* aao);

        void setMVHDRMode(int &i4Mode);

    private:
        /** pre-capture flow */
        int pfStart(FlashExePara *in, FlashExeRep *rep);
        void updateCapInfo(FlashExePara *in);

        int pfRun(FlashExePara *para, FlashExeRep *rep);

        int getFlashModeStyle(int sensorDev, int flashMode);

        /**
         * @brief Check if firing flash or not.
         *
         * This function will check flash hardware, power status and flash style from custom.
         * And decide whether if run flash pre-capture algo and firing capture flash or not.
         * Called in the first step of pre-capture flow.
         *
         * @return 1: fire flash, 0: NOT fire flash.
         */
        int isNeedFiringFlash();

        /**
         * @brief Get flash project parameters.
         *
         * Flash project parameters is classified by AE scene and force flash
         *
         * @param[in] aeScene: AE scene (defined in AEPlinetable.h)
         * @param[in] isForceFlash: is force flash or not
         * @return flash project parameter
         */
        FLASH_PROJECT_PARA &getFlashProjectPara(int aeScene, int isForceFlash);


        /************************************************************
         * Algo adapter layer
         ***********************************************************/
        /**
         * 1. Set pline to algo
         *    It converts AE capture pline or strobe pline to algo pline.
         * 2. set ISO gain to algo
         */
        void hw_setCapPline(FLASH_PROJECT_PARA *pPrjPara);

        /**
         * 1. Set pline to algo
         *    It converts AE preview pline to algo pline.
         */
        void hw_setPfPline();

        /** set flash profile to algo */
        void hw_setFlashProfile();

        /**
         * 1. Set preference to algo
         * 2. Set EV compensation to algo
         * 3. Set debug data size to algo
         */
        void hw_setPreference(FLASH_PROJECT_PARA *pPrjPara);

        /**
         * 1. Set exp, afe to AAA sensor mgr
         * 2. Set isp to AE mgr
         * 3. Set AE pre-flash info
         * 4. Update AE preview parameters
         */
        void hw_setPfExp(FlashAlgExpPara *expPara);

        /**
         * 1. Update AE capture parameters
         */
        void hw_setCapExp(FlashAlgExpPara *expPara);

        /** try to use reduced exp and compensated from iso */
        void hw_speedUpExpPara(FlashAlgExpPara *expPara, int maxAfe);

        /** convert AAO footprint to algo statistic data structure */
        int hw_convertAaStt(FlashAlgStaData *staData, void *staBuf, int w, int h);


        /************************************************************
         * AE wrapper layer
         ***********************************************************/
        int updateAeParams(int exp, int afe, int isp, int isCapture);
        void getAeParams(int *exp, int *afe, int *isp, int isCapture);


        /************************************************************
         * Misc
         ***********************************************************/
        template <class T>
        void gainToIso(int afe, int isp, T *iso, int isCapture, int appMode);
        template <class T>
        void isoToGain(T iso, int *afe, int *isp, int isCapture, int appMode);

        void showProjectPara(FLASH_PROJECT_PARA *pp);

        void dumpDebugData(void *buf, int w, int h,
                NS3Av3::EBitMode_T bitMode, int sceneCount, int frameCount);
        void setDebugDataPath();

        inline void setDebugTag(FLASH_DEBUG_INFO_T &a_rFlashInfo, MINT32 a_i4ID, MINT32 a_i4Value)
        {
            a_rFlashInfo.Tag[a_i4ID].u4FieldID = AAATAG(AAA_DEBUG_FLASH_MODULE_ID, a_i4ID, 0);
            a_rFlashInfo.Tag[a_i4ID].u4FieldValue = a_i4Value;
        }

        void addErr(int err);

        /***********************************************************
         *Manual flash flow
         **********************************************************/
        bool isManualFlashFlow();
        bool isShutterPriorityMode();
        bool isISOPriorityMode();
        void switchManualExposureSetting(int* exp, int* iso, const int mExp, const int mIso);

        /***********************************************************
         *High brightness flow
         **********************************************************/
        int isHighBrightnessFlow();
        void setPfHighBrightnessFlow(int isHighBrightness);

        /***********************************************************
         * Get face information
         **********************************************************/
        void getFaceInformation(MtkCameraFaceMetadata *pFaces);
        void setFaceFlashAlgoInformation(FlashAlgFacePos *pFaceInfo);
        void faceInfoSort();
        void coordinateConverter();
        int setFDInfo(void *a_sFaces, int i4tgwidth, int i4tgheight);

        /************************************************************
         * Flash-AE decider
         ***********************************************************/
        bool isEisStable();
        bool isAcceGyroStable();
        bool isLowReflectance();
        bool isLowConfidence();
        bool isWellControl();
        void searchPlineIndex(strEvSetting &a_rEvSettingInput, strEvSetting &a_rEvSettingOutput, int i4DeltaBv);
        void CalculateEtGainMainFlash();
        void CWVControl();
        void EtGainTrans4();
        void decideMainFlashEng();
        void dumpDeciderInfo();
        int refineFlashAEResult();
        void setFlashOnAAOBuffer(void *buf);
        void* getFlashOnAAOBuffer();

    private:
        mutable Mutex mLock;
        int mSensorDev;
        int mFacingSensor;

        /** preview */
        int mFlashStateStack[FLASH_STATE_STACK_NUM];

        /** pre-capture */
        int mPfFrameCount; /** frame number of each pre-flash */
        int mPfPolicy; /** pre-flash flow policy*/
        FlashAlgExpPara mAlgoExpPara;

        /** capture */
        FlashCapInfo mCapInfo;
        FlashCapRep mCapRep;

        int mFlashAENVRAMIdx;
        int mFlashAWBNVRAMIdx;
        int mFlashCaliNVRAMIdx;

        /** charger */
        int mSetCharger;

        /** calibration */
        int mIsCalibration;
        int mQuickCalibrationResult;

        /** debug */
        int mDebugSceneCnt;

        int mPropFlashMfOn;
        int mPropFlashMfDuty;
        int mPropFlashMfDutyLt;

        int mMainFlashAAOCnt;
        int mCalibrationAAOCnt;
        int mQuickCalibrationAAOCnt;

        /** Face Information */
        FaceInformation mFaceInfo[FACE_MAX_NUM];
        int mConvertedFacesPos[4 * FACE_MAX_NUM + 1]; /*First element is face number and following position order is left, top, right, bottom.*/
        int mFaceNumber;
        int mWidth;
        int mHeight;

        /** CCT tool */
        static int mIsManualFlashEnCct;
        static int mManualDutyCct;
        static int mManualDutyLtCct;
        static int mSpModeLockCct;
        static int mSpModeCct;
        static int mSpMode;

        /** Flash-AE decider */
        int mIsLowRef;
        strAEOutput m_strTorchAEOutput;
        int m_i4EngPreflash;
        int m_i4EngMainflash;
        strEvSetting m_strEtGainNormal;
        strEvSetting m_strEtGainTorchAE;
        strEvSetting m_strEtGainMainFlash;
        int m_IsMvHDREnable;

        /** Pre-flash flow for high brightness environment**/
        int mPfHighBrightness;

        /** duty */
        int mDutyNum;
        int mDutyNumLT;

        /** gain base*/
        int mIspGainBase;
        int mAfeGainBase;

        // ISP6.0 AAO parsing
        int mLinearOutputEn;

        bool m_bCctFlashHalInit;
};


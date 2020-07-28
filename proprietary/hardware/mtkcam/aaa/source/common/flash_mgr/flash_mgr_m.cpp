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

#define LOG_TAG "FlashMgrM"

/***********************************************************
 * Headers
 **********************************************************/
/* camera headers */
#include "mtkcam/def/BuiltinTypes.h"
#include "mtkcam/def/Modes.h"
#include "mtkcam/aaa/aaa_hal_common.h"
#include "flash_param.h" // needed by FlashAlg.h
#include "flash_awb_param.h" // needed by FlashAlg.h
#include "FlashAlg.h"

/* aaa headers */
#include "mtkcam/utils/metadata/client/mtk_metadata_tag.h"
#include "ae_mgr.h"
#include "awb_mgr_if.h"
#include "flicker_hal_if.h"

/* aaa common headers */
#include "file_utils.h"
#include "property_utils.h"
#include "time_utils.h"
#include "interp_utils.h"
#include "log_utils.h"
#include "math_utils.h"
#include <private/aaa_utils.h>
#include <private/aaa_hal_private.h>

/* custom headers */
#include "aaa_error_code.h"
#include "aaa_common_custom.h"
#include "flash_awb_tuning_custom.h"
#include "awb_tuning_custom.h"
#include "flash_tuning_custom.h"
#include "isp_tuning.h"
#include "flash_feature.h"
#include "dbg_flash_param.h"
#include "camera_custom_nvram.h"

/* custom common headers */
#include "custom/aaa/AEPlinetable.h"

/* flash headers */
#include "flash_mgr_m.h"
#include "flash_hal.h"
#include "flash_pline.h"
#include "flash_utils.h"
#include "flash_duty.h"
#include "flash_nvram.h"
#include "tools/flash_custom_utils.h"
#include "flash_custom_adapter.h"
#include "flash_platform_adapter.h"
#include "flash_algo_adapter.h"

/* kernel headers */
#include "kd_camera_feature.h"
#include "kd_camera_feature_enum.h"

/* misc */
#include "math.h"

//aaa_hal_sttCtrl
#include <aaa_hal_sttCtrl.h>
#include <array>

// systrace header
#include <mtkcam/utils/std/Trace.h>

using namespace NS3Av3;

/***********************************************************
 * Define macros
 **********************************************************/
/* android property */

#define PROP_FLASH_BIN_EN              "vendor.flash_bin_en"
#define PROP_FLASH_PF_BMP_EN           "vendor.flash_pf_bmp_en"
#define PROP_FLASH_MF_BMP_EN           "vendor.flash_mf_bmp_en"

#define PROP_FLASH_CALIBRATION_AAO_EN  "vendor.flash_calibration_aao_en"

#define PROP_FLASH_MF_ON               "vendor.flash_mf_on"
#define PROP_FLASH_MF_DUTY             "vendor.flash_mf_duty"
#define PROP_FLASH_MF_DUTYLT           "vendor.flash_mf_dutylt"

#define PROP_FLASH_MF_EXP              "vendor.flash_mf_exp"
#define PROP_FLASH_MF_ISO              "vendor.flash_mf_iso"
#define PROP_FLASH_MF_AFE              "vendor.flash_mf_afe"
#define PROP_FLASH_MF_ISP              "vendor.flash_mf_isp"

#define PROP_FLASH_USE_STROBE_PLINE    "vendor.flash_use_strobe_pline"

#define PROP_FLASH_PF_FOLLOW_PLINE     "vendor.flash_pf_follow_pline"
#define PROP_FLASH_PF_AFE_MAX          "vendor.flash_pf_afe_max"

#define PROP_FLASH_CHECK_CALIBRATION   "vendor.flash_check_calibration"

#define PROP_FLASH_CALIBRATION         "vendor.flash_calibration"
#define PROP_FLASH_QUICK_CALIBRATION_2 "vendor.flash_quick_calibration_2"
#define PROP_FLASH_RATIO               "vendor.flash_ratio" /* keep for legacy */
#define PROP_FLASH_RATIO_QUICK2        "vendor.flash_ratio_quick2" /* keep for legacy */

#define PROP_FLASH_2                   "vendor.hal3a.taskmgr.flash" /* new flow */

#define PROP_FLASH_ALGO_FACE_MODE      "vendor.debug.face_flash_algo_mode" /* face flash algorithm mode */

/* file path */
#define FLASH_DATA_PATH     "/data/vendor/flash/flashdata"
#define FLASH_BMP_DATA_PATH "/data/vendor/flash/flashdata/bmp"

#define FLASH_AAO_DATA_PATH "/data/vendor/flash/flashdata/aao"
#define FLASH_AAO_MEAN_DATA_PATH "/data/vendor/flash/flashdata/aao/mf_rgb_mean.txt"

#define FLASH_FILE_COUNT_PATH "/data/vendor/flash/flash_file_cnt.txt"

#define FLASH_AE_CAPTURE_PLINE_PATH         "/data/vendor/flash/ae_capture_pline.txt"
#define FLASH_AE_STROBE_PLINE_PATH          "/data/vendor/flash/ae_strobe_pline.txt"
#define FLASH_FLASH_ALGO_CAPTURE_PLINE_PATH "/data/vendor/flash/flash_algo_capture_pline.txt"

/* flash internal state */
#define FLASH_PF_STATE_START 0
#define FLASH_PF_STATE_COOLING 1
#define FLASH_PF_STATE_RUN 2
#define FLASH_PF_STATE_END 3

/* aao resize */
#define FLASH_AAO_Y_WIDTH 120
#define FLASH_AAO_Y_HEIGHT 90
#define FLASH_AAO_Y_RESIZE_WIDTH 24
#define FLASH_AAO_Y_RESIZE_HEIGHT 18
#define FLASH_AAO_Y_DIGITAL_ZOOM_WIDTH 0
#define FLASH_AAO_Y_DIGITAL_ZOOM_HEIGHT 0
#define FLASH_AAO_Y_BITNUM 10
#define FLASH_AAO_Y_NORMALIZE_FACTOR 1

#define PLINE_mAfeGainBase 1024.0
#define PLINE_mIspGainBase 1024.0

/*
 * The size of flash debug tag is defined in
 * custom/[PLATFORM]/hal/inc/debug_exif/aaa/dbg_flash_param.h
 *
 * #define FLASH_TAG_NUM (FL_T_NUM + 1600)
 *  FL_T_NUM: currently is 32
 *  1600: for algo use only
 *
 * Each tag is defined by (MUINT32 key, MUINT32 value).
 * So the size is (32 + 1600) * 4 = 6528 (Bytes)
 *
 * The ALG_TAG_SIZE must be less than (1600 * 4)
 */
//#define ALG_TAG_SIZE 6344


/***********************************************************
 * Flash management
 **********************************************************/
int FlashMgrM::mSpMode = e_SpModeNormal;

FlashMgrM::FlashMgrM(int sensorDev)
    : mSensorDev(sensorDev)
    , mFacingSensor(0)
    , mPfFrameCount(0)
    , mPfPolicy(0)
    , mFlashAENVRAMIdx(0)
    , mFlashAWBNVRAMIdx(0)
    , mFlashCaliNVRAMIdx(0)
    , mSetCharger(0)
    , mIsCalibration(0)
    , mQuickCalibrationResult(0)
    , mDebugSceneCnt(0)
    , mPropFlashMfOn(-1)
    , mPropFlashMfDuty(-1)
    , mPropFlashMfDutyLt(-1)
    , mMainFlashAAOCnt(0)
    , mCalibrationAAOCnt(0)
    , mQuickCalibrationAAOCnt(0)
    , mFaceNumber(0)
    , mWidth(1000)
    , mHeight(1000)
    , mIsLowRef(0)
    , m_i4EngPreflash(0)
    , m_i4EngMainflash(0)
    , m_IsMvHDREnable(0)
    , mPfHighBrightness(0)
    , mDutyNum(1)
    , mDutyNumLT(1)
    , mIspGainBase(1024)
    , mAfeGainBase(1024)
    , mLinearOutputEn(0)
    , m_bCctFlashHalInit(false)
{
    /* set debug */
    setDebug();

    logD("FlashMgrM(): sensorDev(%d).", mSensorDev);

    /* clear data structure */
    memset(&mCapInfo, 0, sizeof(FlashCapInfo));
    memset(&mCapRep, 0, sizeof(FlashCapRep));
    memset(&mAlgoExpPara, '\0', sizeof(FlashAlgExpPara));
    memset(mFlashStateStack, 0, sizeof(mFlashStateStack));
    memset(&m_strTorchAEOutput, 0, sizeof(strAEOutput));
    memset(&m_strEtGainNormal, 0, sizeof(strEvSetting));
    memset(&m_strEtGainTorchAE, 0, sizeof(strEvSetting));
    memset(&m_strEtGainMainFlash, 0, sizeof(strEvSetting));
    memset(&mConvertedFacesPos, 0, sizeof(mConvertedFacesPos));
    for (int i = 0; i < FACE_MAX_NUM ; i++) memset(&mFaceInfo[i], 0, sizeof(FaceInformation));

    /* update flash capture info */
    mCapInfo.digRatio = 1;
    mCapInfo.flashMode = LIB3A_FLASH_MODE_FORCE_OFF;
    mCapInfo.inCharge = 1;
    mCapInfo.bitMode = EBitMode_12Bit;
    FlashHal::getInstance(mSensorDev)->hasHw(mCapInfo.hasHw);
    FlashAlgoAdapter::getInstance(mSensorDev)->init();

    int facing = 0, num = 0, facingMain = 0;
    NS3Av3::mapSensorDevToFace(sensorDev, facing, num);
    NS3Av3::mapSensorDevToFace(DUAL_CAMERA_MAIN_SENSOR, facingMain, num);
    if (facing == facingMain)
        mFacingSensor = DUAL_CAMERA_MAIN_SENSOR;
    else
        mFacingSensor = DUAL_CAMERA_SUB_SENSOR;

    logI("mFacingSensor:%d (main:%d sub:%d)", mFacingSensor, DUAL_CAMERA_MAIN_SENSOR, DUAL_CAMERA_SUB_SENSOR);
}

FlashMgrM::~FlashMgrM()
{
    FlashAlgoAdapter::getInstance(mSensorDev)->unInit();
}

#define MY_INST NS3Av3::INST_T<FlashMgrM>
static std::array<MY_INST, SENSOR_IDX_MAX> gMultiton;

FlashMgrM *FlashMgrM::getInstance(int sensorDev)
{
    int sensorOpenIndex = NS3Av3::mapSensorDevToIdx(sensorDev);
    if(sensorOpenIndex >= SENSOR_IDX_MAX || sensorOpenIndex < 0) {
        logE("[%s] Unsupport sensor Index: %d\n", __FUNCTION__, sensorOpenIndex);
        return NULL;
    }

    MY_INST& rSingleton = gMultiton[sensorOpenIndex];
    std::call_once(rSingleton.onceFlag, [&] {
        rSingleton.instance = std::make_unique<FlashMgrM>(sensorDev);
    } );

    return rSingleton.instance.get();
}

/***********************************************************
 * Flash life cycle
 **********************************************************/
int FlashMgrM::init()
{
    logI("init().");

//init_procedure:
    mSetCharger = 0;
    mSpModeCct = e_SpModeNormal;
    mSpMode = e_SpModeNormal;
    mPfFrameCount = 0;
    mFaceNumber = 0;

    /* update capture result */
    mCapRep.isMainFlash = 0;
    mCapRep.pfApplyFrm = 0;
    mCapRep.pfState = FLASH_PF_STATE_START;

    int flashType = 0;

    if (cust_isDualFlashSupport(mFacingSensor))
        flashType = FLASH_DUAL;
    else
        flashType = FLASH_SINGLE;

    FlashAlgoAdapter::getInstance(mSensorDev)->setFlashType(flashType);
    FlashCustomAdapter::getInstance(mSensorDev)->init();

    mDutyNum = FlashCustomAdapter::getInstance(mSensorDev)->getDutyNum();
    mDutyNumLT = FlashCustomAdapter::getInstance(mSensorDev)->getDutyNumLt();
    /* update pre-flash flow policy */
    mPfPolicy = FlashCustomAdapter::getInstance(mSensorDev)->getPfPolicy();

    mIspGainBase = FlashPlatformAdapter::getIspGainBase();
    mAfeGainBase = FlashPlatformAdapter::getAfeGainBase();

    return 0;
}

int FlashMgrM::uninit()
{
    Mutex::Autolock lock(mLock);

    logD("uninit(): sensorDev(%d).", mSensorDev);

    /* increase charger voltage */
    if (mSetCharger) {
        FlashHal::getInstance(mSensorDev)->setCharger(0);
        mSetCharger = 0;
    }

    return 0;
}

int FlashMgrM::start()
{
    Mutex::Autolock lock(mLock);

    /* decrease charger voltage
     *
     * Note, this is NOT a symmetric mechanism.
     * Flash set charger at the FIRST TIME of start() but clear at uninit().
     *
     * Flash CANNOT set charger at init(),
     * becuase camera operation is not the only behavior to open camera device.
     * (ex. boot procedure of Android HAL1 and VT.)
     */

    FlashHal::getInstance(mSensorDev)->hasHw(mCapInfo.hasHw);
    mCapInfo.hasPanelFlash = FlashHal::getInstance(mSensorDev)->getSubFlashCustomization();
    mCapInfo.useFlashFlow = mCapInfo.hasHw || mCapInfo.hasPanelFlash;
    if (!mSetCharger) {
        FlashHal::getInstance(mSensorDev)->setCharger(1);
        mSetCharger = 1;
    }

    FlashPlatformAdapter::getInstance(mSensorDev)->init();
    mLinearOutputEn = FlashPlatformAdapter::getInstance(mSensorDev)->getAwbLinearOutputEn();
    return 0;
}

int FlashMgrM::stop()
{
    return 0;
}

/***********************************************************
 * Flash attributes from API
 **********************************************************/
int FlashMgrM::setDigZoom(int digx100)
{
    mCapInfo.digRatio = digx100 / 100.0;
    logD("setDigZoom(): digRatio(%f).", mCapInfo.digRatio);
    return 0;
}

int FlashMgrM::setEvComp(int index, float evStep)
{
    mCapInfo.evComp = index * evStep;
    logD("setEvComp(): evComp(%f).", mCapInfo.evComp);
    return 0;
}

int FlashMgrM::setManualExposureTime(MINT64 i8ExposureTime)
{
    if(i8ExposureTime == -1)
        mCapInfo.manualExposureTime = -1;
    else
        mCapInfo.manualExposureTime = i8ExposureTime / 1000;
    logI("setManualExposureTime(): manualExp(%d).", mCapInfo.manualExposureTime);
    return 0;
}

int FlashMgrM::setManualSensitivity(MINT32 i4Sensitivity)
{
    mCapInfo.manualSensitivity = i4Sensitivity;
    logI("setManualSensitivity(): ISO(%d)", mCapInfo.manualSensitivity);
    return 0;
}

int FlashMgrM::setNVRAMIndex(FLASH_NVRAM_ENUM eNVRAM, MUINT32 a_eNVRAMIndex)
{
    FlashCustomAdapter::getInstance(mSensorDev)->setNVRAMIndex(eNVRAM, a_eNVRAMIndex);
    //TODO: need to remove
    switch (eNVRAM) {
    case FLASH_NVRAM_AE:
        if(a_eNVRAMIndex >= FlASH_AE_NUM_2) {
            logI("setNVRAMIndex(): F_AE Index(%d) exceed defined maximum(%d)", a_eNVRAMIndex, FlASH_AE_NUM_2);
        } else if(mFlashAENVRAMIdx != (MINT32)a_eNVRAMIndex) {
            mFlashAENVRAMIdx = a_eNVRAMIndex;
            logD("setNVRAMIndex(): FLASH_NVRAM_ENUM(%d), Index(%d)", eNVRAM, a_eNVRAMIndex);
        }
        break;
    case FLASH_NVRAM_AWB:
        if(a_eNVRAMIndex >= FlASH_AWB_NUM_2) {
            logI("setNVRAMIndex(): F_AWB Index(%d) exceed defined maximum(%d)", a_eNVRAMIndex, FlASH_AWB_NUM_2);
        } else if(mFlashAWBNVRAMIdx != (MINT32)a_eNVRAMIndex) {
            mFlashAWBNVRAMIdx = a_eNVRAMIndex;
            logD("setNVRAMIndex(): FLASH_NVRAM_ENUM(%d), Index(%d)", eNVRAM, a_eNVRAMIndex);
        }
        break;
    case FLASH_NVRAM_CALIBRATION:
        if(a_eNVRAMIndex >= FlASH_CALIBRATION_NUM_2) {
            logI("setNVRAMIndex(): F_Cali Index(%d) exceed defined maximum(%d)", a_eNVRAMIndex, FlASH_CALIBRATION_NUM_2);
        } else if(mFlashCaliNVRAMIdx != (MINT32)a_eNVRAMIndex) {
            mFlashCaliNVRAMIdx = a_eNVRAMIndex;
            logD("setNVRAMIndex(): FLASH_NVRAM_ENUM(%d), Index(%d)", eNVRAM, a_eNVRAMIndex);
        }
        break;
    default:
        logE("setNVRAMIndex(): invalid FLASH_NVRAM_ENUM(%d).", eNVRAM);
        return MFALSE;
    }

    return 0;
}

int FlashMgrM::setCamMode(int mode)
{
    mCapInfo.appMode = mode;
    logD("setCamMode(): mode(%d).", mCapInfo.appMode);
    return 0;
}

int FlashMgrM::setBitMode(EBitMode_T mode)
{
    mCapInfo.bitMode = mode;
    logI("setBitMode(): mode(%d).", mCapInfo.bitMode);
    return 0;
}

int FlashMgrM::getFlashSpMode()
{
    return mSpMode;
}

int FlashMgrM::getFlashMode()
{
    logI("getFlashMode(): mode(%d).", mCapInfo.flashMode);
    return mCapInfo.flashMode;
}

int FlashMgrM::getFlashFlowType()
{
    if ((mPfPolicy & (0x11 << CFG_FL_PF_CYC)) >> CFG_FL_PF_CYC){
        logI("getFlashFlowType(): new");
        return 1;
    }
    return 0;
}

int FlashMgrM::setFlashMode(int mode)
{
    /* verify arguments */
    if (mode < LIB3A_FLASH_MODE_MIN || mode > LIB3A_FLASH_MODE_MAX) {
        logE("setFlashMode(): invalid flash mode.");
        return FL_ERR_FlashModeNotSupport;
    }
    mCapInfo.flashMode = mode;
    logD("setFlashMode(): mode(%d).", mCapInfo.flashMode);

    return 0;
}

int FlashMgrM::getAfLampMode()
{
    return 0;
}

int FlashMgrM::setMultiCapture(int multiCapture)
{
    mCapInfo.isBurst = multiCapture;
    logI("setMultiCapture(): (%d).", mCapInfo.isBurst);
    return 0;
}

int FlashMgrM::setInCharge(int inCharge)
{
    mCapInfo.inCharge = inCharge;
    logI("setInCharge(): sensorDev(%d), inCharge(%d).",
            mSensorDev, mCapInfo.inCharge);
    return 0;
}

int FlashMgrM::getInCharge()
{
    return mCapInfo.inCharge;
}

int FlashMgrM::isAvailable()
{
    int isAvailable = mCapInfo.inCharge && mCapInfo.useFlashFlow;
    logD("isAvailable(): sensorDev(%d), isAvailable(%d).",
            mSensorDev, isAvailable);
    return isAvailable;
}

int FlashMgrM::isFlashOnCapture()
{
    if (!isAvailable()) {
        logD("isFlashOnCapture(): sensorDev(%d) not available.", mSensorDev);
        return 0;
    }

    logD("isFlashOnCapture(): (%d).", mCapRep.isMainFlash);
    return mCapRep.isMainFlash;
}

int FlashMgrM::setIsFlashOnCapture(int flashOnCapture)
{
    if (!isAvailable()) {
        logI("setIsFlashOnCapture(): sensorDev(%d) not available.", mSensorDev);
        return -1;
    }

    mCapRep.isMainFlash = flashOnCapture;
    logI("setIsFlashOnCapture(): (%d).", mCapRep.isMainFlash);
    return 0;
}

int FlashMgrM::isFlashOnCalibration()
{
    return mIsCalibration;
}

int FlashMgrM::updateFlashState()
{
    /*
     * There are only 2 states in flash state stack:
     *   MTK_FLASH_STATE_FIRED (if on)
     *   MTK_FLASH_STATE_READY (if off)
     *
     * There are 4 states in flash management:
     *   MTK_FLASH_STATE_FIRED   (if FFF)
     *   MTK_FLASH_STATE_READY   (if RRR)
     *   MTK_FLASH_STATE_PARTIAL (if [FRR|FRF|FFR|RFR|RRF|RFF])
     *   MTK_FLASH_STATE_READY (if RRR)
     *
     * There are 5 states in Android:
     *   UNAVAILABLE
     *   CHARGING
     *   READY
     *   FIRED
     *   PARTIAL
     */

    Mutex::Autolock lock(mLock);

    int i;
    for (i = FLASH_STATE_STACK_NUM - 1; i > 0; i--)
        mFlashStateStack[i] = mFlashStateStack[i - 1];
    if (FlashHal::getInstance(mSensorDev)->isFlashOn())
        mFlashStateStack[0] = MTK_FLASH_STATE_FIRED;
    else
        mFlashStateStack[0] = MTK_FLASH_STATE_READY;

    return 0;
}

int FlashMgrM::getFlashState()
{
    Mutex::Autolock lock(mLock);

    int fState = MTK_FLASH_STATE_PARTIAL;
    int isLow = 0;

    if (!mCapInfo.hasHw)
        fState = MTK_FLASH_STATE_UNAVAILABLE;
    else {
        FlashHal::getInstance(mSensorDev)->isLowPower(&isLow);

        if (isLow == 2)
            fState = MTK_FLASH_STATE_CHARGING;
        else if (mFlashStateStack[2] == MTK_FLASH_STATE_READY &&
                mFlashStateStack[1] == mFlashStateStack[2] &&
                mFlashStateStack[0] == mFlashStateStack[1])
            fState = MTK_FLASH_STATE_READY; /* (R,R,R) */
        else if (mFlashStateStack[2] == MTK_FLASH_STATE_FIRED &&
                mFlashStateStack[1] == mFlashStateStack[2] &&
                mFlashStateStack[0] == mFlashStateStack[1])
            fState = MTK_FLASH_STATE_FIRED; /* (F,F,F) */
    }

    logD("getFlashState(): state(%d).", fState);

    return fState;
}

int FlashMgrM::setAeFlashMode(int aeMode, int flashMode)
{
    logD("setAeFlashMode(): ae mode(%d), flash mode(%d).", aeMode, flashMode);

    int fMode = LIB3A_FLASH_MODE_AUTO;

    if (aeMode == MTK_CONTROL_AE_MODE_ON || aeMode == MTK_CONTROL_AE_MODE_OFF)
        if (flashMode == MTK_FLASH_MODE_OFF)
            fMode = LIB3A_FLASH_MODE_FORCE_OFF;
        else if (flashMode == MTK_FLASH_MODE_SINGLE)
            fMode = LIB3A_FLASH_MODE_FORCE_ON;
        else if (flashMode == MTK_FLASH_MODE_TORCH)
            fMode = LIB3A_FLASH_MODE_FORCE_TORCH;
        else {
            logE("setAeFlashMode(): invalid ae mode(%d) and flash mode(%d).",
                    aeMode, flashMode);
            fMode = LIB3A_FLASH_MODE_FORCE_OFF;
        }
    else if (aeMode == MTK_CONTROL_AE_MODE_ON_ALWAYS_FLASH)
        fMode = LIB3A_FLASH_MODE_FORCE_ON;
    else if (aeMode == MTK_CONTROL_AE_MODE_ON_AUTO_FLASH ||
            aeMode == MTK_CONTROL_AE_MODE_ON_AUTO_FLASH_REDEYE) {
        fMode = LIB3A_FLASH_MODE_AUTO;
    } else {
        logE("setAeFlashMode(): invalid ae mode(%d) and flash mode(%d).",
                aeMode, flashMode);
        fMode = LIB3A_FLASH_MODE_FORCE_OFF;
    }

    /* set flash internal mode */
    setFlashMode(fMode);

    return 0;
}

void FlashMgrM::addErr(int err)
{
    mCapRep.errCnt++;
    mCapRep.err3 = mCapRep.err2;
    mCapRep.err2 = mCapRep.err1;
    mCapRep.err1 = err;

    mCapRep.errTime3 = mCapRep.errTime2;
    mCapRep.errTime2 = mCapRep.errTime1;
    mCapRep.errTime1 = getMs();
}

void FlashMgrM::getFaceInformation(MtkCameraFaceMetadata *pFaces)
{
    int faceIndex;

    /*The position order is left, top, right, bottom.*/
    for (faceIndex = 0; faceIndex < mFaceNumber; faceIndex++)
    {
         mFaceInfo[faceIndex].x1     = pFaces->faces[faceIndex].rect[0];
         mFaceInfo[faceIndex].y1     = pFaces->faces[faceIndex].rect[1];
         mFaceInfo[faceIndex].x2     = pFaces->faces[faceIndex].rect[2];
         mFaceInfo[faceIndex].y2     = pFaces->faces[faceIndex].rect[3];
         mFaceInfo[faceIndex].weight = (float)((pFaces->faces[faceIndex].rect[2] - pFaces->faces[faceIndex].rect[0]) *
                                       (pFaces->faces[faceIndex].rect[3] - pFaces->faces[faceIndex].rect[1]) * 100) / (mWidth * mHeight);
    }
}

void FlashMgrM::setFaceFlashAlgoInformation(FlashAlgFacePos *pFaceInfo)
{

    int faceNumber;
    int faceIndex;

    /*Clamp face number less than 9*/
    faceNumber = mFaceNumber;

    logD("Face Number(%d)", faceNumber);

    if (faceNumber > 9) faceNumber = 9;

    pFaceInfo->cnt = faceNumber;

   /*The position order is left, top, right, bottom.*/
    for (faceIndex = 0; faceIndex < mFaceNumber; faceIndex++)
    {
         mFaceInfo[faceIndex].x1     = (mFaceInfo[faceIndex].x1 * 24 + (mWidth  >>1)) / mWidth;
         mFaceInfo[faceIndex].y1     = (mFaceInfo[faceIndex].y1 * 18 + (mHeight >>1)) / mHeight;
         mFaceInfo[faceIndex].x2     = (mFaceInfo[faceIndex].x2 * 24 + (mWidth  >>1)) / mWidth;
         mFaceInfo[faceIndex].y2     = (mFaceInfo[faceIndex].y2 * 18 + (mHeight >>1)) / mHeight;

        /*For avoiding converted coordinate overpass boundary*/
        if (mFaceInfo[faceIndex].x1 < 0)  mFaceInfo[faceIndex].x1 = 0;
        if (mFaceInfo[faceIndex].y1 < 0)  mFaceInfo[faceIndex].y1 = 0;
        if (mFaceInfo[faceIndex].x2 > 24) mFaceInfo[faceIndex].x2 = 24;
        if (mFaceInfo[faceIndex].y2 > 18) mFaceInfo[faceIndex].y2 = 18;

        logD("The converted face information (Oringal and unsorted data) ==> Face[%d], Position(Left, Top, Right, Bottom) = (%d, %d, %d, %d), Weighting = %f",
                 faceIndex + 1,
                 mFaceInfo[faceIndex].x1,
                 mFaceInfo[faceIndex].y1,
                 mFaceInfo[faceIndex].x2,
                 mFaceInfo[faceIndex].y2,
                 mFaceInfo[faceIndex].weight);
    }

    /*Sort faces information based on weighting*/
    faceInfoSort();

    /*Assign sorted information to algo struct*/
    for (faceIndex = 0; faceIndex < faceNumber; faceIndex++)
    {
        pFaceInfo->block[faceIndex].facePosition.x1 = mFaceInfo[faceIndex].x1;
        pFaceInfo->block[faceIndex].facePosition.y1 = mFaceInfo[faceIndex].y1;
        pFaceInfo->block[faceIndex].facePosition.x2 = mFaceInfo[faceIndex].x2;
        pFaceInfo->block[faceIndex].facePosition.y2 = mFaceInfo[faceIndex].y2;
        pFaceInfo->block[faceIndex].weight          = mFaceInfo[faceIndex].weight;
        pFaceInfo->block[faceIndex].num             = 0;
        pFaceInfo->block[faceIndex].sum             = 0;
        pFaceInfo->block[faceIndex].mean            = 0;
        pFaceInfo->block[faceIndex].inValid         = 0;

        logD("The converted face information (sorted data)==> Face[%d], Position(Left, Top, Right, Bottom) = (%d, %d, %d, %d), Weighting = %f",
                 faceIndex + 1,
                 pFaceInfo->block[faceIndex].facePosition.x1,
                 pFaceInfo->block[faceIndex].facePosition.y1,
                 pFaceInfo->block[faceIndex].facePosition.x2,
                 pFaceInfo->block[faceIndex].facePosition.y2,
                 pFaceInfo->block[faceIndex].weight);
    }
}

void FlashMgrM::faceInfoSort()
{
    int i, j;
    for (i = 0; i < mFaceNumber; i++)
    {
        for (j = i + 1; j < mFaceNumber; j++)
        {
            if (mFaceInfo[i].weight < mFaceInfo[j].weight)
            {
                FaceInformation temp;
                temp = mFaceInfo[i];
                mFaceInfo[i] = mFaceInfo[j];
                mFaceInfo[j] = temp;
            }
        }
    }
}

void FlashMgrM::coordinateConverter()
{
    int faceIndex;

    mConvertedFacesPos[0] = mFaceNumber;
    for (faceIndex = 0; faceIndex < mFaceNumber; faceIndex++)
    {
        mConvertedFacesPos[4 * faceIndex + 1] = (mFaceInfo[faceIndex].x1 * 120) / mWidth;
        mConvertedFacesPos[4 * faceIndex + 2] = (mFaceInfo[faceIndex].y1 * 90)  / mHeight;
        mConvertedFacesPos[4 * faceIndex + 3] = (mFaceInfo[faceIndex].x2 * 120) / mWidth;
        mConvertedFacesPos[4 * faceIndex + 4] = (mFaceInfo[faceIndex].y2 * 90)  / mHeight;
    }
}

int FlashMgrM::setFDInfo(void * a_sFaces, int i4tgwidth, int i4tgheight)
{
    mWidth    = i4tgwidth;
    mHeight   = i4tgheight;

    MtkCameraFaceMetadata *pFaces = (MtkCameraFaceMetadata *) a_sFaces;

    if (getFlashState() == MTK_FLASH_STATE_FIRED && mCapRep.pfState == FLASH_PF_STATE_RUN
        && pFaces->number_of_faces > 0 && pFaces->number_of_faces <= FACE_MAX_NUM)
    {
        mFaceNumber= pFaces->number_of_faces;
        getFaceInformation(pFaces);
        coordinateConverter();

        return 1;
    }

    return 0;
}

int FlashMgrM::isHighBrightnessFlow()
{
    return mPfHighBrightness;
}

void FlashMgrM::setPfHighBrightnessFlow(int isHighBrightness)
{
    mPfHighBrightness = isHighBrightness;
}

/* Need to update debug parser if modify. */
int FlashMgrM::getDebugInfo(FLASH_DEBUG_INFO_T *pDebugInfo)
{
    /* verify arguments */
    if (!pDebugInfo) {
        logE("getDebugInfo(): invalid arguments.");
        return -1;
    }

    /* clear flash debug info */
    memset(pDebugInfo, 0, sizeof(FLASH_DEBUG_INFO_T));

    /* get flash hal time info */
    FlashHalTimeInfo flashHalTimeInfo = FlashHal::getInstance(mSensorDev)->getTimeInfo();
    int fault = FlashHal::getInstance(mSensorDev)->getDriverFault();

    /* set flash debug info */
    setDebugTag(*pDebugInfo, FL_T_VERSION, (((MUINT32)FLASH_DEBUG_TAG_SUBVERSION << 16) | FLASH_DEBUG_TAG_VERSION));
    setDebugTag(*pDebugInfo, FL_T_IS_FLASH_ON, (MUINT32) mCapRep.isMainFlash);
    setDebugTag(*pDebugInfo, FL_T_DUTY, (MUINT32) mCapRep.capDuty);
    setDebugTag(*pDebugInfo, FL_T_DUTY_LT, (MUINT32) mCapRep.capDutyLT);
    setDebugTag(*pDebugInfo, FL_T_EXP_TIME, (MUINT32) mCapRep.capExp);
    setDebugTag(*pDebugInfo, FL_T_ISO, (MUINT32) mCapRep.capIso);
    setDebugTag(*pDebugInfo, FL_T_AFE_GAIN, (MUINT32) mCapRep.capAfeGain);
    setDebugTag(*pDebugInfo, FL_T_ISP_GAIN, (MUINT32) mCapRep.capIspGain);
    setDebugTag(*pDebugInfo, FL_T_DUTY_NUM, (MUINT32) mCapRep.dutyNum);
    setDebugTag(*pDebugInfo, FL_T_DUTY_LT_NUM, (MUINT32) mCapRep.dutyNumLT);
    setDebugTag(*pDebugInfo, FL_T_FLASH_MODE, (MUINT32) mCapInfo.flashMode);
    setDebugTag(*pDebugInfo, FL_T_SCENE_MODE, (MUINT32) mCapInfo.aeScene);
    setDebugTag(*pDebugInfo, FL_T_EV_COMP, (MUINT32) mCapInfo.evComp);
    setDebugTag(*pDebugInfo, FL_T_DIG_ZOOM_100, (MUINT32) (mCapInfo.digRatio * 100 + 0.5));
    setDebugTag(*pDebugInfo, FL_T_IS_BURST, (MUINT32) mCapInfo.isBurst);
    setDebugTag(*pDebugInfo, FL_T_VBAT, (MUINT32) mCapInfo.vBat);
    setDebugTag(*pDebugInfo, FL_T_IS_BATLOW_TIMEOUT, (MUINT32) mCapInfo.isBatLow * 10000 + flashHalTimeInfo.mfIsTimeout);
    setDebugTag(*pDebugInfo, FL_T_PF_START, (MUINT32) mCapRep.pfStartTime);
    setDebugTag(*pDebugInfo, FL_T_PF_RUN, (MUINT32) mCapRep.pfRunTime);
    setDebugTag(*pDebugInfo, FL_T_PF_END, (MUINT32) mCapRep.pfEndTime);
    setDebugTag(*pDebugInfo, FL_T_MF_START, (MUINT32) flashHalTimeInfo.mfStartTime);
    setDebugTag(*pDebugInfo, FL_T_MF_END, (MUINT32) flashHalTimeInfo.mfEndTime);
    setDebugTag(*pDebugInfo, FL_T_TIME_OUT, (MUINT32) flashHalTimeInfo.mfTimeout);
    setDebugTag(*pDebugInfo, FL_T_TIME_OUT_LT, (MUINT32) flashHalTimeInfo.mfTimeoutLt);
    setDebugTag(*pDebugInfo, FL_T_PF_EPOCH_FRAMES, (MUINT32) mCapRep.pfIteration * 10000 + mCapRep.pfRunFrms);
    setDebugTag(*pDebugInfo, FL_T_ERR_NUM, (MUINT32) mCapRep.errCnt);
    setDebugTag(*pDebugInfo, FL_T_ERR1, (MUINT32) mCapRep.err1);
    setDebugTag(*pDebugInfo, FL_T_ERR2, (MUINT32) mCapRep.err2);
    setDebugTag(*pDebugInfo, FL_T_ERR3, (MUINT32) mCapRep.err3 * 1000 + mDebugSceneCnt);
    setDebugTag(*pDebugInfo, FL_T_ERR1_TIME, (MUINT32) mCapRep.errTime1);
    setDebugTag(*pDebugInfo, FL_T_ERR2_TIME, (MUINT32) mCapRep.errTime2);
    setDebugTag(*pDebugInfo, FL_T_ERR3_TIME, (MUINT32) mCapRep.errTime3);
    setDebugTag(*pDebugInfo, FL_T_DRIVER_FAULT, (MUINT32) fault);


    /* set flash algo debug data */
    /*if (mCapRep.isMainFlash == 1) {
        int algoDebugData[ALG_TAG_SIZE / 4];
        memset(algoDebugData, 0, sizeof(algoDebugData));

        if (mCapRep.pfAlgoDone) {
            logD("getDebugInfo(): get algo debug data.");
            mpFlashAlg->fillDebugData2(algoDebugData);
        }

        int i;
        for (i = 0; i < ALG_TAG_SIZE / 4; i++)
            setDebugTag(*pDebugInfo, FL_T_NUM + i, (MUINT32)algoDebugData[i]);
    }*/
// FIX-ME, for IP-Base build pass +
#if (!CAM3_3A_LEGACY_EN)
// FIX-ME, for IP-Base build pass -
	if (mCapRep.isMainFlash == 1) {
        if (mCapRep.pfAlgoDone) {
            logD("getDbgData(): get algo debug data.");
            FlashAlgoAdapter::getInstance(mSensorDev)->getDbgData(pDebugInfo);
        }
	}
#endif
    return 0;
}

void FlashMgrM::dumpDebugData(
        void *buf, int w, int h, EBitMode_T mode,
        int sceneCount, int frameCount)
{
    /* dump AAO to bmp */
    int pfBmpEn = 0;
    getPropInt(PROP_FLASH_PF_BMP_EN, &pfBmpEn, 0);
    if (pfBmpEn) {
        char aeFile[128];
        char awbFile[128];

        snprintf(aeFile, sizeof(aeFile), "%s/pf_ae_%03d_%02d.bmp",
                FLASH_BMP_DATA_PATH, sceneCount, frameCount);
        snprintf(awbFile, sizeof(awbFile), "%s/pf_awb_%03d_%02d.bmp",
                FLASH_BMP_DATA_PATH, sceneCount, frameCount);
        convertAaSttToYrgbBmp(buf, w, h, mode, aeFile, awbFile, mLinearOutputEn);

        if (FlashCustomAdapter::getInstance(mSensorDev)->getFaceFlashSupport()){
            if (mFaceNumber > 0 && mFaceNumber <= FACE_MAX_NUM){
                char aeFacePosFile[128];

                snprintf(aeFacePosFile, sizeof(aeFile), "%s/pf_FacePos_%03d_%02d.bmp",
                         FLASH_BMP_DATA_PATH, sceneCount, frameCount);
                convertAaSttToYBmp(buf, w, h, mode, aeFacePosFile, mConvertedFacesPos);
            }
        }
    }
}

void FlashMgrM::setDebugDataPath()
{
    int pfBmpEn = 0;
    int mfBmpEn = 0;
    int binEn = 0;
    getPropInt(PROP_FLASH_PF_BMP_EN, &pfBmpEn, 0);
    getPropInt(PROP_FLASH_MF_BMP_EN, &mfBmpEn, 0);
    getPropInt(PROP_FLASH_BIN_EN, &binEn, 0);
    logD("setDebugDataPath: pfBmpEn(%d), mfBmpEn(%d), binEn(%d)", pfBmpEn, mfBmpEn, binEn);

    if (pfBmpEn || mfBmpEn || binEn) {
        getFileCount(FLASH_FILE_COUNT_PATH, &mDebugSceneCnt, 0);
        setFileCount(FLASH_FILE_COUNT_PATH, mDebugSceneCnt + 1);
    }
    if (pfBmpEn || mfBmpEn) {
        int ret;

        ret = createDir(FLASH_DATA_PATH);
        if (ret != 0) {
            logE("create dir (%s) failed: %s", FLASH_DATA_PATH, strerror(errno));
        }
        ret = createDir(FLASH_BMP_DATA_PATH);
        if (ret != 0) {
            logE("create dir (%s) failed: %s", FLASH_AAO_DATA_PATH, strerror(errno));
        }
    }
    if (binEn) {
        char prjName[8];
        FlashAlgoAdapter::getInstance(mSensorDev)->setIsSaveSimBinFile(1);
        snprintf(prjName, sizeof(prjName), "%03d", mDebugSceneCnt);
        FlashAlgoAdapter::getInstance(mSensorDev)->setDebugDir(FLASH_DATA_PATH, prjName);
    }
}

/***********************************************************
 * Flash algo
 **********************************************************/
void FlashMgrM::hw_setCapPline(FLASH_PROJECT_PARA *pPrjPara)
{
    logD("hw_setCapPline(): max capture exposure time (%d us).", pPrjPara->maxCapExpTimeUs);
    logD("hw_setCapPline(): flicker mode (%d).", mCapInfo.flickerMode);

    /* get pline from AE management */
    strAETable prvAePline = {};
    strAETable capAePline = {};
    strAETable strobeAePline = {};
    strAFPlineInfo afPlineInfo = {};
    AeMgr::getInstance(mSensorDev).getCurrentPlineTable(prvAePline, capAePline, strobeAePline, afPlineInfo);

    /* convert ae pline to flash algo pline */
    PLine capPline;
    FlashPline fPline;
    int propUseStrobePline = 0;
    getPropInt(PROP_FLASH_USE_STROBE_PLINE, &propUseStrobePline, -1);
    if (propUseStrobePline == 0 ||
            (propUseStrobePline == -1 && pPrjPara->tuningPara.isFollowCapPline == 1)) {
        fPline.convertAePlineToPline(&capPline, &capAePline, pPrjPara->maxCapExpTimeUs, mCapInfo.flickerMode);
        logI("hw_setCapPline(): use capture pline.");
    } else {
        fPline.convertAePlineToPline(&capPline, &strobeAePline, pPrjPara->maxCapExpTimeUs, mCapInfo.flickerMode);
        logI("hw_setCapPline(): use strobe pline.");
    }
#if 0
    FlashPlineTool::dumpAePline(FLASH_AE_CAPTURE_PLINE_PATH, &capAePline);
    FlashPlineTool::dumpAePline(FLASH_AE_STROBE_PLINE_PATH, &strobeAePline);
    FlashPlineTool::dumpPline(FLASH_FLASH_ALGO_CAPTURE_PLINE_PATH, &capPline);
#endif

    /* get gain ratio of capture to preview */
    AE_DEVICES_INFO_T aeDevInfo = {};
    AeMgr::getInstance(mSensorDev).getSensorDeviceInfo(aeDevInfo);

    int cap2PreRatio;
    if (mCapInfo.appMode == eAppMode_ZsdMode)
        cap2PreRatio = 1024;
    else
        cap2PreRatio = aeDevInfo.u4Cap2PreRatio;

    /* set capture pline and gain ratio to algo */
    FlashAlgoAdapter::getInstance(mSensorDev)->setCapturePLine(&capPline, 
        aeDevInfo.u4MiniISOGain * cap2PreRatio / 1024);
    logI("hw_setCapPline(): u4MiniISOGain(%d), u4Cap2PreRatio(%d), cap2PreRatio(%d).",
            aeDevInfo.u4MiniISOGain, aeDevInfo.u4Cap2PreRatio, cap2PreRatio);
}

void FlashMgrM::hw_setPfPline()
{
    logD("hw_setPfPline(): flicker mode (%d).", mCapInfo.flickerMode);

    /* get pline from AE management */
    strAETable prvAePline = {};
    strAETable capAePline = {};
    strAETable strobeAePline = {};
    strAFPlineInfo afPlineInfo = {};
    AeMgr::getInstance(mSensorDev).getCurrentPlineTable(prvAePline, capAePline, strobeAePline, afPlineInfo);

    /* convert ae pline to flash algo pline */
    PLine pfPline;
    FlashPline fPline;
    fPline.convertAePlineToPline(&pfPline, &prvAePline, 0, mCapInfo.flickerMode);

    /* get AE device info */
    AE_DEVICES_INFO_T aeDevInfo = {};
    AeMgr::getInstance(mSensorDev).getSensorDeviceInfo(aeDevInfo);

    /* set pre-capture pline and gain to algo */
    FlashAlgoAdapter::getInstance(mSensorDev)->setPreflashPLine(&pfPline, aeDevInfo.u4MiniISOGain);
    logI("hw_setPfPline(): u4MiniISOGain(%d).", aeDevInfo.u4MiniISOGain);
}

void FlashMgrM::hw_setFlashProfile()
{
    logD("hw_setFlashProfile().");

    FlashAlgStrobeProfile pf;

    /* profile duty */
    int i, j;
    int dutyN;
    if (cust_isDualFlashSupport(mFacingSensor)) {
        dutyN = (mDutyNum + 1) * (mDutyNumLT + 1);
        logI("hw_setFlashProfile(): total duty num(%d), ht num(%d), lt num(%d).",
                dutyN, mDutyNum, mDutyNumLT);
    } else {
        dutyN = mDutyNum;
        logI("hw_setFlashProfile(): total duty num(%d).", dutyN);
    }

    /* verify parameter */
    if (dutyN <= 0) {
        logE("hw_setFlashProfile(): invalid total duty num(%d).", dutyN);
        return ;
    }

    pf.dutyNum = dutyN;
    pf.dutyTickNum = dutyN;
    int *dutyTick = new int[pf.dutyNum];
    if (!dutyTick)
        logE("failed to allocate memory.");
    else
        for (i = 0; i < pf.dutyNum; i++)
            dutyTick[i] = i;
    pf.dutyTick = dutyTick;

    /* profile step */
    pf.stepNum = 1;
    pf.stepTickNum = 1;
    int *stepTick = new int[pf.stepNum];
    if (!stepTick)
        logE("failed to allocate memory.");
    else
        for (i = 0; i < pf.stepNum; i++)
            stepTick[i] = i;
    pf.stepTick = stepTick;

    /* profile energy table */
    float *engTable = new float[pf.dutyNum];
    if (!engTable)
        logE("failed to allocate memory.");
    else {
        /* get available duty range */
        int dutyMin;
        int dutyMax;
        int dutyMinLt;
        int dutyMaxLt;
        dutyMin = FlashCustomAdapter::getInstance(mSensorDev)->getMfDutyMin(&mCapInfo);
        dutyMax = FlashCustomAdapter::getInstance(mSensorDev)->getMfDutyMax(&mCapInfo);
        dutyMinLt = FlashCustomAdapter::getInstance(mSensorDev)->getMfDutyMinLt(&mCapInfo);
        dutyMaxLt = FlashCustomAdapter::getInstance(mSensorDev)->getMfDutyMaxLt(&mCapInfo);

        /* clear energy table */
        for (i = 0; i < pf.dutyNum; i++)
            engTable[i] = -1;

        if (cust_isDualFlashSupport(mFacingSensor)) {
            int dutyIndex = 0;
            int mPropFlashMfOn = 0;
            getPropInt(PROP_FLASH_MF_ON, &mPropFlashMfOn, -1);

            if ((mPropFlashMfOn == -1 || !mPropFlashMfOn) && !mIsManualFlashEnCct) {
                /* crop by duty range */
                for (j = dutyMinLt; j <= dutyMaxLt; j++)
                    for (i = dutyMin; i <= dutyMax; i++) {
                        FlashDuty::duty2ind(dutyIndex, i, j, mDutyNum, mDutyNumLT, 1);
                        engTable[dutyIndex] = FlashCustomAdapter::getInstance(mSensorDev)->getYTabByDutyIndex(dutyIndex);
                    }
            } else {
                getPropInt(PROP_FLASH_MF_DUTY, &mPropFlashMfDuty, -1);
                getPropInt(PROP_FLASH_MF_DUTYLT, &mPropFlashMfDutyLt, -1);
                int duty = mPropFlashMfDuty;
                int dutyLt = mPropFlashMfDutyLt;
                if (mIsManualFlashEnCct) {
                    duty = mManualDutyCct;
                    dutyLt = mManualDutyLtCct;
                }
                FlashDuty::duty2ind(dutyIndex, duty, dutyLt, mDutyNum, mDutyNumLT, 1);
                engTable[dutyIndex] = FlashCustomAdapter::getInstance(mSensorDev)->getYTabByDutyIndex(dutyIndex);
                if (engTable[dutyIndex] == -1)
                    engTable[dutyIndex] = 100;
            }
        } else {
            int mPropFlashMfOn = 0;

            /* verify min duty and max duty */
            if (dutyMin < 0 || dutyMin > pf.dutyNum - 1) {
                logE("hw_setFlashProfile(): dutyMin(%d) is out of bound.", dutyMin);
                dutyMin = 0;
            }
            if (dutyMax < 0 || dutyMax > pf.dutyNum - 1) {
                logE("hw_setFlashProfile(): dutyMax(%d) is out of bound.", dutyMax);
                dutyMax = pf.dutyNum - 1;
            }

            getPropInt(PROP_FLASH_MF_ON, &mPropFlashMfOn, -1);

            if ((mPropFlashMfOn == -1 || !mPropFlashMfOn) && !mIsManualFlashEnCct) {
                /* copy energy table */
                for (i = dutyMin; i <= dutyMax; i++) {
                    engTable[i] = FlashCustomAdapter::getInstance(mSensorDev)->getYTabByDutyIndex(i);
                }
            } else {
                getPropInt(PROP_FLASH_MF_DUTY, &mPropFlashMfDuty, -1);
                int duty = mPropFlashMfDuty;
                if (mIsManualFlashEnCct) {
                    duty = mManualDutyCct;
                }
                engTable[duty] = FlashCustomAdapter::getInstance(mSensorDev)->getYTabByDutyIndex(duty);
                if (engTable[duty] == -1)
                    engTable[duty] = 100;
            }
        }
    }
    pf.engTab = engTable;

    /* profile pre-flash energy */
    pf.pfEng = FlashCustomAdapter::getInstance(mSensorDev)->getPfEng();
    m_i4EngPreflash = pf.pfEng;

    /* profile flash AWB gain */
    pf.pFlWbCalTbl = FlashCustomAdapter::getInstance(mSensorDev)->getFlashWBGain();

    /*
     * Set flashlight profile to algo
     * Note that, the function is NOT efficient because it copy again.
     */
    FlashAlgoAdapter::getInstance(mSensorDev)->setFlashProfile(&pf);

    /* free memory */
    if (dutyTick)
        delete [] dutyTick;
    if (stepTick)
        delete [] stepTick;
    if (engTable)
        delete [] engTable;
}

void FlashMgrM::hw_setPreference(FLASH_PROJECT_PARA *pPrjPara)
{
    int lv = AeMgr::getInstance(mSensorDev).getLVvalue(MTRUE);

    /* set preference */
    FlashAlgoAdapter::getInstance(mSensorDev)->setDefaultPreferences();
    FlashAlgoAdapter::getInstance(mSensorDev)->setTuningPreferences(&pPrjPara->tuningPara, lv);

    /* set EV compensation */
    int maxEvTar;
    int indexNum;
    float *evIndex;
    float *evComp;
    float *evLevel;
    cust_getEvCompPara(maxEvTar, indexNum, evIndex, evComp, evLevel);

    float evCompRet;
    float evLevelRet;
    evCompRet = interpTable(indexNum, evIndex, evComp, mCapInfo.evComp);
    evLevelRet = interpTable(indexNum, evIndex, evLevel, mCapInfo.evComp);
    FlashAlgoAdapter::getInstance(mSensorDev)->setEVCompEx(evCompRet, maxEvTar, evLevelRet);

//    /* set debug data size */
    logD("hw_setPreference(): evIndex(%d), evComp(%d), evLevel(%d).",
            (int)(mCapInfo.evComp * 100), (int)(evCompRet * 100), (int)(evLevelRet * 100));
}

void FlashMgrM::hw_setPfExp(FlashAlgExpPara *expPara)
{
    logD("hw_setPfExp(): isFlash(%d), duty index(%d), exp(%d), iso(%d).",
            expPara->isFlash, expPara->duty, expPara->exp, expPara->iso);

    int propFollowPline = 0;
    int propMaxAfe = 0;
    getPropInt(PROP_FLASH_PF_FOLLOW_PLINE, &propFollowPline, -1);
    getPropInt(PROP_FLASH_PF_AFE_MAX, &propMaxAfe, -1);

    if (propFollowPline == -1) {
        /* default enable speed up exposure */
        strAETable prvAePline = {};
        strAETable capAePline = {};
        strAETable strobeAePline = {};
        strAFPlineInfo afPlineInfo = {};
        AeMgr::getInstance(mSensorDev).getCurrentPlineTable(
                prvAePline, capAePline, strobeAePline, afPlineInfo);

        logI("hw_setPfExp(): afPlineInfo enable(%d).", (int)afPlineInfo.bAFPlineEnable);
        if (afPlineInfo.bAFPlineEnable)
            hw_speedUpExpPara(expPara, 0);

    } else if (!propFollowPline) {
        if (propMaxAfe == -1)
            hw_speedUpExpPara(expPara, 0);
        else
            hw_speedUpExpPara(expPara, propMaxAfe);
    }

    /* get exposure parameters */
    int exp = expPara->exp;
    int iso = expPara->iso;
    int afe;
    int isp;
    isoToGain(iso, &afe, &isp, 0, mCapInfo.appMode);

    logD("hw_setPfExp(): adjusted isFlash(%d), duty index(%d), exp(%d), iso(%d), afe(%d), isp(%d).",
            expPara->isFlash, expPara->duty, exp, iso, afe, isp);

    /* set exp and afe to sensor */
    int err;
    err = AAASensorMgr::getInstance().setSensorExpTime(mSensorDev, exp);
    if (FAILED(err))
        return;
    err = AAASensorMgr::getInstance().setSensorGain(mSensorDev, afe);
    if (FAILED(err))
        return;

    /* set AE pre-flash info and set isp gain */
//    AeMgr::getInstance(mSensorDev).setPfPara(exp, afe, isp);

    /* update AE preview parameters */
    updateAeParams(exp, afe, isp, 0);
}

void FlashMgrM::hw_setCapExp(FlashAlgExpPara *expPara)
{
    logD("hw_setCapExp(): isFlash(%d), duty index(%d), exp(%d), iso(%d).",
            expPara->isFlash, expPara->duty, expPara->exp, expPara->iso);

    /* get exposure parameters */
    int exp = expPara->exp;
    int iso = expPara->iso;
    int afe;
    int isp;

    if (isManualFlashFlow()){
    logI("hw_setCapExp(): M+(%d)/(%d).%d/%d",mCapInfo.manualExposureTime, mCapInfo.manualSensitivity, exp, iso);
        switchManualExposureSetting(&exp, &iso, mCapInfo.manualExposureTime, mCapInfo.manualSensitivity);
    logI("hw_setCapExp(): M-(%d)/(%d).%d/%d",mCapInfo.manualExposureTime, mCapInfo.manualSensitivity, exp, iso);
    }

    isoToGain(iso, &afe, &isp, 1, mCapInfo.appMode);

    /* get exposure parameters from property */
    int propExp = 0;
    int propIso = 0;
    int propAfe = 0;
    int propIsp = 0;
    getPropInt(PROP_FLASH_MF_EXP, &propExp, -1);
    getPropInt(PROP_FLASH_MF_ISO, &propIso, -1);
    getPropInt(PROP_FLASH_MF_AFE, &propAfe, -1);
    getPropInt(PROP_FLASH_MF_ISP, &propIsp, -1);
    if (propExp != -1)
        exp = propExp;
    if (propIso != -1) {
        iso = propIso;
        isoToGain(iso, &afe, &isp, 1, mCapInfo.appMode);
    }
    if (propAfe != -1)
        afe = propAfe;
    if (propIsp != -1)
        isp = propIsp;

    logD("hw_setCapExp(): adjusted isFlash(%d), duty index(%d), exp(%d), iso(%d), afe(%d), isp(%d).",
            expPara->isFlash, expPara->duty, exp, iso, afe, isp);

    /* update AE capture parameters */
    updateAeParams(exp, afe, isp, 1);
}

void FlashMgrM::hw_speedUpExpPara(FlashAlgExpPara *expPara, int maxAfe)
{
    /* get max iso */
    int maxIsp = 15 * mIspGainBase;
    int maxIso;
    AE_DEVICES_INFO_T aeDevInfo = {};
    AeMgr::getInstance(mSensorDev).getSensorDeviceInfo(aeDevInfo);

    if (maxAfe)
        gainToIso(maxAfe, maxIsp, &maxIso, 0, mCapInfo.appMode);
    else
        gainToIso(aeDevInfo.u4MaxGain, maxIsp, &maxIso, 0, mCapInfo.appMode);

    logD("hw_speedUpExpPara(): maxAfe(%d), maxIsp(%d), maxIso(%d).",
            maxAfe, maxIsp, maxIso);

    /* get LV table and FPS table */
    strAETable prvAePline = {};
    strAETable capAePline = {};
    strAETable strobeAePline = {};
    strAFPlineInfo afPlineInfo = {};
    AeMgr::getInstance(mSensorDev).getCurrentPlineTable(prvAePline, capAePline, strobeAePline, afPlineInfo);

    int i;
    int lvTab[5];
    int fpsTab[5];
    for (i = 0; i < 5; i++) {
        lvTab[i] = afPlineInfo.i2FrameRate[i][0];
        fpsTab[i] = afPlineInfo.i2FrameRate[i][1];
        logD("hw_speedUpExpPara(): afPlineInfo %d(%d,%d).",
                i, lvTab[i], fpsTab[i]);
    }
    flash_sortxy_xinc(5, lvTab, fpsTab);

    /* get reduced exposure time */
    int lv = AeMgr::getInstance(mSensorDev).getLVvalue(MTRUE);
    int reducedExp = 1000000 / interpTable(5, lvTab, fpsTab, lv);
    logD("hw_speedUpExpPara(): lv(%d), reducedExp(%d).", lv, reducedExp);

    /* get compensation gain */
    float maxG = (float)maxIso * 0.95 / expPara->iso;
    float g = (float)expPara->exp / reducedExp;
    g = min(g, maxG);
    logI("hw_speedUpExpPara(): exp(%d), iso(%d), g(%f), maxG(%f).",
            expPara->exp, expPara->iso, g, maxG);

    /* give up if not support compensation gain */
    if (g <= 1)
        return;

    /* get new exposure time */
    int expNew = reducedExp;
    double align;
    if (mCapInfo.flickerMode == HAL_FLICKER_AUTO_60HZ)
        align = 1000000 / 120.0;
    else
        align = 1000000 / 100.0;
    int expNew1 = (int)(expNew / align) * align;
    int expNew2 = ((int)(expNew / align) + 1) * align;
    float g1 = 0;
    float g2 = 0;
    if (expNew1)
        g1 = (float)expPara->exp / expNew1;
    if (expNew2)
        g2 = (float)expPara->exp / expNew2;

    if (expNew2 > expPara->exp) {
        /* consider exposure 1 */
        if (!expNew1 || g1 > maxG)
            expNew = expPara->exp;
        else
            expNew = expNew1;
    } else {
        if (g1 > maxG && g2 > maxG)
            expNew = expPara->exp;
        else if (g1 > maxG && g2 <= maxG)
            expNew = expNew2;
        else if (expNew1 == expNew)
            expNew = expNew1;
        else
            expNew = expNew2;
    }

    /* compensate iso to reduce exposure time */
    if (expNew) {
        expPara->iso = expPara->iso * expPara->exp / (float)expNew;
        expPara->exp = expNew;
    }
    logD("hw_speedUpExpPara(): new exp(%d), new iso(%d).",
            expPara->exp, expPara->iso);
}

int FlashMgrM::hw_convertAaStt(FlashAlgStaData *staData, void *staBuf, int w, int h)
{
    /* verify arguments */
    if (!staData || !staBuf || w <= 0 || h <= 0) {
        logE("hw_convertAaStt(): invalid arguments.");
        return -1;
    }

    /* get y table from AAO */
    short *y = new short[w * h];
    if (!y) {
        logE("hw_convertAaStt(): failed to allocate memory.");
        return -1;
    }

    logI("hw_convertAaStt(): w(%d), h(%d)", w, h);

    convertAaSttToY(staBuf, w, h, y, 4, mCapInfo.bitMode);

    /* resize */
    int rzW = 0;
    int rzH = 0;
    short *pData = staData->data;
    resizeWindow(mCapInfo.digRatio, y, w, h,
            FLASH_AAO_Y_RESIZE_WIDTH, FLASH_AAO_Y_RESIZE_HEIGHT,
            pData, &rzW, &rzH);
    staData->col = rzW;
    staData->row = rzH;
    staData->bit = FLASH_AAO_Y_BITNUM;
    staData->normalizeFactor = FLASH_AAO_Y_NORMALIZE_FACTOR;
    staData->dig_row = FLASH_AAO_Y_DIGITAL_ZOOM_WIDTH;
    staData->dig_col = FLASH_AAO_Y_DIGITAL_ZOOM_HEIGHT;

    logI("hw_convertAaStt(): digRatio(%lf), rzW(%d), rzH(%d).",
            (double)mCapInfo.digRatio, rzW, rzH);

    delete [] y;

    return 0;
}


/***********************************************************
 * Preview and pre-Flash
 **********************************************************/
void FlashMgrM::updateCapInfo(FlashExePara *in)
{
    mCapInfo.aeScene = AeMgr::getInstance(mSensorDev).getAEScene();

    mCapInfo.vBat = 4500;
    if (FlashHal::getInstance(mSensorDev)->getBattVol(&mCapInfo.vBat))
        logE("updateCapInfo(): failed to get vbat.");

    mCapInfo.flickerMode = in->flickerMode;

    int isLow = 0;
    FlashHal::getInstance(mSensorDev)->isLowPower(&isLow);
    if (isLow == 2)
        mCapInfo.isBatLow = 2;
    else if (isLow || FlashCustomAdapter::getInstance(mSensorDev)->isVBatLow(&mCapInfo))
        mCapInfo.isBatLow = 1;

    mCapInfo.isChargerReady = 1;
    FlashHal::getInstance(mSensorDev)->isChargerReady(&mCapInfo.isChargerReady);
}

int FlashMgrM::isNeedFiringFlash()
{
    int fMode = getFlashMode();
    int fStyle = FlashCustomAdapter::getInstance(mSensorDev)->getFlashModeStyle(mSensorDev, fMode);

    /* set flash
     *   1. main flash on/off
     *   2. pre-flash state machine run or not
     */
    mCapRep.isPreFlash = 0;
    mCapRep.isMainFlash = 0;;

    /* no flash hardware */
    if (!mCapInfo.useFlashFlow) {
        logD("isNeedFiringFlash(): has no flash hardware.");
        goto set_data_and_exit;
    }

    /* low power */
    if (mCapInfo.isBatLow == 2) {
        logD("isNeedFiringFlash(): low power.");
        goto set_data_and_exit;
    }

    /* flash style */
    if (fStyle == e_FLASH_STYLE_OFF_OFF)
        logD("isNeedFiringFlash(): XX.");
    else if (fStyle == e_FLASH_STYLE_OFF_ON || fStyle == e_FLASH_STYLE_ON_ON) {
        logD("isNeedFiringFlash(): XO OO.");
        mCapRep.isMainFlash = 1;
        mCapRep.isPreFlash = 1;
    } else if (fStyle == e_FLASH_STYLE_ON_TORCH) {
        logD("isNeedFiringFlash(): OT.");
        mCapRep.isMainFlash = 1;
    } else if (fStyle == e_FLASH_STYLE_OFF_AUTO) {
        logD("isNeedFiringFlash(): XA.");
        if (AeMgr::getInstance(mSensorDev).IsStrobeBVTrigger()) {
            mCapRep.isMainFlash = 1;
            mCapRep.isPreFlash = 1;
        }
    } else
        logD("isNeedFiringFlash(): invalid flash style.");

set_data_and_exit:
    logD("isNeedFiringFlash(): main flash(%d), run pre-flash(%d).",
            mCapRep.isMainFlash, mCapRep.isPreFlash);

    return mCapRep.isPreFlash;
}

int FlashMgrM::pfStart(FlashExePara *in, FlashExeRep *out)
{
    logD("pfStart().");

    /* set debug count */
    setDebugDataPath();

    /* prepare capture info */
    updateCapInfo(in);

    /* check if firing flash */
    out->isEnd = !isNeedFiringFlash();

    /* set strobe mode to AE */
    AeMgr::getInstance(mSensorDev).setStrobeMode(mCapRep.isMainFlash);

    if (out->isEnd)
        return 0;

    /* get flash project parameters */
    FLASH_PROJECT_PARA prjPara;
    if (mCapInfo.flashMode == LIB3A_FLASH_MODE_FORCE_ON)
        prjPara = FlashCustomAdapter::getInstance(mSensorDev)->getFlashProjectPara(mCapInfo.aeScene, 1);
    else
        prjPara = FlashCustomAdapter::getInstance(mSensorDev)->getFlashProjectPara(mCapInfo.aeScene, 0);

    /* dump project para */
    FlashCustomAdapter::getInstance(mSensorDev)->dumpProjectPara(&prjPara);

    /* dump energy table */
    FlashCustomAdapter::getInstance(mSensorDev)->dumpNvEngTab();

    /* reset flash algo */
    FlashAlgoAdapter::getInstance(mSensorDev)->Reset();

    /* clear flash AE iteration */
    mCapRep.pfIteration = 0;

    /* read 3A para from nvram */
    NVRAM_CAMERA_3A_STRUCT *buf;
    if (FlashNvram::nvRead3A(buf, mSensorDev))
        logE("pfStart(): failed to get nvram.");

    /* read 3A para from nvram */
    NVRAM_CAMERA_STROBE_STRUCT *bufStrobe;
    if (FlashNvram::nvReadStrobe(bufStrobe, mSensorDev))
        logE("pfStart(): failed to get nvram.");

    /* init flash AWB algo */
    if (isFlashAWBv2Enabled(mSensorDev)) {
        logD("pfStart(): init flash AWB.");

        /* show flash AWB tuning para */
        FlashCustomAdapter::getInstance(mSensorDev)->showAWBTuningPara();

        /* init flash AWB algo */
        FLASH_AWB_INIT_T flashAwbInitParam;
        FlashCustomAdapter::getInstance(mSensorDev)->flashAWBInit(&flashAwbInitParam);
        FlashAlgoAdapter::getInstance(mSensorDev)->Flash_Awb_Init(flashAwbInitParam);
    }

    /* set flash algo info */
    FlashAlgInputInfoStruct flashAlgoInfo;
    flashAlgoInfo.NonPreferencePrvAWBGain = in->NonPreferencePrvAWBGain;
    flashAlgoInfo.PreferencePrvAWBGain = in->PreferencePrvAWBGain;
    flashAlgoInfo.i4AWBCCT = in->i4AWBCCT;
#if 1
    /* Platform: Kib, Alask, Whitne, Bianc, Vinso */
    FlashCustomAdapter::getInstance(mSensorDev)->getAWBRotationMatrix(&flashAlgoInfo);
#else
    /* Platform: Everes, Olympu, Kib */
    flashAlgoInfo.AwbRotationMatrix.i4Cos = buf->rAWBNVRAM[AWB_NVRAM_IDX_NORMAL].rAlgoCalParam.rRotationMatrix.i4Cos;
    flashAlgoInfo.AwbRotationMatrix.i4Sin = buf->rAWBNVRAM[AWB_NVRAM_IDX_NORMAL].rAlgoCalParam.rRotationMatrix.i4Sin;
#endif

    flashAlgoInfo.i4RotationMatrixUnit = 256;

    FlashAlgoAdapter::getInstance(mSensorDev)->setFlashInfo(&flashAlgoInfo);

    /* setup algo */
    hw_setPfPline();
    hw_setCapPline(&prjPara);
    hw_setFlashProfile();
    hw_setPreference(&prjPara);

    /* check algo input para */
    int err = FlashAlgoAdapter::getInstance(mSensorDev)->checkInputParaError(NULL, NULL);
    if (err) {
        addErr(err);
        logE("pfStart(): invalid algo input parameters.");
        out->isEnd = 1;
        mCapRep.isMainFlash = 0;
        return 0;
    }

    /* set first AE parameters to algo */
    FlashAlgExpPara expPara;
    expPara.exp = mAlgoExpPara.exp;
    expPara.iso = mAlgoExpPara.iso;
    logI("pfStart(): input exp/iso(%d/%d).", expPara.exp, expPara.iso);
    expPara.isFlash = 0;
    FlashAlgoAdapter::getInstance(mSensorDev)->CalFirstEquAEPara(&expPara, &mAlgoExpPara);

    /* New Flow for high lightness environment*/
    if (isHighBrightnessFlow() && getFlashFlowType())
    {
        logI("pfStart(): New Flow for hightlightness environment.");
        /* convert 3A statistic data to algo statistic data */
        FlashAlgStaData staData;
        int staDataSize = FLASH_AAO_Y_RESIZE_WIDTH * FLASH_AAO_Y_RESIZE_HEIGHT +
        FLASH_AAO_Y_DIGITAL_ZOOM_WIDTH * FLASH_AAO_Y_DIGITAL_ZOOM_HEIGHT;
        short data[staDataSize];
        staData.data = data;
        hw_convertAaStt(&staData, in->staBuf, in->staX, in->staY);
        logD("staData(%p), staBuf(%p), staX(%d), staY(%d)",&staData, in->staBuf, in->staX, in->staY);
        /* add statistic data to algo */
        int isNeedNext;
        logD("FlashPolicy(%d)",prjPara.tuningPara.cfgFlashPolicy);
        FlashAlgExpPara algoExpParaNext;
        FlashAlgoAdapter::getInstance(mSensorDev)->AddStaData10(&staData, &mAlgoExpPara,
                &isNeedNext, &algoExpParaNext, 0);
        mAlgoExpPara = algoExpParaNext;
        mCapRep.pfIteration++;
    }

    if (getFlashFlowType()){
        out->isCurFlashOn = mCapRep.isPreFlash;
        out->nextIsFlash = mCapRep.isPreFlash;
        mCapRep.pfFrameCycle = 1;
        if(mCapRep.isPreFlash){
            /* notify AP the sub flash state change to pre-flash */
            if (mSensorDev == DUAL_CAMERA_SUB_SENSOR || mSensorDev == DUAL_CAMERA_SUB_2_SENSOR) {
                setSubFlashState(e_preflash);
            }
        }
        logI("pfStart(): New Flow, no run state.");
        return 0;
    }

    out->nextIsFlash = 0;
    out->isCurFlashOn = 0;
    return 0;
}

int FlashMgrM::pfRun(FlashExePara *in, FlashExeRep *out)
{
    logD("pfRun().");

    /* get flash project para */
    FLASH_PROJECT_PARA prjPara;
    if (mCapInfo.flashMode == LIB3A_FLASH_MODE_FORCE_ON)
        prjPara = FlashCustomAdapter::getInstance(mSensorDev)->getFlashProjectPara(mCapInfo.aeScene, 1);
    else
        prjPara = FlashCustomAdapter::getInstance(mSensorDev)->getFlashProjectPara(mCapInfo.aeScene, 0);

    /* convert 3A statistic data to algo statistic data */
    FlashAlgStaData staData;
    int staDataSize = FLASH_AAO_Y_RESIZE_WIDTH * FLASH_AAO_Y_RESIZE_HEIGHT +
        FLASH_AAO_Y_DIGITAL_ZOOM_WIDTH * FLASH_AAO_Y_DIGITAL_ZOOM_HEIGHT;
    short data[staDataSize];
    staData.data = data;
    hw_convertAaStt(&staData, in->staBuf, in->staX, in->staY);

    /* add statistic data to algo */
    int isNeedNext;
    logI("FlashPolicy(%d)",prjPara.tuningPara.cfgFlashPolicy);
    FlashAlgExpPara algoExpParaNext;
    FlashAlgoAdapter::getInstance(mSensorDev)->AddStaData10(&staData, &mAlgoExpPara,
            &isNeedNext, &algoExpParaNext, 0);
    mAlgoExpPara = algoExpParaNext;
    mCapRep.pfIteration++;

    /* update and setup */
    int afe;
    int isp;

    if (mCapRep.pfIteration > 10 || !isNeedNext) {
        FlashAlgFacePos pFaceInfo;
        int faceFlashSupport = FlashCustomAdapter::getInstance(mSensorDev)->getFaceFlashSupport();
        memset(&pFaceInfo, 0, sizeof(FlashAlgFacePos));

        if (faceFlashSupport && !(staData.col < 24 || staData.row < 18)) {
        /*Enable face flash algo*/
            getPropInt(PROP_FLASH_ALGO_FACE_MODE, &(pFaceInfo.faceFlashSt), faceFlashSupport);
            if (pFaceInfo.faceFlashSt)
            {
                /*deliver converted face postion and weighting information to algorithm*/
                if (mFaceNumber > 0 && mFaceNumber <= FACE_MAX_NUM)
                    setFaceFlashAlgoInformation(&pFaceInfo);
                else
                    pFaceInfo.faceFlashSt = 0;
            }
        }

        /* estimate (iso,exp,isFlash,duty,step) from flash AE algo */
        logD("pfRun(): algo estimate & refine START.");
        CAM_TRACE_BEGIN("FlashAEalgo");
        FlashAlgoAdapter::getInstance(mSensorDev)->Estimate(&mAlgoExpPara, &pFaceInfo, &mIsLowRef);
        CAM_TRACE_END();
        logD("pfRun(): estimate END. exp(%d), iso(%d), duty(%d).", mAlgoExpPara.exp, mAlgoExpPara.iso, mAlgoExpPara.duty);
        refineFlashAEResult();
        logD("pfRun(): refine END. exp(%d), iso(%d), duty(%d).", mAlgoExpPara.exp, mAlgoExpPara.iso, mAlgoExpPara.duty);

        /* calculate flash AWB weighting */
        MUINT32 flashResultWeight = 0;
        if (isFlashAWBv2Enabled(mSensorDev)) {
            CAM_TRACE_BEGIN("FlashAWBalgo");
            FlashAlgoAdapter::getInstance(mSensorDev)->Flash_Awb_Algo(mAlgoExpPara.exp, mAlgoExpPara.iso, &flashResultWeight);
            CAM_TRACE_END();
        }
        /* set exposure */
        hw_setCapExp(&mAlgoExpPara);

        /* convert ISO to sensor gain and ISP gain */
        isoToGain(mAlgoExpPara.iso, &afe, &isp, 1, mCapInfo.appMode);

        /* update results */
        out->isEnd = 1;
        out->flashAwbWeight = flashResultWeight;

        /* update capture results */
        mCapRep.capExp = mAlgoExpPara.exp;
        mCapRep.capIso = mAlgoExpPara.iso;
        mCapRep.capAfeGain = afe;
        mCapRep.capIspGain = isp;

        int duty = 0;
        int dutyLt = 0;
        if (cust_isDualFlashSupport(mFacingSensor)) {
            FlashDuty::ind2duty(mAlgoExpPara.duty, duty, dutyLt,
                    mDutyNum, mDutyNumLT, 1);
            mCapRep.capDuty = duty;
            mCapRep.capDutyLT = dutyLt;
            mCapRep.dutyNum = mDutyNum;
            mCapRep.dutyNumLT = mDutyNumLT;
        } else {
            FlashDuty::ind2duty(mAlgoExpPara.duty, duty, dutyLt,
                    mDutyNum, mDutyNumLT, 0);
            mCapRep.capDuty = duty;
            mCapRep.capDutyLT = -1;
            mCapRep.dutyNum = mDutyNum;
            mCapRep.dutyNumLT = 0;
        }

        /* set flash hal info */
        FlashHalInfo flashHalInfo;
        memset(&flashHalInfo, 0, sizeof(FlashHalInfo));
        if (mIsManualFlashEnCct) {
            flashHalInfo.duty = mManualDutyCct;
            flashHalInfo.dutyLt = mManualDutyLtCct;
        } else if (mPropFlashMfOn > 0) {
            flashHalInfo.duty = mPropFlashMfDuty;
            flashHalInfo.dutyLt = mPropFlashMfDutyLt;
        } else {
            flashHalInfo.duty = mCapRep.capDuty;
            flashHalInfo.dutyLt = mCapRep.capDutyLT;
        }
        FlashHal::getInstance(mSensorDev)->setInfoDuty(
                FLASH_HAL_SCENARIO_MAIN_FLASH, flashHalInfo);

    } else if (getFlashFlowType()) {
        out->isEnd = 0;
        logD("pfRun(): isEnd(%d), CFG_FL_PF_CYC-1", out->isEnd);
        return 0;
    } else {
        /* set exposure */
        hw_setPfExp(&mAlgoExpPara);

        /* turn on/off flashlight */
        if (mAlgoExpPara.isFlash) {
            FlashHalInfo flashHalInfo;
            memset(&flashHalInfo, 0, sizeof(FlashHalInfo));
            int dutyIndex = 0;
            flashHalInfo.duty = FlashCustomAdapter::getInstance(mSensorDev)->getPfDuty(&mCapInfo);
            flashHalInfo.dutyLt = FlashCustomAdapter::getInstance(mSensorDev)->getPfDutyLt(&mCapInfo);
            FlashDuty::duty2ind(dutyIndex, flashHalInfo.duty, flashHalInfo.dutyLt,
                    mDutyNum, mDutyNumLT,
                    cust_isDualFlashSupport(mFacingSensor));
            mAlgoExpPara.duty = dutyIndex;
            FlashHal::getInstance(mSensorDev)->setInfoDuty(
                    FLASH_HAL_SCENARIO_PRE_FLASH, flashHalInfo);
        }

        /* convert ISO to sensor gain and ISP gain */
        isoToGain(mAlgoExpPara.iso, &afe, &isp, 0, mCapInfo.appMode);

        /* update results */
        out->isEnd = 0;
    }

    /* update results */
    out->nextExpTime = mAlgoExpPara.exp;
    out->nextIso = mAlgoExpPara.iso;
    out->nextAfeGain = afe;
    out->nextIspGain = isp;
    out->nextIsFlash = mAlgoExpPara.isFlash;
    out->isCurFlashOn = mAlgoExpPara.isFlash;
    out->nextDuty = mAlgoExpPara.duty;

    logD("pfRun(): isEnd(%d).", out->isEnd);
    return 0;
}

int FlashMgrM::doPfOneFrame(FlashExePara *in, FlashExeRep *out)
{
    /* Change the flash special mode ONLY at the first frame of pre-capture.
     *
     * There are five flash special state:
     *   e_SpModeNormal
     *   e_SpModeUserExp
     *   e_SpModeCalibration
     *   e_SpModeQuickCalibration
     *   e_SpModeQuickCalibration2
     */
    if (!mPfFrameCount) {
        /* get flash special mode from CCT tool */
        mSpMode = mSpModeCct;

        if (mSpMode == e_SpModeNormal) {
            /* get flash special mode from property */
            int propFlashCalibration = 0, propFlashRatio = 0;
            int propFlashQuickCalibration2 = 0, propFlashRatioQuick2 = 0;
            getPropInt(PROP_FLASH_CALIBRATION, &propFlashCalibration, 0);
            getPropInt(PROP_FLASH_RATIO, &propFlashRatio, 0);
            getPropInt(PROP_FLASH_QUICK_CALIBRATION_2, &propFlashQuickCalibration2, 0);
            getPropInt(PROP_FLASH_RATIO_QUICK2, &propFlashRatioQuick2, 0);
            if (propFlashCalibration || propFlashRatio)
                mSpMode = e_SpModeCalibration;
            else if (propFlashQuickCalibration2 || propFlashRatioQuick2)
                mSpMode = e_SpModeQuickCalibration2;
        }
    }

    /* dispatch pre-flash flow by special mode */
    int err = 0;
    if (!isAvailable()) {
        logI("doPfOneFrame(): sensorDev(%d) not available, frame count(%d).",
                mSensorDev, mPfFrameCount);
        out->isEnd = 1;
        err = -1;
    } else if (mSpMode == e_SpModeNormal && !mPropFlashMfOn) {
        setFlashMode(LIB3A_FLASH_MODE_FORCE_OFF);
        out->isFlashCalibrationMode = 0;
        err = doPfOneFrameNormal(in, out);
    } else if (mSpMode == e_SpModeNormal && (mIsManualFlashEnCct || mPropFlashMfOn > 0)) {
        setFlashMode(LIB3A_FLASH_MODE_FORCE_ON);
        out->isFlashCalibrationMode = 0;
        err = doPfOneFrameNormal(in, out);
    } else if (mSpMode == e_SpModeNormal) {
        out->isFlashCalibrationMode = 0;
        err = doPfOneFrameNormal(in, out);
    } else if (mSpMode == e_SpModeUserExp) {
        logD("doPfOneFrame(): userExp, frame count(%d).", mPfFrameCount);
        out->isFlashCalibrationMode = 0;
        out->isEnd = 1;
    } else if (mSpMode == e_SpModeCalibration) {
        logD("doPfOneFrame(): calibration, frame count(%d).", mPfFrameCount);
        out->isFlashCalibrationMode = 1;
        mIsCalibration = 1;

        int calibrationAAOEn = 0;
        getPropInt(PROP_FLASH_CALIBRATION_AAO_EN, &calibrationAAOEn, 0);
        if (calibrationAAOEn == 1)
        {
            int ret;

            ret = createDir(FLASH_DATA_PATH);
            if (ret != 0) {
                logE("create dir (%s) failed: %s", FLASH_DATA_PATH, strerror(errno));
                err = -1;
            }
            ret = createDir(FLASH_AAO_DATA_PATH);
            if (ret != 0) {
                logE("create dir (%s) failed: %s", FLASH_AAO_DATA_PATH, strerror(errno));
                err = -1;
            }

            mCalibrationAAOCnt = mCalibrationAAOCnt + 1;
            char aeFileName[256];
            char awbFileName[256];
            snprintf(aeFileName, sizeof(aeFileName), "%s/calibration_%d_ae.bmp", FLASH_AAO_DATA_PATH, mCalibrationAAOCnt);
            snprintf(awbFileName, sizeof(awbFileName), "%s/calibration_%d_awb.bmp", FLASH_AAO_DATA_PATH, mCalibrationAAOCnt);
            convertAaSttToYrgbBmp(in->staBuf, in->staX, in->staY, mCapInfo.bitMode, aeFileName, awbFileName, mLinearOutputEn);

            logI("doPfOneFrame(): mCalibrationAAOCnt(%d).", mCalibrationAAOCnt);
        }

        err = cctCalibration(in, out);
    } else if (mSpMode == e_SpModeQuickCalibration2) {
        logD("doPfOneFrame(): quick calibration 2, frame count(%d).", mPfFrameCount);
        out->isFlashCalibrationMode = 1;
        mIsCalibration = 1;

        int calibrationAAOEn = 0;
        getPropInt(PROP_FLASH_CALIBRATION_AAO_EN, &calibrationAAOEn, 0);
        if (calibrationAAOEn == 1)
        {
            int ret;
            ret = createDir(FLASH_DATA_PATH);
            if (ret != 0) {
                logE("create dir (%s) failed: %s", FLASH_DATA_PATH, strerror(errno));
                err = -1;
            }
            ret = createDir(FLASH_AAO_DATA_PATH);
            if (ret != 0) {
                logE("create dir (%s) failed: %s", FLASH_AAO_DATA_PATH, strerror(errno));
                err = -1;
            }

            mQuickCalibrationAAOCnt = mQuickCalibrationAAOCnt + 1;
            char aeFileName[256];
            char awbFileName[256];
            snprintf(aeFileName, sizeof(aeFileName), "%s/quick_calibration_%d_ae.bmp", FLASH_AAO_DATA_PATH, mQuickCalibrationAAOCnt);
            snprintf(awbFileName, sizeof(awbFileName), "%s/quick_calibration_%d_awb.bmp", FLASH_AAO_DATA_PATH, mQuickCalibrationAAOCnt);
            convertAaSttToYrgbBmp(in->staBuf, in->staX, in->staY, mCapInfo.bitMode, aeFileName, awbFileName, mLinearOutputEn);
            logI("doPfOneFrame(): mQuickCalibrationAAOCnt(%d).", mQuickCalibrationAAOCnt);
        }

        err = cctCalibrationQuick2(in, out);
    } else {
        logE("doPfOneFrame(): invalid spMode(%d), frame count(%d).", mSpMode, mPfFrameCount);
        err = -1;
    }
    mPfFrameCount++;

    return err;
}

int FlashMgrM::doPfOneFrameNormal(FlashExePara *in, FlashExeRep *out)
{
    /*
     * Pre-flash state machine. There four pre-flash state:
     *   START
     *   COOLING
     *   RUN
     *   END
     */
    logD("doPfOneFrameNormal(): frame(%d), state(%d).", mPfFrameCount, mCapRep.pfState);

    if (mCapRep.pfState == FLASH_PF_STATE_START) {
        logD("doPfOneFrameNormal(): START state.");

        /* first frame in start state */
        mCapRep.pfStartTime = getMs();
        mCapRep.pfPreState = FLASH_PF_STATE_START;
        mCapRep.pfAlgoDone = 0;
        out->isEnd = 0;

        /* get frame cycle */
        MINT32 shutterDelay = 0, sensorGainDelay = 0, ispGainDelay = 0, captureDelay = 0;
        AAASensorMgr::getInstance().getSensorSyncinfo(
                mSensorDev, &shutterDelay, &sensorGainDelay,
                &ispGainDelay, &captureDelay);
        int maxDelay;
        maxDelay = max(shutterDelay, sensorGainDelay);
        maxDelay = max(maxDelay, ispGainDelay);
        mCapRep.pfFrameCycle = maxDelay + 1;
        if (mCapRep.pfFrameCycle < 3)
            mCapRep.pfFrameCycle = 3;

        /* do start */
        pfStart(in, out);
        mCapRep.pfApplyFrm = mPfFrameCount;

        if (out->isEnd) {
            mCapRep.pfState = FLASH_PF_STATE_END;
        } else if (getFlashFlowType()) {
            mCapRep.pfState = FLASH_PF_STATE_RUN;
        } else {
            mCapRep.pfState = FLASH_PF_STATE_COOLING;
        }

    } else if (mCapRep.pfState == FLASH_PF_STATE_COOLING) {
        logD("doPfOneFrameNormal(): COOLING state.");

        /* first frame in cooling state */
        if (mCapRep.pfPreState != FLASH_PF_STATE_COOLING) {
            mCapRep.pfCoolTime = getMs();
            mCapRep.pfPreState = FLASH_PF_STATE_COOLING;
        }

        int waitMs;
        if (FlashHal::getInstance(mSensorDev)->isNeedWaitCooling(getMs(), &waitMs))
            logI("doPfOneFrameNormal(): waitMs(%d ms).", waitMs);
        else
            mCapRep.pfState = FLASH_PF_STATE_RUN;
    }

    if (mCapRep.pfState == FLASH_PF_STATE_RUN) {
        logD("doPfOneFrameNormal(): RUN state.");

        /* first frame in run state */
        if (mCapRep.pfPreState != FLASH_PF_STATE_RUN) {
            mCapRep.pfRunTime = getMs();
            mCapRep.pfRunStartFrm = mPfFrameCount;
            mCapRep.pfPreState = FLASH_PF_STATE_RUN;
        }

        /* each cycle */
        if (mPfFrameCount - mCapRep.pfApplyFrm >= mCapRep.pfFrameCycle) {
            /* run */
            pfRun(in, out);

            mCapRep.pfApplyFrm = mPfFrameCount;
            if (out->isEnd == 1){
                mCapRep.pfState = FLASH_PF_STATE_END;
                mCapRep.pfAlgoDone = 1;
            }
        }
    }

    if (mCapRep.pfState == FLASH_PF_STATE_END) {
        logD("doPfOneFrameNormal(): END state.");

        /* first frame in end state */
        mCapRep.pfEndTime = getMs();
        mCapRep.pfRunFrms = mPfFrameCount - mCapRep.pfRunStartFrm + 1;
        mCapRep.pfPreState = FLASH_PF_STATE_END;
    }

    /* dump debug data */
    dumpDebugData((void *)in->staBuf, in->staX, in->staY, mCapInfo.bitMode, mDebugSceneCnt, mPfFrameCount);

    logD("doPfOneFrameNormal(): isEnd(%d).", out->isEnd);
    return 0;
}

int FlashMgrM::endPrecapture()
{
    if (!isAvailable()) {
        logI("endPrecapture(): sensorDev(%d) not available.", mSensorDev);
        return -1;
    }

    logI("endPrecapture().");

    /* clear pre-flash frame count */
    mPfFrameCount = 0;
    mCapRep.pfState = FLASH_PF_STATE_START;

    /* clear SpMode */
    mSpModeCct = e_SpModeNormal;
    mSpMode = e_SpModeNormal;
    mIsCalibration = 0;
    mMainFlashAAOCnt = 0;
    mFaceNumber = 0;
    mPfHighBrightness = 0;

    /* prepare flash for main flash */
    if (mCapRep.isMainFlash == 1) {
        FlashHal::getInstance(mSensorDev)->setPreOn();
        /* notify AP the sub flash state change to main-flash */
        if (mSensorDev == DUAL_CAMERA_SUB_SENSOR || mSensorDev == DUAL_CAMERA_SUB_2_SENSOR) {
            setSubFlashState(e_mainflash);
        }
    }
    return 0;
}

int FlashMgrM::setCapPara()
{
    if (!isAvailable()) {
        logI("setCapPara(): sensorDev(%d) not available.", mSensorDev);
        return -1;
    }

    logI("setCapPara().");

    int fMode = getFlashMode();
    int fStyle = FlashCustomAdapter::getInstance(mSensorDev)->getFlashModeStyle(mSensorDev, fMode);

    if (mCapRep.isMainFlash == 0 || mCapRep.pfAlgoDone == 0){
        AE_MODE_CFG_T aeParam;
        memset(&aeParam, 0, sizeof(AE_MODE_CFG_T));
        AeMgr::getInstance(mSensorDev).getPreviewParams(aeParam);
        mAlgoExpPara.exp = aeParam.u4Eposuretime;
        mAlgoExpPara.iso = aeParam.u4RealISO;
        mAlgoExpPara.duty = 0;
        logI("setCapPara(), No precapture info, use preview setting with minimum intensity");
    }

    if (fStyle != e_FLASH_STYLE_ON_TORCH)
        hw_setCapExp(&mAlgoExpPara);

    return 0;
}

int FlashMgrM::setAlgoExpPara(int exp, int afe, int isp, int isFlashOn)
{
    logD("setAlgoExpPara(): exp(%d), afe(%d), isp(%d)", exp, afe, isp);

    if (isFlashOn) {
        m_strEtGainTorchAE.u4Eposuretime = exp;
        m_strEtGainTorchAE.u4AfeGain = afe;
        m_strEtGainTorchAE.u4IspGain = isp;
    } else {
        m_strEtGainNormal.u4Eposuretime = exp;
        m_strEtGainNormal.u4AfeGain = afe;
        m_strEtGainNormal.u4IspGain = isp;
    }

    int iso = 0;
    gainToIso(afe, isp, &iso, 0, mCapInfo.appMode);
    mAlgoExpPara.exp = exp;
    mAlgoExpPara.iso = iso;

    return 0;
}

int FlashMgrM::updateAeParams(int exp, int afe, int isp, int isCapture)
{
    /* get preview pline */
    strAETable prvAePline = {};
    strAETable capAePline = {};
    strAETable strobeAePline = {};
    strAFPlineInfo afPlineInfo = {};
    AeMgr::getInstance(mSensorDev).getCurrentPlineTable(
            prvAePline, capAePline, strobeAePline, afPlineInfo);

    if (capAePline.pCurrentTable == NULL){
        logE("%s get current pline failed", __func__);
        return -1;
    }

    int bestInd = 0;
    if (!(isManualFlashFlow() && isCapture == 1)){
        /* search the closer exposure parameters from pline */
        if (isCapture) {
            FlashPlineTool::searchAePlineIndex(&bestInd, &capAePline, exp, afe, isp);
            exp = capAePline.pCurrentTable->sPlineTable[bestInd].u4Eposuretime;
            afe = capAePline.pCurrentTable->sPlineTable[bestInd].u4AfeGain;
            isp = capAePline.pCurrentTable->sPlineTable[bestInd].u4IspGain*mIspGainBase/PLINE_mIspGainBase;
        } else {
            FlashPlineTool::searchAePlineIndex(&bestInd, &prvAePline, exp, afe, isp);
            exp = prvAePline.pCurrentTable->sPlineTable[bestInd].u4Eposuretime;
            afe = prvAePline.pCurrentTable->sPlineTable[bestInd].u4AfeGain;
            isp = prvAePline.pCurrentTable->sPlineTable[bestInd].u4IspGain*mIspGainBase/PLINE_mIspGainBase;
        }
    }

    /* update AE info */
    AE_MODE_CFG_T aeParam;
    memset(&aeParam, 0, sizeof(AE_MODE_CFG_T));

    if (isCapture)
        AeMgr::getInstance(mSensorDev).getCaptureParams(aeParam);
    else
        AeMgr::getInstance(mSensorDev).getPreviewParams(aeParam);
    aeParam.u4Eposuretime = exp;
    aeParam.u4AfeGain = afe;
    aeParam.u4IspGain = isp;
    if (isCapture)
        AeMgr::getInstance(mSensorDev).updateCaptureParams(aeParam);
    else
        FlashPlatformAdapter::getInstance(mSensorDev)->updateAePreviewParams(
                &aeParam, bestInd);

    logD("updateAeParams(): exp(%d), afe(%d), isp(%d), best index(%d).",
            exp, afe, isp, bestInd);

    return 0;
}

void FlashMgrM::getAeParams(int *exp, int *afe, int *isp, int isCapture)
{
    AE_MODE_CFG_T aeParam;
    if (isCapture)
        AeMgr::getInstance(mSensorDev).getCaptureParams(aeParam);
    else
        AeMgr::getInstance(mSensorDev).getPreviewParams(aeParam);

    /* flash algo exposure parameters */
    *exp = aeParam.u4Eposuretime;
    *afe = aeParam.u4AfeGain;
    *isp = aeParam.u4IspGain;

    logD("getAeParams(): exp(%d), afe(%d), isp(%d).", *exp, *afe, *isp);
}

template <class T>
void FlashMgrM::gainToIso(int afe, int isp, T *iso, int isCapture, int appMode)
{
    AE_DEVICES_INFO_T aeDevInfo = {};
    AeMgr::getInstance(mSensorDev).getSensorDeviceInfo(aeDevInfo);

    if (isCapture) {
        int cap2PreRatio;
        if (appMode == eAppMode_ZsdMode)
            cap2PreRatio = 1024;
        else
            cap2PreRatio = aeDevInfo.u4Cap2PreRatio;

        *iso = aeDevInfo.u4MiniISOGain * (cap2PreRatio/1024.0) * (afe/(float)mAfeGainBase) * (isp/(float)mIspGainBase);
    } else
        *iso = aeDevInfo.u4MiniISOGain * (afe/(float)mAfeGainBase) * (isp/(float)mIspGainBase);
}

template <class T>
void FlashMgrM::isoToGain(T iso, int *afe, int *isp, int isCapture, int appMode)
{
    AE_DEVICES_INFO_T aeDevInfo = {};
    AeMgr::getInstance(mSensorDev).getSensorDeviceInfo(aeDevInfo);

    if (aeDevInfo.u4MiniISOGain == 0 || aeDevInfo.u4MaxGain == 0) {
        logE("%s get sensor device info failed", __func__);
        return;
    }

    /* get mix gain */
    float mixGain;
    if (isCapture) {
        int cap2PreRatio;
        if (appMode == eAppMode_ZsdMode)
            cap2PreRatio = 1024;
        else
            cap2PreRatio = aeDevInfo.u4Cap2PreRatio;
        mixGain = (float)iso / (aeDevInfo.u4MiniISOGain * cap2PreRatio / 1024.0);
    } else
        mixGain = (float)iso / aeDevInfo.u4MiniISOGain;

    /* get sensor gain and isp gain */
    int maxGain = aeDevInfo.u4MaxGain;
    if (mixGain < (float)maxGain / mAfeGainBase) {
        *afe = mixGain * mAfeGainBase;
        *isp = mIspGainBase;
    } else {
        *afe = maxGain;
        *isp = (mixGain * mAfeGainBase / maxGain) * mIspGainBase;
    }
}

int FlashMgrM::dumpMainFlashAAO(void* aao)
{
    int pfBmpEn = 0, mfBmpEn = 0;
    getPropInt(PROP_FLASH_PF_BMP_EN, &pfBmpEn, 0);
    getPropInt(PROP_FLASH_MF_BMP_EN, &mfBmpEn, 0);
    if(pfBmpEn == 1 || mfBmpEn == 1)
    {
        int ret;

        ret = createDir(FLASH_DATA_PATH);
        if (ret != 0) {
            logE("create dir (%s) failed: %s", FLASH_DATA_PATH, strerror(errno));
        }
        ret = createDir(FLASH_AAO_DATA_PATH);
        if (ret != 0) {
            logE("create dir (%s) failed: %s", FLASH_AAO_DATA_PATH, strerror(errno));
        }

        double yrgb[4] = {0.0, 0.0, 0.0, 0.0};
        cal_1_4_yrgb_mean(aao, FLASH_AAO_Y_WIDTH, FLASH_AAO_Y_HEIGHT, yrgb, mLinearOutputEn);

        FILE* fp;
        fp = fopen(FLASH_AAO_MEAN_DATA_PATH, "a");
        if(fp != NULL)
        {
            double y = (5 * yrgb[1] + 9 * yrgb[2] + 2 * yrgb[3]) / 16.0;
            int dutyIndex = 0;
            if (cust_isDualFlashSupport(mFacingSensor))
            {
                FlashDuty::duty2ind(dutyIndex, mCapRep.capDuty, mCapRep.capDutyLT, mDutyNum, mDutyNumLT, 1);
            }
            else
            {
                FlashDuty::duty2ind(dutyIndex, mCapRep.capDuty, mCapRep.capDutyLT, mDutyNum, mDutyNumLT, 0);
            }
            int eng = FlashCustomAdapter::getInstance(mSensorDev)->getYTabByDutyIndex(dutyIndex);
            int rgain = FlashCustomAdapter::getInstance(mSensorDev)->getWBRGainByDutyIndex(dutyIndex);
            int ggain = FlashCustomAdapter::getInstance(mSensorDev)->getWBGGainByDutyIndex(dutyIndex);
            int bgain = FlashCustomAdapter::getInstance(mSensorDev)->getWBBGainByDutyIndex(dutyIndex);
            double r = yrgb[1] * rgain / ggain;
            double g = yrgb[2] * ggain / ggain;
            double b = yrgb[3] * bgain / ggain;
            double maxRGB = max(r,g);
            if(max(r,g) < max(b,g))
            {
                maxRGB = max(b,g);
            }
            double minRGB = min(r,g);
            if(min(r,g) > min(b,g))
            {
                minRGB = min(b,g);
            }
            double saturation = (maxRGB - minRGB) / maxRGB;
            fprintf(fp, "mDebugSceneCnt(%d), mMainFlashAAOCnt(%d), exp(%d), sensor gain(%d), isp gain(%d), duty(%d,%d), yrgb(%f,%f,%f,%f), eng(%d), rgb gain(%d,%d,%d), saturation(%f)\n",
                mDebugSceneCnt, mMainFlashAAOCnt,
                mCapRep.capExp, mCapRep.capAfeGain, mCapRep.capIspGain,
                mCapRep.capDuty, mCapRep.capDutyLT,
                y, yrgb[1], yrgb[2], yrgb[3],
                eng, rgain, ggain, bgain,
                saturation);
            fclose(fp);
        }

        char aeFileName[256];
        char awbFileName[256];

        snprintf(aeFileName, sizeof(aeFileName), "%s/mf_ae_%03d_%02d.bmp", FLASH_BMP_DATA_PATH, mDebugSceneCnt, mMainFlashAAOCnt);
        snprintf(awbFileName, sizeof(awbFileName), "%s/mf_awb_%03d_%02d.bmp", FLASH_BMP_DATA_PATH, mDebugSceneCnt, mMainFlashAAOCnt);
        convertAaSttToYrgbBmp(aao, FLASH_AAO_Y_WIDTH, FLASH_AAO_Y_HEIGHT, mCapInfo.bitMode, aeFileName, awbFileName, mLinearOutputEn);
        logI("dumpMainFlashAAO(): mDebugSceneCnt(%d), mMainFlashAAOCnt(%d).", mDebugSceneCnt, mMainFlashAAOCnt);

        mMainFlashAAOCnt = mMainFlashAAOCnt + 1;
    }

    return 0;
}

int FlashMgrM::refineFlashAEResult()
{
    if(isLowConfidence()){
        decideMainFlashEng();

        if(isWellControl()){
            /* CWV Control */
            CWVControl();
            logI("CWV Control\n");
        } else {
            /* EtGain Transform */
            EtGainTrans4();
            logI("EtGain Transform\n");
        }
        /* Each case needs to update exposure setting */

        int temp_exp, temp_iso;
        temp_exp = m_strEtGainMainFlash.u4Eposuretime;
        FlashMgrM::gainToIso(m_strEtGainMainFlash.u4AfeGain, m_strEtGainMainFlash.u4IspGain, &temp_iso, 1, mCapInfo.appMode);
        mAlgoExpPara.exp = temp_exp;
        mAlgoExpPara.iso = temp_iso;
        dumpDeciderInfo();
    } else {
        /* FlashAlgoMap */
        logI("FlashAlgoMap, Use Algo Result\n");
    }
    return 0;
}

void FlashMgrM::setMVHDRMode(int &i4Mode)
{
    m_IsMvHDREnable = i4Mode;
    logI("m_IsMvHDREnable: %d\n", m_IsMvHDREnable);
}

bool FlashMgrM::isManualFlashFlow()
{
    return (isShutterPriorityMode() || isISOPriorityMode());
}

bool FlashMgrM::isShutterPriorityMode()
{
    return (mCapInfo.manualExposureTime != -1) && (mCapInfo.manualSensitivity == -1);
}

bool FlashMgrM::isISOPriorityMode()
{
    return (mCapInfo.manualExposureTime == -1) && (mCapInfo.manualSensitivity != -1);
}

void FlashMgrM::switchManualExposureSetting(int* exp, int* iso, const int mExp, const int mIso)
{
    int expLevel = (*exp)*(*iso);
    if (mExp == -1 && mIso != -1 && mIso != 0){
        *iso = mIso;
        *exp = expLevel/mIso;
    } else if (mIso == -1 && mExp != -1 && mExp != 0) {
        *exp = mExp;
        *iso = expLevel/mExp;
    }
}

int FlashMgrM::setSubFlashState(int state)
{
    if(mCapInfo.subFlashState != state){
        logI("setSubFlashState(): state(%d).", mCapInfo.subFlashState);
        mCapInfo.subFlashState = state;
    }
    return 0;
}

int FlashMgrM::getSubFlashState()
{
    logD("getSubFlashState(): state(%d).", mCapInfo.subFlashState);
    return mCapInfo.subFlashState;
}

void FlashMgrM::setFlashOnAAOBuffer(void *buf)
{
    FlashPlatformAdapter::getInstance(mSensorDev)->setFlashOnAAOBuffer(buf);
}

void* FlashMgrM::getFlashOnAAOBuffer()
{
    return FlashPlatformAdapter::getInstance(mSensorDev)->getFlashOnAAOBuffer();
}

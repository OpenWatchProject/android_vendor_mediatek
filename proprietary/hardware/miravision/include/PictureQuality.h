#ifndef VENDOR_MEDIATEK_HARDWARE_PQ_V2_3_PICTUREQUALITY_H
#define VENDOR_MEDIATEK_HARDWARE_PQ_V2_3_PICTUREQUALITY_H

#include <vendor/mediatek/hardware/pq/2.3/IPictureQuality.h>
#include <hidl/MQDescriptor.h>
#include <hidl/Status.h>
#include <android/hidl/memory/1.0/IMemory.h>

#include <utils/threads.h>

#include "ddp_drv.h"
#include "cust_gamma.h"
#include "cust_tdshp.h"
#include "PQPictureMode.h"
#include <PQRszImpl.h>
#include <PQDSImpl.h>
#include <PQDCImpl.h>
#include <PQHDRImpl.h>

#include <map>
#include <string>

class BluLightDefender;
class TChameleonDisplayProcess;
class TPQTransitionProcess;
class PQAshmemProxy;
class PQLightSensor;
class CPQRszFW;
class CPQDSFW;
class CPQDCFW;
class CPQHDRFW;
#ifdef AAL_SERVICE_IN_PQ
class AALService;
#endif

enum DebugFlags {
    eDebugDisabled          = 0x0,
    eDebugBlueLight         = 0x1,
    eDebugChameleon         = 0x2,
    eDebugTransition        = 0x4,
    eDebugTimer             = 0x10,
    eDebugCCORR             = 0x20,
    eDebugColor             = 0x40,
    eDebugMatrix            = 0x100,
    eDebugAll               = 0x1FF,
};

enum PQChangeEvent {
    eEvtBlueLight       = 0x1, // blue light change : on/off/strength change
    eEvtChameleon       = 0x2, // Chameleon change : on/off/strength change
    eEvtTransition      = 0x4, // Transition change : on/off/strength change
    eEvtPQChange        = 0x10, // PQ mode or scenario change
    eEvtALI             = 0x20, // RGBW ambient light strength change
    eEvtBacklightChange = 0x40  // target backlight change
};

struct SensorInput {
    int aliR;
    int aliG;
    int aliB;
    int aliW;
};

struct PQInput {
    SensorInput sensorInput;
    uint32_t ccorrCoef[3][3];
    int oriBacklight;
    DISP_PQ_PARAM *pqparam;
};

struct PQOutput {
    DISPLAY_COLOR_REG_T colorRegTarget;
    uint32_t ccorrCoef[3][3];
    int targetBacklight;
    int configFlag;
    DISP_PQ_PARAM *pqparam;
};

namespace vendor {
namespace mediatek {
namespace hardware {
namespace pq {
namespace V2_3 {
namespace implementation {

using ::android::hidl::base::V1_0::DebugInfo;
using ::android::hidl::base::V1_0::IBase;
using ::vendor::mediatek::hardware::pq::V2_0::PQFeatureID;
using ::vendor::mediatek::hardware::pq::V2_0::Result;
using ::vendor::mediatek::hardware::pq::V2_0::dispPQIndexParams;
using ::vendor::mediatek::hardware::pq::V2_0::dispPQWinParams;
using ::vendor::mediatek::hardware::pq::V2_0::ioctlRequest;
using ::vendor::mediatek::hardware::pq::V2_0::dispPQIoctlParams;
using ::vendor::mediatek::hardware::pq::V2_0::globalPQIndex_t;
using ::android::hardware::hidl_array;
using ::android::hardware::hidl_memory;
using ::android::hardware::hidl_string;
using ::android::hardware::hidl_vec;
using ::android::hardware::Return;
using ::android::hardware::Void;
using ::android::sp;
using ::android::hidl::memory::V1_0::IMemory;
using ::android::Thread;
using ::android::Mutex;
using ::android::status_t;
using namespace std;

struct PictureQuality :
    public IPictureQuality,
    public Thread
{
    PictureQuality();
    ~PictureQuality();

    // Methods from ::vendor::mediatek::hardware::pq::V2_0::IPictureQuality follow.
    Return<Result> setColorRegion(int32_t split_en, int32_t start_x, int32_t start_y, int32_t end_x, int32_t end_y) override;
    Return<void> getColorRegion(getColorRegion_cb _hidl_cb) override;
    Return<Result> setPQMode(int32_t mode, int32_t step) override;
    Return<Result> setTDSHPFlag(int32_t TDSHPFlag) override;
    Return<void> getTDSHPFlag(getTDSHPFlag_cb _hidl_cb) override;
    Return<void> getMappedColorIndex(int32_t scenario, int32_t mode, getMappedColorIndex_cb _hidl_cb) override;
    Return<void> getMappedTDSHPIndex(int32_t scenario, int32_t mode, getMappedTDSHPIndex_cb _hidl_cb) override;
    Return<void> getColorIndex(int32_t scenario, int32_t mode, getColorIndex_cb _hidl_cb) override;
    Return<void> getTDSHPIndex(int32_t scenario, int32_t mode, getTDSHPIndex_cb _hidl_cb) override;
    Return<Result> setPQIndex(int32_t level, int32_t scenario, int32_t tuning_mode, int32_t index, int32_t step) override;
    Return<Result> setDISPScenario(int32_t scenario, int32_t step) override;
    Return<Result> setFeatureSwitch(PQFeatureID id, uint32_t value) override;
    Return<void> getFeatureSwitch(PQFeatureID id, getFeatureSwitch_cb _hidl_cb) override;
    Return<Result> enableBlueLight(bool enable, int32_t step) override;
    Return<void> getBlueLightEnabled(getBlueLightEnabled_cb _hidl_cb) override;
    Return<Result> setBlueLightStrength(int32_t strength, int32_t step) override;
    Return<void> getBlueLightStrength(getBlueLightStrength_cb _hidl_cb) override;
    Return<Result> enableChameleon(bool enable, int32_t step) override;
    Return<void> getChameleonEnabled(getChameleonEnabled_cb _hidl_cb) override;
    Return<Result> setChameleonStrength(int32_t strength, int32_t step) override;
    Return<void> getChameleonStrength(getChameleonStrength_cb _hidl_cb) override;
    Return<Result> setTuningField(int32_t pqModule, int32_t field, int32_t value) override;
    Return<void> getTuningField(int32_t pqModule, int32_t field, getTuningField_cb _hidl_cb) override;
    Return<void> getAshmem(getAshmem_cb _hidl_cb) override;
    Return<Result> setAmbientLightCT(double input_x, double input_y, double input_Y) override;
    Return<Result> setAmbientLightRGBW(int32_t input_R, int32_t input_G, int32_t input_B, int32_t input_W) override;
    Return<Result> setGammaIndex(int32_t index, int32_t step) override;
    Return<void> getGammaIndex(getGammaIndex_cb _hidl_cb) override;
    Return<Result> setExternalPanelNits(uint32_t externalPanelNits) override;
    Return<void> getExternalPanelNits(getExternalPanelNits_cb _hidl_cb) override;
    Return<Result> setColorTransform(const hidl_array<float, 4, 4>& matrix, int32_t hint, int32_t step) override;
    Return<void> execIoctl(const dispPQIoctlParams& arg, execIoctl_cb _hidl_cb) override;
    Return<Result> setRGBGain(uint32_t r_gain, uint32_t g_gain, uint32_t b_gain, int32_t step) override;

    Return<void> debug(const ::android::hardware::hidl_handle& handle, const ::android::hardware::hidl_vec<::android::hardware::hidl_string>& options) override;

    Return<Result> setGlobalPQSwitch(int32_t switch_value) override;
    Return<void> getGlobalPQSwitch(getGlobalPQSwitch_cb _hidl_cb) override;
    Return<Result> setGlobalPQStrength(int32_t strength) override;
    Return<void> getGlobalPQStrength(getGlobalPQStrength_cb _hidl_cb) override;
    Return<void> getGlobalPQStrengthRange(getGlobalPQStrengthRange_cb _hidl_cb) override;
    Return<void> getGlobalPQIndex(getGlobalPQIndex_cb _hidl_cb) override;
    Return<Result> setGlobalPQStableStatus(int32_t stable_status) override;
    Return<void> getGlobalPQStableStatus(getGlobalPQStableStatus_cb _hidl_cb) override;

    // Methods from ::vendor::mediatek::hardware::pq::V2_1::IPictureQuality follow.
    Return<Result> setSmartBacklightStrength(int32_t level) override;

    // Methods from ::vendor::mediatek::hardware::pq::V2_2::IPictureQuality follow.
    Return<Result> setColorMatrix3x3(const hidl_array<int32_t, 3, 3>& matrix, int32_t step) override;

    // Methods from ::vendor::mediatek::hardware::pq::V2_3::IPictureQuality follow.
    Return<Result> setFunction(uint32_t funcFlags, bool PropFlag) override;
    Return<Result> setScreenState(int32_t state, int32_t brightness) override;
    Return<Result> setSmartBacklightRange(int32_t level) override;
    Return<Result> setReadabilityLevel(int32_t level) override;
    Return<Result> setLowBLReadabilityLevel(int32_t level) override;
    Return<void> getESSLEDMinStep(getESSLEDMinStep_cb _hidl_cb) override;
    Return<Result> setESSLEDMinStep(uint32_t value) override;
    Return<void> getESSOLEDMinStep(getESSOLEDMinStep_cb _hidl_cb) override;
    Return<Result> setESSOLEDMinStep(uint32_t value) override;
    Return<Result> custInvoke(int32_t cmd, int64_t arg) override;
    Return<void> readField(uint32_t field, readField_cb _hidl_cb) override;
    Return<Result> writeField(uint32_t field, uint32_t value) override;

    // Methods from ::android::hidl::base::V1_0::IBase follow.
private:
    enum {
        DISPLAY_COLOR,
        CONTENT_COLOR,
        CONTENT_COLOR_VIDEO,
        SHARPNESS,
        DYNAMIC_CONTRAST,
        DYNAMIC_SHARPNESS,
        DISPLAY_CCORR,
        DISPLAY_GAMMA,
        DISPLAY_OVER_DRIVE,
        ISO_ADAPTIVE_SHARPNESS,
        ULTRA_RESOLUTION,
        VIDEO_HDR,
        PQ_FEATURE_MAX,
    };

    enum PQDebugFlag {
        PQDEBUG_OFF,
        PQDEBUG_ON,
        PQDEBUG_SHP_VALUE,
    };

    enum PQTuningMode {
        PQ_TUNING_NORMAL,
        PQ_TUNING_READING,
        PQ_TUNING_OVERWRITTEN,
        PQ_TUNING_END
    };

    enum PQConfigEvent {
        eConfigNone = 0x0,
        eConfigLast = 0x1,
        eConfigColor = 0x2,
        eConfigCCORR = 0x4,
    };

    enum PQRegType {
        eRegColor,
        eRegTransitionIn,
        eRegTransitionOut,
        eRegDrv,
    };

    mutable Mutex mLock;

    PicModeStandard *m_pic_statndard;
    PicModeVivid *m_pic_vivid;
    PicModeUserDef *m_pic_userdef;

    DISP_PQ_MAPPING_PARAM m_pqparam_mapping;
    DISP_PQ_PARAM m_pqparam_table[PQ_PARAM_TABLE_SIZE];
    dispPQIndexParams m_pqparam_client;
    DISP_PQ_DS_PARAM_EX g_PQ_DS_Param;
    DISP_PQ_DC_PARAM g_PQ_DC_Param;
    MDP_TDSHP_REG_EX g_tdshp_reg;
    PQ_RSZ_PARAM rszparam;
    bool mCcorrDebug;

    dispPQWinParams mdp_win_param;
    DISP_PQ_PARAM m_pqparam;    /* for temp m_pqparam calculation or ioctl */
    DISP_PQ_DC_PARAM pqdcparam;
    DISP_PQ_DS_PARAM pqdsparam;
    DISPLAY_PQ_T m_pqindex;
    DISPLAY_TDSHP_T m_tdshpindex; /* try to remove this one */
    gamma_entry_t m_CustGamma[GAMMA_LCM_MAX][GAMMA_INDEX_MAX];
    gamma_entry_t m_NvGamma[GAMMA_LCM_MAX][GAMMA_INDEX_MAX];
    bool m_NvGammaStatus;

    int32_t m_PQMode;
    int32_t m_PQScenario;
    int32_t m_drvID;
    uint32_t m_bFeatureSwitch[PQ_FEATURE_MAX];

    PQInput *mPQInput;
    PQOutput *mPQOutput;

    PQTuningMode mBLInputMode;
    void *mBLInput;
    PQTuningMode mBLOutputMode;
    void *mBLOutput;
    BluLightDefender *blueLight;

#ifdef CHAMELEON_DISPLAY_SUPPORT
    int32_t m_alsdrvID;
    PQLightSensor *mLightSensor;
    int mChameleonBacklightOut;
    int mTargetBacklight;
    PQTuningMode mChameleonInputMode;
    void *mChameleonInput;
    PQTuningMode mChameleonOutputMode;
    void *mChameleonOutput;
    TChameleonDisplayProcess *chameleonDisplayProcess;
    bool mAllowSensorDebounce;
    bool mSensorInputxyY;
    double mInput_x;
    double mInput_y;
    double mInput_Y;
#endif

    PQTuningMode mTransitionInputMode;
    void *mTransitionInput;
    PQTuningMode mTransitionOutputMode;
    void *mTransitionOutput;
    TPQTransitionProcess *mPQTransition;
    int mForceTransitionStep;

    unsigned int mChameleonCcorrCoefOut[3][3];
    int mDriverBacklight;

    unsigned int mBlueLightDebugFlag;

    unsigned int mDebugLevel;

    PQAshmemProxy *m_AshmemProxy;

    bool m_EventEnabled;

    int32_t m_gamma_id;

    uint32_t m_r_gain;
    uint32_t m_g_gain;
    uint32_t m_b_gain;
    bool m_rgbGainChanged;

    int32_t m_ccorr_3x3[3][3];
    bool m_matrixChanged;

    hidl_memory m_hidl_memory;
    bool m_is_ashmem_init;

    unsigned int m_PQParameterSN;

    /*bit0 for sharpness, bit1 for DC */
    uint32_t m_GlobalPQSwitch;
    /*
     * bit0-7 for video sharpness
     * bit8-15 for video DC
     * bit16-23 for ui sharpness
     * bit24-31 for ui DC
     */
    uint32_t m_GlobalPQStrength;
    uint32_t m_GlobalPQStrengthRange;

    GLOBAL_PQ_INDEX_T m_GlobalPQindex;

    int32_t m_GlobalPQSupport;

    int32_t m_GlobalPQStableStatus;

    unsigned int mEventFlags;

    struct {
        int sensorDebugMode;
        SensorInput sensorInput;
    } mDebug;

    map<string, ASHMEM_ENUM> m_mapPQProperty;
#ifdef AAL_SERVICE_IN_PQ
    AALService *m_aalService;
#endif
private:
    void initDefaultPQParam();

    virtual status_t readyToRun();
    virtual bool threadLoop();
    void runThreadLoop();

    unsigned int remapCcorrIndex(unsigned int ccorrindex);

    status_t enableDisplayColor(uint32_t value);
    status_t enableContentColorVideo(uint32_t value);
    status_t enableContentColor(uint32_t value);
    status_t enableSharpness(uint32_t value);
    status_t enableDynamicContrast(uint32_t value);
    status_t enableDynamicSharpness(uint32_t value);
    status_t enableDisplayGamma(uint32_t value);
    status_t enableDisplayOverDrive(uint32_t value);
    status_t enableISOAdaptiveSharpness(uint32_t value);
    status_t enableUltraResolution(uint32_t value);
    status_t enableVideoHDR(uint32_t value);
    status_t enableMdpDRE(uint32_t value);
    status_t enableMdpCCORR(uint32_t value);

    int _getLcmIndexOfGamma();
    void _setGammaIndex(int index);
    void configGamma(int picMode, const gamma_entry_t *entry);
    void configDriverGamma(const gamma_entry_t *entry);

    void setDebuggingPqparam(PQDebugFlag flag, uint32_t mode, uint32_t scenario, uint32_t value);
    bool loadPqparamTable();
    bool loadPqparamMappingCoefficient();
    bool loadPqindexTable();
    void loadTdshpIndexTable();
    void loadGammaEntryTable();
    bool loadCDparamTable();
    bool loadTRSparamTable();
    bool loadRSZTable();
    bool loadDSTable();
    bool loadDCTable();
    bool loadHDRTable();
    bool loadTDSHPTable();
    bool loadCOLORTable();
    status_t loadMetaNvGammaTable(gamma_entry_t *entry);
    status_t loadNormalNvGammaTable(gamma_entry_t *entry);
    status_t loadNvGammaTable(void);
    status_t configCcorrCoef(int32_t coefTableIdx);
    static void onALIChanged(void *obj, int32_t aliR, int32_t aliG, int32_t aliB, int32_t aliW);
    void onBacklightChanged(int32_t level_1024);

    void initBlueLight();
    void initChameleon();
    bool composeColorRegisters(void *_colorReg, const DISP_PQ_PARAM *pqparam, const unsigned int ccorr_coef[3][3], int copyFlag);
    bool composeChameleonRegisters(void *_chameleonInReg, unsigned int ccorr_coef[3][3], SensorInput sensorInput, int backlight);
    bool copyTuningField(void *_dstReg, void *_srcReg, int size);
    bool translateColorRegisters(void *TransitionReg, void *algoReg);
    bool calculatePQParamWithFilter(const PQInput &input, PQOutput *output);
    bool setPQParamWithFilter(int drvID, const PQOutput &output);
    bool getCCorrCoefByIndex(int32_t coefTableIdx, uint32_t coef[3][3]);
    bool copyCCorrCoef(uint32_t dstCoef[3][3], const uint32_t srcCoef[3][3]);
    bool configCCorrCoef(int drvID, const uint32_t coef[3][3]);
    status_t configCCorrCoefByIndex(int32_t coefTableIdx, int32_t drvID);
    void refreshDisplay();
    void dumpColorRegisters(const char *prompt, void *_colorReg);
    void calcPQStrength(void *pqparam_dst, DISP_PQ_PARAM *pqparam_src, int percentage);
    void translateToColorTuning(void *algoReg, void *tuningReg);
    void translateFromColorTuning(void *algoReg, void *tuningReg);
    int32_t getPQStrengthRatio(int scenario);
    void getUserModePQParam();
    void setPQParamlevel(DISP_PQ_PARAM *pqparam_image_ptr, int32_t index, int32_t level);
    bool isStandardPictureMode(int32_t mode, int32_t scenario_index);
    bool isVividPictureMode(int32_t mode, int32_t scenario_index);
    bool isUserDefinedPictureMode(int32_t mode, int32_t scenario_index);
    void initPQProperty(void);
    void initPQCustTable();

    bool blulightDefender(const DISP_PQ_PARAM *pqparam, void *TransitionReg, const unsigned int ccorr_coef[3][3]);
    bool chameleonDisplay(int *backlightOut, uint32_t ccorrCoefOut[3][3], int backlightIn, uint32_t ccorrCoefIn[3][3], SensorInput sensorIn);
    bool translateDrvRegisters(DISPLAY_COLOR_REG_T *drvReg, void *algoReg, int copyFlag);
    void dumpCCORRRegisters(const char *prompt, uint32_t ccorrCoefIn[3][3]);
    void dumpChameleonInRegisters(const char *prompt, void *_chameleonReg);
    void dumpChameleonOutRegisters(const char *prompt, void *_chameleonReg);
    void dumpTransitionRegisters(const char *prompt, void *_transitionReg, int dumpFlag);
    void dumpDrvColorRegisters(const char *prompt, const DISPLAY_COLOR_REG_T *colorReg);
    bool compareDrvColorRegisters(DISPLAY_COLOR_REG_T *drvReg, void *algoReg);
    bool compareDrvCCorrCoef(uint32_t dstCoef[3][3], uint32_t srcCoef[3][3]);

    bool composeConfigColorRegisters(DISPLAY_COLOR_REG_T *drvReg, const DISP_PQ_PARAM *pqparam);

    status_t enablePQEvent(bool enable);
    void setForceTransitionStep(int step);
    void ClearForceTransitionStep(void);

    status_t getGammaIndex_impl(int32_t *index);
    status_t setTuningField_impl(int32_t module, int32_t field, int32_t value);
    status_t getTuningField_impl(int32_t module, int32_t field, int32_t *value);

    void AmbientLightCTChange(double input_x, double input_y, double input_Y);
    int forceSensorDebounce(int32_t aliR, int32_t aliG, int32_t aliB, int32_t aliW);

    bool initAshmem(const unsigned int ashmemSize);

    void initColorShift();
    void ccorrMultiplyUnitGain(int32_t signedCcorrCoef[3][3]);
    bool toSignedCcorr(const uint32_t ccorrCoef[3][3], int32_t signedCcorrCoef[3][3]);
    void toUnsignedCcorr(const int32_t signedCcorrCoef[3][3], uint32_t ccorrCoef[3][3]);
    void ccorrMultiply_3x3(const int32_t sourceCcorrCoef[3][3], int32_t destCcorrCoef[3][3]);

    void initGlobalPQ(void);

    void setEvent(unsigned int event);

    bool isEventSet(unsigned int event) {
        return ((mEventFlags & event) > 0);
    }

    void clearEvents(unsigned int events = 0xffff) {
        mEventFlags &= ~events;
    }
};

extern "C" IPictureQuality* HIDL_FETCH_IPictureQuality(const char* name);

}  // namespace implementation
}  // namespace V2_3
}  // namespace pq
}  // namespace hardware
}  // namespace mediatek
}  // namespace vendor

#endif  // VENDOR_MEDIATEK_HARDWARE_PQ_V2_3_PICTUREQUALITY_H

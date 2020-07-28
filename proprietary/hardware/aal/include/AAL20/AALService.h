#ifndef __AAL_SERVICE_H__
#define __AAL_SERVICE_H__

#include <utils/threads.h>

// HAL
#include <hardware/hardware.h>
#include <hardware/lights.h>

#include <AAL20/IAALService.h>

class AALInterface;
struct AALInitReg;
struct AALInput;
struct AALOutput;

namespace android
{

enum DREMode {
    eDRE2 = 0,
    eDRE30,
    eDREALL,
};

class CustFunctions;
class AALLightSensor;
class AdaptFields;

class AALService :
        public BinderService<AALService>,
        public BnAALService,
        public Thread
{
    friend class BinderService<AALService>;
public:
    AALService();
    ~AALService();

    static char const* getServiceName() { return "AAL"; }

    // IAALServic interface
    virtual status_t setFunction(uint32_t func_bitset);
    virtual status_t setFunctionProperty(uint32_t funcFlags);
    virtual status_t setLightSensorValue(int32_t value);
    virtual status_t setScreenState(int32_t state, int32_t brightness);

    virtual status_t dump(int fd, const Vector<String16>& args);

    virtual status_t setSmartBacklightStrength(int32_t level);
    virtual status_t setSmartBacklightRange(int32_t level);
    virtual status_t setReadabilityLevel(int32_t level);
    virtual status_t setLowBLReadabilityLevel(int32_t level);
    virtual status_t getESSLEDMinStep(uint32_t *value);
    virtual status_t setESSLEDMinStep(uint32_t value);
    virtual status_t getESSOLEDMinStep(uint32_t *value);
    virtual status_t setESSOLEDMinStep(uint32_t value);
    virtual status_t getParameters(AALParameters *outParam);
    virtual status_t custInvoke(int32_t cmd, int64_t arg);
    virtual status_t readField(uint32_t field, uint32_t *value);
    virtual status_t writeField(uint32_t field, uint32_t value);

    virtual status_t setAdaptField(IAALService::AdaptFieldId field, void *data, int32_t size, uint32_t *serial);
    virtual status_t getAdaptSerial(IAALService::AdaptFieldId field, uint32_t *value);
    virtual status_t getAdaptField(IAALService::AdaptFieldId field, void *data, int32_t size, uint32_t *serial);

private:
    virtual void onFirstRef();
    virtual status_t readyToRun();
    bool initDriverRegs();
    virtual bool threadLoop();
    status_t enableAALEvent(bool enable);
    status_t debugDump(unsigned int debugLevel);
    void initAdapt();
    void deinitAdapt();

    void onBacklightChanged(int32_t level_1024);
    void onPartialUpdateChange(int32_t partialUpdateFlag);
    static void onALIChanged(void *obj, int32_t ali);
    void onESSLevelChanged(int32_t strengthIndex_cmd);
    void onESSEnableChanged(int32_t enable_cmd);
    void onDREEnableChanged(int32_t enable_cmd);
    void onPanelTypeChanged(int32_t panel_type);

    void setFunctionImpl(uint32_t funcFlags);

    bool getDREBlock(const unsigned int *pHist, const int block_x, const int block_y, AALInput *algInReg);
    bool translateDRE30InRegisters(const unsigned int *pHist, AALInput *algInReg);
    bool writeBlock(const AALOutput *algOutReg, const int block_x, const int block_y, const int dre_blk_x_num, unsigned int *pGain);
    bool writeCurve16(const AALOutput *algOutReg, const int dre_blk_x_num, const int dre_blk_y_num, unsigned int *pGain);
    bool translateDRE30OutRegisters(const AALOutput *algOutReg, const int dre_blk_x_num, const int dre_blk_y_num, unsigned int *pGain);
    void initDRE30Output(AALOutput *outParam);

     // hardware
    light_device_t *mLight;
    int mDispFd;

    mutable Mutex mLock;
    bool mEventEnabled;
    volatile bool mToEnableEvent;
    volatile bool mUseExternalAli;

    int mScrWidth;
    int mScrHeight;
    int mALI;
    int mPmsScrState; // Screen state of power manager
    int mPrevScrState;
    int mCurrScrState;
    int mBacklight;
    int mTargetBacklight;
    int mLongTermBacklight;
    int mOutBacklight;
    int mOutCabcGain;
    unsigned int mPrevFuncFlags;
    unsigned int mFuncFlags; // bit-set of AALFunction
    int mPrevIsPartialUpdate;
    int mIsPartialUpdate; // bit-set of AALFunction
    int mSupportEssLevelCtlByKernel;
    int mSupportDreEnableCtlByKernel;
    int mInitSmartBacklightStrength;
    int mSupportEssLevelRemapping;
    int mESSLevelMappingTable[17] = {
        0, 16, 32, 48, 64, 80, 96, 112,
        128, 144, 160, 176, 192, 208, 224, 240,
        255};
    int mDreEnCommandId;
    int mEssEnCommandId;
    int mEssLevelCommandId;

    int mUserBrightness;
    bool mBacklightInitFlag;

    // Store in member variable to debug
    AALInitReg *mAALInitReg;
    AALInput *mAALInput;
    AALOutput *mAALOutput;
    AALInterface *mAALInterface;

    CustFunctions *mCustFunc;
    AALLightSensor *mLightSensor;
    AdaptFields *mAdaptFields;

    unsigned int mDebugLevel;
    struct {
        bool panelOn;
        bool panelDisplayed;
        bool alwaysEnable;
        int overwriteALI;
        int overwriteBacklight;
        int overwriteCABC;
        int overwriteDRE;
        int overwritePartial;
        int overwriteRefreshLatency;
        int overwriteDREBlockX;
        int overwriteDREBlockY;
    } mDebug;

    unsigned int mEventFlags;

    DREMode m_dre_mode;
    unsigned int m_cabc_hw;

    bool initDRE30SW(unsigned long dre30_hist_addr);

    void setEvent(unsigned int event) {
        mEventFlags |= event;
    }

    bool isEventSet(unsigned int event) {
        return ((mEventFlags & event) > 0);
    }

    void clearEvents(unsigned int events = 0xffff) {
        mEventFlags &= ~events;
    }

    void unitTest();
};
};

#endif

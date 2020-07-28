#ifndef __AAL_SERVICE_H__
#define __AAL_SERVICE_H__

#include <utils/threads.h>
#include <vector>
#include <iostream>

class CustFunctions;
class AALLightSensor;
class AALInterface;
struct AALInitReg;
struct AALInput;
struct AALOutput;

using ::android::Thread;
using ::android::Mutex;
using ::android::status_t;

enum DREMode {
    eDRE2 = 0,
    eDRE30,
    eDREALL,
};

class AALService :
        public Thread
{
public:
    AALService();
    ~AALService();

    // AALServic interface
    status_t setFunction(uint32_t func_bitset);
    status_t setFunctionProperty(uint32_t funcFlags);
    status_t setScreenState(int32_t state, int32_t brightness);
    status_t setSmartBacklightStrength(int32_t level);
    status_t setSmartBacklightRange(int32_t level);
    status_t setReadabilityLevel(int32_t level);
    status_t setLowBLReadabilityLevel(int32_t level);
    status_t getESSLEDMinStep(uint32_t *value);
    status_t setESSLEDMinStep(uint32_t value);
    status_t getESSOLEDMinStep(uint32_t *value);
    status_t setESSOLEDMinStep(uint32_t value);
    status_t custInvoke(int32_t cmd, int64_t arg);
    status_t readField(uint32_t field, uint32_t *value);
    status_t writeField(uint32_t field, uint32_t value);

    status_t dump(const std::vector<std::string>& args, std::string &result);

private:
    virtual status_t readyToRun();
    virtual bool threadLoop();
    void runThreadLoop();
    bool initDriverRegs();
    status_t enableAALEvent(bool enable);
    status_t debugDump(unsigned int debugLevel);

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
    int mDispFd;

    mutable Mutex mLock;
    bool mEventEnabled;
    volatile bool mToEnableEvent;

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

#endif

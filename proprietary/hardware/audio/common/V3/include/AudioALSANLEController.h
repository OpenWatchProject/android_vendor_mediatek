#ifndef ANDROID_AUDIO_ALSA_NLE_CONTROLLER_H
#define ANDROID_AUDIO_ALSA_NLE_CONTROLLER_H

#include "AudioType.h"
#include "AudioUtility.h"
#include <AudioLock.h>
#include <tinyalsa/asoundlib.h>
#include "AudioALSAPlaybackHandlerBase.h"
#ifdef MTK_HYBRID_NLE_SUPPORT
//#undef MTK_HYBRID_NLE_SUPPORT
#endif

namespace android {

class AudioALSAPlaybackHandlerBase;

#define NLE_SUPPORT_CH_NUMBER_MAX (2)
#define NLE_SUPPORT_SAMPLING_RATE_MAX (192000)
#define NLE_SUPPORT_PREVIEW_MS_MIN  (25) // ms
#define NLE_SUPPORT_HARDWARE_NUM (1)
#define NLE_SKIP_FRAME_COUNT    (0)
#define NLE_24BIT_NAGATIVE_MAX (8388608)
#define NLE_24BIT_POSITIVE_MAX (8388607)
#define NLE_16BIT_NAGATIVE_MAX (32768)
#define NLE_16BIT_POSITIVE_MAX (32767)
#define NLE_GAIN_UNININIAL_VALUE  (10000)
// #define NLE_HW_QUEUE_EXTRA_FRAMECOUNT (16)    //extra check SPL
// #define NLE_HW_QUEUE_SKIP_ZCE_FRAMECOUNT (NLE_HW_QUEUE_EXTRA_FRAMECOUNT+16)    // reduce check ZCE
#define NLE_HW_QUEUE_EXTRA_FRAMECOUNT_MS (4)    //extra check SPL
#define NLE_HW_QUEUE_SKIP_ZCE_FRAMECOUNT_MS (NLE_HW_QUEUE_EXTRA_FRAMECOUNT_MS+2)    // reduce check ZCE
#define NLE_HW_QUEUE_BUFFER_BEFORE_DAC_FRAMECOUNT (16)
#define NLE_WAIT_NLE_BYPASS_MAX_MS (21 + NLE_HW_QUEUE_SKIP_ZCE_FRAMECOUNT_MS - NLE_HW_QUEUE_EXTRA_FRAMECOUNT_MS)
#define NLE_GAIN_STEP_MAX_DB    (2) // greater than 0, don't 0

#define NLE_AFE_RGS_NLE_R_CFG0  (0)
#define NLE_AFE_RGS_NLE_R_CFG1  (1)
#define NLE_AFE_RGS_NLE_R_CFG2  (2)
#define NLE_AFE_RGS_NLE_R_CFG3  (3)

#define NLE_AFE_RGS_NLE_L_CFG0  (4)
#define NLE_AFE_RGS_NLE_L_CFG1  (5)
#define NLE_AFE_RGS_NLE_L_CFG2  (6)
#define NLE_AFE_RGS_NLE_L_CFG3  (7)
#define NLE_AFE_RGS_NLE_MAX  (8)


#define NLE_AFE_DL_NLE_R_CFG0   (0)
#define NLE_AFE_DL_NLE_R_CFG1   (1)
#define NLE_AFE_DL_NLE_R_CFG2   (2)
#define NLE_AFE_DL_NLE_R_CFG3   (3)

#define NLE_AFE_DL_NLE_L_CFG0   (4)
#define NLE_AFE_DL_NLE_L_CFG1   (5)
#define NLE_AFE_DL_NLE_L_CFG2   (6)
#define NLE_AFE_DL_NLE_L_CFG3   (7)
#define NLE_AFE_DL_NLE_MAX  (8)



#define nle_r_clip_overflow_mask (1<<15)
#define nle_r_clip_overflow_len (1)
#define nle_r_gain_dig_cur_mask (1<<8)
#define nle_r_gain_dig_cur_len (6)
#define nle_r_anc_mask_mask (1<<7)
#define nle_r_anc_mask_len (1)
#define nle_r_gain_ana_cur_mask (1<<0)
#define nle_r_gain_ana_cur_len (6)

#define nle_r_gain_dig_tar_cur_mask (1<<8)
#define nle_r_gain_dig_tar_cur_len (6)
#define nle_r_gain_ana_tar_cur_mask (1<<0)
#define nle_r_gain_ana_tar_cur_len (6)

#define nle_r_state_cur_mask (1<<12)
#define nle_r_state_cur_len (3)
#define nle_r_gain_step_cur_mask (1<<8)
#define nle_r_gain_step_cur_len (4)
#define nle_r_toggle_num_cur_mask (1<<0)
#define nle_r_toggle_num_cur_len (6)

#define nle_r_dig_gain_targeted_mask (1<<15)
#define nle_r_dig_gain_targeted_len (1)
#define nle_r_dig_gain_increase_mask (1<<14)
#define nle_r_dig_gain_increase_len (1
#define nle_r_dig_gain_decrease_mask (1<<13)
#define nle_r_dig_gain_decrease_len (1)
#define nle_r_ana_gain_targeted_mask (1<<12)
#define nle_r_ana_gain_targeted_len (1)
#define nle_r_ana_gain_increase_mask (1<<11)
#define nle_r_ana_gain_increase_len (1)
#define nle_r_ana_gain_decrease_mask (1<<10)
#define nle_r_ana_gain_decrease_len (1)
#define nle_r_time_counter_cur_mask (1<<0)
#define nle_r_time_counter_cur_len (9)

#define nle_l_clip_overflow_mask (1<<15)
#define nle_l_clip_overflow_len (1)
#define nle_l_gain_dig_cur_mask (1<<8)
#define nle_l_gain_dig_cur_len (6)
#define nle_l_anc_mask_mask (1<<7)
#define nle_l_anc_mask_len (1)
#define nle_l_gain_ana_cur_mask (1<<0)
#define nle_l_gain_ana_cur_len (6)

#define nle_l_gain_dig_tar_cur_mask (1<<8)
#define nle_l_gain_dig_tar_cur_len (6)
#define nle_l_gain_ana_tar_cur_mask (1<<0)
#define nle_l_gain_ana_tar_cur_len (6)

#define nle_l_state_cur_mask (1<<12)
#define nle_l_state_cur_len (3)
#define nle_l_gain_step_cur_mask (1<<8)
#define nle_l_gain_step_cur_len (4)
#define nle_l_toggle_num_cur_mask (1<<0)
#define nle_l_toggle_num_cur_len (6)

#define nle_l_dig_gain_targeted_mask (1<<15)
#define nle_l_dig_gain_targeted_len (1)
#define nle_l_dig_gain_increase_mask (1<<14)
#define nle_l_dig_gain_increase_len (1
#define nle_l_dig_gain_decrease_mask (1<<13)
#define nle_l_dig_gain_decrease_len (1)
#define nle_l_ana_gain_targeted_mask (1<<12)
#define nle_l_ana_gain_targeted_len (1)
#define nle_l_ana_gain_increase_mask (1<<11)
#define nle_l_ana_gain_increase_len (1)
#define nle_l_ana_gain_decrease_mask (1<<10)
#define nle_l_ana_gain_decrease_len (1)
#define nle_l_time_counter_cur_mask (1<<0)
#define nle_l_time_counter_cur_len (9)

#define DIG_GAIN_TARGETED   (0)
#define DIG_GAIN_INCREASE   (1)
#define DIG_GAIN_DECREASE   (2)
#define ANA_GAIN_TARGETED   (3)
#define ANA_GAIN_INCREASE   (4)
#define ANA_GAIN_DECREASE   (5)
#define TIME_COUNTER_CUR_MSB    (6)
#define TIME_COUNTER_CUR_LSB    (7)

#define DIG_GAIN_CUR    (8)
#define ANA_GAIN_CUR    (9)



typedef enum {
    AUDIO_NLE_STATE_NONE,
    AUDIO_NLE_STATE_INITIALIZED,
    AUDIO_NLE_STATE_FIRST_WRITE,
    AUDIO_NLE_STATE_RUNNING,
} audio_nle_state;

#if 0
typedef enum {
    AUDIO_NLE_CHANNEL_L,
    AUDIO_NLE_CHANNEL_R,
} audio_nle_channel;

typedef struct {
    int32_t mLGainNle;
    int32_t mLGainHP;
    int32_t mLDbpPerStep;
    int32_t mLStepPerZC;

    int32_t mRGainNle;
    int32_t mRGainHP;
    int32_t mRDbpPerStep;
    int32_t mRStepPerZC;
} HwParam_t;

class AudioALSANLEController {
public:
    AudioALSANLEController(const stream_attribute_t *pStream_Attribute_Source); //Input attribute for NLE, however it is the same attribute for writing to HW
    ~AudioALSANLEController();
    status_t init(playback_handler_t ePlaybackHandlerType);     // allocate memory/resource if success
    status_t deinit(void);                                  // deallocate memory/resource if success
    size_t process(void *buffer, size_t Byte, time_info_struct_t *pHWBuffer_Time_Info);          // return the size of data which write to NLE SW module
private:

    status_t checkValidForInit(playback_handler_t ePlaybackHandlerType);
    status_t allocateResource(void);
    status_t freeResource(void);
    size_t writeToEachChannel(void *buffer, size_t Byte);
    status_t gainAllocatePlan(float Spl, size_t ZeroCnt, audio_nle_channel eChannel);
    status_t checkAndSetGain(size_t zuframecount);
    status_t getMaxSPLandZeroCnt(RingBuf *pRingBuf, size_t zuframecount, float *pSPL_Db, size_t *pZeroCnt, uint32_t maxValidValue);
    status_t applySetting2Hw(HwParam_t *pHwParam);
    stream_attribute_t mStreamAttribute;
    playback_handler_t mAattachPlaybackHandlerType;
    audio_nle_state mState;
    size_t mBytePerSample;
    size_t mMinPreviewByte;
    size_t mHwBufTotalFrame;
    RingBuf mLBuf;
    RingBuf mRBuf;
    AudioLock mLock;
    static uint32_t mUseHWNum;
    size_t mUsedFrameCount;
    size_t mMinPreviewFrameCount;
    HwParam_t mCurHwParam;
    HwParam_t mPrevHwParam;
    struct mixer *mMixer;
};
#endif
typedef enum {
    AUDIO_NLE_CHANNEL_L = 0,
    AUDIO_NLE_CHANNEL_R,
} audio_nle_channel;

typedef struct {
    int32_t mGainNle;
    int32_t mGainHP;
    int32_t mDbpPerStep;
    int32_t mStepPerZC;
} NleHwParam_t;

typedef struct {
    NleHwParam_t stCur;
    NleHwParam_t stPrev;
    int32_t mMaxSPL_Db;
    uint32_t mMaxValue;
    size_t mZeroEventCount;
    size_t mLookUpFrameCount;
    audio_nle_channel mChannel;
} NleHwInfo_t;

typedef enum {
    AUDIO_HYBRID_NLE_MNG_STATE_DISABLE = 0, // NLE HW module disable
    AUDIO_HYBRID_NLE_MNG_STATE_BYBASS,  // NLE HW module bypass signal, always keep digital gain at 0 dB
    AUDIO_HYBRID_NLE_MNG_STATE_RUNNING, // NLE HW module run with normal mode
} audio_hybridnlemng_status;

typedef enum {
    AUDIO_STREAM_HW_FM = 1 << 0,
    AUDIO_STREAM_OTHER = 1 << 1,
} audio_stream_hw_path;

typedef enum {
    NLE_ADJUST_GAIN_NORMAL = 0,
    NLE_ADJUST_GAIN_RAMP_TO_BYPASS,
    NLE_ADJUST_GAIN_MAX,
} audio_hybrid_adjust_mode;

class AudioALSANLECtrl {
public:
    AudioALSANLECtrl(playback_handler_t ePlaybackHandlerType, const stream_attribute_t *pStream_Attribute_Source); //Input attribute for NLE, however it is the same attribute for writing to HW
    ~AudioALSANLECtrl();
    status_t init(AudioALSAPlaybackHandlerBase *pPlaybackHandler);     // allocate memory/resource if success
    status_t deinit(void);                                  // deallocate memory/resource if success
    size_t process(void *buffer, size_t Byte);          // return the size of data which write to NLE SW module
    status_t getMaxSPLandZeroCnt(NleHwInfo_t *pNleHwInfo, RingBuf *pRingBuf, size_t zuframecount, float *pSPL_Db, size_t *pZeroCnt, uint32_t maxValidValue);
    status_t setMaxSPLintoSpecificPos(size_t zuOffsetForTail);
    RingBuf mLBuf;
    RingBuf mRBuf;
    size_t mMinPreviewFrameCount;
    size_t mCheckHwExtraFrameCnt;
    size_t mSkipHwZCEFrameCnt;
    size_t mPlaybackedFrameCnt;
    size_t mRamp2ZeroFrameCnt;
    size_t mBytePerSample;
    AudioALSAPlaybackHandlerBase *mAudioALSAPlaybackHandlerBase;
private:
    status_t checkValidForInit(playback_handler_t ePlaybackHandlerType);
    status_t allocateResource(void);
    status_t freeResource(void);
    size_t writeToEachChannel(void *buffer, size_t Byte);
    stream_attribute_t mStreamAttribute;
    playback_handler_t mAattachPlaybackHandlerType;
    audio_nle_state mState;
    size_t mMinPreviewByte;
    size_t mHwBufTotalFrame;
    AudioLock mLock;
};

class AudioALSAHyBridNLEManager {
public:
    AudioALSAHyBridNLEManager();
    static AudioALSAHyBridNLEManager *getInstance();
    // hpMaxGainDb: 12~-32 Db or -128 = muted
    status_t setNleHwConfigByIndex(size_t hpMaxGainIdx);
    status_t getNleHwConfigByDb(int *hpMaxGainDb);
    status_t getNleHwConfigByIndex(size_t *hpMaxGainIdx);
    audio_hybridnlemng_status getStatus(void);
    status_t initPlayBackHandler(playback_handler_t ePlaybackHandlerType, stream_attribute_t *pStreamAttribute, AudioALSAPlaybackHandlerBase *pPlaybackHandler);
    status_t deinitPlayBackHandler(playback_handler_t ePlaybackHandlerType);
    status_t process(playback_handler_t ePlaybackHandlerType, void *buffer, size_t Byte);
    status_t addHwPathStream(audio_stream_hw_path eHwPath);
    status_t removeHwPathStream(audio_stream_hw_path eHwPath);
    status_t setAudioMode(audio_mode_t eMode);
    status_t setBypassNLE(bool bBypass);
    bool getBypassNLE(void);
    status_t setEnableNLE(bool bEnable);
    status_t setNleEopDb(int eopGainDb); //Gain change
    int getNleEopDb(void);
    status_t dump(void);
    static uint32_t getSupportRunNLEHandler(void);
    static status_t setSupportRunNLEHandler(playback_handler_t eHandler);
private:
    status_t setNleHwConfigByDb(int hpMaxGainDb, int eopGainDb); //Gain change
    status_t updateCurStatus(void);
    status_t enableNleHw(void);
    status_t disableNleHw(void);
    status_t ramp2DigitalGainZero(void);
    bool checkIfGainTargeted(void);
    status_t waitForBypass(audio_hybridnlemng_status ePrevStatus, audio_hybridnlemng_status eCurStatus);
    status_t doAdjustGainProcess(AudioALSANLECtrl *pCurNLECtrl, size_t zuCheckOffsetFrameCnt, audio_hybrid_adjust_mode eAdjustMode);
    status_t gainAllocatePlan(float Spl, size_t ZeroCnt, size_t Zuframecount, NleHwInfo_t *pNleHwInfo);
    status_t checkAndSetGain(size_t zuframecount, AudioALSANLECtrl *pNleCtrl, NleHwInfo_t *pNleHwInfo, RingBuf *pRingBuf);
    status_t applyGain2Hw(NleHwInfo_t *pNleHwInfo);
    status_t resetDefaultGain(NleHwInfo_t *pNleHwInfo);
    status_t getGstepAndGnum(int32_t SPL_diff, size_t ZeroCnt, int32_t *pdGstep, int32_t *pdGnum);
    AudioLock mLock;
    static AudioALSAHyBridNLEManager *mAudioALSAHyBridNREManager; // singleton
    status_t getDbFromAnalogIdx(size_t AnalogIdx, int8_t *pDb);
    status_t getAnalogIdx(int8_t Db, size_t *pAnalogIdx);
    status_t getDbFromDigitalIdx(size_t DigitalIdx, int8_t *pDb);
    status_t getDigitalIdx(int8_t Db, size_t *pDigitalIdx);
    void showRegInfo(void);
    int32_t mGainHP_Db_Max;
    int32_t mGainHP_Db_Min;
    int32_t mGainNLE_Db_Max;
    int32_t mGainNLE_Db_Min;
    int32_t mGainEop_Db;
    int32_t mGainSop_Db;
    audio_hybridnlemng_status mStatus;
    playback_handler_t mActivePBHandler;
    uint32_t mActivePBHandlerBitwise;
    uint32_t mInitPBHandlerBitwise;
    NleHwInfo_t mLNleHwInfo;
    NleHwInfo_t mRNleHwInfo;
    audio_mode_t mMode;
    AudioALSANLECtrl *mNleCtrlOfPlaybackHandler[PLAYBACK_HANDLER_MAX];
    uint32_t mNleCtrlOfPlaybackHandlerCounter;
    uint32_t mHwPathStreamCounter;
    bool mBypassNLE;    // Mean fix digital gain to 0, and fix analog gain to User gain
    bool mNleSwitch;    // Turn on/off NLE Hw module, before turn off make sure fix digital gain to 0, and fix analog gain to User gain
    struct mixer *mMixer;
    int8_t mHwAnalogGainMaxDb;
    int8_t mHwAnalogGainMinDb;
    size_t mHwAnalogGainMinValue;
    int8_t mHwAnalogGainMuteDb;
    size_t mHwAnalogGainMuteValue;
    int8_t mHwDigitalGainMaxDb;
    size_t mHwDigitalGainMaxValue;
    int8_t mHwDigitalgGainMinDb;
    uint8_t mHwAnalogDelayTick;
    bool mHwSupport;
    bool mSetEopByProp;
    bool mForceTurnOffNLE;
    uint32_t mMax24BitValidValue;
    uint32_t mMax16BitValidValue;
    static uint32_t mSupportRunNLEHandlerBitwise;
    // uint32_t mNLE_AFE_RGS_NLE[NLE_AFE_RGS_NLE_MAX];
    // uint32_t mNLE_AFE_DL_NLE[NLE_AFE_DL_NLE_MAX];
};

}

#endif

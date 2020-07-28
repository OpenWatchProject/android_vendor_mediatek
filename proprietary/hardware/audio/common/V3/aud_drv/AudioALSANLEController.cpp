#define LOG_TAG "AudioALSANLEController"
#ifdef CONFIG_MT_ENG_BUILD
#define LOG_NDEBUG 0 //enable and then ALOGV will print
#endif
#include "AudioALSANLEController.h"
#include <stdlib.h> //for math
#include <math.h> //for log
#include "AudioALSADriverUtility.h"
#include <cutils/properties.h>

// #define VERY_VERBOSE_LOGGING
#define ENABLE_NLE_HW_CONTROL   // off when soft verify only

#ifdef VERY_VERBOSE_LOGGING
#define ALOGVV ALOGV
#else
#define ALOGVV(a...) do { } while(0)
#endif

// By user
#define NLE_GAIN_HP_DB_USER (3)
#define NLE_GAIN_HP_INDEX_USER  (9)
#define NLE_GAIN_EOP_DB (-40)

// Spec of NLE
#define NLE_GAIN_HP_DB_MAX  (12)
#define NLE_GAIN_HP_DB_MIN  (-32)
#define NLE_GAIN_NLE_DB_MAX (44)
#define NLE_GAIN_NLE_DB_MIN (0)
#define NLE_GAIN_NLE_VALUE_MAX (44)
#define NLE_DELAY_ANA_OUTPUT_T  (0x0D)  // Reg is 0x0C

#define NLE_SUPPORT_ANA_GAIN_MUTE_DB (-40)
#define NLE_SUPPORT_ANA_GAIN_MUTE_VALUE (0x3F)
#define NLE_GAIN_HP_VALUE_MIN  (0x2C)

#if 0
//Digital  0~30 dB
//Analog   8~-22 dB
// By user
#define NLE_GAIN_HP_DB_USER (3)
#define NLE_GAIN_HP_INDEX_USER  (5)
#define NLE_GAIN_EOP_DB (-40)

// Spec of NLE
#define NLE_GAIN_HP_DB_MAX  (8)
#define NLE_GAIN_HP_DB_MIN  (-22)
#define NLE_GAIN_NLE_DB_MAX (30)
#define NLE_GAIN_NLE_DB_MIN (0)
#define NLE_GAIN_NLE_VALUE_MAX (30)
#define NLE_DELAY_ANA_OUTPUT_T  (0x0D)  // Reg is 0x0C

#define NLE_SUPPORT_ANA_GAIN_MUTE_DB (-40)
#define NLE_SUPPORT_ANA_GAIN_MUTE_VALUE (0x3F)
#define NLE_GAIN_HP_VALUE_MIN  (0x1E)
#endif

namespace android {
/*==============================================================================
 *                     Singleton Pattern
 *============================================================================*/

AudioALSAHyBridNLEManager *AudioALSAHyBridNLEManager::mAudioALSAHyBridNREManager = NULL;
uint32_t AudioALSAHyBridNLEManager::mSupportRunNLEHandlerBitwise = (1 << PLAYBACK_HANDLER_NORMAL); // | (1 << PLAYBACK_HANDLER_DEEP_BUFFER);

uint32_t find_bit_pos(uint32_t val) {
    uint32_t bit_pos, i;
    for (i = 1, bit_pos = 1; i != 0 && i <= val; i <<= 1, bit_pos++) {
        if (val & i) {
            return bit_pos;
        }
    }
    return 0;
}

AudioALSAHyBridNLEManager *AudioALSAHyBridNLEManager::getInstance() {
    static AudioLock mGetInstanceLock;
    AL_AUTOLOCK(mGetInstanceLock);

    if (mAudioALSAHyBridNREManager == NULL) {
        mAudioALSAHyBridNREManager = new AudioALSAHyBridNLEManager();
    }
    ASSERT(mAudioALSAHyBridNREManager != NULL);
    return mAudioALSAHyBridNREManager;
}

#if defined(MTK_HYBRID_NLE_SUPPORT)
AudioALSAHyBridNLEManager::AudioALSAHyBridNLEManager() {
    int property_forceoff;
    char property_value[PROPERTY_VALUE_MAX];
    property_get("persist.vendor.audio.nle.eop", property_value, "0");
    int property_eop = atoi(property_value);
    property_get("persist.vendor.audio.nle.forceoff", property_value, "0");
    property_forceoff = atoi(property_value);
    mHwSupport = false;
    mMixer = NULL;
    mMixer = AudioALSADriverUtility::getInstance()->getMixer();
    ASSERT(mMixer != NULL);
#ifdef ENABLE_NLE_HW_CONTROL
    const unsigned int num_values = 20;
    struct mixer_ctl *ctl;
    signed char HwCapability[num_values];
    ctl = mixer_get_ctl_by_name(mMixer, "Audio_HyBridNLE_HwCapability");
    if (NULL == ctl) {
        ALOGE("Err for get Audio_HyBridNLE_HwCapability");
    }
    memset((void *)HwCapability, 0, sizeof(signed char)*num_values);
    if (mixer_ctl_get_array(ctl, (void *)HwCapability, num_values)) {
        ALOGE("Error for HwCapability");
    } else {
        ALOGD("Hw Config from kernel");
        mHwSupport = true;
        mHwAnalogGainMaxDb = (HwCapability[0] == 0) ? (int8_t) HwCapability[1] : ((int8_t) HwCapability[1]) * (-1);
        mHwAnalogGainMinDb = (HwCapability[2] == 0) ? (int8_t) HwCapability[3] : ((int8_t) HwCapability[3]) * (-1);
        mHwAnalogGainMinValue = (HwCapability[4] == 0) ? (size_t) HwCapability[5] : ((size_t) HwCapability[5]) * (-1);
        mHwAnalogGainMuteDb = (HwCapability[6] == 0) ? (int8_t) HwCapability[7] : ((int8_t) HwCapability[7]) * (-1);
        mHwAnalogGainMuteValue = (HwCapability[8] == 0) ? (size_t) HwCapability[9] : ((size_t) HwCapability[9]) * (-1);
        mHwDigitalGainMaxDb = (HwCapability[10] == 0) ? (int8_t) HwCapability[11] : ((int8_t) HwCapability[11]) * (-1);
        mHwDigitalGainMaxValue = (HwCapability[12] == 0) ? (size_t) HwCapability[13] : ((size_t) HwCapability[13]) * (-1);
        mHwDigitalgGainMinDb = (HwCapability[14] == 0) ? (size_t) HwCapability[15] : ((size_t) HwCapability[15]) * (-1);
        mHwAnalogDelayTick = (uint8_t) HwCapability[17];
    }
#endif
    if (mHwSupport == false) {
        ALOGD("Hw Config from user");
        mHwAnalogGainMaxDb = NLE_GAIN_HP_DB_MAX;
        mHwAnalogGainMinDb = NLE_GAIN_HP_DB_MIN;
        mHwAnalogGainMinValue = NLE_GAIN_HP_VALUE_MIN;
        mHwAnalogGainMuteDb = NLE_SUPPORT_ANA_GAIN_MUTE_DB;
        mHwAnalogGainMuteValue = NLE_SUPPORT_ANA_GAIN_MUTE_VALUE;
        mHwDigitalGainMaxDb = NLE_GAIN_NLE_DB_MAX;
        mHwDigitalGainMaxValue = NLE_GAIN_NLE_VALUE_MAX;
        mHwDigitalgGainMinDb = NLE_GAIN_NLE_DB_MIN;
        mHwAnalogDelayTick = NLE_DELAY_ANA_OUTPUT_T;
    }
    ALOGD("mHwAnalogGainMaxDb = %d", mHwAnalogGainMaxDb);
    ALOGD("mHwAnalogGainMinDb = %d", mHwAnalogGainMinDb);
    ALOGD("mHwAnalogGainMinValue = %zu", mHwAnalogGainMinValue);
    ALOGD("mHwAnalogGainMuteDb = %d", mHwAnalogGainMuteDb);
    ALOGD("mHwAnalogGainMuteValue = %zu", mHwAnalogGainMuteValue);
    ALOGD("mHwDigitalGainMaxDb = %d", mHwDigitalGainMaxDb);
    ALOGD("mHwDigitalGainMaxValue = %zu", mHwDigitalGainMaxValue);
    ALOGD("mHwDigitalgGainMinDb = %d", mHwDigitalgGainMinDb);
    ALOGD("mHwAnalogDelayTick = %d", mHwAnalogDelayTick);

    mGainHP_Db_Max = NLE_GAIN_HP_DB_USER;
    mGainHP_Db_Min = (int32_t) mHwAnalogGainMinDb;
    mGainNLE_Db_Min = (int32_t) mHwDigitalgGainMinDb;
    if (property_eop < 0) {
        mGainEop_Db = property_eop;
        mSetEopByProp = true;
    } else {
        mGainEop_Db = NLE_GAIN_EOP_DB;
        mSetEopByProp = false;
    }
    mGainNLE_Db_Max = mGainHP_Db_Max - mGainHP_Db_Min;
    mGainSop_Db = mGainEop_Db - mGainNLE_Db_Max;
#if 0
    mLNleHwInfo.stPrev.mGainNle = mRNleHwInfo.stPrev.mGainNle = NLE_GAIN_UNININIAL_VALUE;
    mLNleHwInfo.stPrev.mGainHP = mRNleHwInfo.stPrev.mGainHP = NLE_GAIN_UNININIAL_VALUE;
    mLNleHwInfo.stCur.mGainNle = mRNleHwInfo.stCur.mGainNle = mGainNLE_Db_Min;
    mLNleHwInfo.stCur.mGainHP = mRNleHwInfo.stCur.mGainHP = mGainHP_Db_Max;
    mLNleHwInfo.stPrev.mDbpPerStep = mLNleHwInfo.stCur.mDbpPerStep = mRNleHwInfo.stPrev.mDbpPerStep = mRNleHwInfo.stCur.mDbpPerStep = 0;
    mLNleHwInfo.stPrev.mStepPerZC = mLNleHwInfo.stCur.mStepPerZC = mRNleHwInfo.stPrev.mStepPerZC = mRNleHwInfo.stCur.mStepPerZC = 0;
#endif
    mStatus = AUDIO_HYBRID_NLE_MNG_STATE_DISABLE;
    mActivePBHandler = PLAYBACK_HANDLER_BASE;
    mActivePBHandlerBitwise = 0;
    mInitPBHandlerBitwise = 0;
    mLNleHwInfo.mChannel = AUDIO_NLE_CHANNEL_L;
    resetDefaultGain(&mLNleHwInfo);
    mRNleHwInfo.mChannel = AUDIO_NLE_CHANNEL_R;
    resetDefaultGain(&mRNleHwInfo);
    mMode = AUDIO_MODE_NORMAL;
    memset(&mNleCtrlOfPlaybackHandler[0], 0, sizeof(AudioALSANLECtrl *)*PLAYBACK_HANDLER_MAX);
    mNleCtrlOfPlaybackHandlerCounter = 0;
    mHwPathStreamCounter = 0;
    mBypassNLE = false;
    mNleSwitch = false;
    if (property_forceoff > 0) {
        mForceTurnOffNLE = true;
    } else {
        mForceTurnOffNLE = false;
    }
    // showRegInfo();
    double mfloatValue = pow(10, (mGainEop_Db / 20.0));
    mMax24BitValidValue = (uint32_t)(NLE_24BIT_POSITIVE_MAX * mfloatValue);
    mMax16BitValidValue = (uint32_t)(NLE_16BIT_POSITIVE_MAX * mfloatValue);
    ALOGD("mMax24BitValidValue %d, mMax16BitValidValue %d", mMax24BitValidValue, mMax16BitValidValue);
    ALOGD("[Init] %s %d", __FUNCTION__, __LINE__);
}

status_t AudioALSAHyBridNLEManager::setNleHwConfigByDb(int hpMaxGainDb, int eopGainDb) {
    status_t dRet = NO_ERROR;
    int32_t mPrevGainEop_Db = mGainEop_Db;

    if (hpMaxGainDb < mGainHP_Db_Min) {
        ALOGE("[Err] mGainHP_Db_Min > hpMaxGainDb, [%d] > [%d]", mGainHP_Db_Min, hpMaxGainDb);
        return BAD_VALUE;
    }

    if (hpMaxGainDb > mHwAnalogGainMaxDb || (hpMaxGainDb < mHwAnalogGainMinDb && hpMaxGainDb != mHwAnalogGainMuteDb)) {
        ALOGE("[Err] hpMaxGainDb [%d] should be not > [%d] or < [%d]", hpMaxGainDb, mHwAnalogGainMaxDb, mHwAnalogGainMinDb);
        return BAD_VALUE;
    }

    if (eopGainDb > 0 || eopGainDb < -96) {
        ALOGE("[Err] eopGainDb > 0 || < -96, [%d]", eopGainDb);
        return BAD_VALUE;
    }

    AL_AUTOLOCK(mLock);
    mGainHP_Db_Max = hpMaxGainDb;
    if (!mSetEopByProp) {
        mGainEop_Db = eopGainDb;
    }
    if (mGainHP_Db_Max == mHwAnalogGainMuteDb) {
        mGainNLE_Db_Max = 0;
    } else {
        mGainNLE_Db_Max = mGainHP_Db_Max - mGainHP_Db_Min;
    }
    mGainSop_Db = mGainEop_Db - mGainNLE_Db_Max;
    if (mPrevGainEop_Db != mGainEop_Db) {
        double mfloatValue = pow(10, (mGainEop_Db / 20.0));
        mMax24BitValidValue = (uint32_t)(NLE_24BIT_POSITIVE_MAX * mfloatValue);
        mMax16BitValidValue = (uint32_t)(NLE_16BIT_POSITIVE_MAX * mfloatValue);
        ALOGD("mMax24BitValidValue %d, mMax16BitValidValue %d", mMax24BitValidValue, mMax16BitValidValue);
    }

    if (mStatus == AUDIO_HYBRID_NLE_MNG_STATE_RUNNING) {
        if (mSupportRunNLEHandlerBitwise & mActivePBHandlerBitwise) {
            AudioALSANLECtrl *pCurNLECtrl = mNleCtrlOfPlaybackHandler[mActivePBHandler];
            dRet = doAdjustGainProcess(pCurNLECtrl, pCurNLECtrl->mCheckHwExtraFrameCnt, NLE_ADJUST_GAIN_NORMAL);
        }
    } else if (mStatus == AUDIO_HYBRID_NLE_MNG_STATE_BYBASS) {
        dRet = ramp2DigitalGainZero();
    } else {
        dRet = FAILED_TRANSACTION;
    }
    return dRet;
}

status_t AudioALSAHyBridNLEManager::setNleHwConfigByIndex(size_t hpMaxGainIdx) {
    status_t dRet = NO_ERROR;
    int8_t bDb;
    dRet = getDbFromAnalogIdx(hpMaxGainIdx, &bDb);
    ALOGV("%s Idx %zu, dB %d, dRet %d", __FUNCTION__, hpMaxGainIdx, bDb, dRet);
    if (dRet == NO_ERROR) {
        int dDb = (int) bDb;
        dRet = setNleHwConfigByDb(dDb, mGainEop_Db);
    }
    return dRet;
}

status_t AudioALSAHyBridNLEManager::getNleHwConfigByDb(int *hpMaxGainDb) {
    ALOGV("%s", __FUNCTION__);
    if (hpMaxGainDb == NULL) {
        ALOGE("[Err] %s %d Bad Argu", __FUNCTION__, __LINE__);
        return BAD_VALUE;
    }
    AL_AUTOLOCK(mLock);
    *hpMaxGainDb = mGainHP_Db_Max;
    return NO_ERROR;
}

status_t AudioALSAHyBridNLEManager::getNleHwConfigByIndex(size_t *hpMaxGainIdx) {
    status_t dRet = NO_ERROR;
    ALOGV("%s", __FUNCTION__);
    if (hpMaxGainIdx == NULL) {
        ALOGE("[Err] %s %d Bad Argu", __FUNCTION__, __LINE__);
        return BAD_VALUE;
    }
    AL_AUTOLOCK(mLock);
    int8_t bDb;
    if (mGainHP_Db_Max < 0) {
        bDb = (mGainHP_Db_Max * (-1)) & 0x00ff;
    } else {
        bDb = mGainHP_Db_Max & 0x00ff;
    }
    dRet = getAnalogIdx(bDb, hpMaxGainIdx);
    if (dRet != NO_ERROR) {
        ALOGE("[Err] %s %d mapping err", __FUNCTION__, __LINE__);
    }
    return dRet;
}

audio_hybridnlemng_status AudioALSAHyBridNLEManager::getStatus(void) {
    audio_hybridnlemng_status CurrentStatus;
    AL_AUTOLOCK(mLock);
    CurrentStatus = mStatus;
    ALOGV("Status: %d", CurrentStatus);
    return CurrentStatus;
}

status_t AudioALSAHyBridNLEManager::initPlayBackHandler(playback_handler_t ePlaybackHandlerType, stream_attribute_t *pStreamAttribute, AudioALSAPlaybackHandlerBase *pPlaybackHandler) {
    ALOGV("%s", __FUNCTION__);

    status_t dRet;
    audio_hybridnlemng_status mOldStatus, mCurStatus;
    if (mForceTurnOffNLE) {
        return INVALID_OPERATION;
    }

    if (ePlaybackHandlerType >= PLAYBACK_HANDLER_MAX) {
        ALOGE("[Err] %s %d", __FUNCTION__, __LINE__);
        return BAD_INDEX;
    }

    if (pPlaybackHandler == NULL) {
        ALOGE("[Err] %s %d", __FUNCTION__, __LINE__);
        return BAD_VALUE;
    }

    if (pStreamAttribute == NULL) {
        ALOGE("[Err] %s %d", __FUNCTION__, __LINE__);
        return BAD_VALUE;
    }

    {
        AL_AUTOLOCK(mLock);
        if (mNleCtrlOfPlaybackHandler[ePlaybackHandlerType] != NULL) {
            ALOGE("[Err] %s %d", __FUNCTION__, __LINE__);
            return ALREADY_EXISTS;
        }

        mNleCtrlOfPlaybackHandler[ePlaybackHandlerType] = new AudioALSANLECtrl(ePlaybackHandlerType, pStreamAttribute);

        if (mNleCtrlOfPlaybackHandler[ePlaybackHandlerType] == NULL) {
            ALOGE("[Err] %s %d", __FUNCTION__, __LINE__);
            return NO_MEMORY;
        }

        dRet = mNleCtrlOfPlaybackHandler[ePlaybackHandlerType]->init(pPlaybackHandler);

        if (dRet != NO_ERROR) {
            ALOGW("[Warn] Unsupport handler %d %s %d", ePlaybackHandlerType, __FUNCTION__, __LINE__);
            delete mNleCtrlOfPlaybackHandler[ePlaybackHandlerType];
            mNleCtrlOfPlaybackHandler[ePlaybackHandlerType] = NULL;
            return dRet;
        }

        mNleCtrlOfPlaybackHandlerCounter++;
        mInitPBHandlerBitwise |= (1 << ePlaybackHandlerType);

        mOldStatus = mStatus;
        updateCurStatus();
        mCurStatus = mStatus;
    }

    waitForBypass(mOldStatus, mCurStatus);

    return NO_ERROR;
}

status_t AudioALSAHyBridNLEManager::deinitPlayBackHandler(playback_handler_t ePlaybackHandlerType) {
    ALOGV("%s ePlaybackHandlerType %d", __FUNCTION__, ePlaybackHandlerType);

    status_t dRet;
    audio_hybridnlemng_status mOldStatus, mCurStatus;

    if (ePlaybackHandlerType >= PLAYBACK_HANDLER_MAX) {
        ALOGE("[Err] %s %d", __FUNCTION__, __LINE__);
        return BAD_INDEX;
    }

    {
        AL_AUTOLOCK(mLock);
        if (mNleCtrlOfPlaybackHandler[ePlaybackHandlerType] == NULL) {
            ALOGE("[Err] %s %d", __FUNCTION__, __LINE__);
            return NO_INIT;
        }

        dRet = mNleCtrlOfPlaybackHandler[ePlaybackHandlerType]->deinit();

        if (dRet != NO_ERROR) {
            ALOGE("[Err] %s %d", __FUNCTION__, __LINE__);
            return dRet;
        }

        delete mNleCtrlOfPlaybackHandler[ePlaybackHandlerType];
        mNleCtrlOfPlaybackHandler[ePlaybackHandlerType] = NULL;

        mNleCtrlOfPlaybackHandlerCounter--;
        mInitPBHandlerBitwise &= ~(1 << ePlaybackHandlerType);

        mOldStatus = mStatus;
        updateCurStatus();
        mCurStatus = mStatus;
    }

    waitForBypass(mOldStatus, mCurStatus);

    return NO_ERROR;
}

status_t AudioALSAHyBridNLEManager::process(playback_handler_t ePlaybackHandlerType, void *buffer, size_t Byte) {
    status_t dRet = NO_ERROR;
#if 0
    nsecs_t sysTimeBegin, sysTimeCur;
    uint32_t sysDiff;
    ALOGD("+");
    sysTimeBegin = systemTime();
#endif

    if (ePlaybackHandlerType >= PLAYBACK_HANDLER_MAX) {
        ALOGE("[Err] %s %d", __FUNCTION__, __LINE__);
        return BAD_INDEX;
    }

    if (!((1 << ePlaybackHandlerType) & mSupportRunNLEHandlerBitwise)) {
        ALOGE("[Err] %s %d", __FUNCTION__, __LINE__);
        return INVALID_OPERATION;
    }

    AL_AUTOLOCK(mLock);
    if (mNleCtrlOfPlaybackHandler[ePlaybackHandlerType] == NULL) {
        // ALOGV("[Warn] %s %d", __FUNCTION__, __LINE__);
        return NO_INIT;
    }

    if (Byte != mNleCtrlOfPlaybackHandler[ePlaybackHandlerType]->process(buffer, Byte)) {
        ALOGV("[Err] %s %d", __FUNCTION__, __LINE__);
        return INVALID_OPERATION;
    }

    if (mStatus == AUDIO_HYBRID_NLE_MNG_STATE_RUNNING) {
        if (ePlaybackHandlerType == mActivePBHandler) {
            AudioALSANLECtrl *pCurNLECtrl = mNleCtrlOfPlaybackHandler[ePlaybackHandlerType];
            dRet = doAdjustGainProcess(pCurNLECtrl, pCurNLECtrl->mCheckHwExtraFrameCnt, NLE_ADJUST_GAIN_NORMAL);
        }
    }
#if 0
    sysTimeCur = systemTime();
    sysDiff = ns2ms(sysTimeCur - sysTimeBegin);
    ALOGD("%d  %u", sysDiff, Byte);
#endif
    return dRet;
}

status_t AudioALSAHyBridNLEManager::setBypassNLE(bool bBypass) {
    ALOGV("%s", __FUNCTION__);
    audio_hybridnlemng_status mOldStatus, mCurStatus;
    {
        AL_AUTOLOCK(mLock);

        if (bBypass == mBypassNLE) {
            ALOGE("[Err] %s %d", __FUNCTION__, __LINE__);
            return INVALID_OPERATION;
        }

        mBypassNLE = bBypass;

        mOldStatus = mStatus;
        updateCurStatus();
        mCurStatus = mStatus;
    }

    waitForBypass(mOldStatus, mCurStatus);

    return NO_ERROR;
}

bool AudioALSAHyBridNLEManager::getBypassNLE(void) {
    ALOGV("%s", __FUNCTION__);
    return mBypassNLE;
}

status_t AudioALSAHyBridNLEManager::setEnableNLE(bool bEnable) {
    ALOGV("%s %d", __FUNCTION__, bEnable);
    audio_hybridnlemng_status mOldStatus, mCurStatus;

    {
        AL_AUTOLOCK(mLock);

        if (bEnable == mNleSwitch) {
            ALOGV("[Err] %s %d mNleSwitch %d", __FUNCTION__, __LINE__, mNleSwitch);
            return INVALID_OPERATION;
        }

        mNleSwitch = bEnable;

        mOldStatus = mStatus;
        updateCurStatus();
        mCurStatus = mStatus;
    }

    waitForBypass(mOldStatus, mCurStatus);

    return NO_ERROR;
}

status_t AudioALSAHyBridNLEManager::dump(void) {
    ALOGD("Dump...");
    ALOGD("mGainHP_Db_Min %d", mGainHP_Db_Min);
    ALOGD("mGainNLE_Db_Max %d", mGainNLE_Db_Max);
    ALOGD("mGainNLE_Db_Min %d", mGainNLE_Db_Min);
    ALOGD("mGainEop_Db %d", mGainEop_Db);
    ALOGD("mGainSop_Db %d", mGainSop_Db);
    ALOGD("mActivePBHandler %d", mActivePBHandler);
    ALOGD("mActivePBHandlerBitwise 0x%x", mActivePBHandlerBitwise);
    ALOGD("mInitPBHandlerBitwise 0x%x", mInitPBHandlerBitwise);
    ALOGD("mMode %d", mMode);
    ALOGD("mNleCtrlOfPlaybackHandlerCounter %d", mNleCtrlOfPlaybackHandlerCounter);
    ALOGD("mHwPathStreamCounter %d", mHwPathStreamCounter);
    ALOGD("mBypassNLE %d", mBypassNLE);
    ALOGD("mNleSwitch %d", mNleSwitch);
    ALOGD("mHwAnalogGainMaxDb = %d", mHwAnalogGainMaxDb);
    ALOGD("mHwAnalogGainMinDb = %d", mHwAnalogGainMinDb);
    ALOGD("mHwAnalogGainMinValue = %zu", mHwAnalogGainMinValue);
    ALOGD("mHwAnalogGainMuteDb = %d", mHwAnalogGainMuteDb);
    ALOGD("mHwAnalogGainMuteValue = %zu", mHwAnalogGainMuteValue);
    ALOGD("mHwDigitalGainMaxDb = %d", mHwDigitalGainMaxDb);
    ALOGD("mHwDigitalGainMaxValue = %zu", mHwDigitalGainMaxValue);
    ALOGD("mHwDigitalgGainMinDb = %d", mHwDigitalgGainMinDb);
    ALOGD("mHwAnalogDelayTick = %d", mHwAnalogDelayTick);
    return NO_ERROR;
}

uint32_t AudioALSAHyBridNLEManager::getSupportRunNLEHandler(void) {
    return mSupportRunNLEHandlerBitwise;
}

status_t AudioALSAHyBridNLEManager::setSupportRunNLEHandler(playback_handler_t eHandler) {
    if ((eHandler != PLAYBACK_HANDLER_NORMAL) && (eHandler != PLAYBACK_HANDLER_DEEP_BUFFER)) {
        ALOGD("Only Support PLAYBACK_HANDLER_NORMAL(%d) and PLAYBACK_HANDLER_DEEPBUF(%d), Not %d",
              PLAYBACK_HANDLER_NORMAL, PLAYBACK_HANDLER_DEEP_BUFFER, eHandler);
        return BAD_VALUE;
    }
    ALOGD("Support %d Handler", eHandler);
    // We overwrite the supported handler, not add/remove so far
    mSupportRunNLEHandlerBitwise = (1 << eHandler);
    return NO_ERROR;
}

status_t AudioALSAHyBridNLEManager::setNleEopDb(int eopGainDb) {
    return setNleHwConfigByDb(mGainHP_Db_Max, eopGainDb);
}

int AudioALSAHyBridNLEManager::getNleEopDb(void) {
    return mGainEop_Db;
}

status_t AudioALSAHyBridNLEManager::setAudioMode(audio_mode_t eMode) {
    ALOGV("%s", __FUNCTION__);
    audio_hybridnlemng_status mOldStatus, mCurStatus;
    {
        AL_AUTOLOCK(mLock);

        if (eMode == mMode) {
            ALOGE("[Err] %s %d", __FUNCTION__, __LINE__);
            return INVALID_OPERATION;
        }

        mMode = eMode;

        mOldStatus = mStatus;
        updateCurStatus();
        mCurStatus = mStatus;
    }

    waitForBypass(mOldStatus, mCurStatus);

    return NO_ERROR;
}

status_t AudioALSAHyBridNLEManager::addHwPathStream(audio_stream_hw_path eHwPath __unused) {
    ALOGV("%s", __FUNCTION__);
    audio_hybridnlemng_status mOldStatus, mCurStatus;

    // eHwPath = eHwPath; //ToDo
    {
        AL_AUTOLOCK(mLock);

        mHwPathStreamCounter++;

        mOldStatus = mStatus;
        updateCurStatus();
        mCurStatus = mStatus;
    }

    waitForBypass(mOldStatus, mCurStatus);

    return NO_ERROR;
}


status_t AudioALSAHyBridNLEManager::removeHwPathStream(audio_stream_hw_path eHwPath __unused) {
    ALOGV("%s", __FUNCTION__);
    audio_hybridnlemng_status mOldStatus, mCurStatus;

    //eHwPath = eHwPath; //ToDo
    {
        AL_AUTOLOCK(mLock);

        if (mHwPathStreamCounter <= 0) {
            ALOGE("[Err] %s %d", __FUNCTION__, __LINE__);
            return UNKNOWN_ERROR;
        }

        mHwPathStreamCounter--;

        mOldStatus = mStatus;
        updateCurStatus();
        mCurStatus = mStatus;
    }

    waitForBypass(mOldStatus, mCurStatus);

    return NO_ERROR;
}

status_t AudioALSAHyBridNLEManager::updateCurStatus(void) {
    audio_hybridnlemng_status nowStatus = mStatus;
    ALOGD("mNleSwitch %d mBypassNLE %d PBHCount %d mMode %d HwPathCnt %d PBH %d PBHBit 0x%x InitBit 0x%x", mNleSwitch, mBypassNLE, mNleCtrlOfPlaybackHandlerCounter, mMode, mHwPathStreamCounter, mActivePBHandler, mActivePBHandlerBitwise, mInitPBHandlerBitwise);
    do {
        if (mNleSwitch == false) {
            if (mStatus == AUDIO_HYBRID_NLE_MNG_STATE_DISABLE) {
                break;
            } else if (mStatus == AUDIO_HYBRID_NLE_MNG_STATE_BYBASS) {
                disableNleHw();
                ALOGV("ts %d -> %d", mStatus, AUDIO_HYBRID_NLE_MNG_STATE_DISABLE);
                mStatus = AUDIO_HYBRID_NLE_MNG_STATE_DISABLE;
            } else if (mStatus == AUDIO_HYBRID_NLE_MNG_STATE_RUNNING) {
                // Do ramp down to digital gain to Zero
                ramp2DigitalGainZero();
                ALOGV("ts %d -> %d", mStatus, AUDIO_HYBRID_NLE_MNG_STATE_BYBASS);
                mStatus = AUDIO_HYBRID_NLE_MNG_STATE_BYBASS;
            }
        } else if (mBypassNLE == true || mNleCtrlOfPlaybackHandlerCounter > 1 || mMode != AUDIO_MODE_NORMAL || mHwPathStreamCounter != 0) {

            if (mStatus == AUDIO_HYBRID_NLE_MNG_STATE_DISABLE) {
                enableNleHw();
                ALOGV("ts %d -> %d", mStatus, AUDIO_HYBRID_NLE_MNG_STATE_BYBASS);
                mStatus = AUDIO_HYBRID_NLE_MNG_STATE_BYBASS;
            } else if (mStatus == AUDIO_HYBRID_NLE_MNG_STATE_BYBASS) {
                break;
            } else if (mStatus == AUDIO_HYBRID_NLE_MNG_STATE_RUNNING) {
                // Do ramp down to digital gain to Zero (Other change)
                ramp2DigitalGainZero();
                ALOGV("ts %d -> %d", mStatus, AUDIO_HYBRID_NLE_MNG_STATE_BYBASS);
                mStatus = AUDIO_HYBRID_NLE_MNG_STATE_BYBASS;
            }
        } else {
            if (mStatus == AUDIO_HYBRID_NLE_MNG_STATE_DISABLE) {
                enableNleHw();
                ALOGV("ts %d -> %d", mStatus, AUDIO_HYBRID_NLE_MNG_STATE_BYBASS);
                mStatus = AUDIO_HYBRID_NLE_MNG_STATE_BYBASS;
            } else if (mStatus == AUDIO_HYBRID_NLE_MNG_STATE_BYBASS) {
                if (mNleCtrlOfPlaybackHandlerCounter == 1 && (mSupportRunNLEHandlerBitwise & mInitPBHandlerBitwise)) {
                    mActivePBHandlerBitwise = mInitPBHandlerBitwise;
                    mActivePBHandler = (playback_handler_t)(find_bit_pos(mActivePBHandlerBitwise) - 1);
                    ALOGV("ts %d -> %d with Handle %d", mStatus, AUDIO_HYBRID_NLE_MNG_STATE_RUNNING, mActivePBHandler);
                    mStatus = AUDIO_HYBRID_NLE_MNG_STATE_RUNNING;
                } else {
                    break;
                }
            } else if (mStatus == AUDIO_HYBRID_NLE_MNG_STATE_RUNNING) {
                if (mNleCtrlOfPlaybackHandlerCounter == 0 || !(mSupportRunNLEHandlerBitwise & mInitPBHandlerBitwise)) {
                    // Do ramp down to digital gain to Zero (Self change)
                    ramp2DigitalGainZero();
                    ALOGV("ts %d -> %d", mStatus, AUDIO_HYBRID_NLE_MNG_STATE_BYBASS);
                    mActivePBHandlerBitwise = 0;
                    mActivePBHandler = PLAYBACK_HANDLER_BASE;
                    mStatus = AUDIO_HYBRID_NLE_MNG_STATE_BYBASS;
                } else {
                    break;
                }
            }
        }
    } while (1);

    if (nowStatus != mStatus) {
        ALOGD("%s status [%d] -> [%d]", __FUNCTION__, nowStatus, mStatus);
    }
    ALOGV("PHC %d mMode %d HwCount %d BypassNLE %d", mNleCtrlOfPlaybackHandlerCounter, mMode, mHwPathStreamCounter, mBypassNLE);
    return NO_ERROR;
}

status_t AudioALSAHyBridNLEManager::enableNleHw(void) {
    //Pass rg_nle_delay_ana into setting
    //Pass initial Analog gain into setting
    ALOGV("%s", __FUNCTION__);
    unsigned char GainConfig[3];
    GainConfig[0] = mHwAnalogDelayTick;
    if (mGainHP_Db_Max < 0) {
        GainConfig[1] = 1;
        GainConfig[2] = (mGainHP_Db_Max * (-1)) & 0x00ff;
    } else {
        GainConfig[1] = 0;
        GainConfig[2] = mGainHP_Db_Max & 0x00ff;
    }
#ifdef ENABLE_NLE_HW_CONTROL
    if (mHwSupport) {
        if (mixer_ctl_set_array(mixer_get_ctl_by_name(mMixer, "Audio_HyBridNLE_TurnOn"), (const void *)GainConfig, 3)) {
            ALOGE("[Errrrrrrrrrrrrrrr] NLE %s %d", __FUNCTION__, __LINE__);
            return INVALID_OPERATION;
        }
    } else {
        ALOGD("%s Kernel unsupport", __FUNCTION__);
    }
#else
    ALOGD("%s Audio_HyBridNLE_TurnOn", __FUNCTION__);
#endif
    return NO_ERROR;
}

status_t AudioALSAHyBridNLEManager::disableNleHw(void) {
    //Pass rg_nle_delay_ana into setting
    //Pass initial Analog gain into setting
    ALOGV("%s", __FUNCTION__);
    unsigned char GainConfig[3];
    GainConfig[0] = mHwAnalogDelayTick;
    if (mGainHP_Db_Max < 0) {
        GainConfig[1] = 1;
        GainConfig[2] = (mGainHP_Db_Max * (-1)) & 0x00ff;
    } else {
        GainConfig[1] = 0;
        GainConfig[2] = mGainHP_Db_Max & 0x00ff;
    }
#ifdef ENABLE_NLE_HW_CONTROL
    if (mHwSupport) {
        if (mixer_ctl_set_array(mixer_get_ctl_by_name(mMixer, "Audio_HyBridNLE_TurnOff"), (const void *)GainConfig, 3)) {
            ALOGE("[Errrrrrrrrrrrrrrr] NLE %s %d", __FUNCTION__, __LINE__);
            return INVALID_OPERATION;
        }
    } else {
        ALOGD("%s Kernel unsupport", __FUNCTION__);
    }
#else
    ALOGD("%s Audio_HyBridNLE_TurnOff", __FUNCTION__);
#endif
    return NO_ERROR;
}


status_t AudioALSAHyBridNLEManager::ramp2DigitalGainZero(void) {
    ALOGV("%s", __FUNCTION__);
    if (mSupportRunNLEHandlerBitwise & mActivePBHandlerBitwise) {
        AudioALSANLECtrl *pCurNLECtrl = mNleCtrlOfPlaybackHandler[mActivePBHandler];
        if (pCurNLECtrl != NULL) {
            return doAdjustGainProcess(pCurNLECtrl, pCurNLECtrl->mRamp2ZeroFrameCnt, NLE_ADJUST_GAIN_RAMP_TO_BYPASS);
        } else {
            // Do rampdown to 0 with NoZCE
            resetDefaultGain(&mLNleHwInfo);
            resetDefaultGain(&mRNleHwInfo);
            applyGain2Hw(&mLNleHwInfo);
            applyGain2Hw(&mRNleHwInfo);
            return NO_ERROR;
        }
    } else {
        // Do rampdown to 0 with NoZCE
        resetDefaultGain(&mLNleHwInfo);
        resetDefaultGain(&mRNleHwInfo);
        applyGain2Hw(&mLNleHwInfo);
        applyGain2Hw(&mRNleHwInfo);
        return NO_ERROR;
    }
}

bool AudioALSAHyBridNLEManager::checkIfGainTargeted(void) {
    // Query Kernel driver to check if target for both LR
#ifdef ENABLE_NLE_HW_CONTROL
    ALOGVV("%s", __FUNCTION__);
    struct mixer_ctl *ctl;
    int dValue = 0;
    if (mHwSupport) {
        ctl = mixer_get_ctl_by_name(mMixer, "Audio_HyBridNLE_Targeted");
        if (NULL == ctl) {
            ALOGE("[%s] [%d]", __FUNCTION__, __LINE__);
            return false;
        }
        dValue = mixer_ctl_get_value(ctl, 0);
    } else {
        ALOGD("%s Kernel unsupport", __FUNCTION__);
    }
    if (dValue == 1) {
        return true;
    } else {
        return false;
    }
#else
    return false;
#endif
}

void AudioALSAHyBridNLEManager::showRegInfo(void) {
#ifdef ENABLE_NLE_HW_CONTROL
    if (mHwSupport) {
        const unsigned int num_values = 32;
        struct mixer_ctl *ctl;
        unsigned char GainConfig[num_values];
        ctl = mixer_get_ctl_by_name(mMixer, "Audio_HyBridNLE_Info");
        if (NULL == ctl) {
            ALOGE("Err for get Audio_HyBridNLE_Info");
        }
        memset((void *)GainConfig, 0, sizeof(unsigned char)*num_values);
        if (mixer_ctl_get_array(ctl, (void *)GainConfig, num_values)) {
            ALOGE("Error for GainConfig");
        }
        ALOGD("NLEREG: AFE_DL_NLE_R_CFG0 = 0x%x", GainConfig[1] << 8 | GainConfig[0]);
        ALOGD("NLEREG: AFE_DL_NLE_R_CFG1 = 0x%x", GainConfig[3] << 8 | GainConfig[2]);
        ALOGD("NLEREG: AFE_DL_NLE_R_CFG2 = 0x%x", GainConfig[5] << 8 | GainConfig[4]);
        ALOGD("NLEREG: AFE_DL_NLE_R_CFG3 = 0x%x", GainConfig[7] << 8 | GainConfig[6]);

        ALOGD("NLEREG: AFE_DL_NLE_L_CFG0 = 0x%x", GainConfig[9] << 8 | GainConfig[8]);
        ALOGD("NLEREG: AFE_DL_NLE_L_CFG0 = 0x%x", GainConfig[11] << 8 | GainConfig[10]);
        ALOGD("NLEREG: AFE_DL_NLE_L_CFG0 = 0x%x", GainConfig[13] << 8 | GainConfig[12]);
        ALOGD("NLEREG: AFE_DL_NLE_L_CFG0 = 0x%x", GainConfig[15] << 8 | GainConfig[14]);

        ALOGD("NLEREG: AFE_RGS_NLE_R_CFG0 = 0x%x", GainConfig[17] << 8 | GainConfig[16]);
        ALOGD("NLEREG: AFE_RGS_NLE_R_CFG1 = 0x%x", GainConfig[19] << 8 | GainConfig[18]);
        ALOGD("NLEREG: AFE_RGS_NLE_R_CFG2 = 0x%x", GainConfig[21] << 8 | GainConfig[20]);
        ALOGD("NLEREG: AFE_RGS_NLE_R_CFG3 = 0x%x", GainConfig[23] << 8 | GainConfig[22]);

        ALOGD("NLEREG: AFE_RGS_NLE_L_CFG0 = 0x%x", GainConfig[25] << 8 | GainConfig[24]);
        ALOGD("NLEREG: AFE_RGS_NLE_L_CFG1 = 0x%x", GainConfig[27] << 8 | GainConfig[26]);
        ALOGD("NLEREG: AFE_RGS_NLE_L_CFG2 = 0x%x", GainConfig[29] << 8 | GainConfig[28]);
        ALOGD("NLEREG: AFE_RGS_NLE_L_CFG3 = 0x%x", GainConfig[31] << 8 | GainConfig[30]);
    } else {
        ALOGD("%s Kernel unsupport", __FUNCTION__);
    }
#else
    ALOGD("%s Audio_HyBridNLE_Info", __FUNCTION__);
#endif
}
status_t AudioALSAHyBridNLEManager::waitForBypass(audio_hybridnlemng_status ePrevStatus, audio_hybridnlemng_status eCurStatus) {
    if ((eCurStatus == AUDIO_HYBRID_NLE_MNG_STATE_BYBASS && ePrevStatus == AUDIO_HYBRID_NLE_MNG_STATE_RUNNING)
        || (eCurStatus == AUDIO_HYBRID_NLE_MNG_STATE_DISABLE && ePrevStatus == AUDIO_HYBRID_NLE_MNG_STATE_BYBASS)) {
        ALOGV("waitForBypass Need wait %d -> %d", ePrevStatus, eCurStatus);
        nsecs_t sysTimeBegin, sysTimeCur;
        uint32_t sysDiff;
        sysTimeBegin = systemTime();
        do {
            if (checkIfGainTargeted() == true) {
                break;
            } else {
                sysTimeCur = systemTime();
                sysDiff = ns2ms(sysTimeCur - sysTimeBegin);
                if (sysDiff >= NLE_WAIT_NLE_BYPASS_MAX_MS) {
                    ALOGW("[Warn] waitForBypass TimeOut %d MS", sysDiff);
                    break;
                }
                usleep(1000);
            }
        } while (1);
        ALOGV("waitForBypass Exit wait");
    } else {
        ALOGV("Do nothing for %d -> %d", ePrevStatus, eCurStatus);
    }

    return NO_ERROR;
}

status_t AudioALSAHyBridNLEManager::doAdjustGainProcess(AudioALSANLECtrl *pCurNLECtrl, size_t zuCheckOffsetFrameCnt, audio_hybrid_adjust_mode eAdjustMode) {
    status_t dRet;
    time_info_struct_t stHWBuffer_Time_Info;
    size_t UsedFrameCount;
    size_t zuCheckExtraFrameCnt;
    size_t zuOffsetForTail;
    if (pCurNLECtrl == NULL || eAdjustMode >= NLE_ADJUST_GAIN_MAX) {
        ALOGE("[Err] doAdjustGainProcess pCurNLECtrl %p eAdjustMode %d", pCurNLECtrl, eAdjustMode);
        return BAD_VALUE;
    } else if (eAdjustMode == NLE_ADJUST_GAIN_RAMP_TO_BYPASS) {
        //Insert MAX value into offset
        zuCheckExtraFrameCnt = 0;
    } else {
        zuCheckExtraFrameCnt = zuCheckOffsetFrameCnt;
    }

    if (pCurNLECtrl->mAudioALSAPlaybackHandlerBase->getHardwareBufferInfo(&stHWBuffer_Time_Info) == NO_ERROR) {
        UsedFrameCount = stHWBuffer_Time_Info.buffer_per_time - stHWBuffer_Time_Info.frameInfo_get;
        if (eAdjustMode == NLE_ADJUST_GAIN_RAMP_TO_BYPASS) {
            if (zuCheckOffsetFrameCnt > UsedFrameCount) {
                zuOffsetForTail = 0;
            } else {
                zuOffsetForTail = UsedFrameCount - zuCheckOffsetFrameCnt;
            }
            pCurNLECtrl->setMaxSPLintoSpecificPos(zuOffsetForTail);
        }
        // ALOGD("UsedFrameCount L %d", UsedFrameCount); - For Debug
        UsedFrameCount = UsedFrameCount + zuCheckExtraFrameCnt;
        dRet = checkAndSetGain(UsedFrameCount, pCurNLECtrl, &(mLNleHwInfo), &(pCurNLECtrl->mLBuf));
        if (dRet == NO_ERROR) {
#if 0
            if (pCurNLECtrl->mAudioALSAPlaybackHandlerBase->getHardwareBufferInfo(&stHWBuffer_Time_Info) == NO_ERROR) {
                UsedFrameCount = stHWBuffer_Time_Info.buffer_per_time - stHWBuffer_Time_Info.frameInfo_get;
                // ALOGD("UsedFrameCount R %d", UsedFrameCount); - For Debug
                UsedFrameCount = UsedFrameCount + pCurNLECtrl->mCheckHwExtraFrameCnt;
                dRet = checkAndSetGain(UsedFrameCount, pCurNLECtrl, &(mRNleHwInfo), &(pCurNLECtrl->mRBuf));
                if (dRet == NO_ERROR) {
                    ;
                } else if (dRet == INVALID_OPERATION) {
                    ;
                } else {
                    ALOGE("[Err] %s %d dRet = %d", __FUNCTION__, __LINE__, dRet);
                }
            } else {
                ALOGVV("2nd getHardwareBufferInfo Null");
                dRet = INVALID_OPERATION;
            }
#else
            dRet = checkAndSetGain(UsedFrameCount, pCurNLECtrl, &(mRNleHwInfo), &(pCurNLECtrl->mRBuf));
#endif
        } else if (dRet == INVALID_OPERATION) {
            ;
        } else {
            ALOGE("[Err] %s %d dRet = %d", __FUNCTION__, __LINE__, dRet);
        }
    } else {
        ALOGVV("1st getHardwareBufferInfo Null");
        dRet = INVALID_OPERATION;
    }
    return dRet;
}

status_t AudioALSAHyBridNLEManager::getDbFromAnalogIdx(size_t AnalogIdx, int8_t *pDb) {
    int8_t mDb;

    if (pDb == NULL) {
        ALOGE("[Err] %s LINE %d", __FUNCTION__, __LINE__);
        return BAD_VALUE;
    }

    if ((AnalogIdx > mHwAnalogGainMinValue && AnalogIdx < mHwAnalogGainMuteValue) || AnalogIdx > mHwAnalogGainMuteValue) {
        ALOGE("[Err] %s LINE %d AnalogIdx %zu Reserved", __FUNCTION__, __LINE__, AnalogIdx);
        return BAD_VALUE;
    }

    if (AnalogIdx == mHwAnalogGainMuteValue) {
        mDb = mHwAnalogGainMuteDb;
    } else {
        mDb = mHwAnalogGainMaxDb - (int8_t) AnalogIdx;
    }

    *pDb = mDb;

    return NO_ERROR;
}

status_t AudioALSAHyBridNLEManager::getAnalogIdx(int8_t Db, size_t *pAnalogIdx) {
    size_t mAnalogIdx;

    if (pAnalogIdx == NULL) {
        ALOGE("[Err] %s LINE %d", __FUNCTION__, __LINE__);
        return BAD_VALUE;
    }

    if (Db > mHwAnalogGainMaxDb || (Db < mHwAnalogGainMinDb && Db != mHwAnalogGainMuteDb)) {
        ALOGE("[Err] %s LINE %d Db %d Reserved", __FUNCTION__, __LINE__, Db);
        return BAD_VALUE;
    }

    if (Db == mHwAnalogGainMuteDb) {
        mAnalogIdx = mHwAnalogGainMuteValue;
    } else {
        mAnalogIdx = (size_t)(mHwAnalogGainMaxDb - Db);
    }

    *pAnalogIdx = mAnalogIdx;

    return NO_ERROR;
}

status_t AudioALSAHyBridNLEManager::getDbFromDigitalIdx(size_t DigitalIdx, int8_t *pDb) {
    int8_t mDb;

    if (pDb == NULL) {
        ALOGE("[Err] %s LINE %d", __FUNCTION__, __LINE__);
        return BAD_VALUE;
    }

    if (DigitalIdx > mHwDigitalGainMaxValue) {
        ALOGE("[Err] %s LINE %d DigitalIdx %zu Reserved", __FUNCTION__, __LINE__, DigitalIdx);
        return BAD_VALUE;
    }

    mDb =  DigitalIdx;

    *pDb = mDb;

    return NO_ERROR;
}

status_t AudioALSAHyBridNLEManager::getDigitalIdx(int8_t Db, size_t *pDigitalIdx) {
    size_t mDigitalIdx;

    if (pDigitalIdx == NULL) {
        ALOGE("[Err] %s LINE %d", __FUNCTION__, __LINE__);
        return BAD_VALUE;
    }

    if (Db < 0 || Db > 0x2C) {
        ALOGE("[Err] %s LINE %d Db %d Reserved", __FUNCTION__, __LINE__, Db);
        return BAD_VALUE;
    }

    mDigitalIdx = Db;

    *pDigitalIdx = mDigitalIdx;

    return NO_ERROR;
}

status_t AudioALSAHyBridNLEManager::gainAllocatePlan(float Spl, size_t ZeroCnt, size_t Zuframecount, NleHwInfo_t *pNleHwInfo) {
    int32_t Gain_Spl;
    int32_t mGainNle;
    int32_t mGainHP;
    if (Spl > 0.0f || pNleHwInfo == NULL) {
        ALOGE("[Err] NLE gainAllocatePlan unreasonable %f (should < 0.0f) nleHwInfo = %p", Spl, pNleHwInfo);
        return BAD_VALUE;
    }
    Gain_Spl = (int) Spl;
    if (Gain_Spl > mGainEop_Db || mGainHP_Db_Max == NLE_SUPPORT_ANA_GAIN_MUTE_DB) {
        mGainNle = mGainNLE_Db_Min;
        mGainHP = mGainHP_Db_Max;
    } else {
        if ((mGainSop_Db <= Gain_Spl) && (Gain_Spl <= mGainEop_Db)) {
            mGainNle = mGainEop_Db - Gain_Spl;
            mGainHP = mGainHP_Db_Max - mGainNle;
        } else {
            mGainNle = mGainNLE_Db_Max;
            mGainHP = mGainHP_Db_Min;
        }
    }

    pNleHwInfo->stPrev.mGainNle = pNleHwInfo->stCur.mGainNle;
    pNleHwInfo->stPrev.mGainHP = pNleHwInfo->stCur.mGainHP;
    pNleHwInfo->stPrev.mDbpPerStep = pNleHwInfo->stCur.mDbpPerStep;
    pNleHwInfo->stPrev.mStepPerZC = pNleHwInfo->stCur.mStepPerZC;

    pNleHwInfo->stCur.mGainNle = mGainNle;
    pNleHwInfo->stCur.mGainHP = mGainHP;
#if 0
    if (pNleHwInfo->mChannel == AUDIO_NLE_CHANNEL_L) {
        ALOGVV("%d->%d", pNleHwInfo->mMaxSPL_Db, Gain_Spl);
    }
#endif
    pNleHwInfo->mMaxSPL_Db = Gain_Spl;
    pNleHwInfo->mZeroEventCount = ZeroCnt;
    pNleHwInfo->mLookUpFrameCount = Zuframecount;
    // mNleHwInfo.stCur.mDbpPerStep ToDo
    // mNleHwInfo.stCur.mStepPerZC ToDo
    return NO_ERROR;
}

status_t AudioALSAHyBridNLEManager::applyGain2Hw(NleHwInfo_t *pNleHwInfo) {
    status_t dRet = NO_ERROR;
    if (pNleHwInfo == NULL) {
        ALOGE("[Err] NLE applyGain2Hw NleHwInfo %p", pNleHwInfo);
        return BAD_VALUE;
    }

    if (pNleHwInfo->stCur.mGainNle != pNleHwInfo->stPrev.mGainNle ||
        pNleHwInfo->stCur.mGainHP != pNleHwInfo->stPrev.mGainHP) {

        if (pNleHwInfo->mChannel == AUDIO_NLE_CHANNEL_L) {
            ALOGVV("Gain_Spl %d, GainNle %d, GainHP %d ZeroCnt %zu lookup %zu", pNleHwInfo->mMaxSPL_Db, pNleHwInfo->stCur.mGainNle, pNleHwInfo->stCur.mGainHP, pNleHwInfo->mZeroEventCount, pNleHwInfo->mLookUpFrameCount);
        }

        unsigned char GainConfig[9];
        GainConfig[0] = pNleHwInfo->mChannel;
        GainConfig[1] = pNleHwInfo->mZeroEventCount & 0x00ff;
        GainConfig[2] = (pNleHwInfo->mZeroEventCount >> 8) & 0x00ff;
        GainConfig[3] = (pNleHwInfo->mZeroEventCount >> 16) & 0x00ff;
        GainConfig[4] = (pNleHwInfo->mZeroEventCount >> 24) & 0x00ff;
        GainConfig[5] = (NLE_GAIN_STEP_MAX_DB & 0x00ff);
        GainConfig[6] = pNleHwInfo->stCur.mGainNle;
        if (pNleHwInfo->stCur.mGainHP < 0) {
            GainConfig[7] = 1;
            GainConfig[8] = (pNleHwInfo->stCur.mGainHP * (-1)) & 0x00ff;
        } else {
            GainConfig[7] = 0;
            GainConfig[8] = pNleHwInfo->stCur.mGainHP & 0x00ff;
        }
#ifdef ENABLE_NLE_HW_CONTROL
        if (mHwSupport) {
            if (mixer_ctl_set_array(mixer_get_ctl_by_name(mMixer, "Audio_HyBridNLE_SetGain"), (const void *)GainConfig, 9)) {
                ALOGE("[Errrrrrrrrrrrrrrr] NLE %s %d", __FUNCTION__, __LINE__);
                return INVALID_OPERATION;
            }
        } else {
            ALOGD("%s Kernel unsupport", __FUNCTION__);
        }
#else
#endif
    }
    return dRet;
}

status_t AudioALSAHyBridNLEManager::resetDefaultGain(NleHwInfo_t *pNleHwInfo) {
    if (pNleHwInfo == NULL) {
        ALOGE("[Err] NLE resetDefaultGain NleHwInfo %p", pNleHwInfo);
        return BAD_VALUE;
    }
    audio_nle_channel Channel = pNleHwInfo->mChannel;
    // ALOGD("%s %p", __FUNCTION__, pNleHwInfo);
    memset(pNleHwInfo, 0x00, sizeof(NleHwInfo_t));
    pNleHwInfo->mChannel = Channel;
    pNleHwInfo->stPrev.mGainNle = NLE_GAIN_UNININIAL_VALUE;
    pNleHwInfo->stPrev.mGainHP = NLE_GAIN_UNININIAL_VALUE;
    pNleHwInfo->stCur.mGainNle = mGainNLE_Db_Min;
    pNleHwInfo->stCur.mGainHP = mGainHP_Db_Max;
    pNleHwInfo->mMaxValue = NLE_24BIT_NAGATIVE_MAX;

    return NO_ERROR;
}

status_t AudioALSAHyBridNLEManager::getGstepAndGnum(int32_t SPL_diff, size_t ZeroCnt, int32_t *pdGstep, int32_t *pdGnum) {
    int32_t dGstep, dGnum;
    int32_t dTempdGstep, dTempdGnum;
    size_t absSPL_diff = abs(SPL_diff);
    if (pdGstep == NULL || pdGnum == NULL || ZeroCnt == 0 || SPL_diff == 0) {
        ALOGE("[Err] NLE getGstepAndGnum pdGstep %p pdGnum %p ZeroCnt %zu SPL_diff %d", pdGstep, pdGnum, ZeroCnt, SPL_diff);
        return BAD_VALUE;
    }

    if (ZeroCnt >= absSPL_diff) {
        dGstep = 1; // 1dB
        dGnum = 1; // 1 times
    } else {
        dTempdGstep = (absSPL_diff + ZeroCnt) / ZeroCnt;
        if (dTempdGstep <= NLE_GAIN_STEP_MAX_DB) {
            dGstep = dTempdGstep;
            dGnum = 1;
        } else {
            dTempdGnum = (dTempdGstep + NLE_GAIN_STEP_MAX_DB) / NLE_GAIN_STEP_MAX_DB;
            dGstep = NLE_GAIN_STEP_MAX_DB;
            dGnum = dTempdGnum;
        }
    }

    *pdGstep = dGstep - 1;// For Reg mapping
    *pdGnum = dGnum;
    return NO_ERROR;
}


status_t AudioALSAHyBridNLEManager::checkAndSetGain(size_t zuframecount, AudioALSANLECtrl *pNleCtrl, NleHwInfo_t *pNleHwInfo, RingBuf *pRingBuf) {
    status_t dRet;
    float SPL_Db = 0.0f;
    size_t Zero_Cnt = 0;
    if (zuframecount == 0 || pNleCtrl == NULL || pNleHwInfo == NULL || pRingBuf == NULL) {
        ALOGW("[Warn] zuframecount = %zu, pNleCtrl = %p, pNleHwInfo = %p, pRingBuf = %p %s  Line%d ", zuframecount, pNleCtrl, pNleHwInfo, pRingBuf, __FUNCTION__, __LINE__);
        return INVALID_OPERATION;
    } else if (pNleCtrl->mMinPreviewFrameCount > zuframecount) {
        ALOGW("[Warn] zuframecount = %zu, min = %zu, %s  Line%d ", zuframecount, pNleCtrl->mMinPreviewFrameCount, __FUNCTION__, __LINE__);
        //return INVALID_OPERATION; We continue to apply NLE even it has risk that ZCE isn't enough
    }
    if (pNleCtrl->mBytePerSample == (sizeof(int32_t))) {
        dRet = pNleCtrl->getMaxSPLandZeroCnt(pNleHwInfo, pRingBuf, zuframecount, &SPL_Db, &Zero_Cnt, mMax24BitValidValue);
    } else {
        dRet = pNleCtrl->getMaxSPLandZeroCnt(pNleHwInfo, pRingBuf, zuframecount, &SPL_Db, &Zero_Cnt, mMax16BitValidValue);
    }

    if (dRet != NO_ERROR) {
        ALOGE("[Err] dRet = %d, %s %d ", dRet, __FUNCTION__, __LINE__);
        return dRet;
    }

    dRet = gainAllocatePlan(SPL_Db, Zero_Cnt, zuframecount, pNleHwInfo);

    if (dRet != NO_ERROR) {
        ALOGE("[Err] dRet = %d, %s %d ", dRet, __FUNCTION__, __LINE__);
        return dRet;
    }

    dRet = applyGain2Hw(pNleHwInfo);

    if (dRet != NO_ERROR) {
        ALOGE("[Err] dRet = %d, %s %d ", dRet, __FUNCTION__, __LINE__);
        return dRet;
    }
    return dRet;
}

AudioALSANLECtrl::AudioALSANLECtrl(playback_handler_t ePlaybackHandlerType, const stream_attribute_t *pStream_Attribute_Source) {
    if (pStream_Attribute_Source != NULL) {
        memcpy(&mStreamAttribute, pStream_Attribute_Source, sizeof(mStreamAttribute));
        mBytePerSample = audio_bytes_per_sample(mStreamAttribute.audio_format);
        mMinPreviewByte = (mBytePerSample * mStreamAttribute.num_channels * mStreamAttribute.sample_rate * (NLE_SUPPORT_PREVIEW_MS_MIN + NLE_HW_QUEUE_SKIP_ZCE_FRAMECOUNT_MS - NLE_HW_QUEUE_EXTRA_FRAMECOUNT_MS)) / 1000;
        mHwBufTotalFrame = mStreamAttribute.buffer_size / (mBytePerSample * mStreamAttribute.num_channels);
        mCheckHwExtraFrameCnt = (mStreamAttribute.sample_rate * NLE_HW_QUEUE_EXTRA_FRAMECOUNT_MS) / 1000;
        mSkipHwZCEFrameCnt = (mStreamAttribute.sample_rate * NLE_HW_QUEUE_SKIP_ZCE_FRAMECOUNT_MS) / 1000;
        mMinPreviewFrameCount = ((mStreamAttribute.sample_rate * NLE_SUPPORT_PREVIEW_MS_MIN + 1000) / 1000) + mCheckHwExtraFrameCnt;
        mPlaybackedFrameCnt = (mStreamAttribute.sample_rate * NLE_HW_QUEUE_EXTRA_FRAMECOUNT_MS) / 1000 - NLE_HW_QUEUE_BUFFER_BEFORE_DAC_FRAMECOUNT;
        mRamp2ZeroFrameCnt = (mStreamAttribute.sample_rate * NLE_WAIT_NLE_BYPASS_MAX_MS) / 1000;
        if ((ePlaybackHandlerType >= PLAYBACK_HANDLER_NORMAL) && (1 << ePlaybackHandlerType) & AudioALSAHyBridNLEManager::getSupportRunNLEHandler()) {
            ALOGD("F 0x%x Dev 0x%x CM 0x%x CN %d SR %d BS %d", mStreamAttribute.audio_format, mStreamAttribute.output_devices, mStreamAttribute.audio_channel_mask, mStreamAttribute.num_channels,
                  mStreamAttribute.sample_rate, mStreamAttribute.buffer_size);
            ALOGD("MB %zu TB %zu CEF %zu SKZ %zu MPB %zu PBDF %zu Ramp2ZeroFC %zu", mMinPreviewByte, mHwBufTotalFrame, mCheckHwExtraFrameCnt, mSkipHwZCEFrameCnt, mMinPreviewFrameCount, mPlaybackedFrameCnt, mRamp2ZeroFrameCnt);
        }
#if 0
        ALOGD("audio_format 0x%x", mStreamAttribute.audio_format);
        ALOGD("output_devices 0x%x", mStreamAttribute.output_devices);
        ALOGD("audio_channel_mask 0x%x", mStreamAttribute.audio_channel_mask);
        ALOGD("num_channels %d", mStreamAttribute.num_channels);
        ALOGD("sample_rate %d", mStreamAttribute.sample_rate);
        ALOGD("byte_per_sample %zu", mBytePerSample);
        ALOGD("buffer_byte %d", mStreamAttribute.buffer_size);
        ALOGD("min_preview_byte %zu", mMinPreviewByte);
        ALOGD("mHwBufTotalFrame %zu", mHwBufTotalFrame);
        ALOGD("mCheckHwExtraFrameCnt %zu", mCheckHwExtraFrameCnt);
        ALOGD("mSkipHwZCEFrameCnt %zu", mSkipHwZCEFrameCnt);
        ALOGD("mMinPreviewFrameCount %zu", mMinPreviewFrameCount);
        ALOGD("mPlaybackedFrameCnt %zu", mPlaybackedFrameCnt);
        ALOGD("mRamp2ZeroFrameCnt %zu", mRamp2ZeroFrameCnt);
#endif
    } else {
        memset(&mStreamAttribute, 0, sizeof(mStreamAttribute));
        mBytePerSample = 0;
        mMinPreviewByte = 0;
        mHwBufTotalFrame = 0;
        mMinPreviewFrameCount = 0;
        mCheckHwExtraFrameCnt = 0;
        mSkipHwZCEFrameCnt = 0;
        mPlaybackedFrameCnt = 0;
        mRamp2ZeroFrameCnt = 0;
    }
    ALOGD("AudioALSANLECtrl ePlaybackHandlerType %d", ePlaybackHandlerType);
    mAattachPlaybackHandlerType = ePlaybackHandlerType;
    mAudioALSAPlaybackHandlerBase = NULL;
    mState = AUDIO_NLE_STATE_NONE;
    memset(&mLBuf, 0, sizeof(mLBuf));
    memset(&mRBuf, 0, sizeof(mRBuf));
}

AudioALSANLECtrl::~AudioALSANLECtrl() {
    freeResource();
    ALOGD("~AudioALSANLECtrl ePlaybackHandlerType %d", mAattachPlaybackHandlerType);
}

status_t AudioALSANLECtrl::checkValidForInit(playback_handler_t ePlaybackHandlerType) {
    if (mState != AUDIO_NLE_STATE_NONE) {
        ALOGW("[Warn] NLE init invalid mState [%d]", mState);
        return INVALID_OPERATION;
    } else if (!((mStreamAttribute.output_devices == AUDIO_DEVICE_OUT_WIRED_HEADSET) || (mStreamAttribute.output_devices == AUDIO_DEVICE_OUT_WIRED_HEADPHONE))) {
        ALOGW("[Warn] NLE init fail device 0x%x", mStreamAttribute.output_devices);
        return BAD_VALUE;
    } else if ((ePlaybackHandlerType != PLAYBACK_HANDLER_NORMAL) && (ePlaybackHandlerType != PLAYBACK_HANDLER_OFFLOAD) && (ePlaybackHandlerType != PLAYBACK_HANDLER_FAST)
               && (ePlaybackHandlerType != PLAYBACK_HANDLER_DEEP_BUFFER)) {
        ALOGW("[Warn] NLE init fail ePlaybackHandlerType %d", ePlaybackHandlerType);
        return BAD_VALUE;
    } else if (mStreamAttribute.audio_format != AUDIO_FORMAT_PCM_16_BIT && mStreamAttribute.audio_format != AUDIO_FORMAT_PCM_8_24_BIT) {
        ALOGW("[Warn] NLE init fail audio_format 0x%x", mStreamAttribute.audio_format);
        return BAD_VALUE;
    } else if (mStreamAttribute.audio_channel_mask != AUDIO_CHANNEL_IN_STEREO) {
        ALOGW("[Warn] NLE init fail audio_channel_mask 0x%x", mStreamAttribute.audio_channel_mask);
        return BAD_VALUE;
    } else if (mStreamAttribute.num_channels > NLE_SUPPORT_CH_NUMBER_MAX) {
        ALOGW("[Warn] NLE init fail num_channels %d", mStreamAttribute.num_channels);
        return BAD_VALUE;
    } else if (mStreamAttribute.sample_rate > NLE_SUPPORT_SAMPLING_RATE_MAX) {
        ALOGW("[Warn] NLE init fail sample_rate %d", mStreamAttribute.sample_rate);
        return BAD_VALUE;
    } else if ((mStreamAttribute.buffer_size < mMinPreviewByte) && ((1 << ePlaybackHandlerType) & AudioALSAHyBridNLEManager::getSupportRunNLEHandler())) {
        ALOGE("[Err] NLE init fail buffer_size %d (Need large than %zu)", mStreamAttribute.buffer_size, mMinPreviewByte);
        return BAD_VALUE;
    } else {
        return NO_ERROR;
    }
}

status_t AudioALSANLECtrl::freeResource(void) {
    if (mLBuf.pBufBase != NULL) {
        delete [] mLBuf.pBufBase;
        memset(&mLBuf, 0, sizeof(mLBuf));
    }

    if (mRBuf.pBufBase != NULL) {
        delete [] mRBuf.pBufBase;
        memset(&mRBuf, 0, sizeof(mRBuf));
    }

    return NO_ERROR;
}

status_t AudioALSANLECtrl::allocateResource(void) {
    size_t bufSize = (mStreamAttribute.buffer_size >> 1) + mCheckHwExtraFrameCnt * mBytePerSample;
    mLBuf.pBufBase = new char[bufSize];
    if (mLBuf.pBufBase == NULL) {
        ALOGE("[Err] %s Line %d allocate fail", __FUNCTION__, __LINE__);
        goto resource_lack;
    } else {
        mLBuf.bufLen = bufSize;
        mLBuf.pRead = mLBuf.pWrite = mLBuf.pBufBase;
        memset(mLBuf.pBufBase, 0, mLBuf.bufLen);
    }

    mRBuf.pBufBase = new char[bufSize];
    if (mRBuf.pBufBase == NULL) {
        ALOGE("[Err] %s Line %d allocate fail", __FUNCTION__, __LINE__);
        goto resource_lack;
    } else {
        mRBuf.bufLen = bufSize;
        mRBuf.pRead = mRBuf.pWrite = mRBuf.pBufBase;
        memset(mRBuf.pBufBase, 0, mRBuf.bufLen);
    }

    return NO_ERROR;
resource_lack:
    freeResource();

    return NO_MEMORY;
}

size_t AudioALSANLECtrl::writeToEachChannel(void *buffer, size_t Byte) {
    size_t frame_count = 0;
    size_t frame_1ch_count = 0;
    size_t RemainByte = Byte;
    size_t oneframe_byte = mBytePerSample * mStreamAttribute.num_channels;

    if (mBytePerSample == sizeof(int32_t)) { // 4 Byte
        int32_t *p32BufSrc = (int32_t *) buffer;
        int32_t *p32BufDstLwptr = (int32_t *) mLBuf.pWrite;
        int32_t *p32BufDstRwptr = (int32_t *) mRBuf.pWrite;
        size_t TailFreeByte = (size_t) mLBuf.bufLen - (size_t)(mLBuf.pWrite - mLBuf.pBufBase);//One Channnel
        size_t HeadFreeByte = (Byte / mStreamAttribute.num_channels) - TailFreeByte; //One Channnel
        frame_1ch_count = 0;
        while (TailFreeByte && RemainByte) {
            p32BufDstLwptr[frame_1ch_count] = p32BufSrc[(frame_count << 1)];
            p32BufDstRwptr[frame_1ch_count] = p32BufSrc[(frame_count << 1) + 1];
            frame_1ch_count++;
            frame_count++;
            TailFreeByte = TailFreeByte - mBytePerSample;
            RemainByte = RemainByte - oneframe_byte;
        }
        if (RemainByte) {
            p32BufDstLwptr = (int32_t *) mLBuf.pBufBase;
            p32BufDstRwptr = (int32_t *) mRBuf.pBufBase;
            frame_1ch_count = 0;
            while (HeadFreeByte) {
                p32BufDstLwptr[frame_1ch_count] = p32BufSrc[(frame_count << 1)];
                p32BufDstRwptr[frame_1ch_count] = p32BufSrc[(frame_count << 1) + 1];
                frame_1ch_count++;
                frame_count++;
                HeadFreeByte = HeadFreeByte - mBytePerSample;
            }
        }
        mLBuf.pWrite = ((char *)p32BufDstLwptr) + frame_1ch_count * mBytePerSample;
        mRBuf.pWrite = ((char *)p32BufDstRwptr) + frame_1ch_count * mBytePerSample;

    } else {
        int16_t *p16BufSrc = (int16_t *) buffer;
        int16_t *p16BufDstLwptr = (int16_t *) mLBuf.pWrite;
        int16_t *p16BufDstRwptr = (int16_t *) mRBuf.pWrite;
        size_t TailFreeByte = (size_t) mLBuf.bufLen - (size_t)(mLBuf.pWrite - mLBuf.pBufBase);
        size_t HeadFreeByte = (Byte / mStreamAttribute.num_channels) - TailFreeByte;
        frame_1ch_count = 0;
        while (TailFreeByte && RemainByte) {
            p16BufDstLwptr[frame_1ch_count] = p16BufSrc[(frame_count << 1)];
            p16BufDstRwptr[frame_1ch_count] = p16BufSrc[(frame_count << 1) + 1];
            frame_1ch_count++;
            frame_count++;
            TailFreeByte = TailFreeByte - mBytePerSample;
            RemainByte = RemainByte - oneframe_byte;
        }
        if (RemainByte) {
            p16BufDstLwptr = (int16_t *) mLBuf.pBufBase;
            p16BufDstRwptr = (int16_t *) mRBuf.pBufBase;
            frame_1ch_count = 0;
            while (HeadFreeByte) {
                p16BufDstLwptr[frame_1ch_count] = p16BufSrc[(frame_count << 1)];
                p16BufDstRwptr[frame_1ch_count] = p16BufSrc[(frame_count << 1) + 1];
                frame_1ch_count++;
                frame_count++;
                HeadFreeByte = HeadFreeByte - mBytePerSample;
            }
        }
        mLBuf.pWrite = ((char *)p16BufDstLwptr) + frame_1ch_count * mBytePerSample;
        mRBuf.pWrite = ((char *)p16BufDstRwptr) + frame_1ch_count * mBytePerSample;
    }

    return Byte;
}

status_t AudioALSANLECtrl::getMaxSPLandZeroCnt(NleHwInfo_t *pNleHwInfo, RingBuf *pRingBuf, size_t zuframecount, float *pSPL_Db, size_t *pZeroCnt, uint32_t maxValidValue) {
    if (pRingBuf == NULL || zuframecount == 0 || pSPL_Db == NULL || pZeroCnt == NULL || pNleHwInfo == NULL) {
        ALOGE("[Err] dRet = %d, %s %d ", INVALID_OPERATION, __FUNCTION__, __LINE__);
        return INVALID_OPERATION;
    }
    bool bPrevSignBit = false;
    size_t zuZeroCount = 0;
    size_t zuTempZeroCount = 0;
    size_t zuIndexOfMaxSPL = 0;
    if (mBytePerSample == sizeof(int32_t)) {
        uint32_t dMaxSPL = 1;
        uint32_t dTempSPL = 0;
        if ((pRingBuf->pBufBase + zuframecount * mBytePerSample) > pRingBuf->pWrite) { //Ring
            int32_t *p32BufSrc = (int32_t *)(pRingBuf->pWrite + pRingBuf->bufLen - zuframecount * mBytePerSample);
            size_t cnt1 = (size_t)((pRingBuf->pBufBase + pRingBuf->bufLen) - ((char *)p32BufSrc));
            cnt1 = cnt1 / mBytePerSample;
            size_t cnt2 = zuframecount - cnt1;
            if (cnt1) {
                if (cnt1 > mSkipHwZCEFrameCnt) {
                    bPrevSignBit = p32BufSrc[0 + mSkipHwZCEFrameCnt] & (1 << 23);
                }
            } else {
                p32BufSrc = (int32_t *) pRingBuf->pBufBase;
                if (cnt2 > mSkipHwZCEFrameCnt) {
                    bPrevSignBit = p32BufSrc[0 + mSkipHwZCEFrameCnt] & (1 << 23);
                }
            }
            //ALOGD("Base %x, Write %x Len %x Src %x cnt1 %d cnt2 %d t %d", pRingBuf->pBufBase, pRingBuf->pWrite, pRingBuf->bufLen, p32BufSrc, cnt1, cnt2, zuframecount);
            for (size_t i = 0; i < cnt1; i++) {
                if (p32BufSrc[i] < 0) {
                    if (i > mSkipHwZCEFrameCnt && bPrevSignBit == false) {
                        bPrevSignBit = true;
                        zuTempZeroCount++;
                    }
                    dTempSPL = abs(p32BufSrc[i]);
                    if (dMaxSPL < dTempSPL) {
                        dMaxSPL = dTempSPL;
                        zuIndexOfMaxSPL = i;
                        zuZeroCount = zuZeroCount + zuTempZeroCount;
                        zuTempZeroCount = 0;
                        if (dMaxSPL >= maxValidValue) {
                            break;
                        }
                    }
                } else {
                    if (i > mSkipHwZCEFrameCnt && bPrevSignBit == true) {
                        bPrevSignBit = false;
                        zuTempZeroCount++;
                    }
                    dTempSPL = (uint32_t) p32BufSrc[i];
                    if (dMaxSPL < dTempSPL) {
                        dMaxSPL = dTempSPL;
                        zuIndexOfMaxSPL = i;
                        zuZeroCount = zuZeroCount + zuTempZeroCount;
                        zuTempZeroCount = 0;
                        if (dMaxSPL >= maxValidValue) {
                            break;
                        }
                    }
                }
            }
            if (dMaxSPL < maxValidValue) {
                p32BufSrc = (int32_t *) pRingBuf->pBufBase;
                for (size_t i = 0; i < cnt2; i++) {
                    if (p32BufSrc[i] < 0) {
                        if (((i + cnt1) > mSkipHwZCEFrameCnt) && bPrevSignBit == false) {
                            bPrevSignBit = true;
                            zuTempZeroCount++;
                        }
                        dTempSPL = abs(p32BufSrc[i]);
                        if (dMaxSPL < dTempSPL) {
                            dMaxSPL = dTempSPL;
                            zuIndexOfMaxSPL = i + cnt1;
                            zuZeroCount = zuZeroCount + zuTempZeroCount;
                            zuTempZeroCount = 0;
                            if (dMaxSPL >= maxValidValue) {
                                break;
                            }
                        }
                    } else {
                        if (((i + cnt1) > mSkipHwZCEFrameCnt) && bPrevSignBit == true) {
                            bPrevSignBit = false;
                            zuTempZeroCount++;
                        }
                        dTempSPL = (uint32_t) p32BufSrc[i];
                        if (dMaxSPL < dTempSPL) {
                            dMaxSPL = dTempSPL;
                            zuIndexOfMaxSPL = i + cnt1;
                            zuZeroCount = zuZeroCount + zuTempZeroCount;
                            zuTempZeroCount = 0;
                            if (dMaxSPL >= maxValidValue) {
                                break;
                            }
                        }
                    }
                }
            }
            if (zuIndexOfMaxSPL < mPlaybackedFrameCnt) {
                zuZeroCount = zuZeroCount + zuTempZeroCount;
                zuTempZeroCount = 0;
            }
            if (pNleHwInfo->mMaxValue > dMaxSPL) {
                zuZeroCount = zuZeroCount + zuTempZeroCount;
                zuTempZeroCount = 0;
            }
        } else {
            int32_t *p32BufSrc = (int32_t *)(pRingBuf->pWrite - zuframecount * mBytePerSample);
            if (zuframecount > mSkipHwZCEFrameCnt) {
                bPrevSignBit = p32BufSrc[0 + mSkipHwZCEFrameCnt] & (1 << 23);
            }
            for (size_t i = 0; i < zuframecount; i++) {
                if (p32BufSrc[i] < 0) {
                    if (i > mSkipHwZCEFrameCnt && bPrevSignBit == false) {
                        bPrevSignBit = true;
                        zuTempZeroCount++;
                    }
                    dTempSPL = abs(p32BufSrc[i]);
                    if (dMaxSPL < dTempSPL) {
                        dMaxSPL = dTempSPL;
                        zuIndexOfMaxSPL = i;
                        zuZeroCount = zuZeroCount + zuTempZeroCount;
                        zuTempZeroCount = 0;
                        if (dMaxSPL >= maxValidValue) {
                            break;
                        }
                    }
                } else {
                    if (i > mSkipHwZCEFrameCnt && bPrevSignBit == true) {
                        bPrevSignBit = false;
                        zuTempZeroCount++;
                    }
                    dTempSPL = (uint32_t) p32BufSrc[i];
                    if (dMaxSPL < dTempSPL) {
                        dMaxSPL = dTempSPL;
                        zuIndexOfMaxSPL = i;
                        zuZeroCount = zuZeroCount + zuTempZeroCount;
                        zuTempZeroCount = 0;
                        if (dMaxSPL >= maxValidValue) {
                            break;
                        }
                    }
                }
            }
            if (zuIndexOfMaxSPL < mPlaybackedFrameCnt) {
                zuZeroCount = zuZeroCount + zuTempZeroCount;
                zuTempZeroCount = 0;
            }
            if (pNleHwInfo->mMaxValue > dMaxSPL) {
                zuZeroCount = zuZeroCount + zuTempZeroCount;
                zuTempZeroCount = 0;
            }
        }
        if (dMaxSPL >= maxValidValue) {
            *pSPL_Db = 0;   // Don't need to calc log dMaxSPL
        } else {
            if (NLE_24BIT_NAGATIVE_MAX == dMaxSPL) {
                dMaxSPL = NLE_24BIT_POSITIVE_MAX;
            }
            pNleHwInfo->mMaxValue = dMaxSPL;
            float fMaxSPL = ((float) dMaxSPL) / 8388607.0;
            *pSPL_Db = 20.0 * log10(fMaxSPL); // 8388607 = 2^23 -1;
        }
        //ALOGD("dMaxSPL %d %f *SPL_Db %f", dMaxSPL, fMaxSPL, *SPL_Db);
    } else {
        uint32_t dMaxSPL = 0;
        uint16_t dTempSPL = 0;
        if ((pRingBuf->pBufBase + zuframecount * mBytePerSample) > pRingBuf->pWrite) { //Ring
            int16_t *p16BufSrc = (int16_t *)(pRingBuf->pWrite + pRingBuf->bufLen - zuframecount * mBytePerSample);
            size_t cnt1 = (size_t)((pRingBuf->pBufBase + pRingBuf->bufLen) - ((char *)p16BufSrc));
            cnt1 = cnt1 / mBytePerSample;
            size_t cnt2 = zuframecount - cnt1;
            if (cnt1) {
                if (cnt1 > mSkipHwZCEFrameCnt) {
                    bPrevSignBit = p16BufSrc[0 + mSkipHwZCEFrameCnt] & (1 << 15);
                }
            } else {
                p16BufSrc = (int16_t *) pRingBuf->pBufBase;
                if (cnt2 > mSkipHwZCEFrameCnt) {
                    bPrevSignBit = p16BufSrc[0 + mSkipHwZCEFrameCnt] & (1 << 15);
                }
            }
            //ALOGD("Base %x, Write %x Len %x Src %x cnt1 %d cnt2 %d t %d", pRingBuf->pBufBase, pRingBuf->pWrite, pRingBuf->bufLen, p32BufSrc, cnt1, cnt2, zuframecount);
            for (size_t i = 0; i < cnt1; i++) {
                if (p16BufSrc[i] < 0) {
                    if (i > mSkipHwZCEFrameCnt && bPrevSignBit == false) {
                        bPrevSignBit = true;
                        zuTempZeroCount++;
                    }
                    dTempSPL = abs(p16BufSrc[i]);
                    if (dMaxSPL < dTempSPL) {
                        dMaxSPL = dTempSPL;
                        zuIndexOfMaxSPL = i;
                        zuZeroCount = zuZeroCount + zuTempZeroCount;
                        zuTempZeroCount = 0;
                        if (dMaxSPL >= maxValidValue) {
                            break;
                        }
                    }
                } else {
                    if (i > mSkipHwZCEFrameCnt && bPrevSignBit == true) {
                        bPrevSignBit = false;
                        zuTempZeroCount++;
                    }
                    dTempSPL = (uint16_t) p16BufSrc[i];
                    if (dMaxSPL < dTempSPL) {
                        dMaxSPL = dTempSPL;
                        zuIndexOfMaxSPL = i;
                        zuZeroCount = zuZeroCount + zuTempZeroCount;
                        zuTempZeroCount = 0;
                        if (dMaxSPL >= maxValidValue) {
                            break;
                        }
                    }
                }
            }
            if (dMaxSPL < maxValidValue) {
                p16BufSrc = (int16_t *) pRingBuf->pBufBase;
                for (size_t i = 0; i < cnt2; i++) {
                    if (p16BufSrc[i] < 0) {
                        if (((i + cnt1) > mSkipHwZCEFrameCnt) && bPrevSignBit == false) {
                            bPrevSignBit = true;
                            zuTempZeroCount++;
                        }
                        dTempSPL = abs(p16BufSrc[i]);
                        if (dMaxSPL < dTempSPL) {
                            dMaxSPL = dTempSPL;
                            zuIndexOfMaxSPL = i + cnt1;
                            zuZeroCount = zuZeroCount + zuTempZeroCount;
                            zuTempZeroCount = 0;
                            if (dMaxSPL >= maxValidValue) {
                                break;
                            }
                        }
                    } else {
                        if (((i + cnt1) > mSkipHwZCEFrameCnt) && bPrevSignBit == true) {
                            bPrevSignBit = false;
                            zuTempZeroCount++;
                        }
                        dTempSPL = (uint16_t) p16BufSrc[i];
                        if (dMaxSPL < dTempSPL) {
                            dMaxSPL = dTempSPL;
                            zuIndexOfMaxSPL = i + cnt1;
                            zuZeroCount = zuZeroCount + zuTempZeroCount;
                            zuTempZeroCount = 0;
                            if (dMaxSPL >= maxValidValue) {
                                break;
                            }
                        }
                    }
                }
            }
            if (zuIndexOfMaxSPL < mPlaybackedFrameCnt) {
                zuZeroCount = zuZeroCount + zuTempZeroCount;
                zuTempZeroCount = 0;
            }
            if (pNleHwInfo->mMaxValue > dMaxSPL) {
                zuZeroCount = zuZeroCount + zuTempZeroCount;
                zuTempZeroCount = 0;
            }
        } else {
            int16_t *p16BufSrc = (int16_t *)(pRingBuf->pWrite - zuframecount * mBytePerSample);
            if (zuframecount > mSkipHwZCEFrameCnt) {
                bPrevSignBit = p16BufSrc[0 + mSkipHwZCEFrameCnt] & (1 << 15);
            }
            for (size_t i = 0; i < zuframecount; i++) {
                if (p16BufSrc[i] < 0) {
                    dTempSPL = abs(p16BufSrc[i]);
                    if (i > mSkipHwZCEFrameCnt && bPrevSignBit == false) {
                        bPrevSignBit = true;
                        zuTempZeroCount++;
                    }
                    if (dMaxSPL < dTempSPL) {
                        dMaxSPL = dTempSPL;
                        zuIndexOfMaxSPL = i;
                        zuZeroCount = zuZeroCount + zuTempZeroCount;
                        zuTempZeroCount = 0;
                        if (dMaxSPL >= maxValidValue) {
                            break;
                        }
                    }
                } else {
                    if (i > mSkipHwZCEFrameCnt && bPrevSignBit == true) {
                        bPrevSignBit = false;
                        zuTempZeroCount++;
                    }
                    dTempSPL = (uint16_t) p16BufSrc[i];
                    if (dMaxSPL < dTempSPL) {
                        dMaxSPL = dTempSPL;
                        zuIndexOfMaxSPL = i;
                        zuZeroCount = zuZeroCount + zuTempZeroCount;
                        zuTempZeroCount = 0;
                        if (dMaxSPL >= maxValidValue) {
                            break;
                        }
                    }
                }
            }
            if (zuIndexOfMaxSPL < mPlaybackedFrameCnt) {
                zuZeroCount = zuZeroCount + zuTempZeroCount;
                zuTempZeroCount = 0;
            }
            if (pNleHwInfo->mMaxValue > dMaxSPL) {
                zuZeroCount = zuZeroCount + zuTempZeroCount;
                zuTempZeroCount = 0;
            }
        }
        if (dMaxSPL >= maxValidValue) {
            *pSPL_Db = 0;   // Don't need to calc log dMaxSPL
        } else {
            if (NLE_16BIT_NAGATIVE_MAX == dMaxSPL) {
                dMaxSPL = NLE_16BIT_POSITIVE_MAX;
            }
            pNleHwInfo->mMaxValue = dMaxSPL;
            *pSPL_Db = 20.0 * log10(dMaxSPL / 32767); // 32767 = 2^15 -1;
        }
    }
    //zuZeroCount = zuZeroCount + zuTempZeroCount; Debug only
    *pZeroCnt = zuZeroCount;
    return NO_ERROR;
}

status_t AudioALSANLECtrl::setMaxSPLintoSpecificPos(size_t zuOffsetForTail) {
    size_t mIndexOfChangeValue = 0;
    if (zuOffsetForTail >= mHwBufTotalFrame + mCheckHwExtraFrameCnt) {
        ALOGE("[Err] %s %zu > %zu + %zu", __FUNCTION__, zuOffsetForTail, mHwBufTotalFrame, mCheckHwExtraFrameCnt);
        return BAD_VALUE;
    }

    if ((mLBuf.pBufBase + zuOffsetForTail * mBytePerSample) > mLBuf.pWrite) { //Ring
        mIndexOfChangeValue = (mLBuf.pWrite - mLBuf.pBufBase + mLBuf.bufLen - zuOffsetForTail * mBytePerSample) / mBytePerSample;
    } else {
        mIndexOfChangeValue = (mLBuf.pWrite - mLBuf.pBufBase - zuOffsetForTail * mBytePerSample) / mBytePerSample;
    }

    if (mIndexOfChangeValue == 0) {
        mIndexOfChangeValue = mHwBufTotalFrame + mCheckHwExtraFrameCnt - 1;
    } else {
        mIndexOfChangeValue = mIndexOfChangeValue - 1;
    }
    ALOGVV("mIndexOfChangeValue %zu Of %zu (Write %zu)", mIndexOfChangeValue, mHwBufTotalFrame + mCheckHwExtraFrameCnt, (mLBuf.pWrite - mLBuf.pBufBase) / mBytePerSample);

    if (mStreamAttribute.audio_format == AUDIO_FORMAT_PCM_16_BIT) {
        int16_t *p16BufSrc = (int16_t *)(mLBuf.pBufBase);
        p16BufSrc[mIndexOfChangeValue] = NLE_16BIT_POSITIVE_MAX;
        p16BufSrc = (int16_t *)(mRBuf.pBufBase);
        p16BufSrc[mIndexOfChangeValue] = NLE_16BIT_POSITIVE_MAX;
    } else {
        int32_t *p32BufSrc = (int32_t *)(mLBuf.pBufBase);
        p32BufSrc[mIndexOfChangeValue] = NLE_24BIT_POSITIVE_MAX;
        p32BufSrc = (int32_t *)(mRBuf.pBufBase);
        p32BufSrc[mIndexOfChangeValue] = NLE_24BIT_POSITIVE_MAX;
    }

    return NO_ERROR;
}

status_t AudioALSANLECtrl::init(AudioALSAPlaybackHandlerBase *pPlaybackHandler) {
    status_t eRet;

    AL_AUTOLOCK(mLock);

    eRet = checkValidForInit(mAattachPlaybackHandlerType);
    if (eRet != NO_ERROR) {
        return eRet;
    }

    eRet = allocateResource();
    if (eRet != NO_ERROR) {
        return eRet;
    }

    if (mStreamAttribute.audio_format == AUDIO_FORMAT_PCM_16_BIT) {
        int16_t *p16BufSrc = (int16_t *)(mLBuf.pBufBase);
        p16BufSrc[mHwBufTotalFrame + mCheckHwExtraFrameCnt - 1] = NLE_16BIT_POSITIVE_MAX;
        p16BufSrc = (int16_t *)(mRBuf.pBufBase);
        p16BufSrc[mHwBufTotalFrame + mCheckHwExtraFrameCnt - 1] = NLE_16BIT_POSITIVE_MAX;
    } else {
        int32_t *p32BufSrc = (int32_t *)(mLBuf.pBufBase);
        p32BufSrc[mHwBufTotalFrame + mCheckHwExtraFrameCnt - 1] = NLE_24BIT_POSITIVE_MAX;
        p32BufSrc = (int32_t *)(mRBuf.pBufBase);
        p32BufSrc[mHwBufTotalFrame + mCheckHwExtraFrameCnt - 1] = NLE_24BIT_POSITIVE_MAX;
    }

    mAudioALSAPlaybackHandlerBase = pPlaybackHandler;

    mState = AUDIO_NLE_STATE_INITIALIZED;

    return NO_ERROR;
}

status_t AudioALSANLECtrl::deinit(void) {
    AL_AUTOLOCK(mLock);

    if (mState == AUDIO_NLE_STATE_NONE) {
        //ALOGE("[Err] NLE deinit invalid mState [%d]", mState);
        return INVALID_OPERATION;
    }

    freeResource();

    mState = AUDIO_NLE_STATE_NONE;

    return NO_ERROR;
}

size_t AudioALSANLECtrl::process(void *buffer, size_t Byte) {
    size_t dWriteByte = 0;
    status_t eRet;

    AL_AUTOLOCK(mLock);

    if (mState == AUDIO_NLE_STATE_NONE) {
        //ALOGE("[Err] NLE process invalid mState [%d]", mState);
        return 0;
    }

    if (buffer == NULL || Byte > mStreamAttribute.buffer_size) {
        ALOGE("[Err] NLE process buffer [0x%p] size [%zu] (should be less than [%d])", buffer, Byte, mStreamAttribute.buffer_size);
        return 0;
    }

    if (mState == AUDIO_NLE_STATE_INITIALIZED) {
        mState = AUDIO_NLE_STATE_FIRST_WRITE;
    } else if (mState == AUDIO_NLE_STATE_FIRST_WRITE) {
        mState = AUDIO_NLE_STATE_RUNNING;
    }

    return writeToEachChannel(buffer, Byte);
}

#else
AudioALSAHyBridNLEManager::AudioALSAHyBridNLEManager() {
    mGainHP_Db_Max = NLE_GAIN_HP_DB_USER;
    mGainHP_Db_Min = NLE_GAIN_HP_DB_MIN;
    mGainNLE_Db_Min = NLE_GAIN_NLE_DB_MIN;
    mGainEop_Db = NLE_GAIN_EOP_DB;
    mGainNLE_Db_Max = mGainHP_Db_Max - mGainHP_Db_Min;
    mGainSop_Db = mGainEop_Db - mGainNLE_Db_Max;
    mStatus = AUDIO_HYBRID_NLE_MNG_STATE_DISABLE;
    mActivePBHandler = PLAYBACK_HANDLER_BASE;
    mActivePBHandlerBitwise = 0;
    mInitPBHandlerBitwise = 0;
    memset(&mLNleHwInfo, 0, sizeof(mLNleHwInfo));
    memset(&mRNleHwInfo, 0, sizeof(mRNleHwInfo));
    mMode = AUDIO_MODE_NORMAL;
    memset(&mNleCtrlOfPlaybackHandler[0], 0, sizeof(AudioALSANLECtrl *)*PLAYBACK_HANDLER_MAX);
    mNleCtrlOfPlaybackHandlerCounter = 0;
    mHwPathStreamCounter = 0;
    mBypassNLE = false;
    mNleSwitch = false;
    mMixer = NULL;
    mHwAnalogGainMaxDb = NLE_GAIN_HP_DB_MAX;
    mHwAnalogGainMinDb = NLE_GAIN_HP_DB_MIN;
    mHwAnalogGainMinValue = NLE_GAIN_HP_VALUE_MIN;
    mHwAnalogGainMuteDb = NLE_SUPPORT_ANA_GAIN_MUTE_DB;
    mHwAnalogGainMuteValue = NLE_SUPPORT_ANA_GAIN_MUTE_VALUE;
    mHwDigitalGainMaxDb = NLE_GAIN_NLE_DB_MAX;
    mHwDigitalGainMaxValue = NLE_GAIN_NLE_VALUE_MAX;
    mHwDigitalgGainMinDb = NLE_GAIN_NLE_DB_MIN;
    mHwAnalogDelayTick = NLE_DELAY_ANA_OUTPUT_T;
    mHwSupport = false;
    mSetEopByProp = false;
    mForceTurnOffNLE = false;
    mMax24BitValidValue = NLE_24BIT_POSITIVE_MAX;
    mMax16BitValidValue = NLE_16BIT_POSITIVE_MAX;
}


status_t AudioALSAHyBridNLEManager::setNleHwConfigByDb(int hpMaxGainDb __unused, int eopGainDb __unused) {
    return INVALID_OPERATION;
}

status_t AudioALSAHyBridNLEManager::setNleHwConfigByIndex(size_t hpMaxGainIdx __unused) {
    return INVALID_OPERATION;
}

status_t AudioALSAHyBridNLEManager::getNleHwConfigByDb(int *hpMaxGainDb __unused) {
    return INVALID_OPERATION;
}

status_t AudioALSAHyBridNLEManager::getNleHwConfigByIndex(size_t *hpMaxGainIdx __unused) {
    return INVALID_OPERATION;
}

audio_hybridnlemng_status AudioALSAHyBridNLEManager::getStatus(void) {
    return AUDIO_HYBRID_NLE_MNG_STATE_DISABLE;
}

status_t AudioALSAHyBridNLEManager::initPlayBackHandler(playback_handler_t ePlaybackHandlerType __unused, stream_attribute_t *pStreamAttribute __unused, AudioALSAPlaybackHandlerBase *pPlaybackHandler __unused) {
    return INVALID_OPERATION;
}

status_t AudioALSAHyBridNLEManager::deinitPlayBackHandler(playback_handler_t ePlaybackHandlerType __unused) {
    return INVALID_OPERATION;
}
status_t AudioALSAHyBridNLEManager::process(playback_handler_t ePlaybackHandlerType __unused, void *buffer __unused, size_t Byte __unused) {
    return INVALID_OPERATION;
}

status_t AudioALSAHyBridNLEManager::setBypassNLE(bool bBypass __unused) {
    return INVALID_OPERATION;
}

bool AudioALSAHyBridNLEManager::getBypassNLE(void) {
    return true;
}

status_t AudioALSAHyBridNLEManager::setEnableNLE(bool bEnable __unused) {
    return INVALID_OPERATION;
}

status_t AudioALSAHyBridNLEManager::dump(void) {
    return INVALID_OPERATION;
}

uint32_t AudioALSAHyBridNLEManager::getSupportRunNLEHandler(void) {
    return mSupportRunNLEHandlerBitwise;
}

status_t AudioALSAHyBridNLEManager::setSupportRunNLEHandler(playback_handler_t eHandler __unused) {
    return INVALID_OPERATION;
}

status_t AudioALSAHyBridNLEManager::setNleEopDb(int eopGainDb __unused) {
    return INVALID_OPERATION;
}

int AudioALSAHyBridNLEManager::getNleEopDb(void) {
    return -96;
}

status_t AudioALSAHyBridNLEManager::setAudioMode(audio_mode_t eMode __unused) {
    return INVALID_OPERATION;
}

status_t AudioALSAHyBridNLEManager::addHwPathStream(audio_stream_hw_path eHwPath __unused) {
    return INVALID_OPERATION;
}

status_t AudioALSAHyBridNLEManager::removeHwPathStream(audio_stream_hw_path eHwPath __unused) {
    return INVALID_OPERATION;
}

status_t AudioALSAHyBridNLEManager::updateCurStatus(void) {
    return INVALID_OPERATION;
}

status_t AudioALSAHyBridNLEManager::enableNleHw(void) {
    return INVALID_OPERATION;
}

status_t AudioALSAHyBridNLEManager::disableNleHw(void) {
    return INVALID_OPERATION;
}

status_t AudioALSAHyBridNLEManager::ramp2DigitalGainZero(void) {
    return INVALID_OPERATION;
}

bool AudioALSAHyBridNLEManager::checkIfGainTargeted(void) {
    return INVALID_OPERATION;
}

status_t AudioALSAHyBridNLEManager::waitForBypass(audio_hybridnlemng_status ePrevStatus __unused, audio_hybridnlemng_status eCurStatus __unused) {
    return INVALID_OPERATION;
}

status_t AudioALSAHyBridNLEManager::doAdjustGainProcess(AudioALSANLECtrl *pCurNLECtrl __unused, size_t zuCheckOffsetFrameCnt __unused, audio_hybrid_adjust_mode eAdjustMode __unused) {
    return INVALID_OPERATION;
}

status_t AudioALSAHyBridNLEManager::getDbFromAnalogIdx(size_t AnalogIdx __unused, int8_t *pDb __unused) {
    return INVALID_OPERATION;
}

status_t AudioALSAHyBridNLEManager::getAnalogIdx(int8_t Db __unused, size_t *pAnalogIdx __unused) {
    return INVALID_OPERATION;
}

status_t AudioALSAHyBridNLEManager::getDbFromDigitalIdx(size_t DigitalIdx __unused, int8_t *pDb __unused) {
    return INVALID_OPERATION;
}

status_t AudioALSAHyBridNLEManager::getDigitalIdx(int8_t Db __unused, size_t *pDigitalIdx __unused) {
    return INVALID_OPERATION;
}

status_t AudioALSAHyBridNLEManager::gainAllocatePlan(float Spl __unused, size_t ZeroCnt __unused, size_t Zuframecount __unused, NleHwInfo_t *pNleHwInfo __unused) {
    return INVALID_OPERATION;
}

status_t AudioALSAHyBridNLEManager::applyGain2Hw(NleHwInfo_t *pNleHwInfo __unused) {
    return INVALID_OPERATION;
}

status_t AudioALSAHyBridNLEManager::resetDefaultGain(NleHwInfo_t *pNleHwInfo __unused) {
    return INVALID_OPERATION;
}

status_t AudioALSAHyBridNLEManager::getGstepAndGnum(int32_t SPL_diff __unused, size_t ZeroCnt __unused, int32_t *pdGstep __unused, int32_t *pdGnum __unused) {
    return INVALID_OPERATION;
}

status_t AudioALSAHyBridNLEManager::checkAndSetGain(size_t zuframecount __unused, AudioALSANLECtrl *pNleCtrl __unused, NleHwInfo_t *pNleHwInfo __unused, RingBuf *pRingBuf __unused) {
    return INVALID_OPERATION;
}

AudioALSANLECtrl::AudioALSANLECtrl(playback_handler_t ePlaybackHandlerType __unused, const stream_attribute_t *pStream_Attribute_Source __unused) {
    memset(&mStreamAttribute, 0, sizeof(mStreamAttribute));
    mBytePerSample = 0;
    mMinPreviewByte = 0;
    mHwBufTotalFrame = 0;
    mMinPreviewFrameCount = 0;
    mCheckHwExtraFrameCnt = 0;
    mSkipHwZCEFrameCnt = 0;
    mPlaybackedFrameCnt = 0;
    mRamp2ZeroFrameCnt = 0;
    mAattachPlaybackHandlerType = PLAYBACK_HANDLER_BASE;
    mAudioALSAPlaybackHandlerBase = NULL;
    mState = AUDIO_NLE_STATE_NONE;
    memset(&mLBuf, 0, sizeof(mLBuf));
    memset(&mRBuf, 0, sizeof(mRBuf));
}

AudioALSANLECtrl::~AudioALSANLECtrl() {
}

status_t AudioALSANLECtrl::checkValidForInit(playback_handler_t ePlaybackHandlerType __unused) {
    return INVALID_OPERATION;
}

status_t AudioALSANLECtrl::freeResource(void) {
    return INVALID_OPERATION;
}

status_t AudioALSANLECtrl::allocateResource(void) {
    return INVALID_OPERATION;
}

size_t AudioALSANLECtrl::writeToEachChannel(void *buffer __unused, size_t Byte __unused) {
    return 0;
}

status_t AudioALSANLECtrl::getMaxSPLandZeroCnt(NleHwInfo_t *pNleHwInfo __unused, RingBuf *pRingBuf __unused, size_t zuframecount __unused, float *pSPL_Db __unused, size_t *pZeroCnt __unused, uint32_t maxValidValue __unused) {
    return INVALID_OPERATION;
}

status_t AudioALSANLECtrl::setMaxSPLintoSpecificPos(size_t zuOffsetForTail __unused) {
    return INVALID_OPERATION;
}

status_t AudioALSANLECtrl::init(AudioALSAPlaybackHandlerBase *pPlaybackHandler __unused) {
    return INVALID_OPERATION;
}

status_t AudioALSANLECtrl::deinit(void) {
    return INVALID_OPERATION;
}

size_t AudioALSANLECtrl::process(void *buffer, size_t Byte) {
    return writeToEachChannel(buffer, Byte);
}
#endif

}

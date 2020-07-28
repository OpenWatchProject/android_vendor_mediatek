#ifndef ANDROID_SPEECH_DRIVER_INTERFACE_H
#define ANDROID_SPEECH_DRIVER_INTERFACE_H

#include <stdint.h>
#include <stdbool.h>
#include <string.h>

#include <utils/threads.h>
#include <log/log.h>
#include <system/audio.h>

#include "AudioAssert.h"
#include "SpeechType.h"



namespace android {


const int16_t kUnreasonableGainValue = 0x8000;

/**
 * callback function prototype
 */
typedef void (*nwRateChangeCbFunc)(void *);


class SpeechDriverInterface {
public:
    virtual ~SpeechDriverInterface() {}

    /**
     * speech control
     */
    virtual status_t SetSpeechMode(const audio_devices_t input_device, const audio_devices_t output_device) = 0;
    virtual status_t SpeechOn() = 0;
    virtual status_t SpeechOff() = 0;
    virtual status_t VideoTelephonyOn() = 0;
    virtual status_t VideoTelephonyOff() = 0;
    virtual status_t SpeechRouterOn() = 0;
    virtual status_t SpeechRouterOff() = 0;
    virtual status_t setMDVolumeIndex(int value, int device, int index) = 0;


    /**
     * record control
     */
    virtual status_t recordOn() = 0;
    virtual status_t recordOff() = 0;

    virtual status_t VoiceMemoRecordOn() = 0;
    virtual status_t VoiceMemoRecordOff() = 0;

    virtual uint16_t GetRecordSampleRate() const = 0;
    virtual uint16_t GetRecordChannelNumber() const = 0;

    virtual status_t recordOn(SpcRecordTypeStruct typeRecord) = 0;
    virtual status_t recordOff(SpcRecordTypeStruct typeRecord) = 0;

    virtual status_t setPcmRecordType(SpcRecordTypeStruct typeRecord) = 0;


    /**
     * background sound control
     */
    virtual status_t BGSoundOn() = 0;
    virtual status_t BGSoundConfig(uint8_t ul_gain, uint8_t dl_gain) = 0;
    virtual status_t BGSoundOff() = 0;


    /**
     * voip rx on/off
     */
    virtual status_t VoipRxOn() { return -ENOSYS; }
    virtual status_t VoipRxOff() { return -ENOSYS; }
    virtual status_t VoipRxConfig(const uint8_t direction __unused, const uint8_t mixType __unused) {
        return -ENOSYS;
    }


    /**
     * telephony tx control
     */
    virtual status_t TelephonyTxOn() { return -ENOSYS; }
    virtual status_t TelephonyTxConfig(uint8_t ul_gain __unused, uint8_t dl_gain __unused) { return -ENOSYS; }
    virtual status_t TelephonyTxOff() { return -ENOSYS; }


    /**
     * pcm 2 way
     */
    virtual status_t PCM2WayPlayOn() { return -ENOSYS; }
    virtual status_t PCM2WayPlayOff() { return -ENOSYS; }
    virtual status_t PCM2WayRecordOn() { return -ENOSYS; }
    virtual status_t PCM2WayRecordOff() { return -ENOSYS; }
    virtual status_t PCM2WayOn(const bool wideband_on) = 0;
    virtual status_t PCM2WayOff() = 0;


    /**
     * tty ctm control
     */
    virtual status_t TtyCtmOn() = 0;
    virtual status_t TtyCtmOff() = 0;
    virtual status_t TtyCtmDebugOn(bool tty_debug_flag __unused) { return 0; }
    virtual int setTtyMode(const TtyModeType ttyMode) {
        ALOGD("%s(), ttyMode = %d, old mTtyMode = %d", __FUNCTION__, ttyMode, mTtyMode);
        mTtyMode = ttyMode;
        return 0;
    }

    /**
     * rtt control
     */
    virtual int RttConfig(int rttMode __unused) { return -ENOSYS; }

    /**
     * acoustic loopback
     */
    virtual status_t SetAcousticLoopback(bool loopback_on) = 0;
    virtual status_t SetAcousticLoopbackBtCodec(bool enable_codec) = 0;

    virtual status_t SetAcousticLoopbackDelayFrames(int32_t delay_frames) = 0;
    virtual status_t setLpbkFlag(bool enableLpbk) = 0;

    /**
     * Modem Audio DVT and Debug
     */
    virtual status_t SetModemLoopbackPoint(uint16_t loopback_point) = 0;

    /**
     * encryption
     */
    virtual status_t SetEncryption(bool encryption_on __unused) { return -ENOSYS; }

    /**
     * volume control
     */
    /**
     * param gain: data range is 0~0xFF00, which is mapping to 0dB to -64dB
     *             The effective interval is 0.25dB by data +/- 1
     */
    virtual status_t SetDownlinkGain(int16_t gain) = 0;
    int16_t          GetDownlinkGain(void) { return mDownlinkGain; }

    virtual status_t SetEnh1DownlinkGain(int16_t gain) = 0;
    int16_t          GetEnh1DownlinkGain(void) { return mDownlinkenh1Gain; }

    /**
     * param gain: data range is 0~120, which is mapping to 0dB to 30dB
     *             The effective interval is 1dB by data +/- 4
     */
    virtual status_t SetUplinkGain(int16_t gain) = 0;

    virtual status_t SetDownlinkMute(bool mute_on) = 0;
    virtual status_t SetUplinkMute(bool mute_on) = 0;
    virtual status_t SetUplinkSourceMute(bool mute_on) = 0;
    virtual void     setMuteInfo(uint32_t muteInfo) { mMuteInfo = muteInfo; }

    virtual status_t SetSidetoneGain(int16_t gain) = 0;
    virtual status_t SetDSPSidetoneFilter(const bool dsp_stf_on __unused) { return NO_INIT; }
    virtual status_t SetDownlinkMuteCodec(bool mute_on) = 0;


    /**
     * device related config
     */
    virtual status_t SetModemSideSamplingRate(uint16_t sample_rate) = 0;
    virtual status_t switchBTMode(uint32_t sample_rate __unused) { return -ENOSYS; }
    virtual void     setBTMode(const int mode __unused) { return; }
    virtual void     setBtSpkDevice(const bool flag __unused) { return; }


    /**
     * speech enhancement control
     */
    virtual void     SetForceDisableSpeechEnhancement(bool force_disable_on) { mForceDisableSpeechEnhancement = force_disable_on; }
    virtual status_t SetSpeechEnhancement(bool enhance_on) = 0;
    virtual status_t SetSpeechEnhancementMask(const sph_enh_mask_struct_t &mask) = 0;

    virtual status_t SetBtHeadsetNrecOn(const bool bt_headset_nrec_on) = 0;


    /**
     * speech enhancement parameters setting
     */
    virtual status_t SetNBSpeechParameters(const void *pSphParamNB __unused) { return -ENOSYS; }
    virtual status_t SetDualMicSpeechParameters(const void *pSphParamDualMic __unused) { return -ENOSYS; }
    virtual status_t SetMagiConSpeechParameters(const void *pSphParamMagiCon __unused) { return -ENOSYS; }
    virtual status_t SetHACSpeechParameters(const void *pSphParamHAC __unused) { return -ENOSYS; }
    virtual status_t SetWBSpeechParameters(const void *pSphParamWB __unused) { return -ENOSYS; }

    virtual status_t GetVibSpkParam(void *eVibSpkParam __unused) { return -ENOSYS; }
    virtual status_t SetVibSpkParam(void *eVibSpkParam __unused) { return -ENOSYS; }

    virtual status_t SetDynamicSpeechParameters(const int type __unused, const void *param_arg __unused) { return -ENOSYS; }
    virtual int updateSpeechFeature(const SpeechFeatureType featureType __unused, const bool flagOn __unused) { return -ENOSYS; }
    virtual int setParameter(const char *keyParameter  __unused) { return -ENOSYS; }
    virtual const char *getParameter(const char *keyParameter  __unused) { return NULL; }

    /**
     * check whether modem is ready.
     */
    virtual bool     CheckModemIsReady() = 0;


    /**
     * get AP side modem function status
     */
    inline bool      GetApSideModemStatus(const modem_status_mask_t modem_status_mask) {
        return ((mApSideModemStatus & modem_status_mask) > 0);
    }


    /**
     * speech driver synchronization
     */
    void WaitUntilSignaledOrTimeout(unsigned milisecond) {
        mMutex.lock();
        status_t retval = mCondition.waitRelative(mMutex, milliseconds(milisecond));
        mMutex.unlock();
        return;
    }
    void Signal() { mMutex.lock(); mCondition.signal(); mMutex.unlock();}
    virtual void SetWarningTone(int toneid __unused) { return; }
    virtual void StopWarningTone() {return;}


    void setBtHeadsetName(const char *bt_headset_name) {
        if (bt_headset_name != NULL && strlen(bt_headset_name) > 0) {
            ALOGV("%s(), mBtHeadsetName: %s => %s", __FUNCTION__, mBtHeadsetName, bt_headset_name);
            strncpy(mBtHeadsetName, bt_headset_name, sizeof(mBtHeadsetName) - 1);
        }
    }

    virtual void setNetworkChangeCallback(void *arg __unused, nwRateChangeCbFunc callback __unused) { return; }

protected:
    SpeechDriverInterface() {
        CleanGainValueAndMuteStatus();
        mPCM2WayState = 0;
        mApSideModemStatus = 0;
        mRecordSampleRateType = RECORD_SAMPLE_RATE_08K;
        mRecordChannelType    = RECORD_CHANNEL_MONO;
        mRecordType.direction = RECORD_TYPE_MIX;
        mRecordType.dlPosition = RECORD_POS_DL_END;

        mBtHeadsetNrecOn = true;
        mAcousticLoopbackDelayFrames = 0;
        mForceDisableSpeechEnhancement = false;
        mUseBtCodec = true;
        mModemIndex = MODEM_1;
        mVolumeIndex = 0;
        mTtyMode = AUD_TTY_OFF;
        mMuteInfo = 0;

        memset(mBtHeadsetName, 0, sizeof(mBtHeadsetName));
    }


    /**
     * recover modem side status (speech/record/bgs/vt/p2w/tty)
     */
    virtual void     RecoverModemSideStatusToInitState() = 0;


    /**
     * Clean gain value and mute status
     */
    virtual void     CleanGainValueAndMuteStatus() {
        mDownlinkGain   = kUnreasonableGainValue;
        mDownlinkenh1Gain = kUnreasonableGainValue;
        mUplinkGain     = kUnreasonableGainValue;
        mSideToneGain   = kUnreasonableGainValue;
        mUplinkMuteOn   = false;
        mUplinkSourceMuteOn   = false;
        mDownlinkMuteOn = false;
        ALOGD("%s(), mUplinkMuteOn %d", __FUNCTION__, mUplinkMuteOn);
    }


    /**
     * set/reset AP side modem function status
     */
    inline void      SetApSideModemStatus(const modem_status_mask_t modem_status_mask) {
        Mutex::Autolock _l(mUpdateApSideModemStatusLock);
        ASSERT(GetApSideModemStatus(modem_status_mask) == false);
        mApSideModemStatus |= modem_status_mask;
    }
    inline void      ResetApSideModemStatus(const modem_status_mask_t modem_status_mask) {
        Mutex::Autolock _l(mUpdateApSideModemStatusLock);
        ASSERT(GetApSideModemStatus(modem_status_mask) == true);
        mApSideModemStatus &= (~modem_status_mask);
    }


    /**
     * check AP side modem function status
     */
    inline void      CheckApSideModemStatusAllOffOrDie() {
        if (mApSideModemStatus != 0) {
            ALOGE("%s(), mApSideModemStatus 0x%x != 0",
                  __FUNCTION__, mApSideModemStatus);
            ASSERT(mApSideModemStatus == 0);
        }
    }


    /**
     * class variables
     */
    modem_index_t    mModemIndex;

    int16_t          mDownlinkGain;
    int16_t          mDownlinkenh1Gain;
    int16_t          mUplinkGain;
    int16_t          mSideToneGain;

    bool             mDownlinkMuteOn;
    bool             mUplinkMuteOn;
    bool             mUplinkSourceMuteOn;
    uint32_t         mMuteInfo;

    uint32_t         mPCM2WayState; // value |= pcmnway_format_t

    // Modem function status : not the modem real status but AP side control status
    uint32_t         mApSideModemStatus; // value |= modem_status_mask_t

    bool             mForceDisableSpeechEnhancement;

    Mutex            mMutex;
    Condition        mCondition;
    Mutex            mUpdateApSideModemStatusLock;

    // Record capability
    record_sample_rate_t mRecordSampleRateType;
    record_channel_t     mRecordChannelType;
    SpcRecordTypeStruct  mRecordType;

    //for BT SW BT CVSD loopback test
    bool mUseBtCodec;

    // BT Headset NREC
    bool mBtHeadsetNrecOn;

    // loopback delay frames (1 frame = 20 ms)
    uint32_t mAcousticLoopbackDelayFrames;

    // volume index
    int16_t mVolumeIndex;

    char mBtHeadsetName[128];

    /* TTY Mode */
    TtyModeType mTtyMode;

    static uint32_t mNetworkRate; // phone_call_network_t

};


} // end namespace android

#endif // end of ANDROID_SPEECH_DRIVER_INTERFACE_H

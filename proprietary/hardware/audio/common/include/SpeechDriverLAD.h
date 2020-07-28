#ifndef ANDROID_SPEECH_DRIVER_LAD_H
#define ANDROID_SPEECH_DRIVER_LAD_H

#include "SpeechDriverInterface.h"
#include <AudioLock.h>
#include "AudioUtility.h"

namespace android {

class SpeechMessengerInterface;

class SpeechDriverLAD : public SpeechDriverInterface {
public:
    virtual ~SpeechDriverLAD();


    /**
     * get instance's pointer
     */
    static SpeechDriverLAD *GetInstance(modem_index_t modem_index);


    /**
     * speech control
     */
    virtual speech_mode_t GetSpeechModeByOutputDevice(const audio_devices_t output_device); // only available for LAD
    virtual status_t SetSpeechMode(const audio_devices_t input_device, const audio_devices_t output_device);
    virtual status_t SpeechOn();
    virtual status_t SpeechOff();
    virtual status_t VideoTelephonyOn();
    virtual status_t VideoTelephonyOff();
    virtual status_t SpeechRouterOn();
    virtual status_t SpeechRouterOff();

    virtual status_t setMDVolumeIndex(int stream, int device, int index);

    /**
     * record control
     */
    virtual status_t recordOn();
    virtual status_t recordOff();
    virtual status_t recordOn(SpcRecordTypeStruct typeRecord);
    virtual status_t recordOff(SpcRecordTypeStruct typeRecord);
    virtual status_t setPcmRecordType(SpcRecordTypeStruct typeRecord);

    virtual status_t VoiceMemoRecordOn();
    virtual status_t VoiceMemoRecordOff();

    virtual uint16_t GetRecordSampleRate() const;
    virtual uint16_t GetRecordChannelNumber() const;


    /**
     * background sound control
     */
    virtual status_t BGSoundOn();
    virtual status_t BGSoundConfig(uint8_t ul_gain, uint8_t dl_gain);
    virtual status_t BGSoundOff();


    /**
     * pcm 2 way
     */
    virtual status_t PCM2WayOn(const bool wideband_on);
    virtual status_t PCM2WayOff();


    /**
     * tty ctm control
     */
    virtual status_t TtyCtmOn();
    virtual status_t TtyCtmOff();
    virtual status_t TtyCtmDebugOn(bool tty_debug_flag);
    virtual int setTtyMode(const TtyModeType ttyMode);

    /**
     * rtt control
     */
    virtual int RttConfig(int rttMode);

    /**
     * acoustic loopback
     */
    virtual status_t SetAcousticLoopback(bool loopback_on);
    virtual status_t SetAcousticLoopbackBtCodec(bool enable_codec);

    virtual status_t SetAcousticLoopbackDelayFrames(int32_t delay_frames);
    virtual status_t setLpbkFlag(bool enableLpbk);

    /**
     * Modem Audio DVT and Debug
     */
    virtual status_t SetModemLoopbackPoint(uint16_t loopback_point);

    /**
     * encryption
     */
    virtual status_t SetEncryption(bool encryption_on);

    /**
     * volume control
     */
    virtual status_t SetDownlinkGain(int16_t gain);
    virtual status_t SetEnh1DownlinkGain(int16_t gain);
    virtual status_t SetUplinkGain(int16_t gain);
    virtual status_t SetDownlinkMute(bool mute_on);
    virtual status_t SetUplinkMute(bool mute_on);
    virtual status_t SetUplinkSourceMute(bool mute_on);
    virtual status_t SetSidetoneGain(int16_t gain);
    virtual status_t SetDSPSidetoneFilter(const bool dsp_stf_on);
    virtual status_t SetDownlinkMuteCodec(bool mute_on);


    /**
     * device related config
     */
    virtual status_t SetModemSideSamplingRate(uint16_t sample_rate);


    /**
     * speech enhancement control
     */
    virtual status_t SetSpeechEnhancement(bool enhance_on);
    virtual status_t SetSpeechEnhancementMask(const sph_enh_mask_struct_t &mask);

    virtual status_t SetBtHeadsetNrecOn(const bool bt_headset_nrec_on);


    /**
     * speech enhancement parameters setting
     */
    virtual status_t SetAllSpeechEnhancementInfoToModem(); // only available for LAD

    virtual status_t SetVariousKindsOfSpeechParameters(const void *param, const uint16_t data_length, const uint16_t ccci_message_id); // only available for LAD

    virtual status_t SetNBSpeechParameters(const void *pSphParamNB);
    virtual status_t SetDualMicSpeechParameters(const void *pSphParamDualMic);
    virtual status_t SetMagiConSpeechParameters(const void *pSphParamMagiCon);
    virtual status_t SetHACSpeechParameters(const void *pSphParamHAC);
    virtual status_t SetWBSpeechParameters(const void *pSphParamWB);

    virtual status_t GetVibSpkParam(void *eVibSpkParam);
    virtual status_t SetVibSpkParam(void *eVibSpkParam);

    virtual status_t GetSmartpaParam(void *eParamSmartpa);
    virtual status_t SetSmartpaParam(void *eParamSmartpa);

    virtual status_t SetDynamicSpeechParameters(const int type, const void *param_arg);


    /**
     * check whether modem is ready.
     */
    virtual bool     CheckModemIsReady();
    virtual int updateParam(int audioTypeUpdate);


protected:
    SpeechDriverLAD() {}
    SpeechDriverLAD(modem_index_t modem_index);


    /**
     * recover status (speech/record/bgs/vt/p2w/tty)
     */
    virtual void RecoverModemSideStatusToInitState();


    /**
     * CCCI Messenger
     */
    SpeechMessengerInterface *pCCCI;


    /**
     * Speech Mode
     */
    speech_mode_t mSpeechMode;

    /**
     * RTT Mode
     */
    int mRttMode;

    /**
     * lock
     */
    AudioLock mLock;

    /**
     * loopback
     */
    bool mIsLpbk;

private:
    /**
     * singleton pattern
     */
    static SpeechDriverLAD *mLad1;
    static SpeechDriverLAD *mLad2;
    static SpeechDriverLAD *mLad3;
};

} // end namespace android

#endif // end of ANDROID_SPEECH_DRIVER_LAD_H

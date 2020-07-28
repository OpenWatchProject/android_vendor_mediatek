#ifndef ANDROID_SPEECH_DRIVER_DUMMY_H
#define ANDROID_SPEECH_DRIVER_DUMMY_H

#include "SpeechDriverInterface.h"

namespace android {

class SpeechDriverDummy : public SpeechDriverInterface {
public:
    SpeechDriverDummy(modem_index_t modem_index);
    virtual ~SpeechDriverDummy();

    /**
     * speech control
     */
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


    /**
     * acoustic loopback
     */
    virtual status_t SetAcousticLoopback(bool loopback_on);
    virtual status_t SetAcousticLoopbackBtCodec(bool enable_codec);

    virtual status_t SetAcousticLoopbackDelayFrames(int32_t delay_frames);
    virtual status_t setLpbkFlag(bool enableLpbk __unused) { return -ENOSYS; }

    /**
     * Modem Audio DVT and Debug
     */
    virtual status_t SetModemLoopbackPoint(uint16_t loopback_point);

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
     * check whether modem is ready.
     */
    virtual bool     CheckModemIsReady();



protected:
    /**
     * recover status (speech/record/bgs/vt/p2w/tty)
     */
    virtual void RecoverModemSideStatusToInitState();



private:
    SpeechDriverDummy() {}
};

} // end namespace android

#endif // end of ANDROID_SPEECH_DRIVER_DUMMY_H

#ifndef ANDROID_AUDIO_ALSA_VOLUME_CONTROLLER_H
#define ANDROID_AUDIO_ALSA_VOLUME_CONTROLLER_H

#include "AudioVolumeInterface.h"
#include "AudioType.h"
#include <utils/Log.h>
#include <utils/String16.h>
#include <cutils/properties.h>
#include <utils/threads.h>

#include <tinyalsa/asoundlib.h>

#include "audio_custom_exp.h"
#include "AudioSpeechEnhanceInfo.h"

#include "AudioGainTableParamParser.h"

namespace android {
/****************************************************
* Define Volume Range of  sound & Voice.
*****************************************************/

#define VOICE_VOLUME_MAX       (160)    // outside
#define VOICE_ONEDB_STEP         (4)    // outside
#define UPLINK_GAIN_MAX            (252)    // param tuner
#define UPLINK_ONEDB_STEP         (4)   // param tuner

//#define AUDIO_SYSTEM_UL_GAIN_MAX            (45)
//#define MAX_PGA_GAIN_RANGE                          (30)    // gain controller
//#define AUDIO_UL_PGA_STEP                               (6) // gain controller
#define AMP_CONTROL_POINT   (5)                             // not used

typedef enum {
    EarPiece_SideTone_Gain = 0,
    Headset_SideTone_Gain,
    LoudSpk_SideTone_Gain,
    HAC_SideTone_gain,
    Num_Side_Tone_Gain
} SIDETOEN_GAIN_MODE;

// these is used by legacy AudioALSAParamTuner, should be removed.
typedef enum {
    Audio_Earpiece = 0,
    Audio_Headset,
    Audio_Headphone,
    Audio_Speaker,
    Audio_DualMode_Earpiece,
    Audio_DualMode_Headset,
    Audio_DualMode_Headphone,
    Audio_DualMode_speaker,
    Ringtone_Earpiece,
    Ringtone_Headset,
    Ringtone_Headphone,
    Ringtone_Speaker,
    Sipcall_Earpiece,
    Sipcall_Headset,
    Sipcall_Headphone,
    Sipcall_Speaker,
    Num_of_Audio_gain
} AUDIO_GAIN_MODE;

typedef enum {
    DRC_VERSION_1  = 0,
    DRC_VERSION_2 = 1,
} DRC_VERSION;

struct HWStreamInfo {
    int stream;
    int devices;
    int index;
    audio_mode_t mode;
};
struct HWvolume {
    int audioBuffer;
    int voiceBuffer;
    int speaker;
    int dLDegardeGain;
    int micGain;
    int swAgc;
    int sideTone;
};
struct HwCaptureInfo {
    audio_mode_t mode;
    audio_source_t source;
    audio_devices_t input_device;
    audio_devices_t output_devices;
};

//class AudioAMPControlInterface;
class AudioALSAStreamManager;
class AudioALSAHardwareResourceManager;

class AudioMTKGainController : public AudioVolumeInterface {
public:
    static AudioMTKGainController *getInstance();
    ~AudioMTKGainController();
    /**
     * check to see if the audio hardware interface has been initialized.
     */
    virtual status_t     setMasterVolume(float v, audio_mode_t mode, uint32_t devices);
    virtual float        getMasterVolume();
    virtual status_t     setVoiceVolume(float v, audio_mode_t mode, uint32_t devices);
    virtual float        getVoiceVolume(void);
    virtual status_t     setVoiceVolume(int MapVolume, uint32_t device);
    virtual void         setVoiceVolumeIndex(int volumeIndex);
    virtual status_t     ApplyVoiceGain(int degradeDb, audio_mode_t mode, uint32_t device);
    virtual status_t     ApplyMicGain(GAIN_MIC_MODE _micMode, GAIN_DEVICE _gainDevice, audio_mode_t _mode);

    // should depend on different usage , FM ,MATV and output device to setline in gain
    virtual status_t     ApplyMicGain(uint32_t MicType, int mode);
    virtual status_t     setFmVolume(const float fm_volume);
public:
    virtual status_t     setAnalogVolume(int stream, int devices, int index, audio_mode_t mode);
    virtual status_t     speechNetworkChange(unsigned int info);
    virtual bool         isNbSpeechBand(void);
    virtual status_t     setBtVolumeCapability(bool support);
public:
    virtual status_t     initCheck();
    virtual status_t     initVolumeController();
    // here only valid stream can be set .
    virtual status_t     setStreamVolume(int stream, float v);
    virtual status_t     setStreamMute(int stream, bool mute);
    virtual float        getStreamVolume(int stream);
    virtual status_t     SetLineInPlaybackGain(int type);
    virtual status_t     SetLineInRecordingGain(int type);
    virtual status_t     SetSideTone(uint32_t Mode, uint32_t devices);
    virtual uint32_t     GetSideToneGain(uint32_t device);
    virtual status_t     SetMicGain(uint32_t Mode, uint32_t devices);
    virtual status_t     SetULTotalGain(uint32_t Mode, unsigned char Volume);
    virtual uint8_t      GetULTotalGain() {return mULTotalGain;}
    virtual status_t     SetDigitalHwGain(uint32_t Mode, uint32_t Gain, uint32_t routes);
    virtual short        GetSWMICGain();
    virtual void         ApplyMdDlGain(int  Gain);
    virtual void         ApplyMdDlEhn1Gain(int32_t Gain);
    virtual void         ApplyMdUlGain(int  Gain);
    virtual uint16_t     MappingToDigitalGain(unsigned char Gain);
    virtual uint16_t     MappingToPGAGain(unsigned char Gain);
    virtual status_t     ApplySideTone(uint32_t Mode);
    // alsa driver set pga gain function
    virtual void         SetReceiverGain(int DrgradeDb);
    virtual void         SetHeadPhoneRGain(int DrgradeDb);
    virtual void         SetHeadPhoneLGain(int DrgradeDb);
    virtual void         SetSpeakerGain(int DegradedBGain);
    virtual void         SetLinoutLGain(int DegradedBGain);
    virtual void         SetLinoutRGain(int DegradedBGain);
    virtual void         SetAdcPga1(int DrgradeDb);
    virtual void         SetAdcPga2(int DrgradeDb);
    virtual int          ApplyAudioGainTuning(int Gain, uint32_t mode, uint32_t device);
    virtual status_t     SetMicGainTuning(uint32_t Mode, uint32_t gain);
    status_t             SetMicGainTuning(GAIN_MIC_MODE micMode, GAIN_DEVICE gainDevice, uint32_t gainDecimal);
    unsigned short       getMicGainDecimal(GAIN_MIC_MODE micMode, GAIN_DEVICE gainDevice);
    virtual status_t     SetCaptureGain(audio_mode_t mode, audio_source_t source, audio_devices_t input_device, audio_devices_t output_devices);
    virtual uint32_t     GetOffloadGain(float vol_f);
public:
    static int           logToLinear(float volume);
    static float         linearToLog(int volume);
    bool GetHeadPhoneImpedance(void);
    void setANCEnable(bool _enable);

    // return 0~255
    int GetDigitalLinearGain(int _volIdx, audio_devices_t _device, audio_stream_type_t _streamType);
    // return 0.0~1.0
    float GetDigitalLogGain(int _volIdx, audio_devices_t _device, audio_stream_type_t _streamType);

    void updateXmlParam(const char *_audioTypeName);
#ifdef MTK_AUDIO_SW_DRE
public:
    void registerPlaybackHandler(uint32_t _identity);
    void removePlaybackHandler(uint32_t _identity);
    void requestMute(uint32_t _identity, bool _mute);

    void setFmEnable(bool _enable);
    bool isOtherModuleWorking();

    void updateSWDREState(bool _numChanged, bool _muteChanged);
    void SWDRERampToMute();
    void SWDRERampToNormal();
private:
    AudioLock mSWDRELock;
    KeyedVector<uint32_t, bool> mMutedHandlerVector;

    bool mSWDREMute;
    bool mHasMuteHandler;
    size_t mNumHandler;
    bool mFmEnable;
#endif
private:
    static AudioMTKGainController *UniqueVolumeInstance;
    AudioMTKGainController();
    AudioMTKGainController(const AudioMTKGainController &);             // intentionally undefined
    AudioMTKGainController &operator=(const AudioMTKGainController &);  // intentionally undefined
    status_t             setVoiceVolume(int index, int devices, audio_mode_t mode);
    status_t             setNormalVolume(int stream, int index, int devices, audio_mode_t mode);
private:
    bool                 isInVoiceCall(audio_mode_t mode);
    bool                 isInVoipCall(audio_mode_t mode);
    bool                 isInCall(audio_mode_t mode);
    GAIN_DEVICE     getGainDevice(audio_devices_t devices);
    GAIN_DEVICE     getGainDeviceForTty(void);
    GAIN_MIC_MODE        getGainMicMode(audio_source_t _source, audio_mode_t _mode);
    uint32_t             getSideToneGainType(uint32_t devices);
    //        BUFFER_TYPE          getBufferType(int device,audio_mode_t mode);
    uint16_t             updateSidetone(int dlPGAGain, int  sidetone, uint8_t ulGain);
    status_t             setAnalogVolume_l(int stream, int devices, int index, audio_mode_t mode);
    // cal and set and set analog gainQuant
    void                 ApplyAudioGain(int Gain, audio_mode_t mode, GAIN_DEVICE gainDevice);
    void                 setAudioBufferGain(int gain);
    void                 setVoiceBufferGain(int gain);
    void                 setSpeakerGain(int gain);
    void                 setAMPGain(void *points, int num, int device);
    void                 ApplyMicGainByDevice(uint32_t device, audio_mode_t mode);
    void                 ApplyMicGainForTty(audio_mode_t mode);
    //bool                 Get_FMPower_info(void);

    int GetReceiverGain(void);
    int GetHeadphoneRGain(void);
    int GetHeadphoneLGain(void);
    int GetSPKGain(void);

    int tuneGainForMasterVolume(int gain, audio_mode_t mode, GAIN_DEVICE gainDevice);
    int tuneGainForHpImpedance(int gain, GAIN_DEVICE gainDevice);

    uint32_t            getHpImpedanceIdx(int32_t impedance);
    int                 getHpImpedanceCompesateValue(void);
    uint32_t            mHpImpedanceIdx;

    bool isValidStreamType(audio_stream_type_t _streamType);
    bool isValidVolIdx(int _idx, audio_mode_t _mode);

    bool isHeadsetCategory(enum GAIN_DEVICE _gainDevice);
    bool isEarpieceCategory(enum GAIN_DEVICE _gainDevice);
    bool isSpeakerCategory(enum GAIN_DEVICE _gainDevice);
    bool isDmicDevice(enum GAIN_DEVICE _gainDevice);

public:
    status_t getSceneGainTableParameter(GainTableForScene *_gainTableForScene);
    status_t getNonSceneGainTableParameter(GainTableForNonScene *_gainTableForNonScene);
    int getSceneCount();
    int getSceneIndex(const char *scene);
    int getCurrentSceneIndex();
    void setScene(const char *scene);

private:
    status_t allocateGainTable();
    status_t freeGainTable();

private:
    GainTableParam mGainTable;
    GainTableSpec *mSpec;
    //        AUDIO_BT_GAIN_STRUCT mCustomVolume_BT;
    AudioALSAHardwareResourceManager *mHardwareResourceManager;
    //        AudioAMPControlInterface * mAmpControl;
    float     mVoiceVolume;
    float     mMasterVolume;
    //int       mFmVolume;
    //int       mFmChipVolume;
    bool      mInitDone;
    GAIN_SPEECH_BAND mBand;
    GAIN_SPEECH_NETWORK mNetwork;
    HWvolume  mHwVolume;
    HWStreamInfo mHwStream;
    HwCaptureInfo mHwCaptureInfo;
    Mutex     mLock;
    bool      mSupportBtVol;

    struct mixer *mMixer;

    AudioSpeechEnhanceInfo *mAudioSpeechEnhanceInfoInstance;
    uint8_t mULTotalGain;

    bool mANCEnable;

    std::vector<std::string> mSceneList;
    int mSceneIndex;
};

}

#endif


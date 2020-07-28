#ifndef ANDROID_AUDIO_MTK_DEVICE_PARSER_H
#define ANDROID_AUDIO_MTK_DEVICE_PARSER_H

#include <stdint.h>
#include <sys/types.h>

#include <utils/Mutex.h>
#include <utils/String8.h>
#include <utils/KeyedVector.h>
#include <tinyalsa/asoundlib.h>
#include "AudioALSADeviceString.h"
#include "AudioType.h"

namespace android {

class AudioPcmDeviceparam {
public:
    AudioPcmDeviceparam() :
        mBufferBytes(0),
        mRateMax(0),
        mRateMin(0),
        mChannelMax(0),
        mChannelMin(0),
        mSampleBitMax(0),
        mSampleBitMin(0),
        mPreriodSizeMax(0),
        mPreriodSizeMin(0),
        mPreriodCountMax(0),
        mPreriodCountMin(0) {
    };
    unsigned int mBufferBytes;
    unsigned int mRateMax;
    unsigned int mRateMin;
    unsigned int mChannelMax;
    unsigned int mChannelMin;
    unsigned int mSampleBitMax;
    unsigned int mSampleBitMin;
    unsigned int mPreriodSizeMax;
    unsigned int mPreriodSizeMin;
    unsigned int mPreriodCountMax;
    unsigned int mPreriodCountMin;
};

class AudioDeviceDescriptor {
public:
    AudioDeviceDescriptor() :
        mCardindex(0),
        mPcmindex(0),
        mplayback(0),
        mRecord(0) {
    };
    String8 mStreamName;
    String8 mCodecName;
    unsigned int mCardindex;
    unsigned int mPcmindex;
    unsigned int mplayback;
    unsigned int mRecord;
    AudioPcmDeviceparam mPlayparam;
    AudioPcmDeviceparam mRecordparam;
};

class AudioALSADeviceParser {
public:
    unsigned int GetPcmIndexByString(String8 stringpair);
    unsigned int GetCardIndexByString(String8 stringpair);
    unsigned int GetPcmBufferSize(unsigned int  pcmindex, unsigned int direction);
    static AudioALSADeviceParser *getInstance();
    void dump();

    unsigned int GetCardIndex() {return mCardIndex;}
private:
    static AudioALSADeviceParser *UniqueAlsaDeviceInstance;
    AudioALSADeviceParser();
    void GetAllPcmAttribute(void);
    void GetAllCompressAttribute(void);
    void AddPcmString(char *InputBuffer);
    status_t QueryPcmDriverCapability();
    status_t GetPcmDriverparameters(AudioPcmDeviceparam *PcmDeviceparam, struct pcm_params *params);
    void SetPcmCapability(AudioDeviceDescriptor *Descriptor, char  *Buffer);
    void getCardName();
    void ParseCardIndex();
    /**
     * Audio Pcm vector
     */
    Vector <AudioDeviceDescriptor *> mAudioDeviceVector;
    Vector <AudioDeviceDescriptor *> mAudioComprDevVector;
    String8 mCardName;
    unsigned int mCardIndex;

    /*
     * flag of dynamic enable verbose/debug log
     */
    int mLogEnable;
};

}

#endif


#define LOG_TAG "AudioMTKVolumeCapability"

#define MTK_LOG_ENABLE 1
#include <log/log.h>
#include "AudioMTKVolumeCapability.h"
#include "AudioAMPControlInterface.h"
#include "audio_custom_exp.h"
namespace android {

#define HARDWARE_MAKE_VOLUME_VERSION(maj,min) \
    ((((maj) & 0xff) << 8) | ((min) & 0xff))

//hw spec db
static const int keyAudioBufferSize       =   8;
static const int KeyAudioBufferGain[]     =  { -5, -3, -1, 1, 3, 5, 7, 9};

static const int keyVoiceBufferSize       =   16;
static const int KeyVoiceBufferGain[]     =  { -21, -19, -17, -15, -13, -11, -9, -7, -5, -3, -1, 1, 3, 5, 7, 9};

static const int keyULPAGSize             =   6;
static const int KeyULPAGGain[]           =  { -6, 0, 6, 12, 18, 24};

static const int keySPKSize               =   16;
static const int KeySPKgain[]             =  { -60, 0, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17};

static const int keyDLDigitalRange        = 64;

static const int keyULSWAGCRange          = 32;

static const int keyMICGainRange          = 47;


static const int keySidetoneRange         = 47;

/*
  //for LGE
DeviceGainPath  keyDevicePath[] = {

{NORMAL_MODE,     CAPABILITY_DEVICE_RECEIVER,  CAPABILITY_BUFFER_AUDIO,  0x0},
{NORMAL_MODE,     CAPABILITY_DEVICE_HEADPHONE, CAPABILITY_BUFFER_AUDIO,  0xC0},
{NORMAL_MODE,     CAPABILITY_DEVICE_SPEAKER,   CAPABILITY_BUFFER_AUDIO,  0x60},
{PHONE_CALL_MODE, CAPABILITY_DEVICE_RECEIVER,  CAPABILITY_BUFFER_VOICE,  0x10},
{PHONE_CALL_MODE, CAPABILITY_DEVICE_HEADPHONE, CAPABILITY_BUFFER_VOICE,  0x90},
{PHONE_CALL_MODE, CAPABILITY_DEVICE_SPEAKER,   CAPABILITY_BUFFER_VOICE,  0x30},

};
*/
//IN1(CP0):00010000,Spk(CP1):00100000,IN2(CP3):01000000,HP(CP3):10000000
DeviceGainPath  keyDevicePath[] = {
    {NORMAL_MODE,     CAPABILITY_DEVICE_RECEIVER,  CAPABILITY_BUFFER_VOICE,  0x0},
    {NORMAL_MODE,     CAPABILITY_DEVICE_HEADPHONE, CAPABILITY_BUFFER_AUDIO,  0x90},
    {NORMAL_MODE,     CAPABILITY_DEVICE_SPEAKER,   CAPABILITY_BUFFER_VOICE,  0x60},
#ifdef USING_EXTAMP_ALL_VOICE_BUFFER
    {PHONE_CALL_MODE, CAPABILITY_DEVICE_RECEIVER,  CAPABILITY_BUFFER_VOICE,  0x0},
    {PHONE_CALL_MODE, CAPABILITY_DEVICE_HEADPHONE, CAPABILITY_BUFFER_VOICE,  0xC0},
    {PHONE_CALL_MODE, CAPABILITY_DEVICE_SPEAKER,   CAPABILITY_BUFFER_VOICE,  0x60},
#else
    {PHONE_CALL_MODE, CAPABILITY_DEVICE_RECEIVER,  CAPABILITY_BUFFER_VOICE,  0x0},
    {PHONE_CALL_MODE, CAPABILITY_DEVICE_HEADPHONE, CAPABILITY_BUFFER_AUDIO,  0x90},
    {PHONE_CALL_MODE, CAPABILITY_DEVICE_SPEAKER,   CAPABILITY_BUFFER_VOICE,  0x60},
#endif
    {NORMAL_MODE,     CAPABILITY_DEVICE_NONE,  CAPABILITY_BUFFER_NONE,  0x0},
};

#if 0
StreamLevel    keyStreamLevel[] = {
    {AUDIO_STREAM_VOICE_CALL, 6},
    {AUDIO_STREAM_SYSTEM, 7},
    {AUDIO_STREAM_RING, 7},
    {AUDIO_STREAM_MUSIC, 13},
    {AUDIO_STREAM_ALARM, 7},
    {AUDIO_STREAM_NOTIFICATION, 7},
    {AUDIO_STREAM_BLUETOOTH_SCO, 15},
    {AUDIO_STREAM_ENFORCED_AUDIBLE, 7},
    {AUDIO_STREAM_DTMF, 15},
    {AUDIO_STREAM_TTS, 15},
#ifndef MTK_BASIC_PACKAGE
    {AUDIO_STREAM_FM, 13},
    {AUDIO_STREAM_MATV, 13},
#endif
};
#else
StreamLevel    keyStreamLevel[] = {
    {AUDIO_STREAM_VOICE_CALL, 6},
    {AUDIO_STREAM_SYSTEM, 15},
    {AUDIO_STREAM_RING, 15},
    {AUDIO_STREAM_MUSIC, 15},
    {AUDIO_STREAM_ALARM, 15},
    {AUDIO_STREAM_NOTIFICATION, 15},
    {AUDIO_STREAM_BLUETOOTH_SCO, 15},
    {AUDIO_STREAM_ENFORCED_AUDIBLE, 15},
    {AUDIO_STREAM_DTMF, 15},
    {AUDIO_STREAM_TTS, 15},
#if 0
#ifndef MTK_BASIC_PACKAGE
    {AUDIO_STREAM_FM, 15},
    {AUDIO_STREAM_MATV, 15},
#endif
#endif
};
#endif

AudioHWVolumeCapability *AudioHWVolumeCapability::mInstance = NULL;

AudioHWVolumeCapability *AudioHWVolumeCapability::getInstance() {
    if (mInstance == NULL) {
        mInstance = new AudioHWVolumeCapability();
    }

    return mInstance;
}

AudioHWVolumeCapability::AudioHWVolumeCapability() {
    mAmpControl = AudioDeviceManger::createInstance();
}

uint32_t  AudioHWVolumeCapability::getVersion() {
    return HARDWARE_MAKE_VOLUME_VERSION(1, 0);
}

//audiobuffer
int AudioHWVolumeCapability::audioBufferGainSize() {
    return keyAudioBufferSize;
}

void AudioHWVolumeCapability::audioBufferGain(void *items) {
    memcpy(items, KeyAudioBufferGain, keyAudioBufferSize);
}

//audiobuffer
int AudioHWVolumeCapability::voiceBufferGainSize() {
    return keyVoiceBufferSize;
}

void AudioHWVolumeCapability::voiceBufferGain(void *items) {
    memcpy(items, KeyVoiceBufferGain, keyVoiceBufferSize);
}

// mic buffer
int  AudioHWVolumeCapability::micGainRange() {
    return keyMICGainRange;
}

int AudioHWVolumeCapability::micPGAGainSize() {
    return keyULPAGSize;
}

void AudioHWVolumeCapability::micPGAGain(void *items) {
    memcpy(items, KeyULPAGGain, keyULPAGSize);
}


// model DL  gain
int AudioHWVolumeCapability::dlDigitalGainRange() {
    return keyDLDigitalRange;
}


// model DL  gain
int      AudioHWVolumeCapability::ulSwAgcRange() {
    return keyULSWAGCRange;
}


//sidetone
int AudioHWVolumeCapability::sidetoneGainRange() {
    return keySidetoneRange;
}

//speaker
int AudioHWVolumeCapability::ampControlPointSize() {

#ifdef USING_EXTAMP_TC1
    if (mAmpControl) {
        return  mAmpControl->getParameters(AUD_AMP_GET_CTRP_NUM, 0, 0);
    }
    return 0;
#else
    return 1;

#endif
}

int AudioHWVolumeCapability::ampGainSizeByPoint(int index) {
#ifdef USING_EXTAMP_TC1

    if (mAmpControl) {
        return  mAmpControl->getParameters(AUD_AMP_GET_CTRP_BITS, index, 0);
    }
    return 0;
#else
    return keySPKSize;

#endif
}

void AudioHWVolumeCapability::ampPointGain(int index, void *items) {
#ifdef USING_EXTAMP_TC1
    if (mAmpControl) {
        mAmpControl->getParameters(AUD_AMP_GET_CTRP_TABLE, index, items);
    }
#else
    memcpy(items, KeySPKgain, keySPKSize);
#endif
}


int AudioHWVolumeCapability::getDevicePathSize() {
    return sizeof(keyDevicePath) / sizeof(DeviceGainPath);
}

/*
int AudioHWVolumeCapability::getDevicePath(DeviceGainPath * path)
{
     memcpy(path,keyDevicePath,sizeof(keyDevicePath));
     return 0;
}
*/
short int AudioHWVolumeCapability::getDevicePath(int mode, int device, bool isSpeaker) {
    short int ret = 0;
    char buffer = 0;
    int i = 0;
    char pointValue = 0;

    for (i = 0; i < getDevicePathSize(); i++) {
        if (keyDevicePath[i].mode == mode && keyDevicePath[i].device == device) {
            break;
        }
    }
    if (i < getDevicePathSize()) {
        if (keyDevicePath[i].buffer == CAPABILITY_BUFFER_AUDIO) {
            buffer = 1;
        } else if (keyDevicePath[i].buffer == CAPABILITY_BUFFER_VOICE) {
            buffer = 2;
        } else if (keyDevicePath[i].buffer == CAPABILITY_BUFFER_NONE) {
            buffer = 0;
        }

        pointValue = (char)keyDevicePath[i].point;
    } else {
        SLOGD("i is %d", i);
        pointValue = 0;
    }

#ifdef MTK_AUDIO_GAIN_TABLE
#ifdef USING_EXTAMP_TC1
    ret = (buffer << 8 | pointValue);
#else
    ret = (buffer << 8 | 0x00);
    if (isSpeaker) {
        ret = (0x0 << 8 | 0x00); // disable PGA gain display (high byte, 0: no PGA gain display, 1:audio buf, 2: voice buf )
        ret = ret | 0x08;
    }
#endif
#endif

    SLOGD("getDevicePath, ret:0x%x", ret);
    return  ret;
}

//get  stream level
int AudioHWVolumeCapability::getStreamSize() {
    return  sizeof(keyStreamLevel) / sizeof(StreamLevel);
}

/*
int AudioHWVolumeCapability::getStreamLevel(StreamLevel * level)
{
    memcpy(level,keyStreamLevel,sizeof(keyStreamLevel));
    return 0;
}
*/
int AudioHWVolumeCapability::getStreamLevel(int stream) {
    int ret = 0;
    for (int i = 0; i < getStreamSize(); i++) {
        if (stream == keyStreamLevel[i].stream) {
            SLOGD("keyStreamLevel[%d].level is %d", i, keyStreamLevel[i].level);
            ret =  keyStreamLevel[i].level;

            ret += 1;   //for digtal  level + 1
            ret = ret << 8 | 0x1;

            break;
        }
    }
    return ret;
}


}

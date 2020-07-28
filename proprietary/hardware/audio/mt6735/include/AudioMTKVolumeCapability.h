#ifndef ANDROID_AUDIO_MTK_VOLUME_CAPALITY_H
#define ANDROID_AUDIO_MTK_VOLUME_CAPALITY_H

namespace android {

enum  capabilityDevice {
    CAPABILITY_DEVICE_INVALID   = -1,
    CAPABILITY_DEVICE_NONE  = 0,
    CAPABILITY_DEVICE_RECEIVER  = 1,
    CAPABILITY_DEVICE_SPEAKER   = 2,
    CAPABILITY_DEVICE_HEADPHONE = 3,
    CAPABILITY_DEVICE_DOC       = 4,
    CAPABILITY_DEVICE_CNT
};
enum  capabilityBuffer {
    CAPABILITY_BUFFER_INVALID  = -1,
    CAPABILITY_BUFFER_NONE  = 0,
    CAPABILITY_BUFFER_AUDIO  = 1,
    CAPABILITY_BUFFER_VOICE  = 2,
    CAPABILITY_BUFFER_CNT
};

#define NORMAL_MODE 0
#define PHONE_CALL_MODE 1
struct DeviceGainPath {
    int mode;
    int device;
    int buffer;
    int point;
};
struct StreamLevel {
    int stream;
    int level;
};
class AudioAMPControlInterface;
class AudioHWVolumeCapability {
public:
    static   AudioHWVolumeCapability *getInstance();
    uint32_t getVersion();
    //audio buffer
    int      audioBufferGainSize();
    void     audioBufferGain(void *items);

    //voice buffer
    int      voiceBufferGainSize();
    void     voiceBufferGain(void *items);

    // mic buffer
    int      micGainRange();
    int      micPGAGainSize();
    void     micPGAGain(void *items);

    // model DL  gain
    int      dlDigitalGainRange();

    // model DL  gain
    int      ulSwAgcRange();

    //sidetone
    int     sidetoneGainRange();

    //external amp
    int     ampControlPointSize();
    int     ampGainSizeByPoint(int index);
    void    ampPointGain(int index, void *items);

    //get speech device path
    int     getDevicePathSize();
    short int     getDevicePath(int mode, int device, bool isSpeaker = false);

    //get  stream level
    int     getStreamSize();
    int     getStreamLevel(int stream);

private:
    static   AudioHWVolumeCapability *mInstance;

private:
    AudioHWVolumeCapability();
    AudioHWVolumeCapability(const AudioHWVolumeCapability &);             // intentionally undefined
    AudioHWVolumeCapability &operator=(const AudioHWVolumeCapability &);
private:
    AudioAMPControlInterface *mAmpControl;
};

}

#endif


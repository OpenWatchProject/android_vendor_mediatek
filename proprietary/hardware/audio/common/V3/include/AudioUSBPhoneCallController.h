#ifndef ANDROID_AUDIO_USB_PHONE_CALL_CONTROLLER_H
#define ANDROID_AUDIO_USB_PHONE_CALL_CONTROLLER_H

#include <system/audio.h>
#include <pthread.h>
extern "C" {
#include <alsa_device_profile.h>
#include <alsa_device_proxy.h>
}

#include <vector>
#include <string>

#include <AudioLock.h>
#include "AudioType.h"

struct mixer;
struct pcm_config;

namespace android {
class MtkAudioSrcBase;

struct USBStream {
    int direction;
    unsigned int speechRate;
    unsigned int latency;

    // usb control
    std::string deviceId;
    size_t deviceParamIdx;
    alsa_device_profile profile;
    alsa_device_proxy proxy;
    float usbLatency;

    // hal 2 afe control
    struct pcm_config pcmConfigHAL2AFE;
    struct pcm *pcmHAL2AFE;
    std::string apTurnOnSequence;

    // blisrc
    MtkAudioSrcBase *blisrc;
    char *blisrcOutBuffer;

    // bit convert
    audio_format_t dstFmt;
    audio_format_t srcFmt;
    char *bitConvertBuffer;

    // data pending
    unsigned int dataPendingOutBufSize;
    char *dataPendingOutBuffer;
    char *dataPendingTempBuffer;
    unsigned int dataPendingRemainBufSize;
    /*
    unsigned int dataPendingDesireOutBufSize;
    unsigned int dataPendingOutBufSize;
    unsigned int dataPendingTempBufSize;
    unsigned int dataPendingOutRemainBufSize;
    char *dataPendingOutBuffer;
    char *dataPendingTempBuffer;
    unsigned int dataPendingRemainBufSize;
    */

    // throttle control
    unsigned int throttleTargetAvail;
    unsigned int throttleKickInAvailDiff;
    unsigned int throttleSteadyAvailDiff;
    unsigned int throttleOrigAvailDiff;
    unsigned int throttleSpeedUpFactor;
    unsigned int throttleSpeedUpCount;
    unsigned int throttleKickInCount;
    unsigned int throttleCurrentInRate;
    int throttleState;

    char *awssrcOutBuffer;
    bool mIsSrcOut_2ch16bit;
    unsigned int throttleCurrentOutRate;
    unsigned int mSrcOutRate;

    uint32_t mReadDumpByteAddr = 0;
    uint32_t mWriteDumpByteAddr = 0;
    uint32_t mAWSoutDumpByteAddr = 0;
};

struct USBDeviceParam {
    std::string id;
    int playbackLatencyUs;
    int captureLatencyUs;
};

struct USBCallParam {
    int speechDlUlLatencyUs; // latency of usb dl + usb ul path
    int speechDlLatencyUs;   // latency of usb dl + audio analog mic path
    int speechUlLatencyUs;   // latency of usb ul + audio analog playback path
    int echoSettlingTimeMs;
    int echoAheadMicDataUs;

    std::vector<struct USBDeviceParam> deviceParam;
    int maxCaptureLatencyUs;
};

class AudioUSBPhoneCallController {
public:
    static AudioUSBPhoneCallController *getInstance();
    ~AudioUSBPhoneCallController();

    int enable(unsigned int speechRate);
    int disable();
    bool isEnable();
    bool isForceUSBCall();
    bool isUsingUSBIn();

    int setUSBOutConnectionState(audio_devices_t devices, bool connect, int card, int device);
    int setUSBInConnectionState(audio_devices_t devices, bool connect, int card, int device);

    unsigned int getSpeechRate();
    audio_devices_t getUSBCallInDevice();

    std::string getUSBIdSpeechDL();
    std::string getUSBIdSpeechUL();

    // param
    void updateXmlParam(const char *_audioTypeName);
private:
    static AudioUSBPhoneCallController *mUSBPhoneCallController;
    AudioUSBPhoneCallController();

    // speech downlink related
    static void *speechDLThread(void *arg);

    // utility function
    static int prepareUSBStream(struct USBStream *stream);
    static int getSpeech2HALPcmConfig(struct USBStream *stream);
    static unsigned int getPeriodByte(const struct pcm_config *config);

    // speech uplink related
    static void *speechULThread(void *arg);
    int speechULPhoneMicPath(bool enable);

    // echo ref
    int setEchoRefPath(bool enable, int stage);
    int setEchoRefDebugPath(bool enable);

    // dump
    static FILE *pcmDumpOpen(const char *name, const char *property, unsigned int rate, audio_format_t fmt, unsigned int chnum);
    static void pcmDumpClose(FILE *file);
    static void pcmDumpWriteData(FILE *file, const void *buffer, size_t bytes);

    // blisrc
    static status_t initBliSrc(struct USBStream *stream);
    static status_t deinitBliSrc(struct USBStream *stream);
    static status_t doBliSrc(struct USBStream *stream, void *pInBuffer, uint32_t inBytes,
                             void **ppOutBuffer, uint32_t *pOutBytes);
    static status_t resetBliSrcBuffer(struct USBStream *stream);

    // awssrc
    static status_t initAwsSrc(struct USBStream *stream);
    static status_t deinitAwsSrc(struct USBStream *stream);
    static status_t doAwsSrc(struct USBStream *stream, void *pInBuffer, uint32_t inBytes,
                         void **ppOutBuffer, uint32_t *pOutBytes);
    // bit convert
    static unsigned int getBitConvertDstBufferSize(audio_format_t dstFmt,
                                                   audio_format_t srcFmt,
                                                   unsigned int srcBufSizeByte);
    static status_t initBitConverter(struct USBStream *stream);
    static status_t deinitBitConverter(struct USBStream *stream);
    static status_t doBitConversion(struct USBStream *stream,
                                    void *pInBuffer, uint32_t inBytes,
                                    void **ppOutBuffer, uint32_t *pOutBytes);

    // data pending
    static status_t initDataPending(struct USBStream *stream);
    static status_t deinitDataPending(struct USBStream *stream);
    static status_t doDataPending(struct USBStream *stream,
                                  void *pInBuffer, uint32_t inBytes,
                                  void **ppOutBuffer, uint32_t *pOutBytes);

    // throttle
    static unsigned int getStreamWriteRate(struct USBStream *stream);
    static unsigned int getStreamReadRate(struct USBStream *stream);

    static status_t throttleInit(struct USBStream *stream);
    static status_t throttleReset(struct USBStream *stream);
    static status_t throttleSetSrcInRate(struct USBStream *stream, unsigned int rate);
    static status_t throttleResetCurrentRate(struct USBStream *stream);
    static status_t throttleSetCompensateInRate(struct USBStream *stream,
                                                unsigned int changeMsPerSecond,
                                                int state);
    static status_t throttleControl(struct USBStream *stream);
    static status_t throttleSetSrcOutRate(struct USBStream *stream, unsigned int rate);
    // lpbk test
    bool getLpbkTime(unsigned int idx, void *buffer, unsigned int bufferSize,
                     unsigned int channel, unsigned int rate, size_t format_size);

    static unsigned int getPcmAvail(struct pcm *pcm);

    // perf service function
    int initPerfService();
    void deinitPerfService();
    void enablePerfCpuScn();
    void disablePerfCpuScn();

    // debug
    static void setDebugInfo(bool enable, int dbgType);

    // param
    status_t loadUSBCallParam();
    status_t loadUSBDeviceParam();

    status_t getDeviceId(struct USBStream *stream);
    status_t getDeviceParam(struct USBStream *stream);
    unsigned int getEchoMaxDelayUs();
    unsigned int getEchoCurrentDelayUs();

private:
    AudioLock mLock;

    bool mEnable;
    bool mAudioHWReady;
    unsigned int mSpeechRate;
    int mModemIndex;

    bool mUSBOutConnected;
    struct USBStream mUSBOutStream;

    bool mUSBInConnected;
    bool mEnableWithUSBInConnected;
    struct USBStream mUSBInStream;

    struct pcm *mPcmMicOut;
    struct pcm *mPcmMicIn;

    struct pcm *mPcmEchoRefOut;
    struct pcm *mPcmEchoRefIn;

    struct pcm *mPcmEchoRefDebugOut;
    struct pcm *mPcmEchoRefDebugOut2;

    static struct mixer *mMixer;

    pthread_t mSphDLThread;
    pthread_t mSphULThread;

    int mLpbkType;
    bool mLpbkStart;
    float mLpbkTime, mLpbkTimePart[4];
    struct timespec mLpbkNewTime, mLpbkOldTime, mLpbkStartTime;
    int mLpbkPulseThres;

    int mDebugType;

    int mEchoRefState;
    AudioLock mEchoRefStateLock;

    struct USBCallParam mParam;

    int32_t mPowerHalHandle;
};

}
#endif


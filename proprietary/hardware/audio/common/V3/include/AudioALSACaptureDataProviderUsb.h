#ifndef ANDROID_AUDIO_ALSA_CAPTURE_DATA_PROVIDER_USB_H
#define ANDROID_AUDIO_ALSA_CAPTURE_DATA_PROVIDER_USB_H

#include "AudioALSACaptureDataProviderBase.h"

extern "C" {
    //#include <tinyalsa/asoundlib.h>
#include "alsa_device_profile.h"
#include "alsa_device_proxy.h"
#include "alsa_logging.h"
#include <audio_utils/channels.h>

void fmt_conv_file_init_ul(void);
void fmt_conv_file_deinit_ul(void);
}

typedef struct {
    unsigned int gPolicyRate;
    audio_format_t gPolicyFormat;
    unsigned int gPolicyChannel;
    audio_channel_mask_t gPolicyChannelmask;
} policy_support_format;

namespace android {
class IAudioALSACaptureDataClient;

class AudioALSACaptureDataProviderUsb : public AudioALSACaptureDataProviderBase {
public:
    virtual ~AudioALSACaptureDataProviderUsb();

    static AudioALSACaptureDataProviderUsb *getInstance();

    /**
     * open/close pcm interface when 1st attach & the last detach
     */

    virtual status_t open();
    virtual status_t close();
    virtual void initUsbInfo(stream_attribute_t stream_attribute_source_usb, alsa_device_proxy *proxy, size_t buffer_size, bool enable, policy_support_format UL_Format);
    virtual bool isNeedEchoRefData();
    virtual ssize_t doBcvProcess(void *buffer, ssize_t bytes);

protected:
    AudioALSACaptureDataProviderUsb();

    status_t updateStartTimeStamp(struct timespec timeStamp);

    status_t updateCaptureTimeStampByStartTime(uint32_t bufferSize);

    struct timespec  mCaptureStartTime;

    struct timespec mEstimatedBufferTimeStamp;



private:
    /**
     * singleton pattern
     */
    static AudioALSACaptureDataProviderUsb *mAudioALSACaptureDataProviderUsb;


    /**
     * pcm read thread
     */
    static void *readThread(void *arg);
    pthread_t hReadThread;

    struct timespec mNewtime, mOldtime; //for calculate latency
    double timerec[3]; //0=>threadloop, 1=>kernel delay, 2=>process delay
    void     *aud_fmt_conv_hdl_in;
    void     *aud_fmt_conv_hdl_out;

};

}

#endif // end of ANDROID_AUDIO_ALSA_CAPTURE_DATA_PROVIDER_NORMAL_H

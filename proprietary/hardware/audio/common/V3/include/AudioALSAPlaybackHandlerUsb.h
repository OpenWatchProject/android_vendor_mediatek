#ifndef ANDROID_AUDIO_ALSA_PLAYBACK_HANDLER_USB_H
#define ANDROID_AUDIO_ALSA_PLAYBACK_HANDLER_USB_H

#include "AudioALSAPlaybackHandlerBase.h"
#include "AudioUSBPhoneCallController.h"

#ifdef MTK_AURISYS_FRAMEWORK_SUPPORT
struct aurisys_lib_manager_t;
struct arsi_task_config_t;
struct audio_pool_buf_t;
#endif
extern "C" {
    //#include <tinyalsa/asoundlib.h>
#include "alsa_device_profile.h"
#include "alsa_device_proxy.h"
#include "alsa_logging.h"
#include <audio_utils/channels.h>
void fmt_conv_file_init(void);
void fmt_conv_file_deinit(void);

}

namespace android {
class AudioALSACaptureDataProviderEchoRefUsb;

class AudioALSAPlaybackHandlerUsb : public AudioALSAPlaybackHandlerBase {
public:
    AudioALSAPlaybackHandlerUsb(const stream_attribute_t *stream_attribute_source);
    virtual ~AudioALSAPlaybackHandlerUsb();


    /**
     * open/close audio hardware
     */
    virtual status_t open();
    virtual status_t close();
    virtual status_t routing(const audio_devices_t output_devices);
    virtual void initUsbInfo(stream_attribute_t mStreamAttributeTargetUSB, alsa_device_proxy *proxy, size_t buffer_size);


    /**
     * write data to audio hardware
     */
    virtual ssize_t  write(const void *buffer, size_t bytes);

    virtual status_t setFilterMng(AudioMTKFilterManager *pFilterMng);


    /**
     * low latency
     */
    virtual status_t setScreenState(bool mode, size_t buffer_size, size_t reduceInterruptSize, bool bforce = false);

    const stream_attribute_t getStreamAttributeTargetEchoRef() { return mStreamAttributeTargetEchoRef; }
private:
    /**
     * For echo ref SW implementation
     */
    bool writeEchoRefDataToDataProvider(AudioALSACaptureDataProviderEchoRefUsb *dataProvider, const char *echoRefData, uint32_t dataSize);
    status_t updateStartTimeStamp();
    status_t loadUSBDeviceParam();
    status_t getDeviceId(struct USBStream *stream);
    uint32_t getUSBDeviceLatency(size_t deviceParamIdx);
    status_t getDeviceParam(struct USBStream *stream);

    stream_attribute_t mStreamAttributeTargetEchoRef; // to echoref
    struct timespec mEchoRefStartTime;
    uint32_t mTotalEchoRefBufSize;

    AudioALSACaptureDataProviderEchoRefUsb *mDataProviderEchoRefUsb;
    struct timespec mNewtime, mOldtime;
    double latencyTime[3];

    struct USBStream mUSBOutStream;
    struct USBCallParam mParam;
    void     *aud_fmt_conv_hdl_out1;
    void     *aud_fmt_conv_hdl_out2;

};

} // end namespace android

#endif // end of ANDROID_AUDIO_ALSA_PLAYBACK_HANDLER_USB_H

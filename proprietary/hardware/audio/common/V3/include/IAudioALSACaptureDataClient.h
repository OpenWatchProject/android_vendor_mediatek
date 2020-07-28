#ifndef ANDROID_I_AUDIO_ALSA_CAPTURE_DATA_CLIENT_H
#define ANDROID_I_AUDIO_ALSA_CAPTURE_DATA_CLIENT_H

#include <AudioType.h>
#include <AudioUtility.h>


namespace android {

class AudioALSACaptureDataProviderBase;

/// Observer pattern: Observer
class IAudioALSACaptureDataClient {
public:
    IAudioALSACaptureDataClient(AudioALSACaptureDataProviderBase *pCaptureDataProvider, stream_attribute_t *stream_attribute_target);
    virtual ~IAudioALSACaptureDataClient() {};


    /**
     * set client index
     */
    virtual void       *getIdentity() const = 0;

    /**
     * set/get raw frame count from hardware
     */
    virtual void    setRawStartFrameCount(int64_t frameCount) = 0;
    virtual int64_t getRawStartFrameCount() = 0;

    /**
     * get / process / offer data
     */
    virtual uint32_t    copyCaptureDataToClient(RingBuf pcm_read_buf) = 0; // called by capture data provider


    /**
     * read data from audio hardware
     */
    virtual ssize_t     read(void *buffer, ssize_t bytes) = 0; // called by capture handler

    //EchoRef+++
    /**
     * get / process / offer data
     */
    virtual uint32_t    copyEchoRefCaptureDataToClient(RingBuf pcm_read_buf) = 0;// called by capture data provider

    virtual void AddEchoRefDataProvider(AudioALSACaptureDataProviderBase *pCaptureDataProvider, stream_attribute_t *stream_attribute_target);
    //EchoRef---

    /**
     * Update BesRecord Parameters
     */
    virtual status_t UpdateBesRecParam() = 0;

    /**
     * check if the attached client has low latency requirement
     */
    virtual bool    IsLowLatencyCapture(void) = 0;

    /**
     * Query captured frames & time stamp
     */
    virtual int getCapturePosition(int64_t *frames, int64_t *time) = 0;

    virtual const stream_attribute_t *getStreamAttributeSource() = 0;

    /**
     * sync pcm start
     */
    virtual bool isNeedSyncPcmStart() { return false; }

    /**
     * StreamIn reopen
     */
    virtual bool getStreamInReopen() { return false; }
    virtual void setStreamInReopen(bool state) { (void)state; }


protected:
    IAudioALSACaptureDataClient() {}



};

} // end namespace android

#endif // end of ANDROID_I_AUDIO_ALSA_CAPTURE_DATA_CLIENT_H

#ifndef ANDROID_AUDIO_ALSA_CAPTURE_HANDLER_BASE_H
#define ANDROID_AUDIO_ALSA_CAPTURE_HANDLER_BASE_H

#include "AudioType.h"

namespace android {

class AudioALSADataProcessor;

class AudioALSAHardwareResourceManager;
class IAudioALSACaptureDataClient;

class AudioALSACaptureHandlerBase {
public:
    AudioALSACaptureHandlerBase(stream_attribute_t *stream_attribute_target);
    virtual ~AudioALSACaptureHandlerBase();


    /**
     * set handler index
     */
    inline void         setIdentity(const uint32_t identity) { mIdentity = identity; }
    inline uint32_t     getIdentity() const { return mIdentity; }

    /**
     * get raw frame count from hardware
     */
    int64_t getRawStartFrameCount();

    /**
     * open/close audio hardware
     */
    virtual status_t open() = 0;
    virtual status_t close() = 0;
    virtual status_t routing(const audio_devices_t input_device) = 0;


    /**
     * read data from audio hardware
     */
    virtual ssize_t  read(void *buffer, ssize_t bytes) = 0;

    /**
     * Update BesRecord Parameters
     */
    virtual status_t UpdateBesRecParam();

    /**
     * Query if the capture handler can run in Call Mode
     */
    virtual bool isSupportConcurrencyInCall();

    capture_handler_t getCaptureHandlerType();

    virtual status_t setLowLatencyMode(bool mode, size_t kernel_buffer_size, size_t reduce_size, bool bforce = false);

    int getCapturePosition(int64_t *frames, int64_t *time);

    bool getStreamInReopen();
    void setStreamInReopen(bool state);


    /**
     * AAudio MMAP
     */
    virtual status_t    start();
    virtual status_t    stop();
    virtual status_t    createMmapBuffer(int32_t min_size_frames,
                                  struct audio_mmap_buffer_info *info);
    virtual status_t    getMmapPosition(struct audio_mmap_position *position);


protected:
    /**
     * init audio hardware
     */
    virtual status_t init();

    AudioALSAHardwareResourceManager *mHardwareResourceManager;
    IAudioALSACaptureDataClient      *mCaptureDataClient;

    stream_attribute_t *mStreamAttributeTarget; // to stream in
    bool mSupportConcurrencyInCall;

    capture_handler_t mCaptureHandlerType;

private:
    AudioALSADataProcessor *mDataProcessor;

    uint32_t mIdentity; // key for mCaptureHandlerVector
};

} // end namespace android

#endif // end of ANDROID_AUDIO_ALSA_CAPTURE_HANDLER_BASE_H

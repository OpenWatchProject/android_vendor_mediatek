#ifndef ANDROID_AUDIO_PCM_DATA_CAPTURE_IN_H
#define ANDROID_AUDIO_PCM_DATA_CAPTURE_IN_H

#include "AudioType.h"
#include <AudioLock.h>
#include "AudioUtility.h"
#include <tinyalsa/asoundlib.h>

namespace android {

class AudioALSAPcmDataCaptureIn {
public:
    virtual ~AudioALSAPcmDataCaptureIn();
    static AudioALSAPcmDataCaptureIn *getInstance();

    /**
     * read data from audio hardware
     */
    virtual ssize_t read(void *buffer, ssize_t bytes); // called by capture handler
    virtual status_t Standby();
    virtual status_t SetPcmConfig(pcm_config mPcmInputConfig);
    virtual status_t GetPcmConfig(pcm_config *mPcmInputConfig);
    virtual status_t SetThreadEnable(void);
    virtual status_t SetThreadDisable(void);

    class AudioPCMInputThread : public Thread {
    public:
        AudioPCMInputThread(AudioALSAPcmDataCaptureIn *mAudioALSAPcmDataCaptureIn);
        virtual ~AudioPCMInputThread();

        // Good place to do one-time initializations
        virtual status_t    readyToRun();
        virtual void        onFirstRef();

        status_t SetPcmConfig(pcm_config mPcmInputConfig);
        status_t GetPcmConfig(pcm_config *mPcmInputConfig);

        status_t SetThreadEnable();
        status_t SetThreadDisable();

        void OpenPCMDump(const char *class_name);
        void ClosePCMDump();

        int readIVData(void *buffer, ssize_t bytes);
        unsigned int FormatTransfer(int SourceFormat, int TargetFormat, void *Buffer, unsigned int mReadBufferSize);

    protected:
        status_t OpenPcm();
        status_t ClosePcm();
        status_t StartPcm();
        status_t ProcessStateChange();
        int GetPcmData();
        status_t AllocateResource(); // allocate buffer
        status_t ReleaseResource(); // release buffer
        status_t ResetResource();    // reset read buffer
        status_t AssignReadSize();  // assign read pcm buffer size

    private:
        virtual bool threadLoop();
        AudioALSAPcmDataCaptureIn *mAudioALSAPcmDataCaptureIn;
        char *mReadBuffer;
        unsigned int mReadBufferSize;
        const unsigned int mReadBufferAllocateSize = 64 * 1024;
        pcm_config mPcmInputConfig;
        struct pcm *mInputPcm;
        bool mbRunning;
        bool mbStatusChange;

        Mutex mLock;     // mutex associated with mCond
        Condition mWaitWorkCV; // condition for status return

        Mutex mRingLock;
        Condition mRingWaitWorkCV;

        Mutex mPcmLock;
        Condition mPcmWaitWorkCV;

        /**
        * local ring buffer
        */
        RingBuf  mRawDataBuf;

        FILE *mPCMIVDumpFile;
        int DumpIVFileNum;

    };

private:
    /**
     * singleton pattern
     */
    static AudioALSAPcmDataCaptureIn *mAudioALSAPcmDataCaptureIn;
    AudioALSAPcmDataCaptureIn();

    sp<AudioPCMInputThread>   mAudioPcmInputThread;
    /**
     * attribute
     */
    stream_attribute_t mStreamAttributeSource; // from audio hw , can set by user

};

} // end namespace android

#endif // end of ANDROID_AUDIO_PCM_DATA_CAPTURE_IN_H

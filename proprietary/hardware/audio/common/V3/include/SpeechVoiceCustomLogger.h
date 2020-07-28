#ifndef ANDROID_SPEECH_VOICE_CUSTOM_LOGGER_H
#define ANDROID_SPEECH_VOICE_CUSTOM_LOGGER_H
#include "AudioUtility.h"
#include <AudioLock.h>

namespace android {

class SpeechVoiceCustomLogger {
public:
    virtual ~SpeechVoiceCustomLogger();

    static SpeechVoiceCustomLogger *GetInstance();

    status_t Open();
    status_t Close();

    uint16_t CopyBufferToVCL(RingBuf ul_ring_buf);
    bool UpdateVCLSwitch() ;

    bool GetVCLRecordStatus() const { return mEnable; } // true for open, false for close


protected:
    SpeechVoiceCustomLogger();

    status_t OpenFile();
    static void *DumpVCLRecordDataThread(void *arg);

    bool mStarting;
    bool mEnable;
    bool mVCLEnable;

    RingBuf mRingBuf; // Internal Input Buffer for Get VM data
    FILE *mDumpFile;

    pthread_t mRecordThread;
    AudioLock mMutex;

private:
    static SpeechVoiceCustomLogger *mSpeechVoiceCustomLogger; // singleton
}; // end of class SpeechVMRecorder

}; // end of namespace android

#endif // end of ANDROID_SPEECH_VM_RECORDER_H

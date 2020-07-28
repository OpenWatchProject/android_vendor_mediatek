#ifndef ANDROID_SPEECH_VM_RECORDER_H
#define ANDROID_SPEECH_VM_RECORDER_H

#include <hardware_legacy/AudioMTKHardwareInterface.h>

#include "AudioType.h"
#include "AudioUtility.h"

#include <AudioLock.h>

namespace android {

/*
 * =============================================================================
 *                     typedef
 * =============================================================================
 */
enum SpeechVmType {
    SPEECH_VM_DISABLE     = 0,
    SPEECH_VM_SPEECH      = 1,
    SPEECH_VM_CTM4WAY     = 2,
    NUM_SPEECH_VM_TYPE
};

/*
 * =============================================================================
 *                     ref struct
 * =============================================================================
 */
class AudioCustParamClient;

class SpeechVMRecorder {
public:
    virtual ~SpeechVMRecorder();
    static SpeechVMRecorder *getInstance();

    /**
     * VM Config
     */
    int configVm(const AUDIO_CUSTOM_PARAM_STRUCT *sphParamNB);
    int configVm(const uint8_t vmConfig);
    int configVmEpl(const bool isEpl);
    SpeechVmType getVmConfig();

    /**
     * VM for vocie call
     */
    status_t open();
    status_t close();
    bool getVMRecordStatus() const { return mIsVmEnable; }
    uint16_t getVmDataFromModem(RingBuf ul_ring_buf);

    /**
     * CTM Debug for TTY
     */
    int  startCtmDebug();
    int  stopCtmDebug();
    void getCtmDebugDataFromModem(RingBuf ul_ring_buf, FILE *pFile);
    FILE *pCtmDumpFileUlIn;
    FILE *pCtmDumpFileDlIn;
    FILE *pCtmDumpFileUlOut;
    FILE *pCtmDumpFileDlOut;

protected:
    SpeechVMRecorder();

    FILE *openFile();
    static void *dumpVMRecordDataThread(void *arg);
    void triggerSpeechVm();

    bool mIsDumpThreadStart;
    AudioLock mThreadStartMutex;

    bool mIsVmEnable;

    RingBuf mRingBuf; // Internal Input Buffer for Get VM data

    pthread_t mRecordThread;
    AudioLock mMutex;

    SpeechVmType mVMConfig;
    uint32_t mOpenIndex;


private:
    static SpeechVMRecorder *mSpeechVMRecorder; // singleton
    bool   mIsCtmDebugStart;
}; // end of class SpeechVMRecorder

}; // end of namespace android

#endif // end of ANDROID_SPEECH_VM_RECORDER_H

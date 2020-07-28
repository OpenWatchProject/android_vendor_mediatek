#ifndef ANDROID_SPEECH_DATA_ENCRYPTER_H
#define ANDROID_SPEECH_DATA_ENCRYPTER_H

#include "AudioType.h"
#include "AudioUtility.h"

#include "AudioCustEncryptClient.h"

namespace android {

/***********************************************************
*   SpeechDataEncrypter Interface -  SpeechDataEncrypter
***********************************************************/

class SpeechDataEncrypter {
public:
    virtual ~SpeechDataEncrypter();

    static SpeechDataEncrypter    *GetInstance();

    int                   Start();
    int                   Stop();

    void SetEnableStatus(bool bEnable);
    bool                GetEnableStatus();
    bool                GetDumpStatus();
    bool GetStartStatus();
    uint16_t                Encryption(char *TargetBuf, char *SourceBuf, const uint16_t SourceByte);
    uint16_t                Decryption(char *TargetBuf, char *SourceBuf, const uint16_t SourceByte);

private:
    SpeechDataEncrypter();
    static SpeechDataEncrypter *mSpeechDataEncrypter; // singleton
    AudioCustEncryptClient *mAudioCustEncryptClient;

    int GetDelay();
    AudioLock mMutexDL;
    AudioLock mMutexUL;


    bool                mEnabled;
    bool                mDumpEnabled;
    bool                mStarted;
    pthread_mutex_t     mSpeechDataEncrypter_ULMutex;  // Mutex to protect internal buffer
    pthread_mutex_t     mSpeechDataEncrypter_DLMutex;  // Mutex to protect internal buffer

    FILE               *pPreEncDumpFile;
    FILE               *pPostEncDumpFile;
    FILE               *pPreDecDumpFile;
    FILE               *pPostDecDumpFile;
    /*
    * flag of dynamic enable verbose/debug log
    */
    int mLogEnable;

};


}; // namespace android

#endif

#ifndef ANDROID_AUDIO_BIT_TRUE_TEST_H
#define ANDROID_AUDIO_BIT_TRUE_TEST_H

#include <tinyalsa/asoundlib.h> // TODO(Harvey): move it

#include <pthread.h>

#include "AudioType.h"
#include <AudioLock.h>

enum {
    BIT_TRUE_TEST_DISABLE = 0,
    BIT_TRUE_TEST_DL_UL,
    BIT_TRUE_TEST_4_PIN_I2S,
};

namespace android {

class AudioBitTrueTest {
public:
    static AudioBitTrueTest *getInstance();
    ~AudioBitTrueTest();

    int setTestType(int testType);
    int getTestState();
private:
    static AudioBitTrueTest *mAudioBitTrueTest;
    AudioBitTrueTest();
    int open(int testType);
    int close();

    static void *playThread(void *arg);
    static void *verifyThread(void *arg);

    static FILE *testDumpOpen(const char *name, const char *property);
    static void testDumpClose(FILE *file);
    static void testDumpWriteData(FILE *file, const void *buffer, size_t bytes);

    AudioLock mLock;

    int mTestState;
    bool mPlayReady;

    struct mixer *mMixer;

    pthread_t mPlayThread;
    pthread_t mVerifyThread;
};

} // end namespace android

#endif // end of ANDROID_AUDIO_BIT_TRUE_TEST_H

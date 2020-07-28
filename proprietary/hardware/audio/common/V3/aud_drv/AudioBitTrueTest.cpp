#include "AudioBitTrueTest.h"
#include "AudioALSADriverUtility.h"
#include "AudioALSADeviceParser.h"
#include "AudioALSAHardwareResourceManager.h"
#include "AudioALSADeviceConfigManager.h"

#include <hardware_legacy/power.h>

#ifdef LOG_TAG
#undef LOG_TAG
#endif
#define LOG_TAG "AudioBitTrueTest"

#ifndef ARRAY_SIZE
#define ARRAY_SIZE(a) (sizeof(a) / sizeof((a)[0]))
#endif

#define TEST_DUMP_FILE_PREFIX DUMP_PATH"bit_true"

#define PLAY_MS_PER_WRITE 5
#define VERIFY_MS_PER_READ 20
#define VERIFY_TIME_MS 3000

const unsigned int sgenTable16Bit[] =
{
    0x0FE50FE5, 0x285E1C44, 0x3F4A285E, 0x53C73414,
    0x650C3F4A, 0x726F49E3, 0x7B6C53C7, 0x7FAB5CDC,
    0x7F02650C, 0x79776C43, 0x6F42726F, 0x60C67781,
    0x4E917B6C, 0x39587E27, 0x21EB7FAB, 0x09307FF4,
    0xF01A7F02, 0xD7A17CD6, 0xC0B67977, 0xAC3874ED,
    0x9AF36F42, 0x8D906884, 0x849360C6, 0x80545818,
    0x80FD4E91, 0x86884449, 0x90BD3958, 0x9F3A2DDA,
    0xB16E21EB, 0xC6A715A8, 0xDE140930, 0xF6CFFCA1,
    0x0FE5F01A, 0x285EE3BB, 0x3F4AD7A1, 0x53C7CBEB,
    0x650CC0B6, 0x726FB61C, 0x7B6CAC38, 0x7FABA323,
    0x7F029AF3, 0x797793BC, 0x6F428D90, 0x60C6887E,
    0x4E918493, 0x395881D8, 0x21EB8054, 0x0930800B,
    0xF01A80FD, 0xD7A18329, 0xC0B68688, 0xAC388B12,
    0x9AF390BD, 0x8D90977B, 0x84939F3A, 0x8054A7E7,
    0x80FDB16E, 0x8688BBB6, 0x90BDC6A7, 0x9F3AD225,
    0xB16EDE14, 0xC6A7EA57, 0xDE14F6CF, 0xF6CF035E
};

// from dvt check_bit_true_half_way_in
static bool checkBitTrue(char *inData, size_t dataSize, const unsigned int *goldenTable, size_t goldenElementCount) {
    bool ret = false;
    bool flagStart = false;

    unsigned int reviceVal;

    unsigned int idxTable = 0;

    unsigned int *data = (unsigned int *)inData;
    unsigned int maxDataIdx = dataSize / sizeof(unsigned int);

    for (size_t i = 0; i < maxDataIdx; i++) {
        reviceVal = data[i];

        if (!flagStart) {
            if (reviceVal == 0) {
                continue;
            } else {
                flagStart = true;
                // Find the corresponding table idx for first data
                for (size_t j = 0; j < goldenElementCount; j++) {
                    if (reviceVal == goldenTable[j]) {
                        idxTable = j;
                        //ALOGD("%s(), start idxTable %d", __FUNCTION__, idxTable);
                        break;
                    }
                }
            }
        }

        if (reviceVal != goldenTable[idxTable++]) {
            if (reviceVal == 0) {   // all zerio
                for (; i < maxDataIdx; i++) {
                    reviceVal = data[i];

                    if (reviceVal != 0) {
                        ALOGW("i %zu, revice_val %x != 0\n", i, reviceVal);
                        return false;
                    }
                }

                return ret;
            } else {
                ALOGW("i %zu, revice_val %x !=  golden %x\n", i, reviceVal, goldenTable[idxTable - 1]);
                ret = false;
                return ret;
            }
        }
        else {
            ret = true;
        }

        if (idxTable == goldenElementCount) {
            idxTable = 0;
        }
    }

    if (!flagStart) {
        ALOGW("[check_bit_true] PANIC, It's all zero in here....");
    }

    return ret;
}

namespace android {
AudioBitTrueTest *AudioBitTrueTest::mAudioBitTrueTest = NULL;
AudioBitTrueTest *AudioBitTrueTest::getInstance() {
    static AudioLock mGetInstanceLock;
    AL_AUTOLOCK(mGetInstanceLock);

    if (mAudioBitTrueTest == NULL) {
        mAudioBitTrueTest = new AudioBitTrueTest();
    }
    ASSERT(mAudioBitTrueTest != NULL);
    return mAudioBitTrueTest;
}

AudioBitTrueTest::AudioBitTrueTest() :
    mTestState(BIT_TRUE_TEST_DISABLE),
    mPlayReady(0),
    mMixer(AudioALSADriverUtility::getInstance()->getMixer()),
    mPlayThread(0),
    mVerifyThread(0) {
}

AudioBitTrueTest::~AudioBitTrueTest() {

}

int AudioBitTrueTest::setTestType(int testType) {
    AL_AUTOLOCK(mLock);

    if (testType == BIT_TRUE_TEST_DISABLE) {
        return close();
    } else {
        return open(testType);
    }
}

static const char bitTrueWakeLockName[] = "BIT_TRUE_WAKELOCK_NAME";
int AudioBitTrueTest::open(int testType) {
    ALOGD("+%s(), testType %d", __FUNCTION__, testType);

    if (mTestState != BIT_TRUE_TEST_DISABLE) {
        ALOGW("%s(), already in mTestState %d", __FUNCTION__, mTestState);
        return -EINVAL;
    }

    mTestState = testType;

    int ret;

    ret = acquire_wake_lock(PARTIAL_WAKE_LOCK, bitTrueWakeLockName);
    if (ret) {
        ALOGW("%s(), acquire_wake_lock fail, ret %d", __FUNCTION__, ret);
    }

    ret = pthread_create(&mPlayThread, NULL, AudioBitTrueTest::playThread, this);
    if (ret) {
        ALOGE("%s() create mPlayThread fail, ret = %d!!", __FUNCTION__, ret);
        ASSERT(0);
    }

    // wait for play setup finish
    int sleepTotal_us = 0;
    while (!mPlayReady) {
        usleep(500);
        if (sleepTotal_us >= 3 * 1000 * 1000) {
            ALOGE("%s(), timeout > 3 sec, mPlayReady %d", __FUNCTION__, mPlayReady);
            ASSERT(0);
            break;
        }
        sleepTotal_us += 500;
    }

    ret = pthread_create(&mVerifyThread, NULL, AudioBitTrueTest::verifyThread, this);
    if (ret) {
        ALOGE("%s() create mPlayThread fail, ret = %d!!", __FUNCTION__, ret);
        ASSERT(0);
    }

    ALOGD("-%s()", __FUNCTION__);
    return 0;
}

int AudioBitTrueTest::close() {
    ALOGD("+%s()", __FUNCTION__);

    if (mTestState == BIT_TRUE_TEST_DISABLE) {
        ALOGW("%s(), no test running", __FUNCTION__);
        return -EINVAL;
    }

    mTestState = BIT_TRUE_TEST_DISABLE;

    int ret;
    void *retval;

    ret = pthread_join(mPlayThread, &retval);
    if (ret) {
        ALOGE("%s(), mPlayThread pthread_join fail, ret = %d", __FUNCTION__, ret);
        ASSERT(0);
    }

    ret = pthread_join(mVerifyThread, &retval);
    if (ret) {
        ALOGE("%s(), mVerifyThread pthread_join fail, ret = %d", __FUNCTION__, ret);
        ASSERT(0);
    }

    release_wake_lock(bitTrueWakeLockName);

    ALOGD("-%s()", __FUNCTION__);
    return 0;
}

int AudioBitTrueTest::getTestState() {
    return mTestState;
}

void *AudioBitTrueTest::playThread(void *arg) {
    AudioBitTrueTest *test = static_cast<AudioBitTrueTest *>(arg);
    int ret;
    int testState = test->getTestState();

    ALOGD("+%s(), pid: %d, tid: %d, testState %d", __FUNCTION__, getpid(), gettid(), testState);
#if defined(MTK_AUDIO_KS)
    // prepare FE
    int pcmIdx = AudioALSADeviceParser::getInstance()->GetPcmIndexByString(keypcmPlayback1);
    int cardIdx = AudioALSADeviceParser::getInstance()->GetCardIndexByString(keypcmPlayback1);

    struct pcm_config pcmConfig;
    memset(&pcmConfig, 0, sizeof(pcmConfig));
    pcmConfig.channels = 2;
    pcmConfig.rate = 48000;
    pcmConfig.period_size = 1024;
    pcmConfig.period_count = 2;
    pcmConfig.format = PCM_FORMAT_S16_LE;
    pcmConfig.start_threshold = 0;
    pcmConfig.stop_threshold = 0;
    pcmConfig.silence_threshold = 0;

    // connect path
    switch (testState) {
    case BIT_TRUE_TEST_DL_UL:
        AudioALSADeviceConfigManager::getInstance()->setMixerCtl(String8("I2S3_CH1 DL1_CH1"), String8("1"));
        AudioALSADeviceConfigManager::getInstance()->setMixerCtl(String8("I2S3_CH2 DL1_CH2"), String8("1"));
        AudioALSADeviceConfigManager::getInstance()->setMixerCtl(String8("I2S3_HD_Mux"), String8("Low_Jitter"));
        break;
    case BIT_TRUE_TEST_4_PIN_I2S:
        AudioALSADeviceConfigManager::getInstance()->setMixerCtl(String8("I2S3_CH1 DL1_CH1"), String8("1"));
        AudioALSADeviceConfigManager::getInstance()->setMixerCtl(String8("I2S3_CH2 DL1_CH2"), String8("1"));
        AudioALSADeviceConfigManager::getInstance()->setMixerCtl(String8("I2S3_HD_Mux"), String8("Low_Jitter"));
        break;
    default:
        break;
    }

    struct pcm *pcm = pcm_open(cardIdx, pcmIdx, PCM_OUT | PCM_MONOTONIC, &pcmConfig);
    if (pcm == NULL) {
        ALOGE("%s(), pcm == NULL!!", __FUNCTION__);
    } else if (pcm_is_ready(pcm) == false) {
        ALOGE("%s(), pcm_is_ready(%p) == false due to %s, close pcm.", __FUNCTION__, pcm, pcm_get_error(pcm));
        pcm_close(pcm);
        pcm = NULL;
    } else if (pcm_prepare(pcm) != 0) {
        ALOGE("%s(), pcm_prepare(%p) == false due to %s, close pcm.", __FUNCTION__, pcm, pcm_get_error(pcm));
        pcm_close(pcm);
        pcm = NULL;
    }

    // prepare data
    size_t sizePerFrame = getSizePerFrame(audio_format_from_pcm_format(pcmConfig.format), pcmConfig.channels);
    size_t sgenTableSize = ARRAY_SIZE(sgenTable16Bit) * sizePerFrame;
    size_t testDataSize = ((PLAY_MS_PER_WRITE * pcmConfig.rate) / 1000) * sizePerFrame;
    testDataSize = (testDataSize / sgenTableSize) * sgenTableSize;
    char *testData = new char[testDataSize];

    ALOGD("%s(), sgenTableSize %zu, testDataSize %zu", __FUNCTION__, sgenTableSize, testDataSize);

    if (!testData) {
        ALOGE("%s(), allocate testData failed", __FUNCTION__);
        ASSERT(0);
    }

    for (size_t i = 0; i < testDataSize; i += sgenTableSize) {
        memcpy(testData + i, sgenTable16Bit, sgenTableSize);
    }

    test->mPlayReady = true;

    // play start
    while (test->getTestState() != BIT_TRUE_TEST_DISABLE && pcm && testData) {
        ret = pcm_write(pcm, testData, testDataSize);
        if (ret) {
            ALOGE("%s(), pcm_write() error, ret = %d", __FUNCTION__, ret);
        }
    }

    // stop pcm
    if (pcm != NULL) {
        pcm_stop(pcm);
        pcm_close(pcm);
        pcm = NULL;
    }

    // disconnect path
    switch (testState) {
    case BIT_TRUE_TEST_DL_UL:
        AudioALSADeviceConfigManager::getInstance()->setMixerCtl(String8("I2S3_CH1 DL1_CH1"), String8("0"));
        AudioALSADeviceConfigManager::getInstance()->setMixerCtl(String8("I2S3_CH2 DL1_CH2"), String8("0"));
        AudioALSADeviceConfigManager::getInstance()->setMixerCtl(String8("I2S3_HD_Mux"), String8("Normal"));
        break;
    case BIT_TRUE_TEST_4_PIN_I2S:
        AudioALSADeviceConfigManager::getInstance()->setMixerCtl(String8("I2S3_CH1 DL1_CH1"), String8("0"));
        AudioALSADeviceConfigManager::getInstance()->setMixerCtl(String8("I2S3_CH2 DL1_CH2"), String8("0"));
        AudioALSADeviceConfigManager::getInstance()->setMixerCtl(String8("I2S3_HD_Mux"), String8("Normal"));
        break;
    default:
        break;
    }

    if (testData) {
        delete[] testData;
    }
#endif
    test->mPlayReady = false;

    ALOGD("-%s(), pid: %d, tid: %d", __FUNCTION__, getpid(), gettid());
    return NULL;
}

static int verifyRoutineUpdate(bool enable, int testState) {
    if (enable) {
        switch (testState) {
        case BIT_TRUE_TEST_4_PIN_I2S:
            AudioALSADeviceConfigManager::getInstance()->setMixerCtl(String8("I2S3_CH1 DL1_CH1"), String8("1"));
            AudioALSADeviceConfigManager::getInstance()->setMixerCtl(String8("I2S3_CH2 DL1_CH2"), String8("1"));
            AudioALSADeviceConfigManager::getInstance()->setMixerCtl(String8("I2S0_Lpbk_Mux"), String8("Lpbk"));
            break;
        case BIT_TRUE_TEST_DL_UL:
        default:
            break;
        }
    } else {
        switch (testState) {
        case BIT_TRUE_TEST_4_PIN_I2S:
            AudioALSADeviceConfigManager::getInstance()->setMixerCtl(String8("I2S3_CH1 DL1_CH1"), String8("0"));
            AudioALSADeviceConfigManager::getInstance()->setMixerCtl(String8("I2S3_CH2 DL1_CH2"), String8("0"));
            break;
        case BIT_TRUE_TEST_DL_UL:
        default:
            break;
        }
    }

    return 0;
}

void *AudioBitTrueTest::verifyThread(void *arg) {
    AudioBitTrueTest *test = static_cast<AudioBitTrueTest *>(arg);
    int ret;
    int testState = test->getTestState();

    unsigned long long round = 0;

    ALOGD("+%s(), pid: %d, tid: %d, testState %d", __FUNCTION__, getpid(), gettid(), testState);
#if defined(MTK_AUDIO_KS)
    // prepare FE
    int pcmIdx = AudioALSADeviceParser::getInstance()->GetPcmIndexByString(keypcmCapture2);
    int cardIdx = AudioALSADeviceParser::getInstance()->GetCardIndexByString(keypcmCapture2);

    struct pcm_config pcmConfig;
    memset(&pcmConfig, 0, sizeof(pcmConfig));
    pcmConfig.channels = 2;
    pcmConfig.rate = 48000;
    pcmConfig.format = PCM_FORMAT_S16_LE;
    pcmConfig.period_size = 1024;
    pcmConfig.period_count = 4;
    pcmConfig.start_threshold = 0;
    pcmConfig.stop_threshold = 0;
    pcmConfig.silence_threshold = 0;

    // connect path
    switch (testState) {
    case BIT_TRUE_TEST_DL_UL:
        AudioALSADeviceConfigManager::getInstance()->setMixerCtl(String8("UL2_CH1 DL1_CH1"), String8("1"));
        AudioALSADeviceConfigManager::getInstance()->setMixerCtl(String8("UL2_CH2 DL1_CH2"), String8("1"));
        break;
    case BIT_TRUE_TEST_4_PIN_I2S:
        AudioALSADeviceConfigManager::getInstance()->setMixerCtl(String8("UL2_CH1 I2S0_CH1"), String8("1"));
        AudioALSADeviceConfigManager::getInstance()->setMixerCtl(String8("UL2_CH2 I2S0_CH2"), String8("1"));
        AudioALSADeviceConfigManager::getInstance()->setMixerCtl(String8("I2S0_HD_Mux"), String8("Low_Jitter"));
        AudioALSADeviceConfigManager::getInstance()->setMixerCtl(String8("I2S0_Lpbk_Mux"), String8("Lpbk"));
        break;
    default:
        break;
    }

    struct pcm *pcm = pcm_open(cardIdx, pcmIdx, PCM_IN | PCM_MONOTONIC, &pcmConfig);
    if (pcm == NULL) {
        ALOGE("%s(), pcm == NULL!!", __FUNCTION__);
    } else if (pcm_is_ready(pcm) == false) {
        ALOGE("%s(), pcm_is_ready(%p) == false due to %s, close pcm.", __FUNCTION__, pcm, pcm_get_error(pcm));
        pcm_close(pcm);
        pcm = NULL;
    } else if (pcm_prepare(pcm) != 0) {
        ALOGE("%s(), pcm_prepare(%p) == false due to %s, close pcm.", __FUNCTION__, pcm, pcm_get_error(pcm));
        pcm_close(pcm);
        pcm = NULL;
    }

    // prepare data
    size_t sizePerFrame = getSizePerFrame(audio_format_from_pcm_format(pcmConfig.format), pcmConfig.channels);
    size_t readDataSize = ((VERIFY_MS_PER_READ * pcmConfig.rate) / 1000) * sizePerFrame;

    char *readData = new char[readDataSize];
    if (!readData) {
        ALOGE("%s(), allocate readData failed", __FUNCTION__);
        ASSERT(0);
    }

    size_t verifyDataSize = ((VERIFY_TIME_MS * pcmConfig.rate) / 1000) * sizePerFrame;
    verifyDataSize = (verifyDataSize / readDataSize) * readDataSize;    // verifyDataSize must be multiple of readDataSize
    char *verifyData = new char[verifyDataSize];
    if (!verifyData) {
        ALOGE("%s(), allocate verifyData failed", __FUNCTION__);
        ASSERT(0);
    }

    ALOGD("%s(), readDataSize %zu, verifyDataSize %zu", __FUNCTION__, readDataSize, verifyDataSize);

    // verify start
    while (test->getTestState() != BIT_TRUE_TEST_DISABLE && pcm && readData) {
        size_t curVerifyDataSize = 0;
        ret = 0;
        bool pass;

        // clean data
        memset(verifyData, 0, verifyDataSize);

        // collect verify data
        verifyRoutineUpdate(true, testState);
        while (test->getTestState() != BIT_TRUE_TEST_DISABLE && curVerifyDataSize < verifyDataSize && ret == 0) {

            ret = pcm_read(pcm, readData, readDataSize);
            if (ret) {
                ALOGE("%s(), echo ref pcm_read() error, ret = %d", __FUNCTION__, ret);
            }

            memcpy(verifyData + curVerifyDataSize, readData, readDataSize);
            curVerifyDataSize += readDataSize;
        }
        pcm_stop(pcm);

        // verify data
        pass = checkBitTrue(verifyData, curVerifyDataSize, sgenTable16Bit, ARRAY_SIZE(sgenTable16Bit));

        // dump data
        FILE *dumpFile;
        if (!pass) {
            ALOGW("%s(), fail at round %llu\n", __FUNCTION__, round);
            dumpFile = testDumpOpen("fail", streamout_propty);
            testDumpWriteData(dumpFile, verifyData, curVerifyDataSize);
            usleep(1 * 1000 * 1000);    // wait for dump ready
            testDumpClose(dumpFile);
        }

        verifyRoutineUpdate(false, testState);
        round++;
    }

    // stop pcm
    if (pcm != NULL) {
        pcm_stop(pcm);
        pcm_close(pcm);
        pcm = NULL;
    }

    // disconnect path
    switch (testState) {
    case BIT_TRUE_TEST_DL_UL:
        AudioALSADeviceConfigManager::getInstance()->setMixerCtl(String8("UL2_CH1 DL1_CH1"), String8("0"));
        AudioALSADeviceConfigManager::getInstance()->setMixerCtl(String8("UL2_CH2 DL1_CH2"), String8("0"));
        break;
    case BIT_TRUE_TEST_4_PIN_I2S:
        AudioALSADeviceConfigManager::getInstance()->setMixerCtl(String8("UL2_CH1 I2S0_CH1"), String8("0"));
        AudioALSADeviceConfigManager::getInstance()->setMixerCtl(String8("UL2_CH2 I2S0_CH2"), String8("0"));
        AudioALSADeviceConfigManager::getInstance()->setMixerCtl(String8("I2S0_HD_Mux"), String8("Normal"));
        AudioALSADeviceConfigManager::getInstance()->setMixerCtl(String8("I2S0_Lpbk_Mux"), String8("Normal"));
        break;
    default:
        break;
    }

    if (readData) {
        delete[] readData;
    }

    if (verifyData) {
        delete[] verifyData;
    }
#endif
    ALOGD("-%s(), pid: %d, tid: %d, round %llu", __FUNCTION__, getpid(), gettid(), round);
    return NULL;
}

// dump file
FILE *AudioBitTrueTest::testDumpOpen(const char *name, const char *property) {
    static unsigned int dumpFileCount = 0;

    ALOGV("%s()", __FUNCTION__);
    FILE *file = NULL;
    char dumpFileName[128];
    sprintf(dumpFileName, "%s.%s.%u.pid%d.tid%d.48000.16bit.2ch.pcm", TEST_DUMP_FILE_PREFIX, name, dumpFileCount, getpid(), gettid());

    file = NULL;
    file = AudioOpendumpPCMFile(dumpFileName, property);

    if (file != NULL) {
        ALOGD("%s DumpFileName = %s", __FUNCTION__, dumpFileName);

        dumpFileCount++;
        dumpFileCount %= MAX_DUMP_NUM;
    }

    return file;
}

void AudioBitTrueTest::testDumpClose(FILE *file) {
    ALOGV("%s()", __FUNCTION__);
    if (file) {
        AudioCloseDumpPCMFile(file);
        ALOGD("%s(), close it", __FUNCTION__);
    }
}

void AudioBitTrueTest::testDumpWriteData(FILE *file, const void *buffer, size_t bytes) {
    if (file) {
        AudioDumpPCMData((void *)buffer, bytes, file);
    }
}

} // end of namespace android

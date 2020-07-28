#ifdef LOG_TAG
#undef LOG_TAG
#endif
#define LOG_TAG "SpeechDataEncrypter"
#include <unistd.h>
#include <sys/time.h>
#include <string.h>
#include "SpeechType.h"
#include "SpeechDriverFactory.h"

#include "SpeechDataEncrypter.h"

/*****************************************************************************
*                          MACRO
******************************************************************************
*/

//#define FORCE_ENABLE_ENCRYPT
//#define FORCE_ENABLE_DUMP
//#define DELAY_SIMULATION

//#define DUMP_PRE_ENCRYPT
//#define DUMP_POST_ENCRYPT
//#define DUMP_PRE_DECRYPT
//#define DUMP_POST_DECRYPT

/*****************************************************************************
*                          C O N S T A N T S
******************************************************************************
*/
static const char     kPrefixOfFileName[] = "/data/vendor/audiohal/audio_dump/SpeechData";
static const uint32_t kSizeOfPrefixOfFileName = sizeof(kPrefixOfFileName) - 1;
static const uint32_t kMaxSizeOfFileName = 128;

/*==============================================================================
 *                     Property keys
 *============================================================================*/
const char* PROPERTY_KEY_SDE_DUMP_ON = "persist.vendor.audiohal.sph_enc_dump_on";
const char* PROPERTY_KEY_SDE_DELAY = "persist.vendor.audiohal.sph_enc_delay";

namespace android {

/***********************************************************
*
*   SpeechDataEncrypter Interface
*
***********************************************************/
SpeechDataEncrypter *SpeechDataEncrypter::mSpeechDataEncrypter = NULL;
SpeechDataEncrypter *SpeechDataEncrypter::GetInstance() {
    static Mutex mGetInstanceLock;
    Mutex::Autolock _l(mGetInstanceLock);

    if (mSpeechDataEncrypter == NULL) {
        mSpeechDataEncrypter = new SpeechDataEncrypter();
    }
    ASSERT(mSpeechDataEncrypter != NULL);
    return mSpeechDataEncrypter;
}

SpeechDataEncrypter::SpeechDataEncrypter() {
    mEnabled = false;
    mDumpEnabled =  false;
    mStarted = false;
    pPreEncDumpFile = NULL;
    pPostEncDumpFile = NULL;
    pPreDecDumpFile = NULL;
    pPostDecDumpFile = NULL;
    mAudioCustEncryptClient = NULL;
    mAudioCustEncryptClient = AudioCustEncryptClient::GetInstance();
    mLogEnable = 0;

    ALOGD("+%s() mAudioCustEncryptClient->Initial()", __FUNCTION__);

    mAudioCustEncryptClient->Initial();

    ALOGD("-%s() mAudioCustEncryptClient->Initial()", __FUNCTION__);
}

SpeechDataEncrypter::~SpeechDataEncrypter() {
    Stop();
    mAudioCustEncryptClient = NULL;
}

int SpeechDataEncrypter::Start() {
    ALOGD("%s()", __FUNCTION__);
    mLogEnable = __android_log_is_loggable(ANDROID_LOG_DEBUG, LOG_TAG, ANDROID_LOG_INFO);

    if (mStarted) {
        ALOGW("%s() is already started", __FUNCTION__);

    } else {
        AL_LOCK(mMutexDL);

        if (mDumpEnabled) {
            char dump_file_path_enc_pre[kMaxSizeOfFileName];
            char dump_file_path_enc_post[kMaxSizeOfFileName];
            char dump_file_path_dec_pre[kMaxSizeOfFileName];
            char dump_file_path_dec_post[kMaxSizeOfFileName];
            memset((void *)dump_file_path_enc_pre, 0, kMaxSizeOfFileName);
            memset((void *)dump_file_path_enc_post, 0, kMaxSizeOfFileName);
            memset((void *)dump_file_path_dec_pre, 0, kMaxSizeOfFileName);
            memset((void *)dump_file_path_dec_post, 0, kMaxSizeOfFileName);
            time_t rawtime;
            time(&rawtime);
            struct tm *timeinfo = localtime(&rawtime);
            strftime(dump_file_path_enc_pre, kMaxSizeOfFileName, "/data/vendor/audiohal/audio_dump/%Y_%m_%d_%H%M%S_EncPre.dump", timeinfo);
            strftime(dump_file_path_enc_post, kMaxSizeOfFileName, "/data/vendor/audiohal/audio_dump/%Y_%m_%d_%H%M%S_EncPost.dump", timeinfo);
            strftime(dump_file_path_dec_pre, kMaxSizeOfFileName, "/data/vendor/audiohal/audio_dump/%Y_%m_%d_%H%M%S_DecPre.dump", timeinfo);
            strftime(dump_file_path_dec_post, kMaxSizeOfFileName, "/data/vendor/audiohal/audio_dump/%Y_%m_%d_%H%M%S_DecPost.dump", timeinfo);
#ifdef DUMP_PRE_ENCRYPT
            if (pPreEncDumpFile == NULL) {
                AudiocheckAndCreateDirectory(dump_file_path_enc_pre);
                pPreEncDumpFile = fopen(dump_file_path_enc_pre, "wb");
            }
            if (pPreEncDumpFile == NULL) {
                ALOGW("%s() Fail to Open \"%s\"", __FUNCTION__, dump_file_path_enc_pre);
            } else {
                ALOGD("%s(), Open: \"%s\"", __FUNCTION__, dump_file_path_enc_pre);
            }
#endif
#ifdef DUMP_POST_ENCRYPT
            if (pPostEncDumpFile == NULL) {
                AudiocheckAndCreateDirectory(dump_file_path_enc_post);
                pPostEncDumpFile = fopen(dump_file_path_enc_post, "wb");
            }
            if (pPostEncDumpFile == NULL) {
                ALOGW("%s() Fail to Open \"%s\"", __FUNCTION__, dump_file_path_enc_post);
            } else {
                ALOGD("%s(), Open \"%s\"", __FUNCTION__, dump_file_path_enc_post);
            }

#endif
#ifdef DUMP_PRE_DECRYPT
            if (pPreDecDumpFile == NULL) {
                AudiocheckAndCreateDirectory(dump_file_path_dec_pre);
                pPreDecDumpFile = fopen(dump_file_path_dec_pre, "wb");
            }
            if (pPreDecDumpFile == NULL) {
                ALOGW("%s() Fail to Open \"%s\"", __FUNCTION__, dump_file_path_dec_pre);
            } else {
                ALOGD("%s(), Open: \"%s\"", __FUNCTION__, dump_file_path_dec_pre);
            }
#endif
#ifdef DUMP_POST_DECRYPT
            if (pPostDecDumpFile == NULL) {
                AudiocheckAndCreateDirectory(dump_file_path_dec_post);
                pPostDecDumpFile = fopen(dump_file_path_dec_post, "wb");
            }
            if (pPostDecDumpFile == NULL) {
                ALOGW("%s() Fail to Open \"%s\"", __FUNCTION__, dump_file_path_dec_post);
            } else {
                ALOGD("%s(), Open: \"%s\"", __FUNCTION__, dump_file_path_dec_post);
            }
#endif

        }
        AL_UNLOCK(mMutexDL);
        mStarted = true;
        SpeechDriverFactory::GetInstance()->GetSpeechDriver()->SetEncryption(true);
    }
    return true;
}

int SpeechDataEncrypter::Stop() {
    ALOGD("%s()", __FUNCTION__);
    if (!mStarted) {
        ALOGW("%s() is already stopped", __FUNCTION__);
    } else {
        AL_LOCK(mMutexDL);

        mStarted = false;


        if (mDumpEnabled) {
#ifdef DUMP_PRE_ENCRYPT
            if (pPreEncDumpFile != NULL) {
                fclose(pPreEncDumpFile);
            } else {
                ALOGW("%s(), pPreEncDumpFile == NULL!!!!!", __FUNCTION__);
            }
#endif
#ifdef DUMP_POST_ENCRYPT
            if (pPostEncDumpFile != NULL) {
                fclose(pPostEncDumpFile);
            } else {
                ALOGW("%s(), pPostEncDumpFile == NULL!!!!!", __FUNCTION__);
            }
#endif
#ifdef DUMP_PRE_DECRYPT
            if (pPreDecDumpFile != NULL) {
                fclose(pPreDecDumpFile);
            } else {
                ALOGW("%s(), pPreDecDumpFile == NULL!!!!!", __FUNCTION__);
            }
#endif
#ifdef DUMP_POST_DECRYPT
            if (pPostDecDumpFile != NULL) {
                fclose(pPostDecDumpFile);
            } else {
                ALOGW("%s(), pPostDecDumpFile == NULL!!!!!", __FUNCTION__);
            }
#endif
        }
        AL_UNLOCK(mMutexDL);
    }
    return true;
}

void SpeechDataEncrypter::SetEnableStatus(bool bEnable) {
    if (mEnabled != bEnable) {
        ALOGD("%s(), ori mEnabled=%d, set value=%d", __FUNCTION__, mEnabled, bEnable);
        SpeechDriverFactory *pSpeechDriverFactory = SpeechDriverFactory::GetInstance();
        SpeechDriverInterface *pSpeechDriver = NULL;
        mEnabled = bEnable;
        for (int modem_index = MODEM_1; modem_index < NUM_MODEM; modem_index++) {
            pSpeechDriver = pSpeechDriverFactory->GetSpeechDriverByIndex((modem_index_t)modem_index);
            if (pSpeechDriver != NULL) { // Might be single talk and some speech driver is NULL
                pSpeechDriver->SetEncryption(mEnabled);
            }
        }
    } else {
        ALOGW("%s(), mEnabled == bEnable(%d) return.", __FUNCTION__, bEnable);
    }
}

bool SpeechDataEncrypter::GetEnableStatus() {
#ifdef FORCE_ENABLE_ENCRYPT
    ALOGD("%s(), Force enable", __FUNCTION__);
    return true;
#else
    ALOGD("%s(), mEnabled=%d", __FUNCTION__, mEnabled);
    return mEnabled;
#endif
}

bool SpeechDataEncrypter::GetStartStatus() {
    ALOGD("-%s(), mStarted=%d", __FUNCTION__, mStarted);
    return mStarted;
}

bool SpeechDataEncrypter::GetDumpStatus() {
#if defined(FORCE_ENABLE_DUMP)
    mDumpEnabled =  true;
#else
    char property_value[PROPERTY_VALUE_MAX];
    property_get(PROPERTY_KEY_SDE_DUMP_ON, property_value, "0"); //"0": default off
    mDumpEnabled = (property_value[0] == '0') ? false : true;
#endif
    ALOGD("-%s(), mDumpEnabled=%d", __FUNCTION__, mDumpEnabled);
    return mDumpEnabled;
}

int SpeechDataEncrypter::GetDelay() {
#ifdef DELAY_SIMULATION
    char property_value[PROPERTY_VALUE_MAX];
    property_get(PROPERTY_KEY_SDE_DELAY, property_value, "0"); //"0": default off
    int DelayMS = atoi(property_value);
    ALOGD("%s(), DelayMS=%d", __FUNCTION__, DelayMS);
    return DelayMS;
#else
    return 0;
#endif
}

uint16_t SpeechDataEncrypter::Encryption(char *TargetBuf, char *SourceBuf, const uint16_t SourceByte) {
    uint16_t TargetByte = 0;

    ALOGD("+%s(), SourceByte= %d", __FUNCTION__, SourceByte);
    struct timespec entertime;
    struct timespec leavetime;
    unsigned long long timediffns = 0;
    entertime = GetSystemTime();
    AL_LOCK(mMutexUL);

    ALOGD_IF(mLogEnable, "+%s(), SourceByte= %d, SourceBuf[0] = 0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x", __FUNCTION__, SourceByte, *SourceBuf, *(SourceBuf + 1), *(SourceBuf + 2), *(SourceBuf + 3), *(SourceBuf + 4), *(SourceBuf + 5), *(SourceBuf + 6), *(SourceBuf + 7));
    //do encrypt
    TargetByte = AudioCustEncryptClient::GetInstance()->EncryptProcess(TargetBuf, SourceBuf, SourceByte);
    ALOGD_IF(mLogEnable, "-%s(), SourceByte= %d, TargetByte= %d, TargetBuf[0] = 0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x", __FUNCTION__, SourceByte, TargetByte, *TargetBuf, *(TargetBuf + 1), *(TargetBuf + 2), *(TargetBuf + 3), *(TargetBuf + 4), *(TargetBuf + 5), *(TargetBuf + 6), *(TargetBuf + 7));

    if (mDumpEnabled) {
#ifdef DUMP_PRE_ENCRYPT
        if (pPreEncDumpFile != NULL) {
            fwrite(SourceBuf, sizeof(char), SourceByte, pPreEncDumpFile);
        } else {
            ALOGW("%s(), pPreEncDumpFile == NULL!!!!!", __FUNCTION__);
        }

#endif
#ifdef DUMP_POST_ENCRYPT
        if (pPostEncDumpFile != NULL) {
            fwrite(TargetBuf, sizeof(char), TargetByte, pPostEncDumpFile);
        } else {
            ALOGW("%s(), pPostEncDumpFile == NULL!!!!!", __FUNCTION__);
        }
#endif

    }
    leavetime = GetSystemTime();

    timediffns = TimeDifference(leavetime, entertime);
    ALOGD_IF(mLogEnable, "-%s(), SourceByte= %d, TargetByte= %d, process time=%lld(ns)", __FUNCTION__, SourceByte, TargetByte, timediffns);

    AL_UNLOCK(mMutexUL);
    int DelayMS = GetDelay();
    if (DelayMS != 0) {
        ALOGD("%s(), delay %d ms", __FUNCTION__, DelayMS);

        usleep(DelayMS * 1000);
    }
    return TargetByte;
}

uint16_t SpeechDataEncrypter::Decryption(char *TargetBuf, char *SourceBuf, const uint16_t SourceByte) {
    uint16_t TargetByte = 0;
    struct timespec entertime;
    struct timespec leavetime;
    unsigned long long timediffns = 0;
    entertime = GetSystemTime();


    AL_LOCK(mMutexDL);
    ALOGD_IF(mLogEnable, "+%s(), SourceByte= %d, SourceBuf[0] = 0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x", __FUNCTION__, SourceByte, *SourceBuf, *(SourceBuf + 1), *(SourceBuf + 2), *(SourceBuf + 3), *(SourceBuf + 4), *(SourceBuf + 5), *(SourceBuf + 6), *(SourceBuf + 7));
    TargetByte = AudioCustEncryptClient::GetInstance()->DecryptProcess(TargetBuf, SourceBuf, SourceByte);
    ALOGD_IF(mLogEnable, "-%s(), SourceByte= %d, TargetByte= %d, TargetBuf[0] = 0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x", __FUNCTION__, SourceByte, TargetByte, *TargetBuf, *(TargetBuf + 1), *(TargetBuf + 2), *(TargetBuf + 3), *(TargetBuf + 4), *(TargetBuf + 5), *(TargetBuf + 6), *(TargetBuf + 7));

    if (mDumpEnabled) {
#ifdef DUMP_PRE_DECRYPT
        if (pPreDecDumpFile != NULL) {
            fwrite(SourceBuf, sizeof(char), SourceByte, pPreDecDumpFile);
        } else {
            ALOGW("%s(), pPreDecDumpFile == NULL!!!!!", __FUNCTION__);
        }

#endif
#ifdef DUMP_POST_DECRYPT
        if (pPostDecDumpFile != NULL) {
            fwrite(TargetBuf, sizeof(char), TargetByte, pPostDecDumpFile);
        } else {
            ALOGW("%s(), pPostDecDumpFile == NULL!!!!!", __FUNCTION__);
        }
#endif

    }
    leavetime = GetSystemTime();

    timediffns = TimeDifference(leavetime, entertime);
    ALOGD_IF(mLogEnable, "-%s(), SourceByte= %d, TargetByte= %d, process time=%lld(ns)", __FUNCTION__, SourceByte, TargetByte, timediffns);

    AL_UNLOCK(mMutexDL);
    int DelayMS = GetDelay();
    if (DelayMS != 0) {
        ALOGD("%s(), delay %d ms", __FUNCTION__, DelayMS);
        usleep(DelayMS * 1000);
    }

    return TargetByte;
}


} // end of namespace android

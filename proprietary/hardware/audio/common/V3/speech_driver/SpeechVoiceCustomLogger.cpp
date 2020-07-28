#include "SpeechVoiceCustomLogger.h"

#include <sys/prctl.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <cutils/properties.h>
#include <utils/threads.h>

#include <ftw.h>
#include <hardware_legacy/power.h>



//#define FORCE_ENABLE_VCL
#define LOG_TAG "SpeechVoiceCustomLogger"

namespace android {

/*==============================================================================
 *                     Property keys
 *============================================================================*/
const char* PROPERTY_KEY_VCL_ON  = "persist.vendor.audiohal.vcl_on";
const char* PROPERTY_KEY_VCL_RECYCLE_ON = "persist.vendor.audiohal.vcl_recycle_on";

/*==============================================================================
 *                     Constant
 *============================================================================*/
static const char VCL_RECORD_WAKELOCK_NAME[] = "VCL_RECORD_WAKELOCK";

static const uint32_t kCondWaitTimeoutMsec = 100; // 100 ms (modem local buf: 10k, and EPL has 2304 byte for each frame (20 ms))

static const uint32_t kReadBufferSize = 0x4000;   // 16 k


/*==============================================================================
 *                     VCL File Recycle
 *============================================================================*/
static const uint32_t kMaxNumOfVCLFiles = 4096;

static const uint32_t kMinKeepNumOfVCLFiles = 16;        // keep at least 16 files which will not be removed
static const uint32_t kMaxSizeOfAllVCLFiles = 209715200; // Total > 200 M

static const char     kFolderOfVCLFile[]     = "/data/vendor/audiohal/audio_dump/";
static const char     kPrefixOfVCLFileName[] = "/data/vendor/audiohal/audio_dump/VCL";
static const uint32_t kSizeOfPrefixOfVCLFileName = sizeof(kPrefixOfVCLFileName) - 1;
static const uint32_t kMaxSizeOfVCLFileName = 128;

typedef struct {
    char     path[kMaxSizeOfVCLFileName];
    uint32_t size;
} vcl_file_info_t;

static vcl_file_info_t gVCLFileList[kMaxNumOfVCLFiles];
static uint32_t       gNumOfVCLFiles = 0;
static uint32_t       gTotalSizeOfVCLFiles; // Total size of VCL files in SD card

static int GetVCLFileList(const char *path, const struct stat *sb, int typeflag) {
    if (strncmp(path, kPrefixOfVCLFileName, kSizeOfPrefixOfVCLFileName) != 0) {
        return 0;
    }

    if (gNumOfVCLFiles >= kMaxNumOfVCLFiles) {
        return 0;
    }

    // path
    audio_strncpy(gVCLFileList[gNumOfVCLFiles].path, path, kMaxSizeOfVCLFileName);

    // size
    gVCLFileList[gNumOfVCLFiles].size = sb->st_size;
    gTotalSizeOfVCLFiles += sb->st_size;

    // increase index
    gNumOfVCLFiles++;

    return 0; // To tell ftw() to continue
}

static int CompareVCLFileName(const void *a, const void *b) {
    return strcmp(((vcl_file_info_t *)a)->path,
                  ((vcl_file_info_t *)b)->path);
}


/*==============================================================================
 *                     Implementation
 *============================================================================*/

SpeechVoiceCustomLogger *SpeechVoiceCustomLogger::mSpeechVoiceCustomLogger = NULL;
SpeechVoiceCustomLogger *SpeechVoiceCustomLogger::GetInstance() {
    static Mutex mGetInstanceLock;
    Mutex::Autolock _l(mGetInstanceLock);

    if (mSpeechVoiceCustomLogger == NULL) {
        mSpeechVoiceCustomLogger = new SpeechVoiceCustomLogger();
    }
    ASSERT(mSpeechVoiceCustomLogger != NULL);
    return mSpeechVoiceCustomLogger;
}

SpeechVoiceCustomLogger::SpeechVoiceCustomLogger() {
    mStarting = false;
    mEnable = false;
    mVCLEnable = false;

    mRecordThread = 0;

    mDumpFile = NULL;
    memset((void *)&mRingBuf, 0, sizeof(RingBuf));
}

SpeechVoiceCustomLogger::~SpeechVoiceCustomLogger() {
    Close();
}


bool SpeechVoiceCustomLogger::UpdateVCLSwitch() {
#if defined(FORCE_ENABLE_VCL)
    mVCLEnable =  true;
#else
    // VCL log recycle mechanism
    char property_value[PROPERTY_VALUE_MAX];
    property_get(PROPERTY_KEY_VCL_ON, property_value, "0"); //"0": default off
    mVCLEnable = (property_value[0] == '0') ? false : true;
#endif
    ALOGD("-%s(), mVCLEnable=%d", __FUNCTION__, mVCLEnable);
    return mVCLEnable;
}

status_t SpeechVoiceCustomLogger::Open() {
    AL_LOCK(mMutex);

    ALOGD("+%s()", __FUNCTION__);

    ASSERT(mEnable == false);

    int ret = acquire_wake_lock(PARTIAL_WAKE_LOCK, VCL_RECORD_WAKELOCK_NAME);
    ALOGD("%s(), acquire_wake_lock: %s, return %d.", __FUNCTION__, VCL_RECORD_WAKELOCK_NAME, ret);

    // create another thread to avoid fwrite() block CCCI read thread
    pthread_create(&mRecordThread, NULL, DumpVCLRecordDataThread, (void *)this);

    AL_UNLOCK(mMutex);
    mEnable = true;

    ALOGD("-%s(), mEnable=%d ", __FUNCTION__, mEnable);
    return NO_ERROR;
}

status_t SpeechVoiceCustomLogger::Close() {
    AL_LOCK(mMutex);

    ALOGD("+%s()", __FUNCTION__);

    if (mEnable == false) {
        ALOGW("-%s(), mEnable == false, return!!", __FUNCTION__);
        AL_UNLOCK(mMutex);
        return INVALID_OPERATION;
    }

    mStarting = false;

    int ret = 0;


    // VCL log recycle mechanism
    char property_value[PROPERTY_VALUE_MAX];
    property_get(PROPERTY_KEY_VCL_RECYCLE_ON, property_value, "1"); //"1": default on
    const bool vcl_recycle_on = (property_value[0] == '0') ? false : true;

    if (vcl_recycle_on == true) {
        memset(gVCLFileList, 0, sizeof(gVCLFileList));
        gNumOfVCLFiles = 0;
        gTotalSizeOfVCLFiles = 0;

        ret = ftw(kFolderOfVCLFile, GetVCLFileList, FTW_D);
        ASSERT(ret == 0);

        // Sort file name
        qsort(gVCLFileList, gNumOfVCLFiles, sizeof(vcl_file_info_t), CompareVCLFileName);

        // Remove VCL files
        uint32_t index_vcl_file_list = 0;
        while (gNumOfVCLFiles > kMinKeepNumOfVCLFiles && gTotalSizeOfVCLFiles > kMaxSizeOfAllVCLFiles) {
            ALOGD("%s(), gNumOfVCLFiles = %lu, gTotalSizeOfVCLFiles = %lu", __FUNCTION__, gNumOfVCLFiles, gTotalSizeOfVCLFiles);

            ALOGD("%s(), remove(%s), size = %lu", __FUNCTION__, gVCLFileList[index_vcl_file_list].path, gVCLFileList[index_vcl_file_list].size);
            ret = remove(gVCLFileList[index_vcl_file_list].path);
            ASSERT(ret == 0);

            gNumOfVCLFiles--;
            gTotalSizeOfVCLFiles -= gVCLFileList[index_vcl_file_list].size;

            index_vcl_file_list++;
        }
    }

    // release wake lock
    ret = release_wake_lock(VCL_RECORD_WAKELOCK_NAME);
    ALOGD("%s(), release_wake_lock:%s return %d.", __FUNCTION__, VCL_RECORD_WAKELOCK_NAME, ret);
    mEnable = false;
    AL_SIGNAL(mMutex); // wake up thread to exit
    AL_UNLOCK(mMutex);

    ALOGD("-%s()", __FUNCTION__);
    return NO_ERROR;
}

status_t SpeechVoiceCustomLogger::OpenFile() {
    char vcl_file_path[kMaxSizeOfVCLFileName];
    memset((void *)vcl_file_path, 0, kMaxSizeOfVCLFileName);

    time_t rawtime;
    time(&rawtime);
    struct tm *timeinfo = localtime(&rawtime);
    audio_strncpy(vcl_file_path, kPrefixOfVCLFileName, kMaxSizeOfVCLFileName);
    strftime(vcl_file_path + kSizeOfPrefixOfVCLFileName, kMaxSizeOfVCLFileName - kSizeOfPrefixOfVCLFileName - 1, "_%Y_%m_%d_%H%M%S.pcm", timeinfo);

    ALOGD("%s(), vcl_file_path: \"%s\"", __FUNCTION__, vcl_file_path);

    // check vcl_file_path is valid
    int ret = AudiocheckAndCreateDirectory(vcl_file_path);
    if (ret < 0) {
        ALOGE("%s(), AudiocheckAndCreateDirectory(%s) fail!!", __FUNCTION__, vcl_file_path);
        return UNKNOWN_ERROR;
    }

    // open VCL file
    mDumpFile = fopen(vcl_file_path, "wb");
    if (mDumpFile == NULL) {
        ALOGE("%s(), fopen(%s) fail!!", __FUNCTION__, vcl_file_path);
        return UNKNOWN_ERROR;
    }

    return NO_ERROR;
}

uint16_t SpeechVoiceCustomLogger::CopyBufferToVCL(RingBuf ul_ring_buf) {
    AL_LOCK(mMutex);

    if (mStarting == false) {
        ALOGD("%s(), mStarting == false, return.", __FUNCTION__);
        AL_SIGNAL(mMutex); // wake up thread to exit
        AL_UNLOCK(mMutex);
        return 0;
    }

    // get free space of internal input buffer
    uint16_t free_space = RingBuf_getFreeSpace(&mRingBuf);
    SLOGV("%s(), mRingBuf remain data count: %u, free_space: %u", __FUNCTION__, RingBuf_getDataCount(&mRingBuf), free_space);

    // get data count in share buffer
    uint16_t ul_data_count = RingBuf_getDataCount(&ul_ring_buf);
    SLOGV("%s(), ul_ring_buf data count: %u", __FUNCTION__, ul_data_count);

    // check free space for internal input buffer
    uint16_t copy_data_count = 0;
    if (ul_data_count <= free_space) {
        copy_data_count = ul_data_count;
    } else {
        ALOGE("%s(), ul_data_count(%u) > free_space(%u)", __FUNCTION__, ul_data_count, free_space);
        copy_data_count = free_space;
    }

    // copy data from modem share buffer to internal input buffer
    if (copy_data_count > 0) {
        SLOGV("%s(), copy_data_count: %u", __FUNCTION__, copy_data_count);
        RingBuf_copyFromRingBuf(&mRingBuf, &ul_ring_buf, copy_data_count);
    }

    // signal
    AL_SIGNAL(mMutex); // wake up thread to fwrite data.
    AL_UNLOCK(mMutex);

    return copy_data_count;
}

void *SpeechVoiceCustomLogger::DumpVCLRecordDataThread(void *arg) {
    pthread_detach(pthread_self());

    // Adjust thread priority
    prctl(PR_SET_NAME, (unsigned long)__FUNCTION__, 0, 0, 0);
    setpriority(PRIO_PROCESS, 0, ANDROID_PRIORITY_AUDIO);

    ALOGD("%s(), pid: %d, tid: %d", __FUNCTION__, getpid(), gettid());

    SpeechVoiceCustomLogger *pSpeechVoiceCustomLogger = (SpeechVoiceCustomLogger *)arg;
    RingBuf &ring_buf = pSpeechVoiceCustomLogger->mRingBuf;

    // open file
    if (pSpeechVoiceCustomLogger->OpenFile() != NO_ERROR) {
        pSpeechVoiceCustomLogger->mEnable = false;
        pthread_exit(NULL);
        return 0;
    }
    // Internal Input Buffer Initialization
    pSpeechVoiceCustomLogger->mRingBuf.pBufBase = new char[kReadBufferSize];
    pSpeechVoiceCustomLogger->mRingBuf.bufLen   = kReadBufferSize;
    pSpeechVoiceCustomLogger->mRingBuf.pRead    = pSpeechVoiceCustomLogger->mRingBuf.pBufBase;
    pSpeechVoiceCustomLogger->mRingBuf.pWrite   = pSpeechVoiceCustomLogger->mRingBuf.pBufBase;

    ASSERT(pSpeechVoiceCustomLogger->mRingBuf.pBufBase != NULL);
    memset(pSpeechVoiceCustomLogger->mRingBuf.pBufBase, 0, pSpeechVoiceCustomLogger->mRingBuf.bufLen);

    pSpeechVoiceCustomLogger->mStarting = true;

    while (1) {
        // lock & wait data
        AL_LOCK(pSpeechVoiceCustomLogger->mMutex);
        if (AL_WAIT_MS(pSpeechVoiceCustomLogger->mMutex, kCondWaitTimeoutMsec) != 0) {
            ALOGW("%s(), waitRelative fail", __FUNCTION__);
        }

        // make sure VCL is still recording after conditional wait
        if (pSpeechVoiceCustomLogger->mEnable == false) {

            // close file
            if (pSpeechVoiceCustomLogger->mDumpFile != NULL) {
                fflush(pSpeechVoiceCustomLogger->mDumpFile);
                fclose(pSpeechVoiceCustomLogger->mDumpFile);
                pSpeechVoiceCustomLogger->mDumpFile = NULL;
            }

            // release local ring buffer
            if (pSpeechVoiceCustomLogger->mRingBuf.pBufBase != NULL) {
                delete []pSpeechVoiceCustomLogger->mRingBuf.pBufBase;
                pSpeechVoiceCustomLogger->mRingBuf.pBufBase = NULL;
                pSpeechVoiceCustomLogger->mRingBuf.pRead    = NULL;
                pSpeechVoiceCustomLogger->mRingBuf.pWrite   = NULL;
                pSpeechVoiceCustomLogger->mRingBuf.bufLen   = 0;
            }

            ALOGD("%s(), pid: %d, tid: %d, mEnable == false, break.", __FUNCTION__, getpid(), gettid());
            AL_UNLOCK(pSpeechVoiceCustomLogger->mMutex);
            break;
        }

        // write data to sd card
        const uint16_t data_count = RingBuf_getDataCount(&ring_buf);
        uint16_t write_bytes = 0;

        if (data_count > 0) {
            const char *end = ring_buf.pBufBase + ring_buf.bufLen;
            if (ring_buf.pRead <= ring_buf.pWrite) {
                write_bytes += fwrite((void *)ring_buf.pRead, sizeof(char), data_count, pSpeechVoiceCustomLogger->mDumpFile);
            } else {
                int r2e = end - ring_buf.pRead;
                write_bytes += fwrite((void *)ring_buf.pRead, sizeof(char), r2e, pSpeechVoiceCustomLogger->mDumpFile);
                write_bytes += fwrite((void *)ring_buf.pBufBase, sizeof(char), data_count - r2e, pSpeechVoiceCustomLogger->mDumpFile);
            }

            ring_buf.pRead += write_bytes;
            if (ring_buf.pRead >= end) { ring_buf.pRead -= ring_buf.bufLen; }

            SLOGV("%s(), data_count: %u, write_bytes: %u", __FUNCTION__, data_count, write_bytes);
        }

        if (write_bytes != data_count) {
            ALOGE("%s(), write_bytes(%d) != data_count(%d), SD Card might be full!!", __FUNCTION__, write_bytes, data_count);
        }

        // unlock
        AL_UNLOCK(pSpeechVoiceCustomLogger->mMutex);
    }

    pthread_exit(NULL);
    return 0;
}

};

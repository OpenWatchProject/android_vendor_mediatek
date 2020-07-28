#ifdef LOG_TAG
#undef LOG_TAG
#endif
#define LOG_TAG "SpeechVMRecorder"
#include "SpeechVMRecorder.h"

#include <sys/prctl.h>
#include <sys/time.h>
#include <sys/resource.h>

#include <cutils/properties.h>

#include <utils/threads.h>

#include <ftw.h>

#include <hardware_legacy/power.h>

#include <SpeechType.h>

#include <audio_memory_control.h>
#include <audio_time.h>

#include <SpeechUtility.h>

#include <SpeechDriverFactory.h>
#include <SpeechDriverInterface.h>

#if !defined(MTK_AUDIO_HIERARCHICAL_PARAM_SUPPORT)
#include <AudioCustParamClient.h>
#include <SpeechEnhancementController.h>
#endif
//#define FORCE_ENABLE_VM
//#define VM_FILENAME_ONLY_USE_VM0_TO_VM7

namespace android {

/*==============================================================================
 *                     Property keys
 *============================================================================*/
const char *PROPERTY_KEY_VM_INDEX      = "persist.vendor.audiohal.vm_index";
const char *PROPERTY_KEY_VM_RECYCLE_ON = "persist.vendor.audiohal.vm_recycle_on";
const char *PROPERTY_KEY_VM_CFG = "persist.vendor.audiohal.vm_cfg";


/*==============================================================================
 *                     Constant
 *============================================================================*/
// 100 ms (modem local buf: 10k, and EPL has 2304 byte for each frame (20 ms))
static const uint32_t kCondWaitTimeoutMsec = 100;

static const uint32_t kReadBufferSize = 0x20000;   // 128 k
static const uint32_t kSdCardBufferSize = 0x4000;   // 16 k

static uint32_t gThreadOpenIndex = 0;
static AudioLock gThreadOpenIndexLock;

/*==============================================================================
 *                     VM File Recycle
 *============================================================================*/
static const uint32_t kMaxNumOfVMFiles = 100;

static const uint32_t kMinKeepNumOfVMFiles = 16;        // keep at least 16 files which will not be removed
static const uint32_t kMaxSizeOfAllVMFiles = 209715200; // Total > 200 M

static const char     kFolderOfVMFile[]     = "/data/vendor/audiohal/audio_dump/";
static const char     kPrefixOfVMFileName[] = "/data/vendor/audiohal/audio_dump/VMLog";
static const uint32_t kSizeOfPrefixOfVMFileName = sizeof(kPrefixOfVMFileName) - 1;
static const uint32_t kMaxSizeOfVMFileName = 128;

typedef struct {
    char     path[kMaxSizeOfVMFileName];
    uint32_t size;
} vm_file_info_t;

static vm_file_info_t gVMFileList[kMaxNumOfVMFiles];
static uint32_t       gNumOfVMFiles = 0;
static uint32_t       gTotalSizeOfVMFiles; // Total size of VM files in SD card

static int getVMFileList(const char *path, const struct stat *sb, int typeflag __unused) {
    if (strncmp(path, kPrefixOfVMFileName, kSizeOfPrefixOfVMFileName) != 0) {
        return 0;
    }

    if (gNumOfVMFiles >= kMaxNumOfVMFiles) {
        return 0;
    }

    // path
    audio_strncpy(gVMFileList[gNumOfVMFiles].path, path, kMaxSizeOfVMFileName);

    // size
    gVMFileList[gNumOfVMFiles].size = sb->st_size;
    gTotalSizeOfVMFiles += sb->st_size;

    // increase index
    gNumOfVMFiles++;

    return 0; // To tell ftw() to continue
}

static int compareVMFileName(const void *a, const void *b) {
    return strcmp(((vm_file_info_t *)a)->path,
                  ((vm_file_info_t *)b)->path);
}


/*==============================================================================
 *                     Implementation
 *============================================================================*/

SpeechVMRecorder *SpeechVMRecorder::mSpeechVMRecorder = NULL;
SpeechVMRecorder *SpeechVMRecorder::getInstance() {
    static Mutex mGetInstanceLock;
    Mutex::Autolock _l(mGetInstanceLock);

    if (mSpeechVMRecorder == NULL) {
        mSpeechVMRecorder = new SpeechVMRecorder();
    }
    ASSERT(mSpeechVMRecorder != NULL);
    return mSpeechVMRecorder;
}

SpeechVMRecorder::SpeechVMRecorder() {
    mVMConfig = SPEECH_VM_DISABLE;
    mIsDumpThreadStart = false;
    mIsVmEnable = false;

    memset((void *)&mRingBuf, 0, sizeof(RingBuf));
    mIsCtmDebugStart = false;
    pCtmDumpFileUlIn = NULL;
    pCtmDumpFileDlIn = NULL;
    pCtmDumpFileUlOut = NULL;
    pCtmDumpFileDlOut = NULL;
    mRecordThread = 0;
    mOpenIndex = 0;
#if !defined(MTK_AUDIO_HIERARCHICAL_PARAM_SUPPORT)
    AUDIO_CUSTOM_PARAM_STRUCT sphParamNB;
    AudioCustParamClient::GetInstance()->GetNBSpeechParamFromNVRam(&sphParamNB);
    char property_value[PROPERTY_VALUE_MAX];
    snprintf(property_value, sizeof(property_value), "%d", (uint8_t) sphParamNB.uAutoVM);
    property_set(PROPERTY_KEY_VM_CFG, property_value);
#endif
    getVmConfig();
    ALOGD("%s(), mVMConfig(%d)", __FUNCTION__, mVMConfig);
}

SpeechVMRecorder::~SpeechVMRecorder() {
    close();
}

/**
 * VM Config
 */
int SpeechVMRecorder::configVm(const AUDIO_CUSTOM_PARAM_STRUCT *sphParamNB) {
    ALOGD("%s(), uAutoVM = 0x%x, debug_info[0] = %u, speech_common_para[0] = %u", __FUNCTION__,
          sphParamNB->uAutoVM, sphParamNB->debug_info[0], sphParamNB->speech_common_para[0]);
    uint8_t vmConfig = (uint8_t)sphParamNB->uAutoVM;
    return configVm(vmConfig);
}

int SpeechVMRecorder::configVm(const uint8_t vmConfig) {
    if (vmConfig < 0 || vmConfig >= NUM_SPEECH_VM_TYPE) {
        ALOGW("%s(), not support type %d", __FUNCTION__, vmConfig);
        return -EINVAL;
    }
    if (mVMConfig == (SpeechVmType) vmConfig) {
        ALOGW("%s(), mVMConfig(%d) == vmConfig(%d), return.", __FUNCTION__, mVMConfig, vmConfig);
        return 0;
    }

    char property_value[PROPERTY_VALUE_MAX];
    snprintf(property_value, sizeof(property_value), "%d", vmConfig);
    property_set(PROPERTY_KEY_VM_CFG, property_value);

    mVMConfig = (SpeechVmType) vmConfig;

    triggerSpeechVm();
    ALOGD("%s(), mVMConfig = %d", __FUNCTION__, mVMConfig);
    return 0;
}

int SpeechVMRecorder::configVmEpl(const bool isEpl) {
    ALOGD("%s(), isEpl=%d,", __FUNCTION__, isEpl);
#if !defined(MTK_AUDIO_HIERARCHICAL_PARAM_SUPPORT)
    AUDIO_CUSTOM_PARAM_STRUCT eSphParamNB;
    AudioCustParamClient::GetInstance()->GetNBSpeechParamFromNVRam(&eSphParamNB);
    if (isEpl) { // EPL
        eSphParamNB.debug_info[0] = 3;
        if (eSphParamNB.speech_common_para[0] == 0) { // if not assign EPL debug type yet, set a default one
            eSphParamNB.speech_common_para[0] = 6;
        }
    } else {// normal VM
        eSphParamNB.debug_info[0] = 0;
    }
    AudioCustParamClient::GetInstance()->SetNBSpeechParamToNVRam(&eSphParamNB);
    SpeechEnhancementController::GetInstance()->SetNBSpeechParametersToAllModem(&eSphParamNB);
#endif

    return 0;
}

SpeechVmType SpeechVMRecorder::getVmConfig() {
#if defined(FORCE_ENABLE_VM)
    mVMConfig = SPEECH_VM_SPEECH;
#else
    // VM Log system property
    char property_value[PROPERTY_VALUE_MAX];
    property_get(PROPERTY_KEY_VM_CFG, property_value, "0"); //"0": default off
    if (property_value[0] == '1') {
        mVMConfig = SPEECH_VM_SPEECH;
    } else if (property_value[0] == '2') {
        mVMConfig = SPEECH_VM_CTM4WAY;
    } else {
        mVMConfig = SPEECH_VM_DISABLE;
    }
#endif
    return mVMConfig;
}

void SpeechVMRecorder::triggerSpeechVm() {
    ALOGD("%s(), mVMConfig = %d", __FUNCTION__, mVMConfig);
    SpeechDriverInterface *pSpeechDriver = SpeechDriverFactory::GetInstance()->GetSpeechDriver();
    const bool isSpeechOn = pSpeechDriver->GetApSideModemStatus(SPEECH_STATUS_MASK);
    const bool isVoiceRecordOn    = pSpeechDriver->GetApSideModemStatus(RECORD_STATUS_MASK);

    if (isSpeechOn) {
        if (getVmConfig() == SPEECH_VM_SPEECH && mIsVmEnable == false) {
            // turn off normal phone record
            if (isVoiceRecordOn == true) {
                ALOGW("%s(), Turn off normal phone recording!!", __FUNCTION__);
                ALOGW("%s(), The following record file will be silence until VM/EPL is closed.", __FUNCTION__);
            }

            ALOGD("%s(), Open VM/EPL record", __FUNCTION__);
            open();
        } else if (getVmConfig() != SPEECH_VM_SPEECH && mIsVmEnable == true) {
            ALOGD("%s(), Close VM/EPL record", __FUNCTION__);
            ALOGD("%s(), Able to continue to do phone record.", __FUNCTION__);
            close();
        }
    }
}

/**
 * VM for vocie call
 */
status_t SpeechVMRecorder::open() {
    AL_LOCK(mMutex);

    // create another thread to avoid fwrite() block CCCI read thread
    ASSERT(mIsVmEnable == false);
    mIsVmEnable = true;
    mOpenIndex++;
    ALOGD("%s(), mOpenIndex: %u", __FUNCTION__, mOpenIndex);
    pthread_create(&mRecordThread, NULL, dumpVMRecordDataThread, (void *)this);

    AL_UNLOCK(mMutex);

    return NO_ERROR;
}

status_t SpeechVMRecorder::close() {
    ALOGD("+%s()", __FUNCTION__);

    AL_LOCK(mThreadStartMutex);
    mIsDumpThreadStart = false;
    AL_UNLOCK(mThreadStartMutex);

    AL_LOCK(mMutex);
    if (mIsVmEnable == false) {
        ALOGW("-%s(), mIsVmEnable == false, return!!", __FUNCTION__);
        AL_SIGNAL(mMutex); // wake up thread to exit
        AL_UNLOCK(mMutex);
        return INVALID_OPERATION;
    }
    // turn off record function
    SpeechDriverInterface *pSpeechDriver = SpeechDriverFactory::GetInstance()->GetSpeechDriver();
    if (pSpeechDriver->GetApSideModemStatus(VM_RECORD_STATUS_MASK) == true) {
        SpeechDriverFactory::GetInstance()->GetSpeechDriver()->VoiceMemoRecordOff();
    }

    mIsVmEnable = false;
    AL_SIGNAL(mMutex); // wake up thread to exit
    AL_UNLOCK(mMutex);

    ALOGD("-%s()", __FUNCTION__);
    return NO_ERROR;
}

uint16_t SpeechVMRecorder::getVmDataFromModem(RingBuf ul_ring_buf) {
    struct timespec ts_start;
    struct timespec ts_stop;

    uint64_t time_diff_lock = 0;
    uint64_t time_diff_copy = 0;
    uint64_t time_diff_unlock = 0;


    audio_get_timespec_monotonic(&ts_start);

    AL_LOCK(mThreadStartMutex);

    if (mIsDumpThreadStart == false) {
        ALOGD("%s(), mIsDumpThreadStart == false, return.", __FUNCTION__);
        AL_UNLOCK(mThreadStartMutex);
        return 0;
    }

    AL_LOCK(mMutex);

    if (mRingBuf.pBufBase == NULL) {
        ALOGD("%s(), mRingBuf.pBufBase == NULL, return.", __FUNCTION__);
        AL_UNLOCK(mMutex);
        AL_UNLOCK(mThreadStartMutex);
        return 0;
    }

    audio_get_timespec_monotonic(&ts_stop);
    time_diff_lock = get_time_diff_ms(&ts_start, &ts_stop);

    // get free space of internal input buffer
    uint16_t free_space = RingBuf_getFreeSpace(&mRingBuf);
    SLOGV("%s(), mRingBuf remain data count: %u, free_space: %u",
          __FUNCTION__, RingBuf_getDataCount(&mRingBuf), free_space);

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

    audio_get_timespec_monotonic(&ts_stop);
    time_diff_copy = get_time_diff_ms(&ts_start, &ts_stop);

    // signal
    AL_SIGNAL(mMutex); // wake up thread to fwrite data.
    AL_UNLOCK(mMutex);
    AL_UNLOCK(mThreadStartMutex);

    audio_get_timespec_monotonic(&ts_stop);
    time_diff_unlock = get_time_diff_ms(&ts_start, &ts_stop);


    if (time_diff_unlock > 10) {
        ALOGW("%s(), time too long, lock %ju, copy %ju, unlock %ju", __FUNCTION__,
              time_diff_lock,
              time_diff_copy - time_diff_lock,
              time_diff_unlock - time_diff_copy);
    }

    return copy_data_count;
}

void *SpeechVMRecorder::dumpVMRecordDataThread(void *arg) {
    pthread_detach(pthread_self());

    char thread_name[128] = {0};
    CONFIG_THREAD(thread_name, ANDROID_PRIORITY_AUDIO);

    AL_LOCK(gThreadOpenIndexLock);
    gThreadOpenIndex++;
    const uint32_t local_open_index = gThreadOpenIndex;
    AL_UNLOCK(gThreadOpenIndexLock);

    SpeechVMRecorder *pSpeechVMRecorder = (SpeechVMRecorder *)arg;
    if (pSpeechVMRecorder == NULL) {
        ALOGW("%s(), pSpeechVMRecorder == NULL!!", __FUNCTION__);
        pthread_exit(NULL);
        return NULL;
    }

    FILE *dumpFile = NULL;

    char *local_buf = NULL;
    char *sd_card_buf = NULL;

    uint32_t data_count = 0;
    uint32_t free_space = 0;
    uint32_t write_bytes = 0;


    // open file
    dumpFile = pSpeechVMRecorder->openFile();
    if (dumpFile == NULL) {
        ALOGE("%s(), openFile() fail!! Return.", __FUNCTION__);
        pthread_exit(NULL);
        return NULL;
    }

    AL_LOCK(pSpeechVMRecorder->mThreadStartMutex);
    AL_LOCK(pSpeechVMRecorder->mMutex);

    // run SpeechVMRecorder::Close() before dumpVMRecordDataThread launched!!
    if (pSpeechVMRecorder->mIsVmEnable == false ||
        local_open_index != pSpeechVMRecorder->mOpenIndex) {
        ALOGW("%s(), mIsVmEnable: %d or index %d != %d!! Return.", __FUNCTION__,
              pSpeechVMRecorder->mIsVmEnable,
              local_open_index, pSpeechVMRecorder->mOpenIndex);
        AL_UNLOCK(pSpeechVMRecorder->mMutex);
        AL_UNLOCK(pSpeechVMRecorder->mThreadStartMutex);
        // close file
        if (dumpFile != NULL) {
            fclose(dumpFile);
            dumpFile = NULL;
        }
        pthread_exit(NULL);
        return NULL;
    }

    // open modem record function
    SpeechDriverInterface *pSpeechDriver = SpeechDriverFactory::GetInstance()->GetSpeechDriver();
    status_t retval = pSpeechDriver->VoiceMemoRecordOn();
    if (retval != NO_ERROR && retval != -ETIMEDOUT) {
        ALOGE("%s(), VoiceMemoRecordOn() fail!! Return.", __FUNCTION__);
        pSpeechVMRecorder->mIsVmEnable = false;
        pSpeechDriver->VoiceMemoRecordOff();
        AL_UNLOCK(pSpeechVMRecorder->mMutex);
        AL_UNLOCK(pSpeechVMRecorder->mThreadStartMutex);
        // close file
        if (dumpFile != NULL) {
            fclose(dumpFile);
            dumpFile = NULL;
        }
        pthread_exit(NULL);
        return NULL;
    }

    // Internal Input Buffer Initialization
    AUDIO_ALLOC_CHAR_BUFFER(local_buf, kReadBufferSize);
    pSpeechVMRecorder->mRingBuf.pBufBase = local_buf;
    pSpeechVMRecorder->mRingBuf.bufLen   = kReadBufferSize;
    pSpeechVMRecorder->mRingBuf.pRead    = pSpeechVMRecorder->mRingBuf.pBufBase;
    pSpeechVMRecorder->mRingBuf.pWrite   = pSpeechVMRecorder->mRingBuf.pBufBase;

    pSpeechVMRecorder->mIsDumpThreadStart = true;

    AL_UNLOCK(pSpeechVMRecorder->mMutex);
    AL_UNLOCK(pSpeechVMRecorder->mThreadStartMutex);

    AUDIO_ALLOC_CHAR_BUFFER(sd_card_buf, kSdCardBufferSize);

    ALOGD("%s(), pid: %d, tid: %d, VM start, local_open_index: %u",
          __FUNCTION__, getpid(), gettid(), local_open_index);

    while (1) {
        // lock & wait data
        AL_LOCK(pSpeechVMRecorder->mMutex);

        // make sure VM is still recording after fwrite
        if (pSpeechVMRecorder->mIsVmEnable == true &&
            local_open_index == pSpeechVMRecorder->mOpenIndex) {
            data_count = (uint32_t)RingBuf_getDataCount(&pSpeechVMRecorder->mRingBuf);
            if (data_count == 0) {
                if (AL_WAIT_MS(pSpeechVMRecorder->mMutex, kCondWaitTimeoutMsec) != 0) {
                    ALOGW("%s(), wait fail", __FUNCTION__);
                }
                data_count = (uint32_t)RingBuf_getDataCount(&pSpeechVMRecorder->mRingBuf);
            }
        }

        // make sure VM is still recording after conditional wait
        if (pSpeechVMRecorder->mIsVmEnable == false ||
            local_open_index != pSpeechVMRecorder->mOpenIndex) {
            ALOGD("%s(), pid: %d, tid: %d, VM stop, mIsVmEnable: %d or index %d != %d",
                  __FUNCTION__, getpid(), gettid(), pSpeechVMRecorder->mIsVmEnable,
                  local_open_index, pSpeechVMRecorder->mOpenIndex);

            // reset ring buffer
            if (pSpeechVMRecorder->mRingBuf.pBufBase == local_buf) {
                memset((void *)&pSpeechVMRecorder->mRingBuf, 0, sizeof(RingBuf));
            }

            AL_UNLOCK(pSpeechVMRecorder->mMutex);
            break;
        }

        if (data_count == 0) {
            AUD_LOG_W("%s(), data_count == 0, continue", __FUNCTION__);
            AL_UNLOCK(pSpeechVMRecorder->mMutex);
            usleep(100);
            continue;
        }

        if (data_count > kSdCardBufferSize) {
            AUD_LOG_W("%s(), data_count %u > kSdCardBufferSize %u!!", __FUNCTION__, data_count, kSdCardBufferSize);
            data_count = kSdCardBufferSize;
        }
        RingBuf_copyToLinear(sd_card_buf, &pSpeechVMRecorder->mRingBuf, data_count);
        AL_UNLOCK(pSpeechVMRecorder->mMutex);

        // write data to sd card
        write_bytes = fwrite((void *)sd_card_buf, sizeof(char), data_count, dumpFile);
        if (write_bytes != data_count) {
            ALOGE("%s(), write_bytes(%u) != data_count(%u), SD Card might be full!!",
                  __FUNCTION__, write_bytes, data_count);
        }
        SLOGV("data_count: %u, write_bytes: %u", data_count, write_bytes);
    }

    // close file
    if (dumpFile != NULL) {
        fflush(dumpFile);
        fclose(dumpFile);
        dumpFile = NULL;
    }

    AUDIO_FREE_POINTER(sd_card_buf);
    AUDIO_FREE_POINTER(local_buf);

    // VM log recycle mechanism
    char property_value[PROPERTY_VALUE_MAX];
    property_get(PROPERTY_KEY_VM_RECYCLE_ON, property_value, "1"); //"1": default on
    const bool vm_recycle_on = (property_value[0] == '0') ? false : true;

    int ret = 0;

    if (vm_recycle_on == true) {
        // Get gVMFileList, gNumOfVMFiles, gTotalSizeOfVMFiles
        memset(gVMFileList, 0, sizeof(gVMFileList));
        gNumOfVMFiles = 0;
        gTotalSizeOfVMFiles = 0;

        ret = ftw(kFolderOfVMFile, getVMFileList, FTW_D);
        ASSERT(ret == 0);

        // Sort file name
        qsort(gVMFileList, gNumOfVMFiles, sizeof(vm_file_info_t), compareVMFileName);
        //for(int i = 0; i < gNumOfVMFiles; i++)
        //    ALOGD("%s(), %s, %u", __FUNCTION__, gVMFileList[i].path, gVMFileList[i].size);

        // Remove VM files
        uint32_t index_vm_file_list = 0;
        while (gNumOfVMFiles > kMinKeepNumOfVMFiles && gTotalSizeOfVMFiles > kMaxSizeOfAllVMFiles) {
            ALOGD("%s(), gNumOfVMFiles = %u, gTotalSizeOfVMFiles = %u",
                  __FUNCTION__, gNumOfVMFiles, gTotalSizeOfVMFiles);

            ALOGD("%s(), remove(%s), size = %u",
                  __FUNCTION__, gVMFileList[index_vm_file_list].path, gVMFileList[index_vm_file_list].size);
            ret = remove(gVMFileList[index_vm_file_list].path);

            if (ret == ENOENT) {
                ALOGW("%s(), file remove fail(%d)! No such file or directory.", __FUNCTION__, ret);
            } else if (ret != 0) {
                ALOGE("%s(), file remove fail(%d)! errno(%d) AP Force Assert.", __FUNCTION__, ret, errno);
                ASSERT(ret == 0);
            }
            gNumOfVMFiles--;
            gTotalSizeOfVMFiles -= gVMFileList[index_vm_file_list].size;

            index_vm_file_list++;
        }
    }

    ALOGD("%s terminated", thread_name);
    pthread_exit(NULL);
    return NULL;
}


FILE *SpeechVMRecorder::openFile() {
    char vm_file_path[kMaxSizeOfVMFileName];
    memset((void *)vm_file_path, 0, kMaxSizeOfVMFileName);

#if defined(VM_FILENAME_ONLY_USE_VM0_TO_VM7)
    char property_value[PROPERTY_VALUE_MAX];
    property_get(PROPERTY_KEY_VM_INDEX, property_value, "0");

    uint8_t vm_file_number = atoi(property_value);
    sprintf(vm_file_path, "%s_%u.vm", kPrefixOfVMFileName, vm_file_number++);

    sprintf(property_value, "%u", vm_file_number & 0x7);
    property_set(PROPERTY_KEY_VM_INDEX, property_value);
#else
    time_t rawtime;
    time(&rawtime);
    struct tm *timeinfo = localtime(&rawtime);
    if (timeinfo != NULL) {
        audio_strncpy(vm_file_path, kPrefixOfVMFileName, kMaxSizeOfVMFileName);
        strftime(vm_file_path + kSizeOfPrefixOfVMFileName, kMaxSizeOfVMFileName - kSizeOfPrefixOfVMFileName - 1,
                 "_%Y_%m_%d_%H%M%S.vm", timeinfo);
    } else {
        char property_value[PROPERTY_VALUE_MAX];
        property_get(PROPERTY_KEY_VM_INDEX, property_value, "0");

        uint8_t vm_file_number = atoi(property_value);
        sprintf(vm_file_path, "%s_%u.vm", kPrefixOfVMFileName, vm_file_number++);

        sprintf(property_value, "%u", vm_file_number & 0x7);
        property_set(PROPERTY_KEY_VM_INDEX, property_value);
    }
#endif

    ALOGD("%s(), vm_file_path: \"%s\"", __FUNCTION__, vm_file_path);

    // check vm_file_path is valid
    int ret = AudiocheckAndCreateDirectory(vm_file_path);
    if (ret < 0) {
        ALOGE("%s(), AudiocheckAndCreateDirectory(%s) fail!!", __FUNCTION__, vm_file_path);
        return NULL;
    }

    // open VM file
    FILE *dumpFile = fopen(vm_file_path, "wb");
    if (dumpFile == NULL) {
        ALOGE("%s(), fopen(%s) fail!!", __FUNCTION__, vm_file_path);
        return NULL;
    }

    return dumpFile;
}

/**
 * CTM Debug for TTY
 */
int SpeechVMRecorder::startCtmDebug() {
    ALOGD("%s()", __FUNCTION__);

    if (mIsCtmDebugStart) { return false; }

    const uint8_t kMaxPathLength = 80;
    char ctm_file_path_UlIn[kMaxPathLength];
    char ctm_file_path_DlIn[kMaxPathLength];
    char ctm_file_path_UlOut[kMaxPathLength];
    char ctm_file_path_DlOut[kMaxPathLength];
    memset((void *)ctm_file_path_UlIn, 0, kMaxPathLength);
    memset((void *)ctm_file_path_DlIn, 0, kMaxPathLength);
    memset((void *)ctm_file_path_UlOut, 0, kMaxPathLength);
    memset((void *)ctm_file_path_DlOut, 0, kMaxPathLength);
    time_t rawtime;
    time(&rawtime);
    struct tm *timeinfo = localtime(&rawtime);
    strftime(ctm_file_path_UlIn, kMaxPathLength,
             "/data/vendor/audiohal/audio_dump/%Y_%m_%d_%H%M%S_CtmUlIn.pcm", timeinfo);
    strftime(ctm_file_path_DlIn, kMaxPathLength,
             "/data/vendor/audiohal/audio_dump/%Y_%m_%d_%H%M%S_CtmDlIn.pcm", timeinfo);
    strftime(ctm_file_path_UlOut, kMaxPathLength,
             "/data/vendor/audiohal/audio_dump/%Y_%m_%d_%H%M%S_CtmUlOut.pcm", timeinfo);
    strftime(ctm_file_path_DlOut, kMaxPathLength,
             "/data/vendor/audiohal/audio_dump/%Y_%m_%d_%H%M%S_CtmDlOut.pcm", timeinfo);
    int ret;
    ret = AudiocheckAndCreateDirectory(ctm_file_path_UlIn);
    if (ret < 0) {
        ALOGE("%s(), AudiocheckAndCreateDirectory(%s) fail!!", __FUNCTION__, ctm_file_path_UlIn);
        return UNKNOWN_ERROR;
    }
    ret = AudiocheckAndCreateDirectory(ctm_file_path_DlIn);
    if (ret < 0) {
        ALOGE("%s(), AudiocheckAndCreateDirectory(%s) fail!!", __FUNCTION__, ctm_file_path_DlIn);
        return UNKNOWN_ERROR;
    }
    ret = AudiocheckAndCreateDirectory(ctm_file_path_UlOut);
    if (ret < 0) {
        ALOGE("%s(), AudiocheckAndCreateDirectory(%s) fail!!", __FUNCTION__, ctm_file_path_UlOut);
        return UNKNOWN_ERROR;
    }
    ret = AudiocheckAndCreateDirectory(ctm_file_path_DlOut);
    if (ret < 0) {
        ALOGE("%s(), AudiocheckAndCreateDirectory(%s) fail!!", __FUNCTION__, ctm_file_path_DlOut);
        return UNKNOWN_ERROR;
    }
    pCtmDumpFileUlIn = fopen(ctm_file_path_UlIn, "wb");
    pCtmDumpFileDlIn = fopen(ctm_file_path_DlIn, "wb");
    pCtmDumpFileUlOut = fopen(ctm_file_path_UlOut, "wb");
    pCtmDumpFileDlOut = fopen(ctm_file_path_DlOut, "wb");

    if (pCtmDumpFileUlIn == NULL) { ALOGW("Fail to Open pCtmDumpFileUlIn"); }
    if (pCtmDumpFileDlIn == NULL) { ALOGW("Fail to Open pCtmDumpFileDlIn"); }
    if (pCtmDumpFileUlOut == NULL) { ALOGW("Fail to Open pCtmDumpFileUlOut"); }
    if (pCtmDumpFileDlOut == NULL) { ALOGW("Fail to Open pCtmDumpFileDlOut"); }

    mIsCtmDebugStart = true;

    return true;
}

int SpeechVMRecorder::stopCtmDebug() {
    ALOGD("%s()", __FUNCTION__);

    if (!mIsCtmDebugStart) { return false; }

    mIsCtmDebugStart = false;

    fclose(pCtmDumpFileUlIn);
    fclose(pCtmDumpFileDlIn);
    fclose(pCtmDumpFileUlOut);
    fclose(pCtmDumpFileDlOut);
    return true;
}

void SpeechVMRecorder::getCtmDebugDataFromModem(RingBuf ul_ring_buf, FILE *pFile) {
    int InpBuf_freeSpace = 0;
    int ShareBuf_dataCnt = 0;

    if (mIsCtmDebugStart == false) {
        ALOGW("%s(), mIsCtmDebugStart=%d, return.", __FUNCTION__, mIsCtmDebugStart);
        return;
    }

    // get data count in share buffer
    ShareBuf_dataCnt = RingBuf_getDataCount(&ul_ring_buf);
    if (ShareBuf_dataCnt >= 0) {
        char linear_buffer[ShareBuf_dataCnt];
        char *pM2AShareBufEnd = ul_ring_buf.pBufBase + ul_ring_buf.bufLen;
        if (ul_ring_buf.pRead + ShareBuf_dataCnt <= pM2AShareBufEnd) {
            memcpy(linear_buffer, ul_ring_buf.pRead, ShareBuf_dataCnt);
        } else {
            uint32_t r2e = pM2AShareBufEnd - ul_ring_buf.pRead;
            memcpy(linear_buffer, ul_ring_buf.pRead, r2e);
            memcpy((void *)(linear_buffer + r2e), ul_ring_buf.pBufBase, ShareBuf_dataCnt - r2e);
        }

        fwrite(linear_buffer, sizeof(char), ShareBuf_dataCnt, pFile);
    }
}

};

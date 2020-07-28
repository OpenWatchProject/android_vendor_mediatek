/*******************************************************************************
 *
 * Filename:
 * ---------
 * AudioALSAParamTuner.cpp
 *
 * Project:
 * --------
 *   Android
 *
 * Description:
 * ------------
 *   This file implements the method for  handling param tuning.
 *
 * Author:
 * -------
 *   Donglei Ji (mtk80823)
 *******************************************************************************/

#define MTK_LOG_ENABLE 1
#include <unistd.h>
#include <sched.h>
#include <sys/prctl.h>
#include <stdint.h>
#include <sys/types.h>
#include <sys/resource.h>
#include <log/log.h>
#include <system/audio.h>

#include "AudioALSAParamTuner.h"
#include "AudioCustParamClient.h"

#include "AudioVolumeFactory.h"
//#include "AudioAnalogControlFactory.h"
//#include "AudioDigitalControlFactory.h"
#include "SpeechEnhancementController.h"
#include "SpeechDriverInterface.h"
#if defined(MTK_AUDIO_GAIN_TABLE) && !defined(MTK_NEW_VOL_CONTROL)
#include "AudioMTKVolumeCapability.h"
#endif

#ifdef LOG_TAG
#undef LOG_TAG
#endif

//#define PLAYBUF_SIZE 6400l
#define PLAYBUF_SIZE 16384
#define A2M_SHARED_BUFFER_OFFSET  (1408)
#define WAV_HEADER_SIZE 44

// define in AudioMtkVolumeControler.cpp
#define  AUDIO_BUFFER_HW_GAIN_STEP (13)

#undef WRITE_RECORDDATA_ON_APSIDEDMNR

#define LOG_TAG "AudioALSAParamTuner"
namespace android {
//digital gain map
static const float Volume_Mapping_Step = 256.0f;
uint32_t MapVoiceVolumetoCustom(uint32_t gain) {
    uint32_t mappingGain = 0;
    if (gain > VOICE_VOLUME_MAX) {
        gain = VOICE_VOLUME_MAX;
    }

    float degradeDb = ((float)(VOICE_VOLUME_MAX - gain)) / (float)(VOICE_ONEDB_STEP);
    mappingGain = (uint32)(Volume_Mapping_Step - ((uint32)(degradeDb * 4.0)));
    ALOGD("%s(), gain:%d, mappingGain:%d", __FUNCTION__, gain, mappingGain);

    return mappingGain;
}

const uint16_t digitOnly_quater_dB_tableForSpeech[264] = {
    4096, 3980, 3867, 3757, /* 0   ~ -0.75   dB*/
    3645, 3547, 3446, 3349, /* -1  ~ -1.75   dB*/ // downlink begin (-1db == 3645 == E3D)
    3254, 3161, 3072, 2984, /* -2  ~ -2.75   dB*/
    2900, 2817, 2738, 2660, /* -3  ~ -3.75   dB*/
    2584, 2511, 2440, 2371, /* -4  ~ -4.75   dB*/
    2303, 2238, 2175, 2113, /* -5  ~ -5.75   dB*/
    2053, 1995, 1938, 1883, /* -6  ~ -6.75   dB*/
    1830, 1778, 1727, 1678, /* -7  ~ -7.75   dB*/
    1631, 1584, 1539, 1496, /* -8  ~ -8.75   dB*/
    1453, 1412, 1372, 1333, /* -9  ~ -9.75   dB*/
    1295, 1259, 1223, 1188, /* -10 ~ -10.75  dB*/
    1154, 1122, 1090, 1059, /* -11 ~ -11.75  dB*/
    1029, 1000, 971, 944,   /* -12 ~ -12.75  dB*/
    917, 891, 866, 841,     /* -13 ~ -13.75  dB*/
    817, 794, 772, 750,     /* -14 ~ -14.75  dB*/
    728, 708, 688, 668,     /* -15 ~ -15.75  dB*/
    649, 631, 613, 595,     /* -16 ~ -16.75  dB*/
    579, 562, 546, 531,     /* -17 ~ -17.75  dB*/
    516, 501, 487, 473,     /* -18 ~ -18.75  dB*/
    460, 447, 434, 422,     /* -19 ~ -19.75  dB*/
    410, 398, 387, 376,     /* -20 ~ -20.75  dB*/
    365, 355, 345, 335,     /* -21 ~ -21.75  dB*/
    325, 316, 307, 298,     /* -22 ~ -22.75  dB*/
    290, 282, 274, 266,     /* -23 ~ -23.75  dB*/
    258, 251, 244, 237,     /* -24 ~ -24.75  dB*/
    230, 224, 217, 211,     /* -25 ~ -25.75  dB*/
    205, 199, 194, 188,     /* -26 ~ -26.75  dB*/
    183, 178, 173, 168,     /* -27 ~ -27.75  dB*/
    163, 158, 154, 150,     /* -28 ~ -28.75  dB*/
    145, 141, 137, 133,     /* -29 ~ -29.75  dB*/
    130, 126, 122, 119,     /* -30 ~ -30.75  dB*/
    115, 112, 109, 106,     /* -31 ~ -31.75  dB*/
    103, 100, 97, 94,       /* -32 ~ -32.75  dB*/
    92, 89, 87, 84,         /* -33 ~ -33.75  dB*/
    82, 79, 77, 75,         /* -34 ~ -34.75  dB*/
    73, 71, 69, 67,         /* -35 ~ -35.75  dB*/
    65, 63, 61, 60,         /* -36 ~ -36.75  dB*/
    58, 56, 55, 53,         /* -37 ~ -37.75  dB*/
    52, 50, 49, 47,         /* -38 ~ -38.75  dB*/
    46, 45, 43, 42,         /* -39 ~ -39.75  dB*/
    41, 40, 39, 38,         /* -40 ~ -40.75  dB*/
    37, 35, 34, 33,         /* -41 ~ -41.75  dB*/
    33, 32, 31, 30,         /* -42 ~ -42.75  dB*/
    29, 28, 27, 27,         /* -43 ~ -43.75  dB*/
    26, 25, 24, 24,         /* -44 ~ -44.75  dB*/
    23, 22, 22, 21,         /* -45 ~ -45.75  dB*/
    21, 20, 19, 19,         /* -46 ~ -46.75  dB*/
    18, 18, 17, 17,         /* -47 ~ -47.75  dB*/
    16, 16, 15, 15,         /* -48 ~ -48.75  dB*/
    15, 14, 14, 13,         /* -49 ~ -49.75  dB*/
    13, 13, 12, 12,         /* -50 ~ -50.75  dB*/
    12, 11, 11, 11,         /* -51 ~ -51.75  dB*/
    10, 10, 10, 9,          /* -52 ~ -52.75  dB*/
    9, 9, 9, 8,             /* -53 ~ -53.75  dB*/
    8, 8, 8, 7,             /* -54 ~ -54.75  dB*/
    7, 7, 7, 7,             /* -55 ~ -55.75  dB*/
    6, 6, 6, 6,             /* -56 ~ -56.75  dB*/
    6, 6, 5, 5,             /* -57 ~ -57.75  dB*/
    5, 5, 5, 5,             /* -58 ~ -58.75  dB*/
    5, 4, 4, 4,             /* -59 ~ -59.75  dB*/
    4, 4, 4, 4,             /* -60 ~ -60.75  dB*/
    4, 4, 3, 3,             /* -61 ~ -61.75  dB*/
    3, 3, 3, 3,             /* -62 ~ -62.75  dB*/
    3, 3, 3, 3,             /* -63 ~ -63.75  dB*/
    3, 3, 2, 2,             /* -64 ~ -64.75  dB*/
    2, 2, 2, 2,             /* -65 ~ -65.75  dB*/
};

static void *Play_PCM_With_SpeechEnhance_Routine(void *arg) {

    ALOGD("%s(), Play_PCM_With_SpeechEnhance_Routine in +", __FUNCTION__);
    AudioALSAParamTuner *pAUDParamTuning = (AudioALSAParamTuner *)arg;

    if (pAUDParamTuning == NULL) {
        ALOGE("%s(), Play_PCM_With_SpeechEnhance_Routine pAUDParamTuning = NULL", __FUNCTION__);
        return 0;
    }

    char *tmp = new char[PLAYBUF_SIZE];

    if (tmp == NULL) {
        ALOGE("%s(), Allocate tmp size = %d Fail", __FUNCTION__, PLAYBUF_SIZE);
        ASSERT(0);
        return 0;
    }
#if defined(SPH_SR32K)||defined(SPH_SR48K)
    uint32_t PCM_BUF_SIZE = 1280;
#else
    uint32_t PCM_BUF_SIZE = pAUDParamTuning->m_bWBMode ? 640 : 320;
#endif
    unsigned long sleepTime = ((PLAYBUF_SIZE / PCM_BUF_SIZE) * 20 * 1000) >> 1;

    // open AudioRecord
    pthread_mutex_lock(&pAUDParamTuning->mPPSMutex);

    // Adjust thread priority
    prctl(PR_SET_NAME, (unsigned long)"PlaybackWithSphEnRoutine", 0, 0, 0);
    setpriority(PRIO_PROCESS, 0, ANDROID_PRIORITY_AUDIO);
    //Prepare file pointer
    FILE *pFd = pAUDParamTuning->m_pInputFile;                 //file for input

    // ----start the loop --------
    pAUDParamTuning->m_bPPSThreadExit = false;

    int numOfBytesPlayed = 0;
    int playBufFreeCnt = 0;
    int cntR = 0;

    if (pFd != NULL) {
        cntR = fread(tmp, sizeof(char), WAV_HEADER_SIZE, pFd);
        if (cntR != WAV_HEADER_SIZE) {
            ALOGE("%s(), read  WAV Fail!!! bytes Read(%d) < Header Size(%d)", __FUNCTION__, cntR, WAV_HEADER_SIZE);
        }
        memset(tmp, 0, PLAYBUF_SIZE);
    }
    ALOGD("%s(), pthread_cond_signal(&pAUDParamTuning->mPPSExit_Cond), buffer size=%d, sleepTime=%lu us", __FUNCTION__, PCM_BUF_SIZE, sleepTime);
    pthread_cond_signal(&pAUDParamTuning->mPPSExit_Cond); // wake all thread
    pthread_mutex_unlock(&pAUDParamTuning->mPPSMutex);

    while ((!pAUDParamTuning->m_bPPSThreadExit) && pFd) {
        pthread_mutex_lock(&pAUDParamTuning->mPlayBufMutex);
        playBufFreeCnt = pAUDParamTuning->mPlay2WayInstance->GetFreeBufferCount();
        cntR = fread(tmp, sizeof(char), playBufFreeCnt, pFd);
        pAUDParamTuning->mPlay2WayInstance->Write(tmp, cntR);
        numOfBytesPlayed += cntR;
        ALOGD("%s(), Playback buffer, free:%d, read from :%d, total play:%d", __FUNCTION__, playBufFreeCnt, cntR, numOfBytesPlayed);
        pthread_mutex_unlock(&pAUDParamTuning->mPlayBufMutex);

        if (cntR < playBufFreeCnt) {
            ALOGD("%s(), File reach the end", __FUNCTION__);
            usleep(sleepTime); ////wait to all data is played
            break;
        }
        usleep(sleepTime);
    }
    if (tmp != NULL) {
        // free buffer
        delete[] tmp;
        tmp = NULL;
    }
    if (!pAUDParamTuning->m_bPPSThreadExit) {
        pAUDParamTuning->m_bPPSThreadExit = true;
        pAUDParamTuning->enableModemPlaybackVIASPHPROC(false);
        AudioTasteTuningStruct sRecoveryParam;
        sRecoveryParam.slected_fir_index = 0;
        sRecoveryParam.cmd_type = (unsigned short)AUD_TASTE_STOP;
        sRecoveryParam.wb_mode  = pAUDParamTuning->m_bWBMode;
        sRecoveryParam.phone_mode  = pAUDParamTuning->mMode;//current mode, not support switch mode during audio taste

        pAUDParamTuning->updataOutputFIRCoffes(&sRecoveryParam);
    }

    //exit thread
    ALOGD("%s(), pthread_mutex_lock", __FUNCTION__);
    pthread_mutex_lock(&pAUDParamTuning->mPPSMutex);
    ALOGD("%s(), pthread_cond_signal(&pAUDParamTuning->mPPSExit_Cond)", __FUNCTION__);
    pthread_cond_signal(&pAUDParamTuning->mPPSExit_Cond); // wake all thread
    pthread_mutex_unlock(&pAUDParamTuning->mPPSMutex);
    return 0;
}


#ifdef DMNR_TUNNING_AT_MODEMSIDE
static void *DMNR_Play_Rec_Routine(void *arg) {
    ALOGD("+%s()", __FUNCTION__);
    AudioALSAParamTuner *pDMNRTuning = (AudioALSAParamTuner *)arg;
    if (pDMNRTuning == NULL) {
        ALOGE("%s(), pDMNRTuning = NULL arg = %x", __FUNCTION__, arg);
        return 0;
    }
#if defined(SPH_SR32K)||defined(SPH_SR48K)
    uint32_t PCM_BUF_SIZE = 1280;
#else
    uint32_t PCM_BUF_SIZE = pDMNRTuning->m_bWBMode ? 640 : 320;
#endif
    unsigned long sleepTime = ((PLAYBUF_SIZE / PCM_BUF_SIZE) * 20 * 1000) >> 1;

    pthread_mutex_lock(&pDMNRTuning->mRecPlayMutex);

    // Adjust thread priority
    prctl(PR_SET_NAME, (unsigned long)"DualMicCalibrationRoutine", 0, 0, 0);
    setpriority(PRIO_PROCESS, 0, ANDROID_PRIORITY_AUDIO);

    //Prepare file pointer
    FILE *pInFp = pDMNRTuning->m_pInputFile;      //file for input
    FILE *pOutFp = pDMNRTuning->m_pOutputFile;    //file for input

    // ----start the loop --------
    char *tmp = new char[PLAYBUF_SIZE];
    pDMNRTuning->m_bRecPlayThreadExit = false;
    int cntR = 0;
    int cntW = 0;
    int numOfBytesPlay = 0;
    int numOfBytesRec = 0;

    int playBufFreeCnt = 0;
    int recBufDataCnt = 0;

    ALOGD("%s(), pthread_cond_signal(&pDMNRTuning->mRecPlayExit_Cond)", __FUNCTION__);
    pthread_cond_signal(&pDMNRTuning->mRecPlayExit_Cond); // wake all thread
    pthread_mutex_unlock(&pDMNRTuning->mRecPlayMutex);

    while ((!pDMNRTuning->m_bRecPlayThreadExit) && pOutFp) {
        //handling playback buffer
        pthread_mutex_lock(&pDMNRTuning->mPlayBufMutex);
        if (pInFp) {
            playBufFreeCnt = pDMNRTuning->mPlay2WayInstance->GetFreeBufferCount() - 8;
            cntR = fread(tmp, sizeof(char), playBufFreeCnt, pInFp);
            pDMNRTuning->mPlay2WayInstance->Write(tmp, cntR);
            numOfBytesPlay += cntR;
            ALOGD("%s(), Playback buffer, free:%d, read from :%d, total play:%d, sleepTime=%lld us", __FUNCTION__, playBufFreeCnt, cntR, numOfBytesPlay, sleepTime);
        }
        pthread_mutex_unlock(&pDMNRTuning->mPlayBufMutex);

        // handling record buffer
        pthread_mutex_lock(&pDMNRTuning->mRecBufMutex);
        recBufDataCnt = pDMNRTuning->mRec2WayInstance->GetBufferDataCount();
        pDMNRTuning->mRec2WayInstance->Read(tmp, recBufDataCnt);
        cntW = fwrite((void *)tmp, sizeof(char), recBufDataCnt, pOutFp);
        numOfBytesRec += cntW;
        ALOGV("%s(), Record buffer, available:%d, write to file:%d, total rec:%d", __FUNCTION__, recBufDataCnt, cntW, numOfBytesRec);
        pthread_mutex_unlock(&pDMNRTuning->mRecBufMutex);

        usleep(sleepTime);
    }

    // free buffer
    delete[] tmp;
    tmp = NULL;

    //exit thread
    ALOGD("%s(), pthread_mutex_lock", __FUNCTION__);
    pthread_mutex_lock(&pDMNRTuning->mRecPlayMutex);
    ALOGD("%s(), pthread_cond_signal(&mRecPlayExit_Cond)", __FUNCTION__);
    pthread_cond_signal(&pDMNRTuning->mRecPlayExit_Cond); // wake all thread
    pthread_mutex_unlock(&pDMNRTuning->mRecPlayMutex);

    return 0;
}
#else
static int PCM_decode_data(WAVEHDR *wavHdr,  char *in_buf, int block_size, char *out_buf, int *out_size) {
    int i = 0, j = 0;
    uint16_t *ptr_d;
    uint8_t  *ptr_s;
    int readlen = 0;
    int writelen = 0;

    uint16_t channels = wavHdr->NumChannels;
    uint16_t bits_per_sample = wavHdr->BitsPerSample;

    ptr_s = (uint8_t *)in_buf;
    ptr_d = (uint16_t *)out_buf;
    readlen = block_size;
    *out_size = 0;

    switch (bits_per_sample) {
    case 8:
        if (channels == 2) {
            for (i = 0; i < readlen; i++) {
                *(ptr_d + j) = (uint16_t)(*(ptr_s + i) - 128) << 8;
                j++;
            }
        } else {
            for (i = 0; i < readlen; i++) {
                *(ptr_d + j) = (uint16_t)(*(ptr_s + i) - 128) << 8;
                *(ptr_d + j + 1) =  *(ptr_d + j);
                j += 2;
            }
        }
        writelen = (j << 1);
        break;
    case 16:
        if (channels == 2) {
            for (i = 0; i < readlen; i += 2) {
                *(ptr_d + j) = *(ptr_s + i) + ((uint16_t)(*(ptr_s + i + 1)) << 8);
                j++;
            }
        } else {
            for (i = 0; i < readlen; i += 2) {
                *(ptr_d + j) = *(ptr_s + i) + ((uint16_t)(*(ptr_s + i + 1)) << 8);
                *(ptr_d + j + 1) = *(ptr_d + j);
                j += 2;
            }
        }
        writelen = (j << 1);
        break;
    default:
        ptr_d = (uint16_t *)(out_buf);
        break;
    }
    *out_size = writelen;
    return true;
}
static void PCM_Apply_DigitalDb(char *out_buf, int out_size, int table_index) {
    short *pcmValue = (short *)out_buf;
    for (int i = 0; i < out_size / 2; i++) {
        *pcmValue = *pcmValue * (digitOnly_quater_dB_tableForSpeech[4 * table_index] / 4096.0);
        pcmValue ++;
    }
}
static void *DMNR_Play_Rec_ApSide_Routine(void *arg) {
    ALOGD("+%s()", __FUNCTION__);
    AudioALSAParamTuner *pDMNRTuning = (AudioALSAParamTuner *)arg;
    if (pDMNRTuning == NULL) {
        ALOGE("%s(), pDMNRTuning = NULL", __FUNCTION__);
        return 0;
    }

    pthread_mutex_lock(&pDMNRTuning->mRecPlayMutex);

    // Adjust thread priority
    prctl(PR_SET_NAME, (unsigned long)"DualMicCalibrationAtApSideRoutine", 0, 0, 0);
    setpriority(PRIO_PROCESS, 0, ANDROID_PRIORITY_AUDIO);

    //Prepare file pointer
    FILE *pInFp = pDMNRTuning->m_pInputFile;      //file for input(use audiomtkstreamout to play)
    FILE *pOutFp = pDMNRTuning->m_pOutputFile;    //file for output(use audiomtkstreamin to record)

    // ----start the loop --------
    pDMNRTuning->m_bRecPlayThreadExit = false;

    ALOGD("%s(), pthread_cond_signal(&pDMNRTuning->mRecPlayExit_Cond)", __FUNCTION__);
    pthread_cond_signal(&pDMNRTuning->mRecPlayExit_Cond); // wake all thread
    pthread_mutex_unlock(&pDMNRTuning->mRecPlayMutex);

    AudioMTKStreamOutInterface *streamOutput = NULL;
    AudioMTKStreamInInterface *streamInput = NULL;

    WAVEHDR waveHeader;
    memset(&waveHeader, 0, sizeof(WAVEHDR));

    char *inBuffer = NULL; //for playback
    char *outBuffer = NULL;
    uint32_t readBlockLen = 0;
    uint32_t hwBufferSize = 0;
    int playbackDb_index = 0;

    char readBuffer[1024] = {0};//for record

    if (pInFp) { //open output stream for playback
        //config output format channel= 2 , bits_per_sample=16
        FILE_FORMAT fileType = pDMNRTuning->playbackFileFormat();

        if (fileType == WAVE_FORMAT) {
            fseek(pInFp, 0, SEEK_END);
            size_t fileSize = ftell(pInFp);

            if (fileSize < WAVE_FORMAT) {
                ALOGW("%s(), play file(size = %zu) is broken.", __FUNCTION__, fileSize);
                return 0;
            }

            /* TODO(JH): sometimes, the play file is correct, but cannot read all header data successfully! Retry 5 times */
            size_t readSize = 0;
            int retryCount = 5;
            for (int i = 0; i < retryCount; i++) {
                rewind(pInFp);
                readSize = fread(&waveHeader, 1, WAV_HEADER_SIZE, pInFp);
                if (readSize == WAV_HEADER_SIZE) {
                    break;
                }
            }

            if (WAV_HEADER_SIZE != readSize) {
                ALOGW("%s(), [Dual-Mic] cannot read wave header successfully! (WAV_HEADER_SIZE = %d, read size = %zu, fileSize = %zu, retryCont = %d)", __FUNCTION__, WAV_HEADER_SIZE, readSize, fileSize, retryCount);
                return 0;
            }
        } else if (fileType == UNSUPPORT_FORMAT) {
            ALOGW("%s(), [Dual-Mic] playback file format is not support", __FUNCTION__);
            return 0;
        }
        uint32_t sampleRate = waveHeader.SampleRate;
        uint32_t channels = AUDIO_CHANNEL_OUT_STEREO;
        int format, status;
#if 0
        if (waveHeader.BitsPerSample == 8 || waveHeader.BitsPerSample == 16) {
            format = AUDIO_FORMAT_PCM_16_BIT;
        } else {
            format = AUDIO_FORMAT_PCM_16_BIT;
        }
#else
        format = AUDIO_FORMAT_PCM_16_BIT;
        status = NO_ERROR;
#endif

        //create output stream
        streamOutput = pDMNRTuning->getStreamManager()->openOutputStream(pDMNRTuning->mDMNROutputDevice, &format, &channels, &sampleRate, &status);

        hwBufferSize = streamOutput->bufferSize(); //16k bytes

        if (waveHeader.NumChannels == 1) {
            switch (waveHeader.BitsPerSample) {
            case 8:
                readBlockLen = hwBufferSize >> 2;
                break;
            case 16:
                readBlockLen = hwBufferSize >> 1;
                break;
            default:
                readBlockLen = 0;
                break;
            }
        } else {
            switch (waveHeader.BitsPerSample) {
            case 8:
                readBlockLen = hwBufferSize >> 1;
                break;
            case 16:
                readBlockLen = hwBufferSize;
                break;
            default:
                readBlockLen = 0;
                break;
            }
        }
        inBuffer = new char[readBlockLen];
        outBuffer = new char[hwBufferSize];
        playbackDb_index = pDMNRTuning->getPlaybackDb();
        ALOGD("%s(), readBlockLen = %d, hwBufferSize = %d,playbackDb_index = %d \n", __FUNCTION__, readBlockLen, hwBufferSize, playbackDb_index);
    }

    if (pOutFp) { //open input stream for record
#ifdef MTK_AUDIO_HD_REC_SUPPORT
        AUDIO_HD_RECORD_SCENE_TABLE_STRUCT hdRecordSceneTable;
        AudioCustParamClient::GetInstance()->GetHdRecordSceneTableFromNV(&hdRecordSceneTable);
        if (hdRecordSceneTable.num_voice_rec_scenes > 0) { //value=0;
            int32_t BesRecScene = 1;//1:cts verifier offset
            pDMNRTuning->getSpeechEnhanceInfoInst()->SetBesRecScene(BesRecScene);
        }
#endif
        uint32_t device = AUDIO_DEVICE_IN_BUILTIN_MIC;
        int format = AUDIO_FORMAT_PCM_16_BIT;
        uint32_t channel = AUDIO_CHANNEL_IN_STEREO;
        uint32_t sampleRate = 16000;
        status_t status = 0;
        streamInput = pDMNRTuning->getStreamManager()->openInputStream(device, &format, &channel, &sampleRate, &status, (audio_in_acoustics_t)0);
        android::AudioParameter paramInputSource = android::AudioParameter();
#ifdef MTK_AURISYS_FRAMEWORK_SUPPORT
        paramInputSource.addInt(android::String8(android::AudioParameter::keyInputSource), AUDIO_SOURCE_CUSTOMIZATION3);   // For aurisys, we defined new input source CUS3 for DMNR cal. (CUS1 is ASR)
#else
        paramInputSource.addInt(android::String8(android::AudioParameter::keyInputSource), AUDIO_SOURCE_CUSTOMIZATION1);   // Legacy arch, we using CUSTOMIZATION1 for DMNR cal.
#endif
        streamInput->setParameters(paramInputSource.toString());

        android::AudioParameter paramDeviceIn = android::AudioParameter();
        paramDeviceIn.addInt(android::String8(android::AudioParameter::keyRouting), AUDIO_DEVICE_IN_BUILTIN_MIC);
        streamInput->setParameters(paramDeviceIn.toString());
    }

    while (!pDMNRTuning->m_bRecPlayThreadExit) {
        //handling playback buffer
        pthread_mutex_lock(&pDMNRTuning->mPlayBufMutex);
        if (pInFp && !feof(pInFp)) {
            int readdata = 0, writedata = 0, out_size = 0;
            memset(inBuffer, 0, readBlockLen);
            memset(outBuffer, 0, hwBufferSize);
            if (readBlockLen > 0) {
                readdata = fread(inBuffer, readBlockLen, 1, pInFp);
            }
            PCM_decode_data(&waveHeader, inBuffer, readBlockLen, outBuffer, &out_size);
            PCM_Apply_DigitalDb(outBuffer, out_size, playbackDb_index);
            writedata = streamOutput->write(outBuffer, out_size);
#if 0
            char filename[] = "/data/vendor/audiohal/xxx.pcm";
            FILE *fp = fopen(filename, "ab+");
            fwrite(outBuffer, writedata, 1, fp);
            fclose(fp);
#endif
        }
        pthread_mutex_unlock(&pDMNRTuning->mPlayBufMutex);

        // handling record buffer
        pthread_mutex_lock(&pDMNRTuning->mRecBufMutex);
        if (pOutFp) {
            memset(readBuffer, 0, sizeof(readBuffer));
            int nRead = streamInput->read(readBuffer, 1024);
#ifdef WRITE_RECORDDATA_ON_APSIDEDMNR
            fwrite(readBuffer, 1, nRead, pOutFp);
#endif
        }
        pthread_mutex_unlock(&pDMNRTuning->mRecBufMutex);
    }

    if (pInFp) {
        streamOutput->standbyStreamOut();
        pDMNRTuning->getStreamManager()->closeOutputStream(streamOutput);
        if (inBuffer) {
            delete[] inBuffer;
            inBuffer = NULL;
        }
        if (outBuffer) {
            delete[] outBuffer;
            outBuffer = NULL;
        }
    }

    if (pOutFp) {
        streamInput->standby(); //this will close input  device
        pDMNRTuning->getStreamManager()->closeInputStream(streamInput);
    }

    //exit thread
    pthread_mutex_lock(&pDMNRTuning->mRecPlayMutex);
    ALOGD("%s(), pthread_cond_signal(&mRecPlayExit_Cond)", __FUNCTION__);
    pthread_cond_signal(&pDMNRTuning->mRecPlayExit_Cond); // wake all thread
    pthread_mutex_unlock(&pDMNRTuning->mRecPlayMutex);

    return 0;
}

static void *FIR_Rec_ApSide_Routine(void *arg)
{
    ALOGD("+%s()", __FUNCTION__);
    AudioALSAParamTuner *pFIRRecording = (AudioALSAParamTuner *)arg;
    if (pFIRRecording == NULL)
    {
        ALOGE("%s(), pFIRRecording = NULL", __FUNCTION__);
        return 0;
    }

    pthread_mutex_lock(&pFIRRecording->mRecPlayMutex);

    //Adjust thread priority
    prctl(PR_SET_NAME, (unsigned long)"FIRRecordAtApSideRoutine", 0, 0, 0);
    setpriority(PRIO_PROCESS, 0, ANDROID_PRIORITY_AUDIO);

    //Prepare file pointer
    FILE *pOutFp = pFIRRecording->m_pOutputFile;    //file for output(use audiomtkstreamin to record)

    // ----start the loop --------
    pFIRRecording->m_bRecPlayThreadExit = false;

    ALOGD("%s(), pthread_cond_signal(&pFIRRecording->mRecPlayExit_Cond)", __FUNCTION__);
    pthread_cond_signal(&pFIRRecording->mRecPlayExit_Cond); // wake all thread
    pthread_mutex_unlock(&pFIRRecording->mRecPlayMutex);

    AudioMTKStreamInInterface *streamInput = NULL;

    WAVEHDR waveHeader;
    memset(&waveHeader, 0, sizeof(WAVEHDR));

    char readBuffer[1024] = {0}; //for record

    //open input stream for record
    if (pOutFp)
    {
#ifdef MTK_AUDIO_HD_REC_SUPPORT
        AUDIO_HD_RECORD_SCENE_TABLE_STRUCT hdRecordSceneTable;
        AudioCustParamClient::GetInstance()->GetHdRecordSceneTableFromNV(&hdRecordSceneTable);
        if (hdRecordSceneTable.num_voice_rec_scenes > 0)  //value=0;
        {
            int32_t BesRecScene = 1;//1:cts verifier offset
            pFIRRecording->getSpeechEnhanceInfoInst()->SetBesRecScene(BesRecScene);
        }
#endif
        uint32_t device = AUDIO_DEVICE_IN_BUILTIN_MIC;
        int format = AUDIO_FORMAT_PCM_16_BIT;
        uint32_t channel = AUDIO_CHANNEL_IN_STEREO;
        uint32_t sampleRate = 48000;
        status_t status = 0;
        streamInput = pFIRRecording->getStreamManager()->openInputStream(device, &format, &channel, &sampleRate, &status, (audio_in_acoustics_t)0);

        android::AudioParameter paramInputSource = android::AudioParameter();
        paramInputSource.addInt(android::String8(android::AudioParameter::keyInputSource), AUDIO_SOURCE_MIC);
        streamInput->setParameters(paramInputSource.toString());

        //get device connection state
        android::AudioParameter paramDeviceIn = android::AudioParameter();
        if (pFIRRecording->getStreamManager()->getDeviceConnectionState(AUDIO_DEVICE_OUT_WIRED_HEADSET) == true)
        {
            paramDeviceIn.addInt(android::String8(android::AudioParameter::keyRouting), AUDIO_DEVICE_IN_WIRED_HEADSET);
            streamInput->setParameters(paramDeviceIn.toString());
        }
        else
        {
            paramDeviceIn.addInt(android::String8(android::AudioParameter::keyRouting), AUDIO_DEVICE_IN_BUILTIN_MIC);
            streamInput->setParameters(paramDeviceIn.toString());
        }

    }

    while (!pFIRRecording->m_bRecPlayThreadExit)
    {
        // handling record buffer
        pthread_mutex_lock(&pFIRRecording->mRecBufMutex);
        if (pOutFp)
        {
            memset(readBuffer, 0, sizeof(readBuffer));
            int nRead = streamInput->read(readBuffer, 1024);
#ifdef WRITE_RECORDDATA_ON_APSIDEDMNR
            fwrite(readBuffer, 1, nRead, pOutFp);
#endif
        }
        pthread_mutex_unlock(&pFIRRecording->mRecBufMutex);
    }

    if (pOutFp)
    {
        streamInput->standby(); //this will close input device
        pFIRRecording->getStreamManager()->closeInputStream(streamInput);
    }

    //exit thread
    pthread_mutex_lock(&pFIRRecording->mRecPlayMutex);
    ALOGD("%s(), pthread_cond_signal(&mRecPlayExit_Cond)", __FUNCTION__);
    pthread_cond_signal(&pFIRRecording->mRecPlayExit_Cond); // wake all thread
    pthread_mutex_unlock(&pFIRRecording->mRecPlayMutex);

    return 0;
}

#endif

AudioALSAParamTuner *AudioALSAParamTuner::UniqueTuningInstance = 0;

AudioALSAParamTuner *AudioALSAParamTuner::getInstance() {
    if (UniqueTuningInstance == 0) {
        ALOGD("%s(), create AudioALSAParamTuner instance --", __FUNCTION__);
        UniqueTuningInstance = new AudioALSAParamTuner();
        ALOGD("%s(), create AudioALSAParamTuner instance ++", __FUNCTION__);
    }

    return UniqueTuningInstance;
}

AudioALSAParamTuner::AudioALSAParamTuner() :
    mMode(0),
    m_bPPSThreadExit(false),
    m_bWBMode(false),
    m_pInputFile(NULL),
    mSideTone(0xFFFFFF40),
    m_bPlaying(false),
    mStartRec(false),
    mTasteThreadID(0),
    mRecPlayThreadID(0) {
    ALOGD("+%s()", __FUNCTION__);
    // create volume instance
    mAudioALSAVolumeController = AudioVolumeFactory::CreateAudioVolumeController();
    mAudioALSAVolumeController->initCheck();

    // create audio resource manager instance
    mAudioResourceManager = AudioALSAHardwareResourceManager::getInstance();

    // create speech driver instance
    mSpeechDriverFactory = SpeechDriverFactory::GetInstance();

    mSphPhonecallCtrl = AudioALSASpeechPhoneCallController::getInstance();

    memset(mOutputVolume, 0, MODE_NUM * sizeof(uint32));
    memset(m_strInputFileName, 0, FILE_NAME_LEN_MAX * sizeof(char));

    int ret = pthread_mutex_init(&mP2WMutex, NULL);
    if (ret != 0) {
        ALOGE("%s(), Failed to initialize pthread mP2WMutex!", __FUNCTION__);
    }

    ret = pthread_mutex_init(&mPPSMutex, NULL);
    if (ret != 0) {
        ALOGE("%s(), Failed to initialize mPPSMutex!", __FUNCTION__);
    }

    ret = pthread_mutex_init(&mPlayBufMutex, NULL);
    if (ret != 0) {
        ALOGE("%s(), Failed to initialize mPlayBufMutex!", __FUNCTION__);
    }

    ret = pthread_cond_init(&mPPSExit_Cond, NULL);
    if (ret != 0) {
        ALOGE("%s(), Failed to initialize mPPSExit_Cond!", __FUNCTION__);
    }

#if !defined(MTK_AUDIO_HD_REC_SUPPORT)
    if (mAudioResourceManager->getNumPhoneMicSupport() >= 2)
#endif
    {
        m_bDMNRPlaying = false;
        m_bRecPlayThreadExit = false;
        m_pOutputFile = NULL;

        mPlay2WayInstance = 0;
        mRec2WayInstance = 0;

        memset(m_strOutFileName, 0, FILE_NAME_LEN_MAX * sizeof(char));

        AUDIO_VER1_CUSTOM_VOLUME_STRUCT VolumeCustomParam;//volume custom data
        AudioCustParamClient::GetInstance()->GetVolumeVer1ParamFromNV(&VolumeCustomParam);

#ifdef MTK_NEW_VOL_CONTROL
        mDualMicTool_micGain[0] = AudioMTKGainController::getInstance()->getMicGainDecimal(Normal_Mic, GAIN_DEVICE_EARPIECE);
#else
        mDualMicTool_micGain[0] = VolumeCustomParam.audiovolume_mic[VOLUME_NORMAL_MODE][3];
#endif
        if (mDualMicTool_micGain[0] > UPLINK_GAIN_MAX) {
            mDualMicTool_micGain[0] = UPLINK_GAIN_MAX;
        }

#ifdef MTK_NEW_VOL_CONTROL
        mDualMicTool_micGain[1] = AudioMTKGainController::getInstance()->getMicGainDecimal(Handfree_Mic, GAIN_DEVICE_SPEAKER);
#else
        mDualMicTool_micGain[1] = VolumeCustomParam.audiovolume_mic[VOLUME_SPEAKER_MODE][3];
#endif
        if (mDualMicTool_micGain[1] > UPLINK_GAIN_MAX) {
            mDualMicTool_micGain[1] = UPLINK_GAIN_MAX;
        }

        mDualMicTool_receiverGain = VolumeCustomParam.audiovolume_sph[VOLUME_NORMAL_MODE][CUSTOM_VOLUME_STEP - 1];
        if (mDualMicTool_receiverGain > MAX_VOICE_VOLUME) {
            mDualMicTool_receiverGain = MAX_VOICE_VOLUME;
        }

        mDualMicTool_headsetGain = VolumeCustomParam.audiovolume_sph[VOLUME_HEADSET_MODE][3];
        if (mDualMicTool_headsetGain > MAX_VOICE_VOLUME) {
            mDualMicTool_headsetGain = MAX_VOICE_VOLUME;
        }
#ifndef DMNR_TUNNING_AT_MODEMSIDE
        mAudioStreamManager = AudioALSAStreamManager::getInstance();
        mAudioSpeechEnhanceInfoInstance = AudioSpeechEnhanceInfo::getInstance();
        mPlaybackDb_index = 0;
#endif
        mDMNROutputDevice = 0;

        ret = pthread_mutex_init(&mRecPlayMutex, NULL);
        if (ret != 0) {
            ALOGE("%s(), Failed to initialize mRecPlayMutex!", __FUNCTION__);
        }

        ret = pthread_mutex_init(&mRecBufMutex, NULL);
        if (ret != 0) {
            ALOGE("%s(), Failed to initialize mRecBufMutex!", __FUNCTION__);
        }

        ret = pthread_cond_init(&mRecPlayExit_Cond, NULL);
        if (ret != 0) {
            ALOGE("%s(), Failed to initialize mRecPlayExit_Cond!", __FUNCTION__);
        }

        ALOGD("%s(), AudioALSAParamTuner: default mic gain-mormal:%d;handsfree:%d, receiver gain:%d, headset Gain:%d", __FUNCTION__, mDualMicTool_micGain[0], mDualMicTool_micGain[1], mDualMicTool_receiverGain,
              mDualMicTool_headsetGain);
    }
}

AudioALSAParamTuner::~AudioALSAParamTuner() {
    ALOGD("~AudioALSAParamTuner");
}

//for taste tool
bool AudioALSAParamTuner::isPlaying() {
    ALOGV("%s(), playing:%d", __FUNCTION__, m_bPlaying);
    bool ret = false;
    pthread_mutex_lock(&mP2WMutex);

#if defined(MTK_AUDIO_HD_REC_SUPPORT)
    ALOGV("%s(), DMNR playing:%d", __FUNCTION__, m_bDMNRPlaying);
    ret = (m_bPlaying | m_bDMNRPlaying) ? true : false;
#else
    if (mAudioResourceManager->getNumPhoneMicSupport() >= 2) {
        ALOGV("%s(), DMNR playing:%d", __FUNCTION__, m_bDMNRPlaying);
        ret = (m_bPlaying | m_bDMNRPlaying) ? true : false;
    } else {
        ret = m_bPlaying;
    }
#endif

    pthread_mutex_unlock(&mP2WMutex);
    return ret;
}

uint32_t AudioALSAParamTuner::getMode() {
    ALOGD("%s(), mode:%d", __FUNCTION__, mMode);
    pthread_mutex_lock(&mP2WMutex);
    uint32_t ret = mMode;
    pthread_mutex_unlock(&mP2WMutex);
    return ret;
}

status_t AudioALSAParamTuner::setMode(uint32_t mode) {
    ALOGD("%s(), mode:%d", __FUNCTION__, mode);
    pthread_mutex_lock(&mP2WMutex);
    mMode = mode;
    pthread_mutex_unlock(&mP2WMutex);
    return NO_ERROR;
}

status_t AudioALSAParamTuner::setPlaybackFileName(const char *fileName) {
    ALOGD("+%s()",  __FUNCTION__);
    pthread_mutex_lock(&mP2WMutex);
    if (fileName != NULL && strlen(fileName) < FILE_NAME_LEN_MAX - 1) {
        ALOGD("%s(), input file name:%s", __FUNCTION__, fileName);
        memset(m_strInputFileName, 0, FILE_NAME_LEN_MAX);
        audio_strncpy(m_strInputFileName, fileName, FILE_NAME_LEN_MAX);
    } else {
        ALOGE("%s(), input file name NULL or too long",  __FUNCTION__);
        pthread_mutex_unlock(&mP2WMutex);
        return BAD_VALUE;
    }
    pthread_mutex_unlock(&mP2WMutex);
    return NO_ERROR;
}

status_t AudioALSAParamTuner::setDLPGA(uint32_t gain) {
    uint32_t outputDev = 0;

    if (gain > MAX_VOICE_VOLUME) {
        ALOGE("%s(), gain error  gain=%x", __FUNCTION__, gain);
        return BAD_VALUE;
    }

    pthread_mutex_lock(&mP2WMutex);
    mOutputVolume[mMode] = gain;
    ALOGD("+%s(), mode=%d, gain=%d, lad volume=0x%x", __FUNCTION__, mMode, gain, mOutputVolume[mMode]);

    if (m_bPlaying) {
        ALOGD("%s(), lad_Volume=%x", __FUNCTION__, mOutputVolume[mMode]);

#ifdef MTK_NEW_VOL_CONTROL
        //MTK_AUDIO_GAIN_TABLE set tuner gain directly to HW
        mAudioALSAVolumeController->ApplySideTone(GAIN_DEVICE_EARPIECE); // in 0.5dB
        setSphVolume(mMode, mOutputVolume[mMode]);
#else

        switch (mMode) {
        case SPEECH_MODE_NORMAL: {
            outputDev = AUDIO_DEVICE_OUT_EARPIECE;
            break;
        }
        case SPEECH_MODE_EARPHONE: {
            outputDev = AUDIO_DEVICE_OUT_WIRED_HEADSET ;
            break;
        }
        case SPEECH_MODE_LOUD_SPEAKER:
            outputDev = AUDIO_DEVICE_OUT_SPEAKER;
            break;
        default:
            outputDev = AUDIO_DEVICE_OUT_EARPIECE;
            break;
        }

        mAudioALSAVolumeController->setVoiceVolume(mOutputVolume[mMode], outputDev);

#endif
    }

    pthread_mutex_unlock(&mP2WMutex);
    return NO_ERROR;
}

void AudioALSAParamTuner::updataOutputFIRCoffes(AudioTasteTuningStruct *pCustParam) {
    SpeechDriverInterface *pSpeechDriver = mSpeechDriverFactory->GetSpeechDriver();
    ALOGD("+%s(), mMode:%d, m_bPlaying:%d,", __FUNCTION__, mMode, m_bPlaying);
#if defined(MTK_AUDIO_HIERARCHICAL_PARAM_SUPPORT)
    unsigned short IdxVolume = pCustParam->slected_fir_index;
    ALOGD("%s(), MTK_AUDIO_HIERARCHICAL_PARAM_SUPPORT update volume index(%d) for Audio Taste", __FUNCTION__, IdxVolume);
    //AUDIO_STREAM_VOICE_CALL 0
    pSpeechDriver->setMDVolumeIndex(0, 0, IdxVolume);

#else
    int ret = 0;
    unsigned short mode = pCustParam->phone_mode;
    unsigned short cmdType = pCustParam->cmd_type;
    uint32_t sampleRate = 16000;

    ALOGD("%s(), mode:%d, mMode:%d, m_bPlaying:%d,", __FUNCTION__, mode, mMode, m_bPlaying);

    pthread_mutex_lock(&mP2WMutex);

    if (m_bPlaying && mode == mMode) {
        pSpeechDriver->PCM2WayOff(); // trun off PCM2Way
        mAudioResourceManager->stopInputDevice(mAudioResourceManager->getInputDevice());
        mAudioResourceManager->stopOutputDevice();
        pthread_mutex_unlock(&mP2WMutex);
        usleep(10 * 1000); //wait to make sure all message is processed
        pthread_mutex_lock(&mP2WMutex);
    }
    AudioCustParamClient *pAudioCustParamClient = AudioCustParamClient::GetInstance();
    if (pCustParam->wb_mode) {
#if defined(MTK_WB_SPEECH_SUPPORT)
        AUDIO_CUSTOM_WB_PARAM_STRUCT sCustWbParam;
        pAudioCustParamClient->GetWBSpeechParamFromNVRam(&sCustWbParam);
        if (cmdType && sCustWbParam.speech_mode_wb_para[mode][7] != pCustParam->dlDigitalGain) {
            ALOGD("%s(), mode=%d, ori dlDG gain=%d, new dlDG gain=%d", __FUNCTION__, mode, sCustWbParam.speech_mode_wb_para[mode][7], pCustParam->dlDigitalGain);
            sCustWbParam.speech_mode_wb_para[mode][7] = pCustParam->dlDigitalGain;
        }
        ret = pSpeechDriver->SetWBSpeechParameters(&sCustWbParam);
#endif
    } else {
        AUDIO_CUSTOM_PARAM_STRUCT sCustParam;
        AUDIO_PARAM_MED_STRUCT sCustMedParam;
        unsigned short index = pCustParam->slected_fir_index;
        unsigned short dlGain = pCustParam->dlDigitalGain;
        pAudioCustParamClient->GetNBSpeechParamFromNVRam(&sCustParam);
        pAudioCustParamClient->GetMedParamFromNV(&sCustMedParam);

        if ((cmdType == (unsigned short)AUD_TASTE_START || cmdType == (unsigned short)AUD_TASTE_INDEX_SETTING) && sCustMedParam.select_FIR_output_index[mode] != index) {
            ALOGD("%s(), mode=%d, old index=%d, new index=%d", __FUNCTION__, mode, sCustMedParam.select_FIR_output_index[mode], index);
            //save  index to MED with different mode.
            sCustMedParam.select_FIR_output_index[mode] = index;

            ALOGD("%s(), ori sph_out_fir[%d][0]=%d, ori sph_out_fir[%d][44]=%d", __FUNCTION__, mode, sCustParam.sph_out_fir[mode][0], mode, sCustParam.sph_out_fir[mode][44]);
            //copy med data into audio_custom param
            memcpy((void *)sCustParam.sph_out_fir[mode], (void *)sCustMedParam.speech_output_FIR_coeffs[mode][index], sizeof(sCustParam.sph_out_fir[index]));
            ALOGD("%s(), new sph_out_fir[%d][0]=%d, new sph_out_fir[%d][44]=%d", __FUNCTION__, mode, sCustParam.sph_out_fir[mode][0], mode, sCustParam.sph_out_fir[mode][44]);
            pAudioCustParamClient->SetNBSpeechParamToNVRam(&sCustParam);
            pAudioCustParamClient->SetMedParamToNV(&sCustMedParam);
        }

        if ((cmdType == (unsigned short)AUD_TASTE_START || cmdType == (unsigned short)AUD_TASTE_DLDG_SETTING) && sCustParam.speech_mode_para[mode][7] != dlGain) {
            ALOGD("%s(), mode=%d, old dlDGGain=%d, new dlDGGain=%d", __FUNCTION__, mode, sCustParam.speech_mode_para[mode][7], dlGain);
            sCustParam.speech_mode_para[mode][7] = dlGain;
        }
        ALOGD("%s(), sph_out_fir[%d][0]=%d, sph_out_fir[%d][44]=%d", __FUNCTION__, mode, sCustParam.sph_out_fir[mode][0], mode, sCustParam.sph_out_fir[mode][44]);
        ret = pSpeechDriver->SetNBSpeechParameters(&sCustParam);
    }

    if (m_bPlaying && mode == mMode) {
        mAudioResourceManager->startInputDevice(AUDIO_DEVICE_IN_BUILTIN_MIC);

//V1 platform only support AFE 16KHz
        sampleRate = m_bWBMode ? 16000 : 8000;

        switch (mMode) {
        case SPEECH_MODE_NORMAL: {
            mAudioResourceManager->startOutputDevice(AUDIO_DEVICE_OUT_EARPIECE, sampleRate);
#ifdef MTK_NEW_VOL_CONTROL
            mAudioALSAVolumeController->ApplySideTone(GAIN_DEVICE_EARPIECE); // in 0.5dB
#else
            mAudioALSAVolumeController->ApplySideTone(EarPiece_SideTone_Gain); // in 0.5dB
#endif
            pSpeechDriver->SetSpeechMode(AUDIO_DEVICE_IN_BUILTIN_MIC, AUDIO_DEVICE_OUT_EARPIECE);
            break;
        }
        case SPEECH_MODE_EARPHONE: {
            mAudioResourceManager->startOutputDevice(AUDIO_DEVICE_OUT_WIRED_HEADSET, sampleRate);
#ifdef MTK_NEW_VOL_CONTROL
            mAudioALSAVolumeController->ApplySideTone(GAIN_DEVICE_HEADSET);
#else
            mAudioALSAVolumeController->ApplySideTone(Headset_SideTone_Gain);
#endif
            pSpeechDriver->SetSpeechMode(AUDIO_DEVICE_IN_BUILTIN_MIC, AUDIO_DEVICE_OUT_WIRED_HEADSET);
            break;
        }
        case SPEECH_MODE_LOUD_SPEAKER:
            mAudioResourceManager->startOutputDevice(AUDIO_DEVICE_OUT_SPEAKER, sampleRate);
#ifdef MTK_NEW_VOL_CONTROL
            mAudioALSAVolumeController->ApplySideTone(GAIN_DEVICE_SPEAKER);
#else
            mAudioALSAVolumeController->ApplySideTone(LoudSpk_SideTone_Gain);
#endif
            pSpeechDriver->SetSpeechMode(AUDIO_DEVICE_IN_BUILTIN_MIC, AUDIO_DEVICE_OUT_SPEAKER);
            break;
        default:
            break;
        }
        setSphVolume(mMode, mOutputVolume[mMode]);

        sph_enh_mask_struct_t sphMask;
        sphMask = SpeechEnhancementController::GetInstance()->GetSpeechEnhancementMask(); // copy DMNR mask
        sphMask.dynamic_func = SPH_ENH_DYNAMIC_MASK_VCE;
        pSpeechDriver->SetSpeechEnhancementMask(sphMask);
        pSpeechDriver->PCM2WayOn(m_bWBMode); // start PCM 2 way
        pSpeechDriver->SetSpeechEnhancement(true);
    }
    pthread_mutex_unlock(&mP2WMutex);

#endif
}

status_t AudioALSAParamTuner::enableModemPlaybackVIASPHPROC(bool bEnable, bool bWB) { //need record path?

    ALOGD("%s(), bEnable:%d, bWBMode:%d", __FUNCTION__, bEnable, bWB);
    int ret = 0;

    // 3 sec for creat thread timeout
    struct timeval now;
    struct timespec timeout;
    gettimeofday(&now, NULL);
    timeout.tv_sec = now.tv_sec + 3;
    timeout.tv_nsec = now.tv_usec * 1000;

    if (mRec2WayInstance == 0) {
        mRec2WayInstance = Record2Way::GetInstance();
    }
    if (mPlay2WayInstance == 0) {
        mPlay2WayInstance = Play2Way::GetInstance();
    }

    if (bEnable && (isPlaying() == false)) {
        ALOGD("%s(), open  Enable:%d, isPlaying():%d", __FUNCTION__, bEnable, isPlaying());
        pthread_mutex_lock(&mP2WMutex);
        m_pInputFile = fopen(m_strInputFileName, "rb");
        if (m_pInputFile == NULL) {
            m_pInputFile = fopen("/mnt/sdcard2/test.wav", "rb");
            if (m_pInputFile == NULL) {
                ALOGD("%s(), open input file fail!!", __FUNCTION__);
                pthread_mutex_unlock(&mP2WMutex);
                return BAD_VALUE;
            }
        }
        m_bWBMode = bWB;
        ALOGD("%s(), ApplySideTone mode=%d",  __FUNCTION__, mMode);
        //Audio Taste: use MODEM_1
        mSphPhonecallCtrl->setParam(String8("AudioTaste=1;"));
        switch (mMode) {
        case SPEECH_MODE_NORMAL: {
#ifdef MTK_NEW_VOL_CONTROL
            mAudioALSAVolumeController->ApplySideTone(GAIN_DEVICE_EARPIECE);// in 0.5dB
#else
            mAudioALSAVolumeController->ApplySideTone(EarPiece_SideTone_Gain);// in 0.5dB
#endif
            mSphPhonecallCtrl->open(AUDIO_MODE_IN_CALL, AUDIO_DEVICE_OUT_EARPIECE, AUDIO_DEVICE_IN_BUILTIN_MIC);
            break;
        }
        case SPEECH_MODE_EARPHONE: {
#ifdef MTK_NEW_VOL_CONTROL
            mAudioALSAVolumeController->ApplySideTone(GAIN_DEVICE_HEADSET);
#else
            mAudioALSAVolumeController->ApplySideTone(Headset_SideTone_Gain);
#endif
            mSphPhonecallCtrl->open(AUDIO_MODE_IN_CALL, AUDIO_DEVICE_OUT_WIRED_HEADSET, AUDIO_DEVICE_IN_WIRED_HEADSET);
            break;
        }
        case SPEECH_MODE_LOUD_SPEAKER:
#ifdef MTK_NEW_VOL_CONTROL
            mAudioALSAVolumeController->ApplySideTone(GAIN_DEVICE_SPEAKER);
#else
            mAudioALSAVolumeController->ApplySideTone(LoudSpk_SideTone_Gain);
#endif
            mSphPhonecallCtrl->open(AUDIO_MODE_IN_CALL, AUDIO_DEVICE_OUT_SPEAKER, AUDIO_DEVICE_IN_BUILTIN_MIC);
            break;
        default:
            break;
        }

        setSphVolume(mMode, mOutputVolume[mMode]);
        // start pcm2way
        mRec2WayInstance->Start();
        mPlay2WayInstance->Start();

        ALOGD("%s(), open taste_threadloop thread~",  __FUNCTION__);
        pthread_mutex_lock(&mPPSMutex);
        ret = pthread_create(&mTasteThreadID, NULL, Play_PCM_With_SpeechEnhance_Routine, (void *)this);
        if (ret != 0) {
            ALOGE("%s(), Play_PCM_With_SpeechEnhance_Routine thread pthread_create error!!",  __FUNCTION__);
            pthread_mutex_unlock(&mPPSMutex);
            pthread_mutex_unlock(&mP2WMutex);
            return UNKNOWN_ERROR;
        }

        ALOGD("%s(), +mPPSExit_Cond wait", __FUNCTION__);
        ret = pthread_cond_timedwait(&mPPSExit_Cond, &mPPSMutex, &timeout);
        ALOGD("%s(), -mPPSExit_Cond receive ret=%d", __FUNCTION__, ret);
        pthread_mutex_unlock(&mPPSMutex);

        pthread_mutex_unlock(&mP2WMutex);
        usleep(100 * 1000);
        pthread_mutex_lock(&mP2WMutex);

        m_bPlaying = true;
        sph_enh_mask_struct_t sphMask;
        sphMask = SpeechEnhancementController::GetInstance()->GetSpeechEnhancementMask(); // copy DMNR mask
        sphMask.dynamic_func = SPH_ENH_DYNAMIC_MASK_VCE;
        mSpeechDriverFactory->GetSpeechDriver()->SetSpeechEnhancementMask(sphMask);
        mSpeechDriverFactory->GetSpeechDriver()->PCM2WayOn(m_bWBMode); // start PCM 2 way
        mSpeechDriverFactory->GetSpeechDriver()->SetSpeechEnhancement(true);
        pthread_mutex_unlock(&mP2WMutex);
    } else if ((!bEnable) && m_bPlaying) {
        ALOGD("%s(), close  Enable:%d, isPlaying():%d", __FUNCTION__, bEnable, isPlaying());
        pthread_mutex_lock(&mP2WMutex);
        pthread_mutex_lock(&mPPSMutex);
        if (!m_bPPSThreadExit) {
            m_bPPSThreadExit = true;
            ALOGD("%s(), +mPPSExit_Cond wait", __FUNCTION__);
            ret = pthread_cond_timedwait(&mPPSExit_Cond, &mPPSMutex, &timeout);
            ALOGD("%s(), -mPPSExit_Cond receive ret=%d", __FUNCTION__, ret);
        }
        pthread_mutex_unlock(&mPPSMutex);

        mSpeechDriverFactory->GetSpeechDriver()->PCM2WayOff();
        mSphPhonecallCtrl->close();

        mRec2WayInstance->Stop();
        mPlay2WayInstance->Stop();
        mSphPhonecallCtrl->setParam(String8("AudioTaste=0;"));
        m_bPlaying = false;
        if (m_pInputFile) { fclose(m_pInputFile); }
        m_pInputFile = NULL;
        pthread_mutex_unlock(&mP2WMutex);
        usleep(200 * 1000); //wait to make sure all message is processed
    } else {
        ALOGD("%s(), The Audio Taste Tool State is error, bEnable=%d, playing=%d", __FUNCTION__, bEnable, m_bPlaying);
        return BAD_VALUE;
    }

    return NO_ERROR;
}

FILE_FORMAT AudioALSAParamTuner::playbackFileFormat() {
    ALOGD("%s(), playback file name:%s", __FUNCTION__, m_strInputFileName);
    FILE_FORMAT ret = UNSUPPORT_FORMAT;
    char *pFileSuffix = m_strInputFileName;

    strsep(&pFileSuffix, ".");
    if (pFileSuffix != NULL) {
        if (strcmp(pFileSuffix, "pcm") == 0 || strcmp(pFileSuffix, "PCM") == 0) {
            ALOGD("%s(), playback file format is pcm", __FUNCTION__);
            ret = PCM_FORMAT;
        } else if (strcmp(pFileSuffix, "wav") == 0 || strcmp(pFileSuffix, "WAV") == 0) {
            ALOGD("%s(), playback file format is wav", __FUNCTION__);
            ret = WAVE_FORMAT;
        } else {
            ALOGD("%s(), playback file format is unsupport", __FUNCTION__);
            ret = UNSUPPORT_FORMAT;
        }
    }

    return ret;
}

// For DMNR Tuning
status_t AudioALSAParamTuner::setRecordFileName(const char *fileName) {
#if !defined(MTK_AUDIO_HD_REC_SUPPORT)
    if (mAudioResourceManager->getNumPhoneMicSupport() < 2) {
        ALOGE("%s(), unsupport", __FUNCTION__);
        return INVALID_OPERATION;
    }
#endif
    ALOGD("+%s()", __FUNCTION__);
    pthread_mutex_lock(&mP2WMutex);
    if (fileName != NULL && strlen(fileName) < FILE_NAME_LEN_MAX - 1) {
        ALOGD("%s(), input file name:%s", __FUNCTION__, fileName);
        memset(m_strOutFileName, 0, FILE_NAME_LEN_MAX);
        audio_strncpy(m_strOutFileName, fileName, FILE_NAME_LEN_MAX);
    } else {
        ALOGE("%s(), input file name NULL or too long!", __FUNCTION__);
        pthread_mutex_unlock(&mP2WMutex);
        return BAD_VALUE;
    }

    pthread_mutex_unlock(&mP2WMutex);
    return NO_ERROR;
}

status_t AudioALSAParamTuner::setDMNRGain(unsigned short type, unsigned short value) {
#if !defined(MTK_AUDIO_HD_REC_SUPPORT)
    if (mAudioResourceManager->getNumPhoneMicSupport() < 2) {
        ALOGE("%s(), unsupport", __FUNCTION__);
        return INVALID_OPERATION;
    }
#endif

    ALOGD("%s(), type=%d, gain=%d", __FUNCTION__, type, value);
    status_t ret = NO_ERROR;

    if (value < 0) {
        return BAD_VALUE;
    }

    pthread_mutex_lock(&mP2WMutex);
    switch (type) {
    case AUD_MIC_GAIN:
        mDualMicTool_micGain[0] = (value > UPLINK_GAIN_MAX) ? UPLINK_GAIN_MAX : value;
        break;
    case AUD_RECEIVER_GAIN:
        mDualMicTool_receiverGain = (value > MAX_VOICE_VOLUME) ? MAX_VOICE_VOLUME : value;
        break;
    case AUD_HS_GAIN:
        mDualMicTool_headsetGain = (value > MAX_VOICE_VOLUME) ? MAX_VOICE_VOLUME : value;
        break;
    case AUD_MIC_GAIN_HF:
        mDualMicTool_micGain[1] = (value > UPLINK_GAIN_MAX) ? UPLINK_GAIN_MAX : value;
        break;
    default:
        ALOGW("%s(), unknown type", __FUNCTION__);
        ret = BAD_VALUE;
        break;
    }
    pthread_mutex_unlock(&mP2WMutex);
    return ret;
}

status_t AudioALSAParamTuner::getDMNRGain(unsigned short type, unsigned short *value) {
#if !defined(MTK_AUDIO_HD_REC_SUPPORT)
    if (mAudioResourceManager->getNumPhoneMicSupport() < 2) {
        ALOGE("%s(), unsupport", __FUNCTION__);
        return INVALID_OPERATION;
    }
#endif

    ALOGD("%s(), type=%d", __FUNCTION__, type);
    status_t ret = NO_ERROR;

    pthread_mutex_lock(&mP2WMutex);
    switch (type) {
    case AUD_MIC_GAIN:
        *value = mDualMicTool_micGain[0]; // normal mic
        break;
    case AUD_RECEIVER_GAIN:
        *value = mDualMicTool_receiverGain;
        break;
    case AUD_HS_GAIN:
        *value = mDualMicTool_headsetGain;
        break;
    case AUD_MIC_GAIN_HF:
        *value = mDualMicTool_micGain[1]; //handsfree mic
        break;
    default:
        ALOGW("%s(), unknown type", __FUNCTION__);
        ret = BAD_VALUE;
        break;
    }
    pthread_mutex_unlock(&mP2WMutex);
    return ret;
}

status_t AudioALSAParamTuner::setPlaybackVolume(uint32_t mode, uint32_t gain, uint32_t device) {
#if !defined(MTK_AUDIO_HD_REC_SUPPORT)
    if (mAudioResourceManager->getNumPhoneMicSupport() < 2) {
        ALOGE("%s(), unsupport", __FUNCTION__);
        return INVALID_OPERATION;
    }
#endif

    ALOGD("+%s(), gain:%u,mode:%u, device=%u", __FUNCTION__, gain, mode, device);

    switch (mode) {
    case VOLUME_NORMAL_MODE:

        mPlaybackDb_index = mAudioALSAVolumeController->ApplyAudioGainTuning(gain, mode, Audio_Earpiece);
        break;
    case VOLUME_HEADSET_MODE:

        mPlaybackDb_index = mAudioALSAVolumeController->ApplyAudioGainTuning(gain, mode, Audio_Headset);
        break;
    case VOLUME_SPEAKER_MODE:
    case VOLUME_HEADSET_SPEAKER_MODE:
        // nothing to do
        ALOGD("%s(), invalid mode!!", __FUNCTION__);
        break;
    default:
        break;
    }

    ALOGV("-%s(), mPlaybackDb_index=%d", __FUNCTION__, mPlaybackDb_index);
    return NO_ERROR;
}

status_t AudioALSAParamTuner::enableDMNRAtApSide(bool bEnable, bool bWBMode, unsigned short outputDevice, unsigned short workMode) {
#if !defined(MTK_AUDIO_HD_REC_SUPPORT)
    if (mAudioResourceManager->getNumPhoneMicSupport() < 2) {
        ALOGE("%s(), unsupport", __FUNCTION__);
        return INVALID_OPERATION;
    }
#endif

    ALOGD("%s(), bEnable:%d, wb mode:%d, outputDevice:%d,work mode:%d", __FUNCTION__, bEnable, bWBMode, outputDevice, workMode);

    // 3 sec for timeout
    struct timeval now;
    struct timespec timeout;
    gettimeofday(&now, NULL);
    timeout.tv_sec = now.tv_sec + 3;
    timeout.tv_nsec = now.tv_usec * 1000;
    int ret;

    if (bEnable && (isPlaying() == false)) {
        pthread_mutex_lock(&mP2WMutex);
        // open input file for playback
        if ((workMode == RECPLAY_MODE) || (workMode == RECPLAY_HF_MODE)) {
            m_pInputFile = fopen(m_strInputFileName, "rb");
            ALOGD("%s(), [Dual-Mic] open input file filename:%s", __FUNCTION__, m_strInputFileName);
            if (m_pInputFile == NULL) {
                ALOGW("%s(), [Dual-Mic] open input file fail!!", __FUNCTION__);
                pthread_mutex_unlock(&mP2WMutex);
                return BAD_VALUE;
            }
        }

        m_pOutputFile = fopen(m_strOutFileName, "wb");
        ALOGD("%s(), [Dual-Mic] open output file filename:%s", __FUNCTION__, m_strOutFileName);
        if (m_pOutputFile == NULL) {
            ALOGW("%s(), [Dual-Mic] open output file fail!!", __FUNCTION__);
            fclose(m_pInputFile);
            pthread_mutex_unlock(&mP2WMutex);
            return BAD_VALUE;
        }

        m_bWBMode = bWBMode;

        //set MIC gain
#ifdef MTK_NEW_VOL_CONTROL
#ifdef MTK_AURISYS_FRAMEWORK_SUPPORT
        GAIN_MIC_MODE mode = GAIN_MIC_CUSTOMIZATION3;
#else
        GAIN_MIC_MODE mode = GAIN_MIC_CUSTOMIZATION1;
#endif
#endif

        if (workMode > RECONLY_MODE) {
            mAudioSpeechEnhanceInfoInstance-> SetAPTuningMode(HANDSFREE_MODE_DMNR);
#ifdef MTK_NEW_VOL_CONTROL
            AudioMTKGainController::getInstance()->SetMicGainTuning(mode, GAIN_DEVICE_SPEAKER, mDualMicTool_micGain[1]);
            AudioMTKGainController::getInstance()->SetMicGainTuning(mode, GAIN_DEVICE_EARPIECE, mDualMicTool_micGain[1]);
#else
            mAudioALSAVolumeController->SetMicGainTuning(Handfree_Mic, mDualMicTool_micGain[1]);
#endif
        } else {
            mAudioSpeechEnhanceInfoInstance-> SetAPTuningMode(NORMAL_MODE_DMNR);
#ifdef MTK_NEW_VOL_CONTROL
            AudioMTKGainController::getInstance()->SetMicGainTuning(mode, GAIN_DEVICE_SPEAKER, mDualMicTool_micGain[0]);
            AudioMTKGainController::getInstance()->SetMicGainTuning(mode, GAIN_DEVICE_EARPIECE, mDualMicTool_micGain[0]);
#else
            mAudioALSAVolumeController->SetMicGainTuning(Normal_Mic, mDualMicTool_micGain[0]);
#endif
        }

        //set output and output gain in dB
        if ((workMode == RECPLAY_MODE) || (workMode == RECPLAY_HF_MODE)) {
            uint32_t dev = outputDevice == OUTPUT_DEVICE_RECEIVER ? AUDIO_DEVICE_OUT_EARPIECE : AUDIO_DEVICE_OUT_WIRED_HEADSET;
            mDMNROutputDevice = dev;
            uint32_t volume = outputDevice == OUTPUT_DEVICE_RECEIVER ? mDualMicTool_receiverGain : mDualMicTool_headsetGain;
            uint32_t mode = outputDevice == OUTPUT_DEVICE_RECEIVER ? VOLUME_NORMAL_MODE : VOLUME_HEADSET_MODE;
            ALOGD("%s(), changeOutputDevice,dev=%d, mDMNROutputDevice=0x%x ", __FUNCTION__, dev, mDMNROutputDevice);
            //mAudioResourceManager->changeOutputDevice(dev); //set downlink path, modify to move it to openstreamout device
            setPlaybackVolume(mode, volume, dev);
            ALOGD("%s(), Play+Rec set dual mic at ap side, dev:0x%x, mode:%d, gain:%d", __FUNCTION__, dev, mode, volume);
        }

        // open buffer thread
        ALOGD("%s(), threadloop thread~", __FUNCTION__);
        pthread_mutex_lock(&mRecPlayMutex);
        ret = pthread_create(&mRecPlayThreadID, NULL, DMNR_Play_Rec_ApSide_Routine, (void *)this);
        if (ret != 0) {
            ALOGE("%s(), pthread_create error!!", __FUNCTION__);
        }

        ALOGD("%s(), +mRecPlayExit_Cond wait", __FUNCTION__);
        ret = pthread_cond_timedwait(&mRecPlayExit_Cond, &mRecPlayMutex, &timeout);
        ALOGD("%s(), -mRecPlayExit_Cond receive ret=%d", __FUNCTION__, ret);
        pthread_mutex_unlock(&mRecPlayMutex);

        m_bDMNRPlaying = true;
        pthread_mutex_unlock(&mP2WMutex);

        usleep(10 * 1000);
    } else if (!bEnable && m_bDMNRPlaying) {
        //stop buffer thread
        ALOGD("%s(), close", __FUNCTION__);
        pthread_mutex_lock(&mRecPlayMutex);
        if (!m_bRecPlayThreadExit) {
            m_bRecPlayThreadExit = true;
            ALOGD("%s(), +mRecPlayExit_Cond wait", __FUNCTION__);
            ret = pthread_cond_timedwait(&mRecPlayExit_Cond, &mRecPlayMutex, &timeout);
            ALOGD("%s(), -mRecPlayExit_Cond receive ret=%d", __FUNCTION__, ret);
        }
        pthread_mutex_unlock(&mRecPlayMutex);

        //wait to make sure all message is processed
        usleep(200 * 1000);

        //set back MIC gain
        AUDIO_VER1_CUSTOM_VOLUME_STRUCT VolumeCustomParam;//volume custom data
        AudioCustParamClient::GetInstance()->GetVolumeVer1ParamFromNV(&VolumeCustomParam);

#ifdef MTK_NEW_VOL_CONTROL
        // reset volume from xml
        AudioMTKGainController::getInstance()->updateXmlParam(REC_VOL_AUDIOTYPE_NAME);
#else
        uint32_t voldB = VolumeCustomParam.audiovolume_mic[VOLUME_NORMAL_MODE][3];
        voldB = voldB > UPLINK_GAIN_MAX ? UPLINK_GAIN_MAX : voldB;
        mAudioALSAVolumeController->SetMicGainTuning(Normal_Mic, voldB);

        voldB = VolumeCustomParam.audiovolume_mic[VOLUME_SPEAKER_MODE][3];
        voldB = voldB > UPLINK_GAIN_MAX ? UPLINK_GAIN_MAX : voldB;
        mAudioALSAVolumeController->SetMicGainTuning(Handfree_Mic, voldB);
#endif
        mAudioSpeechEnhanceInfoInstance-> SetAPTuningMode(TUNING_MODE_NONE);

        m_bDMNRPlaying = false;

        pthread_mutex_lock(&mP2WMutex);
        if (m_pInputFile) { fclose(m_pInputFile); }
        m_pInputFile = NULL;
        pthread_mutex_unlock(&mP2WMutex);

        if (m_pOutputFile) { fclose(m_pOutputFile); }
        m_pOutputFile = NULL;
    } else {
        ALOGD("%s(), The DMNR Tuning State is error, bEnable=%d, playing=%d", __FUNCTION__, bEnable, m_bPlaying);
        return BAD_VALUE;
    }

    return NO_ERROR;
}

status_t AudioALSAParamTuner::enableFIRRecord(bool bEnable)
{
    ALOGD("%s(), bEnable:%d", __FUNCTION__, bEnable);

    // 3 sec for timeout
    struct timeval now;
    struct timespec timeout;
    gettimeofday(&now, NULL);
    timeout.tv_sec = now.tv_sec + 3;
    timeout.tv_nsec = now.tv_usec * 1000;
    int ret;

    if (bEnable && !mStartRec)
    {
        // open output file for record
        m_pOutputFile = fopen(m_strOutFileName, "wb");
        ALOGD("%s(), open output file filename:%s", __FUNCTION__, m_strOutFileName);
        if (m_pOutputFile == NULL)
        {
            ALOGW("%s(), open output file fail!!", __FUNCTION__);
            return BAD_VALUE;
        }

        // open buffer thread
        ALOGD("%s(), threadloop thread~", __FUNCTION__);
        pthread_mutex_lock(&mRecPlayMutex);
        ret = pthread_create(&mRecPlayThreadID, NULL, FIR_Rec_ApSide_Routine, (void *)this);
        if (ret != 0)
        {
            ALOGE("%s(), pthread_create error!!", __FUNCTION__);
        }

        ALOGD("%s(), +mRecPlayExit_Cond wait", __FUNCTION__);
        ret = pthread_cond_timedwait(&mRecPlayExit_Cond, &mRecPlayMutex, &timeout);
        ALOGD("%s(), -mRecPlayExit_Cond receive ret=%d", __FUNCTION__, ret);
        pthread_mutex_unlock(&mRecPlayMutex);

        mStartRec = true;
    }
    else if (!bEnable && mStartRec)
    {
        //stop buffer thread
        ALOGD("%s(), close", __FUNCTION__);
        pthread_mutex_lock(&mRecPlayMutex);
        if (!m_bRecPlayThreadExit)
        {
            m_bRecPlayThreadExit = true;
            ALOGD("%s(), +mRecPlayExit_Cond wait", __FUNCTION__);
            ret = pthread_cond_timedwait(&mRecPlayExit_Cond, &mRecPlayMutex, &timeout);
            ALOGD("%s(), -mRecPlayExit_Cond receive ret=%d", __FUNCTION__, ret);
        }
        pthread_mutex_unlock(&mRecPlayMutex);

        //wait to make sure all message is processed or tuning tool may crash
        usleep(200 * 1000);

        mStartRec = false;

        if (m_pOutputFile) { fclose(m_pOutputFile); }
        m_pOutputFile = NULL;
    }
    else
    {
        ALOGD("%s(), The FIR Record State is error, bEnable=%d", __FUNCTION__, bEnable);
        return BAD_VALUE;
    }

    return NO_ERROR;
}

#if defined(MTK_AUDIO_GAIN_TABLE) && !defined(MTK_NEW_VOL_CONTROL)
status_t AudioALSAParamTuner::getGainInfoForDisp(void *pParam) {
    //for UI Disp
    PCDispTotolStru *ptr = (PCDispTotolStru *)pParam;

    //ptr->itemNum = 0xFFFF;
    ptr->itemNum = 0xF3FF;
    getPCDispItem(pParam);
    getPCDispMic(pParam);

    ptr->gainRangeNum = 0x3FF;
    memcpy(ptr->gainRange, gainRangeCopy, sizeof(gainRangeCopy));

    return NO_ERROR;
}

status_t AudioALSAParamTuner::getPCDispItem(void *pParam) {
    ALOGD("+%s()", __FUNCTION__);
    PCDispTotolStru *ptr = (PCDispTotolStru *)pParam;
    mAudioHWVolumeCapabilityInstance = AudioHWVolumeCapability::getInstance();

    //for DispItem[0]
    memcpy(ptr->DispItem[0].strType, "VOIP", sizeof("VOIP"));
    ptr->DispItem[0].level = mAudioHWVolumeCapabilityInstance->getStreamLevel((int(AUDIO_STREAM_VOICE_CALL)));
    ptr->DispItem[0].subItemNum = 0x7;
    getPCDispSubItem(pParam, int(AUDIO_STREAM_VOICE_CALL));

    //for DispItem[1]
    memcpy(ptr->DispItem[1].strType, "System", sizeof("System"));
    ptr->DispItem[1].level = mAudioHWVolumeCapabilityInstance->getStreamLevel((int(AUDIO_STREAM_SYSTEM)));
    ptr->DispItem[1].subItemNum = 0x6;
    getPCDispSubItem(pParam, int(AUDIO_STREAM_SYSTEM));

    //for DispItem[2]
    memcpy(ptr->DispItem[2].strType, "Ring", sizeof("Ring"));
    ptr->DispItem[2].level = mAudioHWVolumeCapabilityInstance->getStreamLevel((int(AUDIO_STREAM_RING)));
    ptr->DispItem[2].subItemNum = 0x6;
    getPCDispSubItem(pParam, int(AUDIO_STREAM_RING));

    //for DispItem[3]
    memcpy(ptr->DispItem[3].strType, "Music", sizeof("Music"));
    ptr->DispItem[3].level = mAudioHWVolumeCapabilityInstance->getStreamLevel((int(AUDIO_STREAM_MUSIC)));
    ptr->DispItem[3].subItemNum = 0x6;
    getPCDispSubItem(pParam, int(AUDIO_STREAM_MUSIC));

    //for DispItem[4]
    memcpy(ptr->DispItem[4].strType, "Alarm", sizeof("Alarm"));
    ptr->DispItem[4].level = mAudioHWVolumeCapabilityInstance->getStreamLevel((int(AUDIO_STREAM_ALARM)));
    ptr->DispItem[4].subItemNum = 0x6;
    getPCDispSubItem(pParam, int(AUDIO_STREAM_ALARM));

    //for DispItem[5]
    memcpy(ptr->DispItem[5].strType, "Notification", sizeof("Notification"));
    ptr->DispItem[5].level = mAudioHWVolumeCapabilityInstance->getStreamLevel((int(AUDIO_STREAM_NOTIFICATION)));
    ptr->DispItem[5].subItemNum = 0x6;
    getPCDispSubItem(pParam, int(AUDIO_STREAM_NOTIFICATION));

    //for DispItem[6]
    memcpy(ptr->DispItem[6].strType, "Bluetooth_sco", sizeof("Bluetooth_sco"));
    ptr->DispItem[6].level = mAudioHWVolumeCapabilityInstance->getStreamLevel((int(AUDIO_STREAM_BLUETOOTH_SCO)));
    ptr->DispItem[6].subItemNum = 0x2;
    getPCDispSubItem(pParam, int(AUDIO_STREAM_BLUETOOTH_SCO));

    //for DispItem[7]
    memcpy(ptr->DispItem[7].strType, "Enforce Audbile", sizeof("Enforce Audbile"));
    ptr->DispItem[7].level = mAudioHWVolumeCapabilityInstance->getStreamLevel((int(AUDIO_STREAM_ENFORCED_AUDIBLE)));
    ptr->DispItem[7].subItemNum = 0x6;
    getPCDispSubItem(pParam, int(AUDIO_STREAM_ENFORCED_AUDIBLE));

    //for DispItem[8]
    memcpy(ptr->DispItem[8].strType, "DTMF", sizeof("DTMF"));
    ptr->DispItem[8].level = mAudioHWVolumeCapabilityInstance->getStreamLevel((int(AUDIO_STREAM_DTMF)));
    ptr->DispItem[8].subItemNum = 0x6;
    getPCDispSubItem(pParam, int(AUDIO_STREAM_DTMF));

    //for DispItem[9]
    memcpy(ptr->DispItem[9].strType, "TTS", sizeof("TTS"));
    ptr->DispItem[9].level = mAudioHWVolumeCapabilityInstance->getStreamLevel((int(AUDIO_STREAM_TTS)));
    ptr->DispItem[9].subItemNum = 0x6;
    getPCDispSubItem(pParam, int(AUDIO_STREAM_TTS));

    //for DispItem[10]

    //for DispItem[11]

    //for DispItem[12]
    memcpy(ptr->DispItem[12].strType, "SidetoneNB", sizeof("SidetoneNB"));
    ptr->DispItem[12].level = 0x0100;
    ptr->DispItem[12].subItemNum = 0x7;
    getPCDispSubItem(pParam, int(AUDIO_STREAM_VOICE_CALL), 3);

    //for DispItem[13]
    memcpy(ptr->DispItem[13].strType, "SidetoneWB", sizeof("SidetoneWB"));
    ptr->DispItem[13].level = 0x0100;
    ptr->DispItem[13].subItemNum = 0x7;
    getPCDispSubItem(pParam, int(AUDIO_STREAM_VOICE_CALL), 4);

    //for DispItem[14]
    memcpy(ptr->DispItem[14].strType, "SpeechNB", sizeof("SpeechNB"));
    ptr->DispItem[14].level = mAudioHWVolumeCapabilityInstance->getStreamLevel((int(AUDIO_STREAM_VOICE_CALL)));
    ptr->DispItem[14].level = (ptr->DispItem[14].level & 0xFF00) | ((ptr->DispItem[14].level & 0xFF00) >> 8);
    ptr->DispItem[14].subItemNum = 0x7;
    getPCDispSubItem(pParam, int(AUDIO_STREAM_VOICE_CALL), 1);

    //for DispItem[15]
    memcpy(ptr->DispItem[15].strType, "SpeechWB", sizeof("SpeechWB"));
    ptr->DispItem[15].level = mAudioHWVolumeCapabilityInstance->getStreamLevel((int(AUDIO_STREAM_VOICE_CALL)));
    ptr->DispItem[15].level = (ptr->DispItem[15].level & 0xFF00) | ((ptr->DispItem[15].level & 0xFF00) >> 8);
    ptr->DispItem[15].subItemNum = 0x7;
    getPCDispSubItem(pParam, int(AUDIO_STREAM_VOICE_CALL), 2);

    ALOGD("-%s()", __FUNCTION__);
    return NO_ERROR;
}
status_t AudioALSAParamTuner::getPCDispSubItem(void *pParam, int streamType, int speech) {
    ALOGD("+%s()", __FUNCTION__);
    PCDispTotolStru *ptr = (PCDispTotolStru *)pParam;
    mAudioHWVolumeCapabilityInstance = AudioHWVolumeCapability::getInstance();

    switch ((audio_stream_type_t)streamType) {
    case AUDIO_STREAM_VOICE_CALL: {
        ALOGD("%s(), speech is %d", __FUNCTION__, speech);
        if (speech != 1 && speech != 2 && speech != 3 && speech != 4) { //for VOIP
            memcpy(ptr->DispItem[0].subItem[0].outputDevice, "Receiver", sizeof("Receiver"));
            ptr->DispItem[0].subItem[0].AnalogPoint = mAudioHWVolumeCapabilityInstance->getDevicePath(NORMAL_MODE, CAPABILITY_DEVICE_RECEIVER);

            memcpy(ptr->DispItem[0].subItem[1].outputDevice, "Headset", sizeof("Headset"));
            ptr->DispItem[0].subItem[1].AnalogPoint = mAudioHWVolumeCapabilityInstance->getDevicePath(NORMAL_MODE, CAPABILITY_DEVICE_HEADPHONE);

            memcpy(ptr->DispItem[0].subItem[2].outputDevice, "Speaker", sizeof("Speaker"));
#ifdef USING_EXTAMP_HP
            ptr->DispItem[0].subItem[2].AnalogPoint = mAudioHWVolumeCapabilityInstance->getDevicePath(NORMAL_MODE, CAPABILITY_DEVICE_SPEAKER, false);
#else
            ptr->DispItem[0].subItem[2].AnalogPoint = mAudioHWVolumeCapabilityInstance->getDevicePath(NORMAL_MODE, CAPABILITY_DEVICE_SPEAKER, true);
#endif
        } else if (speech == 1) {         // for speech NB
            memcpy(ptr->DispItem[14].subItem[0].outputDevice, "Incall Receiver", sizeof("Incall Receiver"));
            ptr->DispItem[14].subItem[0].AnalogPoint = mAudioHWVolumeCapabilityInstance->getDevicePath(PHONE_CALL_MODE, CAPABILITY_DEVICE_RECEIVER);

            memcpy(ptr->DispItem[14].subItem[1].outputDevice, "Incall Headset", sizeof("Incall Headset"));
            ptr->DispItem[14].subItem[1].AnalogPoint = mAudioHWVolumeCapabilityInstance->getDevicePath(PHONE_CALL_MODE, CAPABILITY_DEVICE_HEADPHONE);

            memcpy(ptr->DispItem[14].subItem[2].outputDevice, "Incall Speaker", sizeof("Incall Speaker"));
#ifdef USING_EXTAMP_HP
            ptr->DispItem[14].subItem[2].AnalogPoint = mAudioHWVolumeCapabilityInstance->getDevicePath(PHONE_CALL_MODE, CAPABILITY_DEVICE_SPEAKER, false);
#else
            ptr->DispItem[14].subItem[2].AnalogPoint = mAudioHWVolumeCapabilityInstance->getDevicePath(PHONE_CALL_MODE, CAPABILITY_DEVICE_SPEAKER, true);
#endif
        } else if (speech == 2) {         //for speech WB
            memcpy(ptr->DispItem[15].subItem[0].outputDevice, "Incall Receiver", sizeof("Incall Receiver"));
            ptr->DispItem[15].subItem[0].AnalogPoint = mAudioHWVolumeCapabilityInstance->getDevicePath(PHONE_CALL_MODE, CAPABILITY_DEVICE_RECEIVER);

            memcpy(ptr->DispItem[15].subItem[1].outputDevice, "Incall Headset", sizeof("Incall Headset"));
            ptr->DispItem[15].subItem[1].AnalogPoint = mAudioHWVolumeCapabilityInstance->getDevicePath(PHONE_CALL_MODE, CAPABILITY_DEVICE_HEADPHONE);

            memcpy(ptr->DispItem[15].subItem[2].outputDevice, "Incall Speaker", sizeof("Incall Speaker"));
#ifdef USING_EXTAMP_HP
            ptr->DispItem[15].subItem[2].AnalogPoint = mAudioHWVolumeCapabilityInstance->getDevicePath(PHONE_CALL_MODE, CAPABILITY_DEVICE_SPEAKER, false);
#else
            ptr->DispItem[15].subItem[2].AnalogPoint = mAudioHWVolumeCapabilityInstance->getDevicePath(PHONE_CALL_MODE, CAPABILITY_DEVICE_SPEAKER, true);
#endif
        } else if (speech == 3) {         // for sidetone NB
            memcpy(ptr->DispItem[12].subItem[0].outputDevice, "Incall Receiver", sizeof("Incall Receiver"));
            ptr->DispItem[12].subItem[0].AnalogPoint = 0x0;

            memcpy(ptr->DispItem[12].subItem[1].outputDevice, "Incall Headset", sizeof("Incall Headset"));
            ptr->DispItem[12].subItem[1].AnalogPoint = 0x0;

            memcpy(ptr->DispItem[12].subItem[2].outputDevice, "Incall Speaker", sizeof("Incall Speaker"));
            //              ptr->DispItem[12].subItem[2].AnalogPoint = 0x0;
#ifdef USING_EXTAMP_HP
            ptr->DispItem[12].subItem[2].AnalogPoint = mAudioHWVolumeCapabilityInstance->getDevicePath(-1, -1, false);
#else
            ptr->DispItem[12].subItem[2].AnalogPoint = mAudioHWVolumeCapabilityInstance->getDevicePath(-1, -1, true);
#endif
        } else if (speech == 4) {         //for sidetone WB
            memcpy(ptr->DispItem[13].subItem[0].outputDevice, "Incall Receiver", sizeof("Incall Receiver"));
            ptr->DispItem[13].subItem[0].AnalogPoint = 0x0;

            memcpy(ptr->DispItem[13].subItem[1].outputDevice, "Incall Headset", sizeof("Incall Headset"));
            ptr->DispItem[13].subItem[1].AnalogPoint = 0x0;

            memcpy(ptr->DispItem[13].subItem[2].outputDevice, "Incall Speaker", sizeof("Incall Speaker"));
            //              ptr->DispItem[13].subItem[2].AnalogPoint = 0x0;
#ifdef USING_EXTAMP_HP
            ptr->DispItem[13].subItem[2].AnalogPoint = mAudioHWVolumeCapabilityInstance->getDevicePath(-1, -1, false);
#else
            ptr->DispItem[13].subItem[2].AnalogPoint = mAudioHWVolumeCapabilityInstance->getDevicePath(-1, -1, true);
#endif
        }
    }
    break;
    case AUDIO_STREAM_SYSTEM: {
        memcpy(ptr->DispItem[1].subItem[1].outputDevice, "Headset", sizeof("Headset"));
        ptr->DispItem[1].subItem[1].AnalogPoint = mAudioHWVolumeCapabilityInstance->getDevicePath(NORMAL_MODE, CAPABILITY_DEVICE_HEADPHONE);

        memcpy(ptr->DispItem[1].subItem[2].outputDevice, "Speaker", sizeof("Speaker"));
#ifdef USING_EXTAMP_HP
        ptr->DispItem[1].subItem[2].AnalogPoint = mAudioHWVolumeCapabilityInstance->getDevicePath(NORMAL_MODE, CAPABILITY_DEVICE_SPEAKER, false);
#else
        ptr->DispItem[1].subItem[2].AnalogPoint = mAudioHWVolumeCapabilityInstance->getDevicePath(NORMAL_MODE, CAPABILITY_DEVICE_SPEAKER, true);
#endif
    }
    break;
    case AUDIO_STREAM_RING: {
        memcpy(ptr->DispItem[2].subItem[1].outputDevice, "Headset", sizeof("Headset"));
        ptr->DispItem[2].subItem[1].AnalogPoint = mAudioHWVolumeCapabilityInstance->getDevicePath(NORMAL_MODE, CAPABILITY_DEVICE_HEADPHONE);

        memcpy(ptr->DispItem[2].subItem[2].outputDevice, "Speaker", sizeof("Speaker"));
#ifdef USING_EXTAMP_HP
        ptr->DispItem[2].subItem[2].AnalogPoint = mAudioHWVolumeCapabilityInstance->getDevicePath(NORMAL_MODE, CAPABILITY_DEVICE_SPEAKER, false);
#else
        ptr->DispItem[2].subItem[2].AnalogPoint = mAudioHWVolumeCapabilityInstance->getDevicePath(NORMAL_MODE, CAPABILITY_DEVICE_SPEAKER, true);
#endif
    }
    break;
    case AUDIO_STREAM_MUSIC: {
        memcpy(ptr->DispItem[3].subItem[1].outputDevice, "Headset", sizeof("Headset"));
        ptr->DispItem[3].subItem[1].AnalogPoint = mAudioHWVolumeCapabilityInstance->getDevicePath(NORMAL_MODE, CAPABILITY_DEVICE_HEADPHONE);

        memcpy(ptr->DispItem[3].subItem[2].outputDevice, "Speaker", sizeof("Speaker"));
#ifdef USING_EXTAMP_HP
        ptr->DispItem[3].subItem[2].AnalogPoint = mAudioHWVolumeCapabilityInstance->getDevicePath(NORMAL_MODE, CAPABILITY_DEVICE_SPEAKER, false);
#else
        ptr->DispItem[3].subItem[2].AnalogPoint = mAudioHWVolumeCapabilityInstance->getDevicePath(NORMAL_MODE, CAPABILITY_DEVICE_SPEAKER, true);
#endif
    }
    break;
    case AUDIO_STREAM_ALARM: {
        memcpy(ptr->DispItem[4].subItem[1].outputDevice, "Headset", sizeof("Headset"));
        ptr->DispItem[4].subItem[1].AnalogPoint = mAudioHWVolumeCapabilityInstance->getDevicePath(NORMAL_MODE, CAPABILITY_DEVICE_HEADPHONE);

        memcpy(ptr->DispItem[4].subItem[2].outputDevice, "Speaker", sizeof("Speaker"));
#ifdef USING_EXTAMP_HP
        ptr->DispItem[4].subItem[2].AnalogPoint = mAudioHWVolumeCapabilityInstance->getDevicePath(NORMAL_MODE, CAPABILITY_DEVICE_SPEAKER, false);
#else
        ptr->DispItem[4].subItem[2].AnalogPoint = mAudioHWVolumeCapabilityInstance->getDevicePath(NORMAL_MODE, CAPABILITY_DEVICE_SPEAKER, true);
#endif
    }
    break;
    case AUDIO_STREAM_NOTIFICATION: {
        memcpy(ptr->DispItem[5].subItem[1].outputDevice, "Headset", sizeof("Headset"));
        ptr->DispItem[5].subItem[1].AnalogPoint = mAudioHWVolumeCapabilityInstance->getDevicePath(NORMAL_MODE, CAPABILITY_DEVICE_HEADPHONE);

        memcpy(ptr->DispItem[5].subItem[2].outputDevice, "Speaker", sizeof("Speaker"));
#ifdef USING_EXTAMP_HP
        ptr->DispItem[5].subItem[2].AnalogPoint = mAudioHWVolumeCapabilityInstance->getDevicePath(NORMAL_MODE, CAPABILITY_DEVICE_SPEAKER, false);
#else
        ptr->DispItem[5].subItem[2].AnalogPoint = mAudioHWVolumeCapabilityInstance->getDevicePath(NORMAL_MODE, CAPABILITY_DEVICE_SPEAKER, true);
#endif
    }
    break;
    case AUDIO_STREAM_BLUETOOTH_SCO: {
        memcpy(ptr->DispItem[6].subItem[1].outputDevice, "Headset", sizeof("Headset"));
        ptr->DispItem[6].subItem[1].AnalogPoint = mAudioHWVolumeCapabilityInstance->getDevicePath(NORMAL_MODE, CAPABILITY_DEVICE_NONE);
    }
    break;
    case AUDIO_STREAM_ENFORCED_AUDIBLE: {
        memcpy(ptr->DispItem[7].subItem[1].outputDevice, "Headset", sizeof("Headset"));
        ptr->DispItem[7].subItem[1].AnalogPoint = mAudioHWVolumeCapabilityInstance->getDevicePath(NORMAL_MODE, CAPABILITY_DEVICE_HEADPHONE);

        memcpy(ptr->DispItem[7].subItem[2].outputDevice, "Speaker", sizeof("Speaker"));
#ifdef USING_EXTAMP_HP
        ptr->DispItem[7].subItem[2].AnalogPoint = mAudioHWVolumeCapabilityInstance->getDevicePath(NORMAL_MODE, CAPABILITY_DEVICE_SPEAKER, false);
#else
        ptr->DispItem[7].subItem[2].AnalogPoint = mAudioHWVolumeCapabilityInstance->getDevicePath(NORMAL_MODE, CAPABILITY_DEVICE_SPEAKER, true);
#endif
    }
    break;
    case AUDIO_STREAM_DTMF: {
        memcpy(ptr->DispItem[8].subItem[1].outputDevice, "Headset", sizeof("Headset"));
        ptr->DispItem[8].subItem[1].AnalogPoint = mAudioHWVolumeCapabilityInstance->getDevicePath(NORMAL_MODE, CAPABILITY_DEVICE_HEADPHONE);

        memcpy(ptr->DispItem[8].subItem[2].outputDevice, "Speaker", sizeof("Speaker"));
#ifdef USING_EXTAMP_HP
        ptr->DispItem[8].subItem[2].AnalogPoint = mAudioHWVolumeCapabilityInstance->getDevicePath(NORMAL_MODE, CAPABILITY_DEVICE_SPEAKER, false);
#else
        ptr->DispItem[8].subItem[2].AnalogPoint = mAudioHWVolumeCapabilityInstance->getDevicePath(NORMAL_MODE, CAPABILITY_DEVICE_SPEAKER, true);
#endif
    }
    break;
    case AUDIO_STREAM_TTS: {
        memcpy(ptr->DispItem[9].subItem[1].outputDevice, "Headset", sizeof("Headset"));
        ptr->DispItem[9].subItem[1].AnalogPoint = mAudioHWVolumeCapabilityInstance->getDevicePath(NORMAL_MODE, CAPABILITY_DEVICE_HEADPHONE);

        memcpy(ptr->DispItem[9].subItem[2].outputDevice, "Speaker", sizeof("Speaker"));
#ifdef USING_EXTAMP_HP
        ptr->DispItem[9].subItem[2].AnalogPoint = mAudioHWVolumeCapabilityInstance->getDevicePath(NORMAL_MODE, CAPABILITY_DEVICE_SPEAKER, false);
#else
        ptr->DispItem[9].subItem[2].AnalogPoint = mAudioHWVolumeCapabilityInstance->getDevicePath(NORMAL_MODE, CAPABILITY_DEVICE_SPEAKER, true);
#endif
    }
    break;
    default:
        break;
    }

    ALOGD("-%s()", __FUNCTION__);
    return NO_ERROR;
}

status_t AudioALSAParamTuner::getPCDispMic(void *pParam) {
    PCDispTotolStru *ptr = (PCDispTotolStru *)pParam;

    memcpy(ptr->DispMic.strType, "Mic", sizeof("Mic"));
    ptr->DispMic.subItemNum = 0xF0FFFF;
    memcpy(ptr->DispMic.subItem, subItemMic, sizeof(ptr->DispMic.subItem));

    return NO_ERROR;
}
#ifdef MTK_AUDIO_GAIN_TABLE_BT
status_t AudioALSAParamTuner::getBtNrecInfoForDisp(void *pParam) {
    PCDispItem *ptr = (PCDispItem *)pParam;

    //for DispItem BT NREC
    memcpy(ptr->strType, "Bluetooth_nrec", sizeof("Bluetooth_nrec"));
    ptr->level = mAudioHWVolumeCapabilityInstance->getStreamLevel((int(AUDIO_STREAM_BLUETOOTH_SCO)));
    ptr->subItemNum = 0x6;

    memcpy(ptr->subItem[1].outputDevice, "Headset", sizeof("Headset"));
    ptr->subItem[1].AnalogPoint = mAudioHWVolumeCapabilityInstance->getDevicePath(NORMAL_MODE, CAPABILITY_DEVICE_HEADPHONE);

    memcpy(ptr->subItem[2].outputDevice, "Speaker", sizeof("Speaker"));
#ifdef USING_EXTAMP_HP
    ptr->subItem[2].AnalogPoint = mAudioHWVolumeCapabilityInstance->getDevicePath(NORMAL_MODE, CAPABILITY_DEVICE_SPEAKER, false);
#else
    ptr->subItem[2].AnalogPoint = mAudioHWVolumeCapabilityInstance->getDevicePath(NORMAL_MODE, CAPABILITY_DEVICE_SPEAKER, true);
#endif

    return NO_ERROR;
}
#endif
#endif

status_t AudioALSAParamTuner::setSphVolume(uint32_t mode, uint32_t gain) {
    ALOGV("+%s(), speech mode=0x%x, gain=0x%x", __FUNCTION__, mode, gain);
    int32_t degradeDb = (DEVICE_VOLUME_STEP - MapVoiceVolumetoCustom(gain)) / VOICE_ONEDB_STEP;
    int voiceAnalogRange = DEVICE_MAX_VOLUME - DEVICE_MIN_VOLUME;

    switch (mode) {
    case SPEECH_MODE_NORMAL:
        if (degradeDb <= AUDIO_BUFFER_HW_GAIN_STEP) {
            mAudioALSAVolumeController->SetReceiverGain(degradeDb);
            mAudioALSAVolumeController->ApplyMdDlGain(0);
        } else {
            mAudioALSAVolumeController->SetReceiverGain(voiceAnalogRange);
            degradeDb -= voiceAnalogRange;
            mAudioALSAVolumeController->ApplyMdDlGain(degradeDb);
        }
        mAudioALSAVolumeController->ApplyMicGain(Normal_Mic, AUDIO_MODE_IN_CALL);
        break;
    case SPEECH_MODE_EARPHONE:
        if (degradeDb <= AUDIO_BUFFER_HW_GAIN_STEP) {
            mAudioALSAVolumeController->SetHeadPhoneRGain(degradeDb);
            mAudioALSAVolumeController->SetHeadPhoneLGain(degradeDb);
            mAudioALSAVolumeController->ApplyMdDlGain(0);
        } else {
            mAudioALSAVolumeController->SetHeadPhoneRGain(voiceAnalogRange);
            mAudioALSAVolumeController->SetHeadPhoneLGain(voiceAnalogRange);
            degradeDb -= voiceAnalogRange;
            mAudioALSAVolumeController->ApplyMdDlGain(degradeDb);
        }
        mAudioALSAVolumeController->ApplyMicGain(Headset_Mic, AUDIO_MODE_IN_CALL);
        break;
    case SPEECH_MODE_LOUD_SPEAKER:
        if (degradeDb <= AUDIO_BUFFER_HW_GAIN_STEP) {
#ifdef USING_EXTAMP_HP
            mAudioALSAVolumeController->SetHeadPhoneRGain(degradeDb);
            mAudioALSAVolumeController->SetHeadPhoneLGain(degradeDb);
#else
            mAudioALSAVolumeController->SetSpeakerGain(degradeDb);

#endif
            mAudioALSAVolumeController->ApplyMdDlGain(0);
        } else {
            voiceAnalogRange = DEVICE_AMP_MAX_VOLUME - DEVICE_AMP_MIN_VOLUME;
#ifdef USING_EXTAMP_HP
            mAudioALSAVolumeController->SetHeadPhoneRGain(voiceAnalogRange);
            mAudioALSAVolumeController->SetHeadPhoneLGain(voiceAnalogRange);
#else
            mAudioALSAVolumeController->SetSpeakerGain(voiceAnalogRange);
#endif
            degradeDb -= voiceAnalogRange;
            mAudioALSAVolumeController->ApplyMdDlGain(degradeDb);
        }
        mAudioALSAVolumeController->ApplyMicGain(Handfree_Mic, AUDIO_MODE_IN_CALL);
        break;
    default:
        break;
    }
    return NO_ERROR;
}
};

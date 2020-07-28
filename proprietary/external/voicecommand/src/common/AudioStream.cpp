
/*******************************************************************************
 *
 * Filename:
 * ---------
 * AudioStream.cpp
 *
 * Project:
 * --------
 *   Android
 *
 * Description:
 * ------------
 *   The class to get uplink and downlink PCM data.
 *
 * Author:
 * -------
 *   Donglei Ji(mtk80823)
 *
 *******************************************************************************/

#define MTK_LOG_ENABLE 1
#include <utils/Errors.h>
#include <utils/String16.h>
#include <cutils/log.h>
#include <cutils/properties.h>
#include <media/AudioSystem.h>
#include <media/stagefright/MetaData.h>
#include <sys/types.h>  /* mkdir use */
#include <sys/stat.h>  /* mkdir use */
#include "AudioStream.h"

static int DROP_THRESHOLD = 6400;  /*100ms for 16k sample rate stereo*/
using namespace android;

#define DL_BUF_SIZE 2048//640  // 20ms for 16k sample rate mono
#define UL_BUF_SIZE (BUF_READ_SIZE * 2)  //40ms *2ch
#define DUMP_FILE_MAX_NUM  50

#ifdef LOG_TAG
#undef LOG_TAG
#endif
#define LOG_TAG "AudioStream"

int AudiocheckAndCreateDirectory(const char *pC)
{
    char tmp[PATH_MAX];
    int i = 0;
    while (*pC) {
        tmp[i] = *pC;
        if (*pC == '/' && i) {
            tmp[i] = '\0';
            if (access(tmp, F_OK) != 0) {
                if (mkdir(tmp, 0770) == -1) {
                    ALOGE("AudioDumpPCM: mkdir error! %s\n", (char *)strerror(errno));
                    return -1;
                }
            }
            tmp[i] = '/';
        }
        i++;
        pC++;
    }
    return 0;
}


AudioStream::AudioStream(audio_source_t inputSource, uint32_t sampleRate, uint32_t channelCount, audio_session_t session_id) :
    m_bStarted(false),
    m_InputSource(inputSource),
    m_SampleRate(sampleRate),
    m_ChannelCount(channelCount),
#if HOTWORD_TRAINING
    m_SessionId(session_id),
#else
    m_SessionId(AUDIO_SESSION_ALLOCATE),
#endif
    mMaxAmplitude_L(0),
    mMaxAmplitude_R(0),
    m_ChannelMask(0)
{
    ALOGD("AudioStream construct in +");
    ALOGD("input source:%d, session_id:%d, sampe rate:%d, Channel count:%d", inputSource, session_id, sampleRate, channelCount);
    ALOGV("m_InputSource:%d, m_SessionId:%d, m_SampleRate:%d,m_ChannelCount:%d", (unsigned int)m_InputSource, (unsigned int)m_SessionId, m_SampleRate, m_ChannelCount);
    m_ULAudioBuf = NULL;
    memset(m_BackupFilePath, 0, FILE_NAME_LEN_MAX);
    m_RecorderState = VOICE_RECORDER_UNINITIALIZE;
    if (sampleRate != 16000) {
        ALOGE("Invalid sampling rate %d for voice recognition", sampleRate);
        return;
    }

    if (channelCount != 1 && channelCount != 2) {
        ALOGE("Invalid number of audio channels %d for voice recognition", channelCount);
        return;
    }
#if HOTWORD_TRAINING
    if (inputSource!=AUDIO_SOURCE_HOTWORD) {
#else
    if (inputSource!=AUDIO_SOURCE_UNPROCESSED) {
#endif
        ALOGE("Invalid audio source: %d", inputSource);
        return;
    }

    if (channelCount == 1) {
        DROP_THRESHOLD /= 2;
    }

    if (channelCount == 2) {
        m_ChannelMask = AUDIO_CHANNEL_IN_STEREO;
    } else if (channelCount == 1) {
        m_ChannelMask = AUDIO_CHANNEL_IN_MONO;
    } else {
        ALOGE("Invalid audio channel: %d", channelCount);
        return;
    }

    m_pAudioRecord = new AudioRecord(m_InputSource,                           // inputSource
                                     m_SampleRate,                            // sampleRate
                                     AUDIO_FORMAT_PCM_16_BIT,                 // format
                                     m_ChannelMask,                           // channelMask
                                     String16("com.mediatek.voicecommand"),   // opPackageName
                                     0,                                       // frameCount
                                     NULL,                                    // cbf
                                     NULL,                                    // user
                                     0,                                       // noticationFrames
                                     m_SessionId                              // sessionId
                                    );
    if (m_pAudioRecord->initCheck() != OK) {
        ALOGE("error!!, create uplink stream fail");
        m_pAudioRecord.clear();
    }

    m_fp_UL = NULL;
    m_fp_uplink1 = NULL;
    m_fp_uplink2 = NULL;
    m_fp_BackupUL = NULL;

    m_RecorderState = VOICE_RECORDER_INITIALIZED;
}

AudioStream::~AudioStream()
{
    ALOGD("~AudioStream +");
    status_t err = OK;
    Mutex::Autolock autoLock(m_Lock);
    if (m_RecorderState != VOICE_RECORDER_INITIALIZED) {
        ALOGD("m_RecorderState(%d) not in INIT, call stop()", m_RecorderState);
        err= stop();
        ALOGD("m_RecorderState(%d) not in INIT, err status(%d)", err);
    }
    if (m_pAudioRecord!=0) {
        ALOGD("m_pAudioRecord.clear +");
        m_pAudioRecord.clear();
        ALOGD("m_pAudioRecord.clear -");
    }
    if (m_ULAudioBuf != NULL) {
        ALOGV("2.m_ULAudioBuf delete +");
        delete [] m_ULAudioBuf;
        ALOGV("2.m_ULAudioBuf delete -");
        m_ULAudioBuf = NULL;
        ALOGV("2.m_ULAudioBuf = NULL");
    }
    ALOGD("~AudioStream -");
}

status_t AudioStream::enterBackupFilePath(char *pFilePath)
{
    if (pFilePath == NULL) {
        ALOGE("%s() path is null!!", __func__);
        return NO_INIT;
    }

    strncpy(m_BackupFilePath, pFilePath, FILE_NAME_LEN_MAX-1);
    ALOGE("%s() path: %s", __func__, m_BackupFilePath);
    return OK;
}

#define DUMP_PCM_DATA_PATH "/sdcard/mtklog/audio_dump/VowTraining"
status_t AudioStream::openPCMDataFile()
{
    char string_time[16];
    char filename_ul1[] = "VR_Uplink1.pcm";
    char filename_ul2[] = "VR_Uplink2.pcm";
    char filename_ul_raw[] = "VR_UL_Raw.pcm";
    char filename_update_ul[] = "training_ul.pcm";
    char filename[100];
    char buffer[4];
    char value[PROPERTY_VALUE_MAX];
    property_get("vendor.streamin.pcm.dump", value, "0");
    int bflag = atoi(value);

    if (bflag) {
        struct timeval now;
        gettimeofday(&now,NULL);

        //get a randam number
        memset(string_time, '\0', 16);
        sprintf(string_time, "%.2lu_%.2lu_%.2lu_%.3lu",
		(now.tv_sec / 3600) % (24),
		(now.tv_sec / 60) % (60),
		(now.tv_sec % 60),
		(now.tv_usec / 1000));

	sprintf(filename, "%s/%s_%s",
		DUMP_PCM_DATA_PATH, string_time, filename_ul1);
        ALOGI("%s, filename=%s", __func__, filename);
        AudiocheckAndCreateDirectory(filename);
        m_fp_uplink1 = fopen(filename, "wb+");
        if (NULL == m_fp_uplink1) {
            ALOGI("%s, fopen m_fp_uplink1 errno:%d", __func__, errno);
        }

        sprintf(filename, "%s/%s_%s",
		DUMP_PCM_DATA_PATH, string_time, filename_ul2);
        AudiocheckAndCreateDirectory(filename);
        ALOGI("%s, filename=%s", __func__, filename);
        m_fp_uplink2 = fopen(filename, "wb+");
        if (NULL == m_fp_uplink2) {
            ALOGI("%s, fopen m_fp_uplink2 errno:%d", __func__, errno);
        }

        sprintf(filename, "%s/%s_%s",
                DUMP_PCM_DATA_PATH, string_time, filename_ul_raw);
        AudiocheckAndCreateDirectory(filename);
        ALOGI("%s, filename=%s", __func__, filename);
        m_fp_UL = fopen(filename, "wb+");
        if (NULL == m_fp_UL) {
            ALOGI("%s, fopen m_fp_UL errno:%d", __func__, errno);
        }
    }
    //if (1) {
        sprintf(filename, "%s%s", m_BackupFilePath, filename_update_ul);
        AudiocheckAndCreateDirectory(filename);
        ALOGI("%s, filename=%s", __func__, filename);
        m_fp_BackupUL = fopen(filename, "wb+");
        if (NULL == m_fp_BackupUL) {
            ALOGI("%s, fopen m_fp_BackupUL errno:%d", __func__, errno);
        }
    //}
    return OK;
}

status_t AudioStream::closePCMDataFile()
{
    if (m_fp_uplink1)
        fclose(m_fp_uplink1);
    if (m_fp_uplink2)
        fclose(m_fp_uplink2);
    if (m_fp_UL)
        fclose(m_fp_UL);
    if (m_fp_BackupUL)
        fclose(m_fp_BackupUL);
    return OK;
}

status_t AudioStream::dumpPCMData(const char* pStr, void * buffer, int count)
{
    FILE *fp = NULL;

    if (strcmp(pStr, "uplink1") == 0) {
        fp = m_fp_uplink1;
    } else if (strcmp(pStr, "uplink2") == 0) {
        fp = m_fp_uplink2;
    } else if (strcmp(pStr, "uplink-raw") == 0) {
        fp = m_fp_UL;
    } else if (strcmp(pStr, "backup_ul") == 0) {
        fp = m_fp_BackupUL;
    } else {
        ALOGI("%s, no match string", __func__);
    }

    if (fp != NULL) {
        fwrite(buffer, count, 1, fp);
    }
    return OK;
}


status_t AudioStream::initCheck()
{
    return m_pAudioRecord == 0 ? NO_INIT : OK;
}

int32_t AudioStream::latency()
{
    return 0;
}

status_t AudioStream::dumpAudioData(short *pULBuf1, short *pULBuf2, uint32_t size)
{
    dumpPCMData("uplink1", (void *)pULBuf1, size);
    if (m_ChannelCount == 2) {
        dumpPCMData("uplink2", (void *)pULBuf2, size);
    }
    dumpPCMData("backup_ul", (void *)pULBuf1, size);
    return OK;
}

status_t AudioStream::start()
{
    Mutex::Autolock autoLock(m_Lock);

    if (m_RecorderState!=VOICE_RECORDER_INITIALIZED) {
        ALOGE("the recorder state is not match, state: %d", m_RecorderState);
        return INVALID_OPERATION;
    }

    if (initCheck()!=OK)
        return NO_INIT;
    if (m_ULAudioBuf == NULL) {
        m_ULAudioBuf = new short[UL_BUF_SIZE];
    }
    if (m_ULAudioBuf == NULL) {
        ALOGD("error!!, m_ULAudioBuf == NULL");
        return NO_INIT;
    }
    memset(m_ULAudioBuf, 0, UL_BUF_SIZE * sizeof(short));

    openPCMDataFile();
    status_t err = m_pAudioRecord->start();
    if (err != OK) {
        ALOGE("start audio source fail %d", err);
        closePCMDataFile();
        return err;
    }

    m_bStarted = true;

    m_RecorderState = VOICE_RECORDER_STARTED;
    return OK;
}

status_t AudioStream::stop()
{
    ALOGD("Stop in +");
    {
        Mutex::Autolock autoLock(m_Lock);
        if (m_RecorderState!=VOICE_RECORDER_STARTED) {
            ALOGE("the recorder state is not match, state: %d", m_RecorderState);
            return INVALID_OPERATION;
        }

        m_bStarted = false;
    }

    Mutex::Autolock autoLock(m_Lock);
    if (initCheck()!=OK)
        return NO_INIT;

    m_pAudioRecord->stop();
    closePCMDataFile();
    m_RecorderState = VOICE_RECORDER_INITIALIZED;
    if (m_ULAudioBuf != NULL) {
        ALOGV("1.m_ULAudioBuf delete +");
        delete [] m_ULAudioBuf;
        ALOGV("1.m_ULAudioBuf delete -");
        m_ULAudioBuf = NULL;
        ALOGV("2.m_ULAudioBuf = NULL");
    }
    ALOGD("Stop in -");
    return OK;
}

status_t AudioStream::readPCM(short *pULBuf1, short *pULBuf2, short *pDLBuf, uint32_t size)
{
    status_t err = OK;
    size_t ulOffset = 0;

    size = size*2; //sample to byte
    size_t ulDataNeeded = size*m_ChannelCount;

    (void)pDLBuf; // no-op use of pDLBuf

    Mutex::Autolock autoLock(m_Lock);
    ALOGV("readPCM obtain lock~~");

    if (m_RecorderState!=VOICE_RECORDER_STARTED) {
        ALOGE("readPCM the recorder state is not match, state: %d", m_RecorderState);
        return INVALID_OPERATION;
    }

    while (ulDataNeeded > 0) {
        if (ulDataNeeded > 0) {
            Mutex::Autolock autoLock(m_ULBufLock);
            ALOGV("+readPCM read data, ch=%d, size=%d", m_ChannelCount, size);
            err = m_pAudioRecord->read(m_ULAudioBuf, ulDataNeeded, true);
            ALOGV("-readPCM read data - size:%d", err);
            if (err < 0) {
                ALOGD("readPCM err: %d", err);
                return err;
            }
            dumpPCMData("uplink-raw", (void *)m_ULAudioBuf, ulDataNeeded);

            ALOGV("readPCM read data, ulDataNeeded:%d", (unsigned int)ulDataNeeded);

            extractULData(pULBuf1+ulOffset, pULBuf2+ulOffset, m_ULAudioBuf, ulDataNeeded);
            ulDataNeeded = 0;

        }
    }

    ALOGV("readPCM obtain lock - out -");

    return OK;
}

status_t AudioStream::reset()
{
    ALOGD("reset in +");
    status_t err = OK;
    err = stop();

    if (m_pAudioRecord!=0) {
        m_Lock.lock();
        m_pAudioRecord.clear();
        m_pAudioRecord = 0;
        m_Lock.unlock();
    }

    usleep(20000);

    if (err == OK) {
        m_Lock.lock();
        m_pAudioRecord = new AudioRecord(m_InputSource,                           // inputSource
                                         m_SampleRate,                            // sampleRate
                                         AUDIO_FORMAT_PCM_16_BIT,                 // format
                                         m_ChannelMask,                           // channelMask
                                         String16("com.mediatek.voicecommand"),   // opPackageName
                                         0,                                       // frameCount
                                         NULL,                                    // cbf
                                         NULL,                                    // user
                                         0,                                       // noticationFrames
                                         m_SessionId                              // sessionId
                                        );
        m_Lock.unlock();
        if (m_pAudioRecord == 0) {
            ALOGE("reset - create uplink stream fail");
            return UNKNOWN_ERROR;
        }
        err = start();
    }

    ALOGD("reset in -");
    return err;
}

int16_t AudioStream::getMaxAmplitude(int Channel_select)
{
    int16_t pcmAmp = 0;

    if (m_pAudioRecord != 0) {
        if (Channel_select == 0) {
            pcmAmp = mMaxAmplitude_L;
        } else {
            pcmAmp = mMaxAmplitude_R;
        }
    }
    mMaxAmplitude_L = 0;
    mMaxAmplitude_R = 0;

    return pcmAmp;
}

void AudioStream::extractULData(short *pULBuf1, short *pULBuf2, short *data, int len)
{
    int16_t value_L;
    int16_t value_R;

    if (m_ChannelCount == 1) {
        memcpy((void *)pULBuf1, (void *)data, len);
    } else if (m_ChannelCount == 2) {
        len /= 4;
        int i = 0;
        while (len--) {
            pULBuf1[i] = data[2 * i];
            pULBuf2[i] = data[2 * i + 1];

            // find max amplitude value
            value_L = pULBuf1[i];
            value_R = pULBuf2[i];
            if (value_L < 0) {
                value_L = -value_L;
            }
            if (value_R < 0) {
                value_R = -value_R;
            }
            if (mMaxAmplitude_L < value_L) {
                mMaxAmplitude_L = value_L;
            }
            if (mMaxAmplitude_R < value_R) {
                mMaxAmplitude_R = value_R;
            }

            i++;
        }
    }
}

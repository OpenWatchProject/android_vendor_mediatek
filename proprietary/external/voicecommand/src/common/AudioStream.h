
/*******************************************************************************
 *
 * Filename:
 * ---------
 * AudioStream.h
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

 #ifndef _AUDIO_STREAM_H_
 #define _AUDIO_STREAM_H_

#include<utils/RefBase.h>
#include <sys/types.h>

#include<system/audio.h>
#include<media/stagefright/AudioSource.h>
#include<media/stagefright/MediaBuffer.h>
#include <media/AudioSystem.h>
#include <media/AudioRecord.h>

using namespace android;

#define HOTWORD_TRAINING 1
#define FILE_NAME_LEN_MAX 256
#define BUF_READ_SIZE (160 * 4) //10msec * 4

enum {
    kKeyDLTSS   = 1, //int64_t
    kKeyDLTSNS
};

#define BUFFERS_NUM_MAX 5

enum voice_recorder_state {
    VOICE_RECORDER_UNINITIALIZE = -1,
    VOICE_RECORDER_INITIALIZED,
    VOICE_RECORDER_STARTED
};

class AudioStream: virtual public RefBase
{
public:
    AudioStream(audio_source_t inputSource=AUDIO_SOURCE_UNPROCESSED, uint32_t sampleRate=16000, uint32_t channelCount=1, audio_session_t session_id=AUDIO_SESSION_ALLOCATE);
    ~AudioStream();

    status_t initCheck();
    int32_t latency();
    status_t start();
    status_t stop();
    status_t readPCM(short *pULBuf1, short *pULBuf2, short *pDLBuf, uint32_t size);
    status_t reset();
    int16_t getMaxAmplitude(int Channel_select);

    status_t dumpAudioData(short *ULBuf1, short *ULBuf2, uint32_t size);
    status_t openPCMDataFile();
    status_t closePCMDataFile();
    status_t dumpPCMData(const char* pStr, void * buffer, int count);
    status_t enterBackupFilePath(char *pFilePath);

private:
    sp<AudioRecord> m_pAudioRecord;
    voice_recorder_state m_RecorderState;

    bool m_bStarted;

    audio_source_t m_InputSource;
    uint32_t m_SampleRate;
    uint32_t m_ChannelCount;

    Mutex m_Lock;
    Mutex m_ULBufLock;

    void extractULData(short *pULBuf1, short *pULBuf2, short *data, int len);

    short *m_ULAudioBuf;
    audio_channel_mask_t m_ChannelMask;
    audio_session_t m_SessionId;

    FILE *m_fp_UL = NULL;
    FILE *m_fp_uplink1 = NULL;
    FILE *m_fp_uplink2 = NULL;
    FILE *m_fp_BackupUL = NULL;

    int16_t mMaxAmplitude_L = 0;
    int16_t mMaxAmplitude_R = 0;

    char m_BackupFilePath[FILE_NAME_LEN_MAX];
};
 #endif


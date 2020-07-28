
/*******************************************************************************
 *
 * Filename:
 * ---------
 * VoiceCmdRecognition.h
 *
 * Project:
 * --------
 *   Android
 *
 * Description:
 * ------------
 *   The voice command handler APIs.
 *
 * Author:
 * -------
 *   Donglei Ji(mtk80823)
 *
 *******************************************************************************/

#ifndef _VOICE_CMD_RECOGNITION_H_
#define _VOICE_CMD_RECOGNITION_H_

#include<pthread.h>
#include <stdint.h>

#include "../common/AudioStream.h"
#include "voiceunlock2/include/VowEngine_training.h"

using namespace android;

#define ID_RIFF 0x46464952
#define ID_WAVE 0x45564157
#define ID_FMT  0x20746d66
#define ID_DATA 0x61746164

#define FORMAT_PCM 1

#define FILE_NAME_LEN_MAX 256
#define PCM_FILE_GAIN 16      /* boost 24dB when writing waveform for voicecommand playing */
#define BACKGROUND_RECORD_SAMPLE_CNT 100  /* 10ms * 100 = 1s */

#define FRAMEWORK_VOW_RECOG_VER "1.0.6"


#define UPPER_BOUND(in,up)      ((in) > (up) ? (up) : (in))
#define LOWER_BOUND(in,lo)      ((in) < (lo) ? (lo) : (in))
#define BOUNDED(in,up,lo)       ((in) > (up) ? (up) : (in) < (lo) ? (lo) : (in))
#define MAXIMUM(a,b)            ((a) > (b) ? (a) : (b))
#define MINIMUM(a,b)            ((a) < (b) ? (a) : (b))

struct ring_buffer_information {
    uint32_t write_pointer;
    uint32_t read_pointer;
    uint32_t buffer_byte_count;
    uint8_t *buffer_base_pointer;
};

/*
Function Name:
    ring_buffer_get_data_byte_count
Return Type:
    uint32_t
Parameters:
    struct ring_buffer_information *p_info
Function Content with Full Assertion Protection:
    uint32_t buffer_byte_count;
    uint32_t write_pointer;
    uint32_t read_pointer;
    uint32_t data_byte_count;
    // ASSERT_IF_FALSE(p_info != NULL);
    buffer_byte_count = p_info->buffer_byte_count;  // ASSERT_IF_FALSE(buffer_byte_count < 0x80000000);
    write_pointer     = p_info->write_pointer;      // ASSERT_IF_FALSE(write_pointer < (buffer_byte_count << 1));
    read_pointer      = p_info->read_pointer;       // ASSERT_IF_FALSE(read_pointer < (buffer_byte_count << 1));
    if (write_pointer >= read_pointer) {
        data_byte_count = write_pointer - read_pointer;
    } else { // write_pointer < read_pointer
        data_byte_count = (buffer_byte_count << 1) - read_pointer + write_pointer;
    }
    // ASSERT_IF_FALSE(data_byte_count <= buffer_byte_count);
    return data_byte_count;
Function Name:
    ring_buffer_get_space_byte_count
Return Type:
    uint32_t
Parameters:
    struct ring_buffer_information *p_info
Function Content with Full Assertion Protection:
    uint32_t buffer_byte_count;
    uint32_t data_byte_count;
    uint32_t space_byte_count;
    // ASSERT_IF_FALSE(p_info != NULL);
    buffer_byte_count = p_info->buffer_byte_count;                  // ASSERT_IF_FALSE(buffer_byte_count < 0x80000000);
    data_byte_count   = ring_buffer_get_data_byte_count(p_info);    // ASSERT_IF_FALSE(data_byte_count <= buffer_byte_count);
    space_byte_count  = buffer_byte_count - data_byte_count;
    return space_byte_count;
Function Name:
    ring_buffer_get_write_information
Return Type:
    void
Parameters:
    struct ring_buffer_information *p_info
    uint8_t **pp_buffer
    uint32_t *p_byte_count
Function Content with Full Assertion Protection:
    uint32_t buffer_byte_count;
    uint32_t space_byte_count;
    uint8_t *buffer_pointer;
    uint32_t write_pointer;
    uint32_t read_pointer;
    uint32_t tail_byte_count;
    uint32_t write_byte_cnt;
    // ASSERT_IF_FALSE(p_info != NULL);
    // ASSERT_IF_FALSE(pp_buffer != NULL);
    // ASSERT_IF_FALSE(p_byte_count != NULL);
    buffer_byte_count = p_info->buffer_byte_count;                  // ASSERT_IF_FALSE(buffer_byte_count < 0x80000000);
    space_byte_count  = ring_buffer_get_space_byte_count(p_info);   // ASSERT_IF_FALSE(space_byte_count <= buffer_byte_count);
    buffer_pointer    = p_info->buffer_base_pointer;                // ASSERT_IF_FALSE(buffer_pointer != NULL);
    write_pointer     = p_info->write_pointer;                      // ASSERT_IF_FALSE(write_pointer < (buffer_byte_count << 1));
    read_pointer      = p_info->read_pointer;                       // ASSERT_IF_FALSE(read_pointer < (buffer_byte_count << 1));
    if (write_pointer < buffer_byte_count) {    // 1st mirror part
        buffer_pointer += write_pointer;
        tail_byte_count = buffer_byte_count - write_pointer;
    } else {                                    // 2nd mirror part
        buffer_pointer += write_pointer - buffer_byte_count;
        tail_byte_count = (buffer_byte_count << 1) - write_pointer;
    }
    write_byte_cnt = MINIMUM(data_byte_count, tail_byte_count);
    *pp_buffer = buffer_pointer;
    *p_byte_count = write_byte_cnt;
    return;
Function Name:
    ring_buffer_get_read_information
Return Type:
    void
Parameters:
    struct ring_buffer_information *p_info
    uint8_t **pp_buffer
    uint32_t *p_byte_count
Function Content with Full Assertion Protection:
    uint32_t buffer_byte_count;
    uint32_t data_byte_count;
    uint8_t *buffer_pointer;
    uint32_t write_pointer;
    uint32_t read_pointer;
    uint32_t tail_byte_count;
    uint32_t read_byte_cnt;
    // ASSERT_IF_FALSE(p_info != NULL);
    // ASSERT_IF_FALSE(pp_buffer != NULL);
    // ASSERT_IF_FALSE(p_byte_count != NULL);
    buffer_byte_count = p_info->buffer_byte_count;                  // ASSERT_IF_FALSE(buffer_byte_count < 0x80000000);
    data_byte_count   = ring_buffer_get_data_byte_count(p_info);    // ASSERT_IF_FALSE(data_byte_count <= buffer_byte_count);
    buffer_pointer    = p_info->buffer_base_pointer;                // ASSERT_IF_FALSE(buffer_pointer != NULL);
    write_pointer     = p_info->write_pointer;                      // ASSERT_IF_FALSE(write_pointer < (buffer_byte_count << 1));
    read_pointer      = p_info->read_pointer;                       // ASSERT_IF_FALSE(read_pointer < (buffer_byte_count << 1));
    if (read_pointer < buffer_byte_count) { // 1st mirror part
        buffer_pointer += read_pointer;
        tail_byte_count = buffer_byte_count - read_pointer;
    } else {                                // 2nd mirror part
        buffer_pointer += read_pointer - buffer_byte_count;
        tail_byte_count = (buffer_byte_count << 1) - read_pointer;
    }
    read_byte_cnt = MINIMUM(data_byte_count, tail_byte_count);
    *pp_buffer = buffer_pointer;
    *p_byte_count = read_byte_cnt;
    return;
Function Name:
    ring_buffer_write_done
Return Type:
    void
Parameters:
    struct ring_buffer_information *p_info
    uint32_t write_byte_count
Function Content with Full Assertion Protection:
    uint32_t buffer_byte_count;
    uint32_t buffer_end;
    // uint32_t space_byte_count;
    uint32_t write_pointer;
    // uint32_t tail_byte_count
    // ASSERT_IF_FALSE(p_info != NULL);
    buffer_byte_count = p_info->buffer_byte_count;                  // ASSERT_IF_FALSE(buffer_byte_count < 0x80000000);
    buffer_end        = buffer_byte_count << 1;
    // space_byte_count  = ring_buffer_get_space_byte_count(p_info);   // ASSERT_IF_FALSE(space_byte_count <= buffer_byte_count);
    //                                                                 // ASSERT_IF_FALSE(write_byte_count <= space_byte_count);
    write_pointer = p_info->write_pointer; // ASSERT_IF_FALSE(write_pointer < buffer_end);
    // if (write_pointer < buffer_byte_count) { // 1st mirror part
    //     tail_byte_count = buffer_byte_count - write_pointer;
    // } else {                                // 2nd mirror part
    //     tail_byte_count = buffer_end - write_pointer;
    // }
    // ASSERT_IF_FALSE(write_byte_count <= tail_byte_count);
    write_pointer += write_byte_count;
    if (write_pointer >= buffer_end) {
        write_pointer -= buffer_end;
    }
    p_info->write_pointer = write_pointer;
    return;
Function Name:
    ring_buffer_read_done
Return Type:
    void
Parameters:
    struct ring_buffer_information *p_info
    uint32_t read_byte_count
Function Content with Full Assertion Protection:
    uint32_t buffer_byte_count;
    uint32_t buffer_end;
    // uint32_t data_byte_count;
    uint32_t read_pointer;
    // uint32_t tail_byte_count
    // ASSERT_IF_FALSE(p_info != NULL);
    buffer_byte_count = p_info->buffer_byte_count;                  // ASSERT_IF_FALSE(buffer_byte_count < 0x80000000);
    buffer_end        = buffer_byte_count << 1;
    // data_byte_count   = ring_buffer_get_data_byte_count(p_info);    // ASSERT_IF_FALSE(data_byte_count <= buffer_byte_count);
    //                                                                 // ASSERT_IF_FALSE(read_byte_count <= data_byte_count);
    read_pointer = p_info->read_pointer; // ASSERT_IF_FALSE(read_pointer < buffer_end);
    // if (read_pointer < buffer_byte_count) { // 1st mirror part
    //     tail_byte_count = buffer_byte_count - read_pointer;
    // } else {                                // 2nd mirror part
    //     tail_byte_count = buffer_end - read_pointer;
    // }
    // ASSERT_IF_FALSE(read_byte_count <= tail_byte_count);
    read_pointer += read_byte_count;
    if (read_pointer >= buffer_end) {
        read_pointer -= buffer_end;
    }
    p_info->read_pointer = read_pointer;
    return;
*/

struct wav_header {
	int riff_id;
	int riff_sz;
	int riff_fmt;
	int fmt_id;
	int fmt_sz;
	short audio_format;
	short num_channels;
	int sample_rate;
	int byte_rate;         /* sample_rate * num_channels * bps / 8 */
	short block_align;     /* num_channels * bps / 8 */
	short bits_per_sample;
	int data_id;
	int data_sz;
};

enum voice_recognition_mode {
    VOICE_IDLE_MODE = 0x00,
    VOICE_PW_TRAINING_MODE = 0x02,
    VOICE_PW_RETRAIN_MODE = 0x03,

    VOICE_RECOGNIZE_MODE_ALL = VOICE_PW_RETRAIN_MODE

};

enum voice_msg {
    VOICE_ERROR = -1,
    VOICE_TRAINING = 1,
    VOICE_TRAINING_PROCESS = 5,
    VOICE_TRAINING_FINISH = 6,
    VOICE_TRAINING_RETRAIN = 7
};

enum voice_training_process {
    VOICE_ON_START_RECORD = 1,
    VOICE_ON_STOP_RECORD = 2,
    VOICE_ON_START_PROCESS = 3,
    VOICE_ON_STOP_PROCESS = 4
};

enum voice_mode {
    VOICE_INVALID_MODE,
    VOICE_NORMAL_MODE,
    VOICE_HEADSET_MODE,
    VOICE_HANDFREE_MODE,

    VOICE_MODE_NUM_MAX
};

enum voice_wakeup {
    VOICE_UNLOCK,
    VOICE_WAKEUP_NO_RECOGNIZE,
    VOICE_WAKEUP_RECOGNIZE,

    VOICE_WAKE_UP_MODE_NUM
};

enum voice_retrain_result {
    VOICE_RETRAIN_FAIL = -1,
    VOICE_RETRAIN_SUCCESS = 0
};

class VoiceCmdRecognitionListener: virtual public RefBase
{
public:
    virtual void notify(int message, int ext1, int ext2, char **ext3 = NULL) = 0;
};

class AudioStream;
class VoiceCmdRecognition: virtual public RefBase
{
public:
    VoiceCmdRecognition(audio_source_t inputSource=AUDIO_SOURCE_UNPROCESSED, unsigned int sampleRate=16000, unsigned int channelCount=1);
    ~VoiceCmdRecognition();
    VoiceCmdRecognition(const VoiceCmdRecognition &);
    VoiceCmdRecognition & operator=(const VoiceCmdRecognition &);

    status_t initCheck();
    status_t setVoicePasswordFile(int fd, int64_t offset, int64_t length);
    status_t setVoicePatternFile(int fd, int64_t offset, int64_t length);
    status_t setVoicePatternFile(const char *path);
    status_t setVoiceUBMFile(const char *path);
    status_t setUpgradeVoiceUBMFile(const char *path);
    status_t setVoiceFeatureFile(int fd, int64_t offset, int64_t length);
    status_t setCommandId(int id);
    status_t setInputMode(int input_mode);

    /*for voice wakeup feature - SWIP in framework*/
    status_t setVoiceTrainingMode(int mode);
    status_t setVoiceWakeupInfoPath(const char * path);
    status_t setVoiceWakeupMode(int mode);
    status_t continueVoiceTraining();

    status_t pauseVoiceTraining();
    status_t getTrainigTimes(int *times);
    status_t setUtteranceTrainingTimeout(int msec);
    status_t setTrainingThreshold(int threshold);
    status_t getTrainingThreshold(int *threshold);

    status_t startCaptureVoice(unsigned int mode);
    status_t stopCaptureVoice(unsigned int mode);
    status_t startVoiceTraining();
    status_t setVoiceModelRetrain();
    status_t getVoiceIntensity(int *maxAmplitude);
    status_t setListener(const sp<VoiceCmdRecognitionListener>& listener);
    status_t voiceRecognitionRelease(unsigned int mode);
    void writeWavFile(const short *pBuf, int64_t length);
    void notify(int message, int ext1, int ext2, char **ext3 = NULL);
    void recordLockSignal();
    int recordLockWait(int delayMS = 0);

    uint32_t ring_buffer_get_data_byte_count(struct ring_buffer_information *p_info);
    uint32_t ring_buffer_get_space_byte_count(struct ring_buffer_information *p_info);
    void ring_buffer_get_write_information(struct ring_buffer_information *p_info, uint8_t **pp_buffer, uint32_t *p_byte_count);
    void ring_buffer_get_read_information(struct ring_buffer_information *p_info, uint8_t **pp_buffer, uint32_t *p_byte_count);
    void ring_buffer_write_done(struct ring_buffer_information *p_info, uint32_t write_byte_count);
    void ring_buffer_read_done(struct ring_buffer_information *p_info, uint32_t read_byte_count);

    sp<AudioStream> m_pAudioStream;

    unsigned int m_RecognitionMode;
    bool m_bStarted;
    bool m_bNeedToWait;
    bool m_bNeedToRelease;
    bool m_bSpecificRefMic;
    bool m_enrolling;
    bool m_onStartRecording;
    bool m_onStartProcessing;
    bool m_setTrainingThreshold;
    bool m_setTrainingTimeout;
    bool m_bStartRecordBackroundVoice;
    bool m_already_read;
    bool m_Training_Lib_Init;
    bool m_CaptureVoicePause;
    unsigned int m_BackroundVoiceCount;
    unsigned int m_trainingTimes;
    unsigned int m_trainingThreshold;
    unsigned int m_trainingTimeoutMsec;
    short *m_voiceULBuf1;
    short *m_voiceULBuf2;
    pthread_t m_ReadDataTid;
    pthread_mutex_t m_BufMutex;
    struct ring_buffer_information m_rb_info1;
    struct ring_buffer_information m_rb_info2;
    char notify_data[2];
private:
    status_t voiceRecognitionInit(unsigned int mode);
    status_t startAudioStream();

    sp<VoiceCmdRecognitionListener> m_pListener;

    pthread_mutex_t m_RecordMutex;
    pthread_cond_t m_RecordExitCond;

    struct wav_header m_WavHeader;
    audio_source_t m_InputSource;

    int m_SampleRate;
    int m_Channels;
    int m_PasswordFd;
    int m_PatternFd;
    int m_FeatureFd;
    int m_CommandId;
    int m_VoiceMode;
    int m_WakeupMode;

    pthread_t m_Tid;
    pthread_t m_enrollTid;

    char *m_pStrWakeupInfoPath;

    char m_strPatternPath[FILE_NAME_LEN_MAX];
    char m_strUBMPath[FILE_NAME_LEN_MAX];
    char m_strUpgradeUBMPath[FILE_NAME_LEN_MAX];

    audio_session_t m_Session;
    audio_io_handle_t m_IoHandle;
    audio_devices_t m_Device;
};
#endif

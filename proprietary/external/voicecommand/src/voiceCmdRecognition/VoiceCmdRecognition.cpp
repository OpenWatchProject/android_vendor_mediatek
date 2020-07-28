/*******************************************************************************
 *
 * Filename:
 * ---------
 * VoiceCmdRecognition.cpp
 *
 * Project:
 * --------
 *   Android
 *
 * Description:
 * ------------
 *   This file implements the  handling about voice recognition features.
 *
 * Author:
 * -------
 *   Donglei Ji (mtk80823)
 *
 *------------------------------------------------------------------------------
 *******************************************************************************/

/*=============================================================================
 *                              Include Files
 *===========================================================================*/
#include <cutils/log.h>
#include <utils/Errors.h>
#include <cutils/properties.h>
#include <sys/prctl.h>
#include <sys/resource.h>
#include <utils/threads.h>
#include "VoiceCmdRecognition.h"
#define MTK_LOG_ENABLE 1
#include "AudioToolkit.h"
#ifdef LOG_TAG
#undef LOG_TAG
#endif
#define LOG_TAG "VoiceCommandRecognition"

#define SAMPLES1 160  // 10ms for 16k sample rate and 1 channel
#define SAMPLES2 240  // 15ms for 16k sample rate and 1 channel
#define MAX_SAMPLE_LENGTH SAMPLES1 * 300  // 10ms * 200 = 2sec

#define kWaitingTimeOutMS 3000 // for wiat lock time out

#if VOW_PHASE25_SUPPORT
#define VOW_P25_UBM_PATH "/vendor/etc/vowphase25/training/ubmfile/"
#define VOW_P25_PATTERN_PATH "/sdcard/"
#define VOW_P25_DEBUG_PATH "/sdcard/"
#endif
#if VOW_SID_SUPPORT
#define VOW_P23_UBM_PATH "/vendor/etc/vowphase23/training/ubmfile/"
#define VOW_P23_PATTERN_PATH "/sdcard/0_vp23.dat"
#endif

using namespace android;

static __inline short ClipToShort(int x)
{
    int sign;

    /* clip to [-32768, 32767] */
    sign = x >> 31;
    if (sign != (x >> 15))
    x = sign ^ ((1 << 15) - 1);

    return (short)x;
}

String8 PrintEncodedString(String8 strKey, size_t len, void *ptr)
{
    String8 returnValue = String8("");
    size_t sz_Needed;
    size_t sz_enc;
    char *buf_enc = NULL;
    bool bPrint = false;

    //ALOGD("%s in, len = %d", __FUNCTION__, len);
    sz_Needed = Base64_OutputSize(true, len);
    buf_enc = new char[sz_Needed + 1];
    buf_enc[sz_Needed] = 0;

    sz_enc = Base64_Encode((unsigned char *)ptr, buf_enc, len);

    if (sz_enc != sz_Needed) {
        ALOGE("%s(), Encode Error!!!after encode (%s), len(%d), sz_Needed(%d), sz_enc(%d)",
            __FUNCTION__, buf_enc, (int)len, (int)sz_Needed, (int)sz_enc);
    } else {
        bPrint = true;
        //ALOGD("%s(), after encode (%s), len(%d), sz_enc(%d)", __FUNCTION__, buf_enc, len, sz_enc);
    }

    if (bPrint) {
        String8 StrVal = String8(buf_enc, sz_enc);
        returnValue += strKey;
        returnValue += StrVal;
        //returnValue += String8(";");
    }

    if (buf_enc != NULL) {
        delete[] buf_enc;
    }

    return returnValue;
}
/*
static status_t GetDecodedData(String8 strPara, size_t len, void *ptr)
{
    size_t sz_in = strPara.size();
    size_t sz_needed = Base64_OutputSize(false, sz_in);
    size_t sz_dec;
    status_t ret = NO_ERROR;

    if (sz_in <= 0) {
        return NO_ERROR;
    }

    //ALOGD("%s in, len = %d", __FUNCTION__, len);
    unsigned char *buf_dec = new unsigned char[sz_needed];
    sz_dec = Base64_Decode(strPara.string(), buf_dec, sz_in);

    if (sz_dec > sz_needed || sz_dec <= sz_needed - 3) {
        ALOGE("%s(), Decode Error!!!after decode (%s), sz_in(%d), sz_needed(%d), sz_dec(%d)",
            __FUNCTION__, buf_dec, (int)sz_in, (int)sz_needed, (int)sz_dec);
    } else {
        // sz_needed-3 < sz_dec <= sz_needed
        //ALOGD("%s(), after decode, sz_in(%d), sz_dec(%d) len(%d) sizeof(ret)=%d",
        //    __FUNCTION__, sz_in, sz_dec, len, sizeof(ret));
        //print_hex_buffer (sz_dec, buf_dec);
    }

    if ((len == 0) || (len == sz_dec-sizeof(ret))) {
       if (len) {
           ret = (status_t)*(buf_dec);
           unsigned char *buff = (buf_dec + 4);
           memcpy(ptr, buff, len);
       } else {
          const char * IntPtr = (char *)buf_dec;
          ret = atoi(IntPtr);
          //ALOGD("%s len = 0 ret(%d)", __FUNCTION__, ret);
       }
    } else {
       ALOGD("%s decoded buffer isn't right format", __FUNCTION__);
    }

    if (buf_dec != NULL) {
        delete[] buf_dec;
    }

    return ret;
}

status_t GetAudioData(int par1, size_t len, void *ptr)
{
    static String8 keyGetBuffer = String8("GetBuffer=");
    int iPara[2];
    iPara[0] = par1;
    iPara[1] = len;

    String8 strPara = PrintEncodedString(keyGetBuffer, sizeof(iPara), iPara);
    String8 returnValue = AudioSystem::getParameters(0, strPara);

    String8 newval; //remove "GetBuffer="
    newval.appendFormat("%s", returnValue.string() + keyGetBuffer.size());

    return GetDecodedData(newval, len, ptr);
}
*/
bool is_support_dual_mic()
{
    #if MTK_DUAL_MIC_SUPPORT
        return true;
    #else
        return false;
    #endif
}
/*
bool is_tablet_library()
{
    char value[PROPERTY_VALUE_MAX];
    property_get("ro.vendor.mtk_is_tablet", value, "0");
    int bflag=atoi(value);
    ALOGD("is_tablet_library:%d", bflag);

    return ((bflag == 1)?true:false);
}
*/

uint32_t ring_buffer_get_data_byte_count(struct ring_buffer_information *p_info)
{
    uint32_t buffer_byte_count = p_info->buffer_byte_count;
    uint32_t write_pointer     = p_info->write_pointer;
    uint32_t read_pointer      = p_info->read_pointer;
    uint32_t data_byte_count;
    if (write_pointer >= read_pointer) {
        data_byte_count = write_pointer - read_pointer;
    } else {
        data_byte_count = (buffer_byte_count << 1) - read_pointer + write_pointer;
    }
    return data_byte_count;
}

uint32_t ring_buffer_get_space_byte_count(struct ring_buffer_information *p_info)
{
    return p_info->buffer_byte_count - ring_buffer_get_data_byte_count(p_info);
}

void ring_buffer_get_write_information(struct ring_buffer_information *p_info, uint8_t **pp_buffer, uint32_t *p_byte_count)
{
    uint32_t buffer_byte_count = p_info->buffer_byte_count;
    uint32_t space_byte_count  = ring_buffer_get_space_byte_count(p_info);
    uint8_t *buffer_pointer    = p_info->buffer_base_pointer;
    uint32_t write_pointer     = p_info->write_pointer;
    uint32_t tail_byte_count;
    if (write_pointer < buffer_byte_count) {
        *pp_buffer = buffer_pointer + write_pointer;
        tail_byte_count = buffer_byte_count - write_pointer;
    } else {
        *pp_buffer = buffer_pointer + write_pointer - buffer_byte_count;
        tail_byte_count = (buffer_byte_count << 1) - write_pointer;
    }
    *p_byte_count = MINIMUM(space_byte_count, tail_byte_count);
    return;
}

void ring_buffer_get_read_information(struct ring_buffer_information *p_info, uint8_t **pp_buffer, uint32_t *p_byte_count)
{
    uint32_t buffer_byte_count = p_info->buffer_byte_count;
    uint32_t data_byte_count   = ring_buffer_get_data_byte_count(p_info);
    uint8_t *buffer_pointer    = p_info->buffer_base_pointer;
    uint32_t read_pointer      = p_info->read_pointer;
    uint32_t tail_byte_count;
    if (read_pointer < buffer_byte_count) {
        *pp_buffer = buffer_pointer + read_pointer;
        tail_byte_count = buffer_byte_count - read_pointer;
    } else {
        *pp_buffer = buffer_pointer + read_pointer - buffer_byte_count;
        tail_byte_count = (buffer_byte_count << 1) - read_pointer;
    }
    *p_byte_count = MINIMUM(data_byte_count, tail_byte_count);
    return;
}

void ring_buffer_write_done(struct ring_buffer_information *p_info, uint32_t write_byte_count)
{
    uint32_t buffer_byte_count = p_info->buffer_byte_count;
    uint32_t buffer_end        = buffer_byte_count << 1;
    uint32_t write_pointer     = p_info->write_pointer + write_byte_count;
    p_info->write_pointer = write_pointer >= buffer_end ? write_pointer - buffer_end : write_pointer;
    return;
}

void ring_buffer_read_done(struct ring_buffer_information *p_info, uint32_t read_byte_count)
{
    uint32_t buffer_byte_count = p_info->buffer_byte_count;
    uint32_t buffer_end        = buffer_byte_count << 1;
    uint32_t read_pointer      = p_info->read_pointer + read_byte_count;
    p_info->read_pointer = read_pointer >= buffer_end ? read_pointer - buffer_end : read_pointer;
    return;
}


static void *enrollTrainingThread(void *pParam)
{
    unsigned int quailty_score;
    VoiceCmdRecognition *pVoiceRecogize = (VoiceCmdRecognition *)pParam;
    // voice data is enough, start to training voice password
    VOWE_training_enroll_return_values info;

    /*
    * Adjust thread priority
    */
    prctl(PR_SET_NAME, (unsigned long)"enroll Training Thread", 0, 0, 0);
    setpriority(PRIO_PROCESS, 0, ANDROID_PRIORITY_AUDIO);
    ALOGD("%s(), pid: %d, tid: %d", __FUNCTION__, getpid(), gettid());

    pVoiceRecogize->m_enrolling = true;
    if (VOWE_training_enroll(&info) == vowe_bad) {
        ALOGW("TrainingEnroll failed!");
        quailty_score = 0;
    } else {
        pVoiceRecogize->writeWavFile(info.replaySignal, info.replaySignalSampleNumber);
        quailty_score = 100;
    }
    ALOGD("%s, call Release", __FUNCTION__);
    pVoiceRecogize->voiceRecognitionRelease(VOICE_PW_TRAINING_MODE);

    pVoiceRecogize->notify(VOICE_TRAINING_FINISH, quailty_score, 0);

    pVoiceRecogize->m_enrolling = false;

    pthread_exit(NULL);
    return 0;
}

static void *captureVoiceLoop(void *pParam)
{
    ALOGD("capture voice thread in +");
    int size = 0;
    int free_size = 0;
    bool drop = false;
    VoiceCmdRecognition *pVoiceRecogize = (VoiceCmdRecognition *)pParam;

    /*
    * Adjust thread priority
    */
    prctl(PR_SET_NAME, (unsigned long)"capture Voice Loop", 0, 0, 0);
    setpriority(PRIO_PROCESS, 0, ANDROID_PRIORITY_AUDIO);
    ALOGD("%s(), pid: %d, tid: %d", __FUNCTION__, getpid(), gettid());

    while (pVoiceRecogize->m_bStarted) {
        if (pVoiceRecogize->m_pAudioStream != 0) {
            size = BUF_READ_SIZE;
            pthread_mutex_lock(&pVoiceRecogize->m_BufMutex);
            free_size = ring_buffer_get_space_byte_count(&pVoiceRecogize->m_rb_info1) / sizeof(short);

            drop = false;
            if (free_size < size) {
                // buffer overflow
                ALOGD("%s(), voice buffer overflow!, size=%d, free_size=%d", __func__, size, free_size);
                drop = true;
            }
            pthread_mutex_unlock(&pVoiceRecogize->m_BufMutex);
            if (drop == false) {
                short *p_buf_1 = NULL;
                short *p_buf_2 = NULL;
                uint32_t buf_size_1 = 0;
                uint32_t buf_size_2 = 0;
                ring_buffer_get_write_information(&pVoiceRecogize->m_rb_info1, (uint8_t **)&p_buf_1, &buf_size_1);
                ring_buffer_get_write_information(&pVoiceRecogize->m_rb_info2, (uint8_t **)&p_buf_2, &buf_size_2);
                if (pVoiceRecogize->m_pAudioStream->readPCM(
                        p_buf_1,
                        p_buf_2,
                        0,
                        size) != OK) {
                    ALOGD("%s(), readPCM error!!", __func__);
                    break;
                }
                pthread_mutex_lock(&pVoiceRecogize->m_BufMutex);
                ring_buffer_write_done(&pVoiceRecogize->m_rb_info1, size * sizeof(short));
                ring_buffer_write_done(&pVoiceRecogize->m_rb_info2, size * sizeof(short));
                pthread_mutex_unlock(&pVoiceRecogize->m_BufMutex);
                pVoiceRecogize->m_already_read = true;
            } else {
                ALOGI("%s(), force drop!", __func__);
            }
        }
    }
    ALOGD("capture voice thread out -");
    pthread_exit(NULL);
    return 0;
}

static void *TrainingVoiceLoop(void *pParam)
{
    ALOGD("Training voice thread in +");
    VoiceCmdRecognition *pVoiceRecogize = (VoiceCmdRecognition *)pParam;
    bool bFirstFrame = true;
    int size = 0;
    int confidence = 0;
    int msg = vowe_next_frame;
    int notify_back = -1;
    float snr_value = 0;
    char *pRtnContactNameArray[2];
    int reserv_size = 0;
    int left_size = 0;
    short *pULBuf1;
    short *pULBuf2;
    short *pULBuf1_temp = new short[SAMPLES2];
    short *pULBuf2_temp = new short[SAMPLES2];

    /*
    * Adjust thread priority
    */
    prctl(PR_SET_NAME, (unsigned long)"Training Voice Loop", 0, 0, 0);
    setpriority(PRIO_PROCESS, 0, ANDROID_PRIORITY_AUDIO + 1);
    ALOGD("%s(), pid: %d, tid: %d", __FUNCTION__, getpid(), gettid());

    memset(pULBuf1_temp, 0, SAMPLES2*sizeof(short));
    memset(pULBuf2_temp, 0, SAMPLES2*sizeof(short));
    memset(pRtnContactNameArray, 0, 2*sizeof(char *));
    ALOGV("pVoiceRecogize->recordLockSignal()");
    pVoiceRecogize->recordLockSignal();

    while (pVoiceRecogize->m_bStarted) {
        // reset msg
        msg = vowe_next_frame;

        if (popcount(pVoiceRecogize->m_RecognitionMode) == 0) {
            usleep(5000);
        }
        // to capture valid voice data
        while ((msg == vowe_next_frame || msg == vowe_not_recording) &&
               popcount(pVoiceRecogize->m_RecognitionMode)) {
            if (pVoiceRecogize->m_pAudioStream != 0) {
                if (pVoiceRecogize->m_already_read) {
                    pthread_mutex_lock(&pVoiceRecogize->m_BufMutex);
                    reserv_size = ring_buffer_get_data_byte_count(&pVoiceRecogize->m_rb_info1) / sizeof(short);
                    pthread_mutex_unlock(&pVoiceRecogize->m_BufMutex);
                } else {
                    reserv_size = 0;
                    ALOGV("reserv_size = 0");
                }
                size = bFirstFrame ? SAMPLES2 : SAMPLES1;
                if (reserv_size >= size) {
                    uint8_t *p_buf_1 = NULL;
                    uint8_t *p_buf_2 = NULL;
                    uint32_t buf_size_1 = 0;
                    uint32_t buf_size_2 = 0;
                    if (bFirstFrame == true) {
                        ALOGD("start receiving data");
                        bFirstFrame = false;
                    }

                    ring_buffer_get_read_information(&pVoiceRecogize->m_rb_info1, &p_buf_1, &buf_size_1);
                    ring_buffer_get_read_information(&pVoiceRecogize->m_rb_info2, &p_buf_2, &buf_size_2);
                    left_size = buf_size_1 / sizeof(short);
                    if (left_size < size) {
                        memcpy(&pULBuf1_temp[0], p_buf_1, left_size * sizeof(short));
                        memcpy(&pULBuf2_temp[0], p_buf_2, left_size * sizeof(short));
                        ring_buffer_read_done(&pVoiceRecogize->m_rb_info1, left_size * sizeof(short));
                        ring_buffer_read_done(&pVoiceRecogize->m_rb_info2, left_size * sizeof(short));
                        ring_buffer_get_read_information(&pVoiceRecogize->m_rb_info1, &p_buf_1, &buf_size_1);
                        ring_buffer_get_read_information(&pVoiceRecogize->m_rb_info2, &p_buf_2, &buf_size_2);
                        memcpy(&pULBuf1_temp[left_size], p_buf_1, (size - left_size) * sizeof(short));
                        memcpy(&pULBuf2_temp[left_size], p_buf_2, (size - left_size) * sizeof(short));
                        ring_buffer_read_done(&pVoiceRecogize->m_rb_info1, (size - left_size) * sizeof(short));
                        ring_buffer_read_done(&pVoiceRecogize->m_rb_info2, (size - left_size) * sizeof(short));
                    } else {
                        memcpy(&pULBuf1_temp[0], p_buf_1, size * sizeof(short));
                        memcpy(&pULBuf2_temp[0], p_buf_2, size * sizeof(short));
                        ring_buffer_read_done(&pVoiceRecogize->m_rb_info1, size * sizeof(short));
                        ring_buffer_read_done(&pVoiceRecogize->m_rb_info2, size * sizeof(short));
                    }
                    pULBuf1 = &pULBuf1_temp[0];
                    pULBuf2 = &pULBuf2_temp[0];

                    if (pVoiceRecogize->m_RecognitionMode & VOICE_PW_TRAINING_MODE) {
                        ALOGV("endPointDetection before");
                        if ((confidence != 100) &&
                            ((pVoiceRecogize->m_CaptureVoicePause == false) ||
                             (pVoiceRecogize->m_bStartRecordBackroundVoice == true))) {
                            //ALOGD("size = %d", size);
                            if (VOWE_training_inputMic(0, pULBuf1, size) == vowe_bad) {
                                ALOGD("Training InputMic error!!");
                                break;
                            }
                            msg = VOWE_training_diagnose(&confidence);
                            ALOGV("[dato]msg=%d, confidence=%d", msg, confidence);
                            if (msg != vowe_not_recording) {
                                pVoiceRecogize->m_pAudioStream->dumpAudioData(pULBuf1, pULBuf2, size<<1);
                            }
                        }
                    }

                    // background voice receiving
                    if (pVoiceRecogize->m_bStartRecordBackroundVoice == true) {
                        if (pVoiceRecogize->m_BackroundVoiceCount >= BACKGROUND_RECORD_SAMPLE_CNT) {
                            ALOGD("Background Voice receving done");
                            if (pVoiceRecogize->m_onStartProcessing == false) {
                                pVoiceRecogize->notify(VOICE_TRAINING_PROCESS, VOICE_ON_START_PROCESS, 0);
                                pVoiceRecogize->m_onStartProcessing = true;
                            }
                            pVoiceRecogize->m_bStartRecordBackroundVoice = false;
                            pVoiceRecogize->m_BackroundVoiceCount = 0;
                        } else {
                            ALOGV("Background cnt=%d", pVoiceRecogize->m_BackroundVoiceCount);
                            pVoiceRecogize->m_BackroundVoiceCount++;
                        }
                    }
                } else {
                    ALOGV("reserv_size(%d)", reserv_size);
                    usleep(3000);  // sleep 3ms
                }
            } else {
                break;
            }
        }

        // for voice password taining mode
        if ((pVoiceRecogize->m_bStartRecordBackroundVoice == false) &&
            (msg != vowe_next_frame) &&
            (pVoiceRecogize->m_RecognitionMode & VOICE_PW_TRAINING_MODE)) {

            pVoiceRecogize->m_bNeedToWait = false;
            pVoiceRecogize->m_bNeedToRelease = true;

            VOWE_training_getFloatArgument(vowe_argid_trainLastUtteranceSnr, &snr_value);
            ALOGD("snr: %f", snr_value);
            pVoiceRecogize->notify_data[0] = (char)snr_value;
            pVoiceRecogize->notify_data[1] = 0;
            if (pVoiceRecogize->notify_data[0] <= 0) {
                pVoiceRecogize->notify_data[0] = 1;  //UTF will let "0" be end condition
            } else if (pVoiceRecogize->notify_data[0] > 100) {
                pVoiceRecogize->notify_data[0] = 100;
            }
            pRtnContactNameArray[0] = &pVoiceRecogize->notify_data[0];
            switch (msg) {
                case vowe_no_speech:
                    notify_back = 6;
                    break;
                case vowe_ready_to_enroll:
                    notify_back = 0;
                    pVoiceRecogize->m_bNeedToWait = true;
                    pVoiceRecogize->m_bNeedToRelease = false;
                    break;
                case vowe_good_utterance:
                    notify_back = 1;
                    break;
                case vowe_bad_utterance:
                    notify_back = 4;//5;
                    break;
                case vowe_noisy_utterance:
                    notify_back = 2;
                    break;
                case vowe_low_snr_utterance:
                    notify_back = 3;
                    break;
                case vowe_mismatched_cmd_utterance:
                    notify_back = 4;
                    break;
                default:
                    ALOGD("[error] return value error, need check lib");
                    break;
            }
            ALOGD("PCMDiagonosis after msg:%d, confidence:%d, snr:%d",
                  msg,
                  confidence,
                  pVoiceRecogize->notify_data[0]);

            if (pVoiceRecogize->m_onStartRecording == true) {
                pVoiceRecogize->notify(VOICE_TRAINING_PROCESS, VOICE_ON_STOP_RECORD, 0);
                pVoiceRecogize->m_onStartRecording = false;
            }
            if (pVoiceRecogize->m_onStartProcessing == true) {
                pVoiceRecogize->notify(VOICE_TRAINING_PROCESS, VOICE_ON_STOP_PROCESS, 0);
                pVoiceRecogize->m_onStartProcessing = false;
            }
            pVoiceRecogize->notify(VOICE_TRAINING, notify_back, confidence, pRtnContactNameArray);

            if ((msg == vowe_ready_to_enroll) && (pVoiceRecogize->m_bNeedToWait)) {
                pVoiceRecogize->recordLockWait(kWaitingTimeOutMS);
            }
            // msg==0 -   voice is enough, it is going to training
            // msg==1 -   voice is not enough, recording on going
            // msg==2 -   the environment is too noisy
            // msg==3 -   the sound is too little
            // msg==4 -   the password is not match with previous password
            // msg==5 -   the password is already exist
            // msg==11 -  the password is not match with the password specified by vendor
        }
    }
    if (pULBuf1_temp!=NULL) delete [] pULBuf1_temp;
    if (pULBuf2_temp!=NULL) delete [] pULBuf2_temp;
    pULBuf1_temp = NULL;
    pULBuf2_temp = NULL;
    pULBuf1 = NULL;
    pULBuf2 = NULL;

    ALOGD("Training voice thread out -");
    pthread_exit(NULL);
    return 0;
}

/*
void writeCallBackWrapper(void *me, const short *pBuf, int64_t length)
{
    //VoiceCmdRecognition *pVoiceRecognize = (VoiceCmdRecognition *)me;
    //pVoiceRecognize->writeWavFile(pBuf, length);
}
*/

VoiceCmdRecognition::VoiceCmdRecognition(audio_source_t inputSource, unsigned int sampleRate, unsigned int channelCount) :
   m_RecognitionMode(VOICE_IDLE_MODE),
   m_bStarted(false),
   m_bNeedToWait(false),
   m_bNeedToRelease(true),
   m_enrolling(false),
   m_onStartRecording(false),
   m_onStartProcessing(false),
   m_setTrainingThreshold(false),
   m_setTrainingTimeout(false),
   m_bStartRecordBackroundVoice(false),
   m_already_read(false),
   m_Training_Lib_Init(false),
   m_CaptureVoicePause(false),
   m_BackroundVoiceCount(0),
   m_trainingTimes(0),
   m_trainingThreshold(50),
   m_trainingTimeoutMsec(5000),
   m_voiceULBuf1(NULL),
   m_voiceULBuf2(NULL),
   m_ReadDataTid(0),
   //m_bSpecificRefMic(false),
   m_pListener(0),
#if HOTWORD_TRAINING
   m_InputSource(AUDIO_SOURCE_HOTWORD),
#else
   m_InputSource(AUDIO_SOURCE_UNPROCESSED),
#endif
   m_SampleRate(sampleRate),
   m_Channels(channelCount),
   m_PasswordFd(-1),
   m_PatternFd(-1),
   m_FeatureFd(-1),
   m_CommandId(-1),
   m_VoiceMode(VOICE_NORMAL_MODE),
   m_WakeupMode(VOICE_UNLOCK),
   m_pStrWakeupInfoPath(NULL)
{
    ALOGD("VoiceCmdRecognition construct in +");
    ALOGD("input source:%d,sampe rate:%d,Channel count:%d, m_InputSource%d, inputSource%d", m_InputSource,m_SampleRate,m_Channels,m_InputSource,inputSource);
    memset(m_strPatternPath, 0, FILE_NAME_LEN_MAX);
    memset(m_strUBMPath, 0, FILE_NAME_LEN_MAX);
    memset(m_strUpgradeUBMPath, 0, FILE_NAME_LEN_MAX);
    memset(&m_rb_info1, 0, sizeof(struct ring_buffer_information));
    memset(&m_rb_info2, 0, sizeof(struct ring_buffer_information));
    if (pthread_mutex_init(&m_RecordMutex, NULL)!=0) {
        ALOGW("Failed to initialize m_RecordMutex!");
    }
    if (pthread_mutex_init(&m_BufMutex, NULL)!=0) {
        ALOGD("Failed to initialize m_BufMutex!");
    }
    if (pthread_cond_init(&m_RecordExitCond, NULL)!=0) {
        ALOGW("Failed to initialize m_RecordExitCond!");
    }
    char *voice_version;
    voice_version = (char *)VOWE_training_version();
    ALOGD("voice unlock SWIP version is:%s", voice_version);
}

VoiceCmdRecognition::~VoiceCmdRecognition()
{
    ALOGD("%s, deconstruct in +", __FUNCTION__);
    if (m_pAudioStream!=0) {
        m_pAudioStream.clear();
    }

    if (m_pListener!=0) {
        m_pListener.clear();
    }

    if (m_PasswordFd>=0) {
        ::close(m_PasswordFd);
    }

    if (m_PatternFd>=0) {
        ::close(m_PatternFd);
    }

    if (m_FeatureFd>=0) {
        ::close(m_FeatureFd);
    }
    if (m_voiceULBuf1 != NULL) {
        ALOGD("%s, m_voiceULBuf1 delete", __FUNCTION__);
        delete[] m_voiceULBuf1;
        m_voiceULBuf1 = NULL;
    }
    if (m_voiceULBuf2 != NULL) {
        ALOGD("%s, m_voiceULBuf2 delete", __FUNCTION__);
        delete[] m_voiceULBuf2;
        m_voiceULBuf2 = NULL;
    }
}

status_t VoiceCmdRecognition::initCheck()
{
    return m_pAudioStream != 0 ? NO_ERROR : NO_INIT;
}

status_t VoiceCmdRecognition::setVoicePasswordFile(int fd, int64_t offset, int64_t length)
{
    ALOGV("%s in +", __FUNCTION__);
    if(fd<0) {
        ALOGE("Invalid file descriptor: %d", fd);
        return -EBADF;
    }

    if (m_PasswordFd>=0){
        ::close(m_PasswordFd);
    }

    m_PasswordFd = dup(fd);
    ALOGD("setVoicePasswordFile m_PasswordFd:%d, fd:%d, offset:%lld length:%lld", m_PasswordFd, fd, (long long)offset, (long long)length);
    return OK;
}

status_t VoiceCmdRecognition::setVoicePatternFile(int fd, int64_t offset, int64_t length)
{
    ALOGD("%s in +", __FUNCTION__);
    if(fd<0) {
        ALOGE("Invalid file descriptor: %d", fd);
        return -EBADF;
    }

    if (m_PatternFd>=0){
        ::close(m_PatternFd);
    }

    m_PatternFd = dup(fd);
    ALOGD("setVoicePatternFile m_PatternFd:%d, fd:%d, offset:%lld, length:%lld", m_PatternFd, fd, (long long)offset, (long long)length);
    return OK;
}


status_t VoiceCmdRecognition::setVoicePatternFile(const char *path)
{
    ALOGD("%s in +, path: %s", __FUNCTION__, path);
    if (path==NULL) {
        ALOGE("voice patter file path is null!!");
        return BAD_VALUE;
    }

    strncpy(m_strPatternPath, path, FILE_NAME_LEN_MAX-1);
    return OK;
}

status_t VoiceCmdRecognition::setVoiceUBMFile(const char *path)
{
    ALOGD("%s in +, path: %s", __FUNCTION__, path);
    if (path==NULL) {
        ALOGE("UBM files path is null: %s", path);
        return BAD_VALUE;
    }

    strncpy(m_strUBMPath, path, FILE_NAME_LEN_MAX-1);

    return OK;
}

status_t VoiceCmdRecognition::setUpgradeVoiceUBMFile(const char *path)
{
    ALOGD("%s in +, path: %s", __FUNCTION__, path);
    if (path==NULL) {
        ALOGE("Upgrade UBM files path is null: %s", path);
        return BAD_VALUE;
    }

    strncpy(m_strUpgradeUBMPath, path, FILE_NAME_LEN_MAX-1);

    return OK;
}

status_t VoiceCmdRecognition::setVoiceFeatureFile(int fd, int64_t offset, int64_t length)
{
    ALOGV("%s in +", __FUNCTION__);
    if(fd<0) {
        ALOGE("Invalid feature file descriptor: %d", fd);
        return -EBADF;
    }

    if (m_FeatureFd>=0){
        ::close(m_FeatureFd);
    }

    m_FeatureFd = dup(fd);
    ALOGD("setVoiceFeatureFile m_FeatureFd:%d, fd:%d, length%lld ,offset%lld", m_FeatureFd, fd, (long long)length, (long long)offset);
    return OK;
}

status_t VoiceCmdRecognition::setCommandId(int id)
{
    ALOGV("%s in +", __FUNCTION__);
    if (id<0) {
        ALOGE("command id is invalide: %d", id);
        return BAD_VALUE;
    }
    m_CommandId = id;
    return OK;
}

status_t VoiceCmdRecognition::setInputMode(int input_mode)
{
    ALOGD("%s +, input_mode: %d", __FUNCTION__, input_mode);
    
    if (input_mode>=VOICE_MODE_NUM_MAX) {
        ALOGW("input mode is invalide!!");
        return BAD_VALUE;
    }
    
    m_VoiceMode = input_mode;
    return OK;
}

/*for voice wakeup feature*/
status_t VoiceCmdRecognition::setVoiceTrainingMode(int mode)
{
    ALOGD("%s +, recongnition mode: %d", __FUNCTION__, mode);

    if (mode < 0 || mode > VOICE_WAKE_UP_MODE_NUM) {
        ALOGD("setVoiceTrainingMode mode error!!");
        return BAD_VALUE;
    }

    if (mode == VOICE_WAKEUP_NO_RECOGNIZE) {
        m_WakeupMode = vowe_mode_pdk_fullRecognizer;
    } else if (mode == VOICE_WAKEUP_RECOGNIZE) {
        m_WakeupMode = vowe_mode_udk_lowPower;
    }
    ALOGD("m_WakeupMode = %d", m_WakeupMode);
    return OK;
}

status_t VoiceCmdRecognition::setVoiceWakeupInfoPath(const char * path)
{
    int size = 0;
    if (path == NULL) {
        ALOGD("%s, file path is NULL!", __FUNCTION__);
        return BAD_VALUE;
    }

    ALOGV("%s +, file path: %s", __FUNCTION__, path);
    size = strlen(path);
    if (size > FILE_NAME_LEN_MAX) {
        ALOGD("%s, file path is too long length:%d!", __FUNCTION__, size);
        return BAD_VALUE;
    }

    if (m_pStrWakeupInfoPath == NULL) {
        m_pStrWakeupInfoPath = new char[size + 1];
        memset(m_pStrWakeupInfoPath, 0, (size + 1) * sizeof(char));
    }

    if (m_pStrWakeupInfoPath) {
        strncpy(m_pStrWakeupInfoPath, path, size);
    } else {
        ALOGW("setVoiceWakeupInfoPath allocate memory fail!");
        return BAD_VALUE;
    }

    return OK;
}

status_t VoiceCmdRecognition::setVoiceWakeupMode(int mode)
{
    ALOGD("%s +, mode=%d", __FUNCTION__, mode);

    if (mode < 0 || mode > VOICE_WAKE_UP_MODE_NUM) {
        ALOGD("setVoiceWakeupMode mode error!!");
        return BAD_VALUE;
    }

    m_WakeupMode = mode;

    return OK;
}

status_t VoiceCmdRecognition::pauseVoiceTraining()
{
    ALOGD("%s +", __FUNCTION__);
    if (m_CaptureVoicePause == false) {
        m_CaptureVoicePause = true;
    }
    return OK;
}

status_t VoiceCmdRecognition::getTrainigTimes(int *times)
{
    ALOGD("%s +", __FUNCTION__);
    int err;

    if (times == NULL) {
        ALOGE("Null pointer argument");
        return BAD_VALUE;
    }
    err = VOWE_training_getIntegerArgument(vowe_argid_trainUtteranceNumber, times);
    if (err != vowe_ok) {
        ALOGE("%s, error!!=%d", __FUNCTION__, err);
        return BAD_VALUE;
    } else {
        ALOGD("%s, times=%d", __FUNCTION__, *times);
        if (*times == 0) {
            ALOGD("%s, SWIP return invalid time, force return 5=", __FUNCTION__);
            *times = 5;
        }
        m_trainingTimes = *times;
        return OK;
    }
}

status_t VoiceCmdRecognition::setUtteranceTrainingTimeout(int msec)
{
    ALOGD("%s +, msec=%d", __FUNCTION__, msec);

    if ( msec < 3000 || msec > 6000 ) {
        ALOGE("%s, bad value", __FUNCTION__);
        return BAD_VALUE;
    }

    m_trainingTimeoutMsec = msec;
    m_setTrainingTimeout = true;
    return OK;
}

status_t VoiceCmdRecognition::setTrainingThreshold(int threshold)
{
    ALOGD("%s +, threshold=%d", __FUNCTION__, threshold);

    if ( threshold < 0 || threshold > 100 ) {
        ALOGE("%s, bad value", __FUNCTION__);
        return BAD_VALUE;
    }
    m_trainingThreshold = threshold;
    m_setTrainingThreshold = true;
    return OK;
}

status_t VoiceCmdRecognition::getTrainingThreshold(int *threshold)
{
    ALOGD("%s +", __FUNCTION__);

    if (threshold == NULL) {
        ALOGE("Null pointer argument");
        return BAD_VALUE;
    }

    *threshold = m_trainingThreshold;
    ALOGD("%s, threshold=%d", __FUNCTION__, *threshold);
    return OK;
}

status_t VoiceCmdRecognition::continueVoiceTraining()
{
    ALOGD("%s +", __FUNCTION__);

    ALOGD("%s, push2talk", __FUNCTION__);
    if (VOWE_training_setArgument(vowe_argid_pushToTalk, 1) == vowe_bad) {
        ALOGD("setArgument error!!");
    }
    if (m_onStartRecording == false) {
        notify(VOICE_TRAINING_PROCESS, VOICE_ON_START_RECORD, 0);
        m_onStartRecording = true;
    }
    if (m_onStartProcessing == false) {
        notify(VOICE_TRAINING_PROCESS, VOICE_ON_START_PROCESS, 0);
        m_onStartProcessing = true;
    }
    if (m_CaptureVoicePause == true) {
        ALOGD("%s, resume Capture Voice", __FUNCTION__);
        m_CaptureVoicePause = false;
    }
    return OK;
}

status_t VoiceCmdRecognition::startCaptureVoice(unsigned int mode)
{
    ALOGD("%s +", __FUNCTION__);

    // for create thread timeout
    struct timeval now;
    struct timespec timeout;
    gettimeofday(&now, NULL);
    timeout.tv_sec  = now.tv_sec + 3;
    timeout.tv_nsec = now.tv_usec*1000;

    // parameters mode:
    // VOICE_IDLE_MODE for idle mode, created but not work
    // VOICE_PW_TRAINING_MODE for voice password training.
    if ((popcount(mode)!=1) || ((mode&VOICE_RECOGNIZE_MODE_ALL)==0))
    return BAD_VALUE;

    if (m_RecognitionMode & VOICE_PW_TRAINING_MODE) {
        ALOGE("voice password training is running");
        return BAD_VALUE;
    }

    // for voice recognition initialize
    status_t ret = OK;
    ret = voiceRecognitionInit(mode);
    if (ret != OK) {
        ALOGW("startCaptureVoice, error!!, voice recognition fail!");
        return ret;
    }
#if HOTWORD_TRAINING
    //=======================
    // do not call AudioSystem methods with mLock held
    AudioSystem::setParameters(0, String8("vow_hotword_record_path=on"));
    AudioSystem::setParameters(0, String8("MTK_VOW_TRAINING=1"));
    ret = AudioSystem::acquireSoundTriggerSession(&m_Session, &m_IoHandle, &m_Device);
    ALOGD("m_Session=%d, m_Session=%d, m_Device=%d", (int)m_Session, (int)m_IoHandle, (int)m_Device);
    if (ret != NO_ERROR) {
        ALOGE("acquire sound trigger session error!!, ret=%d", ret);
        return ret;
    }
    //=======================
#endif

    if (m_voiceULBuf1 != NULL) {
        ALOGD("%s, 1. m_voiceULBuf1 need to delete", __FUNCTION__);
        delete[] m_voiceULBuf1;
        m_voiceULBuf1 = NULL;
    }
    if (m_voiceULBuf2 != NULL) {
        ALOGD("%s, 1. m_voiceULBuf2 need to delete", __FUNCTION__);
        delete[] m_voiceULBuf2;
        m_voiceULBuf2 = NULL;
    }
    ALOGD("m_voiceULBuf1 malloc new");
    m_voiceULBuf1 = new short[MAX_SAMPLE_LENGTH];
    if (m_voiceULBuf1 == NULL) {
        ALOGD("error!!, m_voiceULBuf1 malloc fail");
        goto exit;
    }
    ALOGD("m_voiceULBuf2 malloc new");
    m_voiceULBuf2 = new short[MAX_SAMPLE_LENGTH];
    if (m_voiceULBuf2 == NULL) {
        ALOGD("error!!, m_voiceULBuf2 malloc fail");
        goto exit;
    }
    pthread_mutex_lock(&m_BufMutex);
    pthread_mutex_unlock(&m_BufMutex);
    m_already_read = false;

    ret = startAudioStream();
    if (ret != OK) {
        ALOGW("startAudioStream fail!");
        ALOGD("%s, call Release, mode=%d", __FUNCTION__, mode);
        voiceRecognitionRelease(mode);
        goto exit;
    }

    m_RecognitionMode = m_RecognitionMode | mode;
    m_onStartRecording = false;
    m_onStartProcessing = false;
    if (m_onStartRecording == false) {
        notify(VOICE_TRAINING_PROCESS, VOICE_ON_START_RECORD, 0);
        m_onStartRecording = true;
    }
    m_bStartRecordBackroundVoice = true;
    m_BackroundVoiceCount = 0;
    m_CaptureVoicePause = false;
/*
    if (m_onStartProcessing == false) {
        notify(VOICE_TRAINING_PROCESS, VOICE_ON_START_PROCESS, 0);
        m_onStartProcessing = true;
    }
*/
    ALOGD("%s -", __FUNCTION__);
    return OK;
exit:
    ALOGD("%s error handle", __FUNCTION__);
    if (m_voiceULBuf1 != NULL) {
        ALOGD("%s, error!!, m_voiceULBuf1 need to delete", __FUNCTION__);
        delete[] m_voiceULBuf1;
        m_voiceULBuf1 = NULL;
    }
    if (m_voiceULBuf2 != NULL) {
        ALOGD("%s, error!!, m_voiceULBuf2 need to delete", __FUNCTION__);
        delete[] m_voiceULBuf2;
        m_voiceULBuf2 = NULL;
    }
#if HOTWORD_TRAINING
    if (ret != NO_ERROR) {
        // do not call AudioSystem methods with mLock held
        AudioSystem::releaseSoundTriggerSession(m_Session);
        AudioSystem::setParameters(0, String8("MTK_VOW_TRAINING=0"));
        AudioSystem::setParameters(0, String8("vow_hotword_record_path=off"));
    }
#endif
    return ret;
}

status_t VoiceCmdRecognition::stopCaptureVoice(unsigned int mode)
{
    int count = 1000000;
    ALOGD("%s +", __FUNCTION__);

    // parameters mode:
    // VOICE_IDLE_MODE for idle mode, created but not work
    // VOICE_PW_TRAINING_MODE for voice password training.
    if ((popcount(mode) != 1) || !(mode & VOICE_RECOGNIZE_MODE_ALL) || !(mode & m_RecognitionMode)) {
        ALOGW("stopCaptureVoice mode:0x%x, recognizing mode:0x%x", mode, m_RecognitionMode);
        ALOGD("%s, re-entry, need to release", __FUNCTION__);
        voiceRecognitionRelease(mode);
        return OK;
    }

    while ((m_enrolling == true) && (count > 0)) {
        usleep(10000);
	count--;
    }


    m_RecognitionMode = m_RecognitionMode&(~mode);
    // signal TrainingVoiceLoop thread to go on.
    m_bNeedToWait = false;
    ALOGV("recordLockSignal()");
    recordLockSignal();
    ALOGV("stopCaptureVoice after signal--");

    ALOGV("mode 0x%x m_RecognitionMode 0x%x",mode,m_RecognitionMode);
    if (popcount(m_RecognitionMode) == 0) {
        m_bStarted = false;
        ALOGD("m_Tid wait thread exit");
        pthread_join(m_Tid, NULL);
        ALOGD("m_Tid wait thread exit done");
        ALOGD("m_ReadDataTid wait thread exit");
        pthread_join(m_ReadDataTid, NULL);
        ALOGD("m_ReadDataTid wait thread exit done");

        if (m_pAudioStream!=0) {
            m_pAudioStream->stop();
            m_pAudioStream.clear();
            m_pAudioStream = NULL;
        }
        if (m_onStartRecording == true) {
            notify(VOICE_TRAINING_PROCESS, VOICE_ON_STOP_RECORD, 0);
            m_onStartRecording = false;
        }
        if (m_onStartProcessing == true) {
            notify(VOICE_TRAINING_PROCESS, VOICE_ON_STOP_PROCESS, 0);
            m_onStartProcessing = false;
        }
    }
    m_onStartRecording = false;
    m_onStartProcessing = false;
    if (m_voiceULBuf1 != NULL) {
        ALOGD("%s, m_voiceULBuf1 delete", __FUNCTION__);
        delete[] m_voiceULBuf1;
        m_voiceULBuf1 = NULL;
    }
    if (m_voiceULBuf2 != NULL) {
        ALOGD("%s, m_voiceULBuf2 delete", __FUNCTION__);
        delete[] m_voiceULBuf2;
        m_voiceULBuf2 = NULL;
    }
#if HOTWORD_TRAINING
    // do not call AudioSystem methods with mLock held
    AudioSystem::releaseSoundTriggerSession(m_Session);
    AudioSystem::setParameters(0, String8("MTK_VOW_TRAINING=0"));
    AudioSystem::setParameters(0, String8("vow_hotword_record_path=off"));
#endif
    if (mode == VOICE_PW_TRAINING_MODE) {
        if (m_bNeedToRelease) {
            ALOGD("%s, call Release", __FUNCTION__);
            voiceRecognitionRelease(VOICE_PW_TRAINING_MODE);
        }
        m_bNeedToRelease = true;
    }

    ALOGD("stopCaptureVoice -");
    return OK;
}

status_t VoiceCmdRecognition::startVoiceTraining()
{
    ALOGD("%s in +", __FUNCTION__);
    status_t ret = OK;

    pthread_create(&m_enrollTid, NULL, enrollTrainingThread, this);
    pthread_join(m_enrollTid, NULL);
    ALOGD("%s in -", __FUNCTION__);
    return ret;
}

status_t VoiceCmdRecognition::setVoiceModelRetrain()
{
    ALOGD("%s in +", __FUNCTION__);
    status_t ret = OK;
    int confidence = 0;

    VOWE_training_init_parameters training_init_info;
    training_init_info.mode = m_WakeupMode;
    training_init_info.micNumber = 1;
    training_init_info.frameLength = 400;
    training_init_info.frameShift = 160;
    training_init_info.inputFolder = m_strUBMPath;
    training_init_info.debugFolder = NULL;
    training_init_info.outputFileDescriptor = m_PatternFd;
    if (m_strUpgradeUBMPath[0] == 0) {
        training_init_info.updatedFileListPath = NULL;
    } else {
        training_init_info.updatedFileListPath = m_strUpgradeUBMPath;
    }
    training_init_info.trainConfigFolder = m_strPatternPath;

    ALOGD("%s, mode=%d", __FUNCTION__, training_init_info.mode);
    ALOGD("%s, inputFolder=%s", __FUNCTION__, training_init_info.inputFolder);
    ALOGD("%s, outputFileDescriptor=%d", __FUNCTION__, training_init_info.outputFileDescriptor);
    ALOGD("%s, updatedFileListPath=%s", __FUNCTION__, training_init_info.updatedFileListPath);
    ALOGD("%s, trainConfigFolder=%s", __FUNCTION__, training_init_info.trainConfigFolder);

    VOWE_training_input training_input;
    VOWE_training_output training_output;
    training_output.rtnConfidenceToEnroll = &confidence;
    char filename_update_ul[] = "training_ul.pcm";
    char filename[100];

    sprintf(filename, "%s%s", m_strPatternPath, filename_update_ul);
    training_input.trainPcmPath = filename;
    ALOGD("%s, input path=%s", __FUNCTION__, training_input.trainPcmPath);

    VOWE_training_backgroundUpdate(&training_init_info, training_input, &training_output);

    voiceRecognitionRelease(VOICE_PW_RETRAIN_MODE);

    if (*training_output.rtnConfidenceToEnroll == 100) {
        notify(VOICE_TRAINING_RETRAIN, VOICE_RETRAIN_SUCCESS, 0);
    } else {
        notify(VOICE_TRAINING_RETRAIN, VOICE_RETRAIN_FAIL, 0);
    }
    ALOGD("%s, mode=%d, confidence=%d", __FUNCTION__, training_init_info.mode, *training_output.rtnConfidenceToEnroll);
    return ret;
}

status_t VoiceCmdRecognition::getVoiceIntensity(int *maxAmplitude)
{
    ALOGV("%s in +", __FUNCTION__);

    if (maxAmplitude == NULL) {
        ALOGE("Null pointer argument");
        return BAD_VALUE;
    }

    if (m_pAudioStream!=0) {
        // get the intensity for recording PCM data
        *maxAmplitude = m_pAudioStream->getMaxAmplitude(0); //0: Channel L, 1: Channel_R
    } else {
        *maxAmplitude = 0;
    }

    ALOGV("%s maxAmplitude %d",__FUNCTION__, *maxAmplitude);
    return OK;
}

status_t VoiceCmdRecognition::setListener(const sp<VoiceCmdRecognitionListener>& listener)
{
    ALOGV("%s in +", __FUNCTION__);
    m_pListener = listener;

    return NO_ERROR;
}

void VoiceCmdRecognition::writeWavFile(const short *pBuf, int64_t length)
{
    short *buf_voice_cmd = NULL;
    ALOGV("%s in +", __FUNCTION__);
    if (pBuf==NULL||length==0) {
        ALOGW("buffer pointer is null or date length is zero~");
        return;
    }

    FILE *fd = fdopen(m_PasswordFd, "wb");
    if (fd==NULL) {
        ALOGE("open file descriptor fail, errorno: %s", strerror(errno));
        return;
    }

    // write wave header, this file is for unlock password.
    m_WavHeader.riff_id = ID_RIFF;
    m_WavHeader.riff_sz = length*sizeof(short) + 8 + 16 + 8;
    m_WavHeader.riff_fmt = ID_WAVE;
    m_WavHeader.fmt_id = ID_FMT;
    m_WavHeader.fmt_sz = 16;
    m_WavHeader.audio_format = FORMAT_PCM;
    m_WavHeader.num_channels = 1;
    m_WavHeader.sample_rate = m_SampleRate;
    m_WavHeader.byte_rate = m_WavHeader.sample_rate * m_WavHeader.num_channels * 2;
    m_WavHeader.block_align = m_WavHeader.num_channels * 2;
    m_WavHeader.bits_per_sample = 16;
    m_WavHeader.data_id = ID_DATA;
    m_WavHeader.data_sz = length*sizeof(short);

    // apply gain for voice command playing file
    buf_voice_cmd = new short[length];
    memcpy(buf_voice_cmd, pBuf, length*sizeof(short));
    for(int i=0;i<length;i++) {
        buf_voice_cmd[i] = ClipToShort((int)buf_voice_cmd[i] * (int)PCM_FILE_GAIN);
    }
    fwrite(&m_WavHeader, sizeof(m_WavHeader), 1, fd);
    fwrite((const void *)buf_voice_cmd, length*sizeof(short), 1, fd);
    if (buf_voice_cmd != NULL) {
        delete[] buf_voice_cmd;
    }
    fflush(fd);
    fclose(fd);
}

void VoiceCmdRecognition::notify(int message, int ext1, int ext2, char **ext3)
{
    ALOGD("notify in + msg %d, ext1 %d, ext2 %d", message, ext1, ext2);
    m_pListener->notify(message, ext1, ext2, ext3);
}

void VoiceCmdRecognition::recordLockSignal()
{
    // for sync recording, signal to record lock wait
    pthread_mutex_lock(&m_RecordMutex);
    pthread_cond_signal(&m_RecordExitCond);
    pthread_mutex_unlock(&m_RecordMutex);
}

int VoiceCmdRecognition::recordLockWait(int delayMS)
{
    int ret = 0;
    // wait record signal, if delayMs is not 0, it will time out when reachs the delayMS time.
    pthread_mutex_lock(&m_RecordMutex);
    if (delayMS!=0) {
        struct timeval now;
        struct timespec timeout;
        gettimeofday(&now,NULL);
        timeout.tv_sec  = now.tv_sec + delayMS/1000;
        timeout.tv_nsec = now.tv_usec*1000;
        ret = pthread_cond_timedwait(&m_RecordExitCond, &m_RecordMutex, &timeout);
    }else {
        ret = pthread_cond_wait(&m_RecordExitCond, &m_RecordMutex);
    }
    pthread_mutex_unlock(&m_RecordMutex);
    ALOGV("recordLockWait ret %d",ret);
    return ret;
}

status_t VoiceCmdRecognition::voiceRecognitionInit(unsigned int mode)
{
    ALOGD("voiceRecognitionInit in +");
    ALOGD("FRAMEWORK_VOW_RECOG_VER %s", FRAMEWORK_VOW_RECOG_VER);
    status_t ret = OK;

    switch(mode) {
        case VOICE_PW_TRAINING_MODE:
            if ((m_CommandId < 0) || (m_PatternFd < 0) || (m_FeatureFd < 0)) {
                ALOGE("parameters do not initialize, command id: %d, pattern fd: %d, feature fd: %d", m_CommandId, m_PatternFd, m_FeatureFd);
                return BAD_VALUE;
            }

            VOWE_training_init_parameters training_init_info;

            training_init_info.mode = m_WakeupMode;
            training_init_info.micNumber = 1;
            training_init_info.frameLength = 400;
            training_init_info.frameShift = 160;
            training_init_info.inputFolder = m_strUBMPath;
            training_init_info.debugFolder = NULL;
            training_init_info.outputFileDescriptor = m_PatternFd;
            if (m_strUpgradeUBMPath[0] == 0) {
                training_init_info.updatedFileListPath = NULL;
            } else {
                training_init_info.updatedFileListPath = m_strUpgradeUBMPath;
            }
            training_init_info.trainConfigFolder = m_strPatternPath;
            ALOGD("%s, mode=%d", __FUNCTION__, training_init_info.mode);
            ALOGD("%s, inputFolder=%s", __FUNCTION__, training_init_info.inputFolder);
            ALOGD("%s, outputFileDescriptor=%d", __FUNCTION__, training_init_info.outputFileDescriptor);
            ALOGD("%s, updatedFileListPath=%s", __FUNCTION__, training_init_info.updatedFileListPath);
            ALOGD("%s, trainConfigFolder=%s", __FUNCTION__, training_init_info.trainConfigFolder);
            if (m_Training_Lib_Init == false) {
                if (VOWE_training_init(&training_init_info) == vowe_bad) {
                    ALOGE("error!! Traning init fail!");
                    ret = BAD_VALUE;
                    break;
                } else {
                    m_Training_Lib_Init = true;
                }
            } else {
                ALOGE("error!! m_Training_Lib_Init is not NULL, please check previous training flow!");
                ret = BAD_VALUE;
                break;
            }
            /* Set Training Timeout */
            if (m_setTrainingTimeout) {
                if (VOWE_training_setFloatArgument(vowe_argid_trainTimeout, (float)m_trainingTimeoutMsec) != vowe_ok) {
                    ALOGE("%s, error!! ", __FUNCTION__);
                    ret = BAD_VALUE;
                    break;
                } else {
                    ret = OK;
                }
            }
            /* Set Training Threshold */
            if (m_setTrainingThreshold) {
                float thd[1];
                thd[0] = (float)m_trainingThreshold;
                if (VOWE_training_setFloatArrayArgument(vowe_argid_trainGlobalConfidenceThresholds, thd, 1) != vowe_ok) {
                    ALOGE("%s, error!!", __FUNCTION__);
                    ret = BAD_VALUE;
                    break;
                } else {
                    ret = OK;
                }
            }
            /* Set Warm Up Frame Count, 1frame = 10ms */
            if (VOWE_training_setIntegerArgument(vowe_argid_warmUpFrameNum, BACKGROUND_RECORD_SAMPLE_CNT) != vowe_ok) {
                ALOGE("%s, error!!", __FUNCTION__);
                ret = BAD_VALUE;
                break;
            } else {
                ret = OK;
            }
            break;
        default:
            ALOGE("voiceRecognitionInit - mode: %d is unkown", mode);
            ret = BAD_VALUE;
            break;
    }
    return ret;
}

status_t VoiceCmdRecognition::voiceRecognitionRelease(unsigned int mode)
{
    ALOGD("voiceRecognitionRelease in +");
    switch(mode) {
        case VOICE_PW_TRAINING_MODE:
            if (m_Training_Lib_Init == true) {
                if (VOWE_training_release() == vowe_bad) {
                    ALOGE("Training Release init fail!");
                } else {
                    m_Training_Lib_Init = false;
                }
            }
            if (m_PasswordFd >= 0)
                ::close(m_PasswordFd);
            m_PasswordFd = -1;

            if (m_PatternFd >= 0)
                ::close(m_PatternFd);
            m_PatternFd = -1;

            if (m_FeatureFd >= 0)
                ::close(m_FeatureFd);
            m_FeatureFd = -1;
            break;
        case VOICE_PW_RETRAIN_MODE:
            if (m_PasswordFd >= 0)
                ::close(m_PasswordFd);
            m_PasswordFd = -1;

            if (m_PatternFd >= 0)
                ::close(m_PatternFd);
            m_PatternFd = -1;

            if (m_FeatureFd >= 0)
                ::close(m_FeatureFd);
            m_FeatureFd = -1;
            break;
        default:
            ALOGE("voiceRecognitionRelease - mode: %d is unkown", mode);
            break;
    }
    ALOGD("voiceRecognitionRelease in -");
    return OK;
}

status_t VoiceCmdRecognition::startAudioStream()
{
    ALOGD("startAudioStream in +");
    // for create thread timeout
    struct timeval now;
    struct timespec timeout;
    gettimeofday(&now,NULL);
    timeout.tv_sec  = now.tv_sec + 3;
    timeout.tv_nsec = now.tv_usec*1000;

    if (!m_bStarted) {
        if (is_support_dual_mic())
        {
            m_Channels = m_VoiceMode==VOICE_HEADSET_MODE ? 1 : 2;
        }else {
            m_Channels = 1;
        }

        m_pAudioStream = new AudioStream(m_InputSource, m_SampleRate, m_Channels, m_Session);
        AudioSystem::getParameters(0, String8("GET_FSYNC_FLAG=0"));
        AudioSystem::getParameters(0, String8("GET_FSYNC_FLAG=1"));
        m_pAudioStream->enterBackupFilePath(m_strPatternPath);
        if (m_pAudioStream==0 || m_pAudioStream->start()!=OK) {
            ALOGE("start capture voice fail");
            return UNKNOWN_ERROR;
        }

        m_bStarted = true;
        pthread_mutex_lock(&m_RecordMutex);

        m_rb_info1.write_pointer = 0;
        m_rb_info1.read_pointer = 0;
        m_rb_info1.buffer_byte_count = MAX_SAMPLE_LENGTH * sizeof(short);
        m_rb_info1.buffer_base_pointer = (uint8_t *)&m_voiceULBuf1[0];

        m_rb_info2.write_pointer = 0;
        m_rb_info2.read_pointer = 0;
        m_rb_info2.buffer_byte_count = MAX_SAMPLE_LENGTH * sizeof(short);
        m_rb_info2.buffer_base_pointer = (uint8_t *)&m_voiceULBuf2[0];

        pthread_create(&m_Tid, NULL, TrainingVoiceLoop, this);
        pthread_create(&m_ReadDataTid, NULL, captureVoiceLoop, this);
        if (pthread_cond_timedwait(&m_RecordExitCond, &m_RecordMutex, &timeout)==ETIME) {
            if (m_pAudioStream!=0) {
                m_pAudioStream->stop();
            }
            pthread_mutex_unlock(&m_RecordMutex);
            m_bStarted = false;
            return UNKNOWN_ERROR;
        }
        pthread_mutex_unlock(&m_RecordMutex);
    }else {
        ALOGD("startAudioStream already started!");
    }

    return OK;
}

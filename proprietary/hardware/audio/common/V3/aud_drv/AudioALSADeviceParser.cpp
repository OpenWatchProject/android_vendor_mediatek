#include "AudioALSADeviceParser.h"
#include <stdint.h>
#include <dirent.h>
#include <sys/types.h>
#if defined(MTK_AUDIO_KS)
#include <tinyxml.h>
#endif

#include "audio_custom_exp.h"
#include "AudioUtility.h"

#ifdef LOG_TAG
#undef LOG_TAG
#endif
#define LOG_TAG "AudioALSADeviceParser"

#define ALSASOUND_CARD_LOCATION "proc/asound/cards"
#define ALSASOUND_DEVICE_LOCATION "/proc/asound/devices/"
#define ALSASOUND_ASOUND_LOCATION "/proc/asound"
#define ALSASOUND_PCM_LOCATION "/proc/asound/pcm"
#define PROC_READ_BUFFER_SIZE (256)

#define AUDIO_DEVICE_EXT_CONFIG_FILE "/vendor/etc/audio_device.xml"

namespace android {

static String8 keypcmPlayback = String8("playback");
static String8 keypcmCapture = String8("capture");

AudioALSADeviceParser *AudioALSADeviceParser::UniqueAlsaDeviceInstance = NULL;

AudioALSADeviceParser *AudioALSADeviceParser::getInstance() {
    if (UniqueAlsaDeviceInstance == 0) {
        UniqueAlsaDeviceInstance = new AudioALSADeviceParser();
    }
    return UniqueAlsaDeviceInstance;
}

AudioALSADeviceParser::AudioALSADeviceParser() {
    ALOGV("%s()", __FUNCTION__);

#ifdef CONFIG_MT_ENG_BUILD
    mLogEnable = 1;
#else
    mLogEnable = __android_log_is_loggable(ANDROID_LOG_DEBUG, LOG_TAG, ANDROID_LOG_INFO);
#endif

    getCardName();
    ParseCardIndex();
    GetAllPcmAttribute();
#if defined(MTK_AUDIODSP_SUPPORT) || defined(MTK_AUDIO_TUNNELING_SUPPORT)
    GetAllCompressAttribute();
#endif
    QueryPcmDriverCapability();
    dump();
}

void AudioALSADeviceParser::getCardName() {
    bool cardNameSuccess = false;

#if defined(MTK_AUDIO_KS)
    TiXmlDocument doc(AUDIO_DEVICE_EXT_CONFIG_FILE);

    if (doc.LoadFile()) {
        TiXmlElement *root = doc.FirstChildElement("root");

        if (root) {
            TiXmlElement *card = root->FirstChildElement("card");

            if (card) {
                const char *cardName = card->Attribute("name");

                if (cardName) {
                    mCardName = cardName;
                    cardNameSuccess = true;
                } else {
                    ALOGE("%s(), get card name attribute fail", __FUNCTION__);
                }
            } else {
                ALOGE("%s(), no card element", __FUNCTION__);
            }
        } else {
            ALOGE("%s(), no root element", __FUNCTION__);
        }
    } else {
        ALOGE("%s(), LoadFile fail, %s", __FUNCTION__, AUDIO_DEVICE_EXT_CONFIG_FILE);
    }

#else
#if defined(MT_SOUND_CARD_CUSTOM_NAME) //defined in audio_custom_exp.h
    mCardName = MT_SOUND_CARD_CUSTOM_NAME;
#else
    mCardName = keyCardName;
#endif
    cardNameSuccess = true;
#endif

    if (!cardNameSuccess) {
        ALOGE("%s(), get card name fail", __FUNCTION__);
        ASSERT(0);
    } else {
        ALOGD("%s(), card name %s", __FUNCTION__, mCardName.string());
    }
}

void AudioALSADeviceParser::ParseCardIndex() {
    /*
     * $adb shell cat proc/asound/cards
     *  0 [mtsndcard      ]: mt-snd-card - mt-snd-card
     *                       mt-snd-card
     * mCardIndex = 0;
     */
    ALOGV("%s()", __FUNCTION__);
    FILE *mCardFile = NULL;
    char tempbuffer[PROC_READ_BUFFER_SIZE];
    mCardFile = fopen(ALSASOUND_CARD_LOCATION, "r");
    bool isCardIndexFound = false;
    if (mCardFile) {
        ALOGD("card open success");
        while (!feof(mCardFile)) {
            fgets(tempbuffer, PROC_READ_BUFFER_SIZE, mCardFile);

            if (strchr(tempbuffer, '[')) {  // this line contain '[' character
                char *Rch = strtok(tempbuffer, "[");
                mCardIndex = atoi(Rch);
                ALOGD("\tcurrent CardIndex = %d, Rch = %s", mCardIndex, Rch);
                Rch = strtok(NULL, " ]");
                ALOGD("\tcurrent sound card name = %s", Rch);
                if (strcmp(Rch, mCardName.string()) == 0) {
                    ALOGD("\tmCardIndex found = %d", mCardIndex);
                    isCardIndexFound = true;
                    break;
                }
            }

            memset((void *)tempbuffer, 0, PROC_READ_BUFFER_SIZE);
        }
    } else {
        ALOGE("Pcm open fail");
    }

    if (mCardFile) {
        fclose(mCardFile);
    }

    ASSERT(isCardIndexFound);
}

void AudioALSADeviceParser::dump() {
    size_t i = 0;
    ALOGV("dump size = %zu", mAudioDeviceVector.size());
    for (i = 0 ; i < mAudioDeviceVector.size(); i++) {
        AudioDeviceDescriptor *temp = mAudioDeviceVector.itemAt(i);
        ALOGV("name = %s, card index = %d, pcm index = %d, playback  = %d, capture = %d",
              temp->mStreamName.string(), temp->mCardindex, temp->mPcmindex,
              temp->mplayback, temp->mRecord);
    }
}

unsigned int AudioALSADeviceParser::GetPcmBufferSize(unsigned int  pcmindex, unsigned int direction) {
    size_t i = 0;
    unsigned int buffersize = 0;
    for (i = 0 ; i < mAudioDeviceVector.size(); i++) {
        AudioDeviceDescriptor *temp = mAudioDeviceVector.itemAt(i);
        if (temp->mPcmindex ==  pcmindex) {
            if (direction == PCM_OUT) {
                buffersize =  temp->mPlayparam.mBufferBytes;
            } else if (direction == PCM_IN) {
                buffersize =  temp->mRecordparam.mBufferBytes;
            }
            break;
        }
    }
    ALOGD("%s buffersize = %d", __FUNCTION__, buffersize);
    return buffersize;
}


unsigned int  AudioALSADeviceParser::GetPcmIndexByString(String8 stringpair) {
    ALOGV("%s() stringpair = %s ", __FUNCTION__, stringpair.string());
    int pcmindex = -1;
    size_t i = 0;
    for (i = 0 ; i < mAudioDeviceVector.size(); i++) {
        AudioDeviceDescriptor *temp = mAudioDeviceVector.itemAt(i);
        if (temp->mStreamName.compare(stringpair) == 0) {
            pcmindex = temp->mPcmindex;
            ALOGD("compare pcm success = %d, stringpair = %s", pcmindex, stringpair.string());
            break;
        }
    }

    if (pcmindex < 0) {
        for (i = 0 ; i < mAudioComprDevVector.size(); i++) {
            AudioDeviceDescriptor *temp = mAudioComprDevVector.itemAt(i);
            if (temp->mStreamName.compare(stringpair) == 0) {
                pcmindex = temp->mPcmindex;
                ALOGD("Compare compress success id = %d", pcmindex);
                break;
            }
        }
    }
    return pcmindex;
}

unsigned int  AudioALSADeviceParser::GetCardIndexByString(String8 stringpair) {
    ALOGV("%s() stringpair = %s ", __FUNCTION__, stringpair.string());
    int Cardindex = -1;
    size_t i = 0;
    for (i = 0 ; i < mAudioDeviceVector.size(); i++) {
        AudioDeviceDescriptor *temp = mAudioDeviceVector.itemAt(i);
        if (temp->mStreamName.compare(stringpair) == 0) {
            Cardindex = temp->mCardindex;
            ALOGV(" compare success Cardindex = %d", Cardindex);
            break;
        }
    }

    if (Cardindex < 0) {
        for (i = 0 ; i < mAudioComprDevVector.size(); i++) {
            AudioDeviceDescriptor *temp = mAudioComprDevVector.itemAt(i);
            if (temp->mStreamName.compare(stringpair) == 0) {
                Cardindex = temp->mCardindex;
                ALOGD("compare compress Cardindex success = %d", Cardindex);
                break;
            }
        }
    }
    return Cardindex;
}


void AudioALSADeviceParser::SetPcmCapability(AudioDeviceDescriptor *Descriptor, char  *Buffer) {
    if (Buffer == NULL) {
        return;
    }
    String8 CompareString = String8(Buffer);
    //ALOGD("SetPcmCapability CompareString = %s",CompareString.string ());
    if ((CompareString.compare(keypcmPlayback)) == 0) {
        //ALOGD("SetPcmCapability playback support");
        Descriptor->mplayback = 1;
    }
    if ((CompareString.compare(keypcmCapture)) == 0) {
        //ALOGD("SetPcmCapability capture support");
        Descriptor->mRecord = 1;
    }
}

void AudioALSADeviceParser::AddPcmString(char *InputBuffer) {
    ALOGD_IF(mLogEnable, "AddPcmString InputBuffer = %s", InputBuffer);
    char *Rch;
    char *rest_of_str = NULL;
    AudioDeviceDescriptor *mAudiioDeviceDescriptor = NULL;
    Rch = strtok_r(InputBuffer, "-", &rest_of_str);
    // parse for stream name
    if (Rch  != NULL) {
        mAudiioDeviceDescriptor = new AudioDeviceDescriptor();
        mAudiioDeviceDescriptor->mCardindex = atoi(Rch);
        Rch = strtok_r(NULL, ":", &rest_of_str);
        mAudiioDeviceDescriptor->mPcmindex = atoi(Rch);
        Rch = strtok_r(NULL, ": ", &rest_of_str);
        mAudiioDeviceDescriptor->mStreamName = String8(Rch);
        Rch = strtok_r(NULL, ": ", &rest_of_str);
        mAudioDeviceVector.push(mAudiioDeviceDescriptor);
    }
    // parse for playback or record support
    while (Rch  != NULL) {
        Rch = strtok_r(NULL, ": ", &rest_of_str);
        if (mAudiioDeviceDescriptor != NULL) {
            SetPcmCapability(mAudiioDeviceDescriptor, Rch);
        }
    }
}

void AudioALSADeviceParser::GetAllPcmAttribute(void) {
    ALOGV("%s()", __FUNCTION__);
    FILE *mPcmFile = NULL;
    char tempbuffer[PROC_READ_BUFFER_SIZE];
    int result;
    mPcmFile = fopen(ALSASOUND_PCM_LOCATION, "r");
    if (mPcmFile) {
        ALOGD("%s(), Pcm open success", __FUNCTION__);
        while (!feof(mPcmFile)) {
            fgets(tempbuffer, PROC_READ_BUFFER_SIZE, mPcmFile);
            AddPcmString(tempbuffer);
            memset((void *)tempbuffer, 0, PROC_READ_BUFFER_SIZE);
        }
    } else {
        ALOGD("%s(), Pcm open fail", __FUNCTION__);
    }

    if (mPcmFile) {
        fclose(mPcmFile);
    }
}

void AudioALSADeviceParser::GetAllCompressAttribute(void) {
    ALOGV("%s()", __FUNCTION__);
    FILE *compress_file = NULL;
    char tempbuffer[PROC_READ_BUFFER_SIZE];
    char compress_info_path[PROC_READ_BUFFER_SIZE];
    int result;
    struct dirent *de;
    snprintf(compress_info_path, sizeof(compress_info_path),
             "%s/card%d/", ALSASOUND_ASOUND_LOCATION, mCardIndex);

    DIR *d = opendir(compress_info_path);
    if (d == 0) {
        ALOGD("%s(), Soundcard path open fail", __FUNCTION__);
        return;
    }

    while ((de = readdir(d)) != NULL) {
        if (strncmp(de->d_name, "compr", 5) == 0) {
            strncat(compress_info_path, de->d_name, strlen(de->d_name));
            strncat(compress_info_path, "/info", 5);
            compress_file = fopen(compress_info_path, "r");
            if (compress_file) {
                ALOGD("%s(), Compress info open success", __FUNCTION__);
                char *Rch;
                char *rest_of_str = NULL;
                AudioDeviceDescriptor *mAudiioDeviceDescriptor = NULL;
                mAudiioDeviceDescriptor = new AudioDeviceDescriptor();
                while (!feof(compress_file)) {
                    fgets(tempbuffer, PROC_READ_BUFFER_SIZE, compress_file);
                    Rch = strtok_r(tempbuffer, ": ", &rest_of_str);
                    if (Rch  != NULL) {
                        if (strncmp(Rch, "card", 4) == 0) {
                            mAudiioDeviceDescriptor->mCardindex = atoi(rest_of_str);
                        } else if (strncmp(Rch, "device", 6) == 0) {
                            mAudiioDeviceDescriptor->mPcmindex = atoi(rest_of_str);
                        } else if (strncmp(Rch, "stream", 6) == 0) {
                            Rch = strtok_r(NULL, ": ", &rest_of_str);
                            if ((strncmp(Rch, "PLAYBACK", 8)) == 0) {
                                mAudiioDeviceDescriptor->mplayback = 1;
                            } else if ((strncmp(Rch, "CAPTURE", 7)) == 0) {
                                mAudiioDeviceDescriptor->mRecord = 1;
                            }
                        } else if ((strncmp(Rch, "id", 2)) == 0) {
                            Rch = strtok_r(NULL, " ", &rest_of_str);
                            mAudiioDeviceDescriptor->mStreamName = String8(Rch);
                        }
                    }
                    memset((void *)tempbuffer, 0, PROC_READ_BUFFER_SIZE);
                }
                mAudioComprDevVector.push(mAudiioDeviceDescriptor);
            } else {
                ALOGD("%s(), Compress file open fail", __FUNCTION__);
            }
        }
    }
    if (compress_file) {
        fclose(compress_file);
    }
    if (d) {
        closedir(d);
    }
}

status_t AudioALSADeviceParser::GetPcmDriverparameters(AudioPcmDeviceparam *PcmDeviceparam, struct pcm_params *params) {
    PcmDeviceparam->mRateMin = pcm_params_get_min(params, PCM_PARAM_RATE);
    PcmDeviceparam->mRateMax = pcm_params_get_max(params, PCM_PARAM_RATE);
    ALOGD_IF(mLogEnable, "Rate:\tmin=%uHz\tmax=%uHz\n", PcmDeviceparam->mRateMin, PcmDeviceparam->mRateMax);

    PcmDeviceparam->mChannelMin = pcm_params_get_min(params, PCM_PARAM_CHANNELS);
    PcmDeviceparam->mChannelMax = pcm_params_get_max(params, PCM_PARAM_CHANNELS);
    ALOGD_IF(mLogEnable, "Channels:\tmin=%u\t\tmax=%u\n", PcmDeviceparam->mChannelMin, PcmDeviceparam->mChannelMax);

    PcmDeviceparam->mSampleBitMin = pcm_params_get_min(params, PCM_PARAM_SAMPLE_BITS);
    PcmDeviceparam->mSampleBitMax = pcm_params_get_max(params, PCM_PARAM_SAMPLE_BITS);
    ALOGD_IF(mLogEnable, "Sample bits:\tmin=%u\t\tmax=%u\n", PcmDeviceparam->mSampleBitMin, PcmDeviceparam->mSampleBitMax);

    PcmDeviceparam->mPreriodSizeMin = pcm_params_get_min(params, PCM_PARAM_PERIOD_SIZE);
    PcmDeviceparam->mPreriodSizeMax = pcm_params_get_max(params, PCM_PARAM_PERIOD_SIZE);
    ALOGD_IF(mLogEnable, "Period size:\tmin=%u\t\tmax=%u\n", PcmDeviceparam->mPreriodSizeMin, PcmDeviceparam->mPreriodSizeMax);

    PcmDeviceparam->mPreriodCountMin = pcm_params_get_min(params, PCM_PARAM_PERIODS);
    PcmDeviceparam->mPreriodCountMax = pcm_params_get_max(params, PCM_PARAM_PERIODS);
    ALOGD_IF(mLogEnable, "Period count:\tmin=%u\t\tmax=%u\n", PcmDeviceparam->mPreriodCountMin,    PcmDeviceparam->mPreriodCountMax);

    PcmDeviceparam->mBufferBytes = pcm_params_get_max(params, PCM_PARAM_BUFFER_BYTES);
    ALOGD_IF(mLogEnable, "PCM_PARAM_BUFFER_BYTES :\t max=%u\t\n", PcmDeviceparam->mBufferBytes);

    return NO_ERROR;
}


status_t AudioALSADeviceParser::QueryPcmDriverCapability() {
    struct pcm_params *params = NULL;
    AudioDeviceDescriptor *temp = NULL;;
    int Direction = 0;
    for (size_t i = 0; i < mAudioDeviceVector.size(); i++) {
        temp = mAudioDeviceVector.itemAt(i);
        ALOGD_IF(mLogEnable, "pcm %d %s %s, pcm mCardindex %d mPcmindex %d", temp->mPcmindex, temp->mStreamName.string(),
                 temp->mCodecName.string(), temp->mCardindex, temp->mPcmindex);
        if (temp->mplayback == 1) {
            params = pcm_params_get(temp->mCardindex, temp->mPcmindex, PCM_OUT);
            if (params == NULL) {
                ALOGD("Device %zu does not exist playback", i);
            } else {
                if (temp->mplayback == 1) {
                    GetPcmDriverparameters(&temp->mPlayparam, params);
                }
            }
            if (params != NULL) {
                pcm_params_free(params);
                params = NULL;
            }
        }

        if (temp->mRecord == 1) {
            params = pcm_params_get(temp->mCardindex, temp->mPcmindex, PCM_IN);
            if (params == NULL) {
                ALOGD("Device %zu does not exist capture", i);
            } else {
                if (temp->mRecord == 1) {
                    GetPcmDriverparameters(&temp->mRecordparam, params);
                }
            }
            if (params != NULL) {
                pcm_params_free(params);
                params = NULL;
            }
        }
    }
    return NO_ERROR;
}


}

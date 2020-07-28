#include <errno.h>
#include <pthread.h>
#include <stdint.h>
#include <sys/time.h>
#include <stdlib.h>
#include <math.h>
#include <dlfcn.h>
#include <sys/resource.h>
#include <sys/prctl.h>
#include <utils/threads.h>
#include <dlfcn.h>

#include "AudioALSAPlaybackHandlerSpeakerProtection.h"
#include "AudioALSAHardwareResourceManager.h"
#include "AudioVolumeFactory.h"
#include "AudioMTKFilter.h"
#include "AudioALSADeviceParser.h"
#include "AudioALSADriverUtility.h"
#include "AudioALSASampleRateController.h"
#include "AudioALSAStreamManager.h"

#if defined(MTK_AUDIO_KS)
#include "AudioALSADeviceConfigManager.h"

#define SPK_PROTECT_AP_SEQUENCE AUDIO_CTL_PLAYBACK1_TO_ADDA_I2S
#endif

#include <mlds_api.h>

#ifdef LOG_TAG
#undef LOG_TAG
#endif
#define LOG_TAG "AudioSpkProtect"

// define this to by pass processing
//#define BY_PASS_PROCESSING
#ifndef ARRAY_SIZE
#define ARRAY_SIZE(a) (sizeof(a) / sizeof((a)[0]))
#endif


/*
* need add config for DSM lib
*/

extern "C" {
#include <cutils/str_parms.h>
#include <cutils/hashmap.h>
#include <cutils/memory.h>
#include <log/log.h>
#include <cutils/properties.h>
}

static const char *SpeakerLibPath[] = {
    "/vendor/lib/dsm_interface.so",
    "/system/lib/libdsm_interface.so"
};

static const char *SpeakerLib64Path[] = {
    "/vendor/lib64/dsm_interface.so",
    "/system/lib64/libdsm_interface.so"
};

#define SPEAKER_LIB_PATH ("libdsm_interface.so")
#define DSM_LIB_PATH ("libdsm.so")
#define SPEAKER_CONFIGPARSER_LIB_PATH ("libdsmconfigparser.so")

//#define DEBUG_LATENCY

#define calc_time_diff(x,y) ((x.tv_sec - y.tv_sec )+ (double)( x.tv_nsec - y.tv_nsec ) / (double)1000000000)
static  const char PROPERTY_KEY_BYPASSPROTECTION[PROPERTY_KEY_MAX]  = "vendor.speakerprotection_bypass";

namespace android {

AudioALSAPlaybackHandlerSpeakerProtection::AudioALSAPlaybackHandlerSpeakerProtection(const stream_attribute_t *stream_attribute_source) :
    AudioALSAPlaybackHandlerBase(stream_attribute_source) {
    ALOGD("%s()", __FUNCTION__);
    mPlaybackHandlerType = PLAYBACK_HANDLER_SPEAKERPROTECTION;
    mDumpPlayIVFileNum = 0;
    mDsmpcmOut = NULL;
    mDsmiData = NULL;
    mDsmvData = NULL;
    mSpeakerChannelProcessingBuffer = NULL;
    mDsmProcessingbuffer = NULL;
    mDsmIvReadData = NULL;
    mPCMPlayIVDumpFile = NULL;
    dsm_handler = NULL;
    mDsmMemBuffer = NULL;
    memset((void *)&mPcmIvConfig, 0, sizeof(pcm_config));
    mBypassSpeakerProtection = (bool)AudioALSADriverUtility::getInstance()->GetPropertyValue(PROPERTY_KEY_BYPASSPROTECTION);
    mAudioAlsaPcmIn = AudioALSAPcmDataCaptureIn::getInstance();
    mMlds_handle = NULL;
#ifdef DEBUG_LATENCY
    memset((void *)&mNewtime, 0, sizeof(timespec));
    memset((void *)&mOldtime, 0, sizeof(timespec));
#endif
    mu4DsmMemSize  = 0;
    mDsmMemSize = 0;
    remaining_bytes  = 0;
    mDsmSamples  = 0;
    mDsmChannels = 0;
    mInputChannels  = 0;
    mDsmSampleRate = 0;

    mmlds_task_config = (mlds_task_config_t *)malloc(sizeof(mlds_task_config_t));
    ASSERT(mmlds_task_config != NULL);
    memset((void *)mmlds_task_config, 0, sizeof(mlds_task_config_t));

    mMlds_Interace_pointer = (mlds_interface *)malloc(sizeof(mlds_interface));
    ASSERT(mMlds_Interace_pointer != NULL);
    memset((void *)mMlds_Interace_pointer, 0, sizeof(mlds_interface));
    mDsmConfigHandle = NULL;
    mDsmInterfaceHandle = NULL;
}

AudioALSAPlaybackHandlerSpeakerProtection::~AudioALSAPlaybackHandlerSpeakerProtection() {
    ALOGD("%s()", __FUNCTION__);

    if (mmlds_task_config != NULL) {
        free(mmlds_task_config);
    }

    if (mMlds_Interace_pointer != NULL) {
        free(mMlds_Interace_pointer);
    }
}

uint32_t AudioALSAPlaybackHandlerSpeakerProtection::ChooseTargetSampleRate(uint32_t SampleRate) {
    //ALOGD("ChooseTargetSampleRate SampleRate = %d outputdevice = %d", SampleRate, outputdevice);
    uint32_t TargetSampleRate = 48000;

    if (SampleRate >= 48000) {
        TargetSampleRate = 48000;
    } else if (SampleRate < 48000 && SampleRate >= 44100) {
        TargetSampleRate = 44100;
    } else if (SampleRate == 16000) {
        TargetSampleRate = 16000;
    }
    return TargetSampleRate;
}

#ifdef MTK_MAXIM_SPEAKER_SUPPORT
static int audio_get_parameters(struct str_parms *query,
                                struct str_parms *reply, int *param, void *dsm_handler) {
    int ret, val;
    char value[32] = {0};
    char *str = NULL;

#define AUDIO_DSM_PARAMETER "DSM"
    ret = str_parms_get_str(query, AUDIO_DSM_PARAMETER, value,
                            sizeof(value));
    if (ret >= 0) {
        ret = DsmGetParams(dsm_handler, 1, param);
        if (NO_ERROR != ret) {
            ALOGE("%s: DSM get params failed", __func__);
        }
        ALOGE("%s: DSM get params %d", __func__, param[1]);
        str_parms_add_int(reply, AUDIO_DSM_PARAMETER, param[1]);
    }

    return 0;
}

static void audio_extn_dsm_parameters(struct str_parms *query,
                                      struct str_parms *reply, int *param, void *dsm_handler) {
    char *kv_pairs = NULL;
    audio_get_parameters(query, reply, param, dsm_handler);

    kv_pairs = str_parms_to_str(reply);
    ALOGD_IF(kv_pairs != NULL, "%s: returns %s", __func__, kv_pairs);
    free(kv_pairs);
}

status_t AudioALSAPlaybackHandlerSpeakerProtection::setParameters(const String8 &keyValuePairs) {
    ALOGD("%s %s", __FUNCTION__, keyValuePairs.string());
    struct str_parms *parms = NULL;
    char *str;
    char value[32];
    int val;
    int ret = 0, err;
    parms = str_parms_create_str(keyValuePairs.string());

    {
        int param[4];
        /*parse the id and value*/
        err = str_parms_get_str(parms, "DSM", value, sizeof(value));
        if (err >= 0) {
            char *id, *cp, *ptr;
            const char delimiters[] = " ";
            double ret;

            cp = strdup(value);
            id = strtok(cp, delimiters);
            param[2 * 0] = strtol(id, NULL, 10);
            id = strtok(NULL, delimiters);
            ret = strtod(id, &ptr);
            param[2 * 0 + 1] = ret;
            ALOGD("param[1] = %d", param[1]);
            err = DsmSetParams(dsm_handler, 1, param);
            if (NO_ERROR != err) {
                ALOGE("%s: DSM set params failed err = %d", __func__, err);
            }
            free(cp);
        }
    }
    ALOGD("-%s %s", __FUNCTION__, keyValuePairs.string());
    str_parms_destroy(parms);
    return NO_ERROR;
}

#else
status_t AudioALSAPlaybackHandlerSpeakerProtection::setParameters(const String8 &keyValuePairs) {
    ALOGD("%s %s", __FUNCTION__, keyValuePairs.string());
    return NO_ERROR;
}
#endif

#ifdef MTK_MAXIM_SPEAKER_SUPPORT
String8  AudioALSAPlaybackHandlerSpeakerProtection::getParameters(const String8 &keys) {
    ALOGD("%s %s", __FUNCTION__, keys.string());
    struct str_parms *reply = str_parms_create();
    struct str_parms *query = str_parms_create_str(keys.string());
    char *str = NULL;

    {
        ALOGE("%s: enter: %s", __func__, keys.string());
        int param[4], err;
        char value[32], *id;
        /*parse the id and value*/
        err = str_parms_get_str(query, "DSM", value, sizeof(value));
        if (err >= 0) {
            char *id, *cp, *ptr;
            const char delimiters[] = " ";
            double ret;

            cp = strdup(value);
            id = strtok(cp, delimiters);
            param[0] = strtol(id, NULL, 10);
            ALOGE("%s: param %d id = %s", __func__, param[0], id);
            free(cp);
        }
        ALOGD("%s , dsm_handler = %p", __FUNCTION__, dsm_handler);
        audio_extn_dsm_parameters(query, reply, param, dsm_handler);
    }
    str = str_parms_to_str(reply);

    str_parms_destroy(query);
    str_parms_destroy(reply);

    ALOGE("-%s: enter", __func__);
    String8 ReturnValue(str);
    free(str);
    return ReturnValue;
}

#else
String8  AudioALSAPlaybackHandlerSpeakerProtection::getParameters(const String8 &keys) {
    ALOGD("%s %s", __FUNCTION__, keys.string());
    struct str_parms *reply = str_parms_create();
    struct str_parms *query = str_parms_create_str(keys.string());
    char *str = NULL;
    String8 ReturnValue(str);
    return ReturnValue;
}
#endif

status_t AudioALSAPlaybackHandlerSpeakerProtection::open() {
    ALOGD("+%s(), mDevice = 0x%x", __FUNCTION__, mStreamAttributeSource->output_devices);

    // acquire pmic clk
    mHardwareResourceManager->EnableAudBufClk(true);
#if defined(MTK_AUDIO_KS)
    int pcmindex = AudioALSADeviceParser::getInstance()->GetPcmIndexByString(keypcmPlayback1);
    int cardindex = AudioALSADeviceParser::getInstance()->GetCardIndexByString(keypcmPlayback1);
    AudioALSADeviceConfigManager::getInstance()->ApplyDeviceTurnonSequenceByName(SPK_PROTECT_AP_SEQUENCE);
#else
    int pcmindex = AudioALSADeviceParser::getInstance()->GetPcmIndexByString(keypcmI2S0Dl1Playback);
    int cardindex = AudioALSADeviceParser::getInstance()->GetCardIndexByString(keypcmI2S0Dl1Playback);
#endif

    mStreamAttributeTarget.buffer_size =  AudioALSADeviceParser::getInstance()->GetPcmBufferSize(pcmindex, PCM_OUT);

#ifdef PLAYBACK_USE_24BITS_ONLY
    mStreamAttributeTarget.audio_format = AUDIO_FORMAT_PCM_8_24_BIT;
#else
    mStreamAttributeTarget.audio_format = (mStreamAttributeSource->audio_format == AUDIO_FORMAT_PCM_32_BIT) ? AUDIO_FORMAT_PCM_8_24_BIT : AUDIO_FORMAT_PCM_16_BIT;
#endif
    mStreamAttributeTarget.audio_channel_mask = AUDIO_CHANNEL_IN_STEREO;
    mStreamAttributeTarget.num_channels = popcount(mStreamAttributeTarget.audio_channel_mask);

    mStreamAttributeTarget.sample_rate = ChooseTargetSampleRate(AudioALSASampleRateController::getInstance()->getPrimaryStreamOutSampleRate());

    ALOGD("mStreamAttributeTarget.sample_rate = %d", mStreamAttributeTarget.sample_rate);

#ifdef HIFI_DEEP_BUFFER
    if (mStreamAttributeTarget.sample_rate <= 48000) {
        mStreamAttributeTarget.buffer_size = KERNEL_BUFFER_SIZE_DL1_NORMAL;
    } else if (mStreamAttributeTarget.sample_rate > 48000 && mStreamAttributeTarget.sample_rate <= 96000) {
        if (mStreamAttributeTarget.buffer_size >= KERNEL_BUFFER_SIZE_DL1_HIFI_96K) {
            mStreamAttributeTarget.buffer_size = KERNEL_BUFFER_SIZE_DL1_HIFI_96K;
        }
    } else {
        if (mStreamAttributeTarget.buffer_size >= KERNEL_BUFFER_SIZE_DL1_HIFI_192K) {
            mStreamAttributeTarget.buffer_size = KERNEL_BUFFER_SIZE_DL1_HIFI_192K;
        }
    }
#endif

    // HW pcm config
    mConfig.channels = mStreamAttributeTarget.num_channels;
    mConfig.rate = mStreamAttributeTarget.sample_rate;

    if (mStreamAttributeTarget.sample_rate > 16000) {

        mConfig.period_count = 2;
        mConfig.period_size = (mStreamAttributeTarget.buffer_size / (mConfig.channels * mConfig.period_count)) / ((mStreamAttributeTarget.audio_format == AUDIO_FORMAT_PCM_16_BIT) ? 2 : 4);

        mConfig.format = transferAudioFormatToPcmFormat(mStreamAttributeTarget.audio_format);

        mConfig.start_threshold = 0;
        mConfig.stop_threshold = 0;
        mConfig.silence_threshold = 0;
        ALOGD("%s(), mConfig: channels = %d, rate = %d, period_size = %d, period_count = %d, format = %d",
              __FUNCTION__, mConfig.channels, mConfig.rate, mConfig.period_size, mConfig.period_count, mConfig.format);
    } else { // voice playback , need low latency.
        mConfig.period_count = 12;
        mConfig.period_size = 160;

        mConfig.format = transferAudioFormatToPcmFormat(mStreamAttributeTarget.audio_format);

        mConfig.start_threshold = mConfig.period_size * 3;
        mConfig.stop_threshold = 0;
        mConfig.silence_threshold = 0;
        ALOGD("%s(), mConfig: channels = %d, rate = %d, period_size = %d, period_count = %d, format = %d",
              __FUNCTION__, mConfig.channels, mConfig.rate, mConfig.period_size, mConfig.period_count, mConfig.format);
    }

    // debug pcm dump
    OpenPCMDump(LOG_TAG);

    // post processing
    initPostProcessing();

    // SRCS_ESTSAMPLERATE
    initBliSrc();

    // bit conversion
    initBitConverter();

    initDataPending();

    // init DC Removal
    initDcRemoval();

    // open pcm driver
    openPcmDriver(pcmindex);

    // open codec driver
    if (mStreamAttributeSource->output_devices != AUDIO_DEVICE_OUT_SPEAKER_SAFE) {
        mHardwareResourceManager->startOutputDevice(mStreamAttributeSource->output_devices, mStreamAttributeTarget.sample_rate);
    }

    // maxmim init
    Initmldsconfig(mStreamAttributeSource, &mStreamAttributeTarget);
    //SpeakerProtectionInit(mStreamAttributeTarget.sample_rate, mStreamAttributeTarget.num_channels, mStreamAttributeTarget.audio_format);
    SpeakerProtectionInterfaceInit(mStreamAttributeTarget.sample_rate, mStreamAttributeTarget.num_channels, mStreamAttributeTarget.audio_format);


    pcm_config mIVConfig;
    memcpy((void *)&mIVConfig, (void *)&mConfig, sizeof(pcm_config));
    if (mStreamAttributeTarget.sample_rate > 16000) {
        mIVConfig.period_size = mIVConfig.period_size >> 1;
    }
    ALOGD("%s(), mIVConfig: channels = %d, rate = %d, period_size = %d, period_count = %d, format = %d",
          __FUNCTION__, mIVConfig.channels, mIVConfig.rate, mIVConfig.period_size, mIVConfig.period_count, mIVConfig.format);

    EnableIVTask(mIVConfig);

    ALOGD("-%s()", __FUNCTION__);
    return NO_ERROR;
}


status_t AudioALSAPlaybackHandlerSpeakerProtection::close() {
    ALOGD("+%s()", __FUNCTION__);

    if (mStreamAttributeSource->output_devices != AUDIO_DEVICE_OUT_SPEAKER_SAFE) {
        mHardwareResourceManager->stopOutputDevice();
    }

    // close pcm driver
    closePcmDriver();

#if defined(MTK_AUDIO_KS)
    AudioALSADeviceConfigManager::getInstance()->ApplyDeviceTurnoffSequenceByName(SPK_PROTECT_AP_SEQUENCE);
#endif

    //DC removal
    deinitDcRemoval();

    DeinitDataPending();

    // bit conversion
    deinitBitConverter();

    // SRC
    deinitBliSrc();

    // post processing
    deinitPostProcessing();

    // debug pcm dump
    ClosePCMDump();

    //release pmic clk
    mHardwareResourceManager->EnableAudBufClk(false);

    //ClosePCMIVDump();
    DisableIVTask();

    SpeakerProtectionInterfaceDeinit();

    ALOGD("-%s()", __FUNCTION__);
    return NO_ERROR;
}


status_t AudioALSAPlaybackHandlerSpeakerProtection::routing(const audio_devices_t output_devices) {
    mHardwareResourceManager->changeOutputDevice(output_devices);
#if !defined(MTK_AURISYS_FRAMEWORK_SUPPORT)
    if (mAudioFilterManagerHandler) { mAudioFilterManagerHandler->setDevice(output_devices); }
#endif
    return NO_ERROR;
}

status_t AudioALSAPlaybackHandlerSpeakerProtection::SpeakerProtectionInterfaceInit(unsigned int SampleRate, unsigned int channels, unsigned int Format) {
    ALOGD("+%s SampleRate = %d channels = %d Format = %d", __FUNCTION__, SampleRate, channels, Format);
    unsigned int retCode = 0;
    int i = 0;
    retCode = InitmldsInterface();
    if (dsm_handler == NULL && retCode == NO_ERROR) {
        //retCode = mlds_query_working_buf_size(mmlds_task_config,&mu4DsmMemSize);
        retCode = mMlds_Interace_pointer->mlds_query_working_buf_size(mmlds_task_config, &mu4DsmMemSize);

        if (LIB_OK == retCode) {
            if (dsm_handler == NULL) {
                dsm_handler =  new int[mu4DsmMemSize];
                memset((void *)dsm_handler, 0, mu4DsmMemSize);
                retCode = mMlds_Interace_pointer->mlds_create_handler(mmlds_task_config, 0, NULL, 0, NULL, &dsm_handler);
                ALOGD("- mMlds_Interace_pointer mlds_create_handler dsm_handler = %p", dsm_handler);
            }

            if (0 == dsm_handler) {
                ALOGD("dsm_handler new is %p", dsm_handler);
            }
        } else {
            ALOGD("retCode is %d", retCode);
        }
    } else {
        ALOGE("DSM Handle exist = %p", dsm_handler);
    }

    SpeakerBufferInit();
    if (dsm_handler) {
        DsmConfigure(2, dsm_handler);
    }

    ALOGD("-%s", __FUNCTION__);
    return NO_ERROR;
}

status_t AudioALSAPlaybackHandlerSpeakerProtection::Initmldsconfig(const stream_attribute_t *mStreamAttributeSource, stream_attribute_t *mStreamAttributeTarget) {
    ALOGD("%s", __FUNCTION__);
    mmlds_task_config->api_version = 0x01;
    mmlds_task_config->frame_size_ms = 5;
    mmlds_task_config->vendor_id = 0x10;
    mmlds_task_config->task_scene = TASK_SCENE_SPEAKER_PROTECTION;

    // information about dl playback
    if (mStreamAttributeSource->audio_format == AUDIO_FORMAT_PCM_32_BIT || mStreamAttributeSource->audio_format == AUDIO_FORMAT_PCM_8_24_BIT) {
        mmlds_task_config->stream_downlink.bit_format_in = BIT_FORMAT_S32_LE;
        mmlds_task_config->stream_downlink.bit_format_out = BIT_FORMAT_S32_LE;
    } else {
        mmlds_task_config->stream_downlink.bit_format_in = BIT_FORMAT_S16_LE;
        mmlds_task_config->stream_downlink.bit_format_out = BIT_FORMAT_S16_LE;
    }

    mmlds_task_config->stream_downlink.num_channels_in = mStreamAttributeTarget->num_channels;
    mmlds_task_config->stream_downlink.num_channels_out = mStreamAttributeTarget->num_channels;

    mmlds_task_config->stream_downlink.sample_rate_in = mStreamAttributeTarget->sample_rate;
    mmlds_task_config->stream_downlink.sample_rate_out = mStreamAttributeTarget->sample_rate;

    // only for speaker mode
    mmlds_task_config->stream_downlink.device = TASK_DEVICE_OUT_SPEAKER;
    mmlds_task_config->stream_downlink.device_extra_info = 1; // processing channel

    // information about ref buf
    mmlds_task_config->stream_dl_fbk.bit_format_in = BIT_FORMAT_S16_LE;
    mmlds_task_config->stream_dl_fbk.bit_format_out = BIT_FORMAT_S16_LE;

    mmlds_task_config->stream_dl_fbk.num_channels_in = 2;
    mmlds_task_config->stream_dl_fbk.num_channels_out = 2;

    mmlds_task_config->stream_dl_fbk.sample_rate_in = mStreamAttributeTarget->sample_rate;
    mmlds_task_config->stream_dl_fbk.sample_rate_out = mStreamAttributeTarget->sample_rate;

    mmlds_task_config->stream_downlink.device = TASK_DEVICE_OUT_SPEAKER;

    return NO_ERROR;
}

status_t AudioALSAPlaybackHandlerSpeakerProtection::SpeakerBufferInit(void) {
    ALOGD("+%s", __FUNCTION__);
    if (mDsmpcmOut == NULL) {
        mDsmpcmOut = new int [mDsmBufferSize];
        memset((void *)mDsmpcmOut, 0, mDsmBufferSize * sizeof(int));
    }
    if (mDsmiData == NULL) {
        mDsmiData = new int [mDsmBufferSize];
        memset((void *)mDsmiData, 0, mDsmBufferSize * sizeof(int));
    }
    if (mDsmvData == NULL) {
        mDsmvData = new int [mDsmBufferSize];
        memset((void *)mDsmvData, 0, mDsmBufferSize * sizeof(int));
    }

    if (mDsmProcessingbuffer == NULL) {
        mDsmProcessingbuffer = new int [mDsmBufferSize];
        memset((void *)mDsmProcessingbuffer, 0, mDsmBufferSize * sizeof(int));
    }

    if (mDsmIvReadData == NULL) {
        mDsmIvReadData = new int [mDsmBufferSize];
        memset((void *)mDsmIvReadData, 0, mDsmBufferSize * sizeof(int));
    }

    if (mSpeakerChannelProcessingBuffer == NULL) {
        mSpeakerChannelProcessingBuffer = new int [mDsmBufferSize];
        memset((void *)mSpeakerChannelProcessingBuffer, 0, mDsmBufferSize * sizeof(int));
    }
    ALOGD("-%s", __FUNCTION__);
    return NO_ERROR;
}

status_t AudioALSAPlaybackHandlerSpeakerProtection::SpeakerProtectionInterfaceDeinit() {
    ALOGD("%s", __FUNCTION__);
    int retCode = 0;
    if (dsm_handler != NULL) {
        retCode = mMlds_Interace_pointer->mlds_destroy_handler(dsm_handler);
        if (dsm_handler != NULL) {
            delete[](int *)dsm_handler;
            dsm_handler = NULL;
        }
    }

    if (mDsmMemBuffer) {
        delete[] mDsmMemBuffer;
        mDsmMemBuffer = NULL;
    }

    SpeakerBufferDeInit();
    return NO_ERROR;
}

status_t AudioALSAPlaybackHandlerSpeakerProtection::SpeakerBufferDeInit(void) {
    ALOGD("+%s", __FUNCTION__);
    if (mDsmpcmOut != NULL) {
        delete []mDsmpcmOut;
        mDsmpcmOut = NULL;
    }
    if (mDsmiData != NULL) {
        delete []mDsmiData;
        mDsmiData = NULL;
    }

    if (mDsmvData != NULL) {
        delete []mDsmvData;
        mDsmvData = NULL;
    }

    if (mDsmProcessingbuffer != NULL) {
        delete []mDsmProcessingbuffer;
        mDsmProcessingbuffer = NULL;
    }

    if (mDsmIvReadData != NULL) {
        delete []mDsmIvReadData;
        mDsmIvReadData = NULL;
    }

    if (mSpeakerChannelProcessingBuffer != NULL) {
        delete []mSpeakerChannelProcessingBuffer;
        mSpeakerChannelProcessingBuffer = NULL;
    }

    ALOGD("-%s", __FUNCTION__);
    return NO_ERROR;
}

status_t AudioALSAPlaybackHandlerSpeakerProtection::setScreenState(bool mode, size_t buffer_size, size_t reduceInterruptSize, bool bforce) {
    // don't increase irq period when play hifi
    if (mode == 0 && mStreamAttributeSource->sample_rate > 48000) {
        return NO_ERROR;
    }

    if (0 == buffer_size) {
        buffer_size = mStreamAttributeSource->buffer_size;
    }

    int rate = mode ? (buffer_size / mStreamAttributeSource->num_channels) / ((mStreamAttributeSource->audio_format == AUDIO_FORMAT_PCM_16_BIT) ? 2 : 4) :
               ((mStreamAttributeTarget.buffer_size >> 1) / mConfig.channels) / ((mStreamAttributeTarget.audio_format == AUDIO_FORMAT_PCM_16_BIT) ? 2 : 4);

    // check for low latency mode interrupt rate
    if (mode == true) {
        int mConfigRate =  mConfig.period_count * mConfig.period_size / 2; // at least ping pong

        while ((rate > mConfigRate) && (rate > 0)) {
            rate = rate / 2 ;
        }
        ALOGD("mode = %d rate = %d mConfigRate = %d", mode, rate, mConfigRate);
    }

    mStreamAttributeTarget.mInterrupt = (rate + 0.0) / mStreamAttributeTarget.sample_rate;

    ALOGD("%s, rate %d %f, mode = %d , buffer_size = %d, channel %d, format%d", __FUNCTION__, rate, mStreamAttributeTarget.mInterrupt, mode, buffer_size, mConfig.channels, mStreamAttributeTarget.audio_format);

    mHardwareResourceManager->setInterruptRate(mStreamAttributeSource->mAudioOutputFlags, rate);
    return NO_ERROR;
}

status_t AudioALSAPlaybackHandlerSpeakerProtection::DoSpeakerProctionPacking(int *pbuffer, int ValidInputBuffersize) {
    // first copy data to processing buffer with ValidInputBuffersize
    memcpy((void *)mSpeakerChannelProcessingBuffer, (void *)pbuffer, ValidInputBuffersize);
    int *ppostProcessingL = pbuffer;
    int *ppostProcessingR = (int *)((char *)pbuffer + (ValidInputBuffersize / 2));
    int *pTempprocessingBuffer = mSpeakerChannelProcessingBuffer;


    while (ValidInputBuffersize > 0) {
        *ppostProcessingL = *pTempprocessingBuffer;
        ppostProcessingL++;
        pTempprocessingBuffer++;
        *ppostProcessingR = *pTempprocessingBuffer;
        ppostProcessingR++;
        pTempprocessingBuffer++;
        ValidInputBuffersize -= 8;
    }

    return NO_ERROR;
}

status_t AudioALSAPlaybackHandlerSpeakerProtection::MonoToStereo(int *pbuffer, int ValidInputBuffersize) {
    if (DSM_CHANNELS == 2) {
        return NO_ERROR;
    }

    int *pTempprocessingBuffer = pbuffer + 1;

    while (ValidInputBuffersize > 0) {
        *pTempprocessingBuffer = *pbuffer;
        pTempprocessingBuffer += 2;
        pbuffer += 2;
        ValidInputBuffersize -= 8;
    }
    return NO_ERROR;
}

status_t AudioALSAPlaybackHandlerSpeakerProtection::DoSpeakerProctionUnPacking(int *pbuffer, int ValidInputBuffersize) {
    memcpy((void *)mSpeakerChannelProcessingBuffer, (void *)pbuffer, ValidInputBuffersize);
    int *ppostProcessingL = mSpeakerChannelProcessingBuffer;
    int *ppostProcessingR = (int *)((char *)mSpeakerChannelProcessingBuffer + (ValidInputBuffersize / 2));
    int *pTempprocessingBuffer = pbuffer;

    /*
    ALOGD("%s ValidInputBuffersize =%d ppostProcessingR = % pppostProcessingL = %p Dsmtempbuffer = %p",
        __FUNCTION__,ValidInputBuffersize,ppostProcessingR,ppostProcessingL,pbuffer);*/

    while (ValidInputBuffersize > 0) {
        *pTempprocessingBuffer = *ppostProcessingL;
        pTempprocessingBuffer++;
        ppostProcessingL++;
        *pTempprocessingBuffer = *ppostProcessingR;
        pTempprocessingBuffer++;
        ppostProcessingR++;
        ValidInputBuffersize -= 8;
    }
    return NO_ERROR;
}

// here using android define format
unsigned int  AudioALSAPlaybackHandlerSpeakerProtection::GetSampleSize(unsigned int Format) {
    unsigned returnsize = 2;
    if (Format == AUDIO_FORMAT_PCM_16_BIT) {
        returnsize = 2;
    } else if (Format == AUDIO_FORMAT_PCM_32_BIT || Format == AUDIO_FORMAT_PCM_8_24_BIT) {
        returnsize = 4;
    } else if (Format == AUDIO_FORMAT_PCM_8_BIT) {
        returnsize = 1;
    } else {
        ALOGD("%s Format == %d", __FUNCTION__, Format);
    }
    return returnsize;
}

// here using android define format
unsigned int  AudioALSAPlaybackHandlerSpeakerProtection::GetFrameSize(unsigned int channels, unsigned int Format) {
    unsigned returnsize = 2;
    if (Format == AUDIO_FORMAT_PCM_16_BIT) {
        returnsize = 2;
    } else if (Format == AUDIO_FORMAT_PCM_32_BIT || Format == AUDIO_FORMAT_PCM_8_24_BIT) {
        returnsize = 4;
    } else if (Format == AUDIO_FORMAT_PCM_8_BIT) {
        returnsize = 1;
    } else {
        ALOGD("%s Format = %d", __FUNCTION__, Format);
    }
    returnsize *= channels;
    return returnsize;;
}

status_t AudioALSAPlaybackHandlerSpeakerProtection::DoSpeakerProtionInterfaceProcessing(void *pInBuffer, uint32_t inBytes, void **ppOutBuffer, uint32_t *pOutBytes) {
    //ALOGD("+%s", __FUNCTION__);
    if (mBypassSpeakerProtection == false) {
        unsigned int outSize =  mDsmBufferSize;
        unsigned int IVSize =  inBytes / GetFrameSize(mStreamAttributeTarget.num_channels, mStreamAttributeTarget.audio_format);
        IVSize *= GetFrameSize(mStreamAttributeTarget.num_channels, AUDIO_FORMAT_PCM_16_BIT);
        int retcode = 0;

        if (dsm_handler != NULL) { // do processing
            IVSize = mAudioAlsaPcmIn->read(mDsmIvReadData, IVSize);
            uint32_t InbyteTemp = inBytes;
            /*
            ALOGD("+mlds_process_dl_buf mDsmpcmOut = %p mDsmIvReadData = %p outSize = %d inBytes = %d",
            mDsmpcmOut,mDsmIvReadData, outSize,inBytes);
            */
            retcode = mMlds_Interace_pointer->mlds_process_dl_buf(
                          pInBuffer,
                          &inBytes,
                          mDsmpcmOut,
                          &outSize,
                          mDsmIvReadData,
                          &IVSize,
                          &dsm_handler,
                          NULL);
            //ALOGD("-mlds_process_dl_buf mDsmpcmOut = %p mDsmIvReadData = %p outSize = %d retcode = %d", mDsmpcmOut, mDsmIvReadData, outSize,retcode);
            if (retcode == LIB_OK) {
                *ppOutBuffer = mDsmpcmOut;
                *pOutBytes = outSize;
            } else {
                //ALOGD("-mlds_process_dl_buf ppOutBuffer = %p  pOutBytes %d", ppOutBuffer,*pOutBytes);
                *ppOutBuffer = pInBuffer;
                *pOutBytes = InbyteTemp;
            }
        } else {
            *ppOutBuffer = pInBuffer;
            *pOutBytes = inBytes;
        }
    } else {
        *ppOutBuffer = pInBuffer;
        *pOutBytes = inBytes;
    }
    //ALOGD("-%s outSize = %d   *ppOutBuffer = %p", __FUNCTION__, outSize , *ppOutBuffer);
    ASSERT(*ppOutBuffer != NULL && *pOutBytes != 0);
    return NO_ERROR;
}


ssize_t AudioALSAPlaybackHandlerSpeakerProtection::write(const void *buffer, size_t bytes) {
    //ALOGD("%s(), buffer = %p, bytes = %d", __FUNCTION__, buffer, bytes);

    if (mPcm == NULL) {
        ALOGE("%s(), mPcm == NULL, return", __FUNCTION__);
        return bytes;
    }

    // const -> to non const
    void *pBuffer = const_cast<void *>(buffer);
    ASSERT(pBuffer != NULL);

#ifdef DEBUG_LATENCY
    clock_gettime(CLOCK_REALTIME, &mNewtime);
    latencyTime[0] = calc_time_diff(mNewtime, mOldtime);
    mOldtime = mNewtime;
#endif

    void *pBufferAfterDcRemoval = NULL;
    uint32_t bytesAfterDcRemoval = 0;
    // DC removal before DRC
    doDcRemoval(pBuffer, bytes, &pBufferAfterDcRemoval, &bytesAfterDcRemoval);

    doStereoToMonoConversionIfNeed(pBufferAfterDcRemoval, bytesAfterDcRemoval);

    // post processing (can handle both Q1P16 and Q1P31 by audio_format_t)
    void *pBufferAfterPostProcessing = NULL;
    uint32_t bytesAfterPostProcessing = 0;
    doPostProcessing(pBufferAfterDcRemoval, bytesAfterDcRemoval, &pBufferAfterPostProcessing, &bytesAfterPostProcessing);

    // SRC
    void *pBufferAfterBliSrc = NULL;
    uint32_t bytesAfterBliSrc = 0;
    doBliSrc(pBufferAfterPostProcessing, bytesAfterPostProcessing, &pBufferAfterBliSrc, &bytesAfterBliSrc);

    //
    void *pBufferPostProcessing = NULL;
    uint32_t bytesAfterpostprocessings = 0;
    //DoSpeakerProtionProcessing(pBufferAfterBliSrc, bytesAfterBliSrc, &pBufferPostProcessing, &bytesAfterpostprocessings);
    DoSpeakerProtionInterfaceProcessing(pBufferAfterBliSrc, bytesAfterBliSrc, &pBufferPostProcessing, &bytesAfterpostprocessings);
    //ALOGD("DoSpeakerProtionInterfaceProcessing bytesAfterBliSrc = %d bytesAfterpostprocessings = %d", bytesAfterBliSrc, bytesAfterpostprocessings);

    // pcm dump
    WritePcmDumpData(pBufferPostProcessing, bytesAfterpostprocessings);

    // bit conversion
    void *pBufferAfterBitConvertion = NULL;
    uint32_t bytesAfterBitConvertion = 0;
    doBitConversion(pBufferPostProcessing, bytesAfterpostprocessings, &pBufferAfterBitConvertion, &bytesAfterBitConvertion);

    // data pending
    void *pBufferAfterPending = NULL;
    uint32_t bytesAfterpending = 0;
    dodataPending(pBufferAfterBitConvertion, bytesAfterBitConvertion, &pBufferAfterPending, &bytesAfterpending);

#ifdef DEBUG_LATENCY
    clock_gettime(CLOCK_REALTIME, &mNewtime);
    latencyTime[1] = calc_time_diff(mNewtime, mOldtime);
    mOldtime = mNewtime;
#endif

    // write data to pcm driver
    int retval = pcm_write(mPcm, pBufferAfterPending, bytesAfterpending);

    updateHardwareBufferInfo(bytes, bytesAfterpending);

#ifdef DEBUG_LATENCY
    clock_gettime(CLOCK_REALTIME, &mNewtime);
    latencyTime[2] = calc_time_diff(mNewtime, mOldtime);
    mOldtime = mNewtime;
#endif

    if (retval != 0) {
        ALOGE("%s(), pcm_write() error, retval = %d", __FUNCTION__, retval);
    }

#ifdef DEBUG_LATENCY
    ALOGD("%s ::write (-) latency_in_us,%1.6lf,%1.6lf,%1.6lf", __FUNCTION__, latencyTime[0], latencyTime[1], latencyTime[2]);
#endif

    return bytes;
}


status_t AudioALSAPlaybackHandlerSpeakerProtection::setFilterMng(AudioMTKFilterManager *pFilterMng) {
    ALOGD("+%s() mAudioFilterManagerHandler [0x%x]", __FUNCTION__, pFilterMng);
    mAudioFilterManagerHandler = pFilterMng;
    ALOGD("-%s()", __FUNCTION__);
    return NO_ERROR;
}

status_t AudioALSAPlaybackHandlerSpeakerProtection::CopyIVbuffer(void *InputBuffert, short *Rbuffer, short *Lbuffer, unsigned int samplecount) {
    short *pTemp = (short *)InputBuffert;
    //ALOGD("samplecount = %d",samplecount);
    while (samplecount > 0) {
        *Rbuffer++ = *pTemp++;
        *Lbuffer++ = *pTemp++;
        samplecount--;
    }
    return NO_ERROR;
}

status_t AudioALSAPlaybackHandlerSpeakerProtection::EnableIVTask(pcm_config mPcm_config) {
    ASSERT(mAudioAlsaPcmIn != NULL);

    /*
    ALOGD("%s mPcm_config period_size = %d rate = %d period_count = %d", __FUNCTION__,
          mPcm_config.period_size, mPcm_config.rate, mPcm_config.period_count);
          */

    // settting for IV config setting
    mPcmIvConfig.channels = mPcm_config.channels;
    mPcmIvConfig.format = mPcm_config.format;
    mPcmIvConfig.rate = mPcm_config.rate;
    if (mPcmIvConfig.rate == 16000) {
        mPcmIvConfig.period_count = 8;
        mPcmIvConfig.period_size = 256;
    } else {
        mPcmIvConfig.period_count = 2;
        mPcmIvConfig.period_size = 1024;
    }
    mPcmIvConfig.start_threshold = 0;
    mPcmIvConfig.stop_threshold = 0;
    mPcmIvConfig.silence_threshold = 0;

    mAudioAlsaPcmIn->SetPcmConfig(mPcmIvConfig);
    mAudioAlsaPcmIn->SetThreadEnable();
    ALOGD("-%s period_size = %d rate = %d period_count = %d", __FUNCTION__,
          mPcmIvConfig.period_size, mPcmIvConfig.rate, mPcmIvConfig.period_count);
    return NO_ERROR;
}

status_t AudioALSAPlaybackHandlerSpeakerProtection::DisableIVTask(void) {
    if (mAudioAlsaPcmIn != NULL) {
        mAudioAlsaPcmIn->SetThreadDisable();
    }
    ALOGD("-%s", __func__);

    return NO_ERROR;
}

// using dl to do function open
status_t AudioALSAPlaybackHandlerSpeakerProtection::InitmldsInterface() {
    ALOGD("%s", __FUNCTION__);

    //ALOGD("dlopen %s", SPEAKER_LIB_PATH);
    mMlds_handle = dlopen(SPEAKER_LIB_PATH, RTLD_NOW);
    //ALOGD("dlopen %s", SPEAKER_CONFIGPARSER_LIB_PATH);
    mDsmConfigHandle = dlopen(SPEAKER_CONFIGPARSER_LIB_PATH, RTLD_NOW);
    //ALOGD("dlopen %s", DSM_LIB_PATH);
    mDsmInterfaceHandle = dlopen(DSM_LIB_PATH, RTLD_NOW);

    if (mMlds_handle == NULL || mDsmConfigHandle == NULL || mDsmInterfaceHandle == NULL) {
        ALOGD("%s err, mMlds_handle = %p, mDsmConfigHandle = %p, mDsmInterfaceHandle = %p\n", __FUNCTION__,
              mMlds_handle, mDsmConfigHandle, mDsmInterfaceHandle);
        return INVALID_OPERATION;
    }

    mMlds_Interace_pointer->mlds_query_working_buf_size = (lib_status_t (*)(const mlds_task_config_t *, uint32_t *))dlsym(mMlds_handle, "mlds_query_working_buf_size");
    mMlds_Interace_pointer->mlds_create_handler = (lib_status_t (*)(
                                                       const mlds_task_config_t *p_mlds_task_config,
                                                       const uint32_t param_buf_size,
                                                       void *p_param_buf,
                                                       const uint32_t working_buf_size,
                                                       void *p_working_buf,
                                                       void **pp_handler))dlsym(mMlds_handle, "mlds_create_handler");
    mMlds_Interace_pointer->mlds_process_dl_buf = (lib_status_t (*)(
                                                       void *p_dl_buf_in,
                                                       uint32_t *InSize,
                                                       void *p_dl_buf_out,
                                                       uint32_t *OutSize,
                                                       void *p_ref_buf,
                                                       uint32_t *RefBufSize,
                                                       void *p_handler,
                                                       void *arg))dlsym(mMlds_handle, "mlds_process_dl_buf");
    mMlds_Interace_pointer->mlds_destroy_handler = (lib_status_t (*)(void *p_handler))dlsym(mMlds_handle, "mlds_destroy_handler");

    DsmConfigure = (void (*)(int usecase, void *dsm_handler))dlsym(mDsmConfigHandle, "dsm_configure");
    if (DsmConfigure == NULL) {
        ALOGW("%s(), DsmConfigure dlsym failed, dlerror = %s", __func__);
    }
    DsmSetParams = (unsigned int (*)(void *ipModuleHandler, int iCommandNumber, void *ipParamsBuffer))dlsym(mDsmInterfaceHandle, "DSM_API_Set_Params");
    if (DsmSetParams == NULL) {
        ALOGW("%s(), DsmSetParams dlsym failed, dlerror = %s", __func__);
    }
    DsmGetParams = (unsigned int (*)(void *ipModuleHandler, int iCommandNum, void *opParams))dlsym(mDsmInterfaceHandle, "DSM_API_Get_Params");
    if (DsmGetParams == NULL) {
        ALOGW("%s(), DsmGetParams dlsym failed, dlerror = %s", __func__);
    }

    return NO_ERROR;
}

void AudioALSAPlaybackHandlerSpeakerProtection::OpenPCMIVDump(const char *class_name) {
    ALOGD("%s()", __FUNCTION__);
    char mDumpFileName[128];
    sprintf(mDumpFileName, "%s.%d.%s.pcm", streaminIVCPMIn, mDumpPlayIVFileNum, class_name);

    mPCMPlayIVDumpFile = AudioOpendumpPCMFile(mDumpFileName, streamin_propty);

    if (strlen(mDumpFileName) != 0) {
        ALOGD("%s DumpFileName = %s", __FUNCTION__, mDumpFileName);
        mDumpPlayIVFileNum++;
        mDumpPlayIVFileNum %= MAX_DUMP_NUM;
    }
}

void AudioALSAPlaybackHandlerSpeakerProtection::ClosePCMIVDump() {
    ALOGD("%s()", __FUNCTION__);
    if (mPCMPlayIVDumpFile) {
        AudioCloseDumpPCMFile(mPCMPlayIVDumpFile);
        mPCMPlayIVDumpFile = NULL;
        ALOGD("%s(), close it", __FUNCTION__);
    }
}


} // end of namespace android

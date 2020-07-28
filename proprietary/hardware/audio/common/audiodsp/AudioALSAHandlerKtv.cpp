//#include <inttypes.h>
#include "AudioType.h"
#include <AudioLock.h>

#include <AudioMessengerIPI.h>
#include <audio_task.h>
#include "AudioDspType.h"
#include <audio_dsp_service.h>
#include "AudioDspStreamManager.h"
#include <aurisys_scenario_dsp.h>
#include <aurisys_config.h>
#include <arsi_type.h>
#include <AudioALSAHandlerKtv.h>

#include "AudioALSADeviceConfigManager.h"
#include "AudioALSADriverUtility.h"
#include "AudioDef.h"
#ifdef LOG_TAG
#undef LOG_TAG
#endif
#define LOG_TAG "AudioALSAHandlerKtv"


namespace android {
uint32_t AudioALSAHandlerKtv::mDumpFileNumDsp = 0;

/*==============================================================================
 *                     Singleton Pattern
 *============================================================================*/

AudioALSAHandlerKtv *AudioALSAHandlerKtv::mHandlerKtv = NULL;
AudioALSAHandlerKtv *AudioALSAHandlerKtv::getInstance() {
    static AudioLock mGetInstanceLock;
    AL_AUTOLOCK(mGetInstanceLock);

    if (mHandlerKtv == NULL) {
        mHandlerKtv = new AudioALSAHandlerKtv();
    }
    ASSERT(mHandlerKtv != NULL);
    return mHandlerKtv;
}

AudioALSAHandlerKtv::AudioALSAHandlerKtv() :
    mAudioMessengerIPI(AudioMessengerIPI::getInstance()),
    mDspKtvPcm(NULL),
    mKtvUlPcm(NULL),
    mKtvDlPcm(NULL),
    mDspKtvIndex(0),
    mKtvUlindex(0),
    mKtvDlindex(0),
    mStreamCardIndex(0),
    mTaskKtvActive(0),
    mMixer(AudioALSADriverUtility::getInstance()->getMixer()) {
    ALOGD("%s(), %p", __FUNCTION__, this);

    memset((void *)&mKtvUlConfig, 0, sizeof(struct pcm_config));
    memset((void *)&mKtvDlConfig, 0, sizeof(struct pcm_config));
    memset((void *)&mDspKtvConfig, 0, sizeof(struct pcm_config));
    mAurisysLibManagerKtv = NULL;
    mAurisysDspConfigKtv = NULL;

    mDumpFileNameDsp;
    mPCMDumpFileDsp = NULL;
    for (int i = 0; i < DEBUG_PCMDUMP_NUM; i++) {
        pcmin_dump_array[i] = NULL;
    }
}

AudioALSAHandlerKtv::~AudioALSAHandlerKtv() {
    ALOGD("%s(), %p", __FUNCTION__, this);

    if (mKtvUlPcm != NULL) {
        pcm_close(mKtvUlPcm);
        mKtvUlPcm = NULL;
    }

    if (mKtvDlPcm != NULL) {
        pcm_close(mKtvDlPcm);
        mKtvDlPcm = NULL;
    }

    if (mDspKtvPcm != NULL) {
        pcm_close(mDspKtvPcm);
        mDspKtvPcm = NULL;
    }
}

void AudioALSAHandlerKtv::get_task_pcmdump_info(int task_id, int param, void **pcm_dump) {
    *pcm_dump = pcmin_dump_array[param];
    ALOGV("%s() %p %d %d\n", __FUNCTION__, *((FILE **)pcm_dump), task_id, param);
}

void AudioALSAHandlerKtv::set_task_pcmdump_info(int task_id, int param, void *pcm_dump) {
    pcmin_dump_array[param] = (FILE *)pcm_dump;
    ALOGD("%s() %p %d %d\n", __FUNCTION__, pcmin_dump_array[param], task_id, param);
}

void AudioALSAHandlerKtv::processDmaMsg(
    struct ipi_msg_t *msg,
    void *buf,
    uint32_t size) {
    FILE *pcm_dump = NULL;

    ALOGV("%s() msg_id=0x%x, task_scene=%d, param2=0x%x, size=%d\n",
          __FUNCTION__, msg->msg_id, msg->task_scene, msg->param2, size);

    switch (msg->msg_id) {
    case AUDIO_DSP_TASK_PCMDUMP_DATA:
        get_task_pcmdump_info(msg->task_scene, msg->param2, (void **)&pcm_dump);
        if (pcm_dump != NULL) {
            AudioDumpPCMData(buf, size, pcm_dump);
        }
        break;
    default:
        break;
    }
}

void AudioALSAHandlerKtv::processDmaMsgWrapper(
    struct ipi_msg_t *msg,
    void *buf,
    uint32_t size,
    void *arg) {

    AudioALSAHandlerKtv *AudioALSAHandlerKtv =
        static_cast<class AudioALSAHandlerKtv *>(arg);

    if (AudioALSAHandlerKtv != NULL) {
        AudioALSAHandlerKtv->processDmaMsg(msg, buf, size);
    }
}

void AudioALSAHandlerKtv::openPCMDumpKtv(void) {
#define MAX_TASKNAME_LEN (128)

    const char *audio_dump = DUMP_PATH;

    char mDumpFileName[128];
    char task_name[MAX_TASKNAME_LEN];
    char value[PROPERTY_VALUE_MAX];
    int i, ktv_dump_on = 0;

    struct ipi_msg_t ipi_msg;
    FILE *pcm_dump = NULL;

    property_get(streamindsp_propty, value, "0");
    ktv_dump_on = atoi(value);
    ALOGD("ktv_dump_on = %d", ktv_dump_on);
    if (ktv_dump_on) {
        // check task ktv dump
        strncpy(task_name, "TaskKTV", MAX_TASKNAME_LEN);

        for (i = 0; i < DEBUG_PCMDUMP_NUM; i++) {
            get_task_pcmdump_info(TASK_SCENE_KTV, i, (void **)&pcm_dump);
            if (pcm_dump == NULL) {
                sprintf(mDumpFileName, "%s%s.%d.%d.%d.%s_point%d.pcm",
                        audio_dump, LOG_TAG, mDumpFileNumDsp, getpid(), gettid(), task_name, i);
                mPCMDumpFileDsp = AudioOpendumpPCMFile(mDumpFileName, streamindsp_propty);
                if (mPCMDumpFileDsp != NULL) {
                    ALOGD("%s DumpFileName = %s", __FUNCTION__, mDumpFileName);
                }
                set_task_pcmdump_info(TASK_SCENE_KTV, i, (void *)mPCMDumpFileDsp);
            }
        }

        audio_send_ipi_msg(&ipi_msg,
                           TASK_SCENE_KTV, AUDIO_IPI_LAYER_TO_DSP,
                           AUDIO_IPI_MSG_ONLY, AUDIO_IPI_MSG_BYPASS_ACK,
                           AUDIO_DSP_TASK_PCMDUMP_ON, ktv_dump_on, 0,
                           NULL);

        mDumpFileNumDsp++;
        mDumpFileNumDsp %= MAX_DUMP_NUM;
    } else {
        audio_send_ipi_msg(&ipi_msg,
                           TASK_SCENE_KTV, AUDIO_IPI_LAYER_TO_DSP,
                           AUDIO_IPI_MSG_ONLY, AUDIO_IPI_MSG_BYPASS_ACK,
                           AUDIO_DSP_TASK_PCMDUMP_ON, ktv_dump_on, 0,
                           NULL);
    }
}

void AudioALSAHandlerKtv::closePCMDumpKtv(void) {
    FILE *pcm_dump = NULL;
    int i;

    // Task Ktv
    mAudioMessengerIPI->deregisterDmaCbk(TASK_SCENE_KTV);

    for (i = 0; i < DEBUG_PCMDUMP_NUM; i++) {
        get_task_pcmdump_info(TASK_SCENE_KTV, i, (void **)&pcm_dump);
        if (pcm_dump != NULL) {
            AudioCloseDumpPCMFile(pcm_dump);
            set_task_pcmdump_info(TASK_SCENE_KTV, i, NULL);
        }
    }
}

void AudioALSAHandlerKtv::startKtvTask(stream_attribute_t *attribute) {
    int retval = 0;
    struct ipi_msg_t ipi_msg;
    stream_attribute_t attributeKtv;

    ALOGD("%s+()", __FUNCTION__);
    if (mixer_ctl_set_value(mixer_get_ctl_by_name(mMixer, "adsp_ktv_sharemem_scenario"), 0, 1)) {
        ALOGW("%s(), enable sharemem fail", __FUNCTION__);
    }

    memset((void *)&mKtvUlConfig, 0, sizeof(struct pcm_config));
    memset((void *)&mKtvDlConfig, 0, sizeof(struct pcm_config));
    memset((void *)&mDspKtvConfig, 0, sizeof(struct pcm_config));

    stream_attribute_t *pAttribute = attribute;
    unsigned int multiplier = (pAttribute->sample_rate / 96000) + 1;
    memcpy(&attributeKtv, pAttribute, sizeof(stream_attribute_t));
    attributeKtv.num_channels = 2;

    mKtvUlConfig.rate = pAttribute->sample_rate;
    mKtvUlConfig.channels = attributeKtv.num_channels;
    mKtvUlConfig.format = PCM_FORMAT_S24_LE;
    mKtvUlConfig.period_count = 4;
    mKtvUlConfig.period_size = (1024 * multiplier) / 4;
    mKtvUlConfig.stop_threshold = ~(0U);
    mKtvUlConfig.silence_threshold = 0;

    mKtvDlConfig.rate = pAttribute->sample_rate;
    mKtvDlConfig.channels = attributeKtv.num_channels;
    mKtvDlConfig.format = PCM_FORMAT_S24_LE;
    mKtvDlConfig.period_count = 4;
    mKtvDlConfig.period_size = (1024 * multiplier) / 4;
    mKtvDlConfig.start_threshold = (mKtvDlConfig.period_count * mKtvDlConfig.period_size);
    mKtvDlConfig.stop_threshold = ~(0U);
    mKtvDlConfig.silence_threshold = 0;

    /* todo : can change real dl sample rate ??*/
    mDspKtvConfig.rate = pAttribute->sample_rate;
    mDspKtvConfig.channels = attributeKtv.num_channels;
    mDspKtvConfig.format = PCM_FORMAT_S24_LE;
    mDspKtvConfig.period_count = 4;
    mDspKtvConfig.period_size = (1024 * multiplier) / 4;
    mDspKtvConfig.start_threshold = (mDspKtvConfig.period_count * mDspKtvConfig.period_size);
    mDspKtvConfig.stop_threshold = ~(0U);
    mDspKtvConfig.silence_threshold = 0;

    mStreamCardIndex = AudioALSADeviceParser::getInstance()->GetCardIndexByString(keypcmCapture8);
    mDspKtvIndex = AudioALSADeviceParser::getInstance()->GetPcmIndexByString(keypcmPlaybackDspKtv); //TODO
    mKtvUlindex = AudioALSADeviceParser::getInstance()->GetPcmIndexByString(keypcmCapture8);
    mKtvDlindex = AudioALSADeviceParser::getInstance()->GetPcmIndexByString(keypcmPlayback8);
    if (mixer_ctl_set_value(mixer_get_ctl_by_name(mMixer, "dsp_ktv_runtime_en"), 0, 1)) {
        ALOGW("%s(), enable ktv runtime fail", __FUNCTION__);
    }

    AudioALSADeviceConfigManager::getInstance()->ApplyDeviceTurnonSequenceByName(AUDIO_CTL_MIC_TO_CAPTURE8);
    AudioALSADeviceConfigManager::getInstance()->ApplyDeviceTurnonSequenceByName(AUDIO_CTL_PLAYBACK8_TO_ADDA_DL);

    mAudioMessengerIPI->registerAdspFeature(KTV_FEATURE_ID);

    mAudioMessengerIPI->registerDmaCbk(
        TASK_SCENE_KTV,
        0x10000,
        0x48000,
        processDmaMsgWrapper,
        this);
    openPCMDumpKtv();

#ifdef MTK_AURISYS_FRAMEWORK_SUPPORT
    AudioDspStreamManager::getInstance()->CreateAurisysLibManager(
        &mAurisysLibManagerKtv,
        &mAurisysDspConfigKtv,
        TASK_SCENE_KTV,
        AURISYS_SCENARIO_DSP_KTV,
        ARSI_PROCESS_TYPE_DL_ONLY,
        AUDIO_MODE_NORMAL,
        &attributeKtv,
        &attributeKtv,
        NULL,
        NULL);
#endif

    ASSERT(mKtvUlPcm == NULL);
    mKtvUlPcm = pcm_open(mStreamCardIndex,
                              mKtvUlindex, PCM_IN | PCM_MONOTONIC, &mKtvUlConfig);

    ALOGD("%s(), mKtvUlConfig: channels = %d, rate = %d, period_size = %d, period_count = %d, format = %d",
          __FUNCTION__, mKtvUlConfig.channels, mKtvUlConfig.rate, mKtvUlConfig.period_size,
          mKtvUlConfig.period_count, mKtvUlConfig.format);

    if (mKtvUlPcm == NULL) {
        ALOGE("%s(), mKtvUlPcm == NULL!!", __FUNCTION__);
        ASSERT(mKtvUlPcm != NULL);
    } else if (pcm_is_ready(mKtvUlPcm) == false) {
        ALOGE("%s(), mKtvUlPcm pcm_is_ready(%p) == false due to %s, close pcm.", __FUNCTION__, mKtvUlPcm, pcm_get_error(mKtvUlPcm));
        pcm_close(mKtvUlPcm);
        mKtvUlPcm = NULL;
    } else if (pcm_prepare(mKtvUlPcm) != 0) {
        ALOGE("%s(), mKtvUlPcm pcm_prepare(%p) == false due to %s, close pcm.", __FUNCTION__, mKtvUlPcm, pcm_get_error(mKtvUlPcm));
        pcm_close(mKtvUlPcm);
        mKtvUlPcm = NULL;
    }
    if (pcm_start(mKtvUlPcm) != 0) {
        ALOGE("%s(), mKtvUlPcm pcm_start(%p) fail due to %s", __FUNCTION__, mKtvUlPcm, pcm_get_error(mKtvUlPcm));
    }

    ASSERT(mKtvDlPcm == NULL);
    mKtvDlPcm = pcm_open(mStreamCardIndex,
                              mKtvDlindex, PCM_OUT | PCM_MONOTONIC, &mKtvDlConfig);

    ALOGD("%s(), mKtvDlConfig: channels = %d, rate = %d, period_size = %d, period_count = %d, format = %d",
          __FUNCTION__, mKtvDlConfig.channels, mKtvDlConfig.rate, mKtvDlConfig.period_size,
          mKtvDlConfig.period_count, mKtvDlConfig.format);

    if (mKtvDlPcm == NULL) {
        ALOGE("%s(), mKtvDlPcm == NULL!!", __FUNCTION__);
        ASSERT(mKtvDlPcm != NULL);
    } else if (pcm_is_ready(mKtvDlPcm) == false) {
        ALOGE("%s(), mKtvDlPcm pcm_is_ready(%p) == false due to %s, close pcm.", __FUNCTION__, mKtvDlPcm, pcm_get_error(mKtvDlPcm));
        pcm_close(mKtvDlPcm);
        mKtvDlPcm = NULL;
    } else if (pcm_prepare(mKtvDlPcm) != 0) {
        ALOGE("%s(), mKtvDlPcm pcm_prepare(%p) == false due to %s, close pcm.", __FUNCTION__, mKtvDlPcm, pcm_get_error(mKtvDlPcm));
        pcm_close(mKtvDlPcm);
        mKtvDlPcm = NULL;
    }
    if (pcm_start(mKtvDlPcm) != 0) {
        ALOGE("%s(), mKtvDlPcm pcm_start(%p) fail due to %s", __FUNCTION__, mKtvDlPcm, pcm_get_error(mKtvDlPcm));
    }

    ASSERT(mDspKtvPcm == NULL);
    mDspKtvPcm = pcm_open(mStreamCardIndex,
                       mDspKtvIndex, PCM_OUT | PCM_MONOTONIC, &mDspKtvConfig);

    ALOGD("%s(), mDspKtvConfig: channels = %d, rate = %d, period_size = %d, period_count = %d, format = %d",
          __FUNCTION__, mDspKtvConfig.channels, mDspKtvConfig.rate, mDspKtvConfig.period_size,
          mDspKtvConfig.period_count, mDspKtvConfig.format);

    if (mDspKtvPcm == NULL) {
        ALOGE("%s(), mDspKtvPcm == NULL!!", __FUNCTION__);
        ASSERT(mDspKtvPcm != NULL);
    } else if (pcm_is_ready(mDspKtvPcm) == false) {
        ALOGE("%s(), mDspKtvPcm pcm_is_ready(%p) == false due to %s, close pcm.", __FUNCTION__, mDspKtvPcm, pcm_get_error(mDspKtvPcm));
        pcm_close(mDspKtvPcm);
        mDspKtvPcm = NULL;
    } else if (pcm_prepare(mDspKtvPcm) != 0) {
        ALOGE("%s(), mDspKtvPcm pcm_prepare(%p) == false due to %s, close pcm.", __FUNCTION__, mDspKtvPcm, pcm_get_error(mDspKtvPcm));
        pcm_close(mDspKtvPcm);
        mDspKtvPcm = NULL;
    }
    if (pcm_start(mDspKtvPcm) != 0) {
        ALOGE("%s(), mDspKtvPcm pcm_start(%p) fail due to %s", __FUNCTION__, mDspKtvPcm, pcm_get_error(mDspKtvPcm));
        AudioDspStreamManager::getInstance()->triggerDsp(TASK_SCENE_KTV, AUDIO_DSP_TASK_START);
    }
}

void AudioALSAHandlerKtv::stopKtvTask(void) {
    int retval;
    struct ipi_msg_t ipi_msg;
    ALOGD("%s+()", __FUNCTION__);

    AudioALSADeviceConfigManager::getInstance()->ApplyDeviceTurnoffSequenceByName(AUDIO_CTL_MIC_TO_CAPTURE8);
    AudioALSADeviceConfigManager::getInstance()->ApplyDeviceTurnoffSequenceByName(AUDIO_CTL_PLAYBACK8_TO_ADDA_DL);
    if (mDspKtvPcm != NULL) {
        pcm_stop(mDspKtvPcm);
        pcm_close(mDspKtvPcm);
        mDspKtvPcm = NULL;
    }

    if (mKtvUlPcm != NULL) {
        pcm_stop(mKtvUlPcm);
        pcm_close(mKtvUlPcm);
        mKtvUlPcm = NULL;
    }

    if (mKtvDlPcm != NULL) {
        pcm_stop(mKtvDlPcm);
        pcm_close(mKtvDlPcm);
        mKtvDlPcm = NULL;
    }

    if (mixer_ctl_set_value(mixer_get_ctl_by_name(mMixer, "dsp_ktv_runtime_en"), 0, 0)) {
        ALOGW("%s(), disable ktv runtime fail", __FUNCTION__);
    }
    closePCMDumpKtv();

    retval = mAudioMessengerIPI->sendIpiMsg(
                 &ipi_msg,
                 TASK_SCENE_KTV, AUDIO_IPI_LAYER_TO_DSP,
                 AUDIO_IPI_MSG_ONLY, AUDIO_IPI_MSG_NEED_ACK,
                 AUDIO_DSP_TASK_STOP, 0, 0, NULL);
    if (retval != 0) {
        ALOGE("%s(), fail!! retval = %d", __FUNCTION__, retval);
    }

#ifdef MTK_AURISYS_FRAMEWORK_SUPPORT
    AudioDspStreamManager::getInstance()->DestroyAurisysLibManager(&mAurisysLibManagerKtv, &mAurisysDspConfigKtv, TASK_SCENE_KTV);
#endif

    mAudioMessengerIPI->deregisterDmaCbk(TASK_SCENE_KTV);
    mAudioMessengerIPI->deregisterAdspFeature(KTV_FEATURE_ID);

    if (mixer_ctl_set_value(mixer_get_ctl_by_name(mMixer, "adsp_ktv_sharemem_scenario"), 0, 0)) {
        ALOGW("%s(), disable sharemem fail", __FUNCTION__);
    }
}

bool AudioALSAHandlerKtv::getTaskKtvStatus(void) {
    return mTaskKtvActive;
}

void AudioALSAHandlerKtv::enableKtvTask(bool on) {
    ALOGD("%s(), on = %d", __FUNCTION__, on);

    mTaskKtvActive = on;
    if (mixer_ctl_set_value(mixer_get_ctl_by_name(mMixer, "ktv_status"), 0, on)) {
        ALOGW("%s(), enable fail", __FUNCTION__);
    }
}

void AudioALSAHandlerKtv::recoveryKtvStatus() {
    struct mixer_ctl *ctl = NULL;

    ctl = mixer_get_ctl_by_name(mMixer, "ktv_status");
    if (ctl == NULL)
        mTaskKtvActive = 0;
    mTaskKtvActive = mixer_ctl_get_value(ctl, 0);
}

} // end of namespace android


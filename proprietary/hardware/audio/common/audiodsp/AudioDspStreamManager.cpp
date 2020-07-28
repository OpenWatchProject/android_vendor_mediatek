#include "AudioDspStreamManager.h"
#include <cutils/properties.h>
#include <cutils/bitops.h>
#include <tinyalsa/asoundlib.h>


#include "AudioALSAStreamOut.h"
#include "AudioALSAStreamIn.h"

#include "AudioALSAPlaybackHandlerBase.h"
#include "AudioALSACaptureHandlerBase.h"
#include "AudioALSAHardwareResourceManager.h"

#include "AudioALSADeviceParser.h"
#include "AudioALSADriverUtility.h"
#include "AudioALSADeviceConfigManager.h"

#include "AudioParamParser.h"
#include "AudioSmartPaController.h"
#include <AudioMessengerIPI.h>
#include "audio_task.h"
#include "AudioUtility.h"
#include "audio_a2dp_msg_id.h"
#include <audio_dsp_service.h>

#ifdef MTK_AURISYS_FRAMEWORK_SUPPORT
#include <arsi_type.h>
#include <aurisys_scenario_dsp.h>
#include <aurisys_config.h>
#include <aurisys_utility.h>
#include <aurisys_controller.h>
#include <aurisys_lib_manager.h>
#include <SpeechEnhancementController.h>
#endif

#include "AudioALSAHandlerKtv.h"

#ifdef LOG_TAG
#undef LOG_TAG
#endif
#define LOG_TAG "AudioDspStreamManager"

//#define DEBUG_VERBOSE
#define AUDPLAYBACK_DL_SAMPLES (1024)
#define AUDPLAYBACK_DL_TASK_PEROID_COUNT (4)

#define AUDPLAYBACK_UL_SAMPLES (1024)
#define AUDPLAYBACK_UL_TASK_PEROID_COUNT (4)
#define AUDIO_MULTIPLIER (96000)

// a2dp VUL4 UL irq 14.2ms
#define A2DPPLAYBACK_UL_SAMPLES (2*1024)
#define A2DPPLAYBACK_UL_TASK_PEROID_COUNT (3)

namespace android {

enum {
    DSP_STREAM_NOTSUPPORT = -3,
    DSP_STREAM_VALID = -2,
    DSP_STREAM_NOSTREAM = -1,
    DSP_STREAM_STOP = 0,
    DSP_STREAM_START,
    DSP_STREAM_NOCHANGE,
};

typedef struct {
    uint32_t codec_type; // codec types ex: SBC/AAC/LDAC/APTx
    uint32_t sample_rate;    // sample rate, ex: 44.1/48.88.2/96 KHz
    uint32_t encoded_audio_bitrate;  // encoder audio bitrates
    uint16_t max_latency;    // maximum latency
    uint16_t scms_t_enable;  // content protection enable
    uint16_t acl_hdl;    // connection handle
    uint16_t l2c_rcid;   // l2cap channel id
    uint16_t mtu;        // mtu size
    unsigned char bits_per_sample; // bits per sample, ex: 16/24/32
    unsigned char ch_mode;         // None:0, Left:1, Right:2
    unsigned char codec_info[32];  //codec specific information
} A2DP_CODEC_INFO_T;

/*==============================================================================
 *                     Singleton Pattern
 *============================================================================*/

AudioDspStreamManager *AudioDspStreamManager::mDspStreamManager = NULL;
AudioDspStreamManager *AudioDspStreamManager::getInstance() {
    static AudioLock mGetInstanceLock;
    AL_AUTOLOCK(mGetInstanceLock);

    if (mDspStreamManager == NULL) {
        mDspStreamManager = new AudioDspStreamManager();
    }
    ASSERT(mDspStreamManager != NULL);
    return mDspStreamManager;
}

/*==============================================================================
 *                     Constructor / Destructor / Init / Deinit
 *============================================================================*/

AudioDspStreamManager::AudioDspStreamManager():
    mAudioMessengerIPI(AudioMessengerIPI::getInstance()),
    mDspA2DPStreamState(0),
    mDspA2DPIndex(0),
    mDspDataProviderIndex(0),
    mDspA2DPPcm(NULL),
    mDspDataProviderPcm(NULL),
    mDspTaskA2DPActive(0),
    mPlaybackUlPcm(NULL),
    mPlaybackUlindex(0),
    mPlaybackDlPcm(NULL),
    mPlaybackDlindex(0),
    mPlaybackIVPcm(NULL),
    mPlaybackIVindex(0),
    mDspPcm(NULL),
    mDspIndex(0),
    mDspTaskPlaybackActive(false),
    multiplier(0),
    mDspStreamState(0),
    mStreamCardIndex(0),
    mMixer(AudioALSADriverUtility::getInstance()->getMixer()),
    mPCMDumpFileDSP(NULL),
    mDumpFileNumDSP(0) {
    ALOGD("%s()", __FUNCTION__);

    mPlaybackHandlerVector.clear();
    mCaptureHandlerVector.clear();

    memset((void *)&mPlaybackUlConfig, 0, sizeof(struct pcm_config));
    memset((void *)&mPlaybackIVConfig, 0, sizeof(struct pcm_config));
    memset((void *)&mPlaybackDlConfig, 0, sizeof(struct pcm_config));
    memset((void *)&mDspConfig, 0, sizeof(struct pcm_config));
    memset((void *)&mDspStreamAttribute, 0, sizeof(struct stream_attribute_t));

#ifdef MTK_AURISYS_FRAMEWORK_SUPPORT
    mAurisysLibManagerPlayback = NULL;
    mAurisysDspConfigPlayback = NULL;
#endif

    doRecoveryState();
}


AudioDspStreamManager::~AudioDspStreamManager() {
    ALOGD("%s()", __FUNCTION__);

    /* clean vector*/
    mPlaybackHandlerVector.clear();
    mCaptureHandlerVector.clear();

    /* clean pcm */
    if (mPlaybackUlPcm != NULL) {
        pcm_close(mPlaybackUlPcm);
        mPlaybackUlPcm = NULL;
    }

    if (mPlaybackDlPcm != NULL) {
        pcm_close(mPlaybackDlPcm);
        mPlaybackDlPcm = NULL;
    }

    if (mPlaybackIVPcm != NULL) {
        pcm_close(mPlaybackIVPcm);
        mPlaybackIVPcm = NULL;
    }

    if (mDspPcm != NULL) {
        pcm_close(mDspPcm);
        mDspPcm = NULL;
    }
}

int AudioDspStreamManager::setAfeDspShareMem(bool condition) {
    if (mixer_ctl_set_value(mixer_get_ctl_by_name(mMixer, "adsp_playback_sharemem_scenario"), 0, condition)) {
        ALOGW("%s(), enable fail", __FUNCTION__);
        return -1;
    }
    return 0;
}


int AudioDspStreamManager::setAfeInDspShareMem(bool condition) {
    if (mixer_ctl_set_value(mixer_get_ctl_by_name(mMixer, "adsp_capture_sharemem_scenario"), 0, condition)) {
        ALOGW("%s(), enable fail", __FUNCTION__);
        return -1;
    }
    if (mixer_ctl_set_value(mixer_get_ctl_by_name(mMixer, "adsp_ref_sharemem_scenario"), 0, condition)) {
        ALOGW("%s(), enable fail", __FUNCTION__);
        return -1;
    }
    return 0;
}


int AudioDspStreamManager::setAfeOutDspShareMem(unsigned int flag, bool condition) {
    if (isIsolatedDeepBuffer((const audio_output_flags_t)flag)) {
        if (mixer_ctl_set_value(mixer_get_ctl_by_name(mMixer, "adsp_deepbuffer_sharemem_scenario"), 0, condition)) {
            ALOGW("%s(), enable fail", __FUNCTION__);
            return -1;
        }
    } else if (flag & AUDIO_OUTPUT_FLAG_VOIP_RX) {
         if (mixer_ctl_set_value(mixer_get_ctl_by_name(mMixer, "adsp_voip_sharemem_scenario"), 0, condition)) {
            ALOGW("%s(), enable fail", __FUNCTION__);
            return -1;
        }
    } else {
        if (mixer_ctl_set_value(mixer_get_ctl_by_name(mMixer, "adsp_primary_sharemem_scenario"), 0, condition)) {
            ALOGW("%s(), enable fail", __FUNCTION__);
            return -1;
        }
    }
    return 0;
}

int AudioDspStreamManager::setA2dpDspShareMem(bool condition) {
    if (mixer_ctl_set_value(mixer_get_ctl_by_name(mMixer, "adsp_a2dp_sharemem_scenario"), 0, condition)) {
        ALOGW("%s(), enable fail", __FUNCTION__);
        return -1;
    }
    return 0;
}

int AudioDspStreamManager::getDspRuntimeEn(uint8_t task_scene) {
#define MAX_TASKNAME_LEN (128)
    int ret = 0;
    struct mixer_ctl *ctl = NULL;
    char runtime_en[MAX_TASKNAME_LEN];

    switch (task_scene) {
        case TASK_SCENE_PRIMARY:
            strncpy(runtime_en, "dsp_primary_runtime_en", MAX_TASKNAME_LEN);
            break;
        case TASK_SCENE_DEEPBUFFER:
            strncpy(runtime_en, "dsp_deepbuf_runtime_en", MAX_TASKNAME_LEN);
            break;
        case TASK_SCENE_VOIP:
            strncpy(runtime_en, "dsp_voipdl_runtime_en", MAX_TASKNAME_LEN);
            break;
        case TASK_SCENE_PLAYBACK_MP3:
            strncpy(runtime_en, "dsp_offload_runtime_en", MAX_TASKNAME_LEN);
            break;
        case TASK_SCENE_AUDPLAYBACK:
            strncpy(runtime_en, "dsp_playback_runtime_en", MAX_TASKNAME_LEN);
            break;
        case TASK_SCENE_CAPTURE_UL1:
            strncpy(runtime_en, "dsp_captureul1_runtime_en", MAX_TASKNAME_LEN);
            break;
        case TASK_SCENE_A2DP:
            strncpy(runtime_en, "dsp_a2dp_runtime_en", MAX_TASKNAME_LEN);
            break;
        case TASK_SCENE_DATAPROVIDER:
            strncpy(runtime_en, "dsp_dataprovider_runtime_en", MAX_TASKNAME_LEN);
            break;
        case TASK_SCENE_KTV:
            strncpy(runtime_en, "dsp_ktv_runtime_en", MAX_TASKNAME_LEN);
            break;
        default:
            strncpy(runtime_en, "dsp_primary_runtime_en", MAX_TASKNAME_LEN);
            break;
    }

    ctl = mixer_get_ctl_by_name(mMixer, runtime_en);
    if (ctl == NULL) {
        ALOGE("-%s Mixer of %s = NULL!!", __FUNCTION__, runtime_en);
        return 0;
    }
    ret = mixer_ctl_get_value(ctl, 0);
    return ret;
}

int AudioDspStreamManager::setDspRuntimeEn(uint8_t task_scene, bool condition) {
#define MAX_TASKNAME_LEN (128)
    struct mixer_ctl *ctl = NULL;
    char runtime_en[MAX_TASKNAME_LEN];

    ALOGD("%s(), task_scene = %d, condition = %d", __FUNCTION__, task_scene, condition);
    switch (task_scene) {
        case TASK_SCENE_PRIMARY:
            strncpy(runtime_en, "dsp_primary_runtime_en", MAX_TASKNAME_LEN);
            break;
        case TASK_SCENE_DEEPBUFFER:
            strncpy(runtime_en, "dsp_deepbuf_runtime_en", MAX_TASKNAME_LEN);
            break;
        case TASK_SCENE_VOIP:
            strncpy(runtime_en, "dsp_voipdl_runtime_en", MAX_TASKNAME_LEN);
            break;
        case TASK_SCENE_PLAYBACK_MP3:
            strncpy(runtime_en, "dsp_offload_runtime_en", MAX_TASKNAME_LEN);
            break;
        case TASK_SCENE_AUDPLAYBACK:
            strncpy(runtime_en, "dsp_playback_runtime_en", MAX_TASKNAME_LEN);
            break;
        case TASK_SCENE_CAPTURE_UL1:
            strncpy(runtime_en, "dsp_captureul1_runtime_en", MAX_TASKNAME_LEN);
            break;
        case TASK_SCENE_A2DP:
            strncpy(runtime_en, "dsp_a2dp_runtime_en", MAX_TASKNAME_LEN);
            break;
        case TASK_SCENE_DATAPROVIDER:
            strncpy(runtime_en, "dsp_dataprovider_runtime_en", MAX_TASKNAME_LEN);
            break;
        case TASK_SCENE_KTV:
            strncpy(runtime_en, "dsp_ktv_runtime_en", MAX_TASKNAME_LEN);
            break;
        default:
            strncpy(runtime_en, "dsp_primary_runtime_en", MAX_TASKNAME_LEN);
            break;
    }

    ctl = mixer_get_ctl_by_name(mMixer, runtime_en);
    if (ctl == NULL) {
        ALOGE("-%s Mixer of %s = NULL!!", __FUNCTION__, runtime_en);
        return -1;
    }

    if (mixer_ctl_set_value(ctl, 0, condition)) {
        ALOGW("%s(), mixer_ctl_set_value %s fail", __FUNCTION__, runtime_en);
        return -1;
    }
    return 0;
}

/* which data need to pass to dsp stream manager*/
bool AudioDspStreamManager::dataPasstoDsp(AudioALSAPlaybackHandlerBase *Base) {
    const stream_attribute_t *attribute = Base->getStreamAttributeTarget();
    int playbackType = Base->getPlaybackHandlerType();

    if (playbackType == PLAYBACK_HANDLER_DEEP_BUFFER) {
        return true;
    }
    if (playbackType == PLAYBACK_HANDLER_NORMAL) {
        return true;
    }
    if (playbackType == PLAYBACK_HANDLER_VOIP && (attribute->output_devices & AUDIO_DEVICE_OUT_SPEAKER)) {
        return true;
    }
    if (playbackType == PLAYBACK_HANDLER_OFFLOAD) {
        return true;
    }
    if (playbackType == PLAYBACK_HANDLER_FAST && (attribute->output_devices & AUDIO_DEVICE_OUT_SPEAKER)) {
        return true;
    }
    ALOGD("%s return false attribute->output_devices = 0x%x playbackType = %d",
          __FUNCTION__, attribute->output_devices, playbackType);
    return false;
}

int AudioDspStreamManager::checkPlaybackStatus() {
    size_t i = 0;
    int ret = 0;
    bool dspstartflag = false;

    for (i = 0; i < mPlaybackHandlerVector.size(); i++) {
        AudioALSAPlaybackHandlerBase *Base = mPlaybackHandlerVector.valueAt(i);
        dspstartflag |= dataPasstoDsp(Base);
        if (dspstartflag == true) {
            break;
        }
    }

    if (dspstartflag == true && mDspStreamState != DSP_STREAM_START) {
        ret = DSP_STREAM_START;
    } else if (dspstartflag == false && mDspStreamState != DSP_STREAM_STOP) {
        ret = DSP_STREAM_STOP;
    } else {
        ret = DSP_STREAM_NOCHANGE;
    }

    ALOGD("%s ret = %d mDspStreamState = %d dspstartflag = %d", __FUNCTION__, ret, mDspStreamState, dspstartflag);
    return ret;
}

bool AudioDspStreamManager::dataPasstoA2DPTask(AudioALSAPlaybackHandlerBase *Base) {
    const stream_attribute_t *attribute = Base->getStreamAttributeTarget();
    int playbackType = Base->getPlaybackHandlerType();

    if ((attribute->output_devices & AUDIO_DEVICE_OUT_ALL_A2DP) != 0) {
        ALOGD("%s return true attribute->output_devices = 0x%x playbackType = %d",
              __FUNCTION__, attribute->output_devices, playbackType);
        return true;
    }
    return false;
}

int AudioDspStreamManager::checkA2DPTaskStatus() {
    size_t i = 0;
    int ret = 0;
    bool dspstartflag = false;

    for (i = 0; i < mPlaybackHandlerVector.size(); i++) {
        AudioALSAPlaybackHandlerBase *Base = mPlaybackHandlerVector.valueAt(i);
        dspstartflag |= dataPasstoA2DPTask(Base);
        if (dspstartflag == true) {
            break;
        }
    }

    if (dspstartflag == true && mDspA2DPStreamState != DSP_STREAM_START) {
        ret = DSP_STREAM_START;
        ALOGD("%s ret = DSP_STREAM_START", __FUNCTION__);
    } else if (dspstartflag == false && mDspA2DPStreamState != DSP_STREAM_STOP) {
        ret = DSP_STREAM_STOP;
        ALOGD("%s ret = DSP_STREAM_STOP", __FUNCTION__);
    } else {
        ret = DSP_STREAM_NOCHANGE;
        ALOGV("%s ret = DSP_STREAM_NOCHANGE, mDspA2DPStreamState = %d dspstartflag = %d", __FUNCTION__, mDspA2DPStreamState, dspstartflag);
    }

    return ret;
}

int AudioDspStreamManager::startPlaybackTask(AudioALSAPlaybackHandlerBase *playbackHandler) {
    if (mPlaybackHandlerVector.size() <= 0) {
        return DSP_STREAM_NOSTREAM;
    }

    if (getDspPlaybackEnable() == false) {
        return DSP_STREAM_NOTSUPPORT;
    }

    /* get first handlerbase*/
    AudioALSAPlaybackHandlerBase *Base = mPlaybackHandlerVector.valueAt(0);
    memcpy(&mDspStreamAttribute, Base->getStreamAttributeTarget(), sizeof(stream_attribute_t));
    mDspStreamAttribute.mAudioOutputFlags = AUDIO_OUTPUT_FLAG_PLAYBACK;
    mDspStreamAttribute.audio_mode = AudioALSAStreamManager::getInstance()->getAudioMode();

    setAfeDspShareMem(true);
    setDspRuntimeEn(TASK_SCENE_AUDPLAYBACK, true);
    mAudioMessengerIPI->registerAdspFeature(AUDIO_PLAYBACK_FEATURE_ID);
    mAudioMessengerIPI->registerDmaCbk(
        TASK_SCENE_AUDPLAYBACK,
        0x48000,
        0x48000,
        AudioALSAPlaybackHandlerBase::processDmaMsgWrapper,
        playbackHandler);

    playbackHandler->OpenPCMDumpDSP(LOG_TAG, TASK_SCENE_AUDPLAYBACK);

    /* clean config*/
    memset((void *)&mPlaybackUlConfig, 0, sizeof(struct pcm_config));
    memset((void *)&mPlaybackIVConfig, 0, sizeof(struct pcm_config));
    memset((void *)&mPlaybackDlConfig, 0, sizeof(struct pcm_config));
    memset((void *)&mDspConfig, 0, sizeof(struct pcm_config));

    const stream_attribute_t *pAttributeul = Base->getStreamAttributeTarget();
    multiplier = (pAttributeul->sample_rate / AUDIO_MULTIPLIER) + 1;

    mPlaybackUlConfig.rate = pAttributeul->sample_rate;
    mPlaybackUlConfig.channels = pAttributeul->num_channels;
    mPlaybackUlConfig.format = PCM_FORMAT_S24_LE;
    mPlaybackUlConfig.period_count = AUDPLAYBACK_UL_TASK_PEROID_COUNT;
    mPlaybackUlConfig.period_size = (AUDPLAYBACK_UL_SAMPLES * multiplier) / AUDPLAYBACK_UL_TASK_PEROID_COUNT;
    mPlaybackUlConfig.stop_threshold = ~(0U);
    mPlaybackUlConfig.silence_threshold = 0;

    /* todo : can change real dl sample rate ??*/
    mPlaybackDlConfig.rate = pAttributeul->sample_rate;
    mPlaybackDlConfig.channels = pAttributeul->num_channels;
    mPlaybackDlConfig.format = PCM_FORMAT_S24_LE;
    mPlaybackDlConfig.period_count = AUDPLAYBACK_DL_TASK_PEROID_COUNT;
    mPlaybackDlConfig.period_size = (AUDPLAYBACK_DL_SAMPLES * multiplier) / AUDPLAYBACK_UL_TASK_PEROID_COUNT;
    mPlaybackDlConfig.start_threshold = (mPlaybackDlConfig.period_count * mPlaybackDlConfig.period_size);
    mPlaybackDlConfig.stop_threshold = ~(0U);
    mPlaybackDlConfig.silence_threshold = 0;

    mPlaybackIVConfig.rate = pAttributeul->sample_rate;
    mPlaybackIVConfig.channels = pAttributeul->num_channels;
    mPlaybackIVConfig.format = PCM_FORMAT_S24_LE;
    mPlaybackIVConfig.period_count = AUDPLAYBACK_UL_TASK_PEROID_COUNT;
    mPlaybackIVConfig.period_size = (AUDPLAYBACK_UL_SAMPLES * multiplier) / AUDPLAYBACK_UL_TASK_PEROID_COUNT;
    mPlaybackIVConfig.stop_threshold = ~(0U);
    mPlaybackIVConfig.silence_threshold = 0;

    /* todo : can change real dl sample rate ??*/
    mDspConfig.rate = pAttributeul->sample_rate;
    mDspConfig.channels = pAttributeul->num_channels;
    mDspConfig.format = PCM_FORMAT_S24_LE;
    mDspConfig.period_count = AUDPLAYBACK_DL_TASK_PEROID_COUNT;
    mDspConfig.period_size = (AUDPLAYBACK_DL_SAMPLES * multiplier) / AUDPLAYBACK_UL_TASK_PEROID_COUNT;
    mDspConfig.start_threshold = (mDspConfig.period_count * mDspConfig.period_size);
    mDspConfig.stop_threshold = ~(0U);
    mDspConfig.silence_threshold = 0;

    /* get card index and pcm index*/
    /* todo : pcm should move to platform */
    mStreamCardIndex = AudioALSADeviceParser::getInstance()->GetCardIndexByString(keypcmPlayback4);
    mDspIndex = AudioALSADeviceParser::getInstance()->GetPcmIndexByString(keypcmPlaybackDsp);

    mPlaybackUlindex = AudioALSADeviceParser::getInstance()->GetPcmIndexByString(keypcmCapture6);
    mPlaybackIVindex = AudioALSADeviceParser::getInstance()->GetPcmIndexByString(keypcmCapture4);
    mPlaybackDlindex = AudioALSADeviceParser::getInstance()->GetPcmIndexByString(keypcmPlayback4);

    /* here task audplayback source == target*/
#ifdef MTK_AURISYS_FRAMEWORK_SUPPORT
    uint32_t aurisys_scenario = AURISYS_SCENARIO_DSP_PLAYBACK;

    if (AudioSmartPaController::getInstance()->isSwDspSpkProtect(pAttributeul->output_devices) &&
        !AudioSmartPaController::getInstance()->isBypassSwDspSpkProtect()) {
        aurisys_scenario = AURISYS_SCENARIO_DSP_PLAYBACK_SMARTPA;
    }

    CreateAurisysLibManager(
        &mAurisysLibManagerPlayback,
        &mAurisysDspConfigPlayback,
        TASK_SCENE_AUDPLAYBACK,
        aurisys_scenario,
        ARSI_PROCESS_TYPE_DL_ONLY,
        mDspStreamAttribute.audio_mode,
        &mDspStreamAttribute,
        &mDspStreamAttribute,
        NULL,
        NULL);

#endif
    AudioALSAHardwareResourceManager *pHwResourceManager = AudioALSAHardwareResourceManager::getInstance();
    mApTurnOnSequence = pHwResourceManager->getOutputTurnOnSeq(pAttributeul->output_devices,
                                                               false, AUDIO_CTL_PLAYBACK4);
    if (AudioSmartPaController::getInstance()->isSmartPAUsed() &&
        (pAttributeul->output_devices & AUDIO_DEVICE_OUT_SPEAKER) &&
        popcount(pAttributeul->output_devices) > 1) {
        mApTurnOnSequence2 = pHwResourceManager->getOutputTurnOnSeq(pAttributeul->output_devices,
                                                                    true, AUDIO_CTL_PLAYBACK4);
    }
    if((pAttributeul->output_devices & AUDIO_DEVICE_OUT_SPEAKER) &&
       AudioSmartPaController::getInstance()->isSwDspSpkProtect(pAttributeul->output_devices) &&
       !AudioSmartPaController::getInstance()->isBypassSwDspSpkProtect()) {
        mApTurnOnSequenceIV = AudioSmartPaController::getInstance()->getI2sSequence(AUDIO_CTL_I2S_TO_CAPTURE4, true);
        AudioSmartPaController::getInstance()->setI2sInHD(true);
        pHwResourceManager->enableTurnOnSequence(mApTurnOnSequenceIV);
    }

    pHwResourceManager->enableTurnOnSequence(mApTurnOnSequence);
    pHwResourceManager->enableTurnOnSequence(mApTurnOnSequence2);

    ASSERT(mPlaybackDlPcm == NULL);
    mPlaybackDlPcm = pcm_open(mStreamCardIndex,
                              mPlaybackDlindex, PCM_OUT | PCM_MONOTONIC, &mPlaybackDlConfig);

    if (mPlaybackDlPcm == NULL) {
        ALOGE("%s(), mPlaybackDlPcm == NULL!!", __FUNCTION__);
        ASSERT(mPlaybackDlPcm != NULL);
        return -1;
    } else if (pcm_is_ready(mPlaybackDlPcm) == false) {
        ALOGE("%s(), pcm_is_ready(%p) == false due to %s, close pcm.", __FUNCTION__, mPlaybackDlPcm, pcm_get_error(mPlaybackDlPcm));
        pcm_close(mPlaybackDlPcm);
        mPlaybackDlPcm = NULL;
        return -1;
    } else if (pcm_prepare(mPlaybackDlPcm) != 0) {
        ALOGE("%s(), pcm_prepare(%p) == false due to %s, close pcm.", __FUNCTION__, mPlaybackDlPcm, pcm_get_error(mPlaybackDlPcm));
        pcm_close(mPlaybackDlPcm);
        mPlaybackDlPcm = NULL;
        return -1;
    }
    if (pcm_start(mPlaybackDlPcm) != 0) {
        ALOGE("%s(), pcm_start(%p) fail due to %s", __FUNCTION__, mPlaybackDlPcm, pcm_get_error(mPlaybackDlPcm));
    }

    if (AudioSmartPaController::getInstance()->isSwDspSpkProtect(pAttributeul->output_devices) &&
        (!AudioSmartPaController::getInstance()->isBypassSwDspSpkProtect())) {
        ASSERT(mPlaybackIVPcm == NULL);

        mPlaybackIVPcm = pcm_open(mStreamCardIndex,
                                  mPlaybackIVindex, PCM_IN | PCM_MONOTONIC, &mPlaybackIVConfig);

        if (mPlaybackIVPcm == NULL) {
            ALOGE("%s(), mPlaybackIVPcm == NULL!!", __FUNCTION__);
        } else if (pcm_is_ready(mPlaybackIVPcm) == false) {
            ALOGE("%s(), pcm_is_ready(%p) == false due to %s, close pcm.", __FUNCTION__, mPlaybackIVPcm, pcm_get_error(mPlaybackIVPcm));
            pcm_close(mPlaybackIVPcm);
            mPlaybackIVPcm = NULL;
        } else if (pcm_prepare(mPlaybackIVPcm) != 0) {
            ALOGE("%s(), pcm_prepare(%p) == false due to %s, close pcm.", __FUNCTION__, mPlaybackIVPcm, pcm_get_error(mPlaybackIVPcm));
            pcm_close(mPlaybackIVPcm);
            mPlaybackIVPcm = NULL;
        }
        if (pcm_start(mPlaybackIVPcm) != 0) {
            ALOGE("%s(), pcm_start(%p) fail due to %s", __FUNCTION__, mPlaybackIVPcm, pcm_get_error(mPlaybackIVPcm));
        }
        ASSERT(mPlaybackIVPcm != NULL);
     }

    ASSERT(mPlaybackUlPcm == NULL);
    mPlaybackUlPcm = pcm_open(mStreamCardIndex,
                              mPlaybackUlindex, PCM_IN | PCM_MONOTONIC, &mPlaybackUlConfig);

    if (mPlaybackUlPcm == NULL) {
        ALOGE("%s(), mPlaybackUlPcm == NULL!!", __FUNCTION__);
        ASSERT(mPlaybackUlPcm != NULL);
        return -1;
    } else if (pcm_is_ready(mPlaybackUlPcm) == false) {
        ALOGE("%s(), pcm_is_ready(%p) == false due to %s, close pcm.", __FUNCTION__, mPlaybackUlPcm, pcm_get_error(mPlaybackUlPcm));
        pcm_close(mPlaybackUlPcm);
        mPlaybackUlPcm = NULL;
        return -1;
    } else if (pcm_prepare(mPlaybackUlPcm) != 0) {
        ALOGE("%s(), pcm_prepare(%p) == false due to %s, close pcm.", __FUNCTION__, mPlaybackUlPcm, pcm_get_error(mPlaybackUlPcm));
        pcm_close(mPlaybackUlPcm);
        mPlaybackUlPcm = NULL;
        return -1;
    }
    if (pcm_start(mPlaybackUlPcm) != 0) {
        ALOGE("%s(), pcm_start(%p) fail due to %s", __FUNCTION__, mPlaybackUlPcm, pcm_get_error(mPlaybackUlPcm));
    }

    ASSERT(mDspPcm == NULL);
    mDspPcm = pcm_open(mStreamCardIndex,
                       mDspIndex, PCM_OUT | PCM_MONOTONIC, &mDspConfig);

    if (mDspPcm == NULL) {
        ALOGE("%s(), mDspPcm == NULL!!", __FUNCTION__);
        ASSERT(mDspPcm != NULL);
        return -1;
    } else if (pcm_is_ready(mDspPcm) == false) {
        ALOGE("%s(), pcm_is_ready(%p) == false due to %s, close pcm.", __FUNCTION__, mDspPcm, pcm_get_error(mDspPcm));
        pcm_close(mDspPcm);
        mDspPcm = NULL;
        return -1;
    } else if (pcm_prepare(mDspPcm) != 0) {
        ALOGE("%s(), pcm_prepare(%p) == false due to %s, close pcm.", __FUNCTION__, mDspPcm, pcm_get_error(mDspPcm));
        pcm_close(mDspPcm);
        mDspPcm = NULL;
        return -1;
    }

    if (pcm_start(mDspPcm) != 0) {
        ALOGE("%s(), pcm_start(%p) fail due to %s", __FUNCTION__, mDspPcm, pcm_get_error(mDspPcm));
        triggerDsp(TASK_SCENE_AUDPLAYBACK, AUDIO_DSP_TASK_START);
    }

    mDspTaskPlaybackActive = true;

    ALOGD("%s(), mDspTaskPlaybackActive = %d, mPlaybackUlindex = %d, mPlaybackDlindex = %d, mDspIndex = %d, "
          "mPlaybackDlConfig: channels = %d, rate = %d, period_size = %d, period_count = %d, format = %d, "
          "mPlaybackIVConfig: channels = %d, rate = %d, period_size = %d, period_count = %d, format = %d, "
          "mPlaybackUlConfig: channels = %d, rate = %d, period_size = %d, period_count = %d, format = %d, "
          "mDspConfig: channels = %d, rate = %d, period_size = %d, period_count = %d, format = %d",
          __FUNCTION__, mDspTaskPlaybackActive, mPlaybackUlindex, mPlaybackDlindex, mDspIndex,
          mPlaybackDlConfig.channels, mPlaybackDlConfig.rate, mPlaybackDlConfig.period_size, mPlaybackDlConfig.period_count, mPlaybackDlConfig.format,
          mPlaybackIVConfig.channels, mPlaybackIVConfig.rate, mPlaybackIVConfig.period_size, mPlaybackIVConfig.period_count, mPlaybackIVConfig.format,
          mPlaybackUlConfig.channels, mPlaybackUlConfig.rate, mPlaybackUlConfig.period_size, mPlaybackUlConfig.period_count, mPlaybackUlConfig.format,
          mDspConfig.channels, mDspConfig.rate, mDspConfig.period_size, mDspConfig.period_count, mDspConfig.format);

    return NO_ERROR;
}

int AudioDspStreamManager::stopPlaybackTask(AudioALSAPlaybackHandlerBase *playbackHandler) {
    if (mPlaybackHandlerVector.size() != 0) {
        ALOGE("-%s(), mPlaybackHandlerVector.size = %zd", __FUNCTION__, mPlaybackHandlerVector.size());
    }

    if (getDspPlaybackEnable() == false) {
        return DSP_STREAM_NOTSUPPORT;
    }

    if (mDspPcm != NULL) {
        if (pcm_stop(mDspPcm) != 0) {
            ALOGE("%s(), pcm_stop(%p) fail due to %s", __FUNCTION__, mDspPcm, pcm_get_error(mDspPcm));
            triggerDsp(TASK_SCENE_AUDPLAYBACK, AUDIO_DSP_TASK_STOP);
        }
        pcm_close(mDspPcm);
        mDspPcm = NULL;
    }

    if (mPlaybackUlPcm != NULL) {
        pcm_stop(mPlaybackUlPcm);
        pcm_close(mPlaybackUlPcm);
        mPlaybackUlPcm = NULL;
    }
    if (mPlaybackIVPcm != NULL) {
        pcm_stop(mPlaybackIVPcm);
        pcm_close(mPlaybackIVPcm);
        mPlaybackIVPcm = NULL;
    }
    if (mPlaybackDlPcm != NULL) {
        pcm_stop(mPlaybackDlPcm);
        pcm_close(mPlaybackDlPcm);
        mPlaybackDlPcm = NULL;
    }

    AudioALSAHardwareResourceManager *pHwResourceManager = AudioALSAHardwareResourceManager::getInstance();
    pHwResourceManager->disableTurnOnSequence(mApTurnOnSequence);
    pHwResourceManager->disableTurnOnSequence(mApTurnOnSequence2);
    if (!mApTurnOnSequenceIV.isEmpty()) {
        pHwResourceManager->disableTurnOnSequence(mApTurnOnSequenceIV);
        AudioSmartPaController::getInstance()->setI2sInHD(false);
    }

    setAfeDspShareMem(false);
    setDspRuntimeEn(TASK_SCENE_AUDPLAYBACK, false);
#ifdef MTK_AURISYS_FRAMEWORK_SUPPORT
    DestroyAurisysLibManager(&mAurisysLibManagerPlayback, &mAurisysDspConfigPlayback, TASK_SCENE_AUDPLAYBACK);
#endif
    mAudioMessengerIPI->deregisterDmaCbk(TASK_SCENE_AUDPLAYBACK);
    mAudioMessengerIPI->deregisterAdspFeature(AUDIO_PLAYBACK_FEATURE_ID);
    mDspTaskPlaybackActive = false;

    playbackHandler->ClosePCMDumpDSP(TASK_SCENE_AUDPLAYBACK);

    ALOGD("-%s(), mDspTaskPlaybackActive = %d, mPlaybackUlPcm = %p mPlaybackDlPcm = %p, mDspPcm = %p",
          __FUNCTION__, mDspTaskPlaybackActive, mPlaybackUlPcm, mPlaybackDlPcm, mDspPcm);

    return 0;
}

int AudioDspStreamManager::addPlaybackHandler(AudioALSAPlaybackHandlerBase *playbackHandler) {
    AL_AUTOLOCK(mLock);

    if (getDspPlaybackEnable() == false && getDspA2DPEnable() == false) {
        ALOGW("%s() and not support", __FUNCTION__);
        return -1;
    }

    mPlaybackHandlerVector.add((unsigned long long)playbackHandler, playbackHandler);
    dumpPlaybackHandler();

    if (checkPlaybackStatus() == DSP_STREAM_START) {
        startPlaybackTask(playbackHandler);
        mDspStreamState = DSP_STREAM_START;
        ALOGD("%s() mDspStreamState = DSP_STREAM_START", __FUNCTION__);
    }

    if (checkA2DPTaskStatus() == DSP_STREAM_START) {
        startA2DPTask(playbackHandler);
        mDspA2DPStreamState = DSP_STREAM_START;
        ALOGD("%s() mDspA2DPStreamState = DSP_STREAM_START", __FUNCTION__);
    }

    ALOGD("%s(), playbackHandler = %llu %p", __FUNCTION__, (unsigned long long)playbackHandler, playbackHandler);

    return 0;
}

int AudioDspStreamManager::removePlaybackHandler(AudioALSAPlaybackHandlerBase *playbackHandler) {
    AL_AUTOLOCK(mLock);
    ALOGD("+%s(), playbackHandler = %llu %p", __FUNCTION__, (unsigned long long)playbackHandler, playbackHandler);

    if (getDspPlaybackEnable() == false && getDspA2DPEnable() == false) {
        ALOGW("%s() and not support", __FUNCTION__);
        return -1;
    }

    mPlaybackHandlerVector.removeItem((unsigned long long)playbackHandler);

    if (checkPlaybackStatus() == DSP_STREAM_STOP) {
        stopPlaybackTask(playbackHandler);
        mDspStreamState = DSP_STREAM_STOP;
    }

    if (checkA2DPTaskStatus() == DSP_STREAM_STOP) {
        stopA2DPTask(playbackHandler);
        mDspA2DPStreamState = DSP_STREAM_STOP;
        ALOGD("%s() mDspA2DPStreamState = DSP_STREAM_STOP", __FUNCTION__);
    }

    ALOGV("-%s(), playbackHandler = %llu %p mDspStreamState = %d",
          __FUNCTION__, (unsigned long long)playbackHandler, playbackHandler, mDspStreamState);
    dumpPlaybackHandler();
    return 0;
}

int AudioDspStreamManager::addCaptureHandler(AudioALSACaptureHandlerBase *captureHandler) {
    AL_AUTOLOCK(mLock);
    ALOGD("%s()playbackHandler = %llu %p", __FUNCTION__, (unsigned long long)captureHandler, captureHandler);
    mCaptureHandlerVector.add((unsigned long long)captureHandler, captureHandler);
    dumpCaptureHandler();
    return 0;
}

int AudioDspStreamManager::removeCaptureHandler(AudioALSACaptureHandlerBase *captureHandler) {
    AL_AUTOLOCK(mLock);
    ALOGD("%s()playbackHandler = %llu %p", __FUNCTION__, (unsigned long long)captureHandler, captureHandler);
    mCaptureHandlerVector.removeItem((unsigned long long)captureHandler);
    dumpCaptureHandler();
    return 0;
}

int AudioDspStreamManager::dumpPlaybackHandler() {
#ifdef DEBUG_VERBOSE
    size_t i = 0;
    for (i = 0; i < mPlaybackHandlerVector.size(); i++) {
        AudioALSAPlaybackHandlerBase *Base = mPlaybackHandlerVector.valueAt(i);
        ALOGD("%s()playbackHandler = %llu", __FUNCTION__, (unsigned long long)Base);
    }
#endif
    return 0;
}

int AudioDspStreamManager::dumpCaptureHandler() {
#ifdef DEBUG_VERBOSE
    size_t i = 0;
    for (i = 0; i < mCaptureHandlerVector.size(); i++) {
        AudioALSACaptureHandlerBase *Base = mCaptureHandlerVector.valueAt(i);
        ALOGD("%s()captureHandler = %llu", __FUNCTION__, (unsigned long long)Base);
    }
#endif
    return 0;
}

unsigned int AudioDspStreamManager::getUlLatency(void) {
    if (getDspPlaybackEnable() == true)
        return AUDPLAYBACK_UL_SAMPLES * 1000 * multiplier / mPlaybackDlConfig.rate;
    else
        return 0;
}
unsigned int AudioDspStreamManager::getDlLatency(void) {
    unsigned int dllatency = 0;
    if (getDspPlaybackEnable() == true)
        dllatency = AUDPLAYBACK_DL_SAMPLES * 1000 * multiplier/ mPlaybackDlConfig.rate;
    else
        dllatency =  0;
    return dllatency;
}

unsigned int AudioDspStreamManager::getA2dpPcmLatency(void) {
    unsigned int a2dplatency = A2DPPLAYBACK_UL_SAMPLES * 1000 * multiplier/ mDspConfig.rate;
    return A2DPPLAYBACK_UL_SAMPLES * 1000 * multiplier/ mDspConfig.rate;
}

bool AudioDspStreamManager::getDspTaskPlaybackStatus(void) {
    return mDspTaskPlaybackActive;
}
bool AudioDspStreamManager::getDspTaskA2DPStatus(void) {
    return mDspTaskA2DPActive;
}

/* get audio dsp support stream */
int AudioDspStreamManager::getDspOutHandlerEnable(unsigned int flag) {
    int ret = 0;
    struct mixer_ctl *ctl = NULL;

    /* deep buffer */
    if ((flag & AUDIO_OUTPUT_FLAG_DEEP_BUFFER) && !(flag & AUDIO_OUTPUT_FLAG_PRIMARY)){
        ctl = mixer_get_ctl_by_name(mMixer, "dsp_deepbuf_default_en");
        if (ctl == NULL)
            return 0;
        ret = mixer_ctl_get_value(ctl, 0);
    } else if (flag & AUDIO_OUTPUT_FLAG_VOIP_RX) { /* void dl */
        ctl = mixer_get_ctl_by_name(mMixer, "dsp_voipdl_default_en");
        if (ctl == NULL) {
            return 0;
        }
        ret = mixer_ctl_get_value(ctl, 0);
    } else if (flag & AUDIO_OUTPUT_FLAG_PRIMARY) { /* void dl */
        ctl = mixer_get_ctl_by_name(mMixer, "dsp_primary_default_en");
        if (ctl == NULL) {
            return 0;
        }
        ret = mixer_ctl_get_value(ctl, 0);
    } else {
        ALOGD("%s flag = %x not support", __func__, flag);
        ret = 0;
    }
    ALOGD("%s flag = %x ret = %d", __func__, flag, ret);
    return ret;
}

/* todo : judge input flag */
int AudioDspStreamManager::getDspInHandlerEnable(unsigned int flag) {
    int ret = 0;
    struct mixer_ctl *ctl = NULL;

    ctl = mixer_get_ctl_by_name(mMixer, "dsp_captureul1_default_en");
    if (ctl == NULL)
        return 0;
    ret = mixer_ctl_get_value(ctl, 0);
    ALOGD("%s flag = %x ret = %d", __func__, flag, ret);
    return ret;
}

int AudioDspStreamManager::getDspPlaybackEnable() {
    int ret = 0;
    struct mixer_ctl *ctl = NULL;

    ctl = mixer_get_ctl_by_name(mMixer, "dsp_playback_default_en");
    if (ctl == NULL)
        return 0;
    ret = mixer_ctl_get_value(ctl, 0);
    return ret;
}

int AudioDspStreamManager::getDspA2DPEnable() {
    int ret = 0;
    struct mixer_ctl *ctl = NULL;

    ctl = mixer_get_ctl_by_name(mMixer, "dsp_a2dp_default_en");
    if (ctl == NULL) {
        ALOGE("-%s Mixer of dsp_a2dp_default_en = NULL!!", __FUNCTION__);
        return 0;
    }
    ret = mixer_ctl_get_value(ctl, 0);
    return ret;
}

int AudioDspStreamManager::triggerDsp(unsigned int task_scene, int data_type) {
    int retval;
    struct ipi_msg_t ipi_msg;

    if (data_type != AUDIO_DSP_TASK_START && data_type != AUDIO_DSP_TASK_STOP) {
        ALOGW("%s error task_scene = %u data_type = %u", __FUNCTION__, task_scene, data_type);
        return -1;
    }

    retval = mAudioMessengerIPI->sendIpiMsg(
                 &ipi_msg,
                 task_scene, AUDIO_IPI_LAYER_TO_DSP,
                 AUDIO_IPI_MSG_ONLY, AUDIO_IPI_MSG_BYPASS_ACK,
                 data_type, 0, 0, NULL);
    if (retval != 0) {
        ALOGE("%s(), fail!! retval = %d", __FUNCTION__, retval);
    }

    ALOGD("-%s task_scene = %d data_type = %d", __FUNCTION__, task_scene, data_type);
    return 0;
}

void AudioDspStreamManager::updateMode(audio_mode_t audioMode) {
    ALOGD("%s(), audioMode = %d", __FUNCTION__, audioMode);
    AL_AUTOLOCK(mLock);
    mDspStreamAttribute.audio_mode = audioMode;
#ifdef MTK_AURISYS_FRAMEWORK_SUPPORT
    if (mAurisysLibManagerPlayback != NULL && mAurisysDspConfigPlayback != NULL) {
        UpdateAurisysConfig(
            mAurisysLibManagerPlayback,
            mAurisysDspConfigPlayback,
            mDspStreamAttribute.audio_mode,
            &mDspStreamAttribute,
            &mDspStreamAttribute);
    }
#endif
}


#ifdef MTK_AURISYS_FRAMEWORK_SUPPORT
static void convertAttrFromHalToDsp(const struct stream_attribute_t *hal,
                                    struct stream_attribute_dsp *dsp) {
    if (!hal || !dsp) {
        return;
    }

    dsp->audio_format =  hal->audio_format;
    dsp->audio_offload_format = hal->audio_offload_format;
    dsp->audio_channel_mask = hal->audio_channel_mask;
    dsp->mAudioOutputFlags = hal->mAudioOutputFlags;
    dsp->mAudioInputFlags = hal->mAudioInputFlags;
    dsp->output_devices = hal->output_devices;
    dsp->input_device = hal->input_device;
    dsp->input_source = hal->input_source;
    dsp->num_channels = hal->num_channels;
    dsp->sample_rate = hal->sample_rate;
    dsp->acoustics_mask = hal->acoustics_mask;
    dsp->latency_ms = hal->latency;
}


void AudioDspStreamManager::CreateAurisysLibManager(
    struct aurisys_lib_manager_t **manager,
    struct aurisys_dsp_config_t **config,
    const uint8_t task_scene,
    const uint32_t aurisys_scenario,
    const uint8_t arsi_process_type,
    const uint32_t audio_mode,
    const struct stream_attribute_t *attribute_in,
    const struct stream_attribute_t *attribute_out,
    const struct stream_attribute_t *attribute_ref,
    const struct aurisys_gain_config_t *gain_config) {
    uint8_t *configAndParam = NULL;

    struct aurisys_lib_manager_t *newManager = NULL;
    struct aurisys_dsp_config_t *newConfig = NULL;

    struct aurisys_lib_manager_config_t *pManagerConfig = NULL;
    struct arsi_task_config_t *pTaskConfig = NULL;

    const char *custom_scene = NULL;
    struct ipi_msg_t msg;
    int retval = 0;

    struct data_buf_t paramList;

    paramList.data_size = 0;
    paramList.memory_size = 0;
    paramList.p_buffer = NULL;


    if (!manager) {
        WARNING("manager NULL!!");
        return;
    }
    if (!config) {
        WARNING("config NULL!!");
        return;
    }
    if ((*manager) != NULL || (*config) != NULL) {
        WARNING("already init!!");
        return;
    }

    mAudioMessengerIPI->registerAdspFeature(AURISYS_FEATURE_ID);
    AUDIO_ALLOC_STRUCT(struct aurisys_dsp_config_t, newConfig);
    newConfig->guard_head = AURISYS_GUARD_HEAD_VALUE;
    newConfig->guard_tail = AURISYS_GUARD_TAIL_VALUE;

    /* manager config */
    pManagerConfig = &newConfig->manager_config;

    pManagerConfig->aurisys_scenario = aurisys_scenario;
    pManagerConfig->arsi_process_type = arsi_process_type;
    pManagerConfig->audio_format = attribute_in->audio_format;
    pManagerConfig->sample_rate = attribute_in->sample_rate;
    if (aurisys_scenario == AURISYS_SCENARIO_DSP_VOIP) {
        //Voip can do 16k processinbg
        pManagerConfig->sample_rate = attribute_out->sample_rate;
    }
    pManagerConfig->frame_size_ms = 20;
    pManagerConfig->num_channels_ul = attribute_in->num_channels;
    pManagerConfig->num_channels_dl = attribute_in->num_channels;
    pManagerConfig->core_type = AURISYS_CORE_HIFI3;
    pManagerConfig->dsp_task_scene = task_scene;

    pTaskConfig = &pManagerConfig->task_config;

    /* task config */
    SetArsiTaskConfig(
        pManagerConfig,
        task_scene,
        aurisys_scenario,
        audio_mode,
        attribute_in,
        attribute_out);

    /* attribute */
    if (arsi_process_type == ARSI_PROCESS_TYPE_DL_ONLY) {
        convertAttrFromHalToDsp(attribute_in, &newConfig->attribute[DATA_BUF_DOWNLINK_IN]);
        convertAttrFromHalToDsp(attribute_out, &newConfig->attribute[DATA_BUF_DOWNLINK_OUT]);
    }
    if (AudioSmartPaController::getInstance()->isSwDspSpkProtect(pTaskConfig->output_device_info.devices) &&
        (!AudioSmartPaController::getInstance()->isBypassSwDspSpkProtect()) &&
        (aurisys_scenario == AURISYS_SCENARIO_DSP_PLAYBACK_SMARTPA)) {
        convertAttrFromHalToDsp(attribute_out, &newConfig->attribute[DATA_BUF_IV_BUFFER]);
        newConfig->iv_on = true;
        ALOGV("%s() iv is on. ", __FUNCTION__);
    } else {
        newConfig->iv_on = false;
    }
    if (arsi_process_type == ARSI_PROCESS_TYPE_UL_ONLY) {
        convertAttrFromHalToDsp(attribute_in, &newConfig->attribute[DATA_BUF_UPLINK_IN]);
        convertAttrFromHalToDsp(attribute_out, &newConfig->attribute[DATA_BUF_UPLINK_OUT]);
        if (attribute_ref != NULL) {
            convertAttrFromHalToDsp(attribute_ref, &newConfig->attribute[DATA_BUF_ECHO_REF]);
            newConfig->aec_on = true;
        } else {
            newConfig->aec_on = false;
        }
    }

    /* custom info */
    AppOps *appOps = appOpsGetInstance();
    if (appOps && appOps->appHandleIsFeatureOptionEnabled(appOps->appHandleGetInstance(), "VIR_SCENE_CUSTOMIZATION_SUPPORT")) {
        custom_scene = AudioALSAStreamManager::getInstance()->getCustScene().string();
        snprintf(pManagerConfig->custom_info, MAX_CUSTOM_INFO_LEN, "SetAudioCustomScene=%s", custom_scene);
        if (aurisys_scenario == AURISYS_SCENARIO_DSP_RECORD) {
            if (!strcmp(custom_scene, "rec_interview") || !strcmp(custom_scene, "rec_voice")) {
                pManagerConfig->num_channels_ul = 1;
                ALOGD("%s(), custom_scene = %s, ul ch = %d\n",
                      __FUNCTION__, custom_scene, pManagerConfig->num_channels_ul);
            }
        } else if (aurisys_scenario == AURISYS_SCENARIO_DSP_VOIP) {
            if (!strcmp(custom_scene, "app_gaming_16k")) {
                pManagerConfig->sample_rate = 16000;
                ALOGD("%s(), custom_scene = %s, force using 16k sample rate", __FUNCTION__, custom_scene);
            }
        }
    }

    /* create manager */
    newManager = create_aurisys_lib_manager(pManagerConfig);

    /* parsing param */
    aurisys_parsing_param_file(newManager);

    /* parsing param */
    paramList.data_size = 0;
    paramList.memory_size = 0x48000; /* TODO: refine it */
    AUDIO_ALLOC_BUFFER(paramList.p_buffer, paramList.memory_size);
    do {
        retval = aurisys_get_param_list(newManager, &paramList);
        if (retval == 0) {
            break;
        }
        ALOGE("%s(), paramList.memory_size %u not enough!!",
              __FUNCTION__, paramList.memory_size);
        AUD_WARNING("mem not enough!!");

        AUDIO_FREE_POINTER(paramList.p_buffer);
        paramList.data_size = 0;
        paramList.memory_size *= 2;
        AUDIO_ALLOC_BUFFER(paramList.p_buffer, paramList.memory_size);
    } while (1);


    /* set UL digital gain */
    if (gain_config != NULL) {
        newConfig->gain_config = *gain_config;
    }
    /* send config */
    configAndParam = (uint8_t *)AUDIO_MALLOC(sizeof(struct aurisys_dsp_config_t) + paramList.data_size);
    if (!configAndParam) {
        ALOGE("configAndParam is NULL!!");
        return;
    }
    memcpy(configAndParam, newConfig, sizeof(struct aurisys_dsp_config_t));
    memcpy(configAndParam + sizeof(struct aurisys_dsp_config_t), paramList.p_buffer, paramList.data_size);

    retval = mAudioMessengerIPI->sendIpiMsg(
                 &msg,
                 task_scene, AUDIO_IPI_LAYER_TO_DSP,
                 AUDIO_IPI_DMA, AUDIO_IPI_MSG_NEED_ACK,
                 AUDIO_DSP_TASK_AURISYS_INIT,
                 sizeof(struct aurisys_dsp_config_t) + paramList.data_size,
                 0,
                 configAndParam);
    if (retval != 0) {
        ALOGE("%s(), fail!! retval = %d", __FUNCTION__, retval);
    }
    AUDIO_FREE_POINTER(configAndParam);
    AUDIO_FREE_POINTER(paramList.p_buffer);

    /* update ptr */
    *manager = newManager;
    *config = newConfig;
}


void AudioDspStreamManager::UpdateAurisysConfig(
    struct aurisys_lib_manager_t *pAurisysLibManager,
    struct aurisys_dsp_config_t *pAurisysDspConfig,
    const uint32_t audio_mode,
    const struct stream_attribute_t *attribute_in,
    const struct stream_attribute_t *attribute_out) {
    struct aurisys_lib_manager_config_t *pManagerConfig = NULL;

    struct data_buf_t paramList;
    uint8_t *configAndParam = NULL;

    struct ipi_msg_t msg;
    int retval = 0;

    paramList.data_size = 0;
    paramList.memory_size = 0;
    paramList.p_buffer = NULL;


    if (pAurisysLibManager == NULL || pAurisysDspConfig == NULL) {
        ALOGE("%s(), not init!!", __FUNCTION__);
        return;
    }

    pManagerConfig = &pAurisysDspConfig->manager_config;

    /* task config */
    SetArsiTaskConfig(
        pManagerConfig,
        pManagerConfig->dsp_task_scene,
        pManagerConfig->aurisys_scenario,
        audio_mode,
        attribute_in,
        attribute_out);

    /* parsing param */
    aurisys_parsing_param_file(pAurisysLibManager);

    paramList.data_size = 0;
    paramList.memory_size = 0x48000; /* TODO: refine it */
    AUDIO_ALLOC_BUFFER(paramList.p_buffer, paramList.memory_size);
    do {
        retval = aurisys_get_param_list(pAurisysLibManager, &paramList);
        if (retval == 0) {
            break;
        }
        ALOGE("%s(), paramList.memory_size %u not enough!!",
              __FUNCTION__, paramList.memory_size);

        AUDIO_FREE_POINTER(paramList.p_buffer);
        paramList.data_size = 0;
        paramList.memory_size *= 2;
        AUDIO_ALLOC_BUFFER(paramList.p_buffer, paramList.memory_size);
    } while (1);

    configAndParam = (uint8_t *)AUDIO_MALLOC(paramList.data_size);
    if (!configAndParam) {
        ALOGE("configAndParam is NULL!!");
        return;
    }
    memcpy(configAndParam, paramList.p_buffer, paramList.data_size);

    retval = mAudioMessengerIPI->sendIpiMsg(
             &msg,
             pManagerConfig->dsp_task_scene, AUDIO_IPI_LAYER_TO_DSP,
             AUDIO_IPI_DMA, AUDIO_IPI_MSG_NEED_ACK,
             AUDIO_DSP_TASK_AURISYS_PARAM_LIST,
             paramList.data_size,
             0,
             configAndParam);

    if (retval != 0) {
        ALOGE("%s(), fail!! retval = %d", __FUNCTION__, retval);
    }
    AUDIO_FREE_POINTER(configAndParam);
    AUDIO_FREE_POINTER(paramList.p_buffer);
}


void AudioDspStreamManager::SetArsiTaskConfig(
    struct aurisys_lib_manager_config_t *pManagerConfig,
    const uint8_t task_scene,
    const uint32_t aurisys_scenario,
    const uint32_t audio_mode,
    const struct stream_attribute_t *attribute_in,
    const struct stream_attribute_t *attribute_out) {
    struct arsi_task_config_t *pTaskConfig = &pManagerConfig->task_config;

    pTaskConfig->input_device_info.devices = attribute_in->input_device;
    pTaskConfig->input_device_info.audio_format = attribute_in->audio_format;
    pTaskConfig->input_device_info.sample_rate = attribute_in->sample_rate;
    pTaskConfig->input_device_info.channel_mask = attribute_in->audio_channel_mask;
    pTaskConfig->input_device_info.num_channels = attribute_in->num_channels;
    pTaskConfig->input_device_info.hw_info_mask = 0;

    pTaskConfig->output_device_info.devices = attribute_in->output_devices;
    pTaskConfig->output_device_info.audio_format = attribute_in->audio_format;
    pTaskConfig->output_device_info.sample_rate = attribute_in->sample_rate;
    pTaskConfig->output_device_info.channel_mask = attribute_in->audio_channel_mask;
    pTaskConfig->output_device_info.num_channels = attribute_in->num_channels;
    if (AudioSmartPaController::getInstance()->isSmartPAUsed()) {
        pTaskConfig->output_device_info.hw_info_mask = OUTPUT_DEVICE_HW_INFO_SMARTPA_SPEAKER; /* SMARTPA */
    } else {
        pTaskConfig->output_device_info.hw_info_mask = 0;
    }

    pTaskConfig->task_scene = map_aurisys_scenario_to_task_scene(
                                  pManagerConfig->core_type,
                                  pManagerConfig->aurisys_scenario);

    /* TODO: decouple */
    if (pTaskConfig->task_scene != task_scene) {
        ALOGW("%s(), %d != %d", __FUNCTION__, task_scene, pTaskConfig->task_scene);
    }

    pTaskConfig->audio_mode = audio_mode;
    pTaskConfig->stream_type = attribute_in->stream_type;

    pTaskConfig->max_input_device_sample_rate  = 48000; /* TODO */
    pTaskConfig->max_output_device_sample_rate = 48000; /* TODO */
    pTaskConfig->max_input_device_num_channels  = 2; /* TODO */
    pTaskConfig->max_output_device_num_channels = 2; /* TODO */

    pTaskConfig->output_flags = attribute_in->mAudioOutputFlags;
    pTaskConfig->input_source = attribute_in->input_source; /* TODO: UL*/
    pTaskConfig->input_flags  = attribute_in->mAudioInputFlags; /* TODO: UL*/

    if (pTaskConfig->output_device_info.devices == AUDIO_DEVICE_OUT_EARPIECE &&
        SpeechEnhancementController::GetInstance()->GetHACOn()) {
        pTaskConfig->enhancement_feature_mask |= ENHANCEMENT_FEATURE_EARPIECE_HAC;
    }

    if ((pTaskConfig->input_device_info.devices & AUDIO_DEVICE_IN_ALL_SCO)
        && (pTaskConfig->output_device_info.devices & AUDIO_DEVICE_OUT_ALL_SCO)
        && SpeechEnhancementController::GetInstance()->GetBtHeadsetNrecOn()) {
        pTaskConfig->enhancement_feature_mask |= ENHANCEMENT_FEATURE_BT_NREC;
    }

    if ((aurisys_scenario == AURISYS_SCENARIO_DSP_VOIP)
        && (attribute_out->NativePreprocess_Info.PreProcessEffect_AECOn == true)
        && (pTaskConfig->input_device_info.devices != AUDIO_DEVICE_IN_ALL_SCO)) {
        pTaskConfig->enhancement_feature_mask |= ENHANCEMENT_FEATURE_EC;
    }

    if ((aurisys_scenario == AURISYS_SCENARIO_DSP_VOIP)
        && (attribute_out->NativePreprocess_Info.PreProcessEffect_NSOn == true)
        && (pTaskConfig->input_device_info.devices != AUDIO_DEVICE_IN_ALL_SCO)) {
        pTaskConfig->enhancement_feature_mask |= ENHANCEMENT_FEATURE_NS;
    }

    if ((aurisys_scenario == AURISYS_SCENARIO_DSP_VOIP)
        && (attribute_out->NativePreprocess_Info.PreProcessEffect_AGCOn == true)
        && (pTaskConfig->input_device_info.devices != AUDIO_DEVICE_IN_ALL_SCO)) {
        pTaskConfig->enhancement_feature_mask |= ENHANCEMENT_FEATURE_AGC;
    }

    dump_task_config(pTaskConfig);
}


int AudioDspStreamManager::doRecoveryState(){
    int value = 0;

    value = getDspRuntimeEn(TASK_SCENE_AUDPLAYBACK);
    if (value == true) {
         setAfeDspShareMem(false);
         mAudioMessengerIPI->deregisterAdspFeature(AUDIO_PLAYBACK_FEATURE_ID);
         setDspRuntimeEn(TASK_SCENE_AUDPLAYBACK, false);
         ALOGD("%s() AUDIO_PLAYBACK_FEATURE_ID", __FUNCTION__);
    }

    value = getDspRuntimeEn(TASK_SCENE_DEEPBUFFER);
    if (value == true) {
         setAfeOutDspShareMem(AUDIO_OUTPUT_FLAG_DEEP_BUFFER, false);
         mAudioMessengerIPI->deregisterAdspFeature(getDspFeatureID(AUDIO_OUTPUT_FLAG_DEEP_BUFFER));
         setDspRuntimeEn(TASK_SCENE_DEEPBUFFER, false);
         ALOGD("%s() AUDIO_OUTPUT_FLAG_DEEP_BUFFER", __FUNCTION__);
    }

    value = getDspRuntimeEn(TASK_SCENE_VOIP);
    if (value == true) {
         setAfeOutDspShareMem(AUDIO_OUTPUT_FLAG_VOIP_RX, false);
         mAudioMessengerIPI->deregisterAdspFeature(getDspFeatureID(AUDIO_OUTPUT_FLAG_VOIP_RX));
         setDspRuntimeEn(TASK_SCENE_VOIP, false);
         ALOGD("%s() AUDIO_OUTPUT_FLAG_VOIP_RX", __FUNCTION__);
    }

    value = getDspRuntimeEn(TASK_SCENE_PRIMARY);
    if (value == true) {
         setAfeOutDspShareMem(AUDIO_OUTPUT_FLAG_PRIMARY, false);
         mAudioMessengerIPI->deregisterAdspFeature(getDspFeatureID(AUDIO_OUTPUT_FLAG_PRIMARY));
         setDspRuntimeEn(TASK_SCENE_PRIMARY, false);
         ALOGD("%s() AUDIO_OUTPUT_FLAG_PRIMARY", __FUNCTION__);
    }

    value = getDspRuntimeEn(TASK_SCENE_PLAYBACK_MP3);
    if (value == true) {
         setAfeOutDspShareMem(AUDIO_OUTPUT_FLAG_COMPRESS_OFFLOAD, false);
         mAudioMessengerIPI->deregisterAdspFeature(getDspFeatureID(AUDIO_OUTPUT_FLAG_COMPRESS_OFFLOAD));
         setDspRuntimeEn(TASK_SCENE_PLAYBACK_MP3, false);
         ALOGD("%s() AUDIO_OUTPUT_FLAG_COMPRESS_OFFLOAD", __FUNCTION__);
    }

    value = getDspRuntimeEn(TASK_SCENE_CAPTURE_UL1);
    if (value == true) {
         setAfeInDspShareMem(false);
         mAudioMessengerIPI->deregisterAdspFeature(AUDIO_PLAYBACK_FEATURE_ID);
         setDspRuntimeEn(TASK_SCENE_CAPTURE_UL1, false);
         ALOGD("%s() CAPTURE_UL1_FEATURE_ID", __FUNCTION__);
    }

    value = getDspRuntimeEn(TASK_SCENE_A2DP);
    if (value == true) {
         setA2dpDspShareMem(false);
         mAudioMessengerIPI->deregisterAdspFeature(AUDIO_DATAPROVIDER_FEATURE_ID);
         mAudioMessengerIPI->deregisterAdspFeature(A2DP_PLAYBACK_FEATURE_ID);
         setDspRuntimeEn(TASK_SCENE_A2DP, false);
         ALOGD("%s() A2DP_PLAYBACK_FEATURE_ID", __FUNCTION__);
    }

    AudioALSAHandlerKtv::getInstance()->recoveryKtvStatus();
    value = getDspRuntimeEn(TASK_SCENE_KTV);
    if (value == true) {
         mAudioMessengerIPI->deregisterAdspFeature(KTV_FEATURE_ID);
         setDspRuntimeEn(TASK_SCENE_KTV, false);
         ALOGD("%s() KTV_FEATURE_ID", __FUNCTION__);
    }

    return 0;
}

void AudioDspStreamManager::DestroyAurisysLibManager(
    struct aurisys_lib_manager_t **manager,
    struct aurisys_dsp_config_t **config,
    const uint8_t task_scene) {
    struct ipi_msg_t ipi_msg;
    int retval = 0;

    if (!manager) {
        WARNING("manager NULL!!");
        return;
    }
    if (!config) {
        WARNING("config NULL!!");
        return;
    }
    if ((*manager) == NULL || (*config) == NULL) {
        return;
    }

    retval = mAudioMessengerIPI->sendIpiMsg(
                 &ipi_msg,
                 task_scene,
                 AUDIO_IPI_LAYER_TO_DSP,
                 AUDIO_IPI_MSG_ONLY,
                 AUDIO_IPI_MSG_NEED_ACK,
                 AUDIO_DSP_TASK_AURISYS_DEINIT,
                 0,
                 0,
                 NULL);
    if (retval != 0) {
        ALOGE("%s(), fail!! retval = %d", __FUNCTION__, retval);
    }

    destroy_aurisys_lib_manager(*manager);
    *manager = NULL;

    AUDIO_FREE_POINTER(*config);
    *config = NULL;
    mAudioMessengerIPI->deregisterAdspFeature(AURISYS_FEATURE_ID);
}
#endif /* end of MTK_AURISYS_FRAMEWORK_SUPPORT */

void AudioDspStreamManager::openPCMDumpA2DP(AudioALSAPlaybackHandlerBase *playbackHandler) {
#define MAX_TASKNAME_LEN (128)

    const char *audio_dump = "/data/vendor/audiohal/audio_dump";

    char mDumpFileName[128];
    char task_name[MAX_TASKNAME_LEN];
    char value[PROPERTY_VALUE_MAX];
    int i, a2dp_dump_on = 0;

    struct ipi_msg_t ipi_msg;
    FILE *pcm_dump = NULL;

    property_get(a2dpdsp_propty, value, "0");
    a2dp_dump_on = atoi(value);
    ALOGD("a2dp_dump_on = %d", a2dp_dump_on);
    if (a2dp_dump_on) {
        // check task a2dp dump
        strncpy(task_name, "TaskA2DP", MAX_TASKNAME_LEN);

        for (i = 0; i < DEBUG_PCMDUMP_NUM; i++) {
            playbackHandler->get_task_pcmdump_info(TASK_SCENE_A2DP, i, (void **)&pcm_dump);
            if (pcm_dump == NULL) {
                sprintf(mDumpFileName, "%s/%s.%d.%d.%d.%s_point%d.pcm",
                audio_dump, LOG_TAG, mDumpFileNumDSP, getpid(), gettid(), task_name, i);
                mPCMDumpFileDSP = AudioOpendumpPCMFile(mDumpFileName, a2dpdsp_propty);
                if (mPCMDumpFileDSP != NULL) {
                    ALOGD("%s DumpFileName = %s", __FUNCTION__, mDumpFileName);
                }
                playbackHandler->set_task_pcmdump_info(TASK_SCENE_A2DP, i, (void *)mPCMDumpFileDSP);
            }
        }

        mAudioMessengerIPI->registerDmaCbk(
            TASK_SCENE_A2DP,
            0,
            0x48000,
            AudioALSAPlaybackHandlerBase::processDmaMsgWrapper,
            playbackHandler);

        audio_send_ipi_msg(&ipi_msg,
                           TASK_SCENE_A2DP, AUDIO_IPI_LAYER_TO_DSP,
                           AUDIO_IPI_MSG_ONLY, AUDIO_IPI_MSG_BYPASS_ACK,
                           AUDIO_DSP_TASK_PCMDUMP_ON, a2dp_dump_on, 0,
                           NULL);

        // check task dataprovider dump
        strncpy(task_name, "TaskDataProvider", MAX_TASKNAME_LEN);

        for (i = 0; i < DEBUG_PCMDUMP_NUM; i++) {
            playbackHandler->get_task_pcmdump_info(TASK_SCENE_DATAPROVIDER, i, (void **)&pcm_dump);
            if (pcm_dump == NULL) {
                sprintf(mDumpFileName, "%s/%s.%d.%d.%d.%s_point%d.pcm",
                audio_dump, LOG_TAG, mDumpFileNumDSP, getpid(), gettid(), task_name, i);
                mPCMDumpFileDSP = AudioOpendumpPCMFile(mDumpFileName, a2dpdsp_propty);
                if (mPCMDumpFileDSP != NULL) {
                    ALOGD("%s DumpFileName = %s", __FUNCTION__, mDumpFileName);
                }
                    playbackHandler->set_task_pcmdump_info(TASK_SCENE_DATAPROVIDER, i, (void *)mPCMDumpFileDSP);
            }
        }

        mAudioMessengerIPI->registerDmaCbk(
            TASK_SCENE_DATAPROVIDER,
            0,
            0x48000,
            AudioALSAPlaybackHandlerBase::processDmaMsgWrapper,
            playbackHandler);

        audio_send_ipi_msg(&ipi_msg,
                           TASK_SCENE_DATAPROVIDER, AUDIO_IPI_LAYER_TO_DSP,
                           AUDIO_IPI_MSG_ONLY, AUDIO_IPI_MSG_BYPASS_ACK,
                           AUDIO_DSP_TASK_PCMDUMP_ON, a2dp_dump_on, 0,
                           NULL);

        mDumpFileNumDSP++;
        mDumpFileNumDSP %= MAX_DUMP_NUM;
    }
    else {
        audio_send_ipi_msg(&ipi_msg,
                           TASK_SCENE_A2DP, AUDIO_IPI_LAYER_TO_DSP,
                           AUDIO_IPI_MSG_ONLY, AUDIO_IPI_MSG_BYPASS_ACK,
                           AUDIO_DSP_TASK_PCMDUMP_ON, a2dp_dump_on, 0,
                           NULL);
        audio_send_ipi_msg(&ipi_msg,
                           TASK_SCENE_DATAPROVIDER, AUDIO_IPI_LAYER_TO_DSP,
                           AUDIO_IPI_MSG_ONLY, AUDIO_IPI_MSG_BYPASS_ACK,
                           AUDIO_DSP_TASK_PCMDUMP_ON, a2dp_dump_on, 0,
                           NULL);
    }
}

void AudioDspStreamManager::closePCMDumpA2DP(AudioALSAPlaybackHandlerBase *playbackHandler) {
    FILE *pcm_dump = NULL;
    int i;

    // Task A2DP
    mAudioMessengerIPI->deregisterDmaCbk(TASK_SCENE_A2DP);

    for (i = 0; i < DEBUG_PCMDUMP_NUM; i++) {
        playbackHandler->get_task_pcmdump_info(TASK_SCENE_A2DP, i, (void **)&pcm_dump);
        if (pcm_dump != NULL) {
            AudioCloseDumpPCMFile(pcm_dump);
            playbackHandler->set_task_pcmdump_info(TASK_SCENE_A2DP, i, NULL);
        }
    }
    // Task DataProvider
    mAudioMessengerIPI->deregisterDmaCbk(TASK_SCENE_DATAPROVIDER);

    for (i = 0; i < DEBUG_PCMDUMP_NUM; i++) {
        playbackHandler->get_task_pcmdump_info(TASK_SCENE_DATAPROVIDER, i, (void **)&pcm_dump);
        if (pcm_dump != NULL) {
            AudioCloseDumpPCMFile(pcm_dump);
            playbackHandler->set_task_pcmdump_info(TASK_SCENE_DATAPROVIDER, i, NULL);
        }
    }
}

int AudioDspStreamManager::startA2DPTask(AudioALSAPlaybackHandlerBase *playbackHandler) {
    if (mPlaybackHandlerVector.size() <= 0) {
        return DSP_STREAM_NOSTREAM;
    }

    /* get first handlerbase*/
    AudioALSAPlaybackHandlerBase *Base = mPlaybackHandlerVector.valueAt(0);
    const stream_attribute_t *attribute = Base->getStreamAttributeTarget();

    setA2dpDspShareMem(true);
    setDspRuntimeEn(TASK_SCENE_A2DP, true);
    adsp_register_feature(A2DP_PLAYBACK_FEATURE_ID);
    adsp_register_feature(AUDIO_DATAPROVIDER_FEATURE_ID);

    /* clean config*/
    memset((void *)&mDspConfig, 0, sizeof(struct pcm_config));

    const stream_attribute_t *pAttributeul = Base->getStreamAttributeTarget();
    multiplier = (pAttributeul->sample_rate / AUDIO_MULTIPLIER) + 1;

    /* todo : can change real dl sample rate ??*/
    mDspConfig.rate = pAttributeul->sample_rate;
    mDspConfig.channels = pAttributeul->num_channels;
    mDspConfig.format = PCM_FORMAT_S24_LE;
    mDspConfig.period_count = A2DPPLAYBACK_UL_TASK_PEROID_COUNT;
    mDspConfig.period_size = (A2DPPLAYBACK_UL_SAMPLES * multiplier) / A2DPPLAYBACK_UL_TASK_PEROID_COUNT;
    mDspConfig.start_threshold = (mDspConfig.period_count * mDspConfig.period_size);
    mDspConfig.stop_threshold = ~(0U);
    mDspConfig.silence_threshold = 0;

    /* get card index and pcm index*/
    /* todo : pcm should move to platform */
    mStreamCardIndex = AudioALSADeviceParser::getInstance()->GetCardIndexByString(keypcmPlaybackDspA2DP);
    mDspA2DPIndex = AudioALSADeviceParser::getInstance()->GetPcmIndexByString(keypcmPlaybackDspA2DP);
    mDspDataProviderIndex = AudioALSADeviceParser::getInstance()->GetPcmIndexByString(keypcmPlaybackDspDataProvider);

    ALOGD("%s(), mPlaybackUlindex: %d, mPlaybackDlindex = %d mDspA2DPIndex = %d",
          __FUNCTION__, mPlaybackUlindex, mPlaybackDlindex, mDspA2DPIndex);

    // send a2dp codec info to DSP
    struct ipi_msg_t ipi_msg;
    int retval = audio_send_ipi_msg(&ipi_msg,
                                TASK_SCENE_A2DP, AUDIO_IPI_LAYER_TO_DSP,
                                AUDIO_IPI_PAYLOAD, AUDIO_IPI_MSG_NEED_ACK,
                                AUDIO_DSP_TASK_A2DP_CODECINFO, sizeof(A2DP_CODEC_INFO_T), pAttributeul->sample_rate,
                                (char *)AudioALSAStreamManager::getInstance()->getBluetoothAudioCodecInfo());
    if (retval != NO_ERROR) {
        ALOGW("AUDIO_DSP_TASK_A2DP_CODECINFO fail");
    } else {
        ALOGD("param1 0x%x, param2 0x%x", ipi_msg.param1, ipi_msg.param2);
    }

    openPCMDumpA2DP(playbackHandler); // must be before pcm_start()

    startDSPTaskUL(); // TODO: taskplayback and taska2dp use common api for UL pcm control

    ASSERT(mDspDataProviderPcm == NULL);

    mDspDataProviderPcm = pcm_open(mStreamCardIndex,
                       mDspDataProviderIndex, PCM_OUT | PCM_MONOTONIC, &mDspConfig);

    ALOGD("%s(), mDspConfig: channels = %d, rate = %d, period_size = %d, period_count = %d, format = %d",
          __FUNCTION__, mDspConfig.channels, mDspConfig.rate, mDspConfig.period_size,
          mDspConfig.period_count, mDspConfig.format);

    if (mDspDataProviderPcm == NULL) {
        ALOGE("%s(), mDspDataProviderPcm == NULL!!", __FUNCTION__);
        ASSERT(mDspDataProviderPcm != NULL);
        return -1;
    } else if (pcm_is_ready(mDspDataProviderPcm) == false) {
        ALOGE("%s(), pcm_is_ready(%p) == false due to %s, close pcm.", __FUNCTION__, mDspDataProviderPcm, pcm_get_error(mDspDataProviderPcm));
        pcm_close(mDspDataProviderPcm);
        mDspDataProviderPcm = NULL;
        return -1;
    } else if (pcm_prepare(mDspDataProviderPcm) != 0) {
        ALOGE("%s(), pcm_prepare(%p) == false due to %s, close pcm.", __FUNCTION__, mDspDataProviderPcm, pcm_get_error(mDspDataProviderPcm));
        pcm_close(mDspDataProviderPcm);
        mDspDataProviderPcm = NULL;
        return -1;
    }

    if (pcm_start(mDspDataProviderPcm) != 0) {
        ALOGE("%s(), pcm_start(%p) fail due to %s", __FUNCTION__, mDspDataProviderPcm, pcm_get_error(mDspDataProviderPcm));
        triggerDsp(TASK_SCENE_DATAPROVIDER, AUDIO_DSP_TASK_START);
    }

    ALOGD("-%s(), mDspDataProviderPcm = %p", __FUNCTION__, mDspDataProviderPcm);

    ASSERT(mDspA2DPPcm == NULL);
    mDspA2DPPcm = pcm_open(mStreamCardIndex,
                       mDspA2DPIndex, PCM_OUT | PCM_MONOTONIC, &mDspConfig);

    ALOGD("%s(), mDspConfig: channels = %d, rate = %d, period_size = %d, period_count = %d, format = %d",
          __FUNCTION__, mDspConfig.channels, mDspConfig.rate, mDspConfig.period_size,
          mDspConfig.period_count, mDspConfig.format);

    if (mDspA2DPPcm == NULL) {
        ALOGE("%s(), mDspA2DPPcm == NULL!!", __FUNCTION__);
        ASSERT(mDspA2DPPcm != NULL);
        return -1;
    } else if (pcm_is_ready(mDspA2DPPcm) == false) {
        ALOGE("%s(), pcm_is_ready(%p) == false due to %s, close pcm.", __FUNCTION__, mDspA2DPPcm, pcm_get_error(mDspA2DPPcm));
        pcm_close(mDspA2DPPcm);
        mDspA2DPPcm = NULL;
        return -1;
    } else if (pcm_prepare(mDspA2DPPcm) != 0) {
        ALOGE("%s(), pcm_prepare(%p) == false due to %s, close pcm.", __FUNCTION__, mDspA2DPPcm, pcm_get_error(mDspA2DPPcm));
        pcm_close(mDspA2DPPcm);
        mDspA2DPPcm = NULL;
        return -1;
    }

    if (pcm_start(mDspA2DPPcm) != 0) {
        ALOGE("%s(), pcm_start(%p) fail due to %s", __FUNCTION__, mDspA2DPPcm, pcm_get_error(mDspA2DPPcm));
        triggerDsp(TASK_SCENE_A2DP, AUDIO_DSP_TASK_START);
    }

    ALOGD("-%s(), mDspA2DPPcm = %p", __FUNCTION__, mDspA2DPPcm);

    // notify BT profile to start stream
    if ((AudioALSAStreamManager::getInstance()->getBluetoothAudioHostIf() != NULL) &&
        (AudioALSAStreamManager::getInstance()->getBluetoothAudioSession() > 0)) {
        AudioALSAStreamManager::getInstance()->getBluetoothAudioHostIf()->startStream();
    }
    mDspTaskA2DPActive = true;
    return NO_ERROR;
}

int AudioDspStreamManager::stopA2DPTask(AudioALSAPlaybackHandlerBase *playbackHandler) {
    if (mPlaybackHandlerVector.size() != 0) {
        ALOGE("-%s(), mPlaybackHandlerVector.size = %zd", __FUNCTION__, mPlaybackHandlerVector.size());
    }

    if (mDspA2DPPcm != NULL) {
        if (pcm_stop(mDspA2DPPcm) != 0) {
            ALOGE("%s(), pcm_stop(%p) fail due to %s", __FUNCTION__, mDspA2DPPcm, pcm_get_error(mDspA2DPPcm));
            triggerDsp(TASK_SCENE_A2DP, AUDIO_DSP_TASK_STOP);
        }
        pcm_close(mDspA2DPPcm);
        mDspA2DPPcm = NULL;
    }

    if (mDspDataProviderPcm != NULL) {
        if (pcm_stop(mDspDataProviderPcm) != 0) {
            ALOGE("%s(), pcm_stop(%p) fail due to %s", __FUNCTION__, mDspDataProviderPcm, pcm_get_error(mDspDataProviderPcm));
            triggerDsp(TASK_SCENE_DATAPROVIDER, AUDIO_DSP_TASK_STOP);
        }
        pcm_close(mDspDataProviderPcm);
        mDspDataProviderPcm = NULL;
    }

    stopDSPTaskUL(); // TODO: taskplayback and taska2dp use common api for UL pcm control

    setA2dpDspShareMem(false);
    setDspRuntimeEn(TASK_SCENE_A2DP, false);
    adsp_deregister_feature(AUDIO_DATAPROVIDER_FEATURE_ID);
    adsp_deregister_feature(A2DP_PLAYBACK_FEATURE_ID);
    mDspTaskA2DPActive = false;

    // notify BT profile to stop stream
    if ((AudioALSAStreamManager::getInstance()->getBluetoothAudioHostIf() != NULL) &&
        (AudioALSAStreamManager::getInstance()->getBluetoothAudioSession() > 0)) {
        AudioALSAStreamManager::getInstance()->getBluetoothAudioHostIf()->suspendStream();
    }

    closePCMDumpA2DP(playbackHandler);

    ALOGD("-%s(), mPlaybackUlPcm = %p mPlaybackDlPcm = %p mDspA2DPPcm = %p",
          __FUNCTION__, mPlaybackUlPcm, mPlaybackDlPcm, mDspA2DPPcm);

    return 0;
}

void AudioDspStreamManager::startDSPTaskUL() {

    AudioALSAPlaybackHandlerBase *Base = mPlaybackHandlerVector.valueAt(0);
    const stream_attribute_t *pAttributeul = Base->getStreamAttributeTarget();
    multiplier = (pAttributeul->sample_rate / AUDIO_MULTIPLIER) + 1;

    memset((void *)&mPlaybackUlConfig, 0, sizeof(struct pcm_config));

    mPlaybackUlConfig.rate = pAttributeul->sample_rate;
    mPlaybackUlConfig.channels = pAttributeul->num_channels;
    mPlaybackUlConfig.format = PCM_FORMAT_S24_LE;
    mPlaybackUlConfig.period_count = A2DPPLAYBACK_UL_TASK_PEROID_COUNT;
    mPlaybackUlConfig.period_size = (A2DPPLAYBACK_UL_SAMPLES * multiplier) / A2DPPLAYBACK_UL_TASK_PEROID_COUNT;
    mPlaybackUlConfig.stop_threshold = ~(0U);
    mPlaybackUlConfig.silence_threshold = 0;

    ALOGD("%s(), mPlaybackUlConfig: channels = %d, rate = %d, period_size = %d, period_count = %d, format = %d",
          __FUNCTION__, mPlaybackUlConfig.channels, mPlaybackUlConfig.rate, mPlaybackUlConfig.period_size,
          mPlaybackUlConfig.period_count, mPlaybackUlConfig.format);

    mPlaybackUlindex = AudioALSADeviceParser::getInstance()->GetPcmIndexByString(keypcmCapture6);

    ASSERT(mPlaybackUlPcm == NULL);
    mPlaybackUlPcm = pcm_open(mStreamCardIndex,
                              mPlaybackUlindex, PCM_IN | PCM_MONOTONIC, &mPlaybackUlConfig);

    if (mPlaybackUlPcm == NULL) {
        ALOGE("%s(), mPlaybackUlPcm == NULL!!", __FUNCTION__);
        ASSERT(mPlaybackUlPcm != NULL);
        return;
    } else if (pcm_is_ready(mPlaybackUlPcm) == false) {
        ALOGE("%s(), pcm_is_ready(%p) == false due to %s, close pcm.", __FUNCTION__, mPlaybackUlPcm, pcm_get_error(mPlaybackUlPcm));
        pcm_close(mPlaybackUlPcm);
        mPlaybackUlPcm = NULL;
        ASSERT(mPlaybackUlPcm != NULL);
        return;
    } else if (pcm_prepare(mPlaybackUlPcm) != 0) {
        ALOGE("%s(), pcm_prepare(%p) == false due to %s, close pcm.", __FUNCTION__, mPlaybackUlPcm, pcm_get_error(mPlaybackUlPcm));
        pcm_close(mPlaybackUlPcm);
        mPlaybackUlPcm = NULL;
        ASSERT(mPlaybackUlPcm != NULL);
        return;
    }
    if (pcm_start(mPlaybackUlPcm) != 0) {
        ALOGE("%s(), pcm_start(%p) fail due to %s", __FUNCTION__, mPlaybackUlPcm, pcm_get_error(mPlaybackUlPcm));
    }
}

void AudioDspStreamManager::stopDSPTaskUL() {

    if (mPlaybackUlPcm != NULL) {
        pcm_stop(mPlaybackUlPcm);
        pcm_close(mPlaybackUlPcm);
        mPlaybackUlPcm = NULL;
    }
}

} // end of namespace android

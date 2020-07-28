#include "SpeechDriverOpenDSP.h"

#include <SpeechConfig.h>

#include <utils/threads.h>
#include <cutils/properties.h>

#include <dlfcn.h>


#include <cutils/properties.h>

#include <audio_task.h>

#include <arsi_type.h>
#include <arsi_call_type.h>
#include <aurisys_scenario_dsp.h>
#include <aurisys_config.h>
#include <aurisys_utility.h>
#include <aurisys_controller.h>
#include <aurisys_lib_manager.h>


#include <AudioLock.h>
#include "AudioUtility.h"

#include <SpeechUtility.h>

#include "AudioALSAHardwareResourceManager.h"
#include "AudioALSAStreamManager.h"

#include <SpeechDriverNormal.h>

#include <SpeechEnhancementController.h>
#include <AudioSmartPaController.h>

#include <AudioMessengerIPI.h>
#include "audio_speech_msg_id.h"
#include <tinyalsa/asoundlib.h> // for mixctrl

#ifdef MTK_AUDIODSP_SUPPORT
#include <audio_dsp_service.h>
#endif


#ifdef LOG_TAG
#undef LOG_TAG
#endif
#define LOG_TAG "SpeechDriverOpenDSP"


namespace android {

/**
 * lib related
 */

static const char *PROPERTY_KEY_PCM_DUMP_ON = "persist.vendor.audiohal.aurisys.pcm_dump_on";


#define DUMP_DSP_PCM_DATA_PATH "/data/vendor/audiohal/aurisys_dump/"

//#define AUDIO_SPEECH_ENABLE_3MIC


static FILE     *fp_pcm_dump_ul_in1;
static FILE     *fp_pcm_dump_ul_in2;
static FILE     *fp_pcm_dump_ul_in3;
static FILE     *fp_pcm_dump_ul_ref1;
static FILE     *fp_pcm_dump_ul_ref2;
static FILE     *fp_pcm_dump_ul_out;
static FILE     *fp_pcm_dump_dl_in;
static FILE     *fp_pcm_dump_dl_out;

enum pcm_dump_t {
    UL_IN1,
    UL_IN2,
    UL_IN3,
    UL_REF1,
    UL_REF2,
    UL_OUT,
    DL_IN,
    DL_OUT
};

static bool adsp_register_call = false;

static struct mixer *mMixer;




/*==============================================================================
 *                     utilities
 *============================================================================*/

inline uint8_t getInputChannelsByDevice(const audio_devices_t input_device) {
    uint8_t num_channels = 0;

    switch (input_device) {
    case AUDIO_DEVICE_IN_BUILTIN_MIC:
    case AUDIO_DEVICE_IN_BACK_MIC:
        num_channels = AudioALSAHardwareResourceManager::getInstance()->getNumPhoneMicSupport();
        break;
    case AUDIO_DEVICE_IN_WIRED_HEADSET:
    case AUDIO_DEVICE_IN_BLUETOOTH_SCO_HEADSET:
    case AUDIO_DEVICE_IN_USB_DEVICE:
        num_channels = 1;
        break;
    default: {
        ALOGE("Not support input_device 0x%x", input_device);
        WARNING("Need implement device");
        num_channels = 2;
    }
    }

#ifndef AUDIO_SPEECH_ENABLE_3MIC
    if (num_channels > 2) {
        ALOGW("num_channels %d, but only support dual mic", num_channels);
        num_channels = 2;
    }
#endif

    return num_channels;
}

inline uint32_t getScpSpeechStatus(void) {
    struct mixer_ctl *ctl = mixer_get_ctl_by_name(mMixer, "Speech_SCP_CALL_STATE");
    if (NULL == ctl) {
        ALOGW("unknown name Speech_SCP_CALL_STATE in mixer_ctl");
        return 0;
    } else {
        return mixer_ctl_get_value(ctl, 0);
    }
}

inline void setScpSpeechStatus(uint32_t state) {
    struct mixer_ctl *ctl = mixer_get_ctl_by_name(mMixer, "Speech_SCP_CALL_STATE");
    if (NULL == ctl) {
        ALOGW("unknown name Speech_SCP_CALL_STATE in mixer_ctl");
    } else {
        if (mixer_ctl_set_value(ctl, 0, state)) {
            ALOGE("%s() , Error: Speech_SCP_CALL_STATE %d", __FUNCTION__, state);
        }
    }
}

inline uint32_t getAuriNetworkRateByMdNetwork(uint32_t mdRat) {
    switch(mdRat) {
        case NETWORK_GSM:
            return PHONE_CALL_NET_GSM;
        case NETWORK_WCDMA_TDD:
            return PHONE_CALL_NET_CDMA;
        case NETWORK_WCDMA_FDD:
            return PHONE_CALL_NET_WCDMA;
        case NETWORK_VOLTE:
            return PHONE_CALL_NET_VOLTE;
        case NETWORK_C2K:
            return PHONE_CALL_NET_C2K;
        default:
            ALOGW("unknown mNetworkRate from MD mdRat: %u", mdRat);
            return PHONE_CALL_NET_GSM;
    }
}

/*==============================================================================
 *                     Singleton Pattern
 *============================================================================*/

SpeechDriverOpenDSP *SpeechDriverOpenDSP::mSpeechDriverOpenDSP = NULL;

SpeechDriverInterface *SpeechDriverOpenDSP::GetInstance(
    modem_index_t modem_index) {
    static AudioLock mGetInstanceLock;
    AL_AUTOLOCK(mGetInstanceLock);

    SpeechDriverOpenDSP *pSpeechDriver = NULL;
    ALOGD("%s(), modem_index = %d", __FUNCTION__, modem_index);

    switch (modem_index) {
    case MODEM_1:
        if (mSpeechDriverOpenDSP == NULL) {
            mSpeechDriverOpenDSP = new SpeechDriverOpenDSP(modem_index);
        }
        pSpeechDriver = mSpeechDriverOpenDSP;
        break;
    case MODEM_2:
    case MODEM_EXTERNAL:
        ALOGE("%s(), modem_index = %d not support", __FUNCTION__, modem_index);
        pSpeechDriver = NULL;
        break;
    default:
        ALOGE("%s: no such modem_index %d", __FUNCTION__, modem_index);
        break;
    }

    ASSERT(pSpeechDriver != NULL);
    return pSpeechDriver;
}


/*==============================================================================
 *                     Constructor / Destructor / Init / Deinit
 *============================================================================*/

SpeechDriverOpenDSP::SpeechDriverOpenDSP(modem_index_t modem_index) :
    mAudioMessengerIPI(AudioMessengerIPI::getInstance()),
    pSpeechDriverInternal(NULL) {

    mMixer = AudioALSADriverUtility::getInstance()->getMixer();
    ASSERT(mMixer != NULL);

    ALOGD("%s(), modem_index = %d", __FUNCTION__, modem_index);

    mModemIndex = modem_index;

    // control internal modem & FD216
    pSpeechDriverInternal = SpeechDriverNormal::GetInstance(modem_index);
    pSpeechDriverInternal->setNetworkChangeCallback(this, networkChangeCallback);

    mEnableDump = false;
    mSuperVolumeEnable = false;

    mAurisysDspConfig = NULL;
    mInputDevice = AUDIO_DEVICE_IN_BUILTIN_MIC;
    mOutputDevices = AUDIO_DEVICE_OUT_EARPIECE;

    mAurisysLibManager = NULL;

#ifndef SPH_SR32K /* non-SWB */
    mModemDspMaxSampleRate = 16000;
#elif defined(SPH_SR48K) /* SWB */
    mModemDspMaxSampleRate = 48000;
#else /* SWB */
    mModemDspMaxSampleRate = 32000;
#endif
    mModemPcmSampleRate = mModemDspMaxSampleRate;

    /* get scp function state */
    mScpSideSpeechStatus = getScpSpeechStatus();

    /* recovery scp state for mediaserver die */
    RecoverModemSideStatusToInitState();
}


SpeechDriverOpenDSP::~SpeechDriverOpenDSP() {
    ALOGD("%s()", __FUNCTION__);

    pSpeechDriverInternal = NULL;
}


/*==============================================================================
 *                     Aurisys
 *============================================================================*/


void SpeechDriverOpenDSP::SetArsiTaskConfig(struct arsi_task_config_t *pTaskConfig) {
    if (!pTaskConfig) {
        return;
    }

    pTaskConfig->input_device_info.devices = mInputDevice;
    pTaskConfig->input_device_info.audio_format = AUDIO_FORMAT_PCM_16_BIT;
    pTaskConfig->input_device_info.sample_rate = mModemDspMaxSampleRate;
    pTaskConfig->input_device_info.num_channels = getInputChannelsByDevice(mInputDevice);
    pTaskConfig->input_device_info.channel_mask = get_input_ch_mask(pTaskConfig->input_device_info.num_channels);
    pTaskConfig->input_device_info.hw_info_mask = 0;

    pTaskConfig->output_device_info.devices = mOutputDevices;
    pTaskConfig->output_device_info.audio_format = AUDIO_FORMAT_PCM_16_BIT;
    pTaskConfig->output_device_info.sample_rate = mModemDspMaxSampleRate;
    pTaskConfig->output_device_info.num_channels = 1;
    pTaskConfig->output_device_info.channel_mask = get_output_ch_mask(pTaskConfig->output_device_info.num_channels);
    pTaskConfig->output_device_info.hw_info_mask = 0;

    /* SMARTPA */
    if (mOutputDevices == AUDIO_DEVICE_OUT_SPEAKER &&
        AudioSmartPaController::getInstance()->isSmartPAUsed()) {
        pTaskConfig->output_device_info.hw_info_mask |= OUTPUT_DEVICE_HW_INFO_SMARTPA_SPEAKER;
    } else {
        pTaskConfig->output_device_info.hw_info_mask = 0;
    }


    pTaskConfig->task_scene = TASK_SCENE_PHONE_CALL;
    pTaskConfig->audio_mode = AUDIO_MODE_IN_CALL;
    pTaskConfig->network_info = getAuriNetworkRateByMdNetwork(mNetworkRate);

    pTaskConfig->max_input_device_sample_rate  = mModemDspMaxSampleRate;
    pTaskConfig->max_output_device_sample_rate = mModemDspMaxSampleRate;
    pTaskConfig->max_input_device_num_channels = AudioALSAHardwareResourceManager::getInstance()->getNumPhoneMicSupport();
#ifndef AUDIO_SPEECH_ENABLE_3MIC
    if (pTaskConfig->max_input_device_num_channels > 2) {
        pTaskConfig->max_input_device_num_channels = 2;
    }
#endif

    pTaskConfig->max_output_device_num_channels = 1;

    pTaskConfig->output_flags = 0;
    pTaskConfig->input_source = 0;
    pTaskConfig->input_flags  = 0;


    pTaskConfig->enhancement_feature_mask = (ENHANCEMENT_FEATURE_EC|ENHANCEMENT_FEATURE_NS|ENHANCEMENT_FEATURE_AGC);
    if (mOutputDevices == AUDIO_DEVICE_OUT_EARPIECE &&
        SpeechEnhancementController::GetInstance()->GetHACOn()) {
        pTaskConfig->enhancement_feature_mask |= ENHANCEMENT_FEATURE_EARPIECE_HAC;
    }

    if ((mInputDevice & AUDIO_DEVICE_IN_ALL_SCO)
        && (pTaskConfig->output_device_info.devices & AUDIO_DEVICE_OUT_ALL_SCO)
        && SpeechEnhancementController::GetInstance()->GetBtHeadsetNrecOn()) {
        pTaskConfig->enhancement_feature_mask |= ENHANCEMENT_FEATURE_BT_NREC;
    }

    dump_task_config(pTaskConfig);
}


void SpeechDriverOpenDSP::SetArsiAttribute() {
    struct stream_attribute_dsp *attribute = NULL;

    if (!mAurisysDspConfig) {
        return;
    }

    /* UL in attribute */
    attribute = &mAurisysDspConfig->attribute[DATA_BUF_UPLINK_IN];
    attribute->num_channels = getInputChannelsByDevice(mInputDevice);
    attribute->sample_rate  = mModemDspMaxSampleRate;
    attribute->audio_format = AUDIO_FORMAT_PCM_16_BIT;

    /* UL out attribute */
    attribute = &mAurisysDspConfig->attribute[DATA_BUF_UPLINK_OUT];
    attribute->num_channels = 1;
    attribute->sample_rate  = mModemDspMaxSampleRate;
    attribute->audio_format = AUDIO_FORMAT_PCM_16_BIT;

    /* AEC attribute */
    attribute = &mAurisysDspConfig->attribute[DATA_BUF_ECHO_REF];
    if (AudioSmartPaController::getInstance()->isDualSmartPA()) {
        attribute->num_channels = 2;
    } else {
        attribute->num_channels = 1;
    }
    attribute->sample_rate  = mModemDspMaxSampleRate;
    attribute->audio_format = AUDIO_FORMAT_PCM_16_BIT;


    /* DL in attribute */
    attribute = &mAurisysDspConfig->attribute[DATA_BUF_DOWNLINK_IN];
    attribute->num_channels = 1;
    attribute->sample_rate  = mModemDspMaxSampleRate;
    attribute->audio_format = AUDIO_FORMAT_PCM_16_BIT;

    /* DL out attribute */
    attribute = &mAurisysDspConfig->attribute[DATA_BUF_DOWNLINK_OUT];
    attribute->num_channels = 1;
    attribute->sample_rate  = mModemDspMaxSampleRate;
    attribute->audio_format = AUDIO_FORMAT_PCM_16_BIT;
}


void SpeechDriverOpenDSP::CreateAurisysLibManager() {
    struct aurisys_lib_manager_config_t *pManagerConfig = NULL;

    struct data_buf_t paramList;
    uint8_t *configAndParam = NULL;

    struct ipi_msg_t msg;
    int retval = 0;

    paramList.data_size = 0;
    paramList.memory_size = 0;
    paramList.p_buffer = NULL;

    ALOGD("%s(+)", __FUNCTION__);


    if (mAurisysLibManager != NULL || mAurisysDspConfig != NULL) {
        ALOGE("%p %p already init!!", mAurisysLibManager, mAurisysDspConfig);
        WARNING("already init!!");
        return;
    }


    AUDIO_ALLOC_STRUCT(struct aurisys_dsp_config_t, mAurisysDspConfig);
    mAurisysDspConfig->guard_head = AURISYS_GUARD_HEAD_VALUE;
    mAurisysDspConfig->guard_tail = AURISYS_GUARD_TAIL_VALUE;

    /* manager config */
    pManagerConfig = &mAurisysDspConfig->manager_config;

    pManagerConfig->aurisys_scenario = AURISYS_SCENARIO_DSP_PHONE_CALL;
    pManagerConfig->core_type = AURISYS_CORE_HIFI3;
    pManagerConfig->arsi_process_type = ARSI_PROCESS_TYPE_UL_AND_DL;
    pManagerConfig->frame_size_ms = 20;
    pManagerConfig->num_channels_ul = getInputChannelsByDevice(mInputDevice);
    pManagerConfig->num_channels_dl = 1;
    pManagerConfig->audio_format = AUDIO_FORMAT_PCM_16_BIT;
    pManagerConfig->sample_rate = mModemDspMaxSampleRate;
    pManagerConfig->dsp_task_scene = TASK_SCENE_PHONE_CALL;

    /* custom info */
    if(mSuperVolumeEnable){
        snprintf(pManagerConfig->custom_info, MAX_CUSTOM_INFO_LEN, "vol_level=extra");
    } else {
        snprintf(pManagerConfig->custom_info, MAX_CUSTOM_INFO_LEN, "vol_level=%d", mVolumeIndex);
    }

    ALOGD("manager config: aurisys_scenario %u, core_type %d, " \
          "arsi_process_type %d, frame_size_ms %d, " \
          "num_channels_ul %d, num_channels_dl %d, " \
          "audio_format %u, sample_rate %u, " \
          "custom_info=\"%s\"", \
          pManagerConfig->aurisys_scenario,
          pManagerConfig->core_type,
          pManagerConfig->arsi_process_type,
          pManagerConfig->frame_size_ms,
          pManagerConfig->num_channels_ul,
          pManagerConfig->num_channels_dl,
          pManagerConfig->audio_format,
          pManagerConfig->sample_rate,
          pManagerConfig->custom_info);

    /* task config */
    SetArsiTaskConfig(&pManagerConfig->task_config);


    /* attribute */
    SetArsiAttribute();

    /* gain */
    mAurisysDspConfig->gain_config.ul_digital_gain = 0;
    mAurisysDspConfig->gain_config.ul_analog_gain  = 0;
    mAurisysDspConfig->gain_config.dl_digital_gain = 0;
    mAurisysDspConfig->gain_config.dl_analog_gain  = 0;

    /* func */
    mAurisysDspConfig->voip_on = false;
    mAurisysDspConfig->aec_on = true;



    /* create manager */
    mAurisysLibManager = create_aurisys_lib_manager(pManagerConfig);

    /* parsing param */
    aurisys_parsing_param_file(mAurisysLibManager);

    paramList.data_size = 0;
    paramList.memory_size = 32768; /* TODO: refine it */
    AUDIO_ALLOC_BUFFER(paramList.p_buffer, paramList.memory_size);
    do {
        retval = aurisys_get_param_list(mAurisysLibManager, &paramList);
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


    /* send config */
    configAndParam = (uint8_t *)AUDIO_MALLOC(sizeof(struct aurisys_dsp_config_t) + paramList.data_size);
    memcpy(configAndParam, mAurisysDspConfig, sizeof(struct aurisys_dsp_config_t));
    memcpy(configAndParam + sizeof(struct aurisys_dsp_config_t), paramList.p_buffer, paramList.data_size);

    retval = mAudioMessengerIPI->sendIpiMsg(
                 &msg,
                 TASK_SCENE_PHONE_CALL, AUDIO_IPI_LAYER_TO_DSP,
                 AUDIO_IPI_DMA, AUDIO_IPI_MSG_NEED_ACK,
                 IPI_MSG_A2D_AURISYS_INIT,
                 sizeof(struct aurisys_dsp_config_t) + paramList.data_size,
                 0,
                 configAndParam);
    if (retval != 0) {
        ALOGE("%s(), fail!! retval = %d", __FUNCTION__, retval);
    }
    AUDIO_FREE_POINTER(configAndParam);
    AUDIO_FREE_POINTER(paramList.p_buffer);

    ALOGD("%s(-)", __FUNCTION__);
}


void SpeechDriverOpenDSP::UpdateAurisysConfig(const bool is_enh_need_reset) {
    struct aurisys_lib_manager_config_t *pManagerConfig = NULL;

    struct data_buf_t paramList;
    uint8_t *configAndParam = NULL;

    struct ipi_msg_t msg;
    int retval = 0;

    paramList.data_size = 0;
    paramList.memory_size = 0;
    paramList.p_buffer = NULL;


    if (mAurisysLibManager == NULL || mAurisysDspConfig == NULL) {
        return;
    }

    AL_LOCK(mAurisysDspConfigLock);
    /* manager config */
    pManagerConfig = &mAurisysDspConfig->manager_config;
    pManagerConfig->num_channels_ul = getInputChannelsByDevice(mInputDevice);

    /* custom info */
    if(mSuperVolumeEnable){
        snprintf(pManagerConfig->custom_info, MAX_CUSTOM_INFO_LEN, "vol_level=extra");
    } else {
        snprintf(pManagerConfig->custom_info, MAX_CUSTOM_INFO_LEN, "vol_level=%d", mVolumeIndex);
    }

    /* task config */
    SetArsiTaskConfig(&pManagerConfig->task_config);


    /* attribute */
    SetArsiAttribute();


    /* gain */
    mAurisysDspConfig->gain_config.ul_digital_gain = 0;
    mAurisysDspConfig->gain_config.ul_analog_gain  = 0;
    mAurisysDspConfig->gain_config.dl_digital_gain = 0;
    mAurisysDspConfig->gain_config.dl_analog_gain  = 0;

    /* func */
    mAurisysDspConfig->voip_on = false;
    mAurisysDspConfig->aec_on = true;


    AL_UNLOCK(mAurisysDspConfigLock);

    /* parsing param */
    aurisys_parsing_param_file(mAurisysLibManager);

    paramList.data_size = 0;
    paramList.memory_size = 32768; /* TODO: refine it */
    AUDIO_ALLOC_BUFFER(paramList.p_buffer, paramList.memory_size);
    do {
        retval = aurisys_get_param_list(mAurisysLibManager, &paramList);
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


    /* send config */
    if (is_enh_need_reset) {
        configAndParam = (uint8_t *)AUDIO_MALLOC(sizeof(struct aurisys_dsp_config_t) + paramList.data_size);

        if (configAndParam == NULL) {
            ALOGE("%s(), cannot malloc memory for config!!", __FUNCTION__);
            retval = -1;
        } else {
            memcpy(configAndParam, mAurisysDspConfig, sizeof(struct aurisys_dsp_config_t));
            memcpy(configAndParam + sizeof(struct aurisys_dsp_config_t), paramList.p_buffer, paramList.data_size);

            retval = mAudioMessengerIPI->sendIpiMsg(
                     &msg,
                     TASK_SCENE_PHONE_CALL, AUDIO_IPI_LAYER_TO_DSP,
                     AUDIO_IPI_DMA, AUDIO_IPI_MSG_NEED_ACK,
                     IPI_MSG_A2D_AURISYS_ROUTING,
                     sizeof(struct aurisys_dsp_config_t) + paramList.data_size,
                     0,
                     configAndParam);
        }
    } else {
        configAndParam = (uint8_t *)AUDIO_MALLOC(paramList.data_size);
        if (configAndParam == NULL) {
            ALOGE("%s(), cannot malloc memory for paramList!!", __FUNCTION__);
            retval = -1;
        } else {
            memcpy(configAndParam, paramList.p_buffer, paramList.data_size);

            retval = mAudioMessengerIPI->sendIpiMsg(
                     &msg,
                     TASK_SCENE_PHONE_CALL, AUDIO_IPI_LAYER_TO_DSP,
                     AUDIO_IPI_DMA, AUDIO_IPI_MSG_NEED_ACK,
                     IPI_MSG_A2D_AURISYS_PARAM_LIST,
                     paramList.data_size,
                     0,
                     configAndParam);
        }
    }

    if (retval != 0) {
        ALOGE("%s(), fail!! retval = %d", __FUNCTION__, retval);
    }
    AUDIO_FREE_POINTER(configAndParam);
    AUDIO_FREE_POINTER(paramList.p_buffer);
}



void SpeechDriverOpenDSP::DestroyAurisysLibManager() {
    struct ipi_msg_t ipi_msg;
    int retval = 0;

    ALOGD("%s(+)", __FUNCTION__);

    retval = mAudioMessengerIPI->sendIpiMsg(
                 &ipi_msg,
                 TASK_SCENE_PHONE_CALL,
                 AUDIO_IPI_LAYER_TO_DSP,
                 AUDIO_IPI_MSG_ONLY,
                 AUDIO_IPI_MSG_NEED_ACK,
                 IPI_MSG_A2D_AURISYS_DEINIT,
                 0,
                 0,
                 NULL);
    if (retval != 0) {
        ALOGE("%s(), fail!! retval = %d", __FUNCTION__, retval);
    }

    if (mAurisysLibManager) {
        destroy_aurisys_lib_manager(mAurisysLibManager);
        mAurisysLibManager = NULL;
    }

    AUDIO_FREE_POINTER(mAurisysDspConfig);

    ALOGD("%s(-)", __FUNCTION__);
}



/*==============================================================================
 *                     Speech Control
 *============================================================================*/

static void putTimestampAndEnableToParam(uint32_t *param, const bool enable) {
    char time_h[3];
    char time_m[3];
    char time_s[3];

    uint8_t time_value_h;
    uint8_t time_value_m;
    uint8_t time_value_s;

    time_t rawtime;
    time(&rawtime);

    struct tm *timeinfo = localtime(&rawtime);
    if (timeinfo == NULL) {
        ALOGE("%s(), unknown time info", __FUNCTION__);
    } else {
        strftime(time_h, 3, "%H", timeinfo);
        strftime(time_m, 3, "%M", timeinfo);
        strftime(time_s, 3, "%S", timeinfo);
    }


    time_value_h = (uint8_t)atoi(time_h);
    time_value_m = (uint8_t)atoi(time_m);
    time_value_s = (uint8_t)atoi(time_s);

    /* param[24:31] => hour
     * param[16:23] => minute
     * param[8:15]  => second
     * param[0:7]   => enable */
    *param = (time_value_h << 24) |
             (time_value_m << 16) |
             (time_value_s << 8)  |
             (enable);
}


static void printTimeFromParam(const uint32_t param) {
    uint8_t time_value_h = (param & 0xFF000000) >> 24;
    uint8_t time_value_m = (param & 0x00FF0000) >> 16;
    uint8_t time_value_s = (param & 0x0000FF00) >> 8;

    ALOGD("HAL Time %02d:%02d:%02d", time_value_h, time_value_m, time_value_s);
}



void SpeechDriverOpenDSP::processDmaMsg(struct ipi_msg_t *msg, void *buf, uint32_t size, void *arg) {

    ALOGD("%s() msg_id=0x%x, task_scene=%d, param2=0x%x, size=%d arg=%p\n",
          __FUNCTION__, msg->msg_id, msg->task_scene, msg->param2, size, arg);

    switch (msg->msg_id) {
    case IPI_MSG_D2A_PCM_DUMP_DATA_NOTIFY:
        if (UL_IN1 == msg->param2) {
            if (fp_pcm_dump_ul_in1) {
                AudioDumpPCMData(buf, size, fp_pcm_dump_ul_in1);
            }
        } else if (UL_IN2 == msg->param2) {
            if (fp_pcm_dump_ul_in2) {
                AudioDumpPCMData(buf, size, fp_pcm_dump_ul_in2);
            }
        } else if (UL_REF1 == msg->param2) {
            if (fp_pcm_dump_ul_ref1) {
                AudioDumpPCMData(buf, size, fp_pcm_dump_ul_ref1);
            }
        } else if (UL_REF2 == msg->param2) {
            if (fp_pcm_dump_ul_ref2) {
                AudioDumpPCMData(buf, size, fp_pcm_dump_ul_ref2);
            }
        }  else if (UL_OUT == msg->param2) {
            if (fp_pcm_dump_ul_out) {
                AudioDumpPCMData(buf, size, fp_pcm_dump_ul_out);
            }
        } else if (DL_IN == msg->param2) {
            if (fp_pcm_dump_dl_in) {
                AudioDumpPCMData(buf, size, fp_pcm_dump_dl_in);
            }
        } else if (DL_OUT == msg->param2) {
            if (fp_pcm_dump_dl_out) {
                AudioDumpPCMData(buf, size, fp_pcm_dump_dl_out);
            }
        }
        break;
    default:
        break;
    }
}

void SpeechDriverOpenDSP::createDumpFiles() {

    char file_path_ul_in1[100];
    char file_path_ul_in2[100];
    char file_path_ul_in3[100];
    char file_path_ul_ref1[100];
    char file_path_ul_ref2[100];
    char file_path_ul_out[100];
    char file_path_dl_in[100];
    char file_path_dl_out[100];
    char string_time[100];
    time_t rawtime;


    time(&rawtime);
    struct tm *timeinfo = localtime(&rawtime);
    if (timeinfo == NULL) {
        ALOGE("%s(), unknown time info", __FUNCTION__);
    } else {
        strftime(string_time, 100, "%Y_%m_%d_%H%M%S_", timeinfo);
    }

    sprintf(file_path_ul_in1, "%s%s%s", DUMP_DSP_PCM_DATA_PATH, string_time, "ul_in1.pcm");
    sprintf(file_path_ul_in2, "%s%s%s", DUMP_DSP_PCM_DATA_PATH, string_time, "ul_in2.pcm");
    sprintf(file_path_ul_in3, "%s%s%s", DUMP_DSP_PCM_DATA_PATH, string_time, "ul_in3.pcm");
    sprintf(file_path_ul_ref1, "%s%s%s", DUMP_DSP_PCM_DATA_PATH, string_time, "ul_ref1.pcm");
    sprintf(file_path_ul_ref2, "%s%s%s", DUMP_DSP_PCM_DATA_PATH, string_time, "ul_ref2.pcm");
    sprintf(file_path_ul_out, "%s%s%s", DUMP_DSP_PCM_DATA_PATH, string_time, "ul_out.pcm");
    sprintf(file_path_dl_in, "%s%s%s", DUMP_DSP_PCM_DATA_PATH, string_time, "dl_in.pcm");
    sprintf(file_path_dl_out, "%s%s%s", DUMP_DSP_PCM_DATA_PATH, string_time, "dl_out.pcm");


    mEnableDump = (get_uint32_from_property(PROPERTY_KEY_PCM_DUMP_ON) != 0);


    fp_pcm_dump_ul_in1 = AudioOpendumpPCMFile(file_path_ul_in1, PROPERTY_KEY_PCM_DUMP_ON);
    fp_pcm_dump_ul_in2 = AudioOpendumpPCMFile(file_path_ul_in2, PROPERTY_KEY_PCM_DUMP_ON);
    fp_pcm_dump_ul_in3 = AudioOpendumpPCMFile(file_path_ul_in3, PROPERTY_KEY_PCM_DUMP_ON);
    fp_pcm_dump_ul_ref1 = AudioOpendumpPCMFile(file_path_ul_ref1, PROPERTY_KEY_PCM_DUMP_ON);
    fp_pcm_dump_ul_ref2 = AudioOpendumpPCMFile(file_path_ul_ref2, PROPERTY_KEY_PCM_DUMP_ON);
    fp_pcm_dump_ul_out = AudioOpendumpPCMFile(file_path_ul_out, PROPERTY_KEY_PCM_DUMP_ON);
    fp_pcm_dump_dl_in = AudioOpendumpPCMFile(file_path_dl_in, PROPERTY_KEY_PCM_DUMP_ON);
    fp_pcm_dump_dl_out = AudioOpendumpPCMFile(file_path_dl_out, PROPERTY_KEY_PCM_DUMP_ON);

}


void SpeechDriverOpenDSP::setDspDumpWakelock(bool condition) {

    if (mixer_ctl_set_value(mixer_get_ctl_by_name(mMixer, "audio_dsp_wakelock"), 0, condition)) {
        ALOGW("%s(), set fail", __FUNCTION__);
    }
}


status_t SpeechDriverOpenDSP::ScpSpeechOn() {
    ipi_msg_t ipi_msg;
    status_t retval = NO_ERROR;
    uint32_t param1 = 0;

    ALOGD("%s(+)", __FUNCTION__);

    putTimestampAndEnableToParam(&param1, true);
    printTimeFromParam(param1);

    mAudioMessengerIPI->registerAdspFeature(AURISYS_FEATURE_ID);

    mAudioMessengerIPI->registerDmaCbk(
        TASK_SCENE_PHONE_CALL,
        0x10000,
        0x40000,
        processDmaMsg,
        this);
    CreateAurisysLibManager();


    createDumpFiles();
    mAudioMessengerIPI->registerAdspFeature(VOICE_CALL_FEATURE_ID);
    adsp_register_call = true;

    if (mEnableDump)
        setDspDumpWakelock(true);

    retval = mAudioMessengerIPI->sendIpiMsg(
                 &ipi_msg,
                 TASK_SCENE_PHONE_CALL, AUDIO_IPI_LAYER_TO_DSP,
                 AUDIO_IPI_MSG_ONLY, AUDIO_IPI_MSG_BYPASS_ACK,
                 IPI_MSG_A2D_PCM_DUMP_ON, mEnableDump, 0,
                 NULL);

    retval = mAudioMessengerIPI->sendIpiMsg(
                 &ipi_msg,
                 TASK_SCENE_PHONE_CALL, AUDIO_IPI_LAYER_TO_DSP,
                 AUDIO_IPI_MSG_ONLY, AUDIO_IPI_MSG_NEED_ACK,
                 IPI_MSG_A2D_SPH_ON, param1, mModemIndex,
                 NULL);

    ALOGD("%s(-)", __FUNCTION__);
    return NO_ERROR;
}


status_t SpeechDriverOpenDSP::ScpSpeechOff() {
    ipi_msg_t ipi_msg;
    status_t retval = NO_ERROR;
    uint32_t param1 = 0;

    ALOGD("%s(+)", __FUNCTION__);

    putTimestampAndEnableToParam(&param1, false);
    printTimeFromParam(param1);

    static const char *kPropertyKeyModemStatus = "vendor.audiohal.modem_1.status";

    for (int i = 0; i < 200; i++) {
        if (get_uint32_from_mixctrl(kPropertyKeyModemStatus) == 0) {
            break;
        } else {
            ALOGW("%s(), sleep 10ms, i = %d", __FUNCTION__, i);
            usleep(10000); // sleep 10 ms
        }
    }
    if (adsp_register_call == false) {
        ALOGD("%s call not yet register %d", __FUNCTION__, adsp_register_call);
        mAudioMessengerIPI->registerAdspFeature(VOICE_CALL_FEATURE_ID);
        adsp_register_call = true;
    }
    retval = mAudioMessengerIPI->sendIpiMsg(
                 &ipi_msg,
                 TASK_SCENE_PHONE_CALL, AUDIO_IPI_LAYER_TO_DSP,
                 AUDIO_IPI_MSG_ONLY, AUDIO_IPI_MSG_NEED_ACK,
                 IPI_MSG_A2D_SPH_ON, param1, mModemIndex,
                 NULL);

    AudioCloseDumpPCMFile(fp_pcm_dump_ul_in1);
    AudioCloseDumpPCMFile(fp_pcm_dump_ul_in2);
    AudioCloseDumpPCMFile(fp_pcm_dump_ul_in3);
    AudioCloseDumpPCMFile(fp_pcm_dump_ul_ref1);
    AudioCloseDumpPCMFile(fp_pcm_dump_ul_ref2);
    AudioCloseDumpPCMFile(fp_pcm_dump_ul_out);
    AudioCloseDumpPCMFile(fp_pcm_dump_dl_in);
    AudioCloseDumpPCMFile(fp_pcm_dump_dl_out);

    if (mEnableDump)
        setDspDumpWakelock(false);

    DestroyAurisysLibManager();

    mAudioMessengerIPI->deregisterDmaCbk(TASK_SCENE_PHONE_CALL);
    mAudioMessengerIPI->deregisterAdspFeature(VOICE_CALL_FEATURE_ID);
    adsp_register_call = false;

    mAudioMessengerIPI->deregisterAdspFeature(AURISYS_FEATURE_ID);


    ALOGD("%s(-)", __FUNCTION__);
    return NO_ERROR;
}


status_t SpeechDriverOpenDSP::ScpMaskMDIrq() {
    ipi_msg_t ipi_msg;
    uint32_t param1 = 0;
    status_t retval = NO_ERROR;

    ALOGD("%s()", __FUNCTION__);

    putTimestampAndEnableToParam(&param1, true);
    printTimeFromParam(param1);

    retval = mAudioMessengerIPI->sendIpiMsg(
                 &ipi_msg,
                 TASK_SCENE_PHONE_CALL, AUDIO_IPI_LAYER_TO_DSP,
                 AUDIO_IPI_MSG_ONLY, AUDIO_IPI_MSG_NEED_ACK,
                 IPI_MSG_A2D_MASK_MD_IRQ, param1, mModemIndex,
                 NULL);
    return NO_ERROR;
}
status_t SpeechDriverOpenDSP::SetSpeechMode(const audio_devices_t input_device,
                                            const audio_devices_t output_devices) {
    mInputDevice = input_device;
    mOutputDevices = output_devices;

    if (mApSideModemStatus != 0) {
        UpdateAurisysConfig(true);
    }

    return pSpeechDriverInternal->SetSpeechMode(input_device, output_devices);
}


status_t SpeechDriverOpenDSP::setMDVolumeIndex(int stream, int device,
                                               int index) {
    mVolumeIndex = index;

    if (mApSideModemStatus != 0) {
        UpdateAurisysConfig(false);
    }

    return pSpeechDriverInternal->setMDVolumeIndex(stream, device, index);
}


status_t SpeechDriverOpenDSP::SpeechOn() {
    ALOGD("%s(), mModemIndex = %d", __FUNCTION__, mModemIndex);

    CheckApSideModemStatusAllOffOrDie();
    SetApSideModemStatus(SPEECH_STATUS_MASK);
    SetScpSideSpeechStatus(SPEECH_STATUS_MASK);

    ScpSpeechOn();
    pSpeechDriverInternal->SpeechOn();

    return NO_ERROR;
}

status_t SpeechDriverOpenDSP::SpeechOff() {
    ALOGD("%s(), mModemIndex = %d", __FUNCTION__, mModemIndex);

    ResetScpSideSpeechStatus(SPEECH_STATUS_MASK);
    ResetApSideModemStatus(SPEECH_STATUS_MASK);
    CheckApSideModemStatusAllOffOrDie();

    // Clean gain value and mute status
    CleanGainValueAndMuteStatus();

    ScpMaskMDIrq();
    pSpeechDriverInternal->SpeechOff();
    ScpSpeechOff();

    return NO_ERROR;
}

status_t SpeechDriverOpenDSP::VideoTelephonyOn() {
    ALOGD("%s()", __FUNCTION__);
    CheckApSideModemStatusAllOffOrDie();
    SetApSideModemStatus(VT_STATUS_MASK);
    SetScpSideSpeechStatus(VT_STATUS_MASK);

    ScpSpeechOn();
    pSpeechDriverInternal->VideoTelephonyOn();
    return NO_ERROR;
}

status_t SpeechDriverOpenDSP::VideoTelephonyOff() {
    ALOGD("%s()", __FUNCTION__);
    ResetScpSideSpeechStatus(VT_STATUS_MASK);
    ResetApSideModemStatus(VT_STATUS_MASK);
    CheckApSideModemStatusAllOffOrDie();

    // Clean gain value and mute status
    CleanGainValueAndMuteStatus();

    pSpeechDriverInternal->VideoTelephonyOff();
    ScpSpeechOff();

    return NO_ERROR;
}

status_t SpeechDriverOpenDSP::SpeechRouterOn() {
    return INVALID_OPERATION;
}

status_t SpeechDriverOpenDSP::SpeechRouterOff() {
    return INVALID_OPERATION;
}


/*==============================================================================
 *                     Recording Control
 *============================================================================*/

status_t SpeechDriverOpenDSP::recordOn() {
    return pSpeechDriverInternal->recordOn();
}

status_t SpeechDriverOpenDSP::recordOn(SpcRecordTypeStruct typeRecord) {
    return pSpeechDriverInternal->recordOn(typeRecord);
}

status_t SpeechDriverOpenDSP::recordOff() {
    return pSpeechDriverInternal->recordOff();
}

status_t SpeechDriverOpenDSP::recordOff(SpcRecordTypeStruct typeRecord) {
    return pSpeechDriverInternal->recordOff(typeRecord);
}

status_t SpeechDriverOpenDSP::setPcmRecordType(SpcRecordTypeStruct typeRecord) {
    return pSpeechDriverInternal->setPcmRecordType(typeRecord);
}

status_t SpeechDriverOpenDSP::VoiceMemoRecordOn() {
    ALOGD("%s()", __FUNCTION__);
    SetApSideModemStatus(VM_RECORD_STATUS_MASK);
    return pSpeechDriverInternal->VoiceMemoRecordOn();
}

status_t SpeechDriverOpenDSP::VoiceMemoRecordOff() {
    ALOGD("%s()", __FUNCTION__);
    ResetApSideModemStatus(VM_RECORD_STATUS_MASK);
    return pSpeechDriverInternal->VoiceMemoRecordOff();
}

uint16_t SpeechDriverOpenDSP::GetRecordSampleRate() const {
    return pSpeechDriverInternal->GetRecordSampleRate();
}

uint16_t SpeechDriverOpenDSP::GetRecordChannelNumber() const {
    return pSpeechDriverInternal->GetRecordChannelNumber();
}


/*==============================================================================
 *                     Background Sound
 *============================================================================*/

status_t SpeechDriverOpenDSP::BGSoundOn() {
    return pSpeechDriverInternal->BGSoundOn();
}

status_t SpeechDriverOpenDSP::BGSoundConfig(uint8_t ul_gain, uint8_t dl_gain) {
    return pSpeechDriverInternal->BGSoundConfig(ul_gain, dl_gain);
}

status_t SpeechDriverOpenDSP::BGSoundOff() {
    return pSpeechDriverInternal->BGSoundOff();
}


/*==============================================================================
 *                     Voice Mixer
 *============================================================================*/

status_t SpeechDriverOpenDSP::VoipRxOn() {
    status_t retval = NO_ERROR;
    retval = pSpeechDriverInternal->VoipRxOn();
    SetApSideModemStatus(PCM_MIXER_STATUS_MASK);
    return retval;
}

status_t SpeechDriverOpenDSP::VoipRxOff() {
    status_t retval = NO_ERROR;
    retval = pSpeechDriverInternal->VoipRxOff();
    ResetApSideModemStatus(PCM_MIXER_STATUS_MASK);
    return retval;
}

status_t SpeechDriverOpenDSP::VoipRxConfig(const uint8_t direction, const uint8_t mixType) {
    return pSpeechDriverInternal->VoipRxConfig(direction, mixType);
}


/*==============================================================================
 *                     Telephony Tx
 *============================================================================*/

status_t SpeechDriverOpenDSP::TelephonyTxOn() {
    return pSpeechDriverInternal->TelephonyTxOn();
}


status_t SpeechDriverOpenDSP::TelephonyTxConfig(uint8_t ul_gain, uint8_t dl_gain) {
    return pSpeechDriverInternal->TelephonyTxConfig(ul_gain, dl_gain);
}


status_t SpeechDriverOpenDSP::TelephonyTxOff() {
    return pSpeechDriverInternal->TelephonyTxOff();
}


/*==============================================================================
*                     PCM 2 Way
*============================================================================*/

status_t SpeechDriverOpenDSP::PCM2WayOn(const bool wideband_on) {
    return pSpeechDriverInternal->PCM2WayOn(wideband_on);
}


status_t SpeechDriverOpenDSP::PCM2WayOff() {
    return pSpeechDriverInternal->PCM2WayOff();
}


/*==============================================================================
 *                     TTY-CTM Control
 *============================================================================*/
status_t SpeechDriverOpenDSP::TtyCtmOn() {
    ipi_msg_t ipi_msg;
    status_t retval = NO_ERROR;

    retval = mAudioMessengerIPI->sendIpiMsg(
                 &ipi_msg,
                 TASK_SCENE_PHONE_CALL, AUDIO_IPI_LAYER_TO_DSP,
                 AUDIO_IPI_MSG_ONLY, AUDIO_IPI_MSG_NEED_ACK,
                 IPI_MSG_A2D_TTY_ON, true, mTtyMode,
                 NULL);
    SetApSideModemStatus(TTY_STATUS_MASK);
    return pSpeechDriverInternal->TtyCtmOn();
}

status_t SpeechDriverOpenDSP::TtyCtmOff() {
    ipi_msg_t ipi_msg;
    status_t retval = NO_ERROR;

    retval = mAudioMessengerIPI->sendIpiMsg(
                 &ipi_msg,
                 TASK_SCENE_PHONE_CALL, AUDIO_IPI_LAYER_TO_DSP,
                 AUDIO_IPI_MSG_ONLY, AUDIO_IPI_MSG_NEED_ACK,
                 IPI_MSG_A2D_TTY_ON, false, AUD_TTY_OFF,
                 NULL);

    retval = pSpeechDriverInternal->TtyCtmOff();
    ResetApSideModemStatus(TTY_STATUS_MASK);
    return retval;
}

status_t SpeechDriverOpenDSP::TtyCtmDebugOn(bool tty_debug_flag) {
    return pSpeechDriverInternal->TtyCtmDebugOn(tty_debug_flag);
}

int SpeechDriverOpenDSP::setTtyMode(const TtyModeType ttyMode) {
    mTtyMode = ttyMode;
    return pSpeechDriverInternal->setTtyMode(ttyMode);
}

/*==============================================================================
 *                     RTT
 *============================================================================*/
int SpeechDriverOpenDSP::RttConfig(int rttMode) {
    return pSpeechDriverInternal->RttConfig(rttMode);
}

/*==============================================================================
 *                     Modem Audio DVT and Debug
 *============================================================================*/

status_t SpeechDriverOpenDSP::SetModemLoopbackPoint(uint16_t loopback_point) {
    ALOGD("%s(), loopback_point = %d", __FUNCTION__, loopback_point);
    return pSpeechDriverInternal->SetModemLoopbackPoint(loopback_point);
}

/*==============================================================================
 *                     Speech Encryption
 *============================================================================*/

status_t SpeechDriverOpenDSP::SetEncryption(bool encryption_on) {
    ALOGD("%s(), encryption_on = %d", __FUNCTION__, encryption_on);
    return pSpeechDriverInternal->SetEncryption(encryption_on);
}

/*==============================================================================
 *                     Acoustic Loopback
 *============================================================================*/

status_t SpeechDriverOpenDSP::SetAcousticLoopback(bool loopback_on) {
    ALOGD("%s(), loopback_on = %d", __FUNCTION__, loopback_on);

    if (loopback_on == true) {
        CheckApSideModemStatusAllOffOrDie();
        SetApSideModemStatus(LOOPBACK_STATUS_MASK);
        SetScpSideSpeechStatus(LOOPBACK_STATUS_MASK);

        ScpSpeechOn();
        pSpeechDriverInternal->SetAcousticLoopback(loopback_on);
    } else {
        ResetScpSideSpeechStatus(LOOPBACK_STATUS_MASK);
        ResetApSideModemStatus(LOOPBACK_STATUS_MASK);
        CheckApSideModemStatusAllOffOrDie();

        // Clean gain value and mute status
        CleanGainValueAndMuteStatus();

        pSpeechDriverInternal->SetAcousticLoopback(loopback_on);
        ScpSpeechOff();
    }

    return NO_ERROR;
}

status_t SpeechDriverOpenDSP::SetAcousticLoopbackBtCodec(bool enable_codec) {
    return pSpeechDriverInternal->SetAcousticLoopbackBtCodec(enable_codec);
}

status_t SpeechDriverOpenDSP::SetAcousticLoopbackDelayFrames(int32_t delay_frames) {
    return pSpeechDriverInternal->SetAcousticLoopbackDelayFrames(delay_frames);
}

status_t SpeechDriverOpenDSP::setLpbkFlag(bool enableLpbk) {
    return pSpeechDriverInternal->setLpbkFlag(enableLpbk);
}

/*==============================================================================
 *                     Volume Control
 *============================================================================*/

status_t SpeechDriverOpenDSP::SetDownlinkGain(int16_t gain) {
    ipi_msg_t ipi_msg;
    status_t retval = NO_ERROR;

    ALOGD("%s(), gain = 0x%x, old mDownlinkGain = 0x%x",
          __FUNCTION__, gain, mDownlinkGain);
    if (gain == mDownlinkGain) { return NO_ERROR; }

    mDownlinkGain = gain;
    mAudioMessengerIPI->registerAdspFeature(VOICE_CALL_FEATURE_ID);
    retval = mAudioMessengerIPI->sendIpiMsg(
                 &ipi_msg,
                 TASK_SCENE_PHONE_CALL, AUDIO_IPI_LAYER_TO_DSP,
                 AUDIO_IPI_MSG_ONLY, AUDIO_IPI_MSG_BYPASS_ACK,
                 IPI_MSG_A2D_DL_GAIN, gain, 0/*TODO*/,
                 NULL);
    mAudioMessengerIPI->deregisterAdspFeature(VOICE_CALL_FEATURE_ID);
    return retval;
}

status_t SpeechDriverOpenDSP::SetEnh1DownlinkGain(int16_t gain) {
    ALOGV("%s(), gain = %d", __FUNCTION__, gain);
    return INVALID_OPERATION; // not support anymore
}

status_t SpeechDriverOpenDSP::SetUplinkGain(int16_t gain) {
    ipi_msg_t ipi_msg;
    status_t retval = NO_ERROR;

    ALOGD("%s(), gain = 0x%x, old mUplinkGain = 0x%x",
          __FUNCTION__, gain, mUplinkGain);
    if (gain == mUplinkGain) { return NO_ERROR; }

    mUplinkGain = gain;
    mAudioMessengerIPI->registerAdspFeature(VOICE_CALL_FEATURE_ID);
    retval = mAudioMessengerIPI->sendIpiMsg(
                 &ipi_msg,
                 TASK_SCENE_PHONE_CALL, AUDIO_IPI_LAYER_TO_DSP,
                 AUDIO_IPI_MSG_ONLY, AUDIO_IPI_MSG_BYPASS_ACK,
                 IPI_MSG_A2D_UL_GAIN, gain, 0/*TODO*/,
                 NULL);
    mAudioMessengerIPI->deregisterAdspFeature(VOICE_CALL_FEATURE_ID);
    return retval;
}

status_t SpeechDriverOpenDSP::SetDownlinkMute(bool mute_on) {
    ipi_msg_t ipi_msg;
    status_t retval = NO_ERROR;

    ALOGD("%s(), mute_on = %d, old mDownlinkMuteOn = %d",
          __FUNCTION__, mute_on, mDownlinkMuteOn);
    if (mute_on == mDownlinkMuteOn) { return NO_ERROR; }

    mDownlinkMuteOn = mute_on;
    mAudioMessengerIPI->registerAdspFeature(VOICE_CALL_FEATURE_ID);
    // mute voice dl + bgs
    retval = mAudioMessengerIPI->sendIpiMsg(
                 &ipi_msg,
                 TASK_SCENE_PHONE_CALL, AUDIO_IPI_LAYER_TO_DSP,
                 AUDIO_IPI_MSG_ONLY, AUDIO_IPI_MSG_BYPASS_ACK,
                 IPI_MSG_A2D_DL_MUTE_ON, mute_on, 0,
                 NULL);
    mAudioMessengerIPI->deregisterAdspFeature(VOICE_CALL_FEATURE_ID);
    return pSpeechDriverInternal->SetDownlinkMute(mute_on); // for bgs
}

status_t SpeechDriverOpenDSP::SetDownlinkMuteCodec(bool mute_on) {
    ALOGD("%s(), mute_on = %d, old mDownlinkMuteOn = %d", __FUNCTION__, mute_on, mDownlinkMuteOn);
    mDownlinkMuteOn = mute_on;
    return pSpeechDriverInternal->SetDownlinkMuteCodec(mute_on);
}

status_t SpeechDriverOpenDSP::SetUplinkMute(bool mute_on) {
    ipi_msg_t ipi_msg;
    status_t retval = NO_ERROR;

    ALOGD("%s(), mute_on = %d, old mUplinkMuteOn = %d",
          __FUNCTION__, mute_on, mUplinkMuteOn);
    if (mute_on == mUplinkMuteOn) { return NO_ERROR; }

    mUplinkMuteOn = mute_on;
    mAudioMessengerIPI->registerAdspFeature(VOICE_CALL_FEATURE_ID);
    // mute voice ul + bgs
    retval = mAudioMessengerIPI->sendIpiMsg(
                 &ipi_msg,
                 TASK_SCENE_PHONE_CALL, AUDIO_IPI_LAYER_TO_DSP,
                 AUDIO_IPI_MSG_ONLY, AUDIO_IPI_MSG_BYPASS_ACK,
                 IPI_MSG_A2D_UL_MUTE_ON, mute_on, 0,
                 NULL);
    mAudioMessengerIPI->deregisterAdspFeature(VOICE_CALL_FEATURE_ID);
    return pSpeechDriverInternal->SetUplinkMute(mute_on); // for bgs
}

status_t SpeechDriverOpenDSP::SetUplinkSourceMute(bool mute_on) {
    ALOGD("%s(), mute_on = %d, old mUplinkSourceMuteOn = %d",
          __FUNCTION__, mute_on, mUplinkSourceMuteOn);
    mUplinkSourceMuteOn = mute_on;
    return pSpeechDriverInternal->SetUplinkSourceMute(mute_on);
}

status_t SpeechDriverOpenDSP::SetSidetoneGain(int16_t gain) {
    ALOGD("%s(), gain = 0x%x, old mSideToneGain = 0x%x",
          __FUNCTION__, gain, mSideToneGain);
    if (gain == mSideToneGain) { return NO_ERROR; }

    mSideToneGain = gain;
    return pSpeechDriverInternal->SetSidetoneGain(gain);
}


/*==============================================================================
 *                     Device related Config
 *============================================================================*/

status_t SpeechDriverOpenDSP::SetModemSideSamplingRate(uint16_t sample_rate) {
    ALOGD("%s(), %u => %u", __FUNCTION__, mModemPcmSampleRate, sample_rate);
    mModemPcmSampleRate = sample_rate;
    return pSpeechDriverInternal->SetModemSideSamplingRate(sample_rate);
}

status_t SpeechDriverOpenDSP::switchBTMode(uint32_t sample_rate) {
    ALOGD("%s(), sample_rate: %u", __FUNCTION__, sample_rate);
    return pSpeechDriverInternal->switchBTMode(sample_rate);
}

void SpeechDriverOpenDSP::setBTMode(const int mode) {
    ALOGD("%s(), mBTMode: %d", __FUNCTION__, mode);
    pSpeechDriverInternal->setBTMode(mode);
}

void SpeechDriverOpenDSP::setBtSpkDevice(const bool flag) {
    ALOGD("%s(), set BtSpkDevice : %d", __FUNCTION__, flag);
    pSpeechDriverInternal->setBtSpkDevice(flag);
}


/*==============================================================================
 *                     Speech Enhancement Control
 *============================================================================*/
status_t SpeechDriverOpenDSP::SetSpeechEnhancement(bool enhance_on) {
    ipi_msg_t ipi_msg;
    status_t retval = NO_ERROR;

    mAudioMessengerIPI->registerAdspFeature(VOICE_CALL_FEATURE_ID);
    retval = mAudioMessengerIPI->sendIpiMsg(
                 &ipi_msg,
                 TASK_SCENE_PHONE_CALL, AUDIO_IPI_LAYER_TO_DSP,
                 AUDIO_IPI_MSG_ONLY, AUDIO_IPI_MSG_BYPASS_ACK,
                 IPI_MSG_A2D_UL_ENHANCE_ON, enhance_on, 0,
                 NULL);
    retval = mAudioMessengerIPI->sendIpiMsg(
                 &ipi_msg,
                 TASK_SCENE_PHONE_CALL, AUDIO_IPI_LAYER_TO_DSP,
                 AUDIO_IPI_MSG_ONLY, AUDIO_IPI_MSG_BYPASS_ACK,
                 IPI_MSG_A2D_DL_ENHANCE_ON, enhance_on, 0,
                 NULL);
    mAudioMessengerIPI->deregisterAdspFeature(VOICE_CALL_FEATURE_ID);
    return NO_ERROR;
}

status_t SpeechDriverOpenDSP::SetSpeechEnhancementMask(const sph_enh_mask_struct_t &mask) {
    ALOGV("%s(), main_func = 0x%x, dynamic_func = 0x%x", __FUNCTION__, mask.main_func, mask.dynamic_func);
    return INVALID_OPERATION; // not support anymore
}

status_t SpeechDriverOpenDSP::SetBtHeadsetNrecOn(const bool bt_headset_nrec_on) {
    ALOGD("%s(), bt_headset_nrec_on %d", __FUNCTION__, bt_headset_nrec_on);
    return NO_ERROR;
}


/*==============================================================================
 *                     Speech Enhancement Parameters
 *============================================================================*/
int SpeechDriverOpenDSP::updateSpeechFeature(const SpeechFeatureType featureType, const bool flagOn) {

    if (SPEECH_FEATURE_SUPERVOLUME == featureType) {
        if (flagOn == mSuperVolumeEnable){
            ALOGD("%s(), SPEECH_FEATURE_SUPERVOLUME, flagOn(%d) == currentFeature(%d), return",
                  __FUNCTION__, flagOn, mSuperVolumeEnable);
            return -ENOSYS;
        }
    }
    mSuperVolumeEnable = flagOn;

    if (mApSideModemStatus != 0) {
        UpdateAurisysConfig(false);
    }

    return pSpeechDriverInternal->updateSpeechFeature(featureType, flagOn);
}

status_t SpeechDriverOpenDSP::GetVibSpkParam(void *eVibSpkParam) {
    return pSpeechDriverInternal->GetVibSpkParam(eVibSpkParam);
}

status_t SpeechDriverOpenDSP::SetVibSpkParam(void *eVibSpkParam) {
    return pSpeechDriverInternal->SetVibSpkParam(eVibSpkParam);
}

status_t SpeechDriverOpenDSP::SetDynamicSpeechParameters(const int type,
                                                         const void *param_arg) {
    return pSpeechDriverInternal->SetDynamicSpeechParameters(type, param_arg);
}


/*==============================================================================
 *                     Recover State
 *============================================================================*/

bool SpeechDriverOpenDSP::GetScpSideSpeechStatus(const modem_status_mask_t modem_status_mask) {
    AL_AUTOLOCK(mScpSideSpeechStatusLock);
    return ((mScpSideSpeechStatus & modem_status_mask) > 0);
}


void SpeechDriverOpenDSP::SetScpSideSpeechStatus(const modem_status_mask_t modem_status_mask) {
    ALOGD("%s(), modem_status_mask = 0x%x, mScpSideSpeechStatus = 0x%x",
          __FUNCTION__, modem_status_mask, mScpSideSpeechStatus);

    AL_AUTOLOCK(mScpSideSpeechStatusLock);

    ASSERT(((mScpSideSpeechStatus & modem_status_mask) > 0) == false);
    mScpSideSpeechStatus |= modem_status_mask;

    setScpSpeechStatus(mScpSideSpeechStatus);
}


void SpeechDriverOpenDSP::ResetScpSideSpeechStatus(const modem_status_mask_t modem_status_mask) {
    ALOGD("%s(), modem_status_mask = 0x%x, mScpSideSpeechStatus = 0x%x",
          __FUNCTION__, modem_status_mask, mScpSideSpeechStatus);

    AL_AUTOLOCK(mScpSideSpeechStatusLock);

    ASSERT(((mScpSideSpeechStatus & modem_status_mask) > 0) == true);
    mScpSideSpeechStatus &= (~modem_status_mask);

    setScpSpeechStatus(mScpSideSpeechStatus);
}


void SpeechDriverOpenDSP::RecoverModemSideStatusToInitState() {
    // Phone Call / Loopback
    if (GetScpSideSpeechStatus(SPEECH_STATUS_MASK) == true) {
        ALOGD("%s(), modem_index = %d, speech_on = true", __FUNCTION__, mModemIndex);
        ResetScpSideSpeechStatus(SPEECH_STATUS_MASK);
        ScpSpeechOff();
    } else if (GetScpSideSpeechStatus(VT_STATUS_MASK) == true) {
        ALOGD("%s(), modem_index = %d, vt_on = true", __FUNCTION__, mModemIndex);
        ResetScpSideSpeechStatus(VT_STATUS_MASK);
        ScpSpeechOff();
    } else if (GetScpSideSpeechStatus(LOOPBACK_STATUS_MASK) == true) {
        ALOGD("%s(), modem_index = %d, loopback_on = true", __FUNCTION__, mModemIndex);
        ResetScpSideSpeechStatus(LOOPBACK_STATUS_MASK);
        ScpSpeechOff();
    }
}


/*==============================================================================
 *                     Check Modem Status
 *============================================================================*/
bool SpeechDriverOpenDSP::CheckModemIsReady() {
    // TODO: [OpenDSP] scp ready
    return pSpeechDriverInternal->CheckModemIsReady();
}

void SpeechDriverOpenDSP::networkChangeCallback(void *arg)
{
    SpeechDriverOpenDSP *pSpeechDriver = static_cast<SpeechDriverOpenDSP *>(arg);
    if (pSpeechDriver == NULL) {
        ALOGE("%s(), static_cast failed!!", __FUNCTION__);
        return;
    }

    return pSpeechDriver->UpdateAurisysConfig(true);
}

} // end of namespace android


#include "AudioSCPPhoneCallController.h"
#include <audio_utils/format.h>
#include <tinyalsa/asoundlib.h>
#include <math.h>
#include <sys/resource.h>

#include "SpeechDriverFactory.h"
#include "AudioALSADriverUtility.h"
#include "AudioALSADeviceParser.h"
#include "AudioVolumeFactory.h"
#include "AudioALSAHardwareResourceManager.h"
#include "AudioUtility.h"
#include "SpeechDriverFactory.h"
#include "AudioSmartPaController.h"
#include "AudioALSADeviceConfigManager.h"
#include "AudioSmartPaParam.h"

#ifdef MTK_AURISYS_FRAMEWORK_SUPPORT
#include <audio_task.h>
#endif
#ifdef HAVE_AEE_FEATURE
#include <aee.h>
#endif

#ifdef LOG_TAG
#undef LOG_TAG
#endif
#define LOG_TAG "AudioSCPPhoneCallController"
#define calc_time_diff(x,y) ((x.tv_sec - y.tv_sec )+ (double)( x.tv_nsec - y.tv_nsec ) / (double)1000000000)
#define SCP_SPH_PERIOD_BYTES (2 * 1024)


namespace android {

AudioSCPPhoneCallController *AudioSCPPhoneCallController::mSCPPhoneCallController = NULL;
struct mixer *AudioSCPPhoneCallController::mMixer = NULL;

AudioSCPPhoneCallController *AudioSCPPhoneCallController::getInstance() {
    static AudioLock mGetInstanceLock;
    AL_AUTOLOCK(mGetInstanceLock);

    if (!mSCPPhoneCallController) {
        mSCPPhoneCallController = new AudioSCPPhoneCallController();
    }

    return mSCPPhoneCallController;
}

AudioSCPPhoneCallController::AudioSCPPhoneCallController() {
    mEnable = 0;
    mSpeechRate = 0;
    mModemIndex = MODEM_1;
    memset((void *)&mConfig, 0, sizeof(pcm_config));
    memset(&mLpbkNewTime, 0, sizeof(struct timespec));
    memset(&mLpbkOldTime, 0, sizeof(struct timespec));
    memset(&mLpbkStartTime, 0, sizeof(struct timespec));
    memset((void *)&mScpSpkHwConfig, 0, sizeof(mScpSpkHwConfig));
    mPcmMicIn = NULL;
    mPcmMicOut = NULL;
    mScpSpkPcmIn = NULL;
    mScpSpkMdUlHwPcm = NULL;
    mScpSpkIvHwPcm = NULL;
    mScpSpkDlHwPcm = NULL;

    if (mMixer == NULL) {
        mMixer = AudioALSADriverUtility::getInstance()->getMixer();
        ASSERT(mMixer != NULL);
    }
}

AudioSCPPhoneCallController::~AudioSCPPhoneCallController() {

}

int AudioSCPPhoneCallController::setPcmDump(bool enable) {
    ALOGD("%s() enable = %d", __FUNCTION__, enable);

    char value[PROPERTY_VALUE_MAX];
    int ret;

    enum mixer_ctl_type type;
    struct mixer_ctl *ctl;
    int retval = 0;

    property_get(streamout_propty, value, "0");
    int flag = atoi(value);

    if (flag == 0) {
        ALOGD("%s() %s property not set no dump", __FUNCTION__, streamout_propty);
        return 0;
    }

    ret = AudiocheckAndCreateDirectory(audio_dump_path);
    if (ret < 0) {
        ALOGE("AudiocheckAndCreateDirectory(%s) fail!", audio_dump_path);
        flag = 0;
    }

    ctl = mixer_get_ctl_by_name(mMixer, "mtk_scp_spk_pcm_dump");

    if (ctl == NULL) {
        ALOGE("mtk_scp_spk_pcm_dump not support");
        return -1;
    }

    if (enable == true) {
        retval = mixer_ctl_set_enum_by_string(ctl, "normal_dump");
        ALOGD("%s(), On enable = %d", __FUNCTION__, enable);
        ASSERT(retval == 0);
    } else {
        retval = mixer_ctl_set_enum_by_string(ctl, "off");
        ALOGD("%s(), Off enable = %d", __FUNCTION__, enable);
        ASSERT(retval == 0);
    }

    return 0;
}

int AudioSCPPhoneCallController::closeScpSpkHwPcm() {
    if (mScpSpkMdUlHwPcm != NULL) {
        pcm_close(mScpSpkMdUlHwPcm);
        mScpSpkMdUlHwPcm = NULL;
    }

    if (mScpSpkDlHwPcm != NULL) {
        pcm_close(mScpSpkDlHwPcm);
        mScpSpkDlHwPcm = NULL;
    }

    if (mScpSpkIvHwPcm != NULL) {
        pcm_close(mScpSpkIvHwPcm);
        mScpSpkIvHwPcm = NULL;
    }

    // disconnect path from md to ul memif
    mApTurnOnSequence = (mModemIndex == MODEM_1) ? AUDIO_CTL_MD1_TO_CAPTURE2 : AUDIO_CTL_MD2_TO_CAPTURE2;
    AudioALSADeviceConfigManager::getInstance()->ApplyDeviceTurnoffSequenceByName(mApTurnOnSequence);

    // disconnect dl path
    mApTurnOnSequence = AudioSmartPaController::getInstance()->getI2sSequence(AUDIO_CTL_PLAYBACK1);
    AudioSmartPaController::getInstance()->setI2sOutHD(false);
    AudioALSADeviceConfigManager::getInstance()->ApplyDeviceTurnoffSequenceByName(mApTurnOnSequence);

    // disconnect iv path
    mApTurnOnSequence = AudioSmartPaController::getInstance()->getI2sSequence(
                                                               AUDIO_CTL_I2S_TO_CAPTURE4, true);
    AudioSmartPaController::getInstance()->setI2sInHD(false);
    AudioALSADeviceConfigManager::getInstance()->ApplyDeviceTurnoffSequenceByName(mApTurnOnSequence);

    if (mixer_ctl_set_value(mixer_get_ctl_by_name(mMixer, "mtk_scp_spk_dl_scenario"), 0, false)) {
        ALOGE("Error: mtk_scp_spk_dl_scenario invalid value");
    }

    if (mixer_ctl_set_value(mixer_get_ctl_by_name(mMixer, "mtk_scp_spk_iv_scenario"), 0, false)) {
        ALOGE("Error: mtk_scp_spk_iv_scenario invalid value");
    }

    if (mixer_ctl_set_value(mixer_get_ctl_by_name(mMixer, "mtk_scp_spk_mdul_scenario"), 0, false)) {
        ALOGE("Error: mtk_scp_spk_mdul_scenario invalid value");
    }

    ALOGV("-%s(), mScpSpkDlHwPcm = %p, mScpSpkIvHwPcm = %p",
          __FUNCTION__, mScpSpkDlHwPcm, mScpSpkIvHwPcm);
    return 0;
}

int AudioSCPPhoneCallController::openScpSpkPcmDriverWithFlag(const unsigned int device,
                                                             unsigned int flag) {
    struct pcm *mScpSpkPcm = NULL;
    int ret = NO_ERROR;
    ALOGD("+%s(), pcm device = %d, flag = 0x%x", __FUNCTION__, device, flag);

    if (flag & PCM_IN) {
        if (mScpSpkMdUlHwPcm == NULL) {
            mScpSpkMdUlHwPcm = pcm_open(AudioALSADeviceParser::getInstance()->GetCardIndex(),
                                        device, flag, &mScpSpkHwConfig);
            mScpSpkPcm = mScpSpkMdUlHwPcm;
        } else {
            mScpSpkIvHwPcm = pcm_open(AudioALSADeviceParser::getInstance()->GetCardIndex(),
                                      device, flag, &mScpSpkHwConfig);
            mScpSpkPcm = mScpSpkIvHwPcm;
        }
    } else {
        mScpSpkDlHwPcm = pcm_open(AudioALSADeviceParser::getInstance()->GetCardIndex(),
                                  device, flag, &mScpSpkHwConfig);
        mScpSpkPcm = mScpSpkDlHwPcm;
    }

    if (mScpSpkPcm == NULL) {
        ALOGE("%s(), mScpSpkPcm == NULL!!", __FUNCTION__);
        ret = INVALID_OPERATION;
    } else if (pcm_is_ready(mScpSpkPcm) == false) {
        ALOGE("%s(), pcm_is_ready(%p) == false due to %s, close pcm.",
              __FUNCTION__, mScpSpkPcm, pcm_get_error(mScpSpkPcm));
        pcm_close(mScpSpkPcm);
        mScpSpkPcm = NULL;
        ret = INVALID_OPERATION;
    } else if (pcm_prepare(mScpSpkPcm) != 0) {
        ALOGE("%s(), pcm_prepare(%p) == false due to %s, close pcm.",
              __FUNCTION__, mScpSpkPcm, pcm_get_error(mScpSpkPcm));
        pcm_close(mScpSpkPcm);
        mScpSpkPcm = NULL;
        ret = INVALID_OPERATION;
    }

    ASSERT(mScpSpkPcm != NULL);

    return ret;
}

int AudioSCPPhoneCallController::openScpSpkPcmDriver(const unsigned int mdUlDevice,
                                                     const unsigned int dlDevice,
                                                     const unsigned int ivDevice) {
    int ret;
    ASSERT(mScpSpkMdUlHwPcm == NULL);
    ASSERT(mScpSpkDlHwPcm == NULL);
    ASSERT(mScpSpkIvHwPcm == NULL);
    ret = openScpSpkPcmDriverWithFlag(ivDevice, PCM_IN | PCM_MONOTONIC);
    ret = openScpSpkPcmDriverWithFlag(mdUlDevice, PCM_IN | PCM_MONOTONIC);
    ret = openScpSpkPcmDriverWithFlag(dlDevice, PCM_OUT | PCM_MONOTONIC);

    return ret;
}

int AudioSCPPhoneCallController::openScpSpkHwPcm() {
#if defined(MTK_AUDIO_KS)
    if (mixer_ctl_set_value(mixer_get_ctl_by_name(mMixer, "mtk_scp_spk_dl_scenario"), 0, true)) {
        ALOGE("Error: mtk_scp_spk_dl_scenario invalid value");
    }

    if (mixer_ctl_set_value(mixer_get_ctl_by_name(mMixer, "mtk_scp_spk_iv_scenario"), 0, true)) {
        ALOGE("Error: mtk_scp_spk_iv_scenario invalid value");
    }

    if (mixer_ctl_set_value(mixer_get_ctl_by_name(mMixer, "mtk_scp_spk_mdul_scenario"), 0, true)) {
        ALOGE("Error: mtk_scp_spk_mdul_scenario invalid value");
    }

    // data from md dl to afe ul control path
    int pcmMdUlIdx = AudioALSADeviceParser::getInstance()->GetPcmIndexByString(keypcmCapture2);
    int cardMdUlIdx = AudioALSADeviceParser::getInstance()->GetCardIndexByString(keypcmCapture2);
    mApTurnOnSequence = (mModemIndex == MODEM_1) ? AUDIO_CTL_MD1_TO_CAPTURE2 : AUDIO_CTL_MD2_TO_CAPTURE2;
    AudioALSADeviceConfigManager::getInstance()->ApplyDeviceTurnonSequenceByName(mApTurnOnSequence);

    // scp spk dl control path
    int pcmDlIdx = AudioALSADeviceParser::getInstance()->GetPcmIndexByString(keypcmPlayback1);
    int cardDlIdx = AudioALSADeviceParser::getInstance()->GetCardIndexByString(keypcmPlayback1);
    mApTurnOnSequence = AudioSmartPaController::getInstance()->getI2sSequence(AUDIO_CTL_PLAYBACK1);
    AudioSmartPaController::getInstance()->setI2sOutHD(true);
    AudioALSADeviceConfigManager::getInstance()->ApplyDeviceTurnonSequenceByName(mApTurnOnSequence);


    // scp spk iv control path
    int pcmIvIdx = AudioALSADeviceParser::getInstance()->GetPcmIndexByString(keypcmCapture4);
    int cardIvIdx = AudioALSADeviceParser::getInstance()->GetCardIndexByString(keypcmCapture4);
    mApTurnOnSequence = AudioSmartPaController::getInstance()->getI2sSequence(
                                                               AUDIO_CTL_I2S_TO_CAPTURE4, true);
    AudioALSADeviceConfigManager::getInstance()->ApplyDeviceTurnonSequenceByName(mApTurnOnSequence);
    AudioSmartPaController::getInstance()->setI2sInHD(true);

    ASSERT(mScpSpkMdUlHwPcm == NULL);
    ASSERT(mScpSpkDlHwPcm == NULL);
    ASSERT(mScpSpkIvHwPcm == NULL);

    memcpy(&mScpSpkHwConfig, &mConfig, sizeof(struct pcm_config));

    if (openScpSpkPcmDriver(pcmMdUlIdx, pcmDlIdx, pcmIvIdx)) {
        return INVALID_OPERATION;
    }
#endif
    return NO_ERROR;
}

int AudioSCPPhoneCallController::speechULPhoneMicPath(bool enable) {
    ALOGD("%s(), enable %d", __FUNCTION__, enable);
#if defined(MTK_AUDIO_KS)
    String8 apTurnOnSequence = String8(mModemIndex == MODEM_1 ? AUDIO_CTL_ADDA_UL_TO_MD1 : AUDIO_CTL_ADDA_UL_TO_MD2);

    if (enable) {
        AudioALSADeviceConfigManager::getInstance()->ApplyDeviceTurnonSequenceByName(apTurnOnSequence);

        int pcmIdx = AudioALSADeviceParser::getInstance()->GetPcmIndexByString(keypcmHostlessSpeech);
        int cardIndex = AudioALSADeviceParser::getInstance()->GetCardIndexByString(keypcmHostlessSpeech);

        ASSERT(mPcmMicOut == NULL && mPcmMicIn == NULL);
        mPcmMicIn = pcm_open(cardIndex, pcmIdx, PCM_IN, &mScpSpkHwConfig);
        mPcmMicOut = pcm_open(cardIndex, pcmIdx, PCM_OUT, &mScpSpkHwConfig);
        ASSERT(mPcmMicOut != NULL && mPcmMicIn != NULL);

        AudioALSAHardwareResourceManager::getInstance()->startInputDevice(mInputDevice);

        if (mPcmMicIn == NULL || pcm_is_ready(mPcmMicIn) == false) {
            ALOGE("%s(), Unable to open mPcmMicIn device %u (%s)", __FUNCTION__, pcmIdx, pcm_get_error(mPcmMicIn));
        } else {
            if (pcm_prepare(mPcmMicIn)) {
                ALOGE("%s(), pcm_prepare mPcmMicIn %p fail due to %s", __FUNCTION__, mPcmMicIn, pcm_get_error(mPcmMicIn));
                pcm_close(mPcmMicIn);
                mPcmMicIn = NULL;
            }
        }

        if (mPcmMicOut == NULL || pcm_is_ready(mPcmMicOut) == false) {
            ALOGE("%s(), Unable to open mPcmMicOut device %u (%s)", __FUNCTION__, pcmIdx, pcm_get_error(mPcmMicOut));
        } else {
            if (pcm_prepare(mPcmMicOut)) {
                ALOGE("%s(), pcm_prepare mPcmMicOut %p fail due to %s", __FUNCTION__, mPcmMicOut, pcm_get_error(mPcmMicOut));
                pcm_close(mPcmMicOut);
                mPcmMicOut = NULL;
            }
        }
    } else {
        if (mPcmMicIn != NULL) {
            pcm_close(mPcmMicIn);
            mPcmMicIn = NULL;
        }

        if (mPcmMicOut != NULL) {
            pcm_close(mPcmMicOut);
            mPcmMicOut = NULL;
        }

        AudioALSAHardwareResourceManager::getInstance()->stopInputDevice(mInputDevice);

        AudioALSADeviceConfigManager::getInstance()->ApplyDeviceTurnoffSequenceByName(apTurnOnSequence);
    }
#endif
    return 0;
}

int AudioSCPPhoneCallController::enable(unsigned int speechRate, const audio_devices_t inputDevice) {
    int pcmInIdx, cardIndex, ret;
    mModemIndex = SpeechDriverFactory::GetInstance()->GetActiveModemIndex();
    mInputDevice = inputDevice;

    ALOGD("+%s(), mEnable %d, md %d, rate %u ", __FUNCTION__, mEnable, mModemIndex, speechRate);

    AL_AUTOLOCK(mLock);

    if (mEnable) {
        ALOGW("%s(), already enabled, mEnable %d", __FUNCTION__, mEnable);
        return INVALID_OPERATION;
    }

    // set enable flag
    mEnable = true;
    mSpeechRate = speechRate;

#if defined(MTK_AUDIO_KS)
    pcmInIdx = AudioALSADeviceParser::getInstance()->GetPcmIndexByString(keypcmScpSpkPlayback);
    cardIndex = AudioALSADeviceParser::getInstance()->GetCardIndexByString(keypcmScpSpkPlayback);
#else
    // set modem
    if (mixer_ctl_set_enum_by_string(mixer_get_ctl_by_name(mMixer, "Audio_Scp_Voice_MD_Select"), mModemIndex == MODEM_1 ? "md1" : "md2")) {
        ALOGE("Error: SCP_voice_Modem_Select invalid value");
    }

    pcmInIdx = AudioALSADeviceParser::getInstance()->GetPcmIndexByString(keypcmScpVoicePlayback);
    cardIndex = AudioALSADeviceParser::getInstance()->GetCardIndexByString(keypcmScpVoicePlayback);
#endif

    memset(&mConfig, 0, sizeof(mConfig));
    mConfig.channels = 2;
    mConfig.rate = speechRate;
    mConfig.period_count = 4;
    mConfig.format = PCM_FORMAT_S32_LE;
    mConfig.period_size = (SCP_SPH_PERIOD_BYTES / mConfig.channels) / (mConfig.format == PCM_FORMAT_S32_LE ? 4 : 2);
    mConfig.start_threshold = 0;
    mConfig.stop_threshold = 0;
    mConfig.silence_threshold = 0;

    ASSERT(mScpSpkPcmIn == NULL);

    /* smartpa param*/
    initSmartPaConfig();

    setPcmDump(true);

#if defined(MTK_AUDIO_KS)
    ret = openScpSpkHwPcm();
    if (ret) {
        ALOGE("%s(), openScpSpkHwPcm fail", __FUNCTION__);
        return INVALID_OPERATION;
    }
#endif

    ALOGD("%s(), mConfig: channels = %d, rate = %d, period_size = %d, period_count = %d, format = %d",
          __FUNCTION__, mConfig.channels, mConfig.rate,
          mConfig.period_size, mConfig.period_count, mConfig.format);

    mScpSpkPcmIn = pcm_open(cardIndex, pcmInIdx, PCM_IN, &mConfig);

    if (pcm_start(mScpSpkPcmIn) != 0) {
        ALOGE("%s(), pcm_start(%p) == false due to %s, close pcm.",
              __FUNCTION__, mScpSpkPcmIn, pcm_get_error(mScpSpkPcmIn));
        pcm_close(mScpSpkPcmIn);
        mScpSpkPcmIn = NULL;
    }


#if defined(MTK_AUDIO_KS)
    speechULPhoneMicPath(true);
#endif

    return 0;
}

int AudioSCPPhoneCallController::disable() {
    ALOGD("+%s(), mEnable %d", __FUNCTION__, mEnable);

    AL_AUTOLOCK(mLock);

    if (!mEnable) {
        ALOGW("%s(), already disabled, mEnable %d", __FUNCTION__, mEnable);
        return INVALID_OPERATION;
    }

#if defined(MTK_AUDIO_KS)
    speechULPhoneMicPath(false);
#endif

    if (mScpSpkPcmIn != NULL) {
        pcm_stop(mScpSpkPcmIn);
        pcm_close(mScpSpkPcmIn);
        mScpSpkPcmIn = NULL;
    }

#if defined(MTK_AUDIO_KS)
    closeScpSpkHwPcm();
#endif

    mEnable = false;

    setPcmDump(false);

    return 0;
}

int AudioSCPPhoneCallController::initSmartPaConfig() {
    ALOGD("%s", __FUNCTION__);
    AudioSmartPaParam *mAudioSmartPainstance = AudioSmartPaParam::getInstance();

    if (mAudioSmartPainstance == NULL) {
        return -1;
    }
    /* output device */
    arsi_task_config_t ArsiTaskConfig;
    ArsiTaskConfig.output_device_info.devices = AUDIO_DEVICE_OUT_SPEAKER;
    ArsiTaskConfig.output_device_info.audio_format = AUDIO_FORMAT_PCM_32_BIT;
    ArsiTaskConfig.output_device_info.sample_rate = mConfig.rate;
    ArsiTaskConfig.output_device_info.channel_mask = AUDIO_CHANNEL_IN_STEREO;
    ArsiTaskConfig.output_device_info.num_channels = 2;
    ArsiTaskConfig.output_device_info.hw_info_mask = 0;

    /* task scene */
    ArsiTaskConfig.task_scene = TASK_SCENE_SPEAKER_PROTECTION;

    /* audio mode */
    ArsiTaskConfig.audio_mode = AUDIO_MODE_IN_CALL;

    /* max device capability for allocating memory */
    ArsiTaskConfig.max_output_device_sample_rate = mConfig.rate;
    ArsiTaskConfig.max_output_device_num_channels = 2;

    mAudioSmartPainstance->setArsiTaskConfig(&ArsiTaskConfig);
    mAudioSmartPainstance->setSmartpaParam();

    return 0;
}

bool AudioSCPPhoneCallController::deviceSupport(const audio_devices_t output_devices) {
    if (output_devices == AUDIO_DEVICE_OUT_SPEAKER) {
        return true;
    }
    return false;
}

bool AudioSCPPhoneCallController::isSupportPhonecall(const audio_devices_t output_devices) {
    if (AudioSmartPaController::getInstance()->isSmartPAUsed()) {
        if (deviceSupport(output_devices) &&
            AudioSmartPaController::getInstance()->getSpkProtectType() == SPK_APSCP_DSP) {
            return true;
        }
    }
    return false;
}

bool AudioSCPPhoneCallController::isEnable() {
    return mEnable;
}

unsigned int AudioSCPPhoneCallController::getSpeechRate() {
    return mSpeechRate;
}

unsigned int AudioSCPPhoneCallController::getPeriodByte(const struct pcm_config *config) {
    return config->period_size * config->channels * (pcm_format_to_bits(config->format) / 8);
}

void AudioSCPPhoneCallController::setSCPDebugInfo(bool enable, int dbgType) {
    int previousDebugEnable = mixer_ctl_get_value(mixer_get_ctl_by_name(mMixer, "ScpSpk_Voice_Debug"), 0);
    int debugEnable = 0;

    if (enable) {
        debugEnable = dbgType | previousDebugEnable;
    } else {
        debugEnable = (~dbgType) & previousDebugEnable;
    }


    ALOGD("%s(), enable %d, dbgType 0x%x, previousDebugEnable 0x%x, debugEnable 0x%x",
          __FUNCTION__, enable, dbgType, previousDebugEnable, debugEnable);

    if (mixer_ctl_set_value(mixer_get_ctl_by_name(mMixer, "ScpSpk_Voice_Debug"), 0, debugEnable)) {
        ALOGW("%s(), set ScpSpk_Voice_Debug %d fail", __FUNCTION__, debugEnable);
    }
}

}


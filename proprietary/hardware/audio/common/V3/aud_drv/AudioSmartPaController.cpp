#include "AudioSmartPaController.h"
#include "SpeechDriverFactory.h"
#include "AudioParamParser.h"
#include "AudioUtility.h"
#include "AudioALSAStreamManager.h"
#include "AudioALSASpeechPhoneCallController.h"
#include "AudioALSADeviceParser.h"
#include "AudioALSADeviceConfigManager.h"
#include "LoopbackManager.h"
#include <system/audio.h>

#include <string>
#include <dlfcn.h>

#ifdef LOG_TAG
#undef LOG_TAG
#endif
#define LOG_TAG "AudioSmartPaController"

namespace android {

typedef enum {
    PLAYBACK_DEVICE_NONE = -1,
    PLAYBACK_DEVICE_SPEAKER,
    PLAYBACK_DEVICE_RECEIVER,
    PLAYBACK_DEVICE_NUM
} playback_device_t;

/*
 * singleton
 */
AudioSmartPaController *AudioSmartPaController::mAudioSmartPaController = NULL;
AudioSmartPaController *AudioSmartPaController::getInstance() {
    static AudioLock mGetInstanceLock;
    AL_AUTOLOCK(mGetInstanceLock);

    if (mAudioSmartPaController == NULL) {
        mAudioSmartPaController = new AudioSmartPaController();
    }

    ASSERT(mAudioSmartPaController != NULL);
    return mAudioSmartPaController;
}

/*
 * constructor / destructor
 */
AudioSmartPaController::AudioSmartPaController() :
    mMixer(AudioALSADriverUtility::getInstance()->getMixer()),
    mPcmEcho(NULL),
    mPcmEchoUL(NULL),
    mLibHandle(NULL),
    mtk_smartpa_init(NULL),
    isSmartPA(false),
    isCalibrating(false),
    mPhoneCallEnable(false),
    mSpkPcmOut(NULL) {
    // init variables
    memset(&mSmartPa, 0, sizeof(mSmartPa));
    memset(&mSpkPcmConfig, 0, sizeof(mSpkPcmConfig));

    // init process
    init();
};

AudioSmartPaController::~AudioSmartPaController() {
    deinit();

    if (mLibHandle) {
        if (dlclose(mLibHandle)) {
            ALOGE("%s(), dlclose failed, dlerror = %s", __FUNCTION__, dlerror());
        }
    }
};

/*
 * function implementations
 */
int AudioSmartPaController::init() {
    int ret;

    ret = initSmartPaAttribute();
    if (ret) {
        ALOGE("%s(), initSmartPaAttribute failed, ret = %d", __FUNCTION__, ret);
        return ret;
    }

    if (!isSmartPAUsed()) {
        return 0;
    }

    ret = initSmartPaRuntime();
    if (ret) {
        ALOGE("%s(), initSmartPaRuntime failed, ret = %d", __FUNCTION__, ret);
        ASSERT(ret != 0);
        return ret;
    }

    // load lib
    ALOGD("%s(), dlopen lib path: %s", __FUNCTION__, mSmartPa.attribute.spkLibPath);
    mLibHandle = dlopen(mSmartPa.attribute.spkLibPath, RTLD_NOW);

    if (!mLibHandle) {
        ALOGW("%s(), dlopen failed, dlerror = %s", __FUNCTION__, dlerror());
    } else {
        mtk_smartpa_init = (int (*)(struct SmartPa *))dlsym(mLibHandle, "mtk_smartpa_init");
        if (!mtk_smartpa_init) {
            ALOGW("%s(), dlsym failed, dlerror = %s", __FUNCTION__, dlerror());
        }
    }

    // lib init
    if (mtk_smartpa_init) {
        ret = mtk_smartpa_init(&mSmartPa);
        if (ret) {
            ALOGE("%s(), mtk_smartpa_init failed, ret = %d", __FUNCTION__, ret);
            ASSERT(ret != 0);
            return ret;
        }
    }

#ifndef MTK_APLL_DEFAULT_OFF
    if (mSmartPa.attribute.isApllNeeded) {
        if (mixer_ctl_set_enum_by_string(mixer_get_ctl_by_name(mMixer, "Audio_always_hd_Switch"), "On")) {
            ALOGE("Error: Audio_always_hd_Switch invalid value");
        }
    }
#endif

    // reset
    speakerOff();
    dspOnBoardSpeakerOff();

    // callback init
    if (mSmartPa.ops.init) {
        if (getI2sSetStage() & SPK_I2S_AUDIOSERVER_INIT) {
#if defined(MTK_AUDIO_KS)
            struct pcm_config config;
            String8 apTurnOnSequence = String8(AUDIO_CTL_SPK_INIT);

            memset(&config, 0, sizeof(config));
            config.channels = 2;
            config.rate = 44100;
            config.period_size = 1024;
            config.period_count = 2;
            config.format = PCM_FORMAT_S32_LE;
            config.stop_threshold = ~(0U);

            int cardIndex = AudioALSADeviceParser::getInstance()->GetCardIndexByString(keypcmHostlessSpkInit);
            int pcmIndex = AudioALSADeviceParser::getInstance()->GetPcmIndexByString(keypcmHostlessSpkInit);

            struct pcm *pcm = pcm_open(cardIndex, pcmIndex , PCM_OUT, &config);
            if (pcm == NULL || pcm_is_ready(pcm) == false) {
                ALOGE("%s(), Unable to open pcm device %u (%s)", __FUNCTION__, pcmIndex , pcm_get_error(pcm));
            } else {
                if (pcm_start(pcm)) {
                    ALOGE("%s(), pcm_start %p fail due to %s", __FUNCTION__, pcm, pcm_get_error(pcm));
                }
            }

            // put after pcm_open, since apll + output widget, will trigger i2s at this stage
            AudioALSADeviceConfigManager::getInstance()->ApplyDeviceTurnonSequenceByName(apTurnOnSequence);
#else
            if (mixer_ctl_set_enum_by_string(mixer_get_ctl_by_name(mMixer, "Audio_i2s0_hd_Switch"), "On")) {
                ALOGE("%s(), Error: Audio_i2s0_hd_Switch invalid value", __FUNCTION__);
            }

            if (mixer_ctl_set_enum_by_string(mixer_get_ctl_by_name(mMixer, "Audio_i2s0_SideGen_Switch"), "On44100")) {
                ALOGE("%s(), Error: Audio_i2s0_SideGen_Switch invalid value", __FUNCTION__);
            }
#endif

            mSmartPa.ops.init(&mSmartPa);

#if defined(MTK_AUDIO_KS)
            if (pcm) {
                pcm_stop(pcm);
                pcm_close(pcm);
                pcm = NULL;
            }
            AudioALSADeviceConfigManager::getInstance()->ApplyDeviceTurnoffSequenceByName(apTurnOnSequence);
#else
            if (mixer_ctl_set_enum_by_string(mixer_get_ctl_by_name(mMixer, "Audio_i2s0_SideGen_Switch"), "Off")) {
                ALOGE("%s(), Error: Audio_i2s0_SideGen_Switch invalid value", __FUNCTION__);
            }

            if (mixer_ctl_set_enum_by_string(mixer_get_ctl_by_name(mMixer, "Audio_i2s0_hd_Switch"), "Off")) {
                ALOGE("%s(), Error: Audio_i2s0_hd_Switch invalid value", __FUNCTION__);
            }
#endif
        } else {
            mSmartPa.ops.init(&mSmartPa);
        }
    }

#if defined(MTK_AUDIO_SMARTPASCP_SUPPORT)
    if (mixer_ctl_set_value(mixer_get_ctl_by_name(mMixer, "mtk_scp_spk_set_iv_tcm_buf"), 0, true)) {
        ALOGE("%s(), Error: mtk_scp_spk_set_iv_tcm_buf invalid value", __FUNCTION__);
    }
#endif
    return ret;
}

int AudioSmartPaController::deinit() {
    if (mSmartPa.ops.deinit) {
        mSmartPa.ops.deinit();
    }

    return 0;
}

int AudioSmartPaController::getI2sSetStage() {
    return mSmartPa.attribute.i2sSetStage;
}

bool AudioSmartPaController::isInCalibration() {
    return isCalibrating;
}

bool AudioSmartPaController::isSmartPAUsed() {
    if (isSmartPA)
        return true;
    else
        return false;
}

bool AudioSmartPaController::isSmartPADynamicDetectSupport() {
#if defined(SMARTPA_DYNAMIC_DETECT)
    return true;
#else
    return false;
#endif
}

bool AudioSmartPaController::isDualSmartPA() {
    ALOGV("+%s(), isDualSmartPA\n",__FUNCTION__);

    return false; //TODO: Need to check HW support Dual Smart PA or not
}

int AudioSmartPaController::initSpkAmpType() {
    struct mixer_ctl *ctl = mixer_get_ctl_by_name(mMixer, "MTK_SPK_TYPE_GET");

    if (ctl == NULL) {
        return SPK_INVALID_TYPE;
    }

    return mixer_ctl_get_value(ctl, 0);
}

int AudioSmartPaController::initSmartPaAttribute() {
    struct SmartPaAttribute *attr = &mSmartPa.attribute;
    attr->mMixer = mMixer;

    AppOps *appOps = appOpsGetInstance();
    if (appOps == NULL) {
        ALOGE("%s(), Error: AppOps == NULL", __FUNCTION__);
        return -ENOENT;
    }

    AppHandle *appHandle = appOps->appHandleGetInstance();
    const char *spkType = appOps->appHandleGetFeatureOptionValue(appHandle, "MTK_AUDIO_SPEAKER_PATH");
    const char audioTypeName[] = "SmartPa";

    if (strstr(spkType, "smartpa")) {
        isSmartPA = true;
    }

    ALOGD("%s(), spkType: %s, isSmartPA: %d, SmartPA dynamic detect: %d\n",
          __FUNCTION__, spkType, isSmartPA, isSmartPADynamicDetectSupport());

    if (!isSmartPA) {
        return 0;
    }

    // extract parameters from xml
    AudioType *audioType;
    audioType = appOps->appHandleGetAudioTypeByName(appHandle, audioTypeName);
    if (!audioType) {
        ALOGW("error: get audioType fail, audioTypeName = %s", audioTypeName);
        ASSERT(false);
        return -ENOENT;
    }

    // Read lock
    appOps->audioTypeReadLock(audioType, __FUNCTION__);

    ParamUnit *paramUnit;
    std::string paramName(spkType);
    paramName = "Speaker type," + paramName;
    paramUnit = appOps->audioTypeGetParamUnit(audioType, paramName.c_str());
    if (!paramUnit) {
        ALOGW("error: get paramUnit fail, spkType = %s", paramName.c_str());
        appOps->audioTypeUnlock(audioType);
        ASSERT(false);
        return -ENOENT;
    }

    Param *param;
    param = appOps->paramUnitGetParamByName(paramUnit, "have_dsp");
    ASSERT(param);
    attr->dspType = *(int *)param->data;

    param = appOps->paramUnitGetParamByName(paramUnit, "chip_delay_us");
    ASSERT(param);
    attr->chipDelayUs = *(unsigned int *)param->data;

    // load lib path
    if (In64bitsProcess()) {
        param = appOps->paramUnitGetParamByName(paramUnit, "spk_lib64_path");
    } else {
        param = appOps->paramUnitGetParamByName(paramUnit, "spk_lib_path");
    }
    ASSERT(param);
    ASSERT(sizeof(attr->spkLibPath) / sizeof(char) > strlen((char *)param->data));
    memcpy(attr->spkLibPath, param->data, strlen((char *)param->data));

    // get supported sample rate list, max rate, min rate
    param = appOps->paramUnitGetParamByName(paramUnit, "supported_rate_list");
    ASSERT(param);

    if (param->arraySize * sizeof(attr->supportedRateList[0]) < sizeof(attr->supportedRateList)) {
        memcpy(attr->supportedRateList, param->data, param->arraySize * sizeof(unsigned int));
    } else {
        ALOGE("%s(), support rate list too much", __FUNCTION__);
    }

    //get if is alsa codec
    param = appOps->paramUnitGetParamByName(paramUnit, "is_alsa_codec");
    ASSERT(param);
    attr->isAlsaCodec = *(int *)param->data;

    //get codec control name, for not dsp supported SmartPA
    param = appOps->paramUnitGetParamByName(paramUnit, "codec_ctl_name");
    ASSERT(param);
    ASSERT(sizeof(attr->codecCtlName) / sizeof(char) > strlen((char *)param->data));
    memcpy(attr->codecCtlName, param->data, strlen((char *)param->data));

    //get is_apll_needed
    param = appOps->paramUnitGetParamByName(paramUnit, "is_apll_needed");
    ASSERT(param);
    attr->isApllNeeded = *(int *)param->data;

    //get i2s_set_stage
    param = appOps->paramUnitGetParamByName(paramUnit, "i2s_set_stage");
    ASSERT(param);
    attr->i2sSetStage = *(unsigned int *)param->data;

    ALOGD("dspType %d, chipDelayUs %d, spkLibPath %s, is_alsa_codec %d, codec_ctl_name %s, is_apll_needed %d, i2sSetStage %d, mixer: %p",
          attr->dspType, attr->chipDelayUs, attr->spkLibPath, attr->isAlsaCodec,
          attr->codecCtlName, attr->isApllNeeded, attr->i2sSetStage, attr->mMixer);

    // Unlock
    appOps->audioTypeUnlock(audioType);

    char rateStr[16] = {0};
    char totalRateStr[512] = {0};
    attr->supportedRateMax = 0;
    attr->supportedRateMin = UINT_MAX;
    for (size_t i = 0; i * sizeof(attr->supportedRateList[0]) < sizeof(attr->supportedRateList); i++) {
        if (attr->supportedRateList[i] == 0) {
            break;
        }

        if (attr->supportedRateList[i] > attr->supportedRateMax) {
            attr->supportedRateMax = attr->supportedRateList[i];
        }

        if (attr->supportedRateList[i] < attr->supportedRateMin) {
            attr->supportedRateMin = attr->supportedRateList[i];
        }

        memset(rateStr, 0, sizeof(rateStr));
        snprintf(rateStr, sizeof(rateStr), "%d ", attr->supportedRateList[i]);
        strncat(totalRateStr, rateStr, sizeof(totalRateStr) - strlen(totalRateStr) - 1);
    }
    ALOGD("%s(), supported rate: %s", __FUNCTION__, totalRateStr);

#if defined(MTK_AUDIO_KS)
    struct mixer_ctl *ctl;
    ctl = mixer_get_ctl_by_name(attr->mMixer, "MTK_SPK_I2S_OUT_TYPE_GET");
    if (ctl == NULL) {
        ASSERT(false);
        return AUDIO_I2S_INVALID;
    }
    attr->i2sOutSelect = mixer_ctl_get_value(ctl, 0);

    ctl = mixer_get_ctl_by_name(attr->mMixer, "MTK_SPK_I2S_IN_TYPE_GET");
    if (ctl == NULL) {
        ASSERT(false);
        return AUDIO_I2S_INVALID;
    }
    attr->i2sInSelect = mixer_ctl_get_value(ctl, 0);
    ALOGD("i2sOutSelect = %d, i2sInSelect = %d",
          attr->i2sOutSelect, attr->i2sInSelect);
#endif

    return 0;
}

int AudioSmartPaController::initSmartPaRuntime() {
    mSmartPa.runtime.sampleRate = 48000;
    mSmartPa.runtime.mode = AUDIO_MODE_NORMAL;
    mSmartPa.runtime.device = PLAYBACK_DEVICE_SPEAKER;

    return 0;
}

int AudioSmartPaController::speakerOn(unsigned int sampleRate, unsigned int device) {
    ALOGV("%s(), sampleRate = %d, device = %d", __FUNCTION__, sampleRate, device);
    int ret = 0;

    // set runtime
    mSmartPa.runtime.sampleRate = sampleRate;

    // mixer ctrl
#if !defined(MTK_AUDIO_KS)
    if (getI2sSetStage() & SPK_I2S_BEFORE_SPK_ON)
#endif
    {
        ret = dspOnBoardSpeakerOn(sampleRate);
    }

    if (strlen(mSmartPa.attribute.codecCtlName)) {
        ret = mixer_ctl_set_enum_by_string(mixer_get_ctl_by_name(mMixer, mSmartPa.attribute.codecCtlName), "On");
        if (ret) {
            ALOGE("Error: %s invalid value, ret = %d", mSmartPa.attribute.codecCtlName, ret);
        }
    }

    // speakerOn callback
    if (mSmartPa.ops.speakerOn) {
        setSmartPaRuntime(device);
        mSmartPa.ops.speakerOn(&mSmartPa.runtime);
    }

    return ret;
}

int AudioSmartPaController::speakerOff() {
    ALOGV("%s()", __FUNCTION__);

    int ret = 0;

    // speakerOff callback
    if (mSmartPa.ops.speakerOff) {
        mSmartPa.ops.speakerOff();
    }

    // mixer ctrl
#if !defined(MTK_AUDIO_KS)
    if (getI2sSetStage() & SPK_I2S_BEFORE_SPK_ON)
#endif
    {
        ret = dspOnBoardSpeakerOff();
    }

    if (strlen(mSmartPa.attribute.codecCtlName)) {
        ret = mixer_ctl_set_enum_by_string(mixer_get_ctl_by_name(mMixer, mSmartPa.attribute.codecCtlName), "Off");
        if (ret) {
            ALOGE("Error: %s invalid value, ret = %d", mSmartPa.attribute.codecCtlName, ret);
        }
    }

    setI2sHD(false, getI2sOutSelect());

    return ret;
}

unsigned int AudioSmartPaController::getSmartPaDelayUs() {
    return mSmartPa.attribute.chipDelayUs;
}

unsigned int AudioSmartPaController::getMaxSupportedRate() {
    return mSmartPa.attribute.supportedRateMax;
}

unsigned int AudioSmartPaController::getMinSupportedRate() {
    return mSmartPa.attribute.supportedRateMin;
}

bool AudioSmartPaController::isRateSupported(unsigned int rate) {
    struct SmartPaAttribute *attr = &mSmartPa.attribute;

    for (size_t i = 0; i * sizeof(attr->supportedRateList[0]) < sizeof(attr->supportedRateList); i++) {
        if (rate == attr->supportedRateList[i]) {
            return true;
        }
    }
    return false;
}

bool AudioSmartPaController::isAlsaCodec() {
    if (mSmartPa.attribute.isAlsaCodec) {
        return true;
    } else {
        return false;
    }
}

bool AudioSmartPaController::isHwDspSpkProtect(const int device) {
    if (!(device & AUDIO_DEVICE_OUT_SPEAKER)) {
        return false;
    }

    if (!isSmartPAUsed()) {
        return false;
    }

    if (mSmartPa.attribute.dspType == SPK_ONBOARD_DSP) {
        return true;
    }

    return false;
}

bool AudioSmartPaController::isSwDspSpkProtect(const int device) {
    if (!(device & AUDIO_DEVICE_OUT_SPEAKER)) {
        return false;
    }

    if (!isSmartPAUsed()) {
        return false;
    }

    if (mSmartPa.attribute.dspType != SPK_ONBOARD_DSP) {
        return true;
    }

    return false;
}

bool AudioSmartPaController::isApSideSpkProtect() {
    if (!isSmartPAUsed()) {
        return false;
    }

    if (mSmartPa.attribute.dspType == SPK_AP_DSP) {
        return true;
    } else {
        return false;
    }
}

bool AudioSmartPaController::isBypassSwDspSpkProtect() {
    if(mSmartPa.attribute.dspType != SPK_APSCP_DSP) {
        return false;
    }
#if defined(MTK_AUDIODSP_SUPPORT)
    if(mixer_ctl_get_value(mixer_get_ctl_by_name(mMixer,"swdsp_smartpa_process_enable"),0) == 1) {
        ALOGV("%s(), ap smartpa process enabled", __FUNCTION__);
        return false;
    } else {
        ALOGV("%s(), ap smartpa process not enabled", __FUNCTION__);
        return true;
    }
#elif defined(MTK_AUDIO_SMARTPASCP_SUPPORT)
    return false;
#endif

    return true;
}

unsigned int AudioSmartPaController::getSpkProtectType() {
    return mSmartPa.attribute.dspType;
}

int AudioSmartPaController::getI2sOutSelect() {
    return mSmartPa.attribute.i2sOutSelect;
}

int AudioSmartPaController::getI2sInSelect() {
    return mSmartPa.attribute.i2sInSelect;
}

String8 AudioSmartPaController::getI2sNumSequence(bool input) {
    String8 i2sString;

    switch (input ? getI2sInSelect() : getI2sOutSelect()) {
    case AUDIO_I2S0:
        i2sString = "I2S0";
        break;
    case AUDIO_I2S1:
        i2sString = "I2S1";
        break;
    case AUDIO_I2S2:
        i2sString = "I2S2";
        break;
    case AUDIO_I2S3:
        i2sString = "I2S3";
        break;
    case AUDIO_I2S5:
        i2sString = "I2S5";
        break;
    case AUDIO_TINYCONN_I2S0:
        i2sString = "TINYCONN_I2S0";
        break;
    case AUDIO_TINYCONN_I2S1:
        i2sString = "TINYCONN_I2S1";
        break;
    case AUDIO_TINYCONN_I2S2:
        i2sString = "TINYCONN_I2S2";
        break;
    case AUDIO_TINYCONN_I2S3:
        i2sString = "TINYCONN_I2S3";
        break;
    case AUDIO_TINYCONN_I2S5:
        i2sString = "TINYCONN_I2S5";
        break;
    default:
        ASSERT(0);
        i2sString = input ? "I2S0" : "I2S3";
        break;
    }

    return i2sString;
}

String8 AudioSmartPaController::getI2sSequence(const char *sequence, bool input) {
    String8 i2sString;

    i2sString = getI2sNumSequence(input);
    return input ? i2sString + String8(sequence) : String8(sequence) + i2sString;
}

String8 AudioSmartPaController::getSphEchoRefSequence(bool enable, int md) {
#if defined(MTK_AUDIO_KS)
    switch(getI2sInSelect()) {
    case AUDIO_I2S0:
        if (md == MODEM_1) {
            return String8(enable ? AUDIO_CTL_MD1_ECHO_REF_I2S0_ON : AUDIO_CTL_MD1_ECHO_REF_I2S0_OFF);
        } else {
            return String8(enable ? AUDIO_CTL_MD2_ECHO_REF_I2S0_ON : AUDIO_CTL_MD2_ECHO_REF_I2S0_OFF);
        }
    case AUDIO_I2S2:
        if (md == MODEM_1) {
            return String8(enable ? AUDIO_CTL_MD1_ECHO_REF_I2S2_ON : AUDIO_CTL_MD1_ECHO_REF_I2S2_OFF);
        } else {
            return String8(enable ? AUDIO_CTL_MD2_ECHO_REF_I2S2_ON : AUDIO_CTL_MD2_ECHO_REF_I2S2_OFF);
        }
    default:
        ALOGE("%s(), i2s in %d not support", __FUNCTION__, getI2sInSelect());
        ASSERT(0);
        return String8();
    };
#else
    ALOGV("%s(), enable %d, md %d", __FUNCTION__, enable, md);
    return String8();
#endif
}

int AudioSmartPaController::setI2sHD(bool enable, int i2sSelect) {
#if defined(MTK_AUDIO_KS)
    if (!mSmartPa.attribute.isApllNeeded) {
        return 0;
    }

    String8 setting;

    switch (i2sSelect) {
    case AUDIO_I2S0:
    case AUDIO_TINYCONN_I2S0:
        setting = enable ? AUDIO_CTL_I2S0_HD_ON : AUDIO_CTL_I2S0_HD_OFF;
        break;
    case AUDIO_I2S1:
    case AUDIO_TINYCONN_I2S1:
        setting = enable ? AUDIO_CTL_I2S1_HD_ON : AUDIO_CTL_I2S1_HD_OFF;
        break;
    case AUDIO_I2S2:
    case AUDIO_TINYCONN_I2S2:
        setting = enable ? AUDIO_CTL_I2S2_HD_ON : AUDIO_CTL_I2S2_HD_OFF;
        break;
    case AUDIO_I2S3:
    case AUDIO_TINYCONN_I2S3:
        setting = enable ? AUDIO_CTL_I2S3_HD_ON : AUDIO_CTL_I2S3_HD_OFF;
        break;
    case AUDIO_I2S5:
    case AUDIO_TINYCONN_I2S5:
        setting = enable ? AUDIO_CTL_I2S5_HD_ON : AUDIO_CTL_I2S5_HD_OFF;
        break;
    default:
        ALOGE("%s(), i2sOutSelect %d not support", __FUNCTION__, mSmartPa.attribute.i2sOutSelect);
        ASSERT(0);
        return -EINVAL;
        break;
    };

    AudioALSADeviceConfigManager::getInstance()->ApplyDeviceSettingByName(setting);
    return 0;
#else
    ALOGV("%s(), enable %d, i2sSelect %d", __FUNCTION__, enable, i2sSelect);
    return 0;
#endif
}

int AudioSmartPaController::setI2sOutHD(bool enable) {
    return setI2sHD(enable, getI2sOutSelect());
}

int AudioSmartPaController::setI2sInHD(bool enable) {
    return setI2sHD(enable, getI2sInSelect());
}

int AudioSmartPaController::dspOnBoardSpeakerOn(unsigned int sampleRate) {
    int ret = 0;
    modem_index_t modem_index = SpeechDriverFactory::GetInstance()->GetActiveModemIndex();
    bool isEchoRefEnable = (isPhoneCallOpen() && isHwDspSpkProtect(AUDIO_DEVICE_OUT_SPEAKER)) ||
                           LoopbackManager::GetInstance()->CheckIsModemLoopback(LoopbackManager::GetInstance()->GetLoopbackType()) ||
                           AudioALSASpeechPhoneCallController::getInstance()->isAudioTaste();
    ALOGD("+%s(), SampleRate: %d, MD_type: %d, isEchoRefEnable: %d\n",
          __FUNCTION__, sampleRate, modem_index, isEchoRefEnable);
#if !defined(MTK_AUDIO_KS)
    if (mSmartPa.attribute.isApllNeeded) {
        if (mixer_ctl_set_enum_by_string(mixer_get_ctl_by_name(mMixer, "Audio_i2s0_hd_Switch"), "On")) {
            ALOGE("Error: Audio_i2s0_hd_Switch invalid value");
        }
    }
#endif

    /* Config echo reference */
    if (isEchoRefEnable) {
        ALOGD("Enable speaker echo reference path for MD");
#if defined(MTK_AUDIO_KS)
        struct pcm_config config;

        memset(&config, 0, sizeof(config));
        config.channels = 2;
        config.rate = sampleRate;
        config.period_size = 1024;
        config.period_count = 2;
        config.format = PCM_FORMAT_S16_LE;
        config.stop_threshold = ~(0U);

        int cardIndex = AudioALSADeviceParser::getInstance()->GetCardIndexByString(keypcmHostlessSphEchoRef);
        int pcmIndex = AudioALSADeviceParser::getInstance()->GetPcmIndexByString(keypcmHostlessSphEchoRef);

        mPcmEcho = pcm_open(cardIndex, pcmIndex , PCM_OUT, &config);
        mPcmEchoUL = pcm_open(cardIndex, pcmIndex , PCM_IN, &config);

        // Put after pcm_open. It will trigger i2s in enable bit at this stage.
        setI2sHD(true, getI2sInSelect());
        AudioALSADeviceConfigManager::getInstance()->ApplyDeviceSettingByName(getSphEchoRefSequence(true, modem_index));

        if (mPcmEcho == NULL || pcm_is_ready(mPcmEcho) == false) {
            ALOGE("%s(), Unable to open mPcmEcho device %u (%s)", __FUNCTION__, pcmIndex , pcm_get_error(mPcmEcho));
            ASSERT(0);
        } else {
            if (pcm_start(mPcmEcho)) {
                ALOGE("%s(), pcm_start mPcmEcho %p fail due to %s", __FUNCTION__, mPcmEcho, pcm_get_error(mPcmEcho));
                ASSERT(0);
            }
        }
        if (mPcmEchoUL == NULL || pcm_is_ready(mPcmEchoUL) == false) {
            ALOGE("%s(), Unable to open mPcmEchoUL device %u (%s)", __FUNCTION__, pcmIndex , pcm_get_error(mPcmEchoUL));
            ASSERT(0);
        } else {
            if (pcm_start(mPcmEchoUL)) {
                ALOGE("%s(), pcm_start mPcmEchoUL %p fail due to %s", __FUNCTION__, mPcmEchoUL, pcm_get_error(mPcmEchoUL));
                ASSERT(0);
            }
        }
#else
        if (modem_index == MODEM_1) {
            if (mixer_ctl_set_enum_by_string(mixer_get_ctl_by_name(mMixer, "Audio_ExtCodec_EchoRef_Switch"), "MD1")) {
                ALOGE("Error: Audio_ExtCodec_EchoRef_Switch MD1 invalid value");
            }
        } else {
            if (mixer_ctl_set_enum_by_string(mixer_get_ctl_by_name(mMixer, "Audio_ExtCodec_EchoRef_Switch"), "MD3")) {
                ALOGE("Error: Audio_ExtCodec_EchoRef_Switch MD3 invalid value");
            }
        }
#endif
    }

#if !defined(MTK_AUDIO_KS)
    /* Config smartpa iv data */
    if (getSpkProtectType() == SPK_APSCP_DSP) {
        if (mixer_ctl_set_enum_by_string(mixer_get_ctl_by_name(mMixer, "Audio_ExtCodec_EchoRef_Switch"), "SCP")) {
            ALOGE("Error: Audio_ExtCodec_EchoRef_Switch SCP invalid value");
        }
    }

    /* Enable SmartPa i2s */
    switch (sampleRate) {
    case 8000:
        ret = mixer_ctl_set_enum_by_string(mixer_get_ctl_by_name(mMixer, "Audio_i2s0_SideGen_Switch"), "On8000");
        break;
    case 16000:
        ret = mixer_ctl_set_enum_by_string(mixer_get_ctl_by_name(mMixer, "Audio_i2s0_SideGen_Switch"), "On16000");
        break;
    case 32000:
        ret = mixer_ctl_set_enum_by_string(mixer_get_ctl_by_name(mMixer, "Audio_i2s0_SideGen_Switch"), "On32000");
        break;
    case 44100:
        ret = mixer_ctl_set_enum_by_string(mixer_get_ctl_by_name(mMixer, "Audio_i2s0_SideGen_Switch"), "On44100");
        break;
    case 48000:
        ret = mixer_ctl_set_enum_by_string(mixer_get_ctl_by_name(mMixer, "Audio_i2s0_SideGen_Switch"), "On48000");
        break;
    case 96000:
        ret = mixer_ctl_set_enum_by_string(mixer_get_ctl_by_name(mMixer, "Audio_i2s0_SideGen_Switch"), "On96000");
        break;
    case 192000:
        ret = mixer_ctl_set_enum_by_string(mixer_get_ctl_by_name(mMixer, "Audio_i2s0_SideGen_Switch"), "On192000");
        break;
    }

    if (ret > 0) {
        ALOGE("%s(), ERROR: Audio_i2s0_SideGen_Switch, ret = %d, samplerate = %d\n", __FUNCTION__, ret, sampleRate);
    }
#endif
    return 0;
}

int AudioSmartPaController::dspOnBoardSpeakerOff() {
    ALOGD("+%s()", __FUNCTION__);
#if defined(MTK_AUDIO_KS)
    if (mPcmEcho) {
        pcm_stop(mPcmEcho);
        pcm_close(mPcmEcho);
        mPcmEcho = NULL;
    }
    if (mPcmEchoUL) {
        pcm_stop(mPcmEchoUL);
        pcm_close(mPcmEchoUL);
        mPcmEchoUL = NULL;
    }
    AudioALSADeviceConfigManager::getInstance()->ApplyDeviceSettingByName(getSphEchoRefSequence(false, MODEM_1));
    AudioALSADeviceConfigManager::getInstance()->ApplyDeviceSettingByName(getSphEchoRefSequence(false, MODEM_2));
    setI2sHD(false, getI2sInSelect());
#else
    if (mixer_ctl_get_value(mixer_get_ctl_by_name(mMixer, "Audio_i2s0_SideGen_Switch"), 0) > 0) {
        if (mixer_ctl_set_enum_by_string(mixer_get_ctl_by_name(mMixer, "Audio_i2s0_SideGen_Switch"), "Off")) {
            ALOGE("Error: Audio_i2s0_SideGen_Switch invalid value");
        }
    }

    if (mixer_ctl_set_enum_by_string(mixer_get_ctl_by_name(mMixer, "Audio_ExtCodec_EchoRef_Switch"), "Off")) {
        ALOGE("Error: Audio_ExtCodec_EchoRef_Switch invalid value");
    }

    if (mSmartPa.attribute.isApllNeeded) {
        ALOGV("+%s(), Audio_i2s0_hd_Switch off", __FUNCTION__);
        if (mixer_ctl_set_enum_by_string(mixer_get_ctl_by_name(mMixer, "Audio_i2s0_hd_Switch"), "Off")) {
            ALOGE("Error: Audio_i2s0_hd_Switch invalid value");
        }
    }
#endif
    return 0;
}

void AudioSmartPaController::setSmartPaRuntime(unsigned int device) {
    struct SmartPaRuntime *runtime = &mSmartPa.runtime;

    /* Get information of playback mode */
    if (isPhoneCallOpen()) {
        runtime->mode = AUDIO_MODE_IN_CALL;
    } else if (AudioALSAStreamManager::getInstance()->isModeInVoipCall()) {
        runtime->mode = AUDIO_MODE_IN_COMMUNICATION;
    } else {
        runtime->mode = AUDIO_MODE_NORMAL;
    }

    /* Get information of output device */
    runtime->device = transformDeviceIndex(device);

    ALOGV("+%s(), device = %d, mode = %d", __FUNCTION__, runtime->device, runtime->mode);
}

int AudioSmartPaController::transformDeviceIndex(const unsigned int device) {
    unsigned int ret;

    if (device & AUDIO_DEVICE_OUT_SPEAKER) {
        ret = PLAYBACK_DEVICE_SPEAKER;
    } else if (device == AUDIO_DEVICE_OUT_EARPIECE) {
        ret = PLAYBACK_DEVICE_RECEIVER;
    } else {
        ALOGE("%s(), no such device supported.", __FUNCTION__);
        ret = PLAYBACK_DEVICE_NONE;
        ASSERT(false);
    }

    return ret;
}

int AudioSmartPaController::setSmartPaCalibration(int calibStage) {
    ALOGD("+%s()", __FUNCTION__);
    int result = 0;
    char rmoveFolderCmd[64] = "rm -r ";
    const char calibParamfilePath[] =
            "/system/vendor/etc/smartpa_param/calib.dat";

    if (!isSmartPAUsed()) {
        ALOGD("%s(), SmartPA not support", __FUNCTION__);
        return -1;
    }

    if (mSmartPa.ops.speakerCalibrate == NULL) {
        ALOGE("%s(), speakerCalibrate callback not implement", __FUNCTION__);
        ASSERT(0);
        return -1;
    }

    int spk_type = getSpkProtectType();

    if (spk_type == SPK_APSCP_DSP) {
        switch (calibStage) {
        case SPK_CALIB_STAGE_INIT:
            // only use stream in dump
            property_set(streamout_propty, "0");
            property_set(streamin_propty, "1");
            isCalibrating = true;

#if defined(MTK_AUDIO_SMARTPASCP_SUPPORT)
            AudioSmartPaParam::getInstance()->setParamFilePath(calibParamfilePath);
#endif
            result = mSmartPa.ops.speakerCalibrate(calibStage);
            strncat(rmoveFolderCmd, audio_dump_path,
                    sizeof(rmoveFolderCmd) - strlen(rmoveFolderCmd) - 1);
            strncat(rmoveFolderCmd, "*",
                    sizeof(rmoveFolderCmd) - strlen(rmoveFolderCmd) - 1);
            result = system(rmoveFolderCmd);
            if (result < 0) {
                ALOGE("%s error", rmoveFolderCmd);
                ASSERT(0);
            } else {
                ALOGD("%s pass", rmoveFolderCmd);
            }
            return result;
        case SPK_CALIB_STAGE_DEINIT:
            property_set(streamin_propty, "0");
            isCalibrating = false;
            return result;
        default:
            break;
        }
    }

    result = mSmartPa.ops.speakerCalibrate(SPK_CALIB_STAGE_CALCULATE_AND_SAVE);

    ALOGD("-%s(), result: %d", __FUNCTION__, result);
    return result;
}

void AudioSmartPaController::setPhoneCallEnable(int enable) {
    mPhoneCallEnable = enable;
}

void AudioSmartPaController::setSmartPaPcmEnable(int enable, int sampleRate) {
    int pcmindex = AudioALSADeviceParser::getInstance()->GetPcmIndexByString(keypcmExtSpkMeida);
    int cardindex = AudioALSADeviceParser::getInstance()->GetCardIndexByString(keypcmExtSpkMeida);

    if (enable) {
        memset(&mSpkPcmConfig, 0, sizeof(mSpkPcmConfig));
        mSpkPcmConfig.channels = 2;
        mSpkPcmConfig.rate = sampleRate;
        mSpkPcmConfig.period_size = 1024;
        mSpkPcmConfig.period_count = 2;
        mSpkPcmConfig.format = PCM_FORMAT_S32_LE;
        mSpkPcmConfig.stop_threshold = ~(0U);  // TODO: KC: check if needed

        ASSERT(mSpkPcmOut == NULL);

        mSpkPcmOut = pcm_open(cardindex, pcmindex, PCM_OUT | PCM_MONOTONIC, &mSpkPcmConfig);

        if (pcm_is_ready(mSpkPcmOut) == false) {
            ALOGE("%s(), pcm_is_ready(%p) == false due to %s, close pcm.",
                  __FUNCTION__, mSpkPcmOut, pcm_get_error(mSpkPcmOut));
            pcm_close(mSpkPcmOut);
            mSpkPcmOut = NULL;
        } else {
            if (pcm_start(mSpkPcmOut) != 0) {
                ALOGE("%s(), pcm_start(%p) fail due to %s", __FUNCTION__, mSpkPcmOut, pcm_get_error(mSpkPcmOut));
            }
        }
        ASSERT(mSpkPcmOut != NULL);
        ALOGV("%s(), mSpkPcmOut = %p", __FUNCTION__, mSpkPcmOut);
    } else {
        if (mSpkPcmOut != NULL) {
            pcm_stop(mSpkPcmOut);
            pcm_close(mSpkPcmOut);
            mSpkPcmOut = NULL;
        }
    }
}

int AudioSmartPaController::setSmartPaAutoCalibration(void) {
    int result = 0;
    struct mixer *mMixer = AudioALSADriverUtility::getInstance()->getMixer();

    // only do auto calibration in android mode
    if (InFactoryMode()) {
        return 0;
    }

    result = mSmartPa.ops.speakerCalibrate(SPK_CALIB_STAGE_INIT);
    if (result < 0) {
        ALOGD("%s(), SmartPA already calibrated", __FUNCTION__);
        return 0;
    }

    // i2s on
    if (mixer_ctl_set_enum_by_string(mixer_get_ctl_by_name(mMixer, "Audio_i2s0_hd_Switch"), "On")) {
        ALOGE("%s(), Error: Audio_i2s0_hd_Switch invalid value", __FUNCTION__);
    }
    if (mixer_ctl_set_enum_by_string(mixer_get_ctl_by_name(mMixer, "Audio_i2s0_SideGen_Switch"), "On48000")) {
        ALOGE("%s(), Audio_i2s0_SideGen_Switch, invalid value\n", __FUNCTION__);
    }

    // amp pcm on
    setSmartPaPcmEnable(true);
    if (mSmartPa.ops.speakerOn) {
        mSmartPa.ops.speakerOn(&mSmartPa.runtime);
    }

    // calibration
    result = mSmartPa.ops.speakerCalibrate(SPK_CALIB_STAGE_CALCULATE_AND_SAVE);

    // amp pcm off
    if (mSmartPa.ops.speakerOff) {
        mSmartPa.ops.speakerOff();
    }
    setSmartPaPcmEnable(false);

    // i2s off
    if (mixer_ctl_get_value(mixer_get_ctl_by_name(mMixer, "Audio_i2s0_SideGen_Switch"), 0) > 0) {
        if (mixer_ctl_set_enum_by_string(mixer_get_ctl_by_name(mMixer, "Audio_i2s0_SideGen_Switch"), "Off")) {
            ALOGE("%s(), Error: Audio_i2s0_SideGen_Switch invalid value", __FUNCTION__);
        }
    }
    if (mixer_ctl_set_enum_by_string(mixer_get_ctl_by_name(mMixer, "Audio_i2s0_hd_Switch"), "Off")) {
        ALOGE("%s(), Error: Audio_i2s0_hd_Switch invalid value", __FUNCTION__);
    }

    return result;
}

}

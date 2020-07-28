#include "AudioSmartPaParam.h"
#include "AudioUtility.h"
#include "AudioALSAStreamManager.h"
#include <string>
#include <dlfcn.h>
#include <AudioMessengerIPI.h>
#include <audio_task.h>
#include "audio_spkprotect_msg_id.h"

#ifdef LOG_TAG
#undef LOG_TAG
#endif
#define LOG_TAG "AudioSmartPaParam"
#define ARRAY_SIZE(a) (sizeof(a) / sizeof((a)[0]))
#define DMA_BUFFER_SIZE (4096)
#define AUDIO_SMARTPA_KEY_LEN (512)
#define AUDIO_SMARTPA_DEFAULT_SAMPLERATE (48000)
static char svalue[DMA_BUFFER_SIZE];


namespace android {
enum {
    SMARTPA_VENDOR_DUMMY,
    SMARTPA_VENDOR_MAXIM,
    SMARTPA_VENDOR_NXP,
    SMARTPA_VENDOR_RICHTEK,
    SMARTPA_VENDOR_CIRRUS,
    SMARTPA_VENDOR_MTK,
    SMARTPA_VENDOR_NUM,
};

static const char *aurisys_param_suffix[] = {
    ",PARAM_FILE,",
    ",APPLY_PARAM,",
    ",ADDR_VALUE,",
    ",KEY_VALUE,",
    ",ENABLE_DUMP,",
    ",ENABLE_LOG,",
};

enum {
    AURISYS_SET_PARAM_FILE,
    AURISYS_SET_APPLY_PARAM,
    AURISYS_SET_ADDR_VALUE,
    AURISYS_SET_KEY_VALUE,
    AURISYS_SET_ENABLE_DUMP,
    AURISYS_SET_ENABLE_DSP_LOG,
    AURISYS_SET_ENABLE_HAL_LOG,
    AURISYS_GET_OFFSET = 0x10,
    AURISYS_GET_ADDR_VALUE = AURISYS_GET_OFFSET + AURISYS_SET_ADDR_VALUE,
    AURISYS_GET_KEY_VALUE,
    AURISYS_PARAM_TOTAL_NUM,
};

static const char *aurisys_param_layer[] = {
    ",DSP",
    ",HAL",
};

enum {
    AURISYS_PARAM_LAYER_DSP,
    AURISYS_PARAM_LAYER_HAL,
    AURISYS_PARAM_LAYER_NUM,
};

static const char *PROPERTY_KEY_SMARTPA_PARAM_PATH = "persist.vendor.audiohal.aurisys.smartpa_path";
static const char *PROPERTY_KEY_PRODUCT_NAME = "ro.vendor.mediatek.platform";
static const char *PROPERTY_KEY_PLATFROM_NAME = "ro.product.model";
static const char *PROPERTY_KEY_PCM_DUMP_ON = "persist.vendor.audiohal.aurisys.pcm_dump_on";
static char DEFAULT_SMARTPA_SYSTEM_PARAM_PATH[] = "/system/vendor/etc/smartpa_param/SmartPaVendor1_AudioParam.dat";
static bool mEnableLibLogHAL = true;

/* lib path reserve for flexibility*/
#if defined(__LP64__)
static char aurisys_vendor_lib_path[SMARTPA_VENDOR_NUM][256] = {
    "/vendor/lib64/libaudiosmartpadummy.so",
    "/vendor/lib64/libaudiosmartpamaxim.so",
    "/vendor/lib64/libaudiosmartpanxp.so",
    "/vendor/lib64/libaudiosmartparichtek.so",
    "/vendor/lib64/libaudiosmartpacirrus.so",
    "/vendor/lib64/libaudiosmartpamtk.so",
};
#else
static char aurisys_vendor_lib_path[SMARTPA_VENDOR_NUM][256] = {
    "/vendor/lib/libaudiosmartpadummy.so",
    "/vendor/lib/libaudiosmartpamaxim.so",
    "/vendor/lib/libaudiosmartpanxp.so",
    "/vendor/lib/libaudiosmartparichtek.so",
    "/vendor/lib/libaudiosmartpacirrus.so",
    "/vendor/lib/libaudiosmartpamtk.so",
};
#endif

static void smartPaPrint(const char *message, ...) {
    if (mEnableLibLogHAL) {
        static char printf_msg[DMA_BUFFER_SIZE];

        va_list args;
        va_start(args, message);

        vsnprintf(printf_msg, sizeof(printf_msg), message, args);
        ALOGD("%s", printf_msg);

        va_end(args);
    }
}

/*
 * singleton
 */
AudioSmartPaParam *AudioSmartPaParam::mAudioSmartPaParam = NULL;

AudioSmartPaParam *AudioSmartPaParam::getInstance() {
    static AudioLock mGetInstanceLock;
    AL_UNLOCK(mGetInstanceLock);

    if (mAudioSmartPaParam == NULL) {
        mAudioSmartPaParam = new AudioSmartPaParam();
    }

    ASSERT(mAudioSmartPaParam != NULL);
    return mAudioSmartPaParam;
}

/*
 * constructor / destructor
 */
AudioSmartPaParam::AudioSmartPaParam() :
    mAudioMessengerIPI(AudioMessengerIPI::getInstance()) {
    getDefalutParamFilePath();
    getDefaultProductName();
    char property_value[PROPERTY_VALUE_MAX];

    memset((void *)mSmartParamFilePath, 0, SMARTPA_STR_LENGTH);
    memset((void *)mPhoneProductName, 0, SMARTPA_STR_LENGTH);
    mParamBuf = NULL;
    mEnableLibLogHAL = false;

    mArsiTaskConfig = (arsi_task_config_t *)malloc(sizeof(arsi_task_config_t));
    ASSERT(mArsiTaskConfig != NULL);
    memset(mArsiTaskConfig, 0, sizeof(arsi_task_config_t));
    initArsiTaskConfig();

    mArsiLibConfig = (arsi_lib_config_t *)malloc(sizeof(arsi_lib_config_t));
    ASSERT(mArsiLibConfig != NULL);
    memset(mArsiLibConfig, 0, sizeof(arsi_lib_config_t));

    initArsiLibconfig();

    mParamBuf = (data_buf_t *)malloc(sizeof(data_buf_t));
    ASSERT(mParamBuf != NULL);
    memset(mParamBuf, 0, sizeof(data_buf_t));

    mParamBuf->memory_size = DMA_BUFFER_SIZE;
    mParamBuf->data_size = 0;
    mParamBuf->p_buffer = malloc(mParamBuf->memory_size);
    ASSERT(mParamBuf->p_buffer != NULL);

    mAudioMessengerIPI->registerDmaCbk(TASK_SCENE_SPEAKER_PROTECTION,
                                       DMA_BUFFER_SIZE,
                                       DMA_BUFFER_SIZE,
                                       processSmartPaDmaMsg,
                                       NULL);

    char *dlopen_lib_path = NULL;
    int ret;
    /* check and get dlopen_lib_path */
    for (unsigned int i = 0 ; i < ARRAY_SIZE(aurisys_vendor_lib_path); i++) {
        if (access(aurisys_vendor_lib_path[i], R_OK) == 0) {
            dlopen_lib_path = aurisys_vendor_lib_path[i];
            break;
        }
    }

    // load lib
    mLibHandle = dlopen(dlopen_lib_path, RTLD_NOW);

    if (!mLibHandle) {
        ALOGW("%s(), dlopen failed, dlerror = %s", __FUNCTION__, dlerror());
    } else {
        mtk_smartpa_param_init = (int (*)(struct SmartPAParamOps *))dlsym(mLibHandle, "mtk_smartpa_param_init");
        if (!mtk_smartpa_param_init) {
            ALOGW("%s(), dlsym failed, dlerror = %s", __FUNCTION__, dlerror());
        }
    }

    // lib init
    if (mtk_smartpa_param_init) {
        ret = mtk_smartpa_param_init(&mSmartPAParam);
        if (ret) {
            ALOGE("%s(), mtk_smartpa_param_init failed, ret = %d", __FUNCTION__, ret);
            ASSERT(ret != 0);
        }
    }

    /* get param_file_path */
    char *param_file_path = NULL;
    if (access(DEFAULT_SMARTPA_SYSTEM_PARAM_PATH, R_OK) == 0) {
        param_file_path = DEFAULT_SMARTPA_SYSTEM_PARAM_PATH;
    } else {
        ALOGE("%s(), param_file_path not found!!", __FUNCTION__);
    }

    mEnableLibLogHAL = false;

    setParamFilePath(DEFAULT_SMARTPA_SYSTEM_PARAM_PATH);
    property_get(PROPERTY_KEY_PLATFROM_NAME, mPhoneProductName, "");

    string_buf_t platform_info;
    platform_info.memory_size = strlen(mPhoneProductName) + 1;
    platform_info.string_size = strlen(mPhoneProductName);
    platform_info.p_string = mPhoneProductName;

    string_buf_t file_path;
    file_path.memory_size = strlen(mSmartParamFilePath) + 1;
    file_path.string_size = strlen(mSmartParamFilePath);
    file_path.p_string = mSmartParamFilePath;

    if (mSmartPAParam.loadParam) {
        mSmartPAParam.loadParam(&platform_info, &file_path, smartPaPrint);
    } else {
        ALOGE("%s() mSmartPAParam.loadParam is NULL", __FUNCTION__);
        ASSERT(0);
    }

    ALOGD("%s(), mSmartParamFilePath = %s, dlopen lib path: %s",
          __FUNCTION__, mSmartParamFilePath, dlopen_lib_path);
}

AudioSmartPaParam::~AudioSmartPaParam() {

    mAudioMessengerIPI->deregisterDmaCbk(TASK_SCENE_SPEAKER_PROTECTION);

    if (mLibHandle) {
        if (dlclose(mLibHandle)) {
            ALOGE("%s(), dlclose failed, dlerror = %s",
                  __FUNCTION__, dlerror());
        }
    }

    if (mArsiTaskConfig != NULL) {
        free(mArsiTaskConfig);
        mArsiTaskConfig = NULL;
    }

    if (mArsiLibConfig != NULL) {
        free(mArsiLibConfig);
        mArsiLibConfig = NULL;
    }

    if (mParamBuf->p_buffer != NULL) {
        free(mParamBuf->p_buffer);
    }

    if (mParamBuf != NULL) {
        free(mParamBuf);
        mParamBuf = NULL;
    }
}

void AudioSmartPaParam::initArsiLibconfig() {
    /* alloc buffer in SCP */
    mArsiLibConfig->p_ul_buf_in = NULL;
    mArsiLibConfig->p_ul_buf_out = NULL;
    mArsiLibConfig->p_ul_ref_bufs = NULL;

    mArsiLibConfig->p_dl_buf_in = NULL;
    mArsiLibConfig->p_dl_buf_out = NULL;
    mArsiLibConfig->p_dl_ref_bufs = NULL;

    /* lib */
    mArsiLibConfig->sample_rate = AUDIO_SMARTPA_DEFAULT_SAMPLERATE;
    mArsiLibConfig->audio_format = AUDIO_SMARTPA_DEFAULT_SAMPLERATE;
    mArsiLibConfig->frame_size_ms = 0;
    mArsiLibConfig->b_interleave = 0;
}


void AudioSmartPaParam::initArsiTaskConfig() {
    /* output device */
    mArsiTaskConfig->output_device_info.devices = AUDIO_DEVICE_OUT_SPEAKER;
    mArsiTaskConfig->output_device_info.audio_format = AUDIO_FORMAT_PCM_32_BIT;
    mArsiTaskConfig->output_device_info.sample_rate = AUDIO_SMARTPA_DEFAULT_SAMPLERATE;
    mArsiTaskConfig->output_device_info.channel_mask = AUDIO_CHANNEL_IN_STEREO;
    mArsiTaskConfig->output_device_info.num_channels = 2;
    mArsiTaskConfig->output_device_info.hw_info_mask = 0;

    /* task scene */
    mArsiTaskConfig->task_scene = TASK_SCENE_SPEAKER_PROTECTION;

    /* audio mode */
    mArsiTaskConfig->audio_mode = AUDIO_MODE_NORMAL;

    /* max device capability for allocating memory */
    mArsiTaskConfig->max_output_device_sample_rate = AUDIO_SMARTPA_DEFAULT_SAMPLERATE;
    mArsiTaskConfig->max_output_device_num_channels = 2;
}

int AudioSmartPaParam::setArsiTaskConfig(const arsi_task_config_t *ArsiTaskConfig) {
    memcpy((void *)mArsiTaskConfig, (void *)ArsiTaskConfig, sizeof(arsi_task_config_t));
    return 0;
}

int AudioSmartPaParam::setArsiLibConfig(const arsi_lib_config_t *mArsiLibConfig) {
    memcpy((void *)mArsiLibConfig, (void *)mArsiLibConfig, sizeof(arsi_lib_config_t));
    return 0;
}

/* get param from lib and set to scp*/
int AudioSmartPaParam::setSmartpaParam() {
    ipi_msg_t ipi_msg;
    int retval = NO_ERROR;
    int ret;
    uint32_t param_buf_size = 0;

    string_buf_t platform_info;
    platform_info.memory_size = strlen(mPhoneProductName) + 1;
    platform_info.string_size = strlen(mPhoneProductName);
    platform_info.p_string = mPhoneProductName;

    string_buf_t file_path;
    file_path.memory_size = strlen(mSmartParamFilePath) + 1;
    file_path.string_size = strlen(mSmartParamFilePath);
    file_path.p_string = mSmartParamFilePath;

    string_buf_t custom_info;
    char custom_info_str[2] = "";
    custom_info.memory_size = strlen(custom_info_str) + 1;
    custom_info.string_size = strlen(custom_info_str);
    custom_info.p_string = custom_info_str;

    if (mSmartPAParam.queryParamSize == NULL) {
        ALOGE("%s arsi_query_param_buf_size == NULL", __FUNCTION__);
        return -1;
    }

    ret = mSmartPAParam.queryParamSize(mArsiTaskConfig,
                                       mArsiLibConfig,
                                       &platform_info,
                                       &file_path,
                                       &custom_info,
                                       &param_buf_size,
                                       smartPaPrint);
    if (mSmartPAParam.parsingParamFile == NULL) {
        ALOGE("%s queryParamSize == NULL", __FUNCTION__);
        return UNKNOWN_ERROR;
    }

    memset(mParamBuf->p_buffer, 0, mParamBuf->memory_size);
    ret = mSmartPAParam.parsingParamFile(mArsiTaskConfig,
                                         mArsiLibConfig,
                                         &platform_info,
                                         &file_path,
                                         &custom_info,
                                         mParamBuf,
                                         smartPaPrint);
    ALOGD("%s mParamBuf->data_size = %u p_buffer = %s, param_buf_size = %u",
          __FUNCTION__, mParamBuf->data_size,
          (char *)mParamBuf->p_buffer, param_buf_size);
    if (ret != NO_ERROR) {
        ALOGW("parsingParamFile fail");
        return UNKNOWN_ERROR;
    }

    retval = mAudioMessengerIPI->sendIpiMsg(
                 &ipi_msg,
                 TASK_SCENE_SPEAKER_PROTECTION, AUDIO_IPI_LAYER_TO_DSP,
                 AUDIO_IPI_DMA, AUDIO_IPI_MSG_NEED_ACK,
                 SPK_IPI_MSG_A2D_PARAM, mParamBuf->data_size, 0,
                 (char *)mParamBuf->p_buffer);
    return 0;
}

/* set param path to lib*/
int AudioSmartPaParam::setParamFilePath(const char *str) {

    if (str == NULL) {
        strncpy(mSmartParamFilePath, DEFAULT_SMARTPA_SYSTEM_PARAM_PATH, strlen(DEFAULT_SMARTPA_SYSTEM_PARAM_PATH));
    } else {
        if (access(str, R_OK) != 0) {
            ALOGE("%s(), %s can not access", __FUNCTION__, str);
            return INVALID_OPERATION;
        }
        memset((void *)mSmartParamFilePath, '\0', SMARTPA_STR_LENGTH);
        strncpy(mSmartParamFilePath, str, strlen(str));
        ALOGD("%s(), SmartParamFilePath = %s", __FUNCTION__, mSmartParamFilePath);
    }

    property_set(PROPERTY_KEY_SMARTPA_PARAM_PATH, mSmartParamFilePath);

    return 0;
}

int AudioSmartPaParam::setProductName(const char *str) {
    if (str == NULL) {
        property_get("ro.product.model", mPhoneProductName, "0");
    } else {
        ALOGD("mPhoneProductName = %s", mPhoneProductName);
    }
    return 0;
}

char *AudioSmartPaParam::getParamFilePath(void) {
    return mSmartParamFilePath;
}

char *AudioSmartPaParam::getProductName(void) {
    return mPhoneProductName;
}

int AudioSmartPaParam::getDefalutParamFilePath(void) {
    memset(mSmartParamFilePath, 0, SMARTPA_STR_LENGTH);
    strncpy(mSmartParamFilePath, DEFAULT_SMARTPA_SYSTEM_PARAM_PATH, strlen(DEFAULT_SMARTPA_SYSTEM_PARAM_PATH));
    return 0;
}

int AudioSmartPaParam::getDefaultProductName(void) {
    memset(mPhoneProductName, 0, SMARTPA_STR_LENGTH);
    property_get("ro.product.model", mPhoneProductName, "0");
    return 0;
}


int AudioSmartPaParam::getsetParameterPrefixlength(int paramindex) {
    int parameterslength;
    ALOGD("%s strlen(smartpa_aurisys_set_param_prefix = %zu %s", __FUNCTION__,
          strlen(smartpa_aurisys_set_param_prefix), smartpa_aurisys_set_param_prefix);
    ALOGD("%s strlen(aurisys_param_suffix[paramindex]) = %zu %s", __FUNCTION__,
          strlen(aurisys_param_suffix[paramindex]), aurisys_param_suffix[paramindex]);
    if (paramindex >= 0 && paramindex < AURISYS_PARAM_TOTAL_NUM)
        parameterslength = strlen(smartpa_aurisys_set_param_prefix) + strlen(aurisys_param_suffix[paramindex]);
    else {
        parameterslength = 0;
    }
    ALOGD("%s parameterslength = %d", __FUNCTION__ , parameterslength);
    return parameterslength;
}

int AudioSmartPaParam::getgetParameterPrefixlength(int paramindex) {
    int parameterslength;
    ALOGD("%s strlen(smartpa_aurisys_get_param_prefix = %zu %s", __FUNCTION__ ,
          strlen(smartpa_aurisys_get_param_prefix), smartpa_aurisys_get_param_prefix);
    ALOGD("%s strlen(aurisys_param_suffix[paramindex]) = %zu %s", __FUNCTION__,
          strlen(aurisys_param_suffix[paramindex]), aurisys_param_suffix[paramindex]);
    if (paramindex >= 0 && paramindex < AURISYS_PARAM_TOTAL_NUM)
        parameterslength = strlen(smartpa_aurisys_get_param_prefix) + strlen(aurisys_param_suffix[paramindex]);
    else {
        parameterslength = 0;
    }
    ALOGD("%s parameterslength = %d", __FUNCTION__ , parameterslength);
    return parameterslength;
}

void AudioSmartPaParam::parseSetParameterStr(const char *inputStr, char **outputStr,
                                             const int paramindex) {
    char *end = NULL;
    char *comma = NULL;
    strncpy(*outputStr, inputStr + getsetParameterPrefixlength(paramindex),
            strlen(inputStr) - getsetParameterPrefixlength(paramindex));

    end = strstr(*outputStr, "=SET");
    if (end == NULL) {
        AUD_LOG_W("%s() fail", __FUNCTION__);
    }
    *end = '\0';

    comma = strstr(*outputStr, ",");
    if (comma == NULL) {
        AUD_LOG_W("%s() fail", __FUNCTION__);
    }
    *comma = '=';

    ALOGD("%s parse_str = %s strlen = %zu paramindex = %d",
          __FUNCTION__, *outputStr, strlen(*outputStr), paramindex);
}

bool AudioSmartPaParam::checkParameter(int &paramindex, int &direction, const char *keyValue) {
    int ret = 0;
    /* init for index*/
    paramindex = 0;
    char *pch;
    char keyValuePair[AUDIO_SMARTPA_KEY_LEN];
    if (keyValue == NULL) {
        ALOGD("%s = NULL", __FUNCTION__);
    } else {
        memcpy(keyValuePair, keyValue , AUDIO_SMARTPA_KEY_LEN);
    }
    ALOGD("%s = %s", __FUNCTION__, keyValuePair);

    if (strncmp(smartpa_aurisys_set_param_prefix, keyValuePair, strlen(smartpa_aurisys_set_param_prefix)) == 0) {
        ret = true;
    } else if (strncmp(smartpa_aurisys_get_param_prefix, keyValuePair, strlen(smartpa_aurisys_get_param_prefix)) == 0) {
        ret = true;
        direction = AURISYS_GET_OFFSET;
    } else {
        ALOGE("%s return false", __FUNCTION__);
        ret = false;
    }

    if (ret == true) {
        ALOGD("1 %s = %s", __FUNCTION__, keyValuePair);
        for (unsigned int i = 0; i < ARRAY_SIZE(aurisys_param_suffix) ; i++) {
            pch = strstr(keyValuePair, aurisys_param_suffix[i]);
            if (pch != NULL) {
                paramindex = i;
                ALOGD("%s aurisys_param_suffix pch = %s paramindex = %d", __FUNCTION__, pch, paramindex);
                break;
            }
        }
    }

    return ret;
}

int AudioSmartPaParam::setParameter(const char *keyValuePair) {
    ALOGD("%s keyValuePair = %s strlen = %zu",
          __FUNCTION__, keyValuePair, strlen(keyValuePair));

    int ret = 0;
    const int max_parse_len = DMA_BUFFER_SIZE;
    char output_str[DMA_BUFFER_SIZE];
    char *parse_str = NULL;
    int paramindex, direction;
    ipi_msg_t ipi_msg;
    int enable_dump = 0;
    int enable_log = 0;
    uint32_t aurisys_addr = 0;
    uint32_t aurisys_value = 0;

    memset(output_str, '\0', DMA_BUFFER_SIZE);

    /* check if keyValuePair valid*/
    if (checkParameter(paramindex, direction, keyValuePair) == true) {
        parse_str = output_str;
        parseSetParameterStr(keyValuePair, &parse_str, paramindex);
        switch (paramindex) {
        case AURISYS_SET_PARAM_FILE: {
            setParamFilePath(output_str);
            setSmartpaParam();
            break;
        }
        case AURISYS_SET_ADDR_VALUE: {

            sscanf(output_str, "%x,%x", &aurisys_addr, &aurisys_value);
            ALOGD("addr = 0x%x, value = 0x%x", aurisys_addr, aurisys_value);
            ret = mAudioMessengerIPI->sendIpiMsg(
                      &ipi_msg,
                      TASK_SCENE_SPEAKER_PROTECTION, AUDIO_IPI_LAYER_TO_DSP,
                      AUDIO_IPI_MSG_ONLY, AUDIO_IPI_MSG_NEED_ACK,
                      SPK_IPI_MSG_A2D_SET_ADDR_VALUE, aurisys_addr, aurisys_value, NULL);
            break;
        }
        case AURISYS_SET_KEY_VALUE: {
            ALOGD("key_value = %s", output_str);
            ret = mAudioMessengerIPI->sendIpiMsg(
                      &ipi_msg,
                      TASK_SCENE_SPEAKER_PROTECTION, AUDIO_IPI_LAYER_TO_DSP,
                      AUDIO_IPI_DMA, AUDIO_IPI_MSG_NEED_ACK,
                      SPK_IPI_MSG_A2D_SET_KEY_VALUE, strlen(output_str) + 1, 0,
                      output_str);
            if (ret != NO_ERROR) {
                ALOGW("SPK_IPI_MSG_A2D_SET_KEY_VALUE fail");
            } else {
                ALOGD("return %d", ipi_msg.param1);
            }
            break;
        }
        case AURISYS_SET_ENABLE_DUMP: {
            sscanf(output_str, "%d", &enable_dump);
            ret = mAudioMessengerIPI->sendIpiMsg(
                      &ipi_msg,
                      TASK_SCENE_SPEAKER_PROTECTION, AUDIO_IPI_LAYER_TO_DSP,
                      AUDIO_IPI_MSG_ONLY, AUDIO_IPI_MSG_BYPASS_ACK,
                      SPK_IPI_MSG_A2D_PCM_DUMP_ON, enable_dump, 0,
                      NULL);
            break;
        }
        case AURISYS_SET_ENABLE_DSP_LOG: {
            sscanf(output_str, "%d", &enable_log);
            ALOGV("enh mode = %d", enable_log);
            ret = mAudioMessengerIPI->sendIpiMsg(
                      &ipi_msg,
                      TASK_SCENE_SPEAKER_PROTECTION, AUDIO_IPI_LAYER_TO_DSP,
                      AUDIO_IPI_MSG_ONLY, AUDIO_IPI_MSG_BYPASS_ACK,
                      SPK_IPI_MSG_A2D_LIB_LOG_ON, enable_log, 0,
                      NULL);

            break;
        }
        case AURISYS_SET_ENABLE_HAL_LOG: {
            break;
        }
        default:
            break;
        }
    }

    return ret;

}

char *AudioSmartPaParam::getParameter(const char *key) {
    char *Retval = NULL;
    status_t retval = NO_ERROR;
    const int max_parse_len = DMA_BUFFER_SIZE;
    char parse_str[DMA_BUFFER_SIZE];
    char output_str[DMA_BUFFER_SIZE];
    memset(parse_str, '\0', DMA_BUFFER_SIZE);
    memset(output_str, '\0', DMA_BUFFER_SIZE);
    int paramindex, direction;
    ipi_msg_t ipi_msg;
    uint32_t aurisys_addr = 0;
    uint32_t aurisys_value = 0;

    ALOGD("%s keyValuePair = %s strlen = %zu", __FUNCTION__, key, strlen(key));

    /* check if keyValuePair valid*/
    if (checkParameter(paramindex, direction, key) == true) {
        strncpy(parse_str, key + getgetParameterPrefixlength(paramindex),
                strlen(key) - getgetParameterPrefixlength(paramindex));
        ALOGD("%s parse_str = %s strlen = %zu paramindex = %d direction = %d",
              __FUNCTION__, parse_str, strlen(parse_str), paramindex, direction);

        switch ((paramindex + direction)) {
        case AURISYS_GET_ADDR_VALUE: {
            uint32_t aurisys_addr = 0;
            ALOGD("AURISYS_GET_KEY_VALUE key = %s", parse_str);
            sscanf(parse_str, "%x", &aurisys_addr);
            retval = mAudioMessengerIPI->sendIpiMsg(
                         &ipi_msg,
                         TASK_SCENE_SPEAKER_PROTECTION, AUDIO_IPI_LAYER_TO_DSP,
                         AUDIO_IPI_PAYLOAD, AUDIO_IPI_MSG_NEED_ACK,
                         SPK_IPI_MSG_A2D_GET_ADDR_VALUE, aurisys_addr, 0,
                         NULL);

            if (retval != NO_ERROR) {
                ALOGW("IPI_MSG_A2D_GET_ADDR_VALUE fail");
            } else {
                ALOGD("param1 0x%x, param2 0x%x", ipi_msg.param1, ipi_msg.param2);
            }

            // can be modiied if
            if (ipi_msg.param1 != 0) {
                snprintf(svalue, DMA_BUFFER_SIZE - 1, "0x%x", ipi_msg.param2); // -1: for '\0'
            } else {
                strncpy(svalue, "GET_FAIL", DMA_BUFFER_SIZE - 1); // -1: for '\0'
            }
            ALOGD("svalue = %s", svalue);
            return svalue;
        }
        case AURISYS_GET_KEY_VALUE: {
            ALOGD("AURISYS_GET_KEY_VALUE key = %s", parse_str);

            ipi_msg_t ipi_msg;
            retval = mAudioMessengerIPI->sendIpiMsg(
                         &ipi_msg,
                         TASK_SCENE_SPEAKER_PROTECTION, AUDIO_IPI_LAYER_TO_DSP,
                         AUDIO_IPI_DMA, AUDIO_IPI_MSG_NEED_ACK,
                         SPK_IPI_MSG_A2D_GET_KEY_VALUE, strlen(parse_str) + 1, max_parse_len,
                         parse_str);

            if (retval != NO_ERROR) {
                ALOGW("IPI_MSG_A2D_GET_KEY_VALUE fail");
            } else {
                ALOGD("param1 0x%x, param2 0x%x", ipi_msg.param1, ipi_msg.param2);
                strncpy(output_str, parse_str, ipi_msg.param2);
            }

            ALOGD("key_value = %s", output_str);

            char *p_eq = strstr(output_str, "=");
            if (p_eq != NULL) {
                ALOGD("p_eq = %s", p_eq);
            }

            if (ipi_msg.param1 == 1 &&
                p_eq != NULL && p_eq < output_str + max_parse_len - 1) {
                strncpy(svalue, strstr(output_str, "=") + 1, max_parse_len - 1); // -1: for '\0'
            } else {
                strncpy(svalue, "GET_FAIL", max_parse_len - 1); // -1: for '\0'
            }
            ALOGD("svalue = %s", svalue);
            return svalue;
        }
        default:
            break;
        }
    }

    return Retval;
}


void AudioSmartPaParam::processSmartPaDmaMsg(ipi_msg_t *msg, void *buf, uint32_t size, void *arg) {
    ALOGV("%s() msg_id=0x%x, task_scene=%d, param2=0x%x, size=%d, arg=%p, buf=%p\n",
          __FUNCTION__, msg->msg_id, msg->task_scene, msg->param2, size, arg, buf);

}


}

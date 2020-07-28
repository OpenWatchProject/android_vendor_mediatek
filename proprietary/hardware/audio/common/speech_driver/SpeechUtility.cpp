#include <SpeechUtility.h>

#include <stdio.h>
#include <stdlib.h>

#include <string.h>
#include <stdarg.h> /* va_list, va_start, va_arg, va_end */

#include <cutils/properties.h>

#include <audio_log.h>
#include <audio_time.h>

#include <AudioALSADriverUtility.h>

#ifdef LOG_TAG
#undef LOG_TAG
#endif
#define LOG_TAG "SpeechUtility"

namespace android {

typedef struct {
    char const *kPropertyKey;
    char const *kMixctrlKey;

} mixctrl_table;

const mixctrl_table prop_mix_table[] = {
    {"vendor.audiohal.modem_1.epof",         "Speech_MD_EPOF"},
    {"vendor.audiohal.modem_1.status",       "Speech_MD_Status"},
    {"vendor.audiohal.wait.ack.msgid",       "Speech_A2M_Msg_ID"},
    {"vendor.audiohal.recovery.mic_mute_on", "Speech_Mic_Mute"},
    {"vendor.audiohal.recovery.dl_mute_on",  "Speech_DL_Mute"},
    {"vendor.audiohal.recovery.ul_mute_on",  "Speech_UL_Mute"},
    {"vendor.audiohal.recovery.phone1.md",   "Speech_Phone1_MD_Idx"},
    {"vendor.audiohal.recovery.phone2.md",   "Speech_Phone2_MD_Idx"},
    {"vendor.audiohal.recovery.phone_id",    "Speech_Phone_ID"},
    {"vendor.streamout.btscowb",             "Speech_BT_SCO_WB"},
    {"vendor.audiohal.speech.shm_init",      "Speech_SHM_Init"},
    {"vendor.audiohal.speech.shm_usip",      "Speech_SHM_USIP"},
    {"vendor.audiohal.speech.shm_widx",      "Speech_SHM_Widx"},
    {"vendor.audiohal.modem_1.headversion",  "Speech_MD_HeadVersion"},
    {"vendor.audiohal.modem_1.version",      "Speech_MD_Version"}
};

#ifndef NUM_MIXCTRL_KEY
#define NUM_MIXCTRL_KEY (sizeof(prop_mix_table) / sizeof(prop_mix_table[0]))
#endif

/* dynamic enable log */
static const char *kPropertyKeySpeechLogMask = "vendor.audiohal.speech.log.mask";


void sph_memcpy(void *des, const void *src, uint32_t size) {
    char *p_src = (char *)src;
    char *p_des = (char *)des;
    uint32_t i = 0;

    for (i = 0; i < size; i++) {
        p_des[i] = p_src[i];
        asm("" ::: "memory");
    }
    asm volatile("dsb ish": : : "memory");
}


void sph_memset(void *dest, uint8_t value, uint32_t size) {
    char *p_des = (char *)dest;
    uint32_t i = 0;

    for (i = 0; i < size; i++) {
        p_des[i] = value;
        asm("" ::: "memory");
    }
    asm volatile("dsb ish": : : "memory");
}

uint32_t get_uint32_from_mixctrl(const char *property_name) {

    static AudioLock mixctrlLock;
    AL_AUTOLOCK(mixctrlLock);

    uint32_t value;
    char mixctrl_name[PROPERTY_KEY_MAX];
    uint32_t idx = 0;

    static struct mixer *mMixer = AudioALSADriverUtility::getInstance()->getMixer();
    if (mMixer == NULL) {
        return get_uint32_from_property(property_name);
    }

    for (idx = 0; idx < NUM_MIXCTRL_KEY; ++idx) {
        if (strcmp(prop_mix_table[idx].kPropertyKey, property_name) == 0) {
            strncpy(mixctrl_name, prop_mix_table[idx].kMixctrlKey, PROPERTY_KEY_MAX - 1);
            break;
        }
    }
    if (idx == NUM_MIXCTRL_KEY) {
        ALOGE("%s(), Invalid property name:%s", __FUNCTION__, property_name);
        return get_uint32_from_property(property_name);
    }

    struct mixer_ctl *ctl = mixer_get_ctl_by_name(mMixer, mixctrl_name);
    if (ctl == NULL) {
        value = get_uint32_from_property(property_name);
    } else {
        value = mixer_ctl_get_value(ctl, 0);
    }
    ALOGV("%s(), property:%s, mixctrl:%s, value:0x%x", __FUNCTION__, property_name, mixctrl_name, value);
    return value;
}

void set_uint32_to_mixctrl(const char *property_name, const uint32_t value) {

    static AudioLock mixctrlLock;
    AL_AUTOLOCK(mixctrlLock);

    char mixctrl_name[PROPERTY_KEY_MAX];
    uint32_t idx = 0;

    static struct mixer *mMixer = AudioALSADriverUtility::getInstance()->getMixer();
    if (mMixer == NULL) {
        set_uint32_to_property(property_name, value);
        return;
    }

    for (idx = 0; idx < NUM_MIXCTRL_KEY; ++idx) {
        if (strcmp(prop_mix_table[idx].kPropertyKey, property_name) == 0) {
            strncpy(mixctrl_name, prop_mix_table[idx].kMixctrlKey, PROPERTY_KEY_MAX - 1);
            break;
        }
    }
    if (idx == NUM_MIXCTRL_KEY) {
        ALOGE("%s(), Invalid property name:%s", __FUNCTION__, property_name);
        set_uint32_to_property(property_name, value);
        return;
    }

    struct mixer_ctl *ctl = mixer_get_ctl_by_name(mMixer, mixctrl_name);
    if (ctl == NULL) {
        set_uint32_to_property(property_name, value);
    } else {
        if (mixer_ctl_set_value(ctl, 0, value)) {
            ALOGE("%s() , Error: %s %d", __FUNCTION__, mixctrl_name, value);
        }
    }
    ALOGV("%s(), property:%s, mixctrl:%s, value:0x%x", __FUNCTION__, property_name, mixctrl_name, value);
    return;
}

uint32_t get_uint32_from_property(const char *property_name) {

    uint32_t retval = 0;
    char property_value[PROPERTY_VALUE_MAX];
    struct timespec ts_start;
    struct timespec ts_stop;
    uint64_t time_diff_msg = 0;

    audio_get_timespec_monotonic(&ts_start);
    property_get(property_name, property_value, "0"); // default 0
    audio_get_timespec_monotonic(&ts_stop);

    time_diff_msg = get_time_diff_ms(&ts_start, &ts_stop);
    if ((time_diff_msg) >= 300) {
        ALOGE("%s(), property_name: %s, get %ju ms is too long",
              __FUNCTION__, property_name, time_diff_msg);
    }
    sscanf(property_value, "%u", &retval);
    return retval;
}


void set_uint32_to_property(const char *property_name, const uint32_t value) {
    if (!property_name) {
        return;
    }
    char property_value[PROPERTY_VALUE_MAX];
    snprintf(property_value, sizeof(property_value), "%u", value);

    struct timespec ts_start;
    struct timespec ts_stop;
    uint64_t time_diff_msg = 0;

    audio_get_timespec_monotonic(&ts_start);
    property_set(property_name, property_value);
    audio_get_timespec_monotonic(&ts_stop);

    time_diff_msg = get_time_diff_ms(&ts_start, &ts_stop);
    if ((time_diff_msg) >= 300) {
        ALOGE("%s(), property_name: %s, set %ju ms is too long",
              __FUNCTION__, property_name, time_diff_msg);
    }
}

void get_string_from_property(const char *property_name, char *string, const uint32_t string_size) {
    if (!property_name || !string || !string_size) {
        return;
    }

    char property_string[PROPERTY_VALUE_MAX] = {0};
    struct timespec ts_start;
    struct timespec ts_stop;
    uint64_t time_diff_msg = 0;

    audio_get_timespec_monotonic(&ts_start);
    property_get(property_name, property_string, ""); // default none
    audio_get_timespec_monotonic(&ts_stop);

    time_diff_msg = get_time_diff_ms(&ts_start, &ts_stop);
    if ((time_diff_msg) >= 300) {
        ALOGE("%s(), property_name: %s, get %ju ms is too long",
              __FUNCTION__, property_name, time_diff_msg);
    }
    strncpy(string, property_string, string_size - 1);
}


void set_string_to_property(const char *property_name, const char *string) {
    char property_string[PROPERTY_VALUE_MAX] = {0};
    strncpy(property_string, string, sizeof(property_string) - 1);

    struct timespec ts_start;
    struct timespec ts_stop;
    uint64_t time_diff_msg = 0;

    audio_get_timespec_monotonic(&ts_start);
    property_set(property_name, property_string);
    audio_get_timespec_monotonic(&ts_stop);

    time_diff_msg = get_time_diff_ms(&ts_start, &ts_stop);
    if ((time_diff_msg) >= 300) {
        ALOGE("%s(), property_name:%s, set %ju ms is too long",
              __FUNCTION__, property_name, time_diff_msg);
    }
}


uint16_t sph_sample_rate_enum_to_value(const sph_sample_rate_t sample_rate_enum) {
    uint16_t sample_rate_value = 32000;

    switch (sample_rate_enum) {
    case SPH_SAMPLE_RATE_08K:
        sample_rate_value = 8000;
        break;
    case SPH_SAMPLE_RATE_16K:
        sample_rate_value = 16000;
        break;
    case SPH_SAMPLE_RATE_32K:
        sample_rate_value = 32000;
        break;
    case SPH_SAMPLE_RATE_48K:
        sample_rate_value = 48000;
        break;
    default:
        ALOGW("%s(), sample_rate_enum %d not support!! use 32000 instead",
              __FUNCTION__, sample_rate_enum);
        sample_rate_value = 32000;
    }

    return sample_rate_value;
}


sph_sample_rate_t sph_sample_rate_value_to_enum(const uint16_t sample_rate_value) {
    sph_sample_rate_t sample_rate_enum = SPH_SAMPLE_RATE_32K;

    switch (sample_rate_value) {
    case 8000:
        sample_rate_enum = SPH_SAMPLE_RATE_08K;
        break;
    case 16000:
        sample_rate_enum = SPH_SAMPLE_RATE_16K;
        break;
    case 32000:
        sample_rate_enum = SPH_SAMPLE_RATE_32K;
        break;
    case 48000:
        sample_rate_enum = SPH_SAMPLE_RATE_48K;
        break;
    default:
        ALOGW("%s(), sample_rate_value %d not support!! use 32000 instead",
              __FUNCTION__, sample_rate_value);
        sample_rate_enum = SPH_SAMPLE_RATE_32K;
    }

    return sample_rate_enum;
}


void dynamic_speech_log(uint32_t sph_log_level_mask, const char *file_path, const char *message, ...) {
    if (!file_path || !message) {
        return;
    }

    if ((sph_log_level_mask & get_uint32_from_property(kPropertyKeySpeechLogMask)) == 0) {
        return;
    }

    char printf_msg[256];
    const char *slash = strrchr(file_path, '/');
    const char *file_name = (slash) ? slash + 1 : file_path;

    va_list args;
    va_start(args, message);
    vsnprintf(printf_msg, sizeof(printf_msg), message, args);
    ALOGD("[%s] %s", file_name, printf_msg);
    va_end(args);
}


} /* end namespace android */


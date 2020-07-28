#ifndef ANDROID_SPEECH_UTILITY_H
#define ANDROID_SPEECH_UTILITY_H

#include <stdint.h>
#include <string.h>

#include <errno.h>

#include <sys/prctl.h> /*  for prctl & PR_SET_NAME */
#include <sys/resource.h> /*  for PRIO_PROCESS */

#include <audio_log.h>

#include <SpeechType.h>



namespace android {


/*
 * =============================================================================
 *                     typedeh
 * =============================================================================
 */

/** all on => "adb shell setprop af.speech.log.mask 7" */
typedef uint32_t sph_log_level_mask_t;

enum { /* sph_log_level_mask_t */
    SPH_LOG_LEVEL_MASK_DEBUG    = (1 << 0),
    SPH_LOG_LEVEL_MASK_VERBOSE  = (1 << 1),
    SPH_LOG_LEVEL_MASK_TRASH    = (1 << 2)
};




/*
 * =============================================================================
 *                     ref struct
 * =============================================================================
 */

struct sph_msg_t;


/*
 * =============================================================================
 *                     utility
 * =============================================================================
 */

void sph_memcpy(void *des, const void *src, uint32_t size);
void sph_memset(void *dest, uint8_t value, uint32_t size);

uint32_t get_uint32_from_mixctrl(const char *property_name);
void set_uint32_to_mixctrl(const char *property_name, const uint32_t value);

uint32_t get_uint32_from_property(const char *property_name);
void set_uint32_to_property(const char *property_name, const uint32_t value);

void get_string_from_property(const char *property_name, char *string, const uint32_t string_size);
void set_string_to_property(const char *property_name, const char *string);


uint16_t sph_sample_rate_enum_to_value(const sph_sample_rate_t sample_rate_enum);
sph_sample_rate_t sph_sample_rate_value_to_enum(const uint16_t sample_rate_value);


void dynamic_speech_log(uint32_t sph_log_level_mask, const char *function_name, const char *message, ...);

#define SPH_LOG_D(fmt, arg...) \
    dynamic_speech_log(SPH_LOG_LEVEL_MASK_DEBUG, __FILE__, fmt, ##arg)

#define SPH_LOG_V(fmt, arg...) \
    dynamic_speech_log(SPH_LOG_LEVEL_MASK_VERBOSE, __FILE__, fmt, ##arg)

#define SPH_LOG_T(fmt, arg...) \
    dynamic_speech_log(SPH_LOG_LEVEL_MASK_TRASH, __FILE__, fmt, ##arg)

#ifdef SLOG_ENG
#undef SLOG_ENG
#endif

#ifdef CONFIG_MT_ENG_BUILD
#define SLOG_ENG ALOGD
#else
#define SLOG_ENG ALOGV
#endif


#define PRINT_SPH_MSG(ALOGX, description, p_sph_msg) \
    do { \
        if (description == NULL || (p_sph_msg) == NULL) { \
            break; \
        } \
        if ((p_sph_msg)->buffer_type == SPH_MSG_BUFFER_TYPE_MAILBOX) { \
            ALOGX("%s(), %s, id: 0x%x, param16: 0x%x, param32: 0x%x", \
                  __FUNCTION__, description, \
                  (p_sph_msg)->msg_id,  (p_sph_msg)->param_16bit, (p_sph_msg)->param_32bit); \
        } else if ((p_sph_msg)->buffer_type == SPH_MSG_BUFFER_TYPE_PAYLOAD) { \
            ALOGX("%s(), %s, id: 0x%x, type: %d, size: %u, addr: %p", \
                  __FUNCTION__, description, \
                  (p_sph_msg)->msg_id, (p_sph_msg)->payload_data_type, \
                  (p_sph_msg)->payload_data_size, (p_sph_msg)->payload_data_addr); \
        } else { \
            ALOGW("%s(), buffer_type %d not supporty!!", \
                  __FUNCTION__, (p_sph_msg)->buffer_type); \
        } \
    } while (0)



#define CONFIG_THREAD(thread_name, android_priority) \
    do { \
        snprintf(thread_name, sizeof(thread_name), "%s_%d_%d", __FUNCTION__, getpid(), gettid()); \
        prctl(PR_SET_NAME, (unsigned long)thread_name, 0, 0, 0); \
        int retval = setpriority(PRIO_PROCESS, 0, android_priority); \
        if (retval != 0) { \
            ALOGE("thread %s created. setpriority %s failed!! errno: %d, retval: %d", \
                  thread_name, #android_priority, errno, retval); \
        } else { \
            SLOG_ENG("thread %s created. setpriority %s done", \
                     thread_name, #android_priority); \
        } \
    } while(0)



} /* end namespace android */

#endif /* end of ANDROID_SPEECH_UTILITY_H */


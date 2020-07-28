
#ifndef AUDIO_DSP_SERVICE_HANDLER_H
#define AUDIO_DSP_SERVICE_HANDLER_H

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
 * =============================================================================
 *                     typedef
 * =============================================================================
 */

/* adsp Core ID definition*/
enum adsp_core_id {
    ADSP_A_ID = 0,
    ADSP_B_ID = 1,
    ADSP_CORE_TOTAL
};


/* adsp feature PRI list */
enum adsp_feature_pri {
    ADSP_LOGGER_FEATURE_PRI       = 0,
    SPK_PROTECT_FEATURE_PRI,
    A2DP_PLAYBACK_FEATURE_PRI,
    AURISYS_FEATURE_PRI,
    DEEPBUF_FEATURE_PRI,
    OFFLOAD_FEATURE_PRI,
    PRIMARY_FEATURE_PRI,
    VOIP_FEATURE_PRI,
    CAPTURE_UL1_FEATURE_PRI,
    AUDIO_PLAYBACK_FEATURE_PRI,
    VOICE_CALL_FEATURE_PRI,
    AUDIO_CONTROLLER_FEATURE_PRI,
    SYSTEM_FEATURE_PRI,
};


enum ADSP_CUR_STATUS {
    ADSP_STATUS_RESET = 0x00,
    ADSP_STATUS_SUSPEND = 0x01,
    ADSP_STATUS_SLEEP = 0x10,
    ADSP_STATUS_ACTIVE = 0x11,
};

enum ADSP_NOTIFY_EVENT {
    ADSP_EVENT_STOP = 0,
    ADSP_EVENT_READY = 1,
};

enum { /* adsp_feature_id */
    SYSTEM_FEATURE_ID             = 0,
    ADSP_LOGGER_FEATURE_ID        = 1,
    AURISYS_FEATURE_ID            = 10,
    AUDIO_CONTROLLER_FEATURE_ID,
    PRIMARY_FEATURE_ID            = 20,
    DEEPBUF_FEATURE_ID,
    OFFLOAD_FEATURE_ID,
    AUDIO_PLAYBACK_FEATURE_ID,
    A2DP_PLAYBACK_FEATURE_ID,
    AUDIO_DATAPROVIDER_FEATURE_ID,
    SPK_PROTECT_FEATURE_ID,
    VOICE_CALL_FEATURE_ID,
    VOIP_FEATURE_ID,
    CAPTURE_UL1_FEATURE_ID,
    CALL_FINAL_FEATURE_ID,
    KTV_FEATURE_ID,
    ADSP_NUM_FEATURE_ID,
};

/*
 * =============================================================================
 *                     hook function
 * =============================================================================
 */

typedef void (*adsp_start_cbk_t)(void *arg);
typedef void (*adsp_stop_cbk_t)(void *arg);



/*
 * =============================================================================
 *                     public functions - declaration
 * =============================================================================
 */

void audio_dsp_service_init(void);
void audio_dsp_service_deinit(void);

void adsp_register_feature(const uint16_t feature_id);
void adsp_deregister_feature(const uint16_t feature_id);
void adsp_cbk_register(const uint16_t feature_id,
                       adsp_start_cbk_t start_cbk,
                       adsp_stop_cbk_t stop_cbk,
                       unsigned int priority,
                       void *arg);
void adsp_cbk_deregister(const uint16_t feature_id);
int is_adsp_ready(int core_id);

#ifdef __cplusplus
}  /* extern "C" */
#endif

#endif /* end of AUDIO_DSP_SERVICE_HANDLER_H */

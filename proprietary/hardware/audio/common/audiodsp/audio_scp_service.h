
#ifndef AUDIO_SCP_SERVICE_HANDLER_H
#define AUDIO_SCP_SERVICE_HANDLER_H

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

enum SCP_NOTIFY_EVENT {
    SCP_EVENT_READY = 0,
    SCP_EVENT_STOP,
};

/*
 * =============================================================================
 *                     hook function
 * =============================================================================
 */

typedef void (*scp_start_cbk_t)(void *arg);
typedef void (*scp_stop_cbk_t)(void *arg);



/*
 * =============================================================================
 *                     public functions - declaration
 * =============================================================================
 */

void audio_scp_service_init(void);
void audio_scp_service_deinit(void);

void audio_scp_cbk_register(scp_start_cbk_t start_cbk,
                            scp_stop_cbk_t stop_cbk,
                            void *arg);
void audio_scp_cbk_deregister(void);
int is_scp_ready(void);

#ifdef __cplusplus
}  /* extern "C" */
#endif

#endif /* end of AUDIO_SCP_SERVICE_HANDLER_H */

#ifndef MTK_AUDIO_DSP_CONTROLLER_H
#define MTK_AUDIO_DSP_CONTROLLER_H

#include <stdint.h>
#include <stdbool.h>


#ifdef __cplusplus
extern "C" {
#endif



/*
 * =============================================================================
 *                     ref struct
 * =============================================================================
 */

struct alock_t;


/*
 * =============================================================================
 *                     MACRO
 * =============================================================================
 */

#define MAX_RECOVERY_LOCK_TIMEOUT_MS (10000)


/*
 * =============================================================================
 *                     typedef
 * =============================================================================
 */

enum { /* dsp_id_t */
    AUDIO_OPENDSP_USE_CM4_A, /* => SCP_A_ID */
    AUDIO_OPENDSP_USE_CM4_B, /* => SCP_B_ID */
    AUDIO_OPENDSP_USE_HIFI3_A, /* => ADSP_A_ID */
    AUDIO_OPENDSP_USE_HIFI3_B, /* => ADSP_B_ID */
    NUM_OPENDSP_TYPE,
    AUDIO_OPENDSP_ID_INVALID
};



/*
 * =============================================================================
 *                     struct definition
 * =============================================================================
 */


/*
 * =============================================================================
 *                     hook function
 * =============================================================================
 */

typedef void (*audio_dsp_start_cbk_t)(void *arg);
typedef void (*audio_dsp_stop_cbk_t)(void *arg);


/*
 * =============================================================================
 *                     public function
 * =============================================================================
 */

void audio_dsp_controller_init(void);
void audio_dsp_controller_deinit(void);

void audio_dsp_cbk_register(
    audio_dsp_start_cbk_t start_cbk,
    audio_dsp_stop_cbk_t stop_cbk,
    void *arg);

struct alock_t *get_adsp_recovery_lock(void);

int32_t add_audio_dsp_recovery_lock_cnt(void);
int32_t dec_audio_dsp_recovery_lock_cnt(void);
int32_t get_audio_dsp_recovery_lock_cnt(void);

void set_audio_dsp_recovery_mode(const bool recovery_mode);
bool get_audio_dsp_recovery_mode(void);



#ifdef __cplusplus
}  /* extern "C" */
#endif

#endif /* end of MTK_AUDIO_DSP_CONTROLLER_H */


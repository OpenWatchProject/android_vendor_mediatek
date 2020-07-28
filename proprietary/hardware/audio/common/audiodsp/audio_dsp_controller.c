#include <audio_dsp_controller.h>

#include <cutils/atomic.h>

#include <audio_log.h>
#include <audio_lock.h>

#include <audio_task.h>
#include <errno.h>

#if defined(MTK_AUDIODSP_SUPPORT)
#include <audio_dsp_service.h>
#endif
#if defined(MTK_AUDIO_SCP_SUPPORT)
#include <audio_scp_service.h>
#endif


#ifdef __cplusplus
extern "C" {
#endif


#ifdef LOG_TAG
#undef LOG_TAG
#endif
#define LOG_TAG "audio_dsp_controller"



/*
 * =============================================================================
 *                     MACRO
 * =============================================================================
 */


/*
 * =============================================================================
 *                     typedef
 * =============================================================================
 */

struct audio_dsp_cbk_info_t {
    audio_dsp_start_cbk_t start_cbk;
    audio_dsp_stop_cbk_t  stop_cbk;
    void *arg;
};



/*
 * =============================================================================
 *                     private global members
 * =============================================================================
 */

static struct alock_t *g_dsp_controller_lock;

static bool g_dsp_recovery_mode[NUM_OPENDSP_TYPE]; /* single dsp status */
static bool g_recovery_mode; /* overall dsp status by client */

static uint32_t g_recovery_count; /* all dsp fail count */

static uint32_t g_opendsp_id[NUM_OPENDSP_TYPE]; /* for arg to opendsp_id */

static struct alock_t *g_dsp_recovery_lock;
static volatile int32_t g_dsp_recovery_lock_cnt;

static struct audio_dsp_cbk_info_t g_cbk_info;



/*
 * =============================================================================
 *                     private function implementation
 * =============================================================================
 */

static void audio_dsp_ready_event(void *arg) {
    uint32_t *p_opendsp_id = NULL;
    uint32_t opendsp_id = AUDIO_OPENDSP_ID_INVALID;

    p_opendsp_id = (uint32_t *)arg;
    if (p_opendsp_id == NULL) {
        AUD_LOG_E("%s(), p_opendsp_id == NULL!! arg %p", __FUNCTION__, arg);
        return;
    }
    opendsp_id = *p_opendsp_id;
    if (opendsp_id >= NUM_OPENDSP_TYPE) {
        AUD_LOG_E("%s(), opendsp_id %u error!!", __FUNCTION__, opendsp_id);
        return;
    }

    LOCK_ALOCK_MS(g_dsp_controller_lock, MAX_RECOVERY_LOCK_TIMEOUT_MS);

    if (g_dsp_recovery_mode[opendsp_id] == false) {
        AUD_LOG_E("%s(), current g_dsp_recovery_mode[%u] %d duplicated!! bypass",
                  __FUNCTION__,
                  opendsp_id, g_dsp_recovery_mode[opendsp_id]);
        UNLOCK_ALOCK(g_dsp_controller_lock);
        return;
    }
    AUD_LOG_D("%s(), g_dsp_recovery_mode[%u] %d => 0, old g_recovery_count %u",
              __FUNCTION__,
              opendsp_id,
              g_dsp_recovery_mode[opendsp_id],
              g_recovery_count);
    g_dsp_recovery_mode[opendsp_id] = false;

    if (g_recovery_count == 0) {
        AUD_LOG_E("%s(), g_recovery_count %u error!!",
                  __FUNCTION__, g_recovery_count);
        UNLOCK_ALOCK(g_dsp_controller_lock);
        return;
    }
    g_recovery_count--;

    if (g_recovery_count == 0) {
        if (g_cbk_info.start_cbk != NULL) {
            g_cbk_info.start_cbk(g_cbk_info.arg);
        }
    }

    UNLOCK_ALOCK(g_dsp_controller_lock);
}


static void audio_dsp_stop_event(void *arg) {
    uint32_t *p_opendsp_id = NULL;
    uint32_t opendsp_id = AUDIO_OPENDSP_ID_INVALID;

    p_opendsp_id = (uint32_t *)arg;
    if (p_opendsp_id == NULL) {
        AUD_LOG_E("%s(), p_opendsp_id == NULL!! arg %p", __FUNCTION__, arg);
        return;
    }
    opendsp_id = *p_opendsp_id;
    if (opendsp_id >= NUM_OPENDSP_TYPE) {
        AUD_LOG_E("%s(), opendsp_id %u error!!", __FUNCTION__, opendsp_id);
        return;
    }

    LOCK_ALOCK_MS(g_dsp_controller_lock, MAX_RECOVERY_LOCK_TIMEOUT_MS);

    if (g_dsp_recovery_mode[opendsp_id] == true) {
        AUD_LOG_E("%s(), current g_dsp_recovery_mode[%u] %d duplicated!! bypass",
                  __FUNCTION__,
                  opendsp_id, g_dsp_recovery_mode[opendsp_id]);
        UNLOCK_ALOCK(g_dsp_controller_lock);
        return;
    }
    AUD_LOG_D("%s(), g_dsp_recovery_mode[%u] %d => 1, old g_recovery_count %u",
              __FUNCTION__,
              opendsp_id,
              g_dsp_recovery_mode[opendsp_id],
              g_recovery_count);
    g_dsp_recovery_mode[opendsp_id] = true;

    g_recovery_count++;
    if (g_recovery_count == 1) {
        if (g_cbk_info.stop_cbk != NULL) {
            g_cbk_info.stop_cbk(g_cbk_info.arg);
        }
    }

    UNLOCK_ALOCK(g_dsp_controller_lock);
}


static bool audio_dsp_need_recovery(int core_id) {
    int adsp_status = is_adsp_ready(core_id);

    if (adsp_status == 1) {
        return false;
    } else if (adsp_status == -EINVAL) {
        return false; //no support this adsp, no need recovery
    }
    return true;
}


/*
 * =============================================================================
 *                     public function implementation
 * =============================================================================
 */

void audio_dsp_controller_init(void) {
    uint32_t opendsp_id = AUDIO_OPENDSP_ID_INVALID;

    NEW_ALOCK(g_dsp_controller_lock);
    NEW_ALOCK(g_dsp_recovery_lock);

    LOCK_ALOCK_MS(g_dsp_controller_lock, MAX_RECOVERY_LOCK_TIMEOUT_MS);

    g_dsp_recovery_lock_cnt = 0;
    g_recovery_count = 0;


    for (opendsp_id = 0; opendsp_id < NUM_OPENDSP_TYPE; opendsp_id++) {
        g_dsp_recovery_mode[opendsp_id] = false;
        g_opendsp_id[opendsp_id] = opendsp_id;
    }

    memset(&g_cbk_info, 0, sizeof(g_cbk_info));


    /* query adsp status when boot / HAL reinit */
#ifdef MTK_AUDIODSP_SUPPORT
    adsp_register_feature(AUDIO_CONTROLLER_FEATURE_ID);
    g_dsp_recovery_mode[AUDIO_OPENDSP_USE_HIFI3_A] =
        audio_dsp_need_recovery(ADSP_A_ID);
    g_dsp_recovery_mode[AUDIO_OPENDSP_USE_HIFI3_B] =
        audio_dsp_need_recovery(ADSP_B_ID);
    adsp_deregister_feature(AUDIO_CONTROLLER_FEATURE_ID);

    /* only use core 0 as nofifier for HiFi3 */
    adsp_cbk_register(
        AUDIO_CONTROLLER_FEATURE_ID,
        audio_dsp_ready_event,
        audio_dsp_stop_event,
        AUDIO_CONTROLLER_FEATURE_PRI,
        &g_opendsp_id[AUDIO_OPENDSP_USE_HIFI3_A]);
#endif

#if defined(MTK_AUDIO_SCP_RECOVERY_SUPPORT)
    g_dsp_recovery_mode[AUDIO_OPENDSP_USE_CM4_A] =
        (is_scp_ready() == 1) ? false : true;

    audio_scp_cbk_register(
        audio_dsp_ready_event,
        audio_dsp_stop_event,
        &g_opendsp_id[AUDIO_OPENDSP_USE_CM4_A]);
#endif

    for (opendsp_id = 0; opendsp_id < NUM_OPENDSP_TYPE; opendsp_id++) {
        if (g_dsp_recovery_mode[opendsp_id] == true) {
            AUD_LOG_W("%s(), g_dsp_recovery_mode[%u] = %d", __FUNCTION__,
                      opendsp_id, g_dsp_recovery_mode[opendsp_id]);
            g_recovery_count++;
        }
    }

    AUD_LOG_D("%s(), g_recovery_count: %u", __FUNCTION__, g_recovery_count);
    UNLOCK_ALOCK(g_dsp_controller_lock);
}


void audio_dsp_controller_deinit(void) {
    FREE_ALOCK(g_dsp_recovery_lock);
    FREE_ALOCK(g_dsp_controller_lock);
}


void audio_dsp_cbk_register(
    audio_dsp_start_cbk_t start_cbk,
    audio_dsp_stop_cbk_t stop_cbk,
    void *arg) {
    AUD_LOG_D("%s(), start %p => %p, stop %p => %p, arg %p => %p", __FUNCTION__,
              g_cbk_info.start_cbk, start_cbk,
              g_cbk_info.stop_cbk, stop_cbk,
              g_cbk_info.arg, arg);

    g_cbk_info.start_cbk = start_cbk;
    g_cbk_info.stop_cbk = stop_cbk;
    g_cbk_info.arg = arg;
}


struct alock_t *get_adsp_recovery_lock(void) {
    AUD_ASSERT(g_dsp_recovery_lock != NULL);

    return g_dsp_recovery_lock;
}


int32_t add_audio_dsp_recovery_lock_cnt(void) {
    return android_atomic_inc(&g_dsp_recovery_lock_cnt);
}


int32_t dec_audio_dsp_recovery_lock_cnt(void) {
    return android_atomic_dec(&g_dsp_recovery_lock_cnt);
}


int32_t get_audio_dsp_recovery_lock_cnt(void) {
    return g_dsp_recovery_lock_cnt;
}


void set_audio_dsp_recovery_mode(const bool recovery_mode) {
#ifdef CONFIG_MT_ENG_BUILD // only for bug detection when development
    if (LOCK_ALOCK_TRYLOCK(get_adsp_recovery_lock()) == 0) {
        UNLOCK_ALOCK(get_adsp_recovery_lock());
        AUD_WARNING("get mode w/o lock");
    }
#endif
    AUD_LOG_D("%s(), recovery_mode: %d => %d", __FUNCTION__,
              g_recovery_mode, recovery_mode);
    g_recovery_mode = recovery_mode;
}


bool get_audio_dsp_recovery_mode(void) {
#ifdef CONFIG_MT_ENG_BUILD // only for bug detection when development
    if (LOCK_ALOCK_TRYLOCK(get_adsp_recovery_lock()) == 0) {
        UNLOCK_ALOCK(get_adsp_recovery_lock());
        AUD_WARNING("get mode w/o lock");
    }
#endif
    return g_recovery_mode;
}



#ifdef __cplusplus
}  /* extern "C" */
#endif


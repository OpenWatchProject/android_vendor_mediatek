#include <stdlib.h>
#include <string.h>

#include <errno.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <unistd.h>

#include <pthread.h>



#include <audio_log.h>
#include <audio_lock.h>
#include <audio_assert.h>
#include <audio_task.h>
#include <audio_dsp_service.h>



#ifdef __cplusplus
extern "C" {
#endif


/*
 * =============================================================================
 *                     LOG
 * =============================================================================
 */

#ifdef LOG_TAG
#undef LOG_TAG
#endif
#define LOG_TAG "adsp_service"



/*
 * =============================================================================
 *                     MACRO
 * =============================================================================
 */


/*==============================================================================
 *                     ioctl
 *============================================================================*/

#define AUDIO_DSP_DEVICE_PATH "/dev/adsp"
#define AUDIO_DSP_IOC_MAGIC 'a'

#define AUDIO_DSP_IOCTL_ADSP_REG_FEATURE _IOW(AUDIO_DSP_IOC_MAGIC, 0, unsigned int)
#define AUDIO_DSP_IOCTL_ADSP_QUERY_STATUS \
    _IOR(AUDIO_DSP_IOC_MAGIC, 1, unsigned int)
#define AUDIO_DSP_IOCTL_ADSP_RESET_CBK \
    _IOR(AUDIO_DSP_IOC_MAGIC, 2, unsigned int)


/*
 * =============================================================================
 *                     struct def
 * =============================================================================
 */

struct g_dsp_feature_t {
    /* constructor/destructor */
    adsp_start_cbk_t start_cbk;
    adsp_stop_cbk_t stop_cbk;
    unsigned int priority;
    int fid;
    void *arg;
};

struct audio_dsp_reg_feature_t {
    uint16_t reg_flag;
    uint16_t feature_id;
};
struct audio_dsp_query_status_t {
    int16_t ready_flag;
    uint16_t core_id;
};
/*
 * =============================================================================
 *                     global var
 * =============================================================================
 */

static struct alock_t *g_dsp_drv_lock;

static int g_adsp_drv;

static pthread_t g_read_adsp_thread_hdl;
bool   g_read_adsp_thread_enable;

static struct g_dsp_feature_t g_dsp_feature[ADSP_NUM_FEATURE_ID];

/*
 * =============================================================================
 *                     function declaration
 * =============================================================================
 */

static void *read_adsp_calbak_thread(void *arg);
static int compare_pri(const void *a, const void *b);


/*==============================================================================
 *                     public functions - implementation
 *============================================================================*/
static int compare_pri(const void *a, const void *b) {
    struct g_dsp_feature_t *orderA = (struct g_dsp_feature_t *)a;
    struct g_dsp_feature_t *orderB = (struct g_dsp_feature_t *)b;

    return (orderB->priority - orderA->priority);
}

int is_adsp_ready(int core_id) {
    int ret = 0;
    struct audio_dsp_query_status_t adsp_query;
    adsp_query.core_id = core_id;
    adsp_query.ready_flag = 0;
    LOCK_ALOCK(g_dsp_drv_lock);
    if (g_adsp_drv < 0) {
        AUD_LOG_E("%s(), driver %s file node %d!!", __FUNCTION__,
                  AUDIO_DSP_DEVICE_PATH, g_adsp_drv);
        UNLOCK_ALOCK(g_dsp_drv_lock);
        return -1;
    }
    ret = ioctl(g_adsp_drv, AUDIO_DSP_IOCTL_ADSP_QUERY_STATUS, &adsp_query);
    if (ret != 0) {
        AUD_LOG_E("%s() ioctl fail! ret = %d, errno: %d, ready_flag: %d",
                  __FUNCTION__, ret, errno, adsp_query.ready_flag);
        UNLOCK_ALOCK(g_dsp_drv_lock);
        return -errno;
    }
    if (adsp_query.ready_flag != 1) {
        AUD_LOG_W("%s() adsp not ready! ret: %d, errno: %d, ready_flag: %d",
                  __FUNCTION__, ret, errno, adsp_query.ready_flag);
    }
    UNLOCK_ALOCK(g_dsp_drv_lock);
    return adsp_query.ready_flag;
}

void adsp_register_feature(const uint16_t feature_id) {
    struct audio_dsp_reg_feature_t feature_reg;
    int ret = 0;

    LOCK_ALOCK(g_dsp_drv_lock);
    if (g_adsp_drv < 0) {
        AUD_LOG_E("%s(), driver %s file node %d!!", __FUNCTION__,
                  AUDIO_DSP_DEVICE_PATH, g_adsp_drv);
        goto EXIT;
    }

    feature_reg.feature_id = feature_id;
    feature_reg.reg_flag = 1;

    ret = ioctl(g_adsp_drv, AUDIO_DSP_IOCTL_ADSP_REG_FEATURE, &feature_reg);
    if (ret != 0) {
        AUD_LOG_E("%s() ioctl fail! ret = %d, errno: %d", __FUNCTION__, ret, errno);
    }
EXIT:
    UNLOCK_ALOCK(g_dsp_drv_lock);
}

void adsp_deregister_feature(const uint16_t feature_id) {
    struct audio_dsp_reg_feature_t feature_reg;
    int ret = 0;

    LOCK_ALOCK(g_dsp_drv_lock);
    if (g_adsp_drv < 0) {
        AUD_LOG_E("%s(), driver %s file node %d!!", __FUNCTION__,
                  AUDIO_DSP_DEVICE_PATH, g_adsp_drv);
        goto EXIT;
    }

    feature_reg.feature_id = feature_id;
    feature_reg.reg_flag = 0;

    ret = ioctl(g_adsp_drv, AUDIO_DSP_IOCTL_ADSP_REG_FEATURE, &feature_reg);
    if (ret != 0) {
        AUD_LOG_E("%s() ioctl fail! ret = %d, errno: %d", __FUNCTION__, ret, errno);
    }
EXIT:
    UNLOCK_ALOCK(g_dsp_drv_lock);
}



void adsp_cbk_register(
    const uint16_t feature_id,
    adsp_start_cbk_t start_cbk,
    adsp_stop_cbk_t stop_cbk,
    unsigned int priority,
    void *arg) {
    AUD_LOG_D("%s(), feature_id %u, start_cbk %p, stop_cbk %p, priority %u arg %p", __FUNCTION__,
              feature_id, start_cbk, stop_cbk, priority, arg);
    int i = 0;

    if (feature_id >= ADSP_NUM_FEATURE_ID) {
        return;
    }
    if (start_cbk == NULL || stop_cbk == NULL) {
        return;
    }

    LOCK_ALOCK(g_dsp_drv_lock);
    if (g_dsp_feature[feature_id].start_cbk != NULL ||
        g_dsp_feature[feature_id].stop_cbk != NULL) {
        AUD_LOG_W("%s(), feature_id %u cbk exist!!", __FUNCTION__, feature_id);
        goto EXIT;
    }
    g_dsp_feature[feature_id].start_cbk = start_cbk;
    g_dsp_feature[feature_id].stop_cbk = stop_cbk;
    g_dsp_feature[feature_id].priority = priority;
    g_dsp_feature[feature_id].fid = feature_id;
    g_dsp_feature[feature_id].arg = arg;
    // sort here
    qsort((void *)g_dsp_feature, ADSP_NUM_FEATURE_ID, sizeof(g_dsp_feature[0]), compare_pri);
    for (i = 0; i < ADSP_NUM_FEATURE_ID; i++) {
        if (g_dsp_feature[i].start_cbk != NULL && g_dsp_feature[i].stop_cbk != NULL) {
            AUD_LOG_D("%s(),%d start_cbk = %p priority %d", __FUNCTION__,
                      g_dsp_feature[i].fid, g_dsp_feature[i].start_cbk,
                      g_dsp_feature[i].priority);
        }
    }

EXIT:
    UNLOCK_ALOCK(g_dsp_drv_lock);
}

void adsp_cbk_deregister(const uint16_t feature_id) {
    int ret = 0;
    int num = 0;

    AUD_LOG_D("%s(), feature_id %u", __FUNCTION__, feature_id);

    if (feature_id >= ADSP_NUM_FEATURE_ID) {
        return;
    }

    LOCK_ALOCK(g_dsp_drv_lock);
    for (num = 0; num < ADSP_NUM_FEATURE_ID; num++) {
        if (g_dsp_feature[num].fid == feature_id) {
            break;
        }
    }
    if (num == ADSP_NUM_FEATURE_ID) {
            AUD_LOG_W("%s(), feature_id %u not found!!", __FUNCTION__, feature_id);
            goto EXIT;
    }
    if (g_dsp_feature[num].start_cbk == NULL ||
        g_dsp_feature[num].stop_cbk == NULL) {
        AUD_LOG_W("%s(), feature_id %u cbk null!!", __FUNCTION__, feature_id);
        goto EXIT;
    }
    g_dsp_feature[num].start_cbk = NULL;
    g_dsp_feature[num].stop_cbk = NULL;
    g_dsp_feature[num].priority = 0;
    g_dsp_feature[num].fid = -1;
    g_dsp_feature[num].arg = NULL;

EXIT:
    UNLOCK_ALOCK(g_dsp_drv_lock);
}

void audio_dsp_service_init(void) {
    int ret = 0;
    int i = 0;


    NEW_ALOCK(g_dsp_drv_lock);

    /* init var */
    for (i = 0; i < ADSP_NUM_FEATURE_ID; i++) {
        g_dsp_feature[i].start_cbk = NULL;
        g_dsp_feature[i].stop_cbk = NULL;
        g_dsp_feature[i].priority = 0;
        g_dsp_feature[i].fid = -1;
        g_dsp_feature[i].arg = NULL;
    }

    /* open driver */
    LOCK_ALOCK(g_dsp_drv_lock);
    g_adsp_drv = open(AUDIO_DSP_DEVICE_PATH, O_RDONLY);
    if (g_adsp_drv < 0) {
        AUD_LOG_E("%s(), fail to open %s, errno: %d", __FUNCTION__,
                  AUDIO_DSP_DEVICE_PATH, errno);
        AUD_WARNING("open audio ipi driver fail!!");
    }
    UNLOCK_ALOCK(g_dsp_drv_lock);

    /* read adsp thread */
    g_read_adsp_thread_hdl = 0;
    g_read_adsp_thread_enable = true;

    ret = pthread_create(&g_read_adsp_thread_hdl, NULL,
                         read_adsp_calbak_thread,
                         NULL);
    AUD_ASSERT(ret == 0);
}

void audio_dsp_service_deinit(void) {

    LOCK_ALOCK(g_dsp_drv_lock);
    /* read ipi thread */
    g_read_adsp_thread_enable = false;

    /* close driver */
    if (g_adsp_drv >= 0) {
        close(g_adsp_drv);
        g_adsp_drv = -1;
    }

    UNLOCK_ALOCK(g_dsp_drv_lock);
    FREE_ALOCK(g_dsp_drv_lock);
}


void *read_adsp_calbak_thread(void *arg  __unused) {

    int msg_size = 0;
    int data_size = 0;
    int length_read = 0;
    int ret = 0;
    int i = 0;
    unsigned int magic[2];

    pthread_detach(pthread_self());

    while (g_read_adsp_thread_enable) {
        /* ioctrl & lock here util callback */
        magic[0] = rand();
        magic[1] = 0xFFFFFFFF - magic[0];
        ret = ioctl(g_adsp_drv, AUDIO_DSP_IOCTL_ADSP_RESET_CBK, &magic);

        if (ret < 0) {
            AUD_LOG_W("%s(), read adsp status fail!! ret: %d errno: %d",
                      __FUNCTION__, ret, errno);
            usleep(2000 * 1000); //2s
            continue;
        }
        if (ret == ADSP_EVENT_READY) {
            /* Call StreamManager API */
            for (i = 0; i < ADSP_NUM_FEATURE_ID; i++) {
                if (g_dsp_feature[i].start_cbk != NULL) {
                    AUD_LOG_D("%s, event ready, feature id = %d", __FUNCTION__, g_dsp_feature[i].fid);
                    g_dsp_feature[i].start_cbk(g_dsp_feature[i].arg);
                }
            }
            AUD_LOG_D("%s, event ready", __FUNCTION__);
        } else {
            /* Call StreamManager API */
            for (i = 0; i < ADSP_NUM_FEATURE_ID; i++) {
                if (g_dsp_feature[i].stop_cbk != NULL) {
                    AUD_LOG_D("%s, event stop, feature id = %d", __FUNCTION__, g_dsp_feature[i].fid);
                    g_dsp_feature[i].stop_cbk(g_dsp_feature[i].arg);
                }
            }
            AUD_LOG_D("%s, event stop", __FUNCTION__);
        }
    }

    pthread_exit(NULL);
    return NULL;
}

#ifdef __cplusplus
}  /* extern "C" */
#endif
